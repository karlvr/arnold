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
#include "gunstick.h"
#include "cpc.h"
#include "emudevice.h"
#include "mouse.h"
#include "joystick.h"

/* Connects to 9pin joystick port, requires four 1.5V batteries.
 Trigger Button  ---> Joystick Fire 2 (Row9, Bit4)
 Light Sensor    ---> Joystick Down   (Row9, Bit1)
 */
BOOL GunStick_LightSensor = FALSE;

unsigned char Gunstick_ReadJoystickPort(int nIndex)
{
	unsigned char Data = 0x0ff;

	if (nIndex == 0)
	{
		return 0x0ff;
	}

	/* left mouse button is trigger */
	if (Mouse_GetButtons() & 0x01)
	{
		Data &= ~(1 << 4);
	}
	if (GunStick_LightSensor)
	{
		Data &= ~(1 << 1);
	}

	return Data;

}

void GunStick_LightSensorUpdate(BOOL bState)
{
	GunStick_LightSensor = bState;
}


EmuDevice m_GunstickDevice =
{
	NULL,
	NULL,
	NULL,
	"GSTK",
	"Gunstick",
	"Gunstick",
	CONNECTION_JOYSTICK,   /* connected to expansion */
	0,
	0,                /* no read ports */
	NULL,
	0,                    /* no write ports */
	NULL, /* the ports */
	0,                /* no memory read*/
	NULL,
	0,                /* no memory write */
	NULL,
	NULL, /* reset function */
	NULL, /* memory rethink */
	NULL, /* power function */
	0,                      /* no switchs */
	NULL,
	0,                      /* no button */
	NULL,
	0,                      /* no onboard roms */
	NULL,
	NULL,	/* no cursor update function */
	NULL,	/* expansion roms */
	NULL,	/* printer */
	Gunstick_ReadJoystickPort,	/* joystick */
	0,
	NULL,				/* memory range */
	NULL,				/* sound */
	GunStick_LightSensorUpdate,
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	NULL, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
	NULL,

};

void Gunstick_Init()
{
	RegisterDevice(&m_GunstickDevice);
}
