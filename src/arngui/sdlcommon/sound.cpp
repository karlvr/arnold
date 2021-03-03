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
/* CLICK is NOT caused by silencing audio buffer */
/* CLICK is NOT caused by invalid register settings for psg play or psg */

#include "sound.h"
#include <stdio.h>
#include <stdio.h>
#include <sys/types.h>
#include <memory.h>
#include <math.h>
#include "../../cpc/cpcglob.h"
#include "../arnguiApp.h"

#ifdef _MSC_VER
// both sdl and sdl2
#include <SDL.h>
#else
// choose sdl or sdl2 directory
#ifdef USE_SDL
#include <SDL/SDL.h>
#endif
#ifdef USE_SDL2
#include <SDL.h>
#endif
#endif
bool SDLCommon::m_bHasAudio = false;
bool SDLCommon::m_bHasOpenedAudio = false;
bool SDLCommon::m_bIsResuming = false;
bool SDLCommon::m_bIsPaused = false;
int SDLCommon::m_nPauseTimer = 0;
unsigned long  SDLCommon::m_TicksAudioStart = 0;
unsigned long SDLCommon::m_LockBufferTicks = 0;
bool SDLCommon::m_bIsFirstBufferWriteSinceStart = true;
bool SDLCommon::m_bIsFirstBufferLockSinceStart = true;
bool SDLCommon::m_bIsFirstBufferUnLockSinceStart = true;

//#define AUDIO_WATERMARK 2048	// FIXME

int SDLCommon::m_nActualFrequency = 0;
int SDLCommon::m_nActualBitsPerSample = 0;
int SDLCommon::m_nActualChannels = 0;

static uint64_t samples_to_date = 0;
static int audio_bufsize;
static Uint8 *audio_chunk;
static Uint32 audio_len;
static Uint8 *audio_pos;
static Uint8 *audio_pos_last;
static Uint8 *audio_rec;
//static const int audio_callbacksize = AUDIO_WATERMARK/2;
SDL_AudioSpec obtained;

extern "C" {
	void Audio_Init(int newFrequency, int newBitsPerSample, int newNoOfChannels, BOOL newIsSigned, BOOL newIsBigEndian);
	long GetLenghtBuff(void);
	void CopyBuff(unsigned char **pAudio1, unsigned long AudioBlock1Size,unsigned char **pAudio2, unsigned long AudioBlock2Size);
    void UpdateBuf(void);
//	void Digiblaster_EndFrame(void);
//	void AddDigiblasterVolume(void);

}


void sdl_fill_audio(void *userdata, Uint8 *stream, int len);
bool sdl_LockAudioBuffer(unsigned char **pBlock1, unsigned long *pBlock1Size, unsigned char **pBlock2, unsigned long *pBlock2Size, int AudioBufferSize);
void sdl_UnLockAudioBuffer(void);
void AddDigiblasterVolume(void);

bool SDLCommon::Init()
{
    m_bHasAudio = false;
    // initialise audio
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0 )
    {
         wxGetApp().Log("SDL audio did not initialise\n");
    }
    else
    {
        m_bHasAudio = true;
         wxGetApp().Log("SDL audio initialised\n");
    }

    return m_bHasAudio;
}

void SDLCommon::Shutdown()
{
	if (audio_chunk!=NULL)
	{
		free(audio_chunk);
		audio_chunk = NULL;
	}
   if (m_bHasOpenedAudio)
    {
        m_bHasOpenedAudio = false;
         wxGetApp().Log("Shutting down audio\n");
        SDL_LockAudio();
        SDL_CloseAudio();
    }


}

void SDLCommon::UpdatePauseTimer(int Cycles)
{
	if (m_nPauseTimer < Cycles)
	{
		m_nPauseTimer = 0;
		m_bIsResuming = false;

		// resume audio
// TODO: Fix
//		SDL_PauseAudio(0);

		SilenceAudio();
	}
	else
	{
		m_nPauseTimer -= Cycles;
	}
}

void SDLCommon::ResetPauseTimer()
{
	m_bIsResuming = true;
	m_nPauseTimer = 19968*3;
}

bool SDLCommon::IsResuming()
{
	return m_bIsResuming;
}

void SDLCommon::SilenceAudio()
{
	if (audio_chunk)
	{
		memset(audio_chunk, 0, audio_bufsize);
	}
}

void SDLCommon::PauseSound(bool state)
{
	if (state == m_bIsPaused)
		return;

	if (state)
	{
		SDL_PauseAudio(1);
		SilenceAudio();

		m_bIsPaused = true;
	}
	else
	{
		/* if we are restoring sound we need to wait for a short time before doing it.
		if we stop too quickly then the sound will pop and crackle. */
		ResetPauseTimer();

	}
}


int totallen = 0;
int numtotallen = 0;
int avglen = 0;
int minlen = 1<<30;
int maxlen = 0;

void SDLCommon::ResumeAudio()
{
	SDL_PauseAudio(0);
	m_TicksAudioStart = SDL_GetTicks();
}

bool SDLCommon::InitAudio(int freq,int bit,int chan) {

	 wxGetApp().Log("SDLCommon\n");
	m_bHasOpenedAudio = false;

	SDL_AudioSpec desired;

	/* reset stats when we change the freq etc */
	totallen = 0;
	numtotallen = 0;
	avglen = 0;
	minlen = 1<<30;
	maxlen = 0;

	memset(&desired, 0, sizeof(desired));

	desired.freq = freq;
	if (bit == 16)
	{
		desired.format = AUDIO_S16SYS;
	}
	else
	{
		desired.format = AUDIO_S8;
	}

	float nframes = 1.5f;
	int nSamplesPerFrame = (int)(ceil(freq / 50.0f)*nframes);
	if (nSamplesPerFrame < 256)
	{
		nSamplesPerFrame = 256;
	}
	else if (nSamplesPerFrame < 512)
	{
		nSamplesPerFrame = 512;
	}
	else if (nSamplesPerFrame < 1024)
	{
		nSamplesPerFrame = 1024;
	}
	else if (nSamplesPerFrame < 2048)
	{
		nSamplesPerFrame = 2048;
	}
	else if (nSamplesPerFrame < 4096)
	{
		nSamplesPerFrame = 4096;
	}
	else if (nSamplesPerFrame < 8192)
	{
		nSamplesPerFrame = 8192;
	}


	int bufferSizeRequiredBytes = (nSamplesPerFrame*chan*bit)>>3;
	 wxGetApp().Log("Want to allocate %d bytes\n", bufferSizeRequiredBytes);
	audio_bufsize = bufferSizeRequiredBytes;

	desired.channels = chan;
	desired.samples = freq / 50.0f;
	desired.callback = sdl_fill_audio;      // this function is called each time SDL wants us to update the buffer
	desired.userdata = NULL;

	memset(&obtained, 0, sizeof(obtained));
	 wxGetApp().Log("Desired samples: %d\n", desired.samples);
	 wxGetApp().Log("Desired Audio Frequency: %d\n", desired.freq);
     wxGetApp().Log("Desired Audio Channels: %d\n", desired.channels);
     wxGetApp().Log("Desired Audio Format: %0x\n", desired.format);
	
	if (SDL_OpenAudio(&desired, &obtained)==-1)
    {
       wxGetApp().Log("Failed to open audio\n");
	  return false;
    }
    else
    {
       wxGetApp().Log("Successfully opened audio\n");
	   wxGetApp().Log("Obtained samples: %d\n", obtained.samples);
	   wxGetApp().Log("Obtained Audio Frequency: %d\n", obtained.freq);
       wxGetApp().Log("Obtained Audio Channels: %d\n", obtained.channels);
       wxGetApp().Log("Obtained Audio Format: %0x\n", obtained.format);
	
	    m_nActualFrequency = obtained.freq;
	
	    m_nActualChannels = obtained.channels;

#ifdef USE_SDL2
	   wxGetApp().Log("%d bits per sample\n", (int)SDL_AUDIO_BITSIZE(obtained.format));
	   wxGetApp().Log("is floating point %s\n", SDL_AUDIO_ISFLOAT(obtained.format) ? "yes" : "no");
	   wxGetApp().Log("is int %s\n", SDL_AUDIO_ISINT(obtained.format) ? "yes" : "no");
	   wxGetApp().Log("is big endian %s\n", SDL_AUDIO_ISBIGENDIAN(obtained.format) ? "yes" : "no");
	   wxGetApp().Log("is signed %s\n", SDL_AUDIO_ISSIGNED(obtained.format) ? "yes" : "no");
		
	    m_nActualBitsPerSample = SDL_AUDIO_BITSIZE(obtained.format);

	    #endif

      if (audio_chunk != NULL)
      {
        free(audio_chunk);
      }
	   wxGetApp().Log("Buffer size allocated in bytes: %d\n", audio_bufsize);
      audio_chunk = (Uint8 *)malloc(audio_bufsize);
      if (audio_chunk == NULL)
      {
         wxGetApp().Log("Failed to allocate buffer for audio\n");
		return false;
      }
	  SilenceAudio();
      audio_pos = audio_chunk;
	  audio_pos_last = audio_pos;
      audio_rec = audio_chunk;
      audio_len = audio_bufsize;



#ifdef USE_SDL2
	  Audio_Init(obtained.freq, SDL_AUDIO_BITSIZE(obtained.format), obtained.channels, SDL_AUDIO_ISSIGNED(obtained.format), SDL_AUDIO_ISBIGENDIAN(obtained.format) ? TRUE : FALSE);
#else
	int bitsPerSample = 8;
      BOOL bIsSigned = FALSE;
	  BOOL bIsBigEndian = FALSE;
      switch (obtained.format)
      {
	      case AUDIO_U8:
	      {
		bitsPerSample = 8;
		bIsSigned = FALSE;
		       wxGetApp().Log("8 bit unsigned\n");
	      }
	      break;
	            case AUDIO_S8:
	      {
		bitsPerSample = 8;
		bIsSigned = TRUE;
		       wxGetApp().Log("8 bit signed\n");
	      }
	      break;
	            case AUDIO_U16LSB:
	      {
		bitsPerSample = 16;
		bIsSigned = FALSE;
		bIsBigEndian = FALSE;
		       wxGetApp().Log("16 bit unsigned (little endian)\n");
	      }
	      break;
				case AUDIO_U16MSB:
				{
					bitsPerSample = 16;
					bIsSigned = FALSE;
					bIsBigEndian = TRUE;
					 wxGetApp().Log("16 bit unsigned (big endian)\n");
				}
				break;

	            case AUDIO_S16LSB:
	      {
		bitsPerSample = 16;
		bIsSigned = TRUE;
		bIsBigEndian = FALSE;
		       wxGetApp().Log("16 bit signed (little endian)\n");
	      }
	      break;
				case AUDIO_S16MSB:
				{
					bitsPerSample = 16;
					bIsSigned = TRUE;
					bIsBigEndian = TRUE;
					 wxGetApp().Log("16 bit signed (big endian)\n");
	}
				break;
	      default:
		       wxGetApp().Log("Format %d not handled\n", obtained.format);
			break;
      }
      	    m_nActualBitsPerSample = bitsPerSample;

	  Audio_Init(obtained.freq, bitsPerSample, obtained.channels, bIsSigned, bIsBigEndian);
#endif
      }
	   wxGetApp().Log("Number of samples in buffer: %f\n", (float)((audio_bufsize * 8) / (float)m_nActualBitsPerSample));

	m_bHasOpenedAudio = true;

	return true;
}

void	*halfcpy(void *dest, const void *src, size_t n) {

	Uint8 *d = (Uint8 *) dest;
	Uint8 *s = (Uint8 *) src;
	while(n-- > 0) {
		*d++ = *s++/2;
	}
	return dest;
}

int counter = 0;

/* len is the number of bytes not samples! */
void sdl_fill_audio(void *userdata, Uint8 *stream, int len)
{
#if 0
	if (SDLCommon::m_bIsFirstBufferWriteSinceStart)
	{
		/* how many milliseconds have passed since audio was started? */
		Uint32 CurrentTicks = SDL_GetTicks();
		unsigned long NumTicksSinceAudioStart = CurrentTicks - SDLCommon::m_TicksAudioStart;
		float SamplesPerMillisecond = SDLCommon::m_nActualFrequency / 1000.0f;

		 wxGetApp().Log("first audio write: Ticks: %d Samples passed: %f\n", NumTicksSinceAudioStart, NumTicksSinceAudioStart*SamplesPerMillisecond);
		
		SDLCommon::m_bIsFirstBufferWriteSinceStart = false;
	}
#endif

    if (len==0) return;

	if (len < minlen)
	{
		minlen = len;
	}
	if (len > maxlen)
	{
		maxlen = len;
	}

	/* what buffer size are we seeing? */
	totallen += len;
	numtotallen++;
	avglen = totallen / numtotallen;

#if 1
	if ((stream<audio_chunk) || (stream>(audio_chunk + audio_bufsize)))
	{
		 wxGetApp().Log("stream position outside of our buffer\n");
	}

	Uint8 *audioend= audio_chunk + audio_bufsize;
	if (stream < audioend)
	{
		int lenremaining = audioend - stream;
		if (len > lenremaining)
		{
			 wxGetApp().Log("len is longer than lenremaining\n");
		}
	}
#endif

	// stream is not related to our audio buffer
	// min, max and avg on windows are the same, they are the size we requested
	
	counter++;
	if (counter > 100)
	{
		// wxGetApp().Log("min: %d max: %d avg: %d\n", minlen, maxlen, avglen);
		counter = 0;
	}
	
	if (len > audio_bufsize)
	{
		// wxGetApp().Log("increase audio buffer size! wanted: %d defined: %d\n", len, audio_bufsize);
	}
	
	if (audio_pos != audio_pos_last)
	{
		// wxGetApp().Log("check audio_pos ptr\n");
	}

	/* is there space to write entirely? */
	int size = 0;
	if ( (audio_pos + len) < (audio_chunk + audio_bufsize) )
	{
		memcpy(stream, audio_pos, len);
		size = len;
		audio_pos += len;		
	}
	else
	{
		int begin;

		int remain;
		remain = (audio_chunk + audio_bufsize) - audio_pos;
		size += remain;
		memcpy(stream, audio_pos, remain);

		begin = len - remain;
		if (begin != 0)
		{
			/* wrap around */
			size += begin;
			memcpy(stream + remain, audio_chunk, begin);
		}
		
		audio_pos = audio_chunk + begin;
		
	}
	if (size != len)
	{
		// wxGetApp().Log("something went wrong with copy\n");
	}
	audio_pos_last = audio_pos;

	samples_to_date += len;

}


bool sdl_LockAudioBuffer(unsigned char **pBlock1, unsigned long *pBlock1Size, unsigned char **pBlock2, unsigned long *pBlock2Size, int AudioBufferSize)
{
	int remain;


	if (audio_pos >= audio_rec)
	{
		//printf("audio playback is overtaking recording position. This means playback is faster than recording...\n");
	}

	remain = audio_bufsize - (audio_rec - audio_chunk);

	if (remain > AudioBufferSize)
	{
		if ((audio_pos > audio_rec) && (audio_pos < audio_rec + AudioBufferSize))
		{
		//	printf("audio playback is overtaking recording position. This means playback is faster than recording...\n");
		//	printf("Buffer from %ld to %ld et pos = %ld\n", audio_rec - audio_chunk, audio_rec + AudioBufferSize - audio_chunk, audio_pos - audio_chunk);
			return false;
		}
	}
	else
	{
		if ((audio_pos > audio_rec) || (audio_pos < audio_chunk + AudioBufferSize - remain))
		{
			//printf("audio playback is overtaking recording position. This means playback is faster than recording...\n");
			//printf("Buffer from %ld to %ld et pos = %ld\n", audio_rec - audio_chunk, remain, audio_pos - audio_chunk);
			return false;
		}
	}

	SDL_LockAudio();

	if (remain > AudioBufferSize)
	{

		/* fits within remainder of buffer */
		*pBlock1 = audio_rec;
		*pBlock1Size = AudioBufferSize;
		*pBlock2 = NULL;
		*pBlock2Size = 0;
		audio_rec += AudioBufferSize;
	}
	else
	{
		/* to the end of the buffer, or wraps to the end */
		*pBlock1 = audio_rec;
		*pBlock1Size = remain;

		if (remain != AudioBufferSize)
		{
			*pBlock2 = audio_chunk;
			*pBlock2Size = AudioBufferSize - remain;

			audio_rec = audio_chunk + *pBlock2Size;
		}
		else
		{
			/* full to end of buffer */
			*pBlock2 = NULL;
			*pBlock2Size = 0;
			audio_rec = audio_chunk;
		}
	}

	return true;
}

void sdl_UnLockAudioBuffer(void)
{
	SDL_UnlockAudio();

	Uint32 CurrentTicks = SDL_GetTicks();
#if 0
	if (SDLCommon::m_bIsFirstBufferUnLockSinceStart)
	{
		unsigned long DurationTicks = CurrentTicks = SDLCommon::m_LockBufferTicks;
		float SamplesPerMillisecond = SDLCommon::m_nActualFrequency / 1000.0f;
		 wxGetApp().Log("ticks to fill buffer: %d Samples passed to fill buffer: %f\n", DurationTicks, DurationTicks*SamplesPerMillisecond);
	}
#endif
	if (SDLCommon::m_bIsFirstBufferUnLockSinceStart)
	{
		SDLCommon::ResumeAudio();

#if 0
		unsigned long DurationTicks = CurrentTicks = SDLCommon::m_TicksAudioStart;
		float SamplesPerMillisecond = SDLCommon::m_nActualFrequency / 1000.0f;
		 wxGetApp().Log("ticks to unlock buffer: %d Samples passed to commit of first buffer: %f\n", DurationTicks, DurationTicks*SamplesPerMillisecond);
#endif
	}
	SDLCommon::m_bIsFirstBufferUnLockSinceStart = false;

}

void sdl_close_audio(void)
{
	SDL_CloseAudio();
	if (audio_chunk) free(audio_chunk);
}

#if 0
bool ConvertSound(Uint8 * sounddata,Uint32 soundlength)
{
		SDL_AudioCVT cvt;
		/* Conversion vers le format du tampon audio */
		if (SDL_BuildAudioCVT(&cvt,8, 2, obtained.freq, obtained.format, obtained.channels, obtained.freq) < 0) {
			 wxGetApp().Log("Impossible de construire le convertisseur audio!\n");
			return false;
		}
		/* CrÃ©ation du tampon utilisÃ© pour la conversion */
		cvt.buf = (Uint8*)malloc(soundlength * cvt.len_mult);
		cvt.len = soundlength;
		memcpy(cvt.buf, sounddata, soundlength);

		   /* Conversion... */
		   if (SDL_ConvertAudio(&cvt) != 0) {
			  wxGetApp().Log("Erreur lors de la conversion du fichier audio: %s\n", SDL_GetError());
			 return false;
		   }

	   /* LibÃ©ration de l'ancien tampon, crÃ©ation du nouveau, copie des donnÃ©es converties, effacement du tampon de conversion */
	   SDL_FreeWAV(sounddata);
	   sounddata = (Uint8*)malloc(cvt.len_cvt);
	   memcpy(sounddata, cvt.buf, cvt.len_cvt);
	   free(cvt.buf);

	   soundlength = cvt.len_cvt;
	    wxGetApp().Log("Taille du son converti: %d octets\n", soundlength);

	   return true;
}
#endif


extern "C"
{
	BOOL Host_LockAudioBuffer(unsigned char **ppBlock1, unsigned long *pBlock1Size, unsigned char **ppBlock2, unsigned long *pBlock2Size, int BlockSize)
	{
		return sdl_LockAudioBuffer(ppBlock1, pBlock1Size, ppBlock2, pBlock2Size, BlockSize);
	}

	void	Host_UnLockAudioBuffer(void)
	{
		sdl_UnLockAudioBuffer();
	}

	void Host_ClearAudioBuffer()
	{
	}
}