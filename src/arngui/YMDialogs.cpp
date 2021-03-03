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
// TODO: Fully support CTC-AY recording.

#include "YMDialogs.h"
#include <wx/xrc/xmlres.h>

extern "C"
{
#include "../cpc/dumpym.h"
}
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/config.h>
#include "IntClientData.h"
#include "arnguiApp.h"

YmPreSaveDialog::~YmPreSaveDialog()
{
	wxChoice *pChoice;

	pChoice = XRCCTRL(*this, "m_choice4", wxChoice);

	for (int i = pChoice->GetCount() - 1; i >= 0; i--)
	{
		//  const IntClientData *pIntData = (const IntClientData *)pChoice->GetClientObject(i);
		//m_pPrinterOutputStream  delete pIntData;
		pChoice->Delete(i);
	}


}

bool YmPreSaveDialog::TransferDataToWindow()
{
	wxTextCtrl *pTextCtrl;
	unsigned char *Text;
	wxChoice *pChoice;

	wxString sValue;

	Text = YMOutput_GetName();
	sValue = wxString((const char *)Text, *wxConvCurrent);

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_NAME", wxTextCtrl);
	pTextCtrl->SetValue(sValue);

	Text = YMOutput_GetAuthor();
	sValue = wxString((const char *)Text, *wxConvCurrent);

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_AUTHOR", wxTextCtrl);
	pTextCtrl->SetValue(sValue);

	Text = YMOutput_GetComment();
	sValue = wxString((const char *)Text, *wxConvCurrent);

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_COMMENT", wxTextCtrl);
	pTextCtrl->SetValue(sValue);

	pChoice = XRCCTRL(*this, "m_choice4", wxChoice);

	IntClientData *pIntData;
	pIntData = new IntClientData(YMOUTPUT_VERSION_3);
	pChoice->Append(wxT("ym3"), pIntData);
	pIntData = new IntClientData(YMOUTPUT_VERSION_5);
	pChoice->Append(wxT("ym5"), pIntData);

	int nChoice = YMOutput_GetVersion();

	for (unsigned int i = 0; i < pChoice->GetCount(); i++)
	{
		const IntClientData *pItemIntData = (const IntClientData *)pChoice->GetClientObject(i);
		if (pItemIntData->GetData() == nChoice)
		{
			pChoice->SetSelection(i);
		}
	}

	return true;
}


bool YmPreSaveDialog::TransferDataFromWindow()
{

	wxTextCtrl *pTextCtrl;
	wxChoice *pChoice;

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_NAME", wxTextCtrl);

	{

		wxString sText = pTextCtrl->GetValue();
		wxCharBuffer buffer = sText.utf8_str();

		YMOutput_SetName((unsigned char *)buffer.data());
		buffer.reset();
	}

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_AUTHOR", wxTextCtrl);

	{

		wxString sText = pTextCtrl->GetValue();
		wxCharBuffer buffer = sText.utf8_str();

		YMOutput_SetAuthor((unsigned char *)buffer.data());
		buffer.reset();
	}

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_COMMENT", wxTextCtrl);

	{

		wxString sText = pTextCtrl->GetValue();
		wxCharBuffer buffer = sText.utf8_str();

		YMOutput_SetComment((unsigned char *)buffer.data());
		buffer.reset();
	}

	pChoice = XRCCTRL(*this, "m_choice4", wxChoice);

	int nChoice = pChoice->GetSelection();

	if (nChoice != wxNOT_FOUND)
	{
		const IntClientData *pIntData = (const IntClientData *)pChoice->GetClientObject(nChoice);
		int nData = pIntData->GetData();
		YMOutput_SetVersion(nData);

		wxConfig::Get(false)->Write(wxT("ym/record/version"), YMOutput_GetVersion());
	}

	return true;
}


YmPreSaveDialog::YmPreSaveDialog(wxWindow *pParent)
{
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_DIALOG_YM5_SAVE"));
}


BEGIN_EVENT_TABLE(YmRecordingDialog, wxDialog)
EVT_BUTTON(XRCID("IDC_STOP"), YmRecordingDialog::OnStopRecording)
EVT_BUTTON(XRCID("IDC_RECORD"), YmRecordingDialog::OnStartRecording)
EVT_BUTTON(XRCID("IDC_SAVE"), YmRecordingDialog::OnSaveRecording)
EVT_CHAR_HOOK(YmRecordingDialog::OnCharHook)
EVT_CLOSE(YmRecordingDialog::OnClose)
END_EVENT_TABLE()


void YmRecordingDialog::OnCharHook(wxKeyEvent& event)
{
	if (IsEscapeKey(event))
	{
		Close();
		return;
	}

	event.Skip();
}

void YmRecordingDialog::OnClose(wxCloseEvent & WXUNUSED(event))
{
	wxCheckBox *pCheckBox;
	bool bValue;

	pCheckBox = XRCCTRL(*this, "IDC_CHECK_RECORDING_FLAG1", wxCheckBox);
	bValue = pCheckBox->GetValue();
	YMOutput_SetRecordWhenSilenceEnds(bValue);

	wxConfig::Get(false)->Write(wxT("ym/record/start_record_when_silence_ends"), bValue);

	pCheckBox = XRCCTRL(*this, "IDC_CHECK_RECORDING_FLAG2", wxCheckBox);
	bValue = pCheckBox->GetValue();
	YMOutput_SetStopRecordwhenSilenceBegins(bValue);

	wxConfig::Get(false)->Write(wxT("ym/record/stop_record_when_silence_begins"), bValue);

	wxGetApp().ReadConfigWindowPosAndSize(wxT("windows/ym_recording/"), this);
	wxGetApp().EnsureWindowVisible(this);

	this->Destroy();
	m_pInstance = NULL;

}

void YmRecordingDialog::OnStopRecording(wxCommandEvent & WXUNUSED(event))
{
	YMOutput_StopRecording();
}

extern "C"
{
	extern AY_3_8912 OnBoardAY;
}

void YmRecordingDialog::OnStartRecording(wxCommandEvent & WXUNUSED(event))
{

	YMOutput_SetPSGToDump(&OnBoardAY);

	YMOutput_StartRecording(YMOutput_GetRecordWhenSilenceEnds(), YMOutput_GetStopRecordwhenSilenceBegins());
}


void YmRecordingDialog::OnSaveRecording(wxCommandEvent & WXUNUSED(event))
{
	YmPreSaveDialog dialog(this);
	if (dialog.ShowModal() == wxID_OK)
	{
		wxString sFilename;
		wxString sTitleSuffix;
		if (YMFileType.OpenForWrite(this, sFilename, sTitleSuffix))
		{
			unsigned long OutputSize = YM_GetOutputSize(YMOutput_GetVersion());
			unsigned char *pOutputData = (unsigned char *)malloc(OutputSize);
			if (pOutputData != NULL)
			{
				YM_GenerateOutputData(pOutputData, YMOutput_GetVersion());

				wxGetApp().SaveLocalFile(sFilename, pOutputData, OutputSize);

				free(pOutputData);
			}
		}
	}

}


bool YmRecordingDialog::TransferDataToWindow()
{
	wxCheckBox *pCheckBox;

	pCheckBox = XRCCTRL(*this, "IDC_CHECK_RECORDING_FLAG1", wxCheckBox);
	pCheckBox->SetValue(YMOutput_GetRecordWhenSilenceEnds() ? true : false);


	pCheckBox = XRCCTRL(*this, "IDC_CHECK_RECORDING_FLAG2", wxCheckBox);
	pCheckBox->SetValue(YMOutput_GetStopRecordwhenSilenceBegins() ? true : false);

	return true;
}


bool YmRecordingDialog::TransferDataFromWindow()
{
	wxCheckBox *pCheckBox;
	bool bValue;

	pCheckBox = XRCCTRL(*this, "IDC_CHECK_RECORDING_FLAG1", wxCheckBox);
	bValue = pCheckBox->GetValue();
	YMOutput_SetRecordWhenSilenceEnds(bValue);

	wxConfig::Get(false)->Write(wxT("ym/record/start_record_when_silence_ends"), bValue);

	pCheckBox = XRCCTRL(*this, "IDC_CHECK_RECORDING_FLAG2", wxCheckBox);
	bValue = pCheckBox->GetValue();
	YMOutput_SetStopRecordwhenSilenceBegins(bValue);

	wxConfig::Get(false)->Write(wxT("ym/record/stop_record_when_silence_begins"), bValue);

	return true;
}

YmRecordingDialog::YmRecordingDialog(wxWindow *pParent) : YMFileType(wxT("Load YM"), wxT("Save YM"))

{

	FileFilter YMFilter = { wxT("YM File"), wxT("ym") };

	YMFileType.AddWriteFilter(YMFilter);

	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_DIALOG_YM5"));

	wxGetApp().ReadConfigWindowPosAndSize(wxT("windows/ym_recording/"), this);
	wxGetApp().EnsureWindowVisible(this);

}


YmRecordingDialog *YmRecordingDialog::m_pInstance = NULL;

// creator
YmRecordingDialog *YmRecordingDialog::CreateInstance(wxWindow *pParent)
{
	if (m_pInstance == NULL)
	{
		m_pInstance = new YmRecordingDialog(pParent);
		if (m_pInstance != NULL)
		{
			m_pInstance->Show();
		}
	}
	else
	{
		m_pInstance->Raise();
	}

	return m_pInstance;
}

