/*
 *  Arnold emulator (c) Copyright, Kevin Thacker 2015
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
#ifndef __STACK_WINDOW_HEADER_INCLUDED__
#define __STACK_WINDOW_HEADER_INCLUDED__

#define STACK_FLAGS_SHOW_DATA			0x0001
#define STACK_FLAGS_SHOW_OFFSET			0x0008
#define STACK_FLAGS_FOLLOW_SP           0x0010

#include "../memrange.h"

typedef struct
{
	int PersistentFlags;
	int BaseAddress;
	MemoryRange *pRange;
} STACK_SETTINGS;

typedef struct
{
	/* these flags are toggled on/off */
	unsigned long PersistentFlags;

	int BaseAddress;

    int nNumberSize;
    int nNumberBase;

	int WidthInChars;
	int WindowHeight;
	int CursorXRelative;
	int CursorYRelative;
	int CursorYAbsolute;


	int *AddressessVisible;

	/* current address that is being STACKd */
	int	CurrentAddr;

	/* program counter - used for displaying STACKd listing */
	int SP;

	MemoryRange *pRange;
} STACK_WINDOW;

void Stack_ToSettings(STACK_WINDOW *pWindow, STACK_SETTINGS *pSettings);
void Stack_FromSettings(STACK_WINDOW *pWindow, STACK_SETTINGS *pSettings);
STACK_WINDOW *Stack_Create(void);
void	Stack_CursorUp(STACK_WINDOW *pWindow);
void	Stack_CursorDown(STACK_WINDOW *pWindow);
void		Stack_SelectByCharXY(STACK_WINDOW *pStackWindow, int CharX,int CharY);
int		Stack_GetCursorAddress(STACK_WINDOW *pWindow);
void	Stack_SetAddress(STACK_WINDOW *pWindow, int Address);
int		Stack_GetLineAddress(STACK_WINDOW *pWindow, int nLine);

BOOL Stack_FollowSP(STACK_WINDOW *pStackWindow);
void Stack_SetFollowSP(STACK_WINDOW *pStackWindow, BOOL bState);

BOOL Stack_ShowAddressOffset(STACK_WINDOW *pStackWindow);
void Stack_SetShowAddressOffset(STACK_WINDOW *pStackWindow, BOOL bState);


BOOL	Stack_ShowData(STACK_WINDOW *pStackWindow);
BOOL	Stack_ShowAscii(STACK_WINDOW *pStackWindow);

void	Stack_ToggleOpcodes(STACK_WINDOW *pStackWindow);
void	Stack_ToggleAscii(STACK_WINDOW *pStackWindow);
void	Stack_ToggleBreakpoint(STACK_WINDOW *pStackWindow);


void	Stack_Finish(STACK_WINDOW *pStackWindow);

int	Stack_SetFlagsBasedOnAddress(STACK_WINDOW *pStackWindow,int Address);

char    *Stack_DissassembleLine(STACK_WINDOW *pStackWindow, int Addr, int *OpcodeSize, unsigned long Flags);
void	Stack_PageDown(STACK_WINDOW *pStackWindow);
void	Stack_RefreshState(STACK_WINDOW *pStackWindow);
void	Stack_PageUp(STACK_WINDOW *pStackWindow);
void	Stack_RefreshAddress(STACK_WINDOW *pStackWindow, BOOL bStopped);
char *Stack_OutputNextLine(STACK_WINDOW *pStackWindow);


void    Stack_SetMemoryRange(STACK_WINDOW *pStackWindow, MemoryRange *pRange);

#endif
