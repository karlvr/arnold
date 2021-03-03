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
#ifndef __YM_DIALOGS_HEADER_INCLUDED__
#define __YM_DIALOGS_HEADER_INCLUDED__

#include <wx/dialog.h>

#ifndef wxOVERRIDE
#define wxOVERRIDE
#endif

#include "EmuFileType.h"

class YmPreSaveDialog : public wxDialog
{
public:
	YmPreSaveDialog(wxWindow *pParent);
	~YmPreSaveDialog();
	virtual bool TransferDataToWindow() wxOVERRIDE;
	virtual bool TransferDataFromWindow() wxOVERRIDE;

};


class YmRecordingDialog : public wxDialog
{
private:
	static YmRecordingDialog *m_pInstance;
	YmRecordingDialog(wxWindow *pParent);

	EmuFileType YMFileType;

public:


	static YmRecordingDialog *CreateInstance(wxWindow *pParent);

	virtual bool TransferDataToWindow() wxOVERRIDE;
	virtual bool TransferDataFromWindow() wxOVERRIDE;
protected:
	void OnStopRecording(wxCommandEvent &event);
	void OnStartRecording(wxCommandEvent &event);
	void OnSaveRecording(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	void OnCharHook(wxKeyEvent& event);

	DECLARE_EVENT_TABLE()

};

#endif