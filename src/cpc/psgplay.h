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
#ifndef __PSG_PLAY_HEADER_INCLUDED__
#define __PSG_PLAY_HEADER_INCLUDED__


#include "cpcglob.h"

BOOL AY_IsNoiseEnabled(void);
void  AY_SetNoiseEnabled(BOOL bState);

BOOL AY_IsHardwareEnvelopeEnabled(void);
void AY_SetHardwareEnvelopeEnabled(BOOL bState);

BOOL AY_IsChannelAEnabled(void);
void  AY_SetChannelAEnabled(BOOL bState);
BOOL AY_IsChannelBEnabled(void);
void  AY_SetChannelBEnabled(BOOL bState);
BOOL AY_IsChannelCEnabled(void);
void  AY_SetChannelCEnabled(BOOL bState);


/* raw sound output from AY, before being mixed */
typedef struct
{
	unsigned short A;
	unsigned short B;
	unsigned short C;
} PSG_OUTPUT;

typedef struct
{
	union
	{
		signed long	L;

#ifdef CPC_LSB_FIRST
		struct
		{
			unsigned short Fraction;
			signed short Int;
		} W;
#else
		struct
		{
			signed short Int;
			unsigned short Fraction;
		} W;
#endif

	} FixedPoint;
} FIXED_POINT16;


void PSG_UpdateChannels(PSG_OUTPUT *pOutput, FIXED_POINT16 *pPeriodUpdate );

void	PSG_UpdateState(unsigned long Reg, unsigned long Data);

char	*PSG_GetRegisterName(int Index);

void	PSG_InitialiseToneUpdates(FIXED_POINT16 *pUpdate);


void PSGPlay_Write(int Register, int Data);
void PSGPlay_Reset(void);
void PSGPlay_Initialise(void);


#endif
