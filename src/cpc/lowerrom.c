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
/* Bryce's LowerROM emulation */
#include "cpc.h"
#include "emudevice.h"

static unsigned char LowerROMOS[16384*2];
static unsigned char LowerROMBASIC[16384*2];

/* LowerROM enable */
static BOOL LowerROM_SV1 = TRUE;
/* LowerROM selection */
static BOOL LowerROM_SV2 = FALSE;

BOOL LowerROM_GetSV1(void)
{
    return LowerROM_SV1;
}

void LowerROM_SetSV1(BOOL bState)
{
    LowerROM_SV1 = bState;
}


BOOL LowerROM_GetSV2(void)
{
    return LowerROM_SV2;
}

void LowerROM_SetSV2(BOOL bState)
{
    LowerROM_SV2 = bState;
}

void LowerROM_SetOS1(const unsigned char *pROM, unsigned long RomLength)
{
  EmuDevice_CopyRomData(&LowerROMOS[0x0], 16384, pROM, RomLength);
}

void LowerROM_ClearOS1(void)
{
	EmuDevice_ClearRomData(&LowerROMOS[0x0], 16384);
}

void LowerROM_SetOS2(const unsigned char *pROM, unsigned long RomLength)
{
  EmuDevice_CopyRomData(&LowerROMOS[16384], 16384, pROM, RomLength);
}

void LowerROM_ClearOS2(void)
{
	EmuDevice_ClearRomData(&LowerROMOS[16384], 16384);
}


void LowerROM_SetBASIC1(const unsigned char *pROM, unsigned long RomLength)
{
  EmuDevice_CopyRomData(&LowerROMBASIC[0x0], 16384, pROM, RomLength);
}

void LowerROM_ClearBASIC1(void)
{
	EmuDevice_ClearRomData(&LowerROMBASIC[0x0], 16384);
}

void LowerROM_SetBASIC2(const unsigned char *pROM, unsigned long RomLength)
{
  EmuDevice_CopyRomData(&LowerROMBASIC[16384], 16384, pROM, RomLength);
}

void LowerROM_ClearBASIC2(void)
{
	EmuDevice_ClearRomData(&LowerROMBASIC[16384], 16384);
}

static EmuDeviceRom LowerROMRom[4]=
{
	{
		"LowerROM OS 1",
		"LowerRomOS1",
		LowerROM_SetOS1,
		LowerROM_ClearOS1,
	  16384,
		0   /* ROM CRC - todo */
	},
		{
		"LowerROM OS 2",
		"LowerRomOS2",
		LowerROM_SetOS2,
		LowerROM_ClearOS2,
	  16384,
		0   /* ROM CRC - todo */
	},
		{
		"LowerROM BASIC 1",
		"LowerRomBASIC1",

		LowerROM_SetBASIC1,
		LowerROM_ClearBASIC1,
		16384,
		0   /* ROM CRC - todo */
	},
		{
		"LowerROM BASIC 2",
		"LowerRomBASIC2",
		LowerROM_SetBASIC2,
		LowerROM_ClearBASIC2,
	  16384,
		0   /* ROM CRC - todo */
	}

};

void LowerROM_RethinkMemory(MemoryData *pData)
{
	/* SV1 is lower rom enable */
    if (LowerROM_SV1)
    {
		/* SV2 chooses which lower rom */
		int Offset = LowerROM_SV2 ? (1<<14) : 0;
		unsigned char *pOS = &LowerROMOS[Offset];
		unsigned char *pBasic = &LowerROMBASIC[Offset];
		/* doesn't use ROMEN */
		/* LowerROM respects ROMDIS */
		if (pData->bRomEnable[0] && !pData->bRomDisable[0])
		{
			pData->bRomDisable[0] = TRUE;
			pData->pReadPtr[0] = pOS;
		}
		if (pData->bRomEnable[1] && !pData->bRomDisable[1])
		{
			pData->bRomDisable[1] = TRUE;
			pData->pReadPtr[1] = pOS;
		}
		if (pData->bRomEnable[6] && !pData->bRomDisable[6])
		{
			pData->bRomDisable[6] = TRUE;
			pData->pReadPtr[6] = pBasic-0x0c000;
		}
		if (pData->bRomEnable[7] && !pData->bRomDisable[7])
		{
			pData->bRomDisable[7] = TRUE;
			pData->pReadPtr[7] = pBasic-0x0c000;
		}
    }
}

void LowerROM_Reset(void)
{
	Computer_RestartReset();
}

static EmuDeviceButton LowerROMButtons[1]=
{
  {
      "Reset Button",
      LowerROM_Reset
  },
};

static EmuDeviceSwitch LowerROMSwitches[2]=
{
  {
	  "SV1 - LowerROM Enable",
	  "SV1",
	  LowerROM_GetSV1,
	  LowerROM_SetSV1
  },
  {
      "SV2 - Choose firmware set 1/2",
	  "SV2",
      LowerROM_GetSV2,
	  LowerROM_SetSV2
  }
};

static EmuDevice LowerROMDevice=
{
	NULL,
	NULL,
	NULL,
	"LOWERROM",
	"LowerRom",
	"Bryce's LowerROM",
    CONNECTION_EXPANSION,
	DEVICE_FLAGS_HAS_PASSTHROUGH|DEVICE_WORKING| DEVICE_FLAGS_FROM_SPECIFICATION,
  0,
  NULL,
  0,
  NULL,
  0,                /* no memory read*/
  NULL,
  0,                /* no memory write */
  NULL,
  NULL,
  LowerROM_RethinkMemory,
  NULL,
   	sizeof(LowerROMSwitches)/sizeof(LowerROMSwitches[0]),
	LowerROMSwitches,
    sizeof(LowerROMButtons)/sizeof(LowerROMButtons[0]),                      
    LowerROMButtons,
    sizeof(LowerROMRom)/sizeof(LowerROMRom[0]),                      
    LowerROMRom,
    NULL,                    /* no cursor update */
    NULL,
	NULL,					/* printer */
	NULL,					/* joystick */
	0,
	NULL,					/* memory ranges */
	NULL, /* sound */
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	NULL, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
	NULL
};

void LowerROM_Init(void)
{
	RegisterDevice(&LowerROMDevice);
}









