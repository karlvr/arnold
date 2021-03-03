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
/* ROMBO rom-board emulation */
#include "cpc.h"
#include "emudevice.h"

static int rombo_ROMSelected = 0;

/* ON - 8-15, OFF - 0-7 */
static BOOL rombo_link = TRUE;

static ExpansionRomData m_romboRoms;


BOOL rombo_GetLink(void)
{
	return rombo_link;
}

void rombo_SetLink(BOOL bState)
{
	rombo_link = bState;
}

void	rombo_ROMSelect(Z80_WORD Port, Z80_BYTE Data)
{
	rombo_ROMSelected = Data;
	Computer_RethinkMemory();
}

void rombo_MemoryRethink(MemoryData *pData)
{
	BOOL rombo_Enabled = FALSE;
	
	if (rombo_link)
	{
		if ((rombo_ROMSelected>=8) && (rombo_ROMSelected<=15))
		{
			rombo_Enabled = TRUE;
		}
	}
	else
	{
		if ((rombo_ROMSelected>=0) && (rombo_ROMSelected<=7))
		{
			rombo_Enabled = TRUE;			
		}
	}
	
	if (rombo_Enabled)
	{
		if (ExpansionRom_IsActive(&m_romboRoms, rombo_ROMSelected&0x07))
		{
			const unsigned char *pROMData = ExpansionRom_GetSafe(&m_romboRoms, rombo_ROMSelected&0x07) - 0x0c000;

			/* unknown if uses ROMEN */
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
	}
}

CPCPortWrite romboSelectWrite =
{
	0x02000,            /* and */
	0x00000,            /* compare */
	rombo_ROMSelect
};

static EmuDeviceSwitch romboSwitches[1] =
{
	{
		"Link - 0-7 or 8-15 - (On = 8-15, Off=0-7)",
		"LINK",
		rombo_GetLink,
		rombo_SetLink
	},
};

void romboDevice_Init(void)
{
	int i;

	ExpansionRom_Init(&m_romboRoms, NULL, NULL);
	/* 8 available roms, 0 to 7 or 8 to 15 */
	for (i = 0; i < 8; i++)
	{
		ExpansionRom_SetAvailableState(&m_romboRoms, i, TRUE);
	}
}

static EmuDevice romboDevice =
{
	NULL,
	romboDevice_Init,
	NULL,
	"rombo",
	"rombo",
	"ROMBO Rom-box",
	CONNECTION_EXPANSION,
	DEVICE_FLAGS_HAS_EXPANSION_ROMS | DEVICE_FLAGS_HAS_PASSTHROUGH,
	DEVICE_FLAGS_TESTED| DEVICE_WORKING,
	NULL,
	1,
	&romboSelectWrite,
	0,                /* no memory read*/
	NULL,
	0,                /* no memory write */
	NULL,
	NULL,
	rombo_MemoryRethink,
	NULL,
	sizeof(romboSwitches) / sizeof(romboSwitches[0]),
	romboSwitches,
	0,                      /* no buttons */
	NULL,
	0,                      /* no onboard roms */
	NULL,
	NULL,                    /* no cursor update */
	&m_romboRoms,
	NULL,					/* printer */
	NULL,					/* joystick */
	0,
	NULL,					/* memory ranges */
	NULL, /* sound */
	NULL, /* lpen */
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	NULL, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
	NULL

};

void rombo_Init(void)
{

	RegisterDevice(&romboDevice);
}









