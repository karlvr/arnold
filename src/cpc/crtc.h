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
#ifndef __CRTC_HEADER_INCLUDED__
#define __CRTC_HEADER_INCLUDED__

#include "cpcglob.h"

#define CRTC_VS_FLAG	0x001	/* Vsync active */
#define CRTC_HS_FLAG	0x002	/* Hsync active */
#define CRTC_HDISP_FLAG	0x004	/* Horizontal Display Timing */
#define CRTC_VDISP_FLAG	0x008	/* Vertical Display Timing */
#define CRTC_HTOT_FLAG	0x010	/* HTot reached */
#define CRTC_VTOT_FLAG	0x020	/* VTot reached */
#define CRTC_MR_FLAG	0x040	/* Max Raster reached */
#define CRTC_VADJ_FLAG	0x080
#define CRTC_R8DT_FLAG	0x100
#define CRTC_VSCNT_FLAG	0x200
#define CRTC_HSCNT_FLAG 0x400
#define CRTC_VSALLOWED_FLAG 0x800
#define CRTC_VADJWANTED_FLAG 0x01000
#define CRTC_INTERLACE_ACTIVE 0x02000
#define CRTC_CURSOR_LINE_ACTIVE 0x04000
#define CRTC_CURSOR_ACTIVE 0x08000

void CRTC_DoDispEnable(void);
void CRTC_InitVsync(void);
void CRTC_DoReg1(void);
void CRTC_DoReg8(void);
void CRTC_DoVerticalSyncCounter(void);
void CRTC_SetLPenInput(BOOL bState);

/* used by the code */
typedef struct
{
	unsigned long CRTC_Flags;
	unsigned long CRTC_HalfHtotFlags;
	unsigned long CRTC_FlagsAtLastHsync;
	unsigned long CRTC_FlagsAtLastHtot;
	/* horizontal count */
	unsigned char HCount;
	/* start and end of line in char positions */
	unsigned char HStart, HEnd;
	/* Horizontal sync width */
	unsigned char HorizontalSyncWidth;
	/* horizontal sync width counter */
	unsigned char HorizontalSyncCount;

	/* raster counter (RA) */
	unsigned char RasterCounter;
	/* line counter */
	unsigned char LineCounter;
	/* Vertical sync width */
	unsigned char VerticalSyncWidth;
	unsigned char VerticalSyncWidthCount;
	/* vertical sync width counter */
	unsigned char VerticalSyncCount;

	/* INTERLACE STUFF */
	/* interlace and video mode number 0,1,2,3 */
	//unsigned char InterlaceAndVideoMode;
	/* frame - odd or even - used in interlace */
	unsigned char Frame;
	/* Vert Adjust Counter */
	unsigned char VertAdjustCount;
	/* delay for start and end of line defined by reg 8 */
	unsigned char HDelayReg8;



	/* type index of CRTC */
	unsigned char CRTC_Type;
	/* index of current register selected */
	unsigned char CRTC_Reg;
	/*unsigned short HDispAdd; */


	/* MA (memory address base) */
	int MA;	/* current value */
	/* MA of current line we are rendering (character line) */
	int MAStore;		/* this is the reload value */

	int CursorBlinkCount; /* current flash count */
	int CursorBlinkOutput; /* current output from flash */
	int CursorActiveLine; /* cursor is active on this line */
	int CursorOutput; /* final output */

	int CursorMA;
	/* line function */
	int LinesAfterFrameStart;
	int CharsAfterHsyncStart;
	int LinesAfterVsyncStart;
} CRTC_INTERNAL_STATE;


extern CRTC_INTERNAL_STATE CRTC_InternalState;
extern unsigned char            CRTCRegisters[32];

// useful parameters 
int CRTC_GetLinesAfterFrameStart(void);
int CRTC_GetLinesAfterVsyncStart(void);
int CRTC_GetCharsAfterHsyncStart(void);

int CRTC_GetInterlaceFrame(void);
int CRTC_GetHsyncCounter(void);
int CRTC_GetVsyncCounter(void);
int CRTC_GetVsyncOutput(void);
int CRTC_GetHsyncOutput(void);
int CRTC_GetDispTmgOutput(void);
int CRTC_GetRAOutput(void);
int CRTC_GetMAOutput(void);
int CRTC_GetVCC(void);
int CRTC_GetHCC(void);
int CRTC_GetRA(void);
int CRTC_GetActualHorizontalSyncWidth(void);
int CRTC_GetActualVerticalSyncWidth(void);

void CRTC_DoCycle(void);
void CRTC_DoCycles(int);

void	CRTC_SetMA(int NewMA);

int CRTC_GetCursorOutput(void);

/*---------------------------------------------------------------------------*/
/* reset CRTC */
void	CRTC_Reset(void);

/* select CRTC type emulated */
void	CRTC_SetType(unsigned int);

/* select register */
void	CRTC_RegisterSelect(unsigned int);

/* read data from selected register */
unsigned char CRTC_ReadData(void);

/* write data to selected register */
void	CRTC_WriteData(unsigned int);

/* read status register */
unsigned int CRTC_ReadStatusRegister(void);

#include "crtc_type0.h"
#include "crtc_type1.h"
#include "crtc_type2.h"
#include "crtc_type3.h"
#include "crtc_type4.h"
#include "crtc_type1b.h"

/* get selected CRTC register - for snapshot or multiface */
int		CRTC_GetSelectedRegister(void);
/* get register data - for Snapshot or Multiface */
unsigned char		CRTC_GetRegisterData(int RegisterIndex);

CRTC_INTERNAL_STATE *CRTC_GetInternalState(void);


void	CRTC_Initialise(void);



void	CRTC_LightPen_Trigger(unsigned long X, unsigned long Y);
void	CRTC_LightPen_Update(unsigned long Cycles);

int CPC_GetCRTCType(void);

#endif
