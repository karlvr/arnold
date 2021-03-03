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
/* CRTC type '0' - HD6845S/UM6845 */
#include "crtc.h"
#include "cpc.h"
/* on type 0, border is re-enabled at htot half a character before it ends */

extern CRTC_INTERNAL_STATE CRTC_InternalState;
extern unsigned char            CRTCRegisters[32];

void CRTC0_DoLineChecks(void);

#define CRTC_ClearFlag(x) \
	CRTC_InternalState.CRTC_Flags &=~x

#define CRTC_SetFlag(x)	\
	CRTC_InternalState.CRTC_Flags |=x
/*---------------------------------------------------------------------------*/

#define GET_MA  \
        (CRTCRegisters[12]<<8) | (CRTCRegisters[13])




void CRTC0_Reset(void)
{
	int i;
	/* set light pen registers - this is what my CPC
	type 0 reports! */
	CRTCRegisters[16] = 0x014;
	CRTCRegisters[17] = 0x07c;


	/*

	    UM6845:
	    Reset Signal (/RES) is an input signal used to reset the CRTC. When /RES is at "low" level, it forces the CRTC into the following status:

	    * All the counters in the CRTC are cleared and the device stops the display operation
	    * All the outputs go down to "low" level.
	    * Control registers in the CRTC are not affected and remain unchanged.

	This signal is different from other HD6800 family LSIs in the following functions and has restrictions for usage:

	    * /RES has capability of reset function only when LPSTB is at "low" level.
	    * The CRTC starts the display operation immediatly after /RES goes "high" level.

	    */

	/* vsync counter not active */
	CRTC_ClearFlag(CRTC_VSCNT_FLAG);
	/* not in hsync */
	CRTC_ClearFlag(CRTC_HS_FLAG);
	/* not in a vsync */
	CRTC_ClearFlag(CRTC_VS_FLAG);
	/* not reached end of line */
	CRTC_ClearFlag(CRTC_HTOT_FLAG);
	/* not reached end of frame */
	CRTC_ClearFlag(CRTC_VTOT_FLAG);

/*		CRTC_InternalState.GA_State.GA_Flags &=~GA_HSYNC_FLAG;
		CRTC_InternalState.GA_State.GA_Flags &=~GA_VSYNC_FLAG;
*/
	/* not reached last raster in char */
	CRTC_ClearFlag(CRTC_MR_FLAG);
	/* not in vertical adjust */
	CRTC_ClearFlag(CRTC_VADJ_FLAG);
	/* do not display graphics */
	CRTC_ClearFlag(CRTC_VDISP_FLAG);
	CRTC_ClearFlag(CRTC_HDISP_FLAG);
	CRTC_ClearFlag(CRTC_VADJWANTED_FLAG);
	CRTC_ClearFlag(CRTC_R8DT_FLAG);

	/* reset all registers */
	for (i=0; i<16; i++)
	{
		/* select register */
		CRTC_RegisterSelect(i);

		/* write data */
		CRTC_WriteData(0);
	}

	/* reset CRTC internal registers */

	/* reset horizontal count */
	CRTC_InternalState.HCount = 0;
	/* reset line counter (vertical count) */
	CRTC_InternalState.LineCounter = 0;
	/* reset raster count */
	CRTC_InternalState.RasterCounter = 0;
	/* reset MA */
	CRTC_InternalState.MA = 0;
	CRTC_InternalState.MAStore = CRTC_InternalState.MA;
	CRTC_InternalState.Frame = 0;

	CRTC_InternalState.CursorOutput = 0;
	CRTC_InternalState.CursorBlinkCount = 0;

	CRTC0_DoLineChecks();
}


/*-----------------------------------------------------------------------*/
/* HD6845S */
const unsigned char HD6845S_ReadMaskTable[32]=
{
	0x000,  /* Horizontal Total */
	0x000,  /* Horizontal Displayed */
	0x000,  /* Horizontal Sync Position */
	0x000,  /* Sync Widths */
	0x000,  /* Vertical Total */
	0x000,  /* Vertical Adjust */
	0x000,  /* Vertical Displayed */
	0x000,  /* Vertical Sync Position */
	0x000,  /* Interlace and Skew */
	0x000,  /* Maximum Raster Address */
	0x000,  /* Cursor Start */
	0x000,  /* Cursor End */
	0x0ff,  /* Screen Addr (H) */
	0x0ff,  /* Screen Addr (L) */
	0x0ff,  /* Cursor (H) */
	0x0ff,  /* Cursor (L) */

	0x0ff,  /* Light Pen (H) */
	0x0ff,  /* Light Pen (L) */
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,

};

/* these are anded before data is written */
const unsigned char HD6845S_WriteMaskTable[32] =
{
	0x0ff,
	0x0ff,
	0x0ff,
	0x0ff,
	0x07f,
	0x01f,
	0x07f,
	0x07f,
	0x0f3,
	0x01f,
	0x07f,
	0x01f,
	0x03f,
	0x0ff,
	0x03f,
	0x0ff,

	0x0ff,
	0x0ff,
	0x0ff,
	0x0ff,
	0x0ff,
	0x0ff,
	0x0ff,
	0x0ff,
	0x0ff,
	0x0ff,
	0x0ff,
	0x0ff,
	0x0ff,
	0x0ff,
	0x0ff,
	0x0ff,
};


void CRTC0_RefreshHStartAndHEnd(void)
{

	/* if Reg 8 is used, start and end positions are delayed by amount
	programmed. HStart can also be additionally delayed by ASIC. */

	/* set start and end positions of lines */
	CRTC_InternalState.HEnd = (unsigned char)(CRTCRegisters[1] + CRTC_InternalState.HDelayReg8);
	CRTC_InternalState.HStart = CRTC_InternalState.HDelayReg8;

	/* set HStart and HEnd to same, because Reg1 is set to 0 */
	if (CRTCRegisters[1] == 0)
	{
		CRTC_InternalState.HStart = CRTC_InternalState.HEnd = 0;
	}

	/* update rendering function */
	CRTC_DoDispEnable();
}

void CRTC0_DoReg1(void)
{
	CRTC0_RefreshHStartAndHEnd();
}



void CRTC0_DoReg8(void)
{
	int Delay;

	/* on type 3 changing r8 rapidly shows nothing */

	/* number of characters delay */
	Delay = (CRTCRegisters[8] >> 4) & 0x03;
	CRTC_ClearFlag(CRTC_R8DT_FLAG);

	if (Delay == 3)
	{
		/* Disable display of graphics */
		CRTC_SetFlag(CRTC_R8DT_FLAG);
		Delay = 0;
	}

	CRTC_InternalState.HDelayReg8 = (unsigned char)Delay;

	CRTC0_RefreshHStartAndHEnd();

	// don't think it's immediate.
#if 0
	if (CRTCRegisters[8] & 0x01)
	{
		CRTC_SetFlag(CRTC_INTERLACE_ACTIVE);
	}
#endif

	/* interlace mode and video mode*/
	//CRTC_InternalState.InterlaceAndVideoMode = (unsigned char)(CRTCRegisters[8] & 0x003);

//	if (CRTC_InternalState.InterlaceAndVideoMode == 3)
//	{
//		/* make sure raster counter is even, otherwise the
//		interlace and video mode will not always start correctly! */
	//	CRTC_InternalState.RasterCounter &= ~0x01;
//	}
}

/********************************/
/* CRTC type 0 - HD6845S/UM6845 */
/********************************/

/* vadj is 1 char line? mr checked against vertical adjust counter? */
/* allows repeat of rest; to confirm; we should be able to change value when R9 = 4 at a different pos */


void CRTC0_UpdateState(int RegIndex)
{
	/* re-programming vsync position doesn't cut vsync */
	/* re-programming length doesn't seem to cut vsync */
	switch (RegIndex)
	{
// octoplex title wants this
	case 4:
		if (CRTC_InternalState.CRTC_Flags & CRTC_MR_FLAG)
		{
			if (CRTC_InternalState.LineCounter == CRTCRegisters[4])
			{
				CRTC_SetFlag(CRTC_VTOT_FLAG);
			}
		}
		break;

	case 1:
		CRTC0_DoReg1();
		break;

	case 3:
		break;

	case 8:
		CRTC_DoReg8();
		break;


	case 7:
	{
		/* confirmed: Register can be written at any time and takes immediate effect; not sure if 0 or HCC=R0 */
		if ((CRTC_InternalState.LineCounter == CRTCRegisters[7]) && (CRTC_InternalState.HCount != 0))
		{
			CRTC_InitVsync();
		}

	}
	break;

	case 6:
	{
		/* confirmed: immediate on type 0 */
		if (CRTC_InternalState.LineCounter == CRTCRegisters[6])
		{
			CRTC_ClearFlag(CRTC_VDISP_FLAG);
		}

		if ((CRTC_InternalState.LineCounter == 0) && (CRTC_InternalState.RasterCounter == 0))
		{
			if (CRTCRegisters[6] != 0)
			{
				CRTC_SetFlag(CRTC_VDISP_FLAG);

			}
		}

		CRTC_DoDispEnable();
	}
	break;

	case 9:
	{
		if (CRTC_InternalState.CRTC_Flags & CRTC_VADJ_FLAG)
		{
			if (CRTC_InternalState.VertAdjustCount == CRTCRegisters[9])
			{
				CRTC_SetFlag(CRTC_MR_FLAG);
			}
			else
			{
				CRTC_ClearFlag(CRTC_MR_FLAG);

			}
		}
		else
		{
			// confirm r8
			if (CRTC_InternalState.RasterCounter == CRTCRegisters[9])
			{
				CRTC_SetFlag(CRTC_MR_FLAG);
			}
			else
			{
				CRTC_ClearFlag(CRTC_MR_FLAG);

			}
		}
	}
	break;

		case 14:
		case 15:
			CRTC_InternalState.CursorMA = (CRTCRegisters[14]<<8)|CRTCRegisters[15];
			break;

		default
				:
			break;
	}

}

/*---------------------------------------------------------------------------*/
int CRTC0_ReadStatusRegister(void)
{
	/* no status register; returns databus or 0x0ff */
	return 0x0ff;
}

/*---------------------------------------------------------------------------*/

void CRTC0_WriteData(int Data)
{

	int CRTC_RegIndex = CRTC_InternalState.CRTC_Reg & 0x1f;

	/* store registers using current CRTC information - masking out appropiate bits etc for this CRTC*/
	CRTCRegisters[CRTC_RegIndex] = (unsigned char)(Data & HD6845S_WriteMaskTable[CRTC_RegIndex]);



	CRTC0_UpdateState(CRTC_RegIndex);
}

/*---------------------------------------------------------------------------*/


void CRTC0_DoHDisp(void)
{
	/* confirmed: if rcc=r9 at HDISP time then store MA for reload. It is possible to change R9 around R1 time only
	and get the graphics to repeat but doesn't cause problems for RCC */
	/* confirmed: gerald's tests seem to indicate that MAStore is not updated when vdisp is not active. i.e. in lower border */
	if ((CRTC_InternalState.CRTC_Flags & CRTC_MR_FLAG) && (CRTC_InternalState.CRTC_Flags & CRTC_VDISP_FLAG))
		{
			/* remember it for next line */
			CRTC_InternalState.MAStore = CRTC_InternalState.MA;
		}
}

void CRTC0_RestartFrame(void)
{

	CRTC_InternalState.LinesAfterFrameStart = 0;

	CRTC_InternalState.MAStore = GET_MA;
	CRTC_InternalState.MA = CRTC_InternalState.MAStore;

	CRTC_InternalState.RasterCounter = 0;
	CRTC_InternalState.LineCounter = 0;

//	if (CRTC_InternalState.InterlaceAndVideoMode == 3)
//	{
//		CRTC_InternalState.RasterCounter = CRTC_InternalState.Frame;
//	}
//	else
	{
		CRTC_InternalState.RasterCounter = 0;
	}

	CRTC_SetFlag(CRTC_VDISP_FLAG);

	CRTC_DoDispEnable();


	/* on type 0, the first line is always visible */

#ifdef HD6845S
	/* if type 0 is a HD6845S */
	CRTC_SetFlag(CRTC_VDISP_FLAG);
#endif
	/*  CRTC0_DoLineChecks(); */

	/* incremented when? */
	CRTC_InternalState.CursorBlinkCount++;
	if (CRTCRegisters[10] & (1 << 6))
	{
		/* blink */
		if (CRTCRegisters[11] & (1 << 5))
		{
			/* 32 field period */
			/* should we just test bit 5? */
			if (CRTC_InternalState.CursorBlinkCount == 32)
			{
				CRTC_InternalState.CursorBlinkCount = 0;
				CRTC_InternalState.CursorBlinkOutput ^= 1;
			}
		}
		else
		{
			/* 16 field period */
			/* should we just test bit 4? */
			if (CRTC_InternalState.CursorBlinkCount == 16)
			{
				CRTC_InternalState.CursorBlinkCount = 0;
				CRTC_InternalState.CursorBlinkOutput ^= 1;
			}
		}
		if (CRTC_InternalState.CursorBlinkOutput)
		{
			CRTC_SetFlag(CRTC_CURSOR_ACTIVE);
		}
		else
		{
			CRTC_ClearFlag(CRTC_CURSOR_ACTIVE);
		}
	}
	else
	{
		if (CRTCRegisters[10]&(1<<5))
		{
			/* no blink, no output */
			CRTC_ClearFlag(CRTC_CURSOR_ACTIVE);
			CRTC_InternalState.CursorBlinkOutput = 0;
		}
		else
		{
			/* no blink */
			CRTC_SetFlag(CRTC_CURSOR_ACTIVE);
		}
	}
//	CRTC0_DoLineChecks();
}

void    CRTC0_MaxRasterMatch(void)
{
	if (CRTC_InternalState.CRTC_Flags & CRTC_INTERLACE_ACTIVE)
	{
		if (CRTCRegisters[8] & (1 << 1))
		{
			if (CRTC_InternalState.RasterCounter == (CRTCRegisters[9] >> 1))
			{
				CRTC_SetFlag(CRTC_MR_FLAG);
			}
			else
			{
				CRTC_ClearFlag(CRTC_MR_FLAG);
			}
		}
	}
	else
	{
		if (CRTC_InternalState.CRTC_Flags & CRTC_VADJ_FLAG)
		{
			if (CRTC_InternalState.VertAdjustCount == CRTCRegisters[9])
			{
				CRTC_SetFlag(CRTC_MR_FLAG);
			}

		}
		else
		{
			if (CRTC_InternalState.RasterCounter == CRTCRegisters[9])
			{
				CRTC_SetFlag(CRTC_MR_FLAG);
			}
		}
	}

	if (CRTC_InternalState.CRTC_Flags & CRTC_MR_FLAG)  
	{
		if (CRTC_InternalState.LineCounter == CRTCRegisters[4])
		{
			CRTC_SetFlag(CRTC_VTOT_FLAG);
		}
	}
}


/* appears that on crtc type 0 and type 3, Vertical Sync width can be reprogrammed
while it is active. The Vertical Sync Counter is 4-bit. Comparison for both appears to be equal! */
static void CRTC0_DoVerticalSyncCounter(void)
{
	/* are we counting vertical syncs? */
	if (CRTC_InternalState.CRTC_Flags & CRTC_VSCNT_FLAG)
	{
		/* update vertical sync counter */
		CRTC_InternalState.VerticalSyncCount++;
		
		/* if vertical sync count = vertical sync width then stop vertical sync */
		/* if vertical sync width = 0, the counter will wrap after incrementing from 15 causing
		a vertical sync width of 16*/
		if (CRTC_InternalState.VerticalSyncCount==CRTC_InternalState.VerticalSyncWidth)
		{
			/* count done */
			CRTC_InternalState.VerticalSyncCount=0;

			CRTC_ClearFlag(CRTC_VSCNT_FLAG);
		}
	}
}

void CRTC0_DoLineChecks(void)
{
	/* confirmed: immediate on type 0 */
	if (CRTC_InternalState.LineCounter == CRTCRegisters[6])
	{
		CRTC_ClearFlag(CRTC_VDISP_FLAG);
		CRTC_DoDispEnable();
	}

	/* check Vertical sync position */
	if (CRTC_InternalState.LineCounter == CRTCRegisters[7])
	{
		CRTC_InitVsync();
	}
}


/* executed for each complete line done by the CRTC */
void    CRTC0_DoLine(void)
{
	
	/* to be confirmed; ma works during vadjust */
	/* increment raster counter */
	CRTC_InternalState.RasterCounter = (unsigned char)((CRTC_InternalState.RasterCounter + 1) & 0x01f);

	CRTC0_DoVerticalSyncCounter();

	/* are we in vertical adjust ? */
	if (CRTC_InternalState.CRTC_Flags & CRTC_VADJ_FLAG)
	{
		CRTC_InternalState.VertAdjustCount = (unsigned char)((CRTC_InternalState.VertAdjustCount + 1) & 0x01f);

		/* vertical adjust matches counter? */
		if (CRTC_InternalState.VertAdjustCount == CRTCRegisters[5])
		{
			CRTC_ClearFlag(CRTC_VADJ_FLAG);

			CRTC0_RestartFrame();
		}
	}

	if (CRTC_InternalState.CRTC_Flags & CRTC_MR_FLAG)
	{
		CRTC_ClearFlag(CRTC_MR_FLAG);

		CRTC_InternalState.RasterCounter = 0;

		/* this will trigger once at vtot */
		if (CRTC_InternalState.CRTC_Flags & CRTC_VTOT_FLAG)
		{
			CRTC_ClearFlag(CRTC_VTOT_FLAG);

			/* toggle frame; here or after vadj? */
			CRTC_InternalState.Frame ^= 0x01;

			/* is it active? i.e. VertAdjust!=0 */
			if (CRTCRegisters[5]!=0)
			{
				/* yes */
				CRTC_InternalState.VertAdjustCount = 0;
				CRTC_SetFlag(CRTC_VADJ_FLAG);

				/* confirmed: on type 0, line counter will increment when entering vertical adjust, but not count furthur.
				i.e. if R5!=0 and R7=VTOT then vertical sync will trigger */
				/* increment once going into vertical adjust */
				CRTC_InternalState.LineCounter = (unsigned char)((CRTC_InternalState.LineCounter + 1) & 0x07f);

			}
			else
			{
				/* restart frame */

				CRTC0_RestartFrame();
			}
		}
		else
		{
			/* confirmed: on type 0, line counter will increment when entering vertical adjust, but not count furthur.
			i.e. if R5!=0 and R7=VTOT then vertical sync will trigger */
			/* do not increment during vertical adjust */
			if (!(CRTC_InternalState.CRTC_Flags & CRTC_VADJ_FLAG))
			{
				CRTC_InternalState.LineCounter = (unsigned char)((CRTC_InternalState.LineCounter + 1) & 0x07f);
			}
		}

	}

	/* transfer store value */

	CRTC_InternalState.MA = CRTC_InternalState.MAStore;

	if ((CRTCRegisters[8] & 1) != 0)
	{
		CRTC_SetFlag(CRTC_INTERLACE_ACTIVE);
	}
	else
	{
		CRTC_ClearFlag(CRTC_INTERLACE_ACTIVE);
	}

	CRTC0_MaxRasterMatch();



	/* do last to capture line counter increment in R5 and frame restart */
	CRTC0_DoLineChecks();

//	if ((CRTC_InternalState.CRTC_Flags & CRTC_VTOT_FLAG) && (CRTC_InternalState.CRTC_Flags & CRTC_MR_FLAG) && (CRTCRegisters[5]!=0))
//	{
//		CRTC_InternalState.CRTC_Flags |= CRTC_VADJWANTED_FLAG;
//	}
}


int CRTC0_GetRAOutput(void)
{
	if ((CRTC_InternalState.CRTC_Flags & CRTC_INTERLACE_ACTIVE) != 0)
	{
//		/* if R9 is odd, this will change between 0 or 1 */
//		int Bit0 = ((CRTC_InternalState.LineCounter^CRTC_InternalState.Frame) & CRTCRegisters[9] & 0x01) |
//			/* if R9 is even, we want Frame */
//			(((CRTCRegisters[9] & 0x01) ^ 0x01) & CRTC_InternalState.Frame);

		return (CRTC_InternalState.RasterCounter << 1) | CRTC_InternalState.Frame;
	}
	else
	{
		if (CRTC_InternalState.CRTC_Flags & CRTC_VADJ_FLAG)
		{
			return CRTC_InternalState.VertAdjustCount;
		}
		else
		{
			return CRTC_InternalState.RasterCounter;
		}
	}
//	return CRTC_InternalState.RasterCounter;
}

int CRTC0_GetHorizontalSyncWidth(void)
{
	/* confirmed: a programmed hsync of 0 generates no hsync */
	return CRTCRegisters[3] & 0x0f;
}

int CRTC0_GetVerticalSyncWidth(void)
{
	/* confirmed: a programmed vsync width of 0, results in an actual width of 16 */
	/* 16 can happen when counter overflows */
	int VerticalSyncWidth = (CRTCRegisters[3] >> 4) & 0x0f;
	if (VerticalSyncWidth == 0)
	{
		VerticalSyncWidth = 16;
	}
	return VerticalSyncWidth;
}

/*---------------------------------------------------------------------------*/

int CRTC0_ReadData(void)
{
	int CRTC_RegIndex = (CRTC_InternalState.CRTC_Reg & 0x01f);

	/* unreadable registers return 0 */
	return (CRTCRegisters[CRTC_RegIndex] & HD6845S_ReadMaskTable[CRTC_RegIndex]);
}

/*---------------------------------------------------------------------------*/
