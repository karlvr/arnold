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
#ifndef __CSD_DIALOG_HEADER_INCLUDED__
#define __CSD_DIALOG_HEADER_INCLUDED__

#include <wx/dialog.h>
#include <wx/listctrl.h>

#ifndef wxOVERRIDE
#define wxOVERRIDE
#endif

#include "EmuFileType.h"

class CSDDialog : public wxDialog
{
public:
	CSDDialog(wxWindow *pParent);
	virtual bool TransferDataToWindow() wxOVERRIDE;
	virtual bool TransferDataFromWindow() wxOVERRIDE;
protected:
	void InsertItem(const wxString &sID, int nIndex);

	EmuFileType m_CartridgeFileType;
	wxListCtrl *GetListCtrl();
	void PerformUpdate();

	void OnContextMenu(wxContextMenuEvent &event);

	void OnCartridgeEnable(wxCommandEvent &event);
	void OnCartridgeLoad(wxCommandEvent &event);
	void OnCartridgeRemove(wxCommandEvent &event);
	void OnCartridgePickBuiltin(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

#endif
