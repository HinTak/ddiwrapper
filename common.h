// Copyright 2004 Ulrich Hecht <uli@suse.de>

// This file is part of ddiwrapper.

// ddiwrapper is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#include <unistd.h>	// needed for getpid() prototype

extern VOID (*pDrvDisableDriver)(VOID);
extern BOOL (*pDrvEnableDriver)(IN ULONG iEngineVersion, IN ULONG cj, OUT DRVENABLEDATA *pded);
extern BOOL (*pDrvQueryDriverInfo)(DWORD  dwMode, PVOID  pBuffer, DWORD  cbBuf, PDWORD  pcbNeeded);

// Drv* function pointers, initialized in load_driver()
extern PFN_DrvEnablePDEV pDrvEnablePDEV;
extern PFN_DrvResetPDEV pDrvResetPDEV;
extern PFN_DrvCompletePDEV pDrvCompletePDEV;
extern PFN_DrvDisablePDEV pDrvDisablePDEV;
extern PFN_DrvEnableSurface pDrvEnableSurface;
extern PFN_DrvDisableSurface pDrvDisableSurface;
extern PFN_DrvStartDoc pDrvStartDoc;
extern PFN_DrvStartPage pDrvStartPage;
extern PFN_DrvSendPage pDrvSendPage;
extern PFN_DrvEndDoc pDrvEndDoc;
extern PFN_DrvStartBanding pDrvStartBanding;
extern PFN_DrvNextBand pDrvNextBand;
extern PFN_DrvDitherColor pDrvDitherColor;
extern PFN_DrvBitBlt pDrvBitBlt;
extern PFN_DrvCopyBits pDrvCopyBits;
extern PFN_DrvStretchBlt pDrvStretchBlt;
extern PFN_DrvStretchBltROP pDrvStretchBltROP;
extern PFN_DrvPlgBlt pDrvPlgBlt;
extern PFN_DrvAlphaBlend pDrvAlphaBlend;
extern PFN_DrvGradientFill pDrvGradientFill;
extern PFN_DrvTransparentBlt pDrvTransparentBlt;
extern PFN_DrvTextOut pDrvTextOut;
extern PFN_DrvStrokePath pDrvStrokePath;
extern PFN_DrvFillPath pDrvFillPath;
extern PFN_DrvStrokeAndFillPath pDrvStrokeAndFillPath;
extern PFN_DrvPaint pDrvPaint;
extern PFN_DrvLineTo pDrvLineTo;
extern PFN_DrvIcmCreateColorTransform pDrvIcmCreateColorTransform;
extern PFN_DrvIcmDeleteColorTransform pDrvIcmDeleteColorTransform;
extern PFN_DrvIcmCheckBitmapBits pDrvIcmCheckBitmapBits;
extern PFN_DrvEscape pDrvEscape;

void load_driver(void);		// load the printer driver, init c->dm and function pointers
void load_config(void);		// create a config structure shared memory segment
void parse_options(void);	// parse options and initialize config structure
void render_document(void);	// does the printing

void dumpmem(void* mem, int size);	// for debugging
char* uni2ascii(WCHAR* s);		// converts a wide string to a narrow string

// global configuration structure
typedef struct
{
  SURFOBJ* so;	// surface to paint to

  DHPDEV pdev;	// our printer's physical device handle
  HANDLE spoolhandle;	// winspool printer handle
  HANDLE gdihandle;	// GDI handle passed to Eng* functions

  int verbosity;	// used by the DEBUG macro and in several other places
  LPWSTR printer_name;	// printer name
  LPWSTR environment; // environment, e.g. "Windows NT x86"
  LPWSTR driver_path;	// filename with path of printer graphics DLL (driver)
  LPWSTR data_path;	// filename with path of data file (in DRIVER_INFO structure)
  LPWSTR config_path;	// filename with path of printer interface DLL (UI)

  LPWSTR logaddr;	// system device name, e.g. LPT1
  HSURF phsurfpats[HS_DDI_MAX];		// standard fill patterns supplied by driver
  LPWSTR devname;	// user-readable printer name

  DEVMODEW cdm;		// device mode (public members only), set up by options parser
  DEVMODEW* dm;		// device mode including private members; contains most print job parameters
  GDIINFO gdiinfo;	// filled in by the driver when calling DrvEnablePDEV()
  DEVINFO devinfo;	// filled in by the driver when calling DrvEnablePDEV()
  
  BOOL test_mode;	// if true, print the dimension and resolution the driver expects as input, then exit
  LPSTR input_file;	// input data; use stdin if NULL
  LPSTR output_file;	// output data; use stdout if NULL
  
  ULONG color_mode;	// color format information, set by EngCreatePalette
} config;

// How to access the config structure depends on whether we are in the main program or a DLL.
// In the first case, there is simply a global pointer "c" initialized by the load_config()
// function. In the latter case we have to create a new mapping of the shared memory segment
// and use that.
#ifdef DLL
config* c;

// This function is executed at the time the DLL is loaded.
// FIXME: Same problem with the shared segment name as in load_config()
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
  // construct shared memory segment name
  char config_name[40];
  sprintf(config_name,"gdiprint%d",getpid());

  // get a handle to the mapping
  HANDLE fm=CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(config),config_name);
  if(!fm)
  {
    fprintf(stderr,"Could not map config structure\n");
    exit(48);
  }

  // map it to our address space
  c=MapViewOfFile(fm,FILE_MAP_ALL_ACCESS,0,0,sizeof(config));
  if(!c)
  {
    fprintf(stderr,"Could not create view of config structure\n");
    exit(49);
  }

  // We expect this to already exist, created and initialized by the main program. If it
  // does not exist already, something's seriously wrong.
  if(GetLastError()!=ERROR_ALREADY_EXISTS)
  {
    fprintf(stderr,"WAAAAAAAAAAAAAAAH! config_name %s\n",config_name);
    exit(50);
  }

  //fprintf(stderr,"config %p in DllMain, name %s\n",c,config_name);
  
  return TRUE;
}
#else
extern config* c;
#endif

#define DEBUG(x,y...) do { if(c->verbosity>=(x)) fprintf(stderr,y); } while(0);
#define ERRMSG(x...) DEBUG(0,x)
