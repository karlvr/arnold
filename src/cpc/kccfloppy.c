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
/* Thankyou to http://buebchen.jimdo.com/ */
/* http://buebchen.jimdo.com/selbst-gebaut-diy-homebrew-feito-por-mim/cpc6128-kc-compakt/ */
#include "cpc.h"
#include "emudevice.h"
#include "memrange.h"
#include "riff.h"

/* decoding */
/* 
A7 = DB7
A6 = DB6
A5 = IORQ | WR (=0)
A4 = AB15 ==1 for i/o
A3 = AB14 ==1 for i/o
A2-A0 = output driven from D0-D2 */

/* 00, 40,80,c0 = i/o access */
/* 20, 60,a0,e0 = ram access */

/* output is a value */
/* bit 2 = ramdis */
/* bit 3 = store value */
/* bit 1,0 = bank */

static unsigned char ChosenConfig;
static unsigned char *KCCRam;
static unsigned char FloppyROM[16384];
static unsigned char ConfigROM[256];

void KCCRam_Reset(void)
{
	ChosenConfig = 0;
}

void KCCRam_RethinkMemory(MemoryData *pData)
{
	int i;
	unsigned char Pages[4];
	
	/*
ram access
a7 = x
a6 = x
a5 = 1
a4 = a15
a3 = a14
a2,a1,a0 = chosen config */

	
	/* 0x020 is not i/o */
	Pages[0] = ConfigROM[(ChosenConfig | 0x020)]; /* a4,a3 = 0 */
	Pages[1] = ConfigROM[(ChosenConfig | 0x020) + 8]; /* a4,a3 = 0,1 */
	Pages[2] = ConfigROM[(ChosenConfig | 0x020) + 16]; /* a4,a3 = 1,0 */
	Pages[3] = ConfigROM[(ChosenConfig | 0x020) + 24]; /* a4,a3 = 1,1 */

	for (i=0; i<4; i++)
	{
		//printf("Pages: %d %d\n", i, Pages[i]);
		if (Pages[i] & (1<<2)) /* bit 2 = ramdis */
		{
			int nPage = Pages[i]&0x03;
			int nRamOffset = (i << 14);
			int nOffset = nRamOffset - (nPage << 14);

			pData->pWritePtr[(i << 1) + 0] = KCCRam - nOffset;
			pData->pWritePtr[(i << 1) + 1] = pData->pWritePtr[(i << 1) + 0];
			pData->pReadPtr[(i << 1) + 0] = pData->pWritePtr[(i << 1) + 0];
			pData->pReadPtr[(i << 1) + 1] = pData->pWritePtr[(i << 1) + 1];

			pData->bRamDisable[(i << 1) + 0] = TRUE;
			pData->bRamDisable[(i << 1) + 1] = TRUE;
		}
	}
}

static void KCCRam_PortWrite(Z80_WORD Port, Z80_BYTE Data)
{
	
	if ((Data & 0x0c0) == 0x0c0)
	{
		
/* 7fxx -> 

a7 = 1
a6 = 1
a5 = 0
a4 = 0
a3 = 1
a2,a1,a0 = x - stored
*/
		/* calc address in ROM */
		/* ROM has a bit set to indicate if the value is latched or not
		and this also controls the port decoding */
		int Addr = 0x0c0 | ((Port & 0x0c000)>>11) | (Data & 0x07);
	//	printf("Addr: %d\n", Addr);
		unsigned char RomData = ConfigROM[Addr];
	//	printf("RomData: %d\n", RomData);
		/* bit set to indicate to store? */
		if (RomData & (1<<3))
		{
			ChosenConfig = Data & 0x07;
		//	printf("Config chosen: %d\n", ChosenConfig);
			Computer_RethinkMemory();
		}
		
		/* todo ram dis */
	}
}




CPCPortWrite KCCRamWrite[1]=
{
	/* bits 15,14 can be any value; ROM decides if value is stored or not */
	{
		0x00000,
		0x00000,
		KCCRam_PortWrite
	}
};


void KCCRam_SetROM(const unsigned char *pROM, const unsigned long RomLength)
{
	EmuDevice_CopyRomData(ConfigROM, 256, pROM, RomLength);
}

void KCCFloppy_SetROM(const unsigned char *pROM, const unsigned long RomLength)
{
	EmuDevice_CopyRomData(FloppyROM, 16384, pROM, RomLength);
}

void KCCRam_ClearROM(void)
{
	EmuDevice_ClearRomData(ConfigROM, 256);
}

void KCCFloppy_ClearROM(void)
{
	EmuDevice_ClearRomData(FloppyROM, 16384);
}

static EmuDeviceRom KCCFloppyROM[1]=
{
	{
	    "KCC Ram Configuration System ROM",
		"RamSystemRom",
	    KCCRam_SetROM,
		KCCRam_ClearROM,
	  256,
	  0   /* ROM CRC - todo */
	}
	// disabled until separate fdc is hooked up
#if 0
	{
	    "KCC DOS ROM",
	    KCCFloppy_SetROM,
		KCCFloppy_ClearROM,
	  16384,
	  0   /* ROM CRC - todo */
	},
#endif
};

void KCCFloppyDevice_Init(void)
{
	KCCRam = (unsigned char *)malloc(65536);
	memset(KCCRam, 0x0ff, 65536);
}

void KCCFloppyDevice_Shutdown(void)
{
	if (KCCRam)
	{
		free(KCCRam);
	}
}

static EmuDevice KCCFloppyDevice =
{
	NULL,
	KCCFloppyDevice_Init,
	KCCFloppyDevice_Shutdown,
	"KCC Ram",
	"KCCFloppyAndRam",
	"KCC Ram",
	CONNECTION_EXPANSION,   /* connects to expansion */
	0,
	0,
	NULL,					/* no read ports */
	1,
	KCCRamWrite,			/* 1 write port */
	0,                /* no memory read*/
	NULL,
	0,                /* no memory write */
	NULL,
	KCCRam_Reset,
	KCCRam_RethinkMemory,
	KCCRam_Reset,
	0,                      /* no switches */
	NULL,
	0,                      /* 2 buttons */
	NULL,
	sizeof(KCCFloppyROM)/sizeof(KCCFloppyROM[0]),                      /* 2 onboard roms */
	KCCFloppyROM,
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

void KCCFloppy_Init(void)
{

	RegisterDevice(&KCCFloppyDevice);
}
