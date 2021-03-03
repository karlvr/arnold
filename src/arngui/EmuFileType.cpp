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
#include "EmuFileType.h"
#include <wx/stdpaths.h>
#include "ArchiveDialog.h"
#include <wx/filedlg.h>


EmuFileType::EmuFileType(const wxString &sTitleRead, const wxString &sTitleWrite)
{
	m_nFilterIndex = 0;
	m_sRecentPath = wxStandardPaths::Get().GetDocumentsDir();
	m_sRecentPath += wxFileName::GetPathSeparator();
	m_sTitleRead = sTitleRead;
	m_sTitleWrite = sTitleWrite;

	// add to the end of the filter
}

void EmuFileType::AddReadFilter(const FileFilter &filter)
{
	m_ReadFilters.Add(filter);
}

void EmuFileType::AddWriteFilter(const FileFilter &filter)
{
	m_WriteFilters.Add(filter);
}

EmuFileType::~EmuFileType()
{


}

void EmuFileType::SetInitialPath(const wxString &sInitialPath)
{
	m_sRecentPath = sInitialPath;
}

bool EmuFileType::Open(wxWindow *pParentWindow, wxString &sFilename, const wxString &sTitleSuffix)
{
	BROWSE_TYPE nBrowseType = BROWSE_TYPE_FILESYSTEM;

	wxString sArchiveInitialPath;

	wxString sFilter = GetFilterStringForRead();

	wxString sArchivePath;

	wxString sTitle = m_sTitleRead;
	if (!sTitleSuffix.IsEmpty())
	{
		sTitle += wxT(" - ");
		sTitle += sTitleSuffix;
	}

	wxString sBasePath;
	int nPos = m_sRecentPath.Find(wxT('#'), true);
	if (nPos != -1)
	{
		wxString sPath = m_sRecentPath.Left(nPos);

		// store path of archive on filesystem, we need it for browsing archives
		sArchivePath = sPath;

		wxString sArchiveInternalPath = m_sRecentPath.Right(m_sRecentPath.Length() - (nPos + 1));
		nPos = sArchiveInternalPath.Index(':');
		if (nPos != -1)
		{
			// we need to remove "zip:" stub
			sArchiveInitialPath = sArchiveInternalPath.Right(sArchiveInternalPath.Length() - (nPos + 1));

			wxFileName ArchiveInternalPath(sArchiveInitialPath);
			sArchiveInitialPath = ArchiveInternalPath.GetPath();
		}

		// go to browsing archive
		nBrowseType = BROWSE_TYPE_ARCHIVE;
		sBasePath = sArchivePath;
	}
	else
	{
		sBasePath = m_sRecentPath;
	}

	wxFileName RecentPath(sBasePath);
	wxString sOpenPath = RecentPath.GetPath();
	wxString sOpenName = RecentPath.GetFullName();

	bool bPicking = true;
	bool bPicked = false;

	while (bPicking)
	{
		switch (nBrowseType)
		{

		case BROWSE_TYPE_FILESYSTEM:
		{
			wxFileDialog openFileDialog(
				pParentWindow,
				sTitle,      // title
				sOpenPath,         // default dir
				sOpenName,     // default file
				sFilter,
				wxFD_OPEN | wxFD_FILE_MUST_EXIST);

			openFileDialog.SetFilterIndex(m_nFilterIndex);
			if (openFileDialog.ShowModal() == wxID_OK)
			{
				m_nFilterIndex = openFileDialog.GetFilterIndex();

				// store recently picked file
				wxString sPath = openFileDialog.GetPath();

				if (ArchiveDialog::IsArchive(sPath))
				{
					sArchiveInitialPath.Empty();

					wxFileName ArchivePath(sPath);
					// we generate path and name so that when we go back
					// to file picking we have it ready
					sOpenPath = ArchivePath.GetPath();
					sOpenName = ArchivePath.GetFullName();

					// store name of archive; we need it for browsing archives
					sArchivePath = sPath;

					// go to browsing archive
					nBrowseType = BROWSE_TYPE_ARCHIVE;
				}
				else
				{
					wxString sPathUsed = sPath;

					ArchiveDialog::GetFilterFilename(sPath, sPathUsed);

					m_sRecentPath = sPathUsed;

					sFilename = m_sRecentPath;

					// picked a file which wasn't an archive
					bPicking = false;
					bPicked = true;
				}
			}
			else
			{
				bPicking = false;
			}
		}
		break;

		case BROWSE_TYPE_ARCHIVE:
		{
			// do archive picking
			wxString sPickedPath;
			if (ArchiveDialog::DoPicking(pParentWindow, sArchivePath, sTitle, sPickedPath))
			{
				// go back up a level?
				if (sPickedPath.IsEmpty())
				{
					// back to file system browsing
					nBrowseType = BROWSE_TYPE_FILESYSTEM;
				}
				else
				{
					// picked something
					bPicking = false;
					bPicked = true;
					m_sRecentPath = sPickedPath;
					sFilename = m_sRecentPath;
				}
			}
			else
			{
				// cancelled archive browsing.

				// go back to browsing filesystem
				nBrowseType = BROWSE_TYPE_FILESYSTEM;
			}
		}
		break;
		}
	}


	return bPicked;
}

wxString EmuFileType::GetFilterStringForRead()
{
	wxString sFilter;
	wxString sFilterText = wxT("All supported (");
	for (size_t i = 0; i < m_ReadFilters.GetCount(); i++)
	{
		const FileFilter &ThisFilter = m_ReadFilters[i];
		if (i != 0)
		{
			sFilterText += wxT(";");
		}
		sFilterText += wxT("*.");
		sFilterText += ThisFilter.m_sExtension;
	}
	sFilterText += wxT(")|");
	for (size_t i = 0; i < m_ReadFilters.GetCount(); i++)
	{
		const FileFilter &ThisFilter = m_ReadFilters[i];
		if (i != 0)
		{
			sFilterText += wxT(";");
		}
		sFilterText += wxT("*.");
		sFilterText += ThisFilter.m_sExtension;
	}
	sFilterText += wxT("|");
	sFilter += sFilterText;

	for (size_t i = 0; i < m_ReadFilters.GetCount(); i++)
	{
		const FileFilter &ThisFilter = m_ReadFilters[i];
		sFilterText = ThisFilter.m_sDescription;
		sFilterText += wxT(" (*.");
		sFilterText += ThisFilter.m_sExtension;
		sFilterText += wxT(")|*.");
		sFilterText += ThisFilter.m_sExtension;
		sFilterText += wxT("|");
		sFilter += sFilterText;
	}

	sFilter += wxT("All files (*.*)|*.*|");

	sFilter += wxT("|");


	return sFilter;
}


wxString EmuFileType::GetFilterStringForWrite()
{
	wxString sFilter;
	for (size_t i = 0; i < m_WriteFilters.GetCount(); i++)
	{
		const FileFilter &ThisFilter = m_WriteFilters[i];
		wxString sFilterText;
		sFilterText = ThisFilter.m_sDescription;
		sFilterText += wxT(" (*.");
		sFilterText += ThisFilter.m_sExtension;
		sFilterText += wxT(")|*.");
		sFilterText += ThisFilter.m_sExtension;
		sFilterText += wxT("|");
		sFilter += sFilterText;
	}
	sFilter += wxT("|");

	return sFilter;
}

bool EmuFileType::OpenForWrite(wxWindow *pParentWindow, wxString &sFilename, const wxString &sTitleSuffix)
{
	wxFileName RecentPath(m_sRecentPath);
	wxString sTitle = m_sTitleWrite;
	if (!sTitleSuffix.IsEmpty())
	{
		sTitle += wxT(" - ") + sTitleSuffix;
	}

	wxString sFilter = GetFilterStringForWrite();

	wxFileDialog openFileDialog(
		pParentWindow,
		sTitle,      // title
		RecentPath.GetPath(),         // default dir
		RecentPath.GetFullName(),     // default file
		sFilter,
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);


	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return false;

	// store recently picked file
	m_sRecentPath = openFileDialog.GetPath();

	//  m_History.AddFileToHistory(m_sRecentPath);

	sFilename = m_sRecentPath;

	return true;
}
