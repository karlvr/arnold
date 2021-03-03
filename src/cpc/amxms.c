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
/* AMX Mouse emulation */
#include "amxms.h"
#include "cpc.h"
#include "emudevice.h"
#include "mouse.h"
#include "joystick.h"

/* confirmed: doesn't clash with keyboard; pressing a key and using mouse buttons 
or directions doesn't cause a problem */

/* to read amx correctly you need to deselect the joystick OR change direction
for it to reset? 
*/
/* input goes low for what appears to be one mouse mickey; appears to be half a
scanline?? */
/* when going slow, there is a thin line of it down, then a lot of not down */
/* when going fast, there are loads of thin lines with almost no gap */


/* joystick device needs to pass in state of common etc */
unsigned char AmxMouse_ReadJoystickPort(int Index)
{
	unsigned char Data = 0x0ff;

	/* confirmed: AMX doesn't respond on 1, no input is returned */
	if (Index != 0)
	{
		return 0x0ff;
	}
	
	// TODO!
	signed int X = 0;	// Mouse_GetXMovement();
	signed int Y = 0;	// Mouse_GetYMovement();
	int Buttons = Mouse_GetButtons();

	/* left */
	if (Buttons&1)
	{
		/* confirmed: bit 5 */
		Data &=~(1<<5);
	}
	/* right */
	if (Buttons&2)
	{
		/* confirmed: bit 4 */
		Data &=~(1<<4);
	}

	/* middle */
	if (Buttons&4)
	{
		/* confirmed: bit 6 */
		Data &=~(1<<6);
	}

	if (X<0)
	{
		Data &= ~(1<<2);
	}
	else if (X>0)
	{
		Data &= ~(1<<3);
	}


	if (Y<0)
	{
		Data &= ~(1<<0);
	}
	else if (Y>0)
	{
		Data &= ~(1<<1);
	}
	return Data;
}

EmuDevice m_AmxMouseDevice =
{
	NULL,
	NULL,
	NULL,
    "AMS",
	"AmxMouse",
    "AMX Mouse",
	CONNECTION_JOYSTICK,   /* connected to expansion */
	DEVICE_FLAGS_TESTED,
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
	AmxMouse_ReadJoystickPort,	/* joystick */
	0,
	NULL,
	NULL,	/* no sound */
	NULL,	/* lpen */
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	NULL, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
	NULL,
};

void AmxMouse_Init(void)
{
    RegisterDevice(&m_AmxMouseDevice);
}

/* dktronics mouse: 

no input on joystick 1.
only fire 1 reports anything.
directions on joystick inputs. 

also requires joystick deselect ??  */
