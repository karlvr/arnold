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
#include "psg.h"
#include "cpcglob.h"
/*
0xF: 1V, 0xE: 0.707V, 0xD: 0.5V, 0xC: 0.303V, 0xB: 0.25V, 0xA: 0.15151V, 0x9: 0.125V

log2 on (n/2)
One interesting point is that the channels are mixed equally to the internal speaker but B only contributes 5/16 of the volume to each of left and right  whereas A and C contribute 11/16 to left and right respectively.
*/

/* these are anded with the data when a read is done */
static const int		PSG_ReadAndMask[16] =
{
	0x0ff,	/* channel A tone fine */
	0x00f,	/* channel A tone coarse */
	0x0ff,	/* channel B tone fine */
	0x00f,	/* channel B tone coarse */
	0x0ff,	/* channel C tone fine */
	0x00f,	/* channel C tone coarse */
	0x01f,	/* noise */
	0x0ff,	/* mixer */
	0x01f,	/* volume A */
	0x01f,	/* volume B */
	0x01f,	/* volume C */
	0x0ff,	/* hardware envelope duration fine */
	0x0ff,	/* hardware envelope duration coarse */
	0x00f,	/* hardware envelope */
	0x0ff,	/* I/O port A */
	0x0ff	/* I/O port B */
};


int PSG_GetPortOutputs(AY_3_8912 *ay, int nPort)
{
	return ay->PortOutputs[nPort];
}

void 	PSG_SetBC2State(AY_3_8912 *ay, int State)
{
	if (State!=0)
	{
		ay->state |= (1<<1);
	}
	else
	{
		ay->state &= ~(1<<1);
	}
}


int PSG_GetBC2(AY_3_8912 *ay)
{
	return ay->state & (1<<1);
}

int PSG_GetBC1(AY_3_8912 *ay)
{
	return ay->state & (1<<0);
}

int PSG_GetBDIR(AY_3_8912 *ay)
{
	return ay->state & (1<<2);
}

void	PSG_SetBDIRState(AY_3_8912 *ay, int State)
{
	if (State!=0)
	{
		ay->state |= (1<<2);
	}
	else
	{
		ay->state &= ~(1<<2);
	}
}

void	PSG_SetBC1State(AY_3_8912 *ay, int State)
{
	if (State!=0)
	{
		ay->state |= (1<<0);
	}
	else
	{
		ay->state &= ~(1<<0);
	}
}

void	PSG_RefreshState(AY_3_8912 *ay)
{
	switch (ay->state & 0x07)
	{
			/* latch */
		case 1:
		case 4:
		case 7:
			ay->mode = MODE_LATCH;
			break;

			/* inactive */
		case 0:
		case 2:
		case 5:
			ay->mode = MODE_INACTIVE;
			break;

			/* read */
		case 3:
			ay->mode = MODE_READ;
			break;

			/* write */
		case 6:
			ay->mode = MODE_WRITE;
			break;
	}
}



void    PSG_ResetFlags(AY_3_8912 *ay)
{
	int i;
	for (i=0; i<16; i++)
	{
		ay->PSG_Flags[i] = 0;
	}
}

int PSG_GetFlags(AY_3_8912 *ay, int nRegister)
{
	return ay->PSG_Flags[nRegister];
}

void	PSG_Init(AY_3_8912 *ay)
{
	PSGPlay_Initialise();
}

void PSG_Power(AY_3_8912 *ay)
{
	PSG_Reset(ay);
}

/* reset PSG and put it into it's initial state */
void	PSG_Reset(AY_3_8912 *ay)
{
	int i;

	ay->mode = MODE_INACTIVE;

	/* reset all registers to 0 - as per the Ay-3-8912 spec */
	for (i=0; i<16; i++)
	{
		PSG_RegisterSelect(ay,i);
		ay->PSG_Registers[i] = 0;
		PSG_WriteData(ay,0);
		ay->PSG_Flags[i] = 0;

	}
	PSG_RegisterSelect(ay,0);


	PSG_RefreshPortOutputs(ay,0);
	PSG_RefreshPortOutputs(ay,1);

	PSGPlay_Reset();
}

/*----------------------------------------------------------------------------*/

void PSG_SetType(AY_3_8912 *ay, int type)
{
	ay->type = type;
}

int PSG_Read(AY_3_8912 *ay)
{

	if (ay->mode==MODE_READ)
	{
		return PSG_ReadData(ay);
	}

	return 0x0ff;
}


int PSG_GetMode(AY_3_8912 *ay)
{
	return ay->mode;
}

void  PSG_Write(AY_3_8912 *ay, int Data)
{
	if (ay->mode==MODE_WRITE)
	{
		PSG_WriteData(ay,Data);
	}
	else
		if (ay->mode==MODE_LATCH)
		{
			PSG_RegisterSelect(ay,Data);
		}
}

/*----------------------------------------------------------------------------*/
unsigned int		PSG_ReadData(AY_3_8912 *ay)
{
	/* factory sets bits 7..4 to 0 */
	/* effectively this means only 0-15 give a result */
	switch (ay->PSG_SelectedRegister)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
			return ay->PSG_Registers[ay->PSG_SelectedRegister] & PSG_ReadAndMask[ay->PSG_SelectedRegister];

			/* port A */
		case 14:
		case 15:
		{
			int Port;
			unsigned char Data;
		

        if (ay->type==PSG_TYPE_YMZ284)
        {
            /* 14 does nothing */
            /* 15 bits 7,6,5,4 must be 0 */

			/* is this correct? */
            return 0x0ff;
        }

			Port = ay->PSG_SelectedRegister-14;
			

			if ((ay->type==PSG_TYPE_AY8912) && (ay->PSG_SelectedRegister==15))
			{
				Data = 0x0ff;
			}
			else
			{
				Data = PSG_GetPortInputs(ay,Port);
			}

			/* output has a AND mask of 0, input has a AND mask of 0x0ff */

			/* if port A is set to input, a read will return keyboard line data */
			/* if port A is set to output, a read will return output latch ANDed with port input */
			return ((Data & ay->io_mask[Port]) | (ay->PSG_Registers[Port+14] & Data & (~ay->io_mask[Port])));
		}
		break;

		default
				:
			break;
	}

	/* tests confirm all others will read 0x0ff */
	return 0x0ff;
}
/*----------------------------------------------------------------------------*/

void    PSG_RefreshPortOutputs(AY_3_8912 *ay, int Port)
{
	/* if port is programmed as input, 0x0ff will be seen on the output, otherwise you will see data written */
	int Data = ((0x0ff & ay->io_mask[Port]) | (ay->PSG_Registers[Port+14] & (~ay->io_mask[Port])));
	ay->PortOutputs[Port] = Data;
	PSG_SetPortOutputs(ay,Port, Data);
}
/*----------------------------------------------------------------------------*/

void	PSG_WriteData(AY_3_8912 *ay, unsigned int Data )
{
	/* ignore any writes to registers of 16 or above */
	if (ay->PSG_SelectedRegister>=16)
	{
		return;
	}

	/* store previous value */
	ay->PSG_PreviousRegisters[ay->PSG_SelectedRegister] = ay->PSG_Registers[ay->PSG_SelectedRegister];
	ay->PSG_Flags[ay->PSG_SelectedRegister] |= AY_REG_UPDATED;

	Data = Data & PSG_ReadAndMask[ay->PSG_SelectedRegister];

	/* if port A or port B is set to output, writing to the port register will store the value */
	/* it can be read again as soon as the port is set to output */
	/* write data to register */
	ay->PSG_Registers[ay->PSG_SelectedRegister] = Data;

	/* setup I/O mask for reading/writing register depending on input/output status */
	switch (ay->PSG_SelectedRegister)
	{
		case 7:
		{
			ay->io_mask[0] = (ay->io_mask[1] = 0x0ff);

			if (Data & (1<<7))
			{
				/* port B is output mode */
				ay->io_mask[1] = !ay->io_mask[1];
			}

			if (Data & (1<<6))
			{
				ay->io_mask[0] = !ay->io_mask[0];
			}

			PSG_RefreshPortOutputs(ay,0);
			PSG_RefreshPortOutputs(ay,1);
		}
		break;

		case 14:
		case 15:
		{
			PSG_RefreshPortOutputs(ay,ay->PSG_SelectedRegister-14);
		}
		break;
	}

	if (ay->PSG_SelectedRegister<14)
	{
		/* write register for audio playback */
		PSGPlay_Write(ay->PSG_SelectedRegister, Data);
	}
}

/*----------------------------------------------------------------------------*/

void	PSG_RegisterSelect(AY_3_8912 *ay, unsigned int Data)
{

	ay->PSG_SelectedRegister = Data;
}

/*----------------------------------------------------------------------------*/

/* for debugging */
int		PSG_GetSelectedRegister(AY_3_8912 *ay)
{
	return ay->PSG_SelectedRegister;
}

/*----------------------------------------------------------------------------*/

/* for debugging; not correct for port A and port B */
int		PSG_GetRegisterData(AY_3_8912 *ay, int RegisterIndex)
{
	RegisterIndex &= 0x0f;

	return ay->PSG_Registers[RegisterIndex];
}
