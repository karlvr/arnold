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
/* CRTC type '2' - MC6845 */
#include "crtc.h"

/* on type 2, border is re-enabled at htot half a character before it ends */

extern CRTC_INTERNAL_STATE CRTC_InternalState;
extern unsigned char            CRTCRegisters[32];

void CRTC_DoLineChecksCRTC2(void);

/*---------------------------------------------------------------------------*/

#define GET_MA  \
        (CRTCRegisters[12]<<8) | (CRTCRegisters[13])


#define CRTC_ClearFlag(x) \
	CRTC_InternalState.CRTC_Flags &=~x

#define CRTC_SetFlag(x)	\
	CRTC_InternalState.CRTC_Flags |=x
/*---------------------------------------------------------------------------*/
void CRTC2_DoReg8(void)
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

	CRTC_DoReg1();

	CRTC_DoDispEnable();
}


void CRTC2_DoReg1(void)
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

/* MC6845 (type 2) */

void CRTC2_Reset(void)
{
	int i;

	/* MC69845
	The /RESET input is used to reset the CRTC. A low level on the /RESET input forces the CRTC into the following state.

	    * All counters in the CRTC are cleared and the device stops the display operation.
	    * All the outputs are driven low
	            NOTE: The horizontal sync output is not defined until after R2 is programmed.
	    * The control registers of the CRTC are not affected and remain unchanged

	Functionality of /RESET differs from that of other M6800 parts in the following functions.

	    * The /RESET input and the LPSTB input are encoded as shown in Table 1.

	      TABLE 1 - CRTC OPERATING MODE
	      /RESET 	/LPSTB 	Operating Mode
	      0 	0 	Reset
	      0 	1 	Test Mode
	      1 	0 	Normal Mode
	      1 	1 	Normal Mode


	      The test mode configures the memory addresssess as two independant 7-bit counters to minimize test time.
	    * After /RESET has gone low and (LPSTB=0), MA0-MA13 and RA0-RA4 will be driven low on the falling edge of CLK.
	     /RESET must remain low for at least one cycle of the character clock (CLK).
	    * The CRT resumes the display operation immediatly after the release of /RESET, DE, and the CURSOR are not active until after the first frame has been displayed

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

}


void CRTC2_DoHDisp(void)
{
	CRTC_ClearFlag(CRTC_HDISP_FLAG);
	CRTC_DoDispEnable();

	/* confirmed: on type 2, MA is reloaded on last line of vertical adjust at hdisp time */
	if (CRTC_InternalState.CRTC_Flags & CRTC_VADJ_FLAG)
	{
		/* last line of vertical adjust */
		/* TODO: Separate vertical adjust counter or not? */
		/* TODO: -1 needs to be fixed for r5 overflow */
		if (CRTC_InternalState.VertAdjustCount == (CRTCRegisters[5]-1))
		{
			CRTC_InternalState.MA = GET_MA;
			CRTC_InternalState.MAStore = CRTC_InternalState.MA;
		}
		else
		{
			/* gerald's tests confirm mastore is reloaded when vertical is visible */
			if ((CRTC_InternalState.CRTC_Flags & CRTC_MR_FLAG) && (CRTC_InternalState.CRTC_Flags & CRTC_VDISP_FLAG))
			{
				CRTC_InternalState.MAStore = CRTC_InternalState.MA;
			}
		}
	}
	else
	{
		/* confirmed: on type 2, R12/R13 is read at HDISP time when RCC=R9, HCC=R1 and VCC=R4. Need to test with no border. */
		/* confirmed: on type 2, if VADJ is active at HDISP time we *dont* reload, allow it to continue, this allows the graphics to continue */

		/* confirmed: if rcc=r9 at HDISP time then store MA for reload. It is possible to change R9 around R1 time only
		and get the graphics to repeat but doesn't cause problems for RCC */
		if (
			(CRTC_InternalState.CRTC_Flags & CRTC_MR_FLAG) ||
			// confirmed: MR not set. Line Counter not incremented. RasterCounter continues
			(((CRTCRegisters[8] & 0x03) == 0x03) && (CRTC_InternalState.RasterCounter == (CRTCRegisters[9]>>1)))
			)
		{
			/* VTOT match but not VADJ */
			if ((CRTC_InternalState.CRTC_Flags & CRTC_VTOT_FLAG) && (CRTCRegisters[5] == 0))
			{
				CRTC_InternalState.MA = GET_MA;
				CRTC_InternalState.MAStore = CRTC_InternalState.MA;
			}
			else
			{
				/* VADJ or no VTOT match */
				CRTC_InternalState.MAStore = CRTC_InternalState.MA;
			}
		}
	}
}



const unsigned char MC6845_ReadMaskTable[32]=
{
	0x000,  /* Horizontal Total */
	0x000,  /* Horizontal Displayed */
	0x000,  /* Horizontal Sync Position */
	0x000,  /* Sync Width */
	0x000,  /* Vertical Total */
	0x000,  /* Vertical Total Adjust */
	0x000,  /* Vertical Displayed */
	0x000,  /* V. Sync Position */
	0x000,  /* Interlace Mode and Skew */
	0x000,  /* Max Scan Line Address */
	0x000,  /* Cursor Start */
	0x000,  /* Cursor End */
	0x000,  /* Start Address (H) */
	0x000,  /* Start Address (L) */
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

const unsigned char MC6845_WriteMaskTable[32]=
{
	0x0ff,
	0x0ff,
	0x0ff,
	0x00f,
	0x07f,
	0x01f,
	0x07f,
	0x07f,
	0x003,
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


/*************************/
/* CRTC type 2 - MC6845P */
/*************************/

int CRTC2_GetHorizontalSyncWidth(void)
{
	/* tests show that interrupt is generated for all hsync values */
	/* indicates that 0, gives a hsync of 16 */
	int HorizontalSyncWidth = CRTCRegisters[3] & 0x0f;
	if (HorizontalSyncWidth == 0)
	{
		HorizontalSyncWidth = 16;
	}

	return HorizontalSyncWidth;
}

int CRTC2_GetVerticalSyncWidth(void)
{
	return 16;
}


void CRTC2_DoLineChecks(void)
{

	if (CRTC_InternalState.LineCounter == CRTCRegisters[6])
	{
		CRTC_ClearFlag(CRTC_VDISP_FLAG);

		CRTC_DoDispEnable();
	}

	/* check Vertical sync position */
	if (CRTC_InternalState.LineCounter==CRTCRegisters[7])
	{
		/* confirmed: if hsync is active, don't let vsync start */
		/* however, hsync does generate an interrupt - what is not clear is how
		long the hsync actually is */
		if ((CRTC_InternalState.CRTC_Flags & CRTC_HS_FLAG)==0)
		{
			CRTC_InitVsync();
		}

	}
}

static void CRTC2_RestartFrame(void)
{

	CRTC_InternalState.LinesAfterFrameStart = 0;
	CRTC_InternalState.RasterCounter = 0;

	/* confirmed, MA for new frame is fetched when VC=R4, and HCC=R1 */

	CRTC_InternalState.LineCounter = 0;

	CRTC_SetFlag(CRTC_VDISP_FLAG);

	CRTC_DoDispEnable();

	CRTC_DoLineChecksCRTC2();

}


void	CRTC2_MaxRasterMatch(void)
{
	if (CPC_GetCRTCType() != 2)
		return;

#if 0
	if ((CRTCRegisters[8]&0x03)==0x03)
	{
		/* confirmed: comparison is against R9>>1. RasterCounter increments normally on type 2 */
		if (CRTC_InternalState.RasterCounter==(CRTCRegisters[9]>>1))
		{
			CRTC_SetFlag(CRTC_MR_FLAG);
		}
	}
	else
#endif
	{
		if (CRTC_InternalState.RasterCounter==CRTCRegisters[9])
		{
			CRTC_SetFlag(CRTC_MR_FLAG);
		}
	}

}


/* executed for each complete line done by the CRTC */
void    CRTC2_DoLine(void)
{
	/* confirmed; on type 2, during r5, increment as normal it seems, vcc and rc
	showing normal text */

	/* increment raster counter */
	CRTC_InternalState.RasterCounter = (unsigned char)((CRTC_InternalState.RasterCounter + 1) & 0x01f);


	CRTC_DoVerticalSyncCounter();

	if (CRTC_InternalState.CRTC_Flags & CRTC_MR_FLAG)
	{

		CRTC_ClearFlag(CRTC_MR_FLAG);

		CRTC_InternalState.RasterCounter = 0;

		if (!(CRTC_InternalState.CRTC_Flags & CRTC_VTOT_FLAG))
		{
			CRTC_InternalState.LineCounter = (unsigned char)((CRTC_InternalState.LineCounter+1) & 0x07f);

			CRTC_DoLineChecksCRTC2();
		}

	}


	CRTC_InternalState.MA = CRTC_InternalState.MAStore;


	/* are we in vertical adjust ? */
	if (CRTC_InternalState.CRTC_Flags & CRTC_VADJ_FLAG)
	{
		CRTC_InternalState.VertAdjustCount = (unsigned char)((CRTC_InternalState.VertAdjustCount+1) & 0x01f);

		/* vertical adjust matches counter? */
		if (CRTC_InternalState.VertAdjustCount==CRTCRegisters[5])
		{
			CRTC_ClearFlag(CRTC_VADJ_FLAG);
			CRTC2_RestartFrame();

		}
	}

	if (CRTC_InternalState.CRTC_Flags & CRTC_VTOT_FLAG)
	{
		CRTC_ClearFlag(CRTC_VTOT_FLAG);

		CRTC_InternalState.Frame ^= 0x01;

		/* is it active? i.e. VertAdjust!=0 */
		if (CRTCRegisters[5]!=0)
		{
			/* yes */
			CRTC_InternalState.VertAdjustCount = 0;
			CRTC_SetFlag(CRTC_VADJ_FLAG);

			CRTC_InternalState.LineCounter = (unsigned char)((CRTC_InternalState.LineCounter+1) & 0x07f);

			CRTC_DoLineChecksCRTC2();

		}
		else
		{
			CRTC2_RestartFrame();
		}
	}

	CRTC2_MaxRasterMatch();

	if (CRTC_InternalState.CRTC_Flags & CRTC_MR_FLAG)
	{
		if (!(CRTC_InternalState.CRTC_Flags & CRTC_VADJ_FLAG))
		{
#if 0
			if (CRTC_InternalState.InterlaceAndVideoMode == 3)
			{
				if ((CRTC_InternalState.LineCounter>>1) == CRTCRegisters[4])
				{
					CRTC_SetFlag(CRTC_VTOT_FLAG);
				}
			}
			else
#endif
			{
				if (!(CRTC_InternalState.CRTC_Flags & CRTC_HS_FLAG))
				{
					if (CRTC_InternalState.LineCounter == CRTCRegisters[4])
					{
						/* Vtot match too */

						CRTC_SetFlag(CRTC_VTOT_FLAG);
					}
				}
			}
		}
	}
}


int CRTC2_GetRAOutput(void)
{
	if ((CRTCRegisters[8]&0x03)==0x03)
	{
		/* confirmed: gives results seen in tests */
		return (CRTC_InternalState.RasterCounter << 1) | CRTC_InternalState.Frame;
	}

	return CRTC_InternalState.RasterCounter;
}



/* do line checks for CRTC type 2 */
void             CRTC_DoLineChecksCRTC2(void)
{
	CRTC2_DoLineChecks();
}


static void CRTC2_UpdateState(int RegIndex)
{
	/* registers that can be changing immediatly */
	switch (RegIndex)
	{

	case 4:
	{
		if (CRTC_InternalState.CRTC_Flags & CRTC_MR_FLAG)
		{
			if ((!(CRTC_InternalState.CRTC_Flags & CRTC_VADJ_FLAG)) && (!(CRTC_InternalState.CRTC_Flags & CRTC_HS_FLAG)))
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
		if (CRTC_InternalState.RasterCounter == CRTCRegisters[9])
		{
			CRTC_SetFlag(CRTC_MR_FLAG);
		}
		else
		{
			CRTC_ClearFlag(CRTC_MR_FLAG);

		}
	}
	break;

	case 6:
	{
		/* confirmed immediate change allowed */
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
	
		case 7:
		{
			/* check Vertical sync position */
			if (CRTC_InternalState.LineCounter == CRTCRegisters[7])
			{
				/* confirmed: if hsync is active, don't let vsync start */
				/* however, hsync does generate an interrupt - what is not clear is how
				long the hsync actually is */
				if ((CRTC_InternalState.CRTC_Flags & CRTC_HS_FLAG) == 0)
				{
					CRTC_InitVsync();
				}

			}
		}
		break;
		/* horizontal and vertical sync width registers */
		case 3:
		{
	//		CRTC2_DoReg3();
		}
		break;


		case 8:
		{
			CRTC_DoReg8();

		}
		break;

		case 1:
		{
			CRTC_DoReg1();
		}
		break;

		case 14:
		case 15:
		{
			CRTC_InternalState.CursorMA = (CRTCRegisters[14]<<8)|CRTCRegisters[15];
		}
		break;

		default
				:
			break;
	}




}

/*---------------------------------------------------------------------------*/
int CRTC2_ReadStatusRegister(void)
{
	/* no status register */
	return 0x0ff;
}

/*---------------------------------------------------------------------------*/
int CRTC2_ReadData(void)
{
	int CRTC_RegIndex = (CRTC_InternalState.CRTC_Reg & 0x01f);

	/* unreadable registers return 0 */
	return (CRTCRegisters[CRTC_RegIndex] & MC6845_ReadMaskTable[CRTC_RegIndex]);
}


/*---------------------------------------------------------------------------*/
void CRTC2_WriteData(int Data)
{

	int CRTC_RegIndex = CRTC_InternalState.CRTC_Reg & 0x1f;

	/* store registers using current CRTC information - masking out appropiate bits etc for this CRTC*/
	CRTCRegisters[CRTC_RegIndex] = (unsigned char)(Data & MC6845_WriteMaskTable[CRTC_RegIndex]);


	CRTC2_UpdateState(CRTC_RegIndex);
}
/*---------------------------------------------------------------------------*/

