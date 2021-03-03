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
#include "memory.h"
#include "vrtxram.h"
#include "emudevice.h"

/***********/
/* VORTEX  */

/*
Ram board is internal.
Ram board is designed for CPC464.

The GateArray IC and OS ROM EPROM must be removed. The RAM board connects to the main
PCB at the socket for ROM and GateArray.
Replace GateArray and OS ROM into the positions on the ram board.

Each RAM block is 32K.

2 I/O ports are used for access

FBBD: bank select:

bit 2,1,0: 64kb bank select
bit 3: 0 = enable rom, 1=disable rom (on-board BOS rom)
bit 4: 1 = ram disable (all ram)
bit 5: ramcard enable (0=disable, 1=enable); block access to 7fxx mapping


7fxx:
6 = block select (upper or lower)
5 = mapping enabled (1=switch block selected by bank and block)

*/

/* allocated ram */
static unsigned char *VortexRAM_Ram = NULL;
static unsigned char VortexRAM_Rom[16384]; /* TODO: Confirm BOS size*/
static int VortexRAM_BankSelect = 0;
static int VortexRAM_MappingEnable = 0;
static int Vortex_ROMSelected = 0;

void	Vortex_RamPageWrite(Z80_WORD Port, Z80_BYTE Data)
{
	VortexRAM_BankSelect = Data;
	Computer_RethinkMemory();
}

void	Vortex_RamConfigurationWrite(Z80_WORD Port, Z80_BYTE Data)
{
	VortexRAM_MappingEnable = Data;
	Computer_RethinkMemory();
}


void	Vortex_RomWrite(Z80_WORD Port, Z80_BYTE Data)
{
	Vortex_ROMSelected = Data;

	Computer_RethinkMemory();
}

static CPCPortWrite VortexRAMPortWrite[3]=
{
	{ 0x0442, 0x0000, Vortex_RamPageWrite },	/* decoding confirmed */
	{ 0x0ff00, 0x07f00, Vortex_RamConfigurationWrite }, /* TODO Exact decoding */
	{ 0x02000, 0x0000, Vortex_RomWrite },	/* decoding confirmed */
};

void VortexRAM_MemoryRethink(MemoryData *pData)
{


	if (VortexRAM_BankSelect & (1 << 4))
	{
		/* disable all ram */
		pData->bRamDisable[0] = TRUE;
		pData->bRamDisable[1] = TRUE;
		pData->bRamDisable[2] = TRUE;
		pData->bRamDisable[3] = TRUE;
		pData->bRamDisable[4] = TRUE;
		pData->bRamDisable[5] = TRUE;
		pData->bRamDisable[6] = TRUE;
		pData->bRamDisable[7] = TRUE;
	}
	else
	{
		if (
			/* mapping can be used */
			(VortexRAM_BankSelect & (1 << 5)) &&
			/* perform mapping */
			(VortexRAM_MappingEnable & (1 << 5))
			)
		{
			/* enabled */
			unsigned char *pRamBase = VortexRAM_Ram + ((VortexRAM_BankSelect & 0x07) << 16);
			if (VortexRAM_MappingEnable & (1 << 6))
			{
				/* 0x8000-0x0ffff */
				pData->bRamDisable[4] = TRUE;
				pData->bRamDisable[5] = TRUE;
				pData->bRamDisable[6] = TRUE;
				pData->bRamDisable[7] = TRUE;
				pData->pWritePtr[4] = pRamBase;
				pData->pWritePtr[5] = pRamBase;
				pData->pWritePtr[6] = pRamBase;
				pData->pWritePtr[7] = pRamBase;
				pData->pReadPtr[4] = pData->pWritePtr[4];
				pData->pReadPtr[5] = pData->pWritePtr[5];
				pData->pReadPtr[6] = pData->pWritePtr[6];
				pData->pReadPtr[7] = pData->pWritePtr[7];
			}
			else
			{
				/* 0x0000-0x07fff */
				pData->bRamDisable[0] = TRUE;
				pData->bRamDisable[1] = TRUE;
				pData->bRamDisable[2] = TRUE;
				pData->bRamDisable[3] = TRUE;
				pData->pWritePtr[0] = pRamBase;
				pData->pWritePtr[1] = pRamBase;
				pData->pWritePtr[2] = pRamBase;
				pData->pWritePtr[3] = pRamBase;
				pData->pReadPtr[0] = pData->pWritePtr[0];
				pData->pReadPtr[1] = pData->pWritePtr[1];
				pData->pReadPtr[2] = pData->pWritePtr[2];
				pData->pReadPtr[3] = pData->pWritePtr[3];
			}
		}
	}

	{
		BOOL bRomEnabledUpper = FALSE;
		BOOL bRomEnabledLower = FALSE;

		if (
			(((VortexRAM_BankSelect & (1 << 3)) == 0) && (Vortex_ROMSelected == 6)) ||
			((VortexRAM_BankSelect & (1 << 3)) == (1 << 3))
			)
		{
			bRomEnabledUpper = TRUE;
		}

		if ((VortexRAM_BankSelect & (1 << 3)) == (1 << 3))
		{
			bRomEnabledLower = TRUE;
		}

		if (bRomEnabledUpper)
		{
			const unsigned char *pROMData = VortexRAM_Rom - 0x0c000;

			/* unknown if it really uses ROMEN */
			/* ROMDIS? */
			if (pData->bRomEnable[6] && !pData->bRomDisable[6])
			{
				pData->bRomDisable[6] = TRUE;
				pData->pReadPtr[6] = pROMData;
			}
			if (pData->bRomEnable[7] && !pData->bRomDisable[7])
			{
				pData->bRomDisable[7] = TRUE;
				pData->pReadPtr[7] = pROMData;
			}
		}

		if (bRomEnabledLower)
		{
			const unsigned char *pROMData = VortexRAM_Rom;

			/* ROMDIS? */
			if (pData->bRomEnable[0] && !pData->bRomDisable[0])
			{
				pData->bRomDisable[0] = TRUE;
				pData->pReadPtr[0] = pROMData;
			}
			if (pData->bRomEnable[1] && !pData->bRomDisable[1])
			{
				pData->bRomDisable[1] = TRUE;
				pData->pReadPtr[1] = pROMData;
			}

		}
	}
}


void VortexRAM_Reset(void)
{
	/* confirmed: ROM is enabled on reset */
	
	/* TODO: Confirm reset state */
	VortexRAM_BankSelect = 0;
	VortexRAM_MappingEnable = 0;
	Computer_RethinkMemory();
}

void VortexRAM_SetROM(const unsigned char *pROM, unsigned long RomLength)
{
	EmuDevice_CopyRomData(VortexRAM_Rom, 16384, pROM, RomLength);
}

void VortexRAM_ClearROM(void)
{
	EmuDevice_ClearRomData(VortexRAM_Rom, 16384);
}


static EmuDeviceRom VortexRAMRom=
{
	"Vortex RAM System ROM (BOS)",
	"SystemRom",
	VortexRAM_SetROM,
	VortexRAM_ClearROM,
	16384,
	0   /* ROM CRC - todo */

};

void VortexRAMDevice_Init(void)
{
	VortexRAM_Ram = (unsigned char *)malloc(512 * 1024);
	memset(VortexRAM_Ram, 0x0ff, 512 * 1024);

}

void VortexRAMDevice_Shutdown(void)
{
	if (VortexRAM_Ram)
	{
		free(VortexRAM_Ram);
	}

}

static EmuDevice VortexRamDevice =
{
	NULL,
	VortexRAMDevice_Init,
	VortexRAMDevice_Shutdown,
	"VortexRam",
	"VortexRam",
	"Vortex Ram Expansion",
	CONNECTION_INTERNAL,   /* connects to expansion */
	DEVICE_FLAGS_TESTED|DEVICE_WORKING,					/* provides ram but not dk'tronics compatible */
	0,
	NULL,					/* no read ports */
	sizeof(VortexRAMPortWrite)/sizeof(VortexRAMPortWrite[0]),
	VortexRAMPortWrite,			/* 2 write port */
	0,                /* no memory read*/
	NULL,
	0,                /* no memory write */
	NULL, 
	VortexRAM_Reset,
	VortexRAM_MemoryRethink,
	VortexRAM_Reset,
	0,                      /* no switches */
	NULL,
	0,                      /* 2 buttons */
	NULL,
	1,                      /* 1 onboard roms */
	&VortexRAMRom, 
	NULL,                   /* no cursor function */
	NULL,                   /* no generic roms */
	NULL,					/* printer */
	NULL,					/* joystick */
	0,
	NULL,		/* memory ranges */
	NULL, /* sound */
	NULL, /* lpen */
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	NULL, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
	NULL
};

void VortexRAM_Init(void)
{
	RegisterDevice(&VortexRamDevice);
}

