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
#include "AutoTypeDialog.h"
#include <wx/xrc/xmlres.h>
#include "arnguiApp.h"
#include <wx/stdpaths.h>
#include <wx/dataobj.h>

extern "C"
{
#include "../cpc/autotype.h"

}

AutoTypeDropTarget::AutoTypeDropTarget() : m_pTextCtrl(NULL)
{
	wxDataObjectComposite *pCompositeDataObject = new wxDataObjectComposite();
	pCompositeDataObject->Add(new wxTextDataObject(), true);
	pCompositeDataObject->Add(new wxFileDataObject());
	SetDataObject(pCompositeDataObject);
}

wxDragResult AutoTypeDropTarget::OnDragOver(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y), wxDragResult WXUNUSED(def))
{
	return wxDragCopy;
}


wxDragResult  AutoTypeDropTarget::OnData(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y), wxDragResult WXUNUSED(defaultDragResult))
{
	if (!GetData())
	{
		return wxDragNone;
	}

	wxDataObjectComposite *pDataObjectComposite = static_cast<wxDataObjectComposite *>(GetDataObject());
	wxDataFormat format = pDataObjectComposite->GetReceivedFormat();
	switch (format.GetType())
	{
	case wxDF_TEXT:
	case wxDF_UNICODETEXT:
	{
#if (wxVERSION_NUMBER >= 2900)
		wxDataObject *pDataObject = pDataObjectComposite->GetObject(format);
		if (pDataObject)
		{
			wxTextDataObject *pTextDataObject = static_cast<wxTextDataObject *>(pDataObject);
			if (pTextDataObject)
			{
				wxString sText = pTextDataObject->GetText();
				if (m_pTextCtrl)
				{
					m_pTextCtrl->SetValue(sText);
				}
			}
		}
#else

		size_t Size = pDataObjectComposite->GetDataSize(format);
		if (Size!=0)
		{
			void *pBuffer = malloc(Size);
			if (pBuffer)
			{
				if (pDataObjectComposite->GetDataHere(format, pBuffer))
				{
					wxTextDataObject textDataObject;
					textDataObject.SetData(Size, pBuffer);

					wxString sText = textDataObject.GetText();

					if (m_pTextCtrl)
					{
						m_pTextCtrl->SetValue(sText);
					}

				}
				free(pBuffer);
			}
}
#endif
	}
	return wxDragCopy;

	case wxDF_FILENAME:
	{
#if (wxVERSION_NUMBER >= 2900)
		wxDataObject *pDataObject = pDataObjectComposite->GetObject(format);
		if (pDataObject)
		{
			wxFileDataObject *pFileDataObject = static_cast<wxFileDataObject *>(pDataObject);
			if (pFileDataObject)
			{
				wxArrayString Filenames = pFileDataObject->GetFilenames();
				if (Filenames.Count() != 0 && m_pTextCtrl)
				{
					m_pTextCtrl->LoadFile(Filenames[0]);
				}
			}
		}
#else
		size_t Size = pDataObjectComposite->GetDataSize(format);
		void *pBuffer = malloc(Size);
		if (pBuffer)
		{
			/* strings separated by \n, is there a better way to get this data */
			if (pDataObjectComposite->GetDataHere(format, pBuffer))
			{
				wxFileDataObject fileDataObject;
				fileDataObject.SetData(Size, pBuffer);

				wxArrayString Filenames = fileDataObject.GetFilenames();
				if (Filenames.Count() != 0 && m_pTextCtrl)
				{
					m_pTextCtrl->LoadFile(Filenames[0]);
				}
			}
			free(pBuffer);
		}
#endif
	  }
	return wxDragCopy;

	default:
		break;
	  }

	return wxDragNone;
}

BEGIN_EVENT_TABLE(CAutoTypeDialog, wxDialog)
EVT_CLOSE(CAutoTypeDialog::OnClose)
//	EVT_TEXT_COPY(id, func):
//	EVT_TEXT_CUT(id, func):
//	EVT_TEXT_PASTE(id, func):
EVT_BUTTON(XRCID("wxID_OK"), CAutoTypeDialog::OnOK)
EVT_BUTTON(XRCID("wxID_CANCEL"), CAutoTypeDialog::OnCancel)
EVT_BUTTON(XRCID("m_ButtonStop"), CAutoTypeDialog::OnStop)
END_EVENT_TABLE()

CAutoTypeDialog *CAutoTypeDialog::m_pInstance = NULL;

CAutoTypeDialog::~CAutoTypeDialog()
{

	m_pInstance = NULL;
}

void CAutoTypeDialog::SetText(const wxString &sText)
{
	wxTextCtrl *pTextCtrl = XRCCTRL(*this, "autotype_text", wxTextCtrl);
	pTextCtrl->SetValue(sText);
}

void CAutoTypeDialog::OnCancel(wxCommandEvent & WXUNUSED(event))
{
	Close();
}

void CAutoTypeDialog::OnOK(wxCommandEvent & WXUNUSED(event))
{
	wxTextCtrl *pTextCtrl = XRCCTRL(*this, "autotype_text", wxTextCtrl);
	wxString sText = pTextCtrl->GetValue();
	const wxCharBuffer buffer = sText.utf8_str();

	AutoType_SetString(buffer.data(), TRUE, FALSE, FALSE);
}

void CAutoTypeDialog::OnStop(wxCommandEvent & WXUNUSED(event))
{
	// force stop of autotype
	AutoType_Finish();
}


wxString CAutoTypeDialog::GetAutoTypeRememberFilename()
{
	wxGetApp().CreateUserDataDir();

	// get user data dir
	wxString sUserDataDir = wxStandardPaths::Get().GetUserDataDir();
	// now generate filename
	wxFileName filename(sUserDataDir, wxT("auto_type_text"));

	// and return it
	return filename.GetFullPath();
}

wxTextCtrl *CAutoTypeDialog::GetTextCtrl()
{
	return XRCCTRL(*this, "autotype_text", wxTextCtrl);
}


void CAutoTypeDialog::OnClose(wxCloseEvent & WXUNUSED(event))
{
	wxTextCtrl *pTextCtrl = GetTextCtrl();

	wxString sFilename = GetAutoTypeRememberFilename();
	pTextCtrl->SaveFile(sFilename);
	pTextCtrl->Clear();	// on Mac if we don't clear then the text remains in the window
	
	wxGetApp().WriteConfigWindowPosAndSize(wxT("windows/autotype/"), this);

	this->Destroy();
}

bool CAutoTypeDialog::TransferDataToWindow()
{
	wxTextCtrl *pTextCtrl = GetTextCtrl();

	// get autotype filename
	wxString sFilename = GetAutoTypeRememberFilename();

	// does it exist?
	if (wxFileName::FileExists(sFilename))
	{
		// load it
		pTextCtrl->LoadFile(sFilename);
	}

	// default size
	//this->SetSize(430,390);
	wxGetApp().ReadConfigWindowPosAndSize(wxT("windows/autotype/"), this);
	wxGetApp().EnsureWindowVisible(this);


	return true;
}

CAutoTypeDialog::CAutoTypeDialog(wxWindow *parent) : wxDialog()
{
	wxXmlResource::Get()->LoadDialog(this, parent, wxT("autotype_dialog"));

	wxTextCtrl *pTextCtrl = XRCCTRL(*this, "autotype_text", wxTextCtrl);
	
	m_DropTarget = new AutoTypeDropTarget();
	m_DropTarget->m_pTextCtrl = pTextCtrl;
	pTextCtrl->SetDropTarget(m_DropTarget);

	
	SetEscapeId(wxID_CANCEL);
}
