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
#include "cpc.h"
#include "emudevice.h"

const unsigned char DS1315_RecognitionSequence[8] = {0xC5, 0x3A, 0xA3, 0x5C, 0xC5, 0x3A, 0xA3, 0x5C};
int SequencePosition = 0;
int Recognition = 0;
int Cycles = 0;

int Registers[8];

void ds1315_reset(void)
{
	Cycles = 0;
	Recognition = 0;
}

void ds1315_write(Z80_BYTE Data)
{
	if (!Recognition)
	{
		/* write updates sequence if it matches */
		
		Z80_BYTE SequenceByte = DS1315_RecognitionSequence[SequencePosition>>8];
		Z80_BYTE SequenceBit = (SequenceByte>>(7-(SequencePosition&0x07)))&0x01;
		
		/* move if recognised, no move if not recognised */
		if (SequenceBit==Data)
		{
			if (SequencePosition==64)
			{
				/* we have recognition */
				Recognition = 1;
				Cycles = 0;
			}
			else
			{
				/* update sequence */
				SequencePosition++;
			}
		}
	}
	else
	{
		/* we have recognition */
		
		/* do write */
		Z80_BYTE SequenceByte = Registers[Cycles>>8];
		SequenceByte &=~(1<<((7-(Cycles&0x07))));
		SequenceByte |=Data<<(7-(Cycles&0x07));
		Registers[(Cycles>>8)] = SequenceByte;
		Cycles++;
		Cycles = Cycles&0x07;
		
		if (Cycles==0)
		{
			Recognition = 0;
		}
	}
}

Z80_BYTE ds1315_read(void)
{
	if (!Recognition)
	{
		/* we don't have recognition; reset */
		SequencePosition = 0;
	}
	else
	{
		/* we have recognition */
		
		/* do read */
		Z80_BYTE SequenceByte = Registers[Cycles>>8];
		Z80_BYTE SequenceBit = (SequenceByte>>(7-(Cycles&0x07)))&0x01;

		Cycles++;
		Cycles = Cycles&0x07;
		
		if (Cycles==0)
		{
			Recognition = 0;
		}
		
		return SequenceBit;
	}

	/* Correct?? */
	return 0x0ff;
}



int DXSRealTimeClockSwitches = 0;
unsigned char SystemRom[16384];
int DXSRealTimeClockRomIndex;

void DXSRealTimeClock_MemoryRethinkHandler(MemoryData *pData)
{
	if (pData->bRomEnable[6] && (DXSRealTimeClockRomIndex==((DXSRealTimeClockSwitches&0x070)|0x02)))
	{
		/* disable other roms in the chain */
		pData->bRomDisable[6] = TRUE;
		pData->bRomDisable[7] = TRUE;

		unsigned char *pRamPtr = SystemRom - 0x0c000;
		pData->pReadPtr[7] = pRamPtr;
		pData->pReadPtr[6] = pRamPtr;
	}
}


void DXSRealTimeClockSwitches_SW1Set(BOOL bState)
{
	DXSRealTimeClockSwitches &= ~(1 << 6);
	if (bState)
	{
		DXSRealTimeClockSwitches |= (1 << 6);
	}
}

void DXSRealTimeClockSwitches_SW2Set(BOOL bState)
{
	DXSRealTimeClockSwitches &= ~(1 << 5);
	if (bState)
	{
		DXSRealTimeClockSwitches |= (1 << 5);
	}
}

void DXSRealTimeClockSwitches_SW3Set(BOOL bState)
{
	DXSRealTimeClockSwitches &= ~(1 << 4);
	if (bState)
	{
		DXSRealTimeClockSwitches |= (1 << 4);
	}
}


BOOL DXSRealTimeClockSwitches_SW1Get(void)
{
	return ((DXSRealTimeClockSwitches & (1 << 6)) != 0);
}

BOOL DXSRealTimeClockSwitches_SW2Get(void)
{
	return ((DXSRealTimeClockSwitches & (1 << 5)) != 0);
}

BOOL DXSRealTimeClockSwitches_SW3Get(void)
{
	return ((DXSRealTimeClockSwitches & (1 << 4)) != 0);
}


static EmuDeviceSwitch DXSRealTimeClockDevice_Switches[3] =
{
	{
		"SW1 (On = External, Off = Internal)",
		"SW1",
		DXSRealTimeClockSwitches_SW1Get,
		DXSRealTimeClockSwitches_SW1Set
	},
	{
		"SW2 (On = External, Off = Internal)",
		"SW2",
		DXSRealTimeClockSwitches_SW2Get,
		DXSRealTimeClockSwitches_SW2Set,
	},
	{
		"SW3 (On = External, Off = Internal)",
		"SW3",
		DXSRealTimeClockSwitches_SW3Get,
		DXSRealTimeClockSwitches_SW3Set,
	}
};


void DXSRealTimeClock_SetROM(const unsigned char *pROM, unsigned long RomLength)
{
	EmuDevice_CopyRomData(SystemRom, sizeof(SystemRom), pROM, RomLength);
}

void DXSRealTimeClock_ClearROM(void)
{
	EmuDevice_ClearRomData(SystemRom, sizeof(SystemRom));
}


static EmuDeviceRom DXSRealTimeClockRom[1] =
{
	{
		"Real Time Clock System Rom",
	"SystemRom",
	DXSRealTimeClock_SetROM,
	DXSRealTimeClock_ClearROM,
	16384,
	0   /* ROM CRC - todo */
}
};

void DXSRealTimeClockDevice_Write(Z80_WORD Port, Z80_BYTE Data)
{
	DXSRealTimeClockRomIndex = Data;

	Computer_RethinkMemory();
}

Z80_BYTE DXSRealTimeClockDevice_Read(Z80_WORD Addr)
{
	/* A2 = 1 -> read, 0->write */
	/* A0 = data when write */
	if (Addr & (1 << 2))
	{
		/* read */
		return ds1315_read();
	}
	else
	{
		ds1315_write((Addr & 1));
		return 0x0ff;
	}
}


static CPCPortWrite DXSRealTimeClockPortWrite[1]=
{
	{
	0x01000, /* and */
	0x0000, /* cmp */
	DXSRealTimeClockDevice_Write
	}
};


static EmuDevice DXSRealTimeClockDevice=
{
	NULL,
	NULL,
	NULL,
	"SMARTWATCH",
	"dxsrtc",
	"dxs's Real Time Clock",
    0,   /* connects to rom slot */
	0,
    0,                /* no read ports */
  NULL,
  sizeof(DXSRealTimeClockPortWrite)/sizeof(DXSRealTimeClockPortWrite[0]),                    /* 1 write ports */
  DXSRealTimeClockPortWrite,
  0,                /* no memory read*/
  NULL,
  0,                /* no memory write */
  NULL,
  NULL, /* no reset function */
  DXSRealTimeClock_MemoryRethinkHandler, /* no rethink function */
  NULL, /* no power function */
  sizeof(DXSRealTimeClockDevice_Switches) / sizeof(DXSRealTimeClockDevice_Switches[0]),                      /* no switches */
	DXSRealTimeClockDevice_Switches,
    0,                      /* no buttons */
    NULL,
	sizeof(DXSRealTimeClockRom) / sizeof(DXSRealTimeClockRom[0]),                      /* no onboard roms */
	DXSRealTimeClockRom,
	NULL,	/* no cursor update function */
    NULL,
	NULL,	/* printer */
	NULL,	/* joystick */
	0,
	NULL,
	NULL,	/* no sound */
	NULL, /* lpen */
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	NULL, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
	NULL
};

void DXSRealTimeClock_Init(void)
{
	RegisterDevice(&DXSRealTimeClockDevice);
}










