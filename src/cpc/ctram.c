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

static unsigned char *CTRAM_Ram = NULL;
static int CTRAM_Config = 0;
	
void CTRAM_Reset(void)
{
	/* confirmed, reset clears the 74LS273 which holds the config */
	CTRAM_Config = 0;
}

void CTRAM_MemoryRethink(MemoryData *pData)
{
	/*

	cas0 for bank 0, cas1 for bank 2

	D7D6 D0   D3   D4   D1   D2   NCAS A15  A14  GND  ;pin 1..10
	CPU  A15S AMUX MUX  LCLK CAS1 CAS0 IOWR A14S VCC  ;pin 11..20

	IF (VCC) /LCLK= D7D6 * /A15 * /IOWR               ;load external latch on OUT [7Fxxh],C0h..FFh

	IF (VCC) /CAS0= /NCAS * /D4  +                    ;bank bit4=0, select bank 0..15 (CPU and CRTC)
	/CAS0= /NCAS *  A15 +  (8000-ffff)
	/CAS0= /NCAS * /A14 +  (0000-3fff)
	/CAS0= /NCAS *  CPU

	d4,d3,d2,d1,d0

	a15 a14 d4
	0   0   0    0000-3fff
	0   0   1
	0   1   0    4000-7fff
	0   1   1
	1   0   0	8000-bfff
	1   0   1
	1   1   0   c000-ffff
	1   1   1



	IF (VCC) /CAS1= /NCAS * D4 * /A15 * A14 * /CPU    ;bank bit4=1, select bank 16..31 (CPU at 4000h..7FFFh only)

	4000-7fff, 1

	IF (VCC) /A14S= /A14             +                             ;bank bit0
	/D0  * D2 * /A15 +
	/D0  * D3 * /A15 +
	/D0  * D4 * /A15

	- 0						4000-7fff
	0 -    - - | 1 - 0		0000-3fff  (4, 0,2,6)
	0 -    - 1 | - - 0
	0 -    1 - | - - 0

	IF (VCC) /A15S= /A14 *                    /A15 +               ;bank bit1
	/D1  *                    /A15 +
	/D4  * /D3  * /D2 * /D0 * /A15 +
	/D4  * /D3  * /D2 * /D1 * /A15

	0 0						0000-3fff
	0 -    - - | - 0 -		4000-7fff		1,4,5
	0 -    0 0 | 0 - 0		4000-7fff		2
	0 -    0 0 | 0 0 -						1

	IF (VCC) /AMUX= /D0  *  D1  * /D2 * /D3 * /D4 * /CPU * /MUX +
	A15 *  A14 * /D2 *  D0 *       /CPU * /MUX +
	A15 *  A14 * /D2 *  D1 *       /CPU * /MUX +
	A15 *  A14 *        D3 *       /CPU * /MUX +
	A15 *  A14 *        D4 *       /CPU * /MUX +
	/A15 *  A14 *        D2 *       /CPU * /MUX +  ;bank bit2
	/A15 *  A14 *        D3 *       /CPU *  MUX    ;bank bit3

	- -    0 0 | 0 1 0		2
	1 1    - - | 0 - 1		1,3
	1 1    - 1 | - - -		0, 1,2,3,4,5,6,7
	1 1    1 - | - - -		0,1,2,3,4,5,6,7

	0 1    - - | 1 - -		4,6,7
	0 1    - 1 | - - -		0,1,2,3,4,5,6,7


	IF (GND) /MUX = /MUX      ;dummy (do not output anything on this pin)

	IF (GND) /IOWR=/IOWR      ;dummy (do not output anything on this pin)

	*/
#if 0
	switch (CTRAM_Config & 0x1f)
	{
		/* nothing */
	case 0:
		break;

	case 2:
	{
		int i;
		/* complete switch */
		for (i = 0; i < 4; i++)
		{
			int nPage = i;
			int nRamOffset = (i << 14);
			int nOffset = nRamOffset - (nPage << 14);
			pData->pWritePtr[(i << 1) + 0] = (CTRAM_Ram + (pPALData->Bank << 16)) - nOffset;
			pData->pWritePtr[(i << 1) + 1] = pData->pWritePtr[(i << 1) + 0];
			pData->pReadPtr[(i << 1) + 0] = pData->pWritePtr[(i << 1) + 0];
			pData->pReadPtr[(i << 1) + 1] = pData->pWritePtr[(i << 1) + 1];
			pData->bRamDisable[(i << 1) + 0] = TRUE;
			pData->bRamDisable[(i << 1) + 1] = TRUE;
		}
	}
	break;

	case 1:
	{
		pData->pWritePtr[6] = (pPALData->pRamPtr + (pPALData->Bank << 16)) + (3 << 14) - 0x0c000;
		pData->pWritePtr[7] = pData->pWritePtr[6];
		pData->pReadPtr[6] = pData->pWritePtr[6];
		pData->pReadPtr[7] = pData->pWritePtr[7];
		pData->bRamDisable[6] = TRUE;
		pData->bRamDisable[7] = TRUE;
	}
	break;

	case 3:
	{
		/* pal can do this because it swaps a14/a15 to ram */

		/* page 7 appears at position 3 */
		pData->pWritePtr[7] = (pPALData->pRamPtr + (pPALData->Bank << 16)) + (((7) - 4) << 14) - 0x0c000;
		pData->pWritePtr[6] = pData->pWritePtr[7];
		pData->pReadPtr[6] = pData->pWritePtr[6];
		pData->pReadPtr[7] = pData->pWritePtr[7];
		pData->bRamDisable[7] = TRUE;
		pData->bRamDisable[6] = TRUE;

		/* normal page 3 ram appears in range &4000-&7fff */
		pData->pWritePtr[2] = Z80MemoryBase + (((7) - 4) << 14) - 0x04000;
		pData->pWritePtr[3] = pData->pWritePtr[2];

		pData->pReadPtr[2] = pData->pWritePtr[2];
		pData->pReadPtr[3] = pData->pWritePtr[3];
		pData->bRamDisable[2] = TRUE;
		pData->bRamDisable[3] = TRUE;
	}
	break;

	case 4:
	case 5:
	case 6:
	case 7:
	{
		/* 4000-7fff only */
		pData->pWritePtr[2] = (pCTRAM_Ram + (pPALData->Bank << 16)) + (((pPALData->RamConfig & 0x07) - 4) << 14) - 0x04000;
		pData->pWritePtr[3] = pData->pWritePtr[2];

		pData->pReadPtr[2] = pData->pWritePtr[2];
		pData->pReadPtr[3] = pData->pWritePtr[3];
		pData->bRamDisable[2] = TRUE;
		pData->bRamDisable[3] = TRUE;
	}
	break;
	}
#endif
}

static void CTRAM_PortWrite(Z80_WORD Port, Z80_BYTE Data)
{
	/* Confirmed bit 7 and 6 must be 1 */
	if ((Data & 0x0c0) == 0x0c0)
	{
		CTRAM_Config = Data;

		Computer_RethinkMemory();
	}
}




CPCPortWrite CTRAMPortWrite[1]=
{
	{
		0x08000, /* confirmed A15=0 */
		0x00000,
		CTRAM_PortWrite
	}
};

void CTRAMDevice_Init(void)
{
	CTRAM_Ram = (unsigned char *)malloc(512 * 1024);
	memset(CTRAM_Ram, 0x0ff, 512 * 1024);
}

void CTRAMDevice_Shutdown(void)
{
	if (CTRAM_Ram)
	{

		free(CTRAM_Ram);
	}
}

static EmuDevice CTRAMDevice =
{
	NULL,
	CTRAMDevice_Init,
	CTRAMDevice_Shutdown,
	"CTRAM",
	"CTRAM",
	"CTRAM (CPC6128 extra 64KB RAM)",
	CONNECTION_INTERNAL,   /* connects to expansion */
	0,
	0,						/* confirmed no read ports */
	NULL,					/* no read ports */
	1,
	CTRAMPortWrite,			/* 1 write port */
	0,                /* no memory read*/
	NULL,
	0,                /* no memory write */
	NULL,
	CTRAM_Reset,
	CTRAM_MemoryRethink,
	CTRAM_Reset,
	0,                      /* no switches */
	NULL,
	0,                      /* 2 buttons */
	NULL,
	0,                      /* onboard roms */
	NULL,
	NULL,                   /* no cursor function */
	NULL,                   /* no generic roms */
	NULL,					/* printer */
	NULL,					/* joystick */
	0,
	NULL,					/* memory ranges */
	NULL,	/* no sound */
	NULL,	/* lpen */
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	NULL, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
	NULL
};

int CTRAM_Init(void)
{

	return RegisterDevice(&CTRAMDevice);
}
