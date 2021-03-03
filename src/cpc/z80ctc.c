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
#include "z80ctc.h"

/* NOTE: Software reset stops the counters, you need to program a new time constant */
/* max frequency is half system clock */

void z80ctc_reset(z80ctc *ctc)
{

}

void z80ctc_reti(z80ctc *ctc)
{

}

unsigned char z80ctc_interruptvector(z80ctc *ctc)
{
	int Channel = 0;
	int i;
	for (i=0; i<4; i++)
	{
		if (ctc->InterruptPending&(1<<i))
		{
			Channel = i;
			break;
		}
	}
	return ctc->InterruptVector | (Channel<<1);
}

void z80ctc_write(z80ctc *ctc, Z80_WORD Port, Z80_BYTE Data)
{
	int Channel = Port & 0x03;

	/* time constant to follow? */
	if (ctc->ControlRegister[Channel]&(1<<2))
	{
		/* time constant to follow? */
		ctc->TimeConstant[Channel] = Data;
		ctc->ControlRegister[Channel]&=~(1<<2);
	}
	else
	{
		if (Data&(1<<0))
		{
			/* control word */
			ctc->ControlRegister[Channel] = Data;
		}
		else
		{
			ctc->InterruptVector = Data & 0x018;


		}

	}

}
