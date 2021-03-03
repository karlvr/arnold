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
#ifdef USE_SDL

#include "../arnguiApp.h"
#include "SDLPlatformSpecific.h"
#include "../IntClientData.h"
#include "../sdlcommon/sound.h"

#ifdef _MSC_VER
#include <SDL_syswm.h>
#else
#include <SDL/SDL_syswm.h>
#endif

#include <wx/listbox.h>
#include <wx/window.h>
#include <wx/choice.h>

#ifdef __WXGTK__
#include <sys/time.h>
#endif


#ifdef GTK2_EMBED_WINDOW
#if defined(__WXGTK__ )
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#endif
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

static int	KeySymToCPCKeyAzerty[SDLK_LAST];
static int	KeySymToCPCKeyQwertz[SDLK_LAST];
static int	KeySymToCPCKeyInternational[SDLK_LAST];
static int	KeySymToCPCKeyUserDefined[SDLK_LAST];


// table to map KeySym values to CPC Key values
static int	KeySymToCPCKeyActive[SDLK_LAST];

void SetSDLEnv(const wxChar *chId, const wxChar *chValue);

//=====================================================================
static int KeySetCursors[JOYSTICK_SIMULATED_KEYID_LAST] =
{
	SDLK_UP,
	SDLK_DOWN,
	SDLK_LEFT,
	SDLK_RIGHT,
	SDLK_SPACE,
	SDLK_RCTRL,
	SDLK_RSHIFT,
	0,
	0,
	0,
	0
};

//=====================================================================
static int KeySetWASD[JOYSTICK_SIMULATED_KEYID_LAST] =
{
	SDLK_w,
	SDLK_s,
	SDLK_a,
	SDLK_d,
	SDLK_SPACE,
	SDLK_RCTRL,
	SDLK_RSHIFT,
	0,
	0,
	0,
	0
};

static int KeySetNumPad[JOYSTICK_SIMULATED_KEYID_LAST] =
{
	SDLK_KP8,
	SDLK_KP2,
	SDLK_KP4,
	SDLK_KP6,
	SDLK_KP5,
	SDLK_KP0,
	SDLK_KP_PERIOD,
	SDLK_KP7,
	SDLK_KP9,
	SDLK_KP1,
	SDLK_KP3
};


static int KeySetHome[JOYSTICK_SIMULATED_KEYID_LAST] =
{
	SDLK_HOME,
	SDLK_END,
	SDLK_DELETE,
	SDLK_PAGEDOWN,
	SDLK_INSERT,
	SDLK_PAGEUP,
	SDLK_RCTRL,
	0,
	0,
	0,
	0
};

static int KeySetCustom[JOYSTICK_SIMULATED_KEYID_LAST] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void SetSDLEnv(const wxChar *chId, const wxChar *chValue)
{
#if defined(__WXMSW__)
	wxString sId(chId);
	wxString sValue(chValue);
	wxCharBuffer bufferId = sId.mb_str();
	wxString sStr(sId+wxT("=")+sValue);
	wxCharBuffer bufferStr = sStr.mb_str();
	 wxGetApp().Log("%s\n",bufferStr.data());
	SDL_putenv(bufferStr.data());

	 wxGetApp().Log("Result from SDL_getenv: %s\n",SDL_getenv(bufferId.data()));
#else
	wxString sID(chId);
	wxString sValue(chValue);
	wxCharBuffer bufferId = sID.mb_str();
	wxCharBuffer bufferVal = sValue.mb_str();
	 wxGetApp().Log("%s=%s\n", bufferId.data(), bufferVal.data());
	setenv(bufferId.data(), bufferVal.data(), 1);
#endif
}

void DisableLockKeys(BOOL bDisable)
{
	const SDL_version *pVersion = SDL_Linked_Version();
	if (  SDL_VERSIONNUM(pVersion->major, pVersion->minor, pVersion->patch)< SDL_VERSIONNUM(1, 2, 14))
	{
		SetSDLEnv(wxT("SDL_DISABLE_LOCK_KEYS"), bDisable ? wxT("1") : wxT("0"));
	}
	else
	{
		SetSDLEnv(wxT("SDL_DISABLE_LOCK_KEYS"), bDisable ? wxT("0") : wxT("1"));
	}
}

void PlatformSpecific::DisableJoystickEvents()
{
	m_bDisableJoystickEvents = true;
}

void PlatformSpecific::EnableJoystickEvents()
{
	m_bDisableJoystickEvents = false;
}

wxString PlatformSpecific::GetKeyName(int id)
{
	wxString sName(SDL_GetKeyName((SDLKey)id), *wxConvCurrent);
	return sName;
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

int PlatformSpecific::CheckPressedKey()
{
	// this works if you click to the emulation window; otherwise it doesn't
	const Uint8* currentKeyStates = SDL_GetKeyState(NULL);
	if (currentKeyStates!=NULL)
	{
		for (size_t i=0; i<SDLK_LAST; i++)
		{
			if (currentKeyStates[i]!=0)
			{
				return (int)i;
			}
		}
	}
	return -1;
}


void PlatformSpecific::PopulateKeyDialog(wxChoice *pChoice)
{
	for (int i = 0; i < SDLK_LAST ; i++)
	{
		wxString sKeyName = GetKeyName(i);
		if (!sKeyName.IsEmpty() && sKeyName.CmpNoCase(wxT("unknown key"))!=0)
		{
			IntClientData *pIntData;

			pIntData = new IntClientData(i);
			pChoice->Append(sKeyName, pIntData);
		}
	}

}

// TODO: Make these user defineable
static ActionKey ActionKeys[]=
{
	// windows version has these set
	{SDLK_F2, false, false, false, ACTION_TOGGLE_FULLSCREEN},
	{SDLK_NUMLOCK, false, false, false, ACTION_TOGGLE_KEYSTICK_ACTIVE},
	// {SDLK_F3, false, false, false, ACTION_INSERT_TAPE},
	// {SDLK_F4, false, false, false, ACTION_SAVE_SNAPSHOT},
	{SDLK_F5, false, false, false, ACTION_CPC_RESET},
	{SDLK_F5, true, false, false, ACTION_CPC_RESET_IMMEDIATE},
	{SDLK_F6, false, false, false, ACTION_INSERT_DISK_DRIVE_A},
	{SDLK_F7, false, false, false, ACTION_INSERT_DISK_DRIVE_B},
	{SDLK_F10, false, false, false, ACTION_QUIT},
	//  {SDLK_F10, true, false, false, ACTION_QUIT_IMMEDIATE},
	{SDLK_SYSREQ, false, false, false, ACTION_SAVE_SCREENSHOT},

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

#ifdef __WXGTK__
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
	if (nPercent==0)
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
#ifdef __WXGTK__
		int64_t deltaTime = delta_time_microseconds();
		UpdateTiming(deltaTime/1000);
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
	int FrameTimeMS = (20*100)/nPercent;
	static unsigned long PreviousTime = 0;
	static unsigned long TimeError = 0;

	/* use this to throttle speed */
	unsigned long	TimeDifference;
	unsigned long	Time;

	do
	{
		/* get current time */
		Time = timeGetTime();

		/* calc time difference */
		TimeDifference = Time - (PreviousTime-TimeError);
	}
	while (TimeDifference<FrameTimeMS);

	TimeError = (TimeDifference - FrameTimeMS) % FrameTimeMS;

	UpdateTiming(Time - PreviousTime);

	PreviousTime = Time;

#endif
#ifdef __WXGTK__
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
m_bDisableJoystickEvents(false),
m_bHasAudio(false),
m_bHasJoystick(false),
m_bFullScreenActive(false),
bActive(false),
m_pScreen(NULL),
m_pBackBuffer(NULL),
m_bDisableKeyboardEvents(false),
m_last_statePOV(0)
{
	InitKeySets();

}

PlatformSpecific::~PlatformSpecific()
{
}
//=====================================================================


void PlatformSpecific::ShutdownDisplay()
{
	if (m_pBackBuffer)
	{
		SDL_FreeSurface(m_pBackBuffer);
		m_pScreen = NULL;
	}
	if (m_pScreen)
	{
		SDL_FreeSurface(m_pScreen);
		m_pScreen = NULL;
	}
}


//=====================================================================

void PlatformSpecific::ShutdownInput()
{

	 wxGetApp().Log("SDL restore lock keys\n");

	DisableLockKeys(FALSE);

	// don't allow the idle thread to read from joysticks because we are about to shut them down
	m_bHasJoystick = false;
	CloseJoysticks();

}

//=====================================================================

void PlatformSpecific::Shutdown()
{
	 wxGetApp().Log("About to shut down SDL\n");

	 wxGetApp().Log("Shutting down display\n");
	ShutdownDisplay();

	 wxGetApp().Log("Shutting down input\n");
	ShutdownInput();

	SDLCommon::Shutdown();

	 wxGetApp().Log("Shutting down SDL\n");

	SDL_Quit();
	 wxGetApp().Log("Shut down of SDL done\n");
}

//=====================================================================

bool PlatformSpecific::Init(bool bAudio, bool bJoystick)
{
	// initialise video.. this is required
	if ( SDL_Init(SDL_INIT_VIDEO)<0)
	{
		 wxGetApp().Log("SDL video didn't initialise\n");
		return false;
	}
	else
	{
		 wxGetApp().Log("SDL video initialised\n");
	}

	// initialise joystick?
	if (bJoystick)
	{

		// initialise joystick
		if (SDL_InitSubSystem(SDL_INIT_JOYSTICK)<0)
		{
			 wxGetApp().Log("SDL joystick did not initialise\n");
		}
		else
		{
			m_bHasJoystick = true;
			 wxGetApp().Log("SDL joystick initialised\n");
		}

	}
	else
	{
		 wxGetApp().Log("Didn't initialise SDL joystick. It was not enabled in options.\n");

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
	if ((SDL_GetAppState() & SDL_APPMOUSEFOCUS)!=0)
	{
		 wxGetApp().Log("has mouse focus\n");
	}
	if ((SDL_GetAppState() & SDL_APPINPUTFOCUS)!=0)
	{
		 wxGetApp().Log("has input focus\n");
	}
	if ((SDL_GetAppState() & SDL_APPACTIVE)!=0)
	{
		 wxGetApp().Log("app is active\n");
	}

	return true;
}



//=====================================================================

void PlatformSpecific::ConfigureAudio()
{
	SDLCommon::Shutdown();
	if (m_bHasAudio)
	{
		 wxGetApp().Log("initialising audio\n");
		if (SDLCommon::InitAudio(wxGetApp().GetAudioFrequency(),wxGetApp().GetAudioBits(),wxGetApp().GetAudioChannels()))
		{
			 wxGetApp().Log("SDL Audio Ok\n");
		}
		else
		{
			 wxGetApp().Log("SDL Audio error\n");
		}
	}
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
	const char *				TranslatedChar;
} PlatformSpecificTranslation;

static PlatformSpecificTranslation TranslatedKeys[]=
{
	//	{SDL_SCANCODE_BACKSPACE, 8},
	//	{SDL_SCANCODE_TAB, 9},
	//	{SDL_SCANCODE_RETURN, 13},
	{SDLK_ESCAPE, "\x1b"},

	// these are a bit of a hack
	{SDLK_UP, "\x00"},
	{SDLK_DOWN, "\x01"},
	{SDLK_LEFT, "\x02"},
	{SDLK_RIGHT, "\x03"},

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
	{false, {SDLK_KP9}, KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH|KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_F9, false, false},
#endif //
};

void PlatformSpecific::HandleControllerAxis(int index, int axis, Sint16 value)
{
	// is key joy enabled and uses this physical joystick?
	if (KeyJoy_IsActive() && (KeyJoy_GetPhysical()==index))
	{
		int DeadZone = 5000;
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
		if (nSpecialAction!=-1)
		{
			wxGetApp().ProcessAction((ACTION_CODE)nSpecialAction);
		}

		KeyJoy_UpdateKeyJoyAxisInput(axis,nValue);
	}

	int CPCJoystickID = Joystick_PhysicalToCPC(index);
	if (CPCJoystickID==-1)
	{
		return;
	}

	// map joypad axis to cpc joystick axis
	//normal Axis
	if (Joystick_GetAxisMappingPhysical(CPCJoystickID, axis) == 0)
	{ 
		Joystick_SetXMovement(CPCJoystickID, value);
	}
	else
		if (Joystick_GetAxisMappingPhysical(CPCJoystickID, axis) == 1)
		{
			Joystick_SetYMovement(CPCJoystickID, value);
		}
}

void PlatformSpecific::HandleEvents()
{
	// We do see joystick events!
	// We do have rendering
	// We do have audio.
	// No keyboard.

	// doesn't appear to work??? always says something is pressed?
	// temp; see if keyboard is working still
	//int ArraySize = 0;
	//	const Uint8 *state = SDL_GetKeyState(NULL);
	//	for (int i=0; i<SDLK_LAST; i++)
	//	{
	//		if (state[i]!=0)
	//		{
	//			 wxGetApp().Log("%d pressed\n", i);
	//		}
	//	}

	{
		SDL_Event event;
		while (SDL_PollEvent(&event)!=0)
		{
			switch(event.type)
			{
			default:
			{
				 wxGetApp().Log("Unrecognised event\n");
			}
			break;

			case SDL_QUIT:
			{
				 wxGetApp().Log("quit event\n");
				wxGetApp().ProcessAction(ACTION_QUIT);

			}
			break;
			case SDL_SYSWMEVENT:
			{
				 wxGetApp().Log("system wm event\n");
			}
			break;

			case SDL_VIDEORESIZE:
			{
				 wxGetApp().Log("video resize event\n");
			}
			break;
			case SDL_VIDEOEXPOSE:
			{
				 wxGetApp().Log("video expose event\n");
			}
			break;
			case SDL_USEREVENT:
			{
				 wxGetApp().Log("user event\n");
			}
			break;
			case SDL_ACTIVEEVENT:
			{
				if (event.active.state & SDL_APPACTIVE)
				{
					if (event.active.gain)
					{
						 wxGetApp().Log("App active\n");
					}
					else	
					{
						 wxGetApp().Log("App inactive\n");
					}
				}
				if (event.active.state & SDL_APPMOUSEFOCUS)
				{
					if (event.active.gain)
					{
						 wxGetApp().Log("Has mouse focus\n");
					}
					else 
					{
						 wxGetApp().Log("Doesn't have mouse focus\n");
					}
				}
				if (event.active.state & SDL_APPINPUTFOCUS)
				{	
					if (event.active.gain)
					{
						 wxGetApp().Log("Has input focus\n");
					}
					else
					{
						 wxGetApp().Log("Doesn't have input focus\n");
					}
				}
			}
			break;

			case SDL_KEYDOWN:
			case SDL_KEYUP:
			{
				if (m_bDisableKeyboardEvents)
					break;

				 wxGetApp().Log("key event: %s, %s,%d,%d,%d,%d\n", event.type==SDL_KEYDOWN ? "down" : "up", event.key.state==SDL_PRESSED ? "pressed" : "released", event.key.keysym.scancode,event.key.keysym.sym,event.key.keysym.mod,event.key.keysym.unicode );
				//    SDL_KeyboardEvent &KeyEvent = event.key;
				//          SDL_keysym	*keysym = &KeyEvent.keysym;
				//            SDLKey	keycode = keysym->sym;

				if (Keyboard_GetMode()==0)
				{
					// may need to use scancode!
					SDL_KeyboardEvent &KeyEvent = event.key;
					HandleKey(&KeyEvent);
				}
				else
					if (Keyboard_GetMode()==1) 
					{
						if (event.type==SDL_KEYDOWN)
						{
							SDL_KeyboardEvent &KeyEvent = event.key;
							SDL_keysym	*keysym = &KeyEvent.keysym;
							SDLKey	keycode = keysym->sym;
							Uint32	unicodeChar= keysym->unicode;
							bool bSpecial = false;

							for (size_t i=0; i<sizeof(TranslatedKeys)/sizeof(TranslatedKeys[0]); i++)
							{
								//	if (TranslatedKeys[i].m_nLanguageMaskID & Keyboard_GetLanguage())
								{
									if (TranslatedKeys[i].m_nKeyCode==(unsigned int)keycode)
									{
										bSpecial = true;
										// this will not work with accented keys
										wxGetApp().SetTranslatedKey(TranslatedKeys[i].TranslatedChar);
										break;
									}
								}
							}

							if (!bSpecial)
							{
								/* translated key and keystick?? */

								/* 0 means no unicode char available */
								if (unicodeChar!=0)
								{
									wchar_t wstr[2];
									wstr[0] = unicodeChar;
									wstr[1] = L'\0';
									wxString str(wstr);
									const wxCharBuffer buffer = str.utf8_str();

									wxGetApp().SetTranslatedKey(buffer.data());
								}
								/* if no unicode, it may be a symbol; e.g. using Danish keyboard layout */
								else if (event.key.keysym.sym!=0)
								{
									 wxGetApp().Log("got keysym %d\n", event.key.keysym.sym);
									/* examples here are caps lock, control tab etc */
								}
							}
						}
					}


					else
					{
						 wxGetApp().Log("Unsupported keyboard mode %d!", Keyboard_GetMode());
					}


						{
							SDL_KeyboardEvent &KeyEvent = event.key;
							SDL_keysym	*keysym = &KeyEvent.keysym;
							SDLKey	keycode = keysym->sym;

							/* handle num lock and caps lock */
							if ((keycode == SDLK_NUMLOCK) && (event.type==SDL_KEYUP))
							{
								event.type = SDL_KEYDOWN;
							}
							else if ((keycode == SDLK_CAPSLOCK) && (event.type == SDL_KEYUP))
							{
								event.type = SDL_KEYDOWN;
							}

							// process action keys
							if (event.type==SDL_KEYDOWN)
							{
								for (size_t i=0; i<sizeof(ActionKeys)/sizeof(ActionKeys[0]); i++)
								{
									if (
										// keycodes
										(ActionKeys[i].m_nKeyCode==keycode) &&
										// now check modifiers
										(
										// shift pressed
										(ActionKeys[i].m_bShift && ((keysym->mod & (KMOD_LSHIFT|KMOD_RSHIFT))!=0)) ||
										// shift not pressed
										(!ActionKeys[i].m_bShift && ((keysym->mod & (KMOD_LSHIFT|KMOD_RSHIFT))==0))
										) &&
										(
										// control pressed
										(ActionKeys[i].m_bControl && ((keysym->mod & (KMOD_LCTRL|KMOD_RCTRL))!=0)) ||
										// control not pressed
										(!ActionKeys[i].m_bControl && ((keysym->mod & (KMOD_LCTRL|KMOD_RCTRL))==0))
										) &&
										(
										(ActionKeys[i].m_bAlt && ((keysym->mod & (KMOD_LALT|KMOD_RALT))!=0)) ||
										(!ActionKeys[i].m_bAlt && ((keysym->mod & (KMOD_LALT|KMOD_RALT))==0))
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
								SDL_keysym	*keysym = &KeyEvent.keysym;
								SDLKey	keycode = keysym->sym;
								for (int i=0; i<CPC_NUM_JOYSTICKS; i++)
								{
									if (Joystick_GetType(i)==JOYSTICK_TYPE_SIMULATED_BY_KEYBOARD)
									{
										for (int j=0; j<JOYSTICK_SIMULATED_KEYID_LAST; j++)
										{
											if (keycode==Joystick_GetSimulatedKeyID(i, j))
											{
												Joystick_SetSimulatedKeyIDState(i, j, (event.type==SDL_KEYDOWN));
											}
										}
									}
								}
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

				 wxGetApp().Log("Joystick %d axis : %d  value%d\n",  ControllerEvent.which, ControllerEvent.axis, ControllerEvent.value);

				HandleControllerAxis(ControllerEvent.which, ControllerEvent.axis, ControllerEvent.value);

			}
			break;

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
				int which = ControllerEvent.which;
				int hat = ControllerEvent.hat;
				/* get the hat information and send it on */
				int xvalue = 0;
				int yvalue = 0;
				if ((ControllerEvent.value & SDL_HAT_LEFT)!=0)
				{
					xvalue = -32767;
				}
				else if ((ControllerEvent.value & SDL_HAT_RIGHT)!=0)
				{
					xvalue = 32767;
				}

				if ((ControllerEvent.value & SDL_HAT_UP)!=0)
				{
					yvalue = -32767;
				}
				else if ((ControllerEvent.value & SDL_HAT_DOWN)!=0)
				{
					yvalue = 32767;
				}
				 wxGetApp().Log("Joystick %d hat %d x: %d y: %d\n", which, hat, xvalue, yvalue);

				// is key joy enabled and uses this physical joystick?
				if (KeyJoy_IsActive() && (KeyJoy_GetPhysical()==which))
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

					int nSpecialAction = KeyJoy_GetSpecialActionHat(hat, 0, nValue);
					if (nSpecialAction!=-1)
					{
						wxGetApp().ProcessAction((ACTION_CODE)nSpecialAction);
					}

					KeyJoy_UpdateKeyJoyHatInput(hat,0,nValue);


					nValue = KEYJOY_AXIS_MID;
					if (yvalue<-DeadZone)
					{
						nValue = KEYJOY_AXIS_MIN;
					}
					else if (yvalue>DeadZone)
					{
						nValue = KEYJOY_AXIS_MAX;
					}
					nSpecialAction = KeyJoy_GetSpecialActionHat(hat, 1, nValue);
					if (nSpecialAction!=-1)
					{
						wxGetApp().ProcessAction((ACTION_CODE)nSpecialAction);
					}
					KeyJoy_UpdateKeyJoyHatInput(hat,1,nValue);
				}

				int CPCJoystickID = Joystick_PhysicalToCPC(which);
				if (CPCJoystickID==-1)
				{
					return;
				}
				Joystick_SetXMovement(CPCJoystickID, xvalue);
				Joystick_SetYMovement(CPCJoystickID, yvalue);

			}
			break;

			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
			{
				if (m_bDisableJoystickEvents)
					break;

				// TODO: Button is a bitmask?
				if (!m_bHasJoystick)
				{
					break;
				}

				SDL_JoyButtonEvent &JoyButtonEvent = event.jbutton;
				/* SDL joystick index */
				int index = JoyButtonEvent.which;

				bool bPressed = (JoyButtonEvent.type==SDL_JOYBUTTONDOWN);
				 wxGetApp().Log("Joystick %d button %d %s\n", index, JoyButtonEvent.button, bPressed ? "pressed" : "released");

				// is key joy enabled?
				if (KeyJoy_IsActive() && (KeyJoy_GetPhysical()==index))
				{
					int nSpecialAction = KeyJoy_GetSpecialActionButton(JoyButtonEvent.button);
					if (nSpecialAction!=-1 && (bPressed))
					{
						wxGetApp().ProcessAction((ACTION_CODE)nSpecialAction);
					}

					KeyJoy_UpdateKeyJoyButtonInput(JoyButtonEvent.button,bPressed);

				}

				int CPCJoystickID = Joystick_PhysicalToCPC(index);
				if (CPCJoystickID==-1)
				{
					break;
				}

				int RemappedButton = Joystick_GetButtonMapping(CPCJoystickID, JoyButtonEvent.button);
				if (RemappedButton!=-1)
				{
					Joystick_SetButton(CPCJoystickID, RemappedButton, bPressed);
				}
			}
			break;

			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
			{
				SDL_MouseButtonEvent &MouseButtonEvent = event.button;
				//               wxGetApp().Log("Mouse button %s\n", event.type==SDL_MOUSEBUTTONDOWN ? "down" : "up" );
				{
					for (int i=0; i<CPC_NUM_JOYSTICKS; i++)
					{
						if (Joystick_GetType(i)==JOYSTICK_TYPE_SIMULATED_BY_MOUSE)
						{
							for (int j=0; j<JOYSTICK_SIMULATED_MOUSEID_LAST; j++)
							{
								if (MouseButtonEvent.button==Joystick_GetSimulatedMouseID(i, j))
								{
									Joystick_SetSimulatedMouseIDState(i, j, (event.type==SDL_MOUSEBUTTONDOWN));
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
				Mouse_SetButtons(nButton, (event.type==SDL_MOUSEBUTTONDOWN));
			}
			break;

			case SDL_MOUSEMOTION:
			{
				// doesn't send "no motion"
				SDL_MouseMotionEvent &MouseMotionEvent = event.motion;

				int xpos = MouseMotionEvent.x;
				int ypos = MouseMotionEvent.y;

				// convert into cpc screen space
				CPC_SetLightSensorPos(xpos, ypos);

				int xrel = MouseMotionEvent.xrel;
				int yrel = MouseMotionEvent.yrel;
				int MAXMOVE = 10;
				int MINMOVE = 1;

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
				//Mouse_AddMovement(xrel, yrel);

				Mouse_SetPosition(xpos, ypos);
			}
			break;

			}
		}
	}
}





void PlatformSpecific::DrawDisplay()
{
	if ((SDL_GetAppState() & SDL_APPACTIVE)==0)
	{
		return;
	}
	SDL_Surface *pSurfaceToUse = m_pScreen;
	if (m_pBackBuffer)
	{
		pSurfaceToUse = m_pBackBuffer;
	}

	// Lock surface if needed
	if (SDL_MUSTLOCK(pSurfaceToUse))
	{
		if (SDL_LockSurface(pSurfaceToUse) < 0)
		{
			return;
		}
	}
	if (pSurfaceToUse->pixels)
	{
		BufferInfo.pSurface = (unsigned char *)pSurfaceToUse->pixels;
		BufferInfo.SurfacePitch = pSurfaceToUse->pitch;


		/* dump whole display to screen */
		Render_DumpDisplay(&BufferInfo);
	}

	if (SDL_MUSTLOCK(pSurfaceToUse))
	{
		SDL_UnlockSurface(pSurfaceToUse);
	}

	if (m_pBackBuffer)
	{
		SDL_BlitSurface(m_pBackBuffer, &SourceRect, m_pScreen, &DestinationRect);
	}
	// perform flip
	SDL_Flip(pSurfaceToUse);

}



//=====================================================================


void PlatformSpecific::ConfigureKeyboardMode()
{
	if (Keyboard_GetMode()==1)
	{
		 wxGetApp().Log("SDL keyboard set to unicode\n");

		// enable unicode because we are translating keys
		SDL_EnableUNICODE(1);
		DisableLockKeys(0);
		// set default
		SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);

	}
	else
	{
		 wxGetApp().Log("SDL keyboard not set to unicode\n");

		// turn off unicode because we want to work off raw keys
		SDL_EnableUNICODE(0);

		DisableLockKeys(1);

		// enable key repeat
		SDL_EnableKeyRepeat(1,1);
	}
}

void PlatformSpecific::SetKeySet(int j, int *keys)
{
#if ((defined(GTK2_EMBED_WINDOW) && defined(__WXGTK__)) || (defined(MAC_EMBED_WINDOW) && defined(__WXMAC__)))

#else
	for (int i=0; i<JOYSTICK_SIMULATED_KEYID_LAST; i++)
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


	if (nKeySet==JOYSTICK_KEYSET_CURSORS)
	{
		return KeySetCursors;
	}
	else if (nKeySet==JOYSTICK_KEYSET_WASD)
	{
		return KeySetWASD;
	}
	else if (nKeySet==JOYSTICK_KEYSET_NUMPAD)
	{
		return KeySetNumPad;
	}
	else  if (nKeySet==JOYSTICK_KEYSET_INSERT_HOME)
	{
		return KeySetHome;
	}
	else  if (nKeySet==JOYSTICK_KEYSET_CUSTOM)
	{
		return KeySetCustom;
	}
#endif
	return NULL;
}

void PlatformSpecific::ConfigureJoystickKeySet(int j, int nKeySet)
{
#if ((defined(GTK2_EMBED_WINDOW) && defined(__WXGTK__)) || (defined(MAC_EMBED_WINDOW) && defined(__WXMAC__)))
#else

	if (nKeySet==JOYSTICK_KEYSET_CURSORS)
	{
		SetKeySet(j,KeySetCursors);
	}
	else if (nKeySet==JOYSTICK_KEYSET_NUMPAD)
	{

		SetKeySet(j,KeySetNumPad);
	}
	else  if (nKeySet==JOYSTICK_KEYSET_INSERT_HOME)
	{
		SetKeySet(j,KeySetHome);
	}
	else if (nKeySet==JOYSTICK_KEYSET_CUSTOM)
	{
		SetKeySet(j, KeySetCustom);
	}
#endif
}


//=====================================================================

void PlatformSpecific::ConfigureJoystickAsMouse(int j)
{
	Joystick_SetSimulatedMouseID(j, JOYSTICK_SIMULATED_FIRE1_MOUSEID, SDL_BUTTON_LEFT);
	Joystick_SetSimulatedMouseID(j, JOYSTICK_SIMULATED_FIRE2_MOUSEID, SDL_BUTTON_RIGHT);
	Joystick_SetSimulatedMouseID(j, JOYSTICK_SIMULATED_FIRE3_MOUSEID, SDL_BUTTON_MIDDLE);
}

int *arnguiApp::GetKeySet()
{
	return KeySymToCPCKeyActive;
}


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
	for (size_t i = 0; i < sizeof(KeySymToCPCKeyUserDefined) / sizeof(KeySymToCPCKeyUserDefined[0]); i++)
	{
		KeySymToCPCKeyUserDefined[i] = CPC_KEY_NULL;
	}
}

void PlatformSpecific::SetHostKey(CPC_KEY_ID CPCKey, const int *Keys)
{
	for (int j = 0; j < MaxNumKeys; j++)
	{
		int ScanCode = Keys[j];
		if (ScanCode != CPC_KEY_DEF_UNSET_KEY)
		{
			KeySymToCPCKeyUserDefined[(int)CPCKey] = ScanCode;
		}
	}
}

void PlatformSpecific::GetHostKey(CPC_KEY_ID CPCKey, int *Keys)
{
	int nKeys = 0;

	for (int j = 0; j<MaxNumKeys; j++)
	{
		Keys[j] = CPC_KEY_DEF_UNSET_KEY;
	}

	for (size_t i = 0; i < sizeof(KeySymToCPCKeyUserDefined) / sizeof(KeySymToCPCKeyUserDefined[0]); i++)
	{
		if (KeySymToCPCKeyUserDefined[i] == (int)CPCKey)
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
	for (int i = 0; i<CPC_KEY_NUM_KEYS; i++)
	{
		GetHostKey((CPC_KEY_ID)i, pData->Keys[i]);
	}
}

// forward declarations
void	sdl_InitialiseKeyboardMapping_qwertz();
void	sdl_InitialiseKeyboardMapping_azerty();
void	sdl_InitialiseKeyboardMapping_spanish();

void	PlatformSpecific::InitKeySets()
{
	int	 i;
	for (i=0; i<SDLK_LAST; i++)
	{
		KeySymToCPCKeyUserDefined[i] = CPC_KEY_NULL;
	}

		sdl_InitialiseKeyboardMapping_qwertz();
		sdl_InitialiseKeyboardMapping_azerty();
//		sdl_InitialiseKeyboardMapping_spanish();
	{


		for (int i=0; i<SDLK_LAST; i++)
		{
			KeySymToCPCKeyInternational[i] = CPC_KEY_NULL;
		}

		/* International key mappings */
		KeySymToCPCKeyInternational[SDLK_0] = CPC_KEY_ZERO;
		KeySymToCPCKeyInternational[SDLK_1] = CPC_KEY_1;
		KeySymToCPCKeyInternational[SDLK_2] = CPC_KEY_2;
		KeySymToCPCKeyInternational[SDLK_3] = CPC_KEY_3;
		KeySymToCPCKeyInternational[SDLK_4] = CPC_KEY_4;
		KeySymToCPCKeyInternational[SDLK_5] = CPC_KEY_5;
		KeySymToCPCKeyInternational[SDLK_6] = CPC_KEY_6;
		KeySymToCPCKeyInternational[SDLK_7] = CPC_KEY_7;
		KeySymToCPCKeyInternational[SDLK_8] = CPC_KEY_8;
		KeySymToCPCKeyInternational[SDLK_9] = CPC_KEY_9;
		KeySymToCPCKeyInternational[SDLK_a] = CPC_KEY_A;
		KeySymToCPCKeyInternational[SDLK_b] = CPC_KEY_B;
		KeySymToCPCKeyInternational[SDLK_c] = CPC_KEY_C;
		KeySymToCPCKeyInternational[SDLK_d] = CPC_KEY_D;
		KeySymToCPCKeyInternational[SDLK_e] = CPC_KEY_E;
		KeySymToCPCKeyInternational[SDLK_f] = CPC_KEY_F;
		KeySymToCPCKeyInternational[SDLK_g] = CPC_KEY_G;
		KeySymToCPCKeyInternational[SDLK_h] = CPC_KEY_H;
		KeySymToCPCKeyInternational[SDLK_i] = CPC_KEY_I;
		KeySymToCPCKeyInternational[SDLK_j] = CPC_KEY_J;
		KeySymToCPCKeyInternational[SDLK_k] = CPC_KEY_K;
		KeySymToCPCKeyInternational[SDLK_l] = CPC_KEY_L;
		KeySymToCPCKeyInternational[SDLK_m] = CPC_KEY_M;
		KeySymToCPCKeyInternational[SDLK_n] = CPC_KEY_N;
		KeySymToCPCKeyInternational[SDLK_o] = CPC_KEY_O;
		KeySymToCPCKeyInternational[SDLK_p] = CPC_KEY_P;
		KeySymToCPCKeyInternational[SDLK_q] = CPC_KEY_Q;
		KeySymToCPCKeyInternational[SDLK_r] = CPC_KEY_R;
		KeySymToCPCKeyInternational[SDLK_s] = CPC_KEY_S;
		KeySymToCPCKeyInternational[SDLK_t] = CPC_KEY_T;
		KeySymToCPCKeyInternational[SDLK_u] = CPC_KEY_U;
		KeySymToCPCKeyInternational[SDLK_v] = CPC_KEY_V;
		KeySymToCPCKeyInternational[SDLK_w] = CPC_KEY_W;
		KeySymToCPCKeyInternational[SDLK_x] = CPC_KEY_X;
		KeySymToCPCKeyInternational[SDLK_y] = CPC_KEY_Y;
		KeySymToCPCKeyInternational[SDLK_z] = CPC_KEY_Z;
		KeySymToCPCKeyInternational[SDLK_SPACE] = CPC_KEY_SPACE;
		KeySymToCPCKeyInternational[SDLK_COMMA] = CPC_KEY_COMMA;
		KeySymToCPCKeyInternational[SDLK_PERIOD] = CPC_KEY_DOT;
		KeySymToCPCKeyInternational[SDLK_SEMICOLON] = CPC_KEY_COLON;
		KeySymToCPCKeyInternational[SDLK_MINUS] = CPC_KEY_MINUS;
		KeySymToCPCKeyInternational[SDLK_EQUALS] = CPC_KEY_HAT;
		KeySymToCPCKeyInternational[SDLK_LEFTBRACKET] = CPC_KEY_AT;
		KeySymToCPCKeyInternational[SDLK_RIGHTBRACKET] =CPC_KEY_OPEN_SQUARE_BRACKET;
		KeySymToCPCKeyInternational[SDLK_HASH]= CPC_KEY_CLOSE_SQUARE_BRACKET;
		KeySymToCPCKeyInternational[SDLK_AT]= CPC_KEY_SEMICOLON;
		KeySymToCPCKeyInternational[SDLK_SLASH]= CPC_KEY_FORWARD_SLASH;
		KeySymToCPCKeyInternational[SDLK_TAB] = CPC_KEY_TAB;
		KeySymToCPCKeyInternational[SDLK_RETURN] = CPC_KEY_RETURN;
		KeySymToCPCKeyInternational[SDLK_BACKSPACE] = CPC_KEY_DEL;
		KeySymToCPCKeyInternational[SDLK_ESCAPE] = CPC_KEY_ESC;
		KeySymToCPCKeyInternational[SDLK_BACKSLASH] = CPC_KEY_BACKSLASH;

		//KeySymToCPCKeyInternational[SDLK_Equals & 0x0ff)] = CPC_KEY_CLR;

		KeySymToCPCKeyInternational[SDLK_UP] = CPC_KEY_CURSOR_UP;
		KeySymToCPCKeyInternational[SDLK_DOWN] = CPC_KEY_CURSOR_DOWN;
		KeySymToCPCKeyInternational[SDLK_LEFT] = CPC_KEY_CURSOR_LEFT;
		KeySymToCPCKeyInternational[SDLK_RIGHT] = CPC_KEY_CURSOR_RIGHT;

		KeySymToCPCKeyInternational[SDLK_KP0] = CPC_KEY_F0;
		KeySymToCPCKeyInternational[SDLK_KP1] = CPC_KEY_F1;
		KeySymToCPCKeyInternational[SDLK_KP2] = CPC_KEY_F2;
		KeySymToCPCKeyInternational[SDLK_KP3] = CPC_KEY_F3;
		KeySymToCPCKeyInternational[SDLK_KP4] = CPC_KEY_F4;
		KeySymToCPCKeyInternational[SDLK_KP5] = CPC_KEY_F5;
		KeySymToCPCKeyInternational[SDLK_KP6] = CPC_KEY_F6;
		KeySymToCPCKeyInternational[SDLK_KP7] = CPC_KEY_F7;
		KeySymToCPCKeyInternational[SDLK_KP8] = CPC_KEY_F8;
		KeySymToCPCKeyInternational[SDLK_KP9] = CPC_KEY_F9;

		KeySymToCPCKeyInternational[SDLK_KP_PERIOD] = CPC_KEY_FDOT;

		KeySymToCPCKeyInternational[SDLK_LSHIFT] = CPC_KEY_SHIFT;
		KeySymToCPCKeyInternational[SDLK_RSHIFT] = CPC_KEY_SHIFT;
		KeySymToCPCKeyInternational[SDLK_LCTRL] = CPC_KEY_CONTROL;
		KeySymToCPCKeyInternational[SDLK_RCTRL] = CPC_KEY_CONTROL;
		KeySymToCPCKeyInternational[SDLK_CAPSLOCK] = CPC_KEY_CAPS_LOCK;

		KeySymToCPCKeyInternational[SDLK_KP_ENTER] = CPC_KEY_SMALL_ENTER;
		KeySymToCPCKeyInternational[SDLK_QUOTE] = CPC_KEY_SEMICOLON;

		KeySymToCPCKeyInternational[SDLK_INSERT] = CPC_KEY_COPY;
		KeySymToCPCKeyInternational[SDLK_DELETE] = CPC_KEY_CLR;

	}

	SetKeySet(0);
}


int PlatformSpecific::JoystickFromID(int idWanted)
{
	if (idWanted<SDL_NumJoysticks())
	{
		return idWanted;
	}
	return -1;
}

void	sdl_InitialiseKeyboardMapping_qwertz()
{
	 wxGetApp().Log("qwertz mapping\n");
	for (int i = 0; i < SDLK_LAST; i++)
	{
		KeySymToCPCKeyQwertz[i] = CPC_KEY_NULL;
	}


	/* International key mappings */
	KeySymToCPCKeyQwertz[SDLK_0] = CPC_KEY_ZERO;
	KeySymToCPCKeyQwertz[SDLK_1] = CPC_KEY_1;
	KeySymToCPCKeyQwertz[SDLK_2] = CPC_KEY_2;
	KeySymToCPCKeyQwertz[SDLK_3] = CPC_KEY_3;
	KeySymToCPCKeyQwertz[SDLK_4] = CPC_KEY_4;
	KeySymToCPCKeyQwertz[SDLK_5] = CPC_KEY_5;
	KeySymToCPCKeyQwertz[SDLK_6] = CPC_KEY_6;
	KeySymToCPCKeyQwertz[SDLK_7] = CPC_KEY_7;
	KeySymToCPCKeyQwertz[SDLK_8] = CPC_KEY_8;
	KeySymToCPCKeyQwertz[SDLK_9] = CPC_KEY_9;
	KeySymToCPCKeyQwertz[SDLK_a] = CPC_KEY_A;
	KeySymToCPCKeyQwertz[SDLK_b] = CPC_KEY_B;
	KeySymToCPCKeyQwertz[SDLK_c] = CPC_KEY_C;
	KeySymToCPCKeyQwertz[SDLK_d] = CPC_KEY_D;
	KeySymToCPCKeyQwertz[SDLK_e] = CPC_KEY_E;
	KeySymToCPCKeyQwertz[SDLK_f] = CPC_KEY_F;
	KeySymToCPCKeyQwertz[SDLK_g] = CPC_KEY_G;
	KeySymToCPCKeyQwertz[SDLK_h] = CPC_KEY_H;
	KeySymToCPCKeyQwertz[SDLK_i] = CPC_KEY_I;
	KeySymToCPCKeyQwertz[SDLK_j] = CPC_KEY_J;
	KeySymToCPCKeyQwertz[SDLK_k] = CPC_KEY_K;
	KeySymToCPCKeyQwertz[SDLK_l] = CPC_KEY_L;
	KeySymToCPCKeyQwertz[SDLK_m] = CPC_KEY_M;
	KeySymToCPCKeyQwertz[SDLK_n] = CPC_KEY_N;
	KeySymToCPCKeyQwertz[SDLK_o] = CPC_KEY_O;
	KeySymToCPCKeyQwertz[SDLK_p] = CPC_KEY_P;
	KeySymToCPCKeyQwertz[SDLK_q] = CPC_KEY_Q;
	KeySymToCPCKeyQwertz[SDLK_r] = CPC_KEY_R;
	KeySymToCPCKeyQwertz[SDLK_s] = CPC_KEY_S;
	KeySymToCPCKeyQwertz[SDLK_t] = CPC_KEY_T;
	KeySymToCPCKeyQwertz[SDLK_u] = CPC_KEY_U;
	KeySymToCPCKeyQwertz[SDLK_v] = CPC_KEY_V;
	KeySymToCPCKeyQwertz[SDLK_w] = CPC_KEY_W;
	KeySymToCPCKeyQwertz[SDLK_x] = CPC_KEY_X;
	KeySymToCPCKeyQwertz[SDLK_y] = CPC_KEY_Y;
	KeySymToCPCKeyQwertz[SDLK_z] = CPC_KEY_Z;
	KeySymToCPCKeyQwertz[SDLK_SPACE] = CPC_KEY_SPACE;
	KeySymToCPCKeyQwertz[SDLK_COMMA] = CPC_KEY_COMMA;
	KeySymToCPCKeyQwertz[SDLK_PERIOD] = CPC_KEY_DOT;
	KeySymToCPCKeyQwertz[SDLK_SEMICOLON] = CPC_KEY_COLON;
	KeySymToCPCKeyQwertz[SDLK_MINUS] = CPC_KEY_MINUS;
	KeySymToCPCKeyQwertz[SDLK_EQUALS] = CPC_KEY_HAT;
	KeySymToCPCKeyQwertz[SDLK_LEFTBRACKET] = CPC_KEY_AT;
	KeySymToCPCKeyQwertz[SDLK_RIGHTBRACKET] =CPC_KEY_OPEN_SQUARE_BRACKET;
	KeySymToCPCKeyQwertz[SDLK_HASH]= CPC_KEY_CLOSE_SQUARE_BRACKET;

	KeySymToCPCKeyQwertz[SDLK_TAB] = CPC_KEY_TAB;
	KeySymToCPCKeyQwertz[SDLK_RETURN] = CPC_KEY_RETURN;
	KeySymToCPCKeyQwertz[SDLK_BACKSPACE] = CPC_KEY_DEL;
	KeySymToCPCKeyQwertz[SDLK_ESCAPE] = CPC_KEY_ESC;

	//KeySymToCPCKeyQwertz[SDLK_Equals & 0x0ff)] = CPC_KEY_CLR;

	KeySymToCPCKeyQwertz[SDLK_UP] = CPC_KEY_CURSOR_UP;
	KeySymToCPCKeyQwertz[SDLK_DOWN] = CPC_KEY_CURSOR_DOWN;
	KeySymToCPCKeyQwertz[SDLK_LEFT] = CPC_KEY_CURSOR_LEFT;
	KeySymToCPCKeyQwertz[SDLK_RIGHT] = CPC_KEY_CURSOR_RIGHT;

	KeySymToCPCKeyQwertz[SDLK_KP0] = CPC_KEY_F0;
	KeySymToCPCKeyQwertz[SDLK_KP1] = CPC_KEY_F1;
	KeySymToCPCKeyQwertz[SDLK_KP2] = CPC_KEY_F2;
	KeySymToCPCKeyQwertz[SDLK_KP3] = CPC_KEY_F3;
	KeySymToCPCKeyQwertz[SDLK_KP4] = CPC_KEY_F4;
	KeySymToCPCKeyQwertz[SDLK_KP5] = CPC_KEY_F5;
	KeySymToCPCKeyQwertz[SDLK_KP6] = CPC_KEY_F6;
	KeySymToCPCKeyQwertz[SDLK_KP7] = CPC_KEY_F7;
	KeySymToCPCKeyQwertz[SDLK_KP8] = CPC_KEY_F8;
	KeySymToCPCKeyQwertz[SDLK_KP9] = CPC_KEY_F9;

	KeySymToCPCKeyQwertz[SDLK_KP_PERIOD] = CPC_KEY_FDOT;

	KeySymToCPCKeyQwertz[SDLK_LSHIFT] = CPC_KEY_SHIFT;
	KeySymToCPCKeyQwertz[SDLK_RSHIFT] = CPC_KEY_SHIFT;
	KeySymToCPCKeyQwertz[SDLK_LCTRL] = CPC_KEY_CONTROL;
	KeySymToCPCKeyQwertz[SDLK_RCTRL] = CPC_KEY_CONTROL;
	KeySymToCPCKeyQwertz[SDLK_CAPSLOCK] = CPC_KEY_CAPS_LOCK;

	KeySymToCPCKeyQwertz[SDLK_KP_ENTER] = CPC_KEY_SMALL_ENTER;


	KeySymToCPCKeyQwertz[0x0134] = CPC_KEY_COPY;			/* Alt */
	KeySymToCPCKeyQwertz[0x0137] = CPC_KEY_COPY;			/* Compose */

	/* German key mappings */
	KeySymToCPCKeyQwertz[0x00fc] =CPC_KEY_AT;			/* ue */
	KeySymToCPCKeyQwertz[0x002b] =CPC_KEY_OPEN_SQUARE_BRACKET;	/* Plus */
	KeySymToCPCKeyQwertz[0x00f6] =CPC_KEY_COLON;			/* oe */
	KeySymToCPCKeyQwertz[0x00e4] =CPC_KEY_SEMICOLON;		/* ae */
	KeySymToCPCKeyQwertz[0x0023] =CPC_KEY_CLOSE_SQUARE_BRACKET;	/* Hash */
	KeySymToCPCKeyQwertz[0x00df] =CPC_KEY_MINUS;			/* sz */
	KeySymToCPCKeyQwertz[0x00b4] =CPC_KEY_HAT;			/* Accent */
	KeySymToCPCKeyQwertz[0x005e] =CPC_KEY_CLR;			/* Hat */
	KeySymToCPCKeyQwertz[0x003c] =CPC_KEY_FORWARD_SLASH;		/* Less */

	/* The next one might break US keyboards?!? */
	KeySymToCPCKeyQwertz[SDLK_MINUS] = CPC_KEY_BACKSLASH;
}

void	sdl_InitialiseKeyboardMapping_azerty()
{

	 wxGetApp().Log("azerty mapping\n");
	for (int i = 0; i < SDLK_LAST; i++)
	{
		KeySymToCPCKeyAzerty[i] = CPC_KEY_NULL;
	}

	/* International key mappings */
	KeySymToCPCKeyAzerty[SDLK_0] = CPC_KEY_ZERO;
	KeySymToCPCKeyAzerty[SDLK_1] = CPC_KEY_1;
	KeySymToCPCKeyAzerty[SDLK_2] = CPC_KEY_2;
	KeySymToCPCKeyAzerty[SDLK_3] = CPC_KEY_3;
	KeySymToCPCKeyAzerty[SDLK_4] = CPC_KEY_4;
	KeySymToCPCKeyAzerty[SDLK_5] = CPC_KEY_5;
	KeySymToCPCKeyAzerty[SDLK_6] = CPC_KEY_6;
	KeySymToCPCKeyAzerty[SDLK_7] = CPC_KEY_7;
	KeySymToCPCKeyAzerty[SDLK_8] = CPC_KEY_8;
	KeySymToCPCKeyAzerty[SDLK_9] = CPC_KEY_9;
	KeySymToCPCKeyAzerty[SDLK_a] = CPC_KEY_A;
	KeySymToCPCKeyAzerty[SDLK_b] = CPC_KEY_B;
	KeySymToCPCKeyAzerty[SDLK_c] = CPC_KEY_C;
	KeySymToCPCKeyAzerty[SDLK_d] = CPC_KEY_D;
	KeySymToCPCKeyAzerty[SDLK_e] = CPC_KEY_E;
	KeySymToCPCKeyAzerty[SDLK_f] = CPC_KEY_F;
	KeySymToCPCKeyAzerty[SDLK_g] = CPC_KEY_G;
	KeySymToCPCKeyAzerty[SDLK_h] = CPC_KEY_H;
	KeySymToCPCKeyAzerty[SDLK_i] = CPC_KEY_I;
	KeySymToCPCKeyAzerty[SDLK_j] = CPC_KEY_J;
	KeySymToCPCKeyAzerty[SDLK_k] = CPC_KEY_K;
	KeySymToCPCKeyAzerty[SDLK_l] = CPC_KEY_L;
	KeySymToCPCKeyAzerty[SDLK_m] = CPC_KEY_M;
	KeySymToCPCKeyAzerty[SDLK_n] = CPC_KEY_N;
	KeySymToCPCKeyAzerty[SDLK_o] = CPC_KEY_O;
	KeySymToCPCKeyAzerty[SDLK_p] = CPC_KEY_P;
	KeySymToCPCKeyAzerty[SDLK_q] = CPC_KEY_Q;
	KeySymToCPCKeyAzerty[SDLK_r] = CPC_KEY_R;
	KeySymToCPCKeyAzerty[SDLK_s] = CPC_KEY_S;
	KeySymToCPCKeyAzerty[SDLK_t] = CPC_KEY_T;
	KeySymToCPCKeyAzerty[SDLK_u] = CPC_KEY_U;
	KeySymToCPCKeyAzerty[SDLK_v] = CPC_KEY_V;
	KeySymToCPCKeyAzerty[SDLK_w] = CPC_KEY_W;
	KeySymToCPCKeyAzerty[SDLK_x] = CPC_KEY_X;
	KeySymToCPCKeyAzerty[SDLK_y] = CPC_KEY_Y;
	KeySymToCPCKeyAzerty[SDLK_z] = CPC_KEY_Z;
	KeySymToCPCKeyAzerty[SDLK_SPACE] = CPC_KEY_SPACE;
	KeySymToCPCKeyAzerty[SDLK_COMMA] = CPC_KEY_COMMA;
	KeySymToCPCKeyAzerty[SDLK_PERIOD] = CPC_KEY_DOT;
	KeySymToCPCKeyAzerty[SDLK_SEMICOLON] = CPC_KEY_COLON;
	KeySymToCPCKeyAzerty[SDLK_MINUS] = CPC_KEY_MINUS;
	KeySymToCPCKeyAzerty[SDLK_EQUALS] = CPC_KEY_HAT;
	KeySymToCPCKeyAzerty[SDLK_LEFTBRACKET] = CPC_KEY_AT;
	KeySymToCPCKeyAzerty[SDLK_RIGHTBRACKET] =CPC_KEY_OPEN_SQUARE_BRACKET;
	KeySymToCPCKeyAzerty[SDLK_HASH]= CPC_KEY_CLOSE_SQUARE_BRACKET;

	KeySymToCPCKeyAzerty[SDLK_TAB] = CPC_KEY_TAB;
	KeySymToCPCKeyAzerty[SDLK_RETURN] = CPC_KEY_RETURN;
	KeySymToCPCKeyAzerty[SDLK_BACKSPACE] = CPC_KEY_DEL;
	KeySymToCPCKeyAzerty[SDLK_ESCAPE] = CPC_KEY_ESC;

	//KeySymToCPCKeyAzerty[SDLK_Equals & 0x0ff)] = CPC_KEY_CLR;

	KeySymToCPCKeyAzerty[SDLK_UP] = CPC_KEY_CURSOR_UP;
	KeySymToCPCKeyAzerty[SDLK_DOWN] = CPC_KEY_CURSOR_DOWN;
	KeySymToCPCKeyAzerty[SDLK_LEFT] = CPC_KEY_CURSOR_LEFT;
	KeySymToCPCKeyAzerty[SDLK_RIGHT] = CPC_KEY_CURSOR_RIGHT;

	KeySymToCPCKeyAzerty[SDLK_KP0] = CPC_KEY_F0;
	KeySymToCPCKeyAzerty[SDLK_KP1] = CPC_KEY_F1;
	KeySymToCPCKeyAzerty[SDLK_KP2] = CPC_KEY_F2;
	KeySymToCPCKeyAzerty[SDLK_KP3] = CPC_KEY_F3;
	KeySymToCPCKeyAzerty[SDLK_KP4] = CPC_KEY_F4;
	KeySymToCPCKeyAzerty[SDLK_KP5] = CPC_KEY_F5;
	KeySymToCPCKeyAzerty[SDLK_KP6] = CPC_KEY_F6;
	KeySymToCPCKeyAzerty[SDLK_KP7] = CPC_KEY_F7;
	KeySymToCPCKeyAzerty[SDLK_KP8] = CPC_KEY_F8;
	KeySymToCPCKeyAzerty[SDLK_KP9] = CPC_KEY_F9;

	KeySymToCPCKeyAzerty[SDLK_KP_PERIOD] = CPC_KEY_FDOT;

	KeySymToCPCKeyAzerty[SDLK_LSHIFT] = CPC_KEY_SHIFT;
	KeySymToCPCKeyAzerty[SDLK_RSHIFT] = CPC_KEY_SHIFT;
	KeySymToCPCKeyAzerty[SDLK_LCTRL] = CPC_KEY_CONTROL;
	KeySymToCPCKeyAzerty[SDLK_RCTRL] = CPC_KEY_CONTROL;
	KeySymToCPCKeyAzerty[SDLK_CAPSLOCK] = CPC_KEY_CAPS_LOCK;

	KeySymToCPCKeyAzerty[SDLK_KP_ENTER] = CPC_KEY_SMALL_ENTER;


	KeySymToCPCKeyAzerty[0x0134] = CPC_KEY_COPY;			/* Alt */
	KeySymToCPCKeyAzerty[0x0137] = CPC_KEY_COPY;			/* Compose */

	// Ajout Ramlaid
	KeySymToCPCKeyAzerty[SDLK_LALT] = CPC_KEY_COPY;

	KeySymToCPCKeyAzerty[SDLK_AMPERSAND]  = CPC_KEY_1;
	KeySymToCPCKeyAzerty[SDLK_WORLD_73]   = CPC_KEY_2;
	KeySymToCPCKeyAzerty[SDLK_QUOTEDBL]   = CPC_KEY_3;
	KeySymToCPCKeyAzerty[SDLK_QUOTE]      = CPC_KEY_4;
	KeySymToCPCKeyAzerty[SDLK_LEFTPAREN]  = CPC_KEY_5;
	KeySymToCPCKeyAzerty[SDLK_MINUS]      = CPC_KEY_6;
	KeySymToCPCKeyAzerty[SDLK_WORLD_72]   = CPC_KEY_7;
	KeySymToCPCKeyAzerty[SDLK_UNDERSCORE] = CPC_KEY_8;
	KeySymToCPCKeyAzerty[SDLK_WORLD_71]   = CPC_KEY_9;
	KeySymToCPCKeyAzerty[SDLK_WORLD_64]   = CPC_KEY_ZERO;

	KeySymToCPCKeyAzerty[SDLK_RIGHTPAREN] = CPC_KEY_MINUS;
	KeySymToCPCKeyAzerty[SDLK_EQUALS]     = CPC_KEY_HAT;
	KeySymToCPCKeyAzerty[SDLK_CARET]      = CPC_KEY_AT;
	KeySymToCPCKeyAzerty[SDLK_DOLLAR]     = CPC_KEY_OPEN_SQUARE_BRACKET;
	KeySymToCPCKeyAzerty[SDLK_WORLD_89]   = CPC_KEY_SEMICOLON;
	KeySymToCPCKeyAzerty[SDLK_ASTERISK]   = CPC_KEY_CLOSE_SQUARE_BRACKET;
	KeySymToCPCKeyAzerty[SDLK_COMMA]      = CPC_KEY_COMMA;
	KeySymToCPCKeyAzerty[SDLK_SEMICOLON]  = CPC_KEY_DOT;
	KeySymToCPCKeyAzerty[SDLK_COLON]      = CPC_KEY_COLON;
	KeySymToCPCKeyAzerty[SDLK_EXCLAIM]    = CPC_KEY_BACKSLASH;
	KeySymToCPCKeyAzerty[SDLK_LESS]       = CPC_KEY_FORWARD_SLASH;
}

#if 0
void	sdl_InitialiseKeyboardMapping_spanish()
{
	 wxGetApp().Log("spanish mapping\n");




	/* International key mappings */
	KeySymToCPCKeyActive[SDLK_0] = CPC_KEY_ZERO;
	KeySymToCPCKeyActive[SDLK_1] = CPC_KEY_1;
	KeySymToCPCKeyActive[SDLK_2] = CPC_KEY_2;
	KeySymToCPCKeyActive[SDLK_3] = CPC_KEY_3;
	KeySymToCPCKeyActive[SDLK_4] = CPC_KEY_4;
	KeySymToCPCKeyActive[SDLK_5] = CPC_KEY_5;
	KeySymToCPCKeyActive[SDLK_6] = CPC_KEY_6;
	KeySymToCPCKeyActive[SDLK_7] = CPC_KEY_7;
	KeySymToCPCKeyActive[SDLK_8] = CPC_KEY_8;
	KeySymToCPCKeyActive[SDLK_9] = CPC_KEY_9;
	KeySymToCPCKeyActive[SDLK_a] = CPC_KEY_A;
	KeySymToCPCKeyActive[SDLK_b] = CPC_KEY_B;
	KeySymToCPCKeyActive[SDLK_c] = CPC_KEY_C;
	KeySymToCPCKeyActive[SDLK_d] = CPC_KEY_D;
	KeySymToCPCKeyActive[SDLK_e] = CPC_KEY_E;
	KeySymToCPCKeyActive[SDLK_f] = CPC_KEY_F;
	KeySymToCPCKeyActive[SDLK_g] = CPC_KEY_G;
	KeySymToCPCKeyActive[SDLK_h] = CPC_KEY_H;
	KeySymToCPCKeyActive[SDLK_i] = CPC_KEY_I;
	KeySymToCPCKeyActive[SDLK_j] = CPC_KEY_J;
	KeySymToCPCKeyActive[SDLK_k] = CPC_KEY_K;
	KeySymToCPCKeyActive[SDLK_l] = CPC_KEY_L;
	KeySymToCPCKeyActive[SDLK_m] = CPC_KEY_M;
	KeySymToCPCKeyActive[SDLK_n] = CPC_KEY_N;
	KeySymToCPCKeyActive[SDLK_o] = CPC_KEY_O;
	KeySymToCPCKeyActive[SDLK_p] = CPC_KEY_P;
	KeySymToCPCKeyActive[SDLK_q] = CPC_KEY_Q;
	KeySymToCPCKeyActive[SDLK_r] = CPC_KEY_R;
	KeySymToCPCKeyActive[SDLK_s] = CPC_KEY_S;
	KeySymToCPCKeyActive[SDLK_t] = CPC_KEY_T;
	KeySymToCPCKeyActive[SDLK_u] = CPC_KEY_U;
	KeySymToCPCKeyActive[SDLK_v] = CPC_KEY_V;
	KeySymToCPCKeyActive[SDLK_w] = CPC_KEY_W;
	KeySymToCPCKeyActive[SDLK_x] = CPC_KEY_X;
	KeySymToCPCKeyActive[SDLK_y] = CPC_KEY_Y;
	KeySymToCPCKeyActive[SDLK_z] = CPC_KEY_Z;
	KeySymToCPCKeyActive[SDLK_SPACE] = CPC_KEY_SPACE;
	KeySymToCPCKeyActive[SDLK_COMMA] = CPC_KEY_COMMA;
	KeySymToCPCKeyActive[SDLK_PERIOD] = CPC_KEY_DOT;
	KeySymToCPCKeyActive[SDLK_SEMICOLON] = CPC_KEY_COLON;
	KeySymToCPCKeyActive[SDLK_MINUS] = CPC_KEY_MINUS;
	KeySymToCPCKeyActive[SDLK_EQUALS] = CPC_KEY_HAT;
	KeySymToCPCKeyActive[SDLK_LEFTBRACKET] = CPC_KEY_AT;
	KeySymToCPCKeyActive[SDLK_RIGHTBRACKET] =CPC_KEY_OPEN_SQUARE_BRACKET;

	KeySymToCPCKeyActive[SDLK_TAB] = CPC_KEY_TAB;
	KeySymToCPCKeyActive[SDLK_RETURN] = CPC_KEY_RETURN;
	KeySymToCPCKeyActive[SDLK_BACKSPACE] = CPC_KEY_DEL;
	KeySymToCPCKeyActive[SDLK_ESCAPE] = CPC_KEY_ESC;

	//KeySymToCPCKeyActive[SDLK_Equals & 0x0ff)] = CPC_KEY_CLR;

	KeySymToCPCKeyActive[SDLK_UP] = CPC_KEY_CURSOR_UP;
	KeySymToCPCKeyActive[SDLK_DOWN] = CPC_KEY_CURSOR_DOWN;
	KeySymToCPCKeyActive[SDLK_LEFT] = CPC_KEY_CURSOR_LEFT;
	KeySymToCPCKeyActive[SDLK_RIGHT] = CPC_KEY_CURSOR_RIGHT;

	KeySymToCPCKeyActive[SDLK_KP0] = CPC_KEY_F0;
	KeySymToCPCKeyActive[SDLK_KP1] = CPC_KEY_F1;
	KeySymToCPCKeyActive[SDLK_KP2] = CPC_KEY_F2;
	KeySymToCPCKeyActive[SDLK_KP3] = CPC_KEY_F3;
	KeySymToCPCKeyActive[SDLK_KP4] = CPC_KEY_F4;
	KeySymToCPCKeyActive[SDLK_KP5] = CPC_KEY_F5;
	KeySymToCPCKeyActive[SDLK_KP6] = CPC_KEY_F6;
	KeySymToCPCKeyActive[SDLK_KP7] = CPC_KEY_F7;
	KeySymToCPCKeyActive[SDLK_KP8] = CPC_KEY_F8;
	KeySymToCPCKeyActive[SDLK_KP9] = CPC_KEY_F9;

	KeySymToCPCKeyActive[SDLK_KP_PERIOD] = CPC_KEY_FDOT;

	KeySymToCPCKeyActive[SDLK_LSHIFT] = CPC_KEY_SHIFT;
	KeySymToCPCKeyActive[SDLK_RSHIFT] = CPC_KEY_SHIFT;
	KeySymToCPCKeyActive[SDLK_LCTRL] = CPC_KEY_CONTROL;
	KeySymToCPCKeyActive[SDLK_RCTRL] = CPC_KEY_CONTROL;
	KeySymToCPCKeyActive[SDLK_CAPSLOCK] = CPC_KEY_CAPS_LOCK;

	KeySymToCPCKeyActive[SDLK_KP_ENTER] = CPC_KEY_SMALL_ENTER;


	KeySymToCPCKeyActive[0x0134] = CPC_KEY_COPY;			/* Alt */
	KeySymToCPCKeyActive[0x0137] = CPC_KEY_COPY;			/* Compose */

	//	keyUnicodeFlag = -1;
	//	SDL_EnableUNICODE(1); /* Enable UNICODE keyboard translation */
	/* Needed for special keys of spanish keyboard */
	KeySymToCPCKeyActive[SDLK_QUOTE] = CPC_KEY_HAT;		/* Pta+0x0027 */
	KeySymToCPCKeyActive[SDLK_WORLD_1] = CPC_KEY_CLR;		/* CLR 0x00a1 */
	KeySymToCPCKeyActive[SDLK_PLUS] = CPC_KEY_OPEN_SQUARE_BRACKET; /* [ 0x002b */
	KeySymToCPCKeyActive[SDLK_WORLD_71] = CPC_KEY_CLOSE_SQUARE_BRACKET; /* ] 0x00e7 */
	KeySymToCPCKeyActive[SDLK_WORLD_26] = CPC_KEY_BACKSLASH;	/* / 0x00ba */
	KeySymToCPCKeyActive[SDLK_LESS] = CPC_KEY_FORWARD_SLASH;	/* \ 0x003c */
	KeySymToCPCKeyActive[SDLK_WORLD_81] = CPC_KEY_COLON;		/* : 0x00f1 */
	KeySymToCPCKeyActive[SDLK_WORLD_20] = CPC_KEY_SEMICOLON;	/* ; 0x00b4 */
	KeySymToCPCKeyActive[SDLK_WORLD_8] = CPC_KEY_SEMICOLON;	/* + 0x00a8 */
	KeySymToCPCKeyActive[SDLK_BACKQUOTE] = CPC_KEY_AT;		/* @ 0x0060 */
	KeySymToCPCKeyActive[SDLK_CARET] = CPC_KEY_AT;		/* | +0x005e */
}
#endif


void PlatformSpecific::SetKeySet(int layout)
{
	int i;
	
	int *SourceKeySet = NULL;

	switch (layout)
	{
	default:
	case 0:
	{
		SourceKeySet = KeySymToCPCKeyInternational;
	}
	break;
	case 1:
	{
		SourceKeySet = KeySymToCPCKeyAzerty;
	}
	break;
	case 2:
	{
		SourceKeySet = KeySymToCPCKeyQwertz;
	}
	break;
	case 3:
	{
		SourceKeySet = KeySymToCPCKeyUserDefined;
	}
	break;
	}

	memcpy(KeySymToCPCKeyActive, SourceKeySet, sizeof(KeySymToCPCKeyActive));


	int CPCKeys[10*8];
	for (i=0; i<CPC_KEY_NUM_KEYS; i++)
	{
		CPCKeys[i] = -1;
	}

	/* do not map joysticks here.. provide key sets! */
	CPCKeys[(9*8)+0] = -2;
	CPCKeys[(9*8)+1] = -2;
	CPCKeys[(9*8)+2] = -2;
	CPCKeys[(9*8)+3] = -2;
	CPCKeys[(9*8)+4] = -2;
	CPCKeys[(9*8)+5] = -2;
	CPCKeys[(9*8)+6] = -2;		// unused

	// check all keys are mapped?
	for (i=0; i<SDLK_LAST; i++)
	{
		if (KeySymToCPCKeyActive[i]!=CPC_KEY_NULL)
		{
			if (CPCKeys[KeySymToCPCKeyActive[i]]==-2)
			{
				//       wxString sMessage;
				//    sMessage.Format(wxT("SDL key %d is mapped to joystick! ERROR: Use keyset do not map keys here!\r\n"), i);
				//     TraceMessage(sMessage);
				 wxGetApp().Log("SDL key %d is mapped to joystick! ERROR: Joysticks are mapped differently!",i);
			}
			else
			{
				CPCKeys[KeySymToCPCKeyActive[i]] = 0;
			}
		}
	}

	for (i=0; i<CPC_KEY_NUM_KEYS; i++)
	{
		if (CPCKeys[i]==-1)
		{
			//        wxString sMessage;
			//         sMessage.Format(wxT("CPC Key %d has not been mapped\r\n"), i);
			//          TraceMessage(sMessage);
			 wxGetApp().Log("CPC Key %d (%s) has not been mapped\n",i, CPC_GetKeyName((CPC_KEY_ID)i)); 
		}
	}
}


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



void PlatformSpecific::SetupDisplay(bool bFullScreen, WindowedRenderSettings *pWindowed, void *pHandle)
{
	ShutdownDisplay();

	int mode = SDL_DOUBLEBUF;


	//	SDL_VIDEO_FULLSCREEN_HEAD=0|1 
	if (!bFullScreen)
	{


		// for windows don't have a frame and embed within main window
		// for linux, show a frame with a title
#if defined(WIN_EMBED_WINDOW)
		// WIN32

		mode|=SDL_NOFRAME;

		// windows version works for 32-bit and 64-bit - THIS IS ONLY RECOGNISED WHEN VIDEO IS INITIALISED!
		// wxwidgets 3.0 moans about ul
		wxString sWindowID;
		sWindowID.Printf(wxT("%ul"), pHandle);
		SetSDLEnv(wxT("SDL_WINDOWID"), sWindowID);
#elif defined(GTK2_EMBED_WINDOW) && defined(__WXGTK__)
		// GTK2

		mode|=SDL_NOFRAME;
		 wxGetApp().Log("GTKWidget: %08x\n", pHandle);
		GdkWindow *pWindow = gtk_widget_get_window((GtkWidget *)pHandle);
		 wxGetApp().Log("GDKWindow: %08x\n", (void *)pWindow);
		// a X11 window for SDL1
		 wxGetApp().Log("the X11 id is %u\n",GDK_WINDOW_XWINDOW(pWindow));
		wxString sWindowID;
		sWindowID.Printf(wxT("%ld"), (size_t)GDK_WINDOW_XWINDOW(pWindow));	//GDK_WINDOW_XWINDOW(pWindow));
		wxString sKey(wxT("SDL_WINDOWID"));
		SetSDLEnv(sKey, sWindowID);
#endif


		Init(true, true);

		AutoConfigureJoystick();
		ConfigureAudio();

		// mode|=SDL_HWSURFACE;

		int WindowWidthUsed = pWindowed->m_nWindowWidth;
		int WindowHeightUsed = pWindowed->m_nWindowHeight;
		int clientX;
		int clientY;
		int clientWidth;
		int clientHeight;

		// get desktop size...
		// wxwidgets gets a better size
		::wxClientDisplayRect(&clientX, &clientY, &clientWidth, &clientHeight);

		int WindowOffsetX = 0;
		int WindowOffsetY = 0;
		// the used window width/height we got may be smaller than the size we want, so we need to crop the image
		if (clientWidth < WindowWidthUsed)
		{
			WindowOffsetX = (WindowWidthUsed - clientWidth) >> 1;
			WindowWidthUsed = clientWidth;
		}
		if (clientHeight < WindowHeightUsed)
		{
			WindowOffsetY = (WindowHeightUsed - clientHeight) >> 1;
			WindowHeightUsed = clientHeight;
		}

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

#if defined(WIN_EMBED_WINDOW) || (defined(GTK2_EMBED_WINDOW)&& defined(__WXGTK__)) 
#else
#if defined(WIN_EMBED_WINDOW)
		// set icon before set video mode so it always appears
		// no embed window
		SDL_WM_SetCaption("Arnold - Emulation Window","Arnold");
#else
		// set icon before set video mode so it always appears
		// no embed window
		SDL_WM_SetCaption("Arnold - Emulation Window",NULL);
		SDL_WM_SetIcon(SDL_LoadBMP("arnold.bmp"),NULL);
#endif
#endif
		m_pScreen = SDL_SetVideoMode(pWindowed->m_nWindowWidth, pWindowed->m_nWindowHeight, 0, mode);


		if (Scale != 1)
		{
			SDL_PixelFormat *format = m_pScreen->format;

			m_pBackBuffer = SDL_CreateRGBSurface(0, BufferInfo.SurfaceWidth, BufferInfo.SurfaceHeight, format->BitsPerPixel, format->Rmask, format->Gmask, format->Bmask, format->Amask);
		}

		SDL_ShowCursor(SDL_ENABLE);

		SDL_PixelFormat *format = m_pScreen->format;

		GRAPHICS_BUFFER_COLOUR_FORMAT BufferColourFormat;
		BufferColourFormat.Red.Mask = format->Rmask;
		BufferColourFormat.Red.BPP = CalcBPPFromMask(format->Rmask);
		BufferColourFormat.Red.Shift = format->Rshift;

		BufferColourFormat.Green.Mask = format->Gmask;
		BufferColourFormat.Green.BPP = CalcBPPFromMask(format->Gmask);
		BufferColourFormat.Green.Shift = format->Gshift;

		BufferColourFormat.Blue.Mask = format->Bmask;
		BufferColourFormat.Blue.BPP = CalcBPPFromMask(format->Bmask);
		BufferColourFormat.Blue.Shift = format->Bshift;

		BufferColourFormat.BPP = format->BitsPerPixel;


		Render_SetGraphicsBufferColourFormat(&BufferColourFormat, pWindowed->m_nRenderWidth, pWindowed->m_nRenderHeight);
	}
	else
	{

		mode|=SDL_NOFRAME|SDL_HWSURFACE;
		int nWidth = 800;
		int nHeight = 600;
		int nDepth = 16;

#if defined(WIN_EMBED_WINDOW) || (defined(GTK2_EMBED_WINDOW)&& defined(__WXGTK__)) 
#else
#if defined(WIN_EMBED_WINDOW)
		// set icon before set video mode so it always appears
		// no embed window
		SDL_WM_SetCaption("Arnold - Emulation Window","Arnold");
#else
		// set icon before set video mode so it always appears
		// no embed window
		SDL_WM_SetCaption("Arnold - Emulation Window",NULL);
		SDL_WM_SetIcon(SDL_LoadBMP("arnold.bmp"),NULL);
#endif
#endif
		m_pScreen = SDL_SetVideoMode(nWidth, nHeight, nDepth, mode|SDL_FULLSCREEN);

		SDL_SysWMinfo info;
		SDL_VERSION(&info.version);

		SDL_GetWMInfo(&info);
		//    HWND hWnd = info.data.window;

		GRAPHICS_BUFFER_COLOUR_FORMAT BufferColourFormat;

		SDL_ShowCursor(SDL_ENABLE);

		SDL_PixelFormat *format = m_pScreen->format;

		BufferColourFormat.Red.Mask = format->Rmask;
		BufferColourFormat.Red.BPP = CalcBPPFromMask(format->Rmask);
		BufferColourFormat.Red.Shift = format->Rshift;

		BufferColourFormat.Green.Mask = format->Gmask;
		BufferColourFormat.Green.BPP = CalcBPPFromMask(format->Gmask);
		BufferColourFormat.Green.Shift = format->Gshift;

		BufferColourFormat.Blue.Mask = format->Bmask;
		BufferColourFormat.Blue.BPP = CalcBPPFromMask(format->Bmask);
		BufferColourFormat.Blue.Shift = format->Bshift;

		BufferColourFormat.BPP = format->BitsPerPixel;


		Render_SetGraphicsBufferColourFormat(&BufferColourFormat,nWidth, nDepth);


	}
}




void	PlatformSpecific::HandleKey(SDL_KeyboardEvent *theEvent)
{
	CPC_KEY_ID	theKeyPressed;
	int *pKeySet = wxGetApp().GetKeySet();

	// get KeySym
	SDL_keysym	*keysym = &theEvent->keysym;
	SDLKey	keycode = keysym->sym;

	// if key stick is active and key used by keystick then block
	if (Joystick_IsKeyStickActive() && Joystick_IsKeyUsedByKeyStick(keycode))
	{
		return;
	}

	if ( keycode <= SDLK_LAST )
	{
		theKeyPressed = (CPC_KEY_ID)pKeySet[keycode];
	}
	else
	{
		theKeyPressed = CPC_KEY_NULL;
	}

	if (theKeyPressed!=CPC_KEY_NULL)
	{
		// set or release key depending on state
		if ( theEvent->type == SDL_KEYDOWN )
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
// for saving

// based on SDL_JoystickID return the string we can use to store it by and retrieve it later
wxString PlatformSpecific::GetJoystickIdString(int nIndex)
{
	if (nIndex!=-1)
	{
		//			SDL_Joystick *pJoystick = SDL_JoystickOpen(nIndex);
		//			if (pJoystick!=NULL)
		{
			// get the name
			const char *cName = SDL_JoystickName(nIndex);

			// get the name
			wxString sName(cName, *wxConvCurrent);

			//			SDL_JoystickClose(pJoystick);

			return sName;
		}
	}
	return wxEmptyString;
}

// for loading
int PlatformSpecific::GetJoystickIdFromString(wxString &sNameWanted)
{
	// iterate through all
	for (int i=0; i<SDL_NumJoysticks(); i++)
	{
		// open this joystick 
		//	  SDL_Joystick *pJoystick = SDL_JoystickOpen(i);
		//  if (pJoystick!=NULL)
		{
			// get the name
			const char *cName = SDL_JoystickName(i);

			// get the name
			wxString sName(cName, *wxConvCurrent);

			if (sName==sNameWanted)
			{
				// close it
				//	  SDL_JoystickClose(pJoystick);

				return (int)i;
			}

			//SDL_JoystickClose(pJoystick);
		}
	}
	return -1;
}
//=====================================================================
void PlatformSpecific::AutoConfigureTouch()
{
}

void PlatformSpecific::AutoConfigureJoystick()
{
	 wxGetApp().Log("Auto-configuring joystick\n");

	// setup default configuration for joysticks
	bool bOpenedJoystick = false;

	// does sdl have a joystick setup?
	if (SDL_NumJoysticks()!=0)
	{
		for (int i=0; i<SDL_NumJoysticks(); i++)
		{
			SDL_Joystick *pJoystick = SDL_JoystickOpen(i);
			if (pJoystick)
			{
				 wxGetApp().Log("Enabling digitial joystick 0 and setting it to %s\n",SDL_JoystickName(i));
				 wxGetApp().Log("Enabling Plus analogue joystick 0 and setting it to %s\n",SDL_JoystickName(i));

				// succeeded opening joystick
				Joystick_Activate(CPC_DIGITAL_JOYSTICK0, TRUE);
				Joystick_Activate(CPC_ANALOGUE_JOYSTICK0, TRUE);
				Joystick_SetType(CPC_DIGITAL_JOYSTICK0, JOYSTICK_TYPE_REAL);
				Joystick_SetType(CPC_ANALOGUE_JOYSTICK0, JOYSTICK_TYPE_REAL);
				Joystick_SetPhysical(CPC_DIGITAL_JOYSTICK0,i);
				Joystick_SetPhysical(CPC_ANALOGUE_JOYSTICK0,i);

				Joystick_Reset(CPC_DIGITAL_JOYSTICK0);
				Joystick_Reset(CPC_ANALOGUE_JOYSTICK0);

				KeyJoy_SetPhysical(i);

				SDL_JoystickClose(pJoystick);

				RefreshJoysticks();

				bOpenedJoystick = true;
				break;
			}
		}
	}
	if (bOpenedJoystick)
	{
		 wxGetApp().Log("No physical joysticks connected. Auto configuration has failed\n");
	}
}


//=====================================================================

void PlatformSpecific::CloseJoysticks()
{  
	// stop events
	SDL_JoystickEventState(SDL_DISABLE);

	// close all existing
	for (unsigned int j=0; j!=m_OpenedJoysticks.GetCount(); j++)
	{
		SDL_Joystick *pJoystick = (SDL_Joystick *)m_OpenedJoysticks[j];
		SDL_JoystickClose(pJoystick);
	}
	m_OpenedJoysticks.Clear();

}

//=====================================================================

void PlatformSpecific::RefreshJoysticks()
{
	CloseJoysticks();
	// enable joysticks we are using
	bool bAtLeastOneJoystick = false;
	for (int j=0; j<CPC_NUM_JOYSTICKS; j++)
	{
		Joystick_SetXRange(j, -32768, 32767);
		Joystick_SetYRange(j, -32768, 32767);
		Joystick_Reset(j);

		if (Joystick_GetType(j)==JOYSTICK_TYPE_REAL)
		{
			int nPhysical = Joystick_GetPhysical(j);
			if (nPhysical!=-1)
			{
				if (!SDL_JoystickOpened(nPhysical))
				{
					m_OpenedJoysticks.Add((void *)SDL_JoystickOpen(nPhysical));
				}
				bAtLeastOneJoystick = true;
			}
		}
	}
	if (KeyJoy_IsActive())
	{
		int nPhysical = KeyJoy_GetPhysical();
		if (nPhysical!=-1)
		{
			// we need to find first available joystick
			if (!SDL_JoystickOpened(nPhysical))
			{
				m_OpenedJoysticks.Add((void *)SDL_JoystickOpen(nPhysical));
				bAtLeastOneJoystick = true;
			}
		}
	}

	// enable events if at least one joystick is enabled
	if (bAtLeastOneJoystick)
	{
		SDL_JoystickEventState(SDL_ENABLE);
	}
	else
	{
		SDL_JoystickEventState(SDL_DISABLE);
	}
}

void PlatformSpecific::PopulateJoystickDialog(wxListBox *pListBox)
{
	for (int i=0; i<SDL_NumJoysticks(); i++)
	{

		const char *cName = SDL_JoystickName(i);

		wxString sName(cName, *wxConvCurrent);

		IntClientData *pIntData = new IntClientData(i);

		pListBox->Append(sName, pIntData);
	}
}

void PlatformSpecific::PopulateFullScreenDialog(wxChoice *pChoice)
{
	IntClientData *pIntData;

	// desktop resolution (no resolution switch)
	wxString sResolution;
	sResolution = wxT("Use desktop resolution");
	pIntData = new IntClientData(-1);
	pChoice->Append(sResolution,pIntData);

	// null terminated list of modes
	SDL_Rect ** modes = SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_HWSURFACE);

	for (int i=0; modes[i]; ++i)
	{
		for (int d=0; d<3; d++)
		{

			wxString sResolution;
			int depth = 24;
			switch (d)
			{
			case 0:
			{
				depth = 16;
			}
			break;

			default:
			{
				depth = 24;
			}
			break;

			case 2:
			{

				depth = 32;
			}
			break;

			}

			// TODO: Fix for sdl

			// add in value for 16/24/32 depth

			// for debugging
			 wxGetApp().Log("Width: %d Height: %d BPP:%d\n", modes[i]->w, modes[i]->h, depth);
			sResolution.sprintf(wxT("%d x %d (%d bpp)"), modes[i]->w, modes[i]->h, depth);
			pIntData = new IntClientData(i|d);
			pChoice->Append(sResolution,pIntData);
		}
	}    
}

int PlatformSpecific::GetPressedButton(int index)
{
	int nButton = -1;
	if ((index>=0) && (index<SDL_NumJoysticks()))
	{
		// open this joystick 
		SDL_Joystick *pJoystick = SDL_JoystickOpen(index);
		if (pJoystick!=NULL)
		{
			for (int i=0; i<SDL_JoystickNumButtons(pJoystick); ++i)
			{
				Uint8 buttonPressed = SDL_JoystickGetButton ( pJoystick, i );
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


int PlatformSpecific::GetPressedAxis(int index)
{
	int nAxis = -1;

	if ((index>=0) && (index<SDL_NumJoysticks()))
	{
		// open this joystick 
		SDL_Joystick *pJoystick = SDL_JoystickOpen(index);
		if (pJoystick!=NULL)
		{
			// wxGetApp().Log("Testing joy: %s\n", SDL_JoystickName(pJoystick));

			//testing Axis
			for (int i=0; i < SDL_JoystickNumAxes(pJoystick); ++i)
			{
				Sint16 axisPressed = SDL_JoystickGetAxis ( pJoystick, i );
				if ((axisPressed > 10000) || (axisPressed < -10000))
				{
					nAxis = i;
					break;
				}
			}
			//testing POV
			for (int i=0; i < SDL_JoystickNumHats(pJoystick); ++i)
			{
				Uint8 PovPressed = SDL_JoystickGetHat ( pJoystick, i );
				if (PovPressed != SDL_HAT_CENTERED)
				{
					nAxis = 10 + i;
					break;
				}
			}
		}
	}
	return nAxis;
}

int PlatformSpecific::GetJoystickNumButtons(int index)
{
	int nButtons = 0;

	if ((index>=0) && (index<SDL_NumJoysticks()))
	{
		// open this joystick 
		SDL_Joystick *pJoystick = SDL_JoystickOpen(index);
		if (pJoystick!=NULL)
		{
			nButtons = SDL_JoystickNumButtons(pJoystick);

			// not id we wanted, close joysticks
			SDL_JoystickClose(pJoystick);
		}
	}
	return nButtons;
}


int PlatformSpecific::GetJoystickNumAxis(int index)
{
	int nButtons = 0;

	if ((index>=0) && (index<SDL_NumJoysticks()))
	{
		// open this joystick 
		SDL_Joystick *pJoystick = SDL_JoystickOpen(index);
		if (pJoystick!=NULL)
		{
			nButtons = SDL_JoystickNumAxes(pJoystick);

			// not id we wanted, close joysticks
			SDL_JoystickClose(pJoystick);
		}
	}
	return nButtons;
}


int PlatformSpecific::GetJoystickNumPOV(int index)
{
	int nButtons = 0;

	if ((index>=0) && (index<SDL_NumJoysticks()))
	{
		// open this joystick 
		SDL_Joystick *pJoystick = SDL_JoystickOpen(index);
		if (pJoystick!=NULL)
		{
			nButtons = SDL_JoystickNumHats(pJoystick);

			// not id we wanted, close joysticks
			SDL_JoystickClose(pJoystick);
		}
	}
	return nButtons;
}


wxString PlatformSpecific::GetJoystickButtonName(int id, int nButton)
{
	wxString sName;
	sName.Printf(wxT("Button %d"), nButton);
	return sName;
}

#endif
