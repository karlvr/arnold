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
#include "LabelManager.h"
#include <wx/xrc/xmlres.h>
#include "arnguiMain.h"
#include "arnguiApp.h"
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/splitter.h>
#include <wx/textdlg.h>
#include "DebuggerDialog.h"

static const wxChar *sYes = wxT("Yes");
static const wxChar *sNo = wxT("No");

void LabelsListCtrl::OnCreate(wxWindowCreateEvent & event)
{
	SortableListCtrl::OnCreate(event);

	this->Freeze();
	this->ClearAll();

	wxListItem Column;

	Column.SetMask(wxLIST_MASK_TEXT);

	Column.SetText(wxT("Name"));
	this->InsertColumn(0, Column);
	//	this->SetColumnWidth(0, 80);
	Column.SetText(wxT("Value"));
	this->InsertColumn(1, Column);
	//	this->SetColumnWidth(1, 80);
	Column.SetText(wxT("Enabled"));
	this->InsertColumn(2, Column);
	//	this->SetColumnWidth(2, 80);

	this->Thaw();
#if (wxVERSION_NUMBER >= 2900)
	Connect(wxEVT_LIST_ITEM_ACTIVATED, wxListEventHandler(LabelsListCtrl::OnLabelActivated));
#else
	Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler(LabelsListCtrl::OnLabelActivated));
#endif
}


LabelsListCtrl::LabelsListCtrl() : SortableListCtrl()
{
	m_pDialog = NULL;

}

LabelsListCtrl::~LabelsListCtrl()
{
}



int LabelsListCtrl::PerformSort(wxIntPtr item1id, wxIntPtr item2id)
{
	LABEL *pClientData1 = (LABEL *)item1id;
	LABEL *pClientData2 = (LABEL *)item2id;

	int result = 0;
	switch (m_nSortColumn)
	{
	default:
	case 0:
	{
		// case is not important
#if defined(__WXGTK__) || defined(__WXMAC__)
		result = strcasecmp(pClientData1->m_sName, pClientData1->m_sName);
#else			
		result = stricmp(pClientData1->m_sName, pClientData2->m_sName);
#endif
	}
	break;

	case 1:
	{
		result = pClientData1->m_Address - pClientData2->m_Address;
	}
	break;

	case 2:
	{
		if (pClientData1->m_bActive == pClientData2->m_bActive)
			result = 0;
		else if ((pClientData1->m_bActive) && (!pClientData2->m_bActive))
			result = -1;
		else if ((!pClientData1->m_bActive) && (pClientData2->m_bActive))
			result = 1;
	}
	break;

	}

	if (!m_bSortAscending)
	{
		result = -result;
	}
	return result;
}

IMPLEMENT_DYNAMIC_CLASS(LabelsListCtrl, SortableListCtrl)

BEGIN_EVENT_TABLE(LabelsListCtrl, SortableListCtrl)
EVT_CONTEXT_MENU(LabelsListCtrl::OnContextMenu)
EVT_WINDOW_CREATE(LabelsListCtrl::OnCreate)
EVT_KEY_DOWN(LabelsListCtrl::OnKeyDown)
EVT_COMMAND(XRCID("AddLabel"), wxEVT_COMMAND_MENU_SELECTED, LabelsListCtrl::OnAddLabel)
EVT_COMMAND(XRCID("RemoveLabel"), wxEVT_COMMAND_MENU_SELECTED, LabelsListCtrl::OnRemoveLabel)
EVT_COMMAND(XRCID("EnableLabel"), wxEVT_COMMAND_MENU_SELECTED, LabelsListCtrl::OnEnableLabel)
EVT_COMMAND(XRCID("DisableLabel"), wxEVT_COMMAND_MENU_SELECTED, LabelsListCtrl::OnDisableLabel)
EVT_COMMAND(XRCID("RenameLabel"), wxEVT_COMMAND_MENU_SELECTED, LabelsListCtrl::OnRenameLabel)
EVT_COMMAND(XRCID("GoToLabel"), wxEVT_COMMAND_MENU_SELECTED, LabelsListCtrl::OnGoToLabel)
END_EVENT_TABLE()




void LabelSetListCtrl::OnCreate(wxWindowCreateEvent & event)
{
	SortableListCtrl::OnCreate(event);

	wxListItem Column;

	this->Freeze();
	this->ClearAll();

	Column.SetMask(wxLIST_MASK_TEXT);

	Column.SetText(wxT("Name"));
	this->InsertColumn(0, Column);
	//	this->SetColumnWidth(0, 80);
	Column.SetText(wxT("Enabled"));
	this->InsertColumn(1, Column);
	//	this->SetColumnWidth(1, 80);

	this->Thaw();
#if (wxVERSION_NUMBER >= 2900)
	Connect(wxEVT_LIST_ITEM_SELECTED, wxListEventHandler(LabelSetListCtrl::OnLabelSetChange));
#else
	Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler(LabelSetListCtrl::OnLabelSetChange));
#endif
}

LabelSetListCtrl::LabelSetListCtrl() : SortableListCtrl(), m_pSelectedLabelSet(NULL)
{
}

LabelSetListCtrl::~LabelSetListCtrl()
{
}

void LabelsListCtrl::SendDataToDissassembly(LABEL *pLabel)
{
	if (pLabel != NULL)
	{
		if (DebuggerWindow::GetDissassemblyWindowCount() != 0)
		{
			DissassembleWindow *pWindow = DebuggerWindow::GetDissassemblyWindowByIndex(0);
			if (pWindow)
			{
				DISSASSEMBLE_WINDOW *pDisassembleWindow = pWindow->GetStruct();
				Dissassemble_SetAddress(pDisassembleWindow, pLabel->m_Address);
				pWindow->Refresh();
			}
		}

	}
}
void LabelsListCtrl::OnLabelActivated(wxListEvent & event)
{
	LABEL *pLabel = (LABEL *)event.GetData();
	if (pLabel!=NULL)
	{
		SendDataToDissassembly(pLabel);
}
}


int LabelSetListCtrl::PerformSort(wxIntPtr item1id, wxIntPtr item2id)
{
	LABELSET *pClientData1 = (LABELSET *)item1id;
	LABELSET *pClientData2 = (LABELSET *)item2id;

	int result = 0;
	switch (m_nSortColumn)
	{
	default:
	case 0:
	{
		// case is not important
#if defined(__WXGTK__) || defined(__WXMAC__)
		result = strcasecmp(pClientData1->m_sName, pClientData1->m_sName);
#else
		result = stricmp(pClientData1->m_sName, pClientData2->m_sName);
#endif
	}
	break;

	case 1:
	{
		if (pClientData1->m_bActive == pClientData2->m_bActive)
			result = 0;
		else if ((pClientData1->m_bActive) && (!pClientData2->m_bActive))
			result = -1;
		else if ((!pClientData1->m_bActive) && (pClientData2->m_bActive))
			result = 1;
	}
	break;
	}

	if (!m_bSortAscending)
	{
		result = -result;
	}
	return result;
}

IMPLEMENT_DYNAMIC_CLASS(LabelSetListCtrl, SortableListCtrl)

BEGIN_EVENT_TABLE(LabelSetListCtrl, SortableListCtrl)
EVT_WINDOW_CREATE(LabelSetListCtrl::OnCreate)
EVT_CONTEXT_MENU(LabelSetListCtrl::OnContextMenu)
EVT_KEY_DOWN(LabelSetListCtrl::OnKeyDown)
EVT_COMMAND(XRCID("AddLabel"), wxEVT_COMMAND_MENU_SELECTED, LabelSetListCtrl::OnAddLabel)
EVT_COMMAND(XRCID("RemoveLabel"), wxEVT_COMMAND_MENU_SELECTED, LabelSetListCtrl::OnRemoveLabel)
EVT_COMMAND(XRCID("EnableLabel"), wxEVT_COMMAND_MENU_SELECTED, LabelSetListCtrl::OnEnableLabel)
EVT_COMMAND(XRCID("DisableLabel"), wxEVT_COMMAND_MENU_SELECTED, LabelSetListCtrl::OnDisableLabel)
EVT_COMMAND(XRCID("RenameLabel"), wxEVT_COMMAND_MENU_SELECTED, LabelSetListCtrl::OnRenameLabel)
EVT_COMMAND(XRCID("LoadLabelsNOI"), wxEVT_COMMAND_MENU_SELECTED, LabelSetListCtrl::OnLoadLabelsNOI)
EVT_COMMAND(XRCID("LoadLabelsPasmo"), wxEVT_COMMAND_MENU_SELECTED, LabelSetListCtrl::OnLoadLabelsPasmo)
EVT_COMMAND(XRCID("LoadLabelsSjasm"), wxEVT_COMMAND_MENU_SELECTED, LabelSetListCtrl::OnLoadLabelsSjasm)
EVT_COMMAND(XRCID("SaveLabelsPasmo"), wxEVT_COMMAND_MENU_SELECTED, LabelSetListCtrl::OnSaveLabelsPasmo)
EVT_COMMAND(XRCID("LoadLabelsWinape"), wxEVT_COMMAND_MENU_SELECTED, LabelSetListCtrl::OnLoadLabelsWinape)
EVT_COMMAND(XRCID("LoadLabelsRasm"), wxEVT_COMMAND_MENU_SELECTED, LabelSetListCtrl::OnLoadLabelsRasm)
END_EVENT_TABLE()


LabelManagerDialog *LabelManagerDialog::m_pInstance = NULL;

void LabelManagerDialog::OnClose(wxCloseEvent & WXUNUSED(event))
{
	wxSplitterWindow *pSplitterWindow;

	pSplitterWindow = XRCCTRL(*this, "m_splitter4", wxSplitterWindow);
	wxConfig::Get(false)->Write(wxT("windows/labelmanager/splitter/sashpos"), pSplitterWindow->GetSashPosition());

	wxGetApp().WriteConfigWindowPosAndSize(wxT("windows/labelmanager/"), this);

	this->Destroy();

	LabelManagerDialog::m_pInstance = NULL;
}


LabelManagerDialog::LabelManagerDialog(wxWindow *pParent)
{

	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_DIALOG_LABELMANAGER"));


	wxGetApp().ReadConfigWindowPosAndSize(wxT("windows/labelmanager/"), this);
	wxGetApp().EnsureWindowVisible(this);


	wxSplitterWindow *pSplitterWindow;
	int SashPos;
	pSplitterWindow = XRCCTRL(*this, "m_splitter4", wxSplitterWindow);
	if (wxConfig::Get(false)->Read(wxT("windows/labelmanager/splitter/sashpos"), &SashPos, -1))
	{
		if (SashPos != -1)
		{
			pSplitterWindow->SetSashPosition(SashPos);
		}
	}
}

BEGIN_EVENT_TABLE(LabelManagerDialog, wxDialog)
EVT_CLOSE(LabelManagerDialog::OnClose)

END_EVENT_TABLE()


void LabelSetListCtrl::OnLabelSetChange(wxListEvent & event)
{
	LABELSET *pLabelSet = (LABELSET *)event.GetData();
	m_pSelectedLabelSet = pLabelSet;

	LabelsListCtrl *pLabelsListCtrl = m_pDialog->GetLabelsListCtrl();
	pLabelsListCtrl->Freeze();
	pLabelsListCtrl->DeleteAllItems();

	LABEL *pLabel = pLabelSet->m_pLabels;
	while (pLabel!=NULL)
	{
		pLabelsListCtrl->AddLabel(pLabel);

		pLabel = pLabel->m_pNext;
}

	pLabelsListCtrl->Thaw();
	pLabelsListCtrl->SortNow();
}

void LabelSetListCtrl::AddNewLabelSet()
{
	wxTextEntryDialog dialog(this, wxT("Label set name"), wxT("Enter name for new Label Set"));
	if (dialog.ShowModal() == wxID_OK)
	{
#if (wxVERSION_NUMBER >= 2900)
		// 2.9
		const wxCharBuffer tmp_buf = wxConvCurrent->cWX2MB(dialog.GetValue().wc_str());
#else
		// 2.8
		const wxWX2MBbuf tmp_buf = wxConvCurrent->cWX2MB(dialog.GetValue());
#endif
		const char *tmp_str = (const char*)tmp_buf;

		LABELSET *pNewLabelSet = labelset_create(tmp_str);
		if (pNewLabelSet != NULL)
		{
			AddLabelSet(pNewLabelSet);

			m_pSelectedLabelSet = pNewLabelSet;

		}
	}
}


void LabelSetListCtrl::RenameSelectedLabelSets()
{
	long item = -1;

	item = this->GetNextItem(item,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	while (item != -1)
	{
		LABELSET *pLabelSet = (LABELSET *)this->GetItemData(item);

		wxString sName(pLabelSet->m_sName, *wxConvCurrent);

		wxTextEntryDialog dialog(this, wxT("Label set name"), wxT("Enter name for new Label Set"), sName);
		if (dialog.ShowModal() == wxID_OK)
		{
#if (wxVERSION_NUMBER >= 2900)
			// 2.9
			const wxCharBuffer tmp_buf = wxConvCurrent->cWX2MB(dialog.GetValue().wc_str());
#else
			// 2.8
			const wxWX2MBbuf tmp_buf = wxConvCurrent->cWX2MB(dialog.GetValue());
#endif
			const char *tmp_str = (const char*)tmp_buf;

			labelset_set_name(pLabelSet, tmp_str);

			UpdateLabelSet(item);
		}

		item = this->GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);

	}
}


void LabelsListCtrl::OnGoToLabel(wxCommandEvent & WXUNUSED(event))
{
	long item = -1;

	item = this->GetNextItem(item,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	while (item != -1)
	{
		LABEL *pLabel = (LABEL *)this->GetItemData(item);
		SendDataToDissassembly(pLabel);


		item = this->GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);

	}
}



void LabelSetListCtrl::OnAddLabel(wxCommandEvent & WXUNUSED(event))
{
	AddNewLabelSet();
}

void LabelSetListCtrl::OnRenameLabel(wxCommandEvent & WXUNUSED(event))
{
	RenameSelectedLabelSets();
}


void LabelsListCtrl::AddNewLabel()
{
	LabelSetListCtrl *pLabelSetListCtrl = m_pDialog->GetLabelSetListCtrl();

	if (pLabelSetListCtrl->m_pSelectedLabelSet == NULL)
		return;

	NewLabelDialog dialog(this);
	if (dialog.ShowModal() == wxID_OK)
	{
#if (wxVERSION_NUMBER >= 2900)
		// 2.9
		const wxCharBuffer tmp_buf = wxConvCurrent->cWX2MB(dialog.m_sName.wc_str());
#else
		// 2.8
		const wxWX2MBbuf tmp_buf = wxConvCurrent->cWX2MB(pDialog->m_sName);
#endif
		const char *tmp_str = (const char*)tmp_buf;

		LABEL *pLabel = labelset_add_label(pLabelSetListCtrl->m_pSelectedLabelSet, tmp_str, dialog.m_nAddress);

		AddLabel(pLabel);
	}
}


void LabelsListCtrl::RenameSelectedLabels()
{

	long item = -1;

	item = this->GetNextItem(item,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	while (item!=-1)
	{

		long itemNext = this->GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);

		LABEL *pLabel = (LABEL *)this->GetItemData(item);

		wxString sName(pLabel->m_sName, *wxConvCurrent);

		wxTextEntryDialog dialog(this, wxT("Label name"), wxT("Enter name for label"), sName);
		if (dialog.ShowModal() == wxID_OK)
		{
#if (wxVERSION_NUMBER >= 2900)
			// 2.9
			const wxCharBuffer tmp_buf = wxConvCurrent->cWX2MB(dialog.GetValue().wc_str());
#else
			// 2.8
			const wxWX2MBbuf tmp_buf = wxConvCurrent->cWX2MB(dialog.GetValue());
#endif
			const char *tmp_str = (const char*)tmp_buf;

			label_set_name(pLabel, tmp_str);
		}


		UpdateLabel(item);


		item = itemNext;

	}
}


void LabelsListCtrl::OnAddLabel(wxCommandEvent & WXUNUSED(event))
{
	AddNewLabel();
}


void LabelsListCtrl::RemoveSelectedLabels()
{
	LabelSetListCtrl *pLabelSetListCtrl = m_pDialog->GetLabelSetListCtrl();

	if (pLabelSetListCtrl->m_pSelectedLabelSet == NULL)
		return;

	long item = -1;

	item = this->GetNextItem(item,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	while (item != -1)
	{

		long itemNext = this->GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);

		LABEL *pLabel = (LABEL *)this->GetItemData(item);

		labelset_delete_label(pLabelSetListCtrl->m_pSelectedLabelSet, pLabel);

		this->DeleteItem(item);

		item = itemNext;

	}


}


void LabelSetListCtrl::RemoveSelectedLabelSets()
{

	long item = -1;

	item = this->GetNextItem(item,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	while (item != -1)
	{
		long itemNext = this->GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);
		LABELSET *pLabelSet = (LABELSET *)this->GetItemData(item);

		labelset_delete(pLabelSet);

		m_pSelectedLabelSet = NULL;

		this->DeleteItem(item);


		item = itemNext;

	}

	LabelsListCtrl *pLabelsListCtrl = m_pDialog->GetLabelsListCtrl();
	pLabelsListCtrl->Freeze();
	pLabelsListCtrl->DeleteAllItems();
	pLabelsListCtrl->Thaw();
}

void LabelsListCtrl::OnRemoveLabel(wxCommandEvent & WXUNUSED(event))
{
	RemoveSelectedLabels();
}


void LabelsListCtrl::OnRenameLabel(wxCommandEvent & WXUNUSED(event))
{
	RenameSelectedLabels();
}


void LabelSetListCtrl::OnRemoveLabel(wxCommandEvent & WXUNUSED(event))
{
	RemoveSelectedLabelSets();
}

void LabelsListCtrl::ActivateSelectedLabels(BOOL bState)
{

	long item = -1;

	item = this->GetNextItem(item,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	while (item != -1)
	{
		LABEL *pLabel = (LABEL *)this->GetItemData(item);
		pLabel->m_bActive = bState;
		UpdateLabel(item);

		item = this->GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);

	}



}


void LabelSetListCtrl::ActivateSelectedLabelSets(BOOL bState)
{
	long item = -1;

	item = this->GetNextItem(item,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	while (item != -1)
	{
		LABELSET *pLabelSet = (LABELSET *)this->GetItemData(item);
		pLabelSet->m_bActive = bState;
		UpdateLabelSet(item);

		item = this->GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);

	}
}

void LabelSetListCtrl::OnEnableLabel(wxCommandEvent & WXUNUSED(event))
{
	ActivateSelectedLabelSets(TRUE);
}

void LabelsListCtrl::OnEnableLabel(wxCommandEvent & WXUNUSED(event))
{
	ActivateSelectedLabels(TRUE);
}

void LabelSetListCtrl::OnDisableLabel(wxCommandEvent & WXUNUSED(event))
{
	ActivateSelectedLabelSets(FALSE);
}

void LabelsListCtrl::OnDisableLabel(wxCommandEvent & WXUNUSED(event))
{
	ActivateSelectedLabels(FALSE);
}

void LabelSetListCtrl::OnContextMenu(wxContextMenuEvent &event)
{
	wxPoint Position = event.GetPosition();

	// generate a suitable place for the menu
	if (event.GetPosition() == wxDefaultPosition)
	{
		wxPoint ClientPos(0, 0);
		Position = this->ClientToScreen(ClientPos);
	}

	wxMenuBar *pMenuBar = wxXmlResource::Get()->LoadMenuBar(wxT("MB_MENU_LABELSET_MENU_BAR"));
	if (pMenuBar)
	{
		wxMenu *pMenu = pMenuBar->Remove(0);
		if (pMenu != NULL)
		{
			PopupMenu(pMenu);
			delete pMenu;
		}
		delete pMenuBar;
	}
}

void LabelsListCtrl::OnContextMenu(wxContextMenuEvent &event)
{
	wxPoint Position = event.GetPosition();

	// generate a suitable place for the menu
	if (event.GetPosition() == wxDefaultPosition)
	{
		wxPoint ClientPos(0, 0);
		Position = this->ClientToScreen(ClientPos);
	}

	wxMenuBar *pMenuBar = wxXmlResource::Get()->LoadMenuBar(wxT("MB_MENU_LABEL_MENU_BAR"));
	if (pMenuBar)
	{
		wxMenu *pMenu = pMenuBar->Remove(0);
		if (pMenu != NULL)
		{
			PopupMenu(pMenu);
			delete pMenu;
		}
		delete pMenuBar;
	}
}


LabelManagerDialog::~LabelManagerDialog()
{
}

void LabelsListCtrl::UpdateLabel(int nIndex)
{
	wxString sValue;

	LABEL *pLabel = (LABEL *)this->GetItemData(nIndex);

	wxListItem Item;
	Item.SetMask(wxLIST_MASK_TEXT);
	Item.SetId(nIndex);

	// label name

	sValue = wxString((const char *)pLabel->m_sName, *wxConvCurrent);
	Item.SetText(sValue);
	Item.SetColumn(0);
	Item.SetImage(-1); //Insert a blank icon, not find to insert nothing
	this->SetItem(Item);

	// label value

	sValue.Printf(wxT("&%04x"), pLabel->m_Address);
	Item.SetText(sValue);
	Item.SetColumn(1);
	this->SetItem(Item);

	sValue = pLabel->m_bActive ? sYes : sNo;
	Item.SetText(sValue);
	Item.SetColumn(2);
	this->SetItem(Item);
}

void LabelsListCtrl::AddLabel(LABEL *pLabel)
{
	wxString sValue = wxT("Temp");

	wxListItem Item;

	// name
	Item.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_IMAGE);
	Item.SetId(this->GetItemCount());
	Item.SetText(sValue);
	Item.SetColumn(0);
	Item.SetData(pLabel);
	Item.SetImage(-1); //Insert a blank icon, not find to insert nothing
	long nIndex = this->InsertItem(Item);

	// address
	//Item.SetId(nIndex);
	//Item.SetText(sValue);
	//Item.SetColumn(1);
	//this->SetItem(Item);

	// enabled
	//Item.SetText(sValue);
	//Item.SetColumn(2);
	//	this->SetItem(Item);

	UpdateLabel(nIndex);
}


void LabelSetListCtrl::UpdateLabelSet(int nIndex)
{
	wxString sValue;

	LABELSET *pLabelSet = (LABELSET *)this->GetItemData(nIndex);

	wxListItem Item;
	Item.SetMask(wxLIST_MASK_TEXT);
	Item.SetId(nIndex);

	// labelset name

	sValue = wxString((const char *)pLabelSet->m_sName, *wxConvCurrent);
	Item.SetText(sValue);
	Item.SetColumn(0);
	Item.SetImage(-1); //Insert a blank icon, not find to insert nothing
	this->SetItem(Item);

	sValue = pLabelSet->m_bActive ? sYes : sNo;
	Item.SetText(sValue);
	Item.SetColumn(1);
	this->SetItem(Item);
}

void LabelSetListCtrl::AddLabelSet(LABELSET *pLabelSet)
{
	wxString sValue = wxT("Temp");

	wxListItem Item;

	// name
	Item.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_IMAGE);
	Item.SetId(this->GetItemCount());
	Item.SetText(sValue);
	Item.SetColumn(0);
	Item.SetData(pLabelSet);
	Item.SetImage(-1); //Insert a blank icon, not find to insert nothing
	long nIndex = this->InsertItem(Item);

	// enabled
	//	Item.SetText(sValue);
	//	Item.SetColumn(1);
	//	this->SetItem(Item);

	UpdateLabelSet(nIndex);
}

void LabelsListCtrl::OnKeyDown(wxKeyEvent &event)
{
	switch (event.GetKeyCode())
	{
	default:
		break;

	case WXK_INSERT:
	{
		AddNewLabel();
	}
	return;
	case WXK_DELETE:
	{
		RemoveSelectedLabels();
	}
	return;

	}
	event.Skip();
}


void LabelSetListCtrl::OnKeyDown(wxKeyEvent &event)
{
	switch (event.GetKeyCode())
	{
	default:
		break;

	case WXK_INSERT:
	{
		AddNewLabelSet();
	}
	return;
	case WXK_DELETE:
	{
		RemoveSelectedLabelSets();
	}
	return;

	}
	event.Skip();
}

bool LabelManagerDialog::TransferDataFromWindow()
{

	return true;
}

LabelsListCtrl *LabelManagerDialog::GetLabelsListCtrl()
{
	LabelsListCtrl *pListCtrl;

	pListCtrl = XRCCTRL(*this, "m_listCtrl16", LabelsListCtrl);

	return pListCtrl;
}

LabelSetListCtrl *LabelManagerDialog::GetLabelSetListCtrl()
{
	LabelSetListCtrl *pListCtrl;

	pListCtrl = XRCCTRL(*this, "m_listCtrl17", LabelSetListCtrl);

	return pListCtrl;
}

void LabelSetListCtrl::SelectLabelSet(LABELSET *pLabelSet)
{
	long item = -1;
	for (;;)
	{
		item = this->GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);
		if (item == -1)
			break;

		this->SetItemState(item, wxLIST_STATE_SELECTED, 0);

	}

	long nSelectedItem = -1;

	for (int i = 0; i < this->GetItemCount(); i++)
	{
		if (this->GetItemData(i) == (wxUIntPtr)pLabelSet)
		{
			nSelectedItem = i;
			break;

		}
	}

	this->SetItemState(nSelectedItem, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}


void LabelSetListCtrl::RefreshLabelSets()
{
	this->Freeze();
	this->DeleteAllItems();

	// populate list ctrl
	LABELSET *pLabelSet = (LABELSET *)labelset_get_first();
	while (pLabelSet != NULL)
	{
		this->AddLabelSet(pLabelSet);

		pLabelSet = pLabelSet->m_pNext;
	}

	this->Thaw();
	this->Refresh();

	this->SortNow();

}

bool LabelManagerDialog::TransferDataToWindow()
{
	LabelSetListCtrl *pLabelSetListCtrl = GetLabelSetListCtrl();

	pLabelSetListCtrl->m_pDialog = this;
	pLabelSetListCtrl->RefreshLabelSets();


	LabelsListCtrl *pLabelsListCtrl = GetLabelsListCtrl();
	pLabelsListCtrl->m_pDialog = this;

	return true;
}









BOOL Parse_IsEnd(char *pString)
{
	char ch = pString[0];

	if ((ch == '\0') || (ch == '\r') || (ch == '\n'))
		return TRUE;
	return FALSE;
}

char *Parse_GetToken(char *pString)
{
	char ch;

	do
	{
		ch = pString[0];
		++pString;
	} while ((ch != ' ') && (ch != '\t') && (ch != '\0') && (ch != '\r') && (ch != '\n'));
	pString--;
	return pString;
}

char *Parse_ConsumeWhitespace(char *pString)
{
	char ch;

	do
	{
		ch = pString[0];
		++pString;
	} while ((ch == ' ') || (ch == '\t'));

	pString--;
	return pString;
}

void LabelSetListCtrl::OnLoadLabelsNOI(wxCommandEvent & WXUNUSED(event))
{
	wxString sLoadFilename;
	wxConfig::Get(false)->Read(wxT("windows/labelmanager/import/noi_listing_filename"), &sLoadFilename);

	wxFileName RecentPath(sLoadFilename);

	wxFileDialog openFileDialog(
		NULL,
		wxT("Import Labels from NOI label file"),      // title
		RecentPath.GetPath(),         // default dir
		RecentPath.GetFullName(),     // default file
		wxT("NOI file (*.noi)|*.noi|All files (*.*)|*.*||"),
		wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (openFileDialog.ShowModal() == wxID_OK)
	{
		wxString sFilename = openFileDialog.GetPath();
		wxConfig::Get(false)->Write(wxT("windows/labelmanager/import/noi_listing_filename"), sFilename);

		const wxCharBuffer filenameBuffer = sFilename.utf8_str();

		LABELSET *set = labelset_create("loadedlabels");

		FILE *fh = fopen(filenameBuffer.data(), "r");
		if (fh == NULL)
		{
			printf("Failed to open label file!\n");
			//          printf("Can't open file %s\n", argv[2]);
		}
		else
		{
			char value[1024];
			char label[1024];
			char buffer[1024];

			char *line = fgets(buffer, sizeof(buffer), fh);
			while (line != NULL)
			{
				line = Parse_ConsumeWhitespace(line);

				if (!Parse_IsEnd(line))
				{
					/* looking for def */
					char *sTokenEnd;
					int TokenLength;

					/* look for DEF */
					sTokenEnd = Parse_GetToken(line);

					TokenLength = sTokenEnd - line;
					if (TokenLength != 0)
					{

						/* found def? */
						if (strncmp(line, "DEF", TokenLength) == 0)
						{
							line = sTokenEnd;

							/* consume white space */
							line = Parse_ConsumeWhitespace(line);

							sTokenEnd = Parse_GetToken(line);
							TokenLength = sTokenEnd - line;


							if (TokenLength != 0)
							{

								/* store name of label */
								strncpy(label, line, TokenLength);
								label[TokenLength] = '\0';

								line = sTokenEnd;

								/* consume white space */
								line = Parse_ConsumeWhitespace(line);

								/* try and get value */
								sTokenEnd = Parse_GetToken(line);
								TokenLength = sTokenEnd - line;


								if (TokenLength != 0)
								{
									unsigned int address;
									int got_address = 0;

									/* store name of label */
									strncpy(value, line, TokenLength);
									value[TokenLength] = '\0';

									line = sTokenEnd;

									// sdcc is starting to output with 0: prefix
									// on labels in noi
									//
									// if we find a : output after otherwise keep
									char *pTok = strchr(value, ':');
									if (pTok != NULL)
									{
										if (sscanf(pTok + 1, "%x", &address) == 1)
										{
											got_address = 1;
										}
									}
									else
									{
										if (sscanf(value, "%x", &address) == 1)
										{
											got_address = 1;
										}
									}
									if (got_address)
									{
										printf("adding label: %s %02x\n", label, address);
										labelset_add_label(set, label, address);
									}
								}
							}
						}
					}
				}
				line = fgets(buffer, sizeof(buffer), fh);
			}

			fclose(fh);

			RefreshLabelSets();
			SelectLabelSet(set);

		}
	}
}


void LabelSetListCtrl::OnLoadLabelsPasmo(wxCommandEvent & WXUNUSED(event))
{
	wxString sLoadFilename = wxT("labels.lst");
	wxConfig::Get(false)->Read(wxT("windows/labelmanager/import/pasmo_listing_filename"), &sLoadFilename);

	wxFileName RecentPath(sLoadFilename);


	wxFileDialog openFileDialog(
		NULL,
		wxT("Import Labels from Pasmo listing file"),      // title
		RecentPath.GetPath(),         // default dir
		RecentPath.GetFullName(),     // default file
		wxT("All files (*.*)|*.*||"),
		wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (openFileDialog.ShowModal() == wxID_OK)
	{
		wxString sFilename = openFileDialog.GetPath();
		wxConfig::Get(false)->Write(wxT("windows/labelmanager/import/pasmo_listing_filename"), sFilename);

		const wxCharBuffer filenameBuffer = sFilename.utf8_str();

		LABELSET *set = labelset_create("loadedlabels");

		FILE *fh = fopen(filenameBuffer.data(), "r");
		if (fh == NULL)
		{
			printf("Failed to open label file!\n");
			//          printf("Can't open file %s\n", argv[2]);
		}
		else
		{
			char value[1024];
			char label[1024];
			char buffer[1024];

			char *line = fgets(buffer, sizeof(buffer), fh);
			while (line != NULL)
			{
				line = Parse_ConsumeWhitespace(line);

				if (!Parse_IsEnd(line))
				{
					if (line[0] != ';')
					{
						/* looking for label */
						char *sTokenEnd;
						int TokenLength;

						sTokenEnd = Parse_GetToken(line);

						TokenLength = sTokenEnd - line;
						if (TokenLength != 0)
						{

							/* store name of label */
							strncpy(label, line, TokenLength);
							label[TokenLength] = '\0';

							line = sTokenEnd;

							/* consume white space */
							line = Parse_ConsumeWhitespace(line);

							sTokenEnd = Parse_GetToken(line);
							TokenLength = sTokenEnd - line;

							if (TokenLength != 0)
							{

								/* found def? */
								if ((strncmp(line, "equ", TokenLength) == 0) || (strncmp(line, "EQU", TokenLength) == 0))
								{
									unsigned int address;
									line = sTokenEnd;

									/* consume white space */
									line = Parse_ConsumeWhitespace(line);

									/* try and get value */
									sTokenEnd = Parse_GetToken(line);
									TokenLength = sTokenEnd - line;


									if (TokenLength != 0)
									{
										/* store name of label */
										strncpy(value, line, TokenLength);
										value[TokenLength] = '\0';
										int got_address = 0;

										if ((value[0] == '&') || (value[0] == '#'))
										{
											if (sscanf(value + 1, "%x", &address) == 1)
											{
												got_address = 1;
											}
										}
										else if (
											(value[strlen(value) - 1] == 'h') ||
											(value[strlen(value) - 1] == 'H')
											)
										{
											if (sscanf(value, "%x", &address) == 1)
											{
												got_address = 1;
											}
										}
										else
										{
											if (sscanf(value, "%x", &address) == 1)
											{
												got_address = 1;
											}
										}
										if (got_address)
										{
											printf("adding label: %s %02x\n", label, address);
											labelset_add_label(set, label, address);
										}
									}
								}
							}
						}

					}
				}
				line = fgets(buffer, sizeof(buffer), fh);
			}


			fclose(fh);

			RefreshLabelSets();
			SelectLabelSet(set);
		}
	}
}

void LabelSetListCtrl::OnLoadLabelsWinape(wxCommandEvent & WXUNUSED(event))
{
	wxString sLoadFilename = wxT("labels.lst");
	wxConfig::Get(false)->Read(wxT("windows/labelmanager/import/winape_listing_filename"), &sLoadFilename);

	wxFileName RecentPath(sLoadFilename);


	wxFileDialog openFileDialog(
		NULL,
		wxT("Import Labels from Winape assembler symbol file"),      // title
		RecentPath.GetPath(),         // default dir
		RecentPath.GetFullName(),     // default file
		wxT("All files (*.*)|*.*||"),
		wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (openFileDialog.ShowModal() == wxID_OK)
	{
		wxString sFilename = openFileDialog.GetPath();
		wxConfig::Get(false)->Write(wxT("windows/labelmanager/import/winape_listing_filename"), sFilename);

		const wxCharBuffer filenameBuffer = sFilename.utf8_str();

		LABELSET *set = labelset_create("loadedlabels");

		FILE *fh = fopen(filenameBuffer.data(), "r");
		if (fh == NULL)
		{
			printf("Failed to open label file!\n");
			//          printf("Can't open file %s\n", argv[2]);
		}
		else
		{
			char value[1024];
			char label[1024];
			char buffer[1024];

			char *line = fgets(buffer, sizeof(buffer), fh);
			while (line != NULL)
			{
				line = Parse_ConsumeWhitespace(line);

				if (!Parse_IsEnd(line))
				{
					if (line[0] != ';')
					{
						/* looking for label */
						char *sTokenEnd;
						int TokenLength;

						sTokenEnd = Parse_GetToken(line);

						TokenLength = sTokenEnd - line;
						if (TokenLength != 0)
						{
							/* store name of label */
							strncpy(label, line, TokenLength);
							label[TokenLength] = '\0';

							line = sTokenEnd;

							/* consume white space */
							line = Parse_ConsumeWhitespace(line);

							sTokenEnd = Parse_GetToken(line);
							TokenLength = sTokenEnd - line;

							if (TokenLength != 0)
							{
								/* store name of label */
								strncpy(value, line, TokenLength);
								value[TokenLength] = '\0';
								int got_address = 0;
								int address = 0;

								if (value[0] == '#')
								{
									if (sscanf(value + 1, "%x", &address) == 1)
									{
										got_address = 1;
									}
								}

								if (got_address)
								{
									printf("adding label: %s %02x\n", label, address);
									labelset_add_label(set, label, address);
								}
							}
						}

					}
				}
				line = fgets(buffer, sizeof(buffer), fh);
			}


			fclose(fh);

			RefreshLabelSets();
			SelectLabelSet(set);
		}
	}
}


void LabelSetListCtrl::OnLoadLabelsRasm(wxCommandEvent & WXUNUSED(event))
{
	wxString sLoadFilename = wxT("labels.lst");
	wxConfig::Get(false)->Read(wxT("windows/labelmanager/import/rasm_listing_filename"), &sLoadFilename);

	wxFileName RecentPath(sLoadFilename);


	wxFileDialog openFileDialog(
		NULL,
		wxT("Import Labels from Rasm assembler symbol file"),      // title
		RecentPath.GetPath(),         // default dir
		RecentPath.GetFullName(),     // default file
		wxT("All files (*.*)|*.*||"),
		wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (openFileDialog.ShowModal() == wxID_OK)
	{
		wxString sFilename = openFileDialog.GetPath();
		wxConfig::Get(false)->Write(wxT("windows/labelmanager/import/rasm_listing_filename"), sFilename);

		const wxCharBuffer filenameBuffer = sFilename.utf8_str();

		LABELSET *set = labelset_create("loadedlabels");

		FILE *fh = fopen(filenameBuffer.data(), "r");
		if (fh == NULL)
		{
			printf("Failed to open label file!\n");
			//          printf("Can't open file %s\n", argv[2]);
		}
		else
		{
			char value[1024];
			char label[1024];
			char buffer[1024];

			char *line = fgets(buffer, sizeof(buffer), fh);
			while (line != NULL)
			{
				line = Parse_ConsumeWhitespace(line);

				if (!Parse_IsEnd(line))
				{
					if (line[0] != ';')
					{
						/* looking for label */
						char *sTokenEnd;
						int TokenLength;

						sTokenEnd = Parse_GetToken(line);

						TokenLength = sTokenEnd - line;
						if (TokenLength != 0)
						{

							/* store name of label */
							strncpy(label, line, TokenLength);
							label[TokenLength] = '\0';

							line = sTokenEnd;

							/* consume white space */
							line = Parse_ConsumeWhitespace(line);

							sTokenEnd = Parse_GetToken(line);
							TokenLength = sTokenEnd - line;

							if (TokenLength != 0)
							{
								/* store name of label */
								strncpy(value, line, TokenLength);
								value[TokenLength] = '\0';
								int address = 0;

								if (value[0] == '#')
								{
									if (sscanf(value + 1, "%x", &address) == 1)
									{
										/* consume white space */
										line = Parse_ConsumeWhitespace(line);

										sTokenEnd = Parse_GetToken(line);
										TokenLength = sTokenEnd - line;

										if (TokenLength != 0)
										{
											printf("adding label: %s %02x\n", label, address);
											labelset_add_label(set, label, address);
										}
									}
								}
							}
						}

					}
				}
				line = fgets(buffer, sizeof(buffer), fh);
			}


			fclose(fh);

			RefreshLabelSets();
			SelectLabelSet(set);
		}
	}
}

void LabelSetListCtrl::OnLoadLabelsSjasm(wxCommandEvent & WXUNUSED(event))
{
	wxString sLoadFilename = wxT("labels.sym");
	wxConfig::Get(false)->Read(wxT("windows/labelmanager/import/sjasm_listing_filename"), &sLoadFilename);

	wxFileName RecentPath(sLoadFilename);


	wxFileDialog openFileDialog(
		NULL,
		wxT("Import Labels from SjAsm symbol file"),      // title
		RecentPath.GetPath(),         // default dir
		RecentPath.GetFullName(),     // default file
		wxT("SjAsm Sym files (*.sym)|*.sym|All files (*.*)|*.*||"),
		wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (openFileDialog.ShowModal() == wxID_OK)
	{
		wxString sFilename = openFileDialog.GetPath();
		wxConfig::Get(false)->Write(wxT("windows/labelmanager/import/sjasm_listing_filename"), sFilename);

		const wxCharBuffer filenameBuffer = sFilename.utf8_str();

		LABELSET *set = labelset_create("loadedlabels");

		FILE *fh = fopen(filenameBuffer.data(), "r");
		if (fh == NULL)
		{
			printf("Failed to open label file!\n");
			//          printf("Can't open file %s\n", argv[2]);
		}
		else
		{
			char value[1024];
			char label[1024];
			char buffer[1024];

			char *line = fgets(buffer, sizeof(buffer), fh);
			while (line != NULL)
			{
				line = Parse_ConsumeWhitespace(line);

				if (!Parse_IsEnd(line))
				{
					/* <page in hex>:<address in hex> space <label> */

					/* looking for label */
					char *sTokenEnd;
					int TokenLength;

					sTokenEnd = Parse_GetToken(line);

					TokenLength = sTokenEnd - line;
					if (TokenLength != 0)
					{
						/* store name of value */
						strncpy(value, line, TokenLength);
						value[TokenLength] = '\0';

						line = sTokenEnd;

						char *sBankOffsetSeperator = strchr(value, ':');
						if (sBankOffsetSeperator)
						{
							unsigned int Bank = 0;
							unsigned int Offset = 0;
							unsigned int address = 0;
							int got_bank = 0;
							int got_offset = 0;

							*sBankOffsetSeperator = '\0';

							if (sscanf(value, "%x", &Bank) == 1)
							{
								got_bank = 1;
							}

							if (sscanf(sBankOffsetSeperator + 1, "%x", &Offset) == 1)
							{
								got_offset = 1;
							}

							if (got_bank && got_offset)
							{
								address = (Bank << 14) | Offset;

								/* consume white space */
								line = Parse_ConsumeWhitespace(line);

								sTokenEnd = Parse_GetToken(line);
								TokenLength = sTokenEnd - line;

								if (TokenLength != 0)
								{
									/* store name of label */
									strncpy(label, line, TokenLength);
									label[TokenLength] = '\0';

									printf("adding label: %s %02x\n", label, address);
									labelset_add_label(set, label, address);
								}
							}
						}

					}
				}
				line = fgets(buffer, sizeof(buffer), fh);
			}


			fclose(fh);

			RefreshLabelSets();
			SelectLabelSet(set);
		}
	}
}


void LabelSetListCtrl::OnSaveLabelsPasmo(wxCommandEvent & WXUNUSED(event))
{
	wxString sSaveName = wxT("labels.lst");
	wxConfig::Get(false)->Read(wxT("windows/labelmanager/export/pasmo_listing_filename"), &sSaveName);

	wxFileName RecentPath(sSaveName);

	wxFileDialog openFileDialog(
		NULL,
		wxT("Export Labels to Pasmo listing file"),      // title
		RecentPath.GetPath(),         // default dir
		RecentPath.GetFullName(),     // default file
		wxT("All files (*.*)|*.*||"),
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if (openFileDialog.ShowModal() == wxID_OK)
	{
		wxString sFilename = openFileDialog.GetPath();
		const wxCharBuffer filenameBuffer = sFilename.utf8_str();

		wxConfig::Get(false)->Write(wxT("windows/labelmanager/export/pasmo_listing_filename"), sFilename);

		FILE *fh = fopen(filenameBuffer.data(), "w");
		if (fh != NULL)
		{

			LABEL *pLabel = m_pSelectedLabelSet->m_pLabels;
			while (pLabel != NULL)
			{
				fprintf(fh, "%s equ &%04x\n", pLabel->m_sName, pLabel->m_Address);

				pLabel = pLabel->m_pNext;
			}


			fclose(fh);
		}
	}
}


NewLabelDialog::NewLabelDialog(wxWindow *pParent)
{
	m_nAddress = 0;

	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_DIALOG_NEW_LABEL"));
}


BEGIN_EVENT_TABLE(NewLabelDialog, wxDialog)
END_EVENT_TABLE()


bool NewLabelDialog::TransferDataFromWindow()
{
	wxTextCtrl *pTextCtrl;
	wxString sValue;

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_HEX_VALUE", wxTextCtrl);
	sValue = pTextCtrl->GetValue();

	m_nAddress = wxGetApp().ExpressionEvaluate(sValue);

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_LABEL_NAME", wxTextCtrl);
	m_sName = pTextCtrl->GetValue();

	return true;
}


bool NewLabelDialog::TransferDataToWindow()
{
	return true;
}
