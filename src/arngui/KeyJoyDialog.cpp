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
#include "KeyJoyDialog.h"
#include <wx/xrc/xmlres.h>
#include "arnguiApp.h"
#include <wx/menu.h>
#include <wx/radiobut.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include "IntClientData.h"

extern "C"
{
#include "../cpc/cpc.h"
#include "../cpc/joystick.h"
#include "../cpc/keyjoy.h"
}

KeyJoyItem::KeyJoyItem(int DeviceIndex, int Code, int Index, int SubValue)
{
	m_DeviceIndex = DeviceIndex;
	m_Code = Code;
	m_Index = Index;
	m_SubValue = SubValue;
}

void KeyJoyItem::ClearMapping()
{
	SetMapping(-1);
}

void KeyJoyItem::SetMapping(int Mapping)
{
	switch (m_Code)
	{
	case CODE_AXIS:
	{
		KeyJoy_SetMappingJoyToKeyAxis(m_Index, (KEYJOY_AXIS_VALUE)m_SubValue, Mapping);
	}
	break;

	case CODE_HAT:
	{
		int axis = m_SubValue / KEYJOY_AXIS_MAX;
		KEYJOY_AXIS_VALUE value = (KEYJOY_AXIS_VALUE)(m_SubValue % KEYJOY_AXIS_MAX);

		KeyJoy_SetMappingJoyToKeyHat(m_Index, axis, value, Mapping);
	}
	break;

	case CODE_BUTTON:
	{
		KeyJoy_SetMappingJoyToKeyButton(m_Index, Mapping);
	}
	break;

	default:
		break;
	}
}

// get what the input is mapped to
wxString KeyJoyItem::GetJoyKeyMapping()
{
	wxString sMapping;

	sMapping = wxT("Not mapped");

	int nMapping = -1;
	switch (m_Code)
	{
	case CODE_AXIS:
	{
		nMapping = KeyJoy_GetMappingJoyToKeyAxis(m_Index, (KEYJOY_AXIS_VALUE)m_SubValue);
	}
	break;

	case CODE_HAT:
	{
		int axis = m_SubValue / KEYJOY_AXIS_MAX;
		KEYJOY_AXIS_VALUE value = (KEYJOY_AXIS_VALUE)(m_SubValue % KEYJOY_AXIS_MAX);

		nMapping = KeyJoy_GetMappingJoyToKeyHat(m_Index, axis, value);
	}
	break;

	case CODE_BUTTON:
	{
		nMapping = KeyJoy_GetMappingJoyToKeyButton(m_Index);
	}
	break;

	default:
		break;
	}

	if (nMapping != -1)
	{
		if (nMapping < SPECIALACTIONID)
		{
			sMapping = wxString(CPC_GetKeyName((CPC_KEY_ID)nMapping), wxConvUTF8);
		}
		else
		{
			sMapping = wxString(wxGetApp().GetActionCodeName((ACTION_CODE)(nMapping - SPECIALACTIONID)), wxConvUTF8);
		}
	}
	return sMapping;
}

const wxChar *sAxis[2] =
{
	wxT("Min"),
	wxT("Max")
};

const wxChar *sHatDirections[4] =
{
	wxT("Left"),
	wxT("Right"),
	wxT("Up"),
	wxT("Down")
};

wxString KeyJoyItem::GetLabel()
{
	wxString sName;

	switch (m_Code)
	{
	case CODE_AXIS:
	{
		sName = wxString::Format(wxT("Axis %d (%s)"), m_Index, sAxis[m_SubValue & 0x01]);
	}
	break;

	case CODE_HAT:
	{
		sName = wxString::Format(wxT("POV %d (%s)"), m_Index, sHatDirections[m_SubValue & 0x03]);
	}
	break;

	case CODE_BUTTON:
	{
		sName = wxGetApp().m_PlatformSpecific.GetJoystickButtonName(m_DeviceIndex, m_Index);
	}
	break;

	default:
		break;
	}

	return sName;
}


// is the input mapped to a joystick button instead?
bool KeyJoyItem::MappedToJoystick()
{
	switch (m_Code)
	{
	case CODE_AXIS:
		return
			(
			(Joystick_GetAxisMappingPhysical(m_DeviceIndex, 0) == m_Index) ||
			(Joystick_GetAxisMappingPhysical(m_DeviceIndex, 1) == m_Index));

		//   case CODE_HAT:
		//      return (Joystick_GetHatMappingPhysical(m_DeviceIndex, m_Index)!=-1);

	case CODE_BUTTON:
		return (Joystick_GetButtonMapping(m_DeviceIndex, m_Index) != -1);
	}
	return false;
}

WX_DEFINE_ARRAY_INT(int, ArrayOfInts);

BEGIN_EVENT_TABLE(KeyJoyDialog, wxDialog)
EVT_LISTBOX(XRCID("IDC_LIST_JOY"), KeyJoyDialog::SelJoy)
EVT_BUTTON(XRCID("m_redefinie"), KeyJoyDialog::Redefine)
EVT_BUTTON(XRCID("m_clear"), KeyJoyDialog::Clear)
EVT_BUTTON(XRCID("m_Set_Preset"), KeyJoyDialog::Preset)
EVT_RADIOBUTTON(XRCID("m_normal"), KeyJoyDialog::OnRadio)
EVT_RADIOBUTTON(XRCID("m_special"), KeyJoyDialog::OnRadio)
END_EVENT_TABLE()


KeyJoyDialog::KeyJoyDialog(wxWindow *pParent)
{
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_DIALOG_KEYJOY"));

	//Memorise pListCtrl
	pListCtrl = XRCCTRL(*this, "m_list", wxListCtrl);

	//memorise joystick
	m_nJoystickID = KeyJoy_GetPhysical();
}

KeyJoyDialog::~KeyJoyDialog()
{

}

void KeyJoyDialog::OnRadio(wxCommandEvent & WXUNUSED(event))
{
	RefreshRadio();
}

void KeyJoyDialog::RefreshRadio()
{
	wxRadioButton *pRadioButton;

	wxChoice *pChoiceN;
	wxChoice *pChoiceS;

	pChoiceN = XRCCTRL(*this, "m_normal_choice", wxChoice);
	pChoiceS = XRCCTRL(*this, "m_special_choice", wxChoice);

	pRadioButton = XRCCTRL(*this, "m_special", wxRadioButton);

	if (pRadioButton->GetValue())
	{
		pChoiceN->Disable();
		pChoiceS->Enable();
	}
	else
	{
		pChoiceS->Disable();
		pChoiceN->Enable();
	}

}

void KeyJoyDialog::Preset(wxCommandEvent& WXUNUSED(event))
{

	int choice = GetChoice(XRCID("m_ChoiceSet"));
	/* setup common ones that games use */

	switch (choice)
	{

	case 0:
		KeyJoy_SetMappingJoyToKeyAxis(1, KEYJOY_AXIS_MIN, CPC_KEY_Q);
		KeyJoy_SetMappingJoyToKeyAxis(1, KEYJOY_AXIS_MAX, CPC_KEY_A);
		KeyJoy_SetMappingJoyToKeyAxis(0, KEYJOY_AXIS_MIN, CPC_KEY_O);
		KeyJoy_SetMappingJoyToKeyAxis(0, KEYJOY_AXIS_MAX, CPC_KEY_P);

		KeyJoy_SetMappingJoyToKeyAxis(3, KEYJOY_AXIS_MIN, CPC_KEY_Q);
		KeyJoy_SetMappingJoyToKeyAxis(3, KEYJOY_AXIS_MAX, CPC_KEY_A);
		KeyJoy_SetMappingJoyToKeyAxis(2, KEYJOY_AXIS_MIN, CPC_KEY_O);
		KeyJoy_SetMappingJoyToKeyAxis(2, KEYJOY_AXIS_MAX, CPC_KEY_P);

		//        KeyJoy_SetMappingJoyToKeyHat(0,1,KEYJOY_AXIS_MIN,CPC_KEY_Q);
		//			 KeyJoy_SetMappingJoyToKeyHat(0,1,KEYJOY_AXIS_MAX,CPC_KEY_A);
		//			 KeyJoy_SetMappingJoyToKeyHat(0,0,KEYJOY_AXIS_MIN,CPC_KEY_O);
		//			 KeyJoy_SetMappingJoyToKeyHat(0,0,KEYJOY_AXIS_MAX,CPC_KEY_P);



		KeyJoy_SetMappingJoyToKeyButton(0, CPC_KEY_SPACE);
		KeyJoy_SetMappingJoyToKeyButton(1, CPC_KEY_SPACE);
		KeyJoy_SetMappingJoyToKeyButton(2, CPC_KEY_SPACE);
		KeyJoy_SetMappingJoyToKeyButton(3, CPC_KEY_SPACE);
		break;

	case 1:
		/* axis to up/down/left/right */
		KeyJoy_SetMappingJoyToKeyAxis(1, KEYJOY_AXIS_MIN, CPC_KEY_CURSOR_UP);
		KeyJoy_SetMappingJoyToKeyAxis(1, KEYJOY_AXIS_MAX, CPC_KEY_CURSOR_DOWN);
		KeyJoy_SetMappingJoyToKeyAxis(0, KEYJOY_AXIS_MIN, CPC_KEY_CURSOR_LEFT);
		KeyJoy_SetMappingJoyToKeyAxis(0, KEYJOY_AXIS_MAX, CPC_KEY_CURSOR_RIGHT);

		//    	      KeyJoy_SetMappingJoyToKeyHat(0,1,KEYJOY_AXIS_MIN,CPC_KEY_CURSOR_UP);
		//			 KeyJoy_SetMappingJoyToKeyHat(0,1,KEYJOY_AXIS_MAX,CPC_KEY_CURSOR_DOWN);
		//			 KeyJoy_SetMappingJoyToKeyHat(0,0,KEYJOY_AXIS_MIN,CPC_KEY_CURSOR_LEFT);
		//			 KeyJoy_SetMappingJoyToKeyHat(0,0,KEYJOY_AXIS_MAX,CPC_KEY_CURSOR_RIGHT);




		KeyJoy_SetMappingJoyToKeyAxis(3, KEYJOY_AXIS_MIN, CPC_KEY_CURSOR_UP);
		KeyJoy_SetMappingJoyToKeyAxis(3, KEYJOY_AXIS_MAX, CPC_KEY_CURSOR_DOWN);
		KeyJoy_SetMappingJoyToKeyAxis(2, KEYJOY_AXIS_MIN, CPC_KEY_CURSOR_LEFT);
		KeyJoy_SetMappingJoyToKeyAxis(2, KEYJOY_AXIS_MAX, CPC_KEY_CURSOR_RIGHT);
		/* buttons to space */
		KeyJoy_SetMappingJoyToKeyButton(0, CPC_KEY_SPACE);
		KeyJoy_SetMappingJoyToKeyButton(1, CPC_KEY_SPACE);
		KeyJoy_SetMappingJoyToKeyButton(2, CPC_KEY_SPACE);
		KeyJoy_SetMappingJoyToKeyButton(3, CPC_KEY_SPACE);
		break;
	}

	RefreshList();

}
int KeyJoyDialog::GetChoice(long WinID)
{
	wxChoice *pChoice;
	pChoice = static_cast<wxChoice *>(this->FindWindow(WinID));
	int nSelection = pChoice->GetSelection();
	if (nSelection == wxNOT_FOUND)
	{
		return -1;
	}

	const IntClientData *pIntData = (const IntClientData *)pChoice->GetClientObject(nSelection);
	if (pIntData == NULL)
	{
		return -1;
	}

	return pIntData->GetData();
}

void KeyJoyDialog::Clear(wxCommandEvent& WXUNUSED(event))
{
	//Get button number
	long itemIndex = pListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	if (itemIndex == -1) return;

	KeyJoyItem *pItem = (KeyJoyItem *)pListCtrl->GetItemData(itemIndex);
	pItem->ClearMapping();
	RefreshItem(itemIndex);

}

void KeyJoyDialog::RefreshItem(long item)
{
	KeyJoyItem *pItem = (KeyJoyItem *)pListCtrl->GetItemData(item);

	wxListItem Item;
	Item.SetId(item);
	Item.SetMask(wxLIST_MASK_TEXT);

	Item.SetText(pItem->GetLabel());
	Item.SetColumn(0);
	pListCtrl->SetItem(Item);

	Item.SetText(pItem->GetJoyKeyMapping());
	Item.SetColumn(1);
	pListCtrl->SetItem(Item);

	Item.SetText(pItem->MappedToJoystick() ? wxT("Yes") : wxT("No"));
	Item.SetColumn(2);
	pListCtrl->SetItem(Item);
}

void KeyJoyDialog::Redefine(wxCommandEvent& WXUNUSED(event))
{
	//Get button number
	long itemIndex = pListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	if (itemIndex == -1) return;

	//Get normal or special
	int Function;
	wxRadioButton *pRadioButton;
	pRadioButton = XRCCTRL(*this, "m_normal", wxRadioButton);
		
	if (pRadioButton->GetValue())
	{
		Function = GetChoice(XRCID("m_normal_choice"));
	}
	else
	{
		Function = SPECIALACTIONID + GetChoice(XRCID("m_special_choice"));
	}

	if (Function == -1) return;


	KeyJoyItem *pItem = (KeyJoyItem *)pListCtrl->GetItemData(itemIndex);
	pItem->SetMapping(Function);
	RefreshItem(itemIndex);
}

void KeyJoyDialog::SelJoy(wxCommandEvent& WXUNUSED(event))
{
	wxListBox *pListBox;
	pListBox = XRCCTRL(*this, "IDC_LIST_JOY", wxListBox);
	int nJoystickID = -1;

	// find selected joystick
	int SelectedItem = pListBox->GetSelection();
	if (SelectedItem != wxNOT_FOUND)
	{
		const IntClientData *pIntData = (const IntClientData *)pListBox->GetClientObject(SelectedItem);
		nJoystickID = pIntData->GetData();
	}
	m_nJoystickID = nJoystickID;

	//memorise and refreshjoystick
	KeyJoy_SetPhysical(nJoystickID);
	wxGetApp().m_PlatformSpecific.RefreshJoysticks();

	//refresh Remap list
	RefreshList();
}

bool KeyJoyDialog::TransferDataFromWindow()
{
	wxCheckBox *pCheckBox;
	wxListBox *pListBox;

	//KeyJoy active or not ?
	pCheckBox = XRCCTRL(*this, "m_checkBoxKeyJoy", wxCheckBox);
	KeyJoy_SetActive(pCheckBox->GetValue());

	wxConfig::Get(false)->Write(wxT("keyjoy/enabled"), (KeyJoy_IsActive() == TRUE));
	printf("KeyJoy is : %s\n", KeyJoy_IsActive() ? "enabled" : "disabled");

	// set selected joystick even if we don't use this mode so it can be restored next time dialog is shown
	pListBox = XRCCTRL(*this, "IDC_LIST_JOY", wxListBox);

	int nJoystickID = -1;

	// find selected joystick
	int SelectedItem = pListBox->GetSelection();
	if (SelectedItem != wxNOT_FOUND)
	{
		const IntClientData *pIntData = (const IntClientData *)pListBox->GetClientObject(SelectedItem);
		nJoystickID = pIntData->GetData();
	}
	KeyJoy_SetPhysical(nJoystickID);

	wxString sId = wxGetApp().m_PlatformSpecific.GetJoystickIdString(KeyJoy_GetPhysical());
	wxConfig::Get(false)->Write(wxT("keyjoy/id"), sId);


	wxString sKeyID;
	for (int i = 0; i < MAXREDEFBUTTON; i++)
	{
		sKeyID.Printf(wxT("ButtonID%d"), i);
		wxConfig::Get(false)->Write(wxT("keyjoy/Remap/") + sKeyID, KeyJoy_GetMappingJoyToKeyButton(i));
	}

	for (int i = 0; i < MAXREDEFAXIS; i++)
	{
		sKeyID.Printf(wxT("AxisID%dMin"), i);
		wxConfig::Get(false)->Write(wxT("keyjoy/Remap/") + sKeyID, KeyJoy_GetMappingJoyToKeyAxis(i, KEYJOY_AXIS_MIN));
		sKeyID.Printf(wxT("AxisID%dMax"), i);
		wxConfig::Get(false)->Write(wxT("keyjoy/Remap/") + sKeyID, KeyJoy_GetMappingJoyToKeyAxis(i, KEYJOY_AXIS_MAX));
	}

	for (int i = 0; i < MAXREDEFHAT; i++)
	{
		sKeyID.Printf(wxT("HatID%dXMin"), i);
		wxConfig::Get(false)->Write(wxT("keyjoy/Remap/") + sKeyID, KeyJoy_GetMappingJoyToKeyHat(i, 0, KEYJOY_AXIS_MIN));
		sKeyID.Printf(wxT("HatID%dXMax"), i);
		wxConfig::Get(false)->Write(wxT("keyjoy/Remap/") + sKeyID, KeyJoy_GetMappingJoyToKeyHat(i, 0, KEYJOY_AXIS_MAX));
		sKeyID.Printf(wxT("HatID%dYMin"), i);
		wxConfig::Get(false)->Write(wxT("keyjoy/Remap/") + sKeyID, KeyJoy_GetMappingJoyToKeyHat(i, 1, KEYJOY_AXIS_MIN));
		sKeyID.Printf(wxT("HatID%dYMax"), i);
		wxConfig::Get(false)->Write(wxT("keyjoy/Remap/") + sKeyID, KeyJoy_GetMappingJoyToKeyHat(i, 1, KEYJOY_AXIS_MAX));
	}

	return true;
}

void KeyJoyDialog::FillListBox()
{
	wxListBox *pListBox;

	pListBox = XRCCTRL(*this, "IDC_LIST_JOY", wxListBox);

	// find which was selected?
	pListBox->Freeze();
	pListBox->Clear();
	wxGetApp().m_PlatformSpecific.PopulateJoystickDialog(pListBox);
	pListBox->Thaw();

	int nID = wxNOT_FOUND;

	// select the joystick regardless of whether it's a physical or not so we can see the selection
	int PhysicalJoystickID = KeyJoy_GetPhysical();
	for (unsigned int i = 0; i != pListBox->GetCount(); i++)
	{
		IntClientData *pIntData = (IntClientData *)pListBox->GetClientObject(i);
		if (pIntData->GetData() == PhysicalJoystickID)
		{
			nID = i;
			break;
		}
	}
	pListBox->SetSelection(nID);

}

void KeyJoyDialog::RefreshList(void)
{
	pListCtrl->Freeze();
	pListCtrl->DeleteAllItems();

	if (m_nJoystickID != -1)
	{

		wxListItem Item;
		Item.SetMask(wxLIST_MASK_TEXT);
		int curID = 0;

		for (int i = 0; i < KeyJoyItem::CODE_MAX; i++)
		{
			int num = 0;
			int numItems = 0;
			switch (i)
			{
			case KeyJoyItem::CODE_AXIS:
				num = wxGetApp().m_PlatformSpecific.GetJoystickNumAxis(m_nJoystickID);
				numItems = 2;   // min and max
				break;
			case KeyJoyItem::CODE_HAT:
				num = wxGetApp().m_PlatformSpecific.GetJoystickNumPOV(m_nJoystickID);
				numItems = 4; // up, down, left, right
				break;
			case KeyJoyItem::CODE_BUTTON:
				num = wxGetApp().m_PlatformSpecific.GetJoystickNumButtons(m_nJoystickID);
				numItems = 1; // the button itself
				break;

			default:
				break;
			}

			for (int j = 0; j < num; j++)
			{
				for (int k = 0; k < numItems; k++)
				{
					KeyJoyItem *pItem = new KeyJoyItem(m_nJoystickID, i, j, k);

					Item.SetId(curID);
					Item.SetText(wxT(""));
					Item.SetColumn(0);
					Item.SetImage(-1); //Insert a blank icon, not find to insert nothing

					long index = pListCtrl->InsertItem(Item);
					pListCtrl->SetItemPtrData(index, (wxUIntPtr)pItem);

					Item.SetId(index);
					Item.SetText(wxT(""));
					Item.SetColumn(2);
					pListCtrl->SetItem(Item);

					curID++;

					RefreshItem(index);
				}
			}
		}
	}
	pListCtrl->Thaw();


}


bool KeyJoyDialog::TransferDataToWindow()
{
	wxCheckBox *pCheckBox;
	wxChoice *pChoice;
	IntClientData *pClientData;

	pCheckBox = XRCCTRL(*this, "m_checkBoxKeyJoy", wxCheckBox);
	pCheckBox->SetValue(KeyJoy_IsActive() ? true : false);

	//Preset choice
	pChoice = XRCCTRL(*this, "m_ChoiceSet", wxChoice);
	pClientData = new IntClientData(0);
	pChoice->Append(wxT("Q,A,O,P,Space"), pClientData);
	pClientData = new IntClientData(1);
	pChoice->Append(wxT("Cursor Keys,Space"), pClientData);
	pChoice->Select(0);

	//Fill joystick Box
	FillListBox();

	//refresh key list
	pListCtrl->Freeze();
	pListCtrl->ClearAll();

	wxListItem Column;
	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT("Command"));
	pListCtrl->InsertColumn(0, Column);
	//	pListCtrl->SetColumnWidth(0, 80);
	Column.SetText(wxT("Key"));
	pListCtrl->InsertColumn(1, Column);
	//  	pListCtrl->SetColumnWidth(1, 150);
	Column.SetText(wxT("Used by Joy"));
	pListCtrl->InsertColumn(2, Column);
	// 	pListCtrl->SetColumnWidth(2, 100);
	pListCtrl->Thaw();

	RefreshList();


	wxGetApp().SetColumnSize(pListCtrl, 0);
	wxGetApp().SetColumnSize(pListCtrl, 1);
	wxGetApp().SetColumnSize(pListCtrl, 2);

	//fill combo special
	pChoice = XRCCTRL(*this, "m_special_choice", wxChoice);

	pChoice->Freeze();
	pChoice->Clear();

	for (int i = 0; i < ACTION_CODE_NUM; i++)
	{
		pClientData = new IntClientData(i);
		int SortedIndex = pChoice->Append(wxString(wxGetApp().GetActionCodeName((ACTION_CODE)i), wxConvUTF8));
		pChoice->SetClientObject(SortedIndex, pClientData);
	}
	pChoice->Thaw();
	pChoice->Select(0);

	//fill combo normal
	pChoice = XRCCTRL(*this, "m_normal_choice", wxChoice);
	pChoice->Freeze();
	pChoice->Clear();

	for (int i = 0; i < CPC_KEY_NUM_KEYS; i++)
	{
		pClientData = new IntClientData(i);
		pChoice->Append(wxString(CPC_GetKeyName((CPC_KEY_ID)i), wxConvUTF8), pClientData);
	}
	pChoice->Thaw();
	pChoice->Select(0);


	//Refresh Radio
	wxRadioButton *pRadioButton;
	pRadioButton = XRCCTRL(*this, "m_normal", wxRadioButton);
	pRadioButton->SetValue(true);
	RefreshRadio();

	return true;

}
