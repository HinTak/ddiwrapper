// Copyright 2004 Ulrich Hecht <uli@suse.de>

// This file is part of ddiwrapper.

// ddiwrapper is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#include <windows.h>
#include <winnt.h>
#include "winddi.h"

#include <msvcrt/stdio.h>

#include "common.h"

#include <unistd.h>


VOID (*pDrvDisableDriver)(VOID);
BOOL (*pDrvEnableDriver)(IN ULONG iEngineVersion, IN ULONG cj, OUT DRVENABLEDATA *pded);
BOOL (*pDrvQueryDriverInfo)(DWORD  dwMode, PVOID  pBuffer, DWORD  cbBuf, PDWORD  pcbNeeded);
LONG (*pDrvDocumentPropertySheets)(void*  pPSUIInfo, LPARAM  lParam);

// needed when requesting a properly initialized DEVMODEW structure from the interface DLL
// unfortunately not contained in the w32api headers
typedef struct _DOCUMENTPROPERTYHEADER {
    WORD        cbSize;
    WORD        Reserved;
    HANDLE      hPrinter;
    LPTSTR      pszPrinterName;
    PDEVMODE    pdmIn;
    PDEVMODE    pdmOut;
    DWORD       cbOut;
    DWORD       fMode;
} DOCUMENTPROPERTYHEADER, *PDOCUMENTPROPERTYHEADER;

// instances of the function pointers declared in common.h
PFN_DrvEnablePDEV pDrvEnablePDEV = 0;
PFN_DrvResetPDEV pDrvResetPDEV = 0;
PFN_DrvCompletePDEV pDrvCompletePDEV = 0;
PFN_DrvDisablePDEV pDrvDisablePDEV = 0;
PFN_DrvEnableSurface pDrvEnableSurface = 0;
PFN_DrvDisableSurface pDrvDisableSurface = 0;
PFN_DrvStartDoc pDrvStartDoc = 0;
PFN_DrvStartPage pDrvStartPage = 0;
PFN_DrvSendPage pDrvSendPage = 0;
PFN_DrvEndDoc pDrvEndDoc = 0;
PFN_DrvStartBanding pDrvStartBanding = 0;
PFN_DrvNextBand pDrvNextBand = 0;
PFN_DrvDitherColor pDrvDitherColor = 0;
PFN_DrvBitBlt pDrvBitBlt = 0;
PFN_DrvCopyBits pDrvCopyBits = 0;
PFN_DrvStretchBlt pDrvStretchBlt = 0;
PFN_DrvStretchBltROP pDrvStretchBltROP = 0;
PFN_DrvPlgBlt pDrvPlgBlt = 0;
PFN_DrvAlphaBlend pDrvAlphaBlend = 0;
PFN_DrvGradientFill pDrvGradientFill = 0;
PFN_DrvTransparentBlt pDrvTransparentBlt = 0;
PFN_DrvTextOut pDrvTextOut = 0;
PFN_DrvStrokePath pDrvStrokePath = 0;
PFN_DrvFillPath pDrvFillPath = 0;
PFN_DrvStrokeAndFillPath pDrvStrokeAndFillPath = 0;
PFN_DrvPaint pDrvPaint = 0;
PFN_DrvLineTo pDrvLineTo = 0;
PFN_DrvIcmCreateColorTransform pDrvIcmCreateColorTransform = 0;
PFN_DrvIcmDeleteColorTransform pDrvIcmDeleteColorTransform = 0;
PFN_DrvIcmCheckBitmapBits pDrvIcmCheckBitmapBits = 0;
PFN_DrvEscape pDrvEscape = 0;

// loads the driver and initializes the DEVMODEW (device parameters) structure
// and the Drv* function pointers
void load_driver(void)
{
  HINSTANCE dll;
  DRVENABLEDATA ded;
  int i;
  
  // load the graphics DLL, i.e. the actual printer driver
  DEBUG(3,"loading graphics dll\n");
  dll=LoadLibraryW(c->driver_path);
  if(!dll)
  {
    ERRMSG("failed to load driver, error %ld\n",GetLastError());
    exit(76);
  }
  
  // get the defined API functions
  pDrvDisableDriver=(void*)GetProcAddress(dll,"DrvDisableDriver");
  if(!pDrvDisableDriver) { ERRMSG("No DrvDisableDriver export in graphics DLL\n"); exit(77); }
  pDrvEnableDriver=(void*)GetProcAddress(dll,"DrvEnableDriver");
  if(!pDrvEnableDriver) { ERRMSG("No DrvEnableDriver export in graphics DLL\n"); exit(78); }
  pDrvQueryDriverInfo=(void*)GetProcAddress(dll,"DrvQueryDriverInfo");
  if(!pDrvQueryDriverInfo) { ERRMSG("No DrvQueryDriverInfo export in graphics DLL\n"); exit(79); }
  
  DEBUG(3,"%p\n%p\n%p\n",pDrvDisableDriver,pDrvEnableDriver,pDrvQueryDriverInfo);
  
  int dmsize=0;
#if 0
  // This is the correct way to find out the size of the DEVMODEW structure including the driver's
  // private members and to get a valid initial setup for it. Unfortunately, this info comes from
  // the interface DLL, which drags the entire GDI with it.
  // Canon printer drivers seem to be happy with a zero-initialized device mode structure, so
  // this is disabled for now to avoid having to bring our own full GDI implementation. Other
  // drivers are not so tolerant, however. You absolutely want to enable this if you try to get
  // a non-Canon driver to work.
  // One possible solution is to take the entire GDI32 DLL from WINE and add the additional stuff
  // implemented in our GDI32 DLL; this, however, is a maintenance nightmare. Another solution
  // might be to write code relaying all unimplemented calls from our GDI32 to WINE's.

  DEBUG(3,"loading interface dll\n");

  HINSTANCE idll;
  idll=LoadLibraryW(c->config_path);
  if(!idll)
  {
    ERRMSG("failed to load interface DLL, error %ld\n",GetLastError());
    exit(82);
  }
  
  pDrvDocumentPropertySheets=(void*)GetProcAddress(idll,"DrvDocumentPropertySheets");
  if(!pDrvDocumentPropertySheets) { ERRMSG("No DrvDocumentPropertySheets export in interface DLL\n"); exit(83); }
  
  DOCUMENTPROPERTYHEADER dph;
  dph.cbSize=sizeof(dph);
  dph.fMode=0;
  dph.pdmOut=0;
  dmsize=pDrvDocumentPropertySheets(NULL, (LPARAM)&dph);
  DEBUG(2,"DEVMODEW size %d\n",dmsize);
  c->dm=malloc(dmsize);
  dph.pdmIn=NULL;
  dph.pdmOut=(PDEVMODE)c->dm;
  dph.fMode=DM_OUT_BUFFER;
  //asm("int $3");
  pDrvDocumentPropertySheets(NULL, (LPARAM)&dph);
#else
  dmsize=8192;	// Methinks the Canon drivers I have tested reported something around 5000 bytes.
  c->dm=calloc(1,dmsize);
#endif

  // copy the parameters set at the command-line to the DEVMODEW structure supplied by the driver
  memcpy(c->dm,&c->cdm,sizeof(c->cdm));
  c->dm->dmDriverExtra=dmsize-sizeof(c->cdm);
  
  int mode = 0xabcdef94;	// set to a silly value so we can check if the driver set the value at all

  // check if this is a user-mode or kernel-mode driver; it is probably safe to assume that kernel-mode
  // drivers will not work with the current code, but then I have yet to see a Windows XP kernel mode
  // printer driver.
  DWORD needed;
  pDrvQueryDriverInfo(DRVQUERY_USERMODE, (PVOID)&mode, sizeof(mode), &needed);
  switch(mode)
  {
    case TRUE:
      DEBUG(1,"User-mode driver\n");
      break;
    case FALSE:
      DEBUG(1,"Kernel-mode driver\n");
      break;
    default:
      DEBUG(1,"Unknown mode driver\n");
      break;
  }
  
  // initialize the driver, get the array containing the pointers to the Drv* functions
  if(!pDrvEnableDriver(DDI_DRIVER_VERSION_NT5_01, sizeof(ded), &ded))
  {
    ERRMSG("DrvEnableDriver failed\n");
    exit(1);
  }
  
  DEBUG(1,"iDriverVersion 0x%lx\n",ded.iDriverVersion);
  DEBUG(2,"DRVFN array size %lu\n",ded.c);

  // initialize the function pointers
  for(i=0;i<ded.c;i++)
  {
    DEBUG(3,"DRVFN function %lu at %p\n",ded.pdrvfn[i].iFunc,ded.pdrvfn[i].pfn);
    switch(ded.pdrvfn[i].iFunc)
    {
      case INDEX_DrvEnablePDEV: pDrvEnablePDEV = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvResetPDEV: pDrvResetPDEV = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvCompletePDEV: pDrvCompletePDEV = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvDisablePDEV: pDrvDisablePDEV = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvEnableSurface: pDrvEnableSurface = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvDisableSurface: pDrvDisableSurface = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvStartDoc: pDrvStartDoc = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvStartPage: pDrvStartPage = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvSendPage: pDrvSendPage = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvEndDoc: pDrvEndDoc = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvStartBanding: pDrvStartBanding = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvNextBand: pDrvNextBand = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvDitherColor: pDrvDitherColor = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvBitBlt: pDrvBitBlt = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvCopyBits: pDrvCopyBits = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvStretchBlt: pDrvStretchBlt = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvStretchBltROP: pDrvStretchBltROP = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvPlgBlt: pDrvPlgBlt = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvAlphaBlend: pDrvAlphaBlend = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvGradientFill: pDrvGradientFill = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvTransparentBlt: pDrvTransparentBlt = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvTextOut: pDrvTextOut = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvStrokePath: pDrvStrokePath = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvFillPath: pDrvFillPath = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvStrokeAndFillPath: pDrvStrokeAndFillPath = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvPaint: pDrvPaint = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvLineTo: pDrvLineTo = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvIcmCreateColorTransform: pDrvIcmCreateColorTransform = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvIcmDeleteColorTransform: pDrvIcmDeleteColorTransform = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvIcmCheckBitmapBits: pDrvIcmCheckBitmapBits = (void*) ded.pdrvfn[i].pfn; break;
      case INDEX_DrvEscape: pDrvEscape = (void*) ded.pdrvfn[i].pfn; break;
      default: DEBUG(1,"Unknown function %lu\n",ded.pdrvfn[i].iFunc); break;
    }
    
  }
}

// creates a shared memory segment containing the global configuration structure, better known as "c".
// FIXME: This code assumes that getpid() returns the same value for all threads of a process. This is
// a guess; if Win32 (or rather, WINE) behaves differently, this may fail for multi-threaded drivers.
void load_config(void)
{
  // construct a name for our shared-mem segment
  char config_name[40];
  sprintf(config_name,"gdiprint%d",getpid());

  //fprintf(stderr,"config_name in load_config %s\n",config_name);

  // create it
  HANDLE fm;
  fm=CreateFileMapping(INVALID_HANDLE_VALUE,
                       NULL,
                       PAGE_READWRITE,
                       0,
                       sizeof(config),
                       config_name);
  if(!fm)
  {
    fprintf(stderr,"Could not map config structure\n");
    exit(44);
  }

  // map it to our address space
  c=MapViewOfFile(fm,FILE_MAP_ALL_ACCESS,0,0,sizeof(config));
  if(!c)
  {
    fprintf(stderr,"Could not get view of config structure\n");
    exit(45);
  }
}
