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
#include "ArchiveDialog.h"
#include <wx/xrc/xmlres.h>
#include "arnguiApp.h"
#include <wx/wfstream.h>

ArchiveListCtrl::ArchiveListCtrl() : SortableListCtrl()
{
}

ArchiveListCtrl::~ArchiveListCtrl()
{
}



int ArchiveListCtrl::PerformSort(wxIntPtr item1id, wxIntPtr item2id)
{
	ArchiveListItem *pClientData1 = (ArchiveListItem *)item1id;
	ArchiveListItem *pClientData2 = (ArchiveListItem *)item2id;

	int result = 0;
	switch (m_nSortColumn)
	{
	default:
	case 0:
	{
		// name.

		// directory comes first
		if (pClientData1->m_bIsDirectory && !pClientData2->m_bIsDirectory)
		{
			result = -1;
		}
		else if (!pClientData1->m_bIsDirectory && pClientData2->m_bIsDirectory)
		{
			result = 1;
		}
		else
		{

			// a>b
			// a==b
			// a<b
#if defined(__WXMSW__) || defined(__WXMAC__)
			// case is not important
			result = pClientData1->m_sDisplayName.CmpNoCase(pClientData2->m_sDisplayName);
#else
			// case is important
			result = pClientData1->m_sDisplayName.Cmp(pClientData2->m_sDisplayName);
#endif
		}
	}
	break;

	case 1:
	{
		// size
		if ((!pClientData1->m_bIsDirectory) && (!pClientData2->m_bIsDirectory))
		{
			// files
			result = pClientData1->m_nSize - pClientData2->m_nSize;
		}
		else
		{
			result = 0;
		}
	}
	break;
	case 2:
	{

		// directory comes first
		if (pClientData1->m_bIsDirectory && !pClientData2->m_bIsDirectory)
		{
			result = -1;
		}
		else if (!pClientData1->m_bIsDirectory && pClientData2->m_bIsDirectory)
		{
			result = 1;
		}
		else
		{
			// both files
			result = 0;
		}
	}
	break;

	}

	if (!m_bSortAscending)
	{
		result = -result;
	}
	return result;
}

IMPLEMENT_DYNAMIC_CLASS(ArchiveListCtrl, SortableListCtrl)

BEGIN_EVENT_TABLE(ArchiveListCtrl, SortableListCtrl)
END_EVENT_TABLE()

// there is a leak in this dialog and I can't find where it is.
// seems to hold onto some strings on the items?

BEGIN_EVENT_TABLE(ArchiveDialog, wxDialog)
EVT_LIST_ITEM_ACTIVATED(XRCID("IDC_LIST_FILES"), ArchiveDialog::OnItemActivated)
EVT_BUTTON(XRCID("wxID_OK"), ArchiveDialog::OnOK)
END_EVENT_TABLE()

ArchiveDialog::~ArchiveDialog()
{
	wxListCtrl *pListCtrl = GetListCtrl();

	// free item data
	ClearItems();
	// free all items and columns
	pListCtrl->ClearAll();
}

wxListCtrl *ArchiveDialog::GetListCtrl()
{
	return XRCCTRL(*this, "IDC_LIST_FILES", wxListCtrl);
}

void ArchiveDialog::HandlePickedItem(const wxListItem &Item)
{
	const ArchiveListItem *pClientData = (const ArchiveListItem *)(Item.GetData());

	bool IsBackDir = false;
	bool IsDir = pClientData->m_bIsDirectory;
	if (pClientData->m_sInternalName.CompareTo(wxT("..")) == 0)
	{
		IsBackDir = true;
	}

	if (IsDir)
	{
		bool bPopulate = false;
		if (IsBackDir)
		{
			// go back up a dir

			if (m_sBasePath.IsEmpty())
			{
				// indicate we want to back out
				m_sPickedFilename.Empty();

				// and return ok status
				return EndModal(wxID_OK);
			}
			else
			{
				wxFileName Filename(m_sBasePath);
				Filename.RemoveLastDir();
				m_sBasePath = Filename.GetFullPath();
				bPopulate = true;
			}
		}
		else
		{
			m_sBasePath = pClientData->m_sInternalName;
			bPopulate = true;
		}

		if (bPopulate)
		{
			FillList();
		}
	}
	else
	{
		wxArchiveClassFactory *pArchiveClassFactory;
		wxArchiveInputStream *pInputStream;
		wxFilterClassFactory *pFilterClassFactory;
		GetArchiveClassFactory(m_sFilename, &pFilterClassFactory, &pArchiveClassFactory, &pInputStream);
		delete pInputStream;
		// generate path
		m_sPickedFilename = m_sFilename;
		if (pFilterClassFactory != NULL)
		{
			m_sPickedFilename += wxT("#");
			m_sPickedFilename += pFilterClassFactory->GetProtocol();
			m_sPickedFilename += wxT(":");
		}
		m_sPickedFilename += wxT("#");
		m_sPickedFilename += pArchiveClassFactory->GetProtocol();
		m_sPickedFilename += wxT(":");
		m_sPickedFilename += pClientData->m_sInternalName;
		//delete pArchiveClassFactory;
		//delete pFilterClassFactory;

		// file selected, return as if we had clicked ok
		return EndModal(wxID_OK);
	}
}

void ArchiveDialog::OnItemActivated(wxListEvent& event)
{
	const wxListItem &Item = event.GetItem();
	HandlePickedItem(Item);
}


bool ArchiveDialog::TransferDataToWindow()
{
	wxListCtrl *pListCtrl = GetListCtrl();

	pListCtrl->Freeze();
	pListCtrl->ClearAll();

	wxListItem Column;
	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT("Name"));
	pListCtrl->InsertColumn(0, Column);
	Column.SetText(wxT("Size"));
	pListCtrl->InsertColumn(1, Column);
	Column.SetText(wxT("Type"));
	pListCtrl->InsertColumn(2, Column);
	pListCtrl->Thaw();

	FillList();

	wxGetApp().SetColumnSize(pListCtrl, 0);
	wxGetApp().SetColumnSize(pListCtrl, 1);
	wxGetApp().SetColumnSize(pListCtrl, 2);

	// then restore actual size
	wxGetApp().ReadConfigWindowSize(wxT("windows/archive/"), this);

	wxGetApp().ReadConfigListCtrl(wxT("windows/archive/listctrl/"), pListCtrl);
	wxGetApp().EnsureWindowVisible(this);


	return true;
}

void ArchiveDialog::OnOK(wxCommandEvent& WXUNUSED(event))
{
	wxListCtrl *pListCtrl = GetListCtrl();

	wxGetApp().WriteConfigWindowSize(wxT("windows/archive/"), this);

	wxGetApp().WriteConfigListCtrl(wxT("windows/archive/listctrl/"), pListCtrl);

	// must be something selected and it must be a file in order
	// for dialog to quit
	long item = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item == -1)
		return;


	wxListItem Item;
	Item.SetMask(wxLIST_MASK_DATA);
	Item.SetColumn(0);
	Item.SetImage(-1); //Insert a blank icon, not find to insert nothing
	Item.SetId(item);
	if (pListCtrl->GetItem(Item))
	{
		HandlePickedItem(Item);
	}
}

void ArchiveDialog::AddItemToList(wxListCtrl *pListCtrl, const wxString &sName, const wxString &sInternalName, wxFileOffset nSize, bool bIsDirectory)
{
	ArchiveListItem *pClientData = new ArchiveListItem();
	pClientData->m_sInternalName = sInternalName;
	pClientData->m_bIsDirectory = bIsDirectory;
	pClientData->m_nSize = nSize;
	if (bIsDirectory)
	{
		pClientData->m_sDisplayName = wxT("[") + sName + wxT("]");
	}
	else
	{
		pClientData->m_sDisplayName = sName;
	}
	wxListItem Item;

	// set name field
	Item.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_DATA);
	Item.SetText(pClientData->m_sDisplayName);
	Item.SetColumn(0);
	Item.SetId(pListCtrl->GetItemCount());
	Item.SetImage(-1);
	Item.SetData(pClientData);
	int ItemId = pListCtrl->InsertItem(Item);

	// set size field
	Item.SetMask(wxLIST_MASK_TEXT);
	Item.SetId(ItemId);
	if (bIsDirectory)
	{
		// no size for directory
		Item.SetText(wxEmptyString);
	}
	else
	{
		// size of file
		Item.SetText(wxFileName::GetHumanReadableSize(nSize, wxEmptyString, 2));
	}
	Item.SetColumn(1);
	pListCtrl->SetItem(Item);

	Item.SetMask(wxLIST_MASK_TEXT);
	Item.SetText(bIsDirectory ? wxT("Directory") : wxT("File"));
	Item.SetColumn(2);
	pListCtrl->SetItem(Item);
}

void ArchiveDialog::ClearItems()
{
	wxListCtrl *pListCtrl = GetListCtrl();

	for (int i = 0; i < pListCtrl->GetItemCount(); i++)
	{
		ArchiveListItem *pClientData = (ArchiveListItem *)(pListCtrl->GetItemData(i));
		delete pClientData;
	}

	pListCtrl->DeleteAllItems();
}

void ArchiveDialog::FillListI()
{
	wxListCtrl *pListCtrl = GetListCtrl();

	ClearItems();

	// base
	AddItemToList(pListCtrl, wxT(".."), wxT(".."), 0, true);

	wxArchiveClassFactory *pArchiveClassFactory;
	wxFilterClassFactory *pFilterClassFactory;
	wxArchiveInputStream *pArchiveInputStream;
	ArchiveDialog::GetArchiveClassFactory(m_sFilename, &pFilterClassFactory, &pArchiveClassFactory, &pArchiveInputStream);
	if (pArchiveInputStream == NULL)
		return;

	// a zip file may not have a directory specified separately, it may be specified
	// implicitly through a file being in there or through a sub-directory.
	wxStringToStringHashMap Folders;

	// this goes through all items and it's a bit slow.
	wxArchiveEntry *pEntry = pArchiveInputStream->GetNextEntry();
	while (pEntry!=NULL)
	{
		// get internal name
		wxString sInternalName = pEntry->GetName();

		bool bInclude = false;
		// base path defined?
		if (m_sBasePath.Length() != 0)
		{
			// the name is at least as long as the base path
			if (sInternalName.Length() >= m_sBasePath.Length())
			{
				// see if it begins with the path we are interested in
				wxString sCommonSubString = sInternalName.Left(m_sBasePath.Length());
#if wxUSE_UNICODE==1
				const wchar_t *ch = m_sBasePath.wc_str();
#else
				const char *ch = m_sBasePath.c_str();
#endif

				if (sCommonSubString.CompareTo(ch) == 0)
				{
					bInclude = true;
				}
			}
		}
		else
		{
			bInclude = true;
		}

		if (bInclude)
		{
			wxString sName = sInternalName.Right(sInternalName.Length() - m_sBasePath.Length());

			wxFileName FileName(sName);

			// add the first dir to the list if it's not already there.
			if (FileName.GetDirCount() != 0)
			{
				const wxArrayString &sDirs = FileName.GetDirs();
				sName = sDirs[0];
				wxStringToStringHashMap::const_iterator iterator = Folders.find(sName);
				if (iterator==Folders.end())
				{
					wxFileName DirName(sInternalName);

					wxString sDirName = DirName.GetPathWithSep();
					Folders[sName] = sDirName;
				}
			}
			else if (!FileName.IsDir())
			{
				// a file in this directory
				sName = FileName.GetFullName();

				// get size of file
				wxFileOffset nSize = pEntry->GetSize();

				// add to list
				AddItemToList(pListCtrl, sName, sInternalName, nSize, false);
			}
		}
		delete pEntry;

		pEntry = pArchiveInputStream->GetNextEntry();
	}
	// add all folders discovered
	wxStringToStringHashMap::iterator iterator = Folders.begin();
	for (;iterator!=Folders.end(); ++iterator)
	{
		AddItemToList(pListCtrl, iterator->first, iterator->second, 0, true);
	}

	delete pArchiveInputStream;

	//delete pArchiveClassFactory;
	//delete pFilterClassFactory;
}

void ArchiveDialog::FillList()
{
	wxListCtrl *pListCtrl = GetListCtrl();

	pListCtrl->Freeze();

	// fill list
	FillListI();

	// if we had a base path specified
	if (m_sBasePath.Length() != 0)
	{
		// but we only added ".."
		if (pListCtrl->GetItemCount() == 1)
		{
			// then clear the base path
			m_sBasePath.Empty();

			// and re-fill
			FillListI();

			// perhaps the path no longer exists in that zip
		}
	}
	pListCtrl->Thaw();

}

bool ArchiveDialog::DoPicking(wxWindow *pParentWindow, const wxString &sArchiveFileAndPath, const wxString &sTitle, wxString &sPickedFilename)
{
	// this will hold a list of archives, so we can browse an archive inside an archive.
	wxArrayString sPaths;

	// this is the current archive to view
	wxString sCurrentArchiveFileAndPath = sArchiveFileAndPath;

	bool bCancelPicking = false;
	bool bPickingDone = false;
	while (!bPickingDone)
	{
		wxString sArchivePath;
		wxString sArchiveInitialPath;

		sArchivePath = sCurrentArchiveFileAndPath;

		// if an archive is within an archive this breaks
		int nPos = sCurrentArchiveFileAndPath.Find(wxT('#'), true);
		if (nPos != -1)
		{
			wxString sPath = sCurrentArchiveFileAndPath.Left(nPos);

			// store path of archive on filesystem, we need it for browsing archives
			sArchivePath = sPath;

			wxString sArchiveInternalPath = sCurrentArchiveFileAndPath.Right(sCurrentArchiveFileAndPath.Length() - (nPos + 1));
			nPos = sArchiveInternalPath.Index(':');
			if (nPos != -1)
			{
				// we need to remove "zip:" stub
				sArchiveInitialPath = sArchiveInternalPath.Right(sArchiveInternalPath.Length() - (nPos + 1));

				wxFileName ArchiveInternalPath(sArchiveInitialPath);
				sArchiveInitialPath = ArchiveInternalPath.GetPath();
			}
		}

		ArchiveDialog dialog(pParentWindow, sTitle, sArchivePath, sArchiveInitialPath);
		if (dialog.ShowModal() == wxID_OK)
		{
			// we picked something
			if (dialog.m_sPickedFilename.IsEmpty())
			{
				// user wanted to back out
				//
				// if there are no paths in our list, we've completely done with archive picking
				if (sPaths.GetCount()==0)
				{
					bPickingDone = true;
				}
				else
				{
					// there is a path in our list, go back to previous archive.
					size_t nLastIndex = sPaths.GetCount();

					// get archive path off the end of the list.
					sCurrentArchiveFileAndPath = sPaths[nLastIndex];
					// remove from list
					sPaths.RemoveAt(nLastIndex);
			}
		}
			else
			{
				// user picked something
#if 0
				if (IsArchive(pArchiveDialog->m_sPickedFilename))
				{
					// we picked an archive

					// store current one in the list so we can go back to it if we back out of the 
					// picked archive
					sPaths.Add(sCurrentArchiveFileAndPath);

					// make the picked archive the new archive
					sCurrentArchiveFileAndPath = pArchiveDialog->m_sPickedFilename;
				}
				else
#endif
				{
					// a file within an archive which we've picked
					sPickedFilename = dialog.m_sPickedFilename;

					// done picking without cancelling
					bPickingDone = true;
				}
			}
	}
		else
		{
			// done picking
			bPickingDone = true;
			// cancelled it.
			bCancelPicking = true;
		}
}

	return !bCancelPicking;
}



ArchiveDialog::ArchiveDialog(wxWindow *pParent, const wxString &sTitle, const wxString &sFilename, const wxString &sArchiveInitialPath) : wxDialog()
{
	m_sBasePath = sArchiveInitialPath;
	if (m_sBasePath.Length() != 0)
	{
		if (m_sBasePath[m_sBasePath.Length() - 1] != wxT('\\'))
			m_sBasePath += wxT("\\");
	}
	m_sFilename = sFilename;

	// load the resource
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_DIALOG_ARCHIVE"));

	if (sTitle.Length() != 0)
	{
		wxString sActualTitle = sTitle + wxT(" - Browse Archive");
		SetTitle(sActualTitle);
	}
}

void ArchiveDialog::GetFilterClassFactory(const wxString &sFilename, wxFilterClassFactory **ppFilterClassFactory, wxFilterInputStream **ppInputStream)
{
	*ppFilterClassFactory = NULL;
	*ppInputStream = NULL;

	// make it lower otherwise wxWidgets attempts to find "ZIP" on linux and fails.
	wxString sCurrentFilename = sFilename;
	wxString sCurrentFilenameLower = sFilename.Lower();

	wxInputStream *pInputStream = new wxFFileInputStream(sCurrentFilename);
	if (pInputStream != NULL)
	{
		const wxFilterClassFactory *pFilterClassFactory = wxFilterClassFactory::Find(sCurrentFilenameLower, wxSTREAM_FILEEXT);
		if (pFilterClassFactory)
		{
			wxFilterInputStream *pNewInputStream = pFilterClassFactory->NewStream(pInputStream);
			if (pNewInputStream != NULL)
			{
				*ppInputStream = pNewInputStream;
				*ppFilterClassFactory = const_cast<wxFilterClassFactory *>(pFilterClassFactory);
				return;
			}
		}

		delete pInputStream;
	}

}


void ArchiveDialog::GetArchiveClassFactory(const wxString &sFilename, wxFilterClassFactory **ppFilterClassFactory, wxArchiveClassFactory **ppArchiveClassFactory, wxArchiveInputStream **ppInputStream)
{
	*ppFilterClassFactory = NULL;
	*ppArchiveClassFactory = NULL;
	*ppInputStream = NULL;
	const wxFilterClassFactory *pFilterClassFactory = NULL;

	// make it lower otherwise wxWidgets attempts to find "ZIP" on linux and fails.
	wxString sCurrentFilename = sFilename;
	wxString sCurrentFilenameLower = sFilename.Lower();

	// zip has archive only, no filter
	// tar.gz has archive and filter.

	wxInputStream *pInputStream = new wxFFileInputStream(sCurrentFilename);
	if (pInputStream != NULL)
	{
		pFilterClassFactory = wxFilterClassFactory::Find(sCurrentFilenameLower, wxSTREAM_FILEEXT);
		if (pFilterClassFactory)
		{
			wxFilterInputStream *pNewInputStream = pFilterClassFactory->NewStream(pInputStream);
			if (pNewInputStream != NULL)
			{
				pInputStream = pNewInputStream;

				sCurrentFilenameLower = pFilterClassFactory->PopExtension(sCurrentFilenameLower);
			}
			else
			{
				pFilterClassFactory = NULL;
			}
		}
	}

	const wxArchiveClassFactory *pArchiveClassFactory = wxArchiveClassFactory::Find(sCurrentFilenameLower, wxSTREAM_FILEEXT);
	if (pArchiveClassFactory!=NULL)
	{
		// filter with archive, e.g. .tar.gz
		wxArchiveInputStream *pNewInputStream = pArchiveClassFactory->NewStream(pInputStream);
		if (pNewInputStream != NULL)
		{

			*ppInputStream = pNewInputStream;
			*ppFilterClassFactory = const_cast<wxFilterClassFactory *>(pFilterClassFactory);
			*ppArchiveClassFactory = const_cast<wxArchiveClassFactory *>(pArchiveClassFactory);
			return;
		}
			}
#if 0
	if (pFilterClassFactory != NULL)
	{
		// filter without archive
		*ppInputStream = pInputStream;
		*ppFilterClassFactory = const_cast<wxFilterClassFactory *>(pFilterClassFactory);
		*ppArchiveClassFactory = NULL;
		return;
	}
#endif   
	if (pInputStream != NULL)
	{
		// original stream, or one from the filter
		delete pInputStream;
	}
		}

bool ArchiveDialog::IsArchive(const wxString &sFilename)
{
	bool bIsArchive = false;

	wxFilterClassFactory *pFilterClassFactory;
	wxArchiveClassFactory *pArchiveClassFactory;
	wxArchiveInputStream *pInputStream;
	ArchiveDialog::GetArchiveClassFactory(sFilename, &pFilterClassFactory, &pArchiveClassFactory, &pInputStream);
	if (pArchiveClassFactory != NULL)
	{
		bIsArchive = true;
	}
	if (pInputStream != NULL)
	{
		delete pInputStream;
	}
	return bIsArchive;
}

bool ArchiveDialog::GetFilterFilename(const wxString &sFilename, wxString &sFilterFilename)
{
	wxFilterClassFactory *pFilterClassFactory = NULL;
	wxFilterInputStream *pInputStream = NULL;
	wxString sCurrentFilename = sFilename;

	ArchiveDialog::GetFilterClassFactory(sFilename, &pFilterClassFactory, &pInputStream);
	if (pFilterClassFactory != NULL)
	{
		sFilterFilename = sFilename;
		sFilterFilename += wxT("#");
		sFilterFilename += pFilterClassFactory->GetProtocol();
		//    sFilterFilename += wxT(":");

		//    sCurrentFilename= pFilterClassFactory->PopExtension(sCurrentFilename);
		//   sFilterFilename += sCurrentFilename;

		delete pInputStream;
		return true;
	}

	return false;
}


bool ArchiveDialog::IsFilter(const wxString &sFilename)
{
	bool bIsFilter = false;

	wxFilterClassFactory *pFilterClassFactory;
	wxFilterInputStream *pInputStream;
	ArchiveDialog::GetFilterClassFactory(sFilename, &pFilterClassFactory, &pInputStream);
	if (pFilterClassFactory != NULL)
	{
		bIsFilter = true;
	}
	if (pInputStream != NULL)
	{
		delete pInputStream;
	}
	return bIsFilter;
}
