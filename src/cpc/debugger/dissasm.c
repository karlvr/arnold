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
#include "dissasm.h"
#include "gdebug.h"
#include "breakpt.h"
#include "../cpc.h"
#include "../asic.h"
#include "../gendiss.h"



static void Dissassemble_SetBaseAddress(DISSASSEMBLE_WINDOW *pWindow, int Addr)
{
	pWindow->BaseAddress = Addr;
	pWindow->bCalcEndAddress = TRUE;
    if (pWindow->BaseAddress<0)
    {
        pWindow->BaseAddress+=MemoryRange_GetSize(pWindow->pRange);
    }
   pWindow->BaseAddress = pWindow->BaseAddress % MemoryRange_GetSize(pWindow->pRange);
}

DISSASSEMBLE_WINDOW *Dissassemble_Create(void)
{
	DISSASSEMBLE_WINDOW *pDissassembleWindow;

	pDissassembleWindow = (DISSASSEMBLE_WINDOW *)malloc(sizeof(DISSASSEMBLE_WINDOW));

	if (pDissassembleWindow!=NULL)
	{
		memset(pDissassembleWindow, 0, sizeof(DISSASSEMBLE_WINDOW));
		Dissassemble_SetSevenBitASCII(pDissassembleWindow, FALSE);
		Dissassemble_SetShowAscii(pDissassembleWindow, TRUE);
		Dissassemble_SetShowOpcodes(pDissassembleWindow, TRUE);
		Dissassemble_SetShowLabels(pDissassembleWindow, TRUE);
		Dissassemble_SetFollowPC(pDissassembleWindow, TRUE);
		pDissassembleWindow->bCalcEndAddress = TRUE;
	}

	return pDissassembleWindow;

}

/* what the following piece of code does is the following:

  When we go up a line it attempts to find the instruction,
  which when the opcode count is added onto the address for
  that instruction will give the address of the instruction
  on the screen */
#define MAX_INSTRUCTION_SIZE 4

int		Dissassemble_GetPreviousPageBase(DISSASSEMBLE_WINDOW *pWindow, int Address)
{
	int i;
	int OpcodeCount;
	int CurrentBaseAddress;

	CurrentBaseAddress = Address;

	/* maximum number of bytes in the dissassembly view =
	 max number of bytes per instruction * window height

	 The aim of the following code is to work out a suitable base address
	 for the previous page of dissassembly, so that if we dissassemble from this address
	 we end up with pWindow->WindowHeight lines or greater, before we reach "Address".

	 If the number of lines is greater than "pWindow->WindowHeight", we adjust the address
	 by the size of the first opcode. Hopefully this should give a close approximation to
	 paging up.
	*/
	for (i=0; i<(MAX_INSTRUCTION_SIZE*(pWindow->WindowHeight+1)); i++)
	{
		int TempAddress;
		int LineCount;

		LineCount = 0;
		TempAddress = CurrentBaseAddress;
		do
		{
			/* get opcode count for this address */
			OpcodeCount = pWindow->GetOpcodeCountFunction(pWindow->pRange,TempAddress);

			TempAddress += OpcodeCount;

			if (TempAddress<=Address)
			{
				LineCount++;
			}
		}
		while ((TempAddress<Address) && (LineCount<pWindow->WindowHeight));

		if (LineCount >= pWindow->WindowHeight)
		{
			if (LineCount>pWindow->WindowHeight)
			{
				CurrentBaseAddress += pWindow->GetOpcodeCountFunction(pWindow->pRange,CurrentBaseAddress);
			}
			return CurrentBaseAddress;
		}

		CurrentBaseAddress--;
	}

	/* panic!!! */
	return Address;
}


/* cursor up and page up are more difficult, because the opcodes
can be any size */
void	Dissassemble_CursorUp(DISSASSEMBLE_WINDOW *pWindow)
{
	pWindow->CursorYRelative--;

	if (pWindow->CursorYRelative<0)
	{
		int i;
		int Addr;
		int OpcodeCount;

		int PreviousPageBase;

		pWindow->CursorYRelative = 0;

		PreviousPageBase = Dissassemble_GetPreviousPageBase(pWindow,pWindow->BaseAddress);


		Addr = PreviousPageBase;

		for (i=0; i<pWindow->WindowHeight-1; i++)
		{
			/* update opcode count */
			OpcodeCount = pWindow->GetOpcodeCountFunction(pWindow->pRange,Addr);

			Addr+=OpcodeCount;
		}
		Dissassemble_SetBaseAddress(pWindow, Addr);
	}

	pWindow->CursorYAbsolute = pWindow->CursorYRelative;

	Dissassemble_RefreshAddress(pWindow, FALSE);
}


void	Dissassemble_CursorDown(DISSASSEMBLE_WINDOW *pWindow)
{
	pWindow->CursorYRelative++;

	if (pWindow->CursorYRelative>=(pWindow->WindowHeight-1))
	{
		pWindow->CursorYRelative = (pWindow->WindowHeight-1);
		Dissassemble_SetBaseAddress(pWindow, pWindow->BaseAddress + pWindow->GetOpcodeCountFunction(pWindow->pRange,pWindow->BaseAddress));
	}

	pWindow->CursorYAbsolute = pWindow->CursorYRelative;

	Dissassemble_RefreshAddress(pWindow, FALSE);
}


void Dissassemble_SetID(DISSASSEMBLE_WINDOW *pWindow, int ID)
{
	pWindow->ID = ID;
}

int Dissassemble_GetID(DISSASSEMBLE_WINDOW *pWindow)
{
	return pWindow->ID;
}

void Dissassemble_ToSettings(DISSASSEMBLE_WINDOW *pWindow, DISSASSEMBLE_SETTINGS *pSettings)
{
	pSettings->PersistentFlags = pWindow->PersistentFlags;
	pSettings->BaseAddress = pWindow->BaseAddress;
	pSettings->ViewType = pWindow->ViewType;
	pSettings->pRange = pWindow->pRange;
}

void Dissassemble_FromSettings(DISSASSEMBLE_WINDOW *pWindow, DISSASSEMBLE_SETTINGS *pSettings)
{
	pWindow->PersistentFlags = pSettings->PersistentFlags;
	pWindow->ViewType = pSettings->ViewType;
	pWindow->pRange = pSettings->pRange;
	Dissassemble_SetBaseAddress(pWindow, pSettings->BaseAddress);
	Dissassemble_RefreshAddress(pWindow, FALSE);
}


int Dissassemble_GetStartAddress(DISSASSEMBLE_WINDOW *pWindow)
{
    return pWindow->BaseAddress;
}

int Dissassemble_GetEndAddress(DISSASSEMBLE_WINDOW *pWindow)
{
	if (!pWindow->bCalcEndAddress)
	{
		return pWindow->CachedEndAddr % MemoryRange_GetSize(pWindow->pRange);
	}
	else
	{
		/* recalculate */
	    int i;
		int Addr;
		int OpcodeCount;

		Addr = pWindow->BaseAddress;

		for (i=0; i<pWindow->WindowHeight; i++)
		{
			/* update opcode count */
			OpcodeCount = pWindow->GetOpcodeCountFunction(pWindow->pRange,Addr);

			Addr+=OpcodeCount;
		}
		pWindow->CachedEndAddr = Addr;
		pWindow->bCalcEndAddress = FALSE;
	}

    return pWindow->CachedEndAddr % MemoryRange_GetSize(pWindow->pRange);
}


void	Dissassemble_PageUp(DISSASSEMBLE_WINDOW *pWindow)
{
	Dissassemble_SetBaseAddress(pWindow, Dissassemble_GetPreviousPageBase(pWindow,pWindow->BaseAddress));
	Dissassemble_RefreshAddress(pWindow, FALSE);

}

/* page down is easy because we can calculate the last address
we are displaying and show that for the next page */
void	Dissassemble_PageDown(DISSASSEMBLE_WINDOW *pWindow)
{
	int EndAddr = Dissassemble_GetEndAddress(pWindow);

	Dissassemble_SetBaseAddress(pWindow, EndAddr);
	Dissassemble_RefreshAddress(pWindow, FALSE);
}


/* call after a resize operation */
void	Dissassemble_RefreshState(DISSASSEMBLE_WINDOW *pWindow)
{
	/* ensure Y position is valid */
	if (pWindow->CursorYRelative>=pWindow->WindowHeight)
	{
		pWindow->CursorYRelative = pWindow->WindowHeight-1;
	}

	pWindow->CursorYAbsolute = pWindow->CursorYRelative;
}

void		Dissassemble_SelectByCharXY(DISSASSEMBLE_WINDOW *pDissassembleWindow, int CharX,int CharY)
{
	pDissassembleWindow->CursorYRelative = CharY;
	pDissassembleWindow->CursorYAbsolute = CharY;
}


int		Dissassemble_GetCursorAddress(DISSASSEMBLE_WINDOW *pWindow)
{
	int i;
	int Addr;
	int OpcodeCount;

	Addr = pWindow->BaseAddress;

	for (i=0; i<pWindow->CursorYRelative; i++)
	{
		/* update opcode count */
		OpcodeCount = pWindow->GetOpcodeCountFunction(pWindow->pRange,Addr);

		Addr+=OpcodeCount;
	}

	return Addr % MemoryRange_GetSize(pWindow->pRange);
}


int		Dissassemble_GetLineAddress(DISSASSEMBLE_WINDOW *pWindow, int nLine)
{
	int i;
	int Addr;
	int OpcodeCount;

    if (nLine<0)
    {
        nLine = 0;
    }
    
	Addr = pWindow->BaseAddress;

	for (i=0; i<nLine; i++)
	{
		/* update opcode count */
		OpcodeCount = pWindow->GetOpcodeCountFunction(pWindow->pRange,Addr);

		Addr+=OpcodeCount;
	}

	return Addr % MemoryRange_GetSize(pWindow->pRange);
}


static void	Dissassemble_SetAddressInternal(DISSASSEMBLE_WINDOW *pWindow, int Address)
{
	int i;
	int Addr;
	int OpcodeCount;

	Addr = pWindow->BaseAddress;

	for (i=0; i<pWindow->WindowHeight; i++)
	{
		/* don't set address it is already visible */
		if (Addr == Address)
		{
			return;
		}

		/* update opcode count */
		OpcodeCount = pWindow->GetOpcodeCountFunction(pWindow->pRange,Addr);
		Addr+=OpcodeCount;
	}

	/* address is not visible. Set new address */
	Dissassemble_SetBaseAddress(pWindow, Address);
}

void Dissassemble_SetAddress(DISSASSEMBLE_WINDOW *pWindow, int Address)
{
	Dissassemble_SetAddressInternal(pWindow, Address);
	Dissassemble_RefreshAddress(pWindow, FALSE);
}

int Dissassemble_GetAddress(DISSASSEMBLE_WINDOW *pDissassembleWindow)
{
	return pDissassembleWindow->BaseAddress % MemoryRange_GetSize(pDissassembleWindow->pRange);
}


BOOL	Dissassemble_ShowOpcodes(DISSASSEMBLE_WINDOW *pDissassembleWindow)
{
	return ((pDissassembleWindow->PersistentFlags & DISSASSEMBLE_FLAGS_SHOW_OPCODES)!=0);
}

BOOL	Dissassemble_ShowAscii(DISSASSEMBLE_WINDOW *pDissassembleWindow)
{
	return ((pDissassembleWindow->PersistentFlags & DISSASSEMBLE_FLAGS_SHOW_ASCII)!=0);
}

BOOL	Dissassemble_ShowLabels(DISSASSEMBLE_WINDOW *pDissassembleWindow)
{
	return ((pDissassembleWindow->PersistentFlags & DISSASSEMBLE_FLAGS_SHOW_LABELS)!=0);
}


BOOL	Dissassemble_ShowInstructionTimings(DISSASSEMBLE_WINDOW *pDissassembleWindow)
{
	return ((pDissassembleWindow->PersistentFlags & DISSASSEMBLE_FLAGS_SHOW_INSTRUCTION_TIMINGS)!=0);
}


BOOL	Dissassemble_FollowPC(DISSASSEMBLE_WINDOW *pDissassembleWindow)
{
	return ((pDissassembleWindow->PersistentFlags & DISSASSEMBLE_FLAGS_FOLLOW_PC)!=0);
}


void	Dissassemble_SetFollowPC(DISSASSEMBLE_WINDOW *pDissassembleWindow, BOOL bState)
{
	if (bState)
	{

		pDissassembleWindow->PersistentFlags |= DISSASSEMBLE_FLAGS_FOLLOW_PC;
	}
	else
	{
		pDissassembleWindow->PersistentFlags &= ~DISSASSEMBLE_FLAGS_FOLLOW_PC;

	}
}

void	Dissassemble_SetShowOpcodes(DISSASSEMBLE_WINDOW *pDissassembleWindow, BOOL bState)
{
	if (bState)
	{

		pDissassembleWindow->PersistentFlags |= DISSASSEMBLE_FLAGS_SHOW_OPCODES;
	}
	else
	{
		pDissassembleWindow->PersistentFlags &= ~DISSASSEMBLE_FLAGS_SHOW_OPCODES;

	}
}

void	Dissassemble_SetShowLabels(DISSASSEMBLE_WINDOW *pDissassembleWindow, BOOL bState)
{
	if (bState)
	{

		pDissassembleWindow->PersistentFlags |= DISSASSEMBLE_FLAGS_SHOW_LABELS;
	}
	else
	{
		pDissassembleWindow->PersistentFlags &= ~DISSASSEMBLE_FLAGS_SHOW_LABELS;

	}
}



void	Dissassemble_SetShowInstructionTimings(DISSASSEMBLE_WINDOW *pDissassembleWindow, BOOL bState)
{
	if (bState)
	{

		pDissassembleWindow->PersistentFlags |= DISSASSEMBLE_FLAGS_SHOW_INSTRUCTION_TIMINGS;
	}
	else
	{
		pDissassembleWindow->PersistentFlags &= ~DISSASSEMBLE_FLAGS_SHOW_INSTRUCTION_TIMINGS;

	}
}

void	Dissassemble_SetShowAscii(DISSASSEMBLE_WINDOW *pDissassembleWindow, BOOL bState)
{
	if (bState)
	{

		pDissassembleWindow->PersistentFlags |= DISSASSEMBLE_FLAGS_SHOW_ASCII;
	}
	else
	{
		pDissassembleWindow->PersistentFlags &= ~DISSASSEMBLE_FLAGS_SHOW_ASCII;

	}
}

#if 0
void	Dissassemble_ToggleBreakpoint(DISSASSEMBLE_WINDOW *pDissassembleWindow)
{
	int Addr;

	if (pDissassembleWindow->ViewType != DISSASSEMBLE_VIEW_Z80_INSTRUCTIONS)
	{
		return;
	}

	Addr = Dissassemble_GetCursorAddress(pDissassembleWindow);

	/* breakpoint exists? */
	if (Breakpoints_IsAVisibleBreakpoint(Addr))
	{
		/* yes, remove it */
		Breakpoints_RemoveBreakpointByAddress(Addr);
	}
	else
	{
		/* no, add one */
		Breakpoints_AddBreakpoint(Addr);
	}
}
#endif



void	Dissassemble_Finish(DISSASSEMBLE_WINDOW *pDissassembleWindow)
{
	if (pDissassembleWindow!=NULL)
	{
		free(pDissassembleWindow);
	}
}


void Dissassemble_SetPC(DISSASSEMBLE_WINDOW *pDissassembleWindow, int Address)
{
	switch (pDissassembleWindow->ViewType)
	{
	case DISSASSEMBLE_VIEW_Z80_INSTRUCTIONS:
		{
			CPU_SetReg(CPU_PC, Address);
			pDissassembleWindow->PC = CPU_GetPC();
		}
		break;

	case DISSASSEMBLE_VIEW_DMA_CHANNEL_0_INSTRUCTIONS:
	case DISSASSEMBLE_VIEW_DMA_CHANNEL_1_INSTRUCTIONS:
	case DISSASSEMBLE_VIEW_DMA_CHANNEL_2_INSTRUCTIONS:
		{
			ASIC_DMA_SetChannelAddr(pDissassembleWindow->ViewType - DISSASSEMBLE_VIEW_DMA_CHANNEL_0_INSTRUCTIONS, Address);
			pDissassembleWindow->PC = ASIC_DMA_GetChannelAddr(pDissassembleWindow->ViewType - DISSASSEMBLE_VIEW_DMA_CHANNEL_0_INSTRUCTIONS);
		}
		break;

		default
			:
				break;
	}
}




void Dissassemble_GetPC(DISSASSEMBLE_WINDOW *pDissassembleWindow)
{
	switch (pDissassembleWindow->ViewType)
	{
		case DISSASSEMBLE_VIEW_Z80_INSTRUCTIONS:
		{
			pDissassembleWindow->PC = CPU_GetPC();
		}
		break;

		case DISSASSEMBLE_VIEW_DMA_CHANNEL_0_INSTRUCTIONS:
		case DISSASSEMBLE_VIEW_DMA_CHANNEL_1_INSTRUCTIONS:
		case DISSASSEMBLE_VIEW_DMA_CHANNEL_2_INSTRUCTIONS:
		{
			pDissassembleWindow->PC = ASIC_DMA_GetChannelAddr(pDissassembleWindow->ViewType - DISSASSEMBLE_VIEW_DMA_CHANNEL_0_INSTRUCTIONS);
		}
		break;

		default
				:
			break;
	}
}


extern const char *GetLabelForAddress(MemoryRange *pRange,int Addr);

static char DissassembleString[256];

char    *Dissassemble_DissassembleLine(DISSASSEMBLE_WINDOW *pWindow, int Addr, int *OpcodeSize, unsigned long Flags)
{
	int OpcodeCount;
	int x;
	int nPad = 0;

	char *pDissString = DissassembleString;
	char *pDissStringPreInstruction;
	const char *sLabel = NULL;
	
	if ((Flags & DISSASSEMBLE_FLAGS_SHOW_LABELS)!=0)
	{
		sLabel = GetLabelForAddress(pWindow->pRange,Addr);
	}


	if (sLabel!=NULL)
	{
		int nLabelLength = strlen(sLabel);
		if (nLabelLength>15)
		{
			nLabelLength = 15;
		}
		strncpy(pDissString, sLabel, nLabelLength);
		pDissString+=nLabelLength;
		pDissString[0] = ':';
		++pDissString;
		nPad = 16-(nLabelLength+1);
	}
	else
	{
		pDissString = Diss_WriteHexWord(pDissString, Addr % MemoryRange_GetSize(pWindow->pRange),FALSE, FALSE);
		pDissString[0] = ':';
		++pDissString;
		nPad = 16-5;
	}
	for (x=0; x<nPad; x++)
	{
		pDissString[0] = ' ';
		++pDissString;
	}

	OpcodeCount = pWindow->GetOpcodeCountFunction(pWindow->pRange,Addr);

	/* display opcodes ?*/
	if (Flags & DISSASSEMBLE_FLAGS_SHOW_OPCODES)
	{
		int i;

		for (i=0; i<OpcodeCount; i++)
		{
				pDissString = Diss_WriteHexByte(pDissString,MemoryRange_ReadByte(pWindow->pRange, Addr+i),FALSE, FALSE);
			pDissString[0] = ' ';
			++pDissString;
		}

		nPad = 4*3-(OpcodeCount*3);

		for (x=0; x<nPad; x++)
		{
			pDissString[0] = ' ';
			++pDissString;
		}
	}

	if (Flags & DISSASSEMBLE_FLAGS_SHOW_ASCII)
	{
		int i;
		int byte;

		for (i=0; i<OpcodeCount; i++)
		{
            byte = MemoryRange_ReadByte(pWindow->pRange, Addr + i);
			pDissString = Diss_WriteAscii(pDissString, byte, (pWindow->PersistentFlags & DISSASSEMBLE_FLAGS_SEVEN_BIT_ASCII));

		}

		nPad = 5*1 - OpcodeCount;
		for (x = 0; x<nPad; x++)
		{
			pDissString[0] = ' ';
			++pDissString;
		}
	}

	/* write mneumonic */
	pDissStringPreInstruction = pDissString;
        pDissString = pWindow->DissassembleInstruction(pWindow->pRange, Addr, pDissString);

	if (Flags & DISSASSEMBLE_FLAGS_SHOW_INSTRUCTION_TIMINGS)
	{
		int		nCount;

		nPad = 20-(pDissString-pDissStringPreInstruction);
		for (x=0; x<nPad; x++)
		{
			pDissString[0] = ' ';
			++pDissString;
		}

		nCount = pWindow->GetInstructionTiming(pWindow->pRange, Addr);
		pDissString[0] = '[';
		++pDissString;
		if (nCount<0)
		{
			pDissString[0] = '?';
		}
		else
		{
			pDissString[0] = nCount+'0';
		}
		++pDissString;
		pDissString[0] = ']';
		++pDissString;

	}
	Diss_endstring(pDissString);

	*OpcodeSize = OpcodeCount;

	return DissassembleString;
}

void Dissassemble_RefreshAddress(DISSASSEMBLE_WINDOW *pDissassembleWindow,BOOL bRefreshAddressFromPC)
{
	Dissassemble_GetPC(pDissassembleWindow);

	if (bRefreshAddressFromPC)
	{
		Dissassemble_SetAddressInternal(pDissassembleWindow, pDissassembleWindow->PC);
	}
	pDissassembleWindow->CurrentAddr = pDissassembleWindow->BaseAddress;

	switch (pDissassembleWindow->ViewType)
	{
		case DISSASSEMBLE_VIEW_Z80_INSTRUCTIONS:
		{
			pDissassembleWindow->CurrentAddr &= 0x0ffff;
		}
		break;

		case DISSASSEMBLE_VIEW_DMA_CHANNEL_0_INSTRUCTIONS:
		case DISSASSEMBLE_VIEW_DMA_CHANNEL_1_INSTRUCTIONS:
		case DISSASSEMBLE_VIEW_DMA_CHANNEL_2_INSTRUCTIONS:
		{

			pDissassembleWindow->CurrentAddr &= 0x0ffff;
		}
		break;

		default
				:
			break;
	}

}

extern int Z80_GetNopCountForInstruction(MemoryRange *pRange, int);

void	Dissassemble_BeginDissassemble(DISSASSEMBLE_WINDOW *pDissassembleWindow)
{

	switch (pDissassembleWindow->ViewType)
	{
		case DISSASSEMBLE_VIEW_Z80_INSTRUCTIONS:
		{
			pDissassembleWindow->GetOpcodeCountFunction = Debug_GetOpcodeCount;
			pDissassembleWindow->DissassembleInstruction = Debug_DissassembleInstruction;
			pDissassembleWindow->GetInstructionTiming = Z80_GetNopCountForInstruction;
		}
		break;

		case DISSASSEMBLE_VIEW_DMA_CHANNEL_0_INSTRUCTIONS:
		case DISSASSEMBLE_VIEW_DMA_CHANNEL_1_INSTRUCTIONS:
		case DISSASSEMBLE_VIEW_DMA_CHANNEL_2_INSTRUCTIONS:
		{

			pDissassembleWindow->GetOpcodeCountFunction = ASIC_DMA_GetOpcodeCount;
			pDissassembleWindow->DissassembleInstruction = ASIC_DMA_DissassembleInstruction;
			pDissassembleWindow->GetInstructionTiming = ASIC_DMA_GetInstructionTiming;
		}
		break;

		default
				:
			break;
	}

}


char *Dissassemble_DissassembleNextLine(DISSASSEMBLE_WINDOW *pDissassembleWindow)
{
	int	OpcodeCount;
	char *pDebugString;
	unsigned long DissassemblyFlags;

/*	DissassemblyFlags = Dissassemble_SetDissassemblyFlagsBasedOnAddress(pDissassembleWindow,pDissassembleWindow->CurrentAddr); */
	DissassemblyFlags = pDissassembleWindow->PersistentFlags;

	pDebugString = Dissassemble_DissassembleLine(pDissassembleWindow,pDissassembleWindow->CurrentAddr, &OpcodeCount,DissassemblyFlags);

	pDissassembleWindow->CurrentAddr+= OpcodeCount;
	pDissassembleWindow->CurrentAddr&=0x0ffff;

	return pDebugString;
}

void	Dissassemble_SetView(DISSASSEMBLE_WINDOW *pDissassembleWindow, int nViewType)
{
	pDissassembleWindow->ViewType = nViewType;

	Dissassemble_RefreshState(pDissassembleWindow);
}

int Dissassemble_GetView(DISSASSEMBLE_WINDOW *pDissassembleWindow)
{
	return pDissassembleWindow->ViewType;
}

#if 0
char	*Dissassemble_GetViewName(DISSASSEMBLE_WINDOW *pDissassembleWindow)
{
	switch (pDissassembleWindow->ViewType)
	{
		case DISSASSEMBLE_VIEW_Z80_INSTRUCTIONS:
			return Messages[115];

		case DISSASSEMBLE_VIEW_DMA_CHANNEL_0_INSTRUCTIONS:
			return Messages[116];

		case DISSASSEMBLE_VIEW_DMA_CHANNEL_1_INSTRUCTIONS:
			return Messages[117];

		case DISSASSEMBLE_VIEW_DMA_CHANNEL_2_INSTRUCTIONS:
			return Messages[118];

		default
				:
			break;
	}

	return "";
}
#endif
void Dissassemble_SetMemoryRange(DISSASSEMBLE_WINDOW *pDissassembleWindow, MemoryRange *pRange)
{

    pDissassembleWindow->pRange = pRange;
}

MemoryRange *Dissassemble_GetMemoryRange(DISSASSEMBLE_WINDOW *pDissassembleWindow)
{

    return pDissassembleWindow->pRange;
}

BOOL Dissassemble_GetSevenBitASCII(DISSASSEMBLE_WINDOW *pDissassembleWindow)
{
  return pDissassembleWindow->PersistentFlags & DISSASSEMBLE_FLAGS_SEVEN_BIT_ASCII;
}

void Dissassemble_SetSevenBitASCII(DISSASSEMBLE_WINDOW *pDissassembleWindow, BOOL bState)
{
	pDissassembleWindow->PersistentFlags &= ~DISSASSEMBLE_FLAGS_SEVEN_BIT_ASCII;
	if (bState)
	{
		pDissassembleWindow->PersistentFlags |= DISSASSEMBLE_FLAGS_SEVEN_BIT_ASCII;
	}
}
