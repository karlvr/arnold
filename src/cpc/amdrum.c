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

void	Amdrum_Write(Z80_WORD Port, Z80_BYTE Data)
{
	/* write 8-bit sound data */
}

void Amdrum_SoundCallback(void)
{
}

CPCPortWrite AmdrumPortWrite =
{
	0x0ff00,            /* and */
	0x0ff00,            /* compare */
	Amdrum_Write
};

static EmuDevice AmdrumDevice =
{
	NULL,
	NULL,
	NULL,
	"AMDRUM",
	"Amdrum",
	"Amdrum",
	CONNECTION_EXPANSION,   /* connects to expansion */
	DEVICE_FLAGS_HAS_AUDIO,
	0,                /* no read ports */
	NULL,
	1,                    /* 1 write ports */
	&AmdrumPortWrite,
	0,                /* no memory read*/
	NULL,
	0,                /* no memory write */
	NULL,
	NULL, /* no reset function */
	NULL, /* no rethink function */
	NULL, /* no power function */
	0,                      /* no switches */
	NULL,
	0,                      /* no buttons */
	NULL,
	0,                      /* no onboard roms */
	NULL,
	NULL,	/* no cursor update function */
	NULL, /* no ROM slots */
	NULL,	/* printer */
	NULL, /* joystick */
	0,
	NULL,
	Amdrum_SoundCallback,
	NULL,
	NULL, /* reti handler */
	NULL, /* ack maskable interrupt handler */
	NULL,
	NULL,
	NULL,
	NULL
};

void Amdrum_Init(void)
{
	RegisterDevice(&AmdrumDevice);
}










