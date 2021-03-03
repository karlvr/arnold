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
#include "magnum.h"
#include "cpc.h"
#include "emudevice.h"
#include "crtc.h"
#include "mouse.h"

/**** MAGNUM LIGHT PHASER ****/
/*Trigger Button  ---> Request via output to Port FBFEh, then check CRTC input
 Light Sensor    ---> CRTC Light Pen Input
 */
BOOL Magnum_LightSensor = FALSE;

void	Magnum_DoOut(Z80_WORD Port, Z80_BYTE Data)
{
	/* TODO: Correct? */
	int Buttons = Mouse_GetButtons();

	/* left */
	if (Buttons&1)
	{
		/* d7 is trigger? */
		CRTC_SetLPenInput(((Data&0x080)!=0));
        }

	CRTC_SetLPenInput(Magnum_LightSensor);

}

/* VCC, GND, LPEN and D7 connected */
/* no address used; responds on all addresses */
CPCPortWrite MagnumPortWrite=
{
    0x0,        /* and */
    0x0,        /* compare */
    Magnum_DoOut
};

void Magnum_UpdateLightSensor(BOOL bState)
{
	Magnum_LightSensor = bState;
}

static EmuDevice MagnumLightPhaserDevice =
{
	NULL,
	NULL,
	NULL,
	"MAGNUMLIGHTPHASER",
	"MagnumLightPhaser",
	"Magnum Light Phaser",
	CONNECTION_EXPANSION,
	0,
  0,
  NULL,
  1,
  &MagnumPortWrite,
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
	NULL,					/* joystick */
	0,
	NULL,					/* memory ranges */
	NULL,					/* sound */
	Magnum_UpdateLightSensor,
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	NULL, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
	NULL,
};

void MagnumLightPhaserDevice_Init(void)
{
    RegisterDevice(&MagnumLightPhaserDevice);
}
