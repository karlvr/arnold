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
#ifndef __SORTABLE_LIST_CTRL__
#define __SORTABLE_LIST_CTRL__

#include <wx/listctrl.h>
#include <wx/imaglist.h>

class SortableListCtrl : public wxListCtrl
{
public:
	SortableListCtrl();
	~SortableListCtrl();

public:
	// provide this in your base class to perform the sorting
	virtual int PerformSort(wxIntPtr WXUNUSED(item1), wxIntPtr WXUNUSED(item2))
	{
		// by default no sort
		return 0;
	}
	void OnCreate(wxWindowCreateEvent &event);
	void SortNow();

protected:
	void PerformSort(int nColumn);
	bool m_bSortAscending; // true to sort ascending (a-z), false to sort descending (z-a)
	int m_nSortColumn;		// column to sort on
	wxString GetColumnTextForItem(int item, int col);

private:

	void SetColumnsImage(int col, int image);

	wxImageList * m_imageListSmall; // the image list for the header
	//to enable sorting
	void OnColumnClick(wxListEvent& event);
	DECLARE_EVENT_TABLE()
	DECLARE_DYNAMIC_CLASS(SortableListCtrl)
};

#endif
