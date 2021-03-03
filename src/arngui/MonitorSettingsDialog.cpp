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
#include "MonitorSettingsDialog.h"
#include <wx/xrc/xmlres.h>
#include "arnguiApp.h"

extern "C"
{
#include "../cpc/monitor.h"

}


BEGIN_EVENT_TABLE(CMonitorSettingsDialog, wxDialog)
EVT_COMMAND_SCROLL(XRCID("IDC_BRIGHTNESS"), CMonitorSettingsDialog::OnScrollBrightness)
EVT_COMMAND_SCROLL(XRCID("IDC_CONTRAST"), CMonitorSettingsDialog::OnScrollContrast)
EVT_CLOSE(CMonitorSettingsDialog::OnClose)
END_EVENT_TABLE()

void CMonitorSettingsDialog::OnScrollBrightness(wxScrollEvent &event)
{
	int Position = event.GetPosition();
	Monitor_SetBrightness(Position);
	CPC_SetMonitorType((CPC_MONITOR_TYPE_ID)CPC_GetMonitorType());

}

void CMonitorSettingsDialog::OnScrollContrast(wxScrollEvent &event)
{
	int Position = event.GetPosition();
	Monitor_SetContrast(Position);
	CPC_SetMonitorType((CPC_MONITOR_TYPE_ID)CPC_GetMonitorType());

}

CMonitorSettingsDialog *CMonitorSettingsDialog::m_pInstance = NULL;

void CMonitorSettingsDialog::OnClose(wxCloseEvent & WXUNUSED(event))
{
	wxGetApp().WriteConfigWindowPosAndSize(wxT("windows/monitor/settings/"), this);

	this->Destroy();

	CMonitorSettingsDialog::m_pInstance = NULL;
}

bool CMonitorSettingsDialog::TransferDataToWindow()
{
	wxSlider *pScroll;

	pScroll = XRCCTRL(*this, "IDC_BRIGHTNESS", wxSlider);
	pScroll->SetValue(Monitor_GetContrast());

	pScroll = XRCCTRL(*this, "IDC_CONTRAST", wxSlider);
	pScroll->SetValue(Monitor_GetBrightness());


	// default size
	//this->SetSize(430,390);
	wxGetApp().ReadConfigWindowPosAndSize(wxT("windows/MonitorSettings/"), this);
	wxGetApp().EnsureWindowVisible(this);


	return true;
}

bool CMonitorSettingsDialog::TransferDataFromWindow()
{

	return true;

}

CMonitorSettingsDialog::CMonitorSettingsDialog(wxWindow *parent) : wxDialog()
{
	wxXmlResource::Get()->LoadDialog(this, parent, wxT("DLG_DIALOG_MONITOR"));
}
