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
#include "spo256.h"
#include "emudevice.h"

/* SSA1 speech */

void SSA1_Write(Z80_WORD Port, Z80_BYTE Data)
{
   	SPO256_SetAddress(Data);
}


BOOL SSA1_Read(Z80_WORD Port, Z80_BYTE *pDeviceData)
{
   /*
 bit7   Status 1 (0=Speech Busy, 1=Ready/Halted)   (SBY Pin, Standby)
 bit6   Status 2 (0=Ready to Receive Data, 1=Busy) (/LRQ Pin, Load Request)
 bit5-0 Not used (garbage, probably usually highz)
*/

	unsigned char Data;

	Data = (((SPO256_GetLRQ()^0x01)<<7) | ((SPO256_GetSBY()^0x01)<<6));
    Data |= 0x01f;
	*pDeviceData = Data;
    return TRUE;
}

static CPCPortWrite SSA1PortWrite=
{
    /* A10, A4, A0 are decoded only */
    (1<<10)|(1<<4)|(1<<0),
    0x0,
    SSA1_Write
};

static CPCPortRead SSA1PortRead=
{
    /* A10, A4, A0 are decoded only */
    (1<<10)|(1<<4)|(1<<0),
    0x0,
    SSA1_Read
};

void SSA1_SoundCallback(void)
{
}

static EmuDevice SSA1SpeechDevice=
{
	NULL,
	NULL,
	NULL,
	"SSA1SPEECH",
	"SSA1Speech",
	"SSA-1 Speech",
    CONNECTION_EXPANSION,   /* connects to expansion */
	0,
  1,                /* no read ports */
  &SSA1PortRead,
  1,                    /* 1 write ports */
  &SSA1PortWrite, 
  0,                /* no memory read*/
  NULL,
  0,                /* no memory write */
  NULL,
  NULL, /* reset function */
  NULL, /* memory rethink */
  NULL, /* power function */
	0, /* no switches */
	NULL,
   0,                      /* no buttons */
    NULL,
    0,                      /* no onboard roms */
    NULL,
    NULL,                   /* no cursor function */
    NULL,                   /* no expansion roms */
	NULL,
	NULL,
	0,
	NULL,
	SSA1_SoundCallback,
	NULL, /* lpen */
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	NULL, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
	NULL,
};

void SSA1SpeechDevice_Init(void)
{
	RegisterDevice(&SSA1SpeechDevice);
}

/* DK'tronics speech */

void DkTronicsSpeech_Write(Z80_WORD Port, Z80_BYTE Data)
{
    /* lower 6 bits are connected */
    SPO256_SetAddress(Data&0x01f);

}


BOOL DkTronicsSpeech_Read(Z80_WORD Port, Z80_BYTE *pDeviceData)
{
/* N/A    Status 1 (none, SP0256.Pin8 is not connected) ;SBY Pin, Standby
 bit7   Status 2 (0=Ready to Receive Data, 1=Busy)    ;LRQ Pin, Load Request
 bit6-0 Not used (garbage, probably usually high-z)
*/

	unsigned char Data;

	Data = (SPO256_GetSBY()<<7);
    Data |= 0x03f;
	*pDeviceData = Data;
	
    return TRUE;
}

static CPCPortWrite DkTronicsSpeechPortWrite=
{
    0x0ffff,
    0x0fbfe,
    DkTronicsSpeech_Write
};

static CPCPortRead DkTronicsSpeechPortRead=
{
    0x0ffff,
    0x0fbfe,
    DkTronicsSpeech_Read
};

void DkTronicsSpeech_SoundCallback(void)
{
}

static EmuDevice DkTronicsSpeechDevice=
{
	NULL,
	NULL,
	NULL,
	"DKTRONICSSPEECH",
	"DkTronicsSpeech",
	"Dk'Tronics Speech",
    CONNECTION_EXPANSION,   /* connects to expansion */
	0,
  1,                /* no read ports */
  &DkTronicsSpeechPortRead,
  1,                    /* 1 write ports */
  &DkTronicsSpeechPortWrite, 
  0,                /* no memory read*/
  NULL,
  0,                /* no memory write */
  NULL,
  NULL, /* reset function */
  NULL, /* memory rethink */
  NULL, /* power function */
	0,
	NULL,
   0,                      /* no buttons */
    NULL,
    0,                      /* no onboard roms */
    NULL,
    NULL,                   /* no cursor function */
    NULL,                   /* no expansion roms */
	NULL,
	NULL,
	0,
	NULL,
	DkTronicsSpeech_SoundCallback,
	NULL, /* lpen */
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	NULL, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
	NULL,
    };

void DkTronicsSpeech_Init(void)
{
	RegisterDevice(&DkTronicsSpeechDevice);
}

