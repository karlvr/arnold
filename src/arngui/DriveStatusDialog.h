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
#ifndef __DRIVE_STATUS_DIALOG_HEADER_INCLUDED__
#define __DRIVE_STATUS_DIALOG_HEADER_INCLUDED__

#include <wx/dialog.h>
#include "UpdatableDialog.h"
#include <wx/listctrl.h>

#ifndef wxOVERRIDE
#define wxOVERRIDE
#endif

#include "SortableListCtrl.h"

class DriveStatusListCtrl : public SortableListCtrl
{
public:
	DriveStatusListCtrl();
	~DriveStatusListCtrl();

	// provide this in your base class to perform the sorting
	virtual int PerformSort(wxIntPtr item1, wxIntPtr item2) wxOVERRIDE;

	DECLARE_EVENT_TABLE()
	DECLARE_DYNAMIC_CLASS(DriveStatusListCtrl)
};


struct DriveStatus
{
	unsigned int nTrack;
	int Flags;
};
class DriveStatusDialog : public wxDialog, public UpdatableDialog
{
private:
	DriveStatusDialog(wxWindow *pParent);
	~DriveStatusDialog();
	DriveStatus m_DriveStatus[4];
	static DriveStatusDialog *m_pInstance;
public:
	// creator
	static DriveStatusDialog *CreateInstance(wxWindow *pParent)
	{
		if (m_pInstance == NULL)
		{
			m_pInstance = new DriveStatusDialog(pParent);
			if (m_pInstance != NULL)
			{
				m_pInstance->SetTitle(wxT("Drive Status"));
				m_pInstance->Show();
			}

		}
		else
		{
			m_pInstance->Raise();
		}
		return m_pInstance;
	}

	virtual bool TransferDataToWindow() wxOVERRIDE;
	virtual void TimedUpdate() wxOVERRIDE;

private:
	virtual void UpdateI(bool bUpdate);
	void OnClose(wxCloseEvent &event);
	wxListCtrl *GetListCtrl();
	void OnCharHook(wxKeyEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif
