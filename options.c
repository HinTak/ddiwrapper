// Copyright 2004 Ulrich Hecht <uli@suse.de>

// This file is part of ddiwrapper.

// ddiwrapper is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License version 2.1 as
// published by the Free Software Foundation.

#include <windows.h>
#include <winnt.h>
#include "winddi.h"

#include <msvcrt/stdio.h>

#include "common.h"

#include <unicode.h>
#include <getopt.h>

static LPSTR driverprefix=0;	// prefix prepended to the driver/config/data file names

// convert wide character slash to backslash
static WCHAR slashback(WCHAR c)
{
  if(c=='/') return '\\';
  else return c;
}

// appends a narrow string to a wide string
static VOID strcat2unicode(LPWSTR dst, LPSTR src)
{
  //printf("wcslen %d\n",wcslen(dst));
  dst+=wcslen(dst);
  while((*dst++=slashback((WCHAR)*src++)));
}

// converts a narrow UNIX-style path to a wide DOS-style path, assuming that the Windows Z:
// drive is the UNIX root directory
static LPWSTR ascii_unix_to_unicode_zdos_path(LPSTR path)
{
  int i;
  LPWSTR newstr=malloc((strlen(path)				// driver path
                        +4					// z:, zero and possibly an additional backslash
                        +(driverprefix?strlen(driverprefix):0)	// driver path prefix
                        )*2);					// unicode
  newstr[0]='z'; newstr[1]=':'; newstr[2]=0;
  if(driverprefix)
  {
    strcat2unicode(newstr,driverprefix);
    int l=wcslen(newstr)-1;
    if(newstr[l]!='\\')
    {
      newstr[++l]='\\';
      newstr[++l]=0;
    }
  }
  for(i=wcslen(newstr);*path;i++,path++)
  {
      newstr[i]=slashback((WCHAR)*path);
  }
  newstr[i]=0;
  return newstr;
}

// copied from WINE, we do not want to link to shell32.dll
// converts a wide command-line string to a wide string argv[] array
static LPWSTR* myCommandLineToArgvW(LPCWSTR lpCmdline, int* numargs)
{
    DWORD argc;
    HGLOBAL hargv;
    LPWSTR  *argv;
    LPCWSTR cs;
    LPWSTR arg,s,d;
    LPWSTR cmdline;
    int in_quotes,bcount;

    if (*lpCmdline==0) {
        /* Return the path to the executable */
        DWORD len, size=16;

        hargv=GlobalAlloc(size, 0);
	argv=GlobalLock(hargv);
	for (;;) {
            len = GetModuleFileNameW(0, (LPWSTR)(argv+1), size-sizeof(LPWSTR));
            if (!len) {
                GlobalFree(hargv);
                return NULL;
            }
            if (len < size) break;
            size*=2;
            hargv=GlobalReAlloc(hargv, size, 0);
            argv=GlobalLock(hargv);
        }
        argv[0]=(LPWSTR)(argv+1);
        if (numargs)
            *numargs=2;

        return argv;
    }

    /* to get a writeable copy */
    argc=0;
    bcount=0;
    in_quotes=0;
    cs=lpCmdline;
    while (1) {
        if (*cs==0 || ((*cs==0x0009 || *cs==0x0020) && !in_quotes)) {
            /* space */
            argc++;
            /* skip the remaining spaces */
            while (*cs==0x0009 || *cs==0x0020) {
                cs++;
            }
            if (*cs==0)
                break;
            bcount=0;
            continue;
        } else if (*cs==0x005c) {
            /* '\', count them */
            bcount++;
        } else if ((*cs==0x0022) && ((bcount & 1)==0)) {
            /* unescaped '"' */
            in_quotes=!in_quotes;
            bcount=0;
        } else {
            /* a regular character */
            bcount=0;
        }
        cs++;
    }
    /* Allocate in a single lump, the string array, and the strings that go with it.
     * This way the caller can make a single GlobalFree call to free both, as per MSDN.
     */
    hargv=GlobalAlloc(0, argc*sizeof(LPWSTR)+(strlenW(lpCmdline)+1)*sizeof(WCHAR));
    argv=GlobalLock(hargv);
    if (!argv)
        return NULL;
    cmdline=(LPWSTR)(argv+argc);
    strcpyW(cmdline, lpCmdline);

    argc=0;
    bcount=0;
    in_quotes=0;
    arg=d=s=cmdline;
    while (*s) {
        if ((*s==0x0009 || *s==0x0020) && !in_quotes) {
            /* Close the argument and copy it */
            *d=0;
            argv[argc++]=arg;

            /* skip the remaining spaces */
            do {
                s++;
            } while (*s==0x0009 || *s==0x0020);

            /* Start with a new argument */
            arg=d=s;
            bcount=0;
        } else if (*s==0x005c) {
            /* '\\' */
            *d++=*s++;
            bcount++;
        } else if (*s==0x0022) {
            /* '"' */
            if ((bcount & 1)==0) {
                /* Preceeded by an even number of '\', this is half that
                 * number of '\', plus a quote which we erase.
                 */
                d-=bcount/2;
                in_quotes=!in_quotes;
                s++;
            } else {
                /* Preceeded by an odd number of '\', this is half that
                 * number of '\' followed by a '"'
                 */
                d=d-bcount/2-1;
                *d++='"';
                s++;
            }
            bcount=0;
        } else {
            /* a regular character */
            *d++=*s++;
            bcount=0;
        }
    }
    if (*arg) {
        *d='\0';
        argv[argc++]=arg;
    }
    if (numargs)
        *numargs=argc;

    return argv;
}

static void usage(char* name)
{
  fprintf(stderr,
"Usage: %s [OPTION]...\n"
"Convert bitmap data (BMP format, 24-bit uncompressed) to printer code with\n"
"the help of a Windows 2000/XP printer driver. Reads data from a file or\n"
"standard input, writes data to a file or standard output.\n"
"Example: ddiwrapper `cat /path/to/driver/parms` --input heidi.bmp >/dev/usblp0\n"
"\n"
"Windows driver paths:\n"
"  --DriverPrefix           path to prepend to all other driver paths\n"
"  --DriverPath             path to printer graphics DLL\n"
"  --ConfigPath             path to printer interface DLL\n"
"  --DataPath               path to printer driver data file\n"
"\n"
"Output control:\n"
"  --PrintQuality           printing quality, 'draft', 'low', 'medium',\n"
"                           or 'high'\n"
"  --Duplex                 double-sided printing, 'simplex' or 'horizontal'\n"
"  --Color                  'color' or 'mono'\n"
"  --MediaType              paper type, 'standard', 'glossy', or 'transparent'\n"
"  --PaperSize              paper format, 'letter', 'lettersmall', 'tabloid',\n"
"                           'ledger', 'legal', 'statement', 'executive', 'a3',\n"
"                           'a4'', 'a4small', 'a5', 'b5', 'folio', '10x14',\n"
"                           '11x17', 'note', 'env_9', 'env_10', 'env_11',\n"
"                           'env_12', 'env_14', 'csheet', 'dsheet', 'esheet',\n"
"                           'env_dl', 'env_c5', 'env_c3', 'env_c4', 'env_c6',\n"
"                           'env_c65', 'env_b4', 'env_b5', 'env_b6',\n"
"                           'env_italy', 'env_monarch', 'env_personal',\n"
"                           'fanfold_us', 'fanfold_std_german',\n"
"                           'fanfold_lgl_german', 'iso_b4', 'b4',\n"
"                           'japanese_postcard', '9x11', '10x11', '15x11',\n"
"                           'env_invite', 'letter_extra', 'tabloid_extra',\n"
"                           'a4_extra', 'letter_transverse', 'a_plus',\n"
"                           'b_plus', 'letter_plus', 'a4_plus',\n"
"                           'a5_transverse', 'b5_transverse', 'a3_extra',\n"
"                           'a5_extra', 'b5_extra', 'a2', 'a3_transverse',\n"
"                           'a3_extra_transverse', 'dbl_japanese_postcard',\n"
"                           'a6', 'jenv_kaku2', 'jenv_kaku3', 'jenv_chou3',\n"
"                           'jenv_chou4', 'letter_rotated', 'a3_rotated',\n"
"                           'a4_rotated', 'a5_rotated', 'b4_jis_rotated',\n"
"                           'b5_jis_rotated', 'japanese_postcard_rotated',\n"
"                           'dbl_japanese_postcard_rotated', 'a6_rotated',\n"
"                           'jenv_kaku2_rotated', 'jenv_kaku3_rotated',\n"
"                           'jenv_chou3_rotated', 'jenv_chou4_rotated',\n"
"                           'b6_jis', 'b6_jis_rotated', '12x11', 'jenv_you4',\n"
"                           'jenv_you4_rotated', 'p16k', 'p32k', 'p32kbig',\n"
"                           'p32kbig_rotated', 'penv_1_rotated',\n"
"                           'penv_2_rotated', 'penv_3_rotated',\n"
"                           'penv_4_rotated', 'penv_5_rotated',\n"
"                           'penv_6_rotated', 'penv_7_rotated',\n"
"                           'penv_8_rotated', 'penv_9_rotated', or\n"
"                           'penv_10_rotated'\n"
"Miscellaneous:\n"
"  --input                  BMP input file, defaults to standard input\n"
"  --output                 printer output file/device, defaults to standard\n"
"                           output\n"
"  --test                   prints the input resolution the driver expects for\n"
"                           the given set of options\n"
"\n"
"The BMP input has to have the horizontal resolution the driver expects for\n"
"the given set of options. If the input consists of multiple BMPs, their\n"
"vertical sizes have to be as expected by the driver as well. Exit code is 0\n"
"if the input has been processed successfully. Other values indicate an error.\n"
  ,name);
}

// parse command-line options and set up config structure accordingly
void parse_options()
{
  // set up argc and argv suitable for getopt_long()
  int argc;
  char** argv;
  argv=(char**)myCommandLineToArgvW(GetCommandLineW(),&argc);

  // convert wide strings to char strings
  int i,j;
  for(i=0;i<argc;i++)
  {
    for(j=0;j<wcslen((LPCWSTR)(argv[i]));j++)
      argv[i][j]=argv[i][j*2];
    argv[i][j]=0;
  }
  
  // init config structure with default values
  memset(c,0,sizeof(*c));
  c->gdihandle=(HANDLE)0xcafe0001;
  c->spoolhandle=(HANDLE)0xcafe0002;
  c->verbosity=0;
  c->printer_name=L"Canon Pixma iP3000";	// Canon drivers do not seem to care much about this
                                                // Epson drivers, e.g., construct paths to files from this string
  c->environment=L"Windows NT x86";

  c->logaddr=L"NULL1";	// DOS output device, would be something like LPT1 in real life
  c->devname=L"GDI Printer";	// not sure if any drivers use this internally; Canon drivers seem not to
  
  // initialize DEVMODEW structure with some sane default values
  c->cdm.dmSpecVersion=DM_SPECVERSION;
  c->cdm.dmSize=sizeof(c->cdm);
  c->cdm.dmFields=DM_DEFAULTSOURCE|DM_PRINTQUALITY|DM_PAPERSIZE|DM_ORIENTATION|DM_COLOR;
  c->cdm.dmDefaultSource=DMBIN_FORMSOURCE;
  c->cdm.dmPrintQuality=DMRES_HIGH;
  c->cdm.dmPaperSize=DMPAPER_A4;
  c->cdm.dmOrientation=DMORIENT_PORTRAIT;
  c->cdm.dmDriverExtra=0;
  c->cdm.dmColor=DMCOLOR_COLOR;
  
  // zero-init devinfo and gdiinfo; MS docs say that drivers can expect it to be this way
  memset(&c->devinfo,0,sizeof(c->devinfo));
  memset(&c->gdiinfo,0,sizeof(c->gdiinfo));
  
  int option;	// getopt_long() return value
  
  // option parsing loop
  for(;;)
  {
    int option_index=0;
    static struct option long_options[] =
    {
      {"PrintQuality", 1, 0, 0},
#if 0
      {"Orientation", 1, 0, 0},
#endif
      {"Duplex", 1, 0, 0},
      {"Color", 1, 0, 0},
      {"MediaType", 1, 0, 0},
      {"PaperSize", 1, 0, 0},
      {"DriverPath", 1, 0, 0},
      {"DataPath", 1, 0, 0},
      {"ConfigPath", 1, 0, 0},
      {"DriverPrefix", 1, 0, 0},
      {"test", 0, 0, 0}, 
      {"input", 1, 0, 0},
      {"output", 1, 0, 0},
      {0,0,0,0}
    };
    
    option=getopt_long(argc,argv,"v0",long_options,&option_index);
    if(option==-1) break;
    
    switch(option)
    {
    case 'v':
      c->verbosity++;
      break;

    case 0:	// long option
    
#define IFARG(x) if(!strcmp(long_options[option_index].name, (x)))
#define IFOPTARG(x) if(!strcmp(optarg,x))
#define ARGERR(x) do { ERRMSG("Invalid " x "argument %s\n",optarg); exit(57); } while(0)
    
      IFARG("PrintQuality")
      {
        if(atoi(optarg)>50)	// literal resolution in DPI
        {
          c->cdm.dmPrintQuality=atoi(optarg);
          c->cdm.dmYResolution=atoi(optarg);
        }
        else			// symbolic resolutions
        {
          IFOPTARG("high") c->cdm.dmPrintQuality=DMRES_HIGH;
          else IFOPTARG("medium") c->cdm.dmPrintQuality=DMRES_MEDIUM;
          else IFOPTARG("low") c->cdm.dmPrintQuality=DMRES_LOW;
          else IFOPTARG("draft") c->cdm.dmPrintQuality=DMRES_DRAFT;
          else ARGERR("PrintQuality");
        }
        c->cdm.dmFields|=DM_PRINTQUALITY;
      }
      
#if 0
      // landscape mode is unimplemented because it does not make much sense when operating as a
      // filter as it would require seeking in the input file
      IFARG("Orientation")
      {
        IFOPTARG("portrait") c->cdm.dmOrientation=DMORIENT_PORTRAIT;
        else IFOPTARG("landscape") c->cdm.dmOrientation=DMORIENT_LANDSCAPE;
        c->cdm.dmFields|=DM_ORIENTATION;
      }
#endif
      IFARG("Color")
      {
        IFOPTARG("color") c->cdm.dmColor=DMCOLOR_COLOR;
        else IFOPTARG("mono") c->cdm.dmColor=DMCOLOR_MONOCHROME;
        else ARGERR("Color");
        c->cdm.dmFields|=DM_COLOR;
      }
      
      IFARG("Duplex")
      {
        IFOPTARG("simplex") c->cdm.dmDuplex=DMDUP_SIMPLEX;
        else IFOPTARG("horizontal") c->cdm.dmDuplex=DMDUP_HORIZONTAL;
#if 0
        // when printing duplex vertically the driver wants the input upside
        // down which is not easy to do when reading from a pipe; it is much
        // easier to do this outside ddiwrapper, see ps2ddi for an example
        else IFOPTARG("vertical") c->cdm.dmDuplex=DMDUP_VERTICAL;
#endif
        else ARGERR("Duplex");
        c->cdm.dmFields|=DM_DUPLEX;
      }
      
      IFARG("MediaType")
      {
        IFOPTARG("standard") c->cdm.dmMediaType=DMMEDIA_STANDARD;
        else IFOPTARG("glossy") c->cdm.dmMediaType=DMMEDIA_GLOSSY;
        else IFOPTARG("transparent") c->cdm.dmMediaType=DMMEDIA_TRANSPARENCY;
        else ARGERR("MediaType");
        c->cdm.dmFields|=DM_MEDIATYPE;
      }
      
      IFARG("PaperSize")
      {
        IFOPTARG("letter") c->cdm.dmPaperSize=DMPAPER_LETTER;
        else IFOPTARG("lettersmall") c->cdm.dmPaperSize=DMPAPER_LETTERSMALL;
        else IFOPTARG("tabloid") c->cdm.dmPaperSize=DMPAPER_TABLOID;
        else IFOPTARG("ledger") c->cdm.dmPaperSize=DMPAPER_LEDGER;
        else IFOPTARG("legal") c->cdm.dmPaperSize=DMPAPER_LEGAL;
        else IFOPTARG("statement") c->cdm.dmPaperSize=DMPAPER_STATEMENT;
        else IFOPTARG("executive") c->cdm.dmPaperSize=DMPAPER_EXECUTIVE;
        else IFOPTARG("a3") c->cdm.dmPaperSize=DMPAPER_A3;
        else IFOPTARG("a4") c->cdm.dmPaperSize=DMPAPER_A4;
        else IFOPTARG("a4small") c->cdm.dmPaperSize=DMPAPER_A4SMALL;
        else IFOPTARG("a5") c->cdm.dmPaperSize=DMPAPER_A5;
        else IFOPTARG("b5") c->cdm.dmPaperSize=DMPAPER_B5;
        else IFOPTARG("folio") c->cdm.dmPaperSize=DMPAPER_FOLIO;
        else IFOPTARG("10x14") c->cdm.dmPaperSize=DMPAPER_10X14;
        else IFOPTARG("11x17") c->cdm.dmPaperSize=DMPAPER_11X17;
        else IFOPTARG("note") c->cdm.dmPaperSize=DMPAPER_NOTE;
        else IFOPTARG("env_9") c->cdm.dmPaperSize=DMPAPER_ENV_9;
        else IFOPTARG("env_10") c->cdm.dmPaperSize=DMPAPER_ENV_10;
        else IFOPTARG("env_11") c->cdm.dmPaperSize=DMPAPER_ENV_11;
        else IFOPTARG("env_12") c->cdm.dmPaperSize=DMPAPER_ENV_12;
        else IFOPTARG("env_14") c->cdm.dmPaperSize=DMPAPER_ENV_14;
        else IFOPTARG("csheet") c->cdm.dmPaperSize=DMPAPER_CSHEET;
        else IFOPTARG("dsheet") c->cdm.dmPaperSize=DMPAPER_DSHEET;
        else IFOPTARG("esheet") c->cdm.dmPaperSize=DMPAPER_ESHEET;
        else IFOPTARG("env_dl") c->cdm.dmPaperSize=DMPAPER_ENV_DL;
        else IFOPTARG("env_c5") c->cdm.dmPaperSize=DMPAPER_ENV_C5;
        else IFOPTARG("env_c3") c->cdm.dmPaperSize=DMPAPER_ENV_C3;
        else IFOPTARG("env_c4") c->cdm.dmPaperSize=DMPAPER_ENV_C4;
        else IFOPTARG("env_c6") c->cdm.dmPaperSize=DMPAPER_ENV_C6;
        else IFOPTARG("env_c65") c->cdm.dmPaperSize=DMPAPER_ENV_C65;
        else IFOPTARG("env_b4") c->cdm.dmPaperSize=DMPAPER_ENV_B4;
        else IFOPTARG("env_b5") c->cdm.dmPaperSize=DMPAPER_ENV_B5;
        else IFOPTARG("env_b6") c->cdm.dmPaperSize=DMPAPER_ENV_B6;
        else IFOPTARG("env_italy") c->cdm.dmPaperSize=DMPAPER_ENV_ITALY;
        else IFOPTARG("env_monarch") c->cdm.dmPaperSize=DMPAPER_ENV_MONARCH;
        else IFOPTARG("env_personal") c->cdm.dmPaperSize=DMPAPER_ENV_PERSONAL;
        else IFOPTARG("fanfold_us") c->cdm.dmPaperSize=DMPAPER_FANFOLD_US;
        else IFOPTARG("fanfold_std_german") c->cdm.dmPaperSize=DMPAPER_FANFOLD_STD_GERMAN;
        else IFOPTARG("fanfold_lgl_german") c->cdm.dmPaperSize=DMPAPER_FANFOLD_LGL_GERMAN;
        else IFOPTARG("iso_b4") c->cdm.dmPaperSize=DMPAPER_ISO_B4;
        else IFOPTARG("b4") c->cdm.dmPaperSize=DMPAPER_ISO_B4;
        else IFOPTARG("japanese_postcard") c->cdm.dmPaperSize=DMPAPER_JAPANESE_POSTCARD;
        else IFOPTARG("9x11") c->cdm.dmPaperSize=DMPAPER_9X11;
        else IFOPTARG("10x11") c->cdm.dmPaperSize=DMPAPER_10X11;
        else IFOPTARG("15x11") c->cdm.dmPaperSize=DMPAPER_15X11;
        else IFOPTARG("env_invite") c->cdm.dmPaperSize=DMPAPER_ENV_INVITE;
        else IFOPTARG("letter_extra") c->cdm.dmPaperSize=DMPAPER_LETTER_EXTRA;
        else IFOPTARG("tabloid_extra") c->cdm.dmPaperSize=DMPAPER_TABLOID_EXTRA;
        else IFOPTARG("a4_extra") c->cdm.dmPaperSize=DMPAPER_A4_EXTRA;
        else IFOPTARG("letter_transverse") c->cdm.dmPaperSize=DMPAPER_LETTER_TRANSVERSE;
        else IFOPTARG("a_plus") c->cdm.dmPaperSize=DMPAPER_A_PLUS;
        else IFOPTARG("b_plus") c->cdm.dmPaperSize=DMPAPER_B_PLUS;
        else IFOPTARG("letter_plus") c->cdm.dmPaperSize=DMPAPER_LETTER_PLUS;
        else IFOPTARG("a4_plus") c->cdm.dmPaperSize=DMPAPER_A4_PLUS;
        else IFOPTARG("a5_transverse") c->cdm.dmPaperSize=DMPAPER_A5_TRANSVERSE;
        else IFOPTARG("b5_transverse") c->cdm.dmPaperSize=DMPAPER_B5_TRANSVERSE;
        else IFOPTARG("a3_extra") c->cdm.dmPaperSize=DMPAPER_A3_EXTRA;
        else IFOPTARG("a5_extra") c->cdm.dmPaperSize=DMPAPER_A5_EXTRA;
        else IFOPTARG("b5_extra") c->cdm.dmPaperSize=DMPAPER_B5_EXTRA;
        else IFOPTARG("a2") c->cdm.dmPaperSize=DMPAPER_A2;
        else IFOPTARG("a3_transverse") c->cdm.dmPaperSize=DMPAPER_A3_TRANSVERSE;
        else IFOPTARG("a3_extra_transverse") c->cdm.dmPaperSize=DMPAPER_A3_EXTRA_TRANSVERSE;
        else IFOPTARG("dbl_japanese_postcard") c->cdm.dmPaperSize=DMPAPER_DBL_JAPANESE_POSTCARD;
        else IFOPTARG("a6") c->cdm.dmPaperSize=DMPAPER_A6;
        else IFOPTARG("jenv_kaku2") c->cdm.dmPaperSize=DMPAPER_JENV_KAKU2;
        else IFOPTARG("jenv_kaku3") c->cdm.dmPaperSize=DMPAPER_JENV_KAKU3;
        else IFOPTARG("jenv_chou3") c->cdm.dmPaperSize=DMPAPER_JENV_CHOU3;
        else IFOPTARG("jenv_chou4") c->cdm.dmPaperSize=DMPAPER_JENV_CHOU4;
        else IFOPTARG("letter_rotated") c->cdm.dmPaperSize=DMPAPER_LETTER_ROTATED;
        else IFOPTARG("a3_rotated") c->cdm.dmPaperSize=DMPAPER_A3_ROTATED;
        else IFOPTARG("a4_rotated") c->cdm.dmPaperSize=DMPAPER_A4_ROTATED;
        else IFOPTARG("a5_rotated") c->cdm.dmPaperSize=DMPAPER_A5_ROTATED;
        else IFOPTARG("b4_jis_rotated") c->cdm.dmPaperSize=DMPAPER_B4_JIS_ROTATED;
        else IFOPTARG("b5_jis_rotated") c->cdm.dmPaperSize=DMPAPER_B5_JIS_ROTATED;
        else IFOPTARG("japanese_postcard_rotated") c->cdm.dmPaperSize=DMPAPER_JAPANESE_POSTCARD_ROTATED;
        else IFOPTARG("dbl_japanese_postcard_rotated") c->cdm.dmPaperSize=DMPAPER_DBL_JAPANESE_POSTCARD_ROTATED;
        else IFOPTARG("a6_rotated") c->cdm.dmPaperSize=DMPAPER_A6_ROTATED;
        else IFOPTARG("jenv_kaku2_rotated") c->cdm.dmPaperSize=DMPAPER_JENV_KAKU2_ROTATED;
        else IFOPTARG("jenv_kaku3_rotated") c->cdm.dmPaperSize=DMPAPER_JENV_KAKU3_ROTATED;
        else IFOPTARG("jenv_chou3_rotated") c->cdm.dmPaperSize=DMPAPER_JENV_CHOU3_ROTATED;
        else IFOPTARG("jenv_chou4_rotated") c->cdm.dmPaperSize=DMPAPER_JENV_CHOU4_ROTATED;
        else IFOPTARG("b6_jis") c->cdm.dmPaperSize=DMPAPER_B6_JIS;
        else IFOPTARG("b6_jis_rotated") c->cdm.dmPaperSize=DMPAPER_B6_JIS_ROTATED;
        else IFOPTARG("12x11") c->cdm.dmPaperSize=DMPAPER_12X11;
        else IFOPTARG("jenv_you4") c->cdm.dmPaperSize=DMPAPER_JENV_YOU4;
        else IFOPTARG("jenv_you4_rotated") c->cdm.dmPaperSize=DMPAPER_JENV_YOU4_ROTATED;
        else IFOPTARG("p16k") c->cdm.dmPaperSize=DMPAPER_P16K;
        else IFOPTARG("p32k") c->cdm.dmPaperSize=DMPAPER_P32K;
        else IFOPTARG("p32kbig") c->cdm.dmPaperSize=DMPAPER_P32KBIG;
        else IFOPTARG("p32kbig_rotated") c->cdm.dmPaperSize=DMPAPER_P32KBIG_ROTATED;
        else IFOPTARG("penv_1_rotated") c->cdm.dmPaperSize=DMPAPER_PENV_1_ROTATED;
        else IFOPTARG("penv_2_rotated") c->cdm.dmPaperSize=DMPAPER_PENV_2_ROTATED;
        else IFOPTARG("penv_3_rotated") c->cdm.dmPaperSize=DMPAPER_PENV_3_ROTATED;
        else IFOPTARG("penv_4_rotated") c->cdm.dmPaperSize=DMPAPER_PENV_4_ROTATED;
        else IFOPTARG("penv_5_rotated") c->cdm.dmPaperSize=DMPAPER_PENV_5_ROTATED;
        else IFOPTARG("penv_6_rotated") c->cdm.dmPaperSize=DMPAPER_PENV_6_ROTATED;
        else IFOPTARG("penv_7_rotated") c->cdm.dmPaperSize=DMPAPER_PENV_7_ROTATED;
        else IFOPTARG("penv_8_rotated") c->cdm.dmPaperSize=DMPAPER_PENV_8_ROTATED;
        else IFOPTARG("penv_9_rotated") c->cdm.dmPaperSize=DMPAPER_PENV_9_ROTATED;
        else IFOPTARG("penv_10_rotated") c->cdm.dmPaperSize=DMPAPER_PENV_10_ROTATED;
        else ARGERR("PaperSize");
      }
      
      IFARG("DriverPath") c->driver_path=ascii_unix_to_unicode_zdos_path(optarg);
      IFARG("DataPath") c->data_path=ascii_unix_to_unicode_zdos_path(optarg);
      IFARG("ConfigPath") c->config_path=ascii_unix_to_unicode_zdos_path(optarg);
      
      IFARG("DriverPrefix") driverprefix=optarg;
      
      IFARG("test") c->test_mode=TRUE;
      IFARG("input") c->input_file=strdup(optarg);
      IFARG("output") c->output_file=strdup(optarg);
      
      break;

    default:
      usage(argv[0]);
      exit(56);
    }

  }

  GlobalFree(argv);
}
