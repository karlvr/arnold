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
#include "EmuWindow.h"
#include "arnguiApp.h"
#include "arnguiMain.h"
#include "../cpc/host.h"
#include <wx/joystick.h>
#include "sdlcommon/sound.h"
//#include "../cpc/audio.h"

extern "C"
{
#include "../cpc/keyjoy.h"
#include "../cpc/monitor.h"
	extern void Audio_Commit();

	extern BOOL MonitorNeedupdate(void);
}

//=====================================================================

//IMPLEMENT_DYNAMIC_CLASS(EmulationWindow, wxWindow)

//=====================================================================

BEGIN_EVENT_TABLE(EmulationWindow, wxWindow)
EVT_ERASE_BACKGROUND(EmulationWindow::OnEraseBackground)
EVT_KEY_UP(EmulationWindow::OnKeyUp)
EVT_KEY_DOWN(EmulationWindow::OnKeyDown)
EVT_JOY_BUTTON_DOWN(EmulationWindow::OnJoyButtonDown)
EVT_JOY_BUTTON_UP(EmulationWindow::OnJoyButtonDown)
EVT_JOY_MOVE(EmulationWindow::OnJoyMove)
EVT_CHAR(EmulationWindow::OnChar)
EVT_SHOW(EmulationWindow::OnShow)
// event move only on windows!!!!
EVT_SIZE(EmulationWindow::OnSize)
//	EVT_MOVE(EmulationWindow::OnMove)

#if defined(__WXGTK__) || defined(__WXMAC__)
EVT_WINDOW_CREATE(EmulationWindow::OnCreate)
#endif
EVT_KILL_FOCUS(EmulationWindow::OnKillFocus)
END_EVENT_TABLE()

//=====================================================================

void EmulationWindow::OnKillFocus(wxFocusEvent & WXUNUSED(event))
{
	/* we could pause emulation */

	/* should we just block inputs? */
	/* TODO: verify this works as well as it needs to */
	CPC_ClearKeyboard();
}

void EmulationWindow::OnMove(wxMoveEvent & WXUNUSED(event))
{
	//wxPoint Position = event.GetPosition();
	// wxGetApp().m_PlatformSpecific.SetWindowPosition(Position.x, Position.y);
//	printf("Move event\n");
}

void EmulationWindow::OnSize(wxSizeEvent & WXUNUSED(event))
{
	//	wxSize Size = event.GetSize();
	// wxGetApp().m_PlatformSpecific.SetWindowSize(Size.GetWidth(), Size.GetHeight());
	//printf("size event\n");
}


//=====================================================================

EmulationWindow::EmulationWindow() : wxWindow()
{
	Cycles = 0;
	Windowed.m_nRenderHeight = 0;
	Windowed.m_nRenderWidth = 0;
	Windowed.m_nWindowWidth = 0;
	Windowed.m_nWindowHeight = 0;
	Windowed.m_bUseHardwareScaling = false;
	Fullscreen.m_nRenderHeight = 0;
	Fullscreen.m_nRenderWidth = 0;
	Fullscreen.m_bUseHardwareScaling = false;
	m_bLastTimeStampSet = false;
	LastTimeStamp = 0;
}

//=====================================================================

void EmulationWindow::GoWindowed()
{
	DisplayType(true, true, GetHandle());
}

void EmulationWindow::GoFullScreen()
{
	DisplayType(false, false, NULL);
}

//=====================================================================

EmulationWindow::~EmulationWindow()
{
}

//=====================================================================

extern "C"
{
#include "../cpc/render.h"
#include "../cpc/host.h"
#include "../cpc/cpc.h"
#include "../cpc/autotype.h"
#include "../cpc/autorunfile.h"
#include "../cpc/joystick.h"
#include "../cpc/dumpym.h"
#include "../cpc/mouse.h"
#include "../cpc/debugger/gdebug.h"

}


#if ((defined(GTK2_EMBED_WINDOW) && defined(__WXGTK__)) || (defined(MAC_EMBED_WINDOW) && defined(__WXMAC__)))

// TODO: Make these user defineable
static ActionKey ActionKeys[] =
{
	// windows version has these set
	{ WXK_F2, false, false, false, ACTION_TOGGLE_FULLSCREEN },
	//{ SDL_SCANCODE_NUMLOCKCLEAR, false, false, false, ACTION_TOGGLE_KEYSTICK_ACTIVE },
	// {SDLK_F3, false, false, false, ACTION_INSERT_TAPE},
	// {SDLK_F4, false, false, false, ACTION_SAVE_SNAPSHOT},
	{ WXK_F5, false, false, false, ACTION_CPC_RESET },
	{ WXK_F5, true, false, false, ACTION_CPC_RESET_IMMEDIATE },
	{ WXK_F6, false, false, false, ACTION_INSERT_DISK_DRIVE_A },
	{ WXK_F7, false, false, false, ACTION_INSERT_DISK_DRIVE_B },
	{ WXK_F10, false, false, false, ACTION_QUIT_IMMEDIATE },
	//  {SDLK_F10, true, false, false, ACTION_QUIT_IMMEDIATE},
	{ WXK_PRINT, false, false, false, ACTION_SAVE_SCREENSHOT },

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
#endif

static CPC_KEY_ID WXKeyCodes[400];

//=====================================================================

EmulationWindow::EmulationWindow(bool bFullScreen, wxWindow *parent,
	wxWindowID id,
	const wxPoint &pos,
	const wxSize &size,
	long style,
	const wxString &name)
	// WANTS_CHARS allows our sdl embedded window getting return and other windows keys
	: wxWindow(parent, id, pos, size, style | wxWANTS_CHARS, name)
{
	Cycles = 0;
	Windowed.m_nRenderHeight = 0;
	Windowed.m_nRenderWidth = 0;
	Windowed.m_nWindowWidth = 0;
	Windowed.m_nWindowHeight = 0;
	Windowed.m_bUseHardwareScaling = false;
	Fullscreen.m_nRenderHeight = 0;
	Fullscreen.m_nRenderWidth = 0;
	Fullscreen.m_bUseHardwareScaling = false;
	m_bLastTimeStampSet = false;
	LastTimeStamp = 0;

#if defined(__WXMSW__)
	DoCreate();
#endif

	LastTimeStamp = 0;
	m_bLastTimeStampSet = false;

	for (unsigned int i = 0; i < sizeof(WXKeyCodes) / sizeof(WXKeyCodes[0]); i++)
	{
		WXKeyCodes[i] = CPC_KEY_NULL;
	}

	/* International key mappings */
	WXKeyCodes[(wxChar)'0'] = CPC_KEY_ZERO;
	WXKeyCodes[(wxChar)'1'] = CPC_KEY_1;
	WXKeyCodes[(wxChar)'2'] = CPC_KEY_2;
	WXKeyCodes[(wxChar)'3'] = CPC_KEY_3;
	WXKeyCodes[(wxChar)'4'] = CPC_KEY_4;
	WXKeyCodes[(wxChar)'5'] = CPC_KEY_5;
	WXKeyCodes[(wxChar)'6'] = CPC_KEY_6;
	WXKeyCodes[(wxChar)'7'] = CPC_KEY_7;
	WXKeyCodes[(wxChar)'8'] = CPC_KEY_8;
	WXKeyCodes[(wxChar)'9'] = CPC_KEY_9;
	WXKeyCodes[(wxChar)'a'] = CPC_KEY_A;
	WXKeyCodes[(wxChar)'b'] = CPC_KEY_B;
	WXKeyCodes[(wxChar)'c'] = CPC_KEY_C;
	WXKeyCodes[(wxChar)'d'] = CPC_KEY_D;
	WXKeyCodes[(wxChar)'e'] = CPC_KEY_E;
	WXKeyCodes[(wxChar)'f'] = CPC_KEY_F;
	WXKeyCodes[(wxChar)'g'] = CPC_KEY_G;
	WXKeyCodes[(wxChar)'h'] = CPC_KEY_H;
	WXKeyCodes[(wxChar)'i'] = CPC_KEY_I;
	WXKeyCodes[(wxChar)'j'] = CPC_KEY_J;
	WXKeyCodes[(wxChar)'k'] = CPC_KEY_K;
	WXKeyCodes[(wxChar)'l'] = CPC_KEY_L;
	WXKeyCodes[(wxChar)'m'] = CPC_KEY_M;
	WXKeyCodes[(wxChar)'n'] = CPC_KEY_N;
	WXKeyCodes[(wxChar)'o'] = CPC_KEY_O;
	WXKeyCodes[(wxChar)'p'] = CPC_KEY_P;
	WXKeyCodes[(wxChar)'q'] = CPC_KEY_Q;
	WXKeyCodes[(wxChar)'r'] = CPC_KEY_R;
	WXKeyCodes[(wxChar)'s'] = CPC_KEY_S;
	WXKeyCodes[(wxChar)'t'] = CPC_KEY_T;
	WXKeyCodes[(wxChar)'u'] = CPC_KEY_U;
	WXKeyCodes[(wxChar)'v'] = CPC_KEY_V;
	WXKeyCodes[(wxChar)'w'] = CPC_KEY_W;
	WXKeyCodes[(wxChar)'x'] = CPC_KEY_X;
	WXKeyCodes[(wxChar)'y'] = CPC_KEY_Y;
	WXKeyCodes[(wxChar)'z'] = CPC_KEY_Z;
	WXKeyCodes[(wxChar)'A'] = CPC_KEY_A;
	WXKeyCodes[(wxChar)'B'] = CPC_KEY_B;
	WXKeyCodes[(wxChar)'C'] = CPC_KEY_C;
	WXKeyCodes[(wxChar)'D'] = CPC_KEY_D;
	WXKeyCodes[(wxChar)'E'] = CPC_KEY_E;
	WXKeyCodes[(wxChar)'F'] = CPC_KEY_F;
	WXKeyCodes[(wxChar)'G'] = CPC_KEY_G;
	WXKeyCodes[(wxChar)'H'] = CPC_KEY_H;
	WXKeyCodes[(wxChar)'I'] = CPC_KEY_I;
	WXKeyCodes[(wxChar)'J'] = CPC_KEY_J;
	WXKeyCodes[(wxChar)'K'] = CPC_KEY_K;
	WXKeyCodes[(wxChar)'L'] = CPC_KEY_L;
	WXKeyCodes[(wxChar)'M'] = CPC_KEY_M;
	WXKeyCodes[(wxChar)'N'] = CPC_KEY_N;
	WXKeyCodes[(wxChar)'O'] = CPC_KEY_O;
	WXKeyCodes[(wxChar)'P'] = CPC_KEY_P;
	WXKeyCodes[(wxChar)'Q'] = CPC_KEY_Q;
	WXKeyCodes[(wxChar)'R'] = CPC_KEY_R;
	WXKeyCodes[(wxChar)'S'] = CPC_KEY_S;
	WXKeyCodes[(wxChar)'T'] = CPC_KEY_T;
	WXKeyCodes[(wxChar)'U'] = CPC_KEY_U;
	WXKeyCodes[(wxChar)'V'] = CPC_KEY_V;
	WXKeyCodes[(wxChar)'W'] = CPC_KEY_W;
	WXKeyCodes[(wxChar)'X'] = CPC_KEY_X;
	WXKeyCodes[(wxChar)'Y'] = CPC_KEY_Y;
	WXKeyCodes[(wxChar)'Z'] = CPC_KEY_Z;
	WXKeyCodes[WXK_SPACE] = CPC_KEY_SPACE;

	WXKeyCodes[(wxChar)'['] = CPC_KEY_AT;
	WXKeyCodes[(wxChar)']'] = CPC_KEY_OPEN_SQUARE_BRACKET;
	WXKeyCodes[(wxChar)'{'] = CPC_KEY_AT;
	WXKeyCodes[(wxChar)'}'] = CPC_KEY_OPEN_SQUARE_BRACKET;

	WXKeyCodes[(wxChar)','] = CPC_KEY_COMMA;
	WXKeyCodes[(wxChar)'<'] = CPC_KEY_COMMA;
	WXKeyCodes[(wxChar)'.'] = CPC_KEY_DOT;
	WXKeyCodes[(wxChar)'>'] = CPC_KEY_DOT;
	WXKeyCodes[(wxChar)'/'] = CPC_KEY_BACKSLASH;
	WXKeyCodes[(wxChar)':'] = CPC_KEY_COLON;
	WXKeyCodes[(wxChar)';'] = CPC_KEY_COLON;
	WXKeyCodes[(wxChar)'\''] = CPC_KEY_SEMICOLON;
	WXKeyCodes[(wxChar)'@'] = CPC_KEY_SEMICOLON;
	WXKeyCodes[(wxChar)'#'] = CPC_KEY_CLOSE_SQUARE_BRACKET;
	WXKeyCodes[(wxChar)'~'] = CPC_KEY_CLOSE_SQUARE_BRACKET;
	WXKeyCodes[(wxChar)'-'] = CPC_KEY_MINUS;
	WXKeyCodes[(wxChar)'+'] = CPC_KEY_HAT;
	WXKeyCodes[(wxChar)'='] = CPC_KEY_HAT;


	WXKeyCodes[WXK_SHIFT] = CPC_KEY_SHIFT;
	WXKeyCodes[WXK_ALT] = CPC_KEY_CONTROL;
	WXKeyCodes[WXK_ESCAPE] = CPC_KEY_ESC;
	WXKeyCodes[WXK_TAB] = CPC_KEY_TAB;
	WXKeyCodes[WXK_RETURN] = CPC_KEY_RETURN;
	WXKeyCodes[WXK_BACK] = CPC_KEY_DEL;
	WXKeyCodes[WXK_UP] = CPC_KEY_CURSOR_UP;
	WXKeyCodes[WXK_DOWN] = CPC_KEY_CURSOR_DOWN;
	WXKeyCodes[WXK_LEFT] = CPC_KEY_CURSOR_LEFT;
	WXKeyCodes[WXK_RIGHT] = CPC_KEY_CURSOR_RIGHT;


// ON MAC
	// 306 with no mod is left shift
	// 306 with 4 mod is right shift
	// 308 is apple left
	// 308 with 4 mod is apple right
	// caps lock
	WXKeyCodes[311] = CPC_KEY_CAPS_LOCK;
	// control
	WXKeyCodes[396] = CPC_KEY_CONTROL;


	WXKeyCodes[WXK_NUMPAD0] = CPC_KEY_F0;
	WXKeyCodes[WXK_NUMPAD1] = CPC_KEY_F1;
	WXKeyCodes[WXK_NUMPAD2] = CPC_KEY_F2;
	WXKeyCodes[WXK_NUMPAD3] = CPC_KEY_F3;
	WXKeyCodes[WXK_NUMPAD4] = CPC_KEY_F4;
	WXKeyCodes[WXK_NUMPAD5] = CPC_KEY_F5;
	WXKeyCodes[WXK_NUMPAD6] = CPC_KEY_F6;
	WXKeyCodes[WXK_NUMPAD7] = CPC_KEY_F7;
	WXKeyCodes[WXK_NUMPAD8] = CPC_KEY_F8;
	WXKeyCodes[WXK_NUMPAD9] = CPC_KEY_F9;
	WXKeyCodes[WXK_NUMPAD_INSERT] = CPC_KEY_F0;
	WXKeyCodes[WXK_NUMPAD_END] = CPC_KEY_F1;
	WXKeyCodes[WXK_NUMPAD_DOWN] = CPC_KEY_F2;
	WXKeyCodes[WXK_NUMPAD_PAGEDOWN] = CPC_KEY_F3;
	WXKeyCodes[WXK_NUMPAD_LEFT] = CPC_KEY_F4;
	WXKeyCodes[WXK_NUMPAD_BEGIN] = CPC_KEY_F5;
	WXKeyCodes[WXK_NUMPAD_RIGHT] = CPC_KEY_F6;
	WXKeyCodes[WXK_NUMPAD_HOME] = CPC_KEY_F7;
	WXKeyCodes[WXK_NUMPAD_UP] = CPC_KEY_F8;
	WXKeyCodes[WXK_NUMPAD_PAGEUP] = CPC_KEY_F9;

	WXKeyCodes[WXK_NUMPAD_DECIMAL] = CPC_KEY_FDOT;
	WXKeyCodes[WXK_NUMPAD_DELETE] = CPC_KEY_FDOT;
	WXKeyCodes[WXK_NUMPAD_ENTER] = CPC_KEY_SMALL_ENTER;

	WXKeyCodes[WXK_DELETE] = CPC_KEY_CLR;
	WXKeyCodes[WXK_END] = CPC_KEY_COPY;

}

void EmulationWindow::UpdateSize(bool bFullScreen)
{
	if (!bFullScreen)
	{

		// find size of client area based on display chosen
		int RenderWidth, RenderHeight;
		Render_GetWindowedDisplayDimensions(&RenderWidth, &RenderHeight);

		int WindowWidth = (RenderWidth*CPC_GetWindowScale()) / 100;
		int WindowHeight = (RenderHeight*CPC_GetWindowScale()) / 100;

		Windowed.m_nWindowWidth = WindowWidth;
		Windowed.m_nWindowHeight = WindowHeight;
		Windowed.m_nRenderWidth = RenderWidth;
		Windowed.m_nRenderHeight = RenderHeight;
		Windowed.m_bUseHardwareScaling = true;

		// if we can't get it into a window, don't do this
#if defined(GTK2_EMBED_WINDOW) || defined(MAC_EMBED_WINDOW) || defined(WIN_EMBED_WINDOW)


		// get parent window which will be the frame
		wxWindow *pParentWindow = GetParent();
		// get frame
		wxFrame *pFrame = (wxFrame *)pParentWindow;

		// set client size
		pFrame->SetClientSize(WindowWidth, WindowHeight);

		//printf("Wanted: Width: %d Height: %d\n", WindowWidth, WindowHeight);

		pFrame->GetSize(&WindowWidth, &WindowHeight);

		int clientX;
		int clientY;
		int clientWidth;
		int clientHeight;

		// get desktop size...
		// wxwidgets gets a better size
		::wxClientDisplayRect(&clientX, &clientY, &clientWidth, &clientHeight);
		//printf("Client width: %d height: %d\n", clientWidth, clientHeight);
		if (WindowWidth > clientWidth)
		{
			WindowWidth = clientWidth;
		}
		if (WindowHeight > clientHeight)
		{
			WindowHeight = clientHeight;
		}
		pFrame->SetSize(clientX, clientY, WindowWidth, WindowHeight);

		// set client size
		pFrame->GetClientSize(&WindowWidth, &WindowHeight);

		// we potentially made a smaller window.
#endif

		// send width, height to window so sdl sets it's window accordingly.
		Init(bFullScreen, (void *)this->GetHandle());

	}
	else
	{
		// send width, height to window so sdl  it's window accordingly.
		Init(bFullScreen, (void *)this->GetHandle());
	}
}

void EmulationWindow::DoCreate()
{
	bool bFullScreen = false;

	UpdateSize(bFullScreen);

	//printf("On Create\n");

	//Init(bFullScreen, (void *)this->GetHandle());

	Monitor_SetRendererReady(TRUE);

	//printf("Done init\r\n");

	if (!bFullScreen)
	{
		// setup display
		// get parent window which will be the frame
		wxWindow *pParentWindow = GetParent();
		// get frame
		wxFrame *pFrame = (wxFrame *)pParentWindow;
		pFrame->CentreOnScreen();
	}
	
}

void EmulationWindow::OnCreate(wxWindowCreateEvent & WXUNUSED(event))
{
	DoCreate();
}

bool EmulationWindow::Init(bool bFullScreen, void *pHandle)
{
	if (bFullScreen)
	{
		// setup display
		// get parent window which will be the frame
		wxWindow *pParentWindow = GetParent();
		// get frame
		wxFrame *pFrame = (wxFrame *)pParentWindow;
		pFrame->ShowFullScreen(true);
	}

	wxGetApp().m_PlatformSpecific.SetupDisplay(bFullScreen, &Windowed, pHandle);
	return true;
}

//=====================================================================

void EmulationWindow::OnChar(wxKeyEvent &event)
{
#if ((defined(GTK2_EMBED_WINDOW) && defined(__WXGTK__)) || (defined(MAC_EMBED_WINDOW) && defined(__WXMAC__)))
	if (!Debug_IsStopped())
	{
		// doesn't capture tab, return and chars like that
		// but does capture most symbols
		if (Keyboard_GetMode() == 1)
		{
			if (event.GetUnicodeKey() != 0)
			{
				if (!m_bLastTimeStampSet)
				{
					LastTimeStamp = event.GetTimestamp();
					m_bLastTimeStampSet = true;
				}
				StoredKeyEvent storedEvent;
				storedEvent.SetChar(event.GetTimestamp(), event.GetUnicodeKey(), event.ShiftDown(), event.ControlDown(), event.AltDown());
				StoredKeys.push_back(storedEvent);

#ifndef __WXMAC__
				// if we allow it to continue on mac it makes a beeping noise
				event.Skip();
#endif
			}
		}
	}
#endif
}

//=====================================================================

void EmulationWindow::HandleKey(wxKeyEvent &event)
{
//			printf("%d mod: %d\n", event.GetKeyCode(), event.GetModifiers());
#if defined(MAC_EMBED_WINDOW)
//printf("mac embed window\n");
#endif
#if defined(__WXMAC__)
//printf("wxmac defined\n");
#endif

#if ((defined(GTK2_EMBED_WINDOW) && defined(__WXGTK__)) || (defined(MAC_EMBED_WINDOW) && defined(__WXMAC__)))
	if (!Debug_IsStopped())
	{
		const int keyCode = event.GetKeyCode();

		/* Need to allow non-unicode keys through regardless of keyboard mode, as keyboard mode 1 only handles unicode keys.
		   But we don't want to allow modifier keys through by themselves as they confuse the CPC when translated mode
		   types a key while we've reported that SHIFT is down (e.g. try typing a colon in translated, it would have output a *)
		 */
		if (Keyboard_GetMode() == 0 || (event.GetUnicodeKey() == 0 && (keyCode != WXK_SHIFT && keyCode != WXK_ALT && keyCode != WXK_CONTROL)))
		{
			if (!m_bLastTimeStampSet)
			{
				LastTimeStamp = event.GetTimestamp();
				m_bLastTimeStampSet = true;
			}

			StoredKeyEvent storedEvent;
			storedEvent.SetKey(event.GetTimestamp(), keyCode, event.GetEventType() == wxEVT_KEY_DOWN, event.ShiftDown(), event.ControlDown(), event.AltDown());
			StoredKeys.push_back(storedEvent);
		//	printf("%d mod: %d\n", event.GetKeyCode(), event.GetModifiers());

			if (keyCode == WXK_CAPITAL) {
				/* Caps lock is only reported by WX as a key down, never a key up, so we simulate a key
				   up in the future (by the number of ms that LastTimeStamp is incremented) so the CPC
				   sees the key go back up next frame.
				 */
				StoredKeyEvent storedEvent2;
				storedEvent2.SetKey(event.GetTimestamp() + 21, keyCode, 0, event.ShiftDown(), event.ControlDown(), event.AltDown());
				StoredKeys.push_back(storedEvent2);

				LastTimeStamp = event.GetTimestamp();
			}
		}
	}
#endif
}

//=====================================================================

void EmulationWindow::OnJoyButtonDown(wxJoystickEvent &event)
{
	event.Skip();
}

void EmulationWindow::OnJoyButtonUp(wxJoystickEvent &event)
{
	event.Skip();
}

void EmulationWindow::OnJoyMove(wxJoystickEvent &event)
{
	event.Skip();
}


//=====================================================================

void EmulationWindow::OnKeyUp(wxKeyEvent &event)
{
	HandleKey(event);
	event.Skip();
}

//=====================================================================

void EmulationWindow::OnKeyDown(wxKeyEvent &event)
{
	HandleKey(event);

	event.Skip();
}

//=====================================================================

void EmulationWindow::DisplayType(bool bWindowed, bool WXUNUSED(bShowFrame), void *pWindowHandle)
{

	//  wxGetApp().m_PlatformSpecific.SetupDisplay(!bWindowed, pWindowHandle);
}

//=====================================================================

void EmulationWindow::OnShow(wxShowEvent & WXUNUSED(event))
{
	// wxGetApp().m_PlatformSpecific.SetupDisplay(false, GetHandle());
	//printf("OnShow event\n");
	//	Init(false, (void *)this->GetHandle());

}

//=====================================================================

void EmulationWindow::OnEraseBackground(wxEraseEvent & WXUNUSED(event))
{

}

//=====================================================================

void EmulationWindow::OnIdle(wxIdleEvent & WXUNUSED(event))
{
	// 1. Do not attempt to throttle if debugging (at any time)
	// 2. if debugging, don't update frame specific functions, unless a frame's worth of cycles has been done
	// 3. when input comes, do we put it into order and replay it through the frame to get a better result. Or run two threads


	/* is debugger stopped ? */
	if (!Debug_IsStopped())
	{
		/* handle sdl events - for keyboard, joystick etc, we probably need to ignore events when stopped? */
		wxGetApp().m_PlatformSpecific.HandleEvents();

		/* if running normally, keep looping until we've done a frame */
		while (Cycles < 19968)
		{
			int NopCount;

			/* execute the instruction */
			NopCount = Debugger_Execute();
			if (NopCount == 0)
			{
				break;
			}
			CPC_ExecuteCycles(NopCount);
			Audio_Update(NopCount);

			if (MonitorNeedupdate())
			{
				wxGetApp().m_PlatformSpecific.DrawDisplay();
			}

			Cycles += NopCount;
		}

		/* if debugger has stopped */
		if (Debug_IsStopped())
		{
			SDLCommon::PauseSound(true);
		}
		else
		{
			/* are we resuming sound? */
			if (SDLCommon::IsResuming())
			{
				/* update timer */
				SDLCommon::UpdatePauseTimer(19968);
			}
		}

		// if we've done a frame, perform frame updates
		if (Cycles >= 19968)
		{
			Cycles -= 19968;

			YMOutput_Update();

			// if autorunfile is active, update it.
			if (AutoRunFile_Active())
			{
	//			printf("auto run is active\n");

				AutoRunFile_Update();
			}

			/* TODO: disable joystick on keyboard when using Autotype */
//			Joystick_KeyStickActive
			/* auto type active? */
			if (AutoType_Active())
			{
//				printf("auto type is active\n");
				/* update it */
				AutoType_Update();
			}

			// TODO: If autotype is active block all other keys

			// when there are no events reset movement back. This means to see movement you need to keep dragging
			// ok for some games, not for others.
			if (AutoType_Active())
			{
				//		for (int i = 0; i<CPC_NUM_JOYSTICKS; i++)
				//		{
				//			if (Joystick_GetType(i) == JOYSTICK_TYPE_SIMULATED_BY_MOUSE)
				//			{
				//				Joystick_SetXMovement(i, 0);
				//				Joystick_SetYMovement(i, 0);
				//			}
				//		}
			}

			// ensure joysticks are updated
			Joystick_Update();

			// copy real keys to resolved keys
			CPC_PreResolveKeys();

			// then resolve key joy over that
			KeyJoy_Resolve();

			// always do last after keyboard and joystick updates

			CPC_GenerateKeyboardClash();

			Audio_Commit();
#if ((defined(GTK2_EMBED_WINDOW) && defined(__WXGTK__)) || (defined(MAC_EMBED_WINDOW) && defined(__WXMAC__)))

			// milliseconds = 1000 mills in a second
			// 50th of a second is 20 milliseconds

			LastTimeStamp += 20;

			size_t nRemoveBefore = 0;

			for (size_t i = 0; i < StoredKeys.size(); i++)
			{
				if (StoredKeys[i].m_TimeStamp <= LastTimeStamp)
				{
					if (StoredKeys[i].m_bIsChar)
					{
						// this will not work with accented keys
						wchar_t wstr[2];
						wstr[0] = StoredKeys[i].m_nChar;
						wstr[1] = L'\0';
						wxString str(wstr);
						const wxCharBuffer buffer = str.utf8_str();
						wxGetApp().SetTranslatedKey(buffer.data());
					}
					else if (StoredKeys[i].m_KeyCode)
					{
						CPC_KEY_ID	theKeyPressed;
						int keycode = StoredKeys[i].m_KeyCode;

						// if key stick is active and key used by keystick then block
						if (Joystick_IsKeyStickActive() && Joystick_IsKeyUsedByKeyStick(keycode))
						{
						}
						else
						{

							if (keycode >= (sizeof(WXKeyCodes) / sizeof(WXKeyCodes[0])))
							{
								//printf("Keycode: %d not handled\n", keycode);
							}
							else
							{
								theKeyPressed = (CPC_KEY_ID)WXKeyCodes[keycode];

								if (theKeyPressed != CPC_KEY_NULL)
								{
									// set or release key depending on state
									if (StoredKeys[i].m_bKeyDown)
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
								else
								{
									//printf("Keycode %d not mapped\n", keycode);
								}
							}
						}
					}

					{
						// process action keys
						if (StoredKeys[i].m_bKeyDown)
						{
							for (size_t j = 0; j < sizeof(ActionKeys) / sizeof(ActionKeys[0]); j++)
							{
								if (
									// keycodes
									((int)ActionKeys[j].m_nKeyCode == (int)StoredKeys[i].m_KeyCode) &&
									// now check modifiers
									(
									// shift pressed
									(ActionKeys[j].m_bShift && StoredKeys[i].m_bIsShift) ||
									// shift not pressed
									(!ActionKeys[j].m_bShift && !StoredKeys[i].m_bIsShift)
									) &&
									(
									// control pressed
									(ActionKeys[j].m_bControl && StoredKeys[i].m_bIsControl) ||
									// control not pressed
									(!ActionKeys[j].m_bControl && !StoredKeys[i].m_bIsControl)
									) &&
									(
									(ActionKeys[j].m_bAlt && StoredKeys[i].m_bIsAlt) ||
									(!ActionKeys[j].m_bAlt && !StoredKeys[i].m_bIsAlt)
									)
									)
								{
									wxGetApp().ProcessAction(ActionKeys[j].m_Action);
									break;
								}
							}
						}
					}


					/* handle joysticks simulated by keys */
						{
							for (int j = 0; j < CPC_NUM_JOYSTICKS; j++)
							{
								if ((Joystick_GetType(j) == JOYSTICK_TYPE_SIMULATED_BY_KEYBOARD) && (Joystick_IsActive(j)))
								{
									for (int k = 0; k < JOYSTICK_SIMULATED_KEYID_LAST; k++)
									{
										if (StoredKeys[i].m_KeyCode == Joystick_GetSimulatedKeyID(j, k))
										{
											Joystick_SetSimulatedKeyIDState(j, k, (StoredKeys[i].m_bKeyDown));
										}
									}
								}
							}
						}

						nRemoveBefore++;
				}
				else
				{
					// store the timestamp so we can process this next time
					LastTimeStamp = StoredKeys[i].m_TimeStamp;
					break;
				}
			}

			if (nRemoveBefore != 0)
			{
				StoredKeys.erase(StoredKeys.begin(), StoredKeys.begin() + nRemoveBefore);
			}
#endif
			// if debugger stopped us, don't do throttle
			// need to determine if we're running, or running to something
			// if we're running to something then we don't need to throttle because
			// we are debugging and timing will be inaccurate
			if (!Debug_IsStopped())
			{
				// throttle only if not stopped
				wxGetApp().m_PlatformSpecific.ThrottleSpeed(CPC_GetEmuSpeedPercent());
			}

			CPC_ExecuteUpdateFunctions();

			//wxGetApp().m_PlatformSpecific.DrawDisplay();
		}
	}
}

//=====================================================================

