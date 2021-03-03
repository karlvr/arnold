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
#include "DefineKeyDialog.h"
#include <wx/xrc/xmlres.h>
#include <wx/listctrl.h>
#include <wx/menu.h>
#include "IntClientData.h"
#include "GetKeyDialog.h"

BEGIN_EVENT_TABLE(DefineKeyDialog, wxDialog)
EVT_CONTEXT_MENU(DefineKeyDialog::OnContextMenu)
EVT_LIST_ITEM_ACTIVATED(XRCID("m_listCtrl25"), DefineKeyDialog::OnConfigureListCtrl)
EVT_COMMAND(XRCID("m_menuItemConfigure"), wxEVT_COMMAND_MENU_SELECTED, DefineKeyDialog::OnConfigure)
EVT_COMMAND(XRCID("m_menuItemClear"), wxEVT_COMMAND_MENU_SELECTED, DefineKeyDialog::OnClear)
END_EVENT_TABLE()

void DefineKeyDialog::OnConfigureListCtrl(wxListEvent & WXUNUSED(event))
{
	DoConfigure();
}

void DefineKeyDialog::OnContextMenu(wxContextMenuEvent &event)
{
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


void DefineKeyDialog::DoConfigure()
{
	wxListCtrl *pListCtrl = GetListCtrl();

	long itemIndex = pListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	if (itemIndex != -1)
	{
		GetKeyDialog dialog(this);
		if (dialog.ShowModal() == wxID_OK)
		{
			m_nChosenKeys[itemIndex] = dialog.m_nChosenKey;
		}
	}
	RefreshList();

}

void DefineKeyDialog::OnConfigure(wxCommandEvent & WXUNUSED(event))
{
	DoConfigure();
}


void DefineKeyDialog::OnClear(wxCommandEvent & WXUNUSED(event))
{
	wxListCtrl *pListCtrl = GetListCtrl();

	long itemIndex = pListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	if (itemIndex != -1)
	{
		m_nChosenKeys[itemIndex] = CPC_KEY_DEF_UNSET_KEY;
	}
	RefreshList();

}

wxListCtrl *DefineKeyDialog::GetListCtrl()
{
	return XRCCTRL(*this, "m_listCtrl25", wxListCtrl);
}


bool DefineKeyDialog::TransferDataFromWindow()
{
	return true;
}

void DefineKeyDialog::RefreshList()
{
	wxListCtrl *pListCtrl = GetListCtrl();
	long topItem = pListCtrl->GetTopItem();
	long lastItem = topItem + pListCtrl->GetCountPerPage() - 1;
	long selectedItem = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	pListCtrl->Freeze();
	pListCtrl->DeleteAllItems();

	for (int i = 0; i < MaxNumKeys; i++)
	{
		wxListItem Item;
		Item.SetMask(wxLIST_MASK_TEXT);
		Item.SetId(i);

		// id
		wxString sNumber = wxString::Format(wxT("%d"), i);

		Item.SetText(sNumber);
		Item.SetColumn(0);
		Item.SetImage(-1); //Insert a blank icon, not find to insert nothing
		int nItem = pListCtrl->InsertItem(Item);

		wxString sKeyName(wxT("Not set"));
		int Key = m_nChosenKeys[i];

		if (m_nChosenKeys[i] != CPC_KEY_DEF_UNSET_KEY)
		{
			sKeyName = wxGetApp().m_PlatformSpecific.GetKeyName(Key);
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


bool DefineKeyDialog::TransferDataToWindow()
{
	wxListCtrl *pListCtrl = GetListCtrl();

	pListCtrl->Freeze();
	pListCtrl->ClearAll();

	wxListItem Column;
	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT("#"));
	pListCtrl->InsertColumn(0, Column);
	Column.SetText(wxT("Key"));
	pListCtrl->InsertColumn(1, Column);
	pListCtrl->Thaw();

	RefreshList();

	wxGetApp().SetColumnSize(pListCtrl, 0);
	wxGetApp().SetColumnSize(pListCtrl, 1);

	return true;
}

//constructor
DefineKeyDialog::DefineKeyDialog(wxWindow *pParent, int nCPCKey)
{
	for (int i = 0; i < MaxNumKeys; i++)
	{
		m_nChosenKeys[i] = CPC_KEY_DEF_UNSET_KEY;
	}
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_DIALOG_KEY_MAPPING"));

	wxString sCPCKeyName(CPC_GetKeyName((CPC_KEY_ID)nCPCKey), *wxConvCurrent);
	wxString sTitle(wxT("Define Host Keys for CPC Key: "));
	sCPCKeyName.Replace(wxT("&"), wxT("&&"));
	sTitle += sCPCKeyName;
	SetTitle(sTitle);

}

DefineKeyDialog::~DefineKeyDialog()
{
}
