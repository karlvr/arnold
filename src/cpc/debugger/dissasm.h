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
#ifndef __DISSASSEMBLY_WINDOW_HEADER_INCLUDED__
#define __DISSASSEMBLY_WINDOW_HEADER_INCLUDED__

#include "../cpcglob.h"
#include "../memrange.h"

#define DISSASSEMBLE_FLAGS_SHOW_OPCODES			0x0001
#define DISSASSEMBLE_FLAGS_SHOW_ASCII			0x0002
#define DISSASSEMBLE_FLAGS_FOLLOW_PC			0x0004
#define DISSASSEMBLE_FLAGS_SHOW_INSTRUCTION_TIMINGS 0x0008
#define DISSASSEMBLE_FLAGS_SHOW_LABELS 0x0010
#define DISSASSEMBLE_FLAGS_SEVEN_BIT_ASCII 0x00020

typedef enum
{
	DISSASSEMBLE_VIEW_Z80_INSTRUCTIONS = 0,
	DISSASSEMBLE_VIEW_DMA_CHANNEL_0_INSTRUCTIONS,
	DISSASSEMBLE_VIEW_DMA_CHANNEL_1_INSTRUCTIONS,
	DISSASSEMBLE_VIEW_DMA_CHANNEL_2_INSTRUCTIONS
} DISSASSEMBLE_VIEW_TYPE;

typedef struct
{
	int PersistentFlags;
	int BaseAddress;
	DISSASSEMBLE_VIEW_TYPE ViewType;
	MemoryRange *pRange;
} DISSASSEMBLE_SETTINGS;

#define DISSASSEMBLE_VIEW_FIRST		DISSASSEMBLE_VIEW_Z80_INSTRUCTIONS
#define DISSASSEMBLE_VIEW_LAST		DISSASSEMBLE_VIEW_DMA_CHANNEL_2_INSTRUCTIONS
typedef char *CHAR_PTR;

typedef struct
{
	int ID;
	/* these flags are toggled on/off */
	unsigned long PersistentFlags;

	int BaseAddress;

	int WidthInChars;
	int WindowHeight;
	int CursorXRelative;
	int CursorYRelative;
	int CursorYAbsolute;


/*	int *AddressessVisible; */

	DISSASSEMBLE_VIEW_TYPE ViewType;

	/* current address that is being dissassembled */
	int	CurrentAddr;
	int CachedEndAddr;
	/* program counter - used for displaying dissassembled listing */
	int PC;

    MemoryRange *pRange;
BOOL bCalcEndAddress;
	int	(*GetOpcodeCountFunction)(MemoryRange *,int);
	CHAR_PTR (*DissassembleInstruction)(MemoryRange *pRange, int, char *);
	int (*GetInstructionTiming)(MemoryRange *,int);
} DISSASSEMBLE_WINDOW;

void Dissassemble_SetID(DISSASSEMBLE_WINDOW *pWindow, int ID);
int Dissassemble_GetID(DISSASSEMBLE_WINDOW *pWindow);
void Dissassemble_ToSettings(DISSASSEMBLE_WINDOW *pWindow, DISSASSEMBLE_SETTINGS *pSettings);
void Dissassemble_FromSettings(DISSASSEMBLE_WINDOW *pWindow, DISSASSEMBLE_SETTINGS *pSettings);
DISSASSEMBLE_WINDOW *Dissassemble_Create(void);
void	Dissassemble_CursorUp(DISSASSEMBLE_WINDOW *pWindow);
void	Dissassemble_CursorDown(DISSASSEMBLE_WINDOW *pWindow);
void		Dissassemble_SelectByCharXY(DISSASSEMBLE_WINDOW *pDissassembleWindow, int CharX,int CharY);
int		Dissassemble_GetCursorAddress(DISSASSEMBLE_WINDOW *pWindow);
int		Dissassemble_GetLineAddress(DISSASSEMBLE_WINDOW *pWindow, int nLine);
void	Dissassemble_SetAddress(DISSASSEMBLE_WINDOW *pWindow, int Address);
int     Dissassemble_GetAddress(DISSASSEMBLE_WINDOW *pWindow);
BOOL	Dissassemble_ShowOpcodes(DISSASSEMBLE_WINDOW *pDissassembleWindow);
BOOL	Dissassemble_ShowAscii(DISSASSEMBLE_WINDOW *pDissassembleWindow);
BOOL	Dissassemble_ShowLabels(DISSASSEMBLE_WINDOW *pDissassembleWindow);
BOOL	Dissassemble_FollowPC(DISSASSEMBLE_WINDOW *pDissassembleWindow);
BOOL	Dissassemble_ShowInstructionTimings(DISSASSEMBLE_WINDOW *pDissassembleWindow);
void	Dissassemble_GetPC(DISSASSEMBLE_WINDOW *pDissassembleWindow);
void	Dissassemble_SetPC(DISSASSEMBLE_WINDOW *pDissassembleWindow, int Address); 
void	Dissassemble_SetShowOpcodes(DISSASSEMBLE_WINDOW *pDissassembleWindow, BOOL bState);
void	Dissassemble_SetShowAscii(DISSASSEMBLE_WINDOW *pDissassembleWindow, BOOL bState);
void	Dissassemble_SetShowLabels(DISSASSEMBLE_WINDOW *pDissassembleWindow, BOOL bState);
void	Dissassemble_SetFollowPC(DISSASSEMBLE_WINDOW *pDissassembleWindow, BOOL bState);
void	Dissassemble_SetShowInstructionTimings(DISSASSEMBLE_WINDOW *pDissassembleWindow, BOOL bState);


/*void	Dissassemble_ToggleBreakpoint(DISSASSEMBLE_WINDOW *pDissassembleWindow); */


void	Dissassemble_Finish(DISSASSEMBLE_WINDOW *pDissassembleWindow);
/*int	Dissassemble_SetDissassemblyFlagsBasedOnAddress(DISSASSEMBLE_WINDOW *,int Address); */
char    *Dissassemble_DissassembleLine(DISSASSEMBLE_WINDOW *pWindow, int Addr, int *OpcodeSize, unsigned long Flags);
void	Dissassemble_PageDown(DISSASSEMBLE_WINDOW *pWindow);
void	Dissassemble_RefreshState(DISSASSEMBLE_WINDOW *pWindow);
void	Dissassemble_PageUp(DISSASSEMBLE_WINDOW *pWindow);

void Dissassemble_RefreshAddress(DISSASSEMBLE_WINDOW *pDissassembleWindow,BOOL bStopped);
void	Dissassemble_BeginDissassemble(DISSASSEMBLE_WINDOW *pDissassembleWindow);
char *Dissassemble_DissassembleNextLine(DISSASSEMBLE_WINDOW *pDissassembleWindow);
/*char	*Dissassemble_GetViewName(DISSASSEMBLE_WINDOW *pDissassembleWindow);
void	Dissassemble_ToggleView(DISSASSEMBLE_WINDOW *pDissassembleWindow); */
int Dissassemble_GetView(DISSASSEMBLE_WINDOW *pDissassembleWindow);
void Dissassemble_SetView(DISSASSEMBLE_WINDOW *pDissassembleWindow, int nViewType);
void Dissassemble_SetMemoryRange(DISSASSEMBLE_WINDOW *pDissassembleWindow, MemoryRange *pRange);
MemoryRange *Dissassemble_GetMemoryRange(DISSASSEMBLE_WINDOW *pDissassembleWindow);
void Dissassemble_SetSevenBitASCII(DISSASSEMBLE_WINDOW *pDissassembleWindow, BOOL bState);
BOOL Dissassemble_GetSevenBitASCII(DISSASSEMBLE_WINDOW *pDissassembleWindow);

int Dissassemble_GetStartAddress(DISSASSEMBLE_WINDOW *pDissassembleWindow);
int Dissassemble_GetEndAddress(DISSASSEMBLE_WINDOW *pDissassembleWindow);

#endif

