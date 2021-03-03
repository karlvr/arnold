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
/* THIS EMULATION IS INCOMPLETE. MORE TESTING IS NEEDED ON A REAL DEVICE */

#include "cpc.h"
#include "emudevice.h"
#include "crtc.h"
//#include "digiblaster.h"
#include "printer.h"
#include "audio.h"


static unsigned short DigiBlasterVolume = 100;
static BOOL bDigiblasterEnable = FALSE;
static unsigned short VolDigiBlaster;

/*******************************************************************/

BOOL DigiblasterEnabled(void)
{
	return bDigiblasterEnable;
}

void Digiblaster_Reset(int BitsPerSample)
{
	if (BitsPerSample == 8) {
		VolDigiBlaster = 0x080;//8 bit
	}
	else
	{
		VolDigiBlaster = 0x000;//16 bit
	}
}

void Digiblaster_Enable(BOOL Enable)
{
	bDigiblasterEnable = Enable;
}

unsigned short VolumeDigiBlaster(void)
{
	return VolDigiBlaster;
}


void	Audio_Digiblaster_Write(unsigned char Digiblaster_Data)
{
	Digiblaster_Data = (((Digiblaster_Data ^ 0x080)*DigiBlasterVolume) / 100) ^ 0x080;

	VolDigiBlaster = Digiblaster_Data;
}

/* TODO: how does digiblaster handle printer ready? */
void Digiblaster_PrinterUpdateFunction(void)
{
	unsigned char data;

	data = Printer_Get8BitData();

	if (Printer_GetStrobeOutputState())
	{
		data = data | 128;
	}

	Audio_Digiblaster_Write(data);
}

void Digiblaster_SoundCallback(void)
{
}


/**** DIGIBLASTER ****/

static EmuDevice DigiblasterDevice =
{
	Digiblaster_Enable,
	NULL,
	NULL,
	"DIGIBLASTER",
	"Digiblaster",
	"Digiblaster",
	CONNECTION_PRINTER, 
	DEVICE_FLAGS_HAS_AUDIO,
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
	Digiblaster_PrinterUpdateFunction,					/* printer */
	NULL,					/* joystick */
	0,
	NULL,
	Digiblaster_SoundCallback,
	NULL,	/* lpen */
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	NULL, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
	NULL,


};

int DigiblasterDevice_Init()
{
	return RegisterDevice(&DigiblasterDevice);
}