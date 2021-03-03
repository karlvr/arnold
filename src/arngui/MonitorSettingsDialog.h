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
#ifndef __MONITOR_SETTINGS_DIALOG_HEADER_INCLUDED__
#define __MONITOR_SETTINGS_DIALOG_HEADER_INCLUDED__

#include <wx/dialog.h>
#include <wx/slider.h>

#ifndef wxOVERRIDE
#define wxOVERRIDE
#endif

class CMonitorSettingsDialog : public wxDialog
{
private:
	CMonitorSettingsDialog(wxWindow* parent);
	static CMonitorSettingsDialog *m_pInstance;
public:
	void OnClose(wxCloseEvent & event);
	void OnScrollBrightness(wxScrollEvent &event);
	void OnScrollContrast(wxScrollEvent &event);

	// creator
	static CMonitorSettingsDialog *CreateInstance(wxWindow *pParent)
	{
		if (m_pInstance == NULL)
		{
			m_pInstance = new CMonitorSettingsDialog(pParent);
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
	static CMonitorSettingsDialog *GetInstance()
	{
		return m_pInstance;
	}
	virtual bool TransferDataFromWindow() wxOVERRIDE;
	virtual bool TransferDataToWindow() wxOVERRIDE;
private:

	// This class handles events
	DECLARE_EVENT_TABLE()
};

#endif
