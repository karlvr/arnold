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
#ifdef USE_SDL2

#include "../arnguiApp.h"
#include "SDL2PlatformSpecific.h"
#include "../IntClientData.h"
#include "../sdlcommon/sound.h"
#include <wx/msgdlg.h>

#if defined(__WXGTK__) || defined(__WXMAC__)
#include <sys/time.h>
#endif

#include "../OSD.h"
#ifdef USE_OSD
OSDDisplay m_OSDDisplay;
#endif

#ifdef __WXMAC__
extern "C" 
{
	void *GetWindowForView(void *);
};
#endif

extern "C"
{
#include "../../cpc/render.h"
#include "../../cpc/host.h"
#include "../../cpc/cpc.h"
#include "../../cpc/autotype.h"
#include "../../cpc/autorunfile.h"
#include "../../cpc/joystick.h"
#include "../../cpc/dumpym.h"
#include "../../cpc/mouse.h"
#include "../../cpc/keyjoy.h"
}

static int ScanCodeToCPCKeyInternational[SDL_NUM_SCANCODES];
static int ScanCodeToCPCKeyAzerty[SDL_NUM_SCANCODES];
static int ScanCodeToCPCKeyQwertz[SDL_NUM_SCANCODES];
static int ScanCodeToCPCKeyUserDefined[SDL_NUM_SCANCODES];
//static int ScanCodeToCPCKey[SDL_NUM_SCANCODES];

// table to map ScanCode values to CPC Key values
static int	ScanCodeToCPCKeyActive[SDL_NUM_SCANCODES];

//#define HAVE_GL

#ifdef HAVE_GL
#include <GL/gl.h>
#define SDL_SCRLENX 384     /* Width and height of the CPC screen drawn by Arnold */
#define SDL_SCRLENY 272
#define SDL_TEXLENX 512     /* Width and height of the corresponding texture, must be a power of 2 */
#define SDL_TEXLENY 512
#define SDL_TEXSCALE 5      /* Scaling factor for CPC screen texture to allow zooming in */
#define SDL_CLIPZ 100000    /* Distance of the far Z-clipping plane */
#ifdef MACOS
/* IN MACOSX, THIS IS MUCH FASTER */
#define GL_BGRAFORMAT GL_UNSIGNED_INT_8_8_8_8_REV
#else
/* IN LINUX (ATI RADEON) THIS IS MUCH FASTER */
#define GL_BGRAFORMAT GL_UNSIGNED_BYTE
#endif
int sdl_eyedis;             /* Distance between eye and viewplane */
int sdl_scrposz;            /* The screen Z-position (zoom) */
int sdl_zoomspeed;          /* Current speed of window zooming with Ctrl+Cursor Up/Down */
int sdl_warpfacdisptime;    /* Number of screen updates left until the warp factor is no longer displayed */
GLuint sdl_texture;         /* Texture with CPC screen */
SDL_Surface *sdl_texbuffer; /* Buffer to store texture data before upload */
#endif


#ifdef GTK2_EMBED_WINDOW
#if defined(__WXGTK__ )
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#undef GSocket
#endif
#endif

void SetSDLEnv(const wxChar *chId, const wxChar *chValue)
{
#if defined(__WXMSW__)
	wxString sID(chId);
	wxString sValue(chValue);
	wxCharBuffer bufferId = sID.mb_str();
	wxCharBuffer bufferVal = sValue.mb_str();
	 wxGetApp().Log("%s=%s\n", bufferId.data(), bufferVal.data());
	SDL_setenv(bufferId.data(), bufferVal.data(), 1);

	 wxGetApp().Log("Result from SDL_getenv: %s\n",SDL_getenv(bufferId.data()));
#else
	wxString sID(chId);
	wxString sValue(chValue);
	wxCharBuffer bufferId = sID.mb_str();
	wxCharBuffer bufferVal = sValue.mb_str();
	 wxGetApp().Log("%s=%s\n",bufferId.data(), bufferVal.data());
	setenv(bufferId.data(), bufferVal.data(), 1);

	 wxGetApp().Log("Result from SDL_getenv: %s\n", SDL_getenv(bufferId));
#endif

}

void PlatformSpecific::SetWindowPosition(int x, int y)
{
	SDL_SetWindowPosition(m_pScreen, x, y);
}

void PlatformSpecific::SetWindowSize(int width, int height)
{
	SDL_SetWindowSize(m_pScreen, width, height);
}


wxString PlatformSpecific::GetKeyName(int id)
{
	wxString sKeyName(SDL_GetScancodeName((SDL_Scancode)id), *wxConvCurrent);
	return sKeyName;
}


void PlatformSpecific::PopulateKeyDialog(wxChoice *pChoice)
{
	for (int i = 0; i < SDL_NUM_SCANCODES; i++)
	{
		wxString sKeyName(SDL_GetScancodeName((SDL_Scancode)i), *wxConvCurrent);
		if (!sKeyName.IsEmpty())
		{
			IntClientData *pIntData;

			pIntData = new IntClientData(i);
			pChoice->Append(sKeyName, pIntData);
		}
	}
}


// TODO: Make these user defineable
static ActionKey ActionKeys[] =
{
	// windows version has these set
	{ SDL_SCANCODE_F2, false, false, false, ACTION_TOGGLE_FULLSCREEN },
	{ SDL_SCANCODE_NUMLOCKCLEAR, false, false, false, ACTION_TOGGLE_KEYSTICK_ACTIVE },
	// {SDLK_F3, false, false, false, ACTION_INSERT_TAPE},
	// {SDLK_F4, false, false, false, ACTION_SAVE_SNAPSHOT},
	{ SDL_SCANCODE_F5, false, false, false, ACTION_CPC_RESET },
	{ SDL_SCANCODE_F5, true, false, false, ACTION_CPC_RESET_IMMEDIATE },
	{ SDL_SCANCODE_F6, false, false, false, ACTION_INSERT_DISK_DRIVE_A },
	{ SDL_SCANCODE_F7, false, false, false, ACTION_INSERT_DISK_DRIVE_B },
	{ SDL_SCANCODE_F10, false, false, false, ACTION_QUIT_IMMEDIATE },
	//  {SDLK_F10, true, false, false, ACTION_QUIT_IMMEDIATE},
	{ SDL_SCANCODE_SYSREQ, false, false, false, ACTION_SAVE_SCREENSHOT },

	// linux ones
	//	{SDLK_F1, false, false, false, ACTION_CPC_RESET},
	//	{SDLK_F2, false, false, false, ACTION_TOGGLE_FULLSCREEN},
	//	{SDLK_F4, false, false, false, ACTION_QUIT},
	//   {SDLK_F6, false, false, false, ACTION_DECREASE_WARP_FACTOR},
	//  {SDLK_F7, false, false, false, ACTION_INCREASE_WARP_FACTOR}
	// {SDLK_UP, false, true,  false, ACTION_ZOOM_IN},
	//{SDLK_DOWN, false, true,  false, ACTION_ZOOM_OUT},
	// sdlk_f5 was toggle joystick on/off but we have up to 2 digital and 2 analogue joysticks now
};
bool bFirstTiming = true;
unsigned long PreviousTime = 0;
unsigned long TimeError = 0;

/* constantly updated */
unsigned long MinTime = 0;
/* constantly updated */
unsigned long MaxTime = 0;
/* time of last */
unsigned long LastTime = 0;
/* cumulative time for last MAX_TIMINGS frames */
unsigned long CumulativeTime = 0;
/* number of timings */
unsigned long NumTimings = 0;
#define MAX_TIMINGS 100
unsigned long LastTimings[MAX_TIMINGS];
unsigned long TimingLastIndex = 0;

void PlatformSpecific::ResetTiming()
{
	// reset the error
	TimeError = 0;

	// NOTE: FPS reporting, if going from fast to slow it's fairly quick to report the value
	// if it's going from slow to fast you can see it as it increases. It takes a few frames to get there.
	// We can also see that if you try for 200% you can see if it's achieved it or not.

	NumTimings = 0;
	bFirstTiming = true;
	TimingLastIndex = 0;
	MinTime = 0;
	MaxTime = 0;
	LastTime = 0;
	CumulativeTime = 0;
}

void UpdateTiming(unsigned long Diff)
{
	/* update min and max */
	if (Diff < MinTime)
	{
		MinTime = Diff;
	}
	else if (Diff > MaxTime)
	{
		MaxTime = Diff;
	}

	/* update index */
	TimingLastIndex++;
	if (TimingLastIndex >= MAX_TIMINGS)
	{
		TimingLastIndex = 0;
	}

	/* remove a timing from the cumulative timings */
	if (NumTimings == MAX_TIMINGS)
	{
		CumulativeTime -= LastTimings[TimingLastIndex];
	}
	else
	{
		NumTimings++;
	}

	/* store this */
	LastTimings[TimingLastIndex] = Diff;
	/* and update cumulative */
	CumulativeTime += LastTimings[TimingLastIndex];
}

unsigned long PlatformSpecific::GetMinTime()
{
	return MinTime;
}
unsigned long PlatformSpecific::GetMaxTime()
{
	return MaxTime;
}

unsigned long PlatformSpecific::GetAverageTime()
{
	if (NumTimings == 0)
		return 0;

	return CumulativeTime / NumTimings;
}

int PlatformSpecific::GetAverageFPS()
{
	unsigned long Time = GetAverageTime();

	/* 100% -> 20
	50% -> 40
	*/
	if (Time == 0)
		return 0;

	return (20 * 100) / (Time);
}

#if defined(__WXGTK__) || defined(__WXMAC__)
/* Some comments about throttling on Linux/Unix:
 * Originally I used the nice and portable SDL_GetTicks() and SDL_Delay()
 * functions. Unfortunately the resolution is bad and the result is very
 * dodgy. Therefore I used the gettimeofday() and usleep() functions. This is
 * POSIX, but not SDL and therefore not directly portable to non-POSIX SDL
 * Targets.
 * I left in the SDL in case someone wants to try.
 * FIXME: Maybe we can get rid of floating point here?
 */
bool bFirstDiffTiming = true; /* indicates we need to take initial timing */
struct timeval prev; /* the initial timing */
int64_t delta_time_microseconds()
{
	struct timeval current;
	int64_t dt = 0;
	gettimeofday(&current,NULL);
	/* first timing? */
	if (bFirstDiffTiming)
	{
		/* we have the first timing now */
		bFirstDiffTiming = false;
	}
	else  
	{
		struct timeval diff;
		timersub(&current, &prev, &diff);
		dt = (diff.tv_sec*1000000)+diff.tv_usec;
	}
	memcpy( &prev, &current, sizeof(struct timeval) );
	return dt;
}
#endif

void PlatformSpecific::ThrottleSpeed(int nPercent)
{
	// 0 = unlimited; we skip any attempt to lock the speed
	if (nPercent == 0)
	{
#ifdef WIN32
		if (bFirstTiming)
		{
			/* first timing so record it */
			bFirstTiming = false;
			PreviousTime = timeGetTime();
		}
		else
		{
			unsigned long Current = timeGetTime();
			UpdateTiming(Current - PreviousTime);
			PreviousTime = Current;
		}
#endif
#if defined(__WXGTK__) || defined(__WXMAC__)
		int64_t deltaTime = delta_time_microseconds();
		UpdateTiming(deltaTime / 1000);
#endif
		return;
	}
	// 50fps is 100% speed
	// 50fps = 0.02 seconds or 20ms
	//
	// nPercent = 200 we want it to be 25th of a second
	// nPercent = 50 we want it to be 100th of a second
	//
	// factor = nPercent/100
	// frame_time = normal_time/factor
	//
	// frame_time = (normal_time*100)/nPercent
#ifdef WIN32
	int FrameTimeMS = (20 * 100) / nPercent;

	/* use this to throttle speed */
	unsigned long	TimeDifference;
	unsigned long	Time;

	do
	{
		/* get current time */
		Time = timeGetTime();

		/* calc time difference */
		TimeDifference = Time - (PreviousTime - TimeError);
	} while (TimeDifference < FrameTimeMS);

	TimeError = (TimeDifference - FrameTimeMS) % FrameTimeMS;

	UpdateTiming(Time - PreviousTime);

	PreviousTime = Time;

#endif
#if defined(__WXGTK__) || defined(__WXMAC__)
#if 0
	static Uint32 next_tick = 0;
	Uint32 this_tick;

	/* Wait for the next frame */
	this_tick = SDL_GetTicks(); 
	if ( this_tick < next_tick ) {
		SDL_Delay(next_tick-this_tick);
	}
	next_tick = this_tick + (FrameTimeMS);
#endif
	/* time wanted */
	int64_t FrameTimeMicroseconds = (20000*100)/nPercent;
	/* actual time */
	int64_t ActualTime = delta_time_microseconds();

	UpdateTiming(ActualTime/1000);

	if ((ActualTime>=0) && (ActualTime<=FrameTimeMicroseconds))
	{
		/* running to time, or a little fast */
		int64_t delay = FrameTimeMicroseconds-ActualTime;
		if (delay!=0)
		{
			/* accuracy is quite poor, we could instead decide if we need to add up a number of timings
			to achieve what we want, or try and use nanosleep */

			/* sleep if running a little too fast */
			struct timespec req;
			struct timespec rem;
			req.tv_sec = 0;
			req.tv_nsec =  delay*1000;
			nanosleep(&req,&rem);

			//usleep(delay);
		}
	}
	else 
	{
		/* running slow */
}
#endif
}

//=====================================================================

PlatformSpecific::PlatformSpecific() :
m_bDisableKeyboardEvents(false),
m_bDisableJoystickEvents(false),
m_bHasAudio(false),
m_bHasJoystick(false),
m_bFullScreenActive(false),
bActive(false),
m_pScreen(NULL),
m_pRenderer(NULL),
m_pTexture(NULL),
m_pContext(0),
m_last_statePOV(0)
{
	InitKeySets();
	SetKeySet(0);
}

PlatformSpecific::~PlatformSpecific()
{
}
//=====================================================================

#ifdef SDL

static  unsigned long CalcBPPFromMask(unsigned long Mask)
{
	unsigned long LocalShift = 0;
	unsigned long LocalBPP = 0;
	unsigned long LocalMask = Mask;

	if (LocalMask!=0)
	{
		do
		{
			if ((LocalMask & 0x01)!=0)
			{
				break;
			}

			LocalMask = LocalMask >>1;
			LocalShift++;
		}
		while (1==1);

		do
		{
			if ((LocalMask & 0x01)!=1)
			{
				break;
			}

			LocalMask = LocalMask>>1;
			LocalBPP++;
		}
		while (1==1);
	}

	return LocalBPP;
}
#endif

//=====================================================================

void PlatformSpecific::ShutdownDisplay()
{
	if (m_pTexture)
	{
		SDL_DestroyTexture(m_pTexture);
		m_pTexture = NULL;
	}

	if (m_pRenderer)
	{
		SDL_DestroyRenderer(m_pRenderer);
		m_pRenderer = NULL;
	}

	if (m_pContext)
	{
		SDL_GL_DeleteContext(m_pContext);
		m_pContext = 0;
	}
#if defined(WIN_EMBED_WINDOW)
#elif defined(GTK2_EMBED_WINDOW) && defined(__WXGTK__)
#elif defined(MAC_EMBED_WINDOW) && defined(__WXMAC__)
	// on mac this makes no difference
	if (m_pScreen)
	{
		// if active switch out
		if (m_bFullScreenActive)
		{
			// switch out of full-screen
			SDL_SetWindowFullscreen(m_pScreen, 0);
		}

		SDL_DestroyWindow(m_pScreen);
		m_pScreen = NULL;
	}

#else
	if (m_pScreen)
	{
		// if active switch out
		if (m_bFullScreenActive)
		{
			// switch out of full-screen
			SDL_SetWindowFullscreen(m_pScreen, 0);
		}

		SDL_DestroyWindow(m_pScreen);
		m_pScreen = NULL;
}
#endif
}


//=====================================================================

void PlatformSpecific::RestartDisplay()
{
	if (m_pTexture)
	{
		SDL_DestroyTexture(m_pTexture);
		m_pTexture = NULL;
	}

	if (m_pRenderer)
	{
		SDL_DestroyRenderer(m_pRenderer);
		m_pRenderer = NULL;
	}

	if (m_pContext)
	{
		SDL_GL_DeleteContext(m_pContext);
		m_pContext = 0;
	}

#if defined(WIN_EMBED_WINDOW)
#elif defined(GTK2_EMBED_WINDOW) && defined(__WXGTK__)
#elif defined(MAC_EMBED_WINDOW) && defined(__WXMAC__)
#else
	if (m_pScreen)
	{
		// if active switch out
		if (m_bFullScreenActive)
		{
			// switch out of full-screen
			SDL_SetWindowFullscreen(m_pScreen, 0);
		}

		SDL_DestroyWindow(m_pScreen);
		m_pScreen = NULL;
}
#endif
}

//=====================================================================

void PlatformSpecific::ShutdownInput()
{
	// don't allow the idle thread to read from joysticks because we are about to shut them down
	m_bHasJoystick = false;
	CloseJoysticks();

}

void PlatformSpecific::Shutdown()
{
	SDL_StopTextInput();

	 wxGetApp().Log("About to shut down SDL2\n");

	 wxGetApp().Log("Shutting down display\n");
	ShutdownDisplay();

	 wxGetApp().Log("Shutting down input\n");
	ShutdownInput();

	SDLCommon::Shutdown();

	 wxGetApp().Log("Shutting down SDL2\n");

	SDL_Quit();
}

//=====================================================================

void	sdl_InitialiseKeyboardMapping(int layout);
// forward declarations
void	sdl_InitialiseKeyboardMapping_qwertz();
void	sdl_InitialiseKeyboardMapping_azerty();
void	sdl_InitialiseKeyboardMapping_spanish();

void PlatformSpecific::InitKeySets()
{
	ClearHostKeys();

	sdl_InitialiseKeyboardMapping_qwertz();
	sdl_InitialiseKeyboardMapping_azerty();

	{
		for (int i = 0; i < SDL_NUM_SCANCODES; i++)
		{
			ScanCodeToCPCKeyInternational[i] = CPC_KEY_NULL;
		}

		/* International key mappings */
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_0] = CPC_KEY_ZERO;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_1] = CPC_KEY_1;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_2] = CPC_KEY_2;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_3] = CPC_KEY_3;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_4] = CPC_KEY_4;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_5] = CPC_KEY_5;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_6] = CPC_KEY_6;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_7] = CPC_KEY_7;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_8] = CPC_KEY_8;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_9] = CPC_KEY_9;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_A] = CPC_KEY_A;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_B] = CPC_KEY_B;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_C] = CPC_KEY_C;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_D] = CPC_KEY_D;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_E] = CPC_KEY_E;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_F] = CPC_KEY_F;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_G] = CPC_KEY_G;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_H] = CPC_KEY_H;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_I] = CPC_KEY_I;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_J] = CPC_KEY_J;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_K] = CPC_KEY_K;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_L] = CPC_KEY_L;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_M] = CPC_KEY_M;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_N] = CPC_KEY_N;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_O] = CPC_KEY_O;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_P] = CPC_KEY_P;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_Q] = CPC_KEY_Q;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_R] = CPC_KEY_R;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_S] = CPC_KEY_S;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_T] = CPC_KEY_T;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_U] = CPC_KEY_U;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_V] = CPC_KEY_V;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_W] = CPC_KEY_W;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_X] = CPC_KEY_X;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_Y] = CPC_KEY_Y;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_Z] = CPC_KEY_Z;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_SPACE] = CPC_KEY_SPACE;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_COMMA] = CPC_KEY_COMMA;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_PERIOD] = CPC_KEY_DOT;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_SEMICOLON] = CPC_KEY_COLON;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_MINUS] = CPC_KEY_MINUS;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_EQUALS] = CPC_KEY_HAT;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_LEFTBRACKET] = CPC_KEY_AT;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_RIGHTBRACKET] = CPC_KEY_OPEN_SQUARE_BRACKET;

		ScanCodeToCPCKeyInternational[SDL_SCANCODE_TAB] = CPC_KEY_TAB;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_RETURN] = CPC_KEY_RETURN;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_BACKSPACE] = CPC_KEY_DEL;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_ESCAPE] = CPC_KEY_ESC;

		//ScanCodeToCPCKeyInternational[SDL_SCANCODE_Equals & 0x0ff)] = CPC_KEY_CLR;

		ScanCodeToCPCKeyInternational[SDL_SCANCODE_UP] = CPC_KEY_CURSOR_UP;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_DOWN] = CPC_KEY_CURSOR_DOWN;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_LEFT] = CPC_KEY_CURSOR_LEFT;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_RIGHT] = CPC_KEY_CURSOR_RIGHT;

		ScanCodeToCPCKeyInternational[SDL_SCANCODE_KP_0] = CPC_KEY_F0;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_KP_1] = CPC_KEY_F1;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_KP_2] = CPC_KEY_F2;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_KP_3] = CPC_KEY_F3;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_KP_4] = CPC_KEY_F4;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_KP_5] = CPC_KEY_F5;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_KP_6] = CPC_KEY_F6;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_KP_7] = CPC_KEY_F7;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_KP_8] = CPC_KEY_F8;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_KP_9] = CPC_KEY_F9;

		ScanCodeToCPCKeyInternational[SDL_SCANCODE_KP_PERIOD] = CPC_KEY_FDOT;

		ScanCodeToCPCKeyInternational[SDL_SCANCODE_LSHIFT] = CPC_KEY_SHIFT;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_RSHIFT] = CPC_KEY_SHIFT;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_LCTRL] = CPC_KEY_CONTROL;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_RCTRL] = CPC_KEY_CONTROL;
		ScanCodeToCPCKeyInternational[SDL_SCANCODE_CAPSLOCK] = CPC_KEY_CAPS_LOCK;

		ScanCodeToCPCKeyInternational[SDL_SCANCODE_KP_ENTER] = CPC_KEY_SMALL_ENTER;


#if 0
		ScanCodeToCPCKeyInternational[0x0134] = CPC_KEY_COPY;			/* Alt */
		ScanCodeToCPCKeyInternational[0x0137] = CPC_KEY_COPY;			/* Compose */
#endif

		ScanCodeToCPCKeyInternational[SDL_SCANCODE_INSERT] = CPC_KEY_CLR;

	}
}


bool PlatformSpecific::Init(bool bAudio, bool bJoystick)
{

	 wxGetApp().Log("Initialising SDL2\n");

	//#if defined(__WXMAC__)
	//	SetSDLEnv(wxT("SDL_VIDEODRIVER"),wxT("opengl"));
	//#endif

	// initialise video.. this is required
	// this will also initialise the event system
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		 wxGetApp().Log("SDL2 video didn't initialise\n");
		 wxGetApp().Log("SDL2 error: %s\n", SDL_GetError());
		return false;
	}
	else
	{
		 wxGetApp().Log("SDL2 video initialised\n");
	}

	// initialise joystick?
	if (bJoystick)
	{
		// initialise game controller; this automatically initialises joystick
		if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0)
		{
			 wxGetApp().Log("SDL2 game controller did not initialise\n");
			 wxGetApp().Log("SDL2 error: %s\n", SDL_GetError());
		}
		else
		{
			m_bHasJoystick = true;
			 wxGetApp().Log("SDL2 game controller initialised\n");

			// set joystick background. This is a little hack to make joysticks always
			// work. 
			SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,"1");
#ifdef __WXMAC__
			// stop system input stuff..
			//SDL_SetHint(SDL_HINT_GRAB_KEYBOARD, "1");
#endif
		}
	}

	// initialise audio?
	if (bAudio)
	{
		m_bHasAudio = SDLCommon::Init();
	}
	else
	{
		 wxGetApp().Log("Didn't initialise Audio. It was not enabled in options\n");
	}

	return true;
}

//=====================================================================


void	PlatformSpecific::HandleKey(SDL_KeyboardEvent *theEvent)
{
	CPC_KEY_ID	theKeyPressed;
	int *pKeySet = ScanCodeToCPCKeyActive;

	SDL_Keysym	*keysym = &theEvent->keysym;
	SDL_Scancode keycode = keysym->scancode;
	if (keycode >= SDL_NUM_SCANCODES)
		return;

	// if key stick is active and key used by keystick then block
	if (Joystick_IsKeyStickActive() && Joystick_IsKeyUsedByKeyStick(keycode))
	{
		return;
	}

	theKeyPressed = (CPC_KEY_ID)pKeySet[keycode];

	if (theKeyPressed != CPC_KEY_NULL)
	{
		// set or release key depending on state
		if (theEvent->type == SDL_KEYDOWN)
		{
			// set key
			CPC_SetKey(theKeyPressed);
		}
		else
		{
			// release key
			CPC_ClearKey(theKeyPressed);
		}
	}
}


//=====================================================================
void PlatformSpecific::AutoConfigureTouch()
{
	int nTouchDevices = SDL_GetNumTouchDevices();
	 wxGetApp().Log("%d touch devices\n", nTouchDevices);
}

void PlatformSpecific::AutoConfigureJoystick()
{
	 wxGetApp().Log("Auto-configuring joystick\n");

	// setup default configuration for joysticks
	bool bOpenedJoystick = false;

	// does sdl have a joystick setup?
	if (SDL_NumJoysticks() != 0)
	{
		// choose game controller first, then joystick
		for (int i = 0; i < SDL_NumJoysticks(); i++)
		{
			// game controller?
			if (SDL_IsGameController(i))
			{

				 wxGetApp().Log("Joystick %d is a game controller\n", i);

				SDL_GameController *pController = SDL_GameControllerOpen(i);
				if (pController)
				{
					 wxGetApp().Log("Enabling digitial joystick 0 and setting it to %s\n", SDL_GameControllerName(pController));
					 wxGetApp().Log("Enabling Plus analogue joystick 0 and setting it to %s\n", SDL_GameControllerName(pController));
					// get joystick but don't free
					SDL_Joystick *pJoystick = SDL_GameControllerGetJoystick(pController);
					SDL_JoystickID id = SDL_JoystickInstanceID(pJoystick);

					// succeeded opening joystick
					Joystick_Activate(CPC_DIGITAL_JOYSTICK0, TRUE);
					Joystick_Activate(CPC_ANALOGUE_JOYSTICK0, TRUE);
					Joystick_SetType(CPC_DIGITAL_JOYSTICK0, JOYSTICK_TYPE_REAL);
					Joystick_SetType(CPC_ANALOGUE_JOYSTICK0, JOYSTICK_TYPE_REAL);
					Joystick_SetPhysical(CPC_DIGITAL_JOYSTICK0, (int)id);
					Joystick_SetPhysical(CPC_ANALOGUE_JOYSTICK0, (int)id);

					Joystick_Reset(CPC_DIGITAL_JOYSTICK0);
					Joystick_Reset(CPC_ANALOGUE_JOYSTICK0);

					KeyJoy_SetPhysical((int)id);

					SDL_GameControllerClose(pController);

					RefreshJoysticks();
					bOpenedJoystick = true;
					break;
				}
				else
				{
					 wxGetApp().Log("Failed to open game controller %d\n", i);
				}
			}
		}

		//now joystick turn
		if (!bOpenedJoystick)
		{
			for (int i = 0; i < SDL_NumJoysticks(); i++)
			{
				// skip game controllers
				if (!SDL_IsGameController(i))
				{
					 wxGetApp().Log("Joystick %d is not a game controller\n", i);

					SDL_Joystick *pJoystick = SDL_JoystickOpen(i);
					if (pJoystick)
					{
						 wxGetApp().Log("Enabling digitial joystick 0 and setting it to %s\n", SDL_JoystickName(pJoystick));
						 wxGetApp().Log("Enabling Plus analogue joystick 0 and setting it to %s\n", SDL_JoystickName(pJoystick));

						// get id
						SDL_JoystickID id = SDL_JoystickInstanceID(pJoystick);

						// succeeded opening joystick
						Joystick_Activate(CPC_DIGITAL_JOYSTICK0, TRUE);
						Joystick_Activate(CPC_ANALOGUE_JOYSTICK0, TRUE);
						Joystick_SetType(CPC_DIGITAL_JOYSTICK0, JOYSTICK_TYPE_REAL);
						Joystick_SetType(CPC_ANALOGUE_JOYSTICK0, JOYSTICK_TYPE_REAL);
						Joystick_SetPhysical(CPC_DIGITAL_JOYSTICK0, (int)id);
						Joystick_SetPhysical(CPC_ANALOGUE_JOYSTICK0, (int)id);

						KeyJoy_SetPhysical((int)id);

						Joystick_Reset(CPC_DIGITAL_JOYSTICK0);
						Joystick_Reset(CPC_ANALOGUE_JOYSTICK0);

						if (SDL_JoystickGetAttached(pJoystick))
						{
							SDL_JoystickClose(pJoystick);
						}

						RefreshJoysticks();

						bOpenedJoystick = true;
						break;
					}
					else
					{
						 wxGetApp().Log("Failed to open joystick %d\n", i);
					}
				}
			}
		}
	}
	else
	{
		 wxGetApp().Log("No joysticks or game controllers are attached\n");
	}

	if (!bOpenedJoystick)
	{
		 wxGetApp().Log("No physical joysticks connected. Auto configuration has failed\n");
	}
}


//=====================================================================

void PlatformSpecific::CloseJoysticks()
{
	// stop events
	SDL_JoystickEventState(SDL_DISABLE);
	SDL_GameControllerEventState(SDL_DISABLE);

	// close all existing
	for (unsigned int j = 0; j != m_OpenedJoysticks.GetCount(); j++)
	{
		SDL_Joystick *pJoystick = (SDL_Joystick *)m_OpenedJoysticks[j];
		SDL_JoystickClose(pJoystick);
	}
	m_OpenedJoysticks.Clear();
	// close all existing
	for (unsigned int j = 0; j != m_OpenedGameControllers.GetCount(); j++)
	{
		SDL_GameController *pController = (SDL_GameController *)m_OpenedGameControllers[j];
		SDL_GameControllerClose(pController);
	}
	m_OpenedGameControllers.Clear();
}


//=====================================================================
// opening a joystick uses an index.
// all other messages use an id.
// match id to index just in case they are actually different.  

int PlatformSpecific::JoystickFromID(SDL_JoystickID idWanted)
{
	// iterate through all
	for (int i = 0; i < SDL_NumJoysticks(); i++)
	{
		// open this joystick 
		SDL_Joystick *pJoystick = SDL_JoystickOpen(i);
		if (pJoystick != NULL)
		{
			// get it's id
			SDL_JoystickID id = SDL_JoystickInstanceID(pJoystick);
			if (id == idWanted)
			{
				// id we wanted, so close joystick and return id.
				SDL_JoystickClose(pJoystick);
				return i;
			}
			// not id we wanted, close joysticks
			SDL_JoystickClose(pJoystick);
		}
	}
	// report we didn't find it.
	return -1;
}

//=====================================================================
// for saving

// based on SDL_JoystickID return the string we can use to store it by and retrieve it later
wxString PlatformSpecific::GetJoystickIdString(int id)
{
	int nIndex = JoystickFromID(id);
	if (nIndex != -1)
	{
		SDL_Joystick *pJoystick = SDL_JoystickOpen(nIndex);
		if (pJoystick != NULL)
		{
			// get the name
			const char *cName = SDL_JoystickName(pJoystick);

			// get the name
			wxString sName(cName, *wxConvCurrent);

			SDL_JoystickClose(pJoystick);

			return sName;
		}
	}
	return wxEmptyString;
}

// for loading
int PlatformSpecific::GetJoystickIdFromString(wxString &sNameWanted)
{
	// iterate through all
	for (int i = 0; i < SDL_NumJoysticks(); i++)
	{
		// open this joystick 
		SDL_Joystick *pJoystick = SDL_JoystickOpen(i);
		if (pJoystick != NULL)
		{
			// get the name
			const char *cName = SDL_JoystickName(pJoystick);

			// get the name
			wxString sName(cName, *wxConvCurrent);

			if (sName == sNameWanted)
			{
				// get the id
				SDL_JoystickID id = SDL_JoystickInstanceID(pJoystick);

				// close it
				SDL_JoystickClose(pJoystick);

				return (int)id;
			}

			SDL_JoystickClose(pJoystick);
		}
	}
	return -1;
}

//=====================================================================

void PlatformSpecific::RefreshJoysticks()
{
	 wxGetApp().Log("Refresh joysticks\n");
	CloseJoysticks();
	//enable joysticks we are using
	bool bAtLeastOneJoystick = false;
	for (int j = 0; j < CPC_NUM_JOYSTICKS; j++)
	{
		Joystick_SetXRange(j, -32768, 32767);
		Joystick_SetYRange(j, -32768, 32767);
		Joystick_Reset(j);

		if (Joystick_GetType(j) == JOYSTICK_TYPE_REAL)
		{
			 wxGetApp().Log("CPC Joystick %d is real\n", j);
			int nPhysical = Joystick_GetPhysical(j);
			if (nPhysical != -1)
			{
				 wxGetApp().Log("Lookup joystick by ID: %d\n", nPhysical);

				int index = JoystickFromID((SDL_JoystickID)nPhysical);
				if (index != -1)
				{
					 wxGetApp().Log("Joystick index is: %d\n", index);
					if (SDL_IsGameController(index))
					{
						SDL_GameController *pController = SDL_GameControllerOpen(index);
						if (pController != NULL)
						{
							if (SDL_GameControllerGetAttached(pController))
							{
								 wxGetApp().Log("Enabling game controller %s\n", SDL_GameControllerName(pController));

								m_OpenedGameControllers.Add((void *)pController);
								bAtLeastOneJoystick = true;
							}
							else
							{
								SDL_GameControllerClose(pController);
							}
						}
					}
					else
					{

						SDL_Joystick *pJoystick = SDL_JoystickOpen(index);
						if (pJoystick)
						{
							if (SDL_JoystickGetAttached(pJoystick))
							{
								 wxGetApp().Log("Enabling game joystick %s\n", SDL_JoystickName(pJoystick));

								m_OpenedJoysticks.Add((void *)pJoystick);
								bAtLeastOneJoystick = true;

							}
							else
							{
								SDL_JoystickClose(pJoystick);
							}
						}
					}

				}
			}
			else
			{
				 wxGetApp().Log("Joystick not found\n");
			}
		}
	}
	if (KeyJoy_IsActive())
	{
		 wxGetApp().Log("Keyjoy - KeyJoy active\n");
		int nPhysical = KeyJoy_GetPhysical();
		if (nPhysical != -1)
		{
			 wxGetApp().Log("Keyjoy - Lookup joystick by ID: %d\n", nPhysical);

			int index = JoystickFromID((SDL_JoystickID)nPhysical);
			if (index != -1)
			{
				 wxGetApp().Log("Keyjoy - Keyjoy Joystick index is: %d\n", index);
				if (SDL_IsGameController(index))
				{
					SDL_GameController *pController = SDL_GameControllerOpen(index);
					if (pController != NULL)
					{
						if (SDL_GameControllerGetAttached(pController))
						{
							 wxGetApp().Log("Keyjoy - Enabling game controller %s\n", SDL_GameControllerName(pController));

							m_OpenedGameControllers.Add((void *)pController);
							bAtLeastOneJoystick = true;
						}
						else
						{
							SDL_GameControllerClose(pController);
						}
					}
				}
				else
				{
					SDL_Joystick *pJoystick = SDL_JoystickOpen(index);
					if (pJoystick)
					{
						if (SDL_JoystickGetAttached(pJoystick))
						{
							 wxGetApp().Log("Keyjoy - Enabling game joystick %s\n", SDL_JoystickName(pJoystick));
							m_OpenedJoysticks.Add((void *)pJoystick);
							bAtLeastOneJoystick = true;
						}
						else
						{
							SDL_JoystickClose(pJoystick);
						}
					}
				}
			}
		}
		bAtLeastOneJoystick = true;
	}
	// enable events if at least one joystick is enabled
	if (bAtLeastOneJoystick)
	{
		SDL_JoystickEventState(SDL_ENABLE);

		SDL_GameControllerEventState(SDL_ENABLE);
	}
	else
	{
		SDL_JoystickEventState(SDL_DISABLE);
		SDL_GameControllerEventState(SDL_DISABLE);
	}
}

//=====================================================================


void PlatformSpecific::ConfigureKeyboardMode()
{
	if (Keyboard_GetMode() == 1)
	{
		 wxGetApp().Log("SDL2 keyboard set to translated\n");

		SDL_StartTextInput();
	}
	else
	{
		SDL_StopTextInput();

		 wxGetApp().Log("SDL2 keyboard not set to positional\n");
	}
}

//=====================================================================
// Key To int part / KeyStick
//=====================================================================

#if ((defined(GTK2_EMBED_WINDOW) && defined(__WXGTK__)) || (defined(MAC_EMBED_WINDOW) && defined(__WXMAC__)))
#else
static int KeySetCursors[JOYSTICK_SIMULATED_KEYID_LAST] =
{
	SDL_SCANCODE_UP,
	SDL_SCANCODE_DOWN,
	SDL_SCANCODE_LEFT,
	SDL_SCANCODE_RIGHT,
	SDL_SCANCODE_SPACE,
	SDL_SCANCODE_RSHIFT,
	SDL_SCANCODE_RCTRL,
	0,
	0,
	0,
	0
};

static int KeySetWASD[JOYSTICK_SIMULATED_KEYID_LAST] =
{
	SDL_SCANCODE_W,
	SDL_SCANCODE_S,
	SDL_SCANCODE_A,
	SDL_SCANCODE_D,
	SDL_SCANCODE_SPACE,
	SDL_SCANCODE_RSHIFT,
	SDL_SCANCODE_RCTRL,
	0,
	0,
	0,
	0
};


static int KeySetNumPad[JOYSTICK_SIMULATED_KEYID_LAST] =
{
	SDL_SCANCODE_KP_8,
	SDL_SCANCODE_KP_2,
	SDL_SCANCODE_KP_4,
	SDL_SCANCODE_KP_6,
	SDL_SCANCODE_KP_5,
	SDL_SCANCODE_KP_0,
	SDL_SCANCODE_KP_PERIOD,
	SDL_SCANCODE_KP_7,
	SDL_SCANCODE_KP_9,
	SDL_SCANCODE_KP_1,
	SDL_SCANCODE_KP_3
};


static int KeySetHome[JOYSTICK_SIMULATED_KEYID_LAST] =
{
	SDL_SCANCODE_PAGEUP,
	SDL_SCANCODE_PAGEDOWN,
	SDL_SCANCODE_HOME,
	SDL_SCANCODE_END,
	SDL_SCANCODE_DELETE,
	SDL_SCANCODE_INSERT,
	SDL_SCANCODE_RCTRL,
	0,
	0,
	0,
	0
};

static int KeySetCustom[JOYSTICK_SIMULATED_KEYID_LAST] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
#endif

void PlatformSpecific::SetKeySet(int j, int *keys)
{
#if ((defined(GTK2_EMBED_WINDOW) && defined(__WXGTK__)) || (defined(MAC_EMBED_WINDOW) && defined(__WXMAC__)))
#else
	for (int i = 0; i < JOYSTICK_SIMULATED_KEYID_LAST; i++)
	{
		Joystick_SetSimulatedKeyID(j, (JOYSTICK_SIMULATED_KEYID)i, keys[i]);
	}
#endif
}



int *PlatformSpecific::GetKeySetPtr(int nKeySet)
{
#if ((defined(GTK2_EMBED_WINDOW) && defined(__WXGTK__)) || (defined(MAC_EMBED_WINDOW) && defined(__WXMAC__)))
	return NULL;
#else
	if (nKeySet == JOYSTICK_KEYSET_CURSORS) return KeySetCursors;
	else if (nKeySet == JOYSTICK_KEYSET_WASD) return KeySetWASD;
	else if (nKeySet == JOYSTICK_KEYSET_NUMPAD) return KeySetNumPad;
	else  if (nKeySet == JOYSTICK_KEYSET_INSERT_HOME) return KeySetHome;
	else  if (nKeySet == JOYSTICK_KEYSET_CUSTOM) return KeySetCustom;
	return NULL;
#endif
}

void PlatformSpecific::ConfigureJoystickKeySet(int j, int nKeySet)
{
#if ((defined(GTK2_EMBED_WINDOW) && defined(__WXGTK__)) || (defined(MAC_EMBED_WINDOW) && defined(__WXMAC__)))
#else
	//Never use this fonction, it will reset all
	//Remap list is loaded at start and every time the joystick dialog is closed

	/*
	if (nKeySet==JOYSTICK_KEYSET_CURSORS) SetKeySet(j,KeySetCursors);
	else if (nKeySet==JOYSTICK_KEYSET_NUMPAD) SetKeySet(j,KeySetNumPad);
	else  if (nKeySet==JOYSTICK_KEYSET_INSERT_HOME) SetKeySet(j,KeySetHome);
	else if (nKeySet==JOYSTICK_KEYSET_CUSTOM) SetKeySet(j, KeySetCustom);
	*/
#endif
}

//=====================================================================

void PlatformSpecific::ConfigureJoystickAsMouse(int j)
{
	Joystick_SetSimulatedMouseID(j, JOYSTICK_SIMULATED_FIRE1_MOUSEID, SDL_BUTTON_LEFT);
	Joystick_SetSimulatedMouseID(j, JOYSTICK_SIMULATED_FIRE2_MOUSEID, SDL_BUTTON_RIGHT);
	Joystick_SetSimulatedMouseID(j, JOYSTICK_SIMULATED_FIRE3_MOUSEID, SDL_BUTTON_MIDDLE);
}

//=====================================================================

void PlatformSpecific::ConfigureAudio()
{
	SDLCommon::Shutdown();
	if (m_bHasAudio)
	{
		 wxGetApp().Log("initialising audio\n");
		if (SDLCommon::InitAudio(wxGetApp().GetAudioFrequency(), wxGetApp().GetAudioBits(), wxGetApp().GetAudioChannels()))
		{
			 wxGetApp().Log("SDL Audio Ok\n");
		}
		else
		{
			 wxGetApp().Log("SDL Audio error\n");
		}
	}
}



static  unsigned long CalcBPPFromMask(unsigned long Mask)
{
	unsigned long LocalShift = 0;
	unsigned long LocalBPP = 0;
	unsigned long LocalMask = Mask;

	if (LocalMask != 0)
	{
		do
		{
			if ((LocalMask & 0x01) != 0)
			{
				break;
			}

			LocalMask = LocalMask >> 1;
			LocalShift++;
		} while (1 == 1);

		do
		{
			if ((LocalMask & 0x01) != 1)
			{
				break;
			}

			LocalMask = LocalMask >> 1;
			LocalBPP++;
		} while (1 == 1);
	}

	return LocalBPP;
}


static  unsigned long CalcShiftFromMask(unsigned long Mask)
{
	unsigned long LocalShift = 0;
	unsigned long LocalMask = Mask;

	if (LocalMask != 0)
	{
		do
		{
			if ((LocalMask & 0x01) != 0)
			{
				break;
			}

			LocalMask = LocalMask >> 1;
			LocalShift++;
		} while (1 == 1);
	}

	return LocalShift;
}

#if 0
int SDL_ToggleFS(SDL_Window *win)
{
	Uint32 flags = (SDL_GetWindowFlags(win) ^ SDL_WINDOW_FULLSCREEN_DESKTOP);
	if (SDL_SetWindowFullscreen(win, flags) < 0) // NOTE: this takes FLAGS as the second param, NOT true/false!
	{
		std::cout << "Toggling fullscreen mode failed: " << SDL_GetError() << std::endl;
		return -1;
	}
	int w = 640, h = 480; // TODO: UPDATE ME
	if ((flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0)
	{
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
		SDL_RenderSetLogicalSize(Renderer, w, h); // TODO: pass renderer as param maybe?
		return 1;
	}
	SDL_SetWindowSize(win, w, h);
	return 0;
}
#endif

// SDL_SetWindowFullScreen(window, SDL_WINDOW_FULLSCREEN); or SDL_WINDOW_FULLSCREEN_DESKTOP



void PlatformSpecific::SetupDisplay(bool bFullScreen, WindowedRenderSettings *pWindowed, void *pWindowHandle)
{
	int WindowWidthUsed = pWindowed->m_nWindowWidth;
	int WindowHeightUsed = pWindowed->m_nWindowHeight;
	int WindowX = 0;
	int WindowY = 0;
	int WindowOffsetX = 0;
	int WindowOffsetY = 0;
	m_bFullScreenActive = bFullScreen;
	//sShutdownDisplay();
	RestartDisplay();

	if (!bFullScreen)
	{
		//
		//***     WINDOWED MODE     ***
		//

		 wxGetApp().Log("Setting Windowed display\n");

		 wxGetApp().Log("Window Width: %d\n", pWindowed->m_nWindowWidth);
		 wxGetApp().Log("Window Height: %d\n", pWindowed->m_nWindowHeight);

		 wxGetApp().Log("Render Width: %d\n", pWindowed->m_nRenderWidth);
		 wxGetApp().Log("Render Height: %d\n", pWindowed->m_nRenderHeight);

		//#ifdef WIN32
		//if (!bShowFrame)
		//{
		//		mode|=SDL_NOFRAME;
		//}
		//	mode|=SDL_HWSURFACE;
		 wxGetApp().Log("Creating window\n");

#if defined(WIN_EMBED_WINDOW)
		// WIN32
		if (m_pScreen == NULL)
		{
			 wxGetApp().Log("HWND: %p\n", pWindowHandle);
			m_pScreen = SDL_CreateWindowFrom(pWindowHandle);
		}
		else
		{
			SDL_SetWindowSize(m_pScreen, pWindowed->m_nWindowWidth, pWindowed->m_nWindowHeight);
		}
#elif defined(GTK2_EMBED_WINDOW) && defined(__WXGTK__)
		// GTK2

		 wxGetApp().Log("GTKWidget: %p\n", pWindowHandle);
		GdkWindow *pWindow = gtk_widget_get_window((GtkWidget *)pWindowHandle);
		 wxGetApp().Log("GDKWindow: %p\n", (void *)pWindow);
		 wxGetApp().Log("the X11 id is %u\n",GDK_WINDOW_XWINDOW(pWindow));

		m_pScreen = SDL_CreateWindowFrom((void *)GDK_WINDOW_XWINDOW(pWindow));
		SDL_SetWindowSize(m_pScreen, pWindowed->m_nWindowWidth, pWindowed->m_nWindowHeight);

		 wxGetApp().Log("SDL screen: %p\n", m_pScreen);
#elif defined(MAC_EMBED_WINDOW) && defined(__WXMAC__)
		// MAC
		// convert handle (NSView) to (NSWindow)
		void *window = GetWindowForView(pWindowHandle);

		m_pScreen = SDL_CreateWindowFrom(window);
#else
		// OTHERS
		// I can request a specific window size, but SDL2 on windows on the laptop
		// it will resize the window to fit within the desktop size although the position remains unchanged.
		//
		WindowWidthUsed = pWindowed->m_nWindowWidth;
		WindowHeightUsed = pWindowed->m_nWindowHeight;
		int clientX;
		int clientY;
		int clientWidth;
		int clientHeight;

		// get desktop size...
		// wxwidgets gets a better size
		::wxClientDisplayRect(&clientX, &clientY, &clientWidth, &clientHeight);

		SDL_DisplayMode desktopMode;
		SDL_GetDesktopDisplayMode(0, &desktopMode);
		 wxGetApp().Log("SDL Desktop: w: %d h: %d refresh: %d\n", desktopMode.w, desktopMode.h, desktopMode.refresh_rate);
		 wxGetApp().Log("WX Desktop: x: %d y: %d w: %d h: %d\n", clientX, clientY, clientWidth, clientHeight);

		WindowX = SDL_WINDOWPOS_UNDEFINED;
		WindowY = SDL_WINDOWPOS_UNDEFINED;
		// the used window width/height we got may be smaller than the size we want, so we need to crop the image
		if (clientWidth < WindowWidthUsed)
		{
			WindowX = 0;
			WindowOffsetX = (WindowWidthUsed - clientWidth) >> 1;
			WindowWidthUsed = clientWidth;
		}
		if (clientHeight < WindowHeightUsed)
		{
			WindowY = 0;
			WindowOffsetY = (WindowHeightUsed - clientHeight) >> 1;
			WindowHeightUsed = clientHeight;
		}

		m_pScreen = SDL_CreateWindow("Arnold - Emulation Window", WindowX, WindowY, WindowWidthUsed, WindowHeightUsed, SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN);
		if (m_pScreen)
		{
			SDL_SetWindowMinimumSize(m_pScreen, WindowWidthUsed, WindowHeightUsed);
			SDL_SetWindowMaximumSize(m_pScreen, WindowWidthUsed, WindowHeightUsed);

			int WinW, WinH;
			SDL_GetWindowSize(m_pScreen, &WinW, &WinH);
			 wxGetApp().Log("Window width got: %d %d\n", WinW, WinH);

		}
#endif  

		if (m_pScreen == NULL)
		{
			 wxGetApp().Log("failed to create window\n");
			 wxGetApp().Log("SDL2 error: %s\n", SDL_GetError());
			return;
		}
#if 0
		//Request OpenGL 3.2 context.
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

		SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,0);

		//set double buffer
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		m_nWidth = Width;
		m_nHeight = Height;

		// create context on this window
		m_pContext = SDL_GL_CreateContext(m_pScreen);

		if (m_pContext==NULL)
		{
			 wxGetApp().Log("failed to create opengl context\n");
			 wxGetApp().Log("SDL2 error: %s\n", SDL_GetError());

		}
		int format;
		int accepted;

		/* Setup OpenGL parameters */
		glShadeModel(GL_SMOOTH);
		glDisable(GL_CULL_FACE);
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		/* The three OpenGL transformations */
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		/* Set distance between eye and viewplane, simply the width of the screen */
		sdl_eyedis=Width;
		/* The CPC screen is drawn at five times this depth by default, so that it can zoom
		   in closer before being clipped. sdl_eyedis-1 is for nVIDIA cards which clip too soon */
		sdl_scrposz=sdl_eyedis*SDL_TEXSCALE;
		glFrustum(-Width*0.5,Width*0.5,-Height*0.5,Height*0.5,sdl_eyedis-1,sdl_eyedis+SDL_CLIPZ);
		// wxGetApp().Log("Frustum=%f,%f,%f,%f,%d,%d\n",-Width*0.5,Width*0.5,-Height*0.5,Height*0.5,sdl_eyedis,sdl_eyedis+SDL_CLIPZ);
		glViewport(0,0,(GLsizei)Width,(GLsizei)Height);
		glClearColor(0,0,0,0);
		glDrawBuffer(GL_FRONT);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawBuffer(GL_BACK);
		glClear(GL_COLOR_BUFFER_BIT);
		/* Define texture which will contain the CPC screen */
		/* Warning: The order of the RGB components is flipped to BGR to match texture upload */
		sdl_texbuffer=SDL_CreateRGBSurface(SDL_SWSURFACE,SDL_TEXLENX,SDL_TEXLENY,32,0x00ff0000,0x0000ff00,0x000000ff,0xff000000);
		glGenTextures(1,&sdl_texture);
		glBindTexture(GL_TEXTURE_2D,sdl_texture);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		/* Determine best format for textures and upload */
		format=GL_RGBA8;
		do {
			glTexImage2D(GL_PROXY_TEXTURE_2D,0,format,SDL_TEXLENX,SDL_TEXLENY,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
			glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D,0,GL_TEXTURE_INTERNAL_FORMAT,&accepted);
			if (!accepted) {
				if (format==GL_RGBA8) format=GL_RGB5_A1;
				else {
					f wxGetApp().Log(stderr,"Cannot allocate OpenGL texture");
					exit(1);
				}
			}
		} while (!accepted);
		glTexImage2D(GL_TEXTURE_2D, 0, format, SDL_TEXLENX, SDL_TEXLENY, 0, GL_RGBA, GL_UNSIGNED_BYTE, sdl_texbuffer->pixels);
#endif

		 wxGetApp().Log("Creating renderer\n");

#if defined(GTK2_EMBED_WINDOW) && defined(__WXGTK__)
		// stops hang using software renderer
		m_pRenderer = SDL_CreateRenderer(m_pScreen, -1, SDL_RENDERER_SOFTWARE); 
#elif defined(MAC_EMBED_WINDOW) && defined(__WXMAC__)
		// on mac, software or hardware can be used
		m_pRenderer = SDL_CreateRenderer(m_pScreen, -1, SDL_RENDERER_SOFTWARE);
#else
		// not in a window
		m_pRenderer = SDL_CreateRenderer(m_pScreen, -1, 0);
#endif
		if (m_pRenderer == NULL)
		{
			 wxGetApp().Log("failed to create renderer\n");
			 wxGetApp().Log("SDL2 error: %s\n", SDL_GetError());
			return;
		}
		SDL_RendererInfo renderInfo;
		SDL_GetRendererInfo(m_pRenderer, &renderInfo);
		 wxGetApp().Log("Renderer: %s\n", renderInfo.name);
		 wxGetApp().Log("Renderer flags: %04x\n", renderInfo.flags);
		 wxGetApp().Log("Renderer num texture formats: %d\n", renderInfo.num_texture_formats);
		for (size_t i = 0; i < renderInfo.num_texture_formats; i++)
		{
			 wxGetApp().Log("Renderer texture format: %d\n", renderInfo.texture_formats[i]);
		}
		 wxGetApp().Log("Render Width: %d Render Height: %d\n", pWindowed->m_nRenderWidth, pWindowed->m_nRenderHeight);


		int nTextureWidth = pWindowed->m_nRenderWidth;
		int nTextureHeight = pWindowed->m_nRenderHeight;
		if (nTextureWidth < 256)
		{
			nTextureWidth = 256;
		}
		else if (nTextureWidth < 512)
		{
			nTextureWidth = 512;
		}
		else if (nTextureWidth < 1024)
		{
			nTextureWidth = 1024;
		}
		else if (nTextureWidth < 2048)
		{
			nTextureWidth = 2048;
		}
		if (nTextureHeight < 256)
		{
			nTextureHeight = 256;
		}
		else if (nTextureHeight < 512)
		{
			nTextureHeight = 512;
		}
		else if (nTextureHeight < 1024)
		{
			nTextureHeight = 1024;
		}
		else if (nTextureHeight < 2048)
		{
			nTextureHeight = 2048;
		}

		// these are not quite right when window size is huge.
		// 

		int Scale = (CPC_GetWindowScale() / 100);

		BufferInfo.CPCOffsetX = (WindowOffsetX / Scale);
		BufferInfo.CPCOffsetY = (WindowOffsetY / Scale);
		BufferInfo.SurfaceWidth = pWindowed->m_nRenderWidth - ((WindowOffsetX / Scale) << 1);
		BufferInfo.SurfaceHeight = pWindowed->m_nRenderHeight - ((WindowOffsetY / Scale) << 1);

		/* source rect is a part of the texture if the power of two texture is larger than the
		render rect*/
		SourceRect.x = 0;
		SourceRect.y = 0;
		SourceRect.w = BufferInfo.SurfaceWidth;
		SourceRect.h = BufferInfo.SurfaceHeight;

		DestinationRect.x = 0;
		DestinationRect.y = 0;
		DestinationRect.w = WindowWidthUsed;
		DestinationRect.h = WindowHeightUsed;

		 wxGetApp().Log("Texture Width: %d Height: %d\n", nTextureWidth, nTextureHeight);
		 wxGetApp().Log("Source rect: x: %d y: %d w: %d h: %d\n", SourceRect.x, SourceRect.y, SourceRect.w, SourceRect.h);
		 wxGetApp().Log("Dest rect: x: %d y: %d w: %d h: %d\n", DestinationRect.x, DestinationRect.y, DestinationRect.w, DestinationRect.h);

		m_pTexture = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, nTextureWidth, nTextureHeight);
		 wxGetApp().Log("Creating texture\n");
		if (m_pTexture == NULL)
		{
			 wxGetApp().Log("failed to create texture\n");
			 wxGetApp().Log("SDL2 error: %s\n", SDL_GetError());
		}

		SDL_ShowCursor(SDL_ENABLE);
		 wxGetApp().Log("Windowed mode obtained\n");
		}
	else
	{
		//
		//***     FULLSCREEN MODE     ***
		//

		// when we switch to full-screen we are detached from the wxwidgets window and we don't appear to get key presses like return
		// when in fullscreen.

		// which mode do we want?
		int nModeCode = wxGetApp().GetFullScreenMode();

		// -1 means use desktop mode

		int displayIndex = 0;

		SDL_DisplayMode mode;
		switch (nModeCode)
		{
		case -1:
			// this is the resolution of the desktop on the monitor we want to use
			// note setting the window size in sdl doesn't resize the wxwidgets window
			SDL_GetDesktopDisplayMode(displayIndex, &mode);
			break;
		case -2:


			// perhaps we need to iterate through them and select best

			// we need to decide on format!

			// we want this mode
			SDL_DisplayMode Wanted;
			Wanted.w = 800;
			Wanted.h = 600;
			Wanted.format = 0;
			Wanted.refresh_rate = 50;
			Wanted.driverdata = 0;

			// which display?
			SDL_GetClosestDisplayMode(displayIndex, &Wanted, &mode);
			break;
		default:
			int nDisplay = nModeCode >> 16;
			int nMode = nModeCode & 0x0ffff;
			SDL_GetDisplayMode(nDisplay, nMode, &mode);
			break;
		}
		if (nModeCode != -1)
		{
			//	 wxGetApp().Log("Fullscreen mode obtained: %d %d %d\n", mode.w,mode.h,mode.refresh_rate);
		}

		int tw = mode.w;
		int th = mode.h;
		int nFlags = SDL_WINDOW_OPENGL;
		if (nModeCode == -1)
		{
			nFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}
		else
		{
			// the size we got.
			nFlags |= SDL_WINDOW_FULLSCREEN;
		}
		// if it's desktop, what size does that need to be?
		m_pScreen = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, mode.w, mode.h, nFlags);

		if (m_pScreen == NULL)
		{
			 wxGetApp().Log("failed to create window\n");
			 wxGetApp().Log("SDL2 error: %s\n", SDL_GetError());

		}



		// if (SDL_SetWindowDisplayMode(m_pScreen, &mode)!=0)
		// {
		//     wxGetApp().Log("Failed to set fullscreen display mode\n");
		// }
		// we can pass 0 for the flags
		m_pRenderer = SDL_CreateRenderer(m_pScreen, -1, 0);	//SDL_RENDERER_ACCELERATED);  //|SDL_WINDOW_OPENGL );
		if (m_pRenderer == NULL)
		{
			 wxGetApp().Log("failed to create renderer\n");
			 wxGetApp().Log("SDL2 error: %s\n", SDL_GetError());
		}
		 wxGetApp().Log("tw: %d th: %d\n", tw, th);
		// the texture doesn't need to be and may not be the size of the actual desktop if in desktop mode.
		m_pTexture = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, tw, th);
		if (m_pTexture == NULL)
		{
			 wxGetApp().Log("failed to create texture\n");
			 wxGetApp().Log("SDL2 error: %s\n", SDL_GetError());
		}
	}

	//***********************
	//			common
	//***********************
#ifdef USE_OSD
	//initialise OSD BEFORE Texture filtering enable
	m_OSDDisplay.Init(m_pRenderer,m_pScreen);
	m_OSDDisplay.SetInfoMessage("Test for info message : with autowrap ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ");
	m_OSDDisplay.ToogleOSDMenu();
#endif
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.

	//SDL_RenderSetLogicalSize(sdlRenderer, 640, 480);

	if (m_pTexture)
	{
		Uint32 format;
		int access;
		int w;
		int h;

		if (SDL_QueryTexture(m_pTexture, &format, &access, &w, &h) == 0)
		{
			 wxGetApp().Log("format: %d\naccess: %d\nwidth: %d\nheight: %d\n", format, access, w, h);
		}
		else
		{
			 wxGetApp().Log("SDL2 error: %s\n", SDL_GetError());
		}
		int bpp;
		Uint32 Rmask;
		Uint32 Gmask;
		Uint32 Bmask;
		Uint32 Amask;
		SDL_PixelFormatEnumToMasks(format, &bpp, &Rmask, &Gmask, &Bmask, &Amask);
		 wxGetApp().Log("R mask: %04x G mask: %04x B mask: %04x A mask: %04x\n", Rmask, Gmask, Bmask, Amask);

		GRAPHICS_BUFFER_COLOUR_FORMAT BufferColourFormat;
		BufferColourFormat.Red.Mask = Rmask;
		BufferColourFormat.Red.BPP = CalcBPPFromMask(Rmask);
		BufferColourFormat.Red.Shift = CalcShiftFromMask(Rmask);

		BufferColourFormat.Green.Mask = Gmask;
		BufferColourFormat.Green.BPP = CalcBPPFromMask(Gmask);
		BufferColourFormat.Green.Shift = CalcShiftFromMask(Gmask);

		BufferColourFormat.Blue.Mask = Bmask;
		BufferColourFormat.Blue.BPP = CalcBPPFromMask(Bmask);
		BufferColourFormat.Blue.Shift = CalcShiftFromMask(Bmask);

		BufferColourFormat.BPP = bpp;

		Render_SetGraphicsBufferColourFormat(&BufferColourFormat, pWindowed->m_nRenderWidth, pWindowed->m_nRenderHeight);

	}
	// this scales to the window
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
	//SDL_RenderSetLogicalSize(m_pRenderer, SourceRect.w, SourceRect.h);
	// SDL_RenderSetLogicalSize causes problems if the window doesn't fit the display and we crop it.

	SDL_ShowCursor(SDL_DISABLE);

	SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// this is for the translated keyboard mode
//
// this will translate a character/key into a CPC equivalent.
//
// TODO: Approximate some accented characters into non-accented equivalent?
typedef struct
{
	unsigned int     m_nKeyCode;    // keycode
	const char *TranslatedChar;
} PlatformSpecificTranslation;

static PlatformSpecificTranslation TranslatedKeys[] =
{
	{ SDL_SCANCODE_BACKSPACE, "\b" },
	{ SDL_SCANCODE_TAB, "\t" },
	{ SDL_SCANCODE_RETURN, "\r" },
	{ SDL_SCANCODE_ESCAPE, "\x1b" },

	{ SDL_SCANCODE_UP, "\x0" },
	{ SDL_SCANCODE_DOWN, "\x1" },
	{ SDL_SCANCODE_LEFT, "\x2" },
	{ SDL_SCANCODE_RIGHT, "\x3" },

#if 0
	// if translated and num lock is on this generates a number
	// otherwise keys generate other responses
	{false, {SDLK_KP0}, KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH|KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_F1, false, false},
	{false, {SDLK_KP1}, KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH|KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_F1, false, false},
	{false, {SDLK_KP2}, KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH|KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_F2, false, false},
	{false, {SDLK_KP3}, KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH|KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_F3, false, false},
	{false, {SDLK_KP4}, KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH|KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_F4, false, false},
	{false, {SDLK_KP5}, KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH|KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_F5, false, false},
	{false, {SDLK_KP6}, KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH|KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_F6, false, false},
	{false, {SDLK_KP7}, KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH|KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_F7, false, false},
	{false, {SDLK_KP8}, KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH|KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_F8, false, false},
	{ false, { SDLK_KP9 }, KEYBOARD_LANGUAGE_ID_ENGLISH | KEYBOARD_LANGUAGE_ID_SPANISH | KEYBOARD_LANGUAGE_ID_DANISH | KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_F9, false, false },
#endif //
};


//=====================================================================

void RestrictMouseCursor()
{
	// grab mouse or not
	SDL_SetRelativeMouseMode(SDL_TRUE);
}


//**********************************************************************
//Display Info message, in window or on screen
void PlatformSpecific::DisplayMessage(wxString s_Text)
{
	if (m_bFullScreenActive)
	{
#ifdef USE_OSD
		m_OSDDisplay.SetInfoMessage(s_Text);
#endif
}
	else
	{
		wxMessageDialog *pDialog = new wxMessageDialog(NULL, s_Text, wxT(""), wxOK);
		pDialog->ShowModal();
		delete pDialog;
	}
}

//=====================================================================

void PlatformSpecific::DrawDisplay()
{
	//   wxGetApp().Log("drawing display\n");
	if ((m_pTexture == NULL) || (m_pRenderer == NULL))
		return;
	SDL_SetRenderDrawColor(m_pRenderer, 0, 0, 0, 255);

	// Clear the entire screen to our selected color.
	SDL_RenderClear(m_pRenderer);
#if 1

	void *pPixels = NULL;
	int nPitch = 0;
	if (SDL_LockTexture(m_pTexture, NULL, &pPixels, &nPitch) == 0)
	{
		int w, h;
		SDL_QueryTexture(m_pTexture, NULL, NULL, &w, &h);

		BufferInfo.pSurface = (unsigned char *)pPixels;
		BufferInfo.SurfacePitch = nPitch;

		if (pPixels != NULL)
		{

			/* dump whole display to screen */
			Render_DumpDisplay(&BufferInfo);
		}

		int nResult = SDL_RenderCopy(m_pRenderer, m_pTexture, &SourceRect, &DestinationRect);
		if (nResult != 0)
		{
			 wxGetApp().Log("failed to copy");
		}

#ifdef USE_OSD
		//update OSD
		m_OSDDisplay.OnRender(m_pRenderer);
#endif
		SDL_UnlockTexture(m_pTexture);
	}
	else
	{
		 wxGetApp().Log("Failed to lock texture %s\n", SDL_GetError());
	}

	// if power down is done and then restored the program will assert here.

	// perform flip

	SDL_RenderPresent(m_pRenderer);
#else

	unsigned char coltab[4];
	int i,j,sign;
	Uint32 color;
	Uint32 *pixels;
	float posx,posy,startx,starty,endx,endy,nativeratio,cpcratio,f;
	SDL_PixelFormat *format;

	{
		/* Calculate new Z-position of CPC window (can be zoomed with Ctrl+cursor keys) */
		sdl_scrposz+=sdl_zoomspeed*5*SDL_TEXSCALE;
		if (sdl_scrposz<sdl_eyedis) sdl_scrposz=sdl_eyedis;
		if (sdl_scrposz>sdl_eyedis*2*SDL_TEXSCALE) sdl_scrposz=sdl_eyedis*2*SDL_TEXSCALE;
		/* Determine position of CPC screen */
		nativeratio=(float)m_nWidth/m_nHeight;
		cpcratio=(float)SDL_SCRLENX/SDL_SCRLENY;
		if (nativeratio>cpcratio) {
			/* Native screen is wider than CPC screen */
			posy=m_nHeight*SDL_TEXSCALE*0.5;
			posx=posy*cpcratio;
		} else {
			/* Native screen is higher than CPC screen */
			posx=m_nWidth*SDL_TEXSCALE*0.5;
			posy=posx/cpcratio;
		}
		/* Determine background color: See if three pixel columns at the left and right
		   side of the CPC screen all have the same color (no overscan) */
		color=*(Uint32*)sdl_texbuffer->pixels;
		for (i=0;i<2;i++) {
			for (j=0;j<SDL_SCRLENY;j++) {
				pixels=(Uint32*)((char*)sdl_texbuffer->pixels+j*sdl_texbuffer->pitch)+i*(SDL_SCRLENX-3);
				if (pixels[0]!=color||pixels[1]!=color||pixels[2]!=color) {
					/* Color differs */
					color=0;
					break;
				}
			}
		}
		 wxGetApp().Log("Color=%x\n",color);
		/* Clear screen */
		glBegin(GL_QUADS);
		SDL_GetRGBA(color,sdl_texbuffer->format,coltab,coltab+1,coltab+2,coltab+3);
		glColor3f(coltab[0]/255.,coltab[1]/255.,coltab[2]/255.);
		glVertex3f(m_nWidth,m_nHeight,-sdl_eyedis);
		glVertex3f(-m_nWidth,m_nHeight,-sdl_eyedis);
		glVertex3f(-m_nWidth,-m_nHeight,-sdl_eyedis);
		glVertex3f(m_nWidth,-m_nHeight,-sdl_eyedis);
		glEnd();
		/* Upload texture */
		// wxGetApp().Log("TexSubImage %d...\n",sdl_texture);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,sdl_texture);
		glPixelStorei(GL_UNPACK_ROW_LENGTH,SDL_TEXLENX);
		glTexSubImage2D(GL_TEXTURE_2D,0,0,0,SDL_SCRLENX,SDL_SCRLENY,GL_RGBA,GL_BGRAFORMAT,sdl_texbuffer->pixels);
		/* Draw texture, clipping 1 pixel on each side to prevent the linear texture filter
		   from accessing pixels outside the CPC screen */
		startx=1./SDL_TEXLENX;
		starty=1./SDL_TEXLENY;
		endx=(float)(SDL_SCRLENX-1)/SDL_TEXLENX;
		endy=(float)(SDL_SCRLENY-1)/SDL_TEXLENY;
		glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
		glBegin(GL_QUADS);
		glTexCoord2f(endx,starty);
		glVertex3f(posx,posy,-sdl_scrposz);
		glTexCoord2f(startx,starty);
		glVertex3f(-posx,posy,-sdl_scrposz);
		glTexCoord2f(startx,endy);
		glVertex3f(-posx,-posy,-sdl_scrposz);
		glTexCoord2f(endx,endy);
		glVertex3f(posx,-posy,-sdl_scrposz);
		glEnd();
		glDisable(GL_TEXTURE_2D);
#if 0
		if (sdl_warpfacdisptime>0) {
			/* The warp factor should be displayed on screen */
			sdl_warpfacdisptime--;
			glBegin(GL_QUADS);
			startx=m_nWidth*0.45;
			endx=m_nWidth*0.48;
			for (i=0;i<cpc_warpfactor;i++) {
				starty=m_nHeight*(-0.45+i*0.05);
				endy=m_nHeight*(-0.45+i*0.05-0.03);
				f=(float)i/CPC_WARPFACTORMAX;
				glColor3f(1.0*f,1.0*(1-f),0);
				glVertex3f(endx,starty,-sdl_eyedis);
				glVertex3f(startx,starty,-sdl_eyedis);
				glVertex3f(startx,endy,-sdl_eyedis);
				glVertex3f(endx,endy,-sdl_eyedis);
			}
			glEnd();
		}
#endif
	}

	//    SDL_GL_SwapWindow(m_pScreen);
#endif
	// perform flip
	//		SDL_RenderPresent(m_pRenderer);
	//SDL_GL_SwapBuffers();

	//     wxGetApp().Log("end drawing display\n");

}




void PlatformSpecific::BeginPressedKey()
{

	// this shows the window; which is confusing
	// also it doesn't stop the events so we miss the press
	//SDL_WM_GrabInput(SDL_GRAB_ON);

	m_bDisableKeyboardEvents = true;
}
void PlatformSpecific::EndPressedKey()
{
	m_bDisableKeyboardEvents = false;
	//SDL_WM_GrabInput(SDL_GRAB_OFF);
}

void PlatformSpecific::DisableJoystickEvents()
{
	m_bDisableJoystickEvents = true;

	//	SDL_JoystickEventState(SDL_DISABLE);
	//	SDL_GameControllerEventState(SDL_DISABLE);
}

void PlatformSpecific::EnableJoystickEvents()
{
	m_bDisableJoystickEvents = false;

	//	SDL_JoystickEventState(SDL_ENABLE);
	//	SDL_GameControllerEventState(SDL_ENABLE);
}

//if axis > 10 >> it s POV instead of axes
void PlatformSpecific::HandleControllerAxis(SDL_JoystickID id, int axis, Sint16 value)
{
	// is key joy enabled?
	if (KeyJoy_IsActive() && (KeyJoy_GetPhysical() == (int)id))
	{
		signed int DeadZone = 12000;
		KEYJOY_AXIS_VALUE nValue = KEYJOY_AXIS_MID;
		if (value<-DeadZone)
		{
			nValue = KEYJOY_AXIS_MIN;
		}
		else if (value>DeadZone)
		{
			nValue = KEYJOY_AXIS_MAX;
		}


		int nSpecialAction = KeyJoy_GetSpecialActionAxis(axis, nValue);
		if (nSpecialAction != -1)
		{
			wxGetApp().ProcessAction((ACTION_CODE)nSpecialAction);
		}


		KeyJoy_UpdateKeyJoyAxisInput(axis, nValue);
	}

	int CPCJoystickID = Joystick_PhysicalToCPC((int)id);
	if (CPCJoystickID == -1)
	{
		return;
	}

	// map joypad axis to cpc joystick axis
	//normal Axis
	if (Joystick_GetAxisMappingPhysical(CPCJoystickID, 0) == axis)
	{
		Joystick_SetXMovement(CPCJoystickID, value);
	}
	else
		if (Joystick_GetAxisMappingPhysical(CPCJoystickID, 1) == axis)
		{
			Joystick_SetYMovement(CPCJoystickID, value);
		}
}

void PlatformSpecific::HandleControllerButtons(SDL_JoystickID id, int button, bool bPressed)
{
	// TODO: button is actually a bitmask?

	// is key joy enabled?
	if (KeyJoy_IsActive() && (KeyJoy_GetPhysical() == (int)id))
	{
		KeyJoy_UpdateKeyJoyButtonInput(button, bPressed);

		int nSpecialAction = KeyJoy_GetSpecialActionButton(button);
		if ((nSpecialAction != -1) && (bPressed))
		{
			wxGetApp().ProcessAction((ACTION_CODE)nSpecialAction);
		}
	}

	int CPCJoystickID = Joystick_PhysicalToCPC((int)id);
	if (CPCJoystickID == -1)
	{
		return;
	}

	// TODO: Change it so we can set any joystick input, so a button on the pad can trigger up/down/left/right too.

	// map joypad button to cpc joystick button
	int RemappedButton = Joystick_GetButtonMapping(CPCJoystickID, button);
	if (RemappedButton != -1)
	{
		Joystick_SetButton(CPCJoystickID, RemappedButton, bPressed);
	}
}



void PlatformSpecific::HandleEvents()
{
	{
		SDL_Event event;
		while (SDL_PollEvent(&event) != 0)
		{

			switch (event.type)
			{
			case SDL_DROPFILE:
			{
				if (event.drop.file)
				{
					wxString sFilename(event.drop.file);
					wxGetApp().HandleDropFile(sFilename);
					SDL_free(event.drop.file);
				}
			}
			break;

			case SDL_TEXTINPUT:
			{
				if (m_bDisableKeyboardEvents)
					break;

				// doesn't capture tab, return and chars like that
				// but does capture most symbols
				if (Keyboard_GetMode() == 1)
				{
					SDL_TextInputEvent &TextInputEvent = event.text;
					const char *sUTF8String = TextInputEvent.text;

					// this will not work with accented keys
					wxGetApp().SetTranslatedKey(sUTF8String);
				}
			}
			break;

			case SDL_KEYDOWN:
			case SDL_KEYUP:
			{
				if (m_bDisableKeyboardEvents)
					break;

				if (Keyboard_GetMode() == 0)
				{
					// may need to use scancode!
					SDL_KeyboardEvent &KeyEvent = event.key;
					HandleKey(&KeyEvent);
				}
				else if (Keyboard_GetMode() == 1)
				{
					// keys that don't get passed to text input which we translate
					if (event.type == SDL_KEYDOWN)
					{
						SDL_KeyboardEvent &KeyEvent = event.key;
						SDL_Keysym	*keysym = &KeyEvent.keysym;
						SDL_Scancode keycode = keysym->scancode;
						for (size_t i = 0; i < (sizeof(TranslatedKeys) / sizeof(TranslatedKeys[0])); i++)
						{
							if (TranslatedKeys[i].m_nKeyCode == keycode)
							{
								// this will not work with accented keys
								wxGetApp().SetTranslatedKey(TranslatedKeys[i].TranslatedChar);
								break;
							}
						}
					}
				}
				else
				{
					 wxGetApp().Log("Unsupported keyboard mode!");
				}


					{
						SDL_KeyboardEvent &KeyEvent = event.key;
						SDL_Keysym	*keysym = &KeyEvent.keysym;
						SDL_Scancode	keycode = keysym->scancode;

						// process action keys
						if (event.type == SDL_KEYDOWN)
						{
							for (size_t i = 0; i < sizeof(ActionKeys) / sizeof(ActionKeys[0]); i++)
							{
								if (
									// keycodes
									((int)ActionKeys[i].m_nKeyCode == (int)keycode) &&
									// now check modifiers
									(
									// shift pressed
									(ActionKeys[i].m_bShift && ((keysym->mod & (KMOD_LSHIFT | KMOD_RSHIFT)) != 0)) ||
									// shift not pressed
									(!ActionKeys[i].m_bShift && ((keysym->mod & (KMOD_LSHIFT | KMOD_RSHIFT)) == 0))
									) &&
									(
									// control pressed
									(ActionKeys[i].m_bControl && ((keysym->mod & (KMOD_LCTRL | KMOD_RCTRL)) != 0)) ||
									// control not pressed
									(!ActionKeys[i].m_bControl && ((keysym->mod & (KMOD_LCTRL | KMOD_RCTRL)) == 0))
									) &&
									(
									(ActionKeys[i].m_bAlt && ((keysym->mod & (KMOD_LALT | KMOD_RALT)) != 0)) ||
									(!ActionKeys[i].m_bAlt && ((keysym->mod & (KMOD_LALT | KMOD_RALT)) == 0))
									)
									)
								{
									wxGetApp().ProcessAction(ActionKeys[i].m_Action);
									break;
								}
							}
						}
					}





					/* handle joysticks simulated by keys */
						{
							SDL_KeyboardEvent &KeyEvent = event.key;
							SDL_Keysym	*keysym = &KeyEvent.keysym;
							SDL_Scancode	keycode = keysym->scancode;
							for (int i = 0; i < CPC_NUM_JOYSTICKS; i++)
							{
								if ((Joystick_GetType(i) == JOYSTICK_TYPE_SIMULATED_BY_KEYBOARD) && (Joystick_IsActive(i)))
								{
									for (int j = 0; j < JOYSTICK_SIMULATED_KEYID_LAST; j++)
									{
										if (keycode == Joystick_GetSimulatedKeyID(i, j))
										{
											Joystick_SetSimulatedKeyIDState(i, j, (event.type == SDL_KEYDOWN));
										}
									}
								}
							}
						}

			}
			break;

			case SDL_CONTROLLERDEVICEADDED:
			{
				Sint32 which = event.cdevice.which;
				 wxGetApp().Log("SDL controller device added %d\n", which);
			}
			break;

			case SDL_CONTROLLERDEVICEREMOVED:
			{
				Sint32 which = event.cdevice.which;
				 wxGetApp().Log("SDL controller device removed %d\n", which);
			}
			break;

			case SDL_CONTROLLERDEVICEREMAPPED:
			{
				Sint32 which = event.cdevice.which;
				 wxGetApp().Log("SDL controller device remapped %d\n", which);
			}
			break;

			case SDL_JOYDEVICEADDED:
			{
				Sint32 which = event.jdevice.which;
				 wxGetApp().Log("SDL joystick device added %d\n", which);
			}
			break;

			case SDL_JOYDEVICEREMOVED:
			{
				Sint32 which = event.jdevice.which;
				 wxGetApp().Log("SDL joy device removed %d\n", which);
			}
			break;

			case SDL_CONTROLLERAXISMOTION:
			{
				if (m_bDisableJoystickEvents)
					break;

				if (!m_bHasJoystick)
				{
					break;
				}
				SDL_ControllerAxisEvent &ControllerEvent = event.caxis;

				HandleControllerAxis(ControllerEvent.which, ControllerEvent.axis, ControllerEvent.value);
			}
			break;

			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
			{
				if (m_bDisableJoystickEvents)
					break;

				if (!m_bHasJoystick)
				{
					break;
				}
				SDL_ControllerButtonEvent &ControllerEvent = event.cbutton;
				HandleControllerButtons(ControllerEvent.which, ControllerEvent.button, (ControllerEvent.state == SDL_PRESSED));

				// doesn't seem to be responsive enough?
				int value;

				if (
					(ControllerEvent.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT) ||
					(ControllerEvent.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT)
					)
				{
					// allow digital to be input to axis for 

					// handle left/right

					value = 0;

					// assume left/right can't be down at the same time.
					//

					if (ControllerEvent.state == SDL_PRESSED)
					{
						if (ControllerEvent.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT)
						{
							value = -32767;
						}
						else if (ControllerEvent.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT)
						{
							value = 32767;
						}
					}


					int CPCJoystickID = Joystick_PhysicalToCPC((int)ControllerEvent.which);
					if (CPCJoystickID != -1)
					{
						Joystick_SetXMovement(CPCJoystickID, value);
					}
				}

				if (
					(ControllerEvent.button == SDL_CONTROLLER_BUTTON_DPAD_UP) ||
					(ControllerEvent.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN)
					)
				{
					value = 0;

					// assume left/right can't be down at the same time.
					//

					if (ControllerEvent.state == SDL_PRESSED)
					{
						if (ControllerEvent.button == SDL_CONTROLLER_BUTTON_DPAD_UP)
						{
							value = -32767;
						}
						else if (ControllerEvent.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN)
						{
							value = 32767;
						}
					}

					int CPCJoystickID = Joystick_PhysicalToCPC((int)ControllerEvent.which);
					if (CPCJoystickID != -1)
					{
						Joystick_SetYMovement(CPCJoystickID, value);
					}
				}

			}
			break;
			case SDL_QUIT:
			{
				 wxGetApp().Log("quit event\n");
				wxGetApp().ProcessAction(ACTION_QUIT);

			}
			break;

			case SDL_WINDOWEVENT:
			{
				switch (event.window.event) {
				case SDL_WINDOWEVENT_SHOWN:
					SDL_Log("Window %d shown", event.window.windowID);
					break;
				case SDL_WINDOWEVENT_HIDDEN:
					SDL_Log("Window %d hidden", event.window.windowID);
					break;
				case SDL_WINDOWEVENT_EXPOSED:
					SDL_Log("Window %d exposed", event.window.windowID);
					break;
				case SDL_WINDOWEVENT_MOVED:
					SDL_Log("Window %d moved to %d,%d",
						event.window.windowID, event.window.data1,
						event.window.data2);
					break;
				case SDL_WINDOWEVENT_RESIZED:
					SDL_Log("Window %d resized to %dx%d",
						event.window.windowID, event.window.data1,
						event.window.data2);
					break;
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					SDL_Log("Window %d size changed to %dx%d",
						event.window.windowID, event.window.data1,
						event.window.data2);
					break;
				case SDL_WINDOWEVENT_MINIMIZED:
					SDL_Log("Window %d minimized", event.window.windowID);
					break;
				case SDL_WINDOWEVENT_MAXIMIZED:
					SDL_Log("Window %d maximized", event.window.windowID);
					break;
				case SDL_WINDOWEVENT_RESTORED:
					SDL_Log("Window %d restored", event.window.windowID);
					break;
				case SDL_WINDOWEVENT_ENTER:
					SDL_Log("Mouse entered window %d",
						event.window.windowID);
					break;
				case SDL_WINDOWEVENT_LEAVE:
					SDL_Log("Mouse left window %d", event.window.windowID);
					break;
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					SDL_Log("Window %d gained keyboard focus",
						event.window.windowID);
					break;
				case SDL_WINDOWEVENT_FOCUS_LOST:
					SDL_Log("Window %d lost keyboard focus",
						event.window.windowID);
					break;
				case SDL_WINDOWEVENT_CLOSE:
					SDL_Log("Window %d closed", event.window.windowID);
					break;
				default:
					SDL_Log("Window %d got unknown event %d",
						event.window.windowID, event.window.event);
					break;
				}
			}
			break;


			case SDL_JOYAXISMOTION:
			{
				if (m_bDisableJoystickEvents)
					break;

				if (!m_bHasJoystick)
				{
					break;
				}

				/* SDL joystick index */
				SDL_JoyAxisEvent &ControllerEvent = event.jaxis;

				int index = JoystickFromID(ControllerEvent.which);

				if (SDL_IsGameController(index))
				{
					break;
				}

				HandleControllerAxis(ControllerEvent.which, ControllerEvent.axis, ControllerEvent.value);
			}
			break;

			//SDL_JOYBALLMOTION

			/* game controller doesn't appear to send hat motion events */
			case SDL_JOYHATMOTION:
			{
				if (m_bDisableJoystickEvents)
					break;

				if (!m_bHasJoystick)
				{
					break;
				}

				/* SDL joystick index */
				SDL_JoyHatEvent &ControllerEvent = event.jhat;

				int index = JoystickFromID(ControllerEvent.which);

				if (SDL_IsGameController(index))
				{
					break;
				}


				/* get the hat information and send it on */
				int xvalue = 0;
				int yvalue = 0;
				if ((ControllerEvent.value & SDL_HAT_LEFT) != 0)
				{
					xvalue = -32767;
				}
				else if ((ControllerEvent.value & SDL_HAT_RIGHT) != 0)
				{
					xvalue = 32767;
				}

				if ((ControllerEvent.value & SDL_HAT_UP) != 0)
				{
					yvalue = -32767;
				}
				else if ((ControllerEvent.value & SDL_HAT_DOWN) != 0)
				{
					yvalue = 32767;
				}
				 wxGetApp().Log("Joystick %d Hat %d x: %d y: %d\n", ControllerEvent.which, ControllerEvent.hat, xvalue, yvalue);


				// is key joy enabled and uses this physical joystick?
				if (KeyJoy_IsActive() && (KeyJoy_GetPhysical() == index))
				{
					int DeadZone = 5000;
					KEYJOY_AXIS_VALUE nValue;

					nValue = KEYJOY_AXIS_MID;
					if (xvalue<-DeadZone)
					{
						nValue = KEYJOY_AXIS_MIN;
					}
					else if (xvalue>DeadZone)
					{
						nValue = KEYJOY_AXIS_MAX;
					}

					int nSpecialAction = KeyJoy_GetSpecialActionHat(ControllerEvent.hat, 0, nValue);
					if (nSpecialAction != -1)
					{
						wxGetApp().ProcessAction((ACTION_CODE)nSpecialAction);
					}

					KeyJoy_UpdateKeyJoyHatInput(ControllerEvent.hat, 0, nValue);


					nValue = KEYJOY_AXIS_MID;
					if (yvalue<-DeadZone)
					{
						nValue = KEYJOY_AXIS_MIN;
					}
					else if (yvalue>DeadZone)
					{
						nValue = KEYJOY_AXIS_MAX;
					}
					nSpecialAction = KeyJoy_GetSpecialActionHat(ControllerEvent.hat, 1, nValue);
					if (nSpecialAction != -1)
					{
						wxGetApp().ProcessAction((ACTION_CODE)nSpecialAction);
					}
					KeyJoy_UpdateKeyJoyHatInput(ControllerEvent.hat, 1, nValue);
				}
				// joystick also mapped to hat?
				int CPCJoystickID = Joystick_PhysicalToCPC((int)ControllerEvent.which);
				if (CPCJoystickID != -1)
				{
					Joystick_SetXMovement(CPCJoystickID, xvalue);
					Joystick_SetYMovement(CPCJoystickID, yvalue);
				}
			}
			break;
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
			{
				if (m_bDisableJoystickEvents)
					break;

				if (!m_bHasJoystick)
				{
					break;
				}

				SDL_JoyButtonEvent &JoyButtonEvent = event.jbutton;
				int index = JoystickFromID(JoyButtonEvent.which);

				if (SDL_IsGameController(index))
				{
					break;
				}
				HandleControllerButtons(JoyButtonEvent.which, JoyButtonEvent.button, (JoyButtonEvent.state == SDL_PRESSED));
			}
			break;

			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
			{
				SDL_MouseButtonEvent &MouseButtonEvent = event.button;
				if (MouseButtonEvent.which == SDL_TOUCH_MOUSEID)
				{
					 wxGetApp().Log("Mouse button comes from touchpad\n");
				}
				//               wxGetApp().Log("Mouse button %s\n", event.type==SDL_MOUSEBUTTONDOWN ? "down" : "up" );
			{
				for (int i = 0; i < CPC_NUM_JOYSTICKS; i++)
				{
					if (Joystick_GetType(i) == JOYSTICK_TYPE_SIMULATED_BY_MOUSE)
					{
						for (int j = 0; j < JOYSTICK_SIMULATED_MOUSEID_LAST; j++)
						{
							if (MouseButtonEvent.button == Joystick_GetSimulatedMouseID(i, j))
							{
								Joystick_SetSimulatedMouseIDState(i, j, (event.type == SDL_MOUSEBUTTONDOWN));
							}
						}
					}
				}
			}

			int nButton = 0;
			switch (MouseButtonEvent.button)
			{
			case SDL_BUTTON_LEFT:
				nButton = 0;
				break;

			case SDL_BUTTON_MIDDLE:
				nButton = 2;
				break;

			case SDL_BUTTON_RIGHT:
				nButton = 1;
				break;
			}

			// this is absolute
			Mouse_SetButtons(nButton, (event.type == SDL_MOUSEBUTTONDOWN));
			}
			break;

			case SDL_FINGERDOWN:
			case SDL_FINGERUP:
			case SDL_FINGERMOTION:
			{
				SDL_TouchFingerEvent &FingerEvent = event.tfinger;
				 wxGetApp().Log("Finger event\n");

				//					SDL_TouchID touchId; /**< The touch device id */
				//					SDL_FingerID fingerId;
				//					float x;            /**< Normalized in the range 0...1 */
				//					float y;            /**< Normalized in the range 0...1 */
				//					float dx;           /**< Normalized in the range 0...1 */
				//					float dy;           /**< Normalized in the range 0...1 */
				//					float pressure;     /**< Normalized in the range 0...1 */

				 wxGetApp().Log("X: %f Y: %f\n", FingerEvent.x, FingerEvent.y);

			}
			break;

			case SDL_MOUSEWHEEL:
			{
				SDL_MouseWheelEvent &MouseWheelEvent = event.wheel;
				if (MouseWheelEvent.which == SDL_TOUCH_MOUSEID)
				{
					 wxGetApp().Log("Mouse button comes from touchpad\n");
				}
				 wxGetApp().Log("Mouse wheel event\n");
			}
			break;


			case SDL_MOUSEMOTION:
			{
				// doesn't send "no motion"
				SDL_MouseMotionEvent &MouseMotionEvent = event.motion;
				if (MouseMotionEvent.which == SDL_TOUCH_MOUSEID)
				{
					 wxGetApp().Log("Mouse button comes from touchpad\n");
				}

				int w, h;
				SDL_GetWindowSize(m_pScreen, &w, &h);


				int xpos = MouseMotionEvent.x;
				int ypos = MouseMotionEvent.y;

				int dx = xpos - (w >> 1);
				int dy = ypos - (h >> 1);
				int deadzone = 50;

				int xr = 0;
				if (dx < -deadzone)
				{
					xr = -32767;
				}
				else if (dx > deadzone)
				{
					xr = 32767;
				}
				int yr = 0;
				if (dy < -deadzone)
				{
					yr = -32767;
				}
				else if (dy>deadzone)
				{
					yr = 32767;
				}
				for (int i = 0; i < CPC_NUM_JOYSTICKS; i++)
				{
					if (Joystick_GetType(i) == JOYSTICK_TYPE_SIMULATED_BY_MOUSE)
					{

						Joystick_SetXMovement(i, xr);
						Joystick_SetYMovement(i, yr);
					}
				}

#if 0
				 wxGetApp().Log("x: %d y: %d\n", xpos, ypos);
				// convert into cpc screen space
				CPC_SetLightSensorPos(xpos, ypos);

				int xrel = MouseMotionEvent.xrel;
				int yrel = MouseMotionEvent.yrel;
				int MAXMOVE = 10;
				int MINMOVE = 1;

				if ((xrel==0) && (yrel==0))
				{
					 wxGetApp().Log("zero\n");
				}

				// need to work out based on amount of movement.

			{
				for (int i=0; i<CPC_NUM_JOYSTICKS; i++)
				{
					if (Joystick_GetType(i)==JOYSTICK_TYPE_SIMULATED_BY_MOUSE)
					{
						int xr;
						int yr;

						if (xrel<0)
						{
							xr = xrel;
							if (xr>-MINMOVE)
							{
								xr = 0;
							}
							else if (xr<-MAXMOVE)
							{
								xr = -MAXMOVE;
							}
						}
						else if (xrel>0)
						{
							xr = xrel;
							if (xr<MINMOVE)
							{
								xr = 0;
							}
							else if (xr>MAXMOVE)
							{
								xr = MAXMOVE;
							}
						}
						else
						{
							xr = xrel;
						}

						if (yrel<0)
						{
							yr = yrel;
							if (yr>(-MINMOVE))
							{
								yr = 0;
							}
							else if (yr<-MAXMOVE)
							{
								yr = -MAXMOVE;
							}
						}
						else if (yrel>0)
						{
							yr = yrel;
							if (yr<MINMOVE)
							{
								yr = 0;
							}
							else if (yr>MAXMOVE)
							{
								yr = MAXMOVE;
							}
						}
						else
						{
							yr = yrel;
						}

						xr = (xr*32767)/MAXMOVE ;
						yr = (yr*32767)/MAXMOVE ;

						Joystick_SetXMovement(i, xr);
						Joystick_SetYMovement(i, yr);
					}
				}
			}

			// there could be more than one SDL event queued up

			// update the mouse movement based on this
			Mouse_AddMovement(xrel, yrel);
#endif
			Mouse_SetPosition(xpos, ypos);
			}
			break;

			case 	SDL_APP_TERMINATING:
			case 				SDL_APP_LOWMEMORY:
			case SDL_APP_WILLENTERBACKGROUND:
			case SDL_APP_DIDENTERBACKGROUND:
			case SDL_APP_WILLENTERFOREGROUND:
			case SDL_APP_DIDENTERFOREGROUND:
			case SDL_SYSWMEVENT:
			case SDL_TEXTEDITING:
			case SDL_KEYMAPCHANGED:
			case SDL_JOYBALLMOTION:

				case SDL_DOLLARGESTURE:
				case SDL_DOLLARRECORD:
				case SDL_MULTIGESTURE:
				case SDL_CLIPBOARDUPDATE:
			{

			}
			break;
									   /* Audio hotplug events */
				case SDL_AUDIODEVICEADDED:
			{
				printf("Audio device added\n");
			}
			break;
			
				case SDL_AUDIODEVICEREMOVED:
				{
					printf("Audio device removed\n");
				}
				break;
			case SDL_RENDER_TARGETS_RESET:
			{
				printf("render targets reset\n");
			}
			break;

			case SDL_RENDER_DEVICE_RESET:
			{
				printf("render device reset\n");
			}
			break;
			


			default:
			{
				 wxGetApp().Log("Unhandled SDL event %d", event.type);
			}
			break;

			}
		}
	}


}



//int *arnguiApp::GetKeySet()
//{
//    return ScanCodeToCPCKey;
//}



void PlatformSpecific::WriteKeyList(const  CPCKeyDefData *pData)
{
	ClearHostKeys();
	for (int i = 0; i < CPC_KEY_NUM_KEYS; i++)
	{
		SetHostKey((CPC_KEY_ID)i, &pData->Keys[i][0]);
	}
}

void PlatformSpecific::ClearHostKeys()
{
	for (int i = 0; i < sizeof(ScanCodeToCPCKeyUserDefined) / sizeof(ScanCodeToCPCKeyUserDefined[0]); i++)
	{
		ScanCodeToCPCKeyUserDefined[i] = CPC_KEY_NULL;
	}
}

void PlatformSpecific::SetHostKey(CPC_KEY_ID CPCKey, const int *Keys)
{
	for (int j = 0; j < MaxNumKeys; j++)
	{
		int ScanCode = Keys[j];
		if (ScanCode != CPC_KEY_DEF_UNSET_KEY)
		{
			ScanCodeToCPCKeyUserDefined[ScanCode] = CPCKey;
		}
	}
}

void PlatformSpecific::GetHostKey(CPC_KEY_ID CPCKey, int *Keys)
{
	int nKeys = 0;

	for (int j = 0; j < MaxNumKeys; j++)
	{
		Keys[j] = CPC_KEY_DEF_UNSET_KEY;
	}

	for (int i = 0; i < sizeof(ScanCodeToCPCKeyUserDefined) / sizeof(ScanCodeToCPCKeyUserDefined[0]); i++)
	{
		if (ScanCodeToCPCKeyUserDefined[i] == (int)CPCKey)
		{
			if (nKeys < MaxNumKeys)
			{
				Keys[nKeys] = i;
				nKeys++;
			}
		}
	}
}

void PlatformSpecific::SetupKeyList(CPCKeyDefData *pData)
{
	for (int i = 0; i < CPC_KEY_NUM_KEYS; i++)
	{
		GetHostKey((CPC_KEY_ID)i, pData->Keys[i]);
	}
}



void PlatformSpecific::SetKeySet(int layout)
{
	int i;

	int *SourceKeySet;

	switch (layout)
	{
	default:
	case 0:
	{
		SourceKeySet = ScanCodeToCPCKeyInternational;
	}
	break;
	case 1:
	{
		SourceKeySet = ScanCodeToCPCKeyAzerty;
	}
	break;
	case 2:
	{
		SourceKeySet = ScanCodeToCPCKeyQwertz;
	}
	break;
	case 3:
	{
		SourceKeySet = ScanCodeToCPCKeyUserDefined;
	}
	break;
	}

	memcpy(ScanCodeToCPCKeyActive, SourceKeySet, sizeof(ScanCodeToCPCKeyActive));

	int CPCKeys[CPC_KEY_NUM_KEYS];
	for (i = 0; i < CPC_KEY_NUM_KEYS; i++)
	{
		CPCKeys[i] = -1;
	}

	/* do not map joysticks here.. provide key sets! */
	CPCKeys[(9 * 8) + 0] = -2;
	CPCKeys[(9 * 8) + 1] = -2;
	CPCKeys[(9 * 8) + 2] = -2;
	CPCKeys[(9 * 8) + 3] = -2;
	CPCKeys[(9 * 8) + 4] = -2;
	CPCKeys[(9 * 8) + 5] = -2;
	CPCKeys[(9 * 8) + 6] = -2;		// unused

	// check all keys are mapped?
	for (i = 0; i < sizeof(ScanCodeToCPCKeyActive) / sizeof(ScanCodeToCPCKeyActive[0]); i++)
	{
		if (ScanCodeToCPCKeyActive[i] != CPC_KEY_NULL)
		{
			if (CPCKeys[ScanCodeToCPCKeyActive[i]] == -2)
			{
				 wxGetApp().Log("SDL2 key %d is mapped to joystick! ERROR: Joysticks are mapped differently!", (int)i);
				//            wxString sMessage;
				//            sMessage.Format(wxT("SDL2 key %d is mapped to joystick! ERROR: Use keyset do not map keys here!\r\n"), i);
				//                TraceMessage(sMessage);
			}
			else
			{
				CPCKeys[ScanCodeToCPCKeyActive[i]] = 0;
			}
		}
	}

	for (i = 0; i < CPC_KEY_NUM_KEYS; i++)
	{
		if (CPCKeys[i] == -1)
		{
			//          wxString sMessage;
			//           sMessage.Format(wxT("CPC Key %d has not been mapped\r\n"), i);
			//           TraceMessage(sMessage);
			 wxGetApp().Log("CPC Key %d (%s) has not been mapped\n", (int)i, CPC_GetKeyName((CPC_KEY_ID)i));
		}
	}

}

void	sdl_InitialiseKeyboardMapping_qwertz()
{

	for (int i = 0; i < SDL_NUM_SCANCODES; i++)
	{
		ScanCodeToCPCKeyQwertz[i] = CPC_KEY_NULL;
	}


	/* International key mappings */
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_0] = CPC_KEY_ZERO;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_1] = CPC_KEY_1;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_2] = CPC_KEY_2;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_3] = CPC_KEY_3;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_4] = CPC_KEY_4;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_5] = CPC_KEY_5;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_6] = CPC_KEY_6;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_7] = CPC_KEY_7;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_8] = CPC_KEY_8;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_9] = CPC_KEY_9;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_A] = CPC_KEY_A;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_B] = CPC_KEY_B;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_C] = CPC_KEY_C;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_D] = CPC_KEY_D;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_E] = CPC_KEY_E;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_F] = CPC_KEY_F;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_G] = CPC_KEY_G;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_H] = CPC_KEY_H;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_I] = CPC_KEY_I;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_J] = CPC_KEY_J;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_K] = CPC_KEY_K;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_L] = CPC_KEY_L;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_M] = CPC_KEY_M;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_N] = CPC_KEY_N;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_O] = CPC_KEY_O;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_P] = CPC_KEY_P;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_Q] = CPC_KEY_Q;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_R] = CPC_KEY_R;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_S] = CPC_KEY_S;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_T] = CPC_KEY_T;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_U] = CPC_KEY_U;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_V] = CPC_KEY_V;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_W] = CPC_KEY_W;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_X] = CPC_KEY_X;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_Y] = CPC_KEY_Y;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_Z] = CPC_KEY_Z;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_SPACE] = CPC_KEY_SPACE;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_COMMA] = CPC_KEY_COMMA;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_PERIOD] = CPC_KEY_DOT;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_SEMICOLON] = CPC_KEY_COLON;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_MINUS] = CPC_KEY_MINUS;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_EQUALS] = CPC_KEY_HAT;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_LEFTBRACKET] = CPC_KEY_AT;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_RIGHTBRACKET] = CPC_KEY_OPEN_SQUARE_BRACKET;

	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_TAB] = CPC_KEY_TAB;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_RETURN] = CPC_KEY_RETURN;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_BACKSPACE] = CPC_KEY_DEL;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_ESCAPE] = CPC_KEY_ESC;

	//ScanCodeToCPCKeyQwertz[SDL_SCANCODE_Equals & 0x0ff)] = CPC_KEY_CLR;

	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_UP] = CPC_KEY_CURSOR_UP;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_DOWN] = CPC_KEY_CURSOR_DOWN;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_LEFT] = CPC_KEY_CURSOR_LEFT;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_RIGHT] = CPC_KEY_CURSOR_RIGHT;

	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_KP_0] = CPC_KEY_F0;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_KP_1] = CPC_KEY_F1;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_KP_2] = CPC_KEY_F2;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_KP_3] = CPC_KEY_F3;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_KP_4] = CPC_KEY_F4;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_KP_5] = CPC_KEY_F5;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_KP_6] = CPC_KEY_F6;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_KP_7] = CPC_KEY_F7;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_KP_8] = CPC_KEY_F8;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_KP_9] = CPC_KEY_F9;

	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_KP_PERIOD] = CPC_KEY_FDOT;

	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_LSHIFT] = CPC_KEY_SHIFT;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_RSHIFT] = CPC_KEY_SHIFT;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_LCTRL] = CPC_KEY_CONTROL;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_RCTRL] = CPC_KEY_CONTROL;
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_CAPSLOCK] = CPC_KEY_CAPS_LOCK;

	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_KP_ENTER] = CPC_KEY_SMALL_ENTER;

#if 0
	ScanCodeToCPCKeyQwertz[0x0134] = CPC_KEY_COPY;			/* Alt */
	ScanCodeToCPCKeyQwertz[0x0137] = CPC_KEY_COPY;			/* Compose */

	/* German key mappings */
	ScanCodeToCPCKeyQwertz[0x00fc] =CPC_KEY_AT;			/* ue */
	ScanCodeToCPCKeyQwertz[0x002b] =CPC_KEY_OPEN_SQUARE_BRACKET;	/* Plus */
	ScanCodeToCPCKeyQwertz[0x00f6] =CPC_KEY_COLON;			/* oe */
	ScanCodeToCPCKeyQwertz[0x00e4] =CPC_KEY_SEMICOLON;		/* ae */
	ScanCodeToCPCKeyQwertz[0x0023] =CPC_KEY_CLOSE_SQUARE_BRACKET;	/* Hash */
	ScanCodeToCPCKeyQwertz[0x00df] =CPC_KEY_MINUS;			/* sz */
	ScanCodeToCPCKeyQwertz[0x00b4] =CPC_KEY_HAT;			/* Accent */
	ScanCodeToCPCKeyQwertz[0x005e] =CPC_KEY_CLR;			/* Hat */
	ScanCodeToCPCKeyQwertz[0x003c] =CPC_KEY_FORWARD_SLASH;		/* Less */

	/* The next one might break US keyboards?!? */
	ScanCodeToCPCKeyQwertz[SDL_SCANCODE_MINUS] = CPC_KEY_BACKSLASH;
#endif
}

void	sdl_InitialiseKeyboardMapping_azerty()
{
	for (int i = 0; i < SDL_NUM_SCANCODES; i++)
	{
		ScanCodeToCPCKeyAzerty[i] = CPC_KEY_NULL;
	}



	/* International key mappings */
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_0] = CPC_KEY_ZERO;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_1] = CPC_KEY_1;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_2] = CPC_KEY_2;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_3] = CPC_KEY_3;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_4] = CPC_KEY_4;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_5] = CPC_KEY_5;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_6] = CPC_KEY_6;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_7] = CPC_KEY_7;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_8] = CPC_KEY_8;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_9] = CPC_KEY_9;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_A] = CPC_KEY_A;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_B] = CPC_KEY_B;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_C] = CPC_KEY_C;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_D] = CPC_KEY_D;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_E] = CPC_KEY_E;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_F] = CPC_KEY_F;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_G] = CPC_KEY_G;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_H] = CPC_KEY_H;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_I] = CPC_KEY_I;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_J] = CPC_KEY_J;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_K] = CPC_KEY_K;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_L] = CPC_KEY_L;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_M] = CPC_KEY_M;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_N] = CPC_KEY_N;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_O] = CPC_KEY_O;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_P] = CPC_KEY_P;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_Q] = CPC_KEY_Q;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_R] = CPC_KEY_R;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_S] = CPC_KEY_S;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_T] = CPC_KEY_T;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_U] = CPC_KEY_U;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_V] = CPC_KEY_V;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_W] = CPC_KEY_W;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_X] = CPC_KEY_X;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_Y] = CPC_KEY_Y;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_Z] = CPC_KEY_Z;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_SPACE] = CPC_KEY_SPACE;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_COMMA] = CPC_KEY_COMMA;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_PERIOD] = CPC_KEY_DOT;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_SEMICOLON] = CPC_KEY_COLON;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_MINUS] = CPC_KEY_MINUS;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_EQUALS] = CPC_KEY_HAT;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_LEFTBRACKET] = CPC_KEY_AT;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_RIGHTBRACKET] = CPC_KEY_OPEN_SQUARE_BRACKET;

	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_TAB] = CPC_KEY_TAB;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_RETURN] = CPC_KEY_RETURN;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_BACKSPACE] = CPC_KEY_DEL;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_ESCAPE] = CPC_KEY_ESC;

	//ScanCodeToCPCKeyAzerty[SDL_SCANCODE_Equals & 0x0ff)] = CPC_KEY_CLR;

	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_UP] = CPC_KEY_CURSOR_UP;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_DOWN] = CPC_KEY_CURSOR_DOWN;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_LEFT] = CPC_KEY_CURSOR_LEFT;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_RIGHT] = CPC_KEY_CURSOR_RIGHT;

	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_KP_0] = CPC_KEY_F0;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_KP_1] = CPC_KEY_F1;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_KP_2] = CPC_KEY_F2;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_KP_3] = CPC_KEY_F3;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_KP_4] = CPC_KEY_F4;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_KP_5] = CPC_KEY_F5;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_KP_6] = CPC_KEY_F6;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_KP_7] = CPC_KEY_F7;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_KP_8] = CPC_KEY_F8;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_KP_9] = CPC_KEY_F9;

	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_KP_PERIOD] = CPC_KEY_FDOT;

	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_LSHIFT] = CPC_KEY_SHIFT;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_RSHIFT] = CPC_KEY_SHIFT;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_LCTRL] = CPC_KEY_CONTROL;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_RCTRL] = CPC_KEY_CONTROL;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_CAPSLOCK] = CPC_KEY_CAPS_LOCK;

	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_KP_ENTER] = CPC_KEY_SMALL_ENTER;


#if 0
	ScanCodeToCPCKeyAzerty[0x0134] = CPC_KEY_COPY;			/* Alt */
	ScanCodeToCPCKeyAzerty[0x0137] = CPC_KEY_COPY;			/* Compose */

	// Ajout Ramlaid
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_LALT] = CPC_KEY_COPY;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_AMPERSAND]  = CPC_KEY_1;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_WORLD_73]   = CPC_KEY_2;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_QUOTEDBL]   = CPC_KEY_3;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_QUOTE]      = CPC_KEY_4;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_LEFTPAREN]  = CPC_KEY_5;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_MINUS]      = CPC_KEY_6;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_WORLD_72]   = CPC_KEY_7;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_UNDERSCORE] = CPC_KEY_8;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_WORLD_71]   = CPC_KEY_9;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_WORLD_64]   = CPC_KEY_ZERO;

	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_RIGHTPAREN] = CPC_KEY_MINUS;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_EQUALS]     = CPC_KEY_HAT;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_CARET]      = CPC_KEY_AT;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_DOLLAR]     = CPC_KEY_OPEN_SQUARE_BRACKET;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_WORLD_89]   = CPC_KEY_SEMICOLON;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_ASTERISK]   = CPC_KEY_CLOSE_SQUARE_BRACKET;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_COMMA]      = CPC_KEY_COMMA;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_SEMICOLON]  = CPC_KEY_DOT;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_COLON]      = CPC_KEY_COLON;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_EXCLAIM]    = CPC_KEY_BACKSLASH;
	ScanCodeToCPCKeyAzerty[SDL_SCANCODE_LESS] = CPC_KEY_FORWARD_SLASH;
#endif
}

#if 0
void	sdl_InitialiseKeyboardMapping_spanish()
{


	/* International key mappings */
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_0] = CPC_KEY_ZERO;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_1] = CPC_KEY_1;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_2] = CPC_KEY_2;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_3] = CPC_KEY_3;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_4] = CPC_KEY_4;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_5] = CPC_KEY_5;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_6] = CPC_KEY_6;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_7] = CPC_KEY_7;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_8] = CPC_KEY_8;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_9] = CPC_KEY_9;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_A] = CPC_KEY_A;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_B] = CPC_KEY_B;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_C] = CPC_KEY_C;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_D] = CPC_KEY_D;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_E] = CPC_KEY_E;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_F] = CPC_KEY_F;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_G] = CPC_KEY_G;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_H] = CPC_KEY_H;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_I] = CPC_KEY_I;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_J] = CPC_KEY_J;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_K] = CPC_KEY_K;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_L] = CPC_KEY_L;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_M] = CPC_KEY_M;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_N] = CPC_KEY_N;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_O] = CPC_KEY_O;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_P] = CPC_KEY_P;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_Q] = CPC_KEY_Q;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_R] = CPC_KEY_R;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_S] = CPC_KEY_S;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_T] = CPC_KEY_T;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_U] = CPC_KEY_U;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_V] = CPC_KEY_V;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_W] = CPC_KEY_W;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_X] = CPC_KEY_X;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_Y] = CPC_KEY_Y;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_Z] = CPC_KEY_Z;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_SPACE] = CPC_KEY_SPACE;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_COMMA] = CPC_KEY_COMMA;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_PERIOD] = CPC_KEY_DOT;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_SEMICOLON] = CPC_KEY_COLON;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_MINUS] = CPC_KEY_MINUS;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_EQUALS] = CPC_KEY_HAT;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_LEFTBRACKET] = CPC_KEY_AT;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_RIGHTBRACKET] = CPC_KEY_OPEN_SQUARE_BRACKET;

	ScanCodeToCPCKeyDefault[SDL_SCANCODE_TAB] = CPC_KEY_TAB;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_RETURN] = CPC_KEY_RETURN;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_BACKSPACE] = CPC_KEY_DEL;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_ESCAPE] = CPC_KEY_ESC;

	//ScanCodeToCPCKeyDefault[SDL_SCANCODE_Equals & 0x0ff)] = CPC_KEY_CLR;

	ScanCodeToCPCKeyDefault[SDL_SCANCODE_UP] = CPC_KEY_CURSOR_UP;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_DOWN] = CPC_KEY_CURSOR_DOWN;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_LEFT] = CPC_KEY_CURSOR_LEFT;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_RIGHT] = CPC_KEY_CURSOR_RIGHT;

	ScanCodeToCPCKeyDefault[SDL_SCANCODE_KP_0] = CPC_KEY_F0;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_KP_1] = CPC_KEY_F1;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_KP_2] = CPC_KEY_F2;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_KP_3] = CPC_KEY_F3;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_KP_4] = CPC_KEY_F4;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_KP_5] = CPC_KEY_F5;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_KP_6] = CPC_KEY_F6;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_KP_7] = CPC_KEY_F7;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_KP_8] = CPC_KEY_F8;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_KP_9] = CPC_KEY_F9;

	ScanCodeToCPCKeyDefault[SDL_SCANCODE_KP_PERIOD] = CPC_KEY_FDOT;

	ScanCodeToCPCKeyDefault[SDL_SCANCODE_LSHIFT] = CPC_KEY_SHIFT;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_RSHIFT] = CPC_KEY_SHIFT;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_LCTRL] = CPC_KEY_CONTROL;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_RCTRL] = CPC_KEY_CONTROL;
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_CAPSLOCK] = CPC_KEY_CAPS_LOCK;

	ScanCodeToCPCKeyDefault[SDL_SCANCODE_KP_ENTER] = CPC_KEY_SMALL_ENTER;

#if 0
	ScanCodeToCPCKeyDefault[0x0134] = CPC_KEY_COPY;			/* Alt */
	ScanCodeToCPCKeyDefault[0x0137] = CPC_KEY_COPY;			/* Compose */
	//	keyUnicodeFlag = -1;
	//	SDL_EnableUNICODE(1); /* Enable UNICODE keyboard translation */
	/* Needed for special keys of spanish keyboard */
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_QUOTE] = CPC_KEY_HAT;		/* Pta+0x0027 */
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_WORLD_1] = CPC_KEY_CLR;		/* CLR 0x00a1 */
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_PLUS] = CPC_KEY_OPEN_SQUARE_BRACKET; /* [ 0x002b */
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_WORLD_71] = CPC_KEY_CLOSE_SQUARE_BRACKET; /* ] 0x00e7 */
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_WORLD_26] = CPC_KEY_BACKSLASH;	/* / 0x00ba */
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_LESS] = CPC_KEY_FORWARD_SLASH;	/* \ 0x003c */
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_WORLD_81] = CPC_KEY_COLON;		/* : 0x00f1 */
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_WORLD_20] = CPC_KEY_SEMICOLON;	/* ; 0x00b4 */
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_WORLD_8] = CPC_KEY_S			 wxGetApp().Log("Joystick %d is not a game controller\n");
	EMICOLON;	/* + 0x00a8 */
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_BACKQUOTE] = CPC_KEY_AT;		/* @ 0x0060 */
	ScanCodeToCPCKeyDefault[SDL_SCANCODE_CARET] = CPC_KEY_AT;		/* | +0x005e */
#endif
}
#endif

void PlatformSpecific::PopulateJoystickDialog(wxListBox *pListBox)
{
	for (int i = 0; i < SDL_NumJoysticks(); i++)
	{
		// game controller?
		if (SDL_IsGameController(i))
		{
			 wxGetApp().Log("Joystick %d is a game controller\n", i);

			SDL_GameController *pController = SDL_GameControllerOpen(i);
			if (pController)
			{
				// get joystick - don't close it
				SDL_Joystick *pJoystick = SDL_GameControllerGetJoystick(pController);
				// get id
				SDL_JoystickID id = SDL_JoystickInstanceID(pJoystick);

				// get the name
				const char *cName = SDL_GameControllerName(pController);

				wxString sName(cName, *wxConvCurrent);

				// close the controller now we got the name
				SDL_GameControllerClose(pController);

				// store the id
				IntClientData *pIntData = new IntClientData(id);

				pListBox->Append(sName, pIntData);
			}


		}
		else
		{
			 wxGetApp().Log("Joystick %d is not a game controller\n", i);
			SDL_Joystick *pJoystick = SDL_JoystickOpen(i);
			if (pJoystick)
			{
				// get the name
				const char *cName = SDL_JoystickName(pJoystick);
				// get the id
				SDL_JoystickID id = SDL_JoystickInstanceID(pJoystick);

				// get the name
				wxString sName(cName, *wxConvCurrent);

				// close joystick now we have the name
				SDL_JoystickClose(pJoystick);

				IntClientData *pIntData = new IntClientData(id);

				pListBox->Append(sName, pIntData);
			}
		}
	}
}

void PlatformSpecific::PopulateFullScreenDialog(wxChoice *pChoice)
{
	IntClientData *pIntData;

	// desktop resolution (no resolution switch)
	wxString sResolution;
	sResolution = wxT("Use desktop resolution");
	pIntData = new IntClientData(-1);
	pChoice->Append(sResolution, pIntData);

	for (int displayIndex = 0; displayIndex < SDL_GetNumVideoDisplays(); displayIndex++)
	{
		for (int modeIndex = 0; modeIndex < SDL_GetNumDisplayModes(displayIndex); modeIndex++)
		{
			SDL_DisplayMode mode;
			SDL_GetDisplayMode(displayIndex, modeIndex, &mode);
			bool bIgnore = true;
			// for debugging
			 wxGetApp().Log("Display: %d Mode %d Width: %d Height: %d Refresh: %d Pixel Format:%d - ", displayIndex, modeIndex, mode.w, mode.h, mode.refresh_rate, mode.format);

			switch (SDL_PIXELTYPE(mode.format))
				//       switch (mode.format)
			{
				// we work with rgb so exclude all pixel types we can't handle
			case SDL_PIXELFORMAT_UNKNOWN:
			case SDL_PIXELFORMAT_INDEX1LSB:
			case SDL_PIXELFORMAT_INDEX1MSB:
			case SDL_PIXELFORMAT_INDEX4LSB:
			case SDL_PIXELFORMAT_INDEX4MSB:
			case SDL_PIXELFORMAT_INDEX8:
			case SDL_PIXELFORMAT_YV12:
			case SDL_PIXELFORMAT_IYUV:
			case SDL_PIXELFORMAT_YUY2:
			case SDL_PIXELFORMAT_UYVY:
			case SDL_PIXELFORMAT_YVYU:
				bIgnore = true;
				break;

			default:
				bIgnore = false;
				break;
			}

			if (!bIgnore)
			{
				 wxGetApp().Log("added. Supported\n");

				sResolution.sprintf(wxT("%d: %d x %d (%d hz)"), displayIndex, mode.w, mode.h, mode.refresh_rate);
				pIntData = new IntClientData((displayIndex << 16) | modeIndex);
				pChoice->Append(sResolution, pIntData);
			}
			else
			{
				 wxGetApp().Log("Ignored. Not supported\n");
			}
		}
	}
}



int PlatformSpecific::GetPressedButton(SDL_JoystickID id)
{
	int index = JoystickFromID(id);
	int nButton = -1;

	if (SDL_IsGameController(index))
	{
		SDL_GameController *pGameController = SDL_GameControllerOpen(index);
		if (pGameController)
		{
			// don't get the joystick and check it's buttons, you need to use the game controller buttons to get the correct 
			// name
			for (int i = SDL_CONTROLLER_BUTTON_A; i < SDL_CONTROLLER_BUTTON_MAX; ++i)
			{
				SDL_GameControllerButtonBind bind = SDL_GameControllerGetBindForButton(pGameController, (SDL_GameControllerButton)i);
				if (bind.bindType != SDL_CONTROLLER_BINDTYPE_NONE)
				{
					Uint8 buttonPressed = SDL_GameControllerGetButton(pGameController, (SDL_GameControllerButton)i);
					if (buttonPressed)
					{
						nButton = i;
						break;
					}
				}
			}
			SDL_GameControllerClose(pGameController);
		}
	}
	else
	{
		// open this joystick 
		SDL_Joystick *pJoystick = SDL_JoystickOpen(index);
		if (pJoystick != NULL)
		{
			for (int i = 0; i < SDL_JoystickNumButtons(pJoystick); ++i)
			{
				Uint8 buttonPressed = SDL_JoystickGetButton(pJoystick, i);
				if (buttonPressed)
				{
					nButton = i;
					break;
				}
			}
			SDL_JoystickClose(pJoystick);
		}
	}
	return nButton;
}

int PlatformSpecific::GetPressedAxis(SDL_JoystickID id)
{
	int index = JoystickFromID(id);
	int nAxis = -1;

	if (SDL_IsGameController(index))
	{
		SDL_GameController *pGameController = SDL_GameControllerOpen(index);
		if (pGameController)
		{
			//testing Axis

			for (int i = SDL_CONTROLLER_AXIS_LEFTX; i < SDL_CONTROLLER_AXIS_MAX; ++i)
			{
				SDL_GameControllerButtonBind bind = SDL_GameControllerGetBindForAxis(pGameController, (SDL_GameControllerAxis)i);
				if (bind.bindType != SDL_CONTROLLER_BINDTYPE_NONE)
				{
					Sint16 axisPressed = SDL_GameControllerGetAxis(pGameController, (SDL_GameControllerAxis)i);

					if ((axisPressed > 10000) || (axisPressed < -10000))
					{
						nAxis = i;
						break;
					}
				}
			}
			SDL_GameControllerClose(pGameController);
		}
	}
	else
	{
		// open this joystick 
		SDL_Joystick *pJoystick = SDL_JoystickOpen(index);
		if (pJoystick != NULL)
		{

			//testing Axis
			for (int i = 0; i < SDL_JoystickNumAxes(pJoystick); ++i)
			{
				Sint16 axisPressed = SDL_JoystickGetAxis(pJoystick, i);
				if ((axisPressed > 10000) || (axisPressed < -10000))
				{
					nAxis = i;
					break;
				}
			}
			//testing POV
			for (int i = 0; i < SDL_JoystickNumHats(pJoystick); ++i)
			{
				Uint8 PovPressed = SDL_JoystickGetHat(pJoystick, i);
				if (PovPressed != SDL_HAT_CENTERED)
				{
					nAxis = 10 + i;
					break;
				}
			}
			SDL_JoystickClose(pJoystick);
		}
	}
	return nAxis;
}


int PlatformSpecific::GetJoystickNumButtons(SDL_JoystickID id)
{
	int index = JoystickFromID(id);
	int nButtons = 0;

	if (SDL_IsGameController(index))
	{
		SDL_GameController *pGameController = SDL_GameControllerOpen(index);
		if (pGameController)
		{
			// find which are bound
			for (int i = SDL_CONTROLLER_BUTTON_A; i < SDL_CONTROLLER_BUTTON_MAX; ++i)
			{
				SDL_GameControllerButtonBind bind = SDL_GameControllerGetBindForButton(pGameController, (SDL_GameControllerButton)i);
				if (bind.bindType != SDL_CONTROLLER_BINDTYPE_NONE)
				{
					nButtons++;
				}
			}
			SDL_GameControllerClose(pGameController);
		}
	}
	else
	{
		// open this joystick 
		SDL_Joystick *pJoystick = SDL_JoystickOpen(index);
		if (pJoystick != NULL)
		{
			nButtons = SDL_JoystickNumButtons(pJoystick);
			SDL_JoystickClose(pJoystick);
		}
	}
	return nButtons;
}

int PlatformSpecific::GetJoystickNumAxis(SDL_JoystickID id)
{
	int index = JoystickFromID(id);
	int nAxes = 0;

	if (SDL_IsGameController(index))
	{
		SDL_GameController *pGameController = SDL_GameControllerOpen(index);
		if (pGameController)
		{
			// find which are bound
			for (int i = SDL_CONTROLLER_AXIS_LEFTX; i < SDL_CONTROLLER_AXIS_MAX; ++i)
			{
				SDL_GameControllerButtonBind bind = SDL_GameControllerGetBindForAxis(pGameController, (SDL_GameControllerAxis)i);
				if (bind.bindType != SDL_CONTROLLER_BINDTYPE_NONE)
				{
					nAxes++;
				}
			}

			SDL_GameControllerClose(pGameController);
		}
	}
	else
	{
		// open this joystick 
		SDL_Joystick *pJoystick = SDL_JoystickOpen(index);
		if (pJoystick != NULL)
		{
			nAxes = SDL_JoystickNumAxes(pJoystick);
			SDL_JoystickClose(pJoystick);
		}
	}
	return nAxes;
}

int PlatformSpecific::GetJoystickNumPOV(SDL_JoystickID id)
{
	int index = JoystickFromID(id);
	int nPOV = 0;

	if (SDL_IsGameController(index))
	{
		nPOV = 0;
	}
	else
	{
		// open this joystick 
		SDL_Joystick *pJoystick = SDL_JoystickOpen(index);
		if (pJoystick != NULL)
		{
			nPOV = SDL_JoystickNumHats(pJoystick);
			SDL_JoystickClose(pJoystick);
		}
	}
	return nPOV;
}


wxString PlatformSpecific::GetJoystickButtonName(SDL_JoystickID id, int nButton)
{
	// map SDL_JoystickID to index
	int index = JoystickFromID(id);

	if (SDL_IsGameController(index))
	{
		// it's a game controller, we can get the button name
		const char*sButtonName = SDL_GameControllerGetStringForButton((SDL_GameControllerButton)nButton);

		// get the name
		return wxString(sButtonName, *wxConvCurrent);
	}

	wxString sName;
	sName.Printf(wxT("Button %d"), nButton);
	return sName;
}


///**************************************************************************
void DisplayMessage(wxString t)
{
#ifdef USE_OSD
	m_OSDDisplay.SetInfoMessage(t);
#endif
  }

#endif
