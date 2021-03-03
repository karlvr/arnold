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

#include "FullScreenDialog.h"
#include <wx/xrc/xmlres.h>
#include "arnguiApp.h"
#include <wx/listbox.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/radiobut.h>
#include "IntClientData.h"

BEGIN_EVENT_TABLE(FullScreenDialog, wxDialog)
END_EVENT_TABLE()


/*
Amstrad screen is based on PAL:

* 50Hz
* Progressive
* 288 scanlines visible (equates to 36 CRTC chars in height; normally 35 is used)
48 chars in width is normally visible, sometimes 50. 48 equates to 768.  50*8*2 equates to 800.

* 312 lines total (including blanking)
* 64us per line giving 15Khz line rate

Best to choose a "progressive" display.

576 in height is good and use scanlines to blank if using a monitor.
288 in height is good if you are using a CRT.

800x576 is good.
768x576 is good.

800x288 is good.
768x288 is good.

16Khz clock to gate-array. 2 bytes per us, 16 pixels per byte.

Change full-screen dialog to be a display dialog and merge windowed and full-screen together.

*/

FullScreenDialog::~FullScreenDialog()
{
	wxChoice *pChoice = XRCCTRL(*this, "m_ChoiceResolution", wxChoice);
	for (int i = pChoice->GetCount() - 1; i >= 0; i--)
	{
		pChoice->Delete(i);
	}
}

bool FullScreenDialog::TransferDataToWindow()
{
	wxChoice *pChoice = XRCCTRL(*this, "m_ChoiceResolution", wxChoice);

	// this needs to describe the list of fullscreen resolutions etc
	wxGetApp().m_PlatformSpecific.PopulateFullScreenDialog(pChoice);

	bool   bFound = false;

	for (unsigned int i = 0; i != pChoice->GetCount(); i++)
	{
		const IntClientData *pData = (const IntClientData *)pChoice->GetClientObject(i);

		if (pData->GetData() == wxGetApp().GetFullScreenMode())
		{
			bFound = true;
			pChoice->SetSelection(i);
			break;
		}
	}
	if (!bFound)
	{
		pChoice->SetSelection(0);
	}
	return true;
}

bool FullScreenDialog::TransferDataFromWindow()
{
	wxChoice *pChoice = XRCCTRL(*this, "m_ChoiceResolution", wxChoice);

	int nChoice = pChoice->GetSelection();

	if (nChoice != wxNOT_FOUND)
	{
		const IntClientData *pIntData = (const IntClientData *)pChoice->GetClientObject(nChoice);
		int nMode = pIntData->GetData();

		wxConfig::Get(false)->Write(wxT("fullscreen/mode"), nMode);
		wxGetApp().SetFullScreenMode(nMode);

	}

	wxGetApp().WriteConfigWindowPosAndSize(wxT("windows/fullscreen/"), this);

	return true;

}


FullScreenDialog::FullScreenDialog(wxWindow *pParent)
{
	// load the resource
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_DIALOG_FULLSCREEN_SETTINGS"));
	//this->SetSize(500,300);
	wxGetApp().ReadConfigWindowPosAndSize(wxT("windows/fullscreen/"), this);
	wxGetApp().EnsureWindowVisible(this);

}

