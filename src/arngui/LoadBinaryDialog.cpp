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
#include "LoadBinaryDialog.h"
#include <wx/xrc/xmlres.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include "arnguiApp.h"

BEGIN_EVENT_TABLE(LoadBinaryDialog, wxDialog)
END_EVENT_TABLE()

LoadBinaryDialog::~LoadBinaryDialog()
{
}

bool LoadBinaryDialog::TransferDataToWindow()
{
	wxChoice *pChoice;
	wxTextCtrl *pTextCtrl;
	wxCheckBox *pCheck;

	wxString sValue;

	pCheck = XRCCTRL(*this, "SetBreakpointCheckbox", wxCheckBox);
	pCheck->SetValue(m_bSetBreakpoint);

	sValue.Printf(wxT("&%04x"), m_pFileHeader->HeaderStartAddress);
	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_START", wxTextCtrl);
	pTextCtrl->SetValue(sValue);

	// TODO: wxwidgets complains about this
	sValue.Printf(wxT("&%04x"), m_pFileHeader->HeaderLength);
	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_LENGTH", wxTextCtrl);
	pTextCtrl->SetValue(sValue);

	// TODO: wxwidgets complains about this too.
	sValue.Printf(wxT("&%04x"), (m_pFileHeader->HeaderStartAddress + m_pFileHeader->HeaderLength) & 0x0ffff);
	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_END", wxTextCtrl);
	pTextCtrl->SetValue(sValue);

	sValue.Printf(wxT("&%04x"), m_pFileHeader->HeaderExecutionAddress);
	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_EXECUTION_ADDRESS", wxTextCtrl);
	pTextCtrl->SetValue(sValue);

	pChoice = XRCCTRL(*this, "IDC_CHOICE_TYPE", wxChoice);
	pChoice->SetSelection(m_pFileHeader->HeaderFileType & 0x0f);

	sValue.Printf(wxT("&%04x"), m_pFileHeader->HeaderExecutionAddress);
	pTextCtrl = XRCCTRL(*this, "IDC_BREAKPOINT_ADDRESS", wxTextCtrl);
	pTextCtrl->SetValue(sValue);


	return true;
}

bool LoadBinaryDialog::TransferDataFromWindow()
{
	//    wxChoice *pChoice;
	wxTextCtrl *pTextCtrl;
	wxCheckBox *pCheck;

	wxString sValue;

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_START", wxTextCtrl);
	sValue = pTextCtrl->GetValue();

	m_pFileHeader->HeaderStartAddress = wxGetApp().ExpressionEvaluate(sValue);

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_LENGTH", wxTextCtrl);
	sValue = pTextCtrl->GetValue();

	m_pFileHeader->HeaderLength = wxGetApp().ExpressionEvaluate(sValue);

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_EXECUTION_ADDRESS", wxTextCtrl);
	sValue = pTextCtrl->GetValue();

	m_pFileHeader->HeaderExecutionAddress = wxGetApp().ExpressionEvaluate(sValue);

	pTextCtrl = XRCCTRL(*this, "IDC_BREAKPOINT_ADDRESS", wxTextCtrl);
	sValue = pTextCtrl->GetValue();

	m_BreakpointAddress = wxGetApp().ExpressionEvaluate(sValue);

	pCheck = XRCCTRL(*this, "SetBreakpointCheckbox", wxCheckBox);
	m_bSetBreakpoint = pCheck->GetValue();

	return true;

}


LoadBinaryDialog::LoadBinaryDialog(wxWindow *pParent, FILE_HEADER *pHeader)
{
	m_pFileHeader = pHeader;
	m_BreakpointAddress = 0;
	m_bSetBreakpoint = false;

	// load the resource
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_DIALOG_LOADDATA"));
}

