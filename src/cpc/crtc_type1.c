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
/* CRTC type '1' - UM6845R */
#include "crtc.h"
#include "cpc.h"

/* NOTE: On type 1: colour black is shown when hsync is active and this overrides border.
otherwise real black for hsync is shown.

colours can be seen over the top of the hsync if it's short just like on plus.

2 cycles delay, followed by 4 cycles (where 5 or more is used) */


//void CRTC1_DoReg3();


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

/* UM6845R */

const unsigned char UM6845R_ReadMaskTable[32]=
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
	0x0ff,
};

const unsigned char UM6845R_WriteMaskTable[32]=
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

#define CRTC_STATUS_IN_VERTICAL_BLANKING       (1<<5)
#define CRTC_STATUS_LPEN_REGISTER_FULL          (1<<6)

/* on CRTC1, vsync of 40 only accepted if bc00,5 is 9!! */
/* check that if 39 is programmed with bc00,5==0 no vsync is generated */

/* confirmed: unused bits are 0 */
static char UM6845R_StatusRegister=0;

void CRTC1_Reset(void)
{

	int i;

	/* TODO: Verify CRTC1 reset condition */
	/* set light pen registers - this is what my type
	1 reports */
	CRTCRegisters[16] = 0;
	CRTCRegisters[17] = 0;
	
	/* confirmed: my type 1 reports register full at power on time */
	/* shows 0x040 */
	UM6845R_StatusRegister = CRTC_STATUS_LPEN_REGISTER_FULL;

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


void CRTC1_DoHDisp(void)
{
			CRTC_ClearFlag(CRTC_HDISP_FLAG);
			CRTC1_DoDispEnable();

			/* confirmed: if rcc=r9 at HDISP time then store MA for reload. It is possible to change R9 around R1 time only
			and get the graphics to repeat but doesn't cause problems for RCC */
			/* confirmed: testing from gerald indicates mastore is not updated in vertical border */
			if ((CRTC_InternalState.CRTC_Flags & CRTC_MR_FLAG) && (CRTC_InternalState.CRTC_Flags & CRTC_VDISP_FLAG))
			{
				CRTC_InternalState.MAStore = CRTC_InternalState.MA;
			}
		}

void CRTC1_DoDispEnable(void)
{
	/* matches my tests nearly. line 0 and other lines and line 1 but causes problems for 
	From Scratch */

  /* if R6=0 and line counter is not 0 border is displayed. 	
		R6==VCC comparison also switches on border, but can't be re-enabled after.
		Border is also enabled after HDISP and VDISP */
	if (
		/* 0 = border active in vertical and horizontal */
		/* CRTC_VDISP_FLAG, graphics visible in vertical */
		/* CRTC_HDISP_FLAG, graphics visible in horizontal */
		((CRTC_InternalState.CRTC_Flags & (CRTC_HDISP_FLAG | CRTC_VDISP_FLAG)) != (CRTC_HDISP_FLAG | CRTC_VDISP_FLAG)) ||
		((CRTCRegisters[6] == 0) && (CRTC_InternalState.LineCounter != 0))
		)
	{
		CRTC_SetDispEnable(FALSE);
	}
	else
	{
      CRTC_SetDispEnable(TRUE);
    }
  }
  

/*************************/
/* CRTC type 1 - UM6845R */
/*************************/

static BOOL	CRTC1_HTotMatch(void)
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

void CRTC1_DoReg8(void)
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

	CRTC1_DoDispEnable();

//	if (CRTC_InternalState.InterlaceAndVideoMode == 3)
//	{
//		/* make sure raster counter is even, otherwise the
//		interlace and video mode will not always start correctly! */
//		CRTC_InternalState.RasterCounter &= ~0x01;
//	}

}


void CRTC1_DoReg1(void)
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
	CRTC1_DoDispEnable();
}

static void CRTC1_DoLineChecks(void)
{

	/* confirmed: reg 6 can be reprogrammed at any time, and result takes effect immediately so that you can do changes part way though the line */
	if (CRTC_InternalState.LineCounter == CRTCRegisters[6])
	{
		CRTC_ClearFlag(CRTC_VDISP_FLAG);
	}
	CRTC1_DoDispEnable();
}

void CRTC1_UpdateState(int RegIndex)
{
	switch (RegIndex)
	{
		case 0:
		{
#if 0
			if (!CRTC1_HTotMatch())
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

		case 7:
		{
		//	CRTC1_DoLineChecks();
		}
		break;

		/* horizontal and vertical sync width registers */
		case 3:
		{
			//CRTC1_DoReg3();
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
			/* confirmed: reg 6 can be reprogrammed at any time, and result takes effect immediately so that you can do changes part way though the line */
			if (CRTC_InternalState.LineCounter == CRTCRegisters[6])
			{
				CRTC_ClearFlag(CRTC_VDISP_FLAG);
			}

			/* update anyway for R6=0 */
			CRTC1_DoDispEnable();
		}
		break;


		#if 0
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
#endif
		case 4:
		{
#if 0
			if (CRTC_InternalState.CRTC_Flags & CRTC_MR_FLAG)
			{
				if (!CRTC1_HTotMatch())
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

		case 15:
		case 14:
		{

			CRTC_InternalState.CursorMA = (CRTCRegisters[14]<<8)|CRTCRegisters[15];
		}
		break;

		case 31:
		{
			/* crtc type 1 has 0x0ff for reg 31 */
			CRTCRegisters[31] = 0x0ff;
		}
		break;

		default
				:
			break;
	}
}

static void CRTC1_RestartFrame(void)
{

	CRTC_InternalState.LinesAfterFrameStart = 0;

	CRTC_InternalState.MA = GET_MA;
	CRTC_InternalState.MAStore = CRTC_InternalState.MA;

	CRTC_InternalState.LineCounter = 0;
	CRTC_InternalState.RasterCounter = 0;

	CRTC_SetFlag(CRTC_VDISP_FLAG);

//	if ((CRTC_InternalState.CRTC_Flags & CRTC_INTERLACE_ACTIVE)!=0)
//	{
//		CRTC_InternalState.RasterCounter = CRTC_InternalState.Frame;
//	}
//	else
//	{
		CRTC_InternalState.RasterCounter = 0;
//	}

	CRTC1_DoLineChecks();

	CRTC1_DoDispEnable();
}


void	CRTC1_MaxRasterMatch(void)
{
	if (CPC_GetCRTCType() != 1)
		return;

	if (CRTC_InternalState.RasterCounter == CRTCRegisters[9])
	{
		CRTC_SetFlag(CRTC_MR_FLAG);
	}
	else
	{
		CRTC_ClearFlag(CRTC_MR_FLAG);
	}

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


	/* check Vertical sync position */
	if (CRTC_InternalState.LineCounter == CRTCRegisters[7])
	{
		/* on CRTC type 1, Vsync can be triggered on any line of the char. */
		CRTC_InitVsync();
		/* clear flag at vsync time; to be confirmed */
		CRTC_ClearFlag(CRTC_INTERLACE_ACTIVE);
	}



#if 0
	CRTC_ClearFlag(CRTC_MR_FLAG);

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
	{
		if (CRTC_InternalState.RasterCounter==CRTCRegisters[9])
		{

			CRTC_SetFlag(CRTC_MR_FLAG);
		}
	}
#endif

}





/* on CRTC1, vsync of 40 only accepted if bc00,5 is 9!! */
/* check that if 39 is programmed with bc00,5==0 no vsync is generated */

/* executed for each complete line done by the CRTC */
void    CRTC1_DoLine(void)
{
	/* confirmed on type 1:
	VCC continues to count through vertical adjust.
	RC continues to count through vertical adjust
	see lines as normal; no repeat */

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





	/* are we in vertical adjust ? */
	if (CRTC_InternalState.CRTC_Flags & CRTC_VADJ_FLAG)
	{
		CRTC_InternalState.VertAdjustCount = (unsigned char)((CRTC_InternalState.VertAdjustCount+1) & 0x01f);

		/* vertical adjust matches counter? */
		if (CRTC_InternalState.VertAdjustCount==CRTCRegisters[5])
		{
			CRTC_ClearFlag(CRTC_VADJ_FLAG);
			CRTC1_RestartFrame();

		}
	}



	/* confirmed: on crtc type 1, MA can be changed on char line 0. The MA value is read at the start/end of the line */
	if (
		(CRTC_InternalState.LineCounter == 0) && 
	/* confirmed: on last line of char MA is not programmable at the end of the line
	the value at HDISP is stored and used */
		((CRTC_InternalState.CRTC_Flags & CRTC_MR_FLAG)==0)
		)
	{
		CRTC_InternalState.MAStore = GET_MA;
	}

	CRTC_InternalState.MA = CRTC_InternalState.MAStore;
	
	if (CRTC_InternalState.CRTC_Flags & CRTC_VTOT_FLAG)
	{
		CRTC_ClearFlag(CRTC_VTOT_FLAG);

		/* load vertical adjust */
		/*VertAdjust = CRTCRegisters[5]; */

		/* to be confirmed, when does frame get toggled? */
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
			CRTC1_RestartFrame();
		}
	}


	// if line only
	CRTC1_MaxRasterMatch();

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

	CRTC1_DoLineChecks();
}

int CRTC1_GetRAOutput(void)
{
	if ((CRTC_InternalState.CRTC_Flags & CRTC_INTERLACE_ACTIVE)!=0)
	{
		// doubles up
		return ((CRTC_InternalState.RasterCounter << 1) | CRTC_InternalState.Frame);
		// goes chunky
//		return ((CRTC_InternalState.RasterCounter>>1) | CRTC_InternalState.Frame);
		// works but wraps
	//	return (CRTC_InternalState.RasterCounter+ CRTC_InternalState.Frame);
	}
	return CRTC_InternalState.RasterCounter;
}

int CRTC1_GetHorizontalSyncWidth(void)
{
	/* confirmed: no hsync when 0 is programmed */
	return CRTCRegisters[3] & 0x0f;
}

int CRTC1_GetVerticalSyncWidth(void)
{
	return 16;
}


#if 0
/*---------------------------------------------------------------------------*/
void CRTC1_DoReg3()
{
	int HorizontalSyncWidth = CRTCRegisters[3] & 0x0f;
	/* confirmed: no hsync when 0 is programmed */
	CRTC_InternalState.HorizontalSyncWidth = HorizontalSyncWidth;
	CRTC_InternalState.VerticalSyncWidth = 16;
}
#endif

/*---------------------------------------------------------------------------*/

int CRTC1_ReadData(void)
{
	int   CRTC_RegIndex = (CRTC_InternalState.CRTC_Reg & 0x01f);
	
	/* confirmed: reading either lpen register will reset the status */
	if ((CRTC_RegIndex == 16) || (CRTC_RegIndex == 17))
	{
		/* reading LPEN registers on type 1, clears bit in status register */
		UM6845R_StatusRegister &=~CRTC_STATUS_LPEN_REGISTER_FULL;
	}

	/* unreadable registers return 0 */
	return (CRTCRegisters[CRTC_RegIndex] & UM6845R_ReadMaskTable[CRTC_RegIndex]);
}

/*---------------------------------------------------------------------------*/
void CRTC1_WriteData(int Data)
{
	int CRTC_RegIndex = CRTC_InternalState.CRTC_Reg & 0x1f;

	/* store registers using current CRTC information - masking out appropiate bits etc for this CRTC*/
	CRTCRegisters[CRTC_RegIndex] = (unsigned char)(Data & UM6845R_WriteMaskTable[CRTC_RegIndex]);

	//CRTC_LogRegisterWrite(CRTC_RegIndex, Data);

	CRTC1_UpdateState(CRTC_RegIndex);
	/* crtc type 1 has 0x0ff for reg 31 */
	//CRTCRegisters[ 31 ] = 0x0ff;
}

/*---------------------------------------------------------------------------*/


int CRTC1_ReadStatusRegister(void)
{
	int StatusRegister=UM6845R_StatusRegister;

	/* vertical blanking: confirmed this means VDISP */
	if ((CRTC_InternalState.CRTC_Flags & CRTC_VDISP_FLAG)==0)
	{
		StatusRegister|=CRTC_STATUS_IN_VERTICAL_BLANKING;
	}

	return StatusRegister;
}

/*---------------------------------------------------------------------------*/

