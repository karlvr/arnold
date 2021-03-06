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
#ifndef __JOYSTICK_BUTTON_DIALOG_HEADER_INCLUDED__
#define __JOYSTICK_BUTTON_DIALOG_HEADER_INCLUDED__

#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/timer.h>

#ifndef wxOVERRIDE
#define wxOVERRIDE
#endif

class JoystickButtonDialog : public wxDialog
{
public:
	~JoystickButtonDialog();
	JoystickButtonDialog(wxWindow *pParent, int nJoystickID, int nCPCJoystickID);

protected:

	virtual bool TransferDataToWindow() wxOVERRIDE;
	virtual bool TransferDataFromWindow() wxOVERRIDE;

	void RefreshList();

	int FunctionToAssign;

	int m_nJoystickID;	// host joystick id
	int m_nCPCJoystickID; // cpc joystick id

	wxListCtrl *pListCtrl;
	int m_CLLine;// line in listcontrol
	wxTimer *m_Timer;//timer
	void OnTimer(wxTimerEvent &timer);
	int loopnumber;
	void InitMemButton(int i);

	void SetTextinLV(wxString Text);

	void OnButtonConfigSel(wxListEvent& WXUNUSED(event));
	void OnListKeyDown(wxListEvent& event);
	void ClearAssignment(int);
	void OnConfigure(wxCommandEvent &event);
	void OnClear(wxCommandEvent &event);
	void DoConfigure();
	void DoClear();
	void OnContextMenu(wxContextMenuEvent &event);

	virtual void EndModal(int) wxOVERRIDE;


	DECLARE_EVENT_TABLE()
};

#endif

