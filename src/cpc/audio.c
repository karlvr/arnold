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
#include "psgplay.h"
#include "audio.h"
//#include "digiblaster.h"


extern BOOL Host_LockAudioBuffer(unsigned char **ppBlock1, unsigned long *pBlock1Size, unsigned char **ppBlock2, unsigned long *pBlock2Size, int BlockSize);
extern void Host_UnLockAudioBuffer(void);
extern void Host_ClearAudioBuffer(void);
extern void Digiblaster_Reset(int);
extern BOOL DigiblasterEnabled();
BOOL bAudioIsSigned = FALSE;
BOOL bIsBigEndian = FALSE;
extern unsigned short VolumeDigiBlaster(void);

/*static*/ unsigned char *pAudioBufferBase;
static unsigned char *pAudioBufferPtr;

/* sample rate */
static unsigned long SampleRate;
/* bits per sample */
static int BitsPerSample;
/* no of channels */
static int NoOfChannels;
/*Buffer size max*/
int AudioBufferSize;


/*buffer for sound */
unsigned short LeftBuffer = 0;
unsigned short RightBuffer = 0;

static FIXED_POINT16 fSamplesPerNop;
static FIXED_POINT16 fPSGEventsPerNop;

static FIXED_POINT16 fSample;

void Audio_Init(int newFrequency, int newBitsPerSample, int newNoOfChannels, BOOL newIsSigned, BOOL newIsBigEndian)
{

	float SamplesPerScreen;
	int BytesPerSample = ((newBitsPerSample*newNoOfChannels) + 7) >> 3;
	float ScreenRefreshFrequency;
	float SamplesPerNop;
	float PSGEventsPerNOP;
	float NopsPerSecond;
	SampleRate = newFrequency;
	BitsPerSample = newBitsPerSample;
	NoOfChannels = newNoOfChannels;

	bAudioIsSigned = newIsSigned;
	bIsBigEndian = newIsBigEndian;

	printf("Doing Audio init\n");

	ScreenRefreshFrequency = 50.00f;

	NopsPerSecond = NOPS_PER_FRAME * ScreenRefreshFrequency;
	/* no of samples per screen refresh */
	SamplesPerScreen = (float)SampleRate / (float)ScreenRefreshFrequency;

	/* calculate size of sample buffer; allowing for an extra screen time just in case */
	AudioBufferSize = (int)(SamplesPerScreen * BytesPerSample * 2.0f);

	pAudioBufferBase = (unsigned char *)malloc(AudioBufferSize);
	pAudioBufferPtr = pAudioBufferBase;

	/* samples per nop */
	SamplesPerNop = (float)SamplesPerScreen / (float)NOPS_PER_FRAME;

	fSamplesPerNop.FixedPoint.L = (int)(SamplesPerNop*65536.0f);

	PSGEventsPerNOP = (float)(PSG_CLOCK_FREQUENCY / (float)(NopsPerSecond * 8));
	fPSGEventsPerNop.FixedPoint.L = (int)(PSGEventsPerNOP*65536.0f);

	/* clear all buffer */
	Audio_Reset();

}


void CPC_Stereo_Mixer(PSG_OUTPUT *PSG_Output, unsigned short *LeftVolume, unsigned short *RightVolume)
{
	/* according to the specifications B channel is mixed with A and C, but not exactly half */

	// channel a and c are reduced by 10k
	// channel b is reduced by 22k
	// B is noticeably quieter on plus. say 2/3 
	// max 3723
	unsigned short MiddleChannel = (((unsigned long)PSG_Output->B * 10) / 22) & 0x0ffff;
	// max: 8192+3723=11915
	unsigned long Left = ((unsigned long)PSG_Output->A + (unsigned long)MiddleChannel);
	unsigned long Right = ((unsigned long)PSG_Output->C + (unsigned long)MiddleChannel);

	// clip
	if (Left > 65535)
	{
		Left = 65535;
	}
	if (Right > 65535)
	{
		Right = 65535;
	}

	/* resulting could be as much as 64 */
	*LeftVolume = (Left & 0x0ffff);
	*RightVolume = (Right & 0x0ffff);
}

void CPC_Mono_Mixer(PSG_OUTPUT *PSG_Output, unsigned short *LeftVolume, unsigned short *RightVolume)
{
	// all are reduced by an amount before being added together.
	unsigned long Volume = ((unsigned long)PSG_Output->A + (unsigned long)PSG_Output->B + (unsigned long)PSG_Output->C) / 3;
	if (Volume > 65535)
	{
		Volume = 65535;
	}
	*LeftVolume = (Volume & 0x0ffff);
	*RightVolume = (Volume & 0x0ffff);
}

void CPC_Mono_Speaker_Mixer(PSG_OUTPUT *PSG_Output, unsigned short *LeftVolume, unsigned short *RightVolume)
{
	// all are reduced by an amount before being added together.
	unsigned long Volume = ((unsigned long)PSG_Output->A + (unsigned long)PSG_Output->B + (unsigned long)PSG_Output->C) / 3;

	Volume = (Volume * CPC_GetSpeakerVolume()) / SPEAKER_VOLUME_MAX;

	if (Volume > 65535)
	{
		Volume = 65535;
	}
	*LeftVolume = (Volume & 0x0ffff);
	*RightVolume = (Volume & 0x0ffff);
}



void UpdateSoundBuffer(void) {

	/*unsigned short VolDigiblaster; */


	// DOESN'T HANDLE BIG ENDIAN

	//stereo
	if (NoOfChannels == 2)
	{
		//8bit
		if (BitsPerSample == 8) {
			//			long pos;

			//16 bit > 8 bit convertion
			LeftBuffer = LeftBuffer >> 8;
			RightBuffer = RightBuffer >> 8;

			if (bAudioIsSigned)
			{
				//0x00=>0xff to 0x80 => 0xff convertion for window compatibility in 8 bit
				LeftBuffer = LeftBuffer + 0x080;
				RightBuffer = RightBuffer + 0x080;
			}

			// data is left then right
			pAudioBufferPtr[0] = (unsigned char)(LeftBuffer & 0xff);
			pAudioBufferPtr[1] = (unsigned char)(RightBuffer & 0xff);
			pAudioBufferPtr = (pAudioBufferPtr + 2);
		}
		//16bit
		else
		{
			//0x0000=>0xFFFF to 0x0000=>0x7FFF convertion
			if (bAudioIsSigned)
			{
				LeftBuffer = (LeftBuffer + 0x08000);
				RightBuffer = (RightBuffer + 0x08000);
			}
			if (!bIsBigEndian)
			{
				pAudioBufferPtr[0] = (unsigned char)(LeftBuffer & 0xff);
				pAudioBufferPtr[1] = (unsigned char)((LeftBuffer >> 8) & 0xff);
				pAudioBufferPtr[2] = (unsigned char)(RightBuffer & 0xff);
				pAudioBufferPtr[3] = (unsigned char)((RightBuffer >> 8) & 0xff);
			}
			else
			{
				pAudioBufferPtr[1] = (unsigned char)(LeftBuffer & 0xff);
				pAudioBufferPtr[0] = (unsigned char)((LeftBuffer >> 8) & 0xff);
				pAudioBufferPtr[3] = (unsigned char)(RightBuffer & 0xff);
				pAudioBufferPtr[2] = (unsigned char)((RightBuffer >> 8) & 0xff);
			}
			pAudioBufferPtr = (pAudioBufferPtr + 4);
		}
	}
	//Mono
	else
	{
		// half each or half the result?
		unsigned long MixedValue = (LeftBuffer + RightBuffer) >> 1;
		if (MixedValue > 65535)
		{
			MixedValue = 65535;
		}
		//8bit
		if (BitsPerSample == 8) {

			//16bit > 8 bit convertion
			unsigned char MixedValue8 = MixedValue >> 8;

			if (bAudioIsSigned)
			{
				//0x00=>0xff to 0x80 => 0xff convertion for window compatibility in 8 bit
				MixedValue8 = MixedValue8 + 0x80;
			}

			pAudioBufferPtr[0] = (unsigned char)(MixedValue8 & 0xff);
			pAudioBufferPtr = (pAudioBufferPtr + 1);
		}
		//16bit
		else
		{
			if (bAudioIsSigned)
			{
				MixedValue = (MixedValue + 0x08000);
			}

			if (!bIsBigEndian)
			{
				pAudioBufferPtr[0] = (unsigned char)(MixedValue & 0xff);
				pAudioBufferPtr[1] = (unsigned char)((MixedValue >> 8) & 0xff);
			}
			else
			{
				pAudioBufferPtr[1] = (unsigned char)(MixedValue & 0xff);
				pAudioBufferPtr[2] = (unsigned char)((MixedValue >> 8) & 0xff);
			}
			pAudioBufferPtr = (pAudioBufferPtr + 2);
		}
	}
}


void Audio_Update(int NopCycles)
{
	// this is the current position through the current sample
	const unsigned long PrevFraction = fSample.FixedPoint.W.Fraction;
	// this is the amount of time until this sample ends.
	const unsigned long PrevFractionRemaining = (0x010000 - PrevFraction);
	unsigned short LeftChannel, RightChannel;
	CPC_AUDIO_OUTPUT_TYPE nOutput;

	// current volume set
	//	const unsigned char DigiblasterVolume = Printer_GetDataByte();	//Digiblaster;	//sCPC_GetTapeVolume();

	// number of samples we are generating this time
	FIXED_POINT16 fSamples;
	FIXED_POINT16 fPSGEvents;
	PSG_OUTPUT PSGOutput;
	unsigned short FinalVolumeL, FinalVolumeR;

	fSamples.FixedPoint.L = (fSamplesPerNop.FixedPoint.L * NopCycles);
	fPSGEvents.FixedPoint.L = (fPSGEventsPerNop.FixedPoint.L * NopCycles);

	PSG_InitialiseToneUpdates(&fPSGEvents);

	/* update channels and get raw volumes */
	PSG_UpdateChannels(&PSGOutput, &fPSGEvents);

	nOutput = Audio_GetOutput();
	if (CPC_GetHardware() == CPC_HW_ALESTE)
	{
		/* no stereo on aleste; always mono and expansion */
		nOutput = CPC_AUDIO_OUTPUT_MONO_EXPANSION;
	}
	else if ((CPC_GetHardware() == CPC_HW_CPCPLUS) || (CPC_GetHardware() == CPC_HW_KCCOMPACT))
	{
		/* no internal speaker on kc compact or plus */
		if (nOutput == CPC_AUDIO_OUTPUT_MONO_SPEAKER)
		{
			nOutput = CPC_AUDIO_OUTPUT_MONO_EXPANSION;
		}
	}


	switch (nOutput)
	{

	case CPC_AUDIO_OUTPUT_MONO_EXPANSION: // expansion
	{
		/* mix it according to mono output on expansion port */
		CPC_Mono_Mixer(&PSGOutput, &LeftChannel, &RightChannel);
	}
	break;
	case CPC_AUDIO_OUTPUT_MONO_SPEAKER: // internal speaker
	{
		/* mix it according to mono output on expansion port */
		CPC_Mono_Speaker_Mixer(&PSGOutput, &LeftChannel, &RightChannel);
	}
	break;
	// audio jack
	case CPC_AUDIO_OUTPUT_STEREO:
	{
		CPC_Stereo_Mixer(&PSGOutput, &LeftChannel, &RightChannel);
	}
	break;

	default:
	{
		LeftChannel = RightChannel = 0;
	}
	break;
	}
	FinalVolumeL = LeftChannel;
	FinalVolumeR = RightChannel;

#if 0
	/* calc final volume */
	FinalVolumeL = LeftChannel << 8;
	FinalVolumeR = RightChannel << 8;
#endif

	if (DigiblasterEnabled())
	{
		unsigned short VolDigiBlaster = VolumeDigiBlaster();
		FinalVolumeL = FinalVolumeL / 2 + VolDigiBlaster * 257 / 2;
		FinalVolumeR = FinalVolumeR / 2 + VolDigiBlaster * 257 / 2;
	}

	// add this sample's value weighting it accordingly.
	if (fSamples.FixedPoint.L >= PrevFractionRemaining)
	{
		int nSample;

		LeftBuffer += ((FinalVolumeL*PrevFractionRemaining) >> 16);
		RightBuffer += ((FinalVolumeR*PrevFractionRemaining) >> 16);

		UpdateSoundBuffer();

		fSamples.FixedPoint.L -= PrevFractionRemaining;

		for (nSample = 0; nSample<(fSamples.FixedPoint.W.Int); ++nSample)
		{
			LeftBuffer = FinalVolumeL;
			RightBuffer = FinalVolumeR;

			UpdateSoundBuffer();
		}

		LeftBuffer = 0;
		RightBuffer = 0;

		if ((fSamples.FixedPoint.W.Fraction) != 0)
		{
			LeftBuffer += ((FinalVolumeL*fSamples.FixedPoint.W.Fraction) >> 16);
			RightBuffer += ((FinalVolumeR*fSamples.FixedPoint.W.Fraction) >> 16);
		}
		fSample.FixedPoint.L = fSamples.FixedPoint.W.Fraction;
	}
	else
	{
		LeftBuffer += ((FinalVolumeL*fSamples.FixedPoint.L) >> 16);
		RightBuffer += ((FinalVolumeR*fSamples.FixedPoint.L) >> 16);

		// add on and continue
		fSample.FixedPoint.L += fSamples.FixedPoint.L;
		fSample.FixedPoint.W.Int = 0;
	}
}

void Audio_Reset(void)
{
	Digiblaster_Reset(BitsPerSample);

	/* Re-initialise buffer */
	if (pAudioBufferBase != NULL)
	{

		if (BitsPerSample == 8)
		{
			memset(pAudioBufferBase, bAudioIsSigned ? 0x080 : 0x00, AudioBufferSize);//8 bit
		}
		else
		{
			int i;
			unsigned short Sample = 0x0000;
			unsigned short *pAudioBuffer = (unsigned short *)pAudioBufferBase;
			if (bAudioIsSigned)
			{
				Sample = 0x08000;
			}
			for (i = 0; i < AudioBufferSize >> 1; i++)
			{
				pAudioBuffer[i] = Sample;
			}
		}

	}

	//Reset PSG
	PSGPlay_Reset();

	Reset_Audio_Event();

	//Clear hardware buffer
	Host_ClearAudioBuffer();

}

void Reset_Audio_Event(void)
{
	fSample.FixedPoint.L = 0;
}


void Audio_Finish()
{
	if (pAudioBufferBase) free(pAudioBufferBase);
}

long GetLenghtBuff(void)
{
	return pAudioBufferPtr - pAudioBufferBase;
}

void CopyBuff(unsigned char **pAudio1, unsigned long AudioBlock1Size, unsigned char **pAudio2, unsigned long AudioBlock2Size)
{
	//if (pAudioBufferBase!=NULL) memset(pAudioBufferBase, 0x080, AudioBufferSize);

	memcpy(*pAudio1, pAudioBufferBase, AudioBlock1Size);

	if (*pAudio2 != NULL)
	{
		memcpy(*pAudio2, pAudioBufferBase + AudioBlock1Size, AudioBlock2Size);
	}
}

void UpdateBuf(void) {
	//	/* remember remainder of sample we got to for next audio write */
	//	fSample.FixedPoint.W.Int = 0;

	/* now copy sample we are building to front of buffer ready for next time */

	const int BytesPerSample = (BitsPerSample*NoOfChannels) >> 3;

	// copy current sample being built.
	memcpy(pAudioBufferBase, pAudioBufferPtr, BytesPerSample);

#if 0
	//clear the rest
	memset(pAudioBufferBase + BytesPerSample, 0x000, AudioBufferSize - BytesPerSample);
	if (BitsPerSample == 8) {
		memset(pAudioBufferBase + BytesPerSample, 0x80, AudioBufferSize - BytesPerSample);//8 bit
	}
	else
	{
		memset(pAudioBufferBase + BytesPerSample, 0x00, AudioBufferSize - BytesPerSample);;//16 bit
	}
#endif
	//update pointer
	pAudioBufferPtr = pAudioBufferBase;
}

void Audio_Commit(void)
{

	unsigned long Length;

	/* calc length in bytes of complete number of samples */
	Length = GetLenghtBuff();

	if (Length != 0)
	{
		unsigned char *pAudio1;
		unsigned char *pAudio2;
		unsigned long AudioBlock1Size;
		unsigned long AudioBlock2Size;

		/* lock audio buffer for write */
		/* we may get back to two buffers if it wraps past the end */
		if (Host_LockAudioBuffer(&pAudio1, &AudioBlock1Size, &pAudio2, &AudioBlock2Size, Length))
		{
			/* copy generated buffer to write buffer */
			CopyBuff(&pAudio1, AudioBlock1Size, &pAudio2, AudioBlock2Size);

			/* unlock */
			Host_UnLockAudioBuffer();
		}
	}

	UpdateBuf();
}
