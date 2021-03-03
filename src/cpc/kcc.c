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
#include "cpcglob.h"
#include "kcc.h"
#include "crtc.h"
#include "printer.h"
#include "monitor.h"

/*

  4 = s = 5v
  2 = d = 5v
  3 = c = 8.
  1 = r = reset/request.
  10 = s = 5v
	12 = d = 8
  11 = c = aic
  13 = r = mf/db4 reset.


  s = pr
  d = d
  c = clk
  r = clr

  pr d
*/






/*
	Z8536 OPERATION IN KC COMPACT:

	Port C - Timer - Handles interrupts

	PC0 = Counter/Timer Output = AIC
	PC1 = Counter Input = CRTC HSYNC
	PC2 = Trigger Input = VSYV
	PC3 = Gate Input = 5V


  VSYV is TRUE if a HSYNC ended while VSYNC was active.


  */

/* /FN = pen select */
/* /FW = pen colour */
/* /MF = multi function */
/* /MF and bit 4 = reset interrupt request */

/* interrupt request also reset

VSYNC = VSYNC, /HSYNC

VSYV is the state of VSYNC, when HSYNC ends.

/HSYNC
HSYNC

HSYNC is HSYNC output from CRTC and will be high when active
VSYNC is VSYNC output from CRTC and will be high when active
/VSYNC is notted VSYNC
/HSYNC is notted HSYNC
VSYV
by second hsync after vsync */


/*
10 = PR = 5V
12 = D = D2 = VSYNC
11 = C = CLK 2 = /HSYNC
13 = CLR = 5V

10  13  /HSYNC	VSYNC		VSYV
H	H	^		H			H
H	H	^		L			L

/HSYNC +VE TRANSITION.

HSYNC ACTIVE /HSYNC = 0
HSYNC NOT ACTIVE /HSYNC = 1

/HSYNC INPUT TRIGGERS WHEN HSYNC ENDS.
VSYV = TRUE IF HSYNC TRIGGERS DURING VSYNC
*/
#include "cpc.h"
#include "z8536.h"
#include "i8255.h"
#include "psg.h"
#include "render.h"


static int nHBlankCycle=0;
static int BlankingOutput = 0;
static int CRTCSyncInputs = 0;

extern Z8536 z8536;
extern AY_3_8912 OnBoardAY;
static const unsigned char *pColorROM;
static unsigned char *pColorRAM;
static const unsigned char *pBasic = NULL;
static const unsigned char *pOS = NULL;

static unsigned int PenIndex;
static unsigned int PenSelection;
static unsigned int ColourSelection;
static unsigned int MultiFunction;
static int PenColour[17];
static RGBCOLOUR KCCompact_HardwareColours[64];
static RGBCOLOUR KCC_DisplayColours[64];

static BOOL KCC_InterruptRequest = FALSE;
static int InterruptRequestResetCount = 0;

static unsigned char *pVRAMAddr;
static unsigned int VRAMAddr;
static unsigned short PixelData;
extern unsigned char *Z80MemoryBase;

int KCC_GetVRAMAddr(void)
{
    return VRAMAddr;
}

void KCC_CalcVRAMAddr(void)
{
    int LocalMA = CRTC_GetMAOutput()<<1;
    int Addr = ((LocalMA & 0x06000)<<1) |(LocalMA & 0x07ff);
    /* MA0-MA9, MA12, MA13 used */
    /* RA0-RA7 used */
    Addr |= (CRTC_GetRAOutput()&0x07)<<11;

    VRAMAddr = Addr;

    pVRAMAddr = &Z80MemoryBase[VRAMAddr];
}

void KCC_CachePixelData(void)
{
    PixelData = (pVRAMAddr[0]<<8)|(pVRAMAddr[1]);
}

unsigned short KCC_GetPixelData(void)
{
    return PixelData;
}

void KCC_Init(void)
{
    int i;
    int Colour[4] = {0, 0x07f, 0x07f, 0x0ff};

    pColorRAM = (unsigned char *)malloc(256);
	memset(pColorRAM, 0, 256);

    for (i=0; i<64; i++)
    {
        /* bit 0,1 are blue, bit 2,3 are red, bit 4,5 are green */
        /* 2.2K resistor for both, so this means 01 or 10 combination are the same */
        /* 00 -> 0 */
        /* 01/10 -> same result */
        /* 11 -> max */

        int R = (i>>2) & 0x03;
        int G = (i>>4) & 0x03;
        int B = i&0x03;

        KCCompact_HardwareColours[i].u.element.Red = Colour[R];
        KCCompact_HardwareColours[i].u.element.Green = Colour[G];
        KCCompact_HardwareColours[i].u.element.Blue = Colour[B];
    }
}

void KCC_LoadFromSnapshot(SNAPSHOT_HEADER *pSnapshotHeader)
{
	int i;

#if 0
	;; 0x06b1

		;; start of Z8536 configuration data
		defb & 00, &01;; Master interrupt control : reset
		;; at this point, Z8536 is in reset state, and requires a write with bit 0 = 0
		;; a read will not change it's state.
		defb & 22;; advance CIO to state 0 from RESET STATE
		defb & 2a, &44;; Port B's data path polarity: 01000100 (invert bits 2 and 6)
		defb & 05, &02;; Port C's data path polarity: 00000000 (do not invert any bits)
		defb & 23, &bd;; Port A's data direction:  10111101 (bit 1 is INPUT, bit 6 is INPUT all other bits are OUTPUT)
		defb & 2b, &ee;; Port B's data direction:  11101110 (bit 0 is INPUT, bit 4 is INPUT all other bits are OUTPUT)
		defb & 06, &ee;; Port C's data direction:  11101110 (bit 0 is INPUT, bit 4 is INPUT all other bits are OUTPUT)
		defb & 24, &42;; Port A's special I/O Control 
		defb & 0d, &ff;; Port A's data: 11111111
		defb & 01, &94;; Master Configuration Control : Port B enable, disable counters, port a enable
		defb & 16, &02;; Counter / Timer 1's Time Constant MSB: &02
		defb & 17, &47;; Counter / Timer 1's Time Constant LSB: &47
		defb & 18, &01;; Counter / Timer 2's Time Constant MSB: &01
		defb & 19, &67;; Counter / Timer 2's Time Constant LSB: &67
		defb & 1a, &00;; Counter / Timer 3's Time Constant MSB: &00
		defb & 1b, &1a;; Counter / Timer 3's Time Constant LSB: &1a
		;; Timer 1 time constant : &0247 (583), Timer 2 time constant : &0167 (359), Timer 3 time constant : &1a(26 = 52 * (1 / 2)!!!!)

		defb & 1c, &fc;; Counter / Timer 1's Mode specification: Continuous,external output enable,external count enable,
		;; external trigger enable, external gate enable, retrigger enable bit, pulse output
		defb & 1d, &fc;; Counter / Timer 2's Mode specification: Continuous,external output enable,external count enable,
		;; external trigger enable, external gate enable, retrigger enable bit, pulse output
		defb & 1e, &fc;; Counter / Timer 3's Mode specification: Continuous,external output enable,external count enable,
		;; external trigger enable, external gate enable, retrigger enable bit, pulse output
		defb & 0a, &04;; Counter / Timer 1's Command and Status: Gate Command Bit
		defb &0b, &04;; Counter / Timer 2's Command and Status: Gate Command Bit
		defb & 0c, &04;; Counter / Timer 3's Command and Status: Gate Command Bit
		defb & 01, &f4;; Master Configuration Control : Port B enable, Counter / Timer 1 enable, Counter / Timer 2 enable,
		;; Counter / Timer 3 enable, port A and B operate independantly, port A enable, counter / timers are
		;; idependant
		defb & 0a, &06;; Counter / Timer 1's Command and Status: Gate Command Bit,Trigger Command Bit
		defb &0b, &06;; Counter / Timer 2's Command and Status: Gate Command Bit,Trigger Command Bit

		;; the following are setup for the operating system only
		defb & 01, &f0;; Master Configuration Control : Port B enable, Counter / Timer 1 enable, Counter / Timer 2 enable,
		;; Counter / Timer 3 enable, port A and B operate independantly, port A disable, counter / timers are
		;; idependant
		defb & 22, &80;; Port A's data path polarity: invert bit 7, all other bits unchanged
		defb & 23, &00;; Port A's data direction: all bits output
		defb & 24, &00;; Port A's special I/O control: no actions
		defb & 01, &f4;; Master Configuration Control : Port B enable, Counter / Timer 1 enable, Counter / Timer 2 enable,
		;; Counter / Timer 3 enable, port A and B operate independantly, port A enable, counter / timers are
		;; idependant
		defb & 0d, &7f;; Port A's data: 01111111 (/strobe = 0)
#endif

	/* KCC initialise defaults */
	/**** KCC hardware ****/
	/* initialise colour palette */
	for (i = 0; i < 17; i++)
	{
		unsigned char HwColourIndex = ((char *)pSnapshotHeader)[0x02f + i];

		/* pen select */
		KCC_GA_Write(i);

		/* write colour for pen */
		KCC_GA_Write((HwColourIndex & 0x01f) | 0x040);
		printf("(KCC) GA Pen %d Colour &%02x\n", i, (HwColourIndex & 0x01f) | 0x040);

	}

	/* pen select */
	KCC_GA_Write(((char *)pSnapshotHeader)[0x02e] & 0x01f);
	printf("(KCC) GA Selected Pen %d\n", ((char *)pSnapshotHeader)[0x02e] & 0x01f);

	/* mode and rom configuration select */
	KCC_GA_Write((((char *)pSnapshotHeader)[0x040] & 0x03f) | 0x080);
	printf("(KCC) GA MREM %02x\n", (((char *)pSnapshotHeader)[0x040] & 0x03f) | 0x080);

}

void KCC_SaveToSnapshot(SNAPSHOT_HEADER *pHeader)
{
	/* KCC initialise defaults */
}

void KCC_FillSnapshotMemoryBlocks(SNAPSHOT_MEMORY_BLOCKS *pSnapshotMemoryBlocks, const SNAPSHOT_OPTIONS *pOptions, BOOL bReading)
{
	int i;
	for (i = 0; i < 4; i++)
	{
		SNAPSHOT_MEMORY_BLOCK *pBlock = &pSnapshotMemoryBlocks->Blocks[i];
		pBlock->nSourceId = SNAPSHOT_SOURCE_INTERNAL_ID;
		pBlock->bAvailable = TRUE;
		pBlock->pPtr = Z80MemoryBase + (i << 14);
	}
}

void KCC_RestartPower(void)
{
	int i;

    KCC_RestartReset();

	/* TODO: Find good value to fill memory with */
	for (i=0; i<65536; i++)
	{
		Z80MemoryBase[i] = rand()%256;
	}
}

void KCC_RestartReset(void)
{
	PSG_SetBC2State(&OnBoardAY, 1);

	CRTCSyncInputs = 0;
	BlankingOutput = 0;
	nHBlankCycle = 0;

	/* Z8536 doesn't appear to be reset */

	/* Reset clears interrupt request explicitly */
	KCC_AcknowledgeInterrupt();

	/* 8255 reset */
	PPI_Reset();

	/* crtc reset */
	CRTC_Reset();

	/* Multifunction register is reset to 0 */
	MultiFunction = 0;

	/* set PSG type */
	PSG_SetType(&OnBoardAY, PSG_TYPE_AY8912);

	/* reset AY-3-8912 */
	PSG_Reset(&OnBoardAY);

	CPU_Reset();

	InterruptRequestResetCount = 0;

	Render_SetPixelTranslation(0x0);
}

void    KCC_SetMonitorColourMode(CPC_MONITOR_TYPE_ID MonitorMode)
{
	int i;
	
	for (i=0; i<64; i++)
	{
		KCC_DisplayColours[i] = KCCompact_HardwareColours[i];
	}
	for (i=0; i<64; i++)
	{
		Monitor_AdjustRGBForDisplay(&KCC_DisplayColours[i]);
	}
	Render_SetBlack(&KCC_DisplayColours[0]);

}

int KCC_PPI_Port_Read(int nPort)
{
    switch (nPort)
    {

        case 0:
            return PSG_Read(&OnBoardAY);

        case 1:
        {
                           int Data = 0;

            Data |= (1<<4) | (1<<3) | (1<<2) | (1<<1);

            /* set state of vsync bit */
            if (CRTC_GetVsyncOutput()!=0)
            {
               Data |= (1<<0);
            }

            if (Printer_GetBusyState())
            {
                Data |= (1<<6);
            }

            /* 7 is tape data */
            if (Computer_GetTapeRead())
            {

                Data |= (1<<7);
            }

            return Data;
        }

        /* tests show this is read */
        /* bit 3..0 are keyboard outputs, bit 4 is cassette motor, bit 5 is cassette write/printer bit 7, bit 6,7 are psg control */
        case 2:
            return 0x01f;

    }
    return 0x0ff;
}

extern int SelectedKeyboardLine;

int KCC_PSG_GetPortInputs(int Port)
{
    if (Port==0)
    {
        return Keyboard_Read();
    }

    return 0x0ff;
}


void KCC_PPI_Port_Write(int nPort, int Data)
{
    switch (nPort)
    {
        case 0:
            {
             PSG_Write(&OnBoardAY,Data);

            }
            break;

        case 1:
            break;

    case 2:
        {

            /* bit 3..0 are keyboard */
            SelectedKeyboardLine = Data & 0x0f;

             /* Bit 7 of printer data is cassette write */
            Printer_SetDataBit7State(Data & (1<<5));

            /* bit 4 is tape motor */
            Computer_SetTapeMotor((Data & (1<<4)));

            /* bit 5 is tape write */
            Computer_SetTapeWrite(Data & (1<<5));

            /* bit 6 and 7 are PSG control */
            PSG_SetBDIRState(&OnBoardAY,Data & (1<<7));
            PSG_SetBC1State(&OnBoardAY,Data & (1<<6));
            PSG_RefreshState(&OnBoardAY);
            PSG_Write(&OnBoardAY,PPI_GetOutputPort(0));
        }
        break;
    }

}

void KCC_Finish(void)
{
    if (pColorRAM!=NULL)
    {
        free(pColorRAM);
    }
}

extern unsigned char   *Z80MemoryBase;


void KCC_InitialiseMemoryOutputs(MemoryData *pData)
{
	/* D253 controls /RAMEN, /ROMEN */
	/* A15,A14, DB2, DB3 from D175 are inputs */

	/* Top of D253 generates /RAMEN : 0 is /DB2, 1,2 = 0, 3 = /DB3 */
	/* Bottom of D253 generates /ROMEN: 0 is DB2, 1,2 is 5V, 3=DB3 */

	/* RAMDIS effects /RAMWE */
	/* EPROM is activated with /ROMEN */

	/* /ROMOE effected by ROMDIS */
	/* /RAMOE effected by RAMDIS and /RAMEN */
    pData->bRomEnable[7] = ((MultiFunction & 0x08)==0);
    pData->bRomEnable[6] = pData->bRomEnable[7];

    pData->bRomEnable[5] = FALSE;
    pData->bRomEnable[4] = FALSE;

	pData->bRomEnable[3] = FALSE;
	pData->bRomEnable[2] = FALSE;

	pData->bRomEnable[1] = ((MultiFunction & 0x04)==0);
	pData->bRomEnable[0] = pData->bRomEnable[1];

    pData->bRamRead[7] = !pData->bRomEnable[7];
	pData->bRamRead[6] = !pData->bRomEnable[6];

	pData->bRamRead[5] = TRUE;
    pData->bRamRead[4] = TRUE;

	pData->bRamRead[3] = TRUE;
	pData->bRamRead[2] = TRUE;

    pData->bRamRead[1] = !pData->bRomEnable[1];
	pData->bRamRead[0] = !pData->bRomEnable[0];
}


void KCC_InitialiseDefaultMemory(MemoryData *pData)
{
    int i;
    for (i=0; i<8; i++)
    {
        if (!pData->bRamDisable[i])
        {
            /* Z80 Memory base is continous 64k */
            pData->pWritePtr[i] = Z80MemoryBase;
            if (!pData->bRomEnable[i])
            {
                pData->pReadPtr[i] = pData->pWritePtr[i];
            }
        }
    }

    /* upper rom enabled, and not disabled */
    if ((pData->bRomEnable[6]) && (!pData->bRomDisable[6]))
    {
        const unsigned char *pRomData = pBasic;

        /* override enabled? */
        if (CPC_GetBASICOverrideROMEnable())
        {
            /* override rom set? */
            pRomData = CPC_GetBASICOverrideROM();
        }

        if (pRomData==NULL)
        {
			unsigned char *pRam = GetDummyReadRam();
			pData->pReadPtr[6] = (const unsigned char *)(pRam - 0x0c000);
			pData->pReadPtr[7] = (const unsigned char *)(pRam - 0x0c000);
        }
        else
        {
            pData->pReadPtr[6] = pRomData-0x0c000;
            pData->pReadPtr[7] = pRomData-0x0c000;
        }
    }

    /* lower rom enabled, and not disabled */
    if ((pData->bRomEnable[0]) && (!pData->bRomDisable[0]))
    {
        const unsigned char *pRomData = pOS;

        /* override enabled? */
        if (CPC_GetOSOverrideROMEnable())
        {
            /* override rom set? */
            pRomData = CPC_GetOSOverrideROM();
        }

        if (pRomData==NULL)
        {
			unsigned char *pRam = GetDummyReadRam();

			pData->pReadPtr[0] = (const unsigned char *)(pRam - 0x00000);
			pData->pReadPtr[1] = (const unsigned char *)(pRam - 0x00000);
        }
        else
        {
            pData->pReadPtr[0] = (const unsigned char *) pRomData-0x00000;
            pData->pReadPtr[1] = (const unsigned char *) pRomData-0x00000;
        }
    }
}



void    KCC_SetColorROM(const unsigned char *pData)
{
    pColorROM = pData;
}


const unsigned char *KCC_GetOSRom(void)
{
    return pOS;
}

void KCC_SetOSRom(const unsigned char *pData)
{
    pOS = pData;
}

const unsigned char *KCC_GetBASICRom(void)
{
    return pBasic;
}

void KCC_SetBASICRom(const unsigned char *pData)
{
    pBasic = pData;
}



Z80_BYTE	KCC_AcknowledgeInterrupt(void)
{
	KCC_InterruptRequest = FALSE;
	/* TODO: what does the kcc put on the bus at interrupt time? is it controlled by the z8536 ? */
	return 0x0ff;
}

BOOL KCC_GetInterruptRequest(void)
{
    return KCC_InterruptRequest;
}


extern void CRTC_SetRenderFunction2(void (*pRenderFunction)(void));
extern void CRTC_RenderSync_TrueColour(void);
extern void    CRTC_RenderBlack_TrueColour(void);
extern void Render_RenderBorder_TrueColour(void);
extern void Render_GetGraphicsDataCPC_TrueColour(void);

void KCC_UpdateGraphicsFunction(void)
{
	/* confirmed: KCC will blank the *border* during vertical sync,
	but the blank is the size of the vsync AND graphics are NOT blanked */
	/* confirmed: KCC will blank the *border* during hsync. KCC also outputs HSYNC to 
	display for 4us, with a delay of 2 */
	if (Monitor_DrawSync())
	{
		CRTC_SetRenderFunction2(CRTC_RenderSync_TrueColour);
	}
	else
	if (Computer_GetDrawBlanking() && ((BlankingOutput & HBLANK_ACTIVE) != 0))
	{
		CRTC_SetRenderFunction2(CRTC_RenderBlack_TrueColour);
	}
	else
	if (Computer_GetDrawBorder() && ((BlankingOutput & (DISPTMG_ACTIVE)) != 0))
	{
		if (Computer_GetDrawBlanking() && ((BlankingOutput & (HBLANK_ACTIVE | VBLANK_ACTIVE)) != 0))
		{
			CRTC_SetRenderFunction2(CRTC_RenderBlack_TrueColour);
		}
		else
		{
			CRTC_SetRenderFunction2(Render_RenderBorder_TrueColour);
		}
	}
	else
	{
		CRTC_SetRenderFunction2(Render_GetGraphicsDataCPC_TrueColour);
	}
}

void    KCC_RefreshInterrupts(void)
{
    if (z8536.counter_outputs[2]&0x01)
    {
        KCC_InterruptRequest = TRUE;
    }
    else
    {
        KCC_InterruptRequest = FALSE;
    }
    Computer_RefreshInterrupt();
}

/* confirmed: hsync to monitor is delayed by 2 cycles and lasts for 4us */
/* However, the mode change is on the rising edge and the int is on the trailing edge */

void KCC_UpdateHsync(BOOL bState)
{
	if (bState)
	{
		CRTCSyncInputs |= HSYNC_INPUT;
		BlankingOutput |= HBLANK_ACTIVE;
		nHBlankCycle = 0;

        /* set pixel translation table which is dependant on mode */
        Render_SetPixelTranslation(MultiFunction & 0x03);

        /* hsync start */
        z8536.inputs[2] |= 0x02;
        Z8536_Update();

        KCC_RefreshInterrupts();

		KCC_UpdateGraphicsFunction();
	}
	else
	{
		CRTCSyncInputs &= ~HSYNC_INPUT;
		BlankingOutput &= ~HBLANK_ACTIVE;

		Monitor_DoHsyncEnd();

        /* 2 hsyncs after the start of vsync */
        if (InterruptRequestResetCount!=0)
        {
            InterruptRequestResetCount--;
            if (InterruptRequestResetCount==0)
            {
                KCC_AcknowledgeInterrupt();
            }
        }

        z8536.inputs[2] &= ~0x02;
        Z8536_Update();

        KCC_RefreshInterrupts();

		KCC_UpdateGraphicsFunction();
	}
}

void KCC_Cycle(void)
{

	/* HSYNC is delayed by by 2us */
	if (BlankingOutput & HBLANK_ACTIVE)
	{
		switch (nHBlankCycle)
		{
		case 0:
			break;

		case 1:
		{
			if (CRTCSyncInputs & HSYNC_INPUT)
			{
				Monitor_DoHsyncStart();
			}

			KCC_UpdateGraphicsFunction();
		}
		break;

		case 5:
		{
			/* end monitor hsync but continue blanking */
			Monitor_DoHsyncEnd();

			KCC_UpdateGraphicsFunction();
		}
		break;
		}
		nHBlankCycle++;
	}
}




void z8536_SetPortOutputs(int Port, int Data)
{
	if (Port==0)
	{
		Printer_Write7BitData(Data);

		/* strobe is bit 7, invert is handled by z8536 */
		Printer_SetStrobeState(((Data&0x080)!=0));
	}

}



    /*
 DB1, DB0
     0    0       mode 0
     0    1       mode 1
     1    0       mode 2
     1    1       not used (mode 3)

     DB1, DB0,    clock (TSR)
     0    0       4Mhz
     0    1       8Mhz
     1    0       16Mhz
     1    1       - (no clock)

     DB1  DB0     M3 (/DB1)      /M2
     0    0       1               1
     0    1       1               0
     1    0       0               0
     1    1       0               1

     A0 = DR7
     A1 = DR3 & M3
     A2 = DR5 & M2
     A3 = DR1 & M2
    

     DB1  DB0
     0    0       A0,A1,A2,A3
     0    1       A0,A1
     1    0       A0
     1    1       A0,A2,A3



     BL1, BL2, RT1, RT2, GN1, GN2
     0,1,2,3,4,5,6
    */

void KCC_DoDispEnable(BOOL bDispEnable)
{

    /* DISEN/CCLK enter D074,
     go into D15,D10

     D10 has VSYNC, HSYNC and DISEN
     D20 E
     D15 has DISEN and DB4
     /FN C clear
     /FW E enable goes to ME on rams */
	if (bDispEnable)
	{

		BlankingOutput &= ~DISPTMG_ACTIVE;
		Computer_UpdateGraphicsFunction();
	}
	else
	{
		BlankingOutput |= DISPTMG_ACTIVE;
		Computer_UpdateGraphicsFunction();
	}
}

void    KCC_UpdateVsync(BOOL bState)
{
	BOOL bCurrentState = ((CRTCSyncInputs & VSYNC_INPUT) != 0);
	if (bState == bCurrentState)
		return;

    if (bState)
    {
		CRTCSyncInputs |= VSYNC_INPUT;
		BlankingOutput |= VBLANK_ACTIVE;

        Monitor_DoVsyncStart();

        /* set interrupt request counter */
        InterruptRequestResetCount = 2;

        z8536.inputs[2] |= 0x04;
        Z8536_Update();

        KCC_RefreshInterrupts();
		KCC_UpdateGraphicsFunction();
	}
    else
    {
		BlankingOutput &= ~VBLANK_ACTIVE;
		CRTCSyncInputs &= ~VSYNC_INPUT;

		Monitor_DoVsyncEnd();

        z8536.inputs[2] &= ~0x04;
        Z8536_Update();
		KCC_UpdateGraphicsFunction();
	}
}

int KCC_GetFN(void)
{
    return PenIndex;
}

int KCC_GetFW(void)
{
    return ColourSelection;
}


int KCC_GetMF(void)
{
    return MultiFunction;
}

int KCC_GetColour(int nIndex)
{
    return PenColour[nIndex];
}

void KCC_SetFN(int nIndex)
{
    PenIndex = nIndex;
}

void KCC_SetFW(int nIndex)
{
    ColourSelection = nIndex;
}

void KCC_SetMF(int nData)
{
    MultiFunction = nData;
}

void KCC_SetColour(int nIndex, int nData)
{
    PenColour[nIndex] = nData;
}

void    KCC_GA_Write(int Function)
{
    switch (Function & 0x0c0)
    {
		case 0x000:
		{

		    /* 5    4   3   2   1   0
		     DB4 DB4 DB3 DB2 DB1 DB0 */



			/* function 00xxxxxx */
			PenSelection = (unsigned char)Function;

			if (Function & 0x010)
			{
				PenIndex = (unsigned char)16;
			}
			else
			{
				PenIndex = (unsigned char)(Function & 0x0f);
			}
		}
		break;

		case 0x040:
		{
			/* function 01xxxxxx */
          /* any IOWR will lookup colour from ROM
             using data */
/*            int ColourAddress = Data & 0x0ff;
            int Colour = pColorROM[ColourAddress] & 0x03f;
            int D1 = Colour & 0x0f;
            int D2 = (Colour>>4) & 0x03)|(3<<2);
*/
			int		ColourIndex;
			unsigned char HWColor;

			ColourSelection = (unsigned char)Function;

			ColourIndex = Function & 0x01f;

			PenColour[PenIndex] = (unsigned char)ColourIndex;

            /* now lookup in ROM */
            HWColor = pColorROM[ColourIndex];


            Render_SetColour(&KCC_DisplayColours[HWColor],PenIndex);
		}
		break;

		case 0x080:
        {
			/* function 10xxxxxx */

			MultiFunction = (unsigned char)Function;

            if (Function & 0x010)
            {
                /* seems to be a delay in the output */
                KCC_AcknowledgeInterrupt();
            }

			Computer_RethinkMemory();

		}
		break;

		default:
            break;
	}
}

Z80_BYTE        KCCompact_In(Z80_WORD Port)
{
        Z80_BYTE Data=0x0ff;


		if ((Port & 0x01000)==0)
		{
			int Index;

			Index = ((Port^0x0100) & 0x0300)>>8;

			Data = Z8536_ReadData(Index);
		}


        if ((Port & 0x0800)==0)
        {
            unsigned int            Index;

            Index = (Port & 0x0300)>>8;
            if (Index==3)
            {
                Data = PPI_ReadControl();
            }
            else
            {
                Data = PPI_ReadPort(Index);
            }

        }


        if ((Port & 0x04000)==0)
        {
            unsigned int            Index;

            Index = (Port & 0x0300)>>8;

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

				default:
					break;
			}
        }

        CPC_ExecuteReadPortFunctions(Port, &Data);

        /* now handle ports which will latch data when read */
        if ((Port & 0x04000)==0)
        {
            unsigned int            Index;

            Index = (Port & 0x0300)>>8;

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

				default:
					break;
			}
        }


        return (Z80_BYTE)Data;
}

void	KCCompact_Out(const Z80_WORD Port, const Z80_BYTE Data)
{

	if ((Port & 0x08000)==0)
    {
		/* gate array can be selected if CRTC is also
		selected */
        KCC_GA_Write(Data);
    }

    if ((Port & 0x04000)==0)
    {
		/* crtc selected */

        unsigned int            Index;

        Index = (Port>>8) & 0x03;

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

			default:
				break;
		}
	}

	if ((Port & 0x01000)==0)
	{
		/* CIO chip */
		unsigned int Index;

		Index = ((Port^0x0100) & 0x0300)>>8;

		Z8536_WriteData(Index, Data);
	}

	if ((Port & 0x0800)==0)
    {
        unsigned int            Index;

        Index = (Port & 0x0300)>>8;
        if (Index==3)
        {
            PPI_WriteControl(Data);
        }
        else
        {
            PPI_WritePort(Index,Data);
        }
    }
}

