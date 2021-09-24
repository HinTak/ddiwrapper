// Copyright 2004 Ulrich Hecht <uli@suse.de>

// This file is part of ddiwrapper.

// ddiwrapper is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#include <windows.h>
#include <winspool.h>

#include "winddi.h"

#include <msvcrt/stdio.h>
#include <msvcrt/fcntl.h>

#define DLL
#include "../common.h"

// return info on its environment to the driver; these are mostly good guesses
BOOL WINAPI GetPrinterDriverW(HANDLE hPrinter, LPWSTR pEnvironment,
				      DWORD Level, LPBYTE pDriverInfo,
				      DWORD cbBuf, LPDWORD pcbNeeded
				      )
{
  DEBUG(1,"GetPrinterDriverW called, hPrinter %p, Level %ld, cbBuf %ld\n",hPrinter,Level,cbBuf);

  DRIVER_INFO_2W w2;
  DRIVER_INFO_3W w3;
  DRIVER_INFO_1W w1;

  DEBUG(3,"config is %p\n",c);

  switch(Level)
  {
  case 1:
    *pcbNeeded=sizeof(w1);
    w1.pName=c->printer_name;
    break;
  case 2:
    *pcbNeeded=sizeof(w2);
    w2.cVersion=3;
    w2.pName=c->printer_name;
    w2.pEnvironment=c->environment;
    w2.pDriverPath=c->driver_path;
    w2.pDataFile=c->data_path;
    w2.pConfigFile=c->config_path;
    if(cbBuf<sizeof(w2))
    {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
    }
    else
            memcpy(pDriverInfo,&w2,sizeof(w2));
    break;
  case 3:
    *pcbNeeded=sizeof(w3);
    w3.cVersion=3;
    w3.pName=c->printer_name;
    w3.pEnvironment=c->environment;
    w3.pDriverPath=c->driver_path;
    w3.pDataFile=c->data_path;
    w3.pConfigFile=c->config_path;
    w3.pHelpFile=L"some.hlp";
    w3.pMonitorName=L"clmnmon2.dll";
    w3.pDependentFiles=L"";
    w3.pDefaultDataType=L"BMF";
    if(cbBuf<sizeof(w3))
    {
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      return FALSE;
    }
    else
      memcpy(pDriverInfo,&w3,sizeof(w3));
    break;
  default:
    ERRMSG("GetPrinterDriver Level %ld not implemented\n",Level);
    exit(3);
    break;
  }

  DEBUG(3,"driverpath %s\n",uni2ascii(c->driver_path));
  
  return TRUE;
}

// same as GetPrinterDriverW(), only in ASCII
BOOL WINAPI GetPrinterDriverA(HANDLE hPrinter, LPSTR pEnvironment,
				      DWORD Level, LPBYTE pDriverInfo,
				      DWORD cbBuf, LPDWORD pcbNeeded
				      )
{
  DEBUG(1,"GetPrinterDriverA called, hPrinter %p, Level %ld, cbBuf %ld\n",hPrinter,Level,cbBuf);

  DRIVER_INFO_2A w2;
  DRIVER_INFO_3A w3;

  DEBUG(3,"config is %p\n",c);

  switch(Level)
  {
  case 2:
    *pcbNeeded=sizeof(w2);
    w2.cVersion=3;
    w2.pName=strdup(uni2ascii(c->printer_name));
    w2.pEnvironment=strdup(uni2ascii(c->environment));
    w2.pDriverPath=strdup(uni2ascii(c->driver_path));
    w2.pDataFile=strdup(uni2ascii(c->data_path));
    w2.pConfigFile=strdup(uni2ascii(c->config_path));
    if(cbBuf<sizeof(w2))
    {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
    }
    else
            memcpy(pDriverInfo,&w2,sizeof(w2));
    break;
  case 3:
    *pcbNeeded=sizeof(w3);
    w3.cVersion=3;
    w3.pName=strdup(uni2ascii(c->printer_name));
    w3.pEnvironment=strdup(uni2ascii(c->environment));
    w3.pDriverPath=strdup(uni2ascii(c->driver_path));
    w3.pDataFile=strdup(uni2ascii(c->data_path));
    w3.pConfigFile=strdup(uni2ascii(c->config_path));
    w3.pHelpFile="some.hlp";
    w3.pMonitorName="clmnmon2.dll";
    w3.pDependentFiles="";
    w3.pDefaultDataType="BMF";
    if(cbBuf<sizeof(w3))
    {
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      return FALSE;
    }
    else
      memcpy(pDriverInfo,&w3,sizeof(w3));
    break;
  default:
    ERRMSG("GetPrinterDriver Level %ld not implemented\n",Level);
    exit(3);
    break;
  }

  DEBUG(3,"driverpath %s\n",uni2ascii(c->driver_path));
  
  return TRUE;
}

BOOL WINAPI ClosePrinter(HANDLE hPrinter)
{
	DEBUG(1,"ClosePrinter called, hPrinter %p\n",hPrinter);
	return TRUE;
}

// give a printer handle to the driver that it can use when calling winspool functions
BOOL WINAPI OpenPrinterW(LPWSTR lpPrinterName,HANDLE *phPrinter,
			 LPPRINTER_DEFAULTSW pDefault)
{
	DEBUG(1,"OpenPrinterW called, lpPrinterName %s, phPrinter %p, pDefault %p\n",uni2ascii(lpPrinterName),phPrinter,pDefault);
	*phPrinter=(HANDLE)0xcafe0003;
	return TRUE;
}

// same as OpenPrinterW, only in ASCII
BOOL WINAPI OpenPrinterA(LPSTR lpPrinterName,HANDLE *phPrinter,
			 LPPRINTER_DEFAULTS pDefault)
{
	DEBUG(1,"OpenPrinterA called, lpPrinterName %s, phPrinter %p, pDefault %p\n",lpPrinterName,phPrinter,pDefault);
	*phPrinter=(HANDLE)0xcafe0003;
	return TRUE;
}

// retrieve printer-related registry values; these are mostly bad guesses. You will want to fix
// this when trying to make a new driver work.
DWORD WINAPI GetPrinterDataW(HANDLE hPrinter, LPWSTR pValueName, LPDWORD pType,
			     LPBYTE pData, DWORD nSize, LPDWORD pcbNeeded)
{
	DEBUG(1,"GetPrinterDataW called, hPrinter %p, pType %p, pData %p, nSize %ld, ",hPrinter,pType,pData,nSize);
	DEBUG(1,"pValueName %s\n",uni2ascii(pValueName));
	if(pType) *pType=REG_DWORD;
	if(nSize>50)
	{
	  *pcbNeeded=nSize;
	  memset(pData,0,nSize);
	  return ERROR_SUCCESS;
        }
	if(pcbNeeded) *pcbNeeded=4;
	if(nSize<4)
	{
	  return ERROR_INSUFFICIENT_BUFFER;
        }
	*(ULONG*)pData=1;
	if(!strncmp(uni2ascii(pValueName),"CnmUI_",6)) *(ULONG*)pData=0;
	return ERROR_SUCCESS;
}

// same as GetPrinterDataW(), only in ASCII
DWORD WINAPI GetPrinterDataA(HANDLE hPrinter, LPSTR pValueName, LPDWORD pType,
                             LPBYTE pData, DWORD nSize, LPDWORD pcbNeeded)
{
  WCHAR val[512];
  WCHAR* v=val;
  while((*v++=*pValueName++));

  return GetPrinterDataW(hPrinter,val,pType,pData,nSize,pcbNeeded);
}

BOOL WINAPI EndPagePrinter(HANDLE hPrinter)
{
    DEBUG(1,"EndPagePrinter called, hPrinter=%p\n", hPrinter);
    return TRUE;
}

// always tell the driver that there are no jobs pending; not used by the Canon drivers
BOOL WINAPI EnumJobsW(HANDLE hPrinter, DWORD FirstJob, DWORD NoJobs,
		      DWORD Level, LPBYTE pJob, DWORD cbBuf, LPDWORD pcbNeeded,
		      LPDWORD pcReturned)
{
    DEBUG(1,"EnumJobsW called\n");
    if(pcbNeeded) *pcbNeeded = 0;
    if(pcReturned) *pcReturned = 0;
    return TRUE;
}

// we're getting into seriously muddy water here; most of these values are
// totally bogus; every driver that takes a serious look at them is likely to barf.
BOOL WINAPI GetPrinterW(HANDLE hPrinter, DWORD Level, LPBYTE pPrinter,
			DWORD cbBuf, LPDWORD pcbNeeded)
{
  // gets the handle passed to DrvEnablePDEV
  DEBUG(1,"GetPrinterW called, hPrinter %p, Level %ld, pPrinter %p, cbBuf %ld\n",hPrinter,Level,pPrinter,cbBuf);
	
  PRINTER_INFO_2W pi2;
  PRINTER_INFO_4W pi4;
  PRINTER_INFO_5W pi5;

  switch(Level)
  {
    case 2:	
	*pcbNeeded=sizeof(pi2);
	if(cbBuf<sizeof(pi2))
	{
	  SetLastError(ERROR_INSUFFICIENT_BUFFER);
	  return FALSE;
        }
	pi2.pServerName=0; // local printer
	pi2.pPrinterName=L"Printer Name";
	pi2.pShareName=0;
	pi2.pPortName=L"gpwport";
	pi2.pDriverName=c->driver_path;
	pi2.pComment=L"nice weather, eh?";
	pi2.pLocation=L"here";
	pi2.pDevMode=c->dm;
	pi2.pSepFile=L"sep.fil";
	pi2.pPrintProcessor=L"pproc";
	pi2.pDatatype=L"dtype";
	pi2.pParameters=L"ppparms";
	pi2.pSecurityDescriptor=0;
	pi2.Attributes=PRINTER_ATTRIBUTE_DEFAULT|PRINTER_ATTRIBUTE_LOCAL;
	pi2.Priority=pi2.DefaultPriority=1;
	pi2.StartTime=0;
	pi2.UntilTime=24*60-1;
	pi2.Status=PRINTER_STATUS_SERVER_UNKNOWN; // ?
	pi2.cJobs=0;
	pi2.AveragePPM=10;
	memcpy(pPrinter,&pi2,sizeof(pi2));
	return TRUE;
	break;
    case 5:
	*pcbNeeded=sizeof(pi5);
	if(cbBuf<sizeof(pi5))
	{
	  SetLastError(ERROR_INSUFFICIENT_BUFFER);
	  return FALSE;
        }
	pi5.pPrinterName=L"Printer Name";
	pi5.pPortName=L"gpwport";
	pi5.Attributes=PRINTER_ATTRIBUTE_DEFAULT|PRINTER_ATTRIBUTE_LOCAL;
	
	memcpy(pPrinter,&pi5,sizeof(pi5));
	return TRUE;
	break;
    case 4:
	*pcbNeeded=sizeof(pi4);
	if(cbBuf<sizeof(pi4))
	{
	  SetLastError(ERROR_INSUFFICIENT_BUFFER);
	  return FALSE;
        }
	pi4.pPrinterName=L"Printer Name";
	pi4.pServerName=0; // local printer
	pi4.Attributes=PRINTER_ATTRIBUTE_DEFAULT|PRINTER_ATTRIBUTE_LOCAL;
        
	memcpy(pPrinter,&pi4,sizeof(pi4));
	return TRUE;
	break;
    default:
        ERRMSG("GetPrinterW Level %ld not implemented\n",Level);
        exit(6);
        break;
    }
    
}

// convert a wide string DEVMODE structure to an ASCII one
static VOID devmodewtoa(DEVMODEW* dmw, DEVMODEA* dma)
{
  strcpy(dma->dmDeviceName,uni2ascii(dmw->dmDeviceName));
  dma->dmSpecVersion=dmw->dmSpecVersion;
  dma->dmDriverVersion=dmw->dmDriverVersion;
  dma->dmSize=dmw->dmSize;
  dma->dmDriverExtra=dmw->dmDriverExtra;
  dma->dmFields=dmw->dmFields;
  dma->dmOrientation=dmw->dmOrientation;
  dma->dmPaperSize=dmw->dmPaperSize;
  dma->dmPaperLength=dmw->dmPaperLength;
  dma->dmPaperWidth=dmw->dmPaperWidth;
  dma->dmPosition=dmw->dmPosition;
  dma->dmScale=dmw->dmScale;
  dma->dmCopies=dmw->dmCopies;
  dma->dmDefaultSource=dmw->dmDefaultSource;
  dma->dmPrintQuality=dmw->dmPrintQuality;
  dma->dmColor=dmw->dmColor;
  dma->dmDuplex=dmw->dmDuplex;
  dma->dmYResolution=dmw->dmYResolution;
  dma->dmTTOption=dmw->dmTTOption;
  dma->dmCollate=dmw->dmCollate;
  strcpy(dma->dmFormName,uni2ascii(dmw->dmFormName));
  dma->dmLogPixels=dmw->dmLogPixels;
  dma->dmBitsPerPel=dmw->dmBitsPerPel;
  dma->dmPelsWidth=dmw->dmPelsWidth;
  dma->dmPelsHeight=dmw->dmPelsHeight;
  dma->dmDisplayFlags=dmw->dmDisplayFlags;
  dma->dmDisplayFrequency=dmw->dmDisplayFrequency;
  dma->dmICMMethod=dmw->dmICMMethod;
  dma->dmICMIntent=dmw->dmICMIntent;
  dma->dmMediaType=dmw->dmMediaType;
  dma->dmDitherType=dmw->dmDitherType;
  dma->dmReserved1=dmw->dmReserved1;
  dma->dmReserved2=dmw->dmReserved2;
  dma->dmPanningWidth=dmw->dmPanningWidth;
  dma->dmPanningHeight=dmw->dmPanningHeight;
}

BOOL WINAPI GetPrinterA(HANDLE hPrinter, DWORD Level, LPBYTE pPrinter,
			DWORD cbBuf, LPDWORD pcbNeeded)
{
  // gets the handle passed to DrvEnablePDEV
  DEBUG(1,"GetPrinterA called, hPrinter %p, Level %ld, pPrinter %p, cbBuf %ld\n",hPrinter,Level,pPrinter,cbBuf);
	
  PRINTER_INFO_2A pi2;
  PRINTER_INFO_4A pi4;
  PRINTER_INFO_5A pi5;
  static DEVMODEA dma;
  
  devmodewtoa(c->dm,&dma);

  switch(Level)
  {
    case 2:	
	*pcbNeeded=sizeof(pi2);
	if(cbBuf<sizeof(pi2))
	{
	  SetLastError(ERROR_INSUFFICIENT_BUFFER);
	  return FALSE;
        }
	pi2.pServerName=0; // local printer
	pi2.pPrinterName="Printer Name";
	pi2.pShareName=0;
	pi2.pPortName="gpaport";
	pi2.pDriverName=strdup(uni2ascii(c->driver_path));
	pi2.pComment="beautiful plumage";
	pi2.pLocation="here";
	pi2.pDevMode=&dma;
	pi2.pSepFile="sep.fil";
	pi2.pPrintProcessor="pproc";
	pi2.pDatatype="dtype";
	pi2.pParameters="ppparms";
	pi2.pSecurityDescriptor=0;
	pi2.Attributes=PRINTER_ATTRIBUTE_DEFAULT|PRINTER_ATTRIBUTE_LOCAL;
	pi2.Priority=pi2.DefaultPriority=1;
	pi2.StartTime=1;
	pi2.UntilTime=24*60-1;
	pi2.Status=PRINTER_STATUS_SERVER_UNKNOWN; // ?
	pi2.cJobs=0;
	pi2.AveragePPM=10;
	memcpy(pPrinter,&pi2,sizeof(pi2));
	return TRUE;
	break;
    case 5:
	*pcbNeeded=sizeof(pi5);
	if(cbBuf<sizeof(pi5))
	{
	  SetLastError(ERROR_INSUFFICIENT_BUFFER);
	  return FALSE;
        }
	pi5.pPrinterName="Printer Name";
	pi5.pPortName="gpaport";
	pi5.Attributes=PRINTER_ATTRIBUTE_DEFAULT|PRINTER_ATTRIBUTE_LOCAL;
	
	memcpy(pPrinter,&pi5,sizeof(pi5));
	return TRUE;
	break;
    case 4:
	*pcbNeeded=sizeof(pi4);
	if(cbBuf<sizeof(pi4))
	{
	  SetLastError(ERROR_INSUFFICIENT_BUFFER);
	  return FALSE;
        }
	pi4.pPrinterName="Printer Name";
	pi4.pServerName=0; // local printer
	pi4.Attributes=PRINTER_ATTRIBUTE_DEFAULT|PRINTER_ATTRIBUTE_LOCAL;
        
	memcpy(pPrinter,&pi4,sizeof(pi4));
	return TRUE;
	break;
    default:
        ERRMSG("GetPrinterW Level %ld not implemented\n",Level);
        exit(6);
        break;
    }
    
}

BOOL WINAPI StartPagePrinter(HANDLE hPrinter)
{
    DEBUG(1,"StartPagePrinter called, hPrinter=%p\n", hPrinter);
    return TRUE;
}

// called by the driver when it wants to send data to the printer
// unlike all other functions in this library, whose purporse is solely to make the driver
// feel good, this one actually does something.
BOOL WINAPI
WritePrinter(
  HANDLE  hPrinter,
  LPVOID  pBuf,
  DWORD   cbBuf,
  LPDWORD pcWritten)
{
  static int fd=0;
  int i;

  DEBUG(2,"WritePrinter called, hPrinter %p, pBuf %p, cbBuf %ld, pcWritten %p (%ld)\n",hPrinter,pBuf,cbBuf,pcWritten,*pcWritten);

  // check if the driver passes the correct spooler handle
  // very unlikely to fail as we only have one printer
  if(hPrinter!=c->spoolhandle)
  {
    ERRMSG("driver passed illegal winspool handle\n");
    exit(4);
  }

  // debugging: dump output data to stderr
  if(c->verbosity>=4)
  {
    for(i=0;i<(cbBuf>256?256:cbBuf);i++)
      DEBUG(4,"%02x ",((UCHAR*)pBuf)[i]);
    DEBUG(4,"\n");
    if(cbBuf>256) DEBUG(4,"[ data above 256 omitted]\n");
  }

  // open the output file if not done yet
  if(!fd)
  {
    if(c->output_file)
    {
      DEBUG(3,"opening output file/device\n");
      fd=open(c->output_file,O_CREAT|O_WRONLY|O_BINARY|O_TRUNC);
    }
    else
      fd=STDOUT_FILENO;
  }

  if(!fd)
  {
    ERRMSG("failed to open output file\n");
    exit(5);
  }

  if(write(fd,pBuf,cbBuf)!=cbBuf)
  {
    ERRMSG("write to printer failed\n");
    exit(667);
  }

  // tell the caller that everything has been written
  // If you think that the presence of the pcWritten argument suggests
  // that it is legal (and not an error case) to do a partial write here,
  // you are probably right. If you think that drivers take this possibility
  // into account, you are wrong.
  *pcWritten=cbBuf;

  //asm("int $3");
  return TRUE;
}

// returns a single form of an arbitrary size
// Canon drivers do not seem to use this, but other drivers may require it to be
// implemented properly.
BOOL WINAPI
EnumFormsW(
  HANDLE Printer,
  DWORD Level,
  PBYTE Buffer,
  DWORD BufSize,
  PDWORD Needed,
  PDWORD Returned)
{
  // gets the handle passed to DrvEnablePDEV
  DEBUG(1,"EnumFormsW called, Printer %p, Level %ld, Buffer %p, BufSize %ld\n",Printer,Level,Buffer,BufSize);
  
  if(Level != 1) 
  {
    ERRMSG("EnumFormsW Level %ld not implemented\n",Level);
    exit(10);
  }
  
  FORM_INFO_1W info;
  *Needed=sizeof(info);
  if(BufSize<sizeof(info))
  {
    SetLastError(ERROR_INSUFFICIENT_BUFFER);
    return FALSE;
  }

  info.Flags=FORM_BUILTIN;
  info.pName=L"form";
  info.Size.cx=info.ImageableArea.right=180*1000;
  info.Size.cy=info.ImageableArea.bottom=200*1000;
  info.ImageableArea.left=info.ImageableArea.top=0;
  *Returned=1;
  memcpy(Buffer,&info,sizeof(info));
  return TRUE;
}

BOOL
WINAPI
GetJobW(
   HANDLE   hPrinter,
   DWORD    JobId,
   DWORD    Level,
   LPBYTE   pJob,
   DWORD    cbBuf,
   LPDWORD  pcbNeeded
)
{
  DEBUG(1,"GetJobW called, hPrinter %p, JobId %lu, Level %lu, pJob %p, cbBuf %lu, pcbNeeded %p\n",hPrinter,JobId,Level,pJob,cbBuf,pcbNeeded);
  *pcbNeeded=0;
  return TRUE;
}
