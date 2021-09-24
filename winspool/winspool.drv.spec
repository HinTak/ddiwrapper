# Copyright 2004 Ulrich Hecht <uli@suse.de>

# This file is part of ddiwrapper.

# ddiwrapper is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 2.1 as
# published by the Free Software Foundation.

@ stdcall GetPrinterDriverW(long str long ptr long ptr)
@ stdcall GetPrinterDriverA(long str long ptr long ptr)
@ stdcall ClosePrinter(long)
@ stdcall OpenPrinterW(wstr ptr ptr)
@ stdcall OpenPrinterA(str ptr ptr)
@ stdcall GetPrinterDataW(long wstr ptr ptr long ptr)
@ stdcall GetPrinterDataA(long str ptr ptr long ptr)
@ stdcall EnumFormsW(long long ptr long ptr ptr)
@ stdcall GetPrinterW(long long ptr long ptr)
@ stdcall GetPrinterA(long long ptr long ptr)
@ stdcall WritePrinter(long ptr long ptr)
@ stdcall EnumJobsW(long long long long ptr long ptr ptr)
@ stdcall StartPagePrinter(long)
@ stdcall EndPagePrinter(long)
@ stdcall GetJobW (long long long ptr long ptr)
@ stub SetJobW
@ stub GetPrinterDriverDirectoryA
