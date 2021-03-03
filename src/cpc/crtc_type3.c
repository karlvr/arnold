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
/********************************/
/* CRTC type 3 - CPC+ ASIC CRTC */
/********************************/

/* NOTE: screen is positioned 1 character to the left compared to type 0,1 and 2 */


/* R8 can be turned on/off during a frame including setting interlace sync and video
on/off. It can be used to skip lines */

#include "crtc.h"
#include "cpc.h"
#include "asic.h"
#include "printer.h"

void	ASICCRTC_SetSoftScroll(int SoftScroll);

#define GET_MA  \
        (CRTCRegisters[12]<<8) | (CRTCRegisters[13])

void CRTC3_DoLineChecks(void);


#define CRTC_ClearFlag(x) \
	CRTC_InternalState.CRTC_Flags &=~x

#define CRTC_SetFlag(x)	\
	CRTC_InternalState.CRTC_Flags |=x



const unsigned char AsicCRTC_WriteMaskTable[32] =
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


extern CRTC_INTERNAL_STATE CRTC_InternalState;
extern unsigned char            CRTCRegisters[32];

/*---------------------------------------------------------------------------*/

#define GET_MA  \
        (CRTCRegisters[12]<<8) | (CRTCRegisters[13])


/*---------------------------------------------------------------------------*/
/* holds ASIC Soft Scroll register value - for CRTC use */
static unsigned char ASICCRTC_SoftScroll = 0;
/* holds ASIC vertical soft scroll */
static unsigned char ASICCRTC_VerticalSoftScroll = 0;
/* holds ASIC horizontal soft scroll */
unsigned char ASICCRTC_HorizontalSoftScroll = 0;
/* holds current ASIC CRTC line used for displaying sprites */
BOOL ASICCRTC_MaskSoftScroll = FALSE;
/* holds current ASIC CRTC line used for displaying sprites */
unsigned long ASICCRTC_Line = 0;
/* ASIC RA count */
unsigned char ASICCRTC_RasterLine;

/* if TRUE, draw black on the display. This is caused if reg8 delay is set to  ,and the "extend border over garbage" in soft scroll is set */
/*static BOOL ASICCRTC_BlackDisplay = FALSE; */
/* appears that on crtc type 0 and type 3, Vertical Sync width can be reprogrammed
while it is active. The Vertical Sync Counter is 4-bit. Comparison for both appears to be equal! */
static void CRTC3_DoVerticalSyncCounter(void)
{
	/* are we counting vertical syncs? */
	if (CRTC_InternalState.CRTC_Flags & CRTC_VSCNT_FLAG)
	{
		/* update vertical sync counter */
		CRTC_InternalState.VerticalSyncCount = (CRTC_InternalState.VerticalSyncCount + 1) & 0x0f;

		/* if vertical sync count = vertical sync width then
		 stop vertical sync */
		if (CRTC_InternalState.VerticalSyncCount == (CRTC_InternalState.VerticalSyncWidth & 0x0f))
		{
			CRTC_InternalState.VerticalSyncCount = 0;

			CRTC_ClearFlag(CRTC_VSCNT_FLAG);
		}
	}
}

/*---------------------------------------------------------------------------*/

int CRTC3_GetRAOutput(void)
{
	return ASICCRTC_RasterLine & 0x07;
}

/*---------------------------------------------------------------------------*/

static void ASICCRTC_GenerateOutputRasterLine(void)
{
	if (!ASIC_GetVerticalScrollOverride())
	{
		/* confirmed: During vertical adjust R9 and R4 are not honoured. We see the same 8 scanlines repeated.
		Not sure if internally it's re-using RasterCounter or a separate count, but it's definitely based off some
		count */
		if (CRTC_InternalState.CRTC_Flags & CRTC_VADJ_FLAG)
		{
			ASICCRTC_RasterLine = (unsigned char)CRTC_InternalState.VertAdjustCount;
		}
		else
		{
			ASICCRTC_RasterLine = (unsigned char)CRTC_InternalState.RasterCounter;
		}
	}
	else
	{
		/* confirmed: vertical soft scroll works during vertical adjust, but continues to repeat lines we have already seen */
		if (CRTC_InternalState.CRTC_Flags & CRTC_VADJ_FLAG)
		{
			/* + works on my sscr tests */
			ASICCRTC_RasterLine = (unsigned char)(((CRTC_InternalState.VertAdjustCount + ASICCRTC_VerticalSoftScroll) & 0x1f));
		}
		else
		{
			/* + works on my sscr tests */
			ASICCRTC_RasterLine = (unsigned char)(((CRTC_InternalState.RasterCounter + ASICCRTC_VerticalSoftScroll) & 0x1f));
		}
	}
}

/*---------------------------------------------------------------------------*/

static void ASICCRTC_GenerateLineNumber(void)
{
	/* generate line number for sprites etc */
	ASICCRTC_Line = (((CRTC_InternalState.LineCounter & 0x03f) << 3) | (CRTC_InternalState.RasterCounter & 0x07));
}

/*---------------------------------------------------------------------------*/

void CRTC3_Reset(void)
{
	int i;

	/* confirmed using cartridge, registers are reset to 0 */

	/* not sure about 16/17 though */
	/* as soon as horizontal total is written we start to get interrupts it seems */

	CRTCRegisters[16] = 0;
	CRTCRegisters[17] = 0;


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

	CRTC_ClearFlag(CRTC_R8DT_FLAG);

	/* reset all registers */
	for (i = 0; i < 16; i++)
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

	/* generate output RA */
	ASICCRTC_GenerateOutputRasterLine();

	if (CRTC_InternalState.RasterCounter == (CRTCRegisters[10] & 0x01f))
	{
		CRTC_InternalState.CursorActiveLine = 0x0ff;
	}
	else if (CRTC_InternalState.RasterCounter == (CRTCRegisters[11] & 0x01f))
	{
		CRTC_InternalState.CursorActiveLine = 0;
	}

}

void CRTC3_RefreshHStartAndHEnd(void)
{

	/* confirmed; on gx4000 */
	/* confirmed: r8 and scroll delay the border, but MA continues to count. i.e. if first chars are A,B,C,D,E,F
	then 0 shows A,B,C,D,E,F 1 shows C,D,E,F 2 shows E,F */
	/* tested by running through all combinations of r8 delay and turning border on/off for scroll
	and observing the effect for an entire frame each time */
	/* R8 - SCRL - result
	0     -         0 left, 0 right
	1     -         1 left, 1 right
	2     -         2 left, 2 right
	3     -          border
	0     set       1 left, 0 right
	1     set       2 left, 1 right
	2     set       3 left, 2 right
	3     set       border
	*/

	/* confirmed: r8 delay is read when HCC=0  and when HCC=R1 */
	/* confirmed: if you set R8 before HCC=0 and reset it to 0 after you can change the left side and not the right */


	/* confirmed: border is not hidden by soft scroll when R1>=R0 */
	/* border must be active */
	/* confirmed: border extend doesn't affect R1 */
	/* confirmed: soft scroll border extend is read at start of line only (HCC=0) */
	/* confirmed: soft scroll border extend is not read at HCC=R1 */

	/* to be confirmed: black display? Was this because the hsync interfered? */

	/* set start and end positions of lines */
	CRTC_InternalState.HEnd = (unsigned char)(CRTCRegisters[1] + CRTC_InternalState.HDelayReg8);
	CRTC_InternalState.HStart = CRTC_InternalState.HDelayReg8;
	if (ASIC_GetScrollBorderOverride() && (ASICCRTC_SoftScroll & 0x080))
	{
		CRTC_InternalState.HStart++;
	}
	/* set HStart and HEnd to same, because Reg1 is set to 0 */
	if (CRTCRegisters[1] == 0)
	{
		CRTC_InternalState.HStart = CRTC_InternalState.HEnd = 0;
	}


	/* update rendering function */
	CRTC_DoDispEnable();
}

void CRTC3_DoReg8(void)
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

	CRTC3_RefreshHStartAndHEnd();

	/* following need to be confirmed */
	if (CRTCRegisters[8] & 0x01)
	{
		CRTC_SetFlag(CRTC_INTERLACE_ACTIVE);
	}

	/* interlace mode and video mode*/
//	CRTC_InternalState.InterlaceAndVideoMode = (unsigned char)(CRTCRegisters[8] & 0x003);

//	if (CRTC_InternalState.InterlaceAndVideoMode == 3)
//	{
//		/* make sure raster counter is even, otherwise the
//		interlace and video mode will not always start correctly! */
//		CRTC_InternalState.RasterCounter &= ~0x01;
//	}
}

void CRTC3_DoReg1(void)
{
	CRTC3_RefreshHStartAndHEnd();
}

/*---------------------------------------------------------------------------*/

/* do functions required when Vertical Scroll has been set */
void    ASICCRTC_SetSoftScroll(int SoftScroll)
{
	ASICCRTC_SoftScroll = (unsigned char)SoftScroll;

	ASICCRTC_VerticalSoftScroll = (unsigned char)((SoftScroll >> 4) & 0x07);
	ASICCRTC_HorizontalSoftScroll = (unsigned char)(SoftScroll & 0x0f);
	ASICCRTC_MaskSoftScroll = (SoftScroll & 0x80) != 0;

	ASICCRTC_GenerateOutputRasterLine();

	CRTC3_RefreshHStartAndHEnd();
}

int ASIC_GetPRILine(void)
{
	return ((CRTC_InternalState.LineCounter & 0x03f) << 3) | (CRTC_InternalState.RasterCounter & 0x07);
}

unsigned char ASIC_GetSPLTLine(void)
{
	return ((CRTC_InternalState.LineCounter & 0x01f) << 3) | (CRTC_InternalState.RasterCounter & 0x07);
}


/********************************/
void CRTC3_DoHDisp(void)
{
	/* confirmed: on type 3, once HDISP has been triggered it's not possible
	to clear it to show graphics. To test, set R1 to 32, then after HCC = 32, set R1 to 50.
	Observe screen remains at 32 chars wide. HDISP is cleared at the start of the line */

	/* confirmed: on type 3, border is not enabled until HEnd */

	/*
screen split:

- screen split is read at HDisp time
- screen split is read on the line programmed, the result takes place on the next line
- If we change it when RCC=R9, we see 2 lines repeated. it takes priority
- the generation of the line to compare against the programmed split line, takes the lower 3 bits of the Raster Counter

soft scroll:

if soft scroll + raster count==R9 at HDISP time, then we store MA, this causes the screen to scroll up.

*/
	/* confirmed: HDISP/Split is not captured at HDISP on last scanline of frame */
	if (((CRTC_InternalState.CRTC_Flags & (CRTC_MR_FLAG|CRTC_VTOT_FLAG))==(CRTC_MR_FLAG|CRTC_VTOT_FLAG)) && CRTCRegisters[5]==0)
	{
		return;
	}
	
	/* split screen? */
	if (ASIC_RasterSplitLineMatch(CRTC_InternalState.LineCounter, CRTC_InternalState.RasterCounter))
	{
		CRTC_InternalState.MAStore = ASIC_GetSSA();
	}
	else
	{
		if (
			/* disabled scroll? use normal comparison */
			(!ASIC_GetVerticalScrollOverride() && CRTC_InternalState.RasterCounter >= CRTCRegisters[9]) ||
			/* the following are a bit of a hack until the real operation can be deduced. It looks almost like there is a counter
			inside the asic which is reset on ma store*/
			/* scroll enabled, scroll not set, use normal comparison */
			(ASIC_GetVerticalScrollOverride() && (ASICCRTC_VerticalSoftScroll == 0) && (CRTC_InternalState.RasterCounter >= CRTCRegisters[9])) ||
			(ASIC_GetVerticalScrollOverride() && (ASICCRTC_VerticalSoftScroll != 0) && (CRTCRegisters[9]>=7) && ((CRTC_InternalState.RasterCounter + ASICCRTC_VerticalSoftScroll) == CRTCRegisters[9])) ||
//			(ASIC_GetVerticalScrollOverride() && (ASICCRTC_VerticalSoftScroll != 0) && (CRTCRegisters[9]<7) && (((CRTC_InternalState.RasterCounter + ASICCRTC_VerticalSoftScroll) & 0x07) >= (CRTCRegisters[9] & 0x07))))
			(ASIC_GetVerticalScrollOverride() && (ASICCRTC_VerticalSoftScroll != 0) && (CRTCRegisters[9]<7) && (((CRTC_InternalState.RasterCounter + ASICCRTC_VerticalSoftScroll) & 0x07) >= CRTCRegisters[9])))
		{
			/* confirmed: on type 3. During vertical adjust MAStore is not set. Therefore vertical adjust repeats the same line.
			To test fill the screen with text. Set R6>R4. Set R5=31. Observe during R5 the same lines are repeated. The line that is repeated
			is the one AFTER the last line. */
			if ((CRTC_InternalState.CRTC_Flags & CRTC_VADJ_FLAG) == 0)
			{
				/* get current value */
				CRTC_InternalState.MAStore = CRTC_InternalState.MA;
			}
		}
	}
}

/********************************/

#if 0
void    ASICCRTC_MaxRasterMatch(void)
{
	CRTC_ClearFlag(CRTC_MR_FLAG);

	if (CRTC_InternalState.RasterCounter>=CRTCRegisters[9])
	{
		CRTC_SetFlag(CRTC_MR_FLAG);
	}
}
#endif

/*---------------------------------------------------------------------------*/

/* cannot update reg 7 and get it to start mid line...?
 reg 7 must be changed before line 0 starts!
 */

/* ASIC CRTC Update State */
static void ASICCRTC_UpdateState(int RegIndex)
{
	switch (RegIndex)
	{
	case 12:
	{
		/* TODO: Only if specific LK is connected? */

		/* With CPC Plus, bit 3 of register 12 determines
		if bit 7 of printer data is 1 or 0 */
		Printer_SetDataBit7State((CRTCRegisters[12] >> 3) & 1);
	}
	break;

	case 6:
	{
//		if (CRTC_InternalState.RasterCounter >= CRTCRegisters[9])
//		{
//			CRTC3_DoLineChecks();
//		}
	}
	break;

	case 7:
	{
//		CRTC3_DoLineChecks();
	}
	break;


	case 1:
	{
		CRTC_DoReg1();
	}
	break;

	case 8:
	{
		CRTC_DoReg8();
	}
	break;

	case 3:
	{
		//CRTC3_DoReg3();
	}
	break;

	case 4:
	{
		CRTC_ClearFlag(CRTC_VTOT_FLAG);

		if (CRTC_InternalState.CRTC_Flags & CRTC_MR_FLAG)
		{
			if (!(CRTC_InternalState.CRTC_Flags & CRTC_VADJ_FLAG))
			{
				if (CRTC_InternalState.LineCounter == CRTCRegisters[4])
				{
					/* Vtot match too */

					CRTC_SetFlag(CRTC_VTOT_FLAG);
				}
			}
		}
	}
	break;

	case 9:
	{
		if (CRTC_InternalState.RasterCounter >= CRTCRegisters[9])
		{
			CRTC_SetFlag(CRTC_MR_FLAG);
		}
		else
		{
			CRTC_ClearFlag(CRTC_MR_FLAG);
		}
	}
	break;


	case 14:
	case 15:
	{
		CRTC_InternalState.CursorMA = (CRTCRegisters[14] << 8) | CRTCRegisters[15];
	}
	break;


	default
		:
			break;

	}

}

void CRTC3_DoLineChecks(void)
{
	/* check Vertical Displayed */
	if (CRTC_InternalState.LineCounter == CRTCRegisters[6])
	{
		if (CRTC_InternalState.RasterCounter == 0)
		{
			CRTC_ClearFlag(CRTC_VDISP_FLAG);

			CRTC_DoDispEnable();
		}
	}

	/* confirmed: R7 is checked at the start of the line on line 0 only */
	if (CRTC_InternalState.LineCounter == CRTCRegisters[7])
	{
		/* on CRTC type 3, Vsync will only be triggered on line 0. */
		if (CRTC_InternalState.RasterCounter == 0)
		{
			CRTC_InitVsync();
		}
	}

}

void ASICCRTC_UpdateCursorActive(void)
{
	CRTC_InternalState.CursorActiveLine = 0;
	
	/* cursor is not active in vertical adjust */
	if ((CRTC_InternalState.CRTC_Flags & CRTC_VADJ_FLAG)==0)
	{
		if (CRTC_InternalState.RasterCounter == (CRTCRegisters[10] & 0x01f))
		{
			CRTC_InternalState.CursorActiveLine = 0x0ff;
		}
		else if (CRTC_InternalState.RasterCounter == (CRTCRegisters[11] & 0x01f))
		{
			CRTC_InternalState.CursorActiveLine = 0;
		}
	}
}


static void ASICCRTC_RestartFrame(void)
{
	CRTC_InternalState.CursorBlinkCount++;
	if (CRTC_InternalState.CursorBlinkCount==16)
	{
		CRTC_InternalState.CursorBlinkCount = 0;
		CRTC_InternalState.CursorBlinkOutput^=0x0ff;
	}
	ASICCRTC_UpdateCursorActive();

	CRTC_InternalState.LinesAfterFrameStart = 0;
	/* confirmed on type 3: programming r12/r13 doesn't take effect until the *start* of the frame */
	/* it appears to be reloaded at the very beginning of the very first line */

	/* confirmed: MA and MAStore are loaded from R12/R13 at the start of the frame.
	Repro: Set normal values. Wait until middle of frame so that MAStore should be different to R12/R13.
	Set R1>R0. See all lines are the same. */
	CRTC_InternalState.MA = GET_MA;
	/* confirmed: raster counter and line counter are reset at start of frame */
	CRTC_InternalState.RasterCounter = 0;
	CRTC_InternalState.LineCounter = 0;

#if 0
	/* TODO: confirm */
	if (CRTC_InternalState.InterlaceAndVideoMode == 3)
	{
		CRTC_InternalState.RasterCounter = CRTC_InternalState.Frame;
	}
	else
#endif
	{
		CRTC_InternalState.RasterCounter = 0;
	}

	ASICCRTC_UpdateCursorActive();

	/*	ASICCRTC_VTot(); */

	CRTC_SetFlag(CRTC_VDISP_FLAG);


	CRTC3_DoLineChecks();

	CRTC_DoDispEnable();


	/* generate output RA */
	ASICCRTC_GenerateOutputRasterLine();
}

/********************************/

/* executed for each complete line done by the CRTC */
void    ASICCRTC_DoLine(void)
{
	BOOL SplitSet = FALSE;
	CRTC_InternalState.MA = CRTC_InternalState.MAStore;

	if (ASIC_RasterSplitLineMatch(CRTC_InternalState.LineCounter, CRTC_InternalState.RasterCounter))
	{
		SplitSet = TRUE;
	}
	
	/* confirmed: on type 3, if r8=3 it does have an effect during vertical adjust */
	/* handle raster counter increment depedent on R8 */
//	if (CRTC_InternalState.InterlaceAndVideoMode != 3)
	{
		CRTC_InternalState.RasterCounter = (unsigned char)((CRTC_InternalState.RasterCounter + 1) & 0x01f);
		ASICCRTC_UpdateCursorActive();
	}
#if 0
	else
	{
		CRTC_InternalState.RasterCounter = (unsigned char)((CRTC_InternalState.RasterCounter + 2) & 0x01f);
		ASICCRTC_UpdateCursorActive();
	}
#endif
	CRTC3_DoVerticalSyncCounter();


	if (CRTC_InternalState.CRTC_Flags & CRTC_MR_FLAG)
	{
		CRTC_ClearFlag(CRTC_MR_FLAG);

#if 0
		/* TODO: CONFIRM */
		if (CRTC_InternalState.InterlaceAndVideoMode == 3)
		{
			if (CRTCRegisters[9] & 1)
			{
				/* MaxRaster is odd */
				CRTC_InternalState.RasterCounter = (unsigned char)((CRTC_InternalState.RasterCounter & 0x01) ^ 0x01);
			}
			else
			{
				/* MaxRaster is even */
CRTC_InternalState.RasterCounter = (unsigned char)(CRTC_InternalState.RasterCounter & 0x01);
			}
		}
		else
#endif
		{
			CRTC_InternalState.RasterCounter = 0;

			ASICCRTC_UpdateCursorActive();
		}

		/* Confirmed: On type 3, line counter is not incremented during vertical adjust.  */
		/* R5=8, R7=R4+1 does trigger VSYNC,
		R5=16, R7=R4+2 doesn't trigger VSYNC etc */

		/* Confirmed: Screen split can happen on the first char line of vertical adjust but no other
		lines, this happens because LineCounter is not incremented during vertical adjust.
		So if R5>8, you will not see the screen split happen on the lines. Set R5>16, set screen split to
		repeat in the vertical adjust, observe screen split doesn't happen */
		if (!(CRTC_InternalState.CRTC_Flags & CRTC_VTOT_FLAG))
		{
			CRTC_InternalState.LineCounter = (unsigned char)((CRTC_InternalState.LineCounter + 1) & 0x07f);
		}
	}


	/* are we in vertical adjust ? */
	if (CRTC_InternalState.CRTC_Flags & CRTC_VADJ_FLAG)
	{
		CRTC_InternalState.VertAdjustCount = (unsigned char)((CRTC_InternalState.VertAdjustCount + 1) & 0x01f);

		/* vertical adjust matches counter? */
		if (CRTC_InternalState.VertAdjustCount >= CRTCRegisters[5])
		{
			/* done vertical adjust */
			CRTC_ClearFlag(CRTC_VADJ_FLAG);

			ASICCRTC_RestartFrame();
			CRTC_InternalState.MAStore = GET_MA;
		}
	}




	if (CRTC_InternalState.CRTC_Flags & CRTC_VTOT_FLAG)
	{
		CRTC_ClearFlag(CRTC_VTOT_FLAG);

		CRTC_InternalState.Frame ^= 0x01;

		/* do we want vertical adjust? */
		if (CRTCRegisters[5] != 0)
		{
			/* start vertical adjust */
			CRTC_InternalState.VertAdjustCount = 0;
			CRTC_SetFlag(CRTC_VADJ_FLAG);
		}
		else
		{
			/* restart frame */

			ASICCRTC_RestartFrame();
			if (SplitSet)
			{
				CRTC_InternalState.MAStore = ASIC_GetSSA();
			}
			else
			{
				CRTC_InternalState.MAStore = GET_MA;
			}
		}
	}


	if (CRTC_InternalState.RasterCounter >= CRTCRegisters[9])
	{
		CRTC_SetFlag(CRTC_MR_FLAG);
	}
	else
	{
		CRTC_ClearFlag(CRTC_MR_FLAG);
	}


	if (!(CRTC_InternalState.CRTC_Flags & CRTC_VADJ_FLAG))
	{
		if (CRTC_InternalState.CRTC_Flags & CRTC_MR_FLAG)
		{
			if (!(CRTC_InternalState.CRTC_Flags & CRTC_VADJ_FLAG))
			{
				if (CRTC_InternalState.LineCounter >= CRTCRegisters[4])
				{
					/* Vtot match too */

					CRTC_SetFlag(CRTC_VTOT_FLAG);

				}
			}
			CRTC_SetFlag(CRTC_MR_FLAG);
		}
	}

	if ((CRTC_InternalState.CRTC_Flags & CRTC_HS_FLAG) != 0)
	{
		/* confirmed: PRI interrupts are triggered from the *start* of the HSYNC.

		There is a bug so that if HSYNC from the CRTC is active at the start of the line programmed by PRI, interrupts are triggered immediately.

		The normal position of PRI interrupts is triggered from the start of the interrupt, the position is therefore important.
		The width is normally not important unless it causes the HSYNC to be active on the next line.
		*/
		ASIC_RefreshRasterInterrupt(CRTC_InternalState.LineCounter, CRTC_InternalState.RasterCounter);
	}


	if (CRTCRegisters[1] == 0)
	{
		CRTC3_DoHDisp();
	}


	/* generate output RA */
	ASICCRTC_GenerateOutputRasterLine();

	ASICCRTC_GenerateLineNumber();

	CRTC3_RefreshHStartAndHEnd();

	CRTC3_DoLineChecks();

	/* must be called each line to turn on the correct sprites! */
	ASIC_GenerateSpriteActiveMaskForLine();
}

/*---------------------------------------------------------------------------*/
int CRTC3_GetStatusRegister1(void)
{
	unsigned char Status;

	Status = 0x0ff;

	/* bit 7?? */
	/* bit 6: to be confirmed: 0 when vcc can count, 1 otherwise, because vadj doesn't count */
	/* bit 5: to be confirmed: 0 on line of vertical sync, otherwise 1; if vsync len is 0, this doesn't get cleared */
	/* bit 4: to be confirmed: HC!=HSYNCEND */
	/* bit 3: to be confirmed: HC!=HSYNCPOS */
	/* bit 2: to be confirmed: HC!=DISP */
	/* bit 1: to be confirmed: HC!=HTOT/2 */
	/* bit 0: to be confirmed: HC=HTOT */
	Status &= ~(0x040 | 0x020 | 0x010 | 0x008 | 0x004 | 0x002 | 0x001);

	/* bit 5 = 1 if not on last line of vertical sync, 0 if on last line of vsync */
	if (
		((CRTC_InternalState.CRTC_Flags & CRTC_VSCNT_FLAG) != 0) &&
		(CRTC_InternalState.VerticalSyncCount == (CRTC_InternalState.VerticalSyncWidth-1))
		)
	{
	}
	else
	{
		Status |= 0x020;
	}

	if ((CRTC_InternalState.CRTC_Flags & CRTC_VADJ_FLAG) == 0)
	{
		Status |= 0x040;
	}

	/* bit 0 = 1 if HC == HTOT, 0 otherwise */
	if (CRTC_InternalState.HCount == CRTCRegisters[0])
	{
		Status |= 0x001;
	}

	/* bit 1 = 1 if HC != HTOT/2, 0 otherwise */
	if (CRTC_InternalState.HCount != (CRTCRegisters[0] >> 1))
	{
		Status |= 0x002;
	}

	/* bit 2 = 1 if HC != HDISP, 0 otherwise. ** confirmed ** */
	if (CRTC_InternalState.HCount != CRTCRegisters[1])
	{
		Status |= 0x004;
	}

	/* bit 3 = 1 if HC != HSYNC POS, 0 otherwise */
	if (CRTC_InternalState.HCount != CRTCRegisters[2])
	{
		Status |= 0x008;
	}

	/* never sees end of horizontal sync!?? */
	/* bit 4 = 1 if HC != HSYNC END, 0 otherwise */
	if (CRTC_InternalState.HorizontalSyncCount != CRTC_InternalState.HorizontalSyncWidth)
	{
		Status |= 0x010;
	}


	return Status;


}
/*---------------------------------------------------------------------------*/
int CRTC3_GetStatusRegister2(void)
{
	/* confirm on preasic if these are same as cpc+ status */
	unsigned char Status;

	Status = 0x0ff;

	/* clear bits */
	Status &= ~(0x080 | 0x040 | 0x020| 0x010 | 0x08 );

	/* bit 7: confirmed: 1 if RC==0, 0 otherwise */
	/* bit 6: confirmed: 1 when cursor active, 0 when cursor is active. Seen on all lines */
	/* when cursor set to start on 3 and end on 5, this value is 0 on 3,4 and 5 and 1 at other times */
	/* bit 5: confirmed: 1 if RC!=MR, 0 otherwise */	
	/* bit 4: to be confirmed: 0 if vertical adjust, 1 otherwise */
	/* bit 3: confirmed: 16 frames on/16 frames off. cursor flash rate */
	/* bit 2: ?? */
	/* bit 1: ?? */
	/* bit 0: ?? */

	/* matches cursor blink output */
	if (CRTC_InternalState.CursorBlinkOutput)
	{
		Status|=0x08;
	}
	
	if ((CRTC_InternalState.CRTC_Flags & CRTC_VADJ_FLAG)==0)
	{
		Status |= 0x010;
	}

	if (CRTC_InternalState.CursorActiveLine)
	{
		Status |= 0x040;
	}

	if (CRTC_InternalState.RasterCounter == 0)
	{
		Status |= 0x080;
	}

	/* bit 5: 1 if RC!=MR, 0 otherwise */

	if ((CRTC_InternalState.CRTC_Flags & CRTC_VADJ_FLAG) == 0)
	{
		if (CRTC_InternalState.RasterCounter != CRTCRegisters[9])
		{
			Status |= 0x020;
		}
	}

	return Status;

}
/*---------------------------------------------------------------------------*/
int CRTC3_ReadData(void)
{
	/* PreASIC CRTC - returns same 8 registers repeated; reg 8-15 */
	switch (CRTC_InternalState.CRTC_Reg & 0x07)
	{

		/* light pen high */
	case 0:
		return CRTCRegisters[16];
		/* light pen low */
	case 1:
		return CRTCRegisters[17];
		/* status 1 */
	case 2:
		return CRTC3_GetStatusRegister1();

		/* status 2 */
	case 3:
		return CRTC3_GetStatusRegister2();

		/* screen addr high */
	case 4:
		return CRTCRegisters[12];
		/* screen addr low */
	case 5:
		return CRTCRegisters[13];
		/* cursor addr high */
	case 6:
		return CRTCRegisters[14];
		/* cursor addr low */
	case 7:
		return CRTCRegisters[15];

		default
			:
				break;

	}

	return 0;
}

int CRTC3_GetHorizontalSyncWidth(void)
{
	int HorizontalSyncWidth = CRTCRegisters[3] & 0x0f;
	if (HorizontalSyncWidth == 0)
	{
		HorizontalSyncWidth = 16;
	}
	return HorizontalSyncWidth;
}

int CRTC3_GetVerticalSyncWidth(void)
{
	int VerticalSyncWidth = (CRTCRegisters[3] >> 4) & 0x0f;
	if (VerticalSyncWidth == 0)
	{
		VerticalSyncWidth = 16;
	}
	return VerticalSyncWidth;
}

/*---------------------------------------------------------------------------*/
int CRTC3_ReadStatusRegister(void)
{
	/* mirrors data in register area */
	return CRTC3_ReadData();
}

/*---------------------------------------------------------------------------*/

void CRTC3_WriteData(int Data)
{

	int CRTC_RegIndex = CRTC_InternalState.CRTC_Reg & 0x1f;

	/* store registers using current CRTC information - masking out appropiate bits etc for this CRTC*/
	CRTCRegisters[CRTC_RegIndex] = (unsigned char)(Data & AsicCRTC_WriteMaskTable[CRTC_RegIndex]);

	/* TODO: Only if specific LK is connected? */

	/* With CPC Plus, bit 3 of register 12 determines
	if bit 7 of printer data is 1 or 0 */
	Printer_SetDataBit7State( ( CRTCRegisters[ 12 ] >> 3 ) & 1 );

	ASICCRTC_UpdateState(CRTC_RegIndex);
}
/*---------------------------------------------------------------------------*/
