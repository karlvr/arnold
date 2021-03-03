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
#include "mc146818.h"

enum
{
	MC146818_REGISTER_SECONDS = 0,
	MC146818_REGISTER_SECONDS_ALARM,
	MC146818_REGISTER_MINUTES,
	MC146818_REGISTER_MINUTES_ALARM,
	MC146818_REGISTER_HOURS,
	MC146818_REGISTER_HOURS_ALARM,
	MC146818_REGISTER_DAY_OF_WEEK,
	MC146818_REGISTER_DAY,
	MC146818_REGISTER_MONTH,
	MC146818_REGISTER_YEAR,
	MC146818_REGISTER_REGISTER_A,
	MC146818_REGISTER_REGISTER_B,
	MC146818_REGISTER_REGISTER_C,
	MC146818_REGISTER_REGISTER_D
};


typedef struct
{
	int Registers[64];
    int Register;
} mc146818;

static mc146818 rtc;


void mc146818_update()
{


}

void mc146818_reset()
{
}


void mc146818_write_address(int Data)
{
    rtc.Register = Data;
}

int mc146818_read_address()
{
    return rtc.Register&0x03f;
}



void mc146818_write(int Data)
{
    rtc.Registers[rtc.Register] = Data;
}


int mc146818_read()
{
    return rtc.Registers[rtc.Register];


}

