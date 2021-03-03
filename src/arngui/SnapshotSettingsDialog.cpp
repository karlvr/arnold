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
#include "SnapshotSettingsDialog.h"
#include "arnguiApp.h"
#include <wx/xrc/xmlres.h>
#include <wx/checkbox.h>
#include <wx/config.h>
#include "IntClientData.h"
#include <wx/listctrl.h>
#include <wx/choice.h>

extern "C"
{
#include "../cpc/snapshot.h"
#include "../cpc/emudevice.h"
}


BEGIN_EVENT_TABLE(SnapshotSaveSettingsDialog, wxDialog)
EVT_CHECKBOX(XRCID("m_showSaveable"), SnapshotSaveSettingsDialog::ShowSaveable)
EVT_CHOICE(XRCID("m_choiceVersion"), SnapshotSaveSettingsDialog::OnVersionChanged)
END_EVENT_TABLE()

void SnapshotSaveSettingsDialog::OnVersionChanged(wxCommandEvent & WXUNUSED(checkboxEvent))
{
	wxChoice *pChoice;

	pChoice = XRCCTRL(*this, "m_choiceVersion", wxChoice);

	int nChoice = pChoice->GetSelection();
	if (nChoice != wxNOT_FOUND)
	{
		const IntClientData *pIntData = (const IntClientData *)pChoice->GetClientObject(nChoice);
		int nVersion = pIntData->GetData();
		SnapshotSettings_SetVersion(&Options, nVersion);
	}
	RefreshBlocks();
}

void SnapshotSaveSettingsDialog::ShowSaveable(wxCommandEvent & WXUNUSED(checkboxEvent))
{
	wxCheckBox *pCheckBox;

	pCheckBox = XRCCTRL(*this, "m_showSaveable", wxCheckBox);

	// get value 
	m_bShowExportable = pCheckBox->GetValue();

	// refresh list
	RefreshBlocks();
}

SnapshotSaveSettingsDialog::SnapshotSaveSettingsDialog(wxWindow *pParent)
{
	memcpy(&Options, SnapshotSettings_GetDefault(), sizeof(SNAPSHOT_OPTIONS));

	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_DIALOG_SAVE_SNAPSHOT"));
}

void SnapshotSaveSettingsDialog::RefreshBlocks()
{
	Snapshot_CollectMemory(&SnapshotMemoryBlocks, &Options, TRUE);


	wxListCtrl *pListCtrl = XRCCTRL(*this, "m_configListCtrl", wxListCtrl);
	pListCtrl->Freeze();
	pListCtrl->DeleteAllItems();

	// fill list
	for (unsigned int i = 0; i < MAX_SNAPSHOT_EXPORT_BLOCKS; i++)
	{
		SNAPSHOT_MEMORY_BLOCK *pBlock = &SnapshotMemoryBlocks.Blocks[i];

		if (m_bShowExportable && (!SnapshotMemoryBlocks.QualifyingBlocks[(i / 4)]))
			continue;

		IntClientData *pClientData = new IntClientData();
		pClientData->SetData(i);

		wxListItem Item;
		wxString sName;

		if (i >= 4)
		{
			unsigned int Config = ((((i - 4) & 0x07c) << 1) + ((i - 4) & 0x03)) | 0x0c4;
			sName = wxString::Format(wxT("%02x"), Config);
		}
		else
		{
			sName = wxString::Format(wxT("&%04x - &%04x"), (i << 14), ((i + 1) << 14) - 1);
		}

		// set name field
		Item.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_DATA);
		Item.SetText(sName);
		Item.SetColumn(0);
		Item.SetId(pListCtrl->GetItemCount());
		Item.SetData(pClientData);
		Item.SetImage(-1);
		int ItemID = pListCtrl->InsertItem(Item);

		wxString sSource = wxT("Not mapped - Not available");
		if (pBlock->bAvailable)
		{
			if (pBlock->nSourceId >= 0)
			{
				sSource = wxT("Device - ");
				sSource += wxString(EmuDevice_GetName(pBlock->nSourceId), wxConvUTF8);
			}
			else if (pBlock->nSourceId == SNAPSHOT_SOURCE_INTERNAL_ID)
			{
				sSource = wxT("Internal RAM");
			}
			else
			{

				sSource = wxT("Unknown");
			}
		}

		// set size field
		Item.SetMask(wxLIST_MASK_TEXT);
		Item.SetId(ItemID);
		Item.SetText(sSource);
		Item.SetColumn(1);
		pListCtrl->SetItem(Item);
	}
	pListCtrl->Thaw();
}

bool SnapshotSaveSettingsDialog::TransferDataToWindow()
{
	wxChoice *pChoice;
	wxCheckBox *pCheckBox;

	m_bShowExportable = false;
	pCheckBox = XRCCTRL(*this, "m_showSaveable", wxCheckBox);
	pCheckBox->SetValue(m_bShowExportable);

	{
		IntClientData *pIntData;
		pChoice = XRCCTRL(*this, "m_choiceVersion", wxChoice);
		pIntData = new IntClientData(2);
		pChoice->Append(wxT("2"), pIntData);

		pIntData = new IntClientData(3);
		pChoice->Append(wxT("3"), pIntData);
	}

	int nVersion = SnapshotSettings_GetVersion(&Options);

	int nSelection = 0;//to have the first selected if not configured
	for (unsigned int i = 0; i != pChoice->GetCount(); i++)
	{
		const IntClientData *pIntData = (const IntClientData *)pChoice->GetClientObject(i);
		if (pIntData->GetData() == nVersion)
		{
			nSelection = i;
			break;
		}
	}
	pChoice->SetSelection(nSelection);

	pCheckBox = XRCCTRL(*this, "IDC_CHECK_COMPRESSED", wxCheckBox);
	pCheckBox->SetValue(SnapshotSettings_GetCompressed(&Options) ? true : false);

	wxListCtrl *pListCtrl = XRCCTRL(*this, "m_configListCtrl", wxListCtrl);
	pListCtrl->Freeze();
	pListCtrl->ClearAll();

	wxListItem Column;
	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT("Block"));
	pListCtrl->InsertColumn(0, Column);
	Column.SetText(wxT("Device"));
	pListCtrl->InsertColumn(1, Column);
	pListCtrl->Thaw();

	RefreshBlocks();

	wxGetApp().SetColumnSize(pListCtrl, 0);
	wxGetApp().SetColumnSize(pListCtrl, 1);


	return true;
}

bool SnapshotSaveSettingsDialog::TransferDataFromWindow()
{
	wxChoice *pChoice;
	wxCheckBox *pCheckBox;

	pChoice = XRCCTRL(*this, "m_choiceVersion", wxChoice);

	int nChoice = pChoice->GetSelection();
	if (nChoice != wxNOT_FOUND)
	{
		const IntClientData *pIntData = (const IntClientData *)pChoice->GetClientObject(nChoice);
		int nVersion = pIntData->GetData();
		SnapshotSettings_SetVersion(&Options, nVersion);
		SnapshotSettings_SetVersion(SnapshotSettings_GetDefault(), nVersion);
	}

	wxConfig::Get(false)->Write(wxT("snapshot/save_version"), SnapshotSettings_GetVersion(&Options));

	pCheckBox = XRCCTRL(*this, "IDC_CHECK_COMPRESSED", wxCheckBox);
	SnapshotSettings_SetCompressed(&Options,pCheckBox->GetValue());
	SnapshotSettings_SetCompressed(SnapshotSettings_GetDefault(), pCheckBox->GetValue());

	wxConfig::Get(false)->Write(wxT("snapshot/compressed"), SnapshotSettings_GetCompressed(&Options));


	return true;
}

