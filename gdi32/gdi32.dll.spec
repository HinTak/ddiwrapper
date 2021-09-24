# Copyright 2004 Ulrich Hecht <uli@suse.de>

# This file is part of ddiwrapper.

# ddiwrapper is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

@ stub EngTextOut
@ stub DeleteObject
@ stub CLIPOBJ_cEnumStart
@ stub CLIPOBJ_bEnum
@ stdcall EngCheckAbort (ptr)
@ stdcall EngEraseSurface (ptr ptr long)
@ stub EngPaint
@ stub EngStrokeAndFillPath
@ stub EngFillPath
@ stub EngStrokePath
@ stdcall EngLineTo (ptr ptr ptr long long long long ptr long)
@ stub EngCreateClip
@ stub XLATEOBJ_iXlate
@ stub EngDeleteClip
@ stdcall EngCreateBitmap (long long long long long ptr)
@ stdcall EngAssociateSurface (long long long)
@ stdcall EngLockSurface (long)
@ stdcall EngCreateDeviceSurface (long long long long)
@ stdcall EngMarkBandingSurface (long)
@ stub HT_Get8BPPMaskPalette
@ stdcall EngCreatePalette (long long ptr long long long)
@ stub EngUnlockSurface
@ stub EngDeleteSurface
@ stdcall EngDeletePalette (long)
@ stub BRUSHOBJ_ulGetBrushColor
@ stdcall EngStretchBltROP (ptr ptr ptr ptr ptr ptr ptr ptr ptr ptr long ptr long)
@ stub EngPlgBlt
@ stub EngCopyBits
@ stub EngStretchBlt
@ stub XLATEOBJ_cGetPalette
@ stdcall EngBitBlt (ptr ptr ptr ptr ptr ptr ptr ptr ptr ptr long)
@ stub EngTransparentBlt
@ stub EngGradientFill
@ stub EngAlphaBlend
@ stdcall EngGetCurrentCodePage (ptr ptr)
@ stub STROBJ_vEnumStart
@ stub CLIPOBJ_ppoGetPath
@ stub BRUSHOBJ_pvGetRbrush
@ stub XLATEOBJ_piVector
@ stub BRUSHOBJ_pvAllocRbrush
@ stub PATHOBJ_bEnum
@ stub STROBJ_bEnum
@ stub XFORMOBJ_iGetXform
@ stub XFORMOBJ_bApplyXform
@ stub FONTOBJ_pxoGetXform
@ stub FONTOBJ_pifi
@ stub EngDeletePath
@ stub PATHOBJ_vGetBounds
@ stub FONTOBJ_cGetGlyphs
@ stub PATHOBJ_vEnumStart
@ stub BitBlt
@ stub CombineRgn
@ stub CreateBitmap
@ stub CreateCompatibleDC
@ stub CreateDIBitmap
@ stub CreatePolyPolygonRgn
@ stub CreateRectRgn
@ stub CreateRectRgnIndirect
@ stub DPtoLP
@ stub DeleteDC
@ stub DeleteEnhMetaFile
@ stub DeleteMetaFile
@ stub DeleteMetaFile16
@ stub ExtCreateRegion
@ stub ExtEscape
@ stub GDI_GetObjPtr
@ stub GDI_ReleaseObj
@ stub GetArcDirection
@ stub GetBkMode
@ stub GetBrushOrgEx
@ stub GetClipBox
@ stub GetCurrentObject
@ stub GetCurrentPositionEx
@ stub GetDCBrushColor
@ stub GetDCPenColor
@ stub GetDIBits
@ stub GetDeviceCaps
@ stub GetEnhMetaFileBits
@ stub GetGlyphIndicesW
@ stub GetGlyphOutlineW
@ stub GetMapMode
@ stub GetMetaFileBitsEx
@ stub GetNearestPaletteIndex
@ stub GetObjectA
@ stub GetObjectType
@ stub GetObjectW
@ stub GetOutlineTextMetricsW
@ stdcall GetPaletteEntries (long long long ptr)
@ stub GetPolyFillMode
@ stub GetROP2
@ stub GetRegionData
@ stub GetRgnBox
@ stdcall GetStockObject (long)
@ stub GetStretchBltMode
@ stub GetTextAlign
@ stub GetTextCharacterExtra
@ stub GetTextColor
@ stub GetTextExtentPointI
@ stub GetTextMetricsW
@ stub LPtoDP
@ stub MoveToEx
@ stub OffsetRgn
@ stub PatBlt
@ stub PtInRegion
@ stub SelectObject
@ stub SelectVisRgn16
@ stub SetEnhMetaFileBits
@ stub SetHookFlags16
@ stub SetMetaFileBitsEx
@ stub SetRectRgn
@ stdcall EngFreeModule (long)
@ stdcall EngLoadModule (ptr)
@ stub CreateBitmapIndirect
@ stub CreateCompatibleBitmap
@ stub CreateDCA
@ stub CreateDCW
@ stub CreateDIBSection
@ stub CreateFontA
@ stub CreateFontIndirectA
@ stub CreateFontIndirectW
@ stub CreateFontW
@ stub CreateICA
@ stub CreatePatternBrush
@ stub CreatePen
@ stub CreateSolidBrush
@ stub Ellipse
@ stub ExcludeVisRect16
@ stub ExtSelectClipRgn
@ stub ExtTextOutW
@ stub GetBitmapBits
@ stub GetBkColor
@ stub GetClipRgn
@ stub GetDCOrgEx
@ stub GetDCState16
@ stub GetSystemPaletteEntries
@ stub GetTextExtentExPointW
@ stub GetTextExtentPoint32A
@ stub GetTextExtentPoint32W
@ stub GetTextExtentPointA
@ stub GetTextExtentPointW
@ stub GetTextMetricsA
@ stub IntersectClipRect
@ stub IsDCCurrentPalette16
@ stub LineTo
@ stub OffsetViewportOrgEx
@ stub Pie
@ stub Polygon
@ stub RectInRegion
@ stub Rectangle
@ stub SelectClipRgn
@ stub SetBitmapBits
@ stub SetBkColor
@ stub SetBkMode
@ stub SetDCHook
@ stub SetDCState16
@ stub SetDIBits
@ stub SetMapMode
@ stub SetROP2
@ stub SetStretchBltMode
@ stub SetTextAlign
@ stub SetTextColor
@ stub SetWindowOrgEx
@ stub StretchBlt
@ stub StretchDIBits
@ stub TextOutA
@ stub TextOutW
@ stub UnrealizeObject
