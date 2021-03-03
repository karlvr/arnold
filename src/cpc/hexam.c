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
/* hexam - good */

#include "cpc.h"
#include "emudevice.h"
#include "riff.h"

static int hexam_ROMLatch = 0;
static unsigned char hexam_SystemROM1[16384];
static unsigned char hexam_SystemROM2[16384];

void hexam_SetROM1(const unsigned char *pROM, unsigned long RomLength)
{
	EmuDevice_CopyRomData(hexam_SystemROM1, sizeof(hexam_SystemROM1), pROM, RomLength);
}
void hexam_SetROM2(const unsigned char *pROM, unsigned long RomLength)
{
	EmuDevice_CopyRomData(hexam_SystemROM2, sizeof(hexam_SystemROM2), pROM, RomLength);
}


void hexam_ClearROM1(void)
{
	EmuDevice_ClearRomData(hexam_SystemROM1, sizeof(hexam_SystemROM1));
}

void hexam_ClearROM2(void)
{
	EmuDevice_ClearRomData(hexam_SystemROM2, sizeof(hexam_SystemROM2));
}


static EmuDeviceRom hexamRom[2] =
{
	{
		"hexam System ROM 1 (E-ROM)",
		"SystemRomE",
		hexam_SetROM1,
		hexam_ClearROM1,
		16384,
		0   /* ROM CRC - todo */
	},
	{
		"hexam System ROM 2 (C-ROM)",
		"SystemRomC",
		hexam_SetROM2,
		hexam_ClearROM2,
		16384,
		0   /* ROM CRC - todo */
	},

};


void	hexam_ROMSelect(Z80_WORD Port, Z80_BYTE Data)
{
	/* d0-d3 go into IC5 */
	hexam_ROMLatch = Data & 0x0f;

	Computer_RethinkMemory();

}

CPCPortWrite hexamPortWrites[1] =
{
	{
		0x02000,            /* and */
		0x00000,            /* compare */
		hexam_ROMSelect
	}
};


void hexam_MemoryRethinkHandler(MemoryData *pData)
{
	if (hexam_ROMLatch == 0x02)
	{
		/* uses ROMEN */
		if (pData->bRomEnable[6])
		{
			/* disable other roms in the chain */
			pData->bRomDisable[6] = TRUE;
			pData->bRomDisable[7] = TRUE;

			/* hexam ROM */
			unsigned char *pRomPtr = hexam_SystemROM1 - 0x0c000;
			pData->pReadPtr[7] = pRomPtr;
			pData->pReadPtr[6] = pRomPtr;
		}
	}
	if (hexam_ROMLatch == 0x08)
	{
		if (pData->bRomEnable[6])
		{
			/* disable other roms in the chain */
			pData->bRomDisable[6] = TRUE;
			pData->bRomDisable[7] = TRUE;

			/* hexam ROM */
			unsigned char *pRomPtr = hexam_SystemROM2 - 0x0c000;
			pData->pReadPtr[7] = pRomPtr;
			pData->pReadPtr[6] = pRomPtr;
		}
	}
}


static EmuDevice hexamDevice =
{
	NULL,
	NULL,
	NULL,
	"hexam",
	"hexam",
	"hexam",
	CONNECTION_EXPANSION,   /* connected to expansion */
	DEVICE_FLAGS_HAS_EXPANSION_ROMS | DEVICE_FLAGS_HAS_PASSTHROUGH|DEVICE_WORKING,
	0,                /* no read ports */
	NULL,
	1,                    /* 2 write ports */
	hexamPortWrites, /* the ports */
	0,			/* memory read */
	NULL,
	0,			/* memory write */
	NULL,
	NULL, /* reset function */
	hexam_MemoryRethinkHandler, /* memory rethink */
	NULL, /* power function */
	0,
	NULL,
	0,                      /* has 1 button which is reset; not currently emulated */
	NULL,
	2,                      /* no onboard roms */
	hexamRom,
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
	NULL,
};

void hexam_Init()
{
	RegisterDevice(&hexamDevice);
}









