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

#ifndef __SNAPSHOT_LOAD_SETTINGS_DIALOG_HEADER_INCLUDED__
#define __SNAPSHOT_LOAD_SETTINGS_DIALOG_HEADER_INCLUDED__

#include <wx/dialog.h>

extern "C"
{
#include "../cpc/snapshot.h"
}

#ifndef wxOVERRIDE
#define wxOVERRIDE
#endif

class SnapshotLoadSettingsDialog : public wxDialog
{
public:
	SnapshotLoadSettingsDialog(wxWindow *pParent, const SNAPSHOT_REQUIREMENTS *pRequirements, SNAPSHOT_MEMORY_BLOCKS *pMemoryBlocks);

	virtual bool TransferDataFromWindow() wxOVERRIDE;
	virtual bool TransferDataToWindow() wxOVERRIDE;

	void OnConfigure(wxCommandEvent &event);

	const SNAPSHOT_REQUIREMENTS *m_pRequirements;
	SNAPSHOT_MEMORY_BLOCKS *m_pMemoryBlocks;

	void RefreshDisplay();

	DECLARE_EVENT_TABLE()
};

#endif
