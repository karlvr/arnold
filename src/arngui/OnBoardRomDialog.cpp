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
#include "OnBoardRomDialog.h"
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

wxString GetFilenameForOnBoardRom(int i)
{
	wxString sFilename = wxEmptyString;
	switch (i)
	{
	case 0:
	{
		wxConfig::Get(false)->Read(wxT("onboard_rom_override/os/filename"), &sFilename, wxEmptyString);
	}
	break;

	case 1:
	{
		wxConfig::Get(false)->Read(wxT("onboard_rom_override/basic/filename"), &sFilename, wxEmptyString);
	}
	break;

	case 2:
	{
		wxConfig::Get(false)->Read(wxT("onboard_rom_override/amsdos/filename"), &sFilename, wxEmptyString);
	}
	break;

	}
	return sFilename;
}

BOOL GetEnableForOnBoardRom(int i)
{
	BOOL bEnabled = FALSE;
	switch (i)
	{
	case 0:
	{
		bEnabled = CPC_GetOSOverrideROMEnable();
	}
	break;

	case 1:
	{
		bEnabled = CPC_GetBASICOverrideROMEnable();
	}
	break;

	case 2:
	{
		bEnabled = CPC_GetAmsdosOverrideROMEnable();
	}
	break;

	}
	return bEnabled;
}

OnBoardRomsListCtrl::OnBoardRomsListCtrl() : SortableListCtrl()
{
}

OnBoardRomsListCtrl::~OnBoardRomsListCtrl()
{
}

IMPLEMENT_DYNAMIC_CLASS(OnBoardRomsListCtrl, SortableListCtrl)

BEGIN_EVENT_TABLE(OnBoardRomsListCtrl, SortableListCtrl)
END_EVENT_TABLE()




static const wxChar *sYes = wxT("Yes");
static const wxChar *sNo = wxT("No");
WX_DEFINE_ARRAY_INT(int, ArrayOfInts);

BEGIN_EVENT_TABLE(OnBoardRomDialog, wxDialog)
EVT_CONTEXT_MENU(OnBoardRomDialog::OnContextMenu)
EVT_COMMAND(XRCID("m_ExpansionROMEnable"), wxEVT_COMMAND_MENU_SELECTED, OnBoardRomDialog::OnROMEnable)
EVT_COMMAND(XRCID("m_ExpansionROMRemove"), wxEVT_COMMAND_MENU_SELECTED, OnBoardRomDialog::OnROMRemove)
EVT_COMMAND(XRCID("m_ExpansionROMLoad"), wxEVT_COMMAND_MENU_SELECTED, OnBoardRomDialog::OnROMLoad)
EVT_COMMAND(XRCID("m_ExpansionROMPickBuiltin"), wxEVT_COMMAND_MENU_SELECTED, OnBoardRomDialog::OnROMPickBuiltin)
EVT_CHAR_HOOK(OnBoardRomDialog::OnCharHook)
END_EVENT_TABLE()



void OnBoardRomDialog::OnCharHook(wxKeyEvent& event)
{
	if (IsEscapeKey(event))
	{
		Close();
		return;
	}

	event.Skip();
}

const wxChar *sSlots[] =
{
	wxT("OS"),               // CPC/KCC/Plus
	wxT("BASIC"),         // CPC/KCC/Plus
	wxT("AMSDOS")        // CPC/KCC/Aleste/Plus
};


int OnBoardRomsListCtrl::PerformSort(wxIntPtr item1id, wxIntPtr item2id)
{
	int result = 0;

	switch (m_nSortColumn)
	{
	default:
	case 0:
	{
		result = ::wxStrcmp(sSlots[item1id], sSlots[item2id]);
	}
	break;

	case 1:
	{
		wxString sFilenameA = GetFilenameForOnBoardRom(item1id);
		wxString sFilenameB = GetFilenameForOnBoardRom(item2id);
		result = sFilenameA.CmpNoCase(sFilenameB);
	}
	break;

	case 2:
	{
		BOOL bEnabledA = GetEnableForOnBoardRom(item1id);
		BOOL bEnabledB = GetEnableForOnBoardRom(item2id);
		int a = bEnabledA ? 1 : 0;
		int b = bEnabledB ? 1 : 0;
		result = a - b;
	}
	break;
	}

	if (!m_bSortAscending)
	{
		result = -result;
	}

	return result;
}

void OnBoardRomDialog::OnROMEnable(wxCommandEvent & WXUNUSED(event))
{
	wxListCtrl *pListCtrl = GetListCtrl();

	ArrayOfInts Array;
	{
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
	}

	for (int i = Array.GetCount() - 1; i >= 0; i--)
	{
		int item = Array[i];

		switch (pListCtrl->GetItemData(item))
		{
		case 0:
		{
			CPC_SetOSOverrideROMEnable(!CPC_GetOSOverrideROMEnable());

			wxConfig::Get(false)->Write(wxT("onboard_rom_override/os/enable"), CPC_GetOSOverrideROMEnable() ? true : false);

			wxString sFilename;
			wxConfig::Get(false)->Read(wxT("onboard_rom_override/os/filename"), &sFilename, wxEmptyString);

			if (sFilename.IsEmpty())
			{
				wxMessageBox(wxT("You have enabled the OS rom override but no ROM data is set. You will see a black screen because the CPC will be executing unmapped memory"),
					wxGetApp().GetAppName(), wxICON_WARNING | wxOK);
			}
		}
		break;

		case 1:
		{
			CPC_SetBASICOverrideROMEnable(!CPC_GetBASICOverrideROMEnable());

			wxConfig::Get(false)->Write(wxT("onboard_rom_override/basic/enable"), CPC_GetBASICOverrideROMEnable() ? true : false);

			wxString sFilename;
			wxConfig::Get(false)->Read(wxT("onboard_rom_override/basic/filename"), &sFilename, wxEmptyString);

			if (sFilename.IsEmpty())
			{
				wxMessageBox(wxT("You have enabled the BASIC rom override but no ROM data is set. The CPC will appear to hang on boot"),
					wxGetApp().GetAppName(), wxICON_WARNING | wxOK);
			}

		}
		break;

		case 2:
		{
			CPC_SetAmsdosOverrideROMEnable(!CPC_GetAmsdosOverrideROMEnable());

			wxConfig::Get(false)->Write(wxT("onboard_rom_override/amsdos/enable"), CPC_GetAmsdosOverrideROMEnable() ? true : false);

			wxString sFilename;
			wxConfig::Get(false)->Read(wxT("onboard_rom_override/amsdos/filename"), &sFilename, wxEmptyString);

			if (sFilename.IsEmpty())
			{
				wxMessageBox(wxT("You have enabled the AMSDOS rom override but no ROM data is set. The CPC will boot but disc can't be accessed from BASIC"),
					wxGetApp().GetAppName(), wxICON_WARNING | wxOK);
			}

		}
		break;

		}
	}

	PerformUpdate();
}


void OnBoardRomDialog::OnROMRemove(wxCommandEvent & WXUNUSED(event))
{

	wxListCtrl *pListCtrl = GetListCtrl();


	ArrayOfInts Array;
	{
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
	}

	for (int i = Array.GetCount() - 1; i >= 0; i--)
	{
		int item = Array[i];
		switch (pListCtrl->GetItemData(item))
		{

		case 0:
		{
			CPC_SetOSOverrideROMEnable(FALSE);
			ClearOSOverrideROM();

			wxConfig::Get(false)->Write(wxT("onboard_rom_override/os/enable"), CPC_GetOSOverrideROMEnable() ? true : false);
			wxConfig::Get(false)->Write(wxT("onboard_rom_override/os/filename"), wxEmptyString);
		}
		break;

		case 1:
		{
			CPC_SetBASICOverrideROMEnable(FALSE);
			ClearBASICOverrideROM();
			wxConfig::Get(false)->Write(wxT("onboard_rom_override/basic/enable"), CPC_GetBASICOverrideROMEnable() ? true : false);
			wxConfig::Get(false)->Write(wxT("onboard_rom_override/basic/filename"), wxEmptyString);
		}
		break;

		case 2:
		{
			CPC_SetAmsdosOverrideROMEnable(FALSE);
			ClearAmsdosOverrideROM();

			wxConfig::Get(false)->Write(wxT("onboard_rom_override/amsdos/enable"), CPC_GetAmsdosOverrideROMEnable() ? true : false);
			wxConfig::Get(false)->Write(wxT("onboard_rom_override/amsdos/filename"), wxEmptyString);
		}
		break;

		}
	}

	PerformUpdate();
}

void OnBoardRomDialog::OnROMLoad(wxCommandEvent & WXUNUSED(event))
{

	wxListCtrl *pListCtrl = GetListCtrl();
	long item = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item == -1)
	{
		return;
	}

	// TODO: Fix.
	arnguiFrame * frame = wxDynamicCast(wxGetApp().GetTopWindow(),
		arnguiFrame);

	unsigned char *pRomData = NULL;
	unsigned long RomDataLength = 0;
	wxString sFilename;

	if (frame->OpenROM(&pRomData, &RomDataLength, sFilename, sSlots[item]))
	{
		if (pRomData != NULL)
		{
			switch (pListCtrl->GetItemData(item))
			{

			case 0:
			{
				CPC_SetOSOverrideROM(pRomData, RomDataLength);
				CPC_SetOSOverrideROMEnable(TRUE);
				wxConfig::Get(false)->Write(wxT("onboard_rom_override/os/enable"), CPC_GetOSOverrideROMEnable() ? true : false);
				wxConfig::Get(false)->Write(wxT("onboard_rom_override/os/filename"), sFilename);
			}
			break;

			case 1:
			{
				CPC_SetBASICOverrideROM(pRomData, RomDataLength);
				CPC_SetBASICOverrideROMEnable(TRUE);
				wxConfig::Get(false)->Write(wxT("onboard_rom_override/basic/enable"), CPC_GetBASICOverrideROMEnable() ? true : false);
				wxConfig::Get(false)->Write(wxT("onboard_rom_override/basic/filename"), sFilename);
			}
			break;

			case 2:
			{
				CPC_SetAmsdosOverrideROM(pRomData, RomDataLength);
				CPC_SetAmsdosOverrideROMEnable(TRUE);
				wxConfig::Get(false)->Write(wxT("onboard_rom_override/amsdos/enable"), CPC_GetAmsdosOverrideROMEnable() ? true : false);
				wxConfig::Get(false)->Write(wxT("onboard_rom_override/amsdos/filename"), sFilename);
			}
			break;

			}



			wxMessageBox(wxT("You may need to restart the computer to use the ROM"),
				wxGetApp().GetAppName(), wxICON_WARNING | wxOK);

			free(pRomData);
		}

	}
	PerformUpdate();
}

void OnBoardRomDialog::OnROMPickBuiltin(wxCommandEvent & WXUNUSED(event))
{

	wxListCtrl *pListCtrl = GetListCtrl();
	long item = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item == -1)
	{
		return;
	}

	//   WarnROMLoad(item);

	wxString sROMName = sSlots[item];
	wxString sID;
	unsigned char *pData = NULL;
	unsigned long nLength = 0;
	if (wxGetApp().PickBuiltInMedia(BUILTIN_MEDIA_ROM, sROMName, sID, &pData, &nLength))
	{
		switch (pListCtrl->GetItemData(item))
		{
		case 0:
		{
			CPC_SetOSOverrideROM(pData, nLength);
			CPC_SetOSOverrideROMEnable(TRUE);
			wxConfig::Get(false)->Write(wxT("onboard_rom_override/os/enable"), CPC_GetOSOverrideROMEnable() ? true : false);
			wxConfig::Get(false)->Write(wxT("onboard_rom_override/os/filename"), sID);
		}
		break;

		case 1:
		{
			CPC_SetBASICOverrideROM(pData, nLength);
			CPC_SetBASICOverrideROMEnable(TRUE);
			wxConfig::Get(false)->Write(wxT("onboard_rom_override/basic/enable"), CPC_GetBASICOverrideROMEnable() ? true : false);
			wxConfig::Get(false)->Write(wxT("onboard_rom_override/basic/filename"), sID);
		}
		break;
		case 2:
		{
			CPC_SetAmsdosOverrideROM(pData, nLength);
			CPC_SetAmsdosOverrideROMEnable(TRUE);
			wxConfig::Get(false)->Write(wxT("onboard_rom_override/amsdos/enable"), CPC_GetAmsdosOverrideROMEnable() ? true : false);
			wxConfig::Get(false)->Write(wxT("onboard_rom_override/amsdos/filename"), sID);
		}
		break;
		}

		wxMessageBox(wxT("You may need to restart the computer to use the ROM"),
			wxGetApp().GetAppName(), wxICON_WARNING | wxOK);

	}


	PerformUpdate();

}




void OnBoardRomDialog::OnContextMenu(wxContextMenuEvent &event)
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

void OnBoardRomDialog::PerformUpdate()
{
	wxListCtrl *pListCtrl = GetListCtrl();

	pListCtrl->Freeze();
	pListCtrl->DeleteAllItems();

	wxListItem Item;
	wxString sValue;

	for (size_t i = 0; i < sizeof(sSlots) / sizeof(sSlots[0]); i++)
	{
		Item.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_DATA | wxLIST_MASK_IMAGE);
		Item.SetId(i);
		Item.SetText(sSlots[i]);
		Item.SetColumn(0);
		Item.SetData(i);
		Item.SetImage(-1);
		int ItemID = pListCtrl->InsertItem(Item);

		wxString sFilename = GetFilenameForOnBoardRom(i);

		Item.SetMask(wxLIST_MASK_TEXT);
		Item.SetId(ItemID);
		Item.SetText(sFilename);
		Item.SetColumn(1);
		pListCtrl->SetItem(Item);

		BOOL bEnabled = GetEnableForOnBoardRom(i);
		if (bEnabled)
		{
			sValue = sYes;
		}
		else
		{
			sValue = sNo;
		}
		Item.SetMask(wxLIST_MASK_TEXT);
		Item.SetId(ItemID);
		Item.SetText(sValue);
		Item.SetColumn(2);
		pListCtrl->SetItem(Item);
	}

	pListCtrl->Thaw();
}

bool OnBoardRomDialog::TransferDataToWindow()
{
	wxListCtrl *pListCtrl = GetListCtrl();

	pListCtrl->Freeze();
	pListCtrl->ClearAll();

	//	SetTitle(wxT("Override On-board ROMs"));


	wxListItem Column;
	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT("Name"));
	pListCtrl->InsertColumn(0, Column);
	Column.SetText(wxT("Filename"));
	pListCtrl->InsertColumn(1, Column);
	Column.SetText(wxT("Active"));
	pListCtrl->InsertColumn(2, Column);

	pListCtrl->Thaw();
	PerformUpdate();
	wxGetApp().ReadConfigWindowPosAndSize(wxT("windows/on_board_roms/"), this);
	wxGetApp().EnsureWindowVisible(this);

	wxGetApp().SetColumnSize(pListCtrl, 0);
	wxGetApp().SetColumnSize(pListCtrl, 1);
	wxGetApp().SetColumnSize(pListCtrl, 2);

	wxGetApp().ReadConfigListCtrl(wxT("windows/on_board_roms/listctrl/"), GetListCtrl());
	wxGetApp().EnsureWindowVisible(this);


	return true;
}

wxListCtrl *OnBoardRomDialog::GetListCtrl()
{
	return XRCCTRL(*this, "m_listCtrl6", wxListCtrl);
}

OnBoardRomDialog::OnBoardRomDialog(wxWindow *pParent)
{

	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_DIALOG_ONBOARDROMS"));
}

