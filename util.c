// Copyright 2004 Ulrich Hecht <uli@suse.de>

// This file is part of ddiwrapper.

// ddiwrapper is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#include <windows.h>
#include <msvcrt/stdio.h>

// dumps memory in 32-bit chunks on stderr, used for debugging
void dumpmem(void* mem, int size)
{
  int i;
  for(i=0;i<size/4;i++)
  {
    fprintf(stderr,"0x%08lx ",*((ULONG*)mem));
    mem+=4;
    if(i%8==7) fputs("",stderr);
  }
  fputs("",stderr);
}

// convert a wide string to a narrow string
char* uni2ascii(WCHAR* s)
{
  static char a[1000];
  int i=0;
  for(i=0;s[i];i++)
  {
    a[i]=(char)s[i];
  }
  a[i]=0;
  return &a[0];
}
