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

/* CTC-AY emulation */
/* TODO: Hw reset for ctc */
/* CTC */

#include "cpc.h"
#include "cpcay.h"
#include "psg.h"
#include "emudevice.h"
#include "z80ctc.h"

z80ctc CTCAYCTC;
/* AY on the CTC-AY board */
AY_3_8912 CTCAYLeft;
AY_3_8912 CTCAYRight;

/* Check the jumpers and fix */
/* FALSE = line out (internal or amplified external), TRUE = speaker */
/*BOOL bJumperSound = FALSE; */
/* FALSE = CPC sound frequency, TRUE = ST sound frequency */
/*BOOL bSoundFrequency = FALSE; */

/* %1111 100a 1000 ppcc
 pp  = peripheral
 00 -> ctc (bit 1,0 become ctc register)
 01 -> ymz/ay (right)
 10 -> ymz/ay (left)
 11 -> extension

 cc = ctc register
*/

void CTCAY_CursorUpdateFunction(int nState)
{
	/* TODO: */
	/* cursor is connected to timer */

}

void CTCAY_SoundCallback(void)
{
}

void CTCAY_RetiCallback(void)
{
    z80ctc_reti(&CTCAYCTC);
}

void	CTCAYWrite(Z80_WORD Port, Z80_BYTE Data)
{
    switch (Port & ((1<<2) | (1<<3)))
  {
	default:
      case 0:
      {
          /* ctc */
		  z80ctc_write(&CTCAYCTC,Port&0x03, Data);
      }
      break;

	/* right ay */
      case (1<<2):
	  {
	      if (Port & (1<<8))
          {
              /* register */
              PSG_RegisterSelect(&CTCAYRight, Data);
          }
          else
          {
              /* data */
              PSG_WriteData(&CTCAYRight, Data);
          }

	  }
	  break;

	  /* left ay */
	  case (1<<3):
      {
          if (Port & (1<<8))
          {
              /* register */
              PSG_RegisterSelect(&CTCAYLeft, Data);
          }
          else
          {
              /* data */
              PSG_WriteData(&CTCAYLeft, Data);
          }
      }
      break;

	  case (1<<2)|(1<<3):
	  {

	  }
	  break;
    }
  }

void CTCAY_Reset(void)
  {
    /* CTC reset */
	z80ctc_reset(&CTCAYCTC);

    /* PSG reset */
      PSG_Reset(&CTCAYLeft);
      PSG_Reset(&CTCAYRight);
  }

void	CTCAYBusResetWrite(Z80_WORD Port, Z80_BYTE Data)
{
	CTCAY_Reset();
}

CPCPortWrite CTCAYWritePort[2]=
{
    {
        0x0fef0,  /* and */
        0x0fe80, /* compare */
        CTCAYWrite

    },
    {
        0x0f8ff,    /* and */
        0x0f8ff,    /* compare */
        CTCAYBusResetWrite
    }
};

void CTCAYBusReset(void)
{

}

static EmuDeviceButton CTCAYResetButton=
{
	"Reset",
	CTCAYBusReset
};

static EmuDevice CTCAYDevice=
{
	NULL,
	NULL,
	NULL,
	"CTCAY",
	"PlayCity",
	"ToTO's PlayCity",
	CONNECTION_EXPANSION,
	DEVICE_FLAGS_HAS_AUDIO|DEVICE_FLAGS_HAS_PASSTHROUGH| DEVICE_FLAGS_TESTED,
  0,  /* no read port */
  NULL,
  2,  /* 2 write port */
  CTCAYWritePort,
  0,                /* no memory read*/
  NULL,
  0,                /* no memory write */
  NULL,
  CTCAY_Reset,
  NULL,
  NULL,
	0,                      /* no switches */
	NULL,
    1,                      /* no buttons */
    &CTCAYResetButton,
    0,                      /* no onboard roms */
    NULL,
	CTCAY_CursorUpdateFunction,	/* no cursor update function */
	NULL,                       /* has generic roms in a way but not yet */
	NULL,						/* printer */
	NULL,						/* joystick */
	0,
	NULL,						/* memory ranges */
	CTCAY_SoundCallback,
	NULL,						/* lpen */
	CTCAY_RetiCallback,			/* maskable interrupt */
	NULL, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */

};

void CTCAY_Init(void)
{
	RegisterDevice(&CTCAYDevice);
}







