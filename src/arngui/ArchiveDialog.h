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
#ifndef __ARCHIVE_DIALOG_HEADER_INCLUDED__
#define __ARCHIVE_DIALOG_HEADER_INCLUDED__

#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/archive.h>

#ifndef wxOVERRIDE
#define wxOVERRIDE
#endif

#include "SortableListCtrl.h"

class ArchiveListItem
{
public:
	wxString m_sDisplayName;
	bool m_bIsDirectory;
	wxString m_sInternalName;
	wxFileOffset m_nSize;
};

class ArchiveListCtrl : public SortableListCtrl
{
public:
	ArchiveListCtrl();
	~ArchiveListCtrl();

	// provide this in your base class to perform the sorting
	virtual int PerformSort(wxIntPtr WXUNUSED(item1), wxIntPtr WXUNUSED(item2)) wxOVERRIDE;

	DECLARE_EVENT_TABLE()
	DECLARE_DYNAMIC_CLASS(ArchiveListCtrl)
};

class ArchiveDialog : public wxDialog
{
public:
	ArchiveDialog(wxWindow *pParent, const wxString &sFilename, const wxString &sTitle, const wxString &sArchiveInitialPath);
	virtual ~ArchiveDialog();
	virtual bool TransferDataToWindow() wxOVERRIDE;
	static bool IsArchive(const wxString &sFilename);
	static bool IsFilter(const wxString &sFilename);
	static bool GetFilterFilename(const wxString &sFilename, wxString &sFilterFilename);

	// true indicates we picked something
	// sPickedFilename will be empty if backing out, otherwise it will be the name of a file from the archive
	//
	// false indicates we cancelled
	static bool DoPicking(wxWindow *pParentWindow, const wxString &sArchivePath, const wxString &sTitle, wxString &sPickedFilename);


	wxString m_sPickedFilename;
private:
	wxString m_sFilename;
	wxString m_sBasePath;
	virtual void OnOK(wxCommandEvent &event);
	void HandlePickedItem(const wxListItem &Item);
	static void GetArchiveClassFactory(const wxString &sFilename, wxFilterClassFactory **pFilterClassFactory, wxArchiveClassFactory **ppArchiveClassFactory, wxArchiveInputStream **ppInputStream);
	static void GetFilterClassFactory(const wxString &sFilename, wxFilterClassFactory **pFilterClassFactory, wxFilterInputStream **pInputStream);

	void FillList();
	void FillListI();
	void AddItemToList(wxListCtrl *pListCtrl, const wxString &sName, const wxString &sInternalName, wxFileOffset nSize, bool bIsDirectory);
	void ClearItems();
	void OnItemActivated(wxListEvent& event);
	wxListCtrl *GetListCtrl();

	// This class handles events
	DECLARE_EVENT_TABLE()
};

#endif
