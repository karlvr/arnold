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
#include "cassette.h"

#ifndef CASSETTE_NOSAMPLE
#include "sampload.h"
#endif

#include "tzx.h"


void	Cassette_Init(void)
{
/*	TZX_Write_Initialise("out.tmp");
*/
	TapeImage_Init();

}

void	Cassette_Finish(void)
{
/*	TZX_Write_End();
*/

}

static unsigned long CPC_CassetteType = CASSETTE_TYPE_NONE;

void	CPC_SetCassetteType(int CassetteType)
{
	CPC_CassetteType = CassetteType;
}

unsigned long CPC_GetCassetteType(void)
{
	return CPC_CassetteType;
}

static BOOL NegatePolarity = FALSE;
unsigned long PreviousCassetteNopCount;
BOOL PreviousCassetteNopCountSet = FALSE;

static BOOL PlayState = FALSE;

void	Cassette_PressPlay(BOOL bPlay)
{
	PlayState = bPlay;
}

BOOL	Cassette_GetPlay(void)
{
	return PlayState;
}


static BOOL PauseState = FALSE;

void	Cassette_PressPause(BOOL bPause)
{
	PauseState = bPause;
}

BOOL	Cassette_GetPause(void)
{
	return PauseState;
}


static BOOL bIgnoreRelay = FALSE;

void Cassette_SetIgnoreRelay(BOOL bState)
{
	bIgnoreRelay = bState;
}

BOOL Cassette_GetIgnoreRelay(void)
{
	return bIgnoreRelay;
}


BOOL Cassette_GetMotorState(void)
{
	/* if not ignoring relay and relay is off then no output from tape */
	if (!bIgnoreRelay && (!Computer_GetTapeMotorOutput()))
		return FALSE;

	/* not playing, or paused */
	if (!PlayState || PauseState)
		return FALSE;

	return TRUE;
}


unsigned long Cassette_Read(void)
{
	unsigned long CassetteReadBit = 0;
	unsigned long NopsPassed;

	if (!Cassette_GetMotorState())
	{ 
		PreviousCassetteNopCountSet = FALSE;
		return CassetteReadBit;
	}

	if (!PreviousCassetteNopCountSet)
	{
		PreviousCassetteNopCountSet = TRUE;
		PreviousCassetteNopCount = CPC_GetNopCount();
	}

	NopsPassed = CPC_GetNopCount() - PreviousCassetteNopCount;

/*printf("Cassette_Read\n"); */
	switch (CPC_CassetteType)
	{
#ifndef CASSETTE_NOSAMPLE
		case CASSETTE_TYPE_SAMPLE:
		{
			CassetteReadBit = Sample_GetDataByteTimed(NopsPassed);
		}
		break;
#endif

		case CASSETTE_TYPE_TAPE_IMAGE:
		{
			unsigned long TStatesPassed;

			/* 4 t-states per NOP */
			TStatesPassed = NopsPassed<<2;


			CassetteReadBit = TapeImage_GetBit(TStatesPassed);
		}
		break;

		default:
			break;
	}

    if (NegatePolarity)
    {
        CassetteReadBit = ~CassetteReadBit;
    }
    
	PreviousCassetteNopCount = CPC_GetNopCount();

	return CassetteReadBit&0x01;
}


void    Cassette_NegatePolarity(BOOL fState)
{
    NegatePolarity = fState;
}

BOOL    Cassette_IsPolarityNegated(void)
{
    return NegatePolarity;
}


void	Cassette_Write(unsigned long NopsPassed, unsigned long State)
{
/*	TZX_Write(NopsPassed, State);*/
}



void	Tape_Remove(void)
{
	TapeImage_Remove();
#ifndef CASSETTE_NOSAMPLE
	Sample_Close();
#endif
}
