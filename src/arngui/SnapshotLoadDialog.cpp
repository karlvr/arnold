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
#include "SnapshotLoadDialog.h"
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


BEGIN_EVENT_TABLE(SnapshotLoadSettingsDialog, wxDialog)
EVT_BUTTON(XRCID("m_buttonConfigure"), SnapshotLoadSettingsDialog::OnConfigure)
END_EVENT_TABLE()

void SnapshotLoadSettingsDialog::OnConfigure(wxCommandEvent & WXUNUSED(event))
{
	Snapshot_ConfigureHardware(m_pRequirements, m_pMemoryBlocks);
	RefreshDisplay();
}


SnapshotLoadSettingsDialog::SnapshotLoadSettingsDialog(wxWindow *pParent, const SNAPSHOT_REQUIREMENTS *pRequirements, SNAPSHOT_MEMORY_BLOCKS *pMemoryBlocks) : m_pRequirements(pRequirements), m_pMemoryBlocks(pMemoryBlocks)
{
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_DIALOG_LOAD_SNAPSHOT"));
}

void SnapshotLoadSettingsDialog::RefreshDisplay()
{
	wxListCtrl *pListCtrl = XRCCTRL(*this, "m_configListCtrl", wxListCtrl);
	pListCtrl->Freeze();
	pListCtrl->DeleteAllItems();


	wxListItem Item;
	wxString sName;
	{
		IntClientData *pClientData = new IntClientData();
		pClientData->SetData(-1);

		Item.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_DATA);
		Item.SetText(CPC_GetHardware() == m_pRequirements->HardwareRequired ? wxT("Yes") : wxT(""));
		Item.SetColumn(0);
		Item.SetId(pListCtrl->GetItemCount());
		Item.SetData(pClientData);
		Item.SetImage(-1);
		int ItemID = pListCtrl->InsertItem(Item);

		wxString sHardware = wxT("Unknown");

		switch (m_pRequirements->HardwareRequired)
		{
		case CPC_HW_CPCPLUS:
		{
			sHardware = wxT("Plus features");
		}
		break;
		case CPC_HW_CPC:
		{
			sHardware = wxT("CPC");
		}
		break;
		default:
			sHardware = wxT("Unknown");
			break;
		}

		// set size field
		Item.SetMask(wxLIST_MASK_TEXT);
		Item.SetId(ItemID);
		Item.SetText(sHardware);
		Item.SetColumn(1);
		pListCtrl->SetItem(Item);

		// set size field
		Item.SetMask(wxLIST_MASK_TEXT);
		Item.SetId(ItemID);
		Item.SetText(wxT("Snapshot"));
		Item.SetColumn(2);
		pListCtrl->SetItem(Item);
	}

	// fill list
	for (unsigned int i = 0; i < MAX_SNAPSHOT_EXPORT_BLOCKS; i++)
	{
		if (!m_pRequirements->BlocksRequested[i])
			continue;

		IntClientData *pClientData = new IntClientData();
		pClientData->SetData(i);


		sName = wxT("");
		if (m_pMemoryBlocks->Blocks[i].bAvailable)
		{
			sName = wxT("Yes");
		}

		Item.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_DATA);
		Item.SetText(sName);
		Item.SetColumn(0);
		Item.SetId(pListCtrl->GetItemCount());
		Item.SetData(pClientData);
		Item.SetImage(-1);
		int ItemID = pListCtrl->InsertItem(Item);

		if (i >= 4)
		{
			unsigned int Config = ((((i - 4) & 0x07c) << 1) + ((i - 4) & 0x03)) | 0x0c4;
			sName = wxString::Format(wxT("%02x"), Config);
		}
		else
		{
			sName = wxString::Format(wxT("&%04x - &%04x"), (i << 14), ((i + 1) << 14) - 1);
		}

		// set size field
		Item.SetMask(wxLIST_MASK_TEXT);
		Item.SetId(ItemID);
		Item.SetText(sName);
		Item.SetColumn(1);
		pListCtrl->SetItem(Item);

		const SNAPSHOT_MEMORY_BLOCK *pMemoryBlock = &m_pMemoryBlocks->Blocks[i];

		wxString sSource = wxT("Not mapped - Not available");
		if (pMemoryBlock->bAvailable)
		{
			if (pMemoryBlock->nSourceId >= 0)
			{
				sSource = wxT("Device - ");
				sSource += wxString(EmuDevice_GetName(pMemoryBlock->nSourceId), wxConvUTF8);
			}
			else if (pMemoryBlock->nSourceId == SNAPSHOT_SOURCE_INTERNAL_ID)
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
		Item.SetColumn(2);
		pListCtrl->SetItem(Item);
	}

	{
		sName = wxT("");
		if (!m_pRequirements->UnsupportedRamSize)
		{
			sName = wxT("Yes");
		}

		Item.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_DATA);
		Item.SetText(sName);
		Item.SetColumn(0);
		Item.SetId(pListCtrl->GetItemCount());
		Item.SetImage(-1);
		int ItemID = pListCtrl->InsertItem(Item);

		sName = wxString::Format(wxT("Total memory %s"), wxFileName::GetHumanReadableSize(m_pRequirements->RamSizeRequested, wxEmptyString, 2));
		// set size field
		Item.SetMask(wxLIST_MASK_TEXT);
		Item.SetId(ItemID);
		Item.SetText(sName);
		Item.SetColumn(1);
		pListCtrl->SetItem(Item);

		Item.SetMask(wxLIST_MASK_TEXT);
		Item.SetId(ItemID);
		Item.SetText(wxT("Snapshot"));
		Item.SetColumn(2);
		pListCtrl->SetItem(Item);
	}

	// fill list
	for (unsigned int i = 0; i < m_pRequirements->UnsupportedChunkCount; i++)
	{
		char chChunkName[5];
		chChunkName[4] = '\0';
		int ChunkName = m_pRequirements->UnsupportedChunks[i];
#ifdef CPC_LSB_FIRST
		chChunkName[3] = ((ChunkName >> 24) & 0x0ff);
		chChunkName[2] = ((ChunkName >> 16) & 0x0ff);
		chChunkName[1] = ((ChunkName >> 8) & 0x0ff);
		chChunkName[0] = ((ChunkName >> 0) & 0x0ff);
#else
		chChunkName[0] = ((ChunkName >> 24) & 0x0ff);
		chChunkName[1] = ((ChunkName >> 16) & 0x0ff);
		chChunkName[2] = ((ChunkName >> 8) & 0x0ff);
		chChunkName[3] = ((ChunkName >> 0) & 0x0ff);
#endif
		wxString sChunkName(chChunkName);

		sName = wxT("No");
		Item.SetMask(wxLIST_MASK_TEXT );
		Item.SetText(sName);
		Item.SetColumn(0);
		Item.SetId(pListCtrl->GetItemCount());
		Item.SetImage(-1);
		int ItemID = pListCtrl->InsertItem(Item);

		Item.SetMask(wxLIST_MASK_TEXT);
		Item.SetId(ItemID);
		Item.SetText(sChunkName);
		Item.SetColumn(1);
		pListCtrl->SetItem(Item);

		Item.SetMask(wxLIST_MASK_TEXT);
		Item.SetId(ItemID);
		Item.SetText(wxT("Snapshot"));
		Item.SetColumn(2);
		pListCtrl->SetItem(Item);
	}

	pListCtrl->Thaw();
}

bool SnapshotLoadSettingsDialog::TransferDataToWindow()
{
	wxString sText = wxString::Format(wxT("%d"), m_pRequirements->Version);
	wxTextCtrl *pTextCtrl;
	pTextCtrl = XRCCTRL(*this, "m_textCtrlVersion", wxTextCtrl);
	pTextCtrl->SetValue(sText);

	wxListCtrl *pListCtrl = XRCCTRL(*this, "m_configListCtrl", wxListCtrl);
	pListCtrl->Freeze();
	pListCtrl->ClearAll();

	wxListItem Column;
	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT(""));
	pListCtrl->InsertColumn(0, Column);
	Column.SetText(wxT("Requirement"));
	pListCtrl->InsertColumn(1, Column);
	Column.SetText(wxT("From"));
	pListCtrl->InsertColumn(2, Column);
	pListCtrl->Thaw();

	RefreshDisplay();

	wxGetApp().SetColumnSize(pListCtrl, 0);
	wxGetApp().SetColumnSize(pListCtrl, 1);

	return true;
}

bool SnapshotLoadSettingsDialog::TransferDataFromWindow()
{

	return true;
}

