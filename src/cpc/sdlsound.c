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

#include "sdlsound.h"
#include <stdio.h>
#include <sys/types.h>
#include "cpcglob.h"

#ifdef _MSC_VER
// both sdl and sdl2
#include <SDL.h>
#else
// choose sdl or sdl2 directory
#ifdef USE_SDL
#include <SDL/SDL.h>
#endif
#ifdef USE_SDL2
#include <SDL2/SDL.h>
#endif
#endif

#define AUDIO_WATERMARK 2048	// FIXME

static const int audio_bufsize = 8192;
/*static Uint8 *audio_chunk;
static Uint32 audio_len;
static Uint8 *audio_pos;
static Uint8 *audio_rec;*/
static const int audio_callbacksize = AUDIO_WATERMARK/2;
SDL_AudioSpec obtained;

void Audio_Init(int newFrequency, int newBitsPerSample, int newNoOfChannels);
void sdl_fill_audio(void *userdata, Uint8 *stream, int len);

/*
BOOL Init_SDL_Audio(int freq,int bit,int chan) {

	SDL_AudioSpec desired;

	memset(&desired, 0, sizeof(desired));
	
	desired.freq = freq;
	if (bit == 16)
	{
		desired.format = AUDIO_S16;
	}
	else
	{
		desired.format = AUDIO_S8;
	}
	
	desired.channels = chan;
	desired.samples = audio_callbacksize;
	desired.callback = sdl_fill_audio;
	desired.userdata = NULL;
	
	memset(&obtained, 0, sizeof(obtained));
    //printf("Desired Audio Frequency: %d\n", desired.freq);
    //printf("Desired Audio Channels: %d\n", desired.channels);
    //printf("Desired Audio Format: %0x\n", desired.format);

	if (SDL_OpenAudio(&desired, &obtained)==-1)
    {
      //printf("Failed to open audio\n");
	  return FALSE;
    }
    else
    {
      //printf("Successfully opened audio\n");
      //printf("Obtained Audio Frequency: %d\n", obtained.freq);
      //printf("Obtained Audio Channels: %d\n", obtained.channels);
      //printf("Obtained Audio Format: %0x\n", obtained.format);
      if (audio_chunk != NULL)
      {
        free(audio_chunk);
      }
      audio_chunk = (Uint8 *)malloc(audio_bufsize);
      if (audio_chunk == NULL)
      {
        //printf("Failed to allocate buffer for audio\n");
		return FALSE;
      }
      memset(audio_chunk, 0, audio_bufsize);
      audio_pos = audio_chunk;
      audio_rec = audio_chunk;
      audio_len = audio_bufsize;

      SDL_PauseAudio(0);

	  Audio_Init(obtained.freq, obtained.format & 0xff, obtained.channels);
    }

	return TRUE;
}
*/
/*
void	*halfcpy(void *dest, const void *src, size_t n) {

	Uint8 *d = (Uint8 *) dest;
	Uint8 *s = (Uint8 *) src;
	while(n-- > 0) {
		*d++ = *s++/2;
	}
	return dest;
}
*/
#if 0
BOOL ConvertSound(Uint8 * sounddata,Uint32 soundlength)
{
		SDL_AudioCVT cvt;
		/* Conversion vers le format du tampon audio */
		if (SDL_BuildAudioCVT(&cvt,8, 2, obtained.freq, obtained.format, obtained.channels, obtained.freq) < 0) {
			//printf("Impossible de construire le convertisseur audio!\n");
			return FALSE;
		}
		/* Création du tampon utilisé pour la conversion */
		cvt.buf = (Uint8*)malloc(soundlength * cvt.len_mult);
		cvt.len = soundlength;
		memcpy(cvt.buf, sounddata, soundlength);

		   /* Conversion... */
		   if (SDL_ConvertAudio(&cvt) != 0) {
			 //printf("Erreur lors de la conversion du fichier audio: %s\n", SDL_GetError());
			 return FALSE;
		   }

	   /* Libération de l'ancien tampon, création du nouveau, copie des données converties, effacement du tampon de conversion */
	   SDL_FreeWAV(sounddata);
	   sounddata = (Uint8*)malloc(cvt.len_cvt);
	   memcpy(sounddata, cvt.buf, cvt.len_cvt);
	   free(cvt.buf);

	   soundlength = cvt.len_cvt;
	   //printf("Taille du son converti: %d octets\n", soundlength);

	   return TRUE;
}
#endif

/*
void sdl_fill_audio(void *userdata, Uint8 *stream, int len) {

	//memset(stream, 0, len);

	if ( audio_pos + len < audio_chunk + audio_bufsize )
	{
		memcpy(stream, audio_pos, len);
		//halfcpy(stream, audio_pos, len);
		
		audio_pos += len;
		
	}
	else
	{
		int remain;
		remain = (audio_chunk + audio_bufsize) - audio_pos;
		memcpy(stream, audio_pos, remain);
		//halfcpy(stream, audio_pos, remain);
		
		memcpy(stream + remain, audio_chunk, len - remain);
		//halfcpy(stream + remain, audio_chunk, len - remain);
		
		audio_pos = audio_chunk + len - remain;
		
	}

}
*/

#if 0
BOOL sdl_LockAudioBuffer(unsigned char **pBlock1, unsigned long *pBlock1Size, unsigned char **pBlock2, unsigned long *pBlock2Size, int AudioBufferSize)
{	
	int remain;

	SDL_LockAudio();

	remain = audio_bufsize - (audio_rec - audio_chunk);

	if(remain > AudioBufferSize) {

		*pBlock1 = audio_rec;
		*pBlock1Size = AudioBufferSize;
		*pBlock2 = NULL;
		*pBlock2Size = 0;
		audio_rec += AudioBufferSize;
	}
	else
	{
		*pBlock1 = audio_rec;
		*pBlock1Size = remain;
		*pBlock2 = audio_chunk;
		*pBlock2Size = AudioBufferSize - remain;
		audio_rec = audio_chunk + *pBlock2Size;
		//sleep(10);
	}

	return TRUE;
}

void sdl_UnLockAudioBuffer(void)
{
	SDL_UnlockAudio();
}

void sdl_close_audio(void)
{
	SDL_CloseAudio();
}
#endif
