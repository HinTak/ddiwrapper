#ifndef _DDI_WINNT_H
#define _DDI_WINNT_H
#include_next <windows/winnt.h>

#ifndef _SLIST_HEADER_
#define _SLIST_HEADER_
#define SLIST_ENTRY SINGLE_LIST_ENTRY
#define _SLIST_ENTRY _SINGLE_LIST_ENTRY
#define PSLIST_ENTRY PSINGLE_LIST_ENTRY
typedef union _SLIST_HEADER {
	ULONGLONG Alignment;
	_ANONYMOUS_STRUCT struct {
		SLIST_ENTRY Next;
		WORD Depth;
		WORD Sequence;
	} DUMMYSTRUCTNAME;
} SLIST_HEADER,*PSLIST_HEADER;
#endif

typedef BYTE FCHAR;
typedef WORD FSHORT;
typedef DWORD FLONG;

#endif
