/*
 *  Arnold emulator (c) Copyright, Kevin Thacker 1995-2015
 *
 *  This file is part of the Arnold emulator source code distribution.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef __GENERIC_DEBUGGER_HEADER_INCLUDED__
#define __GENERIC_DEBUGGER_HEADER_INCLUDED__

#include "../cpcglob.h"
#include "../memrange.h"

int IsSeperatorInstruction(MemoryRange *pRange, int Address);

int		ASIC_DMA_GetOpcodeCount(MemoryRange *,int);
char *ASIC_DMA_DissassembleInstruction(MemoryRange *pRange,int, char *);
int     ASIC_DMA_GetInstructionTiming(MemoryRange *,int Address);

void	Debug_SetDebuggerWindowOpenCallback(void (*pOpenDebugWindow)(void));


/*****************************************************************************/

char	*Debug_DumpFromAddress(int Addr, int NoOfBytes, int (*pReadFunction)(int));
int		Debug_ByteSearch(MemoryRange *pRange,int AddrStart,int AddrEnd,char *pSearchString,int NumChars, unsigned char (*pReadFunction)(int));



/*void	Debug_SetState(int);*/
void	Debug_SetRunTo(int);
void Debugger_StepInstruction(void);

void	Debug_WriteMemoryToDisk(char *);
BOOL	Debug_ValidateNumberIsHex(char *HexString, int *HexNumber);
char *	Debug_DissassembleInstruction(MemoryRange *,int Addr, char *OutputString);
int		Debug_GetOpcodeCount(MemoryRange *,int Addr);
int		Debug_CalcNumberOfBytesVisibleInMemDump(int WindowWidth);
char	*Debug_DissassembleLine(int Addr, int *OpcodeSize);
char	*Debug_BinaryString(unsigned char Data);
char	*Debug_FlagsAsString(void);

void Debugger_CheckHalt(void);

BOOL    Debug_IsStopped(void);
BOOL Debug_GetDebuggerRefresh(void);
void Debug_ResetDebuggerRefresh(void);
int	Debugger_Execute(void);
void Debug_TriggerBreak(void);
void Debug_Continue(void);

#include "memdump.h"

typedef struct
{
	BOOL bSyncDissassembly;
    BOOL            bCaseInsensitive;   /* for strings */
	int				FoundAddress;
	unsigned char *pSearchString;       /* bytes and strings */
	unsigned char *pSearchStringMask;   /* both bytes and strings */
	unsigned long  NumBytes;            /* bytes and strings */
} SEARCH_DATA;

unsigned char MemoryRead(MemoryRange *,int, int);
void MemoryWrite(int, int,int);
int	Memdump_FindData(MEMDUMP_WINDOW *pMemdumpWindow,SEARCH_DATA	*pSearchData);
void DebuggerFillMemory(MemoryRange *pRange,int nStart, int nEnd, const char *pString, int nStringLength);

/* break opcode ED, FF */
BOOL Debug_BreakOpcodeEnabled(void);
void Debug_EnableBreakOpcode(BOOL bState);

#endif
