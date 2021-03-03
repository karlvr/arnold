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

/*---------------------------------------------------------------------------*/

#include "crtc.h"
#include "cpcglob.h"
#include "cpc.h"
#include "headers.h"
#include "monitor.h"

/*---------------------------------------------------------------------------*/

#define GET_MA  \
        (CRTCRegisters[12]<<8) | (CRTCRegisters[13])

#define CRTC_ClearFlag(x) \
	CRTC_InternalState.CRTC_Flags &=~x

#define CRTC_SetFlag(x)	\
	CRTC_InternalState.CRTC_Flags |=x


/*---------------------------------------------------------------------------*/

CRTC_INTERNAL_STATE CRTC_InternalState;


void 	CRTC_DoDispEnable(void)
{
  	switch (CRTC_InternalState.CRTC_Type)
	{
    case 1:
    {
      CRTC1_DoDispEnable();
    }
    break;
    
    default:
    {
	    /* disp enable is based on the output of HDISP, VDISP and R8 delay */
	    /* confirmed for type 3 */
    if ((CRTC_InternalState.CRTC_Flags & (CRTC_HDISP_FLAG | CRTC_VDISP_FLAG | CRTC_R8DT_FLAG))==(CRTC_HDISP_FLAG | CRTC_VDISP_FLAG))
    {
      CRTC_SetDispEnable(TRUE);
    }
    else
    {
      CRTC_SetDispEnable(FALSE);
    }
  }
  break;
}
}

/*---------------------------------------------------------------------------*/

CRTC_INTERNAL_STATE *CRTC_GetInternalState(void)
{
	return &CRTC_InternalState;
}

/*---------------------------------------------------------------------------*/


/* CRTC internal register file */
/* Registers 0-18 are used, 19-31 are not */
unsigned char            CRTCRegisters[32];
unsigned char            CRTCRegistersBackup[32];

int CRTC_GetLinesAfterFrameStart(void)
{
	return CRTC_InternalState.LinesAfterFrameStart;
}

int CRTC_GetLinesAfterVsyncStart(void)
{
	return CRTC_InternalState.LinesAfterVsyncStart;
}

int CRTC_GetCharsAfterHsyncStart(void)
{
	return CRTC_InternalState.CharsAfterHsyncStart;
}


/*---------------------------------------------------------------------------*/
int CRTC_GetInterlaceFrame(void)
{
	return CRTC_InternalState.Frame & 0x01;
}

/* set HStart and HEnd */
void CRTC_DoReg1(void)
{
	switch (CRTC_InternalState.CRTC_Type)
	{
	case 0:
	{
		CRTC0_DoReg1();
	}
	break;

	case 1:
	{
		CRTC1_DoReg1();
	}
	break;

	case 2:
	{
		CRTC2_DoReg1();
	}
	break;

	case 3:
	{
		CRTC3_DoReg1();
	}
	break;

	case 4:
	{
		CRTC4_DoReg1();
	}
	break;

	case 5:
	{
		CRTC1B_DoReg1();
	}
	break;


	}

}

/*---------------------------------------------------------------------------*/
void     CRTC_DoReg8(void)
{
	switch (CRTC_InternalState.CRTC_Type)
	{
	case 0:
	{
		CRTC0_DoReg8();
	}
	break;

	case 1:
	{
		CRTC1_DoReg8();
	}
	break;

	case 2:
	{
		CRTC2_DoReg8();
	}
	break;

	case 3:
	{
		CRTC3_DoReg8();
	}
	break;

	case 4:
	{
		CRTC4_DoReg8();
	}
	break;

	case 5:
	{
		CRTC1B_DoReg8();
	}
	break;


	}
}
#if 0
/*---------------------------------------------------------------------------*/
void CRTC_DoReg3(void)
{
	switch (CRTC_InternalState.CRTC_Type)
	{
		case 0:
		{
			CRTC0_DoReg3();
		}
		break;

		case 1:
		{
			CRTC1_DoReg3();
		}
		break;

		case 2:
		{
			CRTC2_DoReg3();
		}
		break;

		case 3:
		{
			CRTC3_DoReg3();
		}
		break;

		case 4:
		{
			CRTC4_DoReg3();
		}
		break;

		case 5:
		{
			CRTC1B_DoReg3();
		}
		break;


	}
}
#endif

/*---------------------------------------------------------------------------*/


void    CRTC_SetType(unsigned int Type)
{
	CRTC_InternalState.CRTC_Type = (unsigned char)Type;


	/* do these registers now, if these have been setup, but not changed,
	we need to re-do them depending on the CRTC chosen, otherwise we will
	not get the correct display !!! */
	/*     CRTC_DoReg8();
	      CRTC_DoReg3();
*/
}

/*------------------------------------------------------------------------------------------------------*/

void    CRTC_RegisterSelect(unsigned int RegisterIndex)
{
	CRTC_InternalState.CRTC_Reg = (unsigned char)RegisterIndex;
}

/*------------------------------------------------------------------------------------------------------*/
int             CRTC_GetSelectedRegister(void)
{
	return CRTC_InternalState.CRTC_Reg;
}

/*------------------------------------------------------------------------------------------------------*/
unsigned char   CRTC_GetRegisterData(int RegisterIndex)
{
	return (unsigned char)CRTCRegisters[RegisterIndex & 0x01f];
}

/*------------------------------------------------------------------------------------------------------*/

unsigned int            CRTC_ReadStatusRegister(void)
{
	switch (CRTC_InternalState.CRTC_Type)
	{
		case 0:
			return CRTC0_ReadStatusRegister();

		case 2:
			return CRTC2_ReadStatusRegister();

		case 1:
			return CRTC1_ReadStatusRegister();

		case 3:
			return CRTC3_ReadStatusRegister();

		case 4:
			return CRTC4_ReadStatusRegister();

		case 5:
			return CRTC1B_ReadStatusRegister();


		default
				:
			break;
	}
	return 0x0ff;
}

/*------------------------------------------------------------------------------------------------------*/

unsigned char CRTC_ReadData(void)
{
	switch (CRTC_InternalState.CRTC_Type)
	{
		case 0:
			return CRTC0_ReadData();
		case 1:
			return CRTC1_ReadData();
		case 2:
			return CRTC2_ReadData();
		case 3:
			return CRTC3_ReadData();
		case 4:
			return CRTC4_ReadData();
		case 5:
			return CRTC1B_ReadData();
	}

	/* or return databus?? */
	return 0x0ff;
}

/*------------------------------------------------------------------------------------------------------*/



void    CRTC_WriteData(unsigned int Data)
{
	/* to allow switching crtcs */
	int CRTC_RegIndex = CRTC_InternalState.CRTC_Reg & 0x1f;
	CRTCRegistersBackup[CRTC_RegIndex] = (unsigned char)Data;

	/* now do CRTC specific writes */
	switch (CRTC_InternalState.CRTC_Type)
	{
		case 0:
		{
			CRTC0_WriteData(Data);
		}
		break;

		case 1:
		{
			CRTC1_WriteData(Data);
		}
		break;

		case 2:
		{
			CRTC2_WriteData(Data);
		}
		break;

		case 3:
		{
			CRTC3_WriteData(Data);
		}
		break;

		case 4:
		{
			CRTC4_WriteData(Data);
		}
		break;

		case 5:
		{
			CRTC1B_WriteData(Data);
		}
		break;

	}
}

/*------------------------------------------------------------------------------*/

void     CRTC_InitVsync(void)
{
	CRTC_InternalState.LinesAfterVsyncStart = 0;

	if (!(CRTC_InternalState.CRTC_Flags & CRTC_VSCNT_FLAG))
	{

		CRTC_InternalState.VerticalSyncCount = 0;
		switch (CRTC_InternalState.CRTC_Type)
		{
		case 0:
		{
			CRTC_InternalState.VerticalSyncWidth = CRTC0_GetVerticalSyncWidth();
		}
		break;

		case 1:
		{
			CRTC_InternalState.VerticalSyncWidth = CRTC1_GetVerticalSyncWidth();
		}
		break;

		case 2:
		{
			CRTC_InternalState.VerticalSyncWidth = CRTC2_GetVerticalSyncWidth();
		}
		break;

		case 3:
		{
			CRTC_InternalState.VerticalSyncWidth = CRTC3_GetVerticalSyncWidth();
		}
		break;

		case 4:
		{
			CRTC_InternalState.VerticalSyncWidth = CRTC4_GetVerticalSyncWidth();
		}
		break;

		case 5:
		{
			CRTC_InternalState.VerticalSyncWidth = CRTC1B_GetVerticalSyncWidth();
		}
		break;

		}
		CRTC_SetFlag(CRTC_VSCNT_FLAG);
		CRTC_SetVsyncOutput(TRUE);
	}

#if 0
	/* it has not been triggered.. */
	if (!(CRTC_InternalState.CRTC_Flags & CRTC_VSYNC_TRIGGERED_FLAG))
	{
		/* re-set counter only if it is active */
		if (!(CRTC_InternalState.CRTC_Flags & CRTC_VSCNT_FLAG))
		{
			/* counter is not active! */

			CRTC_SetFlag(CRTC_VSYNC_TRIGGERED_FLAG);

			/* reset vertical sync count */
			CRTC_InternalState.VerticalSyncCount=0;

			/* enable counter */
			CRTC_SetFlag(CRTC_VSCNT_FLAG);


			CRTC_InterlaceControl_VsyncStart();


		}
	}
#endif


}


void CRTC_DoVerticalSyncCounter(void)
{
	/* are we counting vertical syncs? */
	if (CRTC_InternalState.CRTC_Flags & CRTC_VSCNT_FLAG)
	{
		/* update vertical sync counter */
		CRTC_InternalState.VerticalSyncCount++;

		/* if vertical sync count = vertical sync width then
		 stop vertical sync */
		if (CRTC_InternalState.VerticalSyncCount>=CRTC_InternalState.VerticalSyncWidth)
		{
			/* clear counter */
			CRTC_InternalState.VerticalSyncCount=0;

			/* clear counter */
			CRTC_ClearFlag(CRTC_VSCNT_FLAG);
		}
	}
}





void CRTC_InterlaceControl_SetupDelayedVsync(void)
{
	/* don't set VSYNC */

	/* set VSYNC at next HTOT/2 */
	CRTC_InternalState.CRTC_HalfHtotFlags = CRTC_VS_FLAG;
}

void CRTC_InterlaceControl_FinishDelayedVsync(void)
{
	/* don't clear VSYNC now, clear VSYNC at next HTOT/2 */
	CRTC_InternalState.CRTC_HalfHtotFlags = 0;
}

/* setup a VSYNC to start at the beginning of the line */
void CRTC_InterlaceControl_SetupStandardVsync(void)
{
	/* set VSYNC immediatly */
	CRTC_SetFlag(CRTC_VS_FLAG);

	/* keep VSYNC set at HTOT/2 */
	CRTC_InternalState.CRTC_HalfHtotFlags = CRTC_VS_FLAG;

	CRTC_SetVsyncOutput(TRUE);

}

void CRTC_InterlaceControl_FinishStandardVsync(void)
{
	/* clear vsync */
	CRTC_ClearFlag(CRTC_VS_FLAG);
	/* no VSYNC on next HTOT/2 */
	CRTC_InternalState.CRTC_HalfHtotFlags = 0;
	CRTC_SetVsyncOutput(FALSE);
}

/* call when VSYNC has begun */
void CRTC_InterlaceControl_VsyncStart(void)
{
//	if ((CRTC_InternalState.InterlaceAndVideoMode & 1)==0)
	{
		/* no interlace */
		CRTC_InterlaceControl_SetupStandardVsync();
	}
#if 0
	else
	{
		/* interlace */

		if (CRTC_InternalState.Frame!=0)
		{
			CRTC_InterlaceControl_SetupStandardVsync();
		}
		else
		{
			CRTC_InterlaceControl_SetupDelayedVsync();
		}
	}
#endif

}

void CRTC_InterlaceControl_VsyncEnd(void)
{
//	if ((CRTC_InternalState.InterlaceAndVideoMode & 1)==0)
	{
		/* no interlace */
		CRTC_InterlaceControl_FinishStandardVsync();
	}
#if 0
	else
	{
		/* interlace */

		if (CRTC_InternalState.Frame!=0)
		{
			CRTC_InterlaceControl_FinishStandardVsync();
		}
		else
		{
			CRTC_InterlaceControl_FinishDelayedVsync();
		}
	}
#endif
}

void CRTC_SetLPenInput(BOOL bState)
{


}


BOOL CRTCReady(void)
{
	if ((CRTCRegisters[12] == 0) || (CRTCRegisters[0] == 0)) return FALSE;
	return TRUE;
}

FILE *fh = NULL;
int PreviousNopCount = 0;

void CRTC_LogInit(void)
{
//	fh = fopen("c:\\users\\kev\\desktop\\crtc_log.txt", "w");
}

void CRTC_LogFinish(void)
{
	if (fh != NULL)
	{
		fclose(fh);
		fh = NULL;
	}
}


void CRTC_LogRegisterWrite(int Reg, int Data)
{
	int i;
	int NopDelta = CPC_GetNopCount()-PreviousNopCount;
	PreviousNopCount = CPC_GetNopCount();

	if (fh)
	{
		fprintf(fh, "-------------\n");
		fprintf(fh, "Delta: %d Reg: %02x Data: %02x :\n ", NopDelta, Reg, Data);
		for (i = 0; i < 16; i++)
		{
			fprintf(fh, "\t%02x : %02x\n", i, CRTCRegisters[i]);
		}
		fprintf(fh, "HCC: %02x VCC: %02x RCC: %02x\n", CRTC_InternalState.HCount, CRTC_InternalState.LineCounter, CRTC_InternalState.RasterCounter);
		fprintf(fh, "--------------\n");
		fflush(fh);
	}
	
}


/* executed each NOP cycle performed by the Z80 */
void CRTC_DoCycles(int Cycles)
{
	int i;
	int PreviousCursorOutput;


	/* update light pen */
/*	CRTC_LightPen_Update(Cycles); */

	/* hack
	//wait for CRTC ready
	if (!CRTCReady()) return;
	*/

	for (i=Cycles-1; i>=0; i--)
	{
		CRTC_InternalState.CharsAfterHsyncStart++;
		/* increment horizontal count */
		CRTC_InternalState.HCount=(unsigned char)((CRTC_InternalState.HCount+1) & 0x0ff);
		CRTC_InternalState.MA = (CRTC_InternalState.MA+1) & 0x03fff;

		// method 2
		CRTC1_MaxRasterMatch();
		CRTC2_MaxRasterMatch();

		if (CRTC_InternalState.CRTC_Flags & CRTC_HTOT_FLAG)
		{
			unsigned long PreviousFlags = CRTC_InternalState.CRTC_Flags;
			CRTC_ClearFlag(CRTC_HTOT_FLAG);

			/* zero count */
			CRTC_InternalState.HCount = 0;
			CRTC_InternalState.LinesAfterFrameStart++;
			CRTC_InternalState.LinesAfterVsyncStart++;
			switch (CRTC_InternalState.CRTC_Type)
			{
				case 0:
				{
					CRTC0_DoLine();
				}
				break;

				case 1:
				{
					CRTC1_DoLine();
				}
				break;

				case 2:
				{
					CRTC2_DoLine();
				}
				break;

				case 3:
				{
					ASICCRTC_DoLine();
				}
				break;

				case 4:
				{
					CRTC4_DoLine();
				}
				break;

				case 5:
				{
					CRTC1B_DoLine();
				}
				break;

			}
			if (((PreviousFlags^CRTC_InternalState.CRTC_Flags) & CRTC_VSCNT_FLAG)!=0)
			{
				/* vsync counter bit has changed state */
				if (CRTC_InternalState.CRTC_Flags & CRTC_VSCNT_FLAG)
				{
					/* change from vsync counter inactive to active */
					CRTC_InterlaceControl_VsyncStart();
				}
				else
				{
					/* change from counter active to inactive */
					CRTC_InterlaceControl_VsyncEnd();
				}
			}


			CRTC_InternalState.CRTC_FlagsAtLastHtot = CRTC_InternalState.CRTC_Flags;

		}


		/* does horizontal equal Htot? */
		if (CRTC_InternalState.HCount == CRTCRegisters[0])
		{
			CRTC_SetFlag(CRTC_HTOT_FLAG);
		}

		if (CRTC_InternalState.HCount == (CRTCRegisters[0]>>1))
		{
			unsigned long Flags;

			/* get flags */
			Flags = CRTC_InternalState.CRTC_Flags;
			/* clear VSYNC flag */
			Flags &= ~CRTC_VS_FLAG;
			/* set/clear VSYNC flag */
			Flags |= CRTC_InternalState.CRTC_HalfHtotFlags;
			/* store new flags */
			CRTC_InternalState.CRTC_Flags = Flags;
		}

		/* Horizontal Sync Width Counter */
		/* are we counting horizontal syncs? */
		if (CRTC_InternalState.CRTC_Flags & CRTC_HS_FLAG)
		{
			CRTC_InternalState.HorizontalSyncCount++;
			/* if horizontal sync count = Horizontal Sync Width then
			 stop horizontal sync */
			if (CRTC_InternalState.HorizontalSyncCount==CRTC_InternalState.HorizontalSyncWidth)
			{
				CRTC_InternalState.HorizontalSyncCount=0;

				/* stop horizontal sync counter */
				CRTC_ClearFlag(CRTC_HS_FLAG);


				/* call functions that would happen on a HSYNC */
				CRTC_SetHsyncOutput(FALSE);
			}
		}

		/* does current horizontal count equal position to start horizontal sync? */
		if (CRTC_InternalState.HCount == CRTCRegisters[2])
		{
			CRTC_InternalState.CharsAfterHsyncStart=0;
			switch (CRTC_InternalState.CRTC_Type)
			{
			case 0:
			{
				CRTC_InternalState.HorizontalSyncWidth = CRTC0_GetHorizontalSyncWidth();
			}
			break;

			case 1:
			{
				CRTC_InternalState.HorizontalSyncWidth = CRTC1_GetHorizontalSyncWidth();
			}
			break;

			case 2:
			{
				CRTC_InternalState.HorizontalSyncWidth = CRTC2_GetHorizontalSyncWidth();
			}
			break;

			case 3:
			{
				CRTC_InternalState.HorizontalSyncWidth = CRTC3_GetHorizontalSyncWidth();
			}
			break;

			case 4:
			{
				CRTC_InternalState.HorizontalSyncWidth = CRTC4_GetHorizontalSyncWidth();
			}
			break;

			case 5:
			{
				CRTC_InternalState.HorizontalSyncWidth = CRTC1B_GetHorizontalSyncWidth();
			}
			break;

			}
			/* if horizontal sync = 0, in the HD6845S no horizontal
			sync is generated. The input to the flip-flop is 1 from
			both Horizontal Sync Position and HorizontalSyncWidth, and
			the HSYNC is not even started */
			if (CRTC_InternalState.HorizontalSyncWidth!=0)
			{
				/* are we already in a HSYNC? */
				if (!(CRTC_InternalState.CRTC_Flags & CRTC_HS_FLAG))
				{
					/* no.. */

					/* enable horizontal sync counter */
					CRTC_SetFlag(CRTC_HS_FLAG);

					CRTC_SetHsyncOutput(TRUE);

					/* initialise counter */
					CRTC_InternalState.HorizontalSyncCount = 0;

				}
			}
		}

		/* confirmed: on type 3, border is turned off at HStart */
		if (CRTC_InternalState.HCount == CRTC_InternalState.HStart)
		{
			/* enable horizontal display */
			CRTC_SetFlag(CRTC_HDISP_FLAG);
			CRTC_DoDispEnable();

		}

		/* confirmed: on type 3, border is turned on at HEnd. */
		if (CRTC_InternalState.HCount == CRTC_InternalState.HEnd)
		{
			CRTC_ClearFlag(CRTC_HDISP_FLAG);
			CRTC_DoDispEnable();
		}

		/* confirmed: on type 3, hdisp is triggered from R1 because I don't see the screen distort which would happen
		if it's at HEnd */
		if (CRTC_InternalState.HCount == CRTCRegisters[1])
		{

			switch (CRTC_InternalState.CRTC_Type)
			{
				case 0:
				{
					CRTC0_DoHDisp();
				}
				break;

				case 1:
				{
					CRTC1_DoHDisp();
				}
				break;

				case 2:
				{
					CRTC2_DoHDisp();
				}
				break;

				case 3:
				{
					CRTC3_DoHDisp();
				}
				break;

				case 4:
				{
					CRTC4_DoHDisp();
				}
				break;

				case 5:
				{
					CRTC1B_DoHDisp();
				}
				break;

			}
		}
		
		if (CRTC_InternalState.RasterCounter==(CRTCRegisters[10]&0x01f))
		{
			CRTC_SetFlag(CRTC_CURSOR_LINE_ACTIVE);
		}
		
		if (CRTC_InternalState.RasterCounter==(CRTCRegisters[11]&0x01f))
		{
			CRTC_ClearFlag(CRTC_CURSOR_LINE_ACTIVE);
		}
		
		PreviousCursorOutput = CRTC_InternalState.CursorOutput;
		
		CRTC_InternalState.CursorOutput = 0;
		if (
			(CRTC_InternalState.CursorMA == CRTC_InternalState.MA) && 
			((CRTC_InternalState.CRTC_Flags & (CRTC_CURSOR_LINE_ACTIVE|CRTC_CURSOR_ACTIVE))==(CRTC_CURSOR_LINE_ACTIVE|CRTC_CURSOR_ACTIVE))
			)
		{
			CRTC_InternalState.CursorOutput = 1;
		}
		if (PreviousCursorOutput!=CRTC_InternalState.CursorOutput)
		{
			CRTC_DoCursorOutput(CRTC_InternalState.CursorOutput);
		}

		Graphics_Update();

	}
}

int CRTC_GetCursorOutput(void)
{
    return CRTC_InternalState.CursorOutput;
}

int CRTC_GetVsyncOutput(void)
{
	return CRTC_InternalState.CRTC_Flags & CRTC_VS_FLAG;
}

int CRTC_GetActualHorizontalSyncWidth(void)
{
	switch (CRTC_InternalState.CRTC_Type)
	{
	case 0:
	{
		return CRTC0_GetHorizontalSyncWidth();
	}
	break;

	case 1:
	{
		return CRTC1_GetHorizontalSyncWidth();
	}
	break;

	case 2:
	{
		return CRTC2_GetHorizontalSyncWidth();
	}
	break;

	case 3:
	{
		return CRTC3_GetHorizontalSyncWidth();
	}
	break;

	case 4:
	{
		return CRTC4_GetHorizontalSyncWidth();
	}
	break;

	case 5:
	{
		return CRTC1B_GetHorizontalSyncWidth();
	}
	break;

	}
	return -1;
}

int CRTC_GetActualVerticalSyncWidth(void)
{
	switch (CRTC_InternalState.CRTC_Type)
	{
	case 0:
	{
		return CRTC0_GetVerticalSyncWidth();
	}
	break;

	case 1:
	{
		return CRTC1_GetVerticalSyncWidth();
	}
	break;

	case 2:
	{
		return CRTC2_GetVerticalSyncWidth();
	}
	break;

	case 3:
	{
		return CRTC3_GetVerticalSyncWidth();
	}
	break;

	case 4:
	{
		return CRTC4_GetVerticalSyncWidth();
	}
	break;

	case 5:
	{
		return CRTC1B_GetVerticalSyncWidth();
	}
	break;

	}
	return -1;

}

int CRTC_GetHsyncCounter(void)
{
	return CRTC_InternalState.HorizontalSyncCount;
}

int CRTC_GetVsyncCounter(void)
{
	return CRTC_InternalState.VerticalSyncCount;
}

int CRTC_GetHsyncOutput(void)
{
	return CRTC_InternalState.CRTC_Flags & CRTC_HS_FLAG;
}

int CRTC_GetDispTmgOutput(void)
{
	return CRTC_InternalState.CRTC_Flags & (CRTC_HDISP_FLAG|CRTC_VDISP_FLAG);
}

int CRTC_GetRAOutput(void)
{
	/* this is slow */
 switch (CRTC_InternalState.CRTC_Type)
	{
	 case 0:
		 return CRTC0_GetRAOutput() & 0x07;
	 case 1:
		 return CRTC1_GetRAOutput() & 0x07;
	 case 2:
		 return CRTC2_GetRAOutput() & 0x07;

		case 3:
        return CRTC3_GetRAOutput()&0x07;
		case 4:
			return CRTC4_GetRAOutput() & 0x07;
		case 5:
			return CRTC1B_GetRAOutput() & 0x07;
		default:
      break;
  }
 return 0;
}

int CRTC_GetMAOutput(void)
{
	return CRTC_InternalState.MA;
}

int CRTC_GetVCC(void)
{
	return CRTC_InternalState.LineCounter;
}

int CRTC_GetHCC(void)
{
	return CRTC_InternalState.HCount;
}


int CRTC_GetRA(void)
{
	return CRTC_InternalState.RasterCounter;
}

void    CRTC_Reset(void)
{

	CRTC_InternalState.LinesAfterFrameStart = 0;
	CRTC_InternalState.LinesAfterVsyncStart = 0;
	CRTC_InternalState.CharsAfterHsyncStart = 0;

	switch (CRTC_InternalState.CRTC_Type)
	{
		case 0:
		{
			CRTC0_Reset();
		}
		break;

		case 1:
		{
			CRTC1_Reset();
		}
		break;

		case 2:
		{
			CRTC2_Reset();
		}
		break;

		case 3:
		{
			CRTC3_Reset();
		}
		break;

		case 4:
		{
			CRTC4_Reset();
		}
		break;

		case 5:
		{
			CRTC1B_Reset();
		}
		break;
	}
}


unsigned long ClocksToLightPenTrigger;
unsigned long LightPen_Strobe = 0;
#if 0
/* x pos in pixel coords, y pos in pixel coords */
void	CRTC_LightPen_Trigger(int XPos, int YPos)
{
	unsigned long NopsInX, NopsInY;
	/* x pos is in mode 2 pixels
	 y pos is in mode 2 pixels

	 x,y pos are in visible display area

	 in mode 2, there are 16 pixels per nop.
	 each nop is 2 bytes

	 not correct.
	*/
	/* us per x position */
	NopsInX = (XPos>>4);
	/* 64 us per line */
	NopsInY = (YPos<<6);

	ClocksToLightPenTrigger = NopsInY + NopsInX;


	LightPen_Strobe = 1;

}

void	CRTC_LightPen_Trigger(int X, int Y)
{
	unsigned long Nops;
	unsigned long MonitorLine;
	unsigned long LinesToTrigger;
	unsigned long MonitorX;
	unsigned long CharsToTrigger;

	/* get current monitor line we are on. */
	MonitorLine = CRTC_InternalState.Monitor_State.MonitorScanLineCount;

	/* work out number of complete lines until the trigger should occur */
	if (Y>MonitorLine)
	{
		LinesToTrigger = Y - MonitorLine;
	}
	else
	{
		LinesToTrigger = (312 - MonitorLine) + Y;
	}

	Nops = (LinesToTrigger<<6);

	X = CRTC_InternalState.Monitor_State.MonitorHorizontalCount;

	X = (X>>(4-1)) + X_CRTC_CHAR_OFFSET;

	if (X>MonitorX)
	{
		CharsToTrigger = X - MonitorX;
	}
	else
	{
		CharsToTrigger = (64 - MonitorX) + X;
	}

	Nops += (CharsToTrigger>>1);


	/*		unsigned long Nops;

			Nops = (YPos<<6) + (XPos>>5) + ((8*10)*64);
	*/
/*		CRTC_LightPen_Trigger(Nops); */



	LightPen_Strobe = 1;

	ClocksToLightPenTrigger = Nops;
}

void	CRTC_LightPen_Update(unsigned long NopsPassed)
{
	if (LightPen_Strobe)
	{
		if (NopsPassed>=ClocksToLightPenTrigger)
		{
			/* signal that light pen register is full */
			UM6845R_StatusRegister |=0x01;

			/* grab current MA */
			CRTC_LightPenMA = CRTC_InternalState.MALine + CRTC_InternalState.HCount;

			CRTC_LightPenMA &= ~0x0c000;

			/* re-write Light Pen data (read-only!) */
			CRTCRegisters[16] = (CRTC_LightPenMA>>8) & 0x0ff;
			CRTCRegisters[17] = CRTC_LightPenMA & 0x0ff;


			LightPen_Strobe = 0;
		}

		ClocksToLightPenTrigger -= NopsPassed;

	}
}
#endif
