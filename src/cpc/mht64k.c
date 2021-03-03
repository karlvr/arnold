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
#include "memrange.h"
#include "riff.h"

static unsigned char MHT64ROMData[8192];
static unsigned char *MHT64Ram = NULL;

void MHT64_Reset(void)
{
}

void MHT64_RethinkMemory(MemoryData *pData)
{
}

static void MHT64_PortWrite(Z80_WORD Port, Z80_BYTE Data)
{

}




CPCPortWrite MHT64Write[1] =
{
	{
		0x0ffff,
		0x0f8f8,
		MHT64_PortWrite
	}
};


void MHT64_SetROM(const unsigned char *pROM, const unsigned long RomLength)
{
	EmuDevice_CopyRomData(MHT64ROMData, 8192, pROM, RomLength);
}

void MHT64_ClearROM(void)
{
	EmuDevice_ClearRomData(MHT64ROMData, 8192);
}

static EmuDeviceRom MHT64ROM[1] =
{
	{
		"MHT64 System ROM",
		"SystemRom",
		MHT64_SetROM,
		MHT64_ClearROM,
		8192,
		0   /* ROM CRC - todo */
	},
};

void MHT64_Init(void)
{
	MHT64Ram = (unsigned char *)malloc(65536);
	memset(MHT64Ram, 0x0ff, 65536);
}

void MHT64_Shutdown(void)
{
	if (MHT64Ram)
	{
		free(MHT64Ram);
	}
}

static EmuDevice MHT64KDevice =
{
	NULL,
	MHT64_Init,
	MHT64_Shutdown,
	"MHT64KB",
	"Mht64KB",
	"MHT64KB",
	CONNECTION_EXPANSION,   /* connects to expansion */
	0,
	0,
	NULL,					/* no read ports */
	1,
	MHT64Write,			/* 1 write port */
	0,                /* no memory read*/
	NULL,
	0,                /* no memory write */
	NULL, 
	MHT64_Reset,
	MHT64_RethinkMemory,
	MHT64_Reset,
	0,                      /* no switches */
	NULL,
	0,                      /* 2 buttons */
	NULL,
	1,                      /* 2 onboard roms */
	MHT64ROM,
	NULL,                   /* no cursor function */
	NULL,                   /* no generic roms */
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
	NULL,
};

void MHT64K_Init(void)
{

	RegisterDevice(&MHT64KDevice);
}
