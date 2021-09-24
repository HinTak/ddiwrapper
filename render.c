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

void render_document(void)
{
  int i,j,k;
  FILE* fbm=0;

  // tell driver to create a printing surface; our EngCreateBitmap() implementation returns
  // a pointer to the created surface as the surface handle; this is a bit hackish, but
  // the driver is not supposed to question us, after all.
  HSURF drivsurf;
  DEBUG(1,"Calling DrvEnableSurface, pdev %p, last error %d\n",c->pdev,GetLastError());
  SetLastError(0);
  drivsurf=pDrvEnableSurface(c->pdev);
  if(!drivsurf)
  {
    ERRMSG("DrvEnableSurface failed, error %d\n",GetLastError());
    exit(1);
  }
  else
    DEBUG(3,"drivsurf %p\n",drivsurf);

  DEBUG(4,"so.pvBits %p\n",((SURFOBJ*)drivsurf)->pvBits);
  SURFOBJ* so=(SURFOBJ*)drivsurf;

  // Canon driver does not set this member correctly, possibly due to the improperly initialized
  // DEVMODEW structure; see load.c for details
  for(i=0;i<4;i++)
    so[i].dhpdev=c->pdev;
  
  // debugging
  if(c->verbosity>=3) dumpmem(((void*)c->pdev),0x200);

  // clone the device's surface; for some reason it is impossible to re-use the same instance
  // for calls to Drv* functions
  SURFOBJ bm;
  memset(&bm,0,sizeof(bm));
  memcpy(&bm,&so[0],sizeof(SURFOBJ));
  bm.pvBits=malloc(bm.sizlBitmap.cx*bm.lDelta);
  bm.pvScan0=bm.pvBits;
  
  // tell driver about the document name and job ID
  if(!pDrvStartDoc(&bm,L"test document",42))
  {
    ERRMSG("DrvStartDoc failed, error %d\n",GetLastError());
    exit(1);
  }

  // open input file
  if(c->input_file)
    fbm=fopen(c->input_file,"rb");
  else
    fbm=stdin;
  
  if(!fbm)
  {
    ERRMSG("Could not open input file %s\n",c->input_file);
    exit(2);
  }

  // page loop
  int pagecount=1;
  for(;;)
  {
    // skip BMP header in input stream
    int hdrb=fread(so[0].pvBits,1,0x36,fbm);
    DEBUG(2,"read %d of %d header bytes\n",hdrb,0x36);

    // short read; either we are at the end of the input file, or an error occurred
    if(hdrb<0x36)
    {
      if(feof(fbm)) break;
      else
      {
        ERRMSG("error reading BMP header on page %d\n",pagecount);
        exit(35);
      }
    }
    
    // very basic check for BMP header correctness
    if(*((UCHAR*)so[0].pvBits)!='B' || *((UCHAR*)so[0].pvBits+1)!='M')
    {
      ERRMSG("no valid BMP header at start of page\n")
      exit(34);
    }
    
    // start a new page
    DEBUG(1,"starting page no. %d\n",pagecount++);
    SetLastError(0xffff);
    //asm("int $3");
    if(!pDrvStartPage(&bm))
    {
      ERRMSG("DrvStartPage failed, error %d\n",GetLastError());
      exit(1);
    }
    
    // start banding; banding is the method usually used by raster-based devices such as inkjet printers.
    // vector-based devices such as PostScript printers or plotters want the whole page to be drawn in
    // one go -> unimplemented
    SetLastError(0xffff);
    POINTL p;
    DEBUG(1,"Calling DrvStartBanding\n");
    if(!pDrvStartBanding(&bm,&p))
      ERRMSG("DrvStartBanding failed, error %d\n",GetLastError());
    
    DEBUG(2,"DrvStartBanding returned point %d/%d\n",p.x,p.y);
    DEBUG(3,"so.pvBits %p\n",so->pvBits);

    SetLastError(0xffff);
    //asm("int $3");
    DEBUG(3,"so %p, so.pdev %p, bm %p, bm.pdev %p, size %d\n",so,so->dhpdev,&bm,bm.dhpdev,sizeof(SURFOBJ));

    // band loop; DrvNextBand returns coordinated -1/-1 once the page is completed
    while(p.x!=-1)
    {
      CLIPOBJ co;
      co.iUniq=0;
      co.rclBounds.top=p.y;
      co.rclBounds.left=0;
      co.rclBounds.right=so[0].sizlBitmap.cx-1;
      co.rclBounds.bottom=p.y+so[0].sizlBitmap.cy-1;
      co.iDComplexity=DC_TRIVIAL;
      co.iFComplexity=FC_RECT;
      co.iMode=TC_RECTANGLES;
      RECTL r;
      r.left=r.top=p.y;
      r.right=so[0].sizlBitmap.cx-1;
      r.bottom=p.y+so[0].sizlBitmap.cy-1;
      POINTL s;
      s.x=0; s.y=p.y;
      
      //asm("int $3");

      // EVIL HACK TIME!
      // The right way to do it would be to call DrvBitBlt() which in turn calls EngBitBlt() which would have
      // to copy the bitmap data to the surface object so the driver can convert it to printer code.
      // This doesn't work.
      // I have tried everything to forge a surface object that the DrvBitBlt() function would be happy with,
      // but without any success. Thus, instead of doing it the right way, I chose the working way: copying
      // the bitmap data to the surface and calling DrvLineTo() afterwards to make sure the driver notices that
      // the surface has changed.

      // read until the end of the page, but at most as much as fits in the surface    
      int bytestoread=(c->gdiinfo.ulVertRes-p.y)*so[0].lDelta < so[0].cjBits ? (c->gdiinfo.ulVertRes-p.y)*so[0].lDelta : so[0].cjBits;
      DEBUG(3,"trying to read %d bytes from input\n",bytestoread);

      int bytesread=0;
      while(bytesread<bytestoread && !(feof(fbm) || ferror(fbm)))	// Win32ism - this loop would not be necessary in Unix
      {
        bytesread+=fread(so[0].pvBits+bytesread,1,bytestoread-bytesread,fbm);
        DEBUG(3,"%d read, %d to read, feof %d, ferror %d\n",bytesread,bytestoread,feof(fbm),ferror(fbm));
      }
      if(ferror(fbm))
      {
        ERRMSG("error reading input stream\n");
        exit(51);
      }
      if(bytesread<bytestoread)
      {
        DEBUG(1,"short read on input file, was %d, should have been %d\n",bytesread,bytestoread);
        // be tolerant and fill up the missing surface with white
        memset(so[0].pvBits+bytesread,0xff,bytestoread-bytesread);
      }

      // BMP is upside-down. We cannot seek in a pipe, so we just print the bitmap upside-down.
      // We have to mirror each line, though.
      UCHAR* cp=(UCHAR*)so[0].pvBits;
      for(i=0;i<so[0].cjBits;i+=so[0].lDelta)	// lines
      {
        for(j=0;j<so[0].lDelta/2;j+=3)		// pixels
        {
          UCHAR tmp;
          int start=i+j;
          int end=i-j+so[0].lDelta-3;
          for(k=0;k<3;k++)			// color components
          {
            tmp=cp[start+k];
            cp[start+k]=cp[end+k];
            cp[end+k]=tmp;
          }
        }
      }

      // convert 24-bit BGR (BMP color order) to the input format wanted by the driver
      // So far, BGR (nothing to do) and RGB (swap blue and red) are supported
      // c->color_mode is set by EngCreatePalette() which the driver calls during DrvEnableSurface()
      // execution, when it creates its printing surface
      DEBUG(2,"c->color_mode %d\n",c->color_mode);
      if(c->color_mode == PAL_RGB)
      {
        // swap BGR -> RGB
        UCHAR tmp;
        for(i=0;i<so[0].cjBits;i+=3)
        {
          tmp=((UCHAR*)so[0].pvBits)[i];
          ((UCHAR*)so[0].pvBits)[i]=((UCHAR*)so[0].pvBits)[i+2];
          ((UCHAR*)so[0].pvBits)[i+2]=tmp;
        }
      }
      
      // phony DrvLineTo() call with a NOP EngLineTo() behind it; this assumes that the driver
      // passes this request on to GDI, thus cannot make any assumptions on the state of its
      // printing surface and has to look at the image data we wrote into it.
      BOOL x=pDrvLineTo(&bm,0,0,0,p.y,so[0].sizlBitmap.cx-1,(p.y+bytestoread/so[0].lDelta)-1,&r,0);
      DEBUG(2,"DrvLineTo returned %d\n",x);

      // Workaround for a bug in the Canon driver; it _should_ be possible to just call DrvNextBand()
      // until it returns -1/-1 in p and then call DrvSendPage to finish the page. When doing that
      // the Canon driver outputs a form feed on both DrvNextBand and DrvSendPage, resulting in an empty
      // page. Therefore we have to check for end of page on our own:
      if(p.y + bytestoread/so[0].lDelta >= c->gdiinfo.ulVertRes)
        break;
      else
      {
        // get the position of the next band
        if(!pDrvNextBand(&bm,&p))
          ERRMSG("DrvNextBand failed, error %d\n",GetLastError());

        DEBUG(2,"DrvNextBand returned point %d/%d\n",p.x,p.y);
      }
    }

    // page completed, send end-of-page printer code
    if(!pDrvSendPage(&bm))
    {
      ERRMSG("DrvSendPage failed, error %d\n",GetLastError());
      exit(31);
    }
    else
      DEBUG(1,"DrvSendPage succeeded\n");
  }

  //asm("int $3");
  SetLastError(12345); // set last error to a silly value to check if the driver has changed it
  // fine with the iP3000 and i80 drivers, but always fails with the i905d driver.
  // may be a bug in that driver; it sends the correct end-of-doc sequence anyway,
  // so we can ignore this problem
  if(!pDrvEndDoc(&bm,0))
  {
    DEBUG(1,"DrvEndDoc failed, error %d\n",GetLastError());
    //exit(97);
  }
}
