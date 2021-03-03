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

#include "AudioSettingsDialog.h"
#include <wx/xrc/xmlres.h>
#include "arnguiApp.h"
#include <wx/listbox.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/radiobut.h>
#include "IntClientData.h"

BEGIN_EVENT_TABLE(AudioSettingsDialog, wxDialog)
END_EVENT_TABLE()


AudioSettingsDialog::~AudioSettingsDialog()
{
}

bool AudioSettingsDialog::TransferDataToWindow()
{
	wxChoice *pChoice;

	pChoice = XRCCTRL(*this, "m_ChoiceFrequency", wxChoice);

	// setup from existing settings
	wxString sFrequency;
	sFrequency.sprintf(wxT("%d"), wxGetApp().GetAudioFrequency());


	bool   bFound = false;

	for (unsigned int i = 0; i != pChoice->GetCount(); i++)
	{
		wxString sChoiceString = pChoice->GetString(i);
		if (sChoiceString.CmpNoCase(sFrequency) == 0)
		{
			bFound = true;
			pChoice->SetSelection(i);
			break;
		}
	}
	if (!bFound)
	{
		pChoice->SetSelection(0);
	}




	pChoice = XRCCTRL(*this, "m_ChoiceBits", wxChoice);
	

	// setup from existing settings
	wxString sBits;
	sBits.sprintf(wxT("%d"), wxGetApp().GetAudioBits());


	bFound = false;

	for (unsigned int i = 0; i != pChoice->GetCount(); i++)
	{
		wxString sChoiceString = pChoice->GetString(i);
		if (sChoiceString.CmpNoCase(sBits) == 0)
		{
			bFound = true;
			pChoice->SetSelection(i);
			break;
		}
	}
	if (!bFound)
	{
		pChoice->SetSelection(0);
	}



	pChoice = XRCCTRL(*this, "m_ChoiceChannels", wxChoice);
	

	// setup from existing settings
	wxString sChannels;
	sChannels.sprintf(wxT("%d"), wxGetApp().GetAudioChannels());


	bFound = false;

	for (unsigned int i = 0; i != pChoice->GetCount(); i++)
	{
		wxString sChoiceString = pChoice->GetString(i);
		if (sChoiceString.CmpNoCase(sChannels) == 0)
		{
			bFound = true;
			pChoice->SetSelection(i);
			break;
		}
	}
	if (!bFound)
	{
		pChoice->SetSelection(0);
	}

	return true;
}

bool AudioSettingsDialog::TransferDataFromWindow()
{
	wxChoice *pChoice;

	pChoice = XRCCTRL(*this, "m_ChoiceFrequency", wxChoice);
	
	wxString sFrequency = pChoice->GetString(pChoice->GetSelection());
	long nFrequency;
	if (sFrequency.ToLong(&nFrequency))
	{
		wxGetApp().SetAudioFrequency(nFrequency);
		wxConfig::Get(false)->Write(wxT("audio/frequency"), nFrequency);
	}

	pChoice = XRCCTRL(*this, "m_ChoiceBits", wxChoice);
	
	wxString sBits = pChoice->GetString(pChoice->GetSelection());
	long nBits;
	if (sBits.ToLong(&nBits))
	{
		wxGetApp().SetAudioBits(nBits);
		wxConfig::Get(false)->Write(wxT("audio/bits"), nBits);
	}

	pChoice = XRCCTRL(*this, "m_ChoiceChannels", wxChoice);
	
	wxString sChannels = pChoice->GetString(pChoice->GetSelection());
	long nChannels;
	if (sChannels.ToLong(&nChannels))
	{
		wxGetApp().SetAudioChannels(nChannels);
		wxConfig::Get(false)->Write(wxT("audio/channels"), nChannels);
	}



	wxGetApp().WriteConfigWindowPosAndSize(wxT("windows/audio/"), this);

	return true;

}


AudioSettingsDialog::AudioSettingsDialog(wxWindow *pParent)
{
	// load the resource
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_DIALOG_SOUND_SETTINGS"));
	//	this->SetSize(330,300);
	wxGetApp().ReadConfigWindowPosAndSize(wxT("windows/audio/"), this);
	wxGetApp().EnsureWindowVisible(this);

}

