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
/* AMRAM 2 emulation */
/* TODO: Does it expose itself as RAM or NOT? If not we need to implement
write with RAM and ROM enabled */
#include "cpc.h"
#include "emudevice.h"
#include "riff.h"

static BOOL AmramPage0Enabled = TRUE;
static BOOL AmramPage1Enabled = TRUE;
static int Amram_ROMSelected = 0;
static BOOL Amram_IOEnable = FALSE;
static BOOL Amram_SwitchEnable = FALSE;
static BOOL Amram_Enable = FALSE;
static unsigned char pAMRAM2_RAM[32768];
ExpansionRomData m_Amram2Roms;
static BOOL Amram_Active = FALSE;



MemoryRange Amram2_MemoryRanges[2] =
{
	{ "Amram RAM Page 0", RIFF_FOURCC_CODE('A', 'R', '0', '0'), FALSE, FALSE, pAMRAM2_RAM, 16384 },
	{ "Amram RAM Page 1", RIFF_FOURCC_CODE('A', 'R', '0', '1'), FALSE, FALSE, pAMRAM2_RAM + 16384, 16384 },
};

void    Amram2_UpdateEnable(void)
{
	Amram_Enable = (Amram_IOEnable && Amram_SwitchEnable);
	Computer_RethinkMemory();
}

void    Amram2_SetSwitch(BOOL bState)
{
	Amram_SwitchEnable = bState;

	Amram2_UpdateEnable();
}

BOOL    Amram2_IsSwitchEnabled(void)
{
	return Amram_SwitchEnable;
}


void    Amram2_Page0Enable(BOOL bState)
{
	AmramPage0Enabled = bState;

	Computer_RethinkMemory();
}

BOOL    Amram2_Page0Enabled(void)
{
	return AmramPage0Enabled;
}


void    Amram2_Page1Enable(BOOL bState)
{
	AmramPage1Enabled = bState;

	Computer_RethinkMemory();
}

BOOL    Amram2_Page1Enabled(void)
{
	return AmramPage1Enabled;
}

void	Amram2_Write(Z80_WORD Port, Z80_BYTE Data)
{
	/* only bit 0 of data is important */
	Amram_IOEnable = ((Data & 0x01) != 0);
	Amram2_UpdateEnable();
}



void	Amram2_ROMSelect(Z80_WORD Port, Z80_BYTE Data)
{
	Amram_Active = FALSE;
	Amram_ROMSelected = Data;

	/* no repeat, indicates it is fully decoded */
	/* 1-6 */
	if ((Data == 1) || (Data == 2))
	{
		/* ram */
		Amram_Active = TRUE;
	}
	else
	{
		/* ROM slots */
		if ((Data >= 3) && (Data <= 6))
		{
			if (ExpansionRom_IsActive(&m_Amram2Roms, Amram_ROMSelected))
			{
				Amram_Active = TRUE;
			}
		}
	}

	Computer_RethinkMemory();

}

CPCPortWrite amram2PortWrites[2] =
{
	{
		/* xxxxx10111111xxxx */
		0x00ff0,            /* and */
		0x00bf0,            /* compare */
		Amram2_Write
	},
	{
		/* CONFIRMED */
		0x02000,            /* and */
		0x00000,            /* compare */
		Amram2_ROMSelect
	}
};

void Amram2_Reset(void)
{
	/* confirmed; light goes out on reset */
	Amram_IOEnable = FALSE;
}

void Amram2_Power(void)
{
	/* confirmed light goes out on power on/off */
	Amram_IOEnable = FALSE;
}

void Amram2_MemoryRethinkHandler(MemoryData *pData)
{
	/* if it's slot 1 or 2 */
	if ((Amram_ROMSelected == 1) || (Amram_ROMSelected == 2))
	{
		if (
			((Amram_ROMSelected == 1) && (AmramPage0Enabled)) ||
			((Amram_ROMSelected == 2) && (AmramPage1Enabled))
			)
		{
			/* unknown if uses ROMEN */
			/* if rom enabled then it's readable */
			if (pData->bRomEnable[6] && Amram_Active)
			{
				/* AMRAM RAM */
				unsigned char *pRamPtr = pAMRAM2_RAM + ((Amram_ROMSelected - 1) << 14) - 0x0c000;

				/* disable other roms in the chain */
				pData->bRomDisable[6] = TRUE;
				pData->bRomDisable[7] = TRUE;

				pData->pReadPtr[7] = pRamPtr;
				pData->pReadPtr[6] = pRamPtr;
			}

			/* it's writeable even when rom is not enabled */
			if (Amram_Active && Amram_Enable)
			{
				/* AMRAM RAM */
				unsigned char *pRamPtr = pAMRAM2_RAM + ((Amram_ROMSelected - 1) << 14) - 0x0c000;

				/* write enabled */
				pData->pWritePtr[7] = pRamPtr;
				pData->pWritePtr[6] = pRamPtr;
				pData->bRamDisable[6] = TRUE; /* not correct, it writes to both it's ram and main ram */
				pData->bRamDisable[7] = TRUE;
			}
		}
	}
	else
	{
		/* if rom enabled then it's readable */
		if (pData->bRomEnable[6] && Amram_Active)
		{
			/* ROMS */
			const unsigned char *pROMData = ExpansionRom_GetSafe(&m_Amram2Roms, Amram_ROMSelected) - 0x0c000;
			pData->pReadPtr[7] = pROMData;
			pData->pReadPtr[6] = pROMData;
			pData->bRomDisable[6] = TRUE;
			pData->bRomDisable[7] = TRUE;
		}
	}
}

static EmuDeviceSwitch Amram2Switches[3] =
{
	{
		"Write Enable Switch",          /* the switch on the top */
		"WriteEnable",
		Amram2_IsSwitchEnabled,
		Amram2_SetSwitch
	},
	{
		"Amram 2 RAM Page 0 Enable", /* the dip switch for rom 0 */
		"Page0Enable",
		Amram2_Page0Enabled,
		Amram2_Page0Enable,
	},
	{
		"Amram 2 RAM Page 1 Enable", /* the dip switch for rom 1 */
		"Page1Enable",
		Amram2_Page1Enabled,
		Amram2_Page1Enable,
	}
};

void Amram2Device_Init(void)
{
	memset(pAMRAM2_RAM, 0x0ff, sizeof(pAMRAM2_RAM));
	/* 4 ROM slots */
	/* slot 1,2 are the amram roms */
	/* the others are the slots for EPROMs */
	/* TODO: Save of rom slots 1,2 */
	ExpansionRom_Init(&m_Amram2Roms, NULL, NULL);
	ExpansionRom_SetAvailableState(&m_Amram2Roms, 3, TRUE);
	ExpansionRom_SetAvailableState(&m_Amram2Roms, 4, TRUE);
	ExpansionRom_SetAvailableState(&m_Amram2Roms, 5, TRUE);
	ExpansionRom_SetAvailableState(&m_Amram2Roms, 6, TRUE);
}

static EmuDevice Amram2Device =
{
	NULL,
	Amram2Device_Init,
	NULL,
	"AMRAM2",
	"Amram2",
	"Silicon System's Amram 2",
	CONNECTION_EXPANSION,   /* connected to expansion */
	DEVICE_FLAGS_HAS_EXPANSION_ROMS | DEVICE_FLAGS_HAS_PASSTHROUGH| DEVICE_FLAGS_TESTED| DEVICE_WORKING,
	0,                /* no read ports */
	NULL,
	2,                    /* 2 write ports */
	amram2PortWrites, /* the ports */
	0,
	NULL,
	0,
	NULL,
	Amram2_Reset, /* reset function */
	Amram2_MemoryRethinkHandler, /* memory rethink */
	Amram2_Power, /* power function */
	sizeof(Amram2Switches) / sizeof(Amram2Switches[0]),                      /* 3 switch */
	Amram2Switches,
	0,                      /* has 1 button which is reset; not currently emulated */
	NULL,
	0,                      /* no onboard roms */
	NULL,
	NULL,	/* no cursor update function */
	&m_Amram2Roms,
	NULL,	/* printer */
	NULL,	/* joystick */
	sizeof(Amram2_MemoryRanges) / sizeof(Amram2_MemoryRanges[0]),
	Amram2_MemoryRanges,
	NULL,	/* no sound */
	NULL, /* lpen */
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	NULL, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
	NULL,
};

void Amram2_Init(void)
{

	RegisterDevice(&Amram2Device);
}









