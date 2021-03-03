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
// TODO: Mark lists as sortable
#include "DevicesDialog.h"
#include <wx/xrc/xmlres.h>
#include "arnguiMain.h"
#include "arnguiApp.h"
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include "ExpansionRomDialog.h"

/* Mac is currently broken in respect of pop up menus. We
should put a menu in the window or a toolbar */

WX_DEFINE_ARRAY_INT(int, ArrayOfInts);

extern "C"
{

#include "../cpc/emudevice.h"
}

static const wxChar *sYes = wxT("Yes");
static const wxChar *sNo = wxT("No");

DeviceRomsListCtrl::DeviceRomsListCtrl() : SortableListCtrl()
{
	m_nDevice = 0;
}

DeviceRomsListCtrl::~DeviceRomsListCtrl()
{
}

IMPLEMENT_DYNAMIC_CLASS(DeviceRomsListCtrl, SortableListCtrl)

#ifdef WIN32
#define strcomp stricmp
#else
#define strcomp strcasecmp
#endif

int DeviceRomsListCtrl::PerformSort(wxIntPtr item1id, wxIntPtr item2id)
{
	// item1id and item2id contain the data of the items
	int result = 0;

	switch (m_nSortColumn)
	{
	default:
	case 0:
	{
		result = strcomp(EmuDevice_GetRomName(m_nDevice, item1id), EmuDevice_GetRomName(m_nDevice, item2id));
	}
	break;

	case 1:
	{
		int a = EmuDevice_GetRomSize(m_nDevice, item1id);
		int b = EmuDevice_GetRomSize(m_nDevice, item2id);
		result = a - b;
	}
	break;
	case 2:
	{
		wxString sDeviceName(EmuDevice_GetSaveName(m_nDevice), wxConvUTF8);
		wxString sDevice1RomName(EmuDevice_GetRomSaveName(m_nDevice, item1id), wxConvUTF8);
		wxString sDevice2RomName(EmuDevice_GetRomSaveName(m_nDevice, item2id), wxConvUTF8);
		wxString sKey;

		wxString sFilename1;
		sKey.sprintf(wxT("devices/%s/roms/%s/filename"), sDeviceName.c_str(), sDevice1RomName.c_str());
		wxConfig::Get(false)->Read(sKey, &sFilename1, wxEmptyString);

		wxString sFilename2;
		sKey.sprintf(wxT("devices/%s/roms/%s/filename"), sDeviceName.c_str(), sDevice2RomName.c_str());
		wxConfig::Get(false)->Read(sKey, &sFilename2, wxEmptyString);

		result = sFilename1.CmpNoCase(sFilename2);
	}
	break;

	}

	if (!m_bSortAscending)
	{
		result = -result;
	}

	return result;
}



DevicesRomsDialog::DevicesRomsDialog(wxWindow *pParent, int nDevice) : m_nDevice(nDevice)
{
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DEVICE_ROMS_DIALOG"));
}

BEGIN_EVENT_TABLE(DevicesRomsDialog, wxDialog)
EVT_CONTEXT_MENU(DevicesRomsDialog::OnContextMenu)
EVT_COMMAND(XRCID("m_ExpansionROMRemove"), wxEVT_COMMAND_MENU_SELECTED, DevicesRomsDialog::OnROMRemove)
EVT_COMMAND(XRCID("m_ExpansionROMLoad"), wxEVT_COMMAND_MENU_SELECTED, DevicesRomsDialog::OnROMLoad)
EVT_COMMAND(XRCID("m_ExpansionROMPickBuiltin"), wxEVT_COMMAND_MENU_SELECTED, DevicesRomsDialog::OnROMPickBuiltin)
EVT_CHAR_HOOK(DevicesRomsDialog::OnCharHook)
END_EVENT_TABLE()


void DevicesRomsDialog::OnCharHook(wxKeyEvent& event)
{
	if (IsEscapeKey(event))
	{
		Close();
		return;
	}

	event.Skip();
}


void DevicesRomsDialog::OnROMRemove(wxCommandEvent & WXUNUSED(event))
{
	wxListCtrl *pListCtrl = GetListCtrl();
	long item = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item == -1)
	{
		return;
	}

	int RomIndex = (int)pListCtrl->GetItemData(item);

	EmuDevice_ClearRom(m_nDevice, RomIndex);

	wxString sDeviceName(EmuDevice_GetSaveName(m_nDevice), wxConvUTF8);
	wxString sDeviceRomName(EmuDevice_GetRomSaveName(m_nDevice, RomIndex), wxConvUTF8);

	wxString sKey;
	sKey.sprintf(wxT("devices/%s/roms/%s/filename"), sDeviceName.c_str(), sDeviceRomName.c_str());
	wxConfig::Get(false)->Write(sKey, wxEmptyString);

	PerformUpdate();
}

// TODO: Add CRC so we can say which rom it was checked with.
void DevicesRomsDialog::VerifyRomData(const unsigned char *pRomData, unsigned long RomLength, unsigned long nSize)
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

void DevicesRomsDialog::OnROMLoad(wxCommandEvent & WXUNUSED(event))
{
	wxListCtrl *pListCtrl = GetListCtrl();
	long item = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item == -1)
	{
		return;
	}

	int RomIndex = (int)pListCtrl->GetItemData(item);


	// TODO: Fix.
	arnguiFrame * frame = wxDynamicCast(wxGetApp().GetTopWindow(),
		arnguiFrame);

	unsigned char *pRomData = NULL;
	unsigned long RomDataLength = 0;

	wxString sTitleSuffix(EmuDevice_GetRomName(m_nDevice, RomIndex), wxConvUTF8);

	wxString sFilename;
	if (frame->OpenROM(&pRomData, &RomDataLength, sFilename, sTitleSuffix))
	{
		size_t nSize = EmuDevice_GetRomSize(m_nDevice, RomIndex);
		VerifyRomData(pRomData,RomDataLength, nSize);

		EmuDevice_SetRom(m_nDevice, RomIndex, pRomData, RomDataLength);

		wxString sDeviceName(EmuDevice_GetSaveName(m_nDevice), wxConvUTF8);
		wxString sDeviceRomName(EmuDevice_GetRomSaveName(m_nDevice, RomIndex), wxConvUTF8);

		wxString sKey;
		sKey.sprintf(wxT("devices/%s/roms/%s/filename"), sDeviceName.c_str(), sDeviceRomName.c_str());
		wxConfig::Get(false)->Write(sKey, sFilename);

		free(pRomData);
	}
	PerformUpdate();
}

void DevicesRomsDialog::OnROMPickBuiltin(wxCommandEvent & WXUNUSED(event))
{
	wxListCtrl *pListCtrl = GetListCtrl();
	long item = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item == -1)
	{
		return;
	}
	int RomIndex = (int)pListCtrl->GetItemData(item);


	wxString sROMName(EmuDevice_GetRomName(m_nDevice, RomIndex), wxConvUTF8);
	wxString sID;
	unsigned char *pData = NULL;
	unsigned long nLength = 0;
	if (wxGetApp().PickBuiltInMedia(BUILTIN_MEDIA_ROM, sROMName, sID, &pData, &nLength))
	{
		size_t nSize = EmuDevice_GetRomSize(m_nDevice, RomIndex);
		VerifyRomData(pData, nLength, nSize);


		EmuDevice_SetRom(m_nDevice, RomIndex, pData, nLength);

		wxString sDeviceName(EmuDevice_GetSaveName(m_nDevice), wxConvUTF8);
		wxString sDeviceRomName(EmuDevice_GetRomSaveName(m_nDevice, RomIndex), wxConvUTF8);

		wxString sKey;
		sKey.sprintf(wxT("devices/%s/roms/%s/filename"), sDeviceName.c_str(), sDeviceRomName.c_str());
		wxConfig::Get(false)->Write(sKey, sID);
	}

	PerformUpdate();

}




void DevicesRomsDialog::OnContextMenu(wxContextMenuEvent &event)
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
		if (pMenu != NULL)
		{
			PopupMenu(pMenu);
			delete pMenu;
		}
		delete pMenuBar;
	}
}


wxListCtrl *DevicesRomsDialog::GetListCtrl()
{
	return XRCCTRL(*this, "m_listCtrl5", wxListCtrl);
}


DevicesRomsDialog::~DevicesRomsDialog()
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

void DevicesRomsDialog::UpdateRom(int nIndex)
{
	wxString sValue;
	wxListCtrl *pListCtrl = GetListCtrl();
	int RomIndex = (int)pListCtrl->GetItemData(nIndex);


	wxListItem Item;
	Item.SetMask(wxLIST_MASK_TEXT);
	Item.SetId(nIndex);

	wxString sName(EmuDevice_GetRomName(m_nDevice, RomIndex), wxConvUTF8);

	// rom name

	sValue = sName;
	Item.SetText(sValue);
	Item.SetColumn(0);
	Item.SetImage(-1); //Insert a blank icon, not find to insert nothing
	pListCtrl->SetItem(Item);

	// rom size

	// return in human readable form for UI
	sValue = wxFileName::GetHumanReadableSize(EmuDevice_GetRomSize(m_nDevice, RomIndex), wxT(""), 2);
	Item.SetText(sValue);
	Item.SetColumn(1);
	pListCtrl->SetItem(Item);

	wxString sDeviceName(EmuDevice_GetSaveName(m_nDevice), wxConvUTF8);
	wxString sDeviceRomName(EmuDevice_GetRomSaveName(m_nDevice, RomIndex), wxConvUTF8);

	wxString sKey;
	sKey.sprintf(wxT("devices/%s/roms/%s/filename"), sDeviceName.c_str(), sDeviceRomName.c_str());
	wxConfig::Get(false)->Read(sKey, &sValue, wxEmptyString);
	Item.SetText(sValue);
	Item.SetColumn(2);
	pListCtrl->SetItem(Item);


}

void DevicesRomsDialog::AddRom(int nRom)
{
	wxString sValue = wxT("Temp");

	wxListCtrl *pListCtrl = GetListCtrl();
	wxListItem Item;

	// name
	Item.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_IMAGE);
	Item.SetId(pListCtrl->GetItemCount());
	Item.SetText(sValue);
	Item.SetColumn(0);
	Item.SetData(nRom);
	Item.SetImage(-1); //Insert a blank icon, not find to insert nothing
	long nIndex = pListCtrl->InsertItem(Item);

	//	Item.SetId(nIndex);
	//	Item.SetText(sValue);
	//	Item.SetColumn(1);
	//	pListCtrl->SetItem(Item);



	UpdateRom(nIndex);
}

void DevicesRomsDialog::PerformUpdate()
{
	wxListCtrl *pListCtrl = GetListCtrl();

	pListCtrl->Freeze();
	//pListCtrl->DeleteAllItems();
	for (int i = 0; i < pListCtrl->GetItemCount(); i++)
	{
		UpdateRom(i);
	}

	pListCtrl->Thaw();

}

bool DevicesRomsDialog::TransferDataFromWindow()
{
	wxGetApp().WriteConfigWindowPosAndSize(wxT("windows/devices/roms"), this);

	wxGetApp().WriteConfigListCtrl(wxT("windows/devices/roms/listctrl/"), GetListCtrl());

	return true;
}

bool DevicesRomsDialog::TransferDataToWindow()
{
	wxString sName(EmuDevice_GetName(m_nDevice), wxConvUTF8);
	wxString sTitle = wxT("Roms - ");
	sTitle += sName;
	SetTitle(sTitle);

	DeviceRomsListCtrl *pListCtrl = (DeviceRomsListCtrl *)GetListCtrl();
	pListCtrl->SetDeviceIndex(m_nDevice);

	pListCtrl->Freeze();
	pListCtrl->ClearAll();

	wxListItem Column;

	Column.SetMask(wxLIST_MASK_TEXT);

	// this is the friendly name
	Column.SetText(wxT("Rom Name"));
	pListCtrl->InsertColumn(0, Column);
	Column.SetText(wxT("Rom Size"));
	pListCtrl->InsertColumn(1, Column);
	Column.SetText(wxT("Filename"));
	pListCtrl->InsertColumn(2, Column);

	pListCtrl->Thaw();

	pListCtrl->Freeze();
	pListCtrl->DeleteAllItems();

	for (int i = 0; i < EmuDevice_GetNumRoms(m_nDevice); i++)
	{
		AddRom(i);
	}

	pListCtrl->Thaw();

	wxGetApp().SetColumnSize(pListCtrl, 0);
	wxGetApp().SetColumnSize(pListCtrl, 1);
	wxGetApp().SetColumnSize(pListCtrl, 2);
	pListCtrl->SortNow();

	//arnguiFrame *pFrame = static_cast<arnguiFrame *>(GetParent());

	//pFrame->RegisterWantUpdateFromTimer(this);

	// default settings
	//this->SetSize(530,140);

	wxGetApp().ReadConfigWindowPosAndSize(wxT("windows/devices/roms"), this);

	wxGetApp().ReadConfigListCtrl(wxT("windows/devices/roms/listctrl/"), GetListCtrl());
	wxGetApp().EnsureWindowVisible(this);


	return true;
}












DevicesButtonsDialog::DevicesButtonsDialog(wxWindow *pParent, int nDevice) : m_nDevice(nDevice)
{
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DEVICE_BUTTONS_DIALOG"));
}

BEGIN_EVENT_TABLE(DevicesButtonsDialog, wxDialog)
EVT_CONTEXT_MENU(DevicesButtonsDialog::OnContextMenu)
EVT_COMMAND(XRCID("m_ButtonOperate"), wxEVT_COMMAND_MENU_SELECTED, DevicesButtonsDialog::OnButtonPressed)
EVT_CHAR_HOOK(DevicesButtonsDialog::OnCharHook)

//EVT_CLOSE(DevicesDialog::OnClose)
END_EVENT_TABLE()

void DevicesButtonsDialog::OnCharHook(wxKeyEvent& event)
{
	if (IsEscapeKey(event))
	{
		Close();
		return;
	}

	event.Skip();
}


void DevicesButtonsDialog::OnButtonPressed(wxCommandEvent & WXUNUSED(event))
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
		int ButtonIndex = (int)pListCtrl->GetItemData(item);

		EmuDevice_PressButton(m_nDevice, ButtonIndex);

		if (!EmuDevice_IsEnabled(m_nDevice))
		{
			wxMessageBox(wxT("The device is not enabled. Pressing this button will have no effect."));
		}
			
	}

	PerformUpdate();
}


void DevicesButtonsDialog::OnContextMenu(wxContextMenuEvent &event)
{
	wxPoint Position = event.GetPosition();

	// generate a suitable place for the menu
	if (event.GetPosition()==wxDefaultPosition)
	{
		wxPoint ClientPos(0,0);
		Position = this->ClientToScreen(ClientPos);
	}

	wxMenuBar *pMenuBar =  wxXmlResource::Get()->LoadMenuBar(wxT("MB_MENU_DEVICES_BUTTONS_MENU_BAR"));
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

#if 0
void DevicesDialog::OnClose(wxCloseEvent &event)
{
	wxGetApp().WriteConfigWindowPosAndSize(wxT("windows/devices/"), this);

	wxGetApp().WriteConfigListCtrl(wxT("windows/devices/listctrl/"), GetListCtrl());

	event.
}
#endif


DeviceButtonsListCtrl::DeviceButtonsListCtrl() : SortableListCtrl()
{
	m_nDevice = 0;
}

DeviceButtonsListCtrl::~DeviceButtonsListCtrl()
{
}

IMPLEMENT_DYNAMIC_CLASS(DeviceButtonsListCtrl, SortableListCtrl)


int DeviceButtonsListCtrl::PerformSort(wxIntPtr item1id, wxIntPtr item2id)
{
	int result = 0;

	switch (m_nSortColumn)
	{
	default:
	case 0:
	{
		result = strcomp(EmuDevice_GetButtonName(m_nDevice, item1id), EmuDevice_GetButtonName(m_nDevice, item2id));
	}
	break;


	}

	if (!m_bSortAscending)
	{
		result = -result;
	}

	return result;
}

//BEGIN_EVENT_TABLE(DeviceButtonsListCtrl, SortableListCtrl)
//END_EVENT_TABLE()

wxListCtrl *DevicesButtonsDialog::GetListCtrl()
{
	return XRCCTRL(*this, "m_listCtrl5", wxListCtrl);
}


DevicesButtonsDialog::~DevicesButtonsDialog()
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

void DevicesButtonsDialog::UpdateButton(int nIndex)
{
	wxString sValue;
	wxListCtrl *pListCtrl = GetListCtrl();

	int ButtonIndex = (int)pListCtrl->GetItemData(nIndex);


	wxListItem Item;
	Item.SetMask(wxLIST_MASK_TEXT);
	Item.SetId(nIndex);
	wxString sName(EmuDevice_GetButtonName(m_nDevice, ButtonIndex), wxConvUTF8);

	// name
	sValue = sName;
	Item.SetText(sValue);
	Item.SetColumn(0);
	pListCtrl->SetItem(Item);
}

void DevicesButtonsDialog::AddButton(int nButton)
{
	wxString sValue = wxT("Temp");

	wxListCtrl *pListCtrl = GetListCtrl();
	wxListItem Item;

	// name
	Item.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_DATA | wxLIST_MASK_IMAGE);
	Item.SetId(pListCtrl->GetItemCount());
	Item.SetText(sValue);
	Item.SetColumn(0);
	Item.SetData(nButton);
	Item.SetImage(-1); //Insert a blank icon, not find to insert nothing
	long nIndex = pListCtrl->InsertItem(Item);


	UpdateButton(nIndex);
}

void DevicesButtonsDialog::PerformUpdate()
{
	wxListCtrl *pListCtrl = GetListCtrl();

	pListCtrl->Freeze();
	//pListCtrl->DeleteAllItems();
	for (int i = 0; i < pListCtrl->GetItemCount(); i++)
	{
		UpdateButton(i);
	}

	pListCtrl->Thaw();

}

bool DevicesButtonsDialog::TransferDataFromWindow()
{
	wxGetApp().WriteConfigWindowPosAndSize(wxT("windows/devices/buttons"), this);

	wxGetApp().WriteConfigListCtrl(wxT("windows/devices/buttons/listctrl/"), GetListCtrl());

	return true;
}

bool DevicesButtonsDialog::TransferDataToWindow()
{
	wxString sName(EmuDevice_GetName(m_nDevice), wxConvUTF8);
	wxString sTitle = wxT("Buttons - ");
	sTitle += sName;
	SetTitle(sTitle);


	DeviceButtonsListCtrl *pListCtrl = (DeviceButtonsListCtrl *)GetListCtrl();
	pListCtrl->SetDeviceIndex(m_nDevice);

	pListCtrl->Freeze();
	pListCtrl->ClearAll();

	wxListItem Column;

	Column.SetMask(wxLIST_MASK_TEXT);

	// this is the friendly name
	Column.SetText(wxT("Button Name"));
	pListCtrl->InsertColumn(0, Column);

	pListCtrl->Thaw();

	pListCtrl->Freeze();
	pListCtrl->DeleteAllItems();

	for (int i = 0; i < EmuDevice_GetNumButtons(m_nDevice); i++)
	{
		AddButton(i);
	}

	pListCtrl->Thaw();


	wxGetApp().SetColumnSize(pListCtrl, 0);
	pListCtrl->SortNow();

	wxGetApp().ReadConfigWindowPosAndSize(wxT("windows/devices/buttons"), this);

	wxGetApp().ReadConfigListCtrl(wxT("windows/devices/buttons/listctrl/"), GetListCtrl());
	wxGetApp().EnsureWindowVisible(this);


	return true;
}









DeviceSwitchesDialog::DeviceSwitchesDialog(wxWindow *pParent, int nDevice) : m_nDevice(nDevice)
{
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DEVICE_SWITCHES_DIALOG"));
}

BEGIN_EVENT_TABLE(DeviceSwitchesDialog, wxDialog)
EVT_CONTEXT_MENU(DeviceSwitchesDialog::OnContextMenu)
EVT_COMMAND(XRCID("m_SwitchOperate"), wxEVT_COMMAND_MENU_SELECTED, DeviceSwitchesDialog::OnSwitchEnable)
EVT_CHAR_HOOK(DeviceSwitchesDialog::OnCharHook)

//EVT_CLOSE(DevicesDialog::OnClose)
END_EVENT_TABLE()


void DeviceSwitchesDialog::OnCharHook(wxKeyEvent& event)
{
	if (IsEscapeKey(event))
	{
		Close();
		return;
	}

	event.Skip();
}


void DeviceSwitchesDialog::OnSwitchEnable(wxCommandEvent & WXUNUSED(event))
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
		int SwitchIndex = (int)pListCtrl->GetItemData(item);


		EmuDevice_SetSwitchState(m_nDevice, SwitchIndex, !EmuDevice_GetSwitchState(m_nDevice, SwitchIndex));
		
		wxString sDeviceName(EmuDevice_GetSaveName(m_nDevice), wxConvUTF8);
		wxString sSwitchName(EmuDevice_GetSwitchSaveName(m_nDevice, SwitchIndex), wxConvUTF8);

		wxString sKey;
		sKey.sprintf(wxT("devices/%s/switches/%s/state"), sDeviceName.c_str(), sSwitchName.c_str());
		wxConfig::Get(false)->Write(sKey, EmuDevice_GetSwitchState(m_nDevice, SwitchIndex));

		if (!EmuDevice_IsEnabled(m_nDevice))
		{
			wxMessageBox(wxT("The device is not enabled. Changing the switch state will have no effect."));
		}

	}

	PerformUpdate();
}


void DeviceSwitchesDialog::OnContextMenu(wxContextMenuEvent &event)
{
	wxPoint Position = event.GetPosition();

	// generate a suitable place for the menu
	if (event.GetPosition() == wxDefaultPosition)
	{
		wxPoint ClientPos(0,0);
		Position = this->ClientToScreen(ClientPos);
	}

	wxMenuBar *pMenuBar =  wxXmlResource::Get()->LoadMenuBar(wxT("MB_MENU_DEVICES_SWITCHES_MENU_BAR"));
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

#if 0
void DevicesDialog::OnClose(wxCloseEvent &event)
{
	wxGetApp().WriteConfigWindowPosAndSize(wxT("windows/devices/"), this);

	wxGetApp().WriteConfigListCtrl(wxT("windows/devices/listctrl/"), GetListCtrl());

	event.
}
#endif

wxListCtrl *DeviceSwitchesDialog::GetListCtrl()
{
	return XRCCTRL(*this, "m_listCtrl5", wxListCtrl);
}

DeviceSwitchesListCtrl::DeviceSwitchesListCtrl() : SortableListCtrl()
{
	m_nDevice = 0;
}

DeviceSwitchesListCtrl::~DeviceSwitchesListCtrl()
{
}

IMPLEMENT_DYNAMIC_CLASS(DeviceSwitchesListCtrl, SortableListCtrl)


int DeviceSwitchesListCtrl::PerformSort(wxIntPtr item1id, wxIntPtr item2id)
{
	int result = 0;

	switch (m_nSortColumn)
	{
	default:
	case 0:
	{
		result = strcomp(EmuDevice_GetSwitchName(m_nDevice, item1id), EmuDevice_GetSwitchName(m_nDevice, item2id));
	}
	break;
	case 1:
	{
		int a = EmuDevice_GetSwitchState(m_nDevice, item1id) ? 1 : 0;
		int b = EmuDevice_GetSwitchState(m_nDevice, item2id) ? 1 : 0;
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

//BEGIN_EVENT_TABLE(DeviceSwitchesListCtrl, SortableListCtrl)
//END_EVENT_TABLE()dow(XRCID("m_listCtrl5"));

DeviceSwitchesDialog::~DeviceSwitchesDialog()
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

void DeviceSwitchesDialog::UpdateSwitch(int nIndex)
{
	wxString sValue;
	wxListCtrl *pListCtrl = GetListCtrl();
	int SwitchIndex = (int)pListCtrl->GetItemData(nIndex);


	wxListItem Item;
	Item.SetMask(wxLIST_MASK_TEXT);
	Item.SetId(nIndex);

	wxString sName(EmuDevice_GetSwitchName(m_nDevice, SwitchIndex), wxConvUTF8);

	// name
	sValue = sName;
	Item.SetText(sValue);
	Item.SetColumn(0);
	pListCtrl->SetItem(Item);

	if (EmuDevice_GetSwitchState(m_nDevice, SwitchIndex))
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



}

void DeviceSwitchesDialog::AddSwitch(int nSwitch)
{
	wxString sValue = wxT("Temp");

	wxListCtrl *pListCtrl = GetListCtrl();
	wxListItem Item;

	// name
	Item.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_DATA | wxLIST_MASK_IMAGE);
	Item.SetId(pListCtrl->GetItemCount());
	Item.SetText(sValue);
	Item.SetColumn(0);
	Item.SetData(nSwitch);
	Item.SetImage(-1); //Insert a blank icon, not find to insert nothing
	long nIndex = pListCtrl->InsertItem(Item);

	//    // enabled
	//	Item.SetId(nIndex);
	//	Item.SetColumn(1);
	//	pListCtrl->SetItem(Item);

	UpdateSwitch(nIndex);
}

void DeviceSwitchesDialog::PerformUpdate()
{
	wxListCtrl *pListCtrl = GetListCtrl();

	pListCtrl->Freeze();
	//pListCtrl->DeleteAllItems();
	for (int i = 0; i < pListCtrl->GetItemCount(); i++)
	{
		UpdateSwitch(i);
	}

	pListCtrl->Thaw();

}


bool DeviceSwitchesDialog::TransferDataFromWindow()
{
	wxGetApp().WriteConfigWindowPosAndSize(wxT("windows/devices/switches"), this);

	wxGetApp().WriteConfigListCtrl(wxT("windows/devices/switches/listctrl/"), GetListCtrl());

	return true;
}

bool DeviceSwitchesDialog::TransferDataToWindow()
{
	wxString sName(EmuDevice_GetName(m_nDevice), wxConvUTF8);
	wxString sTitle = wxT("Switches - ");
	sTitle += sName;
	SetTitle(sTitle);


	DeviceSwitchesListCtrl *pListCtrl = (DeviceSwitchesListCtrl *)GetListCtrl();
	pListCtrl->SetDeviceIndex(m_nDevice);

	pListCtrl->Freeze();
	pListCtrl->ClearAll();

	wxListItem Column;

	Column.SetMask(wxLIST_MASK_TEXT);

	// this is the friendly name
	Column.SetText(wxT("Switch Name"));
	pListCtrl->InsertColumn(0, Column);

	Column.SetText(wxT("On?"));
	pListCtrl->InsertColumn(1, Column);

	pListCtrl->Thaw();

	pListCtrl->Freeze();
	pListCtrl->DeleteAllItems();

	for (int i = 0; i < EmuDevice_GetNumSwitches(m_nDevice); i++)
	{
		AddSwitch(i);
	}

	pListCtrl->Thaw();

	wxGetApp().SetColumnSize(pListCtrl, 0);
	wxGetApp().SetColumnSize(pListCtrl, 1);
	pListCtrl->SortNow();

	wxGetApp().ReadConfigWindowPosAndSize(wxT("windows/devices/switches"), this);

	wxGetApp().ReadConfigListCtrl(wxT("windows/devices/switches/listctrl/"), GetListCtrl());
	wxGetApp().EnsureWindowVisible(this);


	return true;
}






DevicesListCtrl::DevicesListCtrl() : SortableListCtrl()
{
}

DevicesListCtrl::~DevicesListCtrl()
{
}

IMPLEMENT_DYNAMIC_CLASS(DevicesListCtrl, SortableListCtrl)

int DevicesListCtrl::PerformSort(wxIntPtr item1id, wxIntPtr item2id)
{
	int result = 0;

	switch (m_nSortColumn)
	{
	default:
	case 0:
	{
		result = strcomp(EmuDevice_GetName(item1id), EmuDevice_GetName(item2id));
	}
	break;

	case 1:
	{
		int a = EmuDevice_HasPassthrough(item1id) ? 1 : 0;
		int b = EmuDevice_HasPassthrough(item2id) ? 1 : 0;
		result = a - b;
	}
	break;


	case 2:
	{
		int a = EmuDevice_GetNumSwitches(item1id) != 0 ? 1 : 0;
		int b = EmuDevice_GetNumSwitches(item2id) != 0 ? 1 : 0;
		result = a - b;
	}
	break;
	case 3:
	{
		int a = EmuDevice_GetNumButtons(item1id) != 0 ? 1 : 0;
		int b = EmuDevice_GetNumButtons(item2id) != 0 ? 1 : 0;
		result = a - b;
	}
	break;

	case 5:
	{
		int a = EmuDevice_IsEnabled(item1id) ? 1 : 0;
		int b = EmuDevice_IsEnabled(item2id) ? 1 : 0;
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



DevicesDialog::DevicesDialog(wxWindow *pParent, int type)
{
	m_ConnectionTypeFilter = type;
	
	m_bShowUntested = false;
	m_bShowNotWorking = false;
	
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DEVICES_DIALOG"));
}

BEGIN_EVENT_TABLE(DevicesDialog, wxDialog)
EVT_CONTEXT_MENU(DevicesDialog::OnContextMenu)
EVT_COMMAND(XRCID("m_DeviceEnable"), wxEVT_COMMAND_MENU_SELECTED, DevicesDialog::OnDeviceEnable)
EVT_COMMAND(XRCID("m_DeviceSwitches"), wxEVT_COMMAND_MENU_SELECTED, DevicesDialog::OnDeviceSwitches)
EVT_COMMAND(XRCID("m_DeviceButtons"), wxEVT_COMMAND_MENU_SELECTED, DevicesDialog::OnDeviceButtons)
EVT_COMMAND(XRCID("m_DeviceRoms"), wxEVT_COMMAND_MENU_SELECTED, DevicesDialog::OnDeviceRoms)
EVT_COMMAND(XRCID("m_DeviceExpansionRoms"), wxEVT_COMMAND_MENU_SELECTED, DevicesDialog::OnDeviceExpansionRoms)

EVT_COMMAND(XRCID("m_ShowUntested"), wxEVT_COMMAND_MENU_SELECTED, DevicesDialog::OnShowUntested)
EVT_COMMAND(XRCID("m_ShowNotWorking"), wxEVT_COMMAND_MENU_SELECTED, DevicesDialog::OnShowNotWorking)

EVT_CHAR_HOOK(DevicesDialog::OnCharHook)
END_EVENT_TABLE()

void DevicesDialog::OnShowUntested(wxCommandEvent & WXUNUSED(event))
{
	m_bShowUntested = !m_bShowUntested;
	UpdateDeviceList();
}

void DevicesDialog::OnShowNotWorking(wxCommandEvent & WXUNUSED(event))
{
	m_bShowNotWorking = !m_bShowNotWorking;
	UpdateDeviceList();
}
	


void DevicesDialog::OnCharHook(wxKeyEvent& event)
{
	if (IsEscapeKey(event))
	{
		Close();
		return;
	}

	event.Skip();
}


void DevicesDialog::OnDeviceExpansionRoms(wxCommandEvent & WXUNUSED(event))
{
	wxListCtrl *pListCtrl = GetListCtrl();

	ArrayOfInts Array;
	long item = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	if (item != -1)
	{
		long Data = pListCtrl->GetItemData(item);
		if (EmuDevice_GetExpansionRomData(Data) == NULL)
		{
			wxMessageBox(wxT("This device doesn't have any expansion rom slots"));
		}
		else
		{
			ExpansionRomsDialog dialog(this, Data);
			dialog.ShowModal();
		}
	}
}

void DevicesDialog::OnDeviceSwitches(wxCommandEvent & WXUNUSED(event))
{
	wxListCtrl *pListCtrl = GetListCtrl();

	ArrayOfInts Array;
	long item = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	if (item != -1)
	{
		wxUIntPtr Data = pListCtrl->GetItemData(item);
		if (EmuDevice_GetNumSwitches(Data) == 0)
		{
			wxMessageBox(wxT("This device doesn't have any switches"));
		}
		else
		{
			DeviceSwitchesDialog dialog(this, Data);
			dialog.ShowModal();
		}
	}
}


void DevicesDialog::OnDeviceButtons(wxCommandEvent & WXUNUSED(event))
{
	wxListCtrl *pListCtrl = GetListCtrl();

	ArrayOfInts Array;
	long item = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	if (item != -1)
	{
		wxUIntPtr Data = pListCtrl->GetItemData(item);
		if (EmuDevice_GetNumButtons(Data) == 0)
		{
			wxMessageBox(wxT("This device doesn't have any buttons"));
		}
		else
		{
			DevicesButtonsDialog dialog(this, Data);
			dialog.ShowModal();
		}
	}
}


void DevicesDialog::OnDeviceRoms(wxCommandEvent & WXUNUSED(event))
{
	wxListCtrl *pListCtrl = GetListCtrl();

	ArrayOfInts Array;
	long item = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	if (item != -1)
	{
		wxUIntPtr Data = pListCtrl->GetItemData(item);
		if (EmuDevice_GetNumRoms(Data) == 0)
		{
			wxMessageBox(wxT("This device doesn't have any system ROMs"));
		}
		else
		{
			DevicesRomsDialog dialog(this, Data);
			dialog.ShowModal();

		}
	}
}


void DevicesDialog::OnDeviceEnable(wxCommandEvent & WXUNUSED(event))
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

		wxUIntPtr Data = pListCtrl->GetItemData(item);
		bool bIsEnabled;

		wxString sName;

		wxString sKey;

		sName = wxString(EmuDevice_GetSaveName(Data), wxConvUTF8);

		EmuDevice_Enable(Data, !EmuDevice_IsEnabled(Data));
		bIsEnabled = EmuDevice_IsEnabled(Data) ? true : false;

		sKey.sprintf(wxT("devices/%s/enabled"), sName.c_str());
		wxConfig::Get(false)->Write(sKey, bIsEnabled);

		if (bIsEnabled)
		{
			wxMessageBox(wxT("You may need to reset the CPC to make full use of this device."));
		}
	}

	PerformUpdate();
}


void DevicesDialog::OnContextMenu(wxContextMenuEvent &event)
{
	wxPoint Position = event.GetPosition();

	// generate a suitable place for the menu
	if (event.GetPosition() == wxDefaultPosition)
	{
		wxPoint ClientPos(0, 0);
		Position = this->ClientToScreen(ClientPos);
	}

	wxMenuBar *pMenuBar =  wxXmlResource::Get()->LoadMenuBar(wxT("MB_MENU_DEVICES_MENU_BAR"));

	if (pMenuBar)
	{
		arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("m_ShowNotWorking"), m_bShowNotWorking);
		arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("m_ShowUntested"), m_bShowUntested);

		wxMenu *pMenu = pMenuBar->Remove(0);
		if (pMenu)
		{
				

			PopupMenu(pMenu);
			delete pMenu;
		}
		delete pMenuBar;
	}

}

#if 0
void DevicesDialog::OnClose(wxCloseEvent &event)
{
	wxGetApp().WriteConfigWindowPosAndSize(wxT("windows/devices/"), this);

	wxGetApp().WriteConfigListCtrl(wxT("windows/devices/listctrl/"), GetListCtrl());

	event.
}
#endif

wxListCtrl *DevicesDialog::GetListCtrl()
{
	return XRCCTRL(*this, "m_listCtrl5", wxListCtrl);
}


DevicesDialog::~DevicesDialog()
{
	wxListCtrl *pListCtrl = GetListCtrl();

	for (int i = 0; i < pListCtrl->GetItemCount(); i++)
	{
		//   wxStringClientData *pClientData = (wxStringClientData *)(pListCtrl->GetItemData(i));
		//  delete pClientData;
		// pListCtrl->SetItemData(i, NULL);
	}

	pListCtrl->ClearAll();
}

void DevicesDialog::UpdateDevice(int nIndex)
{
	wxString sValue;
	wxListCtrl *pListCtrl = GetListCtrl();

	wxUIntPtr Data = pListCtrl->GetItemData(nIndex);


	wxListItem Item;
	Item.SetMask(wxLIST_MASK_TEXT);
	Item.SetId(nIndex);

	wxString sName;

	sName = wxString(EmuDevice_GetName(Data), wxConvUTF8);
	// name
	sValue = sName;
	Item.SetText(sValue);
	Item.SetColumn(0);
	Item.SetImage(-1); //Insert a blank icon, not find to insert nothing
	pListCtrl->SetItem(Item);

	// passthrough
	bool bHasPassthrough = EmuDevice_HasPassthrough(Data) ? true : false;

	if (bHasPassthrough)
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

	bool bHasSwitches = EmuDevice_GetNumSwitches(Data) != 0;
	if (bHasSwitches)
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

	bool bHasButtons = EmuDevice_GetNumButtons(Data) != 0;

	if (bHasButtons)
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

	sValue.Clear();
	if ((EmuDevice_ConnectedTo(Data) & CONNECTION_EXPANSION) != 0)
	{
		if (!sValue.IsEmpty())
		{
			sValue += wxT(",");
		}
		sValue += wxT("Expansion");
	}
	if ((EmuDevice_ConnectedTo(Data) & CONNECTION_JOYSTICK) != 0)
	{
		if (!sValue.IsEmpty())
		{
			sValue += wxT(",");
		}
		sValue += wxT("Joystick");
	}
	if ((EmuDevice_ConnectedTo(Data) & CONNECTION_PRINTER) != 0)
	{
		if (!sValue.IsEmpty())
		{
			sValue += wxT(",");
		}
		sValue += wxT("Printer");
	}
	if ((EmuDevice_ConnectedTo(Data) & CONNECTION_INTERNAL) != 0)
	{
		if (!sValue.IsEmpty())
		{
			sValue += wxT(",");
		}
		sValue += wxT("Internal");
	}

	Item.SetText(sValue);
	Item.SetColumn(4);
	pListCtrl->SetItem(Item);

	bool bIsEnabled = EmuDevice_IsEnabled(Data) ? true : false;

	if (bIsEnabled)
	{
		sValue = sYes;
	}
	else
	{
		sValue = sNo;
	}
	Item.SetText(sValue);
	Item.SetColumn(5);
	pListCtrl->SetItem(Item);



}

void DevicesDialog::AddDevice(int nDevice)
{
	wxString sValue = wxT("Temp");

	wxListCtrl *pListCtrl = GetListCtrl();
	wxListItem Item;

	// name
	Item.SetMask(wxLIST_MASK_TEXT);
	Item.SetId(pListCtrl->GetItemCount());
	Item.SetText(sValue);
	Item.SetColumn(0);
	Item.SetImage(-1); //Insert a blank icon
	long nIndex = pListCtrl->InsertItem(Item);

	//    // passthrough
	//	Item.SetId(nIndex);
	//	Item.SetColumn(1);
	//	pListCtrl->SetItem(Item);

	// switches
	//	Item.SetId(nIndex);
	//	Item.SetColumn(2);
	//	pListCtrl->SetItem(Item);


	// buttons
	//	Item.SetId(nIndex);
	//	Item.SetColumn(3);
	//	pListCtrl->SetItem(Item);

	// connected to
	//	Item.SetColumn(4);
	//	pListCtrl->SetItem(Item);

	// enabled
	//	Item.SetColumn(5);
	//	pListCtrl->SetItem(Item);


	pListCtrl->SetItemPtrData(nIndex, (wxUIntPtr)nDevice);

	UpdateDevice(nIndex);
}

void DevicesDialog::PerformUpdate()
{
	wxListCtrl *pListCtrl = GetListCtrl();

	pListCtrl->Freeze();
	//pListCtrl->DeleteAllItems();
	for (int i = 0; i < pListCtrl->GetItemCount(); i++)
	{
		UpdateDevice(i);
	}

	pListCtrl->Thaw();

}

bool DevicesDialog::TransferDataFromWindow()
{

	wxGetApp().WriteConfigWindowPosAndSize(wxT("windows/devices/"), this);

	wxGetApp().WriteConfigListCtrl(wxT("windows/devices/listctrl/"), GetListCtrl());

	return true;


}

void DevicesDialog::UpdateDeviceList()
{
	wxListCtrl *pListCtrl = GetListCtrl();

	pListCtrl->Freeze();
	pListCtrl->DeleteAllItems();

	for (int i = 0; i < EmuDevice_GetNumDevices(); i++)
	{
//		const char *sName = EmuDevice_GetName(i);

		int DeviceFlags = EmuDevice_GetFlags(i);

		// ignore if it's not the appropiate device type for the connection 
		if (((EmuDevice_ConnectedTo(i) & m_ConnectionTypeFilter) == 0))
			continue;

		bool bShow = false;

		// always show enabled
		if (EmuDevice_IsEnabled(i))
			bShow = true;

		if ((DeviceFlags & DEVICE_WORKING) != 0)
			bShow = true;

		if ((DeviceFlags & DEVICE_FLAGS_TESTED) != 0)
			bShow = true;

		// if show not working and not working
		if (m_bShowNotWorking && ((DeviceFlags & DEVICE_WORKING) == 0))
			bShow = true;

		// if show not tested and not tested
		if (m_bShowUntested && ((DeviceFlags & DEVICE_FLAGS_TESTED) == 0))
			bShow = true;

		if (bShow)
		{
			AddDevice(i);
		}
	}

	pListCtrl->Thaw();
}

bool DevicesDialog::TransferDataToWindow()
{
	switch (m_ConnectionTypeFilter)
	{
	case CONNECTION_PRINTER:
	{
		SetTitle(wxT("Printer Port Devices"));
	}
	break;
	case CONNECTION_EXPANSION:
	{
		SetTitle(wxT("Expansion Port Devices"));
	}
	break;
	case CONNECTION_JOYSTICK:
	{
		SetTitle(wxT("Joystick Port Devices"));
	}
	break;
	case CONNECTION_INTERNAL:
	{
		SetTitle(wxT("Internal Devices"));
	}
	break;
	default:
		break;
	}

	wxListCtrl *pListCtrl = GetListCtrl();

	pListCtrl->Freeze();
	pListCtrl->ClearAll();

	wxListItem Column;

	Column.SetMask(wxLIST_MASK_TEXT);

	// this is the friendly name
	Column.SetText(wxT("Device"));
	pListCtrl->InsertColumn(0, Column);

	// true if it has passthrough
	Column.SetText(wxT("Has Passthrough?"));
	pListCtrl->InsertColumn(1, Column);

	// true if it has passthrough
	Column.SetText(wxT("Has Switches?"));
	pListCtrl->InsertColumn(2, Column);

	// true if it has passthrough
	Column.SetText(wxT("Has Buttons?"));
	pListCtrl->InsertColumn(3, Column);


	// connects to
	Column.SetText(wxT("Connects To"));
	pListCtrl->InsertColumn(4, Column);

	Column.SetText(wxT("Enabled?"));
	pListCtrl->InsertColumn(5, Column);

	pListCtrl->Thaw();

	UpdateDeviceList();

	wxGetApp().SetColumnSize(pListCtrl, 0);
	wxGetApp().SetColumnSize(pListCtrl, 1);
	wxGetApp().SetColumnSize(pListCtrl, 2);
	wxGetApp().SetColumnSize(pListCtrl, 3);
	wxGetApp().SetColumnSize(pListCtrl, 4);
	wxGetApp().SetColumnSize(pListCtrl, 5);

	wxGetApp().ReadConfigWindowPosAndSize(wxT("windows/devices/"), this);

	wxGetApp().ReadConfigListCtrl(wxT("windows/devices/listctrl/"), GetListCtrl());
	wxGetApp().EnsureWindowVisible(this);


	return true;
}












