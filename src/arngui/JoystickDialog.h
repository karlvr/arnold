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
#ifndef __JOYSTICK_DIALOG_HEADER_INCLUDED__
#define __JOYSTICK_DIALOG_HEADER_INCLUDED__

#include <wx/dialog.h>
#include <wx/timer.h>
#include <wx/joystick.h>
#include <wx/textctrl.h>

#ifndef wxOVERRIDE
#define wxOVERRIDE
#endif

extern "C"
{
#include "../cpc/cpc.h"
}

class JoystickDialog : public  wxDialog
{
public:
	JoystickDialog(wxWindow *pParent, CPC_JOYSTICK_ID nJoystickID);
	~JoystickDialog();

	virtual bool TransferDataFromWindow() wxOVERRIDE;
	virtual bool TransferDataToWindow() wxOVERRIDE;

private:
	void DefineJoy(wxCommandEvent &event);
	void DefineKey(wxCommandEvent &event);
	void OnUseController(wxCommandEvent &event);
	void OnUseKeyboard(wxCommandEvent &event);
	void OnUseMouse(wxCommandEvent &event);
	//void OnTimer(wxTimerEvent &event);
	//void OnChoiceKey(wxCommandEvent & WXUNUSED(event));

	void RefreshRadio();
	//wxTimer *m_Timer;
	CPC_JOYSTICK_ID m_nJoystickID;
	int m_nJoystickType;
	void FillListBox();

	DECLARE_EVENT_TABLE()
};


//***************************************************************************************

class RedefineKey : public  wxDialog
{
public:
	RedefineKey(wxWindow *pParent);
	~RedefineKey();
private:

	DECLARE_EVENT_TABLE()
};
/*
class RedefineJoy : public  wxDialog
{
public:
RedefineJoy(wxWindow *pParent, int nJoystickID);
~RedefineJoy();

wxJoystick* m_stick;
wxTextCtrl *pEditBox;
//int FunctionToAssign;

private:
virtual void Button1(wxCommandEvent &event);
virtual void Button2(wxCommandEvent &event);
void OnTimer(wxTimerEvent &event);

void InitMemButton(int i);
void SetInfo(wxString ptext);
int m_nJoystickID;
wxTimer *m_Timer;
int loopnumber;

DECLARE_EVENT_TABLE()
};


*/
#endif
