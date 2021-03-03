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
// TODO: Under wxwidgets 3.0 the dissassembly window is the wrong size, it ignores the xrc settings.

#include "DebuggerDialog.h"
//#include <wx/xrc/xmlres.h>
#include <wx/config.h>
#include <wx/fontenum.h>
#include "arnguiApp.h"
#include <wx/choicdlg.h>
#include <wx/textdlg.h>
#include <wx/menu.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include "LoadBinaryDialog.h"
#include "SaveBinaryDialog.h"
#include <wx/clipbrd.h>
#include <wx/notebook.h>
#include <wx/filedlg.h>
#include "arnguiMain.h"
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>
#include <wx/splitter.h>
#include <wx/timer.h>
#include "IntClientData.h"
#include "LabelManager.h"
#include "sdlcommon/sound.h"

WX_DEFINE_ARRAY_INT(int, ArrayOfInts);

#ifndef min
#define min(a,b) ((a)<(b)) ? (a) : (b)
#endif

// active debugger configuration
static HardwareDetailsDisplay DetailsDisplay;
static DebuggerDisplayState DebuggerDisplay;

#include <wx/xrc/xmlres.h>
#define MARGIN_WIDTH 16

//IMPLEMENT_CLASS(HardwareDetailsItem)

//#include <wx/arrimpl.cpp>
//WX_DEFINE_OBJARRAY(ArrayOfHardwareDetailsItem)
static const wxChar *sYes = wxT("Yes");
static const wxChar *sNo = wxT("No");


extern "C"
{
#include "../cpc/z8536.h"
#include "../cpc/asic.h"
#include "../cpc/kcc.h"
#include "../cpc/i8255.h"
#include "../cpc/aleste.h"
#include "../cpc/cpc.h"
#include "../cpc/debugger/labelset.h"
#include "../cpc/debugger/parse.h"
#include "../cpc/fdc.h"
	Z80_WORD StepOverInstruction(MemoryRange *pRange);
	Z80_WORD StepIntoInstruction(MemoryRange *pRange);
	int IsSeperatorOpcode(MemoryRange *, int Address);


}


BEGIN_EVENT_TABLE(ConvertAddressToolDialog, wxDialog)
EVT_COMMAND(XRCID("ConvertRange1ToRange2"), wxEVT_COMMAND_BUTTON_CLICKED, ConvertAddressToolDialog::OnConvertRange1ToRange2)
EVT_COMMAND(XRCID("ConvertRange2ToRange1"), wxEVT_COMMAND_BUTTON_CLICKED, ConvertAddressToolDialog::OnConvertRange2ToRange1)
EVT_CLOSE(ConvertAddressToolDialog::OnClose)
END_EVENT_TABLE()

ConvertAddressToolDialog *ConvertAddressToolDialog::m_pInstance = NULL;

// creator
ConvertAddressToolDialog *ConvertAddressToolDialog::CreateInstance(wxWindow *pParent)
{
	if (m_pInstance == NULL)
	{
		m_pInstance = new ConvertAddressToolDialog(pParent);
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


void ConvertAddressToolDialog::OnClose(wxCloseEvent & WXUNUSED(event))
{
	TransferDataFromWindow();

	this->Destroy();

	ConvertAddressToolDialog::m_pInstance = NULL;
}

void ConvertAddressToolDialog::ConvertRangeToRange(long RangeFromStartId, long RangeFromEndId, long RangeToStartId, long WXUNUSED(RangeToEndId))
{
	wxWindow *pWindow;
	wxTextCtrl *pTextCtrl;
	int nRangeFromStart, nRangeFromEnd;
	int nRangeToStart/*, nRangeToEnd*/;
	int nAddress;
	int nResult;
	wxString sValue;

	pWindow = this->FindWindow(RangeFromStartId);
	pTextCtrl = static_cast<wxTextCtrl *>(pWindow);
	sValue = pTextCtrl->GetValue();

	nRangeFromStart = wxGetApp().ExpressionEvaluate(sValue);

	pWindow = this->FindWindow(RangeFromEndId);
	pTextCtrl = static_cast<wxTextCtrl *>(pWindow);
	sValue = pTextCtrl->GetValue();

	nRangeFromEnd = wxGetApp().ExpressionEvaluate(sValue);

	pWindow = this->FindWindow(RangeToStartId);
	pTextCtrl = static_cast<wxTextCtrl *>(pWindow);
	sValue = pTextCtrl->GetValue();

	nRangeToStart = wxGetApp().ExpressionEvaluate(sValue);

	/*
	pWindow = this->FindWindow(RangeToEndId);
	pTextCtrl = static_cast<wxTextCtrl *>(pWindow);
	sValue = pTextCtrl->GetValue();


	nRangeToEnd = wxGetApp().ExpressionEvaluate(sValue);
	*/
	pTextCtrl = XRCCTRL(*this, "IDC_ADDRESS_FROM", wxTextCtrl);
	sValue = pTextCtrl->GetValue();

	nAddress = wxGetApp().ExpressionEvaluate(sValue);

	nResult = nAddress;
	if ((nResult >= nRangeFromStart) && (nResult <= nRangeFromEnd))
	{
		nResult -= nRangeFromStart;
		nResult += nRangeToStart;
	}

	pTextCtrl = XRCCTRL(*this, "IDC_ADDRESS_TO", wxTextCtrl);

	sValue.Printf(wxT("&%04x"), nResult);
	pTextCtrl->SetValue(sValue);
}

void ConvertAddressToolDialog::OnConvertRange1ToRange2(wxCommandEvent & WXUNUSED(event))
{
	ConvertRangeToRange(XRCID("IDC_EDIT_RANGE1_START"), XRCID("IDC_EDIT_RANGE1_END"), XRCID("IDC_EDIT_RANGE2_START"), XRCID("IDC_EDIT_RANGE2_END"));
}

void ConvertAddressToolDialog::OnConvertRange2ToRange1(wxCommandEvent & WXUNUSED(event))
{
	ConvertRangeToRange(XRCID("IDC_EDIT_RANGE2_START"), XRCID("IDC_EDIT_RANGE2_END"), XRCID("IDC_EDIT_RANGE1_START"), XRCID("IDC_EDIT_RANGE1_END"));
}

ConvertAddressToolDialog::~ConvertAddressToolDialog()
{
}

bool ConvertAddressToolDialog::TransferDataToWindow()
{
	wxTextCtrl *pTextCtrl;

	wxString sValue;

	wxConfig::Get(false)->Read(wxT("debugger/convert_address_tool/range1_start"), &sValue, wxEmptyString);
	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_RANGE1_START", wxTextCtrl);
	pTextCtrl->SetValue(sValue);

	wxConfig::Get(false)->Read(wxT("debugger/convert_address_tool/range1_end"), &sValue, wxEmptyString);
	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_RANGE1_END", wxTextCtrl);
	pTextCtrl->SetValue(sValue);

	wxConfig::Get(false)->Read(wxT("debugger/convert_address_tool/range2_start"), &sValue, wxEmptyString);
	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_RANGE2_START", wxTextCtrl);
	pTextCtrl->SetValue(sValue);

	wxConfig::Get(false)->Read(wxT("debugger/convert_address_tool/range2_end"), &sValue, wxEmptyString);
	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_RANGE2_END", wxTextCtrl);
	pTextCtrl->SetValue(sValue);

	return true;
}

bool ConvertAddressToolDialog::TransferDataFromWindow()
{
	wxTextCtrl *pTextCtrl;

	wxString sValue;

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_RANGE1_START", wxTextCtrl);
	sValue = pTextCtrl->GetValue();
	wxConfig::Get(false)->Write(wxT("debugger/convert_address_tool/range1_start"), sValue);

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_RANGE1_END", wxTextCtrl);
	sValue = pTextCtrl->GetValue();
	wxConfig::Get(false)->Write(wxT("debugger/convert_address_tool/range1_end"), sValue);

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_RANGE2_START", wxTextCtrl);
	sValue = pTextCtrl->GetValue();
	wxConfig::Get(false)->Write(wxT("debugger/convert_address_tool/range2_start"), sValue);

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_RANGE2_END", wxTextCtrl);
	sValue = pTextCtrl->GetValue();
	wxConfig::Get(false)->Write(wxT("debugger/convert_address_tool/range2_end"), sValue);

	return true;

}


ConvertAddressToolDialog::ConvertAddressToolDialog(wxWindow *pParent)
{
	// on windows
	// cannot convert dialog units: dialog unknown

	// load the resource
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_DIALOG_ADDRESS_TOOL"));
}

// io presets to make it easier for debugging
BREAKPOINT_IO_TEMPLATE IOTemplates[] =
{
	{ FALSE, "Dk'Tronics compatible RAM Write", 0x0, TRUE, 0x08000, TRUE, 0x0c0, TRUE, 0x0c0 },			// Dk'tronics compatible memory write
	{ FALSE, "PPI Port A Write", 0x0, TRUE, 0x0b00, FALSE, 0x000, FALSE, 0x0ff },						// write to ppi port A
	{ FALSE, "PPI Port B Write", 0x0100, TRUE, 0x0b00, FALSE, 0x000, FALSE, 0x0ff },						// write to ppi port B
	{ FALSE, "PPI Port C Write", 0x0200, TRUE, 0x0b00, FALSE, 0x000, FALSE, 0x0ff },						// write to ppi port C
	{ FALSE, "PPI Control Write", 0x0300, TRUE, 0x0b00, FALSE, 0x000, FALSE, 0x0ff },						// write to ppi control
	{ TRUE, "PPI Port A Read", 0x0, TRUE, 0x0b00, FALSE, 0x000, FALSE, 0x0ff },						// write to ppi port A
	{ TRUE, "PPI Port B Read", 0x0100, TRUE, 0x0b00, FALSE, 0x000, FALSE, 0x0ff },						// write to ppi port B
	{ TRUE, "PPI Port C Read", 0x0200, TRUE, 0x0b00, FALSE, 0x000, FALSE, 0x0ff },						// write to ppi port C
	{ TRUE, "PPI Control Read", 0x0300, TRUE, 0x0b00, FALSE, 0x000, FALSE, 0x0ff },						// read from ppi control
	{ FALSE, "Gate Array Pen Write", 0x4000, TRUE, 0x0c000, TRUE, 0x000, TRUE, 0x0c0 },						// write to pen
	{ FALSE, "Gate Array Colour Write", 0x4000, TRUE, 0x0c000, TRUE, 0x040, TRUE, 0x0c0 },				// write to colour
	{ FALSE, "Gate Array Mode Write", 0x4000, TRUE, 0x0c000, TRUE, 0x080, TRUE, 0x0c0 },						// write to mode
	{ FALSE, "Gate Array Write (Any)", 0x4000, TRUE, 0x0c000, FALSE, 0x0, FALSE, 0x0 },						// any write to gate-array port
	{ FALSE, "CRTC Write Register", 0x0, TRUE, 0x04300, FALSE, 0x0, FALSE, 0x0 },						// any write to gate-array port
	{ FALSE, "CRTC Write Data", 0x0100, TRUE, 0x04300, FALSE, 0x0, FALSE, 0x0 },						// any write to gate-array port
	{ TRUE, "CRTC Read Status Register", 0x0200, TRUE, 0x04300, FALSE, 0x0, FALSE, 0x0 },						// any write to gate-array port
	{ TRUE, "CRTC Read Data", 0x0300, TRUE, 0x04300, FALSE, 0x0, FALSE, 0x0 },						// any write to gate-array port
	{ FALSE, "ROM select write", 0x0, TRUE, 0x02000, FALSE, 0x0, FALSE, 0x0 },						// any write to gate-array port
	{ FALSE, "Printer data write", 0x0, TRUE, 0x01000, FALSE, 0x0, FALSE, 0x0 },
	{ FALSE, "ASIC RMR2 Write", 0x4000, TRUE, 0x0c000, TRUE, 0x0a0, TRUE, 0x0e0 },
	{ FALSE, "ASIC Register Page Enabled", 0x4000, TRUE, 0x0c000, TRUE, 0x0b8, TRUE, 0x0f8 },
	{ FALSE, "ASIC Register Page Disabled", 0x4000, TRUE, 0x0c000, TRUE, 0x0a0, TRUE, 0x0f8 },
	{ FALSE, "FDC Data Register Write", 0x0100, TRUE, 0x0580, FALSE, 0x0, FALSE, 0x0 },						// write to mode (confirm data address write)
	{ TRUE, "FDC Data Register Read", 0x0101, TRUE, 0x0581, FALSE, 0x0, TRUE, 0x0 },						// write to mode
	{ TRUE, "FDC Read Main Status Register", 0x0100, TRUE, 0X0581, FALSE, 0x0, FALSE, 0x0 },						// write to mode
};

BEGIN_EVENT_TABLE(BreakpointDialog, wxDialog)
EVT_CHOICE(XRCID("m_choicePreset"), BreakpointDialog::OnChoiceChange)
END_EVENT_TABLE()

BreakpointDialog::~BreakpointDialog()
{
}

// doesn't seem to work on 2.8
void BreakpointDialog::OnChoiceChange(wxCommandEvent & WXUNUSED(event))
{
	wxChoice *pChoice;

	// store selected key set even if we do not use this mode.
	// so it can be restored next time dialog is shown
	pChoice = XRCCTRL(*this, "m_choicePreset", wxChoice);

	int nChoice = pChoice->GetSelection();

	if (nChoice != wxNOT_FOUND)
	{
		const IntClientData *pIntData = (const IntClientData *)pChoice->GetClientObject(nChoice);
		if (pIntData != NULL)
		{
			int nPreset = pIntData->GetData();

			m_nAddress = IOTemplates[nPreset].Address;
			m_nAddressMask = IOTemplates[nPreset].AddressMask;
			m_nData = IOTemplates[nPreset].Data;
			m_nDataMask = IOTemplates[nPreset].DataMask;
			m_bUseAddressMask = IOTemplates[nPreset].bUseAddressMask ? true : false;
			m_bUseData = IOTemplates[nPreset].bUseData ? true : false;
			m_bUseDataMask = IOTemplates[nPreset].bUseDataMask ? true : false;

			m_bWrite = !IOTemplates[nPreset].bRead;

			UpdateDialogFromData();
		}
	}
}

void BreakpointDialog::UpdateDialogFromData()
{
	wxTextCtrl *pTextCtrl;
	wxCheckBox *pCheckBox;
	wxChoice *pChoice;
	wxString sValue;

	bool bHasData = (m_nMajorType == BREAKPOINT_MAJOR_TYPE_IO || m_nMajorType == BREAKPOINT_MAJOR_TYPE_MEMORY);

	// set address value
	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_HEX_VALUE", wxTextCtrl);
	sValue.Printf(wxT("&%04x"), m_nAddress);
	pTextCtrl->SetValue(sValue);

	// set address mask yes/no
	pCheckBox = XRCCTRL(*this, "m_UseAddressMask", wxCheckBox);
	pCheckBox->SetValue(m_bUseAddressMask);

	// set address mask value
	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_HEX_ADDR_AND", wxTextCtrl);
	sValue.Printf(wxT("&%04x"), m_nAddressMask);
	pTextCtrl->SetValue(sValue);

	// has data?
	if (bHasData)
	{
		// set data yes/no
		pCheckBox = XRCCTRL(*this, "m_UseData", wxCheckBox);
		pCheckBox->SetValue(m_bUseData);

		// set data value
		pTextCtrl = XRCCTRL(*this, "m_DataValue", wxTextCtrl);
		sValue.Printf(wxT("&%02x"), m_nData);
		pTextCtrl->SetValue(sValue);

		// set data mask yes/no
		pCheckBox = XRCCTRL(*this, "m_UseDataMask", wxCheckBox);
		pCheckBox->SetValue(m_bUseDataMask);

		// set mask
		pTextCtrl = XRCCTRL(*this, "m_DataMask", wxTextCtrl);
		sValue.Printf(wxT("&%02x"), m_nDataMask);
		pTextCtrl->SetValue(sValue);
	}

	if ((m_nMajorType == BREAKPOINT_MAJOR_TYPE_IO) || (m_nMajorType == BREAKPOINT_MAJOR_TYPE_MEMORY))
	{
		pChoice = XRCCTRL(*this, "m_choiceType", wxChoice);

		pChoice->SetSelection(m_bWrite ? 0 : 1);
	}

}

bool BreakpointDialog::TransferDataToWindow()
{
	wxTextCtrl *pTextCtrl;
	wxCheckBox *pCheckBox;
	wxChoice *pChoice;
	wxString sValue;

	if (m_nMajorType == BREAKPOINT_MAJOR_TYPE_IO)
	{
		// fill choice with all possible presets
		IntClientData *pIntData;

		pChoice = XRCCTRL(*this, "m_choicePreset", wxChoice);

		for (size_t i = 0; i < sizeof(IOTemplates) / sizeof(IOTemplates[0]); i++)
		{
			pIntData = new IntClientData(i);

			wxString sName(IOTemplates[i].sName, *wxConvCurrent);
			pChoice->Append(sName, pIntData);
		}
	}

	pCheckBox = XRCCTRL(*this, "m_checkBoxEnable", wxCheckBox);
	pCheckBox->SetValue(m_bEnabled);

	pTextCtrl = XRCCTRL(*this, "m_textHitCount", wxTextCtrl);
	sValue.Printf(wxT("%d"), m_nCount);
	pTextCtrl->SetValue(sValue);

	UpdateDialogFromData();


	return true;
}

bool BreakpointDialog::TransferDataFromWindow()
{
	wxTextCtrl *pTextCtrl;
	wxCheckBox *pCheckBox;
	wxChoice *pChoice;
	wxString sValue;

	pCheckBox = XRCCTRL(*this, "m_checkBoxEnable", wxCheckBox);
	m_bEnabled = pCheckBox->GetValue();

	bool bHasData = (m_nMajorType == BREAKPOINT_MAJOR_TYPE_IO || m_nMajorType == BREAKPOINT_MAJOR_TYPE_MEMORY);


	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_HEX_VALUE", wxTextCtrl);
	sValue = pTextCtrl->GetValue();

	m_nAddress = wxGetApp().ExpressionEvaluate(sValue);

	pTextCtrl = XRCCTRL(*this, "m_textHitCount", wxTextCtrl);
	sValue = pTextCtrl->GetValue();

	m_nCount = wxGetApp().ExpressionEvaluate(sValue);

	pCheckBox = XRCCTRL(*this, "m_UseAddressMask", wxCheckBox);
	m_bUseAddressMask = pCheckBox->GetValue();

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_HEX_ADDR_AND", wxTextCtrl);
	sValue = pTextCtrl->GetValue();

	m_nAddressMask = wxGetApp().ExpressionEvaluate(sValue);

	// has data?
	if (bHasData)
	{
		pCheckBox = XRCCTRL(*this, "m_UseData", wxCheckBox);
		m_bUseData = pCheckBox->GetValue();

		pTextCtrl = XRCCTRL(*this, "m_DataValue", wxTextCtrl);
		sValue = pTextCtrl->GetValue();

		m_nData = wxGetApp().ExpressionEvaluate(sValue);

		pCheckBox = XRCCTRL(*this, "m_UseDataMask", wxCheckBox);
		m_bUseDataMask = pCheckBox->GetValue();

		pTextCtrl = XRCCTRL(*this, "m_DataMask", wxTextCtrl);
		sValue = pTextCtrl->GetValue();

		m_nDataMask = wxGetApp().ExpressionEvaluate(sValue);
	}

	pChoice = XRCCTRL(*this, "m_choiceType", wxChoice);

	if (m_nMajorType == BREAKPOINT_MAJOR_TYPE_IO)
	{

		m_nType = pChoice->GetSelection() ? BREAKPOINT_TYPE_IO_READ : BREAKPOINT_TYPE_IO_WRITE;
	}
	else
		if (m_nMajorType == BREAKPOINT_MAJOR_TYPE_MEMORY)
		{
			m_nType = pChoice->GetSelection() ? BREAKPOINT_TYPE_MEMORY_READ : BREAKPOINT_TYPE_MEMORY_WRITE;
		}
		else
		{
			m_nType = pChoice->GetSelection() ? BREAKPOINT_TYPE_SP : BREAKPOINT_TYPE_PC;

		}
	return true;

}

void BreakpointDialog::SetBreakpoint(BREAKPOINT *pBreakpoint)
{
	m_bEnabled = pBreakpoint->bEnabled ? true : false;
	m_nCount = pBreakpoint->Description.nReloadHitCount;
	m_nAddress = pBreakpoint->Description.Address;
	m_nAddressMask = pBreakpoint->Description.AddressMask;
	m_nDataMask = pBreakpoint->Description.DataMask;
	m_nData = pBreakpoint->Description.Data;
	m_nType = pBreakpoint->Description.Type;
}


BreakpointDialog::BreakpointDialog(BREAKPOINT_MAJOR_TYPE nMajorType, int nAddress,wxWindow *pParent)
{
	m_nType = BREAKPOINT_TYPE_PC;
	m_nMajorType = nMajorType;

	m_bEnabled = true;
	m_nCount = 1;
	m_nAddress = nAddress;
	m_nAddressMask = 0x0ffff;
	// m_nType = 0;
	m_bUseAddressMask = false;
	m_bUseDataMask = false;
	m_bUseData = false;
	m_nDataMask = 0x0ff;
	m_nData = 0x000;
	m_bWrite = false;

	const wxChar *sDialogName = wxT("DLG_DIALOG_REGISTER_BREAKPOINT");

	switch (nMajorType)
	{
	default:
	case BREAKPOINT_MAJOR_TYPE_REGISTER:
	{
		sDialogName = wxT("DLG_DIALOG_REGISTER_BREAKPOINT");
	}
	break;
	case BREAKPOINT_MAJOR_TYPE_MEMORY:
	{
		sDialogName = wxT("DLG_DIALOG_MEMORY_BREAKPOINT");
	}
	break;
	case BREAKPOINT_MAJOR_TYPE_IO:
	{
		sDialogName = wxT("DLG_DIALOG_IO_BREAKPOINT");
	}
	break;
	}

	// load the resource
	wxXmlResource::Get()->LoadDialog(this, pParent, sDialogName);
}






OurTextWindowProperties g_TextWindowProperties;



void OurTextWindowProperties::Init()
{
	m_nBorderWidth = 2;

	// get the face name the user specified in the config
	// defaulting to our default if it's not found
	wxString sDefault = wxT("Courier New");
	wxString sValue;
	wxConfig::Get(false)->Read(wxT("debugger/font_face_name"), &sValue, sDefault);

	wxString sFaceNameToUse;

	bool bFoundFaceName = false;
	wxArrayString sNames = wxFontEnumerator::GetFacenames(wxFONTENCODING_SYSTEM, true);

	// find the name from the config
	for (unsigned int i = 0; i != sNames.GetCount(); i++)
	{
		wxString sName = sNames.Item(i);
		if (sName == sValue)
		{
			sFaceNameToUse = sName;
			bFoundFaceName = true;
			break;
		}
	}

	if (!bFoundFaceName)
	{
		// failed, find our default as a backup
		for (unsigned int i = 0; i != sNames.GetCount(); i++)
		{
			wxString sName = sNames.Item(i);
			if (sName == sDefault)
			{
				sFaceNameToUse = sName;
				bFoundFaceName = true;
				break;
			}
	}
}

	if (!bFoundFaceName)
	{
		// found neither, default to first in list and hope it's a good un
		sFaceNameToUse = sNames[0];
	}

	// find the size
	int nValue;
#ifdef __WXMAC__
	wxConfig::Get(false)->Read(wxT("debugger/font_point_size"), &nValue, 13);
#else
	wxConfig::Get(false)->Read(wxT("debugger/font_point_size"), &nValue, 10);
#endif	
	m_nPointSize = nValue;
	m_sFaceName = sFaceNameToUse;

	// now create font based on size and face
	m_Font.Create(m_nPointSize,
		wxFONTFAMILY_TELETYPE,
		wxFONTSTYLE_NORMAL,
		wxFONTWEIGHT_NORMAL,
		false,
		m_sFaceName,
		wxFONTENCODING_SYSTEM);

	// load breakpoint bitmap
	wxString sFilename;
	wxFileName BreakPointBitmapPath(wxGetApp().GetAppPath(), wxT("breakpoint.png"));
	sFilename = BreakPointBitmapPath.GetFullPath();
	if (m_BreakpointBitmap.LoadFile(sFilename, wxBITMAP_TYPE_PNG))
	{
		wxMask *breakpointBitmapMask = new wxMask(m_BreakpointBitmap, wxColour(255, 0, 255));
		if (breakpointBitmapMask != NULL)
		{
			m_BreakpointBitmap.SetMask(breakpointBitmapMask);
		}
	}


	// load disabled breakpoint bitmap
	wxFileName DisabledBreakPointBitmapPath(wxGetApp().GetAppPath(), wxT("disabledbreakpoint.png"));
	sFilename = DisabledBreakPointBitmapPath.GetFullPath();
	if (m_DisabledBreakpointBitmap.LoadFile(sFilename, wxBITMAP_TYPE_PNG))
	{
		wxMask *breakpointBitmapMask = new wxMask(m_BreakpointBitmap, wxColour(255, 0, 255));
		if (breakpointBitmapMask != NULL)
		{
			m_DisabledBreakpointBitmap.SetMask(breakpointBitmapMask);
		}
	}


	// load cursor bitmap
	wxFileName CursorBitmapPath(wxGetApp().GetAppPath(), wxT("current.png"));
	sFilename = CursorBitmapPath.GetFullPath();
	if (m_CursorBitmap.LoadFile(sFilename, wxBITMAP_TYPE_PNG))
	{
		wxMask *cursorBitmapMask = new wxMask(m_CursorBitmap, wxColour(255, 0, 255));
		if (cursorBitmapMask != NULL)
		{
			m_CursorBitmap.SetMask(cursorBitmapMask);
		}
	}

	wxColour Default;
	wxColour Value;

	Default.Set(150, 150, 150);
	wxGetApp().ReadConfigColour(wxT("debugger/margin_colour"), Value, Default);
	m_MarginColour = Value;

	Default.Set(0x0d0, 0x0d0, 0x0d0);
	wxGetApp().ReadConfigColour(wxT("debugger/background_colour"), Value, Default);
	m_BackgroundColour = Value;

	Default.Set(255, 201, 20);
	wxGetApp().ReadConfigColour(wxT("debugger/changed_colour"), Value, Default);
	m_ValueChangedColour = Value;



	Default.Set(0, 0, 0);
	wxGetApp().ReadConfigColour(wxT("debugger/text_colour"), Value, Default);
	m_TextColour = Value;

	Default.Set(0x0ff, 0, 0);
	wxGetApp().ReadConfigColour(wxT("debugger/text_modified_colour"), Value, Default);
	m_TextModifiedColour = Value;

	Default.Set(0x0a0,0x0a0,0x0d0);
	wxGetApp().ReadConfigColour(wxT("debugger/cursor_colour"), Value, Default);
	m_CursorColour = Value;

	Default.Set(255,255,255);
	wxGetApp().ReadConfigColour(wxT("debugger/text_under_cursor_colour"), Value, Default);
	m_TextUnderCursorColour = Value;

	Default.Set(255, 0, 0);
	wxGetApp().ReadConfigColour(wxT("debugger/selected_border_colour"), Value, Default);
	m_SelectedBorderColour = Value;

	Default.Set(128, 0, 0);
	wxGetApp().ReadConfigColour(wxT("debugger/unselected_border_colour"), Value, Default);
	m_UnSelectedBorderColour = Value;
#if (wxVERSION_NUMBER >= 3100)

	m_BackgroundBrush = wxBrush(g_TextWindowProperties.m_BackgroundColour, wxBRUSHSTYLE_SOLID);
	m_MarginPen = wxPen(g_TextWindowProperties.m_MarginColour, 1);
	m_MarginBrush = wxBrush(g_TextWindowProperties.m_MarginColour, wxBRUSHSTYLE_SOLID);
	m_CursorBrush = wxBrush(g_TextWindowProperties.m_CursorColour, wxBRUSHSTYLE_SOLID);
	m_CursorPen = wxPen(g_TextWindowProperties.m_CursorColour, 1, wxPENSTYLE_SOLID);
	m_SelectedBorderPen = wxPen(g_TextWindowProperties.m_SelectedBorderColour, m_nBorderWidth);
	m_UnSelectedBorderPen = wxPen(g_TextWindowProperties.m_UnSelectedBorderColour, m_nBorderWidth);
#else

	m_BackgroundBrush = wxBrush(g_TextWindowProperties.m_BackgroundColour, wxSOLID);
	m_MarginPen = wxPen(g_TextWindowProperties.m_MarginColour, 1);
	m_MarginBrush = wxBrush(g_TextWindowProperties.m_MarginColour, wxSOLID);
	m_CursorBrush = wxBrush(g_TextWindowProperties.m_CursorColour, wxSOLID);
	m_CursorPen = wxPen(g_TextWindowProperties.m_CursorColour, 1, wxSOLID);
	m_SelectedBorderPen = wxPen(g_TextWindowProperties.m_SelectedBorderColour, m_nBorderWidth);
	m_UnSelectedBorderPen = wxPen(g_TextWindowProperties.m_UnSelectedBorderColour, m_nBorderWidth);
#endif
}

void OurTextWindow::UpdateContextMenuState(wxMenuBar *)
{
}



void OurTextWindow::SendDataToRegister(int nData)
{
	wxArrayString sRegisters;

	sRegisters.Add(wxT("HL"));
	sRegisters.Add(wxT("DE"));
	sRegisters.Add(wxT("BC"));
	sRegisters.Add(wxT("HL'"));
	sRegisters.Add(wxT("DE'"));
	sRegisters.Add(wxT("BC'"));
	sRegisters.Add(wxT("IX"));
	sRegisters.Add(wxT("IY"));
	sRegisters.Add(wxT("PC"));
	sRegisters.Add(wxT("SP"));

	wxSingleChoiceDialog dialog(this, wxT("Select register"), wxT("Send Cursor Address To..."), sRegisters);
	if (dialog.ShowModal() == wxID_OK)
	{
		int nRegister = CPU_HL;
		wxString sSelection = dialog.GetStringSelection();
		if (sSelection == wxT("HL"))
		{
			nRegister = CPU_HL;
		}
		else if (sSelection == wxT("DE"))
		{
			nRegister = CPU_DE;
		}
		else if (sSelection == wxT("BC"))
		{
			nRegister = CPU_BC;
		}
		else if (sSelection == wxT("HL'"))
		{
			nRegister = CPU_HL2;
		}
		else if (sSelection == wxT("DE'"))
		{
			nRegister = CPU_DE2;
		}
		else if (sSelection == wxT("BC'"))
		{
			nRegister = CPU_BC2;
		}
		else if (sSelection == wxT("IX"))
		{
			nRegister = CPU_IX;
		}
		else if (sSelection == wxT("IY"))
		{
			nRegister = CPU_IY;
		}
		else if (sSelection == wxT("PC"))
		{
			nRegister = CPU_PC;
		}
		else if (sSelection == wxT("SP"))
		{
			nRegister = CPU_SP;
		}
		CPU_SetReg(nRegister, nData);
	}
}

bool OurTextWindow::GetGoToLabel(int *nAddress)
{
	wxString sValue;

	wxString sConfigKey = wxT("debugger/go_to_label");
	wxConfig::Get(false)->Read(sConfigKey, &sValue);

	// todo autocomplete
	wxTextEntryDialog dialog(this, wxT("Label name"), wxT("Add Label at Cursor"), sValue);
	if (dialog.ShowModal() == wxID_OK)
	{
		wxConfig::Get(false)->Write(sConfigKey, dialog.GetValue());

#if (wxVERSION_NUMBER >= 2900)
		// 2.9
		const wxCharBuffer tmp_buf = wxConvCurrent->cWX2MB(dialog.GetValue().wc_str());
#else
		// 2.8
		const wxWX2MBbuf tmp_buf = wxConvCurrent->cWX2MB(dialog.GetValue());
#endif
		const char *tmp_str = (const char*)tmp_buf;

		LABEL *pLabel = labelsets_find_label_by_name(tmp_str);
		if (pLabel != NULL)
		{

			*nAddress = pLabel->m_Address;
			return true;
		}
	}
	return false;

}

bool OurTextWindow::GetGotoAddress(int *nAddress)
{
	wxString sValue;
	wxString sConfigKey = wxT("debugger/go_to_address");
	wxConfig::Get(false)->Read(sConfigKey, &sValue);

	bool bGotAddress = false;
	wxString sMessage = wxT("Please enter an expression to evaluate");
	wxTextEntryDialog dialog(this, sMessage, wxT(""), wxT(""), wxOK | wxCANCEL);
	dialog.SetValue(sValue);
	if (dialog.ShowModal() == wxID_OK)
	{
		wxString str = dialog.GetValue();
		if (str.Length() != 0)
		{
			wxConfig::Get(false)->Write(sConfigKey, str);

			*nAddress = wxGetApp().ExpressionEvaluate(str);
			bGotAddress = true;
		}
	}

	return bGotAddress;
}

void OurTextWindow::OnEraseBackground(wxEraseEvent & WXUNUSED(event))
{

}

void OurTextWindow::DoTab(bool WXUNUSED(bForwards))
{


}

void OurTextWindow::DoSelect(int WXUNUSED(x), int WXUNUSED(y))
{

}

void OurTextWindow::DoClickMargin(int WXUNUSED(x), int WXUNUSED(y))
{


}


void OurTextWindow::DoCursorUp()
{

}

void OurTextWindow::DoCursorDown()
{

}


void OurTextWindow::DoCursorLeft()
{

}

void OurTextWindow::DoCursorRight()
{

}

void OurTextWindow::DoPageUp()
{

}

void OurTextWindow::DoPageDown()
{

}

void OurTextWindow::OnContextMenu(wxContextMenuEvent &event)
{
	wxPoint Position = event.GetPosition();

	// generate a suitable place for the menu
	if (event.GetPosition() == wxDefaultPosition)
	{
		wxPoint ClientPos(0, 0);
		Position = this->ClientToScreen(ClientPos);
	}

	wxMenuBar *pMenuBar = wxXmlResource::Get()->LoadMenuBar(m_sContextMenuName);
	if (pMenuBar)
	{
		UpdateContextMenuState(pMenuBar);

		wxMenu *pMenu = pMenuBar->Remove(0);
		if (pMenu)
		{

			PopupMenu(pMenu);
			delete pMenu;
		}
		delete pMenuBar;
	}
}

BEGIN_EVENT_TABLE(OurTextWindow, wxWindow)
EVT_ERASE_BACKGROUND(OurTextWindow::OnEraseBackground)
EVT_PAINT(OurTextWindow::OnPaint)
EVT_KEY_DOWN(OurTextWindow::OnKeyDown)
EVT_SIZE(OurTextWindow::OnSize)
EVT_CONTEXT_MENU(OurTextWindow::OnContextMenu)
EVT_SET_FOCUS(OurTextWindow::OnSetFocus)
EVT_KILL_FOCUS(OurTextWindow::OnKillFocus)
EVT_MOUSEWHEEL(OurTextWindow::OnMouseWheel)
EVT_LEFT_DOWN(OurTextWindow::OnMouseLeftButtonDown)
END_EVENT_TABLE()

void OurTextWindow::OnMouseLeftButtonDown(wxMouseEvent &event)
{
	if (m_bHasMargin)
	{
		if (event.m_x < MARGIN_WIDTH)
		{
			DoClickMargin(event.m_x, event.m_y);
			return;
		}
	}


	DoSelect(event.m_x, event.m_y);
	this->Refresh();

	event.Skip();
}

void OurTextWindow::OnSetFocus(wxFocusEvent & WXUNUSED(event))
{
	m_bHasFocus = true;
	this->Refresh();
}

void OurTextWindow::OnKillFocus(wxFocusEvent & WXUNUSED(event))
{
	m_bHasFocus = false;
	this->Refresh();
}

void OurTextWindow::OnSize(wxSizeEvent & WXUNUSED(event))
{
	this->Refresh();
}

void OurTextWindow::OnMouseWheel(wxMouseEvent & event)
{
	int Delta = event.GetWheelDelta();
	if (Delta == 0)
		return;

	int Movement = event.GetWheelRotation();
	if (Movement == 0)
		return;

	int nLines = Movement / Delta;

	if (nLines < 0)
	{
		nLines = -nLines;
		for (int i = 0; i < nLines; i++)
		{
			DoCursorDown();
		}
	}
	else
	{
		for (int i = 0; i < nLines; i++)
		{
			DoCursorUp();
		}
	}
	this->Refresh();

}

void OurTextWindow::OnKeyDown(wxKeyEvent &event)
{
	switch (event.GetKeyCode())
	{
	default:
		break;

	case WXK_TAB:
	{
		DoTab(!(event.GetModifiers() == wxMOD_SHIFT));
		this->Refresh();
		return;
	}
	break;

	case WXK_NUMPAD_LEFT:
	case WXK_LEFT:
	{
		if (event.GetModifiers() == 0)
		{
			DoCursorLeft();
			this->Refresh();
			return;
		}
	}
	break;

	case WXK_NUMPAD_RIGHT:
	case WXK_RIGHT:
	{
		if (event.GetModifiers() == 0)
		{
			DoCursorRight();
			this->Refresh();
			return;
		}
	}
	break;

	case WXK_NUMPAD_UP:
	case WXK_UP:
	{
		if (event.GetModifiers() == 0)
		{
			DoCursorUp();
			this->Refresh();
			return;
		}
	}
	break;

	case WXK_NUMPAD_DOWN:
	case WXK_DOWN:
	{
		if (event.GetModifiers() == 0)
		{
			DoCursorDown();
			this->Refresh();
			return;
		}
	}
	break;

	case WXK_NUMPAD_PAGEUP:
	case WXK_PAGEUP:
	{
		if (event.GetModifiers() == 0)
		{
			DoPageUp();
			this->Refresh();
			return;
		}
	}
	break;

	case WXK_NUMPAD_PAGEDOWN:
	case WXK_PAGEDOWN:
	{
		if (event.GetModifiers() == 0)
		{
			DoPageDown();
			this->Refresh();

			return;
		}
	}
	break;

	}
	event.Skip();
}
OurTextWindow::~OurTextWindow()
{

}



OurTextWindow::OurTextWindow()
{
	m_bHasFocus = false;
	m_bHasMargin = false;
	m_nCharsWidth = 0;
	m_nCharsHeight = 0;
}

bool OurTextWindow::Create(wxWindow* parent, wxWindowID id, const wxString & WXUNUSED(label), const wxPoint& pos, const wxSize& size, long style, const wxValidator & WXUNUSED(validator), const wxString& name)
{
	style |= wxNO_FULL_REPAINT_ON_RESIZE;
	style |= wxCLIP_CHILDREN;
	style |= wxWANTS_CHARS;		// we want to accept all chars so we can process them
	style |= wxTAB_TRAVERSAL;	// we want to be tabbed into and out of.
	bool bResult = wxWindow::Create(parent, id, pos, size, style, name);
	if (bResult)
	{
		SetBackgroundStyle(wxBG_STYLE_CUSTOM);
	}
	return bResult;
}

void OurTextWindow::DrawMarkerBitmap(wxAutoBufferedPaintDC  &dc, wxBitmap &Bitmap, int y)
{
	int drawX = ClientOrigin.x + (MARGIN_WIDTH >> 1) - (Bitmap.GetWidth() >> 1);
	int drawY = TextOrigin.y + (y*m_nFontHeight) + ((m_nFontHeight >> 1) - (Bitmap.GetHeight() >> 1));
	dc.DrawBitmap(Bitmap, drawX, drawY, true);
}


void OurTextWindow::DrawBreakpointMarker(wxAutoBufferedPaintDC  &dc, int y, bool bEnabled)
{
	wxBitmap &Bitmap = bEnabled ? g_TextWindowProperties.m_BreakpointBitmap : g_TextWindowProperties.m_DisabledBreakpointBitmap;
	if (!Bitmap.IsNull())
	{
		DrawMarkerBitmap(dc, Bitmap, y);
	}
}


void OurTextWindow::DrawCurrentMarker(wxAutoBufferedPaintDC  &dc, int y)
{
	wxBitmap &Bitmap = g_TextWindowProperties.m_CursorBitmap;
	if (!Bitmap.IsNull())
	{
		DrawMarkerBitmap(dc, Bitmap, y);
	}
}

void OurTextWindow::DrawCursor(wxAutoBufferedPaintDC  &dc, int x, int y, int Width, int Height)
{
	dc.SetPen(g_TextWindowProperties.m_CursorPen);
	dc.SetBrush(g_TextWindowProperties.m_CursorBrush);

	dc.DrawRectangle(TextOrigin.x + (x*m_nFontWidth), TextOrigin.y + (y*m_nFontHeight), Width*m_nFontWidth, Height*m_nFontHeight);

	dc.SetBrush(wxNullBrush);
	dc.SetPen(wxNullPen);
}


/*
void OurTextWindow::SetupText(bool isModified)
{

dc.SetTextBackground()

if (isModified)
{
dc.SetTextForeground(m_ModifiedColour);
}
else
{
dc.SetTextForeground(m_TextColour);
}
}
*/
#include <wx/brush.h>

void OurTextWindow::PixelToChar(int PixelX, int PixelY, int *pCharX, int *pCharY)
{
	*pCharX = (PixelX - TextOrigin.x) / m_nFontWidth;
	*pCharY = (PixelY - TextOrigin.y) / m_nFontHeight;
}

void OurTextWindow::OnPaint(wxPaintEvent & WXUNUSED(event))
{
	// wxStopWatch sw;
	//sw.Start();
	wxAutoBufferedPaintDC  dc(this);


	int nWidth, nHeight;
	this->GetClientSize(&nWidth, &nHeight);

	ClientOrigin.x = g_TextWindowProperties.m_nBorderWidth;
	ClientOrigin.y = g_TextWindowProperties.m_nBorderWidth;
	ClientSize.Set(nWidth - (g_TextWindowProperties.m_nBorderWidth * 2), nHeight - (g_TextWindowProperties.m_nBorderWidth * 2));
	TextOrigin = ClientOrigin;
	TextSize = ClientSize;

	// clear background
	//  dc.SetBrush(g_TextWindowProperties.m_BackgroundBrush);

	// for 3.0 to render background correctly we need this
	dc.SetBackground(g_TextWindowProperties.m_BackgroundBrush);
	dc.Clear();

	// clear background
	//  dc.SetBrush(g_TextWindowProperties.m_BackgroundBrush);
	//dc.SetBrush(wxNullBrush);

	// draw margin if we have one over top
	if (m_bHasMargin)
	{
		dc.SetPen(g_TextWindowProperties.m_MarginPen);
		dc.SetBrush(g_TextWindowProperties.m_MarginBrush);
		dc.DrawRectangle(ClientOrigin.x, ClientOrigin.y, MARGIN_WIDTH, ClientSize.GetHeight());
		dc.SetPen(wxNullPen);
		dc.SetBrush(wxNullBrush);

		// adjust text
		TextSize.SetWidth(ClientSize.GetWidth() - MARGIN_WIDTH);
		TextOrigin.x += MARGIN_WIDTH;
	}

	//    int nBackgroundMode = dc.GetBackgroundMode();

	// faster transparent brush OR transparent background mode?

	// draw text with background colour
	//  dc.SetBackgroundMode(wxTRANSPARENT);
	dc.SetBrush((*wxTRANSPARENT_BRUSH));

	dc.SetFont(g_TextWindowProperties.m_Font);

	dc.SetTextForeground(g_TextWindowProperties.m_TextColour);

	m_nFontHeight = dc.GetCharHeight();
	m_nFontWidth = dc.GetCharWidth();
	if ((m_nFontWidth > 0) && (m_nFontHeight > 0) && (TextSize.GetWidth() > 0) && (TextSize.GetHeight() > 0))
	{
		m_nCharsWidth = TextSize.GetWidth() / m_nFontWidth;
		m_nCharsHeight = TextSize.GetHeight() / m_nFontHeight;

		// set text background
		DoOnPaint(dc);
	}

	dc.SetFont(wxNullFont);
	dc.SetBrush(wxNullBrush);
	//dc.SetBackgroundMode(nBackgroundMode);



	if (m_bHasFocus)
	{
		dc.SetPen(g_TextWindowProperties.m_SelectedBorderPen);

	}
	else
	{
		dc.SetPen(g_TextWindowProperties.m_UnSelectedBorderPen);
	}
	//    dc.SetBrush((*wxTRANSPARENT_BRUSH));
	// dc.DrawRectangle(0,0, nWidth, nHeight);
	// faster? draw rectangle or draw lines?
	dc.DrawLine(0, 0, nWidth, 0);
	dc.DrawLine(nWidth, 0, nWidth, nHeight);
	dc.DrawLine(nWidth, nHeight, 0, nHeight);
	dc.DrawLine(0, nHeight, 0, 0);
	dc.SetPen(wxNullPen);
	//  dc.SetBrush(wxNullBrush);

	//   long nValue = sw.Time();
	//printf("%d\n",nValue);
}

BEGIN_EVENT_TABLE(MemDumpWindow, OurTextWindow)
EVT_COMMAND(XRCID("SendToDissassembly"), wxEVT_COMMAND_MENU_SELECTED, MemDumpWindow::OnSendCursorAddressToDissassembly)
EVT_COMMAND(XRCID("SendToRegister"), wxEVT_COMMAND_MENU_SELECTED, MemDumpWindow::OnSendCursorAddressToRegister)
EVT_COMMAND(XRCID("SendToStack"), wxEVT_COMMAND_MENU_SELECTED, MemDumpWindow::OnSendCursorAddressToStack)
EVT_COMMAND(XRCID("GoToAddress"), wxEVT_COMMAND_MENU_SELECTED, MemDumpWindow::OnGotoAddress)
EVT_COMMAND(XRCID("GoToLabel"), wxEVT_COMMAND_MENU_SELECTED, MemDumpWindow::OnGotoLabel)
EVT_COMMAND(XRCID("m_SetMemoryRange"), wxEVT_COMMAND_MENU_SELECTED, MemDumpWindow::OnSetMemoryRange)
EVT_COMMAND(XRCID("m_Breakpoint"), wxEVT_COMMAND_MENU_SELECTED, MemDumpWindow::OnBreakpoint)
EVT_COMMAND(XRCID("ID_MEMDUMP_LOADDATA"), wxEVT_COMMAND_MENU_SELECTED, MemDumpWindow::OnLoadData)
EVT_COMMAND(XRCID("ID_MEMDUMP_FILLMEMORY"), wxEVT_COMMAND_MENU_SELECTED, MemDumpWindow::OnFillMemory)
EVT_COMMAND(XRCID("ID_MEMDUMP_SAVEDATA"), wxEVT_COMMAND_MENU_SELECTED, MemDumpWindow::OnSaveData)
EVT_COMMAND(XRCID("ID_MEMDUMP_SEARCHFORDATA"), wxEVT_COMMAND_MENU_SELECTED, MemDumpWindow::OnSearchForData)
EVT_COMMAND(XRCID("ID_MEMDUMP_SEARCHNEXT"), wxEVT_COMMAND_MENU_SELECTED, MemDumpWindow::OnSearchNext)
EVT_CHAR(MemDumpWindow::OnChar)
EVT_KEY_DOWN(MemDumpWindow::OnKeyDown)

EVT_COMMAND(XRCID("ByteData"), wxEVT_COMMAND_MENU_SELECTED, MemDumpWindow::OnByteDataSize)
EVT_COMMAND(XRCID("WordData"), wxEVT_COMMAND_MENU_SELECTED, MemDumpWindow::OnWordDataSize)
EVT_COMMAND(XRCID("SevenBitASCII"), wxEVT_COMMAND_MENU_SELECTED, MemDumpWindow::OnSevenBitASCII)
EVT_KILL_FOCUS(MemDumpWindow::OnKillFocus)
END_EVENT_TABLE()

void MemDumpWindow::OnBreakpoint(wxCommandEvent & WXUNUSED(event))
{
	Breakpoints::AddBreakpointI(BREAKPOINT_MAJOR_TYPE_MEMORY, Memdump_GetCursorAddress(pMemdumpWindow),this);
}

void MemDumpWindow::UpdateContextMenuState(wxMenuBar *pMenuBar)
{
	arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ByteData"), Memdump_GetView(pMemdumpWindow) == MEMDUMP_VIEW_BYTES ? true : false);
	arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("WordData"), Memdump_GetView(pMemdumpWindow) == MEMDUMP_VIEW_WORDS ? true : false);
	arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("SevenBitASCII"), Memdump_GetSevenBitASCII(pMemdumpWindow) ? true : false);
}


void MemDumpWindow::OnKillFocus(wxFocusEvent &event)
{
	OurTextWindow::OnKillFocus(event);

	Memdump_DoEdit(pMemdumpWindow);
	this->Refresh();
	UpdateDissassembly();
}

void MemDumpWindow::OnSetMemoryRange(wxCommandEvent & WXUNUSED(event))
{
	wxArrayString sRanges;
	for (int i = 0; i < CPC_GetRegisteredMemoryRangeCount(); i++)
	{
		const MemoryRange *pRange = CPC_GetRegisteredMemoryRange(i);
		wxString sRange;
		sRange = wxString::FromAscii(pRange->sName);
		sRanges.Add(sRange);
	}

	wxString sMessage;
	wxSingleChoiceDialog dialog(this, wxT("Memory range to use:"), wxT("Choose memory range"), sRanges);
	dialog.SetSelection(0);
	if (dialog.ShowModal() == wxID_OK)
	{
		int nRange = dialog.GetSelection();
		const MemoryRange *pRange = CPC_GetRegisteredMemoryRange(nRange);
		Memdump_SetMemoryRange(pMemdumpWindow, (MemoryRange *)pRange);
	}

	Refresh();

}


void MemDumpWindow::OnSevenBitASCII(wxCommandEvent & WXUNUSED(event))
{
	Memdump_SetSevenBitASCII(pMemdumpWindow, !Memdump_GetSevenBitASCII(pMemdumpWindow));
	this->Refresh();
}



void MemDumpWindow::OnByteDataSize(wxCommandEvent & WXUNUSED(event))
{
	Memdump_SetView(pMemdumpWindow, MEMDUMP_VIEW_BYTES);
	this->Refresh();
}


void MemDumpWindow::OnWordDataSize(wxCommandEvent & WXUNUSED(event))
{
	Memdump_SetView(pMemdumpWindow, MEMDUMP_VIEW_WORDS);
	this->Refresh();
}

void MemDumpWindow::OnKeyDown(wxKeyEvent &event)
{
	switch (event.GetKeyCode())
	{
	default:
		break;

#if !defined(__WXMAC__)
	case WXK_F5:
	{
		if (event.GetModifiers() == 0)
		{
			Refresh();
			return;
		}

		}
	break;

	case WXK_F3:
	{
		if (event.GetModifiers() == 0)
		{

			PerformSearch();
			return;
		}
	}
	break;
#endif
	}
#if wxUSE_UNICODE==1
	wxChar ch = event.GetUnicodeKey();
#else
	wxChar ch = (wxChar)event.GetKeyCode();
#endif
	if (event.GetModifiers() == wxMOD_CMD)
	{
		// control, not alt, not shift
		if ((ch == wxT('F')) || (ch == wxT('f')))
		{
			DoFind();
			return;
		}
	}


	OurTextWindow::OnKeyDown(event);
	}


void MemDumpWindow::PerformSearch()
{
	if (SearchData.NumBytes == 0)
	{
		wxMessageBox(wxT("Please enter data to search for"),
			wxGetApp().GetAppName(), wxICON_INFORMATION | wxOK);
	}
	else
	{
		if (Memdump_FindData(pMemdumpWindow, &SearchData) == -1)
		{
			wxMessageBox(wxT("Data not found"),
				wxGetApp().GetAppName(), wxICON_INFORMATION | wxOK);
		}
		else
		{

			// active window?
			DissassembleWindow *pWindow = DebuggerWindow::GetDissassemblyWindowByIndex(0);
			if (pWindow)
			{
				int nAddress = Memdump_GetCursorAddress(pMemdumpWindow);
				DISSASSEMBLE_WINDOW *pDisassembleWindow = pWindow->GetStruct();
				Dissassemble_SetAddress(pDisassembleWindow, nAddress);
				pWindow->Refresh();
			}
		}
	}
	this->Refresh();
}

void MemDumpWindow::OnSearchNext(wxCommandEvent & WXUNUSED(event))
{
	PerformSearch();
}

BEGIN_EVENT_TABLE(FillMemoryDialog, wxDialog)
END_EVENT_TABLE()


FillMemoryDialog::~FillMemoryDialog()
{
}

bool FillMemoryDialog::TransferDataToWindow()
{
	wxTextCtrl *pTextCtrl;
	wxString sValue;

	wxConfig::Get(false)->Read(wxT("debugger/fillmemory/start"), &sValue, wxT(""));

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_START_ADDRESS", wxTextCtrl);
	pTextCtrl->SetValue(sValue);


	wxConfig::Get(false)->Read(wxT("debugger/fillmemory/end"), &sValue, wxT(""));

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_END_ADDRESS", wxTextCtrl);
	pTextCtrl->SetValue(sValue);


	wxConfig::Get(false)->Read(wxT("debugger/fillmemory/data"), &sValue, wxT(""));

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_DATA", wxTextCtrl);
	pTextCtrl->SetValue(sValue);

	return true;
}



extern "C"
{
	extern char SearchString[256];
	extern char SearchStringMask[256];
}


bool FillMemoryDialog::TransferDataFromWindow()
{
	wxTextCtrl *pTextCtrl;
	wxString sValue;


	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_START_ADDRESS", wxTextCtrl);
	sValue = pTextCtrl->GetValue();
	wxConfig::Get(false)->Write(wxT("debugger/fillmemory/start"), sValue);

	m_nStart = wxGetApp().ExpressionEvaluate(sValue);

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_END_ADDRESS", wxTextCtrl);
	sValue = pTextCtrl->GetValue();
	wxConfig::Get(false)->Write(wxT("debugger/fillmemory/end"), sValue);

	m_nEnd = wxGetApp().ExpressionEvaluate(sValue);

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_DATA", wxTextCtrl);
	sValue = pTextCtrl->GetValue();
	wxConfig::Get(false)->Write(wxT("debugger/fillmemory/data"), sValue);


#if (wxVERSION_NUMBER >= 2900)
	// 2.9
	const wxCharBuffer tmp_buf = wxConvCurrent->cWX2MB(sValue.wc_str());
#else
	// 2.8
	const wxWX2MBbuf tmp_buf = wxConvCurrent->cWX2MB(sValue);
#endif
	const char *tmp_str = (const char*)tmp_buf;

	HexSearch_Evaluate(tmp_str, &FillStringCount, &pFillString, &pFillStringMask);
	return true;
}


FillMemoryDialog::FillMemoryDialog(wxWindow *pParent)
{
	m_nStart = 0;
	m_nEnd = 0;
	FillStringCount = 0;
	pFillString = NULL;
	pFillStringMask = NULL;
	// load the resource
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_DIALOG_FILLMEMORY"));
}

BEGIN_EVENT_TABLE(MemdumpSearchDialog, wxDialog)
END_EVENT_TABLE()



MemdumpSearchDialog::~MemdumpSearchDialog()
{
}

bool MemdumpSearchDialog::TransferDataToWindow()
{
	wxChoice *pChoice;
	wxTextCtrl *pTextCtrl;
	wxCheckBox *pCheckbox;

	bool bValue;
	wxConfig::Get(false)->Read(wxT("debugger/memdump/search/case_sensitive_text_search"), &bValue, false);

	pCheckbox = XRCCTRL(*this, "m_CaseSensitiveTextSearch", wxCheckBox);
	pCheckbox->SetValue(bValue);

	wxConfig::Get(false)->Read(wxT("debugger/memdump/search/sync_dissassembly"), &bValue, false);

	pCheckbox = XRCCTRL(*this, "m_SyncDissassembly", wxCheckBox);
	pCheckbox->SetValue(bValue);
	int nValue;
	wxConfig::Get(false)->Read(wxT("debugger/memdump/search/type"), &nValue, 0);

	pChoice = XRCCTRL(*this, "m_ChoiceSearchType", wxChoice);
	pChoice->SetSelection(nValue);

	wxString sValue;
	wxConfig::Get(false)->Read(wxT("debugger/memdump/search/text"), &sValue, wxT(""));

	pTextCtrl = XRCCTRL(*this, "m_TextSearch", wxTextCtrl);
	pTextCtrl->SetValue(sValue);

	return true;
}

bool MemdumpSearchDialog::TransferDataFromWindow()
{
	wxChoice *pChoice;
	wxTextCtrl *pTextCtrl;
	wxCheckBox *pCheckbox;

	bool bCaseSensitiveSearch;

	pCheckbox = XRCCTRL(*this, "m_CaseSensitiveTextSearch", wxCheckBox);
	bCaseSensitiveSearch = pCheckbox->GetValue();

	wxConfig::Get(false)->Write(wxT("debugger/memdump/search/case_sensitive_text_search"), bCaseSensitiveSearch);

	bool bSyncDissassembly = false;

	pCheckbox = XRCCTRL(*this, "m_SyncDissassembly", wxCheckBox);
	bSyncDissassembly = pCheckbox->GetValue();

	wxConfig::Get(false)->Write(wxT("debugger/memdump/search/sync_dissassembly"), bSyncDissassembly);
	int nValue;

	pChoice = XRCCTRL(*this, "m_ChoiceSearchType", wxChoice);
	nValue = pChoice->GetSelection();
	wxConfig::Get(false)->Write(wxT("debugger/memdump/search/type"), nValue);

	wxString sValue;

	pTextCtrl = XRCCTRL(*this, "m_TextSearch", wxTextCtrl);
	sValue = pTextCtrl->GetValue();

	wxConfig::Get(false)->Write(wxT("debugger/memdump/search/text"), sValue);

#if (wxVERSION_NUMBER >= 2900)
	// 2.9
	const wxCharBuffer tmp_buf = wxConvCurrent->cWX2MB(sValue.wc_str());
#else
	// 2.8
	const wxWX2MBbuf tmp_buf = wxConvCurrent->cWX2MB(sValue);
#endif
	const char *tmp_str = (const char*)tmp_buf;

	if (nValue == 0)
	{
		int SearchStringCount;
		char *pSearchString;
		unsigned char *pSearchStringMask;

		HexSearch_Evaluate(tmp_str, &SearchStringCount, &pSearchString, &pSearchStringMask);

		m_pSearchData->bCaseInsensitive = FALSE;
		m_pSearchData->bSyncDissassembly = FALSE;
		m_pSearchData->pSearchString = (unsigned char *)pSearchString;
		m_pSearchData->pSearchStringMask = (unsigned char *)pSearchStringMask;
		m_pSearchData->NumBytes = SearchStringCount;
		//m_pSearchData->FoundAddress = -1;
	}
	else
	{
		strcpy(SearchString, tmp_str);
		memset(SearchStringMask, 0x0ff, strlen(tmp_str));
		m_pSearchData->bSyncDissassembly = bSyncDissassembly;
		m_pSearchData->bCaseInsensitive = bCaseSensitiveSearch ? FALSE : TRUE;
		m_pSearchData->pSearchString = (unsigned char *)SearchString;
		m_pSearchData->pSearchStringMask = (unsigned char *)SearchStringMask;
		m_pSearchData->NumBytes = strlen(tmp_str);
		//m_pSearchData->FoundAddress = -1;
	}

	return true;
}


MemdumpSearchDialog::MemdumpSearchDialog(wxWindow *pParent, SEARCH_DATA *pSearchData) : m_pSearchData(pSearchData)
{
	// load the resource
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_DIALOG_SEARCH"));
}



void MemDumpWindow::DoFind()
{
	SearchData.FoundAddress = Memdump_GetCursorAddress(pMemdumpWindow);

	MemdumpSearchDialog dialog(this, &SearchData);
	if (dialog.ShowModal() == wxID_OK)
	{

		PerformSearch();


	}
}


void MemDumpWindow::OnSearchForData(wxCommandEvent & WXUNUSED(event))
{
	DoFind();
}

void MemDumpWindow::OnLoadData(wxCommandEvent & WXUNUSED(event))
{
	wxString sFilename;
	wxString sTitleSuffix;

	if (m_FileType.Open(this, sFilename, sTitleSuffix))
	{
		unsigned char *pFileData = NULL;
		unsigned long FileLength = 0;

		/* try to load it */
		wxGetApp().LoadLocalFile(sFilename, &pFileData, &FileLength);
		if (pFileData != NULL)
		{
			FILE_HEADER FileHeader;

			GetHeaderDataFromBuffer(pFileData, FileLength, &FileHeader);

			// show this if SHIFT is pressed?
			LoadBinaryDialog dialog(this, &FileHeader);
			if (dialog.ShowModal() == wxID_OK)
			{
				if (dialog.m_bSetBreakpoint)
				{
					BREAKPOINT_DESCRIPTION Description;
					BreakpointDescription_Init(&Description);
					Description.Type = BREAKPOINT_TYPE_PC;
					Description.Address = dialog.m_BreakpointAddress;

					if (!Breakpoints_IsAVisibleBreakpoint(&Description))
					{
						Breakpoints_AddBreakpoint(&Description);
					}
				}

				unsigned long Length;
				FileHeader.MemoryStart = FileHeader.HeaderStartAddress;
				Length = min(FileHeader.HeaderLength, FileLength);
				FileHeader.MemoryEnd = FileHeader.MemoryStart + Length;

				LoadBufferToRam(pMemdumpWindow->pRange, (const char *)pFileData, &FileHeader);
			}

			free(pFileData);
		}
	}
}

void MemDumpWindow::OnFillMemory(wxCommandEvent & WXUNUSED(event))
{
	FillMemoryDialog dialog(this);
	if (dialog.ShowModal() == wxID_OK)
	{
		DebuggerFillMemory(pMemdumpWindow->pRange, dialog.m_nStart, dialog.m_nEnd, dialog.pFillString, dialog.FillStringCount);
		this->Refresh();
	}
}


void MemDumpWindow::OnSaveData(wxCommandEvent & WXUNUSED(event))
{
	FILE_HEADER FileHeader;
	// setup defaults
	FileHeader.bHasHeader = true;
	FileHeader.HeaderStartAddress = 0x0000;
	FileHeader.HeaderLength = 0x0000;
	FileHeader.HeaderExecutionAddress = 0x0000;
	FileHeader.MemoryStart = 0x0000;
	FileHeader.MemoryEnd = 0x00000;
	FileHeader.HeaderFileType = 2;

	// show this if SHIFT is pressed?
	SaveBinaryDialog dialog(this, &FileHeader);
	if (dialog.ShowModal() == wxID_OK)
	{
		wxString sFilename;
		wxString sTitleSuffix;

		if (m_FileType.OpenForWrite(this, sFilename, sTitleSuffix))
		{
			unsigned long Length = SaveRamToBufferGetLength(&FileHeader);
			unsigned char *pBuffer = (unsigned char *)malloc(Length);
			if (pBuffer != NULL)
			{
				SaveRamToBuffer(pMemdumpWindow->pRange, pBuffer, &FileHeader);

				if (wxGetApp().SaveLocalFile(sFilename, pBuffer, Length))
				{
					wxString sMessage(wxT("Data saved to file "));
					sMessage += sFilename;

					wxMessageBox(sMessage, wxGetApp().GetAppName(), wxICON_INFORMATION | wxOK);
				}
				else
				{
					wxString sMessage(wxT("Failed to save data to file "));
					sMessage += sFilename;
					wxMessageBox(sMessage, wxGetApp().GetAppName(), wxICON_INFORMATION | wxOK);
				}

				free(pBuffer);
			}
		}
	}
}

void MemDumpWindow::OnGotoAddress(wxCommandEvent & WXUNUSED(event))
{
	int Address;
	if (GetGotoAddress(&Address))
	{
		MemDump_SetAddress(pMemdumpWindow, Address);
		this->Refresh();
	}

}

void MemDumpWindow::OnGotoLabel(wxCommandEvent & WXUNUSED(event))
{
	int Address;
	if (GetGoToLabel(&Address))
	{
		MemDump_SetAddress(pMemdumpWindow, Address);
		this->Refresh();
	}

}


void MemDumpWindow::UpdateDissassembly()
{
	int Start = Memdump_GetStartAddress(pMemdumpWindow);
	int End = Memdump_GetEndAddress(pMemdumpWindow);
	for (int i = 0; i < DebuggerWindow::GetDissassemblyWindowCount(); i++)
	{
		DissassembleWindow *pWindow = DebuggerWindow::GetDissassemblyWindowByIndex(i);
		pWindow->RefreshIfInRange(Start, End);
	}


}

void MemDumpWindow::OnChar(wxKeyEvent &event)
{
#if wxUSE_UNICODE==1
	wxChar ch = event.GetUnicodeKey();
#else
	wxChar ch = (wxChar)event.GetKeyCode();
#endif
	if (event.GetModifiers() == 0)
	{
		if (ch < 127)
		{
			// no control, no shift, no alt

			char chCharCode = ch & 0x0ff;
			Memdump_UpdateEdit(pMemdumpWindow, chCharCode);
			this->Refresh();

			// refresh dissassembly if viewing within this range
			UpdateDissassembly();
		}

		return;
	}

	event.Skip();
}

void MemDumpWindow::OnSendCursorAddressToDissassembly(wxCommandEvent & WXUNUSED(event))
{
	int nAddr = Memdump_GetCursorAddress(pMemdumpWindow);
	SendDataToDissassembly(nAddr);
}


void MemDumpWindow::OnSendCursorAddressToRegister(wxCommandEvent & WXUNUSED(event))
{
	int nAddr = Memdump_GetCursorAddress(pMemdumpWindow);
	SendDataToRegister(nAddr);
}

void MemDumpWindow::OnSendCursorAddressToStack(wxCommandEvent & WXUNUSED(event))
{
	int nAddr = Memdump_GetCursorAddress(pMemdumpWindow);
	SendDataToStack(nAddr);
}

void MemDumpWindow::DoSelect(int x, int y)
{
	int CharX, CharY;
	PixelToChar(x, y, &CharX, &CharY);
	MemDump_SelectByCharXY(pMemdumpWindow, CharX, CharY);
}

void MemDumpWindow::DoTab(bool bForwards)
{
	Memdump_ToggleLocation(pMemdumpWindow, bForwards ? TRUE : FALSE);
}

void MemDumpWindow::DoCursorUp()
{
	MemDump_CursorUp(pMemdumpWindow);
}

void MemDumpWindow::DoCursorDown()
{
	MemDump_CursorDown(pMemdumpWindow);

}

void MemDumpWindow::DoCursorLeft()
{
	MemDump_CursorLeft(pMemdumpWindow);
}

void MemDumpWindow::DoCursorRight()
{
	MemDump_CursorRight(pMemdumpWindow);

}
void MemDumpWindow::DoPageUp()
{
	MemDump_PageUp(pMemdumpWindow);

}

void MemDumpWindow::DoPageDown()
{
	MemDump_PageDown(pMemdumpWindow);

}

wxObject *MemDumpWindowResourceHandler::DoCreateResource()
{

	XRC_MAKE_INSTANCE(control, MemDumpWindow)

		control->Create(m_parentAsWindow,
		GetID(),
		GetText(wxT("label")),
		GetPosition(), GetSize(),
		GetStyle(),
		wxDefaultValidator,
		GetName());

	SetupWindow(control);
	return control;
}


bool MemDumpWindowResourceHandler::CanHandle(wxXmlNode *node)
{
	return IsOfClass(node, wxT("MemDumpWindow"));
}

MemDumpWindowResourceHandler::MemDumpWindowResourceHandler() : wxXmlResourceHandler()
{


}

MemDumpWindowResourceHandler::~MemDumpWindowResourceHandler()
{

}

bool MemDumpWindow::Create(wxWindow* parent, wxWindowID id, const wxString &label, const wxPoint& pos, const wxSize& size, long style, const wxValidator &validator, const wxString& name)
{
	bool bCreated = OurTextWindow::Create(parent, id, label, pos, size, style, validator, name);
	if (bCreated)
	{
		int MemdumpID = -1;
		if (id == XRCID("m_customControl3"))
		{
			MemdumpID = 0;
		}
		else if (id == XRCID("m_customControl31"))
		{
			MemdumpID = 1;
		}
		else if (id == XRCID("m_customControl32"))
		{
			MemdumpID = 2;
		}
		else if (id == XRCID("m_customControl33"))
		{
			MemdumpID = 3;
		}
		MemDump_SetID(pMemdumpWindow, MemdumpID);

		if (DebuggerDisplay.MemoryDumpSettingValid[MemDump_GetID(pMemdumpWindow)])
		{
			MemDump_FromSettings(pMemdumpWindow, &DebuggerDisplay.MemoryDumpSettings[MemDump_GetID(pMemdumpWindow)]);
		}
	}
	return bCreated;
}

MemDumpWindow::MemDumpWindow() : OurTextWindow(), m_FileType(wxT("Open Binary Data"), wxT("Save Binary Data"))
{
	SearchData.NumBytes = 0;

	// binary
	FileFilter BinaryFilter = { wxT("Binary"), wxT("bin") };
	FileFilter BasicFilter = { wxT("Amstrad Basic"), wxT("bas") };
	// zip
	FileFilter ZipFilter = { wxT("ZIP Archive"), wxT("zip") };

	m_FileType.AddWriteFilter(BinaryFilter);
	m_FileType.AddWriteFilter(BasicFilter);

	m_FileType.AddReadFilter(BinaryFilter);
	m_FileType.AddReadFilter(BasicFilter);
	m_FileType.AddReadFilter(ZipFilter);


	m_bHasMargin = false;
	m_sContextMenuName = wxT("MB_MENU_MEMDUMP_MENU_BAR");

	pMemdumpWindow = MemDump_Create();


	Memdump_SetMemoryRange(pMemdumpWindow, (MemoryRange *)CPC_GetDefaultMemoryRange());

	DebuggerWindow::RegisterMemdumpWindow(this);
}

MemDumpWindow::~MemDumpWindow()
{
	DebuggerWindow::DeRegisterMemdumpWindow(this);
	DebuggerDisplay.MemoryDumpSettingValid[MemDump_GetID(pMemdumpWindow)] = true;
	MemDump_ToSettings(pMemdumpWindow, &DebuggerDisplay.MemoryDumpSettings[MemDump_GetID(pMemdumpWindow)]);
	MemDump_Finish(pMemdumpWindow);
}

void MemDumpWindow::DoOnPaint(wxAutoBufferedPaintDC &dc)
{
	wxCoord x = TextOrigin.x;
	wxCoord y = TextOrigin.y;

	pMemdumpWindow->WidthInChars = m_nCharsWidth;
	pMemdumpWindow->HeightInChars = m_nCharsHeight;
	MemDump_RefreshState(pMemdumpWindow);

	DrawCursor(dc, pMemdumpWindow->CursorXAbsolute, pMemdumpWindow->CursorYAbsolute, 1, 1);


	for (int j = 0; j < m_nCharsHeight; j++)
	{
		//        int TextOut_Length;
		//      TCHAR *pMemDumpString;
		char *pMemDumpString;
		pMemDumpString = Memdump_DumpLine(pMemdumpWindow, j);

		//     TextOut_Length = _tcslen(pMemDumpString);

		//        if (TextOut_Length>pData->pMemdumpWindow->WidthInChars)
		//      {
		//        TextOut_Length = pData->pMemdumpWindow->WidthInChars;
		//  }


		wxString sText = wxString((const char *)pMemDumpString, *wxConvCurrent);
		dc.DrawText(sText, x, y);

		y += m_nFontHeight;
	}
}


// void arnguiFrame::OnCPCMemdump(wxCommandEvent &event)
// {
//    MemDumpWindow *pWindow = new MemDumpWindow(this);
//    pWindow->Show();
// }

#include <wx/xrc/xmlres.h>

BEGIN_EVENT_TABLE(DissassembleWindow, OurTextWindow)
EVT_KEY_DOWN(DissassembleWindow::OnKeyDown)
EVT_COMMAND(XRCID("ID_DISSASSEMBLE_TOGGLEBYTES"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnToggleOpcodes)
EVT_COMMAND(XRCID("ID_DISSASSEMBLE_TOGGLEASCII"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnToggleAscii)
EVT_COMMAND(XRCID("ID_DISSASSEMBLE_TOGGLELABELS"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnToggleLabels)
EVT_COMMAND(XRCID("ID_DISSASSEMBLE_TOGGLETIMINGS"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnToggleTimings)
EVT_COMMAND(XRCID("ID_DISSASSEMBLE_TOGGLEBREAKOPCODE"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnToggleBreakOpcode)
EVT_COMMAND(XRCID("Follow_PC"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnFollowPC)
EVT_COMMAND(XRCID("ResetCycleCounter"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnResetCycleCounter)
EVT_COMMAND(XRCID("Break"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnBreak)
EVT_COMMAND(XRCID("Continue"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnContinue)
EVT_COMMAND(XRCID("StepOver"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnStepOver)
EVT_COMMAND(XRCID("StepInto"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnStepInto)
EVT_COMMAND(XRCID("Set_PC_To_Address"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnSetPCToAddress)
EVT_COMMAND(XRCID("ReturnToPC"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnReturnToPC)
EVT_COMMAND(XRCID("ID_DISSASSEMBLE_RUNTOADDRESS"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnRunToAddress)
EVT_COMMAND(XRCID("ToggleBreakpoint"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnToggleBreakpoint)
EVT_COMMAND(XRCID("SendToMemoryDump"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnSendCursorAddressToMemoryDump)
EVT_COMMAND(XRCID("SendToRegister"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnSendCursorAddressToRegister)
EVT_COMMAND(XRCID("SendToStack"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnSendCursorAddressToStack)

EVT_COMMAND(XRCID("AddLabelAtCursor"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::AddLabelAtCursor)

EVT_COMMAND(XRCID("GoToAddress"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnGotoAddress)
EVT_COMMAND(XRCID("GoToLabel"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnGotoLabel)
EVT_COMMAND(XRCID("ConvertAddressTool"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnConvertAddressTool)

EVT_COMMAND(XRCID("TypeZ80"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnSetType)
EVT_COMMAND(XRCID("TypePlusDMA0"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnSetType)
EVT_COMMAND(XRCID("TypePlusDMA1"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnSetType)
EVT_COMMAND(XRCID("TypePlusDMA2"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnSetType)

EVT_COMMAND(XRCID("m_DissassembleToClipboard"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnDissassembleToClipboard)
EVT_COMMAND(XRCID("m_DissassembleToFile"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnDissassembleToFile)
EVT_COMMAND(XRCID("m_SetMemoryRange"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnSetMemoryRange)

EVT_COMMAND(XRCID("LabelManager"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnLabelManager)

EVT_COMMAND(XRCID("SevenBitASCII"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnSevenBitASCII)

EVT_COMMAND(XRCID("ID_MEMDUMP_LOADDATA"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnLoadData)
EVT_COMMAND(XRCID("ID_MEMDUMP_SAVEDATA"), wxEVT_COMMAND_MENU_SELECTED, DissassembleWindow::OnSaveData)


END_EVENT_TABLE()


void DissassembleWindow::UpdateContextMenuState(wxMenuBar *pMenuBar)
{
	arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("TypeZ80"), Dissassemble_GetView(pDissassembleWindow) == DISSASSEMBLE_VIEW_Z80_INSTRUCTIONS ? true : false);
	arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("TypePlusDMA0"), Dissassemble_GetView(pDissassembleWindow) == DISSASSEMBLE_VIEW_DMA_CHANNEL_0_INSTRUCTIONS ? true : false);
	arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("TypePlusDMA1"), Dissassemble_GetView(pDissassembleWindow) == DISSASSEMBLE_VIEW_DMA_CHANNEL_1_INSTRUCTIONS ? true : false);
	arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("TypePlusDMA2"), Dissassemble_GetView(pDissassembleWindow) == DISSASSEMBLE_VIEW_DMA_CHANNEL_2_INSTRUCTIONS ? true : false);

	arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ID_DISSASSEMBLE_TOGGLEBYTES"), Dissassemble_ShowOpcodes(pDissassembleWindow) ? true : false);
	arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ID_DISSASSEMBLE_TOGGLEASCII"), Dissassemble_ShowAscii(pDissassembleWindow) ? true : false);
	arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ID_DISSASSEMBLE_TOGGLELABELS"), Dissassemble_ShowLabels(pDissassembleWindow) ? true : false);
	arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ID_DISSASSEMBLE_TOGGLETIMINGS"), Dissassemble_ShowInstructionTimings(pDissassembleWindow) ? true : false);
	arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ID_DISSASSEMBLE_TOGGLEBREAKOPCODE"), CPU_GetDebugOpcodeEnabled() ? true : false);
	arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("Follow_PC"), Dissassemble_FollowPC(pDissassembleWindow) ? true : false);
	arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("SevenBitASCII"), Dissassemble_GetSevenBitASCII(pDissassembleWindow) ? true : false);
}


void DissassembleWindow::OnResetCycleCounter(wxCommandEvent & WXUNUSED(event))
{
	Computer_ResetCycleCounter();
}

void DissassembleWindow::OnLoadData(wxCommandEvent & WXUNUSED(event))
{
	wxString sFilename;
	wxString sTitleSuffix;

	if (m_FileType.Open(this, sFilename, sTitleSuffix))
	{
		unsigned char *pFileData = NULL;
		unsigned long FileLength = 0;

		/* try to load it */
		wxGetApp().LoadLocalFile(sFilename, &pFileData, &FileLength);
		if (pFileData != NULL)
		{
			FILE_HEADER FileHeader;

			GetHeaderDataFromBuffer(pFileData, FileLength, &FileHeader);

			LoadBinaryDialog dialog(this, &FileHeader);
			if (dialog.ShowModal() == wxID_OK)
			{
				if (dialog.m_bSetBreakpoint)
				{
					BREAKPOINT_DESCRIPTION Description;
					BreakpointDescription_Init(&Description);
					Description.Type = BREAKPOINT_TYPE_PC;
					Description.Address = dialog.m_BreakpointAddress;

					if (!Breakpoints_IsAVisibleBreakpoint(&Description))
					{
						Breakpoints_AddBreakpoint(&Description);
					}
				}
				unsigned long Length;
				FileHeader.MemoryStart = FileHeader.HeaderStartAddress;
				Length = min(FileHeader.HeaderLength, FileLength);
				FileHeader.MemoryEnd = FileHeader.MemoryStart + Length;

				LoadBufferToRam(pDissassembleWindow->pRange, (const char *)pFileData, &FileHeader);
			}

			free(pFileData);
		}
	}
}

void DissassembleWindow::OnSaveData(wxCommandEvent & WXUNUSED(event))
{
	FILE_HEADER FileHeader;
	// setup defaults
	FileHeader.bHasHeader = true;
	FileHeader.HeaderStartAddress = 0x0000;
	FileHeader.HeaderLength = 0x0000;
	FileHeader.HeaderExecutionAddress = 0x0000;
	FileHeader.MemoryStart = 0x0000;
	FileHeader.MemoryEnd = 0x00000;
	FileHeader.HeaderFileType = 2;

	// show this if SHIFT is pressed?
	SaveBinaryDialog dialog(this, &FileHeader);
	if (dialog.ShowModal() == wxID_OK)
	{
		wxString sFilename;
		wxString sTitleSuffix;

		if (m_FileType.OpenForWrite(this, sFilename, sTitleSuffix))
		{
			unsigned long Length = SaveRamToBufferGetLength(&FileHeader);
			unsigned char *pBuffer = (unsigned char *)malloc(Length);
			if (pBuffer != NULL)
			{
				SaveRamToBuffer(pDissassembleWindow->pRange, pBuffer, &FileHeader);

				if (wxGetApp().SaveLocalFile(sFilename, pBuffer, Length))
				{
					wxString sMessage(wxT("Data saved to file "));
					sMessage += sFilename;

					wxMessageBox(sMessage, wxGetApp().GetAppName(), wxICON_INFORMATION | wxOK);
				}
				else
				{
					wxString sMessage(wxT("Failed to save data to file "));
					sMessage += sFilename;
					wxMessageBox(sMessage, wxGetApp().GetAppName(), wxICON_INFORMATION | wxOK);
				}
				free(pBuffer);
			}
		}
	}


}

void DissassembleWindow::RefreshIfInRange(int Start, int End)
{
	if
		(
		(Dissassemble_GetStartAddress(pDissassembleWindow) >= Start) &&
		(Dissassemble_GetEndAddress(pDissassembleWindow) <= End)
		)
	{

		this->Refresh();
	}
}


void DissassembleWindow::OnSevenBitASCII(wxCommandEvent & WXUNUSED(event))
{
	Dissassemble_SetSevenBitASCII(pDissassembleWindow, !Dissassemble_GetSevenBitASCII(pDissassembleWindow));
	this->Refresh();
}

void DissassembleWindow::OnLabelManager(wxCommandEvent & WXUNUSED(event))
{
	LabelManagerDialog::CreateInstance(this);
}

void DissassembleWindow::OnSetMemoryRange(wxCommandEvent & WXUNUSED(event))
{
	wxArrayString sRanges;
	for (int i = 0; i < CPC_GetRegisteredMemoryRangeCount(); i++)
	{
		wxString sDrive;
		const MemoryRange *pRange = CPC_GetRegisteredMemoryRange(i);
		sDrive = wxString::FromAscii(pRange->sName);
		sRanges.Add(sDrive);
	}

	wxString sMessage;
	wxSingleChoiceDialog dialog(this, wxT("Memory range to use:"), wxT("Choose memory range"), sRanges);
	dialog.SetSelection(0);
	if (dialog.ShowModal() == wxID_OK)
	{
		int nRange = dialog.GetSelection();
		const MemoryRange *pRange = CPC_GetRegisteredMemoryRange(nRange);
		Dissassemble_SetMemoryRange(pDissassembleWindow, (MemoryRange *)pRange);
	}

	Refresh();

}

void DissassembleWindow::OnConvertAddressTool(wxCommandEvent & WXUNUSED(event))
{
	// which parent?
	// should be same parent as the main debugger dialog.
	ConvertAddressToolDialog::CreateInstance(NULL);

}

void DissassembleWindow::AddLabelAtCursor(wxCommandEvent & WXUNUSED(event))
{
	// be part of the label manager?
	int Addr = Dissassemble_GetCursorAddress(pDissassembleWindow);

	wxTextEntryDialog dialog(this, wxT("Label name"), wxT("Add Label at Cursor"));
	if (dialog.ShowModal() == wxID_OK)
	{
#if (wxVERSION_NUMBER >= 2900)
		// 2.9
		const wxCharBuffer tmp_buf = wxConvCurrent->cWX2MB(dialog.GetValue().wc_str());
#else
		// 2.8
		const wxWX2MBbuf tmp_buf = wxConvCurrent->cWX2MB(dialog.GetValue());
#endif
		const char *tmp_str = (const char*)tmp_buf;

		// find user label set
		LABELSET *pLabelSet = labelset_find_by_name("user");
		if (pLabelSet == NULL)
		{
			pLabelSet = labelset_create("user");
		}
		if (pLabelSet)
		{
			// TODO: do we have a label at that address?

			// add it to that
			labelset_add_label(pLabelSet, tmp_str, Addr);
		}

		this->Refresh();

	}
}


void DissassembleWindow::OnGotoLabel(wxCommandEvent & WXUNUSED(event))
{
	int Address;
	if (GetGoToLabel(&Address))
	{
		Dissassemble_SetAddress(pDissassembleWindow, Address);
		this->Refresh();
	}
}


void DissassembleWindow::OnGotoAddress(wxCommandEvent & WXUNUSED(event))
{
	int Address;
	if (GetGotoAddress(&Address))
	{
		Dissassemble_SetAddress(pDissassembleWindow, Address);
		this->Refresh();
	}

}

void DissassembleWindow::DoSelect(int x, int y)
{
	int CharX, CharY;
	PixelToChar(x, y, &CharX, &CharY);
	Dissassemble_SelectByCharXY(pDissassembleWindow, CharX, CharY);
}

void DissassembleWindow::DoClickMargin(int x, int y)
{
	int CharX, CharY;
	PixelToChar(x, y, &CharX, &CharY);
	int	nAddress = Dissassemble_GetLineAddress(pDissassembleWindow, CharY);
	ToggleABreakpoint(nAddress);
}


int DissassembleWindow::MenuIdToTypeId(int nId)
{
	if (nId == XRCID("TypeZ80"))
	{
		return DISSASSEMBLE_VIEW_Z80_INSTRUCTIONS;
	}
	else if (nId == XRCID("TypePlusDMA0"))
	{
		return DISSASSEMBLE_VIEW_DMA_CHANNEL_0_INSTRUCTIONS;
	}
	else if (nId == XRCID("TypePlusDMA1"))
	{
		return DISSASSEMBLE_VIEW_DMA_CHANNEL_1_INSTRUCTIONS;
	}
	else if (nId == XRCID("TypePlusDMA2"))
	{
		return DISSASSEMBLE_VIEW_DMA_CHANNEL_2_INSTRUCTIONS;
	}
	return DISSASSEMBLE_VIEW_Z80_INSTRUCTIONS;
}

void DissassembleWindow::OnSetType(wxCommandEvent &event)
{
	int nTypeId = MenuIdToTypeId(event.GetId());
	Dissassemble_SetView(pDissassembleWindow, nTypeId);
	this->Refresh();
}


void DissassembleWindow::OnSendCursorAddressToMemoryDump(wxCommandEvent & WXUNUSED(event))
{
	int nAddr = Dissassemble_GetCursorAddress(pDissassembleWindow);
	SendDataToMemoryDump(nAddr);
}



void DissassembleWindow::OnSendCursorAddressToRegister(wxCommandEvent & WXUNUSED(event))
{
	int nAddr = Dissassemble_GetCursorAddress(pDissassembleWindow);
	SendDataToRegister(nAddr);
}

void DissassembleWindow::OnSendCursorAddressToStack(wxCommandEvent & WXUNUSED(event))
{
	int nAddr = Dissassemble_GetCursorAddress(pDissassembleWindow);
	SendDataToStack(nAddr);

}


void DissassembleWindow::ToggleABreakpoint(int Addr)
{
	if (pDissassembleWindow->ViewType != DISSASSEMBLE_VIEW_Z80_INSTRUCTIONS)
		return;

	BREAKPOINT_DESCRIPTION Description;
	BreakpointDescription_Init(&Description);
	Description.Type = BREAKPOINT_TYPE_PC;
	Description.Address = Addr;

	/* breakpoint exists? */
	if (Breakpoints_IsAVisibleBreakpoint(&Description))
	{
		/* yes, remove it */
		Breakpoints_RemoveBreakpointByDescription(&Description);
	}
	else
	{
		/* no, add one */
		Breakpoints_AddBreakpoint(&Description);
	}

	this->Refresh();
}

void DissassembleWindow::ToggleACursorBreakpoint()
{
	if (pDissassembleWindow->ViewType == DISSASSEMBLE_VIEW_Z80_INSTRUCTIONS)
	{

		int Addr;

		Addr = Dissassemble_GetCursorAddress(pDissassembleWindow);
		ToggleABreakpoint(Addr);
	}
}

void DissassembleWindow::OnToggleBreakpoint(wxCommandEvent & WXUNUSED(event))
{
	ToggleACursorBreakpoint();
}

void DissassembleWindow::RunToAddress()
{
	if (pDissassembleWindow->ViewType == DISSASSEMBLE_VIEW_Z80_INSTRUCTIONS)
	{

		int NewAddr = Dissassemble_GetCursorAddress(pDissassembleWindow);

		// restore sound
		SDLCommon::PauseSound(0);

		Debug_SetRunTo(NewAddr);

		this->Refresh();

	}
}

void DissassembleWindow::OnRunToAddress(wxCommandEvent & WXUNUSED(event))
{
	if (pDissassembleWindow->ViewType == DISSASSEMBLE_VIEW_Z80_INSTRUCTIONS)
	{
		RunToAddress();
	}
}

void DissassembleWindow::OnBreak(wxCommandEvent & WXUNUSED(event))
{
	Debug_TriggerBreak();
}

void DissassembleWindow::OnContinue(wxCommandEvent & WXUNUSED(event))
{
	// restore sound
	SDLCommon::PauseSound(0);

	Debug_Continue();
}

void DissassembleWindow::DoStepOverInstruction()
{
	if (pDissassembleWindow->ViewType == DISSASSEMBLE_VIEW_Z80_INSTRUCTIONS)
	{
		StepOverInstruction(pDissassembleWindow->pRange);
		

		// restore sound
		SDLCommon::PauseSound(0);
	}
}

void DissassembleWindow::OnStepOver(wxCommandEvent & WXUNUSED(event))
{
	if (pDissassembleWindow->ViewType == DISSASSEMBLE_VIEW_Z80_INSTRUCTIONS)
	{
		DoStepOverInstruction();
	}
}

void DissassembleWindow::DoStepIntoInstruction()
{
	if (pDissassembleWindow->ViewType == DISSASSEMBLE_VIEW_Z80_INSTRUCTIONS)
	{
		StepIntoInstruction(pDissassembleWindow->pRange);  //&Z80);

		// restore sound
		SDLCommon::PauseSound(0);
	}
}

void DissassembleWindow::OnStepInto(wxCommandEvent & WXUNUSED(event))
{
	if (pDissassembleWindow->ViewType == DISSASSEMBLE_VIEW_Z80_INSTRUCTIONS)
	{
		DoStepIntoInstruction();
	}
}


void DissassembleWindow::OnToggleBreakOpcode(wxCommandEvent & WXUNUSED(event))
{
	if (pDissassembleWindow->ViewType == DISSASSEMBLE_VIEW_Z80_INSTRUCTIONS)
	{
		CPU_SetDebugOpcodeEnabled(!CPU_GetDebugOpcodeEnabled());
		wxConfig::Get(false)->Write(wxT("debug/break_opcode_enable"), CPU_GetDebugOpcodeEnabled());

		this->Refresh();
	}
}


void DissassembleWindow::OnToggleOpcodes(wxCommandEvent & WXUNUSED(event))
{
	Dissassemble_SetShowOpcodes(pDissassembleWindow, !Dissassemble_ShowOpcodes(pDissassembleWindow));
	this->Refresh();
}

void DissassembleWindow::OnToggleAscii(wxCommandEvent & WXUNUSED(event))
{
	Dissassemble_SetShowAscii(pDissassembleWindow, !Dissassemble_ShowAscii(pDissassembleWindow));
	this->Refresh();
}

void DissassembleWindow::OnToggleLabels(wxCommandEvent & WXUNUSED(event))
{
	Dissassemble_SetShowLabels(pDissassembleWindow, !Dissassemble_ShowLabels(pDissassembleWindow));
	this->Refresh();
}

void DissassembleWindow::OnToggleTimings(wxCommandEvent & WXUNUSED(event))
{
	Dissassemble_SetShowInstructionTimings(pDissassembleWindow, !Dissassemble_ShowInstructionTimings(pDissassembleWindow));
	this->Refresh();
}

void DissassembleWindow::OnFollowPC(wxCommandEvent & WXUNUSED(event))
{
	Dissassemble_SetFollowPC(pDissassembleWindow, !Dissassemble_FollowPC(pDissassembleWindow));
	this->Refresh();
}

void DissassembleWindow::SetPCToAddress()
{
	int NewAddr = Dissassemble_GetCursorAddress(pDissassembleWindow);

	Dissassemble_SetPC(pDissassembleWindow, NewAddr);

	this->Refresh();

}

void DissassembleWindow::OnSetPCToAddress(wxCommandEvent & WXUNUSED(event))
{
	SetPCToAddress();
}

void DissassembleWindow::OnReturnToPC(wxCommandEvent & WXUNUSED(event))
{
	Dissassemble_GetPC(pDissassembleWindow);

	int PCAddress = CPU_GetReg(CPU_PC);
	Dissassemble_SetAddress(pDissassembleWindow, PCAddress);
	this->Refresh();
}


void DissassembleWindow::OnKeyDown(wxKeyEvent &event)
{
	switch (event.GetKeyCode())
	{
	default:
		break;
#if !defined(__WXMAC__)
	case WXK_F6:
	{
		if (event.GetModifiers() == 0)
		{
			RunToAddress();

			event.Skip(false);

			return;
		}
	}
	break;


	case WXK_F7:
	{
		if (event.GetModifiers() == 0)
		{
			SetPCToAddress();

			event.Skip(false);

			return;
		}

	}
	break;



	case WXK_F8:
	{
		if (event.GetModifiers() == 0)
		{
			Debug_TriggerBreak();
			return;
		}
	}
	break;


	case WXK_F5:
	{
		if (event.GetModifiers() == 0)
		{
			// restore sound
			SDLCommon::PauseSound(0);

			Debug_Continue();

			event.Skip(false);

			return;
		}
	}
	break;


	case WXK_F12:
	{
		if (event.GetModifiers() == 0)
		{
			DoStepOverInstruction();

			event.Skip(false);

			return;
		}
	}
	break;


	case WXK_F11:
	{
		if (event.GetModifiers() == 0)
		{
			DoStepIntoInstruction();

			event.Skip(false);
			return;
		}
	}
	break;


	case WXK_F9:
	{
		if (event.GetModifiers() == 0)
		{
			ToggleACursorBreakpoint();

			event.Skip(false);

			return;
		}
	}
	break;
#endif
	}

	OurTextWindow::OnKeyDown(event);
}

void DissassembleWindow::DoTab(bool WXUNUSED(bForwards))
{

}

void DissassembleWindow::DoCursorUp()
{
	Dissassemble_CursorUp(pDissassembleWindow);
}

void DissassembleWindow::DoCursorDown()
{
	Dissassemble_CursorDown(pDissassembleWindow);

}

void DissassembleWindow::DoCursorLeft()
{
}

void DissassembleWindow::DoCursorRight()
{
}

void DissassembleWindow::DoPageUp()
{
	Dissassemble_PageUp(pDissassembleWindow);

}

void DissassembleWindow::DoPageDown()
{
	Dissassemble_PageDown(pDissassembleWindow);

}

bool DissassembleWindow::Create(wxWindow* parent, wxWindowID id, const wxString &label, const wxPoint& pos, const wxSize& size, long style, const wxValidator &validator, const wxString& name)
{
	bool bCreated = OurTextWindow::Create(parent, id, label, pos, size, style, validator, name);
	if (bCreated)
	{
		int DissassembleID = -1;
		if (id == XRCID("m_customControl2"))
		{
			DissassembleID = 0;
		}
		else if (id == XRCID("m_customControl21"))
		{
			DissassembleID = 1;
		}
		else if (id == XRCID("m_customControl22"))
		{
			DissassembleID = 2;
		}
		else if (id == XRCID("m_customControl23"))
		{
			DissassembleID = 3;
		}
		Dissassemble_SetID(pDissassembleWindow, DissassembleID);

		if (DebuggerDisplay.DissassembleSettingValid[Dissassemble_GetID(pDissassembleWindow)])
		{
			Dissassemble_FromSettings(pDissassembleWindow, &DebuggerDisplay.DissassembleSettings[Dissassemble_GetID(pDissassembleWindow)]);
		}
	}
	return bCreated;
}


wxObject *DissassembleWindowResourceHandler::DoCreateResource()
{

	XRC_MAKE_INSTANCE(control, DissassembleWindow)

		control->Create(m_parentAsWindow,
		GetID(),
		GetText(wxT("label")),
		GetPosition(), GetSize(),
		GetStyle(),
		wxDefaultValidator,
		GetName());

	//   control->SetValue(GetBool( wxT("checked")));
	SetupWindow(control);
	return control;
}


bool DissassembleWindowResourceHandler::CanHandle(wxXmlNode *node)
{
	return IsOfClass(node, wxT("DissassembleWindow"));
}

DissassembleWindowResourceHandler::DissassembleWindowResourceHandler() : wxXmlResourceHandler()
{


}

DissassembleWindowResourceHandler::~DissassembleWindowResourceHandler()
{

}



DissassembleWindow::DissassembleWindow() : OurTextWindow(), m_FileType(wxT("Open Binary Data"), wxT("Save Binary Data"))
{

	// binary
	FileFilter BinaryFilter = { wxT("Binary"), wxT("bin") };
	FileFilter BasicFilter = { wxT("Amstrad Basic"), wxT("bas") };
	// zip
	FileFilter ZipFilter = { wxT("ZIP Archive"), wxT("zip") };

	m_FileType.AddWriteFilter(BinaryFilter);
	m_FileType.AddWriteFilter(BasicFilter);

	m_FileType.AddReadFilter(BinaryFilter);
	m_FileType.AddReadFilter(BasicFilter);
	m_FileType.AddReadFilter(ZipFilter);



	m_bHasMargin = true;
	m_sContextMenuName = wxT("MB_MENU_DISSASSEMBLE_MENU_BAR");


	pDissassembleWindow = Dissassemble_Create();
	Dissassemble_SetMemoryRange(pDissassembleWindow, (MemoryRange *)CPC_GetDefaultMemoryRange());
	DebuggerWindow::RegisterDissassemblyWindow(this);
}

DissassembleWindow::~DissassembleWindow()
{
	DebuggerDisplay.DissassembleSettingValid[Dissassemble_GetID(pDissassembleWindow)] = true;
	Dissassemble_ToSettings(pDissassembleWindow, &DebuggerDisplay.DissassembleSettings[Dissassemble_GetID(pDissassembleWindow)]);
	Dissassemble_Finish(pDissassembleWindow);
	DebuggerWindow::DeRegisterDissassemblyWindow(this);
}


void DissassembleWindow::DissassembleRange(wxString &sString, int nStart, int nEnd, bool bBytes, bool bASCII, bool bLabels)
{
	int nAddr = Dissassemble_GetAddress(pDissassembleWindow);

	// ensure values are in order
	if (nStart > nEnd)
	{
		int nTemp = nStart;
		nStart = nEnd;
		nEnd = nTemp;
	}
	// remember previous
	Dissassemble_SetAddress(pDissassembleWindow, nStart);
	BOOL	bShowOpcodes = Dissassemble_ShowOpcodes(pDissassembleWindow);
	BOOL	bShowASCII = Dissassemble_ShowAscii(pDissassembleWindow);
	BOOL	bShowLabels = Dissassemble_ShowLabels(pDissassembleWindow);
	BOOL	bShowTimings = Dissassemble_ShowInstructionTimings(pDissassembleWindow);

	// set new
	wxString sInfo;
	sInfo.sprintf(wxT("; Start &%04x\n; End &%04x\n; Length: &%04x\n"), nStart, nEnd, (nEnd - nStart));
	sString += sInfo;
	Dissassemble_SetShowAscii(pDissassembleWindow, (bBytes ? TRUE : FALSE));
	Dissassemble_SetShowOpcodes(pDissassembleWindow, (bASCII ? TRUE : FALSE));
	Dissassemble_SetShowLabels(pDissassembleWindow, (bLabels ? TRUE : FALSE));
	Dissassemble_SetShowInstructionTimings(pDissassembleWindow, FALSE);

	// end is inclusive
	while (pDissassembleWindow->CurrentAddr <= nEnd)
	{

		Dissassemble_BeginDissassemble(pDissassembleWindow);

		const char *pDebugString = Dissassemble_DissassembleNextLine(pDissassembleWindow);

		wxString sText = wxString((const char *)pDebugString, *wxConvCurrent);

		sString += sText;
		sString += wxT("\n");

	}

	Dissassemble_SetAddress(pDissassembleWindow, nAddr);
	Dissassemble_SetShowOpcodes(pDissassembleWindow, bShowOpcodes);
	Dissassemble_SetShowAscii(pDissassembleWindow, bShowASCII);
	Dissassemble_SetShowLabels(pDissassembleWindow, bShowLabels);
	Dissassemble_SetShowInstructionTimings(pDissassembleWindow, bShowTimings);
}



BEGIN_EVENT_TABLE(DissassemblyRangeDialog, wxDialog)
END_EVENT_TABLE()


DissassemblyRangeDialog::~DissassemblyRangeDialog()
{
}

bool DissassemblyRangeDialog::TransferDataToWindow()
{
	//    wxChoice *pChoice;
	wxTextCtrl *pTextCtrl;
	wxCheckBox *pCheckBox;

	wxString sValue;

	wxConfig::Get(false)->Read(wxT("debugger/dissassemble_range/start"), &sValue, wxT("#0000"));
	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_MEMORYSTART", wxTextCtrl);
	pTextCtrl->SetValue(sValue);

	wxConfig::Get(false)->Read(wxT("debugger/dissassemble_range/end"), &sValue, wxT("#0000"));
	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_MEMORYEND", wxTextCtrl);
	pTextCtrl->SetValue(sValue);

	bool bValue = false;
	wxConfig::Get(false)->Read(wxT("debugger/dissassemble_range/bytes"), &bValue, false);
	pCheckBox = XRCCTRL(*this, "m_CheckBytes", wxCheckBox);
	pCheckBox->SetValue(bValue);

	wxConfig::Get(false)->Read(wxT("debugger/dissassemble_range/ascii"), &bValue, false);
	pCheckBox = XRCCTRL(*this, "m_CheckASCII", wxCheckBox);
	m_bASCII = pCheckBox->GetValue();

	wxConfig::Get(false)->Read(wxT("debugger/dissassemble_range/labels"), &bValue, false);
	pCheckBox = XRCCTRL(*this, "m_CheckLabels", wxCheckBox);
	m_bLabels = pCheckBox->GetValue();


	return true;
}

bool DissassemblyRangeDialog::TransferDataFromWindow()
{
	//    wxChoice *pChoice;
	wxTextCtrl *pTextCtrl;
	wxCheckBox *pCheckBox;

	wxString sValue;

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_MEMORYSTART", wxTextCtrl);
	sValue = pTextCtrl->GetValue();
	wxConfig::Get(false)->Write(wxT("debugger/dissassemble_range/start"), sValue);

	m_nStart = wxGetApp().ExpressionEvaluate(sValue);

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_MEMORYEND", wxTextCtrl);
	sValue = pTextCtrl->GetValue();
	wxConfig::Get(false)->Write(wxT("debugger/dissassemble_range/end"), sValue);

	m_nEnd = wxGetApp().ExpressionEvaluate(sValue);


	pCheckBox = XRCCTRL(*this, "m_CheckBytes", wxCheckBox);
	m_bBytes = pCheckBox->GetValue();
	wxConfig::Get(false)->Write(wxT("debugger/dissassemble_range/bytes"), m_bBytes);

	pCheckBox = XRCCTRL(*this, "m_CheckASCII", wxCheckBox);
	m_bASCII = pCheckBox->GetValue();
	wxConfig::Get(false)->Write(wxT("debugger/dissassemble_range/ascii"), m_bASCII);

	pCheckBox = XRCCTRL(*this, "m_CheckLabels", wxCheckBox);
	m_bLabels = pCheckBox->GetValue();
	wxConfig::Get(false)->Write(wxT("debugger/dissassemble_range/labels"), m_bLabels);


	return true;

}


DissassemblyRangeDialog::DissassemblyRangeDialog(wxWindow *pParent)
{
	m_nStart = 0;
	m_nEnd = 0;
	m_bBytes = false;
	m_bASCII = false;
	m_bLabels = false;

	// load the resource
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_DIALOG_DISSASSEMBLE"));
}

void DissassembleWindow::OnDissassembleToClipboard(wxCommandEvent & WXUNUSED(event))
{
	DissassemblyRangeDialog dialog(this);
	if (dialog.ShowModal() == wxID_OK)
	{
		// perform dissassembly
		wxString sString;
		DissassembleRange(sString, dialog.m_nStart, dialog.m_nEnd, dialog.m_bBytes, dialog.m_bASCII, dialog.m_bLabels);

		if (wxTheClipboard->Open())
		{
			// Copy to clipboard - can be pasted into scite for example

			// NOTE:
			// This data objects are held by the clipboard,
			// so do not delete them in the app.
			wxTheClipboard->SetData(new wxTextDataObject(sString));
			wxTheClipboard->Close();

			wxMessageBox(wxT("Dissassembly text copied to clipboard"),
				wxGetApp().GetAppName(), wxICON_INFORMATION | wxOK);
		}
		else
		{
			wxMessageBox(wxT("Failed to copy dissassembly text to clipboard"),
				wxGetApp().GetAppName(), wxICON_INFORMATION | wxOK);

		}
	}
}



void DissassembleWindow::OnDissassembleToFile(wxCommandEvent & WXUNUSED(event))
{
	DissassemblyRangeDialog dialog(this);
	if (dialog.ShowModal() == wxID_OK)
	{
		// perform dissassembly
		wxString sString;
		DissassembleRange(sString, dialog.m_nStart, dialog.m_nEnd, dialog.m_bBytes, dialog.m_bASCII, dialog.m_bLabels);

		// suggest a filename if no filename has been used before
		wxFileName sDefaultFilename(wxStandardPaths::Get().GetDocumentsDir(), wxT("dissassembly.asm"));

		// retreive previous filename used
		wxString sValue;
		wxConfig::Get(false)->Read(wxT("debugger/dissassembly/dissassemble/range/filename"), &sValue, sDefaultFilename.GetFullPath());

		wxFileName RecentPath(sValue);

		// show the dialog
		wxString sFilter = wxT("Assembly files (*.asm)|*.asm|All files (*.*)|*.*||");
		wxFileDialog saveFileDialog(
			this,
			wxT("Save dissassembly"),      // title
			RecentPath.GetPath(),         // default dir
			RecentPath.GetFullName(),     // default file
			sFilter,
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

		if (saveFileDialog.ShowModal() == wxID_OK)
		{
			wxString sFilename = saveFileDialog.GetPath();

			// store it for next time
			wxConfig::Get(false)->Write(wxT("debugger/dissassembly/dissassemble/range/filename"), sFilename);

			//  perform the write.
			if (wxGetApp().SaveLocalFile(sFilename, sString))
			{
				wxString sMessage(wxT("Dissassembly saved to file "));
				sMessage += sFilename;

				wxMessageBox(sMessage, wxGetApp().GetAppName(), wxICON_INFORMATION | wxOK);
			}
			else
			{
				wxString sMessage(wxT("Failed to save dissassembly to file "));
				sMessage += sFilename;
				wxMessageBox(sMessage, wxGetApp().GetAppName(), wxICON_INFORMATION | wxOK);
			}
		}
		else
		{
			// user clicked cancel
		}
	}
}

void DissassembleWindow::DoOnPaint(wxAutoBufferedPaintDC &dc)
{
	wxCoord x = TextOrigin.x;
	wxCoord y = TextOrigin.y;
	int i;


	pDissassembleWindow->WidthInChars = m_nCharsWidth;
	pDissassembleWindow->WindowHeight = m_nCharsHeight;
	Dissassemble_RefreshState(pDissassembleWindow);

	DrawCursor(dc, 0, pDissassembleWindow->CursorYAbsolute, m_nCharsWidth, 1);

	Dissassemble_BeginDissassemble(pDissassembleWindow);
	int nWidth, nHeight;
	this->GetClientSize(&nWidth, &nHeight);

	BREAKPOINT_DESCRIPTION Description;
	BreakpointDescription_Init(&Description);
	Description.Type = BREAKPOINT_TYPE_PC;

	for (i = 0; i < pDissassembleWindow->WindowHeight; i++)
	{
		char *pDebugString;
		//int		TextOut_Length;

		//TextOut_Length = _tcslen(pDebugString);

		///	if (TextOut_Length>pDissassembleWindow->WidthInChars)
		//	{
		//	TextOut_Length = pDissassembleWindow->WidthInChars;
		//	}


		if (pDissassembleWindow->ViewType == DISSASSEMBLE_VIEW_Z80_INSTRUCTIONS)
		{
			Description.Address = pDissassembleWindow->CurrentAddr;

			BREAKPOINT *pBreakPoint = Breakpoints_IsAVisibleBreakpoint(&Description);
			if (pBreakPoint)
			{
				this->DrawBreakpointMarker(dc, i, pBreakPoint->bEnabled ? true : false);
			}
		}

		if (pDissassembleWindow->CurrentAddr == pDissassembleWindow->PC)
		{
			this->DrawCurrentMarker(dc, i);
		}

		// should we draw a seperator line after this opcode?
		bool bDrawSeperator = false;
		if (IsSeperatorOpcode(Dissassemble_GetMemoryRange(pDissassembleWindow), pDissassembleWindow->CurrentAddr))
		{
			bDrawSeperator = true;
		}

		pDebugString = Dissassemble_DissassembleNextLine(pDissassembleWindow);


		wxString sText = wxString((const char *)pDebugString, *wxConvCurrent);
		dc.DrawText(sText, x, y);

		// draw the seperator line
		if (bDrawSeperator)
		{
			dc.SetPen(g_TextWindowProperties.m_CursorPen);
			dc.SetBrush(g_TextWindowProperties.m_CursorBrush);

			dc.DrawLine(x, y + m_nFontHeight - 1, nWidth, y + m_nFontHeight - 1);
			dc.SetBrush(wxNullBrush);
			dc.SetPen(wxNullPen);
		}

		y += m_nFontHeight;
	}
	Dissassemble_RefreshAddress(pDissassembleWindow, FALSE);

}


// void arnguiFrame::OnCPCDissassembly(wxCommandEvent &event)
// {
//    DissassembleWindow *pWindow = new DissassembleWindow(this);
//    pWindow->Show();
// }


BEGIN_EVENT_TABLE(StackWindow, OurTextWindow)
EVT_COMMAND(XRCID("Follow_SP"), wxEVT_COMMAND_MENU_SELECTED, StackWindow::OnFollowSP)
EVT_COMMAND(XRCID("m_RunToCursor"), wxEVT_COMMAND_MENU_SELECTED, StackWindow::OnRunToCursor)
EVT_COMMAND(XRCID("Address_Offset"), wxEVT_COMMAND_MENU_SELECTED, StackWindow::OnShowAddressOffset)
EVT_COMMAND(XRCID("Set_SP_To_Address"), wxEVT_COMMAND_MENU_SELECTED, StackWindow::OnSetSPToAddress)
EVT_COMMAND(XRCID("ReturnToSP"), wxEVT_COMMAND_MENU_SELECTED, StackWindow::OnReturnToSP)
EVT_COMMAND(XRCID("ToggleBreakpoint"), wxEVT_COMMAND_MENU_SELECTED, StackWindow::OnToggleBreakpoint)

EVT_COMMAND(XRCID("SendToMemoryDump"), wxEVT_COMMAND_MENU_SELECTED, StackWindow::OnSendDataToMemoryDump)
EVT_COMMAND(XRCID("SendToRegister"), wxEVT_COMMAND_MENU_SELECTED, StackWindow::OnSendDataToRegister)
EVT_COMMAND(XRCID("SendToDissassembly"), wxEVT_COMMAND_MENU_SELECTED, StackWindow::OnSendDataToDissassembly)

EVT_COMMAND(XRCID("GoToAddress"), wxEVT_COMMAND_MENU_SELECTED, StackWindow::OnGotoAddress)
EVT_COMMAND(XRCID("GoToLabel"), wxEVT_COMMAND_MENU_SELECTED, StackWindow::OnGotoLabel)

END_EVENT_TABLE()

void StackWindow::UpdateContextMenuState(wxMenuBar *pMenuBar)
{
	arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("Address_Offset"), Stack_ShowAddressOffset(pStackWindow) ? true : false);
	arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("Follow_SP"), Stack_FollowSP(pStackWindow) ? true : false);
}

void StackWindow::DoClickMargin(int x, int y)
{
	int CharX, CharY;
	PixelToChar(x, y, &CharX, &CharY);
	int	nAddress = Stack_GetLineAddress(pStackWindow, CharY);
	ToggleABreakpoint(nAddress);
}

void StackWindow::ToggleABreakpoint(int Addr)
{
	/* breakpoint exists? */
	BREAKPOINT_DESCRIPTION Description;
	BreakpointDescription_Init(&Description);
	Description.Type = BREAKPOINT_TYPE_SP;
	Description.Address = Addr;

	if (Breakpoints_IsAVisibleBreakpoint(&Description))
	{
		/* yes, remove it */
		Breakpoints_RemoveBreakpointByDescription(&Description);
	}
	else
	{
		/* no, add one */
		Breakpoints_AddBreakpoint(&Description);
	}

	this->Refresh();
}

void StackWindow::ToggleACursorBreakpoint()
{
	int Addr;

	Addr = Stack_GetCursorAddress(pStackWindow);
	ToggleABreakpoint(Addr);
}

void StackWindow::OnToggleBreakpoint(wxCommandEvent & WXUNUSED(event))
{
	ToggleACursorBreakpoint();
}

void StackWindow::OnKeyDown(wxKeyEvent &event)
{
	switch (event.GetKeyCode())
	{
	default:
		break;

#if !defined(__WXMAC__)
	case WXK_F6:
	{
		if (event.GetModifiers() == 0)
		{
			RunToCursor();
			event.Skip(false);
			return;
		}
	}
	break;


	case WXK_F7:
	{
		if (event.GetModifiers() == 0)
		{
			SetSPToCursor();
			event.Skip(false);
			return;
		}

	}
	break;



	case WXK_F9:
	{
		if (event.GetModifiers() == 0)
		{
			ToggleACursorBreakpoint();

			event.Skip(false);

			return;
		}
	}
	break;
	#endif
	}

	OurTextWindow::OnKeyDown(event);
}

void StackWindow::RunToCursor()
{
	int NewAddr = Stack_GetCursorAddress(pStackWindow);

	Debug_SetRunTo(NewAddr);

	// restore sound
	SDLCommon::PauseSound(0);

	this->Refresh();

}
void StackWindow::OnRunToCursor(wxCommandEvent & WXUNUSED(event))
{
	RunToCursor();
}

void StackWindow::OnSendDataToMemoryDump(wxCommandEvent & WXUNUSED(event))
{
	int Address = Stack_GetCursorAddress(pStackWindow);
	int Data = MemoryRange_ReadWord(pStackWindow->pRange, Address);
	SendDataToMemoryDump(Data);
}

void StackWindow::OnSendDataToRegister(wxCommandEvent & WXUNUSED(event))
{
	int Address = Stack_GetCursorAddress(pStackWindow);
	int Data = MemoryRange_ReadWord(pStackWindow->pRange, Address);
	SendDataToRegister(Data);
}

void StackWindow::OnSendDataToDissassembly(wxCommandEvent & WXUNUSED(event))
{
	int Address = Stack_GetCursorAddress(pStackWindow);
	int Data = MemoryRange_ReadWord(pStackWindow->pRange, Address);
	SendDataToDissassembly(Data);
}

void StackWindow::OnGotoLabel(wxCommandEvent & WXUNUSED(event))
{
	int nAddress;
	if (GetGoToLabel(&nAddress))
	{
		Stack_SetAddress(pStackWindow, nAddress);
		this->Refresh();
	}
}


void StackWindow::OnGotoAddress(wxCommandEvent & WXUNUSED(event))
{
	int nAddress;
	if (GetGotoAddress(&nAddress))
	{
		Stack_SetAddress(pStackWindow, nAddress);
		this->Refresh();
	}
}

void StackWindow::DoSelect(int x, int y)
{
	int CharX, CharY;
	PixelToChar(x, y, &CharX, &CharY);
	Stack_SelectByCharXY(pStackWindow, CharX, CharY);

}

void StackWindow::SetSPToCursor()
{
	int NewAddr = Stack_GetCursorAddress(pStackWindow);

	CPU_SetReg(CPU_SP, NewAddr);
	this->Refresh();
}

void StackWindow::OnSetSPToAddress(wxCommandEvent & WXUNUSED(event))
{
	SetSPToCursor();
}

void StackWindow::OnReturnToSP(wxCommandEvent & WXUNUSED(event))
{
	int SPAddress = CPU_GetReg(CPU_SP);
	Stack_SetAddress(pStackWindow, SPAddress);
	this->Refresh();
}

void StackWindow::OnFollowSP(wxCommandEvent & WXUNUSED(event))
{
	Stack_SetFollowSP(pStackWindow, !Stack_FollowSP(pStackWindow));
	this->Refresh();
}

void StackWindow::OnShowAddressOffset(wxCommandEvent & WXUNUSED(event))
{
	Stack_SetShowAddressOffset(pStackWindow, !Stack_ShowAddressOffset(pStackWindow));
	this->Refresh();
}

void StackWindow::DoTab(bool WXUNUSED(bForwards))
{

}

void StackWindow::DoCursorUp()
{
	Stack_CursorUp(pStackWindow);
}

void StackWindow::DoCursorDown()
{
	Stack_CursorDown(pStackWindow);

}

void StackWindow::DoCursorLeft()
{
}

void StackWindow::DoCursorRight()
{
}

void StackWindow::DoPageUp()
{
	Stack_PageUp(pStackWindow);

}

void StackWindow::DoPageDown()
{
	Stack_PageDown(pStackWindow);

}

bool StackWindow::Create(wxWindow* parent, wxWindowID id, const wxString &label, const wxPoint& pos, const wxSize& size, long style, const wxValidator &validator, const wxString& name)
{
	bool bCreated = OurTextWindow::Create(parent, id, label, pos, size, style, validator, name);
	if (bCreated)
	{
		if (DebuggerDisplay.StackSettingValid)
		{
			Stack_FromSettings(pStackWindow, &DebuggerDisplay.StackSettings);
		}
	}
	return bCreated;
}

wxObject *StackWindowResourceHandler::DoCreateResource()
{

	XRC_MAKE_INSTANCE(control, StackWindow)

		control->Create(m_parentAsWindow,
		GetID(),
		GetText(wxT("label")),
		GetPosition(), GetSize(),
		GetStyle(),
		wxDefaultValidator,
		GetName());

	SetupWindow(control);
	return control;
}


bool StackWindowResourceHandler::CanHandle(wxXmlNode *node)
{
	return IsOfClass(node, wxT("StackWindow"));
}


StackWindowResourceHandler::StackWindowResourceHandler() : wxXmlResourceHandler()
{


}

StackWindowResourceHandler::~StackWindowResourceHandler()
{

}

StackWindow::StackWindow() : OurTextWindow()
{
	m_bHasMargin = true;
	m_sContextMenuName = wxT("MB_MENU_STACK_MENU_BAR");

	pStackWindow = Stack_Create();
	Stack_SetMemoryRange(pStackWindow, (MemoryRange *)CPC_GetDefaultMemoryRange());

	DebuggerWindow::RegisterStackWindow(this);

}

StackWindow::~StackWindow()
{
	DebuggerWindow::DeRegisterStackWindow(this);
	DebuggerDisplay.StackSettingValid = true;
	Stack_ToSettings(pStackWindow, &DebuggerDisplay.StackSettings);
	Stack_Finish(pStackWindow);

}

void StackWindow::DoOnPaint(wxAutoBufferedPaintDC &dc)
{
	wxCoord x = TextOrigin.x;
	wxCoord y = TextOrigin.y;
	int i;

	pStackWindow->WidthInChars = m_nCharsWidth;
	pStackWindow->WindowHeight = m_nCharsHeight;
	Stack_RefreshState(pStackWindow);

	DrawCursor(dc, 0, pStackWindow->CursorYAbsolute, m_nCharsWidth, 1);

	BREAKPOINT_DESCRIPTION Description;
	BreakpointDescription_Init(&Description);
	Description.Type = BREAKPOINT_TYPE_SP;

	for (i = 0; i < pStackWindow->WindowHeight; i++)
	{
		char *pDebugString;
		//int		TextOut_Length;

		Description.Address = pStackWindow->CurrentAddr;
		BREAKPOINT *pBreakPoint = Breakpoints_IsAVisibleBreakpoint(&Description);

		if (pBreakPoint)
		{
			this->DrawBreakpointMarker(dc, i, pBreakPoint->bEnabled ? true : false);
		}

		if (pStackWindow->CurrentAddr == pStackWindow->SP)
		{
			this->DrawCurrentMarker(dc, i);

		}
		pDebugString = Stack_OutputNextLine(pStackWindow);

		//TextOut_Length = _tcslen(pDebugString);

		///	if (TextOut_Length>pDissassembleWindow->WidthInChars)
		//	{
		//	TextOut_Length = pDissassembleWindow->WidthInChars;
		//	}

		wxString sText = wxString((const char *)pDebugString, *wxConvCurrent);
		dc.DrawText(sText, x, y);

		y += m_nFontHeight;
	}

	Stack_RefreshAddress(pStackWindow, FALSE);
}


void OurTextWindow::SendDataToDissassembly(int nData)
{
	wxArrayString sRegisters;

	for (int i = 0; i < DebuggerWindow::GetDissassemblyWindowCount(); i++)
	{
		wxString sWindowName;
		sWindowName.Printf(wxT("Disassembly %d"), i + 1);
		sRegisters.Add(sWindowName);
	}

	wxSingleChoiceDialog dialog(this, wxT("Select disassembly window"), wxT("Send Cursor Address To..."), sRegisters);
	if (dialog.ShowModal() == wxID_OK)
	{
		int nSelection = dialog.GetSelection();
		DissassembleWindow *pWindow = DebuggerWindow::GetDissassemblyWindowByIndex(nSelection);
		if (pWindow)
		{
			DISSASSEMBLE_WINDOW *pDisassembleWindow = pWindow->GetStruct();
			Dissassemble_SetAddress(pDisassembleWindow, nData);
			pWindow->Refresh();
		}
	}
}


void OurTextWindow::SendDataToMemoryDump(int nData)
{
	wxArrayString sRegisters;

	for (int i = 0; i < DebuggerWindow::GetMemdumpWindowCount(); i++)
	{
		wxString sWindowName;
		sWindowName.Printf(wxT("Memory Dump %d"), i + 1);
		sRegisters.Add(sWindowName);
	}

	wxSingleChoiceDialog dialog(this, wxT("Select memory dump window"), wxT("Send Cursor Address To..."), sRegisters);
	if (dialog.ShowModal() == wxID_OK)
	{
		int nSelection = dialog.GetSelection();
		MemDumpWindow *pWindow = DebuggerWindow::GetMemdumpWindowByIndex(nSelection);
		if (pWindow)
		{
			MEMDUMP_WINDOW *pMemdumpWindow = pWindow->GetStruct();
			Memdump_SetCursorAddress(pMemdumpWindow, nData);
			pWindow->Refresh();
		}
	}
}


void OurTextWindow::SendDataToStack(int nData)
{
	wxArrayString sRegisters;

	for (int i = 0; i < DebuggerWindow::GetStackWindowCount(); i++)
	{
		wxString sWindowName;
		sWindowName.Printf(wxT("Stack Window %d"), i + 1);
		sRegisters.Add(sWindowName);
	}

	wxSingleChoiceDialog dialog(this, wxT("Select memory dump window"), wxT("Send Cursor Address To..."), sRegisters);
	if (dialog.ShowModal() == wxID_OK)
	{
		int nSelection = dialog.GetSelection();
		StackWindow *pWindow = DebuggerWindow::GetStackWindowByIndex(nSelection);
		if (pWindow)
		{
			STACK_WINDOW *pStackWindow = pWindow->GetStruct();
			Stack_SetAddress(pStackWindow, nData);
			pWindow->Refresh();
		}
	}
}



// void arnguiFrame::OnCPCStack(wxCommandEvent &event)
// {
//    StackWindow *pWindow = new StackWindow(this);
//    pWindow->Show();
// }

DissassemblyWindowArray DebuggerWindow::m_DissassemblyWindows;
MemdumpWindowArray DebuggerWindow::m_MemdumpWindows;
StackWindowArray DebuggerWindow::m_StackWindows;
HardwareDetails *DebuggerWindow::m_HardwareDetails = NULL;
Breakpoints *DebuggerWindow::m_Breakpoints = NULL;

void DebuggerWindow::RegisterDissassemblyWindow(DissassembleWindow *pWindow)
{
	for (unsigned int i = 0; i != m_DissassemblyWindows.GetCount(); i++)
	{
		if (m_DissassemblyWindows.Item(i) == pWindow)
			return;
	}

	m_DissassemblyWindows.Add(pWindow);
}

void DebuggerWindow::DeRegisterDissassemblyWindow(DissassembleWindow *pWindow)
{
	for (unsigned int i = 0; i != m_DissassemblyWindows.GetCount(); i++)
	{
		if (m_DissassemblyWindows.Item(i) == pWindow)
		{
			m_DissassemblyWindows.RemoveAt(i);
			return;
		}
	}
}


void DebuggerWindow::RegisterMemdumpWindow(MemDumpWindow *pWindow)
{
	for (unsigned int i = 0; i != m_MemdumpWindows.GetCount(); i++)
	{
		if (m_MemdumpWindows.Item(i) == pWindow)
			return;
	}

	m_MemdumpWindows.Add(pWindow);
}

void DebuggerWindow::DeRegisterMemdumpWindow(MemDumpWindow *pWindow)
{
	for (unsigned int i = 0; i != m_MemdumpWindows.GetCount(); i++)
	{
		if (m_MemdumpWindows.Item(i) == pWindow)
		{
			m_MemdumpWindows.RemoveAt(i);
			return;
		}
	}
}



void DebuggerWindow::RegisterStackWindow(StackWindow *pWindow)
{
	for (unsigned int i = 0; i != m_StackWindows.GetCount(); i++)
	{
		if (m_StackWindows.Item(i) == pWindow)
			return;
	}

	m_StackWindows.Add(pWindow);
}

void DebuggerWindow::DeRegisterStackWindow(StackWindow *pWindow)
{
	for (unsigned int i = 0; i != m_StackWindows.GetCount(); i++)
	{
		if (m_StackWindows.Item(i) == pWindow)
		{
			m_StackWindows.RemoveAt(i);
			return;
		}
	}
}



void DebuggerWindow::RegisterHardwareDetails(HardwareDetails *pWindow)
{
	m_HardwareDetails = pWindow;
}

void DebuggerWindow::DeRegisterHardwareDetails(HardwareDetails * WXUNUSED(pWindow))
{
	m_HardwareDetails = NULL;
}


void DebuggerWindow::RegisterBreakpoints(Breakpoints *pWindow)
{
	m_Breakpoints = pWindow;
}

void DebuggerWindow::DeRegisterBreakpoints(Breakpoints * WXUNUSED(pWindow))
{
	m_Breakpoints = NULL;
}

Breakpoints *DebuggerWindow::GetBreakpointWindow()
{
	return m_Breakpoints;
}

int DebuggerWindow::GetDissassemblyWindowCount()
{
	return m_DissassemblyWindows.size();
}

DissassembleWindow *DebuggerWindow::GetDissassemblyWindowByIndex(size_t nIndex)
{
	if (nIndex >= m_DissassemblyWindows.size())
		return NULL;

	return m_DissassemblyWindows[nIndex];
}

int DebuggerWindow::GetMemdumpWindowCount()
{
	return m_MemdumpWindows.size();
}

MemDumpWindow *DebuggerWindow::GetMemdumpWindowByIndex(size_t nIndex)
{
	if (nIndex >= m_MemdumpWindows.size())
		return NULL;

	return m_MemdumpWindows[nIndex];
}


int DebuggerWindow::GetStackWindowCount()
{
	return m_StackWindows.size();
}

StackWindow *DebuggerWindow::GetStackWindowByIndex(size_t nIndex)
{
	if (nIndex>=m_StackWindows.size())
		return NULL;

	return m_StackWindows[nIndex];
}


#if 0
bool DebuggerWindow::TransferDataToWindow()
{
	/*
	  wxCheckBox *pCheckBox;
	  wxWindow *pWindow;
	  wxListBox *pListBox;
	  wxRadioButton *pRadioButton;
	  wxChoice *pChoice;

	  pWindow = this->FindWindow(XRCID("ForceReady"));
	  pCheckBox = static_cast<wxCheckBox *>(pWindow);
	  */
	return true;
}
#endif

DebuggerWindow *DebuggerWindow::m_pInstance = NULL;

// creator
DebuggerWindow *DebuggerWindow::CreateInstance(wxWindow *pParent)
{
	if (m_pInstance == NULL)
	{
		m_pInstance = new DebuggerWindow(pParent);
		if (m_pInstance != NULL)
		{
			// if opened for first time cause a break
			if (!Debug_IsStopped())
			{
				Debug_TriggerBreak();
			}
			m_pInstance->Show();
			m_pInstance->InitialUpdate();
		}

	}
	else
	{
		m_pInstance->Raise();
	}
	return m_pInstance;
}


void DebuggerWindow::Init()
{
	bool bValue;
	wxConfig::Get(false)->Read(wxT("debug/break_opcode_enable"), &bValue, false);
	CPU_SetDebugOpcodeEnabled(bValue);


	Debug_SetDebuggerWindowOpenCallback(OpenWindowCallback);
}

void DebuggerWindow::OpenWindowCallback(void)
{
	DebuggerWindow::CreateInstance(wxGetApp().GetTopWindow());
}

DebuggerWindow::~DebuggerWindow()
{

}

BEGIN_EVENT_TABLE(DebuggerWindow, wxDialog)
EVT_CLOSE(DebuggerWindow::OnClose)
END_EVENT_TABLE()


void DebuggerWindow::OnClose(wxCloseEvent & WXUNUSED(event))
{
	arnguiFrame *pFrame = static_cast<arnguiFrame *>(GetParent());
	pFrame->DeRegisterWantUpdateFromTimer(this);

	wxSplitterWindow *pSplitterWindow;

	pSplitterWindow = XRCCTRL(*this, "m_splitter2", wxSplitterWindow); 
	wxConfig::Get(false)->Write(wxT("windows/debugger/splitter1/sashpos"), pSplitterWindow->GetSashPosition());

	pSplitterWindow = XRCCTRL(*this, "m_splitter3", wxSplitterWindow);
	wxConfig::Get(false)->Write(wxT("windows/debugger/splitter2/sashpos"), pSplitterWindow->GetSashPosition());

	pSplitterWindow = XRCCTRL(*this, "m_splitter1", wxSplitterWindow);
	wxConfig::Get(false)->Write(wxT("windows/debugger/splitter3/sashpos"), pSplitterWindow->GetSashPosition());
	


	wxGetApp().WriteConfigWindowPosAndSize(wxT("windows/debugger/"), this);
	this->Destroy();

	DebuggerWindow::m_pInstance = NULL;

	// restore sound
	SDLCommon::PauseSound(0);

	Debug_Continue();
}

DebuggerWindow::DebuggerWindow(wxWindow *pParent)
{
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DebuggerDialog"));

	// default size
	// this->SetSize(800,800);

	wxGetApp().ReadConfigWindowPosAndSize(wxT("windows/debugger/"), this);
	wxGetApp().EnsureWindowVisible(this);


	wxSplitterWindow *pSplitterWindow;
	int SashPos;
	pSplitterWindow = XRCCTRL(*this, "m_splitter2", wxSplitterWindow);
	wxConfig::Get(false)->Read(wxT("windows/debugger/splitter1/sashpos"), &SashPos, -1);
	if (SashPos != -1)
	{
		pSplitterWindow->SetSashPosition(SashPos);
	}
	
	pSplitterWindow = XRCCTRL(*this, "m_splitter3", wxSplitterWindow);
	wxConfig::Get(false)->Read(wxT("windows/debugger/splitter2/sashpos"), &SashPos, -1);
	if (SashPos != -1)
	{
		pSplitterWindow->SetSashPosition(SashPos);
	}

	pSplitterWindow = XRCCTRL(*this, "m_splitter1", wxSplitterWindow);
	wxConfig::Get(false)->Read(wxT("windows/debugger/splitter3/sashpos"), &SashPos, -1);
	if (SashPos != -1)
	{
		pSplitterWindow->SetSashPosition(SashPos);
	}



	arnguiFrame *pFrame = static_cast<arnguiFrame *>(GetParent());
	pFrame->RegisterWantUpdateFromTimer(this);

	wxNotebook *pNotebook;

	pNotebook = XRCCTRL(*this, "DissassemblyNotebook", wxNotebook);
	for (size_t i = 0; i < pNotebook->GetPageCount(); i++)
	{
		// turn off background on page
		wxNotebookPage *pPage = pNotebook->GetPage(i);
		wxColour col = pNotebook->GetThemeBackgroundColour();
		if (col.Ok())
		{
			pPage->SetBackgroundColour(col);
		}
	}
}

void DebuggerWindow::InitialUpdate()
{
	for (unsigned int i = 0; i != m_MemdumpWindows.GetCount(); i++)
	{
		MemDumpWindow *pWindow = m_MemdumpWindows[i];
		if (pWindow->IsShown())
		{
			pWindow->Refresh();
		}
	}

	for (unsigned int i = 0; i != m_DissassemblyWindows.GetCount(); i++)
	{
		DissassembleWindow *pWindow = m_DissassemblyWindows[i];
		Dissassemble_RefreshAddress(pWindow->GetStruct(), Dissassemble_FollowPC(pWindow->GetStruct()));
		if (pWindow->IsShown())
		{
			pWindow->Refresh();
		}
	}
	for (unsigned int i = 0; i != m_StackWindows.GetCount(); i++)
	{
		StackWindow *pWindow = m_StackWindows[i];
		Stack_RefreshAddress(pWindow->GetStruct(), Stack_FollowSP(pWindow->GetStruct()));
		if (pWindow->IsShown())
		{
			pWindow->Refresh();
		}
	}
	if (m_HardwareDetails != NULL)
	{
		if (m_HardwareDetails->IsShown())
		{
			m_HardwareDetails->PerformRefresh();
		}
	}

	if (m_Breakpoints != NULL)
	{
		if (m_Breakpoints->IsShown())
		{
			m_Breakpoints->PerformRefresh();
		}
	}
}

void DebuggerWindow::TimedUpdate()
{
	bool bUpdate = false;
	int nUpdateAddress = 0;

	// debugger has stopped but signalled a refresh?
	if (Debug_GetDebuggerRefresh())
	{
		// reset refresh state
		Debug_ResetDebuggerRefresh();

		bUpdate = true;
		nUpdateAddress = 1;
	}
	else
	{
		// not signalled a refresh.. is it running?
		if (!Debug_IsStopped())
		{
			bUpdate = true;

			// it's running, but decision to update address is based on setting
			nUpdateAddress = 2;
		}
	}

	if (bUpdate)
	{
		// ::wxStartTimer();
		//    wxStopWatch sw;
		//  sw.Start();

		for (unsigned int i = 0; i != m_MemdumpWindows.GetCount(); i++)
		{
			MemDumpWindow *pWindow = m_MemdumpWindows[i];
			if (pWindow->IsShown())
			{
				pWindow->Refresh();
			}
		}
		//{
		//	sw.Pause();
		//	long nValue = sw.Time();
		// printf("d: %d\n", nValue);
		//	sw.Resume();
		//}
		for (unsigned int i = 0; i != m_DissassemblyWindows.GetCount(); i++)
		{
			DissassembleWindow *pWindow = m_DissassemblyWindows[i];
			if (pWindow->IsShown())
			{
				pWindow->Refresh();

			}
			if (nUpdateAddress == 2)
			{
				if (Dissassemble_FollowPC(pWindow->GetStruct()))
				{
					nUpdateAddress = 1;
				}
				else
				{
					nUpdateAddress = 0;
				}
			}
			else if (nUpdateAddress == 1)
			{
				if (!Dissassemble_FollowPC(pWindow->GetStruct()))
				{
					nUpdateAddress = 0;
				}
			}
			Dissassemble_RefreshAddress(pWindow->GetStruct(), (nUpdateAddress == 1));
		}
		//{
		//	sw.Pause();
		//	long nValue = sw.Time();
		//printf("d: %d\n", nValue);
		//sw.Resume();
		//}
		for (unsigned int i = 0; i != m_StackWindows.GetCount(); i++)
		{
			StackWindow *pWindow = m_StackWindows[i];
			if (pWindow->IsShown())
			{
				pWindow->Refresh();
			}
			if (nUpdateAddress == 2)
			{
				if (Stack_FollowSP(pWindow->GetStruct()))
				{
					nUpdateAddress = 1;
				}
				else
				{
					nUpdateAddress = 0;
				}
			}
			else if (nUpdateAddress == 1)
			{
				if (!Stack_FollowSP(pWindow->GetStruct()))
				{
					nUpdateAddress = 0;
				}
			}

			Stack_RefreshAddress(pWindow->GetStruct(), (nUpdateAddress == 1));
		}
		//{
		//	sw.Pause();
		//	long nValue = sw.Time();
		// printf("d: %d\n", nValue);
		//	sw.Resume();
		//}
		if (m_HardwareDetails != NULL)
		{
			if (m_HardwareDetails->IsShown())
			{
				m_HardwareDetails->PerformRefresh();
			}
		}
		//{
		//	sw.Pause();
		//	long nValue = sw.Time();
		//printf("d: %d\n", nValue);
		//	sw.Resume();
		//}
		if (m_Breakpoints != NULL)
		{
			if (m_Breakpoints->IsShown())
			{
				m_Breakpoints->PerformRefresh();
			}
		}
		//{
		//	sw.Pause();
		//	long nValue = sw.Time();
		//printf("d: %d\n", nValue);
		//sw.Resume();
		//}
	}
}


IMPLEMENT_DYNAMIC_CLASS(HardwareDetails, wxListCtrl)

wxString HardwareDetails::OnGetItemText(long item, long column) const
{
	HardwareDetailsItem *pItem = (HardwareDetailsItem *)m_Items.Item(item);
	switch (column)
	{

	case 0:
		return pItem->m_Label;
	case 1:
		return pItem->m_CachedValue;
	case 2:
		if (pItem->m_bChanged)
			return m_sChangedText;
		else
			return wxEmptyString;
	}

	return wxEmptyString;
}


wxObject *HardwareDetailsResourceHandler::DoCreateResource()
{

	XRC_MAKE_INSTANCE(control, HardwareDetails)

		control->Create(m_parentAsWindow,
		GetID(),
		GetPosition(), GetSize(),
		GetStyle(),
		wxDefaultValidator,
		GetName());

	SetupWindow(control);
	return control;
}


bool HardwareDetailsResourceHandler::CanHandle(wxXmlNode *node)
{
	return IsOfClass(node, wxT("HardwareDetails"));
}


HardwareDetailsResourceHandler::HardwareDetailsResourceHandler() : wxXmlResourceHandler()
{


}

HardwareDetailsResourceHandler::~HardwareDetailsResourceHandler()
{

}


void HardwareDetails::OnShowROMSelect(wxCommandEvent & WXUNUSED(event))
{
	DetailsDisplay.m_bDisplayROMSelect = !DetailsDisplay.m_bDisplayROMSelect;
	PerformRefreshShown();
}

void HardwareDetails::OnShowPPI(wxCommandEvent & WXUNUSED(event))
{
	DetailsDisplay.m_bDisplayPPI = !DetailsDisplay.m_bDisplayPPI;
	PerformRefreshShown();
}


void HardwareDetails::OnShowPrinter(wxCommandEvent & WXUNUSED(event))
{
	DetailsDisplay.m_bDisplayPrinter = !DetailsDisplay.m_bDisplayPrinter;
	PerformRefreshShown();
}


void HardwareDetails::OnShowMonitor(wxCommandEvent & WXUNUSED(event))
{
	DetailsDisplay.m_bDisplayMonitor = !DetailsDisplay.m_bDisplayMonitor;
	PerformRefreshShown();
}

void HardwareDetails::OnShowZ80(wxCommandEvent & WXUNUSED(event))
{
	DetailsDisplay.m_bDisplayZ80 = !DetailsDisplay.m_bDisplayZ80;
	PerformRefreshShown();
}

void HardwareDetails::OnShowZ80Registers(wxCommandEvent & WXUNUSED(event))
{
	DetailsDisplay.m_bDisplayZ80Registers = !DetailsDisplay.m_bDisplayZ80Registers;
	PerformRefreshShown();
}

void HardwareDetails::OnShowZ80Inputs(wxCommandEvent & WXUNUSED(event))
{
	DetailsDisplay.m_bDisplayZ80Inputs = !DetailsDisplay.m_bDisplayZ80Inputs;
	PerformRefreshShown();
}

void HardwareDetails::OnShowCRTC(wxCommandEvent & WXUNUSED(event))
{
	DetailsDisplay.m_bDisplayCRTC = !DetailsDisplay.m_bDisplayCRTC;
	PerformRefreshShown();
}

void HardwareDetails::OnShowCRTCRegisters(wxCommandEvent & WXUNUSED(event))
{
	DetailsDisplay.m_bDisplayCRTCRegisters = !DetailsDisplay.m_bDisplayCRTCRegisters;
	PerformRefreshShown();
}

void HardwareDetails::OnShowCRTCOutputs(wxCommandEvent & WXUNUSED(event))
{
	DetailsDisplay.m_bDisplayCRTCOutputs = !DetailsDisplay.m_bDisplayCRTCOutputs;
	PerformRefreshShown();
}

void HardwareDetails::OnShowAY(wxCommandEvent & WXUNUSED(event))
{
	DetailsDisplay.m_bDisplayAY = !DetailsDisplay.m_bDisplayAY;
	PerformRefreshShown();
}

void HardwareDetails::OnShowKeyboard(wxCommandEvent & WXUNUSED(event))
{
	DetailsDisplay.m_bDisplayKeyboard = !DetailsDisplay.m_bDisplayKeyboard;
	PerformRefreshShown();
}

void HardwareDetails::OnShowAYRegisters(wxCommandEvent & WXUNUSED(event))
{
	DetailsDisplay.m_bDisplayAYRegisters = !DetailsDisplay.m_bDisplayAYRegisters;
	PerformRefreshShown();
}

void HardwareDetails::OnShowASIC(wxCommandEvent & WXUNUSED(event))
{
	DetailsDisplay.m_bDisplayASIC = !DetailsDisplay.m_bDisplayASIC;
	PerformRefreshShown();
}

void HardwareDetails::OnShowASICPalette(wxCommandEvent & WXUNUSED(event))
{
	DetailsDisplay.m_bDisplayASICPalette = !DetailsDisplay.m_bDisplayASICPalette;
	PerformRefreshShown();
}

void HardwareDetails::OnShowASICDMA(wxCommandEvent & WXUNUSED(event))
{
	DetailsDisplay.m_bDisplayASICDMA = !DetailsDisplay.m_bDisplayASICDMA;
	PerformRefreshShown();
}

void HardwareDetails::OnShowASICAnalogueInputs(wxCommandEvent & WXUNUSED(event))
{
	DetailsDisplay.m_bDisplayASICAnalogueInputs = !DetailsDisplay.m_bDisplayASICAnalogueInputs;
	PerformRefreshShown();
}

void HardwareDetails::OnShowASICSprites(wxCommandEvent & WXUNUSED(event))
{
	DetailsDisplay.m_bDisplayASICSprites = !DetailsDisplay.m_bDisplayASICSprites;
	PerformRefreshShown();
}

void HardwareDetails::OnShowFDC(wxCommandEvent & WXUNUSED(event))
{
	DetailsDisplay.m_bDisplayFDC = !DetailsDisplay.m_bDisplayFDC;
	PerformRefreshShown();
}

void HardwareDetails::OnShowFDI(wxCommandEvent & WXUNUSED(event))
{
	DetailsDisplay.m_bDisplayFDI = !DetailsDisplay.m_bDisplayFDI;
	PerformRefreshShown();
}

void HardwareDetails::OnShowGA(wxCommandEvent & WXUNUSED(event))
{
	DetailsDisplay.m_bDisplayGA = !DetailsDisplay.m_bDisplayGA;
	PerformRefreshShown();
}

void HardwareDetails::OnShowKCCompact(wxCommandEvent & WXUNUSED(event))
{
	DetailsDisplay.m_bDisplayKCC = !DetailsDisplay.m_bDisplayKCC;
	PerformRefreshShown();
}

void HardwareDetails::OnShowAleste(wxCommandEvent & WXUNUSED(event))
{
	DetailsDisplay.m_bDisplayAleste = !DetailsDisplay.m_bDisplayAleste;
	PerformRefreshShown();
}

BEGIN_EVENT_TABLE(HardwareDetails, wxListCtrl)
EVT_KEY_DOWN(HardwareDetails::OnKeyDown)

EVT_COMMAND(XRCID("ShowZ80"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnShowZ80)
EVT_COMMAND(XRCID("ShowZ80Registers"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnShowZ80Registers)
EVT_COMMAND(XRCID("ShowZ80Inputs"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnShowZ80Inputs)

EVT_COMMAND(XRCID("ShowCRTC"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnShowCRTC)
EVT_COMMAND(XRCID("ShowCRTCRegisters"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnShowCRTCRegisters)

EVT_COMMAND(XRCID("ShowCRTCOutputs"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnShowCRTCOutputs)

EVT_COMMAND(XRCID("ShowKeyboard"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnShowKeyboard)



EVT_COMMAND(XRCID("ShowAY"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnShowAY)
EVT_COMMAND(XRCID("ShowAYRegisters"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnShowAYRegisters)


EVT_COMMAND(XRCID("ShowASIC"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnShowASIC)

EVT_COMMAND(XRCID("ShowASICPalette"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnShowASICPalette)

EVT_COMMAND(XRCID("ShowASICDMA"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnShowASICDMA)

EVT_COMMAND(XRCID("ShowASICAnalogueInputs"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnShowASICAnalogueInputs)
EVT_COMMAND(XRCID("ShowASICSprites"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnShowASICSprites)


EVT_COMMAND(XRCID("ShowFDC"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnShowFDC)


EVT_COMMAND(XRCID("ShowFDI"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnShowFDI)


EVT_COMMAND(XRCID("ShowGA"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnShowGA)


EVT_COMMAND(XRCID("ShowKCCompact"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnShowKCCompact)


EVT_COMMAND(XRCID("ShowAleste"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnShowAleste)


EVT_COMMAND(XRCID("ShowPrinter"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnShowPrinter)


EVT_COMMAND(XRCID("ShowPPI"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnShowPPI)



EVT_COMMAND(XRCID("ShowMonitor"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnShowMonitor)



EVT_COMMAND(XRCID("ShowROMSelect"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnShowROMSelect)


EVT_COMMAND(XRCID("DisplayHex"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnDisplay)


EVT_COMMAND(XRCID("DisplayBinary"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnDisplay)


EVT_COMMAND(XRCID("DisplayDecimal"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnDisplay)


EVT_COMMAND(XRCID("m_ClearChangedIndicator"), wxEVT_COMMAND_MENU_SELECTED, HardwareDetails::OnClearChanged)

EVT_WINDOW_CREATE(HardwareDetails::OnCreate)

//  EVT_LIST_CACHE_HINT(0, HardwareDetails::OnCacheHint)

EVT_CONTEXT_MENU(HardwareDetails::OnContextMenu)
END_EVENT_TABLE()

void HardwareDetails::OnClearChanged(wxCommandEvent & WXUNUSED(event))
{
	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item == -1)
		return;

	HardwareDetailsItem *pItem = (HardwareDetailsItem *)m_Items.Item(item);
	pItem->m_Data.SetValueChanged(false);

	PerformRefresh();

}

void HardwareDetails::OnCacheHint(wxListEvent &event)
{
	long nFrom = event.GetCacheFrom();
	long nTo = event.GetCacheTo();



	bool bRefresh = false;
	if (nFrom != m_nCachedTopItem)
	{
		bRefresh = true;
	}
	if (nTo != m_nCachedBottomItem)
	{
		bRefresh = true;
	}

	if (bRefresh)
	{
		long nCacheFrom;
		long nCacheTo;

		if (
			// from has gone less than cached top item
			(nFrom < m_nCachedTopItem) &&
			// to remained within our cached range
			((nTo >= m_nCachedTopItem) && (nTo <= m_nCachedBottomItem))
			)
		{
			//            wxMessageOutputDebug().Printf(wxT("OnCacheHint Scroll up\r\n"));

			nCacheFrom = nFrom;
			nCacheTo = m_nCachedTopItem;
		}
		else if
			(
			// to has increased
			(nTo > m_nCachedBottomItem) &&
			// from remained within our cached range
			((nFrom >= m_nCachedTopItem) && (nFrom <= m_nCachedBottomItem))
			)
		{
			//          wxMessageOutputDebug().Printf(wxT("OnCacheHint Scroll down/resize\r\n"));

			nCacheFrom = m_nCachedBottomItem;
			nCacheTo = nTo;
		}
		else
		{

			//      wxMessageOutputDebug().Printf(wxT("OnCacheHint New range\r\n"));

			nCacheFrom = nFrom;
			nCacheTo = nTo;
		}

		m_nCachedTopItem = nFrom;
		m_nCachedBottomItem = nTo;




		this->Freeze();
		for (int i = nCacheFrom; i <= nCacheTo; i++)
		{
			HardwareDetailsItem *pItem = (HardwareDetailsItem *)m_Items.Item(i);

			if (pItem->m_Data.RefreshNeeded(pItem->m_CachedValue))
			{
				this->RefreshItem(i);
				pItem->m_bChanged = pItem->m_Data.GetItemChanged();
			}
		}
		this->Thaw();
	}
}

int HardwareDetails::GetDisplayIdFromMenuId(int nMenuId)
{
	if (nMenuId == XRCID("DisplayHex"))
		return (int)ItemData::ITEM_REPRESENTATION_HEX;

	if (nMenuId == XRCID("DisplayDecimal"))
		return (int)ItemData::ITEM_REPRESENTATION_DECIMAL;

	if (nMenuId == XRCID("DisplayBinary"))
		return (int)ItemData::ITEM_REPRESENTATION_BINARY;

	return (int)0;
}

void HardwareDetails::OnDisplay(wxCommandEvent &event)
{
	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item == -1)
		return;

	int DisplayId = GetDisplayIdFromMenuId(event.GetId());

	HardwareDetailsItem *pItem = (HardwareDetailsItem *)m_Items.Item(item);
	pItem->m_Data.SetRepresentation(DisplayId);

	PerformRefresh();
}

#if 0
void HardwareDetails::OnDisplayUpdateUI(wxUpdateUIEvent &event)
{
	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item == -1)
		return;

	int DisplayId = GetDisplayIdFromMenuId(event.GetId());

	HardwareDetailsItem *pItem = (HardwareDetailsItem *)m_Items.Item(item);
	event.Check(pItem->m_Data.GetRepresentation() == DisplayId);

	PerformRefresh();
}
#endif

void HardwareDetails::OnContextMenu(wxContextMenuEvent &event)
{
	wxPoint Position = event.GetPosition();

	// generate a suitable place for the menu
	if (event.GetPosition() == wxDefaultPosition)
	{
		wxPoint ClientPos(0, 0);
		Position = this->ClientToScreen(ClientPos);
	}

	wxMenuBar *pMenuBar = wxXmlResource::Get()->LoadMenuBar(wxT("MB_MENU_HARDWAREINFO_MENU_BAR"));
	if (pMenuBar)
	{

			arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ShowZ80"), DetailsDisplay.m_bDisplayZ80);
		arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ShowZ80Registers"), DetailsDisplay.m_bDisplayZ80Registers);
		arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ShowZ80Inputs"), DetailsDisplay.m_bDisplayZ80Inputs);
		arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ShowCRTC"), DetailsDisplay.m_bDisplayCRTC);
		arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ShowCRTCRegisters"), DetailsDisplay.m_bDisplayCRTCRegisters);
		arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ShowCRTCOutputs"), DetailsDisplay.m_bDisplayCRTCOutputs);
		arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ShowKeyboard"), DetailsDisplay.m_bDisplayKeyboard);
		arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ShowAY"), DetailsDisplay.m_bDisplayAY);
		arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ShowAYRegisters"), DetailsDisplay.m_bDisplayAYRegisters);
		arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ShowASIC"), DetailsDisplay.m_bDisplayASIC);
		arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ShowASICPalette"), DetailsDisplay.m_bDisplayASICPalette);
			arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ShowASICDMA"), DetailsDisplay.m_bDisplayASICDMA);
		arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ShowASICAnalogueInputs"), DetailsDisplay.m_bDisplayASICAnalogueInputs);
		arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ShowASICSprites"), DetailsDisplay.m_bDisplayASICSprites);
			arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ShowFDC"), DetailsDisplay.m_bDisplayFDC);
			arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ShowFDI"), DetailsDisplay.m_bDisplayFDI);
			arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ShowGA"), DetailsDisplay.m_bDisplayGA);
			arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ShowKCCompact"), DetailsDisplay.m_bDisplayKCC);
			arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ShowAleste"), DetailsDisplay.m_bDisplayAleste);
			arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ShowPrinter"), DetailsDisplay.m_bDisplayPrinter);
			arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ShowPPI"), DetailsDisplay.m_bDisplayPPI);
			arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ShowMonitor"), DetailsDisplay.m_bDisplayMonitor);
			arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ShowROMSelect"), DetailsDisplay.m_bDisplayROMSelect);

//			arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("DisplayHex"), HardwareDetails::OnDisplayUpdateUI);
//			arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("DisplayBinary"), HardwareDetails::OnDisplayUpdateUI);

//			arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("DisplayDecimal"), HardwareDetails::OnDisplayUpdateUI);




		wxMenu *pMenu = pMenuBar->Remove(0);
		if (pMenu != NULL)
		{
			PopupMenu(pMenu);
			delete pMenu;
		}
		delete pMenuBar;
	}

}

void HardwareDetails::OnCreate(wxWindowCreateEvent & WXUNUSED(event))
{

	//   style |= wxLC_REPORT|wxLC_VIRTUAL;

	Freeze();
	ClearAll();

	wxListItem Column;
	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT("Name"));
	InsertColumn(0, Column);
	Column.SetText(wxT("Value"));
	InsertColumn(1, Column);
	Column.SetText(wxT("Changed"));
	InsertColumn(2, Column);

	wxGetApp().SetColumnSize(this, 0);
	wxGetApp().SetColumnSize(this, 1);
	wxGetApp().SetColumnSize(this, 2);


	Thaw();

	// connect event
	this->Connect(GetId(), wxEVT_COMMAND_LIST_CACHE_HINT, wxListEventHandler(HardwareDetails::OnCacheHint));

	// now send it

	PerformRefreshShown();
}


HardwareDetails::HardwareDetails() : wxListCtrl(), m_nCachedTopItem(-1), m_nCachedBottomItem(-1)
{
	m_sChangedText = wxT("!");

	DebuggerWindow::RegisterHardwareDetails(this);

}

DebuggerDisplayState::DebuggerDisplayState()
{
	for (int i = 0; i < 4; i++)
	{
		DissassembleSettingValid[i] = false;
	}
	for (int i = 0; i < 4; i++)
	{
		MemoryDumpSettingValid[i] = false;
	}
	StackSettingValid = false;

}

HardwareDetailsDisplay::HardwareDetailsDisplay()
{
	m_bDisplayZ80 = true;
	m_bDisplayZ80Registers = true;
	m_bDisplayZ80Inputs = true;

	m_bDisplayCRTC = false;
	m_bDisplayCRTCRegisters = true;
	m_bDisplayCRTCOutputs = true;

	m_bDisplayKeyboard = false;

	m_bDisplayAY = false;
	m_bDisplayAYRegisters = true;

	m_bDisplayPPI = true;

	m_bDisplayPrinter = true;

	m_bDisplayASIC = true;
	m_bDisplayASICPalette = true;
	m_bDisplayASICSprites = true;
	m_bDisplayASICDMA = true;
	m_bDisplayASICAnalogueInputs = true;

	m_bDisplayFDC = false;
	m_bDisplayFDI = false;
	m_bDisplayGA = true;
	m_bDisplayKCC = true;
	m_bDisplayAleste = true;

	m_bDisplayMonitor = true;
	m_bDisplayROMSelect = true;
}

void HardwareDetails::ClearItems()
{
	for (size_t i = 0; i < m_Items.GetCount(); i++)
	{
		HardwareDetailsItem *pItem = (HardwareDetailsItem *)m_Items.Item(i);
		delete pItem;
	}
	m_Items.Clear();
}

HardwareDetails::~HardwareDetails()
{
	// connect event
	this->Disconnect(this->GetId(), wxEVT_COMMAND_LIST_CACHE_HINT, wxListEventHandler(HardwareDetails::OnCacheHint));

	DebuggerWindow::DeRegisterHardwareDetails(this);

	ClearItems();
}

const wxChar *GAStringTable[32] =
{
	wxT("White"),
	wxT("White"),
	wxT("Sea Green"),
	wxT("Pastel Yellow"),
	wxT("Blue"),
	wxT("Purple"),
	wxT("Cyan"),
	wxT("Pink"),
	wxT("Purple"),
	wxT("Pastel Yellow"),
	wxT("Bright Yellow"),
	wxT("Bright White"),
	wxT("Bright Red"),
	wxT("Bright Magenta"),
	wxT("Orange"),
	wxT("Pastel Magenta"),
	wxT("Blue"),
	wxT("Sea Green"),
	wxT("Bright Green"),
	wxT("Bright Cyan"),
	wxT("Black"),
	wxT("Bright Blue"),
	wxT("Green"),
	wxT("Sky Blue"),
	wxT("Magenta"),
	wxT("Pastel Green"),
	wxT("Lime"),
	wxT("Pastel Cyan"),
	wxT("Red"),
	wxT("Mauve"),
	wxT("Yellow"),
	wxT("Pastel Blue")
};

const wxChar *InterlaceStringTable[4] =
{
	wxT("Non-interlace"),
	wxT("Interlace Sync Mode"),
	wxT("Non-interlace"),
	wxT("Interlace Sync & Video Mode")
};

const wxChar *PSGModeTable[4] =
{
	wxT("Inactive"),
	wxT("Read register"),
	wxT("Write register"),
	wxT("Select register")
};

const wxChar *CursorBlink[2] =
{
	wxT("16"),
	wxT("32")
};

const wxChar *Skew[4]=
{
	wxT("0"),
	wxT("1"),
	wxT("2"),
	wxT("Non display")
};

void HardwareDetails::PerformRefreshShown()
{

	Freeze();
	DeleteAllItems();
	ClearItems();

	if (DetailsDisplay.m_bDisplayMonitor)
	{
		AddBoolItem(wxT("Monitor Vsync Input"), CODE_MONITOR_VSYNC_STATE, 0, true);
		AddBoolItem(wxT("Monitor Hsync Input"), CODE_MONITOR_HSYNC_STATE, 0, true);
#if 0
		Add16BitItem(wxT("Monitor Horizontal Position"), CODE_MONITOR_HORIZONTAL_POSITION, 0);
		Add16BitItem(wxT("Monitor Vertical Position"), CODE_MONITOR_VERTICAL_POSITION, 0);
		AddBoolItem(wxT("Monitor Seen Hsync"), CODE_MONITOR_SEEN_HSYNC, 0, true);
		Add16BitItem(wxT("Monitor Count after Hsync"), CODE_MONITOR_COUNT_AFTER_HSYNC, 0);
		AddBoolItem(wxT("Monitor Seen Vsync"), CODE_MONITOR_SEEN_VSYNC, 0, true);
		Add16BitItem(wxT("Monitor Count after Vsync"), CODE_MONITOR_COUNT_AFTER_VSYNC, 0);
		Add16BitItem(wxT("HStart Adjustment"), CODE_MONITOR_HORZ_ADJUSTMENT, 0);
		Add16BitItem(wxT("VStart Adjustment"), CODE_MONITOR_VERT_ADJUSTMENT, 0);
#endif
	}

	Add16BitItem(wxT("Cycle Counter"), CODE_CYCLE_COUNTER, 0);

	if (DetailsDisplay.m_bDisplayZ80)
	{
		if (DetailsDisplay.m_bDisplayZ80Registers)
		{
			Add16BitItem(wxT("Z80 AF"), CODE_Z80_REG_AF, 0);
			Add8BitItem(wxT("Z80 A"), CODE_Z80_REG_A, 0);
			Add16BitItem(wxT("Z80 BC"), CODE_Z80_REG_BC, 0);
			Add8BitItem(wxT("Z80 B"), CODE_Z80_REG_B, 0);
			Add8BitItem(wxT("Z80 C"), CODE_Z80_REG_C, 0);
			Add16BitItem(wxT("Z80 DE"), CODE_Z80_REG_DE, 0);
			Add8BitItem(wxT("Z80 D"), CODE_Z80_REG_D, 0);
			Add8BitItem(wxT("Z80 E"), CODE_Z80_REG_E, 0);
			Add16BitItem(wxT("Z80 HL"), CODE_Z80_REG_HL, 0);
			Add8BitItem(wxT("Z80 H"), CODE_Z80_REG_H, 0);
			Add8BitItem(wxT("Z80 L"), CODE_Z80_REG_L, 0);
			Add16BitItem(wxT("Z80 PC"), CODE_Z80_REG_PC, 0);
			Add16BitItem(wxT("Z80 SP"), CODE_Z80_REG_SP, 0);
			Add16BitItem(wxT("Z80 AF'"), CODE_Z80_REG_AFALT, 0);
			Add16BitItem(wxT("Z80 BC'"), CODE_Z80_REG_BCALT, 0);
			Add16BitItem(wxT("Z80 DE'"), CODE_Z80_REG_DEALT, 0);
			Add16BitItem(wxT("Z80 HL'"), CODE_Z80_REG_HLALT, 0);
			Add16BitItem(wxT("Z80 IX"), CODE_Z80_REG_IX, 0);
			Add8BitItem(wxT("Z80 IX (High)"), CODE_Z80_REG_IXH, 0);
			Add8BitItem(wxT("Z80 IX (Low)"), CODE_Z80_REG_IXL, 0);
			Add16BitItem(wxT("Z80 IY"), CODE_Z80_REG_IY, 0);
			Add8BitItem(wxT("Z80 IY (High)"), CODE_Z80_REG_IYH, 0);
			Add8BitItem(wxT("Z80 IY (Low)"), CODE_Z80_REG_IYL, 0);
			Add8BitItem(wxT("Z80 R"), CODE_Z80_REG_R, 0);
			Add8BitItem(wxT("Z80 I"), CODE_Z80_REG_I, 0);

			AddBoolItem(wxT("Z80 IFF1"), CODE_Z80_IFF1, 0, true);
			AddBoolItem(wxT("Z80 IFF2"), CODE_Z80_IFF2, 0, true);

			/* bit order and having z80 flag name */
			AddBoolItem(wxT("Z80 Sign Flag (S)"), CODE_CPU_FLAGS, CPU_FLAG_SIGN, true);
			AddBoolItem(wxT("Z80 Zero Flag (Z)"), CODE_CPU_FLAGS, CPU_FLAG_ZERO, true);
			AddBoolItem(wxT("Z80 Flags Bit 5"), CODE_CPU_FLAGS, CPU_FLAG_BIT5, true);
			AddBoolItem(wxT("Z80 Half-Carry Flag (H)"), CODE_CPU_FLAGS, CPU_FLAG_HALF_CARRY, true);
			AddBoolItem(wxT("Z80 Flags Bit 3"), CODE_CPU_FLAGS, CPU_FLAG_BIT3, true);
			AddBoolItem(wxT("Z80 Parity/Overflow Flag (P/V)"), CODE_CPU_FLAGS, CPU_FLAG_ADDSUBTRACT, true);
			AddBoolItem(wxT("Z80 Subtract Flag (N)"), CODE_CPU_FLAGS, CPU_FLAG_ADDSUBTRACT, true);
			AddBoolItem(wxT("Z80 Carry Flag (C)"), CODE_CPU_FLAGS, CPU_FLAG_PARITYOVERFLOW, true);

			AddBoolItem(wxT("Z80 Halted"), CODE_Z80_OUTPUTS, CPU_OUTPUT_HALT, true);
			AddBoolItem(wxT("Z80 Interrupt Acknowledge"), CODE_Z80_OUTPUTS, 0, CPU_OUTPUT_IORQ | CPU_OUTPUT_RD | CPU_OUTPUT_WR | CPU_OUTPUT_M1 | CPU_OUTPUT_MREQ, CPU_OUTPUT_IORQ | CPU_OUTPUT_M1);
			AddBoolItem(wxT("Z80 I/O Read"), CODE_Z80_OUTPUTS, 0, CPU_OUTPUT_IORQ | CPU_OUTPUT_RD | CPU_OUTPUT_WR | CPU_OUTPUT_M1 | CPU_OUTPUT_MREQ, CPU_OUTPUT_IORQ | CPU_OUTPUT_RD);
			AddBoolItem(wxT("Z80 I/O Write "), CODE_Z80_OUTPUTS, 0, CPU_OUTPUT_IORQ | CPU_OUTPUT_RD | CPU_OUTPUT_WR | CPU_OUTPUT_M1 | CPU_OUTPUT_MREQ, CPU_OUTPUT_IORQ | CPU_OUTPUT_WR);
			AddBoolItem(wxT("Z80 Opcode fetch"), CODE_Z80_OUTPUTS, 0, CPU_OUTPUT_IORQ | CPU_OUTPUT_RD | CPU_OUTPUT_WR | CPU_OUTPUT_M1 | CPU_OUTPUT_MREQ, CPU_OUTPUT_M1 | CPU_OUTPUT_MREQ | CPU_OUTPUT_RD);
			AddBoolItem(wxT("Z80 Read Memory"), CODE_Z80_OUTPUTS, 0, CPU_OUTPUT_IORQ | CPU_OUTPUT_RD | CPU_OUTPUT_WR | CPU_OUTPUT_M1 | CPU_OUTPUT_MREQ, CPU_OUTPUT_MREQ | CPU_OUTPUT_RD);
			AddBoolItem(wxT("Z80 Write Memory"), CODE_Z80_OUTPUTS, 0, CPU_OUTPUT_IORQ | CPU_OUTPUT_RD | CPU_OUTPUT_WR | CPU_OUTPUT_M1 | CPU_OUTPUT_MREQ, CPU_OUTPUT_MREQ | CPU_OUTPUT_WR);
		}

		Add8BitItem(wxT("Z80 Interrupt Mode"), CODE_Z80_REG_IM, 0);
		Add16BitItem(wxT("Z80 Interrupt Table Address"), CODE_Z80_INT_TABLE_ADDR, 0);
		Add16BitItem(wxT("Z80 MemPtr"), CODE_Z80_REG_MEMPTR, 0);
		Add16BitItem(wxT("I/O Port"), CODE_Z80_IO_PORT, 0);
		Add8BitItem(wxT("I/O Data"), CODE_Z80_IO_DATA, 0);
		Add8BitItem(wxT("Databus"), CODE_Z80_DATABUS, 0);

		if (DetailsDisplay.m_bDisplayZ80Inputs)
		{
			Add8BitItem(wxT("Z80 Interrupt Vector"), CODE_Z80_INTERRUPT_VECTOR, 0);
			AddBoolItem(wxT("Z80 IRQ Input/Request"), CODE_Z80_INTERRUPT_REQUEST, 0, true);
			AddBoolItem(wxT("Z80 NMI Input"), CODE_Z80_NMI_INPUT, 0, true);
			AddBoolItem(wxT("Z80 NMI Request"), CODE_Z80_NMI_REQUEST, 0, true);
		}

	}


	if (DetailsDisplay.m_bDisplayCRTC)
	{
		if (DetailsDisplay.m_bDisplayCRTCRegisters)
		{
			for (int i = 0; i < 18; i++)
			{
				wxString sReg;
				sReg.Printf(wxT("CRTC R%d"), i);
				Add8BitItem(sReg, CODE_CRTC_REGISTER_DATA, i);
			}
			Add8BitItem(wxT("CRTC R31 (Type 1)"), CODE_CRTC_REGISTER_DATA, 31);
		}

		AddMasked8BitItem(wxT("CRTC Programmed Horizontal Sync Width"), CODE_CRTC_REGISTER_DATA, 3, 0x0f);
		// TODO: Shift
		AddMasked8BitItem(wxT("CRTC Programmed Vertical Sync Width"), CODE_CRTC_REGISTER_DATA, 3, 0x0f0);

		Add8BitItem(wxT("CRTC Actual Horizontal Sync Width"), CODE_CRTC_ACTUAL_HORIZONTAL_SYNC_WIDTH, 0);
		Add8BitItem(wxT("CRTC Actual Vertical Sync Width"), CODE_CRTC_ACTUAL_VERTICAL_SYNC_WIDTH, 0);

		Add8BitItem(wxT("CRTC Status Register (Type 1)"), CODE_CRTC_STATUS_REGISTER, 0);
		AddBoolItem(wxT("CRTC Status Register (Type 1): VBLANK"), CODE_CRTC_STATUS_REGISTER, 0, (1 << 5), (1 << 5));
		AddBoolItem(wxT("CRTC Status Register (Type 1): LPEN register full"), CODE_CRTC_STATUS_REGISTER, 0, (1 << 6), (1 << 6));

		Add8BitItem(wxT("CRTC Status Register 1 (Type 3)"), CODE_CRTC3_STATUS_REGISTER1, 0);
		Add8BitItem(wxT("CRTC Status Register 2 (Type 3)"), CODE_CRTC3_STATUS_REGISTER2, 0);
		Add8BitItem(wxT("CRTC Status Register 1 (Type 4)"), CODE_CRTC4_STATUS_REGISTER1, 0);
		Add8BitItem(wxT("CRTC Status Register 2 (Type 4)"), CODE_CRTC4_STATUS_REGISTER2, 0);

		/* helpers */
		Add8BitItem(wxT("CRTC Displayed Width"), CODE_CRTC_REGISTER_DATA, 1);
		Add8BitItem(wxT("CRTC Displayed Height"), CODE_CRTC_REGISTER_DATA, 6);

		Add8BitItem(wxT("CRTC Frame Width Chars"), CODE_CRTC_FRAME_WIDTH, 0);
		Add8BitItem(wxT("CRTC Frame Height Chars"), CODE_CRTC_FRAME_HEIGHT, 0);
		Add8BitItem(wxT("CRTC Scanlines per char"), CODE_CRTC_CHAR_HEIGHT, 0);


		AddBoolItem(wxT("CRTC Cursor Blinking"), CODE_CRTC_REGISTER_DATA, 10, (1 << 6), (1 << 6));
		AddStringIDItem(wxT("CRTC Cursor Blink Period"), CODE_CRTC_REGISTER_DATA, 10, 0x01, 5, CursorBlink, sizeof(CursorBlink) / sizeof(CursorBlink[0]));

		AddBoolItem(wxT("CRTC Cursor Enabled"), CODE_CRTC_REGISTER_DATA, 10, (1 << 6) | (1 << 5), (1 << 5));

		AddMasked8BitItem(wxT("CRTC Cursor Start Raster"), CODE_CRTC_REGISTER_DATA, 10, 0x01f);
		AddMasked8BitItem(wxT("CRTC Cursor End Raster"), CODE_CRTC_REGISTER_DATA, 11, 0x01f);

		Add8BitItem(wxT("Interlace frame odd/even"), CODE_CRTC_INTERLACE_FRAME, 0);

		AddStringIDItem(wxT("CRTC CUDISP Skew"), CODE_CRTC_REGISTER_DATA, 8, 0x03, 6, Skew, sizeof(Skew) / sizeof(Skew[0]));
		AddStringIDItem(wxT("CRTC DISPTMG Skew"), CODE_CRTC_REGISTER_DATA, 8, 0x03, 4, Skew, sizeof(Skew) / sizeof(Skew[0]));

#if 0
		unsigned short Addr;
		Addr = (CRTC_GetRegisterData(12) & 0x030) << (8 + 2);
		Addr |= (((CRTC_GetRegisterData(12) & 0x03) << 8) | (CRTC_GetRegisterData(13) & 0x0ff)) << 1;

		Add16BitNumber(wxT("Screen base"), Addr);
#endif
		AddStringIDItem(wxT("CRTC Raster Scan Mode"), CODE_CRTC_REGISTER_DATA, 8, 0x03, 0, InterlaceStringTable, sizeof(InterlaceStringTable) / sizeof(InterlaceStringTable[0]));

		Add8BitItem(wxT("CRTC Selected Register"), CODE_CRTC_SELECTED_REGISTER, 0);

		Add8BitItem(wxT("CRTC HCC"), CODE_CRTC_HCC, 0);
		Add8BitItem(wxT("CRTC VCC"), CODE_CRTC_VCC, 0);
		Add8BitItem(wxT("CRTC RA"), CODE_CRTC_RA, 0);
		Add16BitItem(wxT("Chars After Hsync Start"), CODE_CRTC_CHARS_AFTER_HSYNC_START, 0);
		Add16BitItem(wxT("Lines After Frame Start"), CODE_CRTC_LINES_AFTER_FRAME_START, 0);
		Add16BitItem(wxT("Lines After Vsync Start"), CODE_CRTC_LINES_AFTER_VSYNC_START, 0);

		if (DetailsDisplay.m_bDisplayCRTCOutputs)
		{
			Add16BitItem(wxT("CRTC MA Output"), CODE_CRTC_MA_OUTPUT, 0);
			Add8BitItem(wxT("CRTC RA Output"), CODE_CRTC_RA_OUTPUT, 0);
			AddBoolItem(wxT("CRTC VSYNC Output"), CODE_CRTC_VSYNC_OUTPUT, 0, true);
			AddBoolItem(wxT("CRTC HSYNC Output"), CODE_CRTC_HSYNC_OUTPUT, 0, true);
			AddBoolItem(wxT("CRTC DISPTMG Output"), CODE_CRTC_DISPTMG_OUTPUT, 0, true);
			AddBoolItem(wxT("CRTC CUDISP Output"), CODE_CRTC_CUDISP_OUTPUT, 0, true);
		}
	}

	Add16BitItem(wxT("VRAM Addr"), CODE_VRAM_ADDR, 0);

	if (DetailsDisplay.m_bDisplayPPI)
	{
		AddBoolItem(wxT("PPI Port A Input"), CODE_PPI_CONTROL, 0, (1 << 4), (1 << 4));
		AddBoolItem(wxT("PPI Port B Input"), CODE_PPI_CONTROL, 0, (1 << 1), (1 << 1));
		AddBoolItem(wxT("PPI Port C (7..4) Input"), CODE_PPI_CONTROL, 0, (1 << 3), (1 << 3));
		AddBoolItem(wxT("PPI Port C (3..0) Input"), CODE_PPI_CONTROL, 0, (1 << 0), (1 << 0));
		AddMasked8BitItem(wxT("PPI Group A Mode"), CODE_PPI_CONTROL, 0, 3 << 5);
		AddMasked8BitItem(wxT("PPI Group B Mode"), CODE_PPI_CONTROL, 0, (1 << 2));
		for (int i=0; i<3; i++)
		{
			wxString sName;
			sName.sprintf(wxT("PPI Port %c Input"), 'A' + i);
			Add8BitItem(sName, CODE_PPI_PORT_INPUT, i);
	}
		for (int i = 0; i < 3; i++)
		{
			wxString sName;
			sName.sprintf(wxT("PPI Port %c Output"), 'A' + i);
			Add8BitItem(sName, CODE_PPI_PORT_OUTPUT, i);
		}
#if 0
		/* !!!!!!!!!! TODO: Multiple read of PPI inputs can stop the cassette from working or
		stops things being seen!!!!! We need to cache the values */
		AddMasked8BitItem(wxT("PPI Cassette read data"), CODE_PPI_PORT_INPUT, 1, 1 << 7);
		AddMasked8BitItem(wxT("PPI Vsync"), CODE_PPI_PORT_INPUT, 1, 1 << 0);
		AddMasked8BitItem(wxT("PPI Printer Busy"), CODE_PPI_PORT_INPUT, 1, 1 << 6);
		AddMasked8BitItem(wxT("PPI /EXP"), CODE_PPI_PORT_INPUT, 1, 1 << 5);

		AddMasked8BitItem(wxT("PPI Cassette write"), CODE_PPI_PORT_OUTPUT, 2, 1 << 5);
		AddMasked8BitItem(wxT("PPI Cassette motor"), CODE_PPI_PORT_OUTPUT, 2, 1 << 4);
#endif
	}


	if (DetailsDisplay.m_bDisplayASIC)
	{
		if (CPC_GetHardware() != CPC_HW_CPCPLUS)
		{
#if 0
			AddItem(wxT(wxT"ASIC Information not available for this computer type"), wxT(""));
#endif
		}
		else
		{
			Add8BitItem(wxT("ASIC PRI"), CODE_ASIC_PRI, 0);
			Add8BitItem(wxT("ASIC PRI VCC"), CODE_ASIC_PRI_VCC, 0);
			Add8BitItem(wxT("ASIC PRI RCC"), CODE_ASIC_PRI_RCC, 0);
			Add16BitItem(wxT("ASIC Current Comparision Line for PRI"), CODE_ASIC_PRI_LINE, 0);
			AddBoolItem(wxT("ASIC Raster interrupt line set"), CODE_ASIC_PRI, 0, true);

			Add8BitItem(wxT("ASIC SPLT"), CODE_ASIC_SPLT, 0);
			Add8BitItem(wxT("ASIC SPLT VCC"), CODE_ASIC_SPLT_VCC, 0);
			Add8BitItem(wxT("ASIC SPLT RCC"), CODE_ASIC_SPLT_RCC, 0);
			Add8BitItem(wxT("ASIC Current Comparision Line SPLT"), CODE_ASIC_SPLT_LINE, 0);
			AddBoolItem(wxT("ASIC Screen split active"), CODE_ASIC_SPLT, 0, true);

			Add16BitItem(wxT("ASIC SSA"), CODE_ASIC_SSA,0);

			Add8BitItem(wxT("ASIC SSCR"), CODE_ASIC_SSCR,0);

			// TODO: Needs shift!
			AddMasked8BitItem(wxT("ASIC SSCR Vertical"), CODE_ASIC_SSCR, 0,0x070);  //(SSCR>>4) & 0x07);
			AddMasked8BitItem(wxT("ASIC SSCR Horizontal"), CODE_ASIC_SSCR, 0,0x0f);
			AddBoolItem(wxT("ASIC SSCR Extend Border"), CODE_ASIC_SSCR,0,(1<<7), (1<<7));

			Add8BitItem(wxT("ASIC IVR"), CODE_ASIC_IVR,0);
			AddMasked8BitItem(wxT("ASIC IVR Vector"), CODE_ASIC_IVR,0,0x0f8);
			AddBoolItem(wxT("ASIC IVR Autoclear DMA Interrupt"), CODE_ASIC_IVR, 0,(1<<0), 0);

			AddBoolItem(wxT("ASIC Unlocked"), CODE_ASIC_UNLOCKED, 0, true);

			Add8BitItem(wxT("ASIC RMR2"), CODE_ASIC_RMR2,0);

			AddMasked8BitItem(wxT("Lower Cartridge Page"), CODE_ASIC_RMR2, 0,0x07);

#if 0
			int nBank = 0;
			switch ((RMR2>>3)&0x03)
			{
			case 0:
			{
				nBank = 0;
			}
			break;

			case 1:
			{
				nBank = 1;
		}
			break;


			case 2:
			{

				nBank = 2;
			}
			break;


			case 3:
			{

				nBank = 0;
			}
			break;
	}
#endif
			AddBoolItem(wxT("ASIC RAM Visible (&4000-&7FFF)"), CODE_ASIC_RMR2, 0, (3 << 3), (3 << 3));

			//            Add16BitNumber(wxT("Low rom bank start (end+&3FFF)"), (nBank<<14));

			if (DetailsDisplay.m_bDisplayASICPalette)
			{

				for (int i = 0; i < 15; i++)
				{
					wxString sLabel;
					sLabel.Printf(wxT("ASIC Palette %d RGB"), i);

					Add16BitItem(sLabel, CODE_ASIC_PALETTE_RGB, i);
				}
				Add16BitItem(wxT("ASIC Border RGB"), CODE_ASIC_PALETTE_RGB, 16);
				for (int i = 17; i < 32; i++)
				{
					wxString sLabel;
					sLabel.Printf(wxT("ASIC Sprite Palette %d RGB"), i - 16);
					Add16BitItem(sLabel, CODE_ASIC_PALETTE_RGB, i);
				}
			}

			if (DetailsDisplay.m_bDisplayASICAnalogueInputs)
			{
				for (int i = 0; i < 8; i++)
				{
					wxString sLabel;
					sLabel.Printf(wxT("ASIC Analogue Input %d "), i);
					Add8BitItem(sLabel, CODE_ASIC_ANALOGUE_INPUT, i);
				}
			}

			if (DetailsDisplay.m_bDisplayASICSprites)
			{

				for (int i=0; i<16; i++)
				{
					wxString sLabel;
					sLabel.Printf(wxT("ASIC Sprite %d X"), i);
					Add16BitItem(sLabel, CODE_ASIC_SPRITE_X, i);
					sLabel.Printf(wxT("ASIC Sprite %d Y"), i);
					Add16BitItem(sLabel, CODE_ASIC_SPRITE_Y, i);

					sLabel.Printf(wxT("ASIC Sprite %d Magnification"), i);
					Add8BitItem(sLabel, CODE_ASIC_SPRITE_MAG,i);
#if 0
					bool bDisplayed = true;
					if (((Mag& ((1<<1)|(1<<0)))==0) || ((Mag&((1<<3)|(1<<2)))==0))
					{
						bDisplayed = false;
					}

					wxString sValue;

					sLabel.Printf(wxT("ASIC Sprite %d X Magnification"), i);
					if (bDisplayed)
					{
						int XMagnification = 1<<(((Mag>>2)&0x03)-1);
						sValue.Printf(wxT("x%d"),XMagnification);

					}
					else
					{
						sValue = wxT("-");
					}
					AddItem(sLabel, sValue);

					sLabel.Printf(wxT("ASIC Sprite %d Y Magnification"), i);
					if (bDisplayed)
					{
						// 1,2,3
						// x2, x4, x8
						int YMagnification = 1 << ((Mag & 0x03) - 1);
						sValue.Printf(wxT("x%d"), YMagnification);

					}
					else
					{
						sValue = wxT("-");
					}
					AddItem(sLabel, sValue);
#endif
				}

			}

			if (DetailsDisplay.m_bDisplayASICDMA)
			{

				for (int i = 0; i < 3; i++)
				{
					wxString sLabel;
					sLabel.Printf(wxT("ASIC DMA %d Addr"), i);
					Add16BitItem(sLabel, CODE_ASIC_DMA_ADDR, i);
					sLabel.Printf(wxT("ASIC DMA %d Prescale"), i);
					Add16BitItem(sLabel, CODE_ASIC_DMA_PRESCALE, i);
					//             sLabel.Printf(wxT("ASIC DMA %d Prescale Counter"),i);
					//           Add16BitNumber(sLabel, ASIC_DMA_GetChannelPrescaleCount(i));
				}

				Add8BitItem(wxT("ASIC DMA control/status "), CODE_ASIC_DCSR, 0);

				AddBoolItem(wxT("ASIC DMA Channel 0 enable"), CODE_ASIC_DCSR, 0, (1 << 0), (1 << 0));
				AddBoolItem(wxT("ASIC DMA Channel 1 enable"), CODE_ASIC_DCSR, 0, (1 << 1), (1 << 1));
				AddBoolItem(wxT("ASIC DMA Channel 2 enable"), CODE_ASIC_DCSR, 0, (1 << 2), (1 << 2));

				AddBoolItem(wxT("ASIC DMA Channel 0 interrupt request"), CODE_ASIC_DCSR, 0, (1 << 6), (1 << 6));
				AddBoolItem(wxT("ASIC DMA Channel 1 interrupt request"), CODE_ASIC_DCSR, 0, (1 << 5), (1 << 5));
				AddBoolItem(wxT("ASIC DMA Channel 2 interrupt request"), CODE_ASIC_DCSR, 0, (1 << 4), (1 << 4));

				AddBoolItem(wxT("ASIC Raster interrupt request"), CODE_ASIC_DCSR, 0, (1 << 7), (1 << 7));
			}
}

	}


	if (DetailsDisplay.m_bDisplayAY)
	{
		if (DetailsDisplay.m_bDisplayAYRegisters)
		{
			for (int i = 0; i < 16; i++)
			{
				wxString sReg;
				sReg.Printf(wxT("PSG R%d"), i);
				Add8BitItem(sReg, CODE_PSG_REGISTER_DATA, i);
			}
		}

		AddBoolItem(wxT("PSG Hardware Envelope Enabled (for Channel A)"), CODE_PSG_REGISTER_DATA, 8, (1 << 4), (1 << 4));
		AddMasked8BitItem(wxT("PSG Channel A Volume"), CODE_PSG_REGISTER_DATA, 8, 0x0f);
		AddBoolItem(wxT("PSG Hardware Envelope Enabled (for Channel B)"), CODE_PSG_REGISTER_DATA, 9, (1 << 4), (1 << 4));
		AddMasked8BitItem(wxT("PSG Channel B Volume"), CODE_PSG_REGISTER_DATA, 9, 0x0f);
		AddBoolItem(wxT("PSG Hardware Envelope Enabled (for Channel C)"), CODE_PSG_REGISTER_DATA, 10, (1 << 4), (1 << 4));
		AddMasked8BitItem(wxT("PSG Channel C Volume"), CODE_PSG_REGISTER_DATA, 10, 0x0f);


		Add8BitItem(wxT("PSG Selected Register"), CODE_PSG_SELECTED_REGISTER, 0);

		AddBoolItem(wxT("PSG Channel A Tone Enabled"), CODE_PSG_REGISTER_DATA, 7, (1 << 0), 0);
		AddBoolItem(wxT("PSG Channel B Tone Enabled"), CODE_PSG_REGISTER_DATA, 7, (1 << 1), 0);
		AddBoolItem(wxT("PSG Channel C Tone Enabled"), CODE_PSG_REGISTER_DATA, 7, (1 << 2), 0);
		AddBoolItem(wxT("PSG Channel A Noise Enabled"), CODE_PSG_REGISTER_DATA, 7, (1 << 3), 0);
		AddBoolItem(wxT("PSG Channel B Noise Enabled"), CODE_PSG_REGISTER_DATA, 7, (1 << 4), 0);
		AddBoolItem(wxT("PSG Channel C Noise Enabled"), CODE_PSG_REGISTER_DATA, 7, (1 << 5), 0);
		AddBoolItem(wxT("PSG I/O Port A Input"), CODE_PSG_REGISTER_DATA, 7, (1 << 6), 0);
		AddBoolItem(wxT("PSG I/O Port B Input"), CODE_PSG_REGISTER_DATA, 7, (1 << 7), 0);


		Add8BitItem(wxT("PSG Port A Input"), CODE_PSG_IO_INPUT, 0);
		Add8BitItem(wxT("PSG Port B Input"), CODE_PSG_IO_INPUT, 1);
		Add8BitItem(wxT("PSG Port A Output"), CODE_PSG_IO_OUTPUT, 0);
		Add8BitItem(wxT("PSG Port B Output"), CODE_PSG_IO_OUTPUT, 1);


		AddStringIDItem(wxT("PSG Mode"), CODE_PSG_MODE, 0, PSGModeTable, sizeof(PSGModeTable) / sizeof(PSGModeTable[0]));
		AddBoolItem(wxT("PSG BDIR Input"), CODE_PSG_BDIR, 0, true);
		AddBoolItem(wxT("PSG BC1 Input"), CODE_PSG_BC1, 0, true);
		AddBoolItem(wxT("PSG BC2 Input"), CODE_PSG_BC2, 0, true);
	}

	if (DetailsDisplay.m_bDisplayPrinter)
	{
		AddBoolItem(wxT("Printer busy input"), CODE_PRINTER_BUSY, 0, true);
		AddBoolItem(wxT("Printer strobe output "), CODE_PRINTER_STROBE, 0, true);
		Add8BitItem(wxT("Printer 8-bit data"), CODE_PRINTER_DATA, 0);
	}


	if (DetailsDisplay.m_bDisplayKCC)
	{
		if (CPC_GetHardware() == CPC_HW_KCCOMPACT)
		{
			Add8BitItem(wxT("KCC Selected Pen (FN)"), CODE_KCC_FN, 0);

			Add8BitItem(wxT("KCC Multi-Function"), CODE_KCC_MF, 0);
			AddMasked8BitItem(wxT("KCC Multi-Function Mode"), CODE_KCC_MF, 0, 0x03);
			AddBoolItem(wxT("KCC Multi-Function Lower ROM Enabled"), CODE_KCC_MF, 0, (1 << 2), 0);
			AddBoolItem(wxT("KCC Multi-Function Upper ROM Enabled"), CODE_KCC_MF, 0, (1 << 3), 0);
			AddBoolItem(wxT("KCC Multi-Function Interrupt Reset"), CODE_KCC_MF, 0, (1 << 4), 0);


			for (int i = 0; i < 16; i++)
			{
				wxString sLabel;
				sLabel.Printf(wxT("KCC Pen %d Colour"), i);
				Add8BitItem(sLabel, CODE_KCC_COLOUR, i);
			}
			Add8BitItem(wxT("KCC Border Colour"), CODE_KCC_COLOUR, 16);

			for (int i = 0; i < 3; i++)
			{
				wxString sLabel;
				sLabel.Printf(wxT("z8536 Output %d"), i);
				Add8BitItem(sLabel, CODE_Z8536_OUTPUTS, i);
			}


			for (int i = 0; i < 3; i++)
			{
				wxString sLabel;
				sLabel.Printf(wxT("z8536 Input %d"), i);
				Add8BitItem(sLabel, CODE_Z8536_INPUTS, i);
			}

			for (int i = 0; i < 64; i++)
			{
				wxString sLabel;
				sLabel.Printf(wxT("z8536 Registers %d"), i);
				Add8BitItem(sLabel, CODE_Z8536_REGISTERS, i);
			}

		}
	}

	if (DetailsDisplay.m_bDisplayAleste)
	{
		if (CPC_GetHardware() == CPC_HW_ALESTE)
		{

			Add8BitItem(wxT("Aleste Selected Pen (FN)"), CODE_ALESTE_PEN, 0);

			Add8BitItem(wxT("Aleste Multi-Function"), CODE_ALESTE_MF, 0);
			AddMasked8BitItem(wxT("Aleste Multi-Function Mode"), CODE_ALESTE_MF, 0, 0x03);
			AddBoolItem(wxT("Aleste Multi-Function Lower ROM Enabled"), CODE_ALESTE_MF, 0, (1 << 2), 0);
			AddBoolItem(wxT("Aleste Multi-Function Upper ROM Enabled"), CODE_ALESTE_MF, 0, (1 << 3), 0);
			AddBoolItem(wxT("Aleste Multi-Function RUS LED"), CODE_ALESTE_MF, 0, (1 << 4), true);
			AddBoolItem(wxT("Aleste Multi-Function CAPS LED"), CODE_ALESTE_MF, 0, (1 << 5), true);


			for (int i = 0; i < 16; i++)
			{
				wxString sLabel;
				sLabel.Printf(wxT("Aleste Pen %d Colour Value"), i);
				Add8BitItem(sLabel, CODE_ALESTE_COLOUR, i);
			}
			Add8BitItem(wxT("Aleste Border Colour Value"), CODE_ALESTE_COLOUR, 16);

			Add8BitItem(wxT("Aleste Mapper &0000-&3fff"), CODE_ALESTE_MAPPER, 0);
			Add8BitItem(wxT("Aleste Mapper &4000-&7fff"), CODE_ALESTE_MAPPER, 1);
			Add8BitItem(wxT("Aleste Mapper &8000-&bfff"), CODE_ALESTE_MAPPER, 2);
			Add8BitItem(wxT("Aleste Mapper &c000-&ffff"), CODE_ALESTE_MAPPER, 3);

			Add8BitItem(wxT("Aleste Extport"), CODE_ALESTE_EXTPORT, 0);
			AddMasked8BitItem(wxT("Aleste Extport VRAM Bank"), CODE_ALESTE_EXTPORT, 0, 0x01);

			AddMasked8BitItem(wxT("Aleste Extport HIGHTX"), CODE_ALESTE_EXTPORT, 0, (1 << 0));
			AddMasked8BitItem(wxT("Aleste Extport HIGHTY"), CODE_ALESTE_EXTPORT, 0, (1 << 1));
			AddMasked8BitItem(wxT("Aleste Extport MAPMOD"), CODE_ALESTE_EXTPORT, 0, (1 << 2));
			AddMasked8BitItem(wxT("Aleste Extport BLAKS"), CODE_ALESTE_EXTPORT, 0, (1 << 3));
			AddMasked8BitItem(wxT("Aleste Extport CS53"), CODE_ALESTE_EXTPORT, 0, (1 << 4));
			AddMasked8BitItem(wxT("Aleste Extport CSAY"), CODE_ALESTE_EXTPORT, 0, (1 << 5));

			AddBoolItem(wxT("Aleste 8253 selected"), CODE_ALESTE_EXTPORT, 0, (1 << 4), false);
			AddBoolItem(wxT("Aleste AY selected"), CODE_ALESTE_EXTPORT, 0, (1 << 5), false);
			AddBoolItem(wxT("Aleste MC146818 RTC selected"), CODE_ALESTE_EXTPORT, 0, (1 << 5), true);
		}
	}

	if (DetailsDisplay.m_bDisplayGA)
	{
		if (
			(CPC_GetHardware() != CPC_HW_CPCPLUS) &&
			(CPC_GetHardware() != CPC_HW_CPC)
			)
		{
			//   AddItem(wxT("Gate-Array information not available for this computer type"), wxT(""));
		}
		else
		{
			if (CPC_GetHardware() == CPC_HW_CPC)
			{

				Add8BitItem(wxT("GA PPR"), CODE_CPC_GA_SELECTED_PEN, 0);

				AddBoolItem(wxT("GA Border selected"), CODE_CPC_GA_SELECTED_PEN, 0, (1 << 4), (1 << 4));
				AddMasked8BitItem(wxT("GA selected Pen"), CODE_CPC_GA_SELECTED_PEN, 0, 0x0f);

				for (int i = 0; i < 16; i++)
				{
					wxString sLabel;
					sLabel.Printf(wxT("GA Pen %d Colour"), i);
					Add8BitItem(sLabel, CODE_CPC_GA_PALETTE_COLOUR, i);

					sLabel.Printf(wxT("GA Pen %d Colour Name"), i);
					AddStringIDItem(sLabel, CODE_CPC_GA_PALETTE_COLOUR, i, GAStringTable, sizeof(GAStringTable) / sizeof(GAStringTable[0]));
				}

				Add8BitItem(wxT("GA Border Colour"), CODE_CPC_GA_PALETTE_COLOUR, 16);
				AddStringIDItem(wxT("GA Border Colour Name"), CODE_CPC_GA_PALETTE_COLOUR, 16, GAStringTable, sizeof(GAStringTable) / sizeof(GAStringTable[0]));

				Add8BitItem(wxT("GA MRER"), CODE_CPC_GA_MRER, 0);
				AddMasked8BitItem(wxT("GA MRER Mode"), CODE_CPC_GA_MRER, 0, 0x03);
				AddBoolItem(wxT("GA MRER Lower ROM Enabled"), CODE_CPC_GA_MRER, 0, (1 << 2), 0);
				AddBoolItem(wxT("GA MRER Upper ROM Enabled"), CODE_CPC_GA_MRER, 0, (1 << 3), 0);
				AddBoolItem(wxT("GA MRER R52 Reset"), CODE_CPC_GA_MRER, 0, (1 << 4), (1 << 4));

				Add8BitItem(wxT("GA Interrupt Line Count"), CODE_CPC_GA_INTERRUPT_LINE_COUNT, 0);
				AddBoolItem(wxT("GA HBlank Active"), CODE_CPC_GA_HBLANK_ACTIVE, 0, true);
				Add8BitItem(wxT("GA HBlank Count"), CODE_CPC_GA_HBLANK_COUNT, 0);
				AddBoolItem(wxT("GA VBlank Active"), CODE_CPC_GA_VBLANK_ACTIVE, 0, true);
				Add8BitItem(wxT("GA VBlank Count"), CODE_CPC_GA_VBLANK_COUNT, 0);

				AddBoolItem(wxT("GA Interrupt Request"), CODE_CPC_GA_INTERRUPT_OUTPUT, 0, true);
			}
			else
			{
				Add8BitItem(wxT("ASIC PPR"), CODE_ASIC_GA_SELECTED_PEN, 0);

				AddBoolItem(wxT("ASIC Border selected"), CODE_ASIC_GA_SELECTED_PEN, 0, (1 << 4), (1 << 4));
				AddMasked8BitItem(wxT("ASIC selected Pen"), CODE_ASIC_GA_SELECTED_PEN, 0, 0x0f);

				Add8BitItem(wxT("ASIC GA MRER"), CODE_ASIC_GA_MRER, 0);
				AddMasked8BitItem(wxT("ASIC GA MRER Mode"), CODE_ASIC_GA_MRER, 0, 0x03);
				AddBoolItem(wxT("ASIC GA MRER Lower ROM Enabled"), CODE_ASIC_GA_MRER, 0, (1 << 2), (1 << 2));
				AddBoolItem(wxT("ASIC GA MRER Upper ROM Enabled"), CODE_ASIC_GA_MRER, 0, (1 << 3), (1 << 3));
				AddBoolItem(wxT("ASIC GA MRER R52 Reset"), CODE_ASIC_GA_MRER, 0, (1 << 4), (1 << 4));

				Add8BitItem(wxT("ASIC GA Interrupt Line Count"), CODE_ASIC_GA_INTERRUPT_LINE_COUNT, 0);
				AddBoolItem(wxT("ASIC Interrupt Request"), CODE_ASIC_INTERRUPT_OUTPUT, 0, true);
				AddBoolItem(wxT("ASIC GA HBlank Active"), CODE_ASIC_GA_HBLANK_ACTIVE, 0, true);
				Add8BitItem(wxT("ASIC GA HBlank Count"), CODE_ASIC_GA_HBLANK_COUNT, 0);
				AddBoolItem(wxT("ASIC GA VBlank Active"), CODE_ASIC_GA_VBLANK_ACTIVE, 0, true);
				Add8BitItem(wxT("ASIC GA VBlank Count"), CODE_ASIC_GA_VBLANK_COUNT, 0);


			}
		}
	}
	//  {
	//     Add8BitNumber(wxT("PAL Config"), PAL_GetRamConfigurationetMultiConfiguration());
	//}

	if (DetailsDisplay.m_bDisplayROMSelect)
	{
		if (CPC_GetHardware() == CPC_HW_CPCPLUS)
		{
			Add8BitItem(wxT("Upper Cartridge Page"), CODE_ASIC_SELECTED_CARTRIDGE_PAGE, 0);

			//            Add32BitNumber(wxT("ROM Cartridge ROM Address"), (CartPage<<14));
		}
	}
	if (DetailsDisplay.m_bDisplayFDC)
	{
		Add8BitItem(wxT("FDC MSR"), CODE_FDC_MSR, 0);
		AddBoolItem(wxT("FDC MSR (D0B) FDD 0 Busy"), CODE_FDC_MSR, 0, (1 << 0), (1 << 0));
		AddBoolItem(wxT("FDC MSR (D1B) FDD 1 Busy"), CODE_FDC_MSR, 0, (1 << 1), (1 << 1));
		AddBoolItem(wxT("FDC MSR (D2B) FDD 2 Busy"), CODE_FDC_MSR, 0, (1 << 2), (1 << 2));
		AddBoolItem(wxT("FDC MSR (D3B) FDD 3 Busy"), CODE_FDC_MSR, 0, (1 << 3), (1 << 3));
		AddBoolItem(wxT("FDC MSR (CB) FDC Busy"), CODE_FDC_MSR, 0, (1 << 4), (1 << 4));
		AddBoolItem(wxT("FDC MSR (EXM) Execution Phase (non-DMA)"), CODE_FDC_MSR, 0, (1 << 5), (1 << 5));

		AddBoolItem(wxT("FDC MSR (DIO) Data direction (FDC->CPU)"), CODE_FDC_MSR, 0, (1<<6), (1<<6));
		AddBoolItem(wxT("FDC MSR (RQM) Data register ready"), CODE_FDC_MSR, 0, (1<<7), (1<<7));


		for (int i=0; i<FDC_MAX_DRIVES; i++)
		{
			wxString sReg;
			sReg.Printf(wxT("FDC FD%d PCN"), i);
			Add8BitItem(sReg, CODE_FDC_PCN, i);
		}

		Add8BitItem(wxT("FDC US1/0"), CODE_FDC_DRIVE_OUTPUT, 0);

		Add8BitItem(wxT("FDC Side"), CODE_FDC_SIDE_OUTPUT, 0);

		AddBoolItem(wxT("FDC IRQ"), CODE_FDC_INTERRUPT_OUTPUT, 0, true);

		Add8BitItem(wxT("FDC Command Byte"), CODE_FDC_COMMAND_BYTE, 0);
#if 0
		wxString sCommandName = wxT("Invalid");
		char Command = CommandByte & 0x01f;
		switch (Command)
		{
		case 2:
		{
			CommandByte &=~(1<<7);
			sCommandName = wxT("Read track");
		}
		break;

		case 5:
		{
			sCommandName = wxT("Write data");
		}
		break;

		case 6:
		{
			sCommandName = wxT("Read data");
		}
		break;

		case 9:
		{
			sCommandName = wxT("Write deleted data");
		}
		break;

		case 10:
		{
			CommandByte &=~((1<<7)|(1<<5));
			sCommandName = wxT("Read id");
		}
		break;


		case 12:
		{
			sCommandName = wxT("Read deleted data");
		}
		break;


		case 13:
		{
			CommandByte &=~((1<<7)|(1<<5));
			sCommandName = wxT("Format");
		}
		break;


		case 17:
		{
			sCommandName = wxT("Scan Equal");
		}
		break;


		case 25:
		{
			sCommandName = wxT("Scan Low or Equal");
		}
		break;

		case 29:
		{
			sCommandName = wxT("Scan High or Equal");
		}
		break;

		case 7:
		{
			sCommandName = wxT("Recalibrate");
		}
		break;

		case 8:
		{
			sCommandName = wxT("Sense interrupt status");
		}
		break;


		case 3:
		{
			sCommandName = wxT("Specify");
		}
		break;

		case 4:
		{
			sCommandName = wxT("Sense drive status");
		}
		break;


		case 15:
		{
			sCommandName = wxT("Seek");
		}
		break;
	}
		AddItem(wxT("FDC Command Name"), sCommandName);
#endif
		AddBoolItem(wxT("FDC Command Multi-Track"), CODE_FDC_COMMAND_BYTE, 0, (1 << 7), (1 << 7));
		AddBoolItem(wxT("FDC Command MFM"), CODE_FDC_COMMAND_BYTE, 0, (1 << 6), (1 << 6));
		AddBoolItem(wxT("FDC Command Skip"), CODE_FDC_COMMAND_BYTE, 0, (1 << 5), (1 << 5));

		AddBoolItem(wxT("FDC DMA Mode"), CODE_FDC_DMA_MODE, 0, true);
		Add8BitItem(wxT("FDC Head Load Time"), CODE_FDC_HEAD_LOAD_TIME, 0);
		Add8BitItem(wxT("FDC Head Unload Time"), CODE_FDC_HEAD_UNLOAD_TIME, 0);
		Add8BitItem(wxT("FDC Step Rate Time"), CODE_FDC_STEP_RATE_TIME, 0);
	}
	if (DetailsDisplay.m_bDisplayFDI)
	{
		Add8BitItem(wxT("FDI Drive"), CODE_FDI_DRIVE, 0);
		Add8BitItem(wxT("FDI Side"), CODE_FDI_SIDE, 0);
		AddBoolItem(wxT("FDI Motor On"), CODE_FDI_MOTOR, 0, true);

	}

	if (DetailsDisplay.m_bDisplayKeyboard)
	{
		Add8BitItem(wxT("Selected Keyboard Row"), CODE_KEYBOARD_SELECTED_ROW, 0);

		for (int i = 0; i < 10; i++)
		{
			wxString sReg;
			sReg.Printf(wxT("Keyboard Row %d"), i);
			Add8BitItem(sReg, CODE_KEYBOARD_ROW, i);
		}
	}

	// for virtual list control
	SetItemCount(m_Items.GetCount());
	Thaw();

	m_nCachedBottomItem = -1;
	m_nCachedTopItem = -1;

	// refresh all for the first time
	PerformRefresh();
	}
#if 0
void HardwareDetails::PerformRefreshItem(int nItem)
{
	wxUIntPtr Data = GetItemData(nItem);
	ItemClientData *pClientData = (ItemClientData *)Data;
	ItemData &ItemData = pClientData->GetData();

	wxString sValue;
	if (ItemData.RefreshNeeded(sValue))
	{
		wxListItem Item;
		Item.SetId(nItem);
		Item.SetMask(wxLIST_MASK_TEXT);
		Item.SetText(sValue);
		Item.SetColumn(1);
		SetItem(Item);
	}
}
#endif

void HardwareDetails::AddItem(const wxString &sValueName, ItemData &aItemData)  //, ItemClientData *pData)
{
	HardwareDetailsItem *Details = new HardwareDetailsItem();
	Details->m_Label = sValueName;
	Details->m_Data = aItemData;
	m_Items.Add(Details);
#if 0
	int nItems = this->GetItemCount();

	wxListItem Item;
	/Item.SetMask(wxLIST_MASK_TEXT);
	Item.SetId(nItems);

	Item.SetText(sValueName);
	Item.SetImage(-1); //Insert a blank icon, not find to insert nothing
	Item.SetColumn(0);

	// insert and get index
	long nIndex = InsertItem(Item);

	wxString sEmptyString;
	Item.SetId(nIndex);
	Item.SetMask(wxLIST_MASK_TEXT);
	Item.SetText(sEmptyString);
	Item.SetColumn(1);
	SetItem(Item);

	SetItemPtrData(nIndex, (wxUIntPtr)pData);

	RefreshItem(nIndex);
#endif
}


void HardwareDetails::Add16BitItem(const wxString &sValueName, const int nCode, const int nIndex)
{
	ItemData data;  //ItemClientData *pClientData = new ItemClientData();
	data.Set16BitUnsigned(nCode);
	data.SetIndex(nIndex);
	data.SetMask(0x0ffff);
	AddItem(sValueName, data);
}


void HardwareDetails::Add8BitItem(const wxString &sValueName, const int nCode, const int nIndex)
{
	ItemData data;  //ItemClientData *pClientData = new ItemClientData();
	data.Set8BitUnsigned(nCode);
	data.SetMask(0x0ff);
	data.SetIndex(nIndex);
	AddItem(sValueName, data);
}


void HardwareDetails::AddStringIDItem(const wxString &sValueName, int nCode, int nIndex, int nMask, int nShift, const wxChar **sLabels, int nLabels)
{
	ItemData data;
	data.SetStringTable(sLabels, nLabels);
	data.SetStringID(nCode);
	data.SetMask(nMask);
	data.SetIndex(nIndex);
	data.SetShift(nShift);
	AddItem(sValueName, data);


}


void HardwareDetails::AddStringIDItem(const wxString &sValueName, const int nCode, const int nIndex, const wxChar **sLabels, int nLabels)
{
	ItemData data;  //ItemClientData *pClientData = new ItemClientData();
	data.SetStringTable(sLabels, nLabels);
	data.SetStringID(nCode);
	data.SetMask(0x0ff);
	data.SetIndex(nIndex);
	AddItem(sValueName, data);
}


void HardwareDetails::AddMasked8BitItem(const wxString &sValueName, const int nCode, const int nIndex, int nMask)
{
	ItemData data;  //ItemClientData *pClientData = new ItemClientData();
	data.Set8BitUnsigned(nCode);
	data.SetIndex(nIndex);
	data.SetMask(nMask);
	AddItem(sValueName, data);
}


void HardwareDetails::AddBoolItem(const wxString &sValueName, const int nCode, const int nIndex, const int nMask, const int nComparison)
{
	ItemData data;  //ItemClientData *pClientData = new ItemClientData();
	data.SetBool(nCode);
	data.SetIndex(nIndex);
	data.SetMask(nMask);
	data.SetComparison(nComparison);

	AddItem(sValueName, data);
}

void HardwareDetails::AddBoolItem(const wxString &sValueName, const int nCode, const int nIndex, const int nMask, bool bSet)
{
	ItemData data;  //ItemClientData *pClientData = new ItemClientData();
	data.SetBool(nCode);
	data.SetIndex(nIndex);
	data.SetMask(nMask);
	if (bSet)
	{
		data.SetComparison(nMask);
	}
	else
	{
		data.SetComparison(0);
	}

	AddItem(sValueName, data);


}


void HardwareDetails::AddBoolItem(const wxString &sValueName, const int nCode, const int nIndex,bool bSet)
{
	ItemData data;  //ItemClientData *pClientData = new ItemClientData();
	data.SetBool(nCode, bSet);
	data.SetIndex(nIndex);
	AddItem(sValueName, data);
}


#if 0
void HardwareDetails::Add16BitNumber(const wxString &sValueName, unsigned short nValue)
{
	wxString sValue;
	if (m_bDisplayHex)
	{
		sValue.Printf(wxT("&%04x"), nValue);
	}
	else
	{
		sValue.Printf(wxT("%d"),nValue);
	}
	AddItem(sValueName, sValue);
}

void HardwareDetails::Add32BitNumber(const wxString &sValueName, unsigned long nValue)
{
	wxString sValue;
	if (m_bDisplayHex)
	{

		sValue.Printf(wxT("&%08x"), nValue);
	}
	else
	{
		sValue.Printf(wxT("%d"),nValue);
	}
	AddItem(sValueName, sValue);
}


void HardwareDetails::Add16BitSignedNumber(const wxString &sValueName, signed short nValue)
{
	wxString sValue;
	if (m_bDisplayHex)
	{
		if ((nValue & 0x08000)!=0)
		{
			nValue = -nValue;
			sValue.Printf(wxT("-&%04x"), nValue);
		}
		else
		{
			sValue.Printf(wxT("&%04x"), nValue);
		}
	}
	else
	{
		sValue.Printf(wxT("%d"),nValue);
	}
	AddItem(sValueName, sValue);
}

void HardwareDetails::Add8BitNumber(const wxString &sValueName, unsigned char nValue)
{
	wxString sValue;
	if (m_bDisplayHex)
	{
		sValue.Printf(wxT("&%02x"), nValue);
	}
	else
	{
		sValue.Printf(wxT("%d"), nValue);
	}
	AddItem(sValueName, sValue);
}


void HardwareDetails::Add8BitSignedNumber(const wxString &sValueName, signed char nValue)
{
	wxString sValue;
	if (m_bDisplayHex)
	{
		if ((nValue & 0x080)!=0)
		{
			nValue =-nValue;
			sValue.Printf(wxT("-&%02x"), nValue & 0x07f);
		}
		else
		{
			sValue.Printf(wxT("&%02x"), nValue);
		}
	}
	else
	{
		sValue.Printf(wxT("%d"), nValue);
	}
	AddItem(sValueName, sValue);
}

void HardwareDetails::AddBool(const wxString &sValueName, bool bValue)
{
	wxString sValue;
	if (bValue)
	{
		sValue = sYes;
	}
	else
	{
		sValue = sNo;
	}
	AddItem(sValueName, sValue);
}
#endif

void HardwareDetails::PerformRefresh()
{


	if (m_nCachedTopItem != m_nCachedBottomItem)
	{
		this->Freeze();
		for (int i = m_nCachedTopItem; i < m_nCachedBottomItem; i++)
		{
			HardwareDetailsItem *pItem = (HardwareDetailsItem *)m_Items.Item(i);

			if (pItem->m_Data.RefreshNeeded(pItem->m_CachedValue))
			{
				pItem->m_bChanged = pItem->m_Data.GetItemChanged();
				this->RefreshItem(i);

			}

		}
		this->Thaw();
		this->Refresh();
	}
}

void HardwareDetails::OnKeyDown(wxKeyEvent &event)
{
	switch (event.GetKeyCode())
	{
	default:
		break;

	case WXK_F5:
	{
		if (event.GetModifiers() == 0)
		{
			PerformRefresh();
			return;
		}
	}
	break;

	}
	event.Skip();

}




wxObject *BreakpointsResourceHandler::DoCreateResource()
{

	XRC_MAKE_INSTANCE(control, Breakpoints)

		control->Create(m_parentAsWindow,
		GetID(),
		GetPosition(), GetSize(),
		GetStyle(),
		wxDefaultValidator,
		GetName());

	SetupWindow(control);
	return control;
}


bool BreakpointsResourceHandler::CanHandle(wxXmlNode *node)
{
	return IsOfClass(node, wxT("Breakpoints"));
}


BreakpointsResourceHandler::BreakpointsResourceHandler() : wxXmlResourceHandler()
{


}

BreakpointsResourceHandler::~BreakpointsResourceHandler()
{

}


BEGIN_EVENT_TABLE(Breakpoints, wxListCtrl)
EVT_KEY_DOWN(Breakpoints::OnKeyDown)
EVT_COMMAND(XRCID("m_BreakpointRegisterAdd"), wxEVT_COMMAND_MENU_SELECTED, Breakpoints::OnBreakpointRegisterAdd)
EVT_COMMAND(XRCID("m_BreakpointMemoryAdd"), wxEVT_COMMAND_MENU_SELECTED, Breakpoints::OnBreakpointMemoryAdd)
EVT_COMMAND(XRCID("m_BreakpointIOAdd"), wxEVT_COMMAND_MENU_SELECTED, Breakpoints::OnBreakpointIOAdd)
EVT_COMMAND(XRCID("m_BreakpointEnable"), wxEVT_COMMAND_MENU_SELECTED, Breakpoints::OnBreakpointEnable)
EVT_COMMAND(XRCID("m_BreakpointEdit"), wxEVT_COMMAND_MENU_SELECTED, Breakpoints::OnBreakpointEdit)
EVT_COMMAND(XRCID("m_BreakpointDisable"), wxEVT_COMMAND_MENU_SELECTED, Breakpoints::OnBreakpointDisable)
EVT_COMMAND(XRCID("m_GotoBreakpoint"), wxEVT_COMMAND_MENU_SELECTED, Breakpoints::OnGotoBreakpoint)
EVT_COMMAND(XRCID("m_BreakpointRemove"), wxEVT_COMMAND_MENU_SELECTED, Breakpoints::OnBreakpointRemove)
EVT_CONTEXT_MENU(Breakpoints::OnContextMenu)
EVT_COMMAND(XRCID("m_BreakpointEnableAll"), wxEVT_COMMAND_MENU_SELECTED, Breakpoints::OnBreakpointEnableAll)
EVT_COMMAND(XRCID("m_BreakpointDisableAll"), wxEVT_COMMAND_MENU_SELECTED, Breakpoints::OnBreakpointDisableAll)
EVT_COMMAND(XRCID("HSyncStart"), wxEVT_COMMAND_MENU_SELECTED, Breakpoints::OnBreakEvent)
EVT_COMMAND(XRCID("HSyncEnd"), wxEVT_COMMAND_MENU_SELECTED, Breakpoints::OnBreakEvent)
EVT_COMMAND(XRCID("VSyncStart"), wxEVT_COMMAND_MENU_SELECTED, Breakpoints::OnBreakEvent)
EVT_COMMAND(XRCID("VSyncEnd"), wxEVT_COMMAND_MENU_SELECTED, Breakpoints::OnBreakEvent)
EVT_COMMAND(XRCID("Z80Int"), wxEVT_COMMAND_MENU_SELECTED, Breakpoints::OnBreakEvent)
EVT_COMMAND(XRCID("Z80Nmi"), wxEVT_COMMAND_MENU_SELECTED, Breakpoints::OnBreakEvent)
EVT_COMMAND(XRCID("ASICUnlocked"), wxEVT_COMMAND_MENU_SELECTED, Breakpoints::OnBreakEvent)
EVT_COMMAND(XRCID("ASICLocked"), wxEVT_COMMAND_MENU_SELECTED, Breakpoints::OnBreakEvent)
EVT_WINDOW_CREATE(Breakpoints::OnCreate)
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS(Breakpoints, wxListCtrl)


int Breakpoints::IdToMask(int id)
{
	if (id == XRCID("HSyncStart"))
		return BREAK_ON_HSYNC_START;
	if (id == XRCID("HSyncEnd"))
		return BREAK_ON_HSYNC_END;
	if (id == XRCID("VSyncStart"))
		return BREAK_ON_VSYNC_START;
	if (id == XRCID("VSyncEnd"))
		return BREAK_ON_VSYNC_END;
	if (id == XRCID("Z80Int"))
		return BREAK_ON_INT;
	if (id == XRCID("Z80Nmi"))
		return BREAK_ON_NMI;
	if (id == XRCID("ASICUnlocked"))
		return BREAK_ON_ASIC_UNLOCK;
	if (id == XRCID("ASICLocked"))
		return BREAK_ON_ASIC_LOCK;

	return 0;
}

void Breakpoints::OnBreakEvent(wxCommandEvent &event)
{
	int Mask = IdToMask(event.GetId());
	SetBreakOnAction(Mask, !BreakOnAction(Mask));
}


void Breakpoints::OnKeyDown(wxKeyEvent &event)
{
	switch (event.GetKeyCode())
	{
	default:
		break;

	case WXK_DELETE:
	{
		RemoveBreakpoints();

	}
	break;


	case WXK_NUMPAD_ADD:
	{
		if (event.GetModifiers() == 0)
		{
			SetEnableState(TRUE);
		}
		else
		{
			SetEnableStateAll(TRUE);
		}
	}
	break;

	case WXK_NUMPAD_SUBTRACT:
	{
		if (event.GetModifiers() == 0)
		{
			SetEnableState(FALSE);
		}
		else
		{
			SetEnableStateAll(FALSE);
		}

	}
	break;

#if !defined(__WXMAC__)
	case WXK_F5:
	{
		if (event.GetModifiers() == 0)
		{
			PerformRefresh();
			return;
		}

	}
	break;
#endif
	}
	event.Skip();

}

void Breakpoints::GoToBreakpoint(long item)
{
	wxUIntPtr Data = GetItemData(item);
	BREAKPOINT *pBreakpoint = (BREAKPOINT *)Data;
	if (pBreakpoint->Description.Type == BREAKPOINT_TYPE_PC)
	{
		// ideally double click goes to default window of your choice that you can set.
		int nSelection = 0;
		DissassembleWindow *pWindow = DebuggerWindow::GetDissassemblyWindowByIndex(nSelection);
		if (pWindow != NULL)
		{

			DISSASSEMBLE_WINDOW *pDisassembleWindow = pWindow->GetStruct();
			Dissassemble_SetAddress(pDisassembleWindow, pBreakpoint->Description.Address);

			pWindow->Refresh();
		}
	}
	else
	{
		int nSelection = 0;
		MemDumpWindow *pWindow = DebuggerWindow::GetMemdumpWindowByIndex(nSelection);
		if (pWindow)
		{

			MEMDUMP_WINDOW *pMemdumpWindow = pWindow->GetStruct();
			MemDump_SetAddress(pMemdumpWindow, pBreakpoint->Description.Address);
			pWindow->Refresh();
		}
	}
}

void Breakpoints::OnGotoBreakpoint(wxCommandEvent & WXUNUSED(event))
{
	// TODO: Choose where it goes to
	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		GoToBreakpoint(item);
	}
}

void Breakpoints::OnBreakpointActivated(wxListEvent& event)
{
	GoToBreakpoint(event.m_itemIndex);
}

void Breakpoints::AddBreakpointI(BREAKPOINT_MAJOR_TYPE nType, int Address, wxWindow *pParent)
{
	BreakpointDialog dialog(nType, Address,pParent);
	if (dialog.ShowModal() == wxID_OK)
	{
		// does breakpoint already exist?
		{
			BREAKPOINT_DESCRIPTION Description;
			BreakpointDescription_Init(&Description);

			BreakpointDescription_SetHitCount(&Description, dialog.m_nCount);

			if (dialog.m_bUseData)
			{
				if (dialog.m_bUseDataMask)
				{
					BreakpointDescription_SetDataWithMask(&Description, dialog.m_nData, dialog.m_nDataMask);
				}
				else
				{
					BreakpointDescription_SetDataNoMask(&Description, dialog.m_nData);
				}
			}
			else
			{
				BreakpointDescription_SetIgnoreData(&Description);
			}

			if (dialog.m_bUseAddressMask)
			{
				BreakpointDescription_SetAddressMask(&Description, dialog.m_nAddressMask);
			}
			else
			{
				BreakpointDescription_SetNoAddressMask(&Description);
			}
			Description.Type = (BREAKPOINT_TYPE)dialog.m_nType;
			Description.Address = dialog.m_nAddress;

			BREAKPOINT *pBreakpoint = Breakpoints_AddBreakpoint(&Description);
			if (pBreakpoint)
			{
				Breakpoints_SetEnabled(pBreakpoint, dialog.m_bEnabled);
			}
		}
	}
}

void Breakpoints::AddBreakpoint(BREAKPOINT_MAJOR_TYPE nType)
{
	Breakpoints::AddBreakpointI(nType,0, this);
	PerformRefresh();
}


void Breakpoints::OnBreakpointMemoryAdd(wxCommandEvent & WXUNUSED(event))
{
	AddBreakpoint(BREAKPOINT_MAJOR_TYPE_MEMORY);
}

void Breakpoints::OnBreakpointRegisterAdd(wxCommandEvent & WXUNUSED(event))
{
	AddBreakpoint(BREAKPOINT_MAJOR_TYPE_REGISTER);
}

void Breakpoints::OnBreakpointIOAdd(wxCommandEvent & WXUNUSED(event))
{
	AddBreakpoint(BREAKPOINT_MAJOR_TYPE_IO);
}


void Breakpoints::RemoveBreakpoints()
{
	ArrayOfInts Array;

	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	while (item != -1)
	{
		Array.Add(item);

		item = GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);
	}

	for (int i = Array.GetCount() - 1; i >= 0; i--)
	{
		wxUIntPtr Data = GetItemData(Array[i]);
		BREAKPOINT *pBreakpoint = (BREAKPOINT *)Data;
		Breakpoints_RemoveBreakpoint(pBreakpoint);
	}
	PerformRefresh();
}

void Breakpoints::OnBreakpointRemove(wxCommandEvent & WXUNUSED(event))
{
	RemoveBreakpoints();
}

void Breakpoints::SetEnableState(BOOL bState)
{
	ArrayOfInts Array;

	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	while (item != -1)
	{
		wxUIntPtr Data = GetItemData(item);
		BREAKPOINT *pBreakpoint = (BREAKPOINT *)Data;
		if (pBreakpoint)
		{
			Breakpoints_SetEnabled(pBreakpoint, bState);
			Breakpoint_SetRequireRefreshInGUI(pBreakpoint, TRUE);
		}
		
		item = GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);
	}



}

void Breakpoints::SetEnableStateAll(BOOL bState)
{

	BREAKPOINT *pBreakPoint = Breakpoints_GetFirst();
	while (pBreakPoint != NULL)
	{
		if (!pBreakPoint->Description.bTemporary)
		{
			Breakpoints_SetEnabled(pBreakPoint, bState);
			Breakpoint_SetRequireRefreshInGUI(pBreakPoint, TRUE);
		}
		pBreakPoint = Breakpoints_GetNext(pBreakPoint);
	}


}



void Breakpoints::OnBreakpointEdit(wxCommandEvent & WXUNUSED(event))
{
	ArrayOfInts Array;

	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item!=-1)
	{
#if 0
		BreakpointDialog dialog(this);
		wxUIntPtr Data = GetItemData(item);
		BREAKPOINT *pBreakpoint = (BREAKPOINT *)Data;
		pDialog->SetBreakpoint(pBreakpoint);

		if (dialog.ShowModal() == wxID_OK)
		{
			pBreakpoint->Type = (BREAKPOINT_TYPE)dialog.m_nType;
			pBreakpoint->Address = dialog.m_nAddress;
			Breakpoints_SetEnabled(pBreakpoint, dialog.m_bEnabled);
			Breakpoints_SetHitCount(pBreakpoint, dialog.m_nCount);
			Breakpoints_SetData(pBreakpoint, dialog.m_nData, pDialog->m_nMask);
		}
#endif
		PerformRefresh();

		}
	}


void Breakpoints::OnBreakpointEnable(wxCommandEvent & WXUNUSED(event))
{
	SetEnableState(TRUE);
}


void Breakpoints::OnBreakpointDisable(wxCommandEvent & WXUNUSED(event))
{
	SetEnableState(FALSE);
}


void Breakpoints::OnBreakpointEnableAll(wxCommandEvent & WXUNUSED(event))
{
	SetEnableStateAll(TRUE);
}


void Breakpoints::OnBreakpointDisableAll(wxCommandEvent & WXUNUSED(event))
{
	SetEnableStateAll(FALSE);
}



void Breakpoints::OnContextMenu(wxContextMenuEvent &event)
{
	wxPoint Position = event.GetPosition();

	// generate a suitable place for the menu
	if (event.GetPosition() == wxDefaultPosition)
	{
		wxPoint ClientPos(0, 0);
		Position = this->ClientToScreen(ClientPos);
	}

	wxMenuBar *pMenuBar = wxXmlResource::Get()->LoadMenuBar(wxT("MB_MENU_BREAKPOINTS_MENU_BAR"));
	if (pMenuBar)
	{
		arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("HSyncStart"), BreakOnAction(BREAK_ON_HSYNC_START) ? true : false);
		arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("HSyncEnd"), BreakOnAction(BREAK_ON_HSYNC_END) ? true : false);
		arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("VSyncStart"), BreakOnAction(BREAK_ON_VSYNC_START) ? true : false);
		arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("VSyncEnd"), BreakOnAction(BREAK_ON_VSYNC_END) ? true : false);
		arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("Z80Int"), BreakOnAction(BREAK_ON_INT) ? true : false);
		arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("Z80Nmi"), BreakOnAction(BREAK_ON_NMI) ? true : false);
		arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ASICUnlocked"), BreakOnAction(BREAK_ON_ASIC_UNLOCK) ? true : false);
		arnguiApp::RefreshSingleItemState(pMenuBar, XRCID("ASICLocked"), BreakOnAction(BREAK_ON_ASIC_LOCK) ? true : false);


		wxMenu *pMenu = pMenuBar->Remove(0);
		if (pMenu)
		{
			PopupMenu(pMenu);
			delete pMenu;
		}
		delete pMenuBar;
	}
}

void Breakpoints::OnCreate(wxWindowCreateEvent & WXUNUSED(event))
{
	PerformInitialSetup();

	PerformRefreshShown();
}


Breakpoints::Breakpoints() : m_nCachedTopItem(-1), m_nCachedBottomItem(-1)
{
	DebuggerWindow::RegisterBreakpoints(this);
}

void Breakpoints::PerformInitialSetup()
{
	if (GetColumnCount()!=0)
	{
		return;
	}
	
	Freeze();
	ClearAll();

	wxListItem Column;

	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT("Type"));
	InsertColumn(0, Column);

	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT("Address"));
	InsertColumn(1, Column);

	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT("Address Mask"));
	InsertColumn(2, Column);

	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT("Enabled"));
	InsertColumn(3, Column);

	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT("Hit Count"));
	InsertColumn(4, Column);

	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT("Hits Remaining"));
	InsertColumn(5, Column);


	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT("Data"));
	InsertColumn(6, Column);

	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT("Mask"));
	InsertColumn(7, Column);

	wxGetApp().SetColumnSize(this, 0);
	wxGetApp().SetColumnSize(this, 1);
	wxGetApp().SetColumnSize(this, 2);
	wxGetApp().SetColumnSize(this, 3);
	wxGetApp().SetColumnSize(this, 4);
	wxGetApp().SetColumnSize(this, 5);
	wxGetApp().SetColumnSize(this, 6);
	wxGetApp().SetColumnSize(this, 7);

	Thaw();

	Connect(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxListEventHandler(Breakpoints::OnBreakpointActivated), NULL, this);

	// connect event
	Connect(GetId(), wxEVT_COMMAND_LIST_CACHE_HINT, wxListEventHandler(Breakpoints::OnCacheHint));
}


Breakpoints::~Breakpoints()
{
	Disconnect( wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxListEventHandler( Breakpoints::OnBreakpointActivated ), NULL, this);

	// connect event
	Disconnect( this->GetId(), wxEVT_COMMAND_LIST_CACHE_HINT, wxListEventHandler( Breakpoints::OnCacheHint) );

	DebuggerWindow::DeRegisterBreakpoints(this);

}

#if 0
void Breakpoints::AddItem(const wxString &sValueName, const wxString &sValue)
{

	wxListItem Item;
	int nItems = this->GetItemCount();
	Item.SetMask(wxLIST_MASK_TEXT);
	Item.SetId(nItems);

	Item.SetText(sValueName);
	Item.SetColumn(0);
	Item.SetImage(-1); //Insert a blank icon, not find to insert nothing
	InsertItem(Item);


	// set size field
	Item.SetMask(wxLIST_MASK_TEXT);
	Item.SetText(sValue);
	Item.SetColumn(1);
	SetItem(Item);

}
#endif


void Breakpoints::PerformRefreshItem(int nItem)
{
	PerformInitialSetup();

	if (nItem>=GetItemCount())
		return;
	
	wxUIntPtr Data = GetItemData(nItem);
	BREAKPOINT *pBreakpoint = (BREAKPOINT *)Data;
	if (!pBreakpoint)
		return;

	if (!Breakpoints_Valid(pBreakpoint))
		return;


	if (!Breakpoint_RequireRefreshInGUI(pBreakpoint))
		return;

	wxListItem Item;
	Item.SetMask(wxLIST_MASK_TEXT);
	Item.SetId(nItem);

	wxString sValueName;

	switch (pBreakpoint->Description.Type)
	{
	case BREAKPOINT_TYPE_PC:
	{
		sValueName = wxT("PC");
	}
	break;

	case BREAKPOINT_TYPE_SP:
	{
		sValueName = wxT("SP");
	}
	break;

	case BREAKPOINT_TYPE_MEMORY_WRITE:
	{
		sValueName = wxT("MW");
	}
	break;

	case BREAKPOINT_TYPE_MEMORY_READ:
	{
		sValueName = wxT("MR");
	}
	break;

	case BREAKPOINT_TYPE_IO_WRITE:
	{
		sValueName = wxT("IOW");
	}
	break;

	case BREAKPOINT_TYPE_IO_READ:
	{
		sValueName = wxT("IOR");
	}
	break;
	default:
	{
		sValueName = wxT("??");
	}
	break;
	}
	Item.SetText(sValueName);

	Item.SetColumn(0);
	Item.SetImage(-1); //Insert a blank icon, not find to insert nothing
	SetItem(Item);

	sValueName.sprintf(wxT("&%04x"), pBreakpoint->Description.Address);

	Item.SetText(sValueName);
	Item.SetColumn(1);
	SetItem(Item);

	sValueName.sprintf(wxT("&%04x"), pBreakpoint->Description.AddressMask);

	Item.SetText(sValueName);
	Item.SetColumn(2);
	SetItem(Item);


	sValueName = sNo;
	if (pBreakpoint->bEnabled)
	{
		sValueName = sYes;
	}
	Item.SetText(sValueName);
	Item.SetColumn(3);
	SetItem(Item);

	if (pBreakpoint->Description.nReloadHitCount <= 1)
	{
		sValueName = wxT("Always");
	}
	else
	{

		sValueName.Printf(wxT("%d"), pBreakpoint->Description.nReloadHitCount);
	}
	Item.SetText(sValueName);
	Item.SetColumn(4);
	SetItem(Item);

	sValueName.Printf(wxT("%d"), pBreakpoint->nCurrentHitCount);
	Item.SetText(sValueName);
	Item.SetColumn(5);
	SetItem(Item);

	sValueName.Printf(wxT("&%02x"), pBreakpoint->Description.Data);
	Item.SetText(sValueName);
	Item.SetColumn(6);
	SetItem(Item);

	sValueName.Printf(wxT("&%02x"), pBreakpoint->Description.DataMask);
	Item.SetText(sValueName);
	Item.SetColumn(7);
	SetItem(Item);

	Breakpoint_SetRequireRefreshInGUI(pBreakpoint, FALSE);

}


void Breakpoints::OnCacheHint(wxListEvent &event)
{
	if (Breakpoints_FullRefreshInGUI())
		return;

	long nFrom = event.GetCacheFrom();
	long nTo = event.GetCacheTo();

	bool bRefresh = false;
	if (nFrom != m_nCachedTopItem)
	{
		bRefresh = true;
	}
	if (nTo != m_nCachedBottomItem)
	{
		bRefresh = true;
	}

	if (bRefresh)
	{
		long nCacheFrom;
		long nCacheTo;

		if (
			// from has gone less than cahced top item
			(nFrom < m_nCachedTopItem) &&
			// to remained within our cached range
			((nTo >= m_nCachedTopItem) && (nTo <= m_nCachedBottomItem))
			)
		{
			//            wxMessageOutputDebug().Printf(wxT("OnCacheHint Scroll up\r\n"));

			nCacheFrom = nFrom;
			nCacheTo = m_nCachedTopItem;
		}
		else if
			(
			// to has increased
			(nTo > m_nCachedBottomItem) &&
			// from remained within our cached range
			((nFrom >= m_nCachedTopItem) && (nFrom <= m_nCachedBottomItem))
			)
		{
			//          wxMessageOutputDebug().Printf(wxT("OnCacheHint Scroll down/resize\r\n"));

			nCacheFrom = m_nCachedBottomItem;
			nCacheTo = nTo;
		}
		else
		{
			//      wxMessageOutputDebug().Printf(wxT("OnCacheHint New range\r\n"));

			nCacheFrom = nFrom;
			nCacheTo = nTo;
		}

		m_nCachedTopItem = nFrom;
		m_nCachedBottomItem = nTo;




		this->Freeze();
		for (int i = nCacheFrom; i <= nCacheTo; i++)
		{
			PerformRefreshItem(i);
		}
		this->Thaw();
		this->Refresh();
	}
}

void Breakpoints::PerformRefresh()
{
	PerformInitialSetup();

	if (Breakpoints_FullRefreshInGUI())
	{
		PerformRefreshShown();
	}
	else
	{

		if (m_nCachedTopItem != m_nCachedBottomItem)
		{
			this->Freeze();
			for (int i = m_nCachedTopItem; i < m_nCachedBottomItem; i++)
			{
				PerformRefreshItem(i);
			}
			this->Thaw();
			this->Refresh();
		}
	}
}

void Breakpoints::PerformRefreshShown()
{

	Freeze();
	DeleteAllItems();

	BREAKPOINT *pBreakPoint = Breakpoints_GetFirst();
	while (pBreakPoint != NULL)
	{
		if (!pBreakPoint->Description.bTemporary)
		{
			wxListItem Item;
			int nItems = this->GetItemCount();
			Item.SetMask(wxLIST_MASK_TEXT);
			Item.SetId(nItems);
			long nInsertedIndex = InsertItem(Item);

			SetItemPtrData(nInsertedIndex, (wxUIntPtr)pBreakPoint);
			Breakpoint_SetRequireRefreshInGUI(pBreakPoint, TRUE);

			// until we do a more quicker way.. use this
			PerformRefreshItem(nInsertedIndex);
		}
		pBreakPoint = Breakpoints_GetNext(pBreakPoint);
	}

	Thaw();
	Refresh();

}
