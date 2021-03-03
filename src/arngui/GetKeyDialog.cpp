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
#include "arnguiApp.h"
#include "GetKeyDialog.h"
#include <wx/xrc/xmlres.h>
#include <wx/choice.h>
#include "IntClientData.h"

BEGIN_EVENT_TABLE(GetKeyDialog, wxDialog)
END_EVENT_TABLE()

bool GetKeyDialog::TransferDataFromWindow()
{
	wxChoice *pChoice = XRCCTRL(*this, "m_keys", wxChoice);
	int nSelection = pChoice->GetSelection();
	if (nSelection != wxNOT_FOUND)
	{
		const IntClientData *pIntData = (const IntClientData *)pChoice->GetClientObject(nSelection);
		m_nChosenKey = pIntData->GetData();
	}
	return true;
}


bool GetKeyDialog::TransferDataToWindow()
{
	wxChoice *pChoice = XRCCTRL(*this, "m_keys", wxChoice);
	pChoice->Freeze();
	wxGetApp().m_PlatformSpecific.PopulateKeyDialog(pChoice);
	pChoice->Thaw();

	int nSelection = 0;
	for (unsigned int i = 0; i != pChoice->GetCount(); i++)
	{
		const IntClientData *pIntData = (const IntClientData *)pChoice->GetClientObject(i);
		if (pIntData->GetData() == m_nChosenKey)
		{
			nSelection = i;
			break;
		}
	}

	pChoice->SetSelection(nSelection);


	return true;
}

//constructor
GetKeyDialog::GetKeyDialog(wxWindow *pParent) : m_nChosenKey(CPC_KEY_DEF_UNSET_KEY)
{
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_CHOOSE_KEY"));
}

GetKeyDialog::~GetKeyDialog()
{
	wxChoice *pChoice = XRCCTRL(*this, "m_keys", wxChoice);
	for (int i = pChoice->GetCount() - 1; i >= 0; i--)
	{
		pChoice->Delete(i);
	}

}
