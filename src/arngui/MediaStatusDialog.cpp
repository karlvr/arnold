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
#include "MediaStatusDialog.h"
#include <wx/xrc/xmlres.h>
#include "arnguiMain.h"
#include "arnguiApp.h"
#include <wx/menu.h>

static const wxChar *sYes = wxT("Yes");
static const wxChar *sNo = wxT("No");



MediaStatusListCtrl::MediaStatusListCtrl() : SortableListCtrl()
{
}

MediaStatusListCtrl::~MediaStatusListCtrl()
{
}

IMPLEMENT_DYNAMIC_CLASS(MediaStatusListCtrl, SortableListCtrl)

BEGIN_EVENT_TABLE(MediaStatusListCtrl, SortableListCtrl)
END_EVENT_TABLE()


MediaStatusDialog *MediaStatusDialog::m_pInstance = NULL;

MediaStatusDialog::MediaStatusDialog(wxWindow *pParent)
{
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("MEDIA_STATUS_DIALOG"));
}

BEGIN_EVENT_TABLE(MediaStatusDialog, wxDialog)
EVT_CLOSE(MediaStatusDialog::OnClose)
EVT_CONTEXT_MENU(MediaStatusDialog::OnContextMenu)
//EVT_COMMAND(XRCID("m_MediaLoad"), wxEVT_COMMAND_MENU_SELECTED, MediaStatusDialog::OnLoad)
EVT_COMMAND(XRCID("m_MediaReload"), wxEVT_COMMAND_MENU_SELECTED, MediaStatusDialog::OnReload)
EVT_COMMAND(XRCID("m_MediaUnload"), wxEVT_COMMAND_MENU_SELECTED, MediaStatusDialog::OnUnload)
EVT_COMMAND(XRCID("m_MediaForceUnload"), wxEVT_COMMAND_MENU_SELECTED, MediaStatusDialog::OnForceUnload)
EVT_COMMAND(XRCID("m_MediaSaveAs"), wxEVT_COMMAND_MENU_SELECTED, MediaStatusDialog::OnSaveAs)
EVT_COMMAND(XRCID("m_MediaForceSave"), wxEVT_COMMAND_MENU_SELECTED, MediaStatusDialog::OnForceSave)
EVT_CHAR_HOOK(MediaStatusDialog::OnCharHook)
END_EVENT_TABLE()


void MediaStatusDialog::OnCharHook(wxKeyEvent& event)
{
	if (IsEscapeKey(event))
	{
		Close();
		return;
	}

	event.Skip();
}


void MediaStatusDialog::OnUnload(wxCommandEvent & WXUNUSED(event))
{
	wxListCtrl *pListCtrl = GetListCtrl();
	long item = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	while (item != -1)
	{
		wxUIntPtr Data = pListCtrl->GetItemData(item);
		Media *pMedia = (Media *)Data;

		pMedia->Unload();

		item = pListCtrl->GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);
	}


}


void MediaStatusDialog::OnForceUnload(wxCommandEvent & WXUNUSED(event))
{
	wxListCtrl *pListCtrl = GetListCtrl();
	long item = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	while (item != -1)
	{
		wxUIntPtr Data = pListCtrl->GetItemData(item);
		Media *pMedia = (Media *)Data;

		pMedia->Unload(true);

		item = pListCtrl->GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);
	}


}

void MediaStatusDialog::OnSaveAs(wxCommandEvent & WXUNUSED(event))
{
	wxListCtrl *pListCtrl = GetListCtrl();
	long item = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	while (item != -1)
	{
		wxUIntPtr Data = pListCtrl->GetItemData(item);
		Media *pMedia = (Media *)Data;

		pMedia->SaveAs();

		item = pListCtrl->GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);
	}


}



void MediaStatusDialog::OnReload(wxCommandEvent & WXUNUSED(event))
{
	wxListCtrl *pListCtrl = GetListCtrl();
	long item = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	while (item != -1)
	{
		wxUIntPtr Data = pListCtrl->GetItemData(item);
		Media *pMedia = (Media *)Data;

		pMedia->Reload();

		item = pListCtrl->GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);
	}


}


void MediaStatusDialog::OnForceSave(wxCommandEvent & WXUNUSED(event))
{
	wxListCtrl *pListCtrl = GetListCtrl();
	long item = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	while (item != -1)
	{
		wxUIntPtr Data = pListCtrl->GetItemData(item);
		Media *pMedia = (Media *)Data;

		pMedia->ForceSave();

		item = pListCtrl->GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);
	}


}

void MediaStatusDialog::OnContextMenu(wxContextMenuEvent &event)
{
	wxPoint Position = event.GetPosition();

	// generate a suitable place for the menu
	if (event.GetPosition() == wxDefaultPosition)
	{
		wxPoint ClientPos(0, 0);
		Position = this->ClientToScreen(ClientPos);
	}

	wxMenuBar *pMenuBar = wxXmlResource::Get()->LoadMenuBar(wxT("MB_MENU_MEDIA_MENU_BAR"));
	if (pMenuBar)
	{
		wxMenu *pMenu = pMenuBar->Remove(0);
		if (pMenu)
		{
			PopupMenu(pMenu);
			delete pMenu;
		}
		delete pMenuBar;
	}
}

void MediaStatusDialog::OnClose(wxCloseEvent & WXUNUSED(event))
{
	arnguiFrame *pFrame = static_cast<arnguiFrame *>(GetParent());
	pFrame->DeRegisterWantUpdateFromTimer(this);

	wxGetApp().WriteConfigWindowPosAndSize(wxT("windows/media_status/"), this);

	wxGetApp().WriteConfigListCtrl(wxT("windows/media_status/listctrl/"), GetListCtrl());


	this->Destroy();

	MediaStatusDialog::m_pInstance = NULL;
}

wxListCtrl *MediaStatusDialog::GetListCtrl()
{
	return XRCCTRL(*this, "m_listCtrl5", wxListCtrl);
}


MediaStatusDialog::~MediaStatusDialog()
{
	wxListCtrl *pListCtrl = GetListCtrl();

	for (int i = 0; i < pListCtrl->GetItemCount(); i++)
	{
		//   wxStringClientData *pClientData = (wxStringClientData *)(pListCtrl->GetItemData(i));
		//  delete pClientData;
		// pListCtrl->SetItemData(i, NULL);
	}

	pListCtrl->DeleteAllItems();
}

// this causes flicker because it gets refreshed and causes the
// tooltip to flash on/off
// we need to know if the item is dirty before refreshing.
void MediaStatusDialog::UpdateMedia(int nIndex)
{
	wxString sValue;
	wxListCtrl *pListCtrl = GetListCtrl();

	wxUIntPtr Data = pListCtrl->GetItemData(nIndex);
	Media *pMedia = (Media *)Data;

	wxListItem Item;
	Item.SetMask(wxLIST_MASK_TEXT);
	Item.SetId(nIndex);

	// id
	sValue = pMedia->GetName();
	if (sValue.IsEmpty())
	{
		if (pMedia->GetMediaInserted())
		{
			sValue = wxT("-Unnamed-");
		}
	}
	Item.SetText(sValue);
	Item.SetColumn(0);
	Item.SetImage(-1); //Insert a blank icon, not find to insert nothing
	pListCtrl->SetItem(Item);

	wxString sPath = pMedia->GetCurrentPath();
	if (sPath.IsEmpty())
	{
		if (pMedia->GetMediaInserted())
		{
			if (pMedia->IsModified())
			{
				sPath = wxT("Unsaved file");
			}
			else
			{
				sPath = wxT("Unnamed file");
			}
		}
	}

	// path
	Item.SetText(sPath);
	Item.SetColumn(1);
	pListCtrl->SetItem(Item);

	if (pMedia->GetMediaInserted())
	{
		sValue = sYes;
	}
	else
	{
		sValue = sNo;
	}

	Item.SetText(sValue);
	Item.SetColumn(2);
	pListCtrl->SetItem(Item);



	if (pMedia->CanBeModified() /*&& pMedia->GetMediaInserted()*/)
	{

		if (pMedia->IsModified())
		{
			sValue = sYes;
		}
		else
		{
			sValue = sNo;
		}
	}
	else
	{
		sValue = wxT("-");
	}
	Item.SetText(sValue);
	Item.SetColumn(3);
	pListCtrl->SetItem(Item);



}

void MediaStatusDialog::AddMedia(Media *pMedia)
{
	wxString sValue = wxEmptyString;

	wxListCtrl *pListCtrl = GetListCtrl();
	wxListItem Item;


	Item.SetMask(wxLIST_MASK_TEXT);
	Item.SetId(pListCtrl->GetItemCount());
	Item.SetText(sValue);
	Item.SetColumn(0);
	Item.SetImage(-1); //Insert a blank icon, not find to insert nothing
	long nIndex = pListCtrl->InsertItem(Item);

	//	// blank columns
	//	Item.SetId(nIndex);
	//	Item.SetColumn(1);
	//	pListCtrl->SetItem(Item);

	//	Item.SetColumn(2);
	//	pListCtrl->SetItem(Item);

	//	Item.SetColumn(3);
	//	pListCtrl->SetItem(Item);


	pListCtrl->SetItemPtrData(nIndex, (wxUIntPtr)pMedia);

	UpdateMedia(nIndex);
}

void MediaStatusDialog::TimedUpdate()
{
	wxListCtrl *pListCtrl = GetListCtrl();

	pListCtrl->Freeze();
	//	pListCtrl->DeleteAllItems();
	for (int i = 0; i < pListCtrl->GetItemCount(); i++)
	{
		UpdateMedia(i);
	}

	pListCtrl->Thaw();

}

bool MediaStatusDialog::TransferDataToWindow()
{
	wxListCtrl *pListCtrl = GetListCtrl();

	pListCtrl->Freeze();
	pListCtrl->ClearAll();

	wxListItem Column;
	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT("Media"));
	pListCtrl->InsertColumn(0, Column);
	Column.SetText(wxT("Current Filename"));
	pListCtrl->InsertColumn(1, Column);
	Column.SetText(wxT("Inserted"));
	pListCtrl->InsertColumn(2, Column);
	Column.SetText(wxT("Modified"));
	pListCtrl->InsertColumn(3, Column);

	pListCtrl->Thaw();

	pListCtrl->Freeze();
	pListCtrl->DeleteAllItems();

	for (unsigned int i = 0; i != wxGetApp().m_Media.GetCount(); i++)
	{
		Media *pMedia = wxGetApp().m_Media[i];
		AddMedia(pMedia);

	}

	pListCtrl->Thaw();


	wxGetApp().SetColumnSize(pListCtrl, 0);
	wxGetApp().SetColumnSize(pListCtrl, 1);
	wxGetApp().SetColumnSize(pListCtrl, 2);
	wxGetApp().SetColumnSize(pListCtrl, 3);

	arnguiFrame *pFrame = static_cast<arnguiFrame *>(GetParent());

	pFrame->RegisterWantUpdateFromTimer(this);

	// default settings
	//this->SetSize(530,140);

	wxGetApp().ReadConfigWindowPosAndSize(wxT("windows/media_status/"), this);

	wxGetApp().ReadConfigListCtrl(wxT("windows/media_status/listctrl/"), GetListCtrl());
	wxGetApp().EnsureWindowVisible(this);


	return true;
}












