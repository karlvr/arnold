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
/* Hackit - good */

#include "cpc.h"
#include "emudevice.h"
#include "riff.h"

static int hackit_ROMLatch = 0;
static BOOL Hackit_SwitchEnable = FALSE;
unsigned char Hackit_SystemROM[ 16384 ];

void hackit_SetROM( const unsigned char *pROM, unsigned long RomLength )
{
	EmuDevice_CopyRomData( Hackit_SystemROM, sizeof( Hackit_SystemROM ), pROM, RomLength );
}

void hackit_ClearROM(void)
{
	EmuDevice_ClearRomData(Hackit_SystemROM, sizeof(Hackit_SystemROM));
}


static EmuDeviceRom HackitRom[1] =
{
	{
		"HackIt System ROM",
		"SystemRom",
		hackit_SetROM,
		hackit_ClearROM,
		16384,
		0   /* ROM CRC - todo */
	}
};

void    hackit_SetSwitch( BOOL bState )
{
	Hackit_SwitchEnable = bState;

	Computer_RethinkMemory();
}

BOOL    hackit_IsSwitchEnabled( void )
{
	return Hackit_SwitchEnable;
}

void	hackit_ROMSelect( Z80_WORD Port, Z80_BYTE Data )
{
	/* d0,d1,d2 goes into A,B,C of 74LS138 */
	/* d3 goes into G2A which is active low */
	hackit_ROMLatch = Data & 0x0f;

	Computer_RethinkMemory();

}

CPCPortWrite hackitPortWrites[ 1 ] =
{
	{
		0x02000,            /* and */
		0x00000,            /* compare */
		hackit_ROMSelect
	}
};

void hackit_MemoryRethinkHandler( MemoryData *pData )
{
	/* uses ROMEN */
	if ( pData->bRomEnable[ 6 ] && Hackit_SwitchEnable && (hackit_ROMLatch==0x0) )
	{
		/* disable other roms in the chain */
		pData->bRomDisable[ 6 ] = TRUE;
		pData->bRomDisable[ 7 ] = TRUE;

		/* Hackit ROM */
		unsigned char *pRomPtr = Hackit_SystemROM - 0x0c000;
		pData->pReadPtr[ 7 ] = pRomPtr;
		pData->pReadPtr[ 6 ] = pRomPtr;
	}
}

/* two switches, one of which is a busreset */
static EmuDeviceSwitch hackitSwitches[ 1 ] =
{
	{
		"Activate Switch",          /* the switch on the top */
		"Activate",
		hackit_IsSwitchEnabled,
		hackit_SetSwitch
	},
};


static EmuDevice hackitDevice =
{
	NULL,
	NULL,
	NULL,
	"hackit",
	"Hackit",
	"HackIt/Le Hacker",
	CONNECTION_EXPANSION,   /* connected to expansion */
	DEVICE_FLAGS_HAS_EXPANSION_ROMS | DEVICE_FLAGS_HAS_PASSTHROUGH| DEVICE_WORKING,
	0,                /* no read ports */
	NULL,
	sizeof(hackitPortWrites)/sizeof(hackitPortWrites[0]),                    /* 2 write ports */
	hackitPortWrites, /* the ports */
	0,
	NULL,
	0,
	NULL,
	NULL, /* reset function */
	hackit_MemoryRethinkHandler, /* memory rethink */
	NULL, /* power function */
	sizeof( hackitSwitches ) / sizeof( hackitSwitches[ 0 ] ),                      /* 3 switch */
	hackitSwitches,
	0,                      /* has 1 button which is reset; not currently emulated */
	NULL,
	sizeof(HackitRom)/sizeof(HackitRom[0]),                      /* no onboard roms */
	HackitRom,
	NULL,	/* no cursor update function */
	NULL,
	NULL,	/* printer */
	NULL,	/* joystick */
	0,
	NULL,
	NULL,	/* no sound */
	NULL, /* lpen */
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	NULL, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
	NULL,
};

void hackit_Init()
{
	RegisterDevice( &hackitDevice );
}









