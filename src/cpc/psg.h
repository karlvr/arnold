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
#ifndef __PSG_HEADER_INCLUDED__
#define __PSG_HEADER_INCLUDED__

#include "cpcglob.h"
#include "psgplay.h"

#define PSG_MIXER_IO_B_INPUT_ENABLE 0x080
#define PSG_MIXER_IO_A_INPUT_ENABLE 0x040
#define PSG_MIXER_NOISE_C_ENABLE 0x20
#define PSG_MIXER_NOISE_B_ENABLE 0x10
#define PSG_MIXER_NOISE_A_ENABLE 0x08
#define PSG_MIXER_TONE_C_ENABLE 0x04
#define PSG_MIXER_TONE_B_ENABLE	0x02
#define PSG_MIXER_TONE_A_ENABLE	0x01

#define PSG_ENVELOPE_HOLD		0x01
#define PSG_ENVELOPE_ALTERNATE	0x02
#define PSG_ENVELOPE_ATTACK		0x04
#define PSG_ENVELOPE_CONTINUE	0x08

enum
{
	MODE_INACTIVE = 0,
	MODE_READ,
	MODE_WRITE,
	MODE_LATCH
};
/* register was updated this frame */
#define AY_REG_UPDATED 0x0001
/* register data is different from previous frame */
#define AY_REG_DATA_CHANGED 0x0002


typedef struct
{
	/* stores current selected register */
	int		PSG_SelectedRegister;

	/* stores current register data */
	int		PSG_Registers[16];
	int     PSG_PreviousRegisters[16];

	/* io mask for port A and B */
	/* when 0x0ff will return input's, when 0x00 will return state of output latch */
	int		io_mask[2];

	int		PortOutputs[2];

	int     PSG_Flags[16];

	int		mode;

	int 	state;

	int type;
} AY_3_8912;


enum
{
    PSG_TYPE_AY8910,
    PSG_TYPE_AY8912, /* in cpc and kcc */
    PSG_TYPE_YM2149, /* in aleste */
    PSG_TYPE_YMZ284,
    PSG_TYPE_YMZ294 /* same as 284 with selectable 4mhz/8mhz clock */
};

void PSG_SetType(AY_3_8912 *ay, int);
void 	PSG_SetBC2State(AY_3_8912 *ay, int State);
void	PSG_SetBDIRState(AY_3_8912 *ay, int State);
void	PSG_SetBC1State(AY_3_8912 *ay, int State);
int     PSG_GetBC2(AY_3_8912 *ay);
int PSG_GetBDIR(AY_3_8912 *ay);
int    PSG_GetBC1(AY_3_8912 *ay);

int PSG_GetPortOutputs(AY_3_8912 *ay, int nPort);

void	PSG_RefreshState(AY_3_8912 *ay);
int     PSG_GetMode(AY_3_8912 *ay);

void	PSG_Write(AY_3_8912 *ay,int Data);
int		PSG_Read(AY_3_8912 *ay);

/* reset PSG */
void	PSG_Reset(AY_3_8912 *ay);

/* power on PSG */
void PSG_Power(AY_3_8912 *ay);

/* read data from selected register */
unsigned int	PSG_ReadData(AY_3_8912 *ay);

/* write data to selected register */
void	PSG_WriteData(AY_3_8912 *ay,unsigned int);

/* select register */
void	PSG_RegisterSelect(AY_3_8912 *ay, unsigned int);

/* get selected register - for snapshot and multiface, and ASIC */
int		PSG_GetSelectedRegister(AY_3_8912 *ay);
/* get register data - for snapshot and multiface */
int		PSG_GetRegisterData(AY_3_8912 *ay, int );

void	PSG_Init(AY_3_8912 *ay);

void    PSG_ResetFlags(AY_3_8912 *ay);

int PSG_GetFlags(AY_3_8912 *ay, int nRegister);
extern int PSG_GetPortInputs(AY_3_8912 *ay, int Port);
extern void PSG_SetPortOutputs(AY_3_8912 *ay, int Port, int Data);
void    PSG_RefreshPortOutputs(AY_3_8912 *ay, int Port);

#endif
