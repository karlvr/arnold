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
#ifndef __SDL_PLATFORM_SPECIFIC_HEADER_INCLUDED__
#define __SDL_PLATFORM_SPECIFIC_HEADER_INCLUDED__

#ifdef USE_SDL

#ifdef _MSC_VER
#include <SDL.h>
#else
#include <SDL/SDL.h>
#endif

#include <wx/listbox.h>
#include <wx/window.h>
#include <wx/choice.h>

extern "C"
{
#include "../../cpc/render.h"
}
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
  void CloseJoysticks();
void HandleEvents();
void DrawDisplay();
void ConfigureJoystickAsMouse(int nJoystick);
void ConfigureJoystickKeySet(int nJoystick, int nSet);
void ConfigureKeyboardMode();
void RefreshJoysticks();
void ConfigureAudio();
bool IsFullScreen() { return m_bFullScreenActive; }
wxString GetJoystickIdString(int id);
int GetJoystickIdFromString(wxString &);
wxString GetKeyName(int id);
int *GetKeySetPtr(int nKeySet);
void ClearHostKeys();
void SetHostKey(CPC_KEY_ID CPCKey, const int *Keys);
void GetHostKey(CPC_KEY_ID CPCKey, int *Keys);
	void DisableJoystickEvents();
	void EnableJoystickEvents();
	void SetupKeyList(CPCKeyDefData *);
	void WriteKeyList(const CPCKeyDefData *);
	void InitKeySets();
wxString GetJoystickButtonName(int  id, int nButton);
int GetJoystickNumButtons(int id);
void SetupDisplay(bool bFullScreen, WindowedRenderSettings *pWindowed, void *pWindowHandle);
void AutoConfigureJoystick();
void AutoConfigureTouch();
void SetKeySet(int nSet);
void PopulateJoystickDialog(wxListBox *pList);
void PopulateFullScreenDialog(wxChoice *pChoice);
void PopulateKeyDialog(wxChoice *pChoice);
void ThrottleSpeed(int nPercent);
int GetPressedButton(int id);
		int JoystickFromID(int idWanted);
	int GetJoystickNumAxis(int id);
		int GetJoystickNumPOV(int id);
			int GetPressedAxis(int id);
void BeginPressedKey();
int CheckPressedKey();
void EndPressedKey();
void SetWindowPosition(int x,int y);
void SetWindowSize(int width, int height);

  private:
    void SetKeySet(int j, int *keys);
  void HandleControllerAxis(int index, int axis, Sint16 value);
  bool m_bDisableJoystickEvents;

  bool m_bHasAudio;
bool m_bHasJoystick;
bool m_bFullScreenActive;
    bool bActive;
  void ShutdownInput();
  void ShutdownDisplay();
  void ShutdownJoystick();
  	void	HandleKey(SDL_KeyboardEvent *theEvent);
  SDL_Surface *m_pScreen;
    wxArrayPtrVoid m_OpenedJoysticks;
	SDL_Surface* m_pBackBuffer;
    bool m_bDisableKeyboardEvents;
		short m_last_statePOV;
		SDL_Rect SourceRect;
		SDL_Rect DestinationRect;

			RENDER_BUFFER_INFO BufferInfo;
};

#endif

#endif
