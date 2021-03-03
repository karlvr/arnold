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
#ifndef __AUTOTYPE_DIALOG_HEADER_INCLUDED__
#define __AUTOTYPE_DIALOG_HEADER_INCLUDED__

#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/dnd.h>

#ifndef wxOVERRIDE
#define wxOVERRIDE
#endif


class AutoTypeDropTarget : public wxDropTarget
{
public:
	AutoTypeDropTarget();
	wxTextCtrl *m_pTextCtrl;

	virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def) wxOVERRIDE;
	virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult defResult) wxOVERRIDE;
};

class CAutoTypeDialog : public wxDialog
{
private:
	CAutoTypeDialog(wxWindow* parent);
	~CAutoTypeDialog();

	static CAutoTypeDialog *m_pInstance;
public:
	// creator
	static CAutoTypeDialog *CreateInstance(wxWindow *pParent)
	{
		if (m_pInstance == NULL)
		{
			m_pInstance = new CAutoTypeDialog(pParent);
			if (m_pInstance != NULL)
			{
				m_pInstance->Show();
			}

		}
		else
		{
			m_pInstance->Raise();
		}
		return m_pInstance;
	}
	static CAutoTypeDialog *GetInstance()
	{
		return m_pInstance;
	}
	void SetText(const wxString &sText);
	virtual bool TransferDataToWindow() wxOVERRIDE;
private:
	void OnStop(wxCommandEvent & WXUNUSED(event));
	void OnClose(wxCloseEvent &event);
	void OnOK(wxCommandEvent &event);
	void OnCancel(wxCommandEvent &event);

	AutoTypeDropTarget *m_DropTarget;

	wxString GetAutoTypeRememberFilename();
	wxTextCtrl *GetTextCtrl();

	// This class handles events
	DECLARE_EVENT_TABLE()
};

#endif
