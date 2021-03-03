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
/* CRTC Type '5' - HD6845R */

#include "crtc.h"

extern CRTC_INTERNAL_STATE CRTC_InternalState;
extern unsigned char            CRTCRegisters[32];

/*---------------------------------------------------------------------------*/

#define GET_MA  \
        (CRTCRegisters[12]<<8) | (CRTCRegisters[13])


#define CRTC_ClearFlag(x) \
	CRTC_InternalState.CRTC_Flags &=~x

#define CRTC_SetFlag(x)	\
	CRTC_InternalState.CRTC_Flags |=x
/*---------------------------------------------------------------------------*/

/* HD6845R */

const unsigned char HD6845R_ReadMaskTable[32]=
{
	0x0ff,  /* Horizontal Total */
	0x0ff,  /* Horizontal Displayed */
	0x0ff,  /* Horizontal Sync Position */
	0x0ff,  /* Sync Width */
	0x0ff,  /* Vertical Total */
	0x0ff,  /* Vertical Total Adjust */
	0x0ff,  /* Vertical Displayed */
	0x0ff,  /* V. Sync Position */
	0x0ff,  /* Interlace Mode and Skew */
	0x0ff,  /* Max Scan Line Address */
	0x0ff,  /* Cursor Start */
	0x0ff,  /* Cursor End */
	0x0ff,  /* Start Address (H) */
	0x0ff,  /* Start Address (L) */
	0x000,  /* Cursor (H) */
	0x000,  /* Cursor (L) */

	0x000,  /* Light Pen (H) */
	0x000,  /* Light Pen (L) */
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
	0x000,
};

const unsigned char HD6845R_WriteMaskTable[32]=
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

void CRTC1B_Reset(void)
{
	int i;

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

#if 0
/* TODO: Test */
static BOOL	CRTC1B_HTotMatch(void)
{
	if (
		(CRTC_InternalState.HCount == CRTCRegisters[0]) ||
		(CRTC_InternalState.HCount == (CRTCRegisters[0] & 0x0fe))
	)
	{
		return TRUE;
	}

	return FALSE;
}
#endif

void CRTC1B_DoReg8(void)
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

	if (CRTCRegisters[8] & 0x01)
	{
		CRTC_SetFlag(CRTC_INTERLACE_ACTIVE);
	}

	/* interlace mode and video mode*/
//	CRTC_InternalState.InterlaceAndVideoMode = (unsigned char)(CRTCRegisters[8] & 0x003);

	CRTC_DoDispEnable();

//	if (CRTC_InternalState.InterlaceAndVideoMode == 3)
//	{
//		/* make sure raster counter is even, otherwise the
//		interlace and video mode will not always start correctly! */
//		CRTC_InternalState.RasterCounter &= ~0x01;
//	}



}


void CRTC1B_DoReg1(void)
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


/* TODO: test */
static void CRTC1B_DoLineChecks(void)
{

	/* reg 6 can be reprogrammed at any time */
	if (CRTC_InternalState.LineCounter == CRTCRegisters[6])
	{
		CRTC_ClearFlag(CRTC_VDISP_FLAG);

		CRTC_DoDispEnable();
	}

	if (CRTC_InternalState.LineCounter == CRTCRegisters[6])
	{
		CRTC_ClearFlag(CRTC_VDISP_FLAG);

		CRTC_DoDispEnable();
	}

	/* check Vertical sync position */
	if (CRTC_InternalState.LineCounter==CRTCRegisters[7])
	{
		/* on CRTC type 1, Vsync can be triggered on any line of the char. */
		CRTC_InitVsync();
	}
}


void CRTC1B_UpdateState(int RegIndex)
{
	switch (RegIndex)
	{
		case 0:
		{
#if 0
			if (!CRTC1B_HTotMatch())
			{
				CRTC_SetFlag(CRTC_HTOT_FLAG);
			}
			else
			{
				CRTC_ClearFlag(CRTC_HTOT_FLAG);
			}
#endif
		}
		break;

		case 14:
		case 15:
			CRTC_InternalState.CursorMA = (CRTCRegisters[14]<<8)|CRTCRegisters[15];
			break;


		case 7:
		{
		//	CRTC1B_DoLineChecks();
		}
		break;

		/* horizontal and vertical sync width registers */
		case 3:
		{
//			CRTC1B_DoReg3();
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

		case 6:
		{
/* TODO: test */
			/* if type 1, we can change reg 6 at any time,and
			it takes effect*/
			/*CRTC_DoLineChecks(); */
			/* reg 6 can be reprogrammed at any time */
			if (CRTC_InternalState.LineCounter == CRTCRegisters[6])
			{
				CRTC_ClearFlag(CRTC_VDISP_FLAG);
			}
			else
			{
				CRTC_SetFlag(CRTC_VDISP_FLAG);
			}
			CRTC_DoDispEnable();
		}
		break;

		case 9:
		{
#if 0
			if (!CRTC1B_HTotMatch())
			{
				if (CRTC_InternalState.RasterCounter==CRTCRegisters[9])
				{
					CRTC_SetFlag(CRTC_MR_FLAG);

					if (CRTC_InternalState.LineCounter == CRTCRegisters[4])
					{
						CRTC_SetFlag(CRTC_VTOT_FLAG);
					}
				}
				else
				{
					if (CRTC_InternalState.CRTC_Flags & CRTC_MR_FLAG)
					{
						if (!CRTC1B_HTotMatch())
						{
							if (CRTC_InternalState.LineCounter == CRTCRegisters[4])
							{
								CRTC_ClearFlag(CRTC_VTOT_FLAG);
							}
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
			if (CRTC_InternalState.CRTC_Flags & CRTC_MR_FLAG)
			{
				if (!CRTC1B_HTotMatch())
				{
					if (CRTC_InternalState.LineCounter == CRTCRegisters[4])
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

		default
				:
			break;
	}
}

void CRTC1B_DoHDisp(void)
{
			CRTC_ClearFlag(CRTC_HDISP_FLAG);
			CRTC_DoDispEnable();

			/* if max raster matches, store current MA */
			if (CRTC_InternalState.CRTC_Flags & CRTC_MR_FLAG)
			{
				CRTC_InternalState.MAStore = CRTC_InternalState.MA;
			}
    }

static void CRTC1B_RestartFrame(void)
{

	CRTC_InternalState.LinesAfterFrameStart = 0;

	CRTC_InternalState.MA = GET_MA;
	CRTC_InternalState.MAStore = CRTC_InternalState.MA;

	CRTC_InternalState.LineCounter = 0;
	CRTC_InternalState.RasterCounter = 0;


	CRTC_SetFlag(CRTC_VDISP_FLAG);

	CRTC_DoDispEnable();

	CRTC1B_DoLineChecks();



	/*        VertAdjustPush = CRTCRegisters[5]; */


//	if (CRTC_InternalState.InterlaceAndVideoMode == 3)
//	{
//		CRTC_InternalState.RasterCounter = CRTC_InternalState.Frame;
//	}
//	else
	{
		CRTC_InternalState.RasterCounter = 0;
	}


	CRTC_SetFlag(CRTC_VDISP_FLAG);


	CRTC1B_DoLineChecks();

	CRTC_DoDispEnable();
}


void	CRTC1B_MaxRasterMatch(void)
{
	CRTC_ClearFlag(CRTC_MR_FLAG);

#if 0
	if (CRTC_InternalState.InterlaceAndVideoMode == 3)
	{
		/* in interlace sync and video mode, the raster counter increments by 1,
		and the RA output is adjusted. This compares 1/2R9 to CRTC_InternalState.RasterCounter. */
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





/* on CRTC1, vsync of 40 only accepted if bc00,5 is 9!! */
/* check that if 39 is programmed with bc00,5==0 no vsync is generated */

/* executed for each complete line done by the CRTC */
void    CRTC1B_DoLine(void)
{

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
		}

	}

	/* TODO: test */
/* on CRTC type 1, MA can be changed through char line 0 of frame,
	the address generated depends on MA and what the current RA is, this
	is done in the CRTC_WriteReg */
	if (CRTC_InternalState.LineCounter == 0)
	{
		CRTC_InternalState.MAStore = GET_MA;
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
			CRTC1B_RestartFrame();

		}
	}


	if (CRTC_InternalState.CRTC_Flags & CRTC_VTOT_FLAG)
	{
		CRTC_ClearFlag(CRTC_VTOT_FLAG);

		/* load vertical adjust */
		/*VertAdjust = CRTCRegisters[5]; */

		CRTC_InternalState.Frame ^= 0x01;

		/* is it active? i.e. VertAdjust!=0 */
		if (CRTCRegisters[5]!=0)
		{

			/* yes */
			CRTC_InternalState.VertAdjustCount = 0;
			CRTC_SetFlag(CRTC_VADJ_FLAG);

			CRTC_InternalState.LineCounter = (unsigned char)((CRTC_InternalState.LineCounter+1) & 0x07f);

			/*CRTC_DoLineChecks(); */

		}
		else
		{
			CRTC1B_RestartFrame();
		}
	}

	CRTC1B_MaxRasterMatch();

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

	CRTC1B_DoLineChecks();
}


int CRTC1B_GetRAOutput(void)
{
	return CRTC_InternalState.RasterCounter;
}


/*---------------------------------------------------------------------------*/

int CRTC1B_GetHorizontalSyncWidth(void)
{
	int HorizontalSyncWidth = CRTCRegisters[3] & 0x0f;

	if (HorizontalSyncWidth == 0)
	{
		HorizontalSyncWidth = 16;
	}
	return HorizontalSyncWidth;
}

int CRTC1B_GetVerticalSyncWidth(void)
{
	return 16;
}

/*---------------------------------------------------------------------------*/

int CRTC1B_ReadData(void)
{
	int   CRTC_RegIndex = (CRTC_InternalState.CRTC_Reg & 0x01f);

	/* unreadable registers return 0 */
	return (CRTCRegisters[CRTC_RegIndex] & (~HD6845R_ReadMaskTable[CRTC_RegIndex]));
}

/*---------------------------------------------------------------------------*/
void CRTC1B_WriteData(int Data)
{
	int CRTC_RegIndex = CRTC_InternalState.CRTC_Reg & 0x1f;

	/* store registers using current CRTC information - masking out appropiate bits etc for this CRTC*/
	CRTCRegisters[CRTC_RegIndex] = (unsigned char)(Data & HD6845R_WriteMaskTable[CRTC_RegIndex]);

	CRTC1B_UpdateState(CRTC_RegIndex);
}

/*---------------------------------------------------------------------------*/
int CRTC1B_ReadStatusRegister(void)
{
	/* no status register */
	return 0x0ff;
}
