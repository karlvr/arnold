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

/* SYNC to monitor: */
/* aleste allows 2 joysticks */

/* TODO: Aleste with no roms */
#include "cpcglob.h"
#include "cpc.h"
#include "aleste.h"
#include "psg.h"
#include "mc146818.h"
#include "crtc.h"
#include "monitor.h"
#include "render.h"
#include "i8255.h"
#include "i8253.h"
#include "fdc.h"
#include <math.h>
#include "printer.h"
#include "fdi.h"
#include "monitor.h"

/* TODO: Check luminances */
#define ALESTE_BLACK_BLANKING (1<<3)

i8253 timer;
/* pen number */
static int Aleste_NColor;
/* colour */
static int Aleste_CurrentColour;
/* value written */
static int Aleste_Colours[17];
/* hw colour*/
static int Aleste_HwColours[17];

/* mode etc */
static int Aleste_Mode;
/* mapper */
static int Aleste_Mapper[4];

static int Aleste_UpperRomIndex;
extern AY_3_8912 OnBoardAY;

int Aleste_GetCurrentPen(void)
{
	return Aleste_NColor;
}

int Aleste_GetPenColour(int nIndex)
{
	return Aleste_Colours[nIndex];
}

int Aleste_GetMFForSnapshot(void)
{
	return Aleste_Mode & 0x0f;
}

int Aleste_GetMF(void)
{
	return Aleste_Mode;
}

int Aleste_GetMapper(int nIndex)
{
	return Aleste_Mapper[nIndex];
}

/*
  D0 - VRAM Bank 0/1 (64K) (HIGHTY signal)
  D1 - MODE 0-Norm ,1-High resolution (HIGHTX signal)
  D2 - MAPMOD 0-Amstrad ,1-Yamaha mapper (MAPMOD signal)
  D3 - Black screen (BLAKS signal)
  D4 - 580WI53 (CS53 signal)
  D5 - 0-AY8910 ,1-512WI1 (CSAY signal)
  */
static int Aleste_Extport;

int Aleste_GetExtport(void)
{
	return Aleste_Extport;
}

static int Aleste_BlankingOutput;
static int Aleste_CRTCSyncInputs;

static int InterruptLineCount = 0;
static BOOL InterruptRequest = FALSE;
static int InterruptSyncCount;

static unsigned char *Aleste_pRAM;

/* PROM D28 */
static const unsigned char *Aleste_pMapper;

/* RFCOLDAT ROM */
static const unsigned char *Aleste_pRfColDat;

/* ROMRAM ROM */
static const unsigned char *Aleste_pRomRam;

/* AL512 - MAIN ROM */
static const unsigned char *Aleste_Al512;

const unsigned char *Aleste_GetOSRom(void)
{
	/* page 1 of the rom, page 0 is OS */
	return &Aleste_Al512[0];
}


const unsigned char *Aleste_GetBASICRom(void)
{
	/* page 1 of the rom, page 0 is OS */
	return &Aleste_Al512[16384];
}

/* base hardware colours */
static RGBCOLOUR Aleste_HardwareColoursDisplayed[64];
static RGBCOLOUR Aleste_HardwareColoursColour[64];
static int Luminances[64];

void Aleste_MapperUpdate(void);

void Aleste_UpdateGraphicsFunction(void);

void Aleste_Init(void)
{
	int i;

	Aleste_pRAM = (unsigned char *)malloc(512 * 1024);
	memset(Aleste_pRAM, 0x0ff, 512 * 1024);

	for (i = 0; i < 64; i++)
	{
		/* bits 5,4 : B
		bits 3,2: G
		bits 1,0: R */

		int R = (i & 0x03);
		int G = (i >> 2) & 0x03;
		int B = (i >> 4) & 0x03;

		/* to be determined */
		int Lookup[4] = { 0x00, 0x03f, 0x0bf, 0x0ff };
		int ROut = Lookup[R];
		int GOut = Lookup[G];
		int BOut = Lookup[B];

		/* guess, need actual resistor values */
		int Luminance = (int)floor((ROut*3.03f) + (GOut*10.0f) + BOut);

		Aleste_HardwareColoursColour[i].u.element.Red = ROut;
		Aleste_HardwareColoursColour[i].u.element.Green = GOut;
		Aleste_HardwareColoursColour[i].u.element.Blue = BOut;
		Aleste_HardwareColoursColour[i].u.element.pad0 = 0;
		/* TODO check luminances are in range! */

		Luminances[i] = Luminance;
	}

	// init depending on monitor mode
	//memcpy(&Aleste_HardwareColoursDisplayed, &Aleste_HardwareColoursColour, 64*sizeof(unsigned long));

}


static unsigned char *pVRAMAddr;
static unsigned int VRAMAddr;
static unsigned short PixelData;

int Aleste_GetVRAMAddr(void)
{
	return VRAMAddr;
}

/* 108 total, 80 displayed */
/* 128 total if really x2 */
/* 80% */
void Aleste_CalcVRAMAddr(void)
{
	unsigned long Addr = 0;
	unsigned int  LocalMA = 0;

	switch (Aleste_Extport & 3)
	{
	case 0:
	{
		/*
		  A16: VRAM bank
		  A15: MA13
		  A14: MA12
		  A13: RA2
		  A12: RA1
		  A08: MA7
		  A11: RA0
		  A10: MA9
		  A09: MA8
		  A08: MA7
		  A07: MA6
		  A06: MA5
		  A05: MA4
		  A04: MA3
		  A03: MA2
		  A02: MA1
		  A01: MA0
		  A00: CCLK
		  */

		LocalMA = CRTC_GetMAOutput() << 1;
		Addr = ((LocalMA & 0x06000) << 1) | (LocalMA & 0x07ff);
		/* MA0-MA9, MA12, MA13 used */
		/* RA0-RA7 used */
		Addr |= (CRTC_GetRAOutput() & 0x07) << 11;

	}
	break;

	/* highty only */
	case 1:
	{

	}
	break;

	/* hightx only */
	case 2:
	{




	}
	break;

	/* hightx and highty */
	case 3:
	{
		int RA = CRTC_GetRAOutput();
		LocalMA = CRTC_GetMAOutput();
		/* get screen scrolling */
		Addr = (unsigned int)

			(
			((LocalMA & (1 << 12)) << 3) |
			((LocalMA & (1 << 10)) << 3) |
			((LocalMA & 0x03ff) << 1)
			);

		/* hightx, bit 1, swaps V13<->V11
		 highty,

		 RA3 not connected. RA = L in schematic
		 MA11 not conncetd. MA = V in schematic
		 bit 0 of RA -> bit 14
		 bit 1,2 of RA -> bit 12,11

		 swap V13<->V11    MA12<->MA10


		 A16: VRAM bank
		 A15: MA12
		 A14: RA0
		 A13: MA10
		 A12: RA2
		 A11: RA1
		 A10: MA9
		 A09: MA8
		 A08: MA7
		 A07: MA6
		 A06: MA5
		 A05: MA4
		 A04: MA3
		 A03: MA2
		 A02: MA1
		 A01: MA0
		 A00: CCLK
		 */


		Addr |= ((RA & 1) << 14) | (((RA >> 1) & 0x03) << 11);
	}
	break;
	}


	VRAMAddr = Addr;

	pVRAMAddr = &Aleste_pRAM[VRAMAddr];
}

void Aleste_CachePixelData(void)
{
	PixelData = (pVRAMAddr[0] << 8) | (pVRAMAddr[1]);
}

unsigned short Aleste_GetPixelData(void)
{
	return PixelData;
}



void Aleste_Finish(void)
{
	if (Aleste_pRAM != NULL)
	{
		free(Aleste_pRAM);
		Aleste_pRAM = NULL;
	}
}

int Aleste_GetLED0(void)
{
	/* RUS/LAT LED */
	return Aleste_Mode & (1 << 5);
}

int Aleste_GetLED1(void)
{
	/* CAPS LED */
	return Aleste_Mode & (1 << 6);
}


void Aleste_RestartReset(void)
{
	PSG_SetBC2State(&OnBoardAY, 1);

	/* extport reset to 0 */
	Aleste_Extport = 0;

	Aleste_BlankingOutput = 0;
	Aleste_CRTCSyncInputs = 0;

	/* mode, rom, led etc reset to 0 */
	Aleste_Mode = 0;

	/* colour number reset to 0 */
	Aleste_NColor = 0;

	/* RTC reset */
	mc146818_reset();


	CRTC_Reset();


	/* 8255 reset */
	PPI_Reset();


	CPU_Reset();

	/* 765 FDC reset */
	FDC_Reset();

	/* terminal count is connected to reset */
	FDC_SetTerminalCount(1);
	FDC_SetTerminalCount(0);

	/* serial reset needs implementing */

	/* 8253 is not reset */

	PSG_SetType(&OnBoardAY, PSG_TYPE_YM2149);

	PSG_Reset(&OnBoardAY);

	/*	Aleste_MapperUpdate(); */

	/* tape motor is forced on for Aleste */
	Computer_SetTapeMotor(TRUE);

	Render_SetPixelTranslation(0x0);
	// do this for all at reset?
	Computer_UpdateGraphicsFunction();
}


void Aleste_RestartPower(void)
{
	int i;

	/* for now */
	Aleste_RestartReset();

	/* TODO: Find good value to fill memory with */
	for (i = 0; i < 512 * 1024; i++)
	{
		Aleste_pRAM[i] = rand() % 256;
	}

	FDC_Power();
	FDC_Reset();
}


Z80_BYTE    Aleste_AcknowledgeInterrupt(void)
{
	/* reset top bit of interrupt line counter */
	/* this ensures that the next interrupt is no closer than 32 lines */
	InterruptLineCount &= 0x01f;

	/* Gate Array interrupt acknowledged; so clear interrupt request */
	InterruptRequest = FALSE;

	return 0x0ff;

}

BOOL    Aleste_GetInterruptRequest(void)
{
	return InterruptRequest;
}

void	Aleste_TriggerVsyncSynchronisation(void)
{
	InterruptSyncCount = 2;
}


void Aleste_SetMapperROM(const unsigned char *pData)
{
	Aleste_pMapper = pData;
}

void Aleste_SetRfColDatROM(const unsigned char *pData)
{
	Aleste_pRfColDat = pData;
}

void Aleste_SetRomRamROM(const unsigned char *pData)
{
	Aleste_pRomRam = pData;
}

void Aleste_SetAl512(const unsigned char *pData)
{
	Aleste_Al512 = pData;
}
#if 0
void Aleste_RethinkMemory()
{
	Aleste_MapperUpdate();
}
#endif

void Aleste_MapperUpdate(void)
{
	/* address within mapper rom */
	/* NOTE: On actual hardware this is performed during execution */
	/* A14,A15 form bit0 and bit 1 of the address into the ROM */
	unsigned char *pRamPages[4];

	/* MSX Mapper mode */
	if ((Aleste_Extport&(1 << 2)) != 0)
	{
		int i;

		for (i = 0; i < 4; i++)
		{
			unsigned char Mapper = Aleste_Mapper[i];

			unsigned long RomAddr = (((Mapper & (1 << 4)) >> 4) << 7) |
				(((Aleste_Extport&(1 << 2)) >> 2) << 6) |
				((Mapper & 0x0f) << 2);

			unsigned char MapValue = Aleste_pMapper != NULL ? Aleste_pMapper[RomAddr + i] : 0;

			/* calc offset into Aleste RAM */
			unsigned long MapAddr = ((((Mapper & (1 << 4)) >> 4) & 0x01) << 18) | ((MapValue & 0x0f) << 14);

			/* now calculate actual address of start of block within ram */
			unsigned char *pAddr = &Aleste_pRAM[MapAddr];

			/* and convert to a readable address for z80 */
			pRamPages[i] = pAddr;
		}
	}
	else
	{
		/* CPC mapper */
		unsigned char Mapper = Aleste_Mapper[0];
		unsigned long RomAddr = (((Mapper & (1 << 4)) >> 4) << 7) |
			(((Aleste_Extport&(1 << 2)) >> 2) << 6) |
			((Mapper & 0x0f) << 2);

		int i;

		for (i = 0; i < 4; i++)
		{
			unsigned char MapValue = Aleste_pMapper[RomAddr + i];

			/* Amstrad limited to 256K */
			/* calc offset into Aleste RAM */
			unsigned long MapAddr = ((MapValue & 0x0f) << 14);

			/* now calculate actual address of start of block within ram */
			unsigned char *pAddr = &Aleste_pRAM[MapAddr];

			/* and convert to a readable address for z80 */
			pRamPages[i] = pAddr;
		}
	}
#if 0
	pWriteRamPtr[0] = (pRamPages[0]+0x00000)-0x0000;
	pWriteRamPtr[1] = (pRamPages[0]+0x02000)-0x2000;
	pWriteRamPtr[2] = (pRamPages[1]+0x00000)-0x4000;
	pWriteRamPtr[3] = (pRamPages[1]+0x02000)-0x6000;
	pWriteRamPtr[4] = (pRamPages[2]+0x00000)-0x8000;
	pWriteRamPtr[5] = (pRamPages[2]+0x02000)-0xa000;
	pWriteRamPtr[6] = (pRamPages[3]+0x00000)-0xc000;
	pWriteRamPtr[7] = (pRamPages[3]+0x02000)-0xe000;

	/* decoding happens from rom */
	/* check */
	if ((Aleste_Mode & (1<<3))==0)
	{
		/* upper rom is on */
		pReadRamPtr[6] = (&Aleste_Al512[(Aleste_UpperRomIndex<<14)+0x00000])-0x0c000;
		pReadRamPtr[7] = (&Aleste_Al512[(Aleste_UpperRomIndex<<14)+0x02000])-0x0e000;
	}
	else
	{
		pReadRamPtr[6] = pWriteRamPtr[6];
		pReadRamPtr[7] = pWriteRamPtr[7];
	}

	/* decoding happens from rom */
	/* check */
	if ((Aleste_Mode & (1<<2))==0)
	{
		/* lower rom is on */
		pReadRamPtr[0] = (&Aleste_Al512[(0<<14)+0x00000])-0x00000;
		pReadRamPtr[1] = (&Aleste_Al512[(0<<14)+0x02000])-0x02000;
	}
	else
	{
		pReadRamPtr[0] = pWriteRamPtr[0];
		pReadRamPtr[1] = pWriteRamPtr[1];
	}

	pReadRamPtr[2] = pWriteRamPtr[2];
	pReadRamPtr[3] = pWriteRamPtr[3];
	pReadRamPtr[4] = pWriteRamPtr[4];
	pReadRamPtr[5] = pWriteRamPtr[5];
#endif
	pRamPages;
}

void Aleste_UpdateHsync(BOOL bState)
{
	if (bState)
	{
		/* D43 on schematic */
		Aleste_CRTCSyncInputs |= HSYNC_INPUT;
		Aleste_BlankingOutput |= HBLANK_ACTIVE;

		Monitor_DoHsyncStart();
	}
	else
	{
		int Mode;

		/* D43 on schematic */
		Aleste_CRTCSyncInputs &= ~HSYNC_INPUT;
		Aleste_BlankingOutput &= ~HBLANK_ACTIVE;

		Monitor_DoHsyncEnd();

		/* cpc mode:
		 0 = 160x200 16 colours
		 1 = 320x200 4 colours
		 2 = 640x200 2 colours
		 3 = ?
		 msx mode:
		 0 = 640x200 2 colours
		 1 = 320x200 4 colours
		 2 = 640x200 4 colours
		 3 = 320x200 16 colours
		 */
		Mode = Aleste_Mode & 0x03;
		if ((Aleste_Extport&(1 << 1)) != 0)
		{
			/* high res */
			switch (Aleste_Mode & 0x03)
			{
			case 0:
			{
				Mode = 2;
			}
			break;

			case 1:
			{
				Mode = 1;
			}
			break;

			case 2:
			{
				Mode = 6;
			}
			break;

			case 3:
			{
				Mode = 7;
			}
			break;
			}
		}


		Render_SetPixelTranslation(Mode);

		/* increment interrupt line count */
		InterruptLineCount++;

		if (InterruptSyncCount == 0)
		{
			/* if line == 52 then interrupt should be triggered */
			if (InterruptLineCount == 52)
			{
				/* clear counter. */
				InterruptLineCount = 0;

				InterruptRequest = TRUE;
			}
		}
		else
		{
			InterruptSyncCount--;

			/* "2 scans into the VSYNC signal..." */
			if (InterruptSyncCount == 0)
			{
				/* the case where InterruptLineCount == 0, should be covered by */
				/* code above */

				/* has interrupt line counter overflowed? */
				if (InterruptLineCount >= 32)
				{
					/* following might not be required, because it is probably */
					/* set by the code above */

					InterruptRequest = TRUE;
				}

				/* reset interrupt line count */
				InterruptLineCount = 0;
			}
		}
		Computer_RefreshInterrupt();



	}
}




void Aleste_UpdateVsync(BOOL bState)
{
	if (bState)
	{
		/* D43 on schematic */
		Aleste_CRTCSyncInputs |= VSYNC_INPUT;
		Aleste_BlankingOutput |= VBLANK_ACTIVE;

		Monitor_DoVsyncStart();

		Aleste_TriggerVsyncSynchronisation();
	}
	else
	{
		/* D43 on schematic */
		Aleste_CRTCSyncInputs &= ~VSYNC_INPUT;
		Aleste_BlankingOutput &= ~VBLANK_ACTIVE;

		Monitor_DoVsyncEnd();
	}
}

void Aleste_PSG_SetPortOutputs(int Port, int Data)
{
	if (Port == 0)
	{
	}
	else
	{
		/* printer data, 8-bit */
		Printer_Write8BitData(Data);
	}


}

int Aleste_PSG_GetPortInputs(int Port)
{
	if (Port == 0)
	{
		return Keyboard_Read();
	}
	else
	{
		/* read from printer */
	}

	return 0x0ff;

}


Z80_BYTE Aleste_Multiport_Read(Z80_WORD Addr)
{
	/* is this correct, can you read each page value? */
	/* read mapper */
	if ((Aleste_Extport&(1 << 2)) != 0)
	{
		/* 'A' and 'B' signals determine this */
		int Page = (Addr >> 8) & 0x03;
		return Aleste_Mapper[Page];
	}

	/* not sure what happens with real hardware */
	return Aleste_Mapper[0];
}

extern int SelectedKeyboardLine;

void Aleste_PPI_Port_Write(int nPort, int Data)
{

	switch (nPort)
	{
	case 0:
	{
		if ((Aleste_Extport & (1 << 5)) == 0)
		{
			PSG_Write(&OnBoardAY, Data);
		}
		else
		{
			/* mc146818 */
			/* DS bit 2, AS bit 1, R/W bit 0 */
			switch (PPI_GetOutputPort(2) & 0x07)
			{
			case 2:
			{
				mc146818_write_address(Data);
			}
			break;

			case 4:
			{
				mc146818_write(Data);
			}
			break;
			}
		}

	}
	break;

	case 1:
		break;

	case 2:
	{

		/* bits 3..0 are keyboard */
		SelectedKeyboardLine = Data & 0x0f;

		/* strobe is connected to bit 4 of Port C, this is
		then passed through a NOT gate */
		Printer_SetStrobeState((Data & (1 << 4)) ^ (1 << 4));

		/* bit 5 is tape write (tape motor is forced on always */
		Computer_SetTapeWrite(Data & (1 << 5));

		/* bit 6 and 7 are PSG control */
		PSG_SetBDIRState(&OnBoardAY, Data & (1 << 7));
		PSG_SetBC1State(&OnBoardAY, Data & (1 << 6));
		PSG_RefreshState(&OnBoardAY);
		/* write always seems to go to AY */
		PSG_Write(&OnBoardAY, PPI_GetOutputPort(0));

		/* if low also write to rtc */
		if ((Aleste_Extport & (1 << 5)) == 0)
		{
			/* mc146818 or 8253 */
			/* DS bit 2, AS bit 1, R/W bit 0 */
			switch (Data & 0x07)
			{
			case 2:
			{
				mc146818_write_address(PPI_GetOutputPort(0));
			}
			break;

			case 4:
			{
				mc146818_write(PPI_GetOutputPort(0));
			}
			break;
			}
		}
	}
	break;
	}


}

int Aleste_PPI_Port_Read(int nPort)
{

	switch (nPort)
	{

	case 0:
	{

		if ((Aleste_Extport & (1 << 5)) == 0)
		{
			return PSG_Read(&OnBoardAY);
		}
		else
		{
			/* mc146818 */
			/* DS bit 2, AS bit 1, R/W bit 0 */
			switch (PPI_GetOutputPort(2) & 0x07)
			{
			case 5:
				return mc146818_read();
			}

		}
	}
	break;

	case 1:
	{
		int Data = 0;
		/* bit 5 is grounded */

		/* set screen refresh */
		if (CPC_Get50Hz())
		{
			Data |= PPI_SCREEN_REFRESH_50HZ;
		}

		/* bit 2 and 3 are pb2 */

		Data |= (1 << 1);
		/* bit 1 is disc int */
		/*
			if (FDC_GetInterruptOutput()!=0)
			{
			Data |= (1<<1);
			}
			*/
		/* bit 7 is cassette */
		if (Computer_GetTapeRead())
		{

			Data |= (1 << 7);
		}

		/* bit 6 is printer busy */
		if (Printer_GetBusyState())
		{
			Data |= (1 << 6);
		}

		/* bit 0 is vsync */
		if (CRTC_GetVsyncOutput() != 0)
		{
			Data |= 0x01;
		}
		return Data;
	}
	break;

	case 2:
	{

		/* Y3-Y0 */

		/* return (0x02<<4) | 0x0f; */

		return 0x0ff;

	}
	}

	return 0x0ff;
}

void	Aleste_UpdateColours(void)
{
	int i;
	/* update colours using colour scale */
	for (i = 0; i < 17; i++)
	{

		int Aleste_HwColour = Aleste_HwColours[i];
		Render_SetColour(&Aleste_HardwareColoursDisplayed[Aleste_HwColour], i);
	}
}

void    Aleste_SetMonitorColourMode(CPC_MONITOR_TYPE_ID MonitorMode)
{
	int i;

	switch (MonitorMode)
	{
	case CPC_MONITOR_COLOUR:
	{

		memcpy(Aleste_HardwareColoursDisplayed, Aleste_HardwareColoursColour, sizeof(Aleste_HardwareColoursColour));

		/* adjust */
		for (i = 0; i < 64; i++)
		{
			Monitor_AdjustRGBForDisplay(&Aleste_HardwareColoursDisplayed[i]);
		}
	}
	break;

	case CPC_MONITOR_GT64:
	{
		for (i = 0; i < 64; i++)
		{
			int AdjustedLuminance = Monitor_AdjustLuminanceForDisplay(Luminances[i]);
			/* does green show any red/blue at all? */
			Aleste_HardwareColoursDisplayed[i].u.element.Red = 0;
			Aleste_HardwareColoursDisplayed[i].u.element.Green = AdjustedLuminance;
			Aleste_HardwareColoursDisplayed[i].u.element.Blue = 0;
		}
	}
	break;

	case CPC_MONITOR_MM12:
	{
		for (i = 0; i < 64; i++)
		{
			int AdjustedLuminance = Monitor_AdjustLuminanceForDisplay(Luminances[i]);
			Aleste_HardwareColoursDisplayed[i].u.element.Red = AdjustedLuminance;
			Aleste_HardwareColoursDisplayed[i].u.element.Green = AdjustedLuminance;
			Aleste_HardwareColoursDisplayed[i].u.element.Blue = AdjustedLuminance;
		}
	}
	break;
	}
	Render_SetBlack(&Aleste_HardwareColoursDisplayed[0]);

	Aleste_UpdateColours();
}

void Aleste_Multiport_Write(Z80_WORD Addr, Z80_BYTE Data)
{
	switch (Data & 0x0c0)
	{
	case 0x0:
	{
		Aleste_NColor = Data;
	}
	break;

	case 0x040:
	{
		int PenIndex;
		unsigned char Aleste_HwColour;

		if ((Aleste_NColor & (1 << 4)) != 0)
		{
			/* border */
			PenIndex = 16;
		}
		else
		{
			PenIndex = Aleste_NColor & 0x0f;
		}

		Aleste_CurrentColour = Data;

		/* store selected colour */
		Aleste_Colours[PenIndex] = Data;

		/* remap chosen colour into actual hardware colour */
		if ((Aleste_Extport & (1 << 2)) == 0)
		{
			/* Amstrad mode */
			/* MAPMOD is not set, A15 is reset, A13 is set */
			Aleste_HwColour = Aleste_pRfColDat[0x0100 + Aleste_CurrentColour];
		}
		else
		{
			/* Aleste mode */
			/* MAPMOD is set, A15 is reset, A13 is set */
			Aleste_HwColour = Aleste_pRfColDat[0x0500 + Aleste_CurrentColour];
		}

		Aleste_HwColours[PenIndex] = Aleste_HwColour;

		Render_SetColour(&Aleste_HardwareColoursDisplayed[Aleste_HwColour], PenIndex);
	}
	break;

	case 0x080:
	{

		/*
			Mode:
			D0,D1 - screen mode  0-160*200, 1-320*200, 2-640*200
			D2    - ROM page on 0000h        0-On, 1-Off
			D3    - ROM page on C000h        0-On, 1-Off
			D4    - RUS LED	          1-On
			D5    - SCREEN ON	          1-On

			mode0, mode1, prom0,prom1, led0, led1
			*/
		Aleste_Mode = Data;

		Computer_RethinkMemory();

	}
	break;

	case 0x0c0:
	{

		if ((Aleste_Extport&(1 << 2)) != 0)
		{
			/* 'A' and 'B' signals determine this */
			int Page = (Addr >> 8) & 0x03;
			/* int Data = Aleste_pRfColDat[0x0500+Data]; */
			Aleste_Mapper[Page] = Data;
		}
		else
		{
			/* not sure what happens with real hardware */
			int MapperData = Aleste_pRfColDat[0x0100 + Data];
			Aleste_Mapper[0] = MapperData;

		}

		Computer_RethinkMemory();
	}
	break;
	}
}

void Aleste_Extport_Write(Z80_WORD Addr, Z80_BYTE Data)
{
	Aleste_Extport = Data;
	/* BLACK output - forcing blanking? */
	if ((Data & (1 << 3))==0)
	{
		Aleste_BlankingOutput |= ALESTE_BLACK_BLANKING;
	}
	else
	{
		Aleste_BlankingOutput &= ~ALESTE_BLACK_BLANKING;
	}
	Computer_UpdateGraphicsFunction();

	Computer_RethinkMemory();
}


void Aleste_LoadFromSnapshot(SNAPSHOT_HEADER *pHeader)
{
	/* Aleste initialise defaults */
}

void Aleste_SaveToSnapshot(SNAPSHOT_HEADER *pHeader)
{
	/* Aleste initialise defaults */
}

void Aleste_FillSnapshotMemoryBlocks(SNAPSHOT_MEMORY_BLOCKS *pSnapshotMemoryBlocks, const SNAPSHOT_OPTIONS *pOptions, BOOL bReading)
{
	int s;
	/* NOTE: Aleste has 512KB ram total, not 64KB+512KB like a CPC with
							   full ram expansions */
	for (s = 0; s < 32; s++)
	{
		if (bReading || pOptions->bBlocksToExport[s])
		{
			SNAPSHOT_MEMORY_BLOCK *pBlock = &pSnapshotMemoryBlocks->Blocks[s];
			pBlock->nSourceId = SNAPSHOT_SOURCE_INTERNAL_ID;
			pBlock->bAvailable = TRUE;
			pBlock->pPtr = Aleste_pRAM + (s << 14);
		}
	}
}

void Aleste_InitialiseMemoryOutputs(MemoryData *pData)
{
	/* PROM0, PROM1, A14, A15, A0, RAMDIS and D67 control out */
	/* BUFER0, BUFER1, /ROMEN and /RAMEN */

	/* TODO: Correct */
	pData->bRomEnable[7] = ((Aleste_Mode & 0x08) == 0);
	pData->bRomEnable[6] = pData->bRomEnable[7];

	pData->bRomEnable[5] = FALSE;
	pData->bRomEnable[4] = FALSE;

	pData->bRomEnable[3] = FALSE;
	pData->bRomEnable[2] = FALSE;

	pData->bRomEnable[1] = ((Aleste_Mode & 0x04) == 0);
	pData->bRomEnable[0] = pData->bRomEnable[1];


	pData->bRamRead[7] = ((Aleste_Mode & 0x08) != 0);
	pData->bRamRead[6] = pData->bRamRead[7];

	pData->bRamRead[5] = FALSE;
	pData->bRamRead[4] = FALSE;

	pData->bRamRead[3] = FALSE;
	pData->bRamRead[2] = FALSE;

	pData->bRamRead[1] = ((Aleste_Mode & 0x04) != 0);
	pData->bRamRead[0] = pData->bRamRead[1];
}


void Aleste_InitialiseDefaultMemory(MemoryData *pData)
{
	/* address within mapper rom */
	/* NOTE: On actual hardware this is performed during execution */
	/* A14,A15 form bit0 and bit 1 of the address into the ROM */
	unsigned char *pRamPages[4];

	int i;

	/* MSX Mapper mode */
	if ((Aleste_Extport&(1 << 2)) != 0)
	{

		for (i = 0; i < 4; i++)
		{
			unsigned char Mapper = Aleste_Mapper[i];

			unsigned long RomAddr = (((Mapper & (1 << 4)) >> 4) << 7) |
				(((Aleste_Extport&(1 << 2)) >> 2) << 6) |
				((Mapper & 0x0f) << 2);


			unsigned char MapValue = Aleste_pMapper[RomAddr + i];

			/* calc offset into Aleste RAM */
			unsigned long MapAddr = ((((Mapper & (1 << 4)) >> 4) & 0x01) << 18) | ((MapValue & 0x0f) << 14);

			/* now calculate actual address of start of block within ram */
			unsigned char *pAddr = &Aleste_pRAM[MapAddr];

			pRamPages[i] = pAddr;
		}
	}
	else
	{
		/* CPC mapper */
		unsigned char Mapper = Aleste_Mapper[0];
		unsigned long RomAddr = (((Mapper & (1 << 4)) >> 4) << 7) |
			(((Aleste_Extport&(1 << 2)) >> 2) << 6) |
			((Mapper & 0x0f) << 2);



		for (i = 0; i < 4; i++)
		{
			unsigned char MapValue = Aleste_pMapper != NULL ? Aleste_pMapper[RomAddr + i] : 0;

			/* Amstrad limited to 256K */
			/* calc offset into Aleste RAM */
			unsigned long MapAddr = ((MapValue & 0x0f) << 14);

			/* now calculate actual address of start of block within ram */
			unsigned char *pAddr = &Aleste_pRAM[MapAddr];


			pRamPages[i] = pAddr;
		}
	}

	for (i = 0; i < 8; i++)
	{
		int nPage = (i >> 1);
		int nOffset = (nPage << 14);
		if (!pData->bRamDisable[i])
		{
			pData->pWritePtr[i] = pRamPages[nPage] - nOffset;
			if (!pData->bRomEnable[i])
			{
				pData->pReadPtr[i] = pData->pWritePtr[i];
			}
		}
	}

	/* /ROMEN connected to /CS of ROM */
	/* ROMDIS connected to /OE of ROM */
	if ((pData->bRomEnable[6]) && (!pData->bRomDisable[6]))
	{
		/* 0 is OS, 1 = BASIC, 2 = AMSDOS, 3 = BOOT  */
		int ROMDataOffset = (Aleste_UpperRomIndex << 14);
		const unsigned char *pRomData = &Aleste_Al512[ROMDataOffset];

		/* depending on the selected page see if there are any overrides active */
		/* boot page can't be overridden */
		switch (Aleste_UpperRomIndex)
		{
		case 0:
		{
			if (CPC_GetOSOverrideROMEnable())
			{
				pRomData = CPC_GetOSOverrideROM();
			}
		}
		break;

		case 1:
		{
			if (CPC_GetBASICOverrideROMEnable())
			{
				pRomData = CPC_GetBASICOverrideROM();
			}
		}
		break;

		case 2:
		{
			if (CPC_GetAmsdosOverrideROMEnable())
			{
				pRomData = CPC_GetAmsdosOverrideROM();
			}
		}
		break;
		}

		/* upper rom is on */
		pData->pReadPtr[6] = pRomData - 0x0c000;
		pData->pReadPtr[7] = pRomData - 0x0c000;
	}

	if ((pData->bRomEnable[0]) && (!pData->bRomDisable[0]))
	{
		const unsigned char *pRomData = &Aleste_Al512[0];
		/* if override is active, use it */
		if (CPC_GetOSOverrideROMEnable())
		{
			pRomData = CPC_GetOSOverrideROM();
		}
		/* lower rom is on */
		pData->pReadPtr[0] = pRomData;
		pData->pReadPtr[1] = pRomData;
	}
}


Z80_BYTE        Aleste_In(Z80_WORD Port)
{
	Z80_BYTE Data = 0x0ff;


	/* mapper is readable */
	if ((Port & (1 << 15)) == 0)
	{
		Data = Aleste_Multiport_Read(Port);
	}



	if ((Port & (1 << 12)) == 0)
	{
		switch (Port & (1 << 8))
		{
		case 0:
			break;

		case (1 << 8) :
			break;
		}


	}

	/* Disc motor and extended port can't be read */
	if ((Port & (1 << 8)) != 0)
	{
		unsigned int            Index;

		Index = (
			((Port >> (10 - 1)) & 0x02) |
			((Port >> (7 - 0) & 0x01))
			);

		if (Index == 0)
		{
			switch (Port & 0x01)
			{
			case 0:
			{
				return FDC_ReadMainStatusRegister();
			}
			break;

			case 1:
			{
				return FDC_ReadDataRegister();
			}
			break;
			}
		}
	}


	if ((Port & 0x0800) == 0)
	{
		unsigned int            Index;

		Index = (Port & 0x0300) >> 8;
		if (Index == 3)
		{
			Data = PPI_ReadControl();
		}
		else
		{
			Data = PPI_ReadPort(Index);
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

		default
			:
				break;
		}
	}

	CPC_ExecuteReadPortFunctions(Port, &Data);

	/* handle ports that latch data when read */
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

		default
			:
				break;
		}
	}


	return (Z80_BYTE)Data;
}



void	Aleste_Out(const Z80_WORD Port, const Z80_BYTE Data)
{
	if ((Port & 0x08000) == 0)
	{
		Aleste_Multiport_Write(Port, Data);
	}

	/* 8251 */
	if ((Port & (1 << 12)) == 0)
	{
		switch (Port & (1 << 8))
		{
		case 0:
			break;

		case (1 << 8) :
			break;
		}


	}

	/* aleste decodes this port for it's internal rom  */
	if ((Port & (1 << 13)) == 0)
	{
		/* 2 bits are used by hardware */
		Aleste_UpperRomIndex = Aleste_pRfColDat[0x0200 + Data] & 0x03;

		Computer_RethinkMemory();
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

	if ((Port & (1 << 8)) != 0)
	{
		unsigned int            Index;

		Index = (
			((Port >> (10 - 1)) & 0x02) |
			((Port >> (7 - 0) & 0x01))
			);

		if (Index == 0)
		{
			switch (Port & 0x01)
			{
			case 1:
			{
				FDC_WriteDataRegister(Data);
			}
			break;
			}
		}
	}
	else
	{
		unsigned int            Index;

		Index = (
			((Port >> (10 - 1)) & 0x02) |
			((Port >> (7 - 0) & 0x01))
			);

		if (Index == 0)
		{
			FDI_SetMotorState(Data & 0x01);
		}
		else
			if (Index == 1)
			{
				/* extended */
				if ((Port&(1 << 6)) == 0)
				{
					Aleste_Extport_Write(Port, Data);
				}

			}
	}


	if ((Port & 0x0800) == 0)
	{
		unsigned int            Index;
		Index = (Port & 0x0300) >> 8;
		if (Index == 3)
		{
			PPI_WriteControl(Data);
		}
		else
		{
			PPI_WritePort(Index, Data);
		}

	}

	/* 8253 enabled? */
	if ((Aleste_Extport & (1 << 4)) != 0)
	{
		/* any port will write to 8253 */
		i8253_write(&timer, Port & 0x03, PPI_GetOutputPort(0));
	}
}

extern MONITOR_INTERNAL_STATE Monitor_State;

void    Render_GetGraphicsDataAleste_TrueColour(void)
{
	unsigned short GraphicsWord;
	Aleste_CalcVRAMAddr();
	Aleste_CachePixelData();
	GraphicsWord = Aleste_GetPixelData();

	if ((Aleste_GetExtport()&(1 << 1)) != 0)
	{
		Render_TrueColourRGB_PutDataWordHigh(Monitor_State.MonitorHorizontalCount, GraphicsWord, Monitor_State.MonitorScanLineCount);
	}
	else
	{
		Render_TrueColourRGB_PutDataWord(Monitor_State.MonitorHorizontalCount, GraphicsWord, Monitor_State.MonitorScanLineCount);
	}
}
void    Render_RenderBorder_Aleste_TrueColour(void)
{
	if ((Aleste_GetExtport()&(1 << 1)) != 0)
	{
		Render_TrueColourRGB_PutBorderHigh(Monitor_State.MonitorHorizontalCount, Monitor_State.MonitorScanLineCount);
	}
	else
	{
		Render_TrueColourRGB_PutBorder(Monitor_State.MonitorHorizontalCount, Monitor_State.MonitorScanLineCount);
	}
}



void    Render_RenderBlack_Aleste_TrueColour(void)
{
	Render_TrueColourRGB_PutBlack(Monitor_State.MonitorHorizontalCount, Monitor_State.MonitorScanLineCount);
}


void    Render_RenderSync_Aleste_TrueColour(void)
{
	if ((Aleste_GetExtport()&(1 << 1)) != 0)
	{
		Render_TrueColourRGB_PutSyncHigh(Monitor_State.MonitorHorizontalCount, Monitor_State.MonitorScanLineCount);
	}
	else
	{
		Render_TrueColourRGB_PutSync(Monitor_State.MonitorHorizontalCount, Monitor_State.MonitorScanLineCount);
	}
}


void Aleste_DoDispEnable(BOOL bState)
{
	if (bState)
	{
		Aleste_BlankingOutput &= ~DISPTMG_ACTIVE;
	}
	else
	{
		Aleste_BlankingOutput |= DISPTMG_ACTIVE;
	}
	Computer_UpdateGraphicsFunction();

}


void Aleste_UpdateGraphicsFunction(void)
{
	if (
		/* hsync or vsync is active */
		Monitor_DrawSync()
		)
	{
		CRTC_SetRenderFunction2(Render_RenderSync_Aleste_TrueColour);
	}
	else
		if (Computer_GetDrawBlanking() && ((Aleste_BlankingOutput & (HBLANK_ACTIVE | VBLANK_ACTIVE| ALESTE_BLACK_BLANKING)) != 0))
		{
			/* forced to black.. similar to border but not */
			CRTC_SetRenderFunction2(Render_RenderBlack_Aleste_TrueColour);
		}
		else
			if (Computer_GetDrawBorder() && (Aleste_BlankingOutput & DISPTMG_ACTIVE))
			{
				CRTC_SetRenderFunction2(Render_RenderBorder_Aleste_TrueColour);
			}
			else
			{
				CRTC_SetRenderFunction2(Render_GetGraphicsDataAleste_TrueColour);
			}
}
