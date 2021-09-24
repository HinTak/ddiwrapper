// Copyright 2004 Ulrich Hecht <uli@suse.de>

// This file is part of ddiwrapper.

// ddiwrapper is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License version 2.1 as
// published by the Free Software Foundation.

#include <windows.h>
#include <winnt.h>
#include "winddi.h"

#include <msvcrt/stdio.h>
#include <msvcrt/fcntl.h>

#include "common.h"

#include <unistd.h>


config* c;

int WINAPI WinMain(HINSTANCE eins, HINSTANCE zwei, LPSTR arg, int whatever)
{
  // put stdin and stdout in binary mode
  // This is _very_ important as WINE will otherwise convert between LF and CRLF ...
  setmode(STDIN_FILENO,O_BINARY);
  setmode(STDOUT_FILENO,O_BINARY);
  
  load_config();

  parse_options();
  //asm("int $3");
  DEBUG(4,"config in WinMain %p, PID %d\n",c, getpid());

  load_driver();
  
  //asm("int $3");
  c->pdev=pDrvEnablePDEV(c->dm,	// driver data
                      c->logaddr,	// system device name (LPT1 etc.)
                      HS_DDI_MAX,	// number of patterns
                      c->phsurfpats,	// standard fill patterns array
                      sizeof(c->gdiinfo),
                      (ULONG*)&c->gdiinfo,		// device capabilities
                      sizeof(c->devinfo),
                      &c->devinfo,		// device info
                      c->gdihandle,	// pseudo-handle driver passes to GDI
                      c->devname,	// device name
                      c->spoolhandle);	// pseudo-handle driver passes to winspool
  
  if(!c->pdev)
  {
    ERRMSG("DrvEnablePDEV failed\n");
    exit(1);
  }
  else
    DEBUG(3,"pdev %p\n",c->pdev);

  // test mode: print out the dimensions and resolution the driver expects, then exit
  if(c->test_mode)
  {
    printf("XSIZE=%ld\n",c->gdiinfo.ulHorzRes);
    printf("YSIZE=%ld\n",c->gdiinfo.ulVertRes);
    printf("XRES=%ld\n",c->gdiinfo.ulLogPixelsX);
    printf("YRES=%ld\n",c->gdiinfo.ulLogPixelsY);
    exit(0);
  }
  
  // for debugging
  if(c->verbosity>=3)
  {
    DEBUG(3,"pdev dump\n");
    dumpmem(((void*)c->pdev),0x200);  
    DEBUG(3,"dm dump\n");
    dumpmem((void*)&c->dm,sizeof(c->dm));
    DEBUG(3,"gdiinfo dump\n");
    dumpmem((void*)&c->gdiinfo,sizeof(c->gdiinfo));
    DEBUG(3,"devinf dump\n");
    dumpmem((void*)&c->devinfo,sizeof(c->devinfo));
  }
  
  // finish physical device initialization
  pDrvCompletePDEV(c->pdev,(HDEV)0xcafe0042);

  // debugging
  if(c->verbosity>=3) dumpmem(((void*)c->pdev),0x200);
  
  render_document();
  
  return 0;
}
