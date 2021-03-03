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

/* ROMDIS?? */

/* MirageImager emulation */
/* original ROM is scrambled:

D0-D7
D1-D5
D2-D3
D3-D1
D4-D0
D5-D2
D6-D4
D7-D6
*/
/* Mirage imager has two read/write ports which it uses to access on-board ram */
#include "cpc.h"
#include "emudevice.h"

static unsigned char MirageImagerRamData[8192];

static unsigned char MirageImagerRomData[8192];

/* bit 0 is rom enable/disable state */
static unsigned char MirageImagerRomState=0;

void MirageImager_SetROM(const unsigned char *pROM, const unsigned long RomLength)
{
    EmuDevice_CopyRomData(MirageImagerRomData, sizeof(MirageImagerRomData), pROM, RomLength);

}


void MirageImager_ClearROM(void)
{
	EmuDevice_ClearRomData(MirageImagerRomData, sizeof(MirageImagerRomData));

}

void MirageImager_MemoryRethink(MemoryData *pData)
{
    if ((MirageImagerRomState & (1<<0))!=0)
    {
        /* read only */
        pData->pReadPtr[7] = MirageImagerRamData-0x0c000;
        pData->pReadPtr[6] = MirageImagerRamData-0x0c000;

        pData->pWritePtr[7] = MirageImagerRamData-0x0c000;
        pData->pWritePtr[6] = MirageImagerRamData - 0x0c000;


			pData->bRamDisable[7] = FALSE;
			pData->bRamDisable[6] = FALSE;
			pData->bRomDisable[7] = TRUE;
			pData->bRomDisable[6] = TRUE;


    }

    if ((MirageImagerRomState & (1<<1))!=0)
    {
       unsigned char *pRomPtr = NULL;

        pRomPtr = MirageImagerRomData;

        /* read only */
        pData->pReadPtr[1] = pRomPtr;
        pData->pReadPtr[0] = pRomPtr;

        /* we provide a rom */
     //   pData->bRomEnable[0] = TRUE;
      //  pData->bRomEnable[1] = TRUE;
        pData->bRomDisable[0] = TRUE;
        pData->bRomDisable[1] = TRUE;

    }
}

void MirageImager_Reset(void)
{
    /* reset enables their rom */
    MirageImagerRomState = (1<<1);
	Computer_RethinkMemory();

}

/* call here when MirageImager stop button is pressed */
void    MirageImager_Stop(void)
{
    MirageImagerRomState = (1<<1);

/* not going to work.... */
    CPU_SetNMIState(FALSE);
    CPU_SetNMIState(TRUE);
    CPU_SetNMIState(FALSE);

	Computer_RethinkMemory();

}


static EmuDeviceRom MirageImagerRom=
{
    "MirageImager System ROM",
	"SystemRom",
    MirageImager_SetROM,
	MirageImager_ClearROM,
	sizeof(MirageImagerRomData),
    0   /* ROM CRC - todo */

};

static EmuDeviceButton MirageImagerStopButton[2]=
{
    {
        "Stop Button (Red)",
        MirageImager_Stop
    },
    {
        "Reset Button (Black)",
        MirageImager_Reset
    }
};


BOOL MirageImagerRead(Z80_WORD Port,Z80_BYTE *pDeviceData)
{
	*pDeviceData = MirageImagerRamData[Port & 0x0ff];
	return TRUE;
}


void MirageImagerWrite(Z80_WORD Port, Z80_BYTE Data)
{
	MirageImagerRamData[Port&0x0ff] = Data;
}


static CPCPortRead MirageImagerPortRead=
{
    0x0ffff, /* and */
    0x0fbf0, /* cmp */
    MirageImagerRead
};

static CPCPortWrite MirageImagerPortWrite=
{
    0x0ffff, /* and */
    0x0fbf0, /* cmp */
    MirageImagerWrite
};

static EmuDevice MirageImagerDevice=
{
	NULL,
	NULL,
	NULL,
	"MirageImager",
	"MirageImager",
	"Mirage Imager",
	CONNECTION_EXPANSION,   /* connects to expansion */
	0,
  1,
  &MirageImagerPortRead,
  1,
 &MirageImagerPortWrite,
 0,                /* no memory read*/
 NULL,
 0,                /* no memory write */
 NULL,
 MirageImager_Reset,
  MirageImager_MemoryRethink,
  MirageImager_Reset,
	0,                      /* no switches */
	NULL,
    2,                      /* 2 buttons */
    MirageImagerStopButton,
    1,                      /* 1 onboard roms */
    &MirageImagerRom,
    NULL,                   /* no cursor function */
    NULL,                   /* no generic roms */
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

void MirageImager_Init(void)
{
	RegisterDevice(&MirageImagerDevice);
}










