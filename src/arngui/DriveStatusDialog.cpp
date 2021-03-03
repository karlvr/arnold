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
#include "DriveStatusDialog.h"
#include <wx/xrc/xmlres.h>
#include "arnguiMain.h"
#include "arnguiApp.h"
#include <wx/timer.h>

extern "C"
{
#include "../cpc/cpc.h"
#include "../cpc/fdd.h"
#include "../cpc/fdi.h"

}

static const wxChar *sYes = wxT("Yes");
static const wxChar *sNo = wxT("No");


DriveStatusListCtrl::DriveStatusListCtrl() : SortableListCtrl()
{
}

DriveStatusListCtrl::~DriveStatusListCtrl()
{
}

IMPLEMENT_DYNAMIC_CLASS(DriveStatusListCtrl, SortableListCtrl)

BEGIN_EVENT_TABLE(DriveStatusListCtrl, SortableListCtrl)
END_EVENT_TABLE()


int DriveStatusListCtrl::PerformSort(wxIntPtr item1id, wxIntPtr item2id)
{

	int result = 0;

	switch (m_nSortColumn)
	{
	default:
	case 0:
	{
		result = item1id - item2id;
	}
	break;
	case 1:
	{
		FDD *pDrive1 = FDD_GetDrive(item1id);
		FDD *pDrive2 = FDD_GetDrive(item2id);
		result = pDrive1->CurrentTrack - pDrive2->CurrentTrack;
	}
	break;
	case 2:
	{
		FDD *pDrive1 = FDD_GetDrive(item1id);
		FDD *pDrive2 = FDD_GetDrive(item2id);
		int a = ((pDrive1->Flags & FDD_FLAGS_LED_ON) != 0) ? 1 : 0;
		int b = ((pDrive2->Flags & FDD_FLAGS_LED_ON) != 0) ? 1 : 0;
		result = a - b;
	}
	break;
	case 3:
	{
		FDD *pDrive1 = FDD_GetDrive(item1id);
		FDD *pDrive2 = FDD_GetDrive(item2id);
		int a = ((pDrive1->Flags & FDD_FLAGS_DRIVE_READY) != 0) ? 1 : 0;
		int b = ((pDrive2->Flags & FDD_FLAGS_DRIVE_READY) != 0) ? 1 : 0;
		result = a - b;
	}
	break;
	case 4:
	{
		FDD *pDrive1 = FDD_GetDrive(item1id);
		FDD *pDrive2 = FDD_GetDrive(item2id);
		int a = ((pDrive1->Flags & FDD_FLAGS_WRITE_PROTECTED) != 0) ? 1 : 0;
		int b = ((pDrive2->Flags & FDD_FLAGS_WRITE_PROTECTED) != 0) ? 1 : 0;
		result = a - b;
	}
	break;
	case 5:
	{
		FDD *pDrive1 = FDD_GetDrive(item1id);
		FDD *pDrive2 = FDD_GetDrive(item2id);
		int a = ((pDrive1->Flags & FDD_FLAGS_DISK_PRESENT) != 0) ? 1 : 0;
		int b = ((pDrive2->Flags & FDD_FLAGS_DISK_PRESENT) != 0) ? 1 : 0;
		result = a - b;
	}
	break;
	case 6:
	{
		FDD *pDrive1 = FDD_GetDrive(item1id);
		FDD *pDrive2 = FDD_GetDrive(item2id);
		int a = ((pDrive1->Flags & FDD_FLAGS_DRIVE_ENABLED) != 0) ? 1 : 0;
		int b = ((pDrive2->Flags & FDD_FLAGS_DRIVE_ENABLED) != 0) ? 1 : 0;
		result = a - b;
	}
	break;
	case 7:
	{
		FDD *pDrive1 = FDD_GetDrive(item1id);
		FDD *pDrive2 = FDD_GetDrive(item2id);
		int a = ((pDrive1->Flags & FDD_FLAGS_HEAD_AT_TRACK_0) != 0) ? 1 : 0;
		int b = ((pDrive2->Flags & FDD_FLAGS_HEAD_AT_TRACK_0) != 0) ? 1 : 0;
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


void DriveStatusDialog::OnCharHook(wxKeyEvent& event)
{
	if (IsEscapeKey(event))
	{
		Close();
		return;
	}

	event.Skip();
}

DriveStatusDialog *DriveStatusDialog::m_pInstance = NULL;

DriveStatusDialog::DriveStatusDialog(wxWindow *pParent)
{
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DRIVE_STATUS_DIALOG"));
}

BEGIN_EVENT_TABLE(DriveStatusDialog, wxDialog)
EVT_CLOSE(DriveStatusDialog::OnClose)
EVT_CHAR_HOOK(DriveStatusDialog::OnCharHook)
END_EVENT_TABLE()

void DriveStatusDialog::OnClose(wxCloseEvent & WXUNUSED(event))
{
	arnguiFrame *pFrame = static_cast<arnguiFrame *>(GetParent());
	pFrame->DeRegisterWantUpdateFromTimer(this);

	wxGetApp().WriteConfigWindowPosAndSize(wxT("windows/drive_status/"), this);

	wxGetApp().WriteConfigListCtrl(wxT("windows/drive_status/listctrl/"), GetListCtrl());


	this->Destroy();

	DriveStatusDialog::m_pInstance = NULL;
}

wxListCtrl *DriveStatusDialog::GetListCtrl()
{
	return XRCCTRL(*this, "m_listCtrl5", wxListCtrl);
}


DriveStatusDialog::~DriveStatusDialog()
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


void DriveStatusDialog::UpdateI(bool bRefresh)
{
	wxListCtrl *pListCtrl = GetListCtrl();

	//  ::wxStartTimer();
	pListCtrl->Freeze();

	if (!bRefresh)
	{
		pListCtrl->DeleteAllItems();
		for (int i = 0; i < CPC_MAX_DRIVES; i++)
		{
			FDD *pDrive = FDD_GetDrive(i);
			wxString sValue;

			wxListItem Item;

			Item.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_DATA | wxLIST_MASK_IMAGE);
			Item.SetId(i);

			// id
			sValue.Printf(wxT("%d"), i);
			Item.SetText(sValue);
			Item.SetColumn(0);
			Item.SetData(i);
			Item.SetImage(-1); //Insert a blank icon, not find to insert nothing
			if (!bRefresh)
			{

				pListCtrl->InsertItem(Item);
			}
			else
			{
				pListCtrl->SetItem(Item);
			}


			// track
			sValue.Printf(wxT("%02d"), (int)pDrive->CurrentTrack);
			Item.SetText(sValue);
			Item.SetColumn(1);
			pListCtrl->SetItem(Item);

			// LED
			if (pDrive->Flags & FDD_FLAGS_LED_ON)
			{
				sValue = wxT("On");
			}
			else
			{
				sValue = wxT("Off");
			}
			Item.SetText(sValue);
			Item.SetColumn(2);
			pListCtrl->SetItem(Item);


			// ready
			if (pDrive->Flags & FDD_FLAGS_DRIVE_READY)
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

			// write protected
			if (pDrive->Flags & FDD_FLAGS_WRITE_PROTECTED)
			{
				sValue = sYes;
			}
			else
			{
				sValue = sNo;
			}
			Item.SetText(sValue);
			Item.SetColumn(4);
			pListCtrl->SetItem(Item);

			// has disk
			if (pDrive->Flags & FDD_FLAGS_DISK_PRESENT)
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

			// enabled?
			if (pDrive->Flags & FDD_FLAGS_DRIVE_ENABLED)
			{
				sValue = sYes;
			}
			else
			{
				sValue = sNo;
			}
			Item.SetText(sValue);
			Item.SetColumn(6);
			pListCtrl->SetItem(Item);
			m_DriveStatus[i].nTrack = pDrive->CurrentTrack;

			// track 0?
			if (pDrive->Flags & FDD_FLAGS_HEAD_AT_TRACK_0)
			{
				sValue = sYes;
			}
			else
			{
				sValue = sNo;
			}
			Item.SetText(sValue);
			Item.SetColumn(7);
			pListCtrl->SetItem(Item);


			m_DriveStatus[i].Flags = pDrive->Flags;
		}

	}
	else
	{
		// this will probably not work.
		// we need to use a virtual list to update it???
		wxString sValue;
		wxListItem Item;
		Item.SetMask(wxLIST_MASK_TEXT);
		for (int i = 0; i < CPC_MAX_DRIVES; i++)
		{
			long Drive = pListCtrl->GetItemData(i);

			FDD *pDrive = FDD_GetDrive(Drive);

			sValue.Empty();

			Item.SetId(i);

			if (pDrive->CurrentTrack != m_DriveStatus[Drive].nTrack)
			{
				// track
				sValue.Printf(wxT("%02d"),pDrive->CurrentTrack);
				Item.SetText(sValue);
				Item.SetColumn(1);
				pListCtrl->SetItem(Item);
			}
			int DiffFlags = pDrive->CurrentTrack^m_DriveStatus[Drive].Flags;
			if (DiffFlags & FDD_FLAGS_LED_ON)
			{

				// LED
				if (pDrive->Flags & FDD_FLAGS_LED_ON)
				{
					sValue = wxT("On");
				}
				else
				{
					sValue = wxT("Off");
				}
				Item.SetText(sValue);
				Item.SetColumn(2);
				pListCtrl->SetItem(Item);
			}

			if (DiffFlags & FDD_FLAGS_DRIVE_READY)
			{

				// ready
				if (pDrive->Flags & FDD_FLAGS_DRIVE_READY)
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

			if (DiffFlags & FDD_FLAGS_WRITE_PROTECTED)
			{

				// write protected
				if (pDrive->Flags & FDD_FLAGS_WRITE_PROTECTED)
				{
					sValue = sYes;
				}
				else
				{
					sValue = sNo;
				}
				Item.SetText(sValue);
				Item.SetColumn(4);
				pListCtrl->SetItem(Item);
			}
			if (DiffFlags & FDD_FLAGS_DISK_PRESENT)
			{
				// has disk
				if (pDrive->Flags & FDD_FLAGS_DISK_PRESENT)
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

			if (DiffFlags & FDD_FLAGS_DRIVE_ENABLED)
			{

				// enabled?
				if (pDrive->Flags & FDD_FLAGS_DRIVE_ENABLED)
				{
					sValue = sYes;
				}
				else
				{
					sValue = sNo;
				}
				Item.SetText(sValue);
				Item.SetColumn(6);
				pListCtrl->SetItem(Item);
			}
			m_DriveStatus[Drive].nTrack = pDrive->CurrentTrack;


			if (DiffFlags & FDD_FLAGS_HEAD_AT_TRACK_0)
			{

				// enabled?
				if (pDrive->Flags & FDD_FLAGS_HEAD_AT_TRACK_0)
				{
					sValue = sYes;
				}
				else
				{
					sValue = sNo;
				}
				Item.SetText(sValue);
				Item.SetColumn(7);
				pListCtrl->SetItem(Item);
			}


			m_DriveStatus[Drive].Flags = pDrive->Flags;
		}
	}
	// long nValue = ::wxGetElapsedTime(true);
	// printf("%d\n", nValue);

	pListCtrl->Thaw();



}


void DriveStatusDialog::TimedUpdate()
{
	UpdateI(true);
}

bool DriveStatusDialog::TransferDataToWindow()
{
	wxListCtrl *pListCtrl = GetListCtrl();

	pListCtrl->Freeze();
	pListCtrl->ClearAll();

	wxListItem Column;
	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT("Drive"));
	pListCtrl->InsertColumn(0, Column);
	Column.SetText(wxT("Current Track"));
	pListCtrl->InsertColumn(1, Column);
	Column.SetText(wxT("LED"));
	pListCtrl->InsertColumn(2, Column);
	Column.SetText(wxT("Ready?"));
	pListCtrl->InsertColumn(3, Column);
	Column.SetText(wxT("Write Protected?"));
	pListCtrl->InsertColumn(4, Column);
	Column.SetText(wxT("Has Disk?"));
	pListCtrl->InsertColumn(5, Column);
	Column.SetText(wxT("Enabled?"));
	pListCtrl->InsertColumn(6, Column);
	Column.SetText(wxT("Track 0?"));
	pListCtrl->InsertColumn(7, Column);
	pListCtrl->Thaw();
	UpdateI(false);

	wxGetApp().SetColumnSize(pListCtrl, 0);
	wxGetApp().SetColumnSize(pListCtrl, 1);
	wxGetApp().SetColumnSize(pListCtrl, 2);
	wxGetApp().SetColumnSize(pListCtrl, 3);
	wxGetApp().SetColumnSize(pListCtrl, 4);
	wxGetApp().SetColumnSize(pListCtrl, 5);
	wxGetApp().SetColumnSize(pListCtrl, 6);
	wxGetApp().SetColumnSize(pListCtrl, 7);


	arnguiFrame *pFrame = static_cast<arnguiFrame *>(GetParent());

	pFrame->RegisterWantUpdateFromTimer(this);

	wxGetApp().ReadConfigWindowPosAndSize(wxT("windows/drive_status/"), this);

	wxGetApp().ReadConfigListCtrl(wxT("windows/drive_status/listctrl/"), GetListCtrl());
	wxGetApp().EnsureWindowVisible(this);


	return true;
}

