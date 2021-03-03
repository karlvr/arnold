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
#include "PositionalKeyboardDialog.h"
#include <wx/xrc/xmlres.h>
#include "arnguiApp.h"
#include <wx/menu.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include "DefineKeyDialog.h"

BEGIN_EVENT_TABLE(PositionalKeyboardDialog, wxDialog)
EVT_CONTEXT_MENU(PositionalKeyboardDialog::OnContextMenu)
EVT_LIST_ITEM_ACTIVATED(XRCID("m_listCtrl7"), PositionalKeyboardDialog::OnButtonConfigSel)
EVT_COMMAND(XRCID("m_menuItemConfigure"), wxEVT_COMMAND_MENU_SELECTED, PositionalKeyboardDialog::OnConfigure)
EVT_COMMAND(XRCID("m_menuItemClear"), wxEVT_COMMAND_MENU_SELECTED, PositionalKeyboardDialog::OnClear)
END_EVENT_TABLE()

void PositionalKeyboardDialog::OnConfigure(wxCommandEvent & WXUNUSED(event))
{
	DoConfigure();
}


void PositionalKeyboardDialog::OnClear(wxCommandEvent & WXUNUSED(event))
{
	DoClear();
}


void PositionalKeyboardDialog::OnContextMenu(wxContextMenuEvent &event)
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

bool PositionalKeyboardDialog::TransferDataFromWindow()
{
	wxGetApp().m_PlatformSpecific.WriteKeyList(&KeyDef);

	return true;
}

void PositionalKeyboardDialog::OnButtonConfigSel(wxListEvent& WXUNUSED(event))
{
	DoConfigure();
}


void PositionalKeyboardDialog::RefreshList()
{
	long topItem = pListCtrl->GetTopItem();
	long lastItem = topItem + pListCtrl->GetCountPerPage() - 1;
	long selectedItem = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	pListCtrl->Freeze();
	pListCtrl->DeleteAllItems();

	// exclude joystick?
	for (size_t i = 0; i < CPC_KEY_NUM_KEYS; i++)
	{
		if ((i >= CPC_KEY_JOY_UP) && (i < CPC_KEY_DEL))
			continue;

		wxListItem Item;
		Item.SetMask(wxLIST_MASK_TEXT);
		Item.SetId(i);

		// id
		wxString sCPCKeyName(CPC_GetKeyName((CPC_KEY_ID)i), *wxConvCurrent);
		Item.SetText(sCPCKeyName);
		Item.SetColumn(0);
		Item.SetImage(-1); //Insert a blank icon, not find to insert nothing
		int nItem = pListCtrl->InsertItem(Item);

		for (int j = 0; j < MaxNumKeys; j++)
		{

			wxString sKeyName = wxT("Not set");
			if (KeyDef.Keys[i][j] != CPC_KEY_DEF_UNSET_KEY)
			{
				sKeyName = wxGetApp().m_PlatformSpecific.GetKeyName(KeyDef.Keys[i][j]);
			}

			Item.SetId(nItem);
			Item.SetText(sKeyName);
			Item.SetColumn(1 + j);
			pListCtrl->SetItem(Item);
		}
	}
	pListCtrl->Thaw();
	pListCtrl->Refresh();

	pListCtrl->EnsureVisible(lastItem);
	if (selectedItem != -1)
	{
		pListCtrl->SetItemState(selectedItem, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}
}


void PositionalKeyboardDialog::ClearAssignment(int nIndex)
{
	for (int i = 0; i < MaxNumKeys; i++)
	{
		KeyDef.Keys[nIndex][i] = CPC_KEY_DEF_UNSET_KEY;
	}

	RefreshList();
}

void PositionalKeyboardDialog::DoClear()
{
	long itemIndex = pListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (itemIndex == -1)
	{
		return;
	}
	ClearAssignment(itemIndex);
}



void PositionalKeyboardDialog::DoConfigure()
{
	long itemIndex = pListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	DefineKeyDialog dialog(this, itemIndex);
	for (int i = 0; i < MaxNumKeys; i++)
	{
		dialog.m_nChosenKeys[i] = KeyDef.Keys[itemIndex][i];
	}
	if (dialog.ShowModal() == wxID_OK)
	{
		for (int i = 0; i < MaxNumKeys; i++)
		{
			KeyDef.Keys[itemIndex][i] = dialog.m_nChosenKeys[i];
		}
		RefreshList();
	}
}

void PositionalKeyboardDialog::OnListKeyDown(wxListEvent& event)
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

bool PositionalKeyboardDialog::TransferDataToWindow()
{

	pListCtrl->Freeze();
	pListCtrl->ClearAll();

	wxListItem Column;
	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT("CPC Key"));
	pListCtrl->InsertColumn(0, Column);
	for (int i = 0; i < MaxNumKeys; i++)
	{
		wxString sKeyLabel = wxString::Format(wxT("Host Key %d"), i);
		Column.SetText(sKeyLabel);
		pListCtrl->InsertColumn(i + 1, Column);
	}
	pListCtrl->Thaw();

	RefreshList();

	wxGetApp().SetColumnSize(pListCtrl, 0);
	for (int i=0; i<MaxNumKeys; i++)
	{
		wxGetApp().SetColumnSize(pListCtrl, i+1);
	}

	return true;
}

//constructor
PositionalKeyboardDialog::PositionalKeyboardDialog(wxWindow *pParent)
{
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_DIALOG_KEYBOARD"));

	pListCtrl = XRCCTRL(*this, "m_listCtrl7", wxListCtrl);

	wxGetApp().m_PlatformSpecific.SetupKeyList(&KeyDef);

}



