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
#include "JoystickButtonDialog.h"
#include <wx/xrc/xmlres.h>
#include "arnguiApp.h"
#include <wx/menu.h>

extern "C"
{
#include "../cpc/joystick.h"
}


const wxChar *sCPCJoystickButtonName[] = {
	wxT("X Axis"),
	wxT("Y Axis"),
	wxT("Fire 1"),
	wxT("Fire 2"),
	wxT("Fire 3")
};

BEGIN_EVENT_TABLE(JoystickButtonDialog, wxDialog)
EVT_CONTEXT_MENU(JoystickButtonDialog::OnContextMenu)
EVT_LIST_KEY_DOWN(XRCID("m_listCtrl7"), JoystickButtonDialog::OnListKeyDown)
EVT_LIST_ITEM_ACTIVATED(XRCID("m_listCtrl7"), JoystickButtonDialog::OnButtonConfigSel)
EVT_COMMAND(XRCID("m_menuItemConfigure"), wxEVT_COMMAND_MENU_SELECTED, JoystickButtonDialog::OnConfigure)
EVT_COMMAND(XRCID("m_menuItemClear"), wxEVT_COMMAND_MENU_SELECTED, JoystickButtonDialog::OnClear)
EVT_TIMER(2, JoystickButtonDialog::OnTimer)
END_EVENT_TABLE()

void JoystickButtonDialog::EndModal(int retCode)
{
	if (m_Timer) {
		if (m_Timer->IsRunning()) m_Timer->Stop();
		delete m_Timer;
		m_Timer = NULL;
	}
	wxDialog::EndModal(retCode);
}

void JoystickButtonDialog::OnConfigure(wxCommandEvent & WXUNUSED(event))
{
	DoConfigure();
}


void JoystickButtonDialog::OnClear(wxCommandEvent & WXUNUSED(event))
{
	DoClear();
}


void JoystickButtonDialog::OnContextMenu(wxContextMenuEvent &event)
{
	if (pListCtrl->GetSelectedItemCount() == 0)
	{
		return;
	}
	wxPoint Position = event.GetPosition();

	// generate a suitable place for the menu
	if (event.GetPosition() == wxDefaultPosition)
	{
		wxPoint ClientPos(0, 0);
		Position = this->ClientToScreen(ClientPos);
	}

	wxMenuBar *pMenuBar = wxXmlResource::Get()->LoadMenuBar(wxT("MB_MENU_CONFIGURE_MENU_BAR"));
	if (pMenuBar)
	{
		wxMenu *pMenu = pMenuBar->Remove(0);
		if (pMenu)
		{
			PopupMenu(pMenu);
		}
		delete pMenu;
	}
	delete pMenuBar;
}

bool JoystickButtonDialog::TransferDataFromWindow()
{
	return true;
}

void JoystickButtonDialog::ClearAssignment(int nIndex)
{
	if (nIndex < CPC_JOYSTICK_NUM_AXES)
	{
		Joystick_SetAxisMappingPhysical(m_nCPCJoystickID, nIndex, -1);
	}
	else
	{
		Joystick_SetButtonMapping(m_nCPCJoystickID, nIndex - CPC_JOYSTICK_NUM_AXES, -1);
	}
	RefreshList();
}

void JoystickButtonDialog::DoClear()
{

	//Get button line config
	long itemIndex = pListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (itemIndex == -1)
	{
		return;
	}
	ClearAssignment(itemIndex);
}

void JoystickButtonDialog::OnListKeyDown(wxListEvent& event)
{
	if (event.m_code == WXK_DELETE)
	{
		DoClear();
	}
	event.Skip(true);
}

void JoystickButtonDialog::RefreshList()
{
	m_Timer->Stop();//just to be sure

	pListCtrl->DeleteAllItems();

	wxString sName;
	wxString sMapping;

	for (size_t i = 0; i < sizeof(sCPCJoystickButtonName) / sizeof(sCPCJoystickButtonName[0]); i++)
	{
		wxListItem Item;

		Item.SetMask(wxLIST_MASK_TEXT);
		Item.SetId(i);

		sName = sCPCJoystickButtonName[i];

		// id
		Item.SetText(sName);
		Item.SetColumn(0);
		Item.SetImage(-1); //Insert a blank icon, not find to insert nothing
		int Index = pListCtrl->InsertItem(Item);

		// which joystick button it's mapped to
		sMapping = wxT("Not mapped");

		if (i < CPC_JOYSTICK_NUM_AXES)
		{
			//AXIS
			int nMapping = Joystick_GetAxisMappingPhysical(m_nCPCJoystickID, i);
			if (nMapping != -1)
			{
				sMapping = wxT("Axis (") + wxString::Format(wxT("%i"), nMapping) + wxT(")");
				if (nMapping >= 9)//if it s POV
				{
					sMapping = wxT("POV (") + wxString::Format(wxT("%i"), nMapping - 10) + wxT(")");
				}
			}
		}
		else
		{
			//BUTTON

			//search at wich one system button this cpc button is mapped
			int nMapping = Joystick_GetButtonMappingCPC(m_nCPCJoystickID, i - CPC_JOYSTICK_NUM_AXES);

			if (nMapping != -1)
			{
				sMapping = wxGetApp().m_PlatformSpecific.GetJoystickButtonName(m_nJoystickID, nMapping);
			}
		}
		Item.SetId(Index);
		Item.SetText(sMapping);
		Item.SetColumn(1);
		pListCtrl->SetItem(Item);
	}

	//reset line selector
	m_CLLine = -1;
}

bool JoystickButtonDialog::TransferDataToWindow()
{

	pListCtrl->Freeze();
	pListCtrl->ClearAll();

	wxListItem Column;
	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT("Key"));
	pListCtrl->InsertColumn(0, Column);
	Column.SetText(wxT("Name"));
	pListCtrl->InsertColumn(1, Column);
	pListCtrl->Thaw();

	RefreshList();
	wxGetApp().SetColumnSize(pListCtrl, 0);
	wxGetApp().SetColumnSize(pListCtrl, 1);

	return true;
}

//destructor
JoystickButtonDialog::~JoystickButtonDialog()
{
	if (m_Timer) {
		if (m_Timer->IsRunning()) m_Timer->Stop();
		delete m_Timer;
		m_Timer = NULL;
	}
}

//constructor
JoystickButtonDialog::JoystickButtonDialog(wxWindow *pParent, int JoystickID, int nCPCJoystickID)
{
	FunctionToAssign = 0;
	m_CLLine = 0;
	loopnumber = 0;

	m_nJoystickID = JoystickID;
	m_nCPCJoystickID = nCPCJoystickID;

	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_DIALOG_JOYSTICK_BUTTONS"));


	wxString sJoypadName = wxGetApp().m_PlatformSpecific.GetJoystickIdString(JoystickID);

	wxString sJoystickName;
	switch (nCPCJoystickID)
	{
	case 0:
	{
		sJoystickName = wxT("Digital Joystick 0");
	}
	break;
	case 1:
	{
		sJoystickName = wxT("Digital Joystick 1");
	}
	break;
	case 2:
	{
		sJoystickName = wxT("Analogue Joystick 0");
	}
	break;
	case 3:
	{
		sJoystickName = wxT("Analogue Joystick 1");
	}
	break;
	}
	wxString sTitle = wxT("Configure '") + sJoypadName + wxT("' mapping to ") + sJoystickName;
	SetTitle(sTitle);

	//Memorise pListCtrl
	pListCtrl = XRCCTRL(*this, "m_listCtrl7", wxListCtrl);

	//set a timer
	m_Timer = new wxTimer(this, 2);

}

#define MILLISECONDS_UPDATE 20
#define UPDATES_PER_SECOND (1000/MILLISECONDS_UPDATE)

void JoystickButtonDialog::InitMemButton(int i)
{
	if (i == -1) return;

	FunctionToAssign = i;

	// time is in milliseconds; but really is approx, but not important for this one if it runs slow or not
	// 20 milliseconds is 50hz; we want it fast enough for scanning button press
	m_Timer->Start(MILLISECONDS_UPDATE);
	// give 10 seconds
	loopnumber = UPDATES_PER_SECOND * 10;

	wxGetApp().m_PlatformSpecific.DisableJoystickEvents();

}

void JoystickButtonDialog::OnTimer(wxTimerEvent& WXUNUSED(event))
{
	bool bDone = false;

	//Update message
	if (loopnumber % UPDATES_PER_SECOND == 0)//to avoid flickering
	{
		wxString Message;
		Message = wxString::Format(wxT("(%d) Press a Button or Direction"), loopnumber / UPDATES_PER_SECOND);
		SetTextinLV(Message);
	}

	loopnumber--;
	if (loopnumber < 1)//if no button pressed
	{
		bDone = true;
	}
	else
	{
		//can't use SDL_PollEvent(&event) because of timer :(
		if (FunctionToAssign >= CPC_JOYSTICK_NUM_AXES)
		{
			//test button
			int n = wxGetApp().m_PlatformSpecific.GetPressedButton(m_nJoystickID);
			if (n != -1)
			{
				//printf ( "found you pressed button %i\n", n );
				if (FunctionToAssign != -1)
				{
					Joystick_SetButtonMappingCPC(m_nCPCJoystickID, FunctionToAssign - CPC_JOYSTICK_NUM_AXES, n);
				}

				bDone = true;
			}
		}
		else
		{
			//test Axis
			int n = wxGetApp().m_PlatformSpecific.GetPressedAxis(m_nJoystickID);
			if (n != -1)
			{
				//printf ( "found you pressed axes %i\n", n );
				if (FunctionToAssign != -1)
				{
					if (n >= 10)
					{
						// if pov set both axes

						// n is either 10 or 11, axis 0 goes to 10, axis 1 goes to 11.
						// the code allows multiple pov, but 2 axes for each
						Joystick_SetAxisMappingPhysical(m_nCPCJoystickID, (FunctionToAssign&(~1)), (n&(~1)));

						Joystick_SetAxisMappingPhysical(m_nCPCJoystickID, (FunctionToAssign&(~1)) + 1, (n&(~1)) + 1);
					}
					else
					{
						Joystick_SetAxisMappingPhysical(m_nCPCJoystickID, FunctionToAssign, n);

					}
				}

				bDone = true;

			}
		}
	}

	if (bDone)
	{
		m_Timer->Stop();
		FunctionToAssign = -1;
		RefreshList();
		wxGetApp().m_PlatformSpecific.EnableJoystickEvents();
	}

}

void JoystickButtonDialog::DoConfigure()
{

	//Get button line config
	long itemIndex = pListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (itemIndex == -1)
	{
		return;
	}

	//memorise line
	m_CLLine = itemIndex;

	//star timer
	InitMemButton(m_CLLine);

}

void JoystickButtonDialog::OnButtonConfigSel(wxListEvent& WXUNUSED(event))
{
	//only one possible at the time
	if (m_CLLine != -1) return;

	DoConfigure();
}

void JoystickButtonDialog::SetTextinLV(wxString Text)
{
	if (m_CLLine == -1) return;

	wxListItem item;
	item.SetId(m_CLLine);
	item.SetColumn(1);

	item.SetText(Text);
	pListCtrl->SetItem(item);

}
