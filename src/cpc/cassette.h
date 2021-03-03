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
#ifndef __CASSETTE_HEADER_INCLUDED__
#define __CASSETTE_HEADER_INCLUDED__

#include "cpcglob.h"

typedef enum
{
	CASSETTE_TYPE_SAMPLE = 0,
	CASSETTE_TYPE_TAPE_IMAGE,
	CASSETTE_TYPE_NONE
} CPC_CASSETTE_TYPE_ID;

void	CPC_SetCassetteType(int);

/* press play button on player */
void	Cassette_PressPlay(BOOL bPlay);
/* get play button state on player */
BOOL	Cassette_GetPlay(void);

BOOL	Cassette_GetMotorState(void);

/* does cassette player ignore motor? - i.e. relay not connected to cassette */
void	Cassette_SetIgnoreRelay(BOOL bPlay);
/* does cassette player ignore motor */
BOOL	Cassette_GetIgnoreRelay(void);

/* press pause button on player */
void	Cassette_PressPause(BOOL bPause);
/* get pause button on player */
BOOL	Cassette_GetPause(void);


unsigned long Cassette_Read(void);
void	Cassette_Write(unsigned long NopsPassed, unsigned long State);
void	Cassette_Init(void);
void	Cassette_Finish(void);

unsigned long CPC_GetCassetteType(void);

void    Cassette_NegatePolarity(BOOL fState);
BOOL    Cassette_IsPolarityNegated(void);

void	Tape_Remove(void);


#endif

