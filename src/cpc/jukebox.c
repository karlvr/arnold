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
/* Arnold JukeBox */
/* TODO: Testing! */
/* fbe1, fbe2, fbe3, fbe0 */

/*
fbe0 is used to read settings from board:

dip switches define bits 7..4.

off = 1, on = 0

then lower 4 bits are defined by a dial (0-15).

bit 7 is unused (appears to be locked to 0 on the pcb). it is in the on position?

bit 6,5 is timer value (software). it is the time between choosing next cart on the main screen.
(timer enabled must also be set)

off off = 8			1 1
off on = 16			1 0
on off = 24			0 1
on on = 32 			0 0

bit 4 = timer on/off

bit 3 ignored

bit 2..0: timer value when game manually selected (0-15)

0 = forever?
1 = 8s,
7 = 56s
(
*/

/* fbe2 is read */
/* bits 7..3 are current cartridge value, lower 4 bits are not used */

/* fbe1

bits 7,6: ?
bits 5..0 are cartridge number
*/

/* fbe3 is write */
/* bits 7..4 are cartridge */
/* bits 0..3: 3 bit value, or 2 bit value */

/* an array of cartridges */
/* first element is actually the jukebox cartridge itself */
#include "cpc.h"
#include "jukebox.h"
#include "asic.h"

static int Jukebox_CurrentCartridge = 0;

static int Jukebox_SelectedCartridge = 0;

static BOOL Jukebox_TimerActive = FALSE;
static int Jukebox_CurrentTimerValue = 0;

static BOOL Jukebox_EnableState = FALSE;

/* no timer */
/* time is set to longest */
/* manual time of max is set */
/* auto time of max is set */
static unsigned char Jukebox_SettingsInput = ( 1 << 4 );

void Jukebox_SetTimerSelection( int nValue )
{
	Jukebox_SettingsInput &= ~( 0x03 << 5 );
	Jukebox_SettingsInput |= ( ( nValue ^ 0xff ) & 0x03 ) << 5;
}

int Jukebox_GetTimerSelection()
{
	return ( ( ( Jukebox_SettingsInput >> 5 ) ^ 0x0ff ) & 0x03 );

}

void Jukebox_SetManualTimerSelection( int nValue )
{
	Jukebox_SettingsInput &= ~0x0f;
	Jukebox_SettingsInput |= ( ( nValue ^ 0x0f ) & 0x0f );
}


int Jukebox_GetManualTimerSelection()
{
	return ( Jukebox_SettingsInput ^ 0x0f ) & 0x0f;
}

void Jukebox_SetTimerEnabled( BOOL bEnabled )
{
	if ( bEnabled )
	{
		Jukebox_SettingsInput &= ~( 1 << 4 );
	}
	else
	{
		Jukebox_SettingsInput |= ( 1 << 4 );
	}
}

BOOL Jukebox_IsTimerEnabled()
{
	return ( ( Jukebox_SettingsInput & ( 1 << 4 ) ) == 0 );

}

int Jukebox_GetSettingsInput()
{
	return Jukebox_SettingsInput;
}

typedef struct
{
	unsigned char *pFilename;
	unsigned char *pData;
	unsigned long nLength;
	BOOL bEnabled;
} Jukebox_Cartridge;

/* 12 cartridges */
static Jukebox_Cartridge Cartridges[ 14 ];

static CPCPortRead JukeboxRead =
{
	0x0ffe0,
	0x0fbe0,
	Jukebox_Read
};


static CPCPortWrite JukeboxWrite =
{
	0x0ffe0,
	0x0fbe0,
	Jukebox_Write
};


void Jukebox_Update( void )
{
	if ( Jukebox_TimerActive )
	{
		Jukebox_CurrentTimerValue--;
		if ( Jukebox_CurrentTimerValue == 0 )
		{
			Jukebox_TimerActive = FALSE;

			Computer_RestartReset();
		}
	}
}


BOOL Jukebox_IsEnabled()
{
	return Jukebox_EnableState;
}


void Jukebox_Power( void )
{
	Jukebox_SelectedCartridge = 0;

	Jukebox_SelectCartridge( 0 );
	Jukebox_TimerActive = FALSE;
}

void Jukebox_Reset( void )
{
	Jukebox_SelectCartridge( 0 );
	Jukebox_TimerActive = FALSE;

}

void Jukebox_SetCartridgeEnable( int nCartridge, BOOL bEnable )
{
	if ( nCartridge <= 12 )
	{
		Cartridges[ nCartridge ].bEnabled = bEnable;
	}
}

BOOL Jukebox_IsCartridgeEnabled( int nCartridge )
{
	return Cartridges[ nCartridge ].bEnabled;
}

/* up to 12 cartridges */
void Jukebox_Init()
{
	int i;

	for ( i = 0; i < 14; i++ )
	{
		Cartridges[ i ].pData = NULL;
		Cartridges[ i ].nLength = 0;
	}

	Cartridges[ 0 ].bEnabled = TRUE;
	Jukebox_EnableState = FALSE;

}

void Jukebox_CartridgeInsert( int nCartridge, const unsigned char *pData, const unsigned long nLength )
{
	Cartridges[ nCartridge ].pData = (unsigned char *)malloc( nLength );
	if ( Cartridges[ nCartridge ].pData != NULL )
	{
		memcpy( Cartridges[ nCartridge ].pData, pData, nLength );
	}
	Cartridges[ nCartridge ].nLength = nLength;
}

void Jukebox_CartridgeRemove( int nCartridge )
{
	if ( Cartridges[ nCartridge ].pData != NULL )
	{
		free( Cartridges[ nCartridge ].pData );
		Cartridges[ nCartridge ].pData = NULL;
	}
	Cartridges[ nCartridge ].nLength = 0;

	Computer_RethinkMemory();
}

void Jukebox_InsertSystemCartridge( const unsigned char *pCartridgeData, const unsigned long CartridgeLength )
{
	Jukebox_CartridgeInsert( 0, pCartridgeData, CartridgeLength );

}

void Jukebox_Finish()
{
	int i;

	for ( i = 0; i <= 12; i++ )
	{

		if ( Cartridges[ i ].pData != NULL )
		{
			free( Cartridges[ i ].pData );
			Cartridges[ i ].pData = NULL;
		}
		Cartridges[ i ].nLength = 0;
	}
}

void Jukebox_RethinkMemory( MemoryData *pData )
{
	pData; /* not used */

	if (
		/* cartridge index is valid? */
		( Jukebox_CurrentCartridge > 12 ) ||
		/* cartridge data loaded? */
		( Cartridges[ Jukebox_CurrentCartridge ].pData == NULL ) ||
		/* cartridge enabled in UI? */
		( !Cartridges[ Jukebox_CurrentCartridge ].bEnabled )
		)
	{
		/* if no cartridge exists here then map empty memory */
		Cartridge_RemoveI();
	}
	else
	{
		/* insert the cartridge */

		/* CPR? */
		if ( Cartridge_ValidateCartridge( Cartridges[ Jukebox_CurrentCartridge ].pData, Cartridges[ Jukebox_CurrentCartridge ].nLength ) )
		{
			/* insert it */
			Cartridge_Insert( Cartridges[ Jukebox_CurrentCartridge ].pData, Cartridges[ Jukebox_CurrentCartridge ].nLength );
		}
		else
		{
			/* insert as binary */
			Cartridge_InsertBinary( Cartridges[ Jukebox_CurrentCartridge ].pData, Cartridges[ Jukebox_CurrentCartridge ].nLength );
		}
	}

}

void Jukebox_SelectCartridge( int nCartridge )
{

	/* 0x0c0 what does that mean? */

	Jukebox_CurrentCartridge = nCartridge & 0x01f;

	Computer_RethinkMemory();

}

void Jukebox_Write( Z80_WORD nAddr, Z80_BYTE nValue )
{
	switch ( nAddr & 0x03 )
	{
		/* don't know if 0 or 2 can be written; CSD system cartridge doesn't read them */

		default
			:
	case 0:
	case 2:
		break;

	case 3:
	{
		/* store selected cartridge */
		Jukebox_SelectedCartridge = ( nValue >> 4 ) & 0x0f;
		/* set timer value */
		Jukebox_CurrentTimerValue = ( nValue & 0x0f ) * 50 * 8;
	}
	break;

	case 1:
	{
		/* assume 0x0c0 means start timer, not sure which bit */
		if ( ( nValue & 0x0c0 ) != 0 )
		{
			Jukebox_TimerActive = TRUE;
		}
		else
		{
			Jukebox_TimerActive = FALSE;
		}

		Jukebox_CurrentCartridge = nValue & 0x01f;
		Jukebox_SelectCartridge( Jukebox_CurrentCartridge );
	}
	break;
	}
}


BOOL Jukebox_Read( Z80_WORD nAddr, Z80_BYTE *pDeviceData )
{
	switch ( nAddr & 0x03 )
	{
	case 0:
		*pDeviceData = Jukebox_SettingsInput;
		return TRUE;

	case 2:
		*pDeviceData = ( Jukebox_SelectedCartridge << 4 );
		return TRUE;

	default:
		break;
	}
	return FALSE;
}



void Jukebox_Enable( BOOL bEnable )
{
	Jukebox_EnableState = bEnable;

	if ( bEnable )
	{
		CPC_InstallResetFunction( Jukebox_Reset );
		CPC_InstallPowerFunction( Jukebox_Power );
		CPC_InstallReadPort( &JukeboxRead );
		CPC_InstallWritePort( &JukeboxWrite );
		CPC_InstallUpdateFunction( Jukebox_Update );
		CPC_InstallMemoryRethinkHandler( Jukebox_RethinkMemory );
	}
	else
	{
		CPC_UnInstallResetFunction( Jukebox_Reset );
		CPC_UnInstallPowerFunction( Jukebox_Power );
		CPC_UninstallReadPort( &JukeboxRead );
		CPC_UninstallWritePort( &JukeboxWrite );
		CPC_UnInstallUpdateFunction( Jukebox_Update );
		CPC_UnInstallMemoryRethinkHandler( Jukebox_RethinkMemory );
	}
}
