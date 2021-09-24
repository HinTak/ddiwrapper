#ifndef _DDI_WINGDI_H
#define _DDI_WINGDI_H

#define MM_MAX_NUMAXES  16
typedef struct _DESIGNVECTOR {
	DWORD dvReserved;
	DWORD dvNumAxes;
	LONG dvValues[MM_MAX_NUMAXES];
} DESIGNVECTOR, *PDESIGNVECTOR, FAR *LPDESIGNVECTOR;

#include_next <wingdi.h>
#endif
