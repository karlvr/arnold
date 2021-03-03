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
/* Bryce's MegaROM emulation */
#include "cpc.h"
#include "megarom.h"
#include "emudevice.h"

static int MegaROM_ROMSelected = 0;
static BOOL MegaROM_Active = TRUE;

/* 128k/256k - allows 0-7 or 0-15 */
static BOOL MegaROM_J3 = TRUE;
/* Enable ROM 0 on MegaROM */
static BOOL MegaROM_J2 = FALSE;
/* Enable ROM 7 on MegaROM */
static BOOL MegaROM_J1 = FALSE;

static ExpansionRomData m_MegaRomRoms;


BOOL MegaROM_GetJ3(void)
{
	return MegaROM_J3;
}

void MegaROM_SetJ3(BOOL bState)
{
	MegaROM_J3 = bState;
}


BOOL MegaROM_GetJ1(void)
{
	return MegaROM_J1;
}

void MegaROM_SetJ1(BOOL bState)
{
	MegaROM_J1 = bState;
}

BOOL MegaROM_GetJ2(void)
{
	return MegaROM_J2;
}

void MegaROM_SetJ2(BOOL bState)
{
	MegaROM_J2 = bState;
}

void	MegaROM_ROMSelect(Z80_WORD Port, Z80_BYTE Data)
{
	BOOL bIgnore = TRUE;

	/* need to set/enable ROMDIS */
	MegaROM_Active = FALSE;
	MegaROM_ROMSelected = Data;

	/* always do rethink? */

	/* full decoding means we ignore numbers above 16 */
	if (Data < 16)
	{
		switch (Data)
		{
		case 0:
		{
			/* ignore if J2 is not set */
			if (MegaROM_J2)
			{
				bIgnore = FALSE;
			}
		}
		break;

		case 7:
		{
			/* ignore if J1 is not set */
			if (MegaROM_J1)
			{
				bIgnore = FALSE;
			}
		}
		break;

		default:
		{
			if (Data >= 8)
			{
				/* ignore if j3 is not set */
				if (MegaROM_J3)
				{
					bIgnore = FALSE;
				}
			}
			else
			{
				/* all others don't ignore */
				bIgnore = FALSE;
			}
		}
		break;

		}
	}

	if (!bIgnore)
	{
		if (ExpansionRom_IsActive(&m_MegaRomRoms, MegaROM_ROMSelected))
		{
			MegaROM_Active = TRUE;
		}
	}

	Computer_RethinkMemory();
}

void MegaROM_MemoryRethink(MemoryData *pData)
{
	if (MegaROM_Active)
	{
		const unsigned char *pROMData = ExpansionRom_GetSafe(&m_MegaRomRoms, MegaROM_ROMSelected) - 0x0c000;
		/* uses ROMEN */
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

CPCPortWrite megaROMSelectWrite =
{
	/* A15 = 1, A13 = 0 */
	0x0A000,            /* and */
	0x08000,            /* compare */
	MegaROM_ROMSelect
};

static EmuDeviceSwitch MegaRomSwitches[3] =
{
	{
		"J1 - 128k/256k - (On = Allow 0-15, Off=0-7 Only)",
		"J1",
		MegaROM_GetJ3,
		MegaROM_SetJ3
	},
	{
		"J2 - Enable ROM 0 on MegaROM",
		"J2",
		MegaROM_GetJ2,
		MegaROM_SetJ2
	},
	{
		"J3 - Enable ROM 7 on MegaROM ",
		"J3",
		MegaROM_GetJ1,
		MegaROM_SetJ1
	}
};

void MegaROMDevice_Init(void)
{
	int i;
	/* technically the megarom doesn't have slots, but as it's a ROM we
	treat it like it does */
	ExpansionRom_Init(&m_MegaRomRoms, NULL, NULL);
	/* 16 available roms, 0 to 15 */
	for (i = 0; i < 16; i++)
	{
		ExpansionRom_SetAvailableState(&m_MegaRomRoms, i, TRUE);
	}
}

static EmuDevice MegaromDevice =
{
	NULL,
	MegaROMDevice_Init,
	NULL,
	"MEGAROM",
	"Megarom",
	"Bryce's Megarom",
	CONNECTION_EXPANSION,
	DEVICE_FLAGS_HAS_EXPANSION_ROMS | DEVICE_FLAGS_HAS_PASSTHROUGH|DEVICE_WORKING| DEVICE_FLAGS_FROM_SPECIFICATION,
	0,
	NULL,
	1,
	&megaROMSelectWrite,
	0,                /* no memory read*/
	NULL,
	0,                /* no memory write */
	NULL,
	NULL,
	MegaROM_MemoryRethink,
	NULL,
	sizeof(MegaRomSwitches) / sizeof(MegaRomSwitches[0]),
	MegaRomSwitches,
	0,                      /* no buttons */
	NULL,
	0,                      /* no onboard roms */
	NULL,
	NULL,                    /* no cursor update */
	&m_MegaRomRoms,
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

void MegaROM_Init(void)
{

	RegisterDevice(&MegaromDevice);
}









