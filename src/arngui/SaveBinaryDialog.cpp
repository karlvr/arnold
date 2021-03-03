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
#include "SaveBinaryDialog.h"
#include <wx/xrc/xmlres.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include "arnguiApp.h"

BEGIN_EVENT_TABLE(SaveBinaryDialog, wxDialog)
EVT_TEXT(XRCID("IDC_EDIT_MEMORYSTART"), SaveBinaryDialog::OnTextChangedMemoryStart)
EVT_TEXT(XRCID("IDC_EDIT_MEMORYEND"), SaveBinaryDialog::OnTextChangedMemoryEnd)
EVT_TEXT(XRCID("IDC_EDIT_START"), SaveBinaryDialog::OnTextChangedStart)
EVT_CHECKBOX(XRCID("IDC_CHECK_START_IS_MEMORY_START"), SaveBinaryDialog::OnCheckStartIsMemoryStart)
EVT_CHECKBOX(XRCID("IDC_CHECK_EXECUTION_ADDRESS_IS_START"), SaveBinaryDialog::OnCheckExecutionAddressIsStart)
EVT_CHECKBOX(XRCID("IDC_CHECK_LENGTH_IS_MEMORY_LENGTH"), SaveBinaryDialog::OnCheckLengthIsMemoryLength)
END_EVENT_TABLE()

void SaveBinaryDialog::UpdateHeaderLength()
{
	wxCheckBox *pCheck;
	wxTextCtrl *pTextCtrl;

	pCheck = XRCCTRL(*this, "IDC_CHECK_LENGTH_IS_MEMORY_LENGTH", wxCheckBox);
	if (pCheck->GetValue())
	{

		wxString sExpression;

		pTextCtrl = XRCCTRL(*this, "IDC_EDIT_MEMORYEND", wxTextCtrl);
		sExpression = wxT("(") + pTextCtrl->GetValue() + wxT(")");

		pTextCtrl = XRCCTRL(*this, "IDC_EDIT_MEMORYSTART", wxTextCtrl);
		sExpression += wxT("-(") + pTextCtrl->GetValue() + wxT(")");

		pTextCtrl = XRCCTRL(*this, "IDC_EDIT_LENGTH", wxTextCtrl);
		pTextCtrl->SetValue(sExpression);
	}

}

void SaveBinaryDialog::UpdateTextChange(long nCheckID, long nSourceText, long nDestText)
{
	wxWindow *pWindow;
	wxCheckBox *pCheck;
	wxTextCtrl *pTextCtrl;

	pWindow = this->FindWindow(nCheckID);
	pCheck = static_cast<wxCheckBox *>(pWindow);
	if (pCheck->GetValue())
	{
		wxString sValue;

		pWindow = this->FindWindow(nSourceText);
		pTextCtrl = static_cast<wxTextCtrl *>(pWindow);
		sValue = pTextCtrl->GetValue();

		pWindow = this->FindWindow(nDestText);
		pTextCtrl = static_cast<wxTextCtrl *>(pWindow);
		pTextCtrl->SetValue(sValue);
	}
}

void SaveBinaryDialog::OnCheckStartIsMemoryStart(wxCommandEvent &event)
{
	if (event.IsChecked())
	{
		UpdateTextChange(XRCID("IDC_CHECK_START_IS_MEMORY_START"), XRCID("IDC_EDIT_MEMORYSTART"), XRCID("IDC_EDIT_START"));
	}
}

void SaveBinaryDialog::OnTextChangedMemoryStart(wxCommandEvent & WXUNUSED(event))
{
	UpdateTextChange(XRCID("IDC_CHECK_START_IS_MEMORY_START"), XRCID("IDC_EDIT_MEMORYSTART"), XRCID("IDC_EDIT_START"));
}

void SaveBinaryDialog::OnTextChangedMemoryEnd(wxCommandEvent & WXUNUSED(event))
{
	UpdateHeaderLength();
}


void SaveBinaryDialog::OnCheckExecutionAddressIsStart(wxCommandEvent &event)
{
	if (event.IsChecked())
	{
		UpdateTextChange(XRCID("IDC_CHECK_EXECUTION_ADDRESS_IS_START"), XRCID("IDC_EDIT_START"), XRCID("IDC_EDIT_EXECUTION_ADDRESS"));
	}
}


void SaveBinaryDialog::OnCheckLengthIsMemoryLength(wxCommandEvent &event)
{
	if (event.IsChecked())
	{
		UpdateHeaderLength();
	}
}

void SaveBinaryDialog::OnTextChangedStart(wxCommandEvent & WXUNUSED(event))
{
	UpdateTextChange(XRCID("IDC_CHECK_EXECUTION_ADDRESS_IS_START"), XRCID("IDC_EDIT_START"), XRCID("IDC_EDIT_EXECUTION_ADDRESS"));
}

SaveBinaryDialog::~SaveBinaryDialog()
{
}

bool SaveBinaryDialog::TransferDataToWindow()
{
	wxChoice *pChoice;
	wxTextCtrl *pTextCtrl;
	wxCheckBox *pCheckBox;

	wxString sValue;

	pCheckBox = XRCCTRL(*this, "IDC_CHECK_START_IS_MEMORY_START", wxCheckBox);
	pCheckBox->SetValue(true);

	pCheckBox = XRCCTRL(*this, "IDC_CHECK_EXECUTION_ADDRESS_IS_START", wxCheckBox);
	pCheckBox->SetValue(true);

	pCheckBox = XRCCTRL(*this, "IDC_CHECK_LENGTH_IS_MEMORY_LENGTH", wxCheckBox);
	pCheckBox->SetValue(true);

	sValue.Printf(wxT("&%04x"), m_pFileHeader->MemoryStart);
	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_MEMORYSTART", wxTextCtrl);
	pTextCtrl->SetValue(sValue);

	sValue.Printf(wxT("&%04x"), m_pFileHeader->MemoryEnd);
	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_MEMORYEND", wxTextCtrl);
	pTextCtrl->SetValue(sValue);



	sValue.Printf(wxT("&%04x"), m_pFileHeader->HeaderStartAddress);
	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_START", wxTextCtrl);
	pTextCtrl->SetValue(sValue);



	sValue.Printf(wxT("&%04x"), m_pFileHeader->HeaderLength);
	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_LENGTH", wxTextCtrl);
	pTextCtrl->SetValue(sValue);

	sValue.Printf(wxT("&%04x"), m_pFileHeader->HeaderExecutionAddress);
	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_EXECUTION_ADDRESS", wxTextCtrl);
	pTextCtrl->SetValue(sValue);

	pChoice = XRCCTRL(*this, "IDC_CHOICE_TYPE", wxChoice);
	pChoice->SetSelection(m_pFileHeader->HeaderFileType);

	pCheckBox = XRCCTRL(*this, "IDC_CHECK_WRITE_AMSDOS_HEADER", wxCheckBox);
	pCheckBox->SetValue(m_pFileHeader->bHasHeader ? true : false);


	return true;
}

bool SaveBinaryDialog::TransferDataFromWindow()
{
	wxChoice *pChoice;
	wxTextCtrl *pTextCtrl;
	wxCheckBox *pCheck;

	wxString sValue;


	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_MEMORYSTART", wxTextCtrl);
	sValue = pTextCtrl->GetValue();

	m_pFileHeader->MemoryStart = wxGetApp().ExpressionEvaluate(sValue);

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_MEMORYEND", wxTextCtrl);
	sValue = pTextCtrl->GetValue();

	m_pFileHeader->MemoryEnd = wxGetApp().ExpressionEvaluate(sValue);


	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_START", wxTextCtrl);
	sValue = pTextCtrl->GetValue();

	m_pFileHeader->HeaderStartAddress = wxGetApp().ExpressionEvaluate(sValue);

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_LENGTH", wxTextCtrl);
	sValue = pTextCtrl->GetValue();

	m_pFileHeader->HeaderLength = wxGetApp().ExpressionEvaluate(sValue);

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_EXECUTION_ADDRESS", wxTextCtrl);
	sValue = pTextCtrl->GetValue();

	m_pFileHeader->HeaderExecutionAddress = wxGetApp().ExpressionEvaluate(sValue);

	pChoice = XRCCTRL(*this, "IDC_CHOICE_TYPE", wxChoice);
	m_pFileHeader->HeaderFileType = pChoice->GetSelection();

	pCheck = XRCCTRL(*this, "IDC_CHECK_WRITE_AMSDOS_HEADER", wxCheckBox);
	m_pFileHeader->bHasHeader = pCheck->GetValue();

	return true;

}


SaveBinaryDialog::SaveBinaryDialog(wxWindow *pParent, FILE_HEADER *pHeader)
{
	m_pFileHeader = pHeader;

	// load the resource
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_DIALOG_SAVEDATA"));
}

