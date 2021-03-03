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
/* CRTC type '4' - costdown CPC ASIC */

#include "crtc.h"


/*---------------------------------------------------------------------------*/

#define GET_MA  \
        (CRTCRegisters[12]<<8) | (CRTCRegisters[13])


#define CRTC_ClearFlag(x) \
	CRTC_InternalState.CRTC_Flags &=~x

#define CRTC_SetFlag(x)	\
	CRTC_InternalState.CRTC_Flags |=x

void CRTC4_DoReg8(void)
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

	/* interlace mode and video mode*/
//	CRTC_InternalState.InterlaceAndVideoMode = (unsigned char)(CRTCRegisters[8] & 0x003);
	if (CRTCRegisters[8] & 0x01)
	{
		CRTC_SetFlag(CRTC_INTERLACE_ACTIVE);
	}

	CRTC_DoDispEnable();

#if 0
	if (CRTC_InternalState.InterlaceAndVideoMode == 3)
	{
		/* make sure raster counter is even, otherwise the
		interlace and video mode will not always start correctly! */
		CRTC_InternalState.RasterCounter &= ~0x01;
	}
#endif

}


void CRTC4_DoReg1(void)
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

void CRTC4_Reset(void)
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

const unsigned char PreAsicCRTC_WriteMaskTable[32] =
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

int CRTC4_GetStatusRegister1(void)
{
	/* confirm on preasic if these are same as cpc+ status */
	unsigned char Status;
	unsigned char *pCRTCRegisters = CRTCRegisters;

	/* bit 7 is something; lots of time per frame - interrupt? */
	/* bit 6 is nothing */
	/* bit 5 is each frame */
	/* bit 4 is every char line */
	/* bit 3 is every char line too */
	/* bit 2 is every char line ? */
	/* bit 1 is every char line or so */
	/* bit 0 is */

	Status = 0x0ff;
	Status &= ~(0x020 | 0x010 | 0x008 | 0x004 | 0x002 |0x001 );

	/* bit 5 = 1 if not on last line of vertical sync, 0 if on last line of vsync */
	if (CRTC_InternalState.VerticalSyncCount!=CRTC_InternalState.VerticalSyncWidth)
	{
		Status |= 0x020;
	}

	/* bit 0 = 1 if HC == HTOT, 0 otherwise */
	if (CRTC_InternalState.HCount == pCRTCRegisters[0])
	{
		Status |= 0x001;
	}

	/* bit 1 = 1 if HC != HTOT/2, 0 otherwise */
	if (CRTC_InternalState.HCount != (pCRTCRegisters[0]>>1))
	{
		Status |= 0x002;
	}

	/* bit 2 = 1 if HC != HDISP, 0 otherwise. ** confirmed **, bars on left of screen, dark blue on light blue */
	if (CRTC_InternalState.HCount != pCRTCRegisters[1])
	{
		Status |= 0x004;
	}

	/* bit 3 = 1 if HC != HSYNC POS, 0 otherwise */
	if (CRTC_InternalState.HCount != pCRTCRegisters[2])
	{
		Status |= 0x008;
	}

	/* blue lines scrolling up, near the middle of the display, quite wide */

	/* never sees end of horizontal sync!?? */
	/* bit 4 = 1 if HC != HSYNC END, 0 otherwise */
	if (CRTC_InternalState.HorizontalSyncCount!=CRTC_InternalState.HorizontalSyncWidth)
	{
		Status |= 0x010;
	}

	return Status;

}

int CRTC4_GetStatusRegister2(void)
{
	/* confirm on preasic if these are same as cpc+ status */
	unsigned char Status;

	/* bit 7 is */
	/* i, j */

	Status = 0x0ff;

	/* clear bits */
	Status &= ~(0x080 | 0x040 | 0x020);

	/* bit 7: 1 if RC==0, 0 otherwise */
	if (CRTC_InternalState.RasterCounter==0)
	{
		Status|=0x080;
	}

	/* bit 5: 1 if RC!=MR, 0 otherwise */
	if (!(CRTC_InternalState.RasterCounter>=CRTCRegisters[9]))
	{
		Status|=0x020;
	}

	/* bit 3 -? begining of new line but not every frame??,
	bit 2 - hend? */

	return Status;
}

int CRTC4_ReadData(void)
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
			return CRTC4_GetStatusRegister1();

			/* status 2 */
		case 3:
			return CRTC4_GetStatusRegister2();

			/* screen addr high */
		case 4:
			return CRTCRegisters[12];
			/* screen addr low */
		case 5:
			return CRTCRegisters[13];
			/* cursor position high */
		case 6:
			return CRTCRegisters[14];
			/* cursor position low */
		case 7:
			return CRTCRegisters[15];



		default
				:
			break;

	}

	return 0;
}


void CRTC4_DoHDisp(void)
{

			/* if max raster matches, store current MA */
			if (CRTC_InternalState.CRTC_Flags & CRTC_MR_FLAG)
			{
				CRTC_InternalState.MAStore = CRTC_InternalState.MA;
			}
    }
    
static void CRTC4_DoLineChecks(void)
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

	/* check Vertical sync position */
	if (CRTC_InternalState.LineCounter==CRTCRegisters[7])
	{
		/* on CRTC type 3, Vsync will only be triggered on line 0. */
		if (CRTC_InternalState.RasterCounter==0)
		{
			CRTC_InitVsync();
		}
	}
}

/* appears that on crtc type 0 and type 3, Vertical Sync width can be reprogrammed
while it is active. The Vertical Sync Counter is 4-bit. Comparison for both appears to be equal! */
static void CRTC4_DoVerticalSyncCounter(void)
{
	/* are we counting vertical syncs? */
	if (CRTC_InternalState.CRTC_Flags & CRTC_VSCNT_FLAG)
	{
		/* update vertical sync counter */
		CRTC_InternalState.VerticalSyncCount=(CRTC_InternalState.VerticalSyncCount+1)&0x0f;

		/* if vertical sync count = vertical sync width then
		 stop vertical sync */
		if (CRTC_InternalState.VerticalSyncCount==(CRTC_InternalState.VerticalSyncWidth & 0x0f))
		{
			CRTC_InternalState.VerticalSyncCount=0;

			CRTC_ClearFlag(CRTC_VSCNT_FLAG);
		}
	}
}



static void CRTC4_RestartFrame(void)
{

	CRTC_InternalState.LinesAfterFrameStart = 0;

	CRTC_InternalState.MA = GET_MA;
	CRTC_InternalState.MAStore= CRTC_InternalState.MA;

	CRTC_InternalState.RasterCounter = 0;
	CRTC_InternalState.LineCounter = 0;

#if 0
	if (CRTC_InternalState.InterlaceAndVideoMode == 3)
	{
		CRTC_InternalState.RasterCounter = CRTC_InternalState.Frame;
	}
	else
#endif
	{
		CRTC_InternalState.RasterCounter = 0;
	}


	CRTC_SetFlag(CRTC_VDISP_FLAG);


	CRTC4_DoLineChecks();

	CRTC_DoDispEnable();
}


/* executed for each complete line done by the CRTC */
void    CRTC4_DoLine(void)
{
#if 0
	if (CRTC_InternalState.InterlaceAndVideoMode!=3)
	{
		/* increment raster counter */
		CRTC_InternalState.RasterCounter = (unsigned char)((CRTC_InternalState.RasterCounter + 1) & 0x01f);
	}
	else
#endif
	{
		CRTC_InternalState.RasterCounter = (unsigned char)((CRTC_InternalState.RasterCounter + 2) & 0x01f);
	}

	CRTC4_DoVerticalSyncCounter();

	if (CRTC_InternalState.CRTC_Flags & CRTC_MR_FLAG)
	{
		/*			CRTC_ClearFlag(CRTC_VSYNC_TRIGGERED_FLAG); */

		CRTC_ClearFlag(CRTC_MR_FLAG);

#if 0
		if (CRTC_InternalState.InterlaceAndVideoMode == 3)
		{
			if (CRTCRegisters[9] & 1)
			{
				/* MaxRaster is odd */
				CRTC_InternalState.RasterCounter = (unsigned char)((CRTC_InternalState.RasterCounter & 0x01)^ 0x01);
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
		}

		/* if we are to restart the frame, do not increment counter */
		if (!(CRTC_InternalState.CRTC_Flags & CRTC_VTOT_FLAG))
		{
			CRTC_InternalState.LineCounter = (unsigned char)((CRTC_InternalState.LineCounter+1) & 0x07f);

			/*CRTC_DoLineChecks(); */
		}
	}

	CRTC_InternalState.MA = CRTC_InternalState.MAStore;

	/* are we in vertical adjust ? */
	if (CRTC_InternalState.CRTC_Flags & CRTC_VADJ_FLAG)
	{
		/* vertical adjust matches counter? */
		if (CRTC_InternalState.RasterCounter>=CRTCRegisters[5])
		{
			CRTC_ClearFlag(CRTC_VADJ_FLAG);

			CRTC4_RestartFrame();
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
		}
		else
		{
			/* restart frame */

			CRTC4_RestartFrame();
		}
	}



	if (!(CRTC_InternalState.CRTC_Flags & CRTC_VADJ_FLAG))
	{
		if (CRTC_InternalState.RasterCounter>=CRTCRegisters[9])
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

	CRTC4_DoLineChecks();
}

int CRTC4_GetRAOutput(void)
{
	return CRTC_InternalState.RasterCounter;
}


/* cannot update reg 7 and get it to start mid line...?
 reg 7 must be changed before line 0 starts!
*/



int CRTC4_GetHorizontalSyncWidth(void)
{
	/* all values of horizontal sync generate an interrupt; this indicates
	that hsync can never be 0 length */
	int HorizontalSyncWidth = CRTCRegisters[3] & 0x0f;
	if (HorizontalSyncWidth == 0)
	{
		HorizontalSyncWidth = 16;
	}
	return HorizontalSyncWidth;
}

int CRTC4_GetVerticalSyncWidth(void)
{
	/* tests confirm: 0 is a length of 16, all others are the length defined
	by the value (e.g. 1 gives vsync length of 1, 2 gives vsync length of 2 etc */
	int VerticalSyncWidth = (CRTCRegisters[3] >> 4) & 0x0f;
	if (VerticalSyncWidth == 0)
	{
		VerticalSyncWidth = 16;
	}
	return VerticalSyncWidth;
}


/* ASIC CRTC Update State */
static void CRTC4_UpdateState(int RegIndex)
{
	switch (RegIndex)
	{
		case 6:
		{
//			if (CRTC_InternalState.RasterCounter>=CRTCRegisters[9])
//			{
//				CRTC4_DoLineChecks();
//			}
		}
		break;

		case 7:
		{
	//		CRTC4_DoLineChecks();
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
		//	CRTC4_DoReg3();
		}
		break;


		case 9:
		{
#if 0
			if (CRTC_InternalState.HCount!=CRTCRegisters[0])
			{
				/* CRTC_InternalState.RasterCounter>=CRTC_REG9, Not latched */
				if (CRTC_InternalState.RasterCounter>=CRTCRegisters[9])
				{
					CRTC_SetFlag(CRTC_MR_FLAG);

					if (CRTC_InternalState.LineCounter==CRTCRegisters[4])
					{
						CRTC_SetFlag(CRTC_VTOT_FLAG);
					}
				}
				else
				{
					if (CRTC_InternalState.CRTC_Flags & CRTC_MR_FLAG)
					{
						/* was max raster set? */
						if (CRTC_InternalState.LineCounter == CRTCRegisters[4])
						{
							/* we were going to end the frame - but don't!*/
							CRTC_ClearFlag(CRTC_VTOT_FLAG);
						}
					}

					CRTC_ClearFlag(CRTC_MR_FLAG);
				}
			}
#endif
		}
		break;

		case 4:
		{
#if 0
			/* not sure if should be equals or greater.. */
			if (CRTC_InternalState.CRTC_Flags & CRTC_MR_FLAG)
			{
				if (CRTC_InternalState.HCount!=CRTCRegisters[0])
				{
					if (CRTC_InternalState.LineCounter >= CRTCRegisters[4])
					{
						CRTC_SetFlag(CRTC_VTOT_FLAG);
					}
					else
					{
						CRTC_ClearFlag(CRTC_VTOT_FLAG);
					}
				}
			}
#endif
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

int CRTC4_ReadStatusRegister(void)
{
	/* type 4 mirrors it's data here */
	return CRTC4_ReadData();
}



void CRTC4_WriteData(int Data)
{
	int CRTC_RegIndex = CRTC_InternalState.CRTC_Reg & 0x1f;

	/* store registers using current CRTC information - masking out appropiate bits etc for this CRTC*/
	CRTCRegisters[CRTC_RegIndex] = (unsigned char)(Data & PreAsicCRTC_WriteMaskTable[CRTC_RegIndex]);

	CRTC4_UpdateState(CRTC_RegIndex);
}
