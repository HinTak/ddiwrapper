// Copyright 2004 Ulrich Hecht <uli@suse.de>

// This file is part of ddiwrapper.

// ddiwrapper is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License version 2.1 as
// published by the Free Software Foundation.

#include <wtypes.h>
#include <windows.h>
#include <winddi.h>

#include <msvcrt/stdio.h>

#define DLL
#include "../common.h"

VOID WINAPI
EngGetCurrentCodePage(OUT PUSHORT OemCodePage,
                      OUT PUSHORT AnsiCodePage)
{
  DEBUG(1,"EngGetCurrentCodePage called\n");
  *OemCodePage=850;
  *AnsiCodePage=850;
}

HPALETTE WINAPI
EngCreatePalette(IN ULONG Mode,
                 IN ULONG NumColors,
                 IN ULONG *Colors,
                 IN ULONG Red,
                 IN ULONG Green,
                 IN ULONG Blue)
{
  DEBUG(1,"EngCreatePalette called, Mode %u, NumColors %u, Colors %p, Red %u, Green %u, Blue %u\n",Mode,NumColors,Colors,Red,Green,Blue);
  c->color_mode=Mode;
  return (HPALETTE)0xcafe0004;
}

BOOL WINAPI EngDeletePalette(IN HPALETTE Palette)
{
  DEBUG(1,"EngDeletePalette called, Palette %p\n",Palette);
  return TRUE;
}

static SURFOBJ so[4];

HBITMAP WINAPI EngCreateBitmap(IN SIZEL  sizl, IN LONG  lWidth, IN ULONG  iFormat, IN FLONG  fl, IN PVOID  pvBits)
{
  static int i=0;
  DEBUG(1,"EngCreateBitmap called, sizl %d/%d, lWidth %d, iFormat 0x%x, fl 0x%x, pvBits %p\n",sizl.cx,sizl.cy,lWidth,iFormat,fl,pvBits);
  memset(&so[i],0,sizeof(so[i]));
  so[i].sizlBitmap=sizl;
  so[i].iBitmapFormat=iFormat;
  so[i].lDelta=lWidth;
  so[i].fjBitmap=fl;
  so[i].cjBits=lWidth*sizl.cy; // This is bytes, not bits!
  if(pvBits)
    so[i].pvBits=pvBits;
  else
  {
    so[i].pvBits=malloc(lWidth*sizl.cy);
    memset(so[i].pvBits,0,lWidth*sizl.cy);
  }
  DEBUG(2,"EngCreateBitmap: setting so[i].pvBits to %p\n",so[i].pvBits);
  so[i].pvScan0=so[i].pvBits;
  so[i].dhsurf=0;	// not device-handled
  so[i].dhpdev=0; // ??
  so[i].hdev=(HDEV)0xcafe0042; // unsure, given to DrvCompletePDEV
  so[i].iUniq=0;
  so[i].iType=STYPE_BITMAP;
  DEBUG(2,"returning SURFOBJ at %p\n",&so[i]);
  return (HBITMAP)&so[i++];
}

BOOL WINAPI
  EngAssociateSurface(
    IN HSURF  hsurf,
    IN HDEV  hdev,
    IN FLONG  flHooks
    )
{
  SURFOBJ* so=(SURFOBJ*)hsurf;
  DEBUG(1,"EngAssociateSurface called, hsurf %p, hdev %p, flHooks 0x%x\n",hsurf,hdev,flHooks);
  so->hsurf=hsurf;
  so->hdev=hdev;
  return TRUE;
}

SURFOBJ* WINAPI EngLockSurface(
    IN HSURF  hsurf
    )
{
  SURFOBJ* so=(SURFOBJ*)hsurf;
  DEBUG(1,"EngLockSurface called, hsurf %p, returning SURFOBJ %p\n",hsurf,so);
  so->hsurf=hsurf;
  return so;
}

HSURF WINAPI
  EngCreateDeviceSurface(
    DHSURF  dhsurf,
    SIZEL  sizl,
    ULONG  iFormatCompat
    )
{
  DEBUG(1,"EngCreateDeviceSurface, dhsurf %p, sizl %d/%d, iFormatCompat %u\n",dhsurf,sizl.cx,sizl.cy,iFormatCompat);
  DEBUG(2,"returning handle %p\n",&so[0]);
  return (HSURF)(VOID*)&so[0];
}

BOOL WINAPI
  EngMarkBandingSurface(
    IN HSURF  hsurf
    )
{
  DEBUG(1,"EngMarkBandingSurface called, hsurf %p\n",hsurf);
  return TRUE;
}

BOOL WINAPI
  EngCheckAbort(
    IN SURFOBJ  *pso
    )
{
  return FALSE;
}

BOOL WINAPI
  EngEraseSurface(
    IN SURFOBJ  *pso,
    IN RECTL  *prcl,
    IN ULONG  iColor)
{
  DEBUG(1,"EngEraseSurface called, pso %p, prcl %d/%d/%d/%d, iColor 0x%x\n",pso,prcl->top,
	prcl->left,prcl->right,prcl->bottom,iColor);
  return TRUE;
}

BOOL WINAPI
  EngBitBlt(
    IN SURFOBJ  *psoTrg,
    IN SURFOBJ  *psoSrc,
    IN SURFOBJ  *psoMask,
    IN CLIPOBJ  *pco,
    IN XLATEOBJ  *pxlo,
    IN RECTL  *prclTrg,
    IN POINTL  *pptlSrc,
    IN POINTL  *pptlMask,
    IN BRUSHOBJ  *pbo,
    IN POINTL  *pptlBrush,
    IN ROP4  rop4
    )
{
  DEBUG(1,"EngBitBlt called, target %p, src %p\n",psoTrg,psoSrc);
  return TRUE;
}

BOOL WINAPI
  EngStretchBltROP(
    IN SURFOBJ  *psoDest,
    IN SURFOBJ  *psoSrc,
    IN SURFOBJ  *psoMask,
    IN CLIPOBJ  *pco,
    IN XLATEOBJ  *pxlo,
    IN COLORADJUSTMENT  *pca,
    IN POINTL  *pptlHTOrg,
    IN RECTL  *prclDest,
    IN RECTL  *prclSrc,
    IN POINTL  *pptlMask,
    IN ULONG  iMode,
    IN BRUSHOBJ  *pbo,
    IN DWORD  rop4
    )
{
  DEBUG(1,"EngStretchBltROP called, dest %p, src %p\n",psoDest,psoSrc);
  return TRUE;
}

BOOL WINAPI
  EngLineTo(
    SURFOBJ  *pso,
    CLIPOBJ  *pco,
    BRUSHOBJ  *pbo,
    LONG  x1,
    LONG  y1,
    LONG  x2,
    LONG  y2,
    RECTL  *prclBounds,
    MIX  mix
    )
{
  DEBUG(1,"EngLineTo called, pso %p, pco %p, pbo %p, coords %d/%d/%d/%d, pcrlBounds %p, mix %u\n",pso,pco,pbo,x1,y1,x2,y2,prclBounds,mix);
  return TRUE;
}

VOID WINAPI EngFreeModule(IN HANDLE h)
{
  DEBUG(1,"EngFreeModule called, handle %p\n",h);
}

HANDLE WINAPI EngLoadModule(IN LPWSTR pwsz)
{
  DEBUG(1,"EngLoadModule called, pwsz %s\n",uni2ascii(pwsz));
  return (HANDLE)0xcafef001;
}

HGDIOBJ WINAPI GetStockObject(
  INT fnObject   // stock object type
  )
{
  DEBUG(1,"GetStockObject called, fnObject %d\n",fnObject);
  return (HGDIOBJ)0xcafee001;
}

UINT WINAPI GetPaletteEntries(
  HPALETTE hpal,        // handle to logical palette
  UINT iStartIndex,     // first entry to retrieve
  UINT nEntries,        // number of entries to retrieve
  LPPALETTEENTRY lppe   // array that receives entries
  )
{
  DEBUG(1,"GetPaletteEntries called, hpal %p, iStartIndex %u, nEntries %u, lppe %p\n",hpal,iStartIndex,nEntries,lppe);
  return 0;
}

HBRUSH WINAPI CreateSolidBrush(COLORREF color)
{
  DEBUG(0,"CreateSolidBrush with color %d\n",color);
  return (HBRUSH)0xcafedead;
}

HPEN WINAPI CreatePen( INT style, INT width, COLORREF color )
{
  DEBUG(0,"CreatePen\n");
  return (HPEN)0xcafedead;
}


HBITMAP WINAPI CreateBitmap( INT width, INT height, UINT planes,
                             UINT bpp, LPCVOID bits )
{
  DEBUG(0,"CreateBitMap\n");
  return (HBITMAP)0xcafedead;
}

HBRUSH WINAPI CreatePatternBrush( HBITMAP hbitmap )
{
 DEBUG(0,"CreatePatternBrush\n");
  return (HBRUSH)0xcafedead;
}

HDC WINAPI CreateDCW( LPCWSTR driver, LPCWSTR device, LPCWSTR output,
                      const DEVMODEW *initData )
{
 DEBUG(0,"CreateDCW\n");
  return (HDC)0xcafedead;
}

INT WINAPI SetDIBits( HDC hdc, HBITMAP hbitmap, UINT startscan,
                      UINT lines, LPCVOID bits, const BITMAPINFO *info,
                      UINT coloruse )
{
  DEBUG(0,"SetDIBits %u\n", lines);
  return lines;
}

INT WINAPI GetObjectA( HGDIOBJ handle, INT count, LPVOID buffer )
{
  DEBUG(0,"GetObjectA %d\n", count);
  return count;
}

BOOL WINAPI DeleteObject( HGDIOBJ obj )
{
  DEBUG(0,"DeleteObject\n")
  return TRUE;
}

void __wine_make_gdi_object_system( HGDIOBJ handle, BOOL set)
{
  DEBUG(0,"__wine_make_gdi_object_system %08x %d\n", handle, set);
}
