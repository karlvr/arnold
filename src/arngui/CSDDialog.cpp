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
#include "CSDDialog.h"
#include <wx/xrc/xmlres.h>
#include "arnguiApp.h"
#include <wx/menu.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
extern "C"
{
#include "../cpc/jukebox.h"
}

static const wxChar *sYes = wxT("Yes");
static const wxChar *sNo = wxT("No");


WX_DEFINE_ARRAY_INT(int, ArrayOfInts);

BEGIN_EVENT_TABLE(CSDDialog, wxDialog)
EVT_CONTEXT_MENU(CSDDialog::OnContextMenu)
EVT_COMMAND(XRCID("m_ExpansionROMEnable"), wxEVT_COMMAND_MENU_SELECTED, CSDDialog::OnCartridgeEnable)
EVT_COMMAND(XRCID("m_ExpansionROMLoad"), wxEVT_COMMAND_MENU_SELECTED, CSDDialog::OnCartridgeLoad)
EVT_COMMAND(XRCID("m_ExpansionROMRemove"), wxEVT_COMMAND_MENU_SELECTED, CSDDialog::OnCartridgeRemove)
EVT_COMMAND(XRCID("m_ExpansionROMPickBuiltin"), wxEVT_COMMAND_MENU_SELECTED, CSDDialog::OnCartridgePickBuiltin)
END_EVENT_TABLE()

void CSDDialog::OnCartridgeEnable(wxCommandEvent & WXUNUSED(event))
{
	wxListCtrl *pListCtrl = GetListCtrl();
	long item = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	while (item != -1)
	{

		Jukebox_SetCartridgeEnable(item, !Jukebox_IsCartridgeEnabled(item));

		wxString sKey;
		sKey.sprintf(wxT("csd/cartridge%d/enabled"), item);
		wxConfig::Get(false)->Write(sKey, Jukebox_IsCartridgeEnabled(item) ? true : false);
		printf("CSD cartridge %d: %s\n", (int)item, Jukebox_IsCartridgeEnabled(item) ? "enabled" : "disabled");

		item = pListCtrl->GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);

	}

	PerformUpdate();
}

void CSDDialog::OnCartridgeRemove(wxCommandEvent & WXUNUSED(event))
{
	wxListCtrl *pListCtrl = GetListCtrl();

	ArrayOfInts Array;

	long item = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	while (item != -1)
	{
		Array.Add(item);

		item = pListCtrl->GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);
	}

	for (int i = Array.GetCount() - 1; i >= 0; i--)
	{
		Jukebox_CartridgeRemove(Array[i]);

		wxString sKey;
		sKey.sprintf(wxT("csd/cartridge%d/enabled"), Array[i]);
		wxConfig::Get(false)->Write(sKey, Jukebox_IsCartridgeEnabled(Array[i]) ? true : false);
		printf("CSD cartridge %d: %s\n", (int)Array[i], Jukebox_IsCartridgeEnabled(Array[i]) ? "enabled" : "disabled");

		sKey.sprintf(wxT("csd/cartridge%d/filename"), Array[i]);
		wxConfig::Get(false)->Write(sKey, wxEmptyString);

	}

	PerformUpdate();
}


void CSDDialog::OnCartridgeLoad(wxCommandEvent & WXUNUSED(event))
{
	wxListCtrl *pListCtrl = GetListCtrl();
	long item = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item == -1)
	{
		return;
	}

	wxString sFilename;
	wxString sTitleSuffix;
	if (item == 0)
	{
		sTitleSuffix = wxT("CSD System Cartridge");
	}
	else
	{
		sTitleSuffix.sprintf(wxT("CSD Slot %d"), (int)item);
	}

	if (m_CartridgeFileType.Open(this, sFilename, sTitleSuffix))
	{
		unsigned char *pCartridgeData;
		unsigned long CartridgeLength;

		/* try to load it */
		wxGetApp().LoadLocalFile(sFilename, &pCartridgeData, &CartridgeLength);

		if (pCartridgeData != NULL)
		{
			// load it
			Jukebox_CartridgeInsert(item, pCartridgeData, CartridgeLength);

			// enable this cart
			Jukebox_SetCartridgeEnable(item, TRUE);

			wxString sKey;
			sKey.sprintf(wxT("csd/cartridge%d/enabled"), item);
			wxConfig::Get(false)->Write(sKey, Jukebox_IsCartridgeEnabled(item) ? true : false);
			printf("CSD cartridge %d: %s\n", (int)item, Jukebox_IsCartridgeEnabled(item) ? "enabled" : "disabled");

			sKey.sprintf(wxT("csd/cartridge%d/filename"), item);
			wxConfig::Get(false)->Write(sKey, sFilename);

			free(pCartridgeData);
		}
	}
	PerformUpdate();
}


void CSDDialog::OnCartridgePickBuiltin(wxCommandEvent & WXUNUSED(event))
{
	wxListCtrl *pListCtrl = GetListCtrl();
	long item = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item == -1)
	{
		return;
	}

	wxString sROMName;
	if (item == 0)
	{
		sROMName = wxT("CSD System Cartridge");
	}
	else
	{
		sROMName.sprintf(wxT("CSD Cartridge Slot %d"), (int)item);
	}

	wxString sID;
	unsigned char *pData = NULL;
	unsigned long nLength = 0;
	if (wxGetApp().PickBuiltInMedia(BUILTIN_MEDIA_CART, sROMName, sID, &pData, &nLength))
	{
		// load it
		Jukebox_CartridgeInsert(item, pData, nLength);

		// enable this cart
		Jukebox_SetCartridgeEnable(item, TRUE);

		wxString sKey;
		sKey.sprintf(wxT("csd/cartridge%d/enabled"), item);
		wxConfig::Get(false)->Write(sKey, Jukebox_IsCartridgeEnabled(item) ? true : false);
		printf("CSD cartridge %d: %s\n", (int)item, Jukebox_IsCartridgeEnabled(item) ? "enabled" : "disabled");

		sKey.sprintf(wxT("csd/cartridge%d/filename"), item);
		wxConfig::Get(false)->Write(sKey, sID);

	}

	PerformUpdate();

}



void CSDDialog::OnContextMenu(wxContextMenuEvent &event)
{
	wxListCtrl *pListCtrl = GetListCtrl();
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

	wxMenuBar *pMenuBar = wxXmlResource::Get()->LoadMenuBar(wxT("MB_MENU_EXPANSIONROMS_MENU_BAR"));
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

void CSDDialog::InsertItem(const wxString &sID, int nIndex)
{
	wxListCtrl *pListCtrl = GetListCtrl();
	int nID = pListCtrl->GetItemCount();
	wxListItem Item;

	// id
	Item.SetMask(wxLIST_MASK_TEXT);
	Item.SetId(nID);
	Item.SetText(sID);
	Item.SetColumn(0);
	Item.SetImage(-1); //Insert a blank icon, not find to insert nothing
	int ItemID = pListCtrl->InsertItem(Item);

	Item.SetId(ItemID);

	wxString sValue;
	// enabled
	if (Jukebox_IsCartridgeEnabled(nIndex))
	{
		sValue = sYes;
	}
	else
	{
		sValue = sNo;
	}

	Item.SetText(sValue);
	Item.SetColumn(1);
	pListCtrl->SetItem(Item);

	wxString sKey;
	sKey.sprintf(wxT("csd/cartridge%d/filename"), nIndex);
	wxConfig::Get(false)->Read(sKey, &sValue, wxEmptyString);

	// filename
	Item.SetText(sValue);
	Item.SetColumn(2);
	pListCtrl->SetItem(Item);
}

void CSDDialog::PerformUpdate()
{
	wxListCtrl *pListCtrl = GetListCtrl();

	pListCtrl->Freeze();
	pListCtrl->DeleteAllItems();

	InsertItem(wxT("CSD System Cartridge"), 0);

	for (int i = 1; i < 13; i++)
	{
		wxString sID;
		sID.Printf(wxT("CSD Cartridge %d"), i);

		InsertItem(sID, i);

	}
	pListCtrl->Thaw();
}


bool CSDDialog::TransferDataFromWindow()
{
	wxCheckBox *pCheckBox;
	wxChoice *pChoice;

	pCheckBox = XRCCTRL(*this, "m_CSDTimer", wxCheckBox);
	Jukebox_SetTimerEnabled(pCheckBox->GetValue());

	wxConfig::Get(false)->Write(wxT("csd/auto_timer_enabled"), Jukebox_IsTimerEnabled());


	pChoice = XRCCTRL(*this, "m_TimerChoice", wxChoice);
	Jukebox_SetTimerSelection(pChoice->GetSelection());

	wxConfig::Get(false)->Write(wxT("csd/auto_timer_selection"), Jukebox_GetTimerSelection());

	for (int i = pChoice->GetCount() - 1; i >= 0; i--)
	{
		pChoice->Delete(i);
	}

	pChoice = XRCCTRL(*this, "m_CartridgeChoice", wxChoice);
	Jukebox_SetManualTimerSelection(pChoice->GetSelection());

	wxConfig::Get(false)->Write(wxT("csd/manual_timer_selection"), Jukebox_GetManualTimerSelection());

	for (int i = pChoice->GetCount() - 1; i >= 0; i--)
	{
		pChoice->Delete(i);
	}


	return true;

}
bool CSDDialog::TransferDataToWindow()
{
	wxListCtrl *pListCtrl = GetListCtrl();
	wxCheckBox *pCheckBox;
	wxChoice *pChoice;

	pCheckBox = XRCCTRL(*this, "m_CSDTimer", wxCheckBox);
	pCheckBox->SetValue(Jukebox_IsTimerEnabled() ? true : false);

	pChoice = XRCCTRL(*this, "m_TimerChoice", wxChoice);
	pChoice->SetSelection(Jukebox_GetTimerSelection());

	pChoice = XRCCTRL(*this, "m_CartridgeChoice", wxChoice);
	pChoice->SetSelection(Jukebox_GetManualTimerSelection());


	pListCtrl->Freeze();
	pListCtrl->ClearAll();

	wxListItem Column;
	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT("Id"));
	pListCtrl->InsertColumn(0, Column);
	Column.SetText(wxT("Active"));
	pListCtrl->InsertColumn(1, Column);
	Column.SetText(wxT("Filename"));
	pListCtrl->InsertColumn(2, Column);

	pListCtrl->Thaw();
	PerformUpdate();
	wxGetApp().ReadConfigWindowPosAndSize(wxT("windows/csd_cartridges/"), this);

	wxGetApp().SetColumnSize(pListCtrl, 0);
	wxGetApp().SetColumnSize(pListCtrl, 1);
	wxGetApp().SetColumnSize(pListCtrl, 2);

	wxGetApp().ReadConfigListCtrl(wxT("windows/csd_cartridges/listctrl/"), GetListCtrl());
	wxGetApp().EnsureWindowVisible(this);


	return true;
}

wxListCtrl *CSDDialog::GetListCtrl()
{
	return XRCCTRL(*this, "m_listCtrl7", wxListCtrl);
}

CSDDialog::CSDDialog(wxWindow *pParent) : m_CartridgeFileType(_T("Open Cartridge"), _T("Save Cartridge"))
{
	wxXmlResource::Get()->LoadDialog( this, pParent, wxT( "DLG_DIALOG_CSD" ) );


	// file
	FileFilter CPRFilter = { wxT("Cartridge Image"), wxT("cpr") };
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	FileFilter CPRFilterAlt= {wxT("Cartridge Image"), wxT("CPR")};
#endif
	FileFilter BinaryFilter = { wxT("Amstrad Binary"), wxT("bin") };
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	// case sensitive filesystems
	FileFilter BinaryFilterAlt = { wxT("Binary"), wxT("BIN") };
#endif
	// zip
	FileFilter ZipFilter = {wxT( "ZIP Archive" ), wxT( "zip" )};
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	FileFilter ZipFilterAlt = { wxT("ZIP Archive"), wxT("ZIP") };
#endif
	FileFilter CRTFilter = {wxT( "Cartridge Image" ), wxT( "crt" )};
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	FileFilter CRTFilterAlt = { wxT("Cartridge Image"), wxT("CRT") };
#endif

	m_CartridgeFileType.AddReadFilter(CPRFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	m_CartridgeFileType.AddReadFilter(CPRFilterAlt);
#endif
	m_CartridgeFileType.AddReadFilter(CRTFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	m_CartridgeFileType.AddReadFilter(CRTFilterAlt);
#endif
	m_CartridgeFileType.AddReadFilter(BinaryFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	m_CartridgeFileType.AddReadFilter(BinaryFilterAlt);
#endif
	m_CartridgeFileType.AddReadFilter(ZipFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	m_CartridgeFileType.AddReadFilter(ZipFilterAlt);
#endif
}

