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
#include "JoystickDialog.h"
#include <wx/xrc/xmlres.h>
#include "arnguiApp.h"
#include <wx/listbox.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/slider.h>
#include <wx/button.h>
#include <wx/radiobut.h>
#include <wx/msgdlg.h>
#include "IntClientData.h"
#include "KeyStickDialog.h"
#include "JoystickButtonDialog.h"


extern "C"
{
#include "../cpc/joystick.h"
}

BEGIN_EVENT_TABLE(JoystickDialog, wxDialog)
EVT_RADIOBUTTON(XRCID("IDC_RADIO_USE_CONTROLLER"), JoystickDialog::OnUseController)
EVT_RADIOBUTTON(XRCID("IDC_RADIO_USE_KEYBOARD"), JoystickDialog::OnUseKeyboard)
EVT_RADIOBUTTON(XRCID("IDC_RADIO_USE_MOUSE"), JoystickDialog::OnUseMouse)
EVT_BUTTON(XRCID("m_DefineJoy"), JoystickDialog::DefineJoy)
EVT_BUTTON(XRCID("m_DefineKey"), JoystickDialog::DefineKey)
//EVT_CHOICE(XRCID("IDC_SIMULATED_CHOICE"), OnChoiceKey)
//EVT_TIMER(2, JoystickDialog::OnTimer)
END_EVENT_TABLE()


JoystickDialog::~JoystickDialog()
{
	wxListBox *pListBox;
	wxChoice *pChoice;

	pListBox = XRCCTRL(*this, "IDC_LIST_JOY", wxListBox);

	for (int i = pListBox->GetCount() - 1; i >= 0; i--)
	{
		pListBox->Delete(i);
	}
	pChoice = XRCCTRL(*this, "IDC_SIMULATED_CHOICE", wxChoice);
	for (int i = pChoice->GetCount() - 1; i >= 0; i--)
	{
		pChoice->Delete(i);
	}

	//delete m_Timer;
}



void JoystickDialog::RefreshRadio()
{
	wxRadioButton *pRadioButton;

	// You can't have a radio with nothing set.
	pRadioButton = XRCCTRL(*this, "IDC_RADIO_USE_KEYBOARD", wxRadioButton);
	pRadioButton->SetValue((m_nJoystickType == JOYSTICK_TYPE_SIMULATED_BY_KEYBOARD));

	pRadioButton = XRCCTRL(*this, "IDC_RADIO_USE_MOUSE", wxRadioButton);
	pRadioButton->SetValue((m_nJoystickType == JOYSTICK_TYPE_SIMULATED_BY_MOUSE));

	pRadioButton = XRCCTRL(*this, "IDC_RADIO_USE_CONTROLLER", wxRadioButton);
	pRadioButton->SetValue((m_nJoystickType == JOYSTICK_TYPE_REAL));

	wxListBox *pListBox;
	wxButton *pButton;
	pListBox = XRCCTRL(*this, "IDC_LIST_JOY", wxListBox);
	pButton = XRCCTRL(*this, "m_DefineJoy", wxButton);
	if (m_nJoystickType == JOYSTICK_TYPE_REAL)
	{
		pListBox->Enable();
		pButton->Enable();
	}
	else
	{
		pListBox->Disable();
		pButton->Disable();
	}

	wxChoice *pChoice;
	pChoice = XRCCTRL(*this, "IDC_SIMULATED_CHOICE", wxChoice);
	pButton = XRCCTRL(*this, "m_DefineKey", wxButton);
	if (m_nJoystickType == JOYSTICK_TYPE_SIMULATED_BY_KEYBOARD)
	{
		pChoice->Enable();
		pButton->Enable();
	}
	else
	{
		pChoice->Disable();
		pButton->Disable();
	}
}


void JoystickDialog::DefineJoy(wxCommandEvent & WXUNUSED(event))
{
	wxListBox *pListBox;
	pListBox = XRCCTRL(*this, "IDC_LIST_JOY", wxListBox);

	int nJoystickID = -1;

	// find selected joystick in listbox
	int SelectedItem = pListBox->GetSelection();
	if (SelectedItem != wxNOT_FOUND)
	{
		const IntClientData *pIntData = (const IntClientData *)pListBox->GetClientObject(SelectedItem);
		nJoystickID = pIntData->GetData();
	}

	if (nJoystickID == -1)
	{
		wxMessageDialog dialog(NULL, wxT("Please select a joystick first."), wxT(""), wxOK);
		dialog.ShowModal();
		return;
	}

	//need to force joystick memorisation and refresh Joystick management
	Joystick_SetPhysical(m_nJoystickID, nJoystickID);
	wxGetApp().m_PlatformSpecific.RefreshJoysticks();

	JoystickButtonDialog dialog(this->GetParent(), nJoystickID, m_nJoystickID);
	dialog.ShowModal();
}

void JoystickDialog::OnUseController(wxCommandEvent & WXUNUSED(event))
{
	m_nJoystickType = JOYSTICK_TYPE_REAL;
	RefreshRadio();
}

void JoystickDialog::OnUseKeyboard(wxCommandEvent & WXUNUSED(event))
{
	m_nJoystickType = JOYSTICK_TYPE_SIMULATED_BY_KEYBOARD;
	RefreshRadio();
}


void JoystickDialog::OnUseMouse(wxCommandEvent & WXUNUSED(event))
{
	m_nJoystickType = JOYSTICK_TYPE_SIMULATED_BY_MOUSE;
	RefreshRadio();
}

void JoystickDialog::DefineKey(wxCommandEvent & WXUNUSED(event))
{
	wxChoice *pChoice;

	pChoice = XRCCTRL(*this, "IDC_SIMULATED_CHOICE", wxChoice);

	//This concern the dialog so it s associed to CPC joystick
	int nChoice = pChoice->GetSelection();

	if (nChoice != wxNOT_FOUND)
	{
		const IntClientData *pIntData = (const IntClientData *)pChoice->GetClientObject(nChoice);
		int nKeySet = pIntData->GetData();

		if (nKeySet == -1) return;

		KeyStickDialog dialog(this->GetParent(), nKeySet);
		dialog.ShowModal();
	}
}

/*
void JoystickDialog::OnChoiceKey(wxCommandEvent & WXUNUSED(event))
{
wxWindow *pWindow;
wxChoice *pChoice;

pChoice = static_cast<wxChoice *>(this->FindWindow(XRCID("IDC_SIMULATED_CHOICE")));

wxButton *pButton = (wxButton *)(this->FindWindow(XRCID("m_DefineKey")));

}
*/


void JoystickDialog::FillListBox()
{
	wxListBox *pListBox;

	pListBox = XRCCTRL(*this, "IDC_LIST_JOY", wxListBox);

	//Make the joy list
	pListBox->Freeze();
	pListBox->Clear();
	wxGetApp().m_PlatformSpecific.PopulateJoystickDialog(pListBox);
	pListBox->Thaw();

	int nID = wxNOT_FOUND;

	// select the joystick regardless of whether it's a physical or not so we can see the selection
	int PhysicalJoystickID = Joystick_GetPhysical(m_nJoystickID);
	for (unsigned int i = 0; i != pListBox->GetCount(); i++)
	{
		IntClientData *pIntData = (IntClientData *)pListBox->GetClientObject(i);
		if (pIntData->GetData() == PhysicalJoystickID)
		{
			nID = i;
			break;
		}
	}
	pListBox->SetSelection(nID);

}

/*
// the timer was meant to refresh the list so you can plug joysticks in and out and see the list update.
void JoystickDialog::OnTimer(wxTimerEvent &timer)
{
FillListBox();
}
*/

bool JoystickDialog::TransferDataToWindow()
{
	wxCheckBox *pCheckBox;
	wxSlider *pSlider;
	wxChoice *pChoice;

	// initial refresh of joysticks
	wxGetApp().m_PlatformSpecific.RefreshJoysticks();

	//m_Timer = new wxTimer(this, 2);
	//m_Timer->Start();
	//printf("Starting timer\r\n");

	pCheckBox = XRCCTRL(*this, "IDC_ENABLEJOYSTICK", wxCheckBox);
	pCheckBox->SetValue(Joystick_IsActive(m_nJoystickID) ? true : false);

	pCheckBox = XRCCTRL(*this, "IDC_ENABLEAUTOFIRE", wxCheckBox);
	pCheckBox->SetValue(JoystickAF_IsActive(m_nJoystickID) ? true : false);

	pSlider = XRCCTRL(*this, "IDC_AUTOFIRE_TRACKBAR", wxSlider);
	pSlider->SetMin(MINAUTOFIRERATE);
	pSlider->SetMax(MAXAUTOFIRERATE);
	pSlider->SetValue(JoystickAF_GetRate(m_nJoystickID));

	m_nJoystickType = Joystick_GetType(m_nJoystickID);

	//Fill joystick list
	FillListBox();

	IntClientData *pCustomIntData = NULL;

	{
		IntClientData *pIntData;
		pChoice = XRCCTRL(*this, "IDC_SIMULATED_CHOICE", wxChoice);
		pIntData = new IntClientData(JOYSTICK_KEYSET_CURSORS);
		pChoice->Append(wxT("Cursors (L,R,U,D), Fire (1: Space, 2: Right Control, 3: Right Shift) "), pIntData);

		pIntData = new IntClientData(JOYSTICK_KEYSET_NUMPAD);
		pChoice->Append(wxT("Number pad (4,6,8,2), Fire (1: 5, 2: 0, 3: . )"), pIntData);

		pIntData = new IntClientData(JOYSTICK_KEYSET_INSERT_HOME);
		pChoice->Append(wxT("Ins/Del etc (Delete,Page Down,Home,End), Fire (1: Insert, 2: Page Up 3: Right Ctrl)"), pIntData);

		pIntData = new IntClientData(JOYSTICK_KEYSET_WASD);
		pChoice->Append(wxT("WASD (A,D,W,S), Fire (1: Space, 2: Right Control 3: Right Shift)"), pIntData);

		pCustomIntData = pIntData = new IntClientData(JOYSTICK_KEYSET_CUSTOM);
		pChoice->Append(wxT("Custom"), pIntData);
	}


	int nSelection = 0;//to have the first selected if not configured

	int nKeySet = Joystick_GetKeySet(m_nJoystickID);
	if (nKeySet != -1)
	{
		for (unsigned int i = 0; i != pChoice->GetCount(); i++)
		{
			const IntClientData *pIntData = (const IntClientData *)pChoice->GetClientObject(i);
			if (pIntData->GetData() == nKeySet)
			{
				nSelection = i;
				break;
			}
		}
	}

	pChoice->SetSelection(nSelection);

	if (pCustomIntData!=NULL)
	{
		//fill Custom remap list
#if ((defined(GTK2_EMBED_WINDOW) && defined(__WXGTK__)) || (defined(MAC_EMBED_WINDOW) && defined(__WXMAC__)))
		int * m_keys = wxGetApp().GetKeySetPtr(pCustomIntData->GetData());
#else
		int * m_keys = wxGetApp().m_PlatformSpecific.GetKeySetPtr(pCustomIntData->GetData());
#endif
		for (int i = 0; i < JOYSTICK_SIMULATED_KEYID_LAST; i++)
		{
			m_keys[i] = Joystick_GetSimulatedKeyID(m_nJoystickID, i);
		}
	}
	RefreshRadio();

	return true;
}

bool JoystickDialog::TransferDataFromWindow()
{
	wxCheckBox *pCheckBox;
	wxListBox *pListBox;
	wxChoice *pChoice;
	wxSlider *pSlider;

	//m_Timer->Stop();

	// joystick enable
	pCheckBox = XRCCTRL(*this, "IDC_ENABLEJOYSTICK", wxCheckBox);

	Joystick_Activate(m_nJoystickID, pCheckBox->GetValue());
	Joystick_SetType(m_nJoystickID, m_nJoystickType);

	// autofire enable
	pCheckBox = XRCCTRL(*this, "IDC_ENABLEAUTOFIRE", wxCheckBox);
	JoystickAF_Activate(m_nJoystickID, pCheckBox->GetValue());

	// autofire rate
	pSlider = XRCCTRL(*this, "IDC_AUTOFIRE_TRACKBAR", wxSlider);

	JoystickAF_SetRate(m_nJoystickID, pSlider->GetValue());

	// set selected joystick even if we don't use this mode so it can be restored next time dialog is shown
	pListBox = XRCCTRL(*this, "IDC_LIST_JOY", wxListBox);

	int nJoystickID = -1;

	// find selected joystick
	int SelectedItem = pListBox->GetSelection();
	if (SelectedItem != wxNOT_FOUND)
	{
		const IntClientData *pIntData = (const IntClientData *)pListBox->GetClientObject(SelectedItem);
		nJoystickID = pIntData->GetData();
	}
	Joystick_SetPhysical(m_nJoystickID, nJoystickID);

	// store selected key set even if we do not use this mode. So it can be restored next time dialog is shown
	pChoice = XRCCTRL(*this, "IDC_SIMULATED_CHOICE", wxChoice);

	int nChoice = pChoice->GetSelection();

	int nKeySet;
	if (nChoice != wxNOT_FOUND)
	{
		const IntClientData *pIntData = (const IntClientData *)pChoice->GetClientObject(nChoice);
		nKeySet = pIntData->GetData();

		Joystick_SetKeySet(m_nJoystickID, nKeySet);

		//Save custom remap list
#if ((defined(GTK2_EMBED_WINDOW) && defined(__WXGTK__)) || (defined(MAC_EMBED_WINDOW) && defined(__WXMAC__)))
		int * m_keys = wxGetApp().GetKeySetPtr(nKeySet);
#else
		int * m_keys = wxGetApp().m_PlatformSpecific.GetKeySetPtr(nKeySet);
#endif
		for (int i = 0; i < JOYSTICK_SIMULATED_KEYID_LAST; i++)
		{
			Joystick_SetSimulatedKeyID(m_nJoystickID, i, m_keys[i]);
		}

	}

	wxGetApp().WriteConfigWindowPosAndSize(wxT("windows/joystick/"), this);

	return true;

	}


JoystickDialog::JoystickDialog(wxWindow *pParent, CPC_JOYSTICK_ID nJoystickID) : m_nJoystickID(nJoystickID), m_nJoystickType(JOYSTICK_TYPE_REAL)
{
	// load the resource
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_DIALOG_JOYSTICK"));

	//this->SetSize(330,300);
	wxGetApp().ReadConfigWindowPosAndSize(wxT("windows/joystick/"), this);
	wxGetApp().EnsureWindowVisible(this);

	wxString sJoystickName;
	switch (m_nJoystickID)
	{
	case CPC_NUM_JOYSTICKS:
	case CPC_DIGITAL_JOYSTICK0:
	{
		sJoystickName = wxT("Digital Joystick 0");
	}
	break;
	case CPC_DIGITAL_JOYSTICK1:
	{
		sJoystickName = wxT("Digital Joystick 1");
	}
	break;
	case CPC_ANALOGUE_JOYSTICK0:
	{
		sJoystickName = wxT("Analogue Joystick 0");
	}
	break;
	case CPC_ANALOGUE_JOYSTICK1:
	{
		sJoystickName = wxT("Analogue Joystick 1");
	}
	break;
	case MULTIPLAY_JOYSTICK0:
	{
		sJoystickName = wxT("Multiplay Joystick 0");
	}
	break;
	case MULTIPLAY_JOYSTICK1:
	{
		sJoystickName = wxT("Multiplay Joystick 1");
	}
	break;
	}
	wxString sTitle = sJoystickName + wxT(" - Configuration");
	SetTitle(sTitle);

}

