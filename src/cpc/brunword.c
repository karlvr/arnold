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

/* Brunword MK2 emulation */
#include "cpc.h"
#include "brunword.h"
#include "emudevice.h"

/* ROMDIS?? */
/* ROMEN?? */
/* CHECK: */
/* what happens on reset...? */
/* /EXP? */
/* IM2? */

static int BrunWordMK2_ROMSelected[4] = { 0, 0, 0, 0 };
static BOOL BrunWordMK2_ROMEnabled[4]={0,0,0,0};
static unsigned char BrunWordMK2RomData[256 * 1024];

/*
bit 6 must be 0:

bit 7 is ignored
bits  5..0 are rom select

rom is enabled/disabled using normal upper rom disable

rom repeats every 64.

only page 1,2,3 are visible.

write through to ram works.

bit 6 must be 1:

x11pperr

bit 7 is ignored
bit 6 must be 1, bit 5 must be 1,
if bit 2 is 0: rom is readable in range &4000.
if bit 2 is 1: rom is not readable in range &4000.

rr is one of 4 16k pages, pp is which 64k bank.

only brunword roms are selectable.

this means roms repeat 2 times, blocks limited to 0-3, and aligned to 8 rom select boundary

rom is always enabled in that range

write through works

page at &4000 can be visible at the same time as one at &c000.
*/


void BrunWordMK2_Reset(void)
{
	BrunWordMK2_ROMEnabled[1] = BrunWordMK2_ROMEnabled[3] = FALSE;
}

void BrunWordMK2_Power(void)
{
	BrunWordMK2_ROMEnabled[1] = BrunWordMK2_ROMEnabled[3] = FALSE;
}

void	BrunWordMK2_ROMSelect(Z80_WORD Port, Z80_BYTE Data)
{
  /* bit 6 must be 1, bit 5 must be 1 */
    if ((Data & ((1<<6)|(1<<5)))==((1<<6)|(1<<5)))
    {
      int nRomPage;
      /* rom in range &4000-&7ffff now visible */
      if ((Data & (1<<2))==0)
      {
          BrunWordMK2_ROMEnabled[1] = TRUE;
      }
      else
      {
         /* lower rom not enabled */
          BrunWordMK2_ROMEnabled[1] = FALSE;
      }
      /* bank & page convert to rom number to select; all can be selected */

      nRomPage = ((Data>>1) & 0x0c) | (Data & 0x03);
      BrunWordMK2_ROMSelected[1] = nRomPage;
	  BrunWordMK2_ROMEnabled[3] = FALSE;
	}
	else
	{
    int nRomSelect = Data & 0x03f;

    /* 3 rom pages are selected using upper rom port, these are repeated every 64 */
    if ((nRomSelect>=1) && (nRomSelect<=3))
	    {
	        BrunWordMK2_ROMEnabled[3] = TRUE;
            BrunWordMK2_ROMSelected[3] = nRomSelect;
        }
        else
        {
            BrunWordMK2_ROMEnabled[3] = FALSE;
        }
	}

    Computer_RethinkMemory();
}

void BrunWordMK2_SetROM(const unsigned char *pROM, const unsigned long RomLength)
{
	EmuDevice_CopyRomData(BrunWordMK2RomData, sizeof(BrunWordMK2RomData), pROM, RomLength);
}

void BrunWordMK2_ClearROM(void)
{
	EmuDevice_ClearRomData(BrunWordMK2RomData, sizeof(BrunWordMK2RomData));
}

void BrunWordMK2_MemoryRethink(MemoryData *pData)
{
    if (pData->bRomEnable[6] && BrunWordMK2_ROMEnabled[3])
    {
        int RomIndex = BrunWordMK2_ROMSelected[3];
        int RomOffset = RomIndex<<14;
		const unsigned char *pROMData = &BrunWordMK2RomData[RomOffset] - 0x0c000;
        pData->bRomDisable[7] = TRUE;
        pData->bRomDisable[6] = TRUE;
        pData->pReadPtr[7] = pROMData;
        pData->pReadPtr[6] = pROMData;

    }

    /* always enabled */
    if (BrunWordMK2_ROMEnabled[1])
    {
        int RomIndex = BrunWordMK2_ROMSelected[1];
        int RomOffset = RomIndex<<14;
		const unsigned char *pROMData = &BrunWordMK2RomData[RomOffset] - 0x04000;
		/* RAMDIS on read, but not on write */
        pData->pReadPtr[3] = pROMData;
        pData->pReadPtr[2] = pROMData;
		/* technically RAMDIS on read and not write */
		pData->bRomEnable[3] = TRUE; 
		pData->bRomEnable[2] = TRUE;
    }
}

CPCPortWrite BrunWordMK2SelectWrite=
{
	/* CONFIRMED */
    /* A13 = 0 */
    0x02000,            /* and */
    0x00000,            /* compare */
    BrunWordMK2_ROMSelect
};

static EmuDeviceRom BrunwordMk2Rom=
{
    "BrunWord Mk2 System ROM",
	"SystemRom",
    BrunWordMK2_SetROM,
	BrunWordMK2_ClearROM,
	sizeof(BrunWordMK2RomData),
  0   /* ROM CRC - todo */
};

static EmuDevice BrunwordMk2Device=
{
	NULL,
	NULL,
	NULL,
	"BRUNWORDMK2",
	"BrunwordMk2",
	"Brunword Mk 2",
	CONNECTION_EXPANSION,   /* connects to expansion */
	DEVICE_FLAGS_TESTED| DEVICE_WORKING,
   0,   /* no read port */
  NULL,
  1,    /* 1 write port */
  &BrunWordMK2SelectWrite,
  0,                /* no memory read*/
  NULL,
  0,                /* no memory write */
  NULL,
  BrunWordMK2_Reset,
  BrunWordMK2_MemoryRethink,
  BrunWordMK2_Power,
	0,                      /* no switches */
	NULL,
    0,                      /* no buttons */
    NULL,
    1,                      /* 1 onboard roms */
    &BrunwordMk2Rom,
  NULL,                    /* no cursor function */
    NULL,                   /* no generic roms */
	NULL,					/* printer */
	NULL,					/* joystick */
	0,
	NULL,				/* memory range list */
	NULL,	/* no sound */
	NULL,	/* lpen */
		NULL, /* reti */
	NULL, /* ack maskable interrupt */
	NULL, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
};

void BrunWordMk2_Init(void)
{
	RegisterDevice(&BrunwordMk2Device);
}


/* Brunword MK4 emulation */

static int BrunWordMK4_ROMSelected[4] = { 0, 0, 0, 0 };
static BOOL BrunWordMK4_ROMEnabled[4] = { 0, 0, 0, 0 };
static unsigned char BrunwordMk4RomData[512 * 1024];

/*
based on MK2. Details are not as exact for MK4.

bit 6 must be 0:

bit 7 is ignored
bits  5..0 are rom select

rom is enabled/disabled using normal upper rom disable

rom repeats every 64.

only page 1,2,3 are visible.

write through to ram works.

11ppperr

bit 7 must be 1, bit 6 must be 1,
if bit 2 is 0: rom is readable in range &4000.
if bit 2 is 1: rom is not readable in range &4000.

rr is one of 4 16k pages, pp is which 64k bank.

only brunword roms are selectable.

this means roms repeat 2 times, blocks limited to 0-3, and aligned to 8 rom select boundary

rom is always enabled in that range

write through works

page at &4000 can be visible at the same time as one at &c000.
*/


void BrunWordMK4_Reset(void)
{
	BrunWordMK4_ROMEnabled[1] = BrunWordMK2_ROMEnabled[3] = FALSE;
}

void BrunWordMK4_Power(void)
{
	BrunWordMK4_ROMEnabled[1] = BrunWordMK2_ROMEnabled[3] = FALSE;
}

void	BrunWordMK4_ROMSelect(Z80_WORD Port, Z80_BYTE Data)
{
  /* bit 6 must be 1, bit 5 must be 1 */
    if ((Data & ((1<<7)|(1<<6)))==((1<<7)|(1<<6)))
    {
      int nRomPage;
      /* rom in range &4000-&7ffff now visible */
      if ((Data & (1<<2))==0)
      {
          BrunWordMK4_ROMEnabled[1] = TRUE;
      }
      else
      {
         /* lower rom not enabled */
          BrunWordMK4_ROMEnabled[1] = FALSE;
      }
      /* bank & page convert to rom number to select; all can be selected */

      nRomPage = ((Data>>1) & 0x01c) | (Data & 0x03);
      BrunWordMK4_ROMSelected[1] = nRomPage;
	  BrunWordMK4_ROMEnabled[3] = FALSE;
	}
	else
	{
    int nRomSelect = Data & 0x01f;

    /* 3 rom pages are selected using upper rom port, these are repeated every 64 */
    if ((nRomSelect>=1) && (nRomSelect<=3))
	    {
	        BrunWordMK4_ROMEnabled[3] = TRUE;
            BrunWordMK4_ROMSelected[3] = nRomSelect;
        }
        else
        {
            BrunWordMK4_ROMEnabled[3] = FALSE;
        }
	}

    Computer_RethinkMemory();
}

void BrunWordMK4_SetROM(const unsigned char *pROM, const unsigned long RomLength)
{
	EmuDevice_CopyRomData(BrunwordMk4RomData, sizeof(BrunwordMk4RomData), pROM, RomLength);
}

void BrunWordMK4_ClearROM(void)
{
	EmuDevice_ClearRomData(BrunwordMk4RomData, sizeof(BrunwordMk4RomData));
}

void BrunWordMK4_MemoryRethink(MemoryData *pData)
{
    if (pData->bRomEnable[6] && BrunWordMK4_ROMEnabled[3])
    {
        int RomIndex = BrunWordMK4_ROMSelected[3];
        int RomOffset = RomIndex<<14;
		const unsigned char *pROMData = &BrunwordMk4RomData[RomOffset] - 0x0c000;
        pData->bRomDisable[7] = TRUE;
        pData->bRomDisable[6] = TRUE;
        pData->pReadPtr[7] = pROMData;
        pData->pReadPtr[6] = pROMData;

    }

    /* always enabled */
    if (BrunWordMK4_ROMEnabled[1])
    {
        int RomIndex = BrunWordMK4_ROMSelected[1];
        int RomOffset = RomIndex<<14;
		const unsigned char *pROMData = &BrunwordMk4RomData[RomOffset] - 0x04000;
        pData->bRomEnable[3] = TRUE;
        pData->bRomEnable[2] = TRUE;
        pData->pReadPtr[3] = pROMData;
        pData->pReadPtr[2] = pROMData;

    }
}

CPCPortWrite BrunWordMK4SelectWrite=
{
	/* port needs to be confirmed */
    /* A13 = 0 */
    0x02000,            /* and */
    0x00000,            /* compare */
    BrunWordMK4_ROMSelect
};

static EmuDeviceRom BrunwordMk4Rom=
{
    "BrunWord Mk4 System ROM",
	"SystemRom",
	BrunWordMK4_SetROM,
	BrunWordMK4_ClearROM,
	sizeof(BrunwordMk4RomData),
    0   /* ROM CRC - todo */

};

static EmuDevice BrunwordMk4Device=
{
	NULL,
	NULL,
	NULL,
	"BRUNWORDMK4",
	"BrunwordMk4",
	"Brunword Mk 4",
	CONNECTION_EXPANSION,   /* connects to expansion */
	0,
  0,
  NULL,
  1,
  &BrunWordMK4SelectWrite,
  0,                /* no memory read*/
  NULL,
  0,                /* no memory write */
  NULL,
  BrunWordMK4_Reset,
  BrunWordMK4_MemoryRethink,
  BrunWordMK4_Power,
	0,                      /* no switches */
	NULL,
    0,                      /* no buttons */
    NULL,
    1,                      /* 1 onboard roms */
    &BrunwordMk4Rom,
    NULL,                    /* no cursor function */
    NULL,                   /* no generic roms */
	NULL,					/* printer */
	NULL,					/* joystick */
	0,
	NULL,					/* memory range list */
	NULL,	/* no sound */
	NULL,	/* lpen */
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	NULL, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
};

void BrunWordMk4_Init(void)
{
	RegisterDevice(&BrunwordMk4Device);
}










