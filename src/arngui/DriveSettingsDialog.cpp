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
#include "DriveSettingsDialog.h"
#include <wx/xrc/xmlres.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include "IntClientData.h"
#include <wx/config.h>

extern "C"
{
#include "../cpc/cpc.h"
#include "../cpc/fdd.h"

}

BEGIN_EVENT_TABLE(DriveSettingsDialog, wxDialog)
END_EVENT_TABLE()

DriveSettingsDialog::~DriveSettingsDialog()
{
	wxChoice *pChoice;

	pChoice = XRCCTRL(*this, "TracksChoice", wxChoice);

	for (int i = pChoice->GetCount() - 1; i >= 0; i--)
	{
		//  const IntClientData *pIntData = (const IntClientData *)pChoice->GetClientObject(i);
		//     delete pIntData;
		pChoice->Delete(i);
	}
}


bool DriveSettingsDialog::TransferDataToWindow()
{
	wxCheckBox *pCheckBox;
	wxChoice *pChoice;

	pCheckBox = XRCCTRL(*this, "ForceReady", wxCheckBox);
	pCheckBox->SetValue(FDD_IsAlwaysReady(m_nDriveId) ? true : false);

	pCheckBox = XRCCTRL(*this, "ForceWriteProtect", wxCheckBox);
	pCheckBox->SetValue(FDD_IsAlwaysWriteProtected(m_nDriveId) ? true : false);

	pChoice = XRCCTRL(*this, "SidesChoice", wxChoice);
	int nChoice = 0;
	if (FDD_GetDoubleSided(m_nDriveId))
	{
		nChoice = 1;
	}
	pChoice->SetSelection(nChoice);

	pChoice = XRCCTRL(*this, "TracksChoice", wxChoice);
	for (int i = 40; i < MAX_TRACKS_40_TRACK + 1; i++)
	{
		wxString sText;
		IntClientData *pIntData = new IntClientData(i);
		sText.Printf(wxT("%d"), i);
		pChoice->Append(sText, pIntData);
	}

	for (int i = 80; i < MAX_TRACKS_80_TRACK + 1; i++)
	{
		wxString sText;
		IntClientData *pIntData = new IntClientData(i);
		sText.Printf(wxT("%d"), i);
		pChoice->Append(sText, pIntData);
	}

	pChoice->SetSelection(0);
	int nMaxTracks = FDD_GetTracks(m_nDriveId);
	for (unsigned int i = 0; i != pChoice->GetCount(); i++)
	{
		const IntClientData *pIntData = (const IntClientData *)pChoice->GetClientObject(i);
		if (pIntData->GetData() == nMaxTracks)
		{
			pChoice->SetSelection(i);
		}
	}

	return true;
}

bool DriveSettingsDialog::TransferDataFromWindow()
{
	wxCheckBox *pCheckBox;
	wxChoice *pChoice;

	pCheckBox = XRCCTRL(*this, "ForceReady", wxCheckBox);
	FDD_SetAlwaysReady(m_nDriveId, pCheckBox->GetValue());

	wxString sConfig;
	sConfig.Printf(wxT("drive/%c/force_ready"), m_nDriveId + 'a');
	wxConfig::Get(false)->Write(sConfig, FDD_IsAlwaysReady(m_nDriveId) ? true : false);

	pCheckBox = XRCCTRL(*this, "ForceWriteProtect", wxCheckBox);
	FDD_SetAlwaysWriteProtected(m_nDriveId, pCheckBox->GetValue());

	sConfig.Printf(wxT("drive/%c/force_write_protect"), m_nDriveId + 'a');
	wxConfig::Get(false)->Write(sConfig, FDD_IsAlwaysWriteProtected(m_nDriveId) ? true : false);

	pChoice = XRCCTRL(*this, "SidesChoice", wxChoice);
	int nChoice = pChoice->GetSelection();
	if (nChoice != wxNOT_FOUND)
	{
		bool bDoubleSided = (nChoice == 1);
		FDD_SetDoubleSided(m_nDriveId, bDoubleSided);
	}
	else
	{
		FDD_SetDoubleSided(m_nDriveId, FALSE);
	}

	sConfig.Printf(wxT("drive/%c/double_sided"), m_nDriveId + 'a');
	wxConfig::Get(false)->Write(sConfig, FDD_GetDoubleSided(m_nDriveId) ? true : false);


	pChoice = XRCCTRL(*this, "TracksChoice", wxChoice);

	nChoice = pChoice->GetSelection();

	if (nChoice != wxNOT_FOUND)
	{
		const IntClientData *pIntData = (const IntClientData *)pChoice->GetClientObject(nChoice);
		int nTracks = pIntData->GetData();
		FDD_SetTracks(m_nDriveId, nTracks);
	}
	else
	{
		FDD_SetTracks(m_nDriveId, 40);
	}
	sConfig.Printf(wxT("drive/%c/max_tracks"), m_nDriveId + 'a');
	wxConfig::Get(false)->Write(sConfig, FDD_GetTracks(m_nDriveId));


	return true;

}


DriveSettingsDialog::DriveSettingsDialog(wxWindow *pParent, int nDriveId) : m_nDriveId(nDriveId)
{
	// load the resource
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DriveSettingsDialog"));
	wxString sTitle;
	sTitle.Printf(wxT("Drive %c Settings"), (char)(nDriveId + 'A'));
	SetTitle(sTitle);
}
