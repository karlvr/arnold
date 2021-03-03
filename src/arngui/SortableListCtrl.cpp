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
#include "SortableListCtrl.h"
#include <wx/icon.h>

#if defined(__GNUC__) || defined(__llvm__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

#ifndef __WXWIN__
/* XPM */
static char *sort_asc_xpm[] = {
	/* columns rows colors chars-per-pixel */
	"16 16 2 1",
	". c #ACA899",
	"  c #FF0000",
	/* pixels */
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"        .       ",
	"       ...      ",
	"      .....     ",
	"     .......    ",
	"    .........   ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                "
};

/* XPM */
static char *sort_desc_xpm[] = {
	/* columns rows colors chars-per-pixel */
	"16 16 2 1",
	". c #ACA899",
	"  c #FF0000",
	/* pixels */
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"    .........   ",
	"     .......    ",
	"      .....     ",
	"       ...      ",
	"        .       ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                "
};
#endif

#if defined(__GNUC__)  || defined(__llvm__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic pop
#endif


IMPLEMENT_DYNAMIC_CLASS(SortableListCtrl, wxListCtrl)


wxString SortableListCtrl::GetColumnTextForItem(int item, int col)
{
	wxListItem item1;
	item1.SetId(item);
	item1.SetColumn(col);
	item1.SetMask(wxLIST_MASK_TEXT);
	if (GetItem(item1))
	{
		return item1.GetText();
	}
	return wxEmptyString;
}

BEGIN_EVENT_TABLE(SortableListCtrl, wxListCtrl)
EVT_WINDOW_CREATE(SortableListCtrl::OnCreate)
END_EVENT_TABLE()

static int wxCALLBACK MyCompareFunction(wxIntPtr item1, wxIntPtr item2, wxIntPtr
sortData)
{
	SortableListCtrl	*pCtrl = (SortableListCtrl*)sortData;
	return pCtrl->PerformSort(item1, item2);
}

SortableListCtrl::SortableListCtrl() : m_bSortAscending(true), m_nSortColumn(0), m_imageListSmall(NULL)
{

}

void SortableListCtrl::OnCreate(wxWindowCreateEvent & WXUNUSED(event))
{
	//load arrow icon
	m_imageListSmall = new wxImageList(16, 16, true);
	if (m_imageListSmall != NULL)
	{
#if defined(__WXMSW__)
		wxString sIconUp(wxT("Arrowup"));
		wxString sIconDown(wxT("Arrowdown"));

		m_imageListSmall->Add(wxIcon(sIconUp, wxBITMAP_TYPE_ICO_RESOURCE));
		m_imageListSmall->Add(wxIcon(sIconDown, wxBITMAP_TYPE_ICO_RESOURCE));
#else
		m_imageListSmall->Add(wxBitmap(sort_asc_xpm), wxColor(255, 0, 0));
		m_imageListSmall->Add(wxBitmap(sort_desc_xpm), wxColor(255, 0, 0));
#endif

		SetImageList(m_imageListSmall, wxIMAGE_LIST_SMALL);
	}
	Connect(GetId(), wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler(SortableListCtrl::OnColumnClick));
}

void SortableListCtrl::SetColumnsImage(int col, int image)
{
	wxListItem item;
	item.SetColumn(col);
	item.SetMask(wxLIST_MASK_IMAGE);
	item.SetImage(image);
	SetColumn(col, item);
}




SortableListCtrl::~SortableListCtrl()
{
	delete m_imageListSmall;
}

void SortableListCtrl::PerformSort(int nColumn)
{
	// remove old image
	SetColumnsImage(m_nSortColumn, -1);

	if (m_nSortColumn != nColumn)
	{
		// set state for new column
		m_bSortAscending = true;
	}
	else
	{
		//toggle state
		m_bSortAscending = !(m_bSortAscending);
	}
	m_nSortColumn = nColumn;

	SetColumnsImage(m_nSortColumn, m_bSortAscending ? 0 : 1);

	SortItems(MyCompareFunction, (wxIntPtr)this);
}

void SortableListCtrl::SortNow()
{
	PerformSort(m_nSortColumn);
}

void SortableListCtrl::OnColumnClick(wxListEvent& event)
{
	PerformSort(event.GetColumn());
}
