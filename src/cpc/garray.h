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
#ifndef __GATE_ARRAY_HEADER_INCLUDED__
#define __GATE_ARRAY_HEADER_INCLUDED__
		
#include "cpcglob.h"
#include "cpc.h"


/* reset gate array */
void    GateArray_Reset(void);

/* write data to gate array */
void    GateArray_Write(int);

/* re-setup memory pointers */
void    GateArray_RethinkMemory(void);

void    GateArray_Initialise(void);

int             GateArray_GetRed(int);
int             GateArray_GetGreen(int);
int             GateArray_GetBlue(int);

/* used for Snapshot and Multiface */
int             GateArray_GetPaletteColour(int PenIndex);
int             GateArray_GetSelectedPen(void);
int             GateArray_GetMultiConfiguration(void);
void            GateArray_RethinkMemory(void);
void            GateArray_SetRethinkMemoryCallback(void (*)(void));
void			CPC_GateArray_RethinkMemory(void);
void			CPC_GateArray_Write(int);
void			GateArray_SetWriteCallback(void (*)(int));
void GateArray_DoDispEnable(BOOL);
void CPC_UpdateGraphicsFunction(void);
unsigned short CPC_GetPixelData(void);

void    GateArray_SetMonitorColourMode(CPC_MONITOR_TYPE_ID Mode);

#define CRTC_CLOCK_FREQUENCY 1000000
#define RAM_CLOCK 1000000
#define GATE_ARRAY_FREQUENCY 16000000
#define GATE_ARRAY_INT_PULSE_LENGTH_IN_NOPS 5
/* wait every 4th clock. */

typedef struct
{
	unsigned long	InterruptLineCount;
	unsigned long	InterruptSyncCount;

	unsigned char   PenSelection;
	unsigned char   ColourSelection;
	unsigned char   RomConfiguration;

	unsigned char   PenColour[18];

	unsigned char	PenIndex;
	unsigned char	pad0;
	unsigned char	pad1;

	unsigned long	BlankingOutput;
	unsigned long	CRTCSyncInputs;
	
	int nHBlankCycle;
	int nVBlankCycle;

	BOOL RasterInterruptRequest;

	
} GATE_ARRAY_STATE;

void	GateArray_SetInterruptLineCount(int Count);
int		GateArray_GetInterruptLineCount(void);

void GateArray_Cycle(void);
void GateArray_UpdateHsync(BOOL bState);
void GateArray_UpdateVsync(BOOL bState);
void GateArray_DoDispTmg(BOOL bState);
void GateArray_RestartReset(void);
Z80_BYTE    GateArray_AcknowledgeInterrupt(void);
void	GateArray_UpdateColours(void);
void	GateArray_Update(void);
BOOL GateArray_GetInterruptRequest(void);


BOOL GateArray_GetHBlankActive(void);
int GateArray_GetHBlankCount(void);

BOOL GateArray_GetVBlankActive(void);
int GateArray_GetVBlankCount(void);

#endif
