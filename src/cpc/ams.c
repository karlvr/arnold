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

#if 0
/* ams emulation */

#include "cpc.h"
#include "emudevice.h"
#include "riff.h"

/* IC6B = I/O write */
/* IC2B = int acknowledge? */
/* IC6A A2,A10 */



/* static ram */
static unsigned char amsRamData[8192];

void ams_MemoryRethink(MemoryData *pData)
{

}

void ams_Reset(void)
{
    /* reset enables the rom; doesn't enable the ram */
	amsPortData = (1 << 1);
	Computer_RethinkMemory();

}

/* call here when ams stop button is pressed */
void    ams_Stop(void)
{
	Computer_RethinkMemory();

}



void amsWrite(Z80_WORD Port, Z80_BYTE Data)
{
	/* bit 0 */
    Computer_RethinkMemory();
}

static CPCPortWrite amsPortWrite=
{
	0x0fffc, /* and */
	0x0fbf0, /* cmp */
	amsWrite
};

static EmuDevice amsDevice=
{
	NULL,
	NULL,
	NULL,
	"ams",
	"ams",
	CONNECTION_EXPANSION,   /* connects to expansion */
	0,
  0,
  NULL,
  1,
 &amsPortWrite,
 0,                /* no memory read*/
 NULL,
 0,                /* no memory write */
 NULL,
 NULL,
  ams_MemoryRethink,
  NULL,
	0,                      /* no switches */
	NULL,
    2,                      /* 2 buttons */
    amsStopButton,
    0,                      /* 1 onboard roms */
    NULL,
    NULL,                   /* no cursor function */
    NULL,                   /* no generic roms */
	NULL,
	NULL,
	0,
	NULL,
	NULL, /* sound */
	NULL, /* lpen */
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	NULL, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
};

void ams_Init()
{
	RegisterDevice(&amsDevice);
}
#endif









