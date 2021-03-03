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
#ifndef __SDL2_PLATFORM_SPECIFIC_HEADER_INCLUDED__
#define __SDL2_PLATFORM_SPECIFIC_HEADER_INCLUDED__

#ifdef USE_SDL2

#ifdef _MSC_VER
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

extern "C"
{
#include "../../cpc/render.h"
}


#include <wx/listbox.h>
#include <wx/window.h>
#include <wx/choice.h>

#include "../KeyDefStruct.h"
class PlatformSpecific
{
public:
	unsigned long GetMinTime();
	unsigned long GetMaxTime();
	unsigned long GetAverageTime();
	int GetAverageFPS();
	void ResetTiming();
	PlatformSpecific();
	~PlatformSpecific();
	void Shutdown();
	bool Init(bool bWantAudio, bool bWantJoysticks);
	void HandleEvents();
	void DrawDisplay();
	void ConfigureJoystickAsMouse(int nJoystick);
	void ConfigureJoystickKeySet(int nJoystick, int nSet);
	void ConfigureKeyboardMode();
	void ConfigureAudio();
	void ThrottleSpeed(int nPercent);
	void InitKeySets();

	//Keyboard
	wxString GetKeyName(int id);
	void PopulateKeyDialog(wxChoice *pChoice);
	int *GetKeySetPtr(int nSet);
	void SetKeySet(int nSet);
	void BeginPressedKey();
	void EndPressedKey();
	void SetupKeyList(CPCKeyDefData *);
	void WriteKeyList(const CPCKeyDefData *);

	void DisplayMessage(wxString s_Text);

	//display
	void SetupDisplay(bool bFullScreen, WindowedRenderSettings *pWindowed, void *pWindowHandle);
	void PopulateFullScreenDialog(wxChoice *pChoice);
	bool IsFullScreen() { return m_bFullScreenActive; }
	void SetWindowPosition(int x, int y);
	void SetWindowSize(int width, int height);

	// touch
	void AutoConfigureTouch();


	//joystick
	void AutoConfigureJoystick();
	void DisableJoystickEvents();
	void EnableJoystickEvents();
	void PopulateJoystickDialog(wxListBox *pList);
	void CloseJoysticks();
	void RefreshJoysticks();
	int GetPressedButton(SDL_JoystickID nJoystick);
	int GetPressedAxis(SDL_JoystickID nJoystick);
	wxString GetJoystickButtonName(SDL_JoystickID id, int nButton);
	int GetJoystickNumAxis(SDL_JoystickID index);
	int GetJoystickNumPOV(SDL_JoystickID index);
	int GetJoystickNumButtons(SDL_JoystickID index);
	wxString GetJoystickIdString(SDL_JoystickID id);
	int GetJoystickIdFromString(wxString &);
	int JoystickFromID(SDL_JoystickID idWanted);
	void ClearHostKeys();
	void SetHostKey(CPC_KEY_ID CPCKey, const int *Keys);
	void GetHostKey(CPC_KEY_ID CPCKey, int *Keys);

private:
	void SetKeySet(int j, int *keys);
	void HandleControllerAxis(SDL_JoystickID id, int axis, Sint16 value);
	void HandleControllerButtons(SDL_JoystickID id, int button, bool bPressed);

	bool m_bDisableKeyboardEvents;
	bool m_bDisableJoystickEvents;
	bool m_bHasAudio;
	bool m_bHasJoystick;
	bool m_bFullScreenActive;
	bool bActive;
	void ShutdownInput();
	void ShutdownDisplay();
	void RestartDisplay();
	void ShutdownJoystick();
	void HandleKey(SDL_KeyboardEvent *theEvent);

	SDL_Window *m_pScreen;
	SDL_Renderer *m_pRenderer;
	SDL_Texture *m_pTexture;
	SDL_GLContext *m_pContext;
	wxArrayPtrVoid m_OpenedJoysticks;
	wxArrayPtrVoid m_OpenedGameControllers;

	short m_last_statePOV;

	SDL_Rect SourceRect;
	SDL_Rect DestinationRect;
	RENDER_BUFFER_INFO BufferInfo;
};

#endif

#endif


