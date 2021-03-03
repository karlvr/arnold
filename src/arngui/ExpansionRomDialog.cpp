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

#include "ExpansionRomDialog.h"
#include <wx/xrc/xmlres.h>
#include "arnguiApp.h"
#include <wx/choicdlg.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include "arnguiMain.h"

extern "C"
{
#include "../cpc/cpc.h"

}



ExpansionRomsListCtrl::ExpansionRomsListCtrl() : SortableListCtrl()
{
}

ExpansionRomsListCtrl::~ExpansionRomsListCtrl()
{
}

IMPLEMENT_DYNAMIC_CLASS(ExpansionRomsListCtrl, SortableListCtrl)

BEGIN_EVENT_TABLE(ExpansionRomsListCtrl, SortableListCtrl)
END_EVENT_TABLE()

// need seperate slots for on-board upper/lower rom.
// reason; upper rom takes priority when no rom is asserted by an expansion board.


const int nExpansionRoms = 256;


static const wxChar *sYes = wxT("Yes");
static const wxChar *sNo = wxT("No");

WX_DEFINE_ARRAY_INT(int, ArrayOfInts);

BEGIN_EVENT_TABLE(ExpansionRomsDialog, wxDialog)
EVT_CONTEXT_MENU(ExpansionRomsDialog::OnContextMenu)
EVT_COMMAND(XRCID("m_ExpansionROMEnable"), wxEVT_COMMAND_MENU_SELECTED, ExpansionRomsDialog::OnROMEnable)
EVT_COMMAND(XRCID("m_ExpansionROMRemove"), wxEVT_COMMAND_MENU_SELECTED, ExpansionRomsDialog::OnROMRemove)
EVT_COMMAND(XRCID("m_ExpansionROMLoad"), wxEVT_COMMAND_MENU_SELECTED, ExpansionRomsDialog::OnROMLoad)
EVT_COMMAND(XRCID("m_ExpansionROMPickBuiltin"), wxEVT_COMMAND_MENU_SELECTED, ExpansionRomsDialog::OnROMPickBuiltin)
EVT_CHAR_HOOK(ExpansionRomsDialog::OnCharHook)
END_EVENT_TABLE()

void ExpansionRomsDialog::OnCharHook(wxKeyEvent& event)
{
	if (IsEscapeKey(event))
	{
		Close();
		return;
	}

	event.Skip();
}


ExpansionRomsDialog::~ExpansionRomsDialog()
{
	wxListCtrl *pListCtrl = GetListCtrl();
	pListCtrl->DeleteAllItems();
}


void ExpansionRomsDialog::OnROMEnable(wxCommandEvent & WXUNUSED(event))
{
	wxListCtrl *pListCtrl = GetListCtrl();

	ArrayOfInts Array;
	long item = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	while (item != -1)
	{

		Array.Add(pListCtrl->GetItemData(item));

		item = pListCtrl->GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);
	}
	wxString sDeviceName(EmuDevice_GetSaveName(m_nDevice), wxConvUTF8);

	for (int i = Array.GetCount() - 1; i >= 0; i--)
	{
		int RomIndex = Array[i];
		ExpansionRom_SetActiveState(m_pData, RomIndex, !ExpansionRom_IsActive(m_pData, RomIndex));

		wxString sConfigPrefix;
		sConfigPrefix.Printf(wxT("devices/%s/ExpansionRoms/ExpansionRom%d/"), sDeviceName.c_str(), RomIndex);

		wxConfig::Get(false)->Write(sConfigPrefix + wxT("enabled"), ExpansionRom_IsActive(m_pData, RomIndex));
	}

	PerformUpdate();
}


void ExpansionRomsDialog::OnROMRemove(wxCommandEvent & WXUNUSED(event))
{
	wxListCtrl *pListCtrl = GetListCtrl();


	ArrayOfInts Array;

	long item = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	while (item != -1)
	{
		Array.Add(pListCtrl->GetItemData(item));

		item = pListCtrl->GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);
	}

	wxString sDeviceName(EmuDevice_GetSaveName(m_nDevice), wxConvUTF8);


	for (int i = Array.GetCount() - 1; i >= 0; i--)
	{
		int RomIndex = Array[i];

		ExpansionRom_Remove(m_pData, RomIndex);

		wxString sKey;
		sKey.sprintf(wxT("devices/%s/ExpansionRoms/ExpansionRom%d/filename"), sDeviceName.c_str(), RomIndex);
		wxConfig::Get(false)->Write(sKey, wxEmptyString);
	}

	PerformUpdate();
}

bool ExpansionRomsDialog::WarnROMLoad(long RomIndex)
{
	int answer = wxOK;
	if (RomIndex == 0)
	{
		answer = wxMessageBox(wxT("You are overriding the Amstrad BASIC ROM. BASIC will not be available."),
			wxGetApp().GetAppName(), wxICON_WARNING | wxOK | wxCANCEL);
	}

	if (RomIndex == 7)
	{
		answer = wxMessageBox(wxT("You are potentially overriding the Amstrad disc ROM, disc operations may not be available!"),
			wxGetApp().GetAppName(), wxICON_WARNING | wxOK | wxCANCEL);
	}

	if (!Firmware_Detect16Roms())
	{
		answer = wxMessageBox(wxT("Some versions of the OS/firmware can't see ROMs in slots 8 or above!"),
			wxGetApp().GetAppName(), wxICON_WARNING | wxOK | wxCANCEL);
	}

	if (RomIndex >= 16)
	{
		answer = wxMessageBox(wxT("CPC6128/KCC/Aleste/Plus OS can't see ROMs in slots 16 or above!"),
			wxGetApp().GetAppName(), wxICON_WARNING | wxOK | wxCANCEL);
	}

	return (answer == wxOK);
}

void ExpansionRomsDialog::VerifyRomData(const unsigned char *pRomData, unsigned long RomLength, unsigned long nSize)
{
	const unsigned char *pData = pRomData;
	unsigned long DataLength = RomLength;

	AMSDOS_GetUseableSize(&pData, &DataLength);

	if (DataLength<nSize)
	{
		wxString sMessage;
		sMessage.sprintf(wxT("Loaded file has size %d.\nRequired size is %d.\nFile doesn't have enough data. Rom may not work correctly"), RomLength, nSize);

		wxMessageBox(sMessage);
	}
	else if (DataLength>nSize)
	{
		wxString sMessage;
		sMessage.sprintf(wxT("Loaded file has size %d.\nRequired size is %d.\nFile has too much data. Rom may not work correctly"), RomLength, nSize);

		wxMessageBox(sMessage);

	}
}


void ExpansionRomsDialog::OnROMLoad(wxCommandEvent & WXUNUSED(event))
{
	wxListCtrl *pListCtrl = GetListCtrl();
	long item = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item == -1)
	{
		return;
	}
	int nRom = pListCtrl->GetItemData(item);

	if (!WarnROMLoad(nRom))
		return;

	// TODO: Fix.
	arnguiFrame * frame = wxDynamicCast(wxGetApp().GetTopWindow(),
		arnguiFrame);

	unsigned char *pRomData = NULL;
	unsigned long RomDataLength = 0;

	wxString sTitleSuffix;
	sTitleSuffix.sprintf(wxT("Expansion/Upper ROM %d"), nRom);
	wxString sFilename;
	if (frame->OpenROM(&pRomData, &RomDataLength, sFilename, sTitleSuffix))
	{
		if (pRomData != NULL)
		{
			VerifyRomData(pRomData, RomDataLength, 16384);

			int nResult = ExpansionRom_SetRomData(m_pData, pRomData, RomDataLength, nRom);

			if (nResult == ARNOLD_STATUS_OK)
			{
				wxString sDeviceName(EmuDevice_GetSaveName(m_nDevice), wxConvUTF8);

				wxString sKey;
				sKey.sprintf(wxT("devices/%s/ExpansionRoms/ExpansionRom%d/filename"), sDeviceName.c_str(), nRom);
				wxConfig::Get(false)->Write(sKey, sFilename);


				wxMessageBox(wxT("You may need to restart the computer to use the ROM"),
					wxGetApp().GetAppName(), wxICON_WARNING | wxOK);
			}
			else
			{
				wxMessageBox(wxT("Failed to open ROM"),
					wxGetApp().GetAppName(), wxICON_WARNING | wxOK);
			}

			free(pRomData);
		}

	}
	PerformUpdate();
}

void ExpansionRomsDialog::OnROMPickBuiltin(wxCommandEvent & WXUNUSED(event))
{
	wxListCtrl *pListCtrl = GetListCtrl();
	long item = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item == -1)
	{
		return;
	}

	int nRom = pListCtrl->GetItemData(item);

	if (!WarnROMLoad(nRom))
		return;

	unsigned char *pData = NULL;
	unsigned long nLength = 0;
	wxString sROMName;
	wxString sID;
	sROMName.sprintf(wxT("Expansion/Upper ROM %d"), nRom);
	if (wxGetApp().PickBuiltInMedia(BUILTIN_MEDIA_ROM, sROMName, sID, &pData, &nLength))
	{
		int nResult = ExpansionRom_SetRomData(m_pData, pData, nLength, nRom);

		if (nResult == ARNOLD_STATUS_OK)
		{
			wxString sDeviceName(EmuDevice_GetSaveName(m_nDevice), wxConvUTF8);

			wxString sKey;
			sKey.sprintf(wxT("devices/%s/ExpansionRoms/ExpansionRom%d/filename"), sDeviceName.c_str(), nRom);
			wxConfig::Get(false)->Write(sKey, sID);

			wxMessageBox(wxT("You may need to restart the computer to use the ROM"),
				wxGetApp().GetAppName(), wxICON_WARNING | wxOK);
		}
	}

	PerformUpdate();

}




void ExpansionRomsDialog::OnContextMenu(wxContextMenuEvent &event)
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

void ExpansionRomsDialog::PerformUpdate()
{
	wxListCtrl *pListCtrl = GetListCtrl();

	pListCtrl->Freeze();
	pListCtrl->DeleteAllItems();
	int nItem = 0;

	wxString sName(EmuDevice_GetSaveName(m_nDevice), wxConvUTF8);

	for (int i = 0; i < nExpansionRoms; i++)
	{
		wxListItem Item;
		char *pName;
		wxString sValue;

		if (!ExpansionRom_IsAvailable(m_pData, i))
			continue;

		Item.SetMask(wxLIST_MASK_TEXT);
		Item.SetId(nItem);
		sValue.Printf(wxT("Expansion/Upper ROM %d"), i);
		Item.SetText(sValue);
		Item.SetColumn(0);
		Item.SetImage(-1); //Insert a blank icon, not find to insert nothing
		Item.SetData(i);			// this is the actual rom index
		int nIndex = pListCtrl->InsertItem(Item);
		++nItem;

		// initial
		sValue = wxEmptyString;
		if (ExpansionRom_Get(m_pData, i) != NULL)
		{
			bool bGotName = false;

			// get name reported by rom
			sValue = wxEmptyString;
			if (ExpansionRom_GetRomName(m_pData, i, &pName))
			{
				if ((pName != NULL) && (strlen(pName) != 0))
				{
					sValue = wxString((const char *)pName, *wxConvCurrent);
					bGotName = true;
					free(pName);
				}
			}

			if (!bGotName)
			{
				sValue = wxT("Unknown");
			}
		}

		Item.SetId(nIndex);
		Item.SetText(sValue);
		Item.SetColumn(1);
		pListCtrl->SetItem(Item);

		wxString sKey;
		sKey.sprintf(wxT("devices/%s/ExpansionRoms/ExpansionRom%d/filename"), sName.c_str(), i);
		wxConfig::Get(false)->Read(sKey, &sValue, wxEmptyString);

		Item.SetText(sValue);
		Item.SetColumn(2);
		pListCtrl->SetItem(Item);

		if (ExpansionRom_IsActive(m_pData, i))
		{
			sValue = sYes;
		}
		else
		{
			sValue = sNo;
		}
		Item.SetText(sValue);
		Item.SetColumn(3);
		pListCtrl->SetItem(Item);
	}
	pListCtrl->Thaw();
}

bool ExpansionRomsDialog::TransferDataToWindow()
{
	wxListCtrl *pListCtrl = GetListCtrl();

	wxString sName(EmuDevice_GetName(m_nDevice), wxConvUTF8);
	wxString sTitle = wxT("Expansion Roms - ");
	sTitle += sName;
	SetTitle(sTitle);

	pListCtrl->Freeze();
	pListCtrl->ClearAll();

	wxListItem Column;
	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT("Id"));
	pListCtrl->InsertColumn(0, Column);
	Column.SetText(wxT("Name"));
	pListCtrl->InsertColumn(1, Column);
	Column.SetText(wxT("Filename"));
	pListCtrl->InsertColumn(2, Column);
	Column.SetText(wxT("Active"));
	pListCtrl->InsertColumn(3, Column);

	pListCtrl->Thaw();
	PerformUpdate();
	wxGetApp().ReadConfigWindowPosAndSize(wxT("windows/expansion_roms/"), this);
	wxGetApp().EnsureWindowVisible(this);


	wxGetApp().SetColumnSize(pListCtrl, 0);
	wxGetApp().SetColumnSize(pListCtrl, 1);
	wxGetApp().SetColumnSize(pListCtrl, 2);
	wxGetApp().SetColumnSize(pListCtrl, 3);



	wxGetApp().ReadConfigListCtrl(wxT("windows/expansion_roms/listctrl/"), GetListCtrl());
	wxGetApp().EnsureWindowVisible(this);


	return true;
}

wxListCtrl *ExpansionRomsDialog::GetListCtrl()
{
	return XRCCTRL(*this, "m_listCtrl6", wxListCtrl);
}

ExpansionRomsDialog::ExpansionRomsDialog(wxWindow *pParent, int nDevice)
{
	m_nDevice = nDevice;
	m_pData = EmuDevice_GetExpansionRomData(nDevice);
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_DIALOG_EXPANSION_ROMS"));

}

