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
/* code to emulate the sound */

#include "psgplay.h"
/*#include "audioevent.h" */
#include "psg.h"
#include "headers.h"

/*8Bit Amplitude table */
/*static int	AY_VolumeTranslation[]= {0,0,0,1,1,1,2,3,4,6,8,10,14,19,24,31}; */
/*16 bit Amplitude table */
static unsigned short Amplitudes_AY[16] = {0,104,151,221,327,484,674,1102,1299,2088,2917,3661,4621,5802,6899,8191};

BOOL ChannelEnabled[3] = { TRUE, TRUE, TRUE };
BOOL NoiseEnabled = TRUE;
BOOL HardwareEnvelopeEnabled = TRUE;

BOOL AY_IsNoiseEnabled(void)
{
	return NoiseEnabled;
}
void  AY_SetNoiseEnabled(BOOL bState)
{
	NoiseEnabled = bState;
}

BOOL AY_IsHardwareEnvelopeEnabled(void)
{
	return HardwareEnvelopeEnabled;
}
void  AY_SetHardwareEnvelopeEnabled(BOOL bState)
{
	HardwareEnvelopeEnabled = bState;
}


BOOL AY_IsChannelAEnabled(void)
{
	return ChannelEnabled[0];
}
void  AY_SetChannelAEnabled(BOOL bState)
{
	ChannelEnabled[0] = bState;
}

BOOL AY_IsChannelBEnabled(void)
{
	return ChannelEnabled[1];
}
void  AY_SetChannelBEnabled(BOOL bState)
{
	ChannelEnabled[1] = bState;
}

BOOL AY_IsChannelCEnabled(void)
{
	return ChannelEnabled[2];
}
void  AY_SetChannelCEnabled(BOOL bState)
{
	ChannelEnabled[2] = bState;
}


/* these are only updated when the audio event update requests it */
static int		PSGPlay_Registers[16];

/*static unsigned long             VolumeLookup8Bit[32]; */


/*static int PSGPlay_ChannelOutputVolumes[3]; */

typedef struct
{
	/* tone period for this channel. Period is time for 1/2
	the square wave */
	unsigned long Period;

	/* Period up count */
	FIXED_POINT16 PeriodCount;
	/* state of square wave. 0x00 or 0x0ffff */
	unsigned short	WaveFormState;
	/*FIXED_POINT16	ToneUpdate; */
} CHANNEL_PERIOD;


CHANNEL_PERIOD	ChannelPeriods[3]=
{

	{0,{{0}},0x0ff},
	{0,{{0}},0x0ff},
	{0,{{0}},0x0ff}

};

/*static 	FIXED_POINT16	NoisePeriod; */
static FIXED_POINT16 NoisePeriodCount;
static int RNG=1;
static unsigned short NoiseOutput = 0x0ffff;
static int NoisePeriod = 0;
/*static FIXED_POINT16 NewNoiseUpdate;
static FIXED_POINT16 NoiseUpdate;
*/
static unsigned short EnvelopePeriod= 0;


static FIXED_POINT16 EnvelopePeriodCount;

static int PositionInEnvelope = 0;
/*static char *pEnvelope = (char *)Attack; */
static int EnvelopeVolume = 0;
static int CurrentEnvelope;
static BOOL bHeld = FALSE;
static int Repeats = 0;

void	InitChannelTone(int ChannelIndex);
void	InitNoisePeriod(void);


void	PSG_Envelope_Initialise(void);
void	PSG_Envelope_SetPeriod(void);


static FIXED_POINT16	Update;

void	PSG_InitialiseToneUpdates(FIXED_POINT16 *pUpdate)
{
	Update.FixedPoint.L = pUpdate->FixedPoint.L;
}


void	PSGPlay_Reset(void)
{
	int i;
	memset(PSGPlay_Registers, 0, sizeof(PSGPlay_Registers));
	NoisePeriodCount.FixedPoint.L=0;
	NoisePeriod = 0;
	EnvelopePeriod = 0;
	Update.FixedPoint.L = 0;
	for (i = 0; i < 3; i++)
	{
		ChannelPeriods[i].Period = 0;
		ChannelPeriods[i].PeriodCount.FixedPoint.L = 0;
		ChannelPeriods[i].WaveFormState = 0xffff;
	}

	RNG=1;
	NoiseOutput=0x0ffff;

	EnvelopePeriodCount.FixedPoint.L = 0;
	PositionInEnvelope = 0;
	EnvelopeVolume = 0;
	CurrentEnvelope=0;
	bHeld = FALSE;
	Repeats = 0;
	
	for (i = 0; i < 16; i++)
	{
		PSG_UpdateState(i, 0);
	}
}

void	PSGPlay_ReloadChannelTone(int ChannelIndex)
{
/*	unsigned long CurrentCount; */
	unsigned long Period;
	CHANNEL_PERIOD *pChannelPeriod = &ChannelPeriods[ChannelIndex];

	Period = (
		PSGPlay_Registers[(ChannelIndex<<1)] |
		(PSGPlay_Registers[(ChannelIndex<<1)+1]<<8)
		);

	Period = Period & 0x0fff;

	/* calculate current count */
	if (Period<pChannelPeriod->PeriodCount.FixedPoint.W.Int)
	{
		/* reset counter */
		pChannelPeriod->PeriodCount.FixedPoint.L = 0;
	}

	/* set new period */
	pChannelPeriod->Period = Period;

	#if 0
	if (Period<5)
	{
		/* if period <5 is programmed I can't hear any sound  */
		/* but it is possible to hear sound up to 0x0fff */
		pChannelPeriod->ToneUpdate.FixedPoint.L = 0;
	}
	else
	{
		/* set new update */
		pChannelPeriod->ToneUpdate.FixedPoint.L = Update.FixedPoint.L/Period;
	}
#endif
	}


void	PSG_UpdateState(unsigned long Reg, unsigned long Data)
{
	if (Reg >= 0x010)
		return;

	/* write register data */
	PSGPlay_Registers[Reg & 0x0f] = Data;

	switch(Reg)
	{
		case 0:
		case 1:
		{
			PSGPlay_ReloadChannelTone(0);
		}
		break;

		case 2:
		case 3:
		{
			PSGPlay_ReloadChannelTone(1);

		}
		break;

		case 4:
		case 5:
		{
			PSGPlay_ReloadChannelTone(2);
		}
		break;

		case 6:
		{

			InitNoisePeriod();
		}
		break;

		case 11:
		case 12:
			PSG_Envelope_SetPeriod();
			break;

		case 13:
		{
			PSG_Envelope_Initialise();
		}
		break;

		case 7:
		{


		}
		break;
#if 0
		case 8:
		case 9:
		case 10:
			{
				/* setup the output volume - to remove extra lookup */

				/* use hardware envelope? */
				if ((Data & (1<<4))!=0)
				{
					/* no */
					PSGPlay_ChannelOutputVolumes[Reg-8] = Data;
				}

			}
			break;
#endif
	}
}

/*
* 0	0	-	-	\________________________
*

* 0	1	-	-	/|_______________________
*

* 1	0	0	0	\|\|\|\|\|\|\|\|\|\|\|\|\

* 1	0	0	1	\________________________
*

* 1	0	1	0	\/\/\/\/\/\/\/\/\/\/\/\/

*				  _______________________
* 1	0	1	1	\|

* 1	1	0	0	/|/|/|/|/|/|/|/|/|/|/|/|/|

*				 _________________________
* 1	1	0	1	/

* 1	1	1	0	/\/\/\/\/\/\/\/\/\/\/\/\/\

* 1	1	1	1	/|________________________
*
*/

/*static int EnvelopePhase = 0; */

static FIXED_POINT16 EnvelopePeriodCount;

/*static char *pEnvelope = (char *)Attack; */
static BOOL bReverse = FALSE;

void	PSG_Envelope_Initialise(void)
{
	bHeld = FALSE;
	Repeats = 0;
	bReverse = FALSE;

	/* convert some envelopes into other forms hopefully making code a bit quicker */
	CurrentEnvelope = PSGPlay_Registers[13];
	if (CurrentEnvelope & PSG_ENVELOPE_ATTACK)
	{
		EnvelopeVolume = 0;

	}
	else
	{
		EnvelopeVolume = 15;
	}
	/* reset position in envelope */
	PositionInEnvelope = 0;
}



void	PSG_Envelope_SetPeriod(void)
{
	unsigned long CurrentCount;
	unsigned long Period = (PSGPlay_Registers[11] & 0x0ff) | ((PSGPlay_Registers[12] & 0x0ff) << 8);

	/* on amstrad I can't tell the difference between Period of 0 and period of 1 */
	if (Period == 0)
	{
		Period = 1;
	}

	/* calculate current count */
	CurrentCount = EnvelopePeriodCount.FixedPoint.L >> 16; /* *EnvelopePeriod)>>16; */

	if (Period < CurrentCount)
	{
		/* reset counter */
		EnvelopePeriodCount.FixedPoint.L = 0;    /*0x0ffff; */
	}


	/* set new period */
	EnvelopePeriod = Period;
}


void	Envelope_Update(FIXED_POINT16 *pUpdate)
{
	if (!bHeld)
	{
		int NoOfCycles;

		/* update position in period */
		EnvelopePeriodCount.FixedPoint.L += pUpdate->FixedPoint.L / 2;

		NoOfCycles = 0;
		while (EnvelopePeriodCount.FixedPoint.W.Int >= EnvelopePeriod)
		{
			EnvelopePeriodCount.FixedPoint.W.Int -= EnvelopePeriod;
			NoOfCycles++;
		}

		PositionInEnvelope += NoOfCycles;

		if (PositionInEnvelope >= 16)
		{
			if ((CurrentEnvelope & PSG_ENVELOPE_CONTINUE) == 0)
			{
				/* shape 0-8 */
				bHeld = TRUE;
				PositionInEnvelope = 0;
				EnvelopeVolume = 0;
				return;
			}
			else
			{
				if ((CurrentEnvelope & PSG_ENVELOPE_HOLD) != 0)
				{
					/* 9, 11, 13, 15 */
					if ((CurrentEnvelope & PSG_ENVELOPE_ALTERNATE) != 0)
					{
						/* 11, 15 */
						if ((CurrentEnvelope & PSG_ENVELOPE_ATTACK) != 0)
						{
							/* 15 */
							PositionInEnvelope = 0;
						}
						else
						{
							/* 11 */
							PositionInEnvelope = 0;
						}

					}
					else
					{
						/* 9,13 */
						if ((CurrentEnvelope & PSG_ENVELOPE_ATTACK) != 0)
						{
							/* 13 */
							PositionInEnvelope = 15;
						}
						else
						{
							/* 9 */
							PositionInEnvelope = 15;
						}
					}
					bHeld = TRUE;
				}
				else
				{
					//Repeats += (PositionInEnvelope >> 4);
					PositionInEnvelope = 0;

					/* no hold */
					/* 8, 10, 12, 14 */
					switch (CurrentEnvelope)
					{
					case 8:
					{

					}
					break;
					case 10:
					{

						bReverse = bReverse ? FALSE : TRUE;
					}
					break;
					case 12:
					{

					}
					break;
					case 14:
					{
						bReverse = bReverse ? FALSE : TRUE;
					}
					break;

					default:
						break;
					}
				}
			}
		}

		if (CurrentEnvelope & PSG_ENVELOPE_ATTACK)
		{
			EnvelopeVolume = PositionInEnvelope;
		}
		else
		{
			EnvelopeVolume = 15 - PositionInEnvelope;
		}

		if (bReverse)
		{
			EnvelopeVolume = 15 - EnvelopeVolume;
		}


	}
}

void	UpdateChannelToneState(int ChannelIndex, FIXED_POINT16 *pUpdate)
{
	CHANNEL_PERIOD *pChannelPeriod = &ChannelPeriods[ChannelIndex];

	if (pChannelPeriod->Period == 0) return;

	/*	int NoOfCycles;

	NoOfCycles = 0; */
	pChannelPeriod->PeriodCount.FixedPoint.L += pUpdate->FixedPoint.L; /*pChannelPeriod->ToneUpdate.FixedPoint.L; */
	while (pChannelPeriod->PeriodCount.FixedPoint.W.Int >= pChannelPeriod->Period)
	{
		pChannelPeriod->WaveFormState ^= 0x0ffff;
		pChannelPeriod->PeriodCount.FixedPoint.W.Int -= pChannelPeriod->Period;
		/*   NoOfCycles++; */
	}

	/* update position in waveform */

	/* how many cycles have occured in this update ? */
	/*NoOfCycles = (pChannelPeriod->PeriodPosition.FixedPoint.L>>16); */

	/* if odd, invert state, else keep state the same */
	/*	if (NoOfCycles & 0x01)
	{*/
	/* odd number of cycles - invert state*/
	/*	pChannelPeriod->WaveFormState^=0x0ff;
	}
	*/
	/* zeroise integer part */
	/*	pChannelPeriod->PeriodCount.FixedPoint.L &= 0x0ffff; */
}

void	NoiseChooseOutput(void)
{
	/* Is noise output going to change? */
	if ((RNG + 1) & 2)	/* (bit0^bit1)? */
	{
		NoiseOutput = NoiseOutput ^ 0x0ffff;
	}

	/* The Random Number Generator of the 8910 is a 17-bit shift */
	/* register. The input to the shift register is bit0 XOR bit2 */
	/* (bit0 is the output). */

	/* The following is a fast way to compute bit 17 = bit0^bit2. */
	/* Instead of doing all the logic operations, we only check */
	/* bit 0, relying on the fact that after two shifts of the */
	/* register, what now is bit 2 will become bit 0, and will */
	/* invert, if necessary, bit 16, which previously was bit 18. */
	if (RNG & 1) RNG ^= 0x28000;
	RNG >>= 1;
}


void	UpdateNoise(FIXED_POINT16 *pUpdate)
{
	/*int i;
	//int NoOfCycles; */

	NoisePeriodCount.FixedPoint.L += pUpdate->FixedPoint.L;  /*NoiseUpdate.FixedPoint.L; */

															 /*NoOfCycles = 0; */
	while (NoisePeriodCount.FixedPoint.W.Int >= NoisePeriod)
	{
		NoisePeriodCount.FixedPoint.W.Int -= NoisePeriod;
		NoiseChooseOutput();
		/*NoOfCycles++; */
	}


	/*
	// how many cycles have occured in this update ?
	NoOfCycles = NoisePeriodPosition.FixedPoint.L>>16;

	NoisePeriodCount.FixedPoint.L &= 0x0ffff;

	for (i=0; i<NoOfCycles; i++)
	{
	NoiseChooseOutput();
	}
	NoisePeriod.FixedPoint.L = NewNoisePeriod<<16;
	NoiseUpdate.FixedPoint.L = NewNoiseUpdate.FixedPoint.L;
	*/

}

void	InitNoisePeriod(void)
{
	int Period = (PSGPlay_Registers[6] & 0x01f);

	if (Period == 0)
	{
		Period = 1;
	}

	if (Period<NoisePeriodCount.FixedPoint.W.Int)
	{
		/* reset counter */
		NoisePeriodCount.FixedPoint.L = 0;
	}

	NoisePeriod = Period;
}

unsigned short GetMixedOutputForChannel(int ChannelIndex)
{
	const unsigned char Mixer = PSGPlay_Registers[7];
	unsigned short Mixer_ToneOutput = 0x0ffff;
	unsigned short Mixer_NoiseOutput = 0x0ffff;
	unsigned short ChannelToneOutput;
	unsigned short ChannelNoiseOutput;

	if ((Mixer & (1 << ChannelIndex)) == 0)
	{
		Mixer_ToneOutput = 0x0;
	}

	if ((Mixer & (1 << (ChannelIndex + 3))) == 0)
	{
		Mixer_NoiseOutput = 0x0;
	}

	ChannelToneOutput = ChannelPeriods[ChannelIndex].WaveFormState | Mixer_ToneOutput;

	/*Noise ?*/
	ChannelNoiseOutput = NoiseOutput | Mixer_NoiseOutput;

	if (!ChannelEnabled[ChannelIndex])
	{
		ChannelToneOutput = 0x0ffff;
	}
	if (!NoiseEnabled)
	{
		ChannelNoiseOutput = 0x0ffff;
	}
	return ChannelToneOutput & ChannelNoiseOutput;
}


unsigned short GetFinalVolumeForChannel(int ChannelIndex)
{
	unsigned short ChannelOutput;
	int ChannelOutputVolume8b;
	unsigned char ChannelVolume;
	unsigned short ChannelOutputVolume;

	/* get programmed volume */
	ChannelVolume = PSGPlay_Registers[8 + ChannelIndex];

	/* get state of waveform and mix noise*/
	ChannelOutput = GetMixedOutputForChannel(ChannelIndex);

	/* if bit is set, envelope takes control */
	if ((ChannelVolume & 0x010) != 0)
	{
		/* debug, allow hardware envelope to be disabled */
		if (HardwareEnvelopeEnabled)
		{
			/* envelope controls volume */
			ChannelOutputVolume8b = EnvelopeVolume;
		}
		else
		{
			/* force volume */
			ChannelOutputVolume8b = 0x0f;
		}
	}
	else
	{
		/* volume channel controls volume; it's the lower 4 bits only */
		ChannelOutputVolume8b = ChannelVolume & 0x0f;
	}

	/*convert ChannelOutputVolume to 16 value using amplitude table */
	ChannelOutputVolume = Amplitudes_AY[ChannelOutputVolume8b];

	/* if channel output is 0, then volume will be zero */
	ChannelOutputVolume = ChannelOutputVolume & ChannelOutput;/*NEED TO CHECK, POSSIBLE ERROR DURING 8bit> 16bit conversion. */

															  /* return volume for that channel */
															  /* the response is not linear, this converts from linear volume to output response */
															  /*return AY_VolumeTranslation[ChannelOutputVolume]; */
	if (!ChannelEnabled[ChannelIndex])
	{
		ChannelOutputVolume = 0;
	}
	return ChannelOutputVolume;
}


void PSG_UpdateChannels(PSG_OUTPUT *pOutput, FIXED_POINT16 *pPeriodUpdate)
{
	/* update envelope */
	Envelope_Update(pPeriodUpdate);

	UpdateNoise(pPeriodUpdate);

	/* update tones */
	UpdateChannelToneState(0, pPeriodUpdate);
	UpdateChannelToneState(1, pPeriodUpdate);
	UpdateChannelToneState(2, pPeriodUpdate);

	pOutput->A = GetFinalVolumeForChannel(0);
	pOutput->B = GetFinalVolumeForChannel(1);
	pOutput->C = GetFinalVolumeForChannel(2);
}


void PSGPlay_Initialise(void)
{
	/*	PSG_InitialiseEnvelopeShapes(); */
}

void PSGPlay_Write(int Register, int Data)
{
	/*	AudioEvent_AddEventToBuffer(AUDIO_EVENT_PSG, Register, Data); */
	PSG_UpdateState(Register, Data);
}
