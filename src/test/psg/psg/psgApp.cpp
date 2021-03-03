/***************************************************************
 * Name:      psgApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Kevin Thacker (kev@arnoldemu.freeserve.co.uk)
 * Created:   2011-06-25
 * Copyright: Kevin Thacker (arnold.cpc-live.com)
 * License:
 **************************************************************/

#ifdef WX_PRECOMP
#include "wx_pch.h"
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "psgApp.h"
#include "psgMain.h"

#include <SDL/SDL.h>


extern "C"
{
    #include "../../cpc/cpc.h"
    #include "../../cpc/psg.h"
};
IMPLEMENT_APP(psgApp);


#define AUDIO_WATERMARK 2048	/* FIXME */

static const int audio_bufsize = 8192;
static const int audio_BitsPerSample = 8;
static const int audio_Frequency = 44100;
const int audio_NumberOfChannels = 2;
static Uint8 *audio_chunk;
static Uint32 audio_len;
static Uint8 *audio_pos;
static Uint8 *audio_rec;
static const int audio_callbacksize = AUDIO_WATERMARK/2;


void	sdl_fill_audio(void *userdata, Uint8 *stream, int len) {
}


int nRecords = 0;
unsigned char *pData;
unsigned char *pCurrent;
int nRecord = 0;

/* call 1 time per frame */
void UpdatePSG()
{
    nRecord++;
    if (nRecord==nRecords)
    {
        // reset to start
        pCurrent = pData;
        nRecord = 0;
    }

    PSG_SetBC2State(1);
    for (int i=0; i<14; i++)
    {
        bool bWrite = false;
        unsigned char Data = pCurrent[i];

        if (i==13)
        {
            if (Data!=0x0ff)
            {
                bWrite = true;
            }
        }
        else
        {
            bWrite = true;
        }

        if (bWrite)
        {
            /* select register */
            PSG_SetBDIRState(1);
            PSG_SetBC1State(1);
            PSG_RefreshState();
            PSG_Write(i);

            /* write data for register */
            PSG_SetBDIRState(1);
            PSG_SetBC1State(1);
            PSG_Write(Data);
        }
    }
    pCurrent += 14;
}

extern "C"
{
    int PSG_GetPortInputs(int nPort)
    {
        return 0x0ff;
    }

    void PSG_SetPortOutputs(int nPort, int nData)
    {


    }
};

void UpdateSDL()
{
       SDL_LockAudio();


    SDL_UnlockAudio();
}


bool psgApp::OnInit()
{
          if ( SDL_Init(SDL_INIT_AUDIO) < 0 )
    {
        return false;
    }

    FILE *fh;
    fh = fopen("test.ym","rb");
    if (fh!=NULL)
    {
        fseek(fh,0, SEEK_END);
        unsigned long Length = ftell(fh);
        fseek(fh, 0, SEEK_SET);
        pData = (unsigned char *)malloc(Length);
        fread(pData, Length, 1, fh);
        fclose(fh);
        nRecords = Length/14;
    }
    pCurrent = pData;
    nRecord = 0;

    SDL_AudioSpec desired;
    desired.freq = audio_Frequency;
    if (audio_BitsPerSample == 16) {
		desired.format = AUDIO_S16;
	} else {
		desired.format = AUDIO_S8;
	}
	desired.channels = audio_NumberOfChannels;
	desired.samples = audio_callbacksize;
	desired.callback = sdl_fill_audio;
	desired.userdata = NULL;

    SDL_AudioSpec obtained;
    SDL_OpenAudio(&desired, &obtained);
//    fprintf(stderr, "Opened Audio device: %i/%0x/%i\n",
	//	audioSpec->freq, audioSpec->format, audioSpec->samples);
	if (audio_chunk != NULL) free(audio_chunk);
	audio_chunk = (Uint8 *)malloc(audio_bufsize);
	if (audio_chunk == NULL) {
	//	fprintf(stderr,Messages[83],
		//	audio_bufsize);
	//	exit(1);
	}
	memset(audio_chunk, 0, audio_bufsize);
	audio_pos = audio_chunk;
	audio_rec = audio_chunk;
	audio_len = audio_bufsize;
	//audio_waterlevel = 0;
	SDL_PauseAudio(0);


    psgDialog* dlg = new psgDialog(0L);
    dlg->SetIcon(wxICON(aaaa)); // To Set App Icon
    dlg->Show();
    return true;
}

int psgApp::OnExit()
{
    SDL_Quit();
    return 0;
}
