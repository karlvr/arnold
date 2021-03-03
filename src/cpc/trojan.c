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
#include "crtc.h"
#include "mouse.h"
#include "joystick.h"

/**** TROJAN LIGHT PHASER for PLUS ****/

/* Light Sensor    ---> CRTC Light Pen Input
Trigger Button  ---> Joystick Fire 2 (Row9, Bit4) */
BOOL Trojan_LightSensor = FALSE;

unsigned char Trojan_ReadJoystickPort(int nIndex)
{
	unsigned char Data = 0x0ff;
	if (nIndex != 0)
	{
		return 0x0ff;
	}

	int Buttons = Mouse_GetButtons();

	/* left */
	if (Buttons & 1)
	{
		Data &= ~(1 << 4);
	}

	return Data;
}

void Trojan_LightSensorUpdate(BOOL bState)
{
	Trojan_LightSensor = bState;
	CRTC_SetLPenInput(Trojan_LightSensor);
}

static EmuDevice TrojanLightPhaserDevice =
{
	NULL,
	NULL,
	NULL,
	"TROJANLIGHTPHASER",
	"TrojanLightPhaser",
	"Trojan Light Phaser",
	CONNECTION_JOYSTICK, /* Aux actually */
	0,
	0,
	NULL,
	0,
	NULL,
	0,                /* no memory read*/
	NULL,
	0,                /* no memory write */
	NULL,
	NULL,
	NULL,
	NULL,
	0,                      /* no switches */
	NULL,
	0,                      /* no buttons */
	NULL,
	0,                      /* no onboard roms */
	NULL,
	NULL,                   /* no cursor update */
	NULL,                   /* no expansion roms */
	NULL,					/* printer */
	Trojan_ReadJoystickPort,					/* joystick */
	0,
	NULL,
	NULL,					/* sound */
	Trojan_LightSensorUpdate,
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	NULL, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
	NULL
};

void TrojanLightPhaserDevice_Init(void)
{

	RegisterDevice(&TrojanLightPhaserDevice);
}
