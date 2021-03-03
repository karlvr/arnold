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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "../cpcglob.h"
#include "stack.h"
#include "../gendiss.h"

#include "../cpc.h"

STACK_WINDOW *Stack_Create(void)
{
	STACK_WINDOW *pStackWindow;

	pStackWindow = (STACK_WINDOW *)malloc(sizeof(STACK_WINDOW));

	if (pStackWindow!=NULL)
	{
		memset(pStackWindow, 0, sizeof(STACK_WINDOW));
		pStackWindow->nNumberBase = NUMBER_BASE_HEXADECIMAL;
		pStackWindow->nNumberSize = NUMBER_SIZE_WORD;
		Stack_SetFollowSP(pStackWindow, TRUE);
	}

	return pStackWindow;
	
}

void Stack_ToSettings(STACK_WINDOW *pStackWindow, STACK_SETTINGS *pSettings)
{
	pSettings->PersistentFlags = pStackWindow->PersistentFlags;
	pSettings->BaseAddress = pStackWindow->BaseAddress;
	pSettings->pRange = pStackWindow->pRange;
}

void Stack_FromSettings(STACK_WINDOW *pStackWindow, STACK_SETTINGS *pSettings)
{
	pStackWindow->PersistentFlags = pSettings->PersistentFlags;
	pStackWindow->BaseAddress = pSettings->BaseAddress;
	pStackWindow->pRange = pSettings->pRange;
	Stack_RefreshAddress(pStackWindow, FALSE);
}


BOOL Stack_FollowSP(STACK_WINDOW *pStackWindow)
{
	return ((pStackWindow->PersistentFlags & STACK_FLAGS_FOLLOW_SP)!=0);
}

void Stack_SetFollowSP(STACK_WINDOW *pStackWindow, BOOL bState)
{
	if (bState)
	{
		pStackWindow->PersistentFlags |= STACK_FLAGS_FOLLOW_SP;

	}
	else
	{

		pStackWindow->PersistentFlags &=~STACK_FLAGS_FOLLOW_SP;
	}
}


BOOL Stack_ShowAddressOffset(STACK_WINDOW *pStackWindow)
{
	return ((pStackWindow->PersistentFlags & STACK_FLAGS_SHOW_OFFSET)!=0);
}

void Stack_SetShowAddressOffset(STACK_WINDOW *pStackWindow, BOOL bState)
{
	if (bState)
	{
		pStackWindow->PersistentFlags |= STACK_FLAGS_SHOW_OFFSET;

	}
	else
	{

		pStackWindow->PersistentFlags &=~STACK_FLAGS_SHOW_OFFSET;
	}
}

/* cursor up and page up are more difficult, because the opcodes
can be any size */
void	Stack_CursorUp(STACK_WINDOW *pWindow)
{
	pWindow->CursorYRelative--;

	if (pWindow->CursorYRelative<0)
	{
		pWindow->CursorYRelative = 0;

		pWindow->BaseAddress = (pWindow->BaseAddress-2)&0x0ffff;
	}

	pWindow->CursorYAbsolute = pWindow->CursorYRelative;
}


void	Stack_CursorDown(STACK_WINDOW *pWindow)
{
	pWindow->CursorYRelative++;

	if (pWindow->CursorYRelative>=(pWindow->WindowHeight-1))
	{
		pWindow->CursorYRelative = (pWindow->WindowHeight-1);
		pWindow->BaseAddress = (pWindow->BaseAddress+2)&0x0ffff;
	}

	pWindow->CursorYAbsolute = pWindow->CursorYRelative;
	Stack_RefreshAddress(pWindow, FALSE);

}


void	Stack_PageUp(STACK_WINDOW *pWindow)
{
	pWindow->BaseAddress = pWindow->BaseAddress-(pWindow->WindowHeight*2);
	Stack_RefreshAddress(pWindow, FALSE);

}

/* page down is easy because we can calculate the last address
we are displaying and show that for the next page */
void	Stack_PageDown(STACK_WINDOW *pWindow)
{
	pWindow->BaseAddress = pWindow->BaseAddress+(pWindow->WindowHeight*2);
	Stack_RefreshAddress(pWindow, FALSE);

}


/* call after a resize operation */
void	Stack_RefreshState(STACK_WINDOW *pWindow)
{
	/* ensure Y position is valid */
	if (pWindow->CursorYRelative>=pWindow->WindowHeight)
	{
		pWindow->CursorYRelative = pWindow->WindowHeight-1;
	}

	pWindow->CursorYAbsolute = pWindow->CursorYRelative;
}

void		Stack_SelectByCharXY(STACK_WINDOW *pStackWindow, int CharX,int CharY)
{
	pStackWindow->CursorYRelative = CharY;
	pStackWindow->CursorYAbsolute = CharY;
}


int		Stack_GetCursorAddress(STACK_WINDOW *pWindow)
{
	return (pWindow->BaseAddress + (pWindow->CursorYRelative*2)) % MemoryRange_GetSize(pWindow->pRange);
}

/* get memory address for start of line */
int Stack_GetLineAddress(STACK_WINDOW *pWindow, int Line)
{
	return (pWindow->BaseAddress + (Line*2)) % MemoryRange_GetSize(pWindow->pRange);
}

static void	Stack_SetAddressInternal(STACK_WINDOW *pWindow, int Address)
{
	int TopAddress = pWindow->BaseAddress;
	int BottomAddress = pWindow->BaseAddress+(pWindow->WindowHeight<<1);
	if (
		/* within address range */
		(Address>=TopAddress) &&
		(Address<=BottomAddress) &&
		/* AND has the same offset */
		(((Address^TopAddress)&1)!=0)
	)
	{
		return;
	}

	/* TODO: Better logic which tries to keep address near middle of viewport?


	Set it
	 happens if outside of window above, or below, or ends up being odd
	 when we are showing from even or similar */
	pWindow->BaseAddress = Address-((pWindow->WindowHeight>>1)<<1);

}

void	Stack_SetAddress(STACK_WINDOW *pWindow, int Address)
{
	Stack_SetAddressInternal(pWindow, Address);
	Stack_RefreshAddress(pWindow, FALSE);
}

BOOL	Stack_ShowData(STACK_WINDOW *pStackWindow)
{
	return ((pStackWindow->PersistentFlags & STACK_FLAGS_SHOW_DATA)!=0);
}

#if 0
BOOL	Stack_ShowAscii(STACK_WINDOW *pStackWindow)
{
	return ((pStackWindow->PersistentFlags & STACK_FLAGS_SHOW_ASCII)!=0);
}
#endif


void	Stack_ToggleData(STACK_WINDOW *pStackWindow)
{
	pStackWindow->PersistentFlags ^= STACK_FLAGS_SHOW_DATA;
}

#if 0
void	Stack_ToggleAscii(STACK_WINDOW *pStackWindow)
{
	pStackWindow->PersistentFlags ^= STACK_FLAGS_SHOW_ASCII;
}
#endif

void	Stack_Finish(STACK_WINDOW *pStackWindow)
{
	if (pStackWindow!=NULL)
	{
		free(pStackWindow);
	}
}

#if 0
int	Stack_SetFlagsBasedOnAddress(STACK_WINDOW *pStackWindow,int Address)
{
	int Flags;

	Flags = 0;

	/* mark line with PC marker */
	if (Address == pStackWindow->SP)
	{
		Flags |= STACK_FLAGS_MARK_AS_SP;
	}

	return Flags;

}
#endif

static char DissassembleString[256];

char    *Stack_OutputLine(STACK_WINDOW *pWindow, int Addr, unsigned long Flags)
{
	char *pDissString = DissassembleString;

	pDissString = Diss_WriteHexWord(pDissString, Addr,FALSE, FALSE);
	pDissString[0] = ' ';
	++pDissString;

	if (Flags & STACK_FLAGS_SHOW_OFFSET )
	{
		/* write offset? */
		int nOffset = Addr-pWindow->SP;
		pDissString[0] = '(';
		++pDissString;

		pDissString = Diss_WriteHexWord(pDissString, nOffset, TRUE, TRUE);
		pDissString[0] = ')';
		++pDissString;
	}

	/*     pDissString[0] = ':';
	     ++pDissString;
	     nPad = 16-5;

		for (x=0; x<nPad; x++)
		{
	pDissString[0] = ' ';
	++pDissString;
		}
*/
	/* display bytes ?*/
	if (pWindow->nNumberSize==NUMBER_SIZE_BYTE)
	{
		int i;

		for (i=0; i<2; i++)
		{
			if (pWindow->nNumberBase==NUMBER_BASE_HEXADECIMAL)
			{
                    pDissString = Diss_WriteHexByte(pDissString,MemoryRange_ReadByte(pWindow->pRange,Addr+i),FALSE,FALSE);
			}
			else
			{
                    pDissString = Diss_WriteDecByte(pDissString,MemoryRange_ReadByte(pWindow->pRange,Addr+1),FALSE);
			}
			pDissString[0] = ' ';
			++pDissString;
		}

		pDissString[0] = ' ';
		++pDissString;


	}
	else
		if (pWindow->nNumberSize==NUMBER_SIZE_WORD)
		{
            int Data = MemoryRange_ReadWord(pWindow->pRange,Addr);
			if (pWindow->nNumberBase==NUMBER_BASE_HEXADECIMAL)
			{
				pDissString = Diss_WriteHexWord(pDissString,Data,FALSE,FALSE);
			}
			else
			{
				pDissString = Diss_WriteDecWord(pDissString,Data,FALSE);
			}
			pDissString[0] = ' ';
			++pDissString;

		}
#if 0
	if (Flags & STACK_FLAGS_SHOW_ASCII)
	{
		int i;
		int byte;

		for (i=0; i<2; i++)
		{
				byte = MemoryRange_ReadByte(pWindow->pRange,Addr + i);

			/* convert un-printable chars into '.' */
			if ((byte<21) || (byte>127))
			{
				byte = '.';
			}

			pDissString[0] = byte;
			++pDissString;
		}

		nPad = 4*1 - 2;
		for (x = 0; x<nPad; x++)
		{
			pDissString[0] = ' ';
			++pDissString;
		}
	}
#endif

#if 0
	/* highlight program counter? */
	if (Flags & STACK_FLAGS_MARK_AS_SP)
	{
		pDissString[0] = '>';
	}
	else
	{
		pDissString[0] = ' ';
	}
	++pDissString;
#endif
	Diss_endstring(pDissString);
	return DissassembleString;
}

void	Stack_RefreshAddress(STACK_WINDOW *pStackWindow, BOOL bRefreshAddressFromSP)
{
	pStackWindow->SP = CPU_GetReg(CPU_SP);
	if (bRefreshAddressFromSP)
	{
		Stack_SetAddressInternal(pStackWindow, pStackWindow->SP);
	}

	pStackWindow->CurrentAddr = pStackWindow->BaseAddress;


}


char *Stack_OutputNextLine(STACK_WINDOW *pStackWindow)
{
	char *pDebugString;
	unsigned long Flags;

	Flags = 0;  /*Stack_SetFlagsBasedOnAddress(pStackWindow,pStackWindow->CurrentAddr); */
	Flags |= pStackWindow->PersistentFlags;

	pDebugString = Stack_OutputLine(pStackWindow,pStackWindow->CurrentAddr,Flags);

	pStackWindow->CurrentAddr+= 2;

	return pDebugString;
}

void Stack_SetMemoryRange(STACK_WINDOW *pStackWindow, MemoryRange *pRange)
{
    pStackWindow->pRange = pRange;
}

