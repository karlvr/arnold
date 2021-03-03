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

#ifndef __EXPANSION_ROM_DIALOG_HEADER_INCLUDED__
#define __EXPANSION_ROM_DIALOG_HEADER_INCLUDED__

#include <wx/dialog.h>
#include <wx/listctrl.h>

#ifndef wxOVERRIDE
#define wxOVERRIDE
#endif

#include "EmuFileType.h"
#include "SortableListCtrl.h"

extern "C"
{
#include "../cpc/emudevice.h"
}


class ExpansionRomsListCtrl : public SortableListCtrl
{
public:
	ExpansionRomsListCtrl();
	~ExpansionRomsListCtrl();

	// provide this in your base class to perform the sorting
	virtual int PerformSort(wxIntPtr WXUNUSED(item1), wxIntPtr WXUNUSED(item2))  wxOVERRIDE{ return 0; }

		DECLARE_EVENT_TABLE()
	DECLARE_DYNAMIC_CLASS(ExpansionRomsListCtrl)
};


class ExpansionRomsDialog : public wxDialog
{
public:
	ExpansionRomsDialog(wxWindow *pParent, int nDevice);
	~ExpansionRomsDialog();
	bool TransferDataToWindow() wxOVERRIDE;
protected:
	int m_nDevice;
	ExpansionRomData *m_pData;
	wxListCtrl *GetListCtrl();
	void PerformUpdate();
	void OnContextMenu(wxContextMenuEvent &event);
	bool WarnROMLoad(long item);
	void VerifyRomData(const unsigned char *pRomData, unsigned long RomLength, unsigned long nSize);

	void OnROMEnable(wxCommandEvent &event);
	void OnROMRemove(wxCommandEvent &event);
	void OnROMLoad(wxCommandEvent &event);
	void OnROMPickBuiltin(wxCommandEvent &event);
	void OnCharHook(wxKeyEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif
