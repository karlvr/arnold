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
/* Multiplay emulation */

#include "cpc.h"
#include "emudevice.h"

#include "multiplay.h"

static BOOL MultiplayInputSwitches[2] = { TRUE, TRUE };
static Z80_BYTE MultiplayMouse[MULTIPLAY_NUM_MICE][2] = {{0x0,0x0},{0x0,0x0}};
static Z80_BYTE MultiplayJoystick[MULTIPLAY_NUM_JOYSTICKS] = { 0x0, 0x0 };

BOOL Multiplay_GetInputModeA(void)
{
	return MultiplayInputSwitches[0];
}

void Multiplay_SetInputModeA(BOOL bState)
{
	MultiplayInputSwitches[0] = bState;
}


BOOL Multiplay_GetInputModeB(void)
{
	return MultiplayInputSwitches[1];
}

void Multiplay_SetInputModeB(BOOL bState)
{
	MultiplayInputSwitches[1]= bState;
}



BOOL Multiplay_IsUsingJoystick(int Index)
{
	if ((Index < 0) || (Index >= MULTIPLAY_NUM_JOYSTICKS))
		return FALSE;

	return MultiplayInputSwitches[Index];
}

void Multiplay_SetInput(int Index, MULTIPLAY_INPUT Input, BOOL bState)
{
	if ((Index < 0) || (Index >= MULTIPLAY_NUM_JOYSTICKS))
		return;

	MultiplayJoystick[Index] &= ~(1 << (int)Input);
	if (bState)
	{
		MultiplayJoystick[Index] |= (1 << (int)Input);
	}
}

BOOL Multiplay_Read(Z80_WORD Port, Z80_BYTE *pDeviceData)
{
	switch (Port&0x07)
	{
		/* bit 7 reads as 0 */
		case 0:
			*pDeviceData =  MultiplayJoystick[0]&0x07f;
			return TRUE;
		
		/* bit 7 reads as 0 */
		case 1:
			*pDeviceData =  MultiplayJoystick[1]&0x07f;
			return TRUE;
		/* check: bits 7..4 read as? */
		case 2:
			*pDeviceData =  MultiplayMouse[0][0]&0x0f;
			return TRUE;
		case 3:
			*pDeviceData =  MultiplayMouse[0][1]&0x0f;
			return TRUE;
		case 4:
			*pDeviceData =  MultiplayMouse[1][0]&0x0f;
			return TRUE;
		case 5:
			*pDeviceData =  MultiplayMouse[1][1]&0x0f;
			return TRUE;
	}	
	return FALSE;
}


static EmuDeviceSwitch MultiplaySwitches[2] =
{
	{
		"Input Mode A - On=Joystick, Off=Mouse",
		"InputModeA",
		Multiplay_GetInputModeA,
		Multiplay_SetInputModeA
	},
	{
		"Input Mode B - On=Joystick, Off=Mouse",
		"InputModeB",
		Multiplay_GetInputModeB,
		Multiplay_SetInputModeB
	},
};


CPCPortRead MultiplayPortRead[1]=
{
	{
		/* 1111 100- 1001 0xxx */
		0xfef8,            /* and */
		0xf890,            /* compare */
		Multiplay_Read
	}
};

static EmuDevice MultiplayDevice =
{
	NULL,
	NULL,
	NULL,
	"MULTIPLAY",
	"Multiplay",
	"Multiplay",
	CONNECTION_EXPANSION,   /* connected to expansion */
	DEVICE_FLAGS_HAS_PASSTHROUGH|DEVICE_WORKING| DEVICE_FLAGS_FROM_SPECIFICATION,
 1,                /* 1 read ports */
  MultiplayPortRead,
  0,                    /* no write ports*/
  NULL, /* the ports */
  0,                /* no memory read*/
  NULL,
  0,                /* no memory write */
  NULL,
  NULL, /* reset function */
  NULL, /* memory rethink */
  NULL, /* power function */
	sizeof(MultiplaySwitches)/sizeof(MultiplaySwitches[0]),						/* no switches */
	MultiplaySwitches,
    0,                      /* no buttons */
    NULL,
    0,                      /* no onboard roms */
    NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	0,
	NULL,				/* memory ranges */
	NULL, /* sound */
	NULL, /* lpen */
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	NULL, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
	NULL,
	
};

void Multiplay_Init()
{
	RegisterDevice(&MultiplayDevice);
}









