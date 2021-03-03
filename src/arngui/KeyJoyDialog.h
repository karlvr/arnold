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
#ifndef __KEY_JOY_DIALOG_HEADER
#define __KEY_JOY_DIALOG_HEADER

#include <wx/dialog.h>
#include <wx/listctrl.h>

#ifndef wxOVERRIDE
#define wxOVERRIDE
#endif

#include "EmuFileType.h"

class KeyJoyItem
{
public:
	enum
	{
		CODE_AXIS = 0,
		CODE_HAT,
		CODE_BUTTON,
		CODE_MAX
	};

	KeyJoyItem(int DeviceIndex, int Code, int Index, int SubValue);
	void ClearMapping();
	void SetMapping(int);

	wxString GetLabel();
	wxString GetJoyKeyMapping();
	bool MappedToJoystick();
private:
	int m_DeviceIndex;

	int m_Code; //axis, hat, button
	int m_Index; // index of the axis, hat, button
	int m_SubValue;

};



class KeyJoyDialog : public wxDialog
{
public:
	~KeyJoyDialog();
	KeyJoyDialog(wxWindow *pParent);
	virtual bool TransferDataToWindow() wxOVERRIDE;
	virtual bool TransferDataFromWindow() wxOVERRIDE;

protected:
	int m_nJoystickID;	// host joystick id
	wxListCtrl *pListCtrl;

	void FillListBox();
	void RefreshList(void);
	int GetChoice(long WinID);
	void RefreshRadio(void);
	void RefreshItem(long line);

	void SelJoy(wxCommandEvent& WXUNUSED(event));
	void Redefine(wxCommandEvent& WXUNUSED(event));
	void Preset(wxCommandEvent& WXUNUSED(event));
	void Clear(wxCommandEvent& WXUNUSED(event));
	void OnRadio(wxCommandEvent & WXUNUSED(event));


	DECLARE_EVENT_TABLE()
};


#endif //__KEY_JOY_DIALOG_HEADER
