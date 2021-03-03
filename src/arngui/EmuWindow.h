/*
*  Arnold emulator (c) Copyright, Kevin Thacker 1995-2015
*
*  This file is part of the Arnold emulator source code distribution.
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any version.
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
#ifndef __EMU_WINDOW_HEADER_INCLUDED__
#define __EMU_WINDOW_HEADER_INCLUDED__

#include <wx/wx.h>
#include <wx/event.h>
#include "arnguiApp.h"
#include <vector>

class StoredKeyEvent
{
public:
	StoredKeyEvent()
	{
		m_TimeStamp = 0;
		m_bIsChar = false;
		m_nChar = 0;
		m_KeyCode = 0;
		m_bKeyDown = false;
		m_bIsShift = false;
		m_bIsAlt = false;
		m_bIsControl = false;
	}

	void SetChar(long TimeStamp, wxChar ch, bool bShift, bool bControl, bool bAlt)
	{
		m_TimeStamp = TimeStamp;
		m_bIsChar = true;
		m_nChar = ch;
		m_bIsShift = bShift;
		m_bIsControl = bControl;
		m_bIsAlt = bAlt;
	}
	void SetKey(long TimeStamp, int Key, bool down, bool bShift, bool bControl, bool bAlt)
	{
		m_TimeStamp = TimeStamp;
		m_bIsChar = false;
		m_KeyCode = Key;
		m_bKeyDown = down;
		m_bIsShift = bShift;
		m_bIsControl = bControl;
		m_bIsAlt = bAlt;

	}

	long m_TimeStamp;
	bool m_bIsChar;
	bool m_bKeyDown;
	bool m_bIsShift;
	bool m_bIsControl;
	bool m_bIsAlt;
	wxChar m_nChar;
	int m_KeyCode;
};



class EmulationWindow : public wxWindow
{
	//  DECLARE_DYNAMIC_CLASS(EmulationWindow)
public:
	EmulationWindow();
	EmulationWindow(bool bFullScreen, wxWindow *parent, wxWindowID id, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = 0, const wxString &name = wxPanelNameStr);
	virtual ~EmulationWindow();
	void DoCreate();

	void DisplayType(bool bWindowed, bool bShowFrame, void *);

	void OnIdle(wxIdleEvent &event);
	void GoFullScreen();
	void GoWindowed();
	void UpdateSize(bool bFullScreen);

	bool Init(bool bFullscreen, void *pHandle);
private:
	int Cycles;
	void OnEraseBackground(wxEraseEvent &event);
	void OnShow(wxShowEvent &event);
	void OnKeyDown(wxKeyEvent &event);
	void OnKeyUp(wxKeyEvent &event);
	void OnJoyButtonDown(wxJoystickEvent &event);
	void OnJoyButtonUp(wxJoystickEvent &event);
	void OnJoyMove(wxJoystickEvent &event);
	void OnChar(wxKeyEvent &event);
	void OnKillFocus(wxFocusEvent &event);
	void OnCreate(wxWindowCreateEvent &event);
	void OnMove(wxMoveEvent &event);
	void OnSize(wxSizeEvent &event);
	void HandleKey(wxKeyEvent &event);

	WindowedRenderSettings Windowed;
	FullscreenRenderSettings Fullscreen;

	bool m_bLastTimeStampSet;
	long LastTimeStamp;
	std::vector<StoredKeyEvent> StoredKeys;

	DECLARE_EVENT_TABLE()
};

#endif
