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
#ifndef __MEDIA_STATUS_HEADER_INCLUDED__
#define __MEDIA_STATUS_HEADER_INCLUDED__
#include <wx/dialog.h>
#include <wx/listctrl.h>

#ifndef wxOVERRIDE
#define wxOVERRIDE
#endif

#include "UpdatableDialog.h"
#include "Media.h"
#include "SortableListCtrl.h"

class MediaStatusListCtrl : public SortableListCtrl
{
public:
	MediaStatusListCtrl();
	~MediaStatusListCtrl();

	// provide this in your base class to perform the sorting
	virtual int PerformSort(wxIntPtr WXUNUSED(item1), wxIntPtr WXUNUSED(item2)) wxOVERRIDE
	{
		// by default no sort
		return 0;
	}

		DECLARE_EVENT_TABLE()
	DECLARE_DYNAMIC_CLASS(MediaStatusListCtrl)
};


class MediaStatusDialog : public wxDialog, public UpdatableDialog
{
private:
	MediaStatusDialog(wxWindow *pParent);
	~MediaStatusDialog();

	void AddMedia(Media *pMedia);

	static MediaStatusDialog *m_pInstance;
public:
	// creator
	static MediaStatusDialog *CreateInstance(wxWindow *pParent)
	{
		if (m_pInstance == NULL)
		{
			m_pInstance = new MediaStatusDialog(pParent);
			if (m_pInstance != NULL)
			{
				m_pInstance->SetTitle(wxT("Media Status"));
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

private:
	virtual void TimedUpdate() wxOVERRIDE;
	void UpdateMedia(int nIndex);
	void OnClose(wxCloseEvent &event);
	wxListCtrl *GetListCtrl();
	void OnUnload(wxCommandEvent &event);
	void OnReload(wxCommandEvent &event);
	void OnForceUnload(wxCommandEvent &event);
	void OnSaveAs(wxCommandEvent &event);
	void OnForceSave(wxCommandEvent &event);
	void OnContextMenu(wxContextMenuEvent &event);
	void OnCharHook(wxKeyEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif
