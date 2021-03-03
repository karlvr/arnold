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

#ifndef __EMU_FILE_TYPE_HEADER_INCLUDED__
#define __EMU_FILE_TYPE_HEADER_INCLUDED__

#include "FileFilter.h"
#include <wx/window.h>

class EmuFileType
{
private:
	typedef enum
	{
		BROWSE_TYPE_FILESYSTEM = 0,
		BROWSE_TYPE_ARCHIVE
	} BROWSE_TYPE;

	wxString m_sRecentPath;     // last picked path
	//  wxFileHistory m_History;    // history of picked paths suitable for ui
	wxString m_sTitleRead;          // title
	wxString m_sTitleWrite;
	FileFilterArray m_ReadFilters;
	FileFilterArray m_WriteFilters;
	wxString m_sSelectedFilter;
	int m_nFilterIndex;

	wxString GetFilterStringForRead();
	wxString GetFilterStringForWrite();
public:
	void AddReadFilter(const FileFilter &filter);
	void AddWriteFilter(const FileFilter &filter);

	EmuFileType(const wxString &sTitleRead, const wxString &sTitleWrite);
	virtual ~EmuFileType();

	void SetInitialPath(const wxString &sInitialPath);

	bool Open(wxWindow *pParentWindow, wxString &sFilename, const wxString &sTitleSuffix);
	bool OpenForWrite(wxWindow *pParentWindow, wxString &sFilename, const wxString &sTitleSuffix);
};

#endif
