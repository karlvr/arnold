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
/* Transtape emulation */
/* TODO: Capture memory read for rom enable state */
/* TODO: Capture ROMEN state (A13) */
/* TODO: Capture opcode read (0011 1xxx xxxx xxxx) */
/* TODO: FIX! */
#include "cpc.h"
#include "emudevice.h"
#include "riff.h"

/* ram is in the range &c000-&ffff and repeats */
static unsigned char TransTapeRamData[8192];

/* rom is in the range &0000-&3fff */
static unsigned char TransTapeRomData[16384];

/* bit 7 is upper/lower rom state */
static unsigned char TransTapeRomState = 0;

/* captured from rethink memory to determine ROM states*/
/* bit 0 used to store lower rom state, bit 1 used to store upper rom state */
static unsigned char TransTapeOSRomState=0;

/* bit 0 is ram enable/disable state */
/* bit 1 is rom enable/disable state */
/* bit 2 is A10 on RAM */
/* bit 3 is A11 on RAM */
/* bit 4 is A12 on RAM */
static unsigned char TransTapePortData = 0;


MemoryRange TransTape_MemoryRange[1]=
{
	{"Transtape RAM", RIFF_FOURCC_CODE('T','T','0','0'), FALSE, FALSE, TransTapeRamData, 8192},
};

void TransTape_SetROM(const unsigned char *pROM, unsigned long RomLength)
{
	EmuDevice_CopyRomData(TransTapeRomData, sizeof(TransTapeRomData), pROM, RomLength);
}
void TransTape_ClearROM(void)
{
	EmuDevice_ClearRomData(TransTapeRomData, sizeof(TransTapeRomData));
}

void TransTape_MemoryRethink(MemoryData *pData)
{
	/* really does use ROMEN */
	TransTapeOSRomState = 0;
	if (pData->bRomEnable[0] || pData->bRomEnable[1])
	{
		TransTapeOSRomState = (1 << 0);
	}
	if (pData->bRomEnable[6] || pData->bRomEnable[7])
	{
		TransTapeOSRomState = (1 << 1);
	}

	if ((TransTapePortData & (1 << 0)) != 0)
    {
		/* ram repeats in the memory range */
		unsigned char *pRamAddr = &TransTapeRamData[((TransTapePortData >> 1) << 10)];

		pData->pReadPtr[7] = pRamAddr-0x0e000;
		pData->pReadPtr[6] = pRamAddr-0x0c000;

		pData->pWritePtr[7] = pRamAddr-0x0e000;
		pData->pWritePtr[6] = pRamAddr-0x0c000;

		pData->bRamDisable[7] = TRUE;
		pData->bRamDisable[6] = TRUE;
		pData->bRomDisable[7] = TRUE;
		pData->bRomDisable[6] = TRUE;
    }

	if ((TransTapePortData & (1 << 1)) != 0)
    {
        unsigned char *pRomPtr = NULL;

        pRomPtr = TransTapeRomData;

        /* read only */
        pData->pReadPtr[1] = pRomPtr;
        pData->pReadPtr[0] = pRomPtr;
		pData->pWritePtr[1] = GetDummyWriteRam();
		pData->pWritePtr[0] = GetDummyWriteRam();

		/* disable existing rom; we provide a rom */
        pData->bRomDisable[1] = TRUE;
        pData->bRomDisable[0] = TRUE;
		/* disable ram behind rom */
		pData->bRamDisable[1] = TRUE;
		pData->bRamDisable[0] = TRUE;

    }
}

void TransTape_Reset(void)
{
    /* reset enables the rom; doesn't enable the ram */
	TransTapePortData = (1 << 1);
	Computer_RethinkMemory();

}

/* call here when Transtape stop button is pressed */
void    TransTape_Stop(void)
{
	/* pressing stop enables the rom but doesn't enable the ram */
	TransTapePortData = (1 << 1);

	/* not going to work.... */
    CPU_SetNMIState(FALSE);
    CPU_SetNMIState(TRUE);
    CPU_SetNMIState(FALSE);

	Computer_RethinkMemory();

}


static EmuDeviceRom TransTapeRom=
{
    "TransTape System ROM",
	"SystemRom",
    TransTape_SetROM,
	TransTape_ClearROM,
  16384,
    0   /* ROM CRC - todo */

};

static EmuDeviceButton TransTapeStopButton[2]=
{
    {
        "Stop Button (Red)",
        TransTape_Stop
    },
    {
        "Reset Button (Black)",
        TransTape_Reset
    }
};


BOOL TransTapeRead(Z80_WORD Port, Z80_BYTE *pDeviceData)
{
	*pDeviceData = 0x07f | TransTapeRomState;
	return TRUE;
}


void TransTapeWrite(Z80_WORD Port, Z80_BYTE Data)
{
	/* bit 0 */
    TransTapePortData = Data;
    Computer_RethinkMemory();
}


static CPCPortRead TransTapePortRead=
{
	0x0fffc, /* and */
    0x0fbfc, /* cmp */
    TransTapeRead
};

/* TODO: Do not return data, we want to return what is there, we are only interested in 
watching the cpu outputs */
BOOL TransTapeMemoryReadHandler(Z80_WORD Addr, Z80_BYTE *pDeviceData)
{
//	if (CPU_GetOutput() & (CPU_OUTPUT_RD|CPU_OUTPUT_MREQ)==(CPU_OUTPUT_RD|CPU_OUTPUT_MREQ))
//	{		
//		TransTapePortData = 0;
//		Computer_RethinkMemory();
//	}
	return FALSE;
}

static CPCPortRead TransTapeMemoryRead =
{
	0xf800, /* and */
	0x3800, /* cmp */
	TransTapeMemoryReadHandler
};


static CPCPortWrite TransTapePortWrite=
{
	0x0fffc, /* and */
	0x0fbf0, /* cmp */
	TransTapeWrite
};

static EmuDevice TransTapeDevice=
{
	NULL,
	NULL,
	NULL,
	"TRANSTAPE",
	"Transtape",
	"Transtape",
	CONNECTION_EXPANSION,   /* connects to expansion */
	DEVICE_FLAGS_FROM_SPECIFICATION,
  1,
  &TransTapePortRead,
  1,
 &TransTapePortWrite,
 1,                /* no memory read*/
 &TransTapeMemoryRead,
 0,                /* no memory write */
 NULL,
 TransTape_Reset,
  TransTape_MemoryRethink,
  TransTape_Reset,
	0,                      /* no switches */
	NULL,
    2,                      /* 2 buttons */
    TransTapeStopButton,
    1,                      /* 1 onboard roms */
    &TransTapeRom,
    NULL,                   /* no cursor function */
    NULL,                   /* no generic roms */
	NULL,
	NULL,
	sizeof(TransTape_MemoryRange)/sizeof(TransTape_MemoryRange[0]),
	TransTape_MemoryRange,
	NULL, /* sound */
	NULL, /* lpen */
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	NULL, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
	NULL
};

void TransTape_Init(void)
{
	memset(TransTapeRamData, 0x0ff, sizeof(TransTapeRamData));

	RegisterDevice(&TransTapeDevice);
}










