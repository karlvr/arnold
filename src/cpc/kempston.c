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
#include "kempston.h"
#include "cpc.h"
#include "emudevice.h"
#include "mouse.h"

static unsigned char CounterX = 0;
static unsigned char CounterY = 0;

/* confirmed: counters reset at power on but not reset */
void KempstonMouse_PowerOn(void)
{
	CounterX = 0;
	CounterY = 0;
}

BOOL KempstonMouse_ReadX(Z80_WORD Port, Z80_BYTE *pDeviceData)
{
	*pDeviceData =  CounterX;
	return TRUE;
}

BOOL KempstonMouse_ReadY(Z80_WORD Port, Z80_BYTE *pDeviceData)
{
	*pDeviceData = CounterY;
	return TRUE;
}

BOOL KempstonMouse_ReadButtons(Z80_WORD Port, Z80_BYTE *pDeviceData)
{
	unsigned char Data = 0x0ff;
	
	/* confirmed: */
	/* bit 1 = left button */
	/* bit 0 = right button */
	/* all other bits are 1's */
	/* button is active low */
	int Buttons = Mouse_GetButtons();

	/* left */
	if (Buttons&1)
	{
		Data &=~(1<<1);
	}
	/* right */
	if (Buttons&2)
	{
		Data &=~(1<<0);
	}
	*pDeviceData = Data;
	return TRUE;

}


CPCPortRead kempstonReadPorts[3]=
{
	/* confirmed: port decoding */
	{0x00511, 0x00100, KempstonMouse_ReadX}, /* xxxx x0x1 xxx0 xxx0 */
	{0x00511, 0x00101, KempstonMouse_ReadY}, /* xxxx x0x1 xxx0 xxx1 */
	{0x00510, 0x00000, KempstonMouse_ReadButtons}, /* xxxx x0x0 xxx0 xxxx */
};

static EmuDevice KempstonMouseDevice =
{
	NULL,
	NULL,
	NULL,
	"KEMPSTONMOUSE",
	"KempstonMouse",
	"Kempston Mouse",
	CONNECTION_EXPANSION,
	DEVICE_FLAGS_TESTED,	/* flags */
  3,
  kempstonReadPorts,
  0,
  NULL,
  0,                /* no memory read*/
  NULL,
  0,                /* no memory write */
  NULL,
  NULL,		/* no reset */
  NULL,
  KempstonMouse_PowerOn, /* power on */
	0,                      /* no switches */
	NULL,
    0,                      /* no buttons */
    NULL,
    0,                      /* no onboard roms */
    NULL,
    NULL,                    /* no cursor update */
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
	NULL
};

void KempstonMouse_Init(void)
{
    RegisterDevice(&KempstonMouseDevice);
}
