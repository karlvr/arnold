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

#include "ddi.h"

void    ASIC_DMA_ExecuteCommand(int ChannelIndex);
void ASIC_ClearPRIInterruptRequest(void);

#include "headers.h"

#include "asic.h"
#include "render.h"
#include "cpcglob.h"
#include "monitor.h"
#include "cpc.h"
#include "riff.h"
#include "psg.h"
#include "crtc.h"
#include "printer.h"
#include "fdc.h"
#include "debugger/breakpt.h"
#include <math.h>
#include "render.h"

static unsigned char *ASIC_ExtraRam = NULL;

void ASIC_Pal_Reset(void);
void ASIC_ROM_Select(Z80_BYTE);
void ASIC_RefreshRasterInterruptState(void);
void ASIC_RefreshRasterInterrupt(int, int);
void ASICCRTC_SetSoftScroll(int SoftScroll);
static ASIC_GATE_ARRAY_STATE	GateArray_State;

static ASIC_DATA ASIC_Data;

void ASIC_UpdateInternalDCSR(void);


extern AY_3_8912 OnBoardAY;

static BOOL ASIC_R128 = FALSE;
static BOOL ASIC_R129 = FALSE;
static BOOL ASIC_R130 = FALSE;


static int SpriteOverride = 0x0ffff;
static int ScrollOverride = 0x0ff;
static BOOL ScreenSplitEnableOverride = TRUE;
static BOOL PRIInterruptOverride = TRUE;

void ASIC_SetHorizontalScrollOverride(BOOL bState)
{
	if (bState)
	{
		ScrollOverride |= 0x0f;
	}
	else
	{
		ScrollOverride &= ~0x0f;
	}
}

BOOL ASIC_GetHorizontalScrollOverride(void)
{
	return ((ScrollOverride&0x0f)!=0);
}

void ASIC_SetVerticalScrollOverride(BOOL bState)
{
	if (bState)
	{
		ScrollOverride |= (0x07<<4);
	}
	else
	{
		ScrollOverride &= ~(0x07<<4);
	}
}

BOOL ASIC_GetVerticalScrollOverride(void)
{
	return ((ScrollOverride & (0x07<<4)) != 0);
}

void ASIC_SetScrollBorderOverride(BOOL bState)
{
	if (bState)
	{
		ScrollOverride |= (1<<7);
	}
	else
	{
		ScrollOverride &= ~(1<<7);
	}
}

BOOL ASIC_GetScrollBorderOverride(void)
{
	return ((ScrollOverride & (1<<7)) != 0);
}


void ASIC_SetScreenSplitOverride(BOOL bState)
{
	ScreenSplitEnableOverride = bState;
}

BOOL ASIC_GetScreenSplitOverride(void)
{
	return ScreenSplitEnableOverride;
}


void ASIC_SetPRIInterruptOverride(BOOL bState)
{
	PRIInterruptOverride = bState;
}

BOOL ASIC_GetPRIInterruptOverride(void)
{
	return PRIInterruptOverride;
}


void ASIC_SetSpriteOverride(int nOverride)
{
	SpriteOverride = nOverride;
}

int ASIC_GetSpriteOverride(void)
{
	return SpriteOverride;
}


/* 128k dram */
void ASIC_SetR128(BOOL bState)
{
	ASIC_R128 = bState;
}

/* disc vers */
void ASIC_SetR129(BOOL bState)
{
	ASIC_R129 = bState;

}

/* 8-bit printer */
void ASIC_SetR130(BOOL bState)
{
	ASIC_R130 = bState;
}


#define ASIC_PPI_CONTROL_BYTE_FUNCTION               0x080
#define ASIC_PPI_CONTROL_PORT_A_STATUS               0x010

typedef struct
{
	/* latched data written to outputs */
	unsigned char latched_outputs[4];

	unsigned char final_outputs[4];

	/* current inputs */
	unsigned char	inputs[4];

	/* masks for input/output. 0x0ff = keep input, 0x00 = keep output */
	unsigned char	io_mask[4];
	/* control information */
	unsigned char	control;
} ASIC_PPI;

ASIC_PPI asic_ppi;

void    ASIC_PPI_Reset(void)
{
	/* confirmed */
	/* at reset time */
	/* port A is input */
	/* port B is input */
	/* port C is output */

	/* 1--10-10 */
	ASIC_PPI_WriteControl(0x092);
}


 static int asic_ppi_read_port(int port_index)
{
	return ((asic_ppi.inputs[port_index] & asic_ppi.io_mask[port_index]) |
		(asic_ppi.latched_outputs[port_index] & (~asic_ppi.io_mask[port_index])));
}

 static void asic_ppi_write_port(int port_index, int data)
{
	asic_ppi.latched_outputs[port_index] = data;

	asic_ppi.final_outputs[port_index] = ((asic_ppi.latched_outputs[port_index] & (~asic_ppi.io_mask[port_index])) |
		(0x0ff & asic_ppi.io_mask[port_index]));
}

unsigned int ASIC_PPI_ReadPort(int nPort)
{
	asic_ppi.inputs[nPort] = ASIC_PPI_GetPortInput(nPort);

	return asic_ppi_read_port(nPort);
}


int ASIC_PPI_GetOutputPort(int nPort)
{
	return asic_ppi.final_outputs[nPort];
}


void    ASIC_PPI_WritePort(int nPort, int Data)
{
	asic_ppi_write_port(nPort, Data);

	if (
		/* writing to port A */
		(nPort == 0) &&
		/* select register */
		((asic_ppi.final_outputs[2] & 0x0c0) == 0x0c0)
		)
	{
		ASIC_Data.CurrentAyRegister = Data;
	}

	if (nPort == 2)
	{
		if ((Data & 0x0c0) == 0x0c0)
		{
			ASIC_Data.CurrentAyRegister = asic_ppi.final_outputs[0];
		}

		ASIC_Data.Current8255Select = Data;
	}

	ASIC_PPI_SetPortOutput(nPort, asic_ppi.final_outputs[nPort]);
}


int ASIC_PPI_GetControlForSnapshot(void)
{
	return asic_ppi.control;
}

void    ASIC_PPI_SetPortDataFromSnapshot(int nPort, int Data)
{
	asic_ppi.inputs[nPort] = Data;
}

int ASIC_PPI_GetPortDataForSnapshot(int nPort)
{
	return asic_ppi.inputs[nPort];
}


int ASIC_PPI_ReadControl(void)
{
	/* for CPC+, this is the result of reading PPI Control port, value
	returned is based on value last written. */

	/* in ranges:

		0x080-0x08f, 0x0a0-0x0af,0x0c0-0x0cf,0x0e0-0x0ef?
		*/
	if ((asic_ppi.control & 0x090) == 0x080)
	{
		/* yes */
		return 0x0;
	}

	/* TODO: Check */
	return 0x0ff;
}



void    ASIC_PPI_WriteControl(int Data)
{
	if (Data & ASIC_PPI_CONTROL_BYTE_FUNCTION)
	{
		/* Configuration control byte */
		/* CPC type is CPC+ */

		/* on CPC+ port B is always in input mode and
		port C is always in output mode */
		asic_ppi.io_mask[1] = 0x0ff;
		asic_ppi.io_mask[2] = 0x000;

		asic_ppi.control = Data;

		/* on CPC+ port A can be programmed as input or output */
		if (Data & ASIC_PPI_CONTROL_PORT_A_STATUS)
		{
			/* port A is input */
			asic_ppi.io_mask[0] = 0x0ff;
		}
		else
		{
			/* port A is output */
			asic_ppi.io_mask[0] = 0x000;
		}
		asic_ppi_write_port(0, asic_ppi.latched_outputs[0]);
		ASIC_PPI_SetPortOutput(0, asic_ppi.final_outputs[0]);
	}
	else
	{
		/* Bit Set/Reset control bit */

		int     BitIndex = (Data >> 1) & 0x07;

		if (Data & 1)
		{
			/* set bit */

			int     OrData;

			OrData = (1 << BitIndex);

			asic_ppi.latched_outputs[2] |= OrData;
		}
		else
		{
			/* clear bit */

			int     AndData;

			AndData = (~(1 << BitIndex));

			asic_ppi.latched_outputs[2] &= AndData;
		}
		asic_ppi_write_port(2, asic_ppi.latched_outputs[2]);
		ASIC_PPI_SetPortOutput(2, asic_ppi.final_outputs[2]);
	}
}

int ASIC_GetSelectedCartridgePage(void)
{
	return ASIC_Data.ASIC_Cartridge_Page;
}

/* really the value depends on what was on the bus last, with bit 0 changing */
#define ASIC_UNUSED_RAM_DATA 0x0b0


/* for colour/greyscale monitor support */
static RGBCOLOUR ASIC_DisplayColours[4096];
/* source luminances */
static int Luminances[4096];

static int CurrentLine;


void ASIC_WriteRAMIfEnabled(Z80_WORD Addr, Z80_BYTE Data)
{
	/* asic ram enabled? */
	if (!ASIC_Data.ASICRamEnabled)
		return;

	if ((Addr & 0x0c000) != 0x04000)
	{
		return;
	}

	/* execute function which determines if writes
	to ASIC ram have been performed */
	ASIC_WriteRamFull(Addr, Data);
}



/* byte mask, which has bits set according to the lines occupied by this sprite */

/* cartridge data */
/*static  unsigned char   *pCartridge=NULL; */

extern unsigned char *Z80MemoryBase;
static unsigned char *pVRAMAddr;
static unsigned int VRAMAddr;
static unsigned long PixelData;


extern unsigned long ASICCRTC_Line;

void ASIC_CalcVRAMAddr(void)
{
	int LocalMA = CRTC_GetMAOutput() << 1;

	int Addr = ((LocalMA & 0x06000) << 1) | (LocalMA & 0x07ff);
	/* MA0-MA9, MA12, MA13 used */
	/* RA0-RA7 used */
	Addr |= (CRTC_GetRAOutput() << 11);

	VRAMAddr = Addr;

	pVRAMAddr = &Z80MemoryBase[VRAMAddr];
}

int ASIC_GetVRAMAddr(void)
{
	return VRAMAddr;
}

void ASIC_CachePixelData(void)
{
	PixelData = PixelData << 16;
	PixelData &= 0x0ffff0000;
	PixelData |= (pVRAMAddr[0] << 8) | (pVRAMAddr[1]);
}

unsigned long ASIC_GetPixelData(void)
{
	/* test clearly shows:

	1. ASIC fetches 2 bytes of data, it has a 4 byte store it seems necessary for doing
	pixel scroll in mode 2.
	2. Scrolling by mode 2 sized pixels works. So the pixels are decoded THEN shifted
	at mode 2 rate.
	*/
	
	if (!ASIC_GetHorizontalScrollOverride())
		return PixelData;
	
	return PixelData;// >> (ASIC_Data.ASIC_SoftScroll & 0x0f);
}


extern void    Render_RenderBorder_Paletted(void);
extern void    Render_RenderBorder_TrueColour(void);
extern void    Render_GetGraphicsDataCPC_TrueColour(void);
extern void    Render_GetGraphicsDataCPC_Paletted(void);
extern void    CRTC_RenderSync_Paletted(void);
extern void    CRTC_RenderSync_TrueColour(void);
extern void    CRTC_RenderBlack_Paletted(void);
extern void    CRTC_RenderBlack_TrueColour(void);


extern MONITOR_INTERNAL_STATE Monitor_State;
int Pixels[16];
void    Render_GetGraphicsDataPlus_TrueColour(void)
{
	/*	unsigned short Addr; */
	unsigned long Mask;

	ASIC_CalcVRAMAddr();
	/*	Addr = ASIC_GetVRAMAddr(); */

	ASIC_CachePixelData();

	PixelData = ASIC_GetPixelData();

	/* Line, Column, ActualX, ActualY */
	Mask = ASIC_BuildDisplayReturnMaskWithPixels(ASICCRTC_Line /*VisibleRasterCount*/, CRTC_InternalState.HCount, /*MonitorHorizontalCount, MonitorScanLineCount,*/Pixels);

	if (Mask == 0x0ffffffff)
	{
		Render_TrueColourRGB_PutDataWord(Monitor_State.MonitorHorizontalCount, PixelData, Monitor_State.MonitorScanLineCount);
	}
	else
	{

		Render_TrueColourRGB_PutDataWordPLUS(Monitor_State.MonitorHorizontalCount, PixelData, Monitor_State.MonitorScanLineCount, Mask, Pixels);


	}


}

void Plus_UpdateGraphicsFunction(void)
{
	/* during hsync from CRTC asic outputs black palette colour */
	/* it does this for the entire duration of the programmed hsync */
	/* the black colour is different from the hsync colour which is much more black */

	/* vsync is harder to see */

	/* confirmed: blank and border have higher priority than sprite */
	/* sprite only in visible area */
	if (Monitor_DrawSync())
	{

		CRTC_SetRenderFunction2(CRTC_RenderSync_TrueColour);
	}
	else
		if (Computer_GetDrawBlanking() && ((GateArray_State.BlankingOutput & (HBLANK_ACTIVE | VBLANK_ACTIVE)) != 0))
		{
			CRTC_SetRenderFunction2(CRTC_RenderBlack_TrueColour);
		}
		else
			if (Computer_GetDrawBorder() && ((GateArray_State.BlankingOutput & (DISPTMG_ACTIVE)) != 0))
			{
				CRTC_SetRenderFunction2(Render_RenderBorder_TrueColour);
			}
			else
			{
				/* original arnold code would decide when to enable plus sprites
				 because they do slow it down a bit.
				 CRTC_SetRenderFunction2(Render_GetGraphicsDataCPC_TrueColour); */
				CRTC_SetRenderFunction2(Render_GetGraphicsDataPlus_TrueColour);
			}
}

/*------------------------------------------------------------------------------------*/

void ASIC_UpdateHsync(BOOL bState)
{
	/* This is the HSYNC signal from the CRTC */

	if (bState)
	{
		GateArray_State.CRTCSyncInputs |= HSYNC_INPUT;
		GateArray_State.BlankingOutput |= HBLANK_ACTIVE;
		GateArray_State.nHBlankCycle = 0;
		ASIC_Data.DmaPhase = DMA_PHASE_DEAD_CYCLE;

		/* confirmed: PRI based interrupts are triggered from the start of the HSYNC */
				ASIC_RefreshRasterInterrupt(CRTC_InternalState.LineCounter, CRTC_InternalState.RasterCounter);

		Plus_UpdateGraphicsFunction();
	}
	else
	{
		GateArray_State.CRTCSyncInputs &= ~HSYNC_INPUT;
		GateArray_State.BlankingOutput &= ~HBLANK_ACTIVE;

		Monitor_DoHsyncEnd();

		/* increment interrupt line count */
		GateArray_State.InterruptLineCount++;

		/* if line == 52 then interrupt should be triggered */
		if (GateArray_State.InterruptLineCount == 52)
		{
			/* clear counter. */
			GateArray_State.InterruptLineCount = 0;

			GateArray_State.RasterInterruptRequest = TRUE;
			
			ASIC_RefreshRasterInterruptState();
		}

		/* CHECK: Vblank triggered off hsync end or start? */
		if (GateArray_State.BlankingOutput & VBLANK_ACTIVE)
		{
			/* CHECK: VSYNC to monitor switched off at next HSYNC? */

			GateArray_State.nVBlankCycle++;
			if (GateArray_State.nVBlankCycle == 2)
			{
				/* has interrupt line counter overflowed? */
				if (GateArray_State.InterruptLineCount >= 32)
				{
					/* following might not be required, because it is probably */
					/* set by the code above */

					GateArray_State.RasterInterruptRequest = TRUE;

					ASIC_RefreshRasterInterruptState();
				}

				/* reset interrupt line count */
				GateArray_State.InterruptLineCount = 0;


				/* TODO: Check */
				if (GateArray_State.BlankingOutput & VSYNC_INPUT)
				{
					Monitor_DoVsyncStart();
				}
			}
			else if (GateArray_State.nVBlankCycle == 6)
			{
				Monitor_DoVsyncEnd();
			}
			else if (GateArray_State.nVBlankCycle == 26)
			{
				GateArray_State.BlankingOutput &= ~VBLANK_ACTIVE;
			}
		}

		Plus_UpdateGraphicsFunction();
	}
}

/*------------------------------------------------------------------------------------*/

void ASIC_UpdateVsync(BOOL bState)
{
	if (bState)
	{

		GateArray_State.CRTCSyncInputs |= VSYNC_INPUT;
		GateArray_State.BlankingOutput |= VBLANK_ACTIVE;
		GateArray_State.nVBlankCycle = 0;
		Plus_UpdateGraphicsFunction();
	}
	else
	{

		GateArray_State.CRTCSyncInputs &= ~VSYNC_INPUT;


		Monitor_DoVsyncEnd();

		Plus_UpdateGraphicsFunction();

	}
}


void ASIC_DoDispEnable(BOOL bState)
{
	if (bState)
	{

		GateArray_State.BlankingOutput &= ~DISPTMG_ACTIVE;
		Computer_UpdateGraphicsFunction();
	}
	else
	{

		GateArray_State.BlankingOutput |= DISPTMG_ACTIVE;
		Computer_UpdateGraphicsFunction();
	}
}

BOOL ASIC_GateArray_GetHBlankActive(void)
{
	return ((GateArray_State.BlankingOutput & HBLANK_ACTIVE) != 0);
}

int ASIC_GateArray_GetHBlankCount(void)
{
	return GateArray_State.nHBlankCycle;
}


BOOL ASIC_GateArray_GetVBlankActive(void)
{
	return ((GateArray_State.BlankingOutput & VBLANK_ACTIVE) != 0);
}

int ASIC_GateArray_GetVBlankCount(void)
{
	return GateArray_State.nVBlankCycle;
}

/*------------------------------------------------------------------------------------*/
void ASIC_Cycle(void)
{
	/* HSYNC is delayed by Gate-Array by 2us */
	if (GateArray_State.BlankingOutput & HBLANK_ACTIVE)
	{
		switch (GateArray_State.nHBlankCycle)
		{
		case 0:
			//			case 1:
			break;

		case 1:
		{
			/* set pixel translation table which is dependant on mode */
			Render_SetPixelTranslation(GateArray_State.RomConfiguration & 0x03);


			if (GateArray_State.CRTCSyncInputs & HSYNC_INPUT)
			{
				Monitor_DoHsyncStart();
			}
			Plus_UpdateGraphicsFunction();
		}
		break;

		case 5:
		{
			/* end Hsync to monitor */
			Monitor_DoHsyncEnd();
			Plus_UpdateGraphicsFunction();
		}
		break;
		}
		GateArray_State.nHBlankCycle++;
	}

	/* TODO: Handle Paused channels */
	switch (ASIC_Data.DmaPhase)
	{
	case DMA_PHASE_DONE:
		break;

	case DMA_PHASE_DEAD_CYCLE:
	{
		/* which channels are enabled and require opcode fetch? */
		if (ASIC_Data.DMAChannelEnables == 0)
		{
			ASIC_Data.DmaPhase = DMA_PHASE_DONE;
		}
		else
		{
			ASIC_Data.OpcodeFetchChannels = ASIC_Data.DMAChannelEnables & ~ASIC_Data.DMAChannelPaused;
			ASIC_Data.OpcodeExecuteChannels = ASIC_Data.DMAChannelEnables;
			/* if all channels are paused then skip to execute directly */
			if (ASIC_Data.OpcodeFetchChannels == 0)
			{
				ASIC_Data.DmaPhase = DMA_PHASE_OPCODE_EXECUTE;
			}
			else
			{
				ASIC_Data.DmaPhase = DMA_PHASE_OPCODE_FETCH;
			}
		}
	}
	break;

	case DMA_PHASE_OPCODE_FETCH:
	{
		/* opcode  fetch */

		/* confirmed for opcode fetch/execute:

		Test used: Dma channel 2 always enabled and will execute STOP and INT.
		We then enable channel 0 then 1 then 0 and 1 together. Dma channel and 1 execute STOP.
		When we execute the interrupt set a colour on the screen. We see that the following happens.

		results:

		read channel 2, execute channel 2
		read channel 0, read channel 2, execute channel 0, execute channel 2
		read channel 1, read channel 2, execute channel 1, execute channel 2
		read channel 0, read channel 1, read channel 2, execute channel 0, execute channel 1, execute channel 2

		The position of the int changes indicating that the opcode fetches are "packed" together and the opcode executes
		are packed together.

		*/

		/* channel 2 only:
		4 ints in same pos
		channel 0 and channel 2 shows channel 2 shifted by 1 when channel 0 is paused

		channel 0:
		pause, pause, nop, stop
		channel 2:
		int, int int, int

		channel 0 or channel 1 with channel 2 shows same result.
		channel 0 and channel 1 with channel 2 shows shifted values

		exe delay, exe delay, exe delay, stop; no execute?
		*/
		/* seems pause same as nop? */



		int i = 0;

		for (i = 0; i <= 2; i++)
		{
			if (ASIC_Data.OpcodeFetchChannels & (1 << i))
			{
				ASIC_DMA_FetchOpcode(i);
				ASIC_Data.OpcodeFetchChannels &= ~(1 << i);
				break;
			}
			
		}
		/* fetched all opcodes? */
		if (ASIC_Data.OpcodeFetchChannels==0)
		{
			ASIC_Data.DmaPhase = DMA_PHASE_OPCODE_EXECUTE;
		}
	}
	break;

	/* TODO: Additional phases to handle writing to AY
	command (1)

	thank you to power ukonx for this information:

	DMAx :
	select AY register
	AY inactive
	write register
	AY inactive
	restore previous ay register selection
	AY inactive

	command(1),
	select ay register(1),
	ay inactive(1),
	write register(1),
	ay inactive(1),
	restore ay register(1),
	ay inactive(1)
	restore 8255?? (1) -> 8
	
	there is more here.. bdir and bc1 are restored too
	
	*/

	case DMA_PHASE_SELECT_AY_REGISTER:
	{
		/* select register */
		PSG_SetBDIRState(&OnBoardAY, 1);
		PSG_SetBC1State(&OnBoardAY, 1);
		PSG_RefreshState(&OnBoardAY);

		PSG_Write(&OnBoardAY, ASIC_Data.DmaAyRegister);

		ASIC_Data.DmaPhase = DMA_PHASE_SELECT_AY_REGISTER_INACTIVE;
	}
	break;

	case DMA_PHASE_SELECT_AY_REGISTER_INACTIVE:
	{
		/* inactive */
		PSG_SetBDIRState(&OnBoardAY, 0);
		PSG_SetBC1State(&OnBoardAY, 0);
		PSG_RefreshState(&OnBoardAY);

		ASIC_Data.DmaPhase = DMA_PHASE_WRITE_AY_REGISTER;
	}
	break;

	case DMA_PHASE_WRITE_AY_REGISTER:
	{
		/* write register */
		PSG_SetBDIRState(&OnBoardAY, 1);
		PSG_SetBC1State(&OnBoardAY, 0);
		PSG_RefreshState(&OnBoardAY);

		PSG_Write(&OnBoardAY, ASIC_Data.DmaAyData);

		ASIC_Data.DmaPhase = DMA_PHASE_WRITE_AY_REGISTER_INACTIVE;
	}
	break;

	case DMA_PHASE_WRITE_AY_REGISTER_INACTIVE:
	{
		/* inactive */
		PSG_SetBDIRState(&OnBoardAY, 0);
		PSG_SetBC1State(&OnBoardAY, 0);
		PSG_RefreshState(&OnBoardAY);

		ASIC_Data.DmaPhase = DMA_PHASE_RESTORE_AY_REGISTER;
	}
	break;

	case DMA_PHASE_RESTORE_AY_REGISTER:
	{
		/* select register */
		PSG_SetBDIRState(&OnBoardAY, 1);
		PSG_SetBC1State(&OnBoardAY, 1);
		PSG_RefreshState(&OnBoardAY);

		PSG_Write(&OnBoardAY, ASIC_Data.CurrentAyRegister);

		/* restore previous register */
		ASIC_Data.DmaPhase = DMA_PHASE_RESTORE_AY_REGISTER_INACTIVE;
	}
	break;

	case DMA_PHASE_RESTORE_AY_REGISTER_INACTIVE:
	{
		/* inactive */
		PSG_SetBDIRState(&OnBoardAY, 0);
		PSG_SetBC1State(&OnBoardAY, 0);
		PSG_RefreshState(&OnBoardAY);

		ASIC_Data.DmaPhase = DMA_PHASE_RESTORE_PPI;
	}
	break;

	case DMA_PHASE_RESTORE_PPI:
	{
		/* inactive */
		PSG_SetBDIRState(&OnBoardAY, ASIC_Data.Current8255Select&(1<<7));
		PSG_SetBC1State(&OnBoardAY, ASIC_Data.Current8255Select&(1<<6));
		PSG_RefreshState(&OnBoardAY);

		ASIC_Data.DmaPhase = DMA_PHASE_OPCODE_EXECUTE;
	}
	break;


	case DMA_PHASE_OPCODE_EXECUTE:
	{
		int i = 0;

		for (i = 0; i <= 2; i++)
		{
			if (ASIC_Data.OpcodeExecuteChannels & (1 << i))
			{
				if ((ASIC_Data.DMAChannelPaused & (1 << i)) == 0)
				{
					ASIC_DMA_ExecuteCommand(i);
				}

				if (ASIC_Data.DMAChannelPaused & (1 << i))
				{
					ASIC_DMA_CHANNEL        *pChannel = &ASIC_Data.DMAChannel[i];

					if (pChannel->PrescaleCount != 0)
					{
						pChannel->PrescaleCount--;
					}
					else
					{
						pChannel->PauseCount--;

						/* is there a pause active? */
						if (pChannel->PauseCount == 0)
						{
							ASIC_Data.DMAChannelPaused &= ~(1 << i);
						}
						else
						{
							pChannel->PrescaleCount = ASIC_Data.DMA[i].Prescale;
						}
					}
				}
				ASIC_Data.OpcodeExecuteChannels &= ~(1 << i);
				break;
			}
		}
		/* fetched all opcodes? */
		if ((ASIC_Data.DmaPhase == DMA_PHASE_OPCODE_EXECUTE) && (ASIC_Data.OpcodeExecuteChannels == 0))
		{
			ASIC_Data.DmaPhase = DMA_PHASE_DONE;
		}
	}
	break;
	}
}
/*------------------------------------------------------------------------------------*/

int ASIC_GetPRIVCC(void)
{
	return (ASIC_Data.ASIC_RasterInterruptLine >> 3) & 0x01f;
}
int ASIC_GetPRIRCC(void)
{
	return (ASIC_Data.ASIC_RasterInterruptLine & 0x07);
}


unsigned char ASIC_GetPRI(void)
{
	return (unsigned char)ASIC_Data.ASIC_RasterInterruptLine;
}

unsigned char ASIC_GetSPLT(void)
{
	return (unsigned char)ASIC_Data.ASIC_RasterSplitLine;
}

int ASIC_GetSPLTVCC(void)
{
	return (ASIC_Data.ASIC_RasterSplitLine >> 3) & 0x01f;
}
int ASIC_GetSPLTRCC(void)
{
	return (ASIC_Data.ASIC_RasterSplitLine & 0x07);
}

unsigned short ASIC_GetSSA(void)
{
	return ASIC_Data.ASIC_SecondaryScreenAddress.Addr_W;
}

unsigned char ASIC_GetSSCR(void)
{
	return (unsigned char)ASIC_Data.ASIC_SoftScroll;
}

unsigned char ASIC_GetIVR(void)
{
	return (unsigned char)ASIC_Data.ASIC_InterruptVector;
}


void ASIC_SetPRI(unsigned char PRI)
{
	ASIC_Data.ASIC_RasterInterruptLine = PRI;
}

void	ASIC_SetSPLT(unsigned char SPLT)
{
	ASIC_Data.ASIC_RasterSplitLine = SPLT;
}

void ASIC_SetSSA(unsigned long SSA)
{
	ASIC_Data.ASIC_SecondaryScreenAddress.Addr_W = (unsigned short)SSA;
}

void ASIC_SetSSCR(unsigned char SSCR)
{
	ASIC_Data.ASIC_SoftScroll = SSCR;
}


void ASIC_SetColour(int Index)
{
	unsigned long PackedRGBLookup;
	int Addr = (Index << 1) + 0x02400;

	PackedRGBLookup = ASIC_Data.ASIC_Ram[Addr] | (((ASIC_Data.ASIC_Ram[Addr + 1]) & 0x0f) << 8);

	Render_SetColour(&ASIC_DisplayColours[PackedRGBLookup], Index);
}


void	ASIC_SetIVR(unsigned char IVR)
{
	ASIC_Data.ASIC_InterruptVector = IVR;
}

/* byte 0: upper nibble is red, lower nibble is blue.
   byte 1: lower nibble is green */

#ifdef CPC_LSB_FIRST
/* value is 0GRB for writing LSB first */
#define CPC_TO_ASIC_COLOUR(R,G,B) ((G<<8) | (R<<4) | (B))
#else
/* value is RB0G for writing MSB first */
#define CPC_TO_ASIC_COLOUR(R,G,B) ((R<<12) | (B<<8) | (G))
#endif

/* ASIC RGB of CPC hw colour. These values were obtained by
sending the CPC colour and reading back the RGB from the ASIC palette,
using a test program */

static unsigned short	CPCToASICColours[32] =
{
	CPC_TO_ASIC_COLOUR(0x06, 0x06, 0x06),                     /* White */
	CPC_TO_ASIC_COLOUR(0x06, 0x06, 0x06),                     /* White */
	CPC_TO_ASIC_COLOUR(0x00, 0x0f, 0x06),                     /* Sea Green */
	CPC_TO_ASIC_COLOUR(0x0f, 0x0f, 0x06),                     /* Pastel yellow */
	CPC_TO_ASIC_COLOUR(0x00, 0x00, 0x06),                     /* Blue */
	CPC_TO_ASIC_COLOUR(0x0f, 0x00, 0x06),                     /* Purple */
	CPC_TO_ASIC_COLOUR(0x00, 0x06, 0x06),                     /* Cyan */
	CPC_TO_ASIC_COLOUR(0x0f, 0x06, 0x06),                     /* Pink */
	CPC_TO_ASIC_COLOUR(0x0f, 0x00, 0x06),                     /* Purple */
	CPC_TO_ASIC_COLOUR(0x0f, 0x0f, 0x06),                    /* Pastel yellow */
	CPC_TO_ASIC_COLOUR(0x0f, 0x0f, 0x00),                     /* Bright Yellow */
	CPC_TO_ASIC_COLOUR(0x0f, 0x0f, 0x0f),                     /* Bright White */
	CPC_TO_ASIC_COLOUR(0x0f, 0x00, 0x00),                     /* Bright Red */
	CPC_TO_ASIC_COLOUR(0x0f, 0x00, 0x0f),                     /* Bright Magenta */
	CPC_TO_ASIC_COLOUR(0x0f, 0x06, 0x00),                     /* Orange */
	CPC_TO_ASIC_COLOUR(0x0f, 0x06, 0x0f),                     /* Pastel Magenta */
	CPC_TO_ASIC_COLOUR(0x00, 0x00, 0x06),                     /* Blue */
	CPC_TO_ASIC_COLOUR(0x00, 0x0f, 0x06),                     /* Sea Green */
	CPC_TO_ASIC_COLOUR(0x00, 0x0f, 0x00),                     /* Bright green */
	CPC_TO_ASIC_COLOUR(0x00, 0x0f, 0x0f),                     /* Bright Cyan */
	CPC_TO_ASIC_COLOUR(0x00, 0x00, 0x00),                     /* Black */
	CPC_TO_ASIC_COLOUR(0x00, 0x00, 0x0f),                     /* Bright Blue */
	CPC_TO_ASIC_COLOUR(0x00, 0x06, 0x00),                     /* Green */
	CPC_TO_ASIC_COLOUR(0x00, 0x06, 0x0f),                     /* Sky Blue */
	CPC_TO_ASIC_COLOUR(0x06, 0x00, 0x06),                     /* Magenta */
	CPC_TO_ASIC_COLOUR(0x06, 0x0f, 0x06),                     /* Pastel green */
	CPC_TO_ASIC_COLOUR(0x06, 0x0f, 0x00),                     /* Lime */
	CPC_TO_ASIC_COLOUR(0x06, 0x0f, 0x0f),                     /* Pastel cyan */
	CPC_TO_ASIC_COLOUR(0x06, 0x00, 0x00),                     /* Red */
	CPC_TO_ASIC_COLOUR(0x06, 0x00, 0x0f),                     /* Mauve */
	CPC_TO_ASIC_COLOUR(0x06, 0x06, 0x00),                     /* Yellow */
	CPC_TO_ASIC_COLOUR(0x06, 0x06, 0x0f)                      /* Pastel blue */
};

static unsigned char    *CartridgePages[32];                    /* pointer to cartridge pages */
static unsigned long	NumCartridgeBlocks;
static unsigned long    HighestBlockIndex;
static unsigned char	*CartridgeBlocks[32];
static unsigned char CartridgeDummyPage[16384];

static MemoryRange  CartridgeMemoryRanges[32];
static MemoryRange  ASICRamMemoryRange;


static const int ASIC_EnableSequence[] =
{
	0x0ff, 0x077, 0x0b3, 0x051, 0x0a8, 0x0d4, 0x062, 0x039,
	0x09c, 0x046, 0x02b, 0x015, 0x08a
};

#define ASIC_EnableSequenceLength (sizeof(ASIC_EnableSequence)/sizeof(const int))


void	ASIC_SetUnLockState(BOOL State)
{
	if (State)
	{
		ASIC_Data.Flags |= ASIC_ENABLED;
	}
	else
	{
		ASIC_Data.Flags &= ~ASIC_ENABLED;
	}
}

BOOL	ASIC_GetUnLockState(void)
{
	return ((ASIC_Data.Flags & ASIC_ENABLED) == ASIC_ENABLED);
}


/* when an out to 0x0bc is done, call this code
 this code enables or disables the ASIC depending on the un-locking/locking
 sequence */
void    ASIC_EnableDisable(int Data)
{
	switch (ASIC_Data.RecogniseSequenceState)
	{
	case    SEQUENCE_SYNCHRONISE_FIRST_BYTE:
	{
		/* we are waiting for the first byte of synchronisation */
		if (Data != 0)
		{
			ASIC_Data.RecogniseSequenceState = SEQUENCE_SYNCHRONISE_SECOND_BYTE;
		}
	}
	break;

	case    SEQUENCE_SYNCHRONISE_SECOND_BYTE:
	{
		/* at this point we already have a non-zero byte to start the synchronisation.

		 we are waiting for the second byte of synchronisation

		 this byte must be zero for synchronisation. If it is non-zero,
		 then it can still count as part of the synchronisation. */

		if (Data == 0x00)
		{
			/* got zero. We are now waiting for the first byte of the sequence */
			ASIC_Data.RecogniseSequenceState = SEQUENCE_SYNCHRONISE_THIRD_BYTE;
		}
	}
	break;

	case SEQUENCE_SYNCHRONISE_THIRD_BYTE:
	{
		/* we are waiting for the first data byte of the sequence. To get here
		 we must have had a non-zero byte followed by a zero byte */

		/* have we got the first byte of the sequence? */
		if (Data == 0x0ff)
		{
			/* first byte of sequence, get ready to recognise the sequence. */
			ASIC_Data.RecogniseSequenceState = SEQUENCE_RECOGNISE;
			ASIC_Data.CurrentSequencePos = 1;
		}
		else
		{
			if (Data != 0)
			{
				/* we got a non-zero byte, and it wasn't the first part of the sequence.
				/ this could act as the first byte of synchronisation ready for a zero
				/ to follow. */
				ASIC_Data.RecogniseSequenceState = SEQUENCE_SYNCHRONISE_SECOND_BYTE;
			}

			/* if we got a zero, we are still synchronised. We are still waiting for
			the first byte of sequence. */
		}

	}
	break;

	case    SEQUENCE_RECOGNISE:
	{
		/* we want to recognise the sequence. We already recognise the first byte. */

		if (Data == 0x000)
		{
			ASIC_Data.RecogniseSequenceState = SEQUENCE_SYNCHRONISE_THIRD_BYTE;
		}
		else
		{
			if (Data == ASIC_EnableSequence[ASIC_Data.CurrentSequencePos])
			{
				/* data byte the same as sequence */

				/* ready for next char in sequence */
				ASIC_Data.CurrentSequencePos++;

				if (ASIC_Data.CurrentSequencePos == ASIC_EnableSequenceLength)
				{

					/* sequence is almost complete. If next byte sent is
					 a 0x0ee then the asic will be enabled.  */
					ASIC_Data.RecogniseSequenceState = SEQUENCE_GET_LOCK_STATUS;
					break;
				}
			}
			else
			{
				ASIC_Data.CurrentSequencePos = 0;
				ASIC_Data.RecogniseSequenceState = SEQUENCE_SYNCHRONISE_SECOND_BYTE;
			}
		}
	}
	break;

	case    SEQUENCE_GET_LOCK_STATUS:
	{
		/* the sequence has been correct up to this point, therefore
		 we want to lock or un-lock the ASIC */

		if (Data != 0x0cd)
		{
			/* not correct byte, set the lock */
			ASIC_Data.Flags &= ~ASIC_ENABLED;
			TriggerBreakOn(BREAK_ON_ASIC_LOCK);
			ASIC_Data.RecogniseSequenceState = SEQUENCE_SYNCHRONISE_FIRST_BYTE;
		}
		else
		{
			/* correct byte, but need one more write to set the lock */
			ASIC_Data.RecogniseSequenceState = SEQUENCE_GET_POST_LOCK;
		}
	}
	break;

	case SEQUENCE_GET_POST_LOCK:
	{
		ASIC_Data.Flags |= ASIC_ENABLED;
		TriggerBreakOn(BREAK_ON_ASIC_UNLOCK);
		ASIC_Data.RecogniseSequenceState = SEQUENCE_SYNCHRONISE_FIRST_BYTE;
	}
	break;
	}
}

unsigned char   *ASIC_GetCartPage(int Page)
{
	return CartridgePages[Page & 0x01f];
}


void ASIC_SetInitialAnalogueInputs(void)
{
	int i;

	/* confirmed, with nothing attached the channels report 0x03f.
	same result as my 464 and same result on gx4000, although 5/6 rapidly switch between 0 and 1 */
	/* with amstrad analogue joystick, and calibrated, 1b/1c is the middle and it flashes rapidly */

	/*  analogue inputs */
	/* inputs x0, x1, x2, x3 are used and connected so match the actual inputs */
	ASIC_Data.AnalogueInputs[0] = 0x03f;
	ASIC_Data.AnalogueInputs[1] = 0x03f;
	ASIC_Data.AnalogueInputs[2] = 0x03f;
	ASIC_Data.AnalogueInputs[3] = 0x03f;

	/* x4, x6 are 5v, x5,x7 are 0 */
	ASIC_Data.AnalogueInputs[4] = 0x03f;
	ASIC_Data.AnalogueInputs[5] = 0x000;
	ASIC_Data.AnalogueInputs[6] = 0x03f;
	ASIC_Data.AnalogueInputs[7] = 0x000;

	/* ensure ram is up to date */
	for (i = 0; i < 8; i++)
	{
		ASIC_Data.ASIC_Ram[0x02808] = ASIC_Data.AnalogueInputs[i];
	}
}

void Plus_WriteSnapshotBlock(unsigned char *pBlock)
{
	int i;
	unsigned char *pPtr = pBlock;
	int State = 0;

	/* sprite data */
	for (i = 0; i < ((16 * 16 * 16) >> 1); i++)
	{
		unsigned char PackedPixels;

		/* generate the packed pixel from 2 sprite pixels */
		PackedPixels = (ASIC_Data.ASIC_Ram[(i<<1)+0] & 0x0f);
		PackedPixels = PackedPixels << 4;
		PackedPixels |= (ASIC_Data.ASIC_Ram[(i << 1)+1] & 0x0f);
		pPtr[0] = PackedPixels;
		++pPtr;
	}

	/* sprite attributes */
	for (i = 0; i < 16; i++)
	{
		pPtr[0] = (ASIC_Data.Sprites[i].SpriteX.SpriteX_W&0x0ff);
		pPtr[1] = ((ASIC_Data.Sprites[i].SpriteX.SpriteX_W>>8) & 0x0ff);
		pPtr[2] = (ASIC_Data.Sprites[i].SpriteY.SpriteY_W & 0x0ff);
		pPtr[3] = ((ASIC_Data.Sprites[i].SpriteY.SpriteY_W >> 8) & 0x0ff);
		pPtr[4] = ((ASIC_Data.Sprites[i].SpriteMag & 0x0f));
		pPtr[5] = 0;
		pPtr[6] = 0;
		pPtr[7] = 0;
		pPtr += 8;
	}

	/* palettes */
	for (i = 0; i < 32; i++)
	{
		pPtr[0] = ASIC_Data.ASIC_Ram[0x06400 - 0x04000 + (i<<1)+0];
		++pPtr;
		pPtr[0] = ASIC_Data.ASIC_Ram[0x06400 - 0x04000 + (i<<1)+1];
		++pPtr;
	}

	pPtr[0] = ASIC_GetPRI();
	++pPtr;
	pPtr[0] = ASIC_GetSPLT();
	++pPtr;
	pPtr[0] = ((ASIC_GetSSA()>>8)&0x0ff);
	++pPtr;
	pPtr[0] = (ASIC_GetSSA()&0x0ff);
	++pPtr;
	pPtr[0] = ASIC_GetSSCR();
	++pPtr;
	pPtr[0] = ASIC_GetIVR();
	++pPtr;

	/* secondary rom mapping; make sure bits are compatible for winape */
	pPtr[0] = (ASIC_GetSecondaryRomMapping() & 0x01f)|0x0a0;
	++pPtr;
	/* unused */
	pPtr[0] = 0;
	++pPtr;

	/* analogue inputs */
	for (i = 0; i < 8; i++)
	{
		pPtr[0] = ASIC_GetAnalogueInput(i);
		++pPtr;
	}

	/* DMA */
	for (i = 0; i < 3; i++)
	{
		pPtr[0] = ASIC_DMA_GetChannelAddr(i)&0x0ff;
		++pPtr;
		pPtr[0] = ASIC_DMA_GetChannelAddr(i) & 0x0ff;
		++pPtr;
		pPtr[0] = ASIC_DMA_GetChannelPrescale(i)&0x0ff;
		++pPtr;
		pPtr[0] = 0; /* unused */
		++pPtr;
	}

	/* unused */
	pPtr[0] = 0;
	++pPtr;
	pPtr[0] = 0;
	++pPtr;

	/* DCSR */
	pPtr[0] = ASIC_GetDCSR();
	++pPtr;

	/* DMA internal */
	for (i = 0; i < 3; i++)
	{
		pPtr[0] = ASIC_Data.DMAChannel[i].RepeatCount & 0x0ff;
		++pPtr;
		pPtr[0] = (ASIC_Data.DMAChannel[i].RepeatCount>>8) & 0x0ff;
		++pPtr;
		pPtr[0] = ASIC_Data.DMAChannel[i].LoopStart  & 0x0ff;
		++pPtr;
		pPtr[0] = (ASIC_Data.DMAChannel[i].LoopStart >> 8) & 0x0ff;
		++pPtr;
		pPtr[0] = ASIC_Data.DMAChannel[i].PauseCount & 0x0ff;
		++pPtr;
		pPtr[0] = (ASIC_Data.DMAChannel[i].PauseCount >> 8) & 0x0ff;
		++pPtr;
		pPtr[0] = ASIC_Data.DMAChannel[i].PrescaleCount & 0x0ff;
		++pPtr;
	}

	/* secondary rom mapping (original specification) */
	pPtr[0] = (ASIC_GetSecondaryRomMapping() & 0x01f)|0x0a0;
	++pPtr;

	/* unlock state */
	pPtr[0] = ASIC_GetUnLockState() ? 1 : 0;
	++pPtr;

	/* position in unlock sequence */
	
	switch (ASIC_Data.RecogniseSequenceState)
	{
		case SEQUENCE_SYNCHRONISE_FIRST_BYTE:
		{
			State = 0;
		}
		break;
		case SEQUENCE_SYNCHRONISE_SECOND_BYTE:
		{
			State = 1;
		}
		break;
		case SEQUENCE_SYNCHRONISE_THIRD_BYTE:
		{
			State = 2;
		}
		break;

		default:
		{
			State = (ASIC_Data.CurrentSequencePos - 1)+3;
		}
		break;
	}
	pPtr[0] = State;
	++pPtr;
}

void Plus_ReadSnapshotChunk(unsigned char *pChunkPtr)
{
	unsigned long ASICRamOffset;
	int i;
	int Position;

	ASIC_Data.Flags |= (ASIC_ENABLED);

	/* sprite data */
	ASICRamOffset = 0x04000;

	/* each sprite is 16x16 and there are 16 of these */
	/* in the snapshot the x pixels are compressed so there are 2 pixels per byte,
	therefore each sprite takes up 8*16 bytes. */
	for (i = 0; i < ((16 * 16 * 16) >> 1); i++)
	{
		unsigned char PackedPixels;

		PackedPixels = pChunkPtr[0];
		++pChunkPtr;
		ASIC_WriteRamFull(ASICRamOffset+(i<<1)+0, (PackedPixels >> 4) & 0x0f);
		ASIC_WriteRamFull(ASICRamOffset + (i << 1) + 1, PackedPixels & 0x0f);
	}

	/* sprite attributes */
	ASICRamOffset = 0x06000;

	for (i = 0; i < 16; i++)
	{
		/* x */
		ASIC_WriteRamFull(ASICRamOffset, pChunkPtr[0]);
		pChunkPtr++;
		ASICRamOffset++;

		ASIC_WriteRamFull(ASICRamOffset, pChunkPtr[0]);
		pChunkPtr++;
		ASICRamOffset++;

		/* y */
		ASIC_WriteRamFull(ASICRamOffset, pChunkPtr[0]);
		pChunkPtr++;
		ASICRamOffset++;

		ASIC_WriteRamFull(ASICRamOffset, pChunkPtr[0]);
		pChunkPtr++;
		ASICRamOffset++;

		/* mag */
		ASIC_WriteRamFull(ASICRamOffset, pChunkPtr[0]);
		pChunkPtr++;
		ASICRamOffset++;

		/* unused */
		pChunkPtr += 3;
		ASICRamOffset += 3;
	}

	/* palettes */
	ASICRamOffset = 0x06400;

	for (i = 0; i < 32; i++)
	{
		ASIC_WriteRamFull(ASICRamOffset, pChunkPtr[0]);
		pChunkPtr++;
		ASICRamOffset++;

		ASIC_WriteRamFull(ASICRamOffset, pChunkPtr[0]);
		pChunkPtr++;
		ASICRamOffset++;
	}

	/* misc */
	ASIC_WriteRamFull(0x06800, pChunkPtr[0]);
	pChunkPtr++;

	ASIC_WriteRamFull(0x06801, pChunkPtr[0]);
	pChunkPtr++;

	ASIC_WriteRamFull(0x06802, pChunkPtr[0]);
	pChunkPtr++;

	ASIC_WriteRamFull(0x06803, pChunkPtr[0]);
	pChunkPtr++;

	ASIC_WriteRamFull(0x06804, pChunkPtr[0]);
	pChunkPtr++;

	ASIC_WriteRamFull(0x06805, pChunkPtr[0]);
	pChunkPtr++;

	/* set secondary rom mapping (Winape stores it here) */
	{
		unsigned char Data = pChunkPtr[0];
		if ((Data & 0x0e0) == 0x0a0)
		{
			ASIC_SetSecondaryRomMapping(0x0a0 | pChunkPtr[0]);
		}
	}
	pChunkPtr++;

	/* skip unused */
	pChunkPtr++;	// pChunkPtr += 2;

	/* analogue inputs */
	for (i = 0; i < 8; i++)
	{
		ASIC_Data.AnalogueInputs[i] = pChunkPtr[0] & 0x03f;
		pChunkPtr++;
	}

	/* dma */
	ASICRamOffset = 0x06c00;

	for (i = 0; i < 3; i++)
	{
		/* address */
		ASIC_WriteRamFull(ASICRamOffset, pChunkPtr[0]);
		pChunkPtr++;
		ASICRamOffset++;

		ASIC_WriteRamFull(ASICRamOffset, pChunkPtr[0]);
		pChunkPtr++;
		ASICRamOffset++;

		/* prescale */
		ASIC_WriteRamFull(ASICRamOffset, pChunkPtr[0]);
		pChunkPtr++;
		ASICRamOffset++;

		/* unused */
		pChunkPtr++;
		ASICRamOffset++;
	}

	pChunkPtr += 3;

	/* DCSR */
	ASIC_WriteRamFull(0x06c0f, pChunkPtr[0]);
	pChunkPtr++;

	/* TO BE COMPLETED! */
	/* FIX NOW */
	pChunkPtr += (3 * 7);

	/* set secondary rom mapping (original specification stores it here) */
	{
		unsigned char Data = pChunkPtr[0];
		if ((Data & 0x0e0)==0x0a0)
		{
			ASIC_SetSecondaryRomMapping(0x0a0 | pChunkPtr[0]);
		}
	}
	pChunkPtr++;

	/* set lock state */
	if (pChunkPtr[0] & 0x01)
	{
		/* unlock */
		ASIC_SetUnLockState(TRUE);
	}
	else
	{
		/* lock */
		ASIC_SetUnLockState(FALSE);
	}
	++pChunkPtr;

	Position = pChunkPtr[0];
	switch (Position)
	{
	case 0:
	{
		ASIC_Data.RecogniseSequenceState = SEQUENCE_SYNCHRONISE_FIRST_BYTE;
	}
	break;

	case 1:
	{
		ASIC_Data.RecogniseSequenceState = SEQUENCE_SYNCHRONISE_SECOND_BYTE;
	}
	break;

	case 2:
	{
		ASIC_Data.RecogniseSequenceState = SEQUENCE_SYNCHRONISE_THIRD_BYTE;
	}
	break;

	default:
	{
		ASIC_Data.RecogniseSequenceState = SEQUENCE_RECOGNISE;
		ASIC_Data.CurrentSequencePos = Position - 2;
	}
	break;
	}


}

void Plus_LoadFromSnapshot(SNAPSHOT_HEADER *pSnapshotHeader)
{
	int i;

	/* initialise Plus hardware as if we are loading for CPC */

	ASIC_Data.ASIC_RAM_Config = (((char *)pSnapshotHeader)[0x041] & 0x03f);

	/* initialise colour palette */
	for (i = 0; i < 17; i++)
	{
		unsigned char HwColourIndex = ((char *)pSnapshotHeader)[0x02f + i];

		/* pen select */
		ASIC_GateArray_Write(i);

		/* write colour for pen */
		ASIC_GateArray_Write((HwColourIndex & 0x01f) | 0x040);
		printf("GA Pen %d Colour &%02x\n", i, (HwColourIndex & 0x01f) | 0x040);

	}

	/* pen select */
	ASIC_GateArray_Write(((char *)pSnapshotHeader)[0x02e] & 0x01f);
	printf("GA Selected Pen %d\n", ((char *)pSnapshotHeader)[0x02e] & 0x01f);

	/* mode and rom configuration select */
	ASIC_GateArray_Write((((char *)pSnapshotHeader)[0x040] & 0x03f) | 0x080);
	printf("GA MREM &%02x\n", (((char *)pSnapshotHeader)[0x040] & 0x03f) | 0x080);

	/**** ASIC PPI ****/

	/* set control */
	ASIC_PPI_WriteControl(((char *)pSnapshotHeader)[0x059]);
	printf("(Plus) PPI Control &%02x\n", ((char *)pSnapshotHeader)[0x059]);

	/* these depend on input/output! */
	/* set port A data */
	ASIC_PPI_SetPortDataFromSnapshot(0, ((char *)pSnapshotHeader)[0x056]);
	printf("(Plus) PPI Port A Data &%02x\n", ((char *)pSnapshotHeader)[0x056]);
	/* set port B data */
	ASIC_PPI_SetPortDataFromSnapshot(1, ((char *)pSnapshotHeader)[0x057]);
	printf("(Plus) PPI Port B Data &%02x\n", ((char *)pSnapshotHeader)[0x057]);
	/* set port C data */
	ASIC_PPI_SetPortDataFromSnapshot(2, ((char *)pSnapshotHeader)[0x058]);
	printf("(Plus) PPI Port C Data &%02x\n", ((char *)pSnapshotHeader)[0x058]);

	PSG_SetBDIRState(&OnBoardAY, ((char *)pSnapshotHeader)[0x058] & (1 << 7));
	PSG_SetBC1State(&OnBoardAY, ((char *)pSnapshotHeader)[0x058] & (1 << 6));
	PSG_RefreshState(&OnBoardAY);

}

void Plus_SaveToSnapshot(SNAPSHOT_HEADER *pHeader)
{
	/* Plus initialise defaults */
}
void Plus_FillSnapshotMemoryBlocks(SNAPSHOT_MEMORY_BLOCKS *pSnapshotMemoryBlocks, const SNAPSHOT_OPTIONS *pOptions, BOOL bReading)
{
	int i;
	for (i = 0; i < 4; i++)
	{
		SNAPSHOT_MEMORY_BLOCK *pBlock = &pSnapshotMemoryBlocks->Blocks[i];
		pBlock->nSourceId = SNAPSHOT_SOURCE_INTERNAL_ID;
		pBlock->bAvailable = TRUE;
		pBlock->pPtr = Z80MemoryBase + (i << 14);
	}
	/* 128KB Plus machines? */
	if (ASIC_R128)
	{
		for (i = 0; i < 4; i++)
		{
			if (bReading || pOptions->bBlocksToExport[(i + 4)])
			{
				SNAPSHOT_MEMORY_BLOCK *pBlock = &pSnapshotMemoryBlocks->Blocks[i+4];
				pBlock->nSourceId = SNAPSHOT_SOURCE_INTERNAL_ID;
				pBlock->bAvailable = TRUE;
				pBlock->pPtr = ASIC_ExtraRam + (i << 14);
			}
		}
	}
}

/* initialise ASIC emulation */
BOOL    ASIC_Initialise(void)
{
	/* allocate ASIC Ram */
	ASIC_Data.ASIC_Ram = (unsigned char *)malloc(16384);

	ASIC_ExtraRam = (unsigned char *)malloc(65536);

	if (ASIC_Data.ASIC_Ram == NULL || ASIC_ExtraRam == NULL)
	{
		return FALSE;
	}
	memset(ASIC_Data.ASIC_Ram, 0x0ff, 16384);
	memset(ASIC_ExtraRam, 0x0ff, 16384);

	ASICRamMemoryRange.m_nID = RIFF_FOURCC_CODE('A', 'S', 'I', 'C');
	ASICRamMemoryRange.sName = "ASIC Ram";
	ASICRamMemoryRange.m_bCPU = FALSE;
	ASICRamMemoryRange.pBase = ASIC_Data.ASIC_Ram;
	ASICRamMemoryRange.m_bReadOnly = FALSE;
	ASICRamMemoryRange.m_nLength = 16384;
	CPC_RegisterMemoryRange(&ASICRamMemoryRange);

	/* TODO: This is actually what was on the bus last */
	memset(ASIC_Data.ASIC_Ram, ASIC_UNUSED_RAM_DATA, 16384);

	ASIC_Data.ASIC_Ram_Adjusted = ASIC_Data.ASIC_Ram - 0x004000;

	/* Initialise cartridge pages */
	ASIC_InitCart();

	ASIC_SetInitialAnalogueInputs();

	/* initialise a table of colours and grey-scales. Given a ASIC
	colour, this will give the new colour to pass to the render part
	to give the appearance of a colour or grey-scale/paper-white display */
	ASIC_InitialiseMonitorColourModes();


	return TRUE;
}

void	ASIC_SetAnalogueInput(int InputID, unsigned char Data)
{
	if ((InputID >= 0) && (InputID <= 3))
	{
		ASIC_Data.AnalogueInputs[InputID] = Data;
		/* update ram with value */
		if (ASIC_Data.ASIC_Ram != NULL)
		{
			ASIC_Data.ASIC_Ram[0x02808 + InputID] = ASIC_Data.AnalogueInputs[InputID];
		}
	}
}

unsigned char ASIC_GetAnalogueInput(int InputID)
{
	return ASIC_Data.AnalogueInputs[InputID];
}

void	ASIC_WriteRamFull(int Addr, int Data)
{
	ASIC_Data.ASIC_Ram[Addr & 0x03fff] = Data;
	ASIC_WriteRam(Addr, Data);
}

void    ASIC_Finish(void)
{
	if (ASIC_ExtraRam)
	{
		free(ASIC_ExtraRam);
		ASIC_ExtraRam = NULL;
	}
	CPC_UnRegisterMemoryRange(&ASICRamMemoryRange);

	if (ASIC_Data.ASIC_Ram != NULL)
	{
		free(ASIC_Data.ASIC_Ram);
		ASIC_Data.ASIC_Ram = NULL;
	}

	Cartridge_RemoveI();
}

static void     ASIC_UpdateRAMWithInternalDCSR(void)
{
	ASIC_UpdateInternalDCSR();
	{

		int i;
		unsigned char *pAddr = &ASIC_Data.ASIC_Ram[0x02c00];

		for (i = 15; i >= 0; i--)
		{
			pAddr[i] = (unsigned char)ASIC_Data.InternalDCSR;
		}
	}
}

void    ASIC_Reset(void)
{
	int     i;


		/* SPRITES */

	for (i=0; i<3; i++)
	{
		ASIC_Data.DMA[i].Flags = 0x0ff;
		ASIC_Data.DMAChannel[i].PauseCount = 0;
		ASIC_Data.DMAChannel[i].PrescaleCount = 0;
		ASIC_Data.DMAChannel[i].LoopStart = 0;
		ASIC_Data.DMAChannel[i].RepeatCount = 0;
		ASIC_Data.DMAChannel[i].Instruction = 0x04000;
	}

	ASIC_Data.SpriteEnableMaskOnLine = 0;
	
	ASIC_Data.ASICRamEnabled = 0;
	ASIC_Data.ASIC_RasterInterruptLine = 0;
	ASIC_Data.ASIC_SoftScroll= 0;
	ASIC_Data.ASIC_RasterSplitLine = 0;
	ASIC_Data.DMAChannelEnables = 0;
	ASIC_Data.DMAChannelPaused = 0;	/* TODO: Check channel pauses are cleared at reset */
	ASIC_Data.OpcodeFetchChannels = 0;
	ASIC_Data.OpcodeExecuteChannels = 0;
	ASIC_Data.InternalDCSR = 0;
	ASIC_Data.ASIC_Cartridge_Page = 0;
	ASIC_Data.SpriteEnableMask = 0;

	/* correct? */
	ASIC_Data.RecogniseSequenceState = SEQUENCE_SYNCHRONISE_FIRST_BYTE;
	ASIC_Data.CurrentSequencePos = 0;

	ASIC_Data.RasterInterruptAcknowledged = 0;
	ASIC_Data.DMAChannelEnables = 0;
	ASIC_Data.DMAChannelPaused = 0;
	ASIC_Data.InterruptRequest = 0;

	GateArray_State.RasterInterruptRequest = FALSE;
	ASIC_Data.ASIC_PRI_Request = FALSE;

	/* confirmed: DF00 is reset to 0 on restart. This means on GX4000 we see the "basic" page
	or whatever is at page 1. On Plus it depends on /EXP, we will either see dos or basic */
	ASIC_ROM_Select(0);
	ASIC_Data.ASIC_Cartridge_Page_Lower = 0;
	ASIC_Data.LowerRomIndex = 0;

	ASIC_Data.Flags |= (ASIC_ENABLED);

	/* all sprite magnification registers to 0 */
	for (i = 0; i < 16; i++)
	{
		ASIC_WriteRamFull(0x06000 + (i << 3) + 4, 0);
	}


	/* no scan-line interrupt */
	ASIC_WriteRamFull(0x06800, 0);

	/* no split */
	ASIC_WriteRamFull(0x06801, 0);

	/* no automatic clearing of DMA ints */
	/* bits 7-1 are undefined at reset */
	/* bit 0 set to 1 at reset */

	/* confirmed: at reset time the interrupt vector appears to be 0 */
	/* bit 0 set to 1 */
	ASIC_Data.ASIC_InterruptVector = 1;

	ASIC_Pal_Reset();

	/* set soft scroll register */
	ASIC_WriteRamFull(0x06804, 0);

	/* DCSR */
	ASIC_WriteRamFull(0x06c0f, 0);

	ASIC_SetInitialAnalogueInputs();

	ASIC_SetSecondaryRomMapping(0x0);

	/* disable asic - requires unlocking sequence to use features */
	ASIC_Data.Flags &= ~(ASIC_ENABLED);

	ASICCRTC_SetSoftScroll(0);

	/* ram reset */

	/* expansion rom reset */

	ASIC_PPI_Reset();

	/* mode and rom select reset */
	ASIC_GateArray_Write(0x080 | 0x00);

	GateArray_State.CRTCSyncInputs = 0;
	GateArray_State.BlankingOutput = 0;
	GateArray_State.nHBlankCycle = 0;
	GateArray_State.nVBlankCycle = 0;
	ASIC_Data.DmaPhase = DMA_PHASE_DONE;

	Render_SetPixelTranslation(0x0);

	ASIC_UpdateRAMWithInternalDCSR();
}

static const char *sCartridgeMemPageNames[32] =
{
	"Cartridge Page 0",
	"Cartridge Page 1",
	"Cartridge Page 2",
	"Cartridge Page 3",
	"Cartridge Page 4",
	"Cartridge Page 5",
	"Cartridge Page 6",
	"Cartridge Page 7",
	"Cartridge Page 8",
	"Cartridge Page 9",
	"Cartridge Page 10",
	"Cartridge Page 11",
	"Cartridge Page 12",
	"Cartridge Page 13",
	"Cartridge Page 14",
	"Cartridge Page 15",
	"Cartridge Page 16",
	"Cartridge Page 17",
	"Cartridge Page 18",
	"Cartridge Page 19",
	"Cartridge Page 20",
	"Cartridge Page 21",
	"Cartridge Page 22",
	"Cartridge Page 23",
	"Cartridge Page 24",
	"Cartridge Page 25",
	"Cartridge Page 26",
	"Cartridge Page 27",
	"Cartridge Page 28",
	"Cartridge Page 29",
	"Cartridge Page 30",
	"Cartridge Page 31",
};

/* initialise cartridge pointers */
void    ASIC_InitCart(void)
{
	int     i;

	memset(CartridgeDummyPage, 0x0ff, 16384);

	for (i = 0; i < 32; i++)
	{
		CartridgePages[i] = CartridgeDummyPage;
		CartridgeBlocks[i] = NULL;
	}
	NumCartridgeBlocks = 0;

	for (i = 0; i < 32; i++)
	{
		CartridgeMemoryRanges[i].m_nID = RIFF_FOURCC_CODE('C', 'P', '0' + (i / 10), '0' + (i % 10));
		CartridgeMemoryRanges[i].m_bCPU = FALSE;
		CartridgeMemoryRanges[i].m_bReadOnly = TRUE;
		CartridgeMemoryRanges[i].sName = sCartridgeMemPageNames[i];
		CartridgeMemoryRanges[i].m_nLength = 16384;
		CartridgeMemoryRanges[i].pBase = CartridgePages[i];
		CPC_RegisterMemoryRange(&CartridgeMemoryRanges[i]);
	}


}

/* check if cartridge file is valid, return TRUE if it is, else
false */
BOOL    Cartridge_ValidateCartridge(const unsigned char *pData, const unsigned long FileSize)
{
	RIFF_CHUNK *pHeader = (RIFF_CHUNK *)pData;
	unsigned char *pChunkData;

	/* check header of file has 'RIFF' */
	if (pHeader->ChunkName == RIFF_FOURCC_CODE('R', 'I', 'F', 'F'))
	{
		/* file is a RIFF */

		int FileLength;

		/* get size of file from header chunk */
		FileLength = Riff_GetChunkLength(pHeader);

		/* allow extra data on the end. cpc-power adds this */
		if (FileLength <= (int)(FileSize - sizeof(RIFF_CHUNK)))
		{
			if (Riff_CheckChunkSizesAreValid((unsigned char *)pHeader, FileLength))
			{
				/* get pointer to chunk data */
				pChunkData = Riff_GetChunkDataPtr(pHeader);

				/* is RIFF of type 'AMS!'? but also allow mixed case spelling */
				if (
					((pChunkData[0] == 'A') || (pChunkData[0] == 'a')) &&
					((pChunkData[1] == 'M') || (pChunkData[1] == 'm')) &&
					((pChunkData[2] == 'S') || (pChunkData[2] == 's')) &&
					(pChunkData[3] == '!')
					)
				{
					/* RIFF is a AMS! */
					int nDataBlocksValidSize = 0;
					int nDataBlocks = 0;

					/* check each cart block is of the correct size */

					int i;

					/* we need a number of consecutive chunks to be found to be deemed valid */

					for (i = 0; i < 32; i++)
					{
						RIFF_CHUNK      *pChunk;
						unsigned long CartBlockChunkName;
						int                     ChunkSize;

						/* create chunk name */
						CartBlockChunkName = RIFF_FOURCC_CODE('c', 'b', ((i / 10) + '0'), ((i % 10) + '0'));

						/* find chunk */
						pChunk = Riff_FindNamedSubChunk(pHeader, CartBlockChunkName);

						if (pChunk != NULL)
						{
							nDataBlocks++;

							/* get chunk size */
							ChunkSize = Riff_GetChunkLength(pChunk);

							/* specification says max of 16384 */
							if ((ChunkSize < 0) || (ChunkSize>16384))
							{
								break;
							}

							nDataBlocksValidSize++;
						}
						else
						{
						}
					}

					/* all blocks that were found have valid sizes */
					if ((nDataBlocks!=0) && (nDataBlocks== nDataBlocksValidSize))
					{
						return TRUE;
					}
				}
			}
		}
	}

	return FALSE;
}

/* calc size of cart in blocks */
int             Cartridge_CalcCartSizeInBlocks(int NoOfBlocks)
{
	int i;
	int Pow2 = 1;

	/* (1<<5) = 32 and the max we support */
	for (i = 0; i < 5; i++)
	{
		if (Pow2 >= NoOfBlocks)
		{
			break;
		}

		Pow2 = Pow2 << 1;
	}
	return Pow2;
}


int Cartridge_AttemptInsert(unsigned char *pCartridgeData, unsigned long CartridgeLength)
{
	int Status;

	Status = Cartridge_Insert(pCartridgeData, CartridgeLength);
	if (Status == ARNOLD_STATUS_OK)
	{
		/* refresh language */
		Keyboard_DetectLanguage();

		//printf("Cartridge inserted, starting cartridge\n");
		/* if cartridge was inserted ok, then auto-start it */
		Cartridge_Autostart();
	}


	return Status;
}

const int nPageSize = 16384;

static const unsigned char *pDefaultCartridgeData = NULL;
static unsigned long DefaultCartridgeDataLength = 0;

void Cartridge_SetDefault(const unsigned char *pCartridgeData, const unsigned long CartridgeDataLength)
{
	pDefaultCartridgeData = pCartridgeData;
	DefaultCartridgeDataLength = CartridgeDataLength;
}

void Cartridge_InsertDefault(void)
{
	Cartridge_Insert(pDefaultCartridgeData, DefaultCartridgeDataLength);

	/* refresh language */
	Keyboard_DetectLanguage();

	Cartridge_Autostart();

}


void Cartridge_SetupPage(int nPage, const unsigned char *pData, const unsigned long nDataLength)
{
	unsigned long CopyLength;

	/* if there is too much data, then it will be cut */
	/* if there is not enough data, then pad it with 0x0ff */
	CopyLength = nPageSize;
	if (nDataLength < CopyLength)
	{
		CopyLength = nDataLength;
	}
	CartridgeBlocks[NumCartridgeBlocks] = (unsigned char *)malloc(nPageSize);
	if (CartridgeBlocks[NumCartridgeBlocks] != NULL)
	{
		memset(CartridgeBlocks[NumCartridgeBlocks], 0x0ff, nPageSize);
		memcpy(CartridgeBlocks[NumCartridgeBlocks], pData, CopyLength);
		if (CopyLength < nPageSize)
		{

			/* fill remaining with 0x0ff */
			memset(CartridgeBlocks[NumCartridgeBlocks] + CopyLength, 0x0ff, nPageSize - CopyLength);
		}
	}

	CartridgePages[nPage] = CartridgeBlocks[NumCartridgeBlocks];
	++NumCartridgeBlocks;

	HighestBlockIndex = nPage;
}

/* This code works out the smallest cartridge ROM size that would contain the inserted cpr data,
and then mirrors it through the 512KB range. e.g. insert 32KB cart rom, this mirrors,
or 128KB.

TODO: We need to allow this or have fixed cartridge sizes. With fixed sizes we fill the remainder
with empty data as if the data was written to a fixed size cartridge. A fixed size cartridge
will also mirror, but differently than actual size. e.g. fixed at 512KB means remainder is not
used */

void Cartridge_MirrorPages(void)
{
	int i;
	int CartSizeInBlocks = 0;

	{

		/* the following code works out the size of the cart,
		and then fills in the NULL pointers with other pointers
		to repeat the data */


		CartSizeInBlocks = Cartridge_CalcCartSizeInBlocks((HighestBlockIndex+1));


		/* fill in missing cart pages */
		for (i = CartSizeInBlocks; i < 32; i++)
		{
			if (CartridgePages[i] == CartridgeDummyPage)
			{
				CartridgePages[i] = CartridgeBlocks[i & (CartSizeInBlocks - 1)];
			}
		}

		for (i = 0; i < 32; i++)
		{
			CartridgeMemoryRanges[i].pBase = CartridgePages[i];
		}
	}

}

void Cartridge_InsertBinary(const unsigned char *pData, const unsigned long CartridgeDataLength)
{
	int i;
	int nPages = (CartridgeDataLength + 16383) >> 14;
	if (nPages > 32)
	{
		nPages = 32;
	}

	/* remove old one if it is present */
	Cartridge_RemoveI();

	HighestBlockIndex = 0;
	NumCartridgeBlocks = 0;


	for (i = 0; i < nPages; i++)
	{
		Cartridge_SetupPage(i, pData + (i << 14), CartridgeDataLength - (i << 14));
	}

	Cartridge_MirrorPages();
}

int		Cartridge_Insert(const unsigned char *pCartridgeData, const unsigned long CartridgeDataLength)
{
	unsigned long CartridgeLength;
	const unsigned char *pCartridge;

	//printf("*** Inserting cartridge\n");
	pCartridge = pCartridgeData;
	CartridgeLength = CartridgeDataLength;

	if (pCartridge != NULL)
	{
		//printf("*** Got cartridge data\n");
		/* check cartridge is valid */
		if (Cartridge_ValidateCartridge(pCartridge, CartridgeLength))
		{
			/* get cartridge data blocks */
			RIFF_CHUNK *pHeader = (RIFF_CHUNK *)pCartridge;
			int i;

			//printf("*** Cartridge validated\n");
			/* cartridge is valid */
			HighestBlockIndex = 0;

			NumCartridgeBlocks = 0;

			/* remove old one if it is present */
			Cartridge_RemoveI();

			for (i = 0; i < 32; i++)
			{
				unsigned long CartBlockChunkName = RIFF_FOURCC_CODE('c', 'b', ((i / 10) + '0'), ((i % 10) + '0'));
				RIFF_CHUNK *pChunk;

				/* find cart block chunk */
				pChunk = Riff_FindNamedSubChunk(pHeader, CartBlockChunkName);

				if (pChunk != NULL)
				{
					/* found cart block chunk */
					const unsigned char *pChunkData = Riff_GetChunkDataPtr(pChunk);
					const unsigned long ChunkLength = Riff_GetChunkLength(pChunk);

					Cartridge_SetupPage(i, pChunkData, ChunkLength);
				}
			}
			//printf("*** Cartridge inserted OK\n");


			Cartridge_MirrorPages();

			return ARNOLD_STATUS_OK;
		}
		else
		{
			//printf("Cartridge failed validation - must not be a cartridge\n");
		}
	}
	else
	{
		//printf("Cartridge failed validation - must not be a cartridge\n");
	}

	return ARNOLD_STATUS_INVALID;
}


/*---------------------------------------------------------------------------*/
void Plus_RestartReset(void)
{
	PSG_SetBC2State(&OnBoardAY, 1);

	ASIC_Reset();

	FDC_Reset();

	CRTC_Reset();

	PSG_SetType(&OnBoardAY, PSG_TYPE_AY8912);
	PSG_Reset(&OnBoardAY);

	CPU_Reset();

}

void Plus_RestartPower(void)
{
	int i;

	/* fill it with some data; TODO: Find values hardware seems to use */
	/* for the moment fill with random data */
	for (i = 0; i < 64 * 1024; i++)
	{
		Z80MemoryBase[i] = rand() % 256;
	}

	/* fill extra ram with data. TODO: Find what it initialises to */
	for (i = 0; i < 64 * 1024; i++)
	{
		ASIC_ExtraRam[i] = rand() % 256;
	}

	/* confirmed: sprite pixels are random at power on */
	for (i = 0; i < 16 * 16 * 16; i++)
	{
		ASIC_WriteRamFull(0x04000 + i, rand() & 0x0ff);
	}

	/* setup random sprite coords */
	/* seem to be mostly zero, not always but mostly */
	for (i = 0; i < 16 * 8; i += 8)
	{
		/* X, Y are undefined */
		ASIC_WriteRamFull(0x06000 + i, 0x0);
		ASIC_WriteRamFull(0x06000 + i + 1, 0x0);
		ASIC_WriteRamFull(0x06000 + i + 2, 0x0);
		ASIC_WriteRamFull(0x06000 + i + 3, 0x0);
	}

	/* confirmed: whole palette is effectively random. main colour and sprite colours */
	for (i = 0; i < 64; i += 2)
	{
		ASIC_WriteRamFull(0x06400 + i, rand() & 0x0ff);
		ASIC_WriteRamFull(0x06400 + i + 1, rand() & 0x0ff);
	}

	/* SSA - confirmed: seems to be zero at power on. Do more testing to see if it changes */
	ASIC_WriteRamFull(0x06802, 0);	
	ASIC_WriteRamFull(0x06802 + 1, 0);	

	/* IVR - to be confirmed */
	ASIC_WriteRamFull(0x06805, (rand() & 0x0ff) | 0x01);

	/* DMA */
	for (i = 0; i < 4 * 4; i += 4)
	{
		ASIC_WriteRamFull(0x06c00 + i, rand() & 0x0ff); // to be confirmed
		ASIC_WriteRamFull(0x06c00 + i + 1, rand() & 0x0ff); // to be confirmed.
		ASIC_WriteRamFull(0x06c00 + i + 2, 0); // test shows it's mostly 0; but needs fixing 
	}

	PSG_SetBC2State(&OnBoardAY, 1);

	ASIC_Reset();

	FDC_Power();
	FDC_Reset();

	CRTC_Reset();

	PSG_SetType(&OnBoardAY, PSG_TYPE_AY8912);
	PSG_Power(&OnBoardAY);

	CPU_Power();

}


void Cartridge_Autostart(void)
{
	/* are we in CPC+ mode? */
	if (CPC_GetHardware() != CPC_HW_CPCPLUS)
	{
		/* set CPC+ hardware */
		CPC_SetHardware(CPC_HW_CPCPLUS);
	}

	/* already in CPC+ mode, just reset */
	Computer_RestartPower();
}

void    Cartridge_Remove(void)
{
	Cartridge_RemoveI();
	Cartridge_Autostart();
}

void    Cartridge_RemoveI(void)
{
	int i;

	for (i = 0; i < 32; i++)
	{
		if (CartridgeBlocks[i] != NULL)
		{
			/* free block */
			free(CartridgeBlocks[i]);
			CartridgeBlocks[i] = NULL;

		}
	}

	for (i = 0; i < 32; i++)
	{
		CartridgePages[i] = CartridgeDummyPage;
	}
}

int             ASIC_GateArray_GetPaletteColour(int PenIndex)
{
	return GateArray_State.PenColour[PenIndex];
}

int             ASIC_GateArray_GetSelectedPen(void)
{
	return GateArray_State.PenSelection;
}


int             ASIC_GateArray_GetMultiConfiguration(void)
{
	return GateArray_State.RomConfiguration;
}


void	ASIC_GateArray_SetInterruptLineCount(int Count)
{
	GateArray_State.InterruptLineCount = Count;
}

int		ASIC_GateArray_GetInterruptLineCount(void)
{
	return GateArray_State.InterruptLineCount;
}

BOOL ASIC_GateArray_GetInterruptRequest(void)
{
	return GateArray_State.RasterInterruptRequest;
}

/* This is called when a OUT to Gate Array rom configuration/mode
select register, bit 4 is set */
void    ASIC_GateArray_ClearInterrupt(void)
{
	GateArray_State.RasterInterruptRequest = FALSE;
	GateArray_State.InterruptLineCount = 0;
	ASIC_RefreshRasterInterruptState();
}

/*---------------------------------------------------------------------------*/

/* called when Z80 acknowledges interrupt */
void    ASIC_GateArray_AcknowledgeInterrupt(void)
{
	/* reset top bit of interrupt line counter */
	/* this ensures that the next interrupt is no closer than 32 lines */
	GateArray_State.InterruptLineCount &= 0x01f;

	/* Gate Array interrupt acknowledged; so clear interrupt request */
	GateArray_State.RasterInterruptRequest = FALSE;
}

void	ASIC_GateArray_Update(void)
{

	/* increment interrupt line count */
	GateArray_State.InterruptLineCount++;

	if (GateArray_State.InterruptSyncCount == 0)
	{
		/* if line == 52 then interrupt should be triggered */
		if (GateArray_State.InterruptLineCount == 52)
		{
			/* clear counter. */
			GateArray_State.InterruptLineCount = 0;

			GateArray_State.RasterInterruptRequest = TRUE;

			ASIC_RefreshRasterInterruptState();
		}
	}
	else
	{
		GateArray_State.InterruptSyncCount--;

		/* "2 scans into the VSYNC signal..." */
		if (GateArray_State.InterruptSyncCount == 0)
		{
			/* the case where InterruptLineCount == 0, should be covered by */
			/* code above */

			/* has interrupt line counter overflowed? */
			if (GateArray_State.InterruptLineCount >= 32)
			{
				/* following might not be required, because it is probably */
				/* set by the code above */

				GateArray_State.RasterInterruptRequest = TRUE;
				ASIC_RefreshRasterInterruptState();


			}

			/* reset interrupt line count */
			GateArray_State.InterruptLineCount = 0;
		}
	}
}



void    ASIC_GateArray_Write(int Function)
{
	switch (Function & 0x0c0)
	{
	case 0x000:
	{
		/* function 00xxxxxx */
		GateArray_State.PenSelection = (unsigned char)Function;

		if (Function & 0x010)
		{
			GateArray_State.PenIndex = (unsigned char)16;
		}
		else
		{
			GateArray_State.PenIndex = (unsigned char)(Function & 0x0f);
		}
	}
	break;

	case 0x040:
	{

		/* function 01xxxxxx */

		int     PenIndex;
		int		ColourIndex;

		GateArray_State.ColourSelection = (unsigned char)Function;

		ColourIndex = Function & 0x01f;

		PenIndex = GateArray_State.PenIndex;

		GateArray_State.PenColour[PenIndex] = (unsigned char)ColourIndex;

		/* update ASIC palette ram */
		ASIC_GateArray_UpdatePaletteRam((unsigned char)PenIndex, (unsigned char)ColourIndex);
	}
	break;

	case 0x080:
	{
		/* function 10xxxxxx */

		/* lock state effects access to the I/O port only */
		if (ASIC_Data.Flags & ASIC_ENABLED)
		{
			if ((Function & 0x0e0) == 0x0a0)
			{
				/* function 101xxxxx */
				ASIC_SetSecondaryRomMapping(Function);
				return;
			}
		}

		GateArray_State.RomConfiguration = (unsigned char)Function;

		/* clear interrupt counter? */
		if (Function & 0x010)
		{
			ASIC_GateArray_ClearInterrupt();
			ASIC_ClearPRIInterruptRequest();
		}

		Computer_RethinkMemory();
	}
	break;

	default
		:
			break;
	}
}


const unsigned char *Plus_GetBASICRom(void)
{
	return ASIC_GetCartPage(1);
}

/* set upper rom logical page; convert this into physical page */
void ASIC_ROM_Select(Z80_BYTE Data)
{
	if (Data & (1 << 7))
	{
		/* select cartridge page from lowest 5 bits */
		ASIC_Data.ASIC_Cartridge_Page = Data & 0x01f;
	}
	else
	{
		/* by default BASIC is selected */
		ASIC_Data.ASIC_Cartridge_Page = 1;

		/* confirmed: not for gx4000; page 1 is selected because the appropiate hardware
		is not enabled */
		if (!ASIC_GetGX4000())
		{
			int nRom = 7;

			/* if EXP is low, DISC is selected with 7, else disc is selected with 0 */
			if (!CPC_GetExpLow())
			{
				nRom = 0;
			}

			if (Data == nRom)
			{
				ASIC_Data.ASIC_Cartridge_Page = 3;
			}
		}
	}

	Computer_RethinkMemory();
}


void ASIC_InitialiseMemoryOutputs(MemoryData *pData)
{
	/* TODO: RAMDIS and ROMDIS */
	unsigned char RomConfiguration = ASIC_GateArray_GetMultiConfiguration();

	pData->bRomEnable[7] = ((RomConfiguration & 0x08) == 0);
	pData->bRomEnable[6] = pData->bRomEnable[7];

	pData->bRomEnable[5] = FALSE;
	pData->bRomEnable[4] = FALSE;
	pData->bRomEnable[3] = FALSE;
	pData->bRomEnable[2] = FALSE;
	pData->bRomEnable[1] = FALSE;
	pData->bRomEnable[0] = FALSE;

	/* now set depending on lower rom index and rom enabled state */
	pData->bRomEnable[(ASIC_Data.LowerRomIndex << 1) + 0] = ((RomConfiguration & 0x04) == 0);
	pData->bRomEnable[(ASIC_Data.LowerRomIndex << 1) + 1] = pData->bRomEnable[(ASIC_Data.LowerRomIndex << 1) + 0];

	pData->bRamRead[7] = !pData->bRomEnable[7];
	pData->bRamRead[6] = !pData->bRomEnable[6];

	pData->bRamRead[5] = !pData->bRomEnable[5];
	pData->bRamRead[4] = !pData->bRomEnable[4];

	pData->bRamRead[3] = !pData->bRomEnable[3];
	pData->bRamRead[2] = !pData->bRomEnable[2];

	pData->bRamRead[1] = !pData->bRomEnable[1];
	pData->bRamRead[0] = !pData->bRomEnable[0];

}

void ASIC_HandleASICRam(MemoryData *pData)
{
	if (ASIC_Data.ASICRamEnabled)
	{
		unsigned char *pDummyWriteRam = GetDummyWriteRam();

		/* asic ram is always written to. If asic ram is enabled set the read */
		pData->pReadPtr[2] = ASIC_Data.ASIC_Ram_Adjusted;
		pData->pReadPtr[3] = ASIC_Data.ASIC_Ram_Adjusted;

		/* if expansion ram didn't assert ramdis, then it's our internal ram */
		if (!pData->bRamDisable[2])
		{
			/* write to dummy ram. i.e. no write through */
			pData->pWritePtr[2] = pDummyWriteRam - 0x04000;
		}
		if (!pData->bRamDisable[3])
		{
			pData->pWritePtr[3] = pDummyWriteRam - 0x04000;
		}
	}
}


void ASIC_InitialiseDefaultMemory(MemoryData *pData)
{
	unsigned long Page;
	/* TODO: RAMDIS and ROMDIS */




	/* external ROMs take priority */
	/* then internal ROM */
	/* then ram */
	/* upper rom */
	if ((pData->bRomEnable[6]) && (!pData->bRomDisable[6]))
	{
		unsigned char *pCartPage = ASIC_GetCartPage(ASIC_Data.ASIC_Cartridge_Page) - 0x0c000;
		pData->pReadPtr[7] = pCartPage;
		pData->pReadPtr[6] = pCartPage;
	}

	Page = ASIC_Data.LowerRomIndex << 1;

	/* lower rom */
	if ((pData->bRomEnable[Page]) && (!pData->bRomDisable[Page]))
	{

		unsigned char *pCartPage = ASIC_GetCartPage(ASIC_Data.ASIC_Cartridge_Page_Lower) - (ASIC_Data.LowerRomIndex << 14);
		pData->pReadPtr[Page + 1] = pCartPage;
		pData->pReadPtr[Page + 0] = pCartPage;
	}

	/* TODO: RAMDIS and ROMDIS */
	switch (ASIC_Data.ASIC_RAM_Config & 0x07)
	{
	case 0:
	{
		int i;
		for (i = 0; i < 8; i++)
		{
			if (!pData->bRamDisable[i])
			{
				pData->pWritePtr[i] = Z80MemoryBase;
				if (!pData->bRomEnable[i])
				{
					pData->pReadPtr[i] = pData->pWritePtr[i];
				}
			}
		}
	}
	break;

	case 2:
	{
		int i;
		/* complete switch */
		for (i = 0; i < 4; i++)
		{
			int nPage = i;
			int nRamOffset = (i << 14);
			int nOffset = nRamOffset - (nPage << 14);
			if (!pData->bRamDisable[i << 1])
			{
				pData->pWritePtr[(i << 1) + 0] = ASIC_ExtraRam - nOffset;
				pData->pWritePtr[(i << 1) + 1] = pData->pWritePtr[(i << 1) + 0];
				if (!pData->bRomEnable[(i << 1) + 0])
				{
					pData->pReadPtr[(i << 1) + 0] = pData->pWritePtr[(i << 1) + 0];
				}
				if (!pData->bRomEnable[(i << 1) + 1])
				{
					pData->pReadPtr[(i << 1) + 1] = pData->pWritePtr[(i << 1) + 1];
				}
			}
		}
	}
	break;

	case 1:
	{
		int i;
		for (i = 0; i < 8; i++)
		{
			if (!pData->bRamDisable[i])
			{
				pData->pWritePtr[i] = Z80MemoryBase;
				if (!pData->bRomEnable[i])
				{
					pData->pReadPtr[i] = pData->pWritePtr[i];
				}
			}
		}

		if (!pData->bRamDisable[6])
		{
			/* 4000-7fff only */
			pData->pWritePtr[6] = ASIC_ExtraRam + (3 << 14) - 0x0c000;
			pData->pWritePtr[7] = pData->pWritePtr[6];

			if (!pData->bRomEnable[7])
			{
				pData->pReadPtr[7] = pData->pWritePtr[7];

			}

			if (!pData->bRomEnable[6])
			{
				pData->pReadPtr[6] = pData->pWritePtr[6];
			}
		}
	}
	break;

	case 3:
	{
		int i;
		for (i = 0; i < 8; i++)
		{
			if (!pData->bRamDisable[i])
			{
				pData->pWritePtr[i] = Z80MemoryBase;
				if (!pData->bRomEnable[i])
				{
					pData->pReadPtr[i] = pData->pWritePtr[i];
				}
			}
		}

		if (!pData->bRamDisable[6])
		{
			/* 4000-7fff only */
			pData->pWritePtr[6] = ASIC_ExtraRam + (3 << 14) - 0x0c000;
			pData->pWritePtr[7] = pData->pWritePtr[6];

			if (!pData->bRomEnable[7])
			{
				pData->pReadPtr[7] = pData->pWritePtr[7];
			}

			if (!pData->bRomEnable[6])
			{
				pData->pReadPtr[6] = pData->pWritePtr[6];
			}
		}

		if (!pData->bRamDisable[2])
		{
			pData->pWritePtr[2] = ((Z80MemoryBase + 0x0c000) - 0x04000);
			pData->pWritePtr[3] = pData->pWritePtr[2];

			pData->pReadPtr[2] = pData->pWritePtr[2];
			pData->pReadPtr[3] = pData->pWritePtr[3];
		}

	}
	break;

	case 4:
	case 5:
	case 6:
	case 7:
	{
		int i;
		for (i = 0; i < 8; i++)
		{
			if (!pData->bRamDisable[i])
			{
				pData->pWritePtr[i] = Z80MemoryBase;
				if (!pData->bRomEnable[i])
				{
					pData->pReadPtr[i] = pData->pWritePtr[i];
				}
			}
		}

		if (!pData->bRamDisable[2])
		{
			/* 4000-7fff only */
			pData->pWritePtr[2] = ASIC_ExtraRam + (((ASIC_Data.ASIC_RAM_Config & 0x07) - 4) << 14) - 0x04000;
			pData->pWritePtr[3] = pData->pWritePtr[2];

			pData->pReadPtr[2] = pData->pWritePtr[2];
			pData->pReadPtr[3] = pData->pWritePtr[3];
		}
	}
	break;
	}

	ASIC_HandleASICRam(pData);

#if 0
	int i;
	for (i=0; i<8; i++)
	{
		if (!pData->bRamDisable[i])
		{
			pData->pWritePtr[i] = Z80MemoryBase;
			if (!pData->bRomEnable[i])
			{
				pData->pReadPtr[i] = pData->pWritePtr[i];
			}
		}
	}
#endif

}

static BOOL bHasCassetteInterface = FALSE;

void ASIC_SetHasCassetteInterface(BOOL bState)
{
	bHasCassetteInterface = bState;
}

BOOL ASIC_GetHasCassetteInterface(void)
{
	return bHasCassetteInterface;
}

static BOOL bGX4000 = FALSE;

void ASIC_SetGX4000(BOOL bState)
{
	bGX4000 = bState;
}

BOOL ASIC_GetGX4000(void)
{
	return bGX4000;
}


int	ASIC_GetSecondaryRomMapping(void)
{
	return ASIC_Data.SecondaryRomMapping;
}


/* update asic ram with chosen Gate Array colour */
void	ASIC_GateArray_UpdatePaletteRam(unsigned char PenIndex, unsigned char Colour)
{

	/* write ASIC RGB version of CPC hardware colour
	to ASIC colour registers */
	((unsigned short *)(ASIC_Data.ASIC_Ram + 0x02400 + (PenIndex << 1)))[0] = CPCToASICColours[Colour];

	ASIC_SetColour(PenIndex);
}

void     ASIC_SetSecondaryRomMapping(unsigned char Data)
{
	unsigned char ASIC_State;

	/* store it for snapshot */
	ASIC_Data.SecondaryRomMapping = Data;
	ASIC_State = (unsigned char)(((Data >> 3) & 3));

	if (ASIC_State == 0x03)
	{
		/* ASIC Ram enabled, lower rom 0x0000-0x03fff */
		ASIC_Data.LowerRomIndex = 0;
		ASIC_Data.ASICRamEnabled = TRUE;
	}
	else
	{
		/* 0, 1 or 2 */
		ASIC_Data.LowerRomIndex = ASIC_State;
		ASIC_Data.ASICRamEnabled = FALSE;
	}

	ASIC_Data.ASIC_Cartridge_Page_Lower = (Data & 0x07);


	Computer_RethinkMemory();
}

unsigned char *ASIC_GetRamPtr(void)
{
	return ASIC_Data.ASIC_Ram;
}


unsigned char   ASIC_GetRed(int ColourIndex)
{
	return (unsigned char)((ASIC_Data.ASIC_Ram[0x02400 + (ColourIndex << 1)] & 0x0f0) >> 4);
}

unsigned char ASIC_GetGreen(int ColourIndex)
{
	return (unsigned char)((ASIC_Data.ASIC_Ram[0x02400 + (ColourIndex << 1) + 1] & 0x0f));
}


unsigned char ASIC_GetBlue(int ColourIndex)
{
	return (unsigned char)((ASIC_Data.ASIC_Ram[0x02400 + (ColourIndex << 1)] & 0x0f));
}

unsigned char ASIC_GetSpritePixel(int SpriteIndex, int X, int Y)
{
	unsigned char *pSpriteData = &ASIC_Data.ASIC_Ram[(SpriteIndex << 8)];

	return (unsigned char)(pSpriteData[(Y << 4) + X] & 0x0f);
}

static BOOL ASIC_IsSpriteOnLine(ASIC_SPRITE_RENDER_INFO *pRenderInfo, unsigned long Line)
{
	unsigned long DeltaLines;

	/* calculate delta lines */
	/* sprite coordinate range is &000-&1ff */
	DeltaLines = (Line - pRenderInfo->y) & 0x01ff;

	return (DeltaLines < pRenderInfo->HeightInLines);
}

unsigned long ASIC_BuildDisplayReturnMaskWithPixels(int Line, int HCount, /*int MonitorHorizontalCount,*/ /*int ActualY,*/ int *pPixels)
{
	/* based on line we can find which HW sprites are here */

	unsigned long GraphicsMask = 0;

	if ((ASIC_Data.SpriteEnableMask & ASIC_Data.SpriteEnableMaskOnLine & SpriteOverride) != 0)
	{
		int     i;
		ASIC_SPRITE_RENDER_INFO *pRenderInfo = &ASIC_Data.SpriteInfo[0];

		/* do each sprite in turn (from highest to lowest priority) */
		for (i = 0; i < 16; i++)
		{
			/* is sprite active on this line? */
			if (ASIC_Data.SpriteEnableMaskOnLine & SpriteOverride & (1 << i))
			{
				/* sprite is active on this line */
				unsigned long DeltaColumns;

				/* calculate delta columns */
				/* sprite coordinate range is &000-&3ff, and this corresponds to hcount of &00-&3f */
				DeltaColumns = (HCount - pRenderInfo->MinColumn) & 0x03f;

				/* if column delta is within width in columns; this sprite is visible
				at this hcount */
				if (DeltaColumns < pRenderInfo->WidthInColumns)
				{
					unsigned char   *pSpriteGraphics;
					int                     SpriteX, SpriteY;
					int                     j, SprY;
					int                     XStart;

					pSpriteGraphics = ASIC_Data.ASIC_Ram + (i << 8);

					SpriteY = (Line - pRenderInfo->y) & 0x01ff;

					if (DeltaColumns == 0)
					{
						XStart = pRenderInfo->x & 0x0f;
						SpriteX = 0;
					}
					else
					{
						SpriteX = ((HCount << 4) - pRenderInfo->x) & 0x03ff;
						XStart = 0;
					}

					SprY = SpriteY >> ASIC_Data.SpriteInfo[i].YMagShift;

					/* rendering 16 pixels in one go = 1 CRTC char width */
					for (j = XStart; j < 16; j++)
					{
						/* if no pixel rendered here */
						if ((GraphicsMask & (1 << j)) == 0)
						{
							unsigned short    SprX;


							SprX = (unsigned short)((SpriteX + (j - XStart)) >> ASIC_Data.SpriteInfo[i].XMagShift);

							if (SprX < 16)
							{
								int     Colour;

								Colour = (pSpriteGraphics[(SprY << 4) | SprX] & 0x0f);

								if (Colour != 0)
								{

									pPixels[j] = Colour + 16;
									/* mark this pixel as used */
									GraphicsMask |= (1 << j);
								}
							}
						}
					}

					/* if all are used, quit */
					if (GraphicsMask == 0x00000ffff)
					{
						break;
					}
				}
			}
			pRenderInfo++;
		}
	}
	return (GraphicsMask ^ 0x0ffff);
}


/* screen split */
BOOL    ASIC_RasterSplitLineMatch(int CRTC_LineCounter, int CRTC_RasterCounter)
{
	int SplitLine;
	int CurrLine;

	/* debug, disable screen split? */
	if (!ScreenSplitEnableOverride)
		return FALSE;

	/* if zero, no split is done */
	if (ASIC_Data.ASIC_RasterSplitLine == 0)
	{
		return FALSE;
	}

	/* confirmed: counter wraps. this causes the bug mentioned in the arnold v documentation */
	CurrLine = ((CRTC_LineCounter & 0x01f) << 3) | (CRTC_RasterCounter & 0x07);

	SplitLine = ASIC_Data.ASIC_RasterSplitLine & 0x0ff;

	if (CurrLine == SplitLine)
	{
		return TRUE;
	}

	return FALSE;
}

void ASIC_RefreshRasterInterruptState(void)
{
	/* PRI? */
	BOOL bInterrupt = FALSE;

	/* is raster interrupt line active? */
	if (ASIC_Data.ASIC_RasterInterruptLine != 0)
	{
		bInterrupt = ASIC_Data.ASIC_PRI_Request;
	}
	else
	{
		bInterrupt = GateArray_State.RasterInterruptRequest;
	}

	if (bInterrupt)
	{
		/* confirmed: both PRI and normal CPC style interrupts set bit 7 to indicate a raster interrupt */
		ASIC_Data.InterruptRequest |= 0x080;
	}
	else
	{
		ASIC_Data.InterruptRequest &= ~0x080;
	}

	Computer_RefreshInterrupt();
}

void ASIC_ClearPRIInterruptRequest(void)
{
	ASIC_Data.ASIC_PRI_Request = FALSE;
	ASIC_RefreshRasterInterruptState();
}

/* raster interrupt */
/* at beginning of line AND at Hsync time */
void ASIC_RefreshRasterInterrupt(int CRTC_LineCounter, int CRTC_RasterCounter)
{
	/* is raster interrupt line active? */
	if ((ASIC_Data.ASIC_RasterInterruptLine != 0) && PRIInterruptOverride)
	{
		int SplitLine;
		int CurrLine;


		/* confirmed: 0-255 with normal height screen shows no repeat */
		/* LineCounter is not ANDed with 0x01f, the range is greater */
		CurrLine = ((CRTC_LineCounter & 0x03f) << 3) | (CRTC_RasterCounter & 0x07);
		SplitLine = ASIC_Data.ASIC_RasterInterruptLine & 0x0ff;
		if
			/* line matches? */
			(CurrLine == SplitLine)
		{
			ASIC_Data.ASIC_PRI_Request = TRUE;


			/* trigger int if any sources require an interrupt */
		}
	}
	/* raster interrupt occured */
	ASIC_RefreshRasterInterruptState();

}


BOOL ASIC_GetInterruptRequest(void)
{
	return ((ASIC_Data.InterruptRequest & 0x0f0)!=0);
}

/* calculate a interrupt vector to supply in IM2 or ignore in IM1 based on interrupts active. */
int     ASIC_CalculateInterruptVector(void)
{
	unsigned char      Vector = 0;

	/* priority is raster, dma 2, dma 1 then dma 0 */

	/* lowest priority to highest priority */

	/* is DMA channel 0 interrupt triggered? */
	if (ASIC_Data.InterruptRequest & 0x040)
	{
		Vector = 0x04;
	}

	/* is DMA channel 1 interrupt triggered? */
	if (ASIC_Data.InterruptRequest & 0x020)
	{
		Vector = 0x02;
	}

	/* is DMA channel 2 interrupt triggered? */
	if (ASIC_Data.InterruptRequest & 0x010)
	{
		Vector = 0x00;
	}

	/* is raster interrupt triggered */
	if (ASIC_Data.InterruptRequest & 0x080)
	{
		/* confirmed: raster interrupt always gives a vector of 6,
		regardless of whether the asic has been enabled or not.
		For Chany's Dreamend demo it will fetch at an even address
		which will cause it to jump to the function indicating it's a Plus.
		The demo expects a vector of FF for CPC which is not always true */

		/* raster int line specified */
		Vector = 0x06;
	}

	return ((ASIC_Data.ASIC_InterruptVector & 0x0f8) | Vector);
}

/* clear dma ints by a manual write to DCSR */
void    ASIC_ClearDMAInterruptsManual(int Data)
{
	ASIC_Data.InterruptRequest = (unsigned char)(ASIC_Data.InterruptRequest & ~(Data & (0x07 << 4)));
}

static void ASIC_ClearDMAInterruptsAutomatic(void)
{
	/* priority is 2 then 1 then 0 */

	/* is DMA channel 2 interrupt triggered? */
	if (ASIC_Data.InterruptRequest & 0x010)
	{
		/* clear interrupt */
		ASIC_Data.InterruptRequest = (unsigned char)(ASIC_Data.InterruptRequest & (~0x010));
		return;
	}

	/* is DMA channel 1 interrupt triggered? */
	if (ASIC_Data.InterruptRequest & 0x020)
	{
		/* clear interrupt */
		ASIC_Data.InterruptRequest = (unsigned char)(ASIC_Data.InterruptRequest & (~0x020));
		return;
	}


	/* is DMA channel 0 interrupt triggered? */
	if (ASIC_Data.InterruptRequest & 0x040)
	{
		/* clear interrupt */
		ASIC_Data.InterruptRequest = (unsigned char)(ASIC_Data.InterruptRequest & (~0x040));
		return;
	}
}

/* clear dma interrupts automatically when ints are done, only clears */
/* highest priority DMA int active, so other ints may be done too */
void    ASIC_ClearDMAInterrupts(void)
{
	/* clear dma interrupts in order of priority */
	ASIC_ClearDMAInterruptsAutomatic();

	/* update asic ram with DCSR value */
	ASIC_UpdateRAMWithInternalDCSR();
}

/* this function is called whenever the Z80 acknowledges a interrupt */
Z80_BYTE    ASIC_AcknowledgeInterrupt(void)
{
	/* calculate vector while we have knowledge of what the interrupt source is */
	Z80_BYTE Vector = ASIC_CalculateInterruptVector();

	/* any pri or cpc interrupt? */
	if ((ASIC_Data.InterruptRequest & 0x080) == 0)
	{
		/* not a raster interrupt. Is it a DMA interrupt ? */
		ASIC_Data.RasterInterruptAcknowledged = 0;

		/* confirmed: if vector supplied had bit 0 not set,
		then we automatically clear dma interrupts on a acknowledge */
		/* otherwise they are not cleared and have to be done manually */
		if ((ASIC_Data.ASIC_InterruptVector & 0x001) == 0)
		{
			/* clear the dma interrupt */
			ASIC_ClearDMAInterrupts();
		}
	}
	else
	{
		/* true or always cleared? */
		if (ASIC_Data.ASIC_RasterInterruptLine != 0)
		{
			ASIC_Data.ASIC_PRI_Request = FALSE;
		}

		/* clear raster interrupt request */
		ASIC_Data.InterruptRequest &= ~0x080;

		ASIC_Data.RasterInterruptAcknowledged = 0x080;

		/* clear raster int */
		ASIC_GateArray_AcknowledgeInterrupt();
		ASIC_RefreshRasterInterruptState();
	}

	/* write InternalDCSR to ram */
	ASIC_UpdateRAMWithInternalDCSR();

	Computer_RefreshInterrupt();

	return Vector;
}



/******************************************/
/* ASIC handling code */



static void ASIC_SetupSpriteRenderInfo(int SpriteIndex)
{
	/* write sprite info */

	unsigned char SpriteMag;
	ASIC_SPRITE_RENDER_INFO *pRenderInfo;

	pRenderInfo = &ASIC_Data.SpriteInfo[SpriteIndex];

	SpriteMag = ASIC_Data.Sprites[SpriteIndex].SpriteMag;

	/* is sprite renderable ? */
	/* is XMag!=0 and YMag!=0. For both to be not equal to zero, then they must be
	greater or equal to %0101! */

	if (SpriteMag >= 5)
	{

		/* sprite is renderable */
		unsigned int    XMag, YMag;

		/* get X and Y mag */
		XMag = (SpriteMag >> 2) & 0x003;
		YMag = SpriteMag & 0x003;

		/* enable rendering of sprite */

		pRenderInfo->XMagShift = XMag - 1;
		pRenderInfo->YMagShift = YMag - 1;

		/* get sprite min coordinates */

		pRenderInfo->x = (unsigned short)(ASIC_Data.Sprites[SpriteIndex].SpriteX.SpriteX_W);
		pRenderInfo->y = (unsigned short)(ASIC_Data.Sprites[SpriteIndex].SpriteY.SpriteY_W);

		/* get sprite max coordinates */
		pRenderInfo->HeightInLines = (1 << (YMag - 1 + 4));

		pRenderInfo->WidthInColumns = ((pRenderInfo->x & 0x0f) + 15 + (1 << (XMag - 1 + 4))) >> 4;
		pRenderInfo->MinColumn = (pRenderInfo->x >> 4) & 0x03f;

		ASIC_Data.SpriteEnableMask |= (1 << SpriteIndex);

		/* active on this line? */
		if (ASIC_IsSpriteOnLine(pRenderInfo, CurrentLine))
		{
			/* say it's active in the mask */
			ASIC_Data.SpriteEnableMaskOnLine |= (1 << SpriteIndex);

		}
	}
	else
	{

		ASIC_Data.SpriteEnableMask &= ~(1 << SpriteIndex);

		/* active on this line */
		/*				if (ASIC_IsSpriteOnLine(pRenderInfo, CurrentLine)) */
		{
			/* say it's active in the mask */
			ASIC_Data.SpriteEnableMaskOnLine &= ~(1 << SpriteIndex);
		}

	}
}

void ASIC_UpdateInternalDCSR(void)
{
	ASIC_Data.InternalDCSR = ASIC_Data.RasterInterruptAcknowledged | ASIC_Data.DMAChannelEnables | (ASIC_Data.InterruptRequest & 0x070);
}

unsigned char ASIC_GetDCSR(void)
{
	return ASIC_Data.InternalDCSR;
}


unsigned short ASIC_GetSpriteX(int SpriteIndex)
{
	return (unsigned short)ASIC_Data.Sprites[SpriteIndex].SpriteX.SpriteX_W;
}

unsigned short ASIC_GetSpriteY(int SpriteIndex)
{
	return (unsigned short)ASIC_Data.Sprites[SpriteIndex].SpriteY.SpriteY_W;

}

unsigned char ASIC_GetSpriteMagnification(int SpriteIndex)
{
	return ASIC_Data.Sprites[SpriteIndex].SpriteMag;
}


/* data will have already been poked into ram */
void    ASIC_WriteRam(int Addr, int Data)
{
	if ((Addr & 0x0c000) != 0x04000)
	{
		return;
	}

	Addr = Addr & 0x03fff;

	if ((Addr & 0x0f000) == 0x00000)
	{
		/* write to sprite ram */

		/* remove upper nibble from sprite data */
		ASIC_Data.ASIC_Ram[Addr/* & 0x0fff*/] = (unsigned char)(Data & 0x0f);
		return;
	}

	if ((Addr & 0x03f80) == 0x02000)
	{
		int SpriteIndex = (Addr & 0x078) >> 3;

		switch (Addr & 0x07)
		{
		case 0:
		{
			if (ASIC_Data.Sprites[SpriteIndex].SpriteX.SpriteX_B.l == Data)
			{
				return;
			}

			/* set X coordinate low byte */
			ASIC_Data.Sprites[SpriteIndex].SpriteX.SpriteX_B.l = (unsigned char)Data;

			/* mirror */
			ASIC_Data.ASIC_Ram[Addr + 4] = Data;
		}
		break;

		case 1:
		{
			/* if bit 0 = 1 and bit 1 = 1, then reading this byte will return 0x0ff */
			/* otherwise bit 7-2 are forced to zero */

			unsigned char LocalData = Data & 0x03;
			unsigned char PokeData;

			PokeData = LocalData;
			if (PokeData == 3)
			{
				PokeData = 0x0ff;
			}

			/* change value in ram and mirror */
			ASIC_Data.ASIC_Ram[Addr] = (ASIC_Data.ASIC_Ram[Addr + 4] = PokeData);

			/* move this ahead if possible to speed things up a bit */
			if (ASIC_Data.Sprites[SpriteIndex].SpriteX.SpriteX_B.h == LocalData)
			{
				return;
			}

			/* set X coordinate high byte */
			ASIC_Data.Sprites[SpriteIndex].SpriteX.SpriteX_B.h = LocalData;

		}
		break;

		case 2:
		{
			if (ASIC_Data.Sprites[SpriteIndex].SpriteY.SpriteY_B.l == Data)
			{
				return;
			}

			/* set Y coordinate low byte */
			ASIC_Data.Sprites[SpriteIndex].SpriteY.SpriteY_B.l = (unsigned char)Data;

			/* mirror */
			ASIC_Data.ASIC_Ram[Addr + 4] = Data;
		}
		break;

		case 3:
		{

			/* if bit 0 = 1 then reading this byte will return 0x0ff */
			/* otherwise bit 7-1 are forced to zero */

			unsigned char LocalData = Data & 0x01;
			unsigned char PokeData;

			PokeData = LocalData;
			if (PokeData != 0)
			{
				PokeData = 0x0ff;
			}

			ASIC_Data.ASIC_Ram[Addr] = (ASIC_Data.ASIC_Ram[Addr + 4] = PokeData);


			/* watch out if this is moved; if bit 0 is set, then 0x0ff must be poked into
			ASIC ram!*/
			if (ASIC_Data.Sprites[SpriteIndex].SpriteY.SpriteY_B.h == LocalData)
			{
				return;
			}

			/* set Y coordinate high byte */
			ASIC_Data.Sprites[SpriteIndex].SpriteY.SpriteY_B.h = (unsigned char)LocalData;


		}
		break;


		default
			:
		{
			if (ASIC_Data.Sprites[SpriteIndex].SpriteMag == (Data & 0x0f))
			{
				/* offset 4, mirrors offset 0,
				offset 3, mirrors offset 1.. */
				ASIC_Data.ASIC_Ram[Addr] = ASIC_Data.ASIC_Ram[Addr & 0x03ffb];
				return;
			}

			/* store sprite magnification */
			ASIC_Data.Sprites[SpriteIndex].SpriteMag = (unsigned char)(Data & 0x0f);

			/* offset 4, mirrors offset 0,
			offset 3, mirrors offset 1.. */
			ASIC_Data.ASIC_Ram[Addr] = ASIC_Data.ASIC_Ram[Addr & 0x03ffb];
		}
		break;


		}

		/* update sprite render information */
		ASIC_SetupSpriteRenderInfo(SpriteIndex);

		return;
	}

	if ((Addr & 0x0fff8) == 0x02800)
	{
		switch (Addr & 0x07)
		{
		case 0:
		{
			ASIC_Data.ASIC_RasterInterruptLine = (unsigned char)Data;
			return;
		}

		case 1:
		{
			ASIC_Data.ASIC_RasterSplitLine = (unsigned char)Data;
			return;
		}


		case 2:
		{
			ASIC_Data.ASIC_SecondaryScreenAddress.Addr_B.h = (unsigned char)Data;
			return;
		}

		case 3:
		{
			ASIC_Data.ASIC_SecondaryScreenAddress.Addr_B.l = (unsigned char)Data;
			return;
		}

		case 4:
		{
			ASIC_Data.ASIC_SoftScroll = (unsigned char)Data;

			ASICCRTC_SetSoftScroll(Data);
			return;
		}


		case 5:
		{
			/* interrupt vector supplied by ASIC */
			ASIC_Data.ASIC_InterruptVector = (unsigned char)Data;
			return;
		}

		default
			:
				return;

		}
	}

	/* analogue input channels */
	if ((Addr & 0x03ff8) == 0x02808)
	{
		ASIC_Data.ASIC_Ram[Addr] = ASIC_Data.AnalogueInputs[Addr & 0x07];
		return;
	}


	/* write colour palette */
	if ((Addr & 0x0ffc0) == 0x02400)
	{
		int             Index;

		Addr = Addr & 0x03ffe;
		Index = (Addr & 0x003f) >> 1;

		/* upper nibble is reset to 0 */
		ASIC_Data.ASIC_Ram[Addr + 1] &= 0x00f;

		ASIC_SetColour(Index);
		return;
	}

	if ((Addr & 0x0fff0) == 0x02c00)
	{
		if (Addr == 0x02c0f)
		{
			/* writing 1 to DMA int bits */

			/* clear dma interrupts */
			ASIC_ClearDMAInterruptsManual(Data);

			ASIC_Data.DMAChannelEnables = Data & 0x07;

			ASIC_UpdateRAMWithInternalDCSR();

			Computer_RefreshInterrupt();

			return;
		}
		else
		{
			int ChannelIndex = (Addr & 0x0f) >> 2;

			switch (Addr & 0x03)
			{
			case 0:
			{
				ASIC_Data.DMA[ChannelIndex].Addr.Addr_B.l = (unsigned char)(Data & 0x0fe);
			}
			break;

			case 1:
			{
				ASIC_Data.DMA[ChannelIndex].Addr.Addr_B.h = (unsigned char)Data;
			}
			break;

			case 2:
			{
				ASIC_Data.DMA[ChannelIndex].Prescale = (unsigned char)Data;
				ASIC_Data.DMAChannel[ChannelIndex].PrescaleCount = 0;
			}
			break;
			}

			ASIC_UpdateRAMWithInternalDCSR();

			return;
		}
	}
}


/********************/
/* ASIC SOUND "DMA" */
/********************/

 static Z80_WORD ASIC_DMA_GetOpcode(Z80_WORD Addr)
{
	return CPU_RD_BASE_WORD(Addr);
}

int ASIC_DMA_GetChannelPrescale(int ChannelIndex)
{
	/* get pre-scalar */
	return (ASIC_Data.DMA[ChannelIndex].Prescale);
}


int ASIC_DMA_GetChannelPrescaleCount(int ChannelIndex)
{
	/* get pre-scalar */
	return (ASIC_Data.DMAChannel[ChannelIndex].PrescaleCount);
}

int ASIC_DMA_GetChannelAddr(int ChannelIndex)
{
	return (ASIC_Data.DMA[ChannelIndex].Addr.Addr_W);
}

void ASIC_DMA_SetChannelAddr(int ChannelIndex, int Address)
{
	ASIC_Data.DMA[ChannelIndex].Addr.Addr_W = Address & 0x0fffe;
}

 static void ASIC_DMA_WriteChannelAddr(int ChannelIndex, int Addr)
{
	ASIC_Data.DMA[ChannelIndex].Addr.Addr_W = (unsigned short)Addr;
}

void ASIC_DMA_FetchOpcode(int ChannelIndex)
{
	int     Addr;
	ASIC_DMA_CHANNEL        *pChannel = &ASIC_Data.DMAChannel[ChannelIndex];

	Addr = ASIC_Data.DMA[ChannelIndex].Addr.Addr_W;

	pChannel->Instruction = ASIC_DMA_GetOpcode((Z80_WORD)(Addr & 0x0fffe));
	
	Addr += 2;

	ASIC_DMA_WriteChannelAddr(ChannelIndex, Addr);
}

void    ASIC_DMA_ExecuteCommand(int ChannelIndex)
{
	Z80_WORD        Command;
	int CommandOpcode;
	ASIC_DMA_CHANNEL        *pChannel = &ASIC_Data.DMAChannel[ChannelIndex];

	Command = pChannel->Instruction;
	CommandOpcode = (Command & 0x07000) >> 12;

	if (CommandOpcode == 0)
	{
		/* LOAD R,D */

		int     Register;
		int     Data;

		/* PSG register */
		Register = (Command >> 8) & 0x0f;
		/* data to write */
		Data = (Command & 0x0ff);
		/* store values ready to load into ay*/
		ASIC_Data.DmaAyRegister = Register;
		ASIC_Data.DmaAyData = Data;
		/* now do sequence */
		ASIC_Data.DmaPhase = DMA_PHASE_SELECT_AY_REGISTER;
	}
	else
	{
		if (CommandOpcode & 0x01)
		{
			/* PAUSE n */

			/* confirmed:
			when prescale = 0:
			PAUSE 0 = PAUSE 1 = NOP.
			when prescale != 0:
			PAUSE 0 = NOP
			PAUSE 1..n = uses prescale
			*/

			int PauseCount = Command & 0x0fff;

			if (PauseCount != 0)
			{
				pChannel->PauseCount = PauseCount;
				
				pChannel->PrescaleCount = ASIC_Data.DMA[ChannelIndex].Prescale;
				/* mark it as paused */
				ASIC_Data.DMAChannelPaused |= (1 << ChannelIndex);
			}
		}

		if (CommandOpcode & 0x02)
		{
			/* REPEAT n */
			int     Addr;

			/* store repeat count */
			pChannel->RepeatCount = Command & 0x0fff;
			if (pChannel->RepeatCount != 0)
			{
				pChannel->RepeatCount--;
			}
			Addr = ASIC_Data.DMA[ChannelIndex].Addr.Addr_W;
			
			/* confirmed: loop is set regardless of repeat count */
			
			/* set next instruction as loop start */
			pChannel->LoopStart = Addr & 0x0fffe;
		}

		if (CommandOpcode & 0x04)
		{
			/* NOP, LOOP, INT, STOP */

			if (Command & 0x0001)
			{
				/* LOOP */

				/* if loop count is 0, this acts like a NOP */

				/* check repeat count */
				if (pChannel->RepeatCount != 0)
				{
					int     Addr;

					/* decrement count */
					pChannel->RepeatCount--;

					/* reload channel addr from stored loop start */
					Addr = pChannel->LoopStart;

					ASIC_DMA_WriteChannelAddr(ChannelIndex, Addr);
				}


			}

			if (Command & 0x0010)
			{
				/* INT */

				/* set channel interrupt */
				ASIC_Data.InterruptRequest |= 1 << (6 - ChannelIndex);

				/* we are requesting an interrupt so trigger one, not sure exactly what happens for channel 2 then 1 then 0 */
				/* TODO: Check which order we get the interrupt in */
				Computer_RefreshInterrupt();

			}

			if (Command & 0x0020)
			{
				/* STOP */

				/* stop channel */
				ASIC_Data.DMAChannelEnables &= ~(1 << ChannelIndex);
			}
		}
	}


	/*        ASIC_Data.DMA[ChannelIndex].Addr.Addr_W=(unsigned short)Addr; */


	ASIC_UpdateRAMWithInternalDCSR();

}

/* return TRUE if raster ints are enabled, FALSE if not */
BOOL    ASIC_RasterIntEnabled(void)
{
	if (ASIC_Data.ASIC_RasterInterruptLine != 0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


BOOL    ASIC_SpritesActive(void)
{
	if (ASIC_Data.SpriteEnableMask != 0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void	ASIC_GenerateSpriteActiveMaskForLine(void)
{
	unsigned long Line;
	unsigned long SpriteActiveMask = 0;
	int i;
	unsigned long EnableTestMask = (1 << 15);
	Line = ASICCRTC_Line;

	CurrentLine = Line;

	{
		ASIC_SPRITE_RENDER_INFO *pRenderInfo = &ASIC_Data.SpriteInfo[15];

		/* do each sprite in turn */
		for (i = 15; i >= 0; i--)
		{
			/* enabled ? */
			if (ASIC_Data.SpriteEnableMask & EnableTestMask)
			{
				/* active on this line */
				if (ASIC_IsSpriteOnLine(pRenderInfo, CurrentLine))
				{
					/* say it's active in the mask */
					SpriteActiveMask |= EnableTestMask;
				}
			}

			pRenderInfo--;
			EnableTestMask = EnableTestMask >> 1;
		}
	}

	/* return mask */
	ASIC_Data.SpriteEnableMaskOnLine = SpriteActiveMask;

	/*		return SpriteActiveMask; */
}

void    ASIC_InitialiseMonitorColourModes(void)
{
	int i;

	for (i = 0; i < 4096; i++)
	{
		float Red, Green, Blue;
		float Luminance;

		Red = (unsigned char)((i >> 4) & 0x0f) / 15.0f;
		Green = (unsigned char)((i >> 8) & 0x0f) / 15.0f;
		Blue = (unsigned char)((i)& 0x0f) / 15.0f;

		/* colours are a bit weaker than CPC */
		/* CPC: 900mv Plus: 850mv */
		/* compensate for that here */
		/* not clear if this applies to luminance or not */
		Red = Red * (850.0f / 900.0f);
		Green = Green * (850.0f / 900.0f);
		Blue = Blue * (850.0f / 900.0f);

		/* MM14 monitor actually uses Plus LUM output */
		/* Green is brighter than red which is brighter than blue */
		/* 9:3:1 */
		/* vout = vin r2/(r1+r2) */
		/* Blue: R2 = 91, R1= 910 */
		/* Red: R2 = 330, R1 = 680 */
		/* uses a voltage divider resistor setup */
		Luminance = ((Blue * 91.0f) / (91.0f + 910.0f)) + ((Red * 330.0f) / (330.0f + 680.0f)) + Green;

		/* luminance will be greater than 1; this is the max I am seeing but there is more going on here in the schematic */
		Luminance /= 1.42f;
		/* 0-0.95 */
		/* From Arnold V documentation:
		The MM12 monochrome incorporates a 12" paper white tube, similar to
		that used on the PCW9512.

		The input will be the same as the earlier GTM65 versions, i.e. impedance 470 ohms to 0V,
		analogue voltage input which is linear between 0.8V (Black)and 1.75V (Peak white). */

		//		//printf("Luminance: %f\n", Luminance);
		Luminance = Luminance*255.0f;
		Luminance = floorf(Luminance);
		Luminances[i] = (int)Luminance;
		//	//printf("Luminance (int): %d\n", Luminances[i]);
	}
}


void    ASIC_SetMonitorColourMode(CPC_MONITOR_TYPE_ID MonitorMode)
{
	int i;

	switch (MonitorMode)
	{
	case CPC_MONITOR_COLOUR:
	{
		/* from arnold v:
		The new monitor must present an input impedance of 100 ohms to 0V, and accept an analogue input current of 0-10mA for each gun. The levels shall be defined such that 0mA is black and 10mA is full on. The response must be linear between these limits. */
		for (i = 0; i < 4096; i++)
		{
			float Red, Green, Blue;

			/* R,G,B that would be represented by this colour index */
			/* convert to 0-1.0f; assume linear dac */
			Red = ((i >> 4) & 0x0f) / 15.0f;
			Green = ((i >> 8) & 0x0f) / 15.0f;
			Blue = (i & 0x0f) / 15.0f;

			/* colours are a bit weaker than CPC */
			/* CPC: 900mv Plus: 850mv */
			/* compensate for that here */
			Red = Red * (850.0f / 900.0f);
			Green = Green * (850.0f / 900.0f);
			Blue = Blue * (850.0f / 900.0f);

			/* convert to int r,g,b range */
			ASIC_DisplayColours[i].u.element.Red = (int)floorf(Red*255.0f);
			ASIC_DisplayColours[i].u.element.Green = (int)floorf(Green*255.0f);
			ASIC_DisplayColours[i].u.element.Blue = (int)floorf(Blue*255.0f);
		}

		for (i = 0; i < 4096; i++)
		{
			Monitor_AdjustRGBForDisplay(&ASIC_DisplayColours[i]);
		}
	}
	break;

	case CPC_MONITOR_GT64:
	{
		for (i = 0; i < 4096; i++)
		{
			int AdjustedLuminance = Monitor_AdjustLuminanceForDisplay(Luminances[i]);
			/* does green show any red/blue at all? results depends on monitor */
			ASIC_DisplayColours[i].u.element.Red = 0;
			ASIC_DisplayColours[i].u.element.Green = AdjustedLuminance;
			ASIC_DisplayColours[i].u.element.Blue = 0;
		}
	}
	break;

	case CPC_MONITOR_MM12:
	{
		/* The MM12 monochrome incorporates a 12" paper white tube, similar to that used on the PCW9512.

			The input will be the same as the earlier GTM65 versions, i.e.impedance 470 ohms to 0V, analogue voltage input which is linear between 0.8V (Black)and 1.75V (Peak white). */
		for (i = 0; i < 4096; i++)
		{
			/* actual colour depends on monitor */
			int AdjustedLuminance = Monitor_AdjustLuminanceForDisplay(Luminances[i]);
			ASIC_DisplayColours[i].u.element.Red = AdjustedLuminance;
			ASIC_DisplayColours[i].u.element.Green = AdjustedLuminance;
			ASIC_DisplayColours[i].u.element.Blue = AdjustedLuminance;
		}
	}
	break;
	}


	ASIC_UpdateColours();
}


void	ASIC_UpdateColours(void)
{
	int i;

	/* update colours using colour scale */
	for (i = 0; i < 32; i++)
	{
		ASIC_SetColour(i);
	}

	Render_SetBlack(&ASIC_DisplayColours[0x0]);
}

void ASIC_Pal_Reset(void)
{
	ASIC_Data.ASIC_RAM_Config = 0;
}

void ASIC_Pal_Write(int Data)
{
	if ((Data & 0x0c0) == 0x0c0)
	{
		ASIC_Data.ASIC_RAM_Config = Data;
		Computer_RethinkMemory();
	}
}

/*--------------------------------------------------------------*/

int ASIC_PPI_GetPortInput(int nPort)
{
	switch (nPort)
	{
	case 0:
		return PSG_Read(&OnBoardAY);

	case 1:
	{
		/* 0x5e on 464+ */
		/* no tape read if nothing in and running */
		/* centronics busy if nothing connected */
		/* ppi expansion port = 0 */
		int Data = 0;

		/* set computer name; also on gx4000 */
		Data |= ((CPC_GetComputerNameIndex() & 0x07) << 1);

		/* set screen refresh; also on gx4000 */
		if (CPC_Get50Hz())
		{
			Data |= PPI_SCREEN_REFRESH_50HZ;
		}

		/* not connected on gx4000 */
		if (!ASIC_GetGX4000())
		{
			if (!CPC_GetExpLow())
			{
				Data |= PPI_EXPANSION_PORT;
			}
		}
		else
		{
			/* high on gx4000 */
			Data |= PPI_EXPANSION_PORT;
		}

		/* not connected on gx4000 */
		if (!ASIC_GetGX4000())
		{
			if (Printer_GetBusyState())
			{
				Data |= PPI_CENTRONICS_BUSY;
			}
		}
		else
		{
			/* high on GX4000 */
			Data |= PPI_CENTRONICS_BUSY;
		}

		/* set state of vsync bit; also on gx4000 */
		if (CRTC_GetVsyncOutput() != 0)
		{
			Data |= (1 << 0);
		}

		/* tape not on gx4000 */
		if (!ASIC_GetGX4000() && ASIC_GetHasCassetteInterface())
		{
			if (Computer_GetTapeRead())
			{

				Data |= (1 << 7);
			}
		}
		else
		{
			/* high on gx4000 */
			Data |= PPI_TAPE_READ_DATA;
		}

		return Data;
	}

	case 2:
		return 0x0ff;
	}
	return 0x0ff;
}

extern int SelectedKeyboardLine;
/*------------------------------------------------------------------------*/

void ASIC_PPI_SetPortOutput(int nPort, int Data)
{
	switch (nPort)
	{

	case 0:
		PSG_Write(&OnBoardAY, Data);
		break;

	case 1:
		break;

	case 2:
	{

		/* bits 3..0 are keyboard */
		SelectedKeyboardLine = Data & 0x0f;

		/* bit 5 is tape write */
		/* not connected on gx4000 */
		if (!ASIC_GetGX4000())
		{
			Computer_SetTapeWrite((Data & (1 << 5)));
			
			Computer_SetTapeMotor((Data & (1 << 4)) ? TRUE : FALSE);
		}

		/* bit 6,7 are PSG control */
		PSG_SetBDIRState(&OnBoardAY, Data & (1 << 7));
		PSG_SetBC1State(&OnBoardAY, Data & (1 << 6));
		PSG_RefreshState(&OnBoardAY);

		PSG_Write(&OnBoardAY, ASIC_PPI_GetOutputPort(0));
	}
	break;
	}
}

/*------------------------------------------------------------------------*/

int ASIC_PSG_GetPortInputs(int Port)
{
	if (Port == 0)
	{
		return Keyboard_Read();
	}

	return 0x0ff;
}



void	CPCPLUS_Out(const Z80_WORD Port, const Z80_BYTE Data)
{
	if ((Port & 0x0c000) == 0x04000)
	{
		/* gate array cannot be selected if CRTC is also
		selected */
		ASIC_GateArray_Write(Data);
	}


	if (ASIC_R128)
	{
		/* TODO: Confirm decoding */
		if ((Port & 0x08000) == 0x00000)
		{
			/* RAM expansion PAL16L8 write in CPC6128*/
			ASIC_Pal_Write(Data);
		}
	}

	if ((Port & 0x04000) == 0)
	{
		/* crtc selected */

		unsigned int            Index;

		Index = (Port >> 8) & 0x03;

		switch (Index)
		{
		case 0:
		{
			CRTC_RegisterSelect(Data);
			/* data is passed to ASIC for enable/disable detection */
			ASIC_EnableDisable(Data);
		}
		break;

		case 1:
		{
			CRTC_WriteData(Data);
		}
		break;

		default
			:
				break;
		}
	}

	/* to be checked */
	if ((Port & 0x02000) == 0)
	{
		ASIC_ROM_Select(Data);
	}

	if ((Port & 0x01000) == 0)
	{
		if (!ASIC_GetGX4000())
		{
			CPC_PrinterWrite(Data);

			/* bit 8 of data for cpc+ is handled elsewhere */
		}
	}

	if ((Port & 0x0800) == 0)
	{
		unsigned int            Index;

		Index = (Port & 0x0300) >> 8;
		if (Index == 3)
		{
			ASIC_PPI_WriteControl(Data);
		}
		else
		{
			ASIC_PPI_WritePort(Index, Data);
		}
	}

	if (ASIC_R129)
	{
		if ((Port & 0x0480) == 0)
		{
			Amstrad_DiscInterface_PortWrite(Port, Data);
		}
	}
}

/* In of printer on CPC+? */
Z80_BYTE        CPCPlus_In(Z80_WORD Port)
{
	/* CPC6128+ gives 0x079; 6128 may OR on a 1 to be tested */
	Z80_BYTE Data = CPU_GetDataBus();

	if ((Port & 0x0800) == 0)
	{
		unsigned int            Index;

		Index = (Port & 0x0300) >> 8;
		if (Index == 3)
		{
			Data = ASIC_PPI_ReadControl();
		}
		else
		{

			Data = ASIC_PPI_ReadPort(Index);
		}
	}


	if (ASIC_R129)
	{
		if ((Port & 0x0480) == 0)
		{
			Z80_BYTE DeviceData = 0x0ff;
			if (Amstrad_DiscInterface_PortRead(Port, &DeviceData))
			{
				Data = DeviceData;
			}
		}
	}


	if ((Port & 0x04000) == 0)
	{
		unsigned int            Index;

		Index = (Port & 0x0300) >> 8;

		switch (Index)
		{
		case 2:
		{
			Data = CRTC_ReadStatusRegister();
		}
		break;

		case 3:
		{
			Data = CRTC_ReadData();
		}
		break;
		}
	}


	CPC_ExecuteReadPortFunctions(Port, &Data);

	if ((Port & 0x0c000) == 0x04000)
	{
		ASIC_GateArray_Write(Data);
	}

	/* handle ports that write data when in is done */
	if ((Port & 0x04000) == 0)
	{
		unsigned int            Index;

		Index = (Port & 0x0300) >> 8;

		switch (Index)
		{
		case 0:
		{
			CRTC_RegisterSelect(Data);
		}
		break;

		case 1:
		{
			CRTC_WriteData(Data);
		}
		break;
		}
	}

	return (Z80_BYTE)Data;
}

