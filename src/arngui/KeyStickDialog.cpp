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
#include "arnguiApp.h"
#include "KeyStickDialog.h"
#include <wx/xrc/xmlres.h>
#include <wx/msgdlg.h>
#include <wx/menu.h>
#include "GetKeyDialog.h"

const wxChar *sKeyNames[JOYSTICK_SIMULATED_KEYID_LAST] =
{
	wxT("Up"),
	wxT("Down"),
	wxT("Left"),
	wxT("Right"),
	wxT("Fire1"),
	wxT("Fire2"),
	wxT("Fire3"),
	wxT("Left-Up"),
	wxT("Right-Up"),
	wxT("Left-Down"),
	wxT("Right-Down")
};

BEGIN_EVENT_TABLE(KeyStickDialog, wxDialog)
EVT_CONTEXT_MENU(KeyStickDialog::OnContextMenu)
EVT_LIST_ITEM_ACTIVATED(XRCID("m_listCtrl7"), KeyStickDialog::OnButtonConfigSel)
EVT_COMMAND(XRCID("m_menuItemConfigure"), wxEVT_COMMAND_MENU_SELECTED, KeyStickDialog::OnConfigure)
EVT_COMMAND(XRCID("m_menuItemClear"), wxEVT_COMMAND_MENU_SELECTED, KeyStickDialog::OnClear)
END_EVENT_TABLE()


void KeyStickDialog::OnConfigure(wxCommandEvent & WXUNUSED(event))
{
	DoConfigure();
}


void KeyStickDialog::OnClear(wxCommandEvent & WXUNUSED(event))
{
	DoClear();
}


void KeyStickDialog::OnContextMenu(wxContextMenuEvent &event)
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


bool KeyStickDialog::TransferDataFromWindow()
{
	//Custom mode ?
	if (m_nKeySet == 4)
	{
		if (m_keys)
		{
			//Save custom remap
			for (int i = 0; i < JOYSTICK_SIMULATED_KEYID_LAST; i++)
			{
				m_keys[i] = KeySetlist[i];
			}
		}
	}

	return true;
}

void KeyStickDialog::OnButtonConfigSel(wxListEvent& WXUNUSED(event))
{
	DoConfigure();
}


void KeyStickDialog::RefreshList()
{
	long topItem = pListCtrl->GetTopItem();
	long lastItem = topItem + pListCtrl->GetCountPerPage() - 1;
	long selectedItem = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	pListCtrl->Freeze();
	pListCtrl->DeleteAllItems();

	for (size_t i = 0; i < sizeof(sKeyNames) / sizeof(sKeyNames[0]); i++)
	{
		wxListItem Item;
		Item.SetMask(wxLIST_MASK_TEXT);
		Item.SetId(i);

		// id
		Item.SetText(sKeyNames[i]);
		Item.SetColumn(0);
		Item.SetImage(-1); //Insert a blank icon, not find to insert nothing
		int nItem = pListCtrl->InsertItem(Item);


		wxString sKeyName = wxT("Not set");
		if (KeySetlist[i] != 0)
		{
			sKeyName = wxGetApp().m_PlatformSpecific.GetKeyName(KeySetlist[i]);
		}

		Item.SetId(nItem);
		Item.SetText(sKeyName);
		Item.SetColumn(1);
		pListCtrl->SetItem(Item);
	}
	pListCtrl->Thaw();
	pListCtrl->Refresh();

	pListCtrl->EnsureVisible(lastItem);
	if (selectedItem != -1)
	{
		pListCtrl->SetItemState(selectedItem, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}
}


void KeyStickDialog::ClearAssignment(int nIndex)
{
	KeySetlist[nIndex] = 0;

	RefreshList();
}

void KeyStickDialog::DoClear()
{

	//Get button line config
	long itemIndex = pListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (itemIndex == -1)
	{
		return;
	}
	ClearAssignment(itemIndex);
}



void KeyStickDialog::DoConfigure()
{
	//Custom mode ?
	if (m_nKeySet != 4)
	{
		wxMessageDialog dialog(NULL, wxT("Redefinition is only available for 'Custom' keystick setting."), wxT(""), wxOK);
		dialog.ShowModal();
		return;
	}

	//Get button line config
	long itemIndex = pListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	GetKeyDialog dialog(this);
	dialog.m_nChosenKey = KeySetlist[itemIndex];
	if (dialog.ShowModal() == wxID_OK)
	{
		KeySetlist[itemIndex] = dialog.m_nChosenKey;
		RefreshList();
	}

}

void KeyStickDialog::OnListKeyDown(wxListEvent& event)
{
	if (event.GetKeyCode() == WXK_DELETE)
	{
		DoClear();
	}
	else if (event.GetKeyCode() == WXK_RETURN)
	{
		DoConfigure();
	}
	event.Skip();
}

bool KeyStickDialog::TransferDataToWindow()
{

	pListCtrl->Freeze();
	pListCtrl->ClearAll();

	wxListItem Column;
	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT("CPC Joystick Input"));
	pListCtrl->InsertColumn(0, Column);
	Column.SetText(wxT("Key"));
	pListCtrl->InsertColumn(1, Column);
	pListCtrl->Thaw();

	RefreshList();

	wxGetApp().SetColumnSize(pListCtrl, 0);
	wxGetApp().SetColumnSize(pListCtrl, 1);

	if (!m_keys)
	{
		pListCtrl->Disable();
	}
		
	return true;
}

//constructor
KeyStickDialog::KeyStickDialog(wxWindow *pParent, int nKeySet)
{
	m_nKeySet = nKeySet;

	//Make a copy for temporary modification in custom mode
#if ((defined(GTK2_EMBED_WINDOW) && defined(__WXGTK__)) || (defined(MAC_EMBED_WINDOW) && defined(__WXMAC__)))
	m_keys = wxGetApp().GetKeySetPtr(nKeySet);
#else
	m_keys = wxGetApp().m_PlatformSpecific.GetKeySetPtr(nKeySet);
#endif
	if (m_keys)
	{
		for (int i = 0; i < JOYSTICK_SIMULATED_KEYID_LAST; i++)
		{
			KeySetlist[i] = m_keys[i];
		}
	}
	else
	{
		for (int i = 0; i < JOYSTICK_SIMULATED_KEYID_LAST; i++)
		{
			KeySetlist[i] = 0;
		}
	}		
	
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_DIALOG_KEYSTICK"));

	//Memorise pListCtrl
	pListCtrl = XRCCTRL(*this, "m_listCtrl7", wxListCtrl);
}



