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
#include "pal.h"
#include "emudevice.h"
#include "memrange.h"
#include "riff.h"

static unsigned char *InicronRam = NULL;
static int InicronRamConfig = 0;

static BOOL InicronActiveSwitch = TRUE;
static BOOL Inicron64KBSwitch = FALSE;
static BOOL BatterySwitch = FALSE;

BOOL  InicronRAM_GetBattery(void)
{
	return BatterySwitch;
}

void  InicronRAM_SetBattery(BOOL bState)
{
	BatterySwitch = bState;
}

BOOL InicronRAM_GetS2(void)
{
	return InicronActiveSwitch;
}

void InicronRAM_SetS2(BOOL bState)
{
	InicronActiveSwitch = bState;
}


BOOL InicronRAM_GetS1(void)
{
	return Inicron64KBSwitch;
}

void InicronRAM_SetS1(BOOL bState)
{
	Inicron64KBSwitch = bState;
}


void InicronRam_Reset(void)
{
	/* TODO: Confirm reset/power on */
	InicronRamConfig = 0;

}

void InicronRam_MemoryRethink(MemoryData *pData)
{
	/* TODO: Confirm RAMDIS is recognised */
	if (InicronActiveSwitch && (InicronRamConfig & (1<<2)))
	{
		switch (InicronRamConfig&0x03)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		{
			int Page = InicronRamConfig & 0x03;
			int Bank = (InicronRamConfig>>3)&0x07;

			/* inicron 64kb not active and page 0 selected */
			if (!Inicron64KBSwitch && (Bank==0))
				break;

			/* 4000-7fff only */
			pData->pWritePtr[2] = ((InicronRam + (Bank << 16)) + ((Page << 14)) - 0x04000);
			pData->pWritePtr[3] = pData->pWritePtr[2];

			pData->pReadPtr[2] = pData->pWritePtr[2];
			pData->pReadPtr[3] = pData->pWritePtr[3];
			/* disable internal ram */
			pData->bRamDisable[2] = TRUE;
			pData->bRamDisable[3] = TRUE;
		}
		break;
		default:
		break;
		}
	}
}

static void InicronRam_PortWrite(Z80_WORD Port, Z80_BYTE Data)
{
	/* TODO: Confirm bit 7,6 decoding */
    if ((Data & 0x0c0)==0x0c0)
	{
		InicronRamConfig = Data;

		Computer_RethinkMemory();
	}
}



static EmuDeviceSwitch InicronRamSwitches[3]=
{
  {
      "S1 - Override extra 64K RAM (CPC6128)",
	  "S1",
      InicronRAM_GetS1,
      InicronRAM_SetS1
  },
  {
      "S2 - Activate",
	  "S2",
      InicronRAM_GetS2,
      InicronRAM_SetS2
  },
   {
      "Battery jumper",
	  "BatteryJumper",
      InicronRAM_GetBattery,
      InicronRAM_SetBattery
  }
};

/* TODO: Confirm port decoding */
CPCPortWrite InicronRamPortWrite[1]=
{
	{
		0x0ff00,
		0x07f00,
		InicronRam_PortWrite
	}
};


void InicronRAMDevice_Init(void)
{
	InicronRam = (unsigned char *)malloc(512 * 1024);
	memset(InicronRam, 0x0ff, 512 * 1024);

}

void InicronRAMDevice_Shutdown(void)
{
	if (InicronRam)
	{
		free(InicronRam);
	}
}

static EmuDevice InicronRamDevice =
{
	NULL,
	InicronRAMDevice_Init,
	InicronRAMDevice_Shutdown,
	"INICRONRAM",
	"InicronRam",
	"InicronRam (External 512KB RAM)",
	CONNECTION_EXPANSION,   /* connects to expansion */
	DEVICE_WORKING,	/* has ram but not fully dk'tronics compatible */
	0,
	NULL,					/* no read ports */
	sizeof(InicronRamPortWrite) / sizeof(InicronRamPortWrite[0]),
	InicronRamPortWrite,			/* 1 write port */
	0,                /* no memory read*/
	NULL,
	0,                /* no memory write */
	NULL,
	InicronRam_Reset,
	InicronRam_MemoryRethink,
	InicronRam_Reset,
	sizeof(InicronRamSwitches)/sizeof(InicronRamSwitches[0]),                      /* no switches */
	InicronRamSwitches,
	0,                      /* 2 buttons */
	NULL,
	0,                      /* 1 onboard roms */
	NULL,
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
	NULL,
};

int InicronRAM_Init()
{
	return RegisterDevice(&InicronRamDevice);
}

