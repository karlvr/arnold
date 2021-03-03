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
// TODO: Where sprites are multi-width it is hard to see other sprites around.
// Is there a way to make it easier to see?
// TODO: Be able to set method and width at a specific address.
// TODO Current mode is a problem if multi-mode.
// TODO: Specify mode in settings
// TODO: Draw mode 0 wider
// TODO: Capture palette so it doesn't change when it refreshes.
// TODO: Revert from ASIC ram if we choose a config that is not asic related
// TODO: If mode uses current mode, then make sure it re-captures it?
// TODO: Easy way to switch to screen vs sprite mode.
// TODO: Let graphics hardware do the zoom not us.
// TODO: Update of memory address
// TODO: VIsibility of memory region used
// TODO: Click on pixel/byte and address is shown
// TODO: Reverse pixels?
// TODO: FLip graphics vertically?
// TODO: When viewing as screen, take current crtc values as base?
// TODO: Make sure we can scroll through memory and view graphics with wrap.
#include "GraphicsEditor.h"
#include "arnguiMain.h"
#include "arnguiApp.h"
#include <wx/msgdlg.h>
#include <wx/choice.h>
#include <wx/menu.h>
#include <wx/choicdlg.h>
#include <wx/textdlg.h>

extern "C"
{
#include "../cpc/asic.h"
#include "../cpc/cpc.h"
#include "../cpc/kcc.h"
#include "../cpc/aleste.h"
#include "../cpc/crtc.h"
#include "../cpc/riff.h"
}

#define CONFIG_DISPLAY_HEIGHT (1<<0)	// display height
#define CONFIG_CAPTURE_MODE (1<<1)	// capture current mode
#define CONFIG_CAPTURE_CRTC (1<<2) // capture screen size/dimensions
#define CONFIG_CAPTURE_PALETTE (1<<3)
#define CONFIG_CAPTURE_SPRITE_PALETTE (1<<4) // use sprite palette
#define CONFIG_ADDRESS (1<<5)	// use a specific address
#define CONFIG_USE_WIDTH (1<<6)
#define CONFIG_USE_METHOD (1<<7)

#define TRANSPARENCY_UNDEFINED 0
#define TRANSPARENCY_PEN_0 (1<<0)
#define TRANSPARENCY_AND_MASK (1<<1)

typedef struct
{
	const char *sDescription;
	int m_nFlags;
	int m_nLineMethod;
	int m_nAddress;
	int m_nMethod;
	int m_nWidthInBytes;
	int m_nHeight;
	int m_nAddressRange; // 0 = no change, 1=asic ram
	BOOL m_bHasTransparency;
	int m_nTransparencyMode;
} GraphicsEditorConfig;

const GraphicsEditorConfig Configs[] =
{
	{
		"Plus Sprites in ASIC RAM",
		CONFIG_DISPLAY_HEIGHT | CONFIG_CAPTURE_SPRITE_PALETTE | CONFIG_ADDRESS | CONFIG_USE_WIDTH | CONFIG_USE_METHOD,
		LINE_METHOD_LINEAR,
		0x0,
		METHOD_PLUS_SPRITE_PIXELS,
		16,
		16,
		1,
		TRUE,
		TRANSPARENCY_PEN_0,
	},
	{
		"Displayed Screen (current mode)",
		CONFIG_CAPTURE_MODE | CONFIG_CAPTURE_CRTC | CONFIG_CAPTURE_PALETTE,
		LINE_METHOD_CRTC,
		0x0,		// no specific screen
		0,		/// not used
		0,			// not used
		0,			// not used
		0,
		FALSE,
		TRANSPARENCY_UNDEFINED,
	},
	{
		"Software sprites (no transparency) (current mode)",
		CONFIG_CAPTURE_MODE | CONFIG_CAPTURE_PALETTE,
		LINE_METHOD_LINEAR,
		0x0,		// no specific screen
		0,			// not used
		0,			// not used
		0,			// not used
		0,
		FALSE,
		TRANSPARENCY_UNDEFINED,
	},
	{
		"Software sprites (transparency pen 0) (current mode)",
		CONFIG_CAPTURE_MODE | CONFIG_CAPTURE_PALETTE,
		LINE_METHOD_LINEAR,
		0x0,		// no specific screen
		0,			// not used
		0,			// not used
		0,			// not used
		0,
		TRUE,
		TRANSPARENCY_PEN_0,
	},
	{
		"Software sprites (transparency AND mask) (current mode)",
		CONFIG_CAPTURE_MODE | CONFIG_CAPTURE_PALETTE,
		LINE_METHOD_LINEAR,
		0x0,		// no specific screen
		0,			// not used
		0,			// not used
		0,			// not used
		0,
		TRUE,
		TRANSPARENCY_AND_MASK,
	},
	{
		"Renegade sprites",
		CONFIG_USE_METHOD|CONFIG_CAPTURE_PALETTE,
		LINE_METHOD_LINEAR,
		0x0,		// no specific screen
		METHOD_4BPP_RENEGADE,
		0,			// not used
		0,			// not used
		0,
		TRUE,
		TRANSPARENCY_PEN_0,
	},
};





BEGIN_EVENT_TABLE(GraphicsEditorWindow, wxWindow)
//   EVT_ERASE_BACKGROUND(GraphicsEditorWindow::OnEraseBackground)
EVT_PAINT(GraphicsEditorWindow::OnPaint)
//  EVT_SIZE(GraphicsEditorWindow::OnSize)
EVT_CONTEXT_MENU(GraphicsEditorWindow::OnContextMenu)
//   EVT_SET_FOCUS(GraphicsEditorWindow::OnSetFocus)
//   EVT_KILL_FOCUS(GraphicsEditorWindow::OnKillFocus)
EVT_MOUSEWHEEL(GraphicsEditorWindow::OnMouseWheel)
//    EVT_LEFT_DOWN(GraphicsEditorWindow::OnMouseLeftButtonDown)
EVT_KEY_DOWN(GraphicsEditorWindow::OnKeyDown)
EVT_SIZE(GraphicsEditorWindow::OnSize)
EVT_COMMAND(XRCID("m_SetMemoryRange"), wxEVT_COMMAND_MENU_SELECTED, GraphicsEditorWindow::OnSetMemoryRange)
EVT_COMMAND(XRCID("m_SpritePalette"), wxEVT_COMMAND_MENU_SELECTED, GraphicsEditorWindow::OnSetSpritePalette)
EVT_COMMAND(XRCID("m_SetAddress"), wxEVT_COMMAND_MENU_SELECTED, GraphicsEditorWindow::OnSetAddress)
EVT_COMMAND(XRCID("m_MainPalette"), wxEVT_COMMAND_MENU_SELECTED, GraphicsEditorWindow::OnSetMainPalette)
EVT_COMMAND(XRCID("m_DecreaseWidth"), wxEVT_COMMAND_MENU_SELECTED, GraphicsEditorWindow::OnDecreaseWidth)
EVT_COMMAND(XRCID("m_IncreaseWidth"), wxEVT_COMMAND_MENU_SELECTED, GraphicsEditorWindow::OnIncreaseWidth)
EVT_COMMAND(XRCID("m_PageUp"), wxEVT_COMMAND_MENU_SELECTED, GraphicsEditorWindow::OnPageUp)
EVT_COMMAND(XRCID("m_PageDown"), wxEVT_COMMAND_MENU_SELECTED, GraphicsEditorWindow::OnPageDown)
//EVT_COMMAND(XRCID("m_CaptureMode"), wxEVT_COMMAND_MENU_SELECTED, GraphicsEditorWindow::OnSetMode)
//EVT_COMMAND(XRCID("m_CaptureScreenBase"), wxEVT_COMMAND_MENU_SELECTED, GraphicsEditorWindow::OnSetScreenBase)

EVT_COMMAND(XRCID("m_RepeatedRefreshPixels"), wxEVT_COMMAND_MENU_SELECTED, GraphicsEditorWindow::OnSetRepeatedRefreshPixels)
EVT_UPDATE_UI(XRCID("m_RepeatedRefreshPixels"), GraphicsEditorWindow::OnUpdateRepeatedRefreshPixels)

EVT_COMMAND(XRCID("m_FlipVertically"), wxEVT_COMMAND_MENU_SELECTED, GraphicsEditorWindow::OnFlipVertically)
EVT_UPDATE_UI(XRCID("m_FlipVertically"), GraphicsEditorWindow::OnUpdateFlipVertically)

EVT_COMMAND(XRCID("m_RepeatedRefreshPalette"), wxEVT_COMMAND_MENU_SELECTED, GraphicsEditorWindow::OnSetRepeatedRefreshPalette)
EVT_UPDATE_UI(XRCID("m_RepeatedRefreshPalette"), GraphicsEditorWindow::OnUpdateRepeatedRefreshPalette)

EVT_COMMAND(XRCID("m_ShowTransparency"), wxEVT_COMMAND_MENU_SELECTED, GraphicsEditorWindow::OnSetShowTransparency)
EVT_UPDATE_UI(XRCID("m_ShowTransparency"), GraphicsEditorWindow::OnUpdateShowTransparency)

EVT_COMMAND(XRCID("m_Config"), wxEVT_COMMAND_MENU_SELECTED, GraphicsEditorWindow::OnSelectConfig)

END_EVENT_TABLE()

void GraphicsEditorWindow::OnSetPreset(int nPreset)
{
	//    m_nA = Configs[nPreset].m_nAddress;
	// m_nWidth = Configs[nPreset].m_nWidth;
	// m_nMethod = Configs[nPreset].m_nMethod;

	m_bFlipVertically = false;
	m_bFlipHorizontally = false;
	if (Configs[nPreset].m_nAddressRange == 1)
	{
		for (int i = 0; i < CPC_GetRegisteredMemoryRangeCount(); i++)
		{
			wxString sDrive;
			const MemoryRange *pRange = CPC_GetRegisteredMemoryRange(i);
			if (pRange)
			{
				if (pRange->m_nID == RIFF_FOURCC_CODE('A', 'S', 'I', 'C'))
				{
					m_pRange = pRange;
					break;
				}
			}
		}

	}

	if (Configs[nPreset].m_nFlags & CONFIG_USE_METHOD)
	{
		m_nDisplayMethod = Configs[nPreset].m_nMethod;
	}

	if (Configs[nPreset].m_nFlags & CONFIG_CAPTURE_MODE)
	{
		// get the mode and choose an appropiate pixel method
		int nMode = 0;

		// TODO KCC, Aleste
		if (CPC_GetHardware() == CPC_HW_CPC)
		{
			nMode = GateArray_GetMultiConfiguration() & 0x03;
		}
		else if (CPC_GetHardware() == CPC_HW_CPCPLUS)
		{
			nMode = ASIC_GateArray_GetMultiConfiguration() & 0x03;
		}

		switch (nMode)
		{
		case 0:
		{
			m_nDisplayMethod = METHOD_4BPP_INTERLEAVED;   //  mode 0
		}
		break;
		case 1:
		{
			m_nDisplayMethod = METHOD_2BPP_INTERLEAVED;   //  mode 1
		}
		break;
		case 2:
		{
			m_nDisplayMethod = METHOD_1BPP;   //  mode 2
		}
		break;
		case 3:
		{
			m_nDisplayMethod = METHOD_2BPP_INTERLEAVED_MODE3;   //  mode 3
		}
		break;

		default:
			break;
		}
	}

	if (Configs[nPreset].m_nFlags & CONFIG_ADDRESS)
	{
		m_nAddress = Configs[nPreset].m_nAddress;
	}

	if (Configs[nPreset].m_nFlags & CONFIG_USE_WIDTH)
	{
		m_nPlotWidth = Configs[nPreset].m_nWidthInBytes;
	}

	// which method to use for calculating next line
	m_nLineMethod = Configs[nPreset].m_nLineMethod;

	// display the height?
	if (Configs[nPreset].m_nFlags & CONFIG_DISPLAY_HEIGHT)
	{
		m_nHeight = Configs[nPreset].m_nHeight;
	}
	else
	{
		m_nHeight = -1;
	}
	if (Configs[nPreset].m_nFlags & CONFIG_CAPTURE_CRTC)
	{
		CaptureCRTC();
	}
//	if (Configs[nPreset].m_nFlags & CONFIG_CAPTURE_MODE)
//	{
//		CaptureMode();
//	}
	if (Configs[nPreset].m_nFlags & (CONFIG_CAPTURE_PALETTE | CONFIG_CAPTURE_SPRITE_PALETTE))
	{
		m_nPaletteId = 0;
		if (Configs[nPreset].m_nFlags & CONFIG_CAPTURE_SPRITE_PALETTE)
		{
			m_nPaletteId = 1;
		}

		CapturePalette(m_nPaletteId);
	}

	m_bTransparencyUsesPen = false;
	m_bTransparencyUsesInterleavedMask = false;
	if (Configs[nPreset].m_bHasTransparency)
	{
		switch (Configs[nPreset].m_nTransparencyMode)
		{
		case TRANSPARENCY_PEN_0:
		{
			m_bTransparencyUsesPen = true;
			m_nTransparentPen = 0;
		}
		break;

		case TRANSPARENCY_AND_MASK:
		{
			m_bTransparencyUsesInterleavedMask = true;
		}
		break;
		default:
			break;
		}
	}

	m_bResizeRequired = true;
	m_bRefreshRequired = true;
}

void GraphicsEditorWindow::OnSelectConfig(wxCommandEvent & WXUNUSED(event))
{

	wxArrayString sRegisters;

	for (size_t i = 0; i < sizeof(Configs) / sizeof(Configs[0]); i++)
	{
		wxString sConfigName(Configs[i].sDescription, wxConvUTF8);
		sRegisters.Add(sConfigName);
	}

	wxSingleChoiceDialog dialog(this, wxT("Select preset"), wxT("Presets..."), sRegisters);
	if (dialog.ShowModal() == wxID_OK)
	{
		int nSelection = dialog.GetSelection();
		OnSetPreset(nSelection);
	}
}

void GraphicsEditorWindow::OnSize(wxSizeEvent & WXUNUSED(event))
{
	m_bResizeRequired = true;
	m_bRefreshRequired = false;

}

void GraphicsEditorWindow::OnSetRepeatedRefreshPixels(wxCommandEvent & WXUNUSED(event))
{
	m_bRepeatRefreshPixels = !m_bRepeatRefreshPixels;

	m_bRefreshRequired = true;
}

void GraphicsEditorWindow::OnUpdateRepeatedRefreshPixels(wxUpdateUIEvent &event)
{
	event.Check(m_bRepeatRefreshPixels);
}

void GraphicsEditorWindow::OnFlipVertically(wxCommandEvent & WXUNUSED(event))
{
	m_bFlipVertically = !m_bFlipVertically;

	m_bRefreshRequired = true;
}

void GraphicsEditorWindow::OnUpdateFlipVertically(wxUpdateUIEvent &event)
{
	event.Check(m_bFlipVertically);
}

void GraphicsEditorWindow::OnSetRepeatedRefreshPalette(wxCommandEvent & WXUNUSED(event))
{
	m_bRepeatRefreshPalette = !m_bRepeatRefreshPalette;

	m_bRefreshRequired = true;
}

void GraphicsEditorWindow::OnUpdateRepeatedRefreshPalette(wxUpdateUIEvent &event)
{
	event.Check(m_bRepeatRefreshPalette);
}


void GraphicsEditorWindow::OnSetShowTransparency(wxCommandEvent & WXUNUSED(event))
{
	m_bShowTransparency = !m_bShowTransparency;

	m_bRefreshRequired = true;
}

void GraphicsEditorWindow::OnUpdateShowTransparency(wxUpdateUIEvent &event)
{
	event.Check(m_bShowTransparency);
}

void GraphicsEditorWindow::CaptureMode()
{
#if 0
	int nMode;

	if (CPC_GetHardware() == CPC_HW_CPCPLUS)
	{
		nMode = GateArray_GetMultiConfiguration() & 0x03;
	}
	else if (CPC_GetHardware() == CPC_HW_CPCPLUS)
	{
		nMode = ASIC_GateArray_GetMultiConfiguration() & 0x03;
	}
	else if (CPC_GetHardware() == CPC_HW_KCCOMPACT)
	{
		nMode = KCC_GetMF() & 0x03;
	}
	else if (CPC_GetHardware() == CPC_HW_ALESTE)
	{
		nMode = Aleste_GetMF() & 0x03;
	}
#endif
}


void GraphicsEditorWindow::CaptureScreenBase()
{
	int nBaseHigh = CRTC_GetRegisterData(12);
	int nBaseLow = CRTC_GetRegisterData(13);
	int nAddress = ((nBaseHigh & 0x030) << 10) | ((nBaseHigh & 0x03) << 9) | ((nBaseLow & 0x0ff) << 1);

	m_nAddress = nAddress;
}

void GraphicsEditorWindow::CaptureCRTC()
{
	// get number of chars displayed in width * 2
	m_nPlotWidth = CRTC_GetRegisterData(1) << 1;
	// number of scanlines per character + 1
	nLinesPerChar = CRTC_GetRegisterData(9) + 1;

	CaptureScreenBase();
}

// capture the palette we want to use
void GraphicsEditorWindow::CapturePalette(int nPalette)
{
	wxColour colour;
	for (int i = 0; i < 16; i++)
	{
		// TODO: KCC/Aleste
		if (CPC_GetHardware() == CPC_HW_CPCPLUS)
		{
			if (nPalette == 1)
			{
				if (i == 0)
				{
					// sprite transparent pen (use main pen 0)
					colour.Set((ASIC_GetRed(i) << 4), ASIC_GetGreen(i) << 4, ASIC_GetBlue(i) << 4);

				}
				else
				{
					// sprite
					colour.Set((ASIC_GetRed(i+16)<<4),ASIC_GetGreen(i+16)<<4,ASIC_GetBlue(i+16)<<4);
				}
			}
			else
			{
				// main
				colour.Set((ASIC_GetRed(i) << 4), ASIC_GetGreen(i) << 4, ASIC_GetBlue(i) << 4);
			}
				}
		else
		{
			// cpc
			//
			// need to implement same for kcc/aleste

			colour.Set((GateArray_GetRed(i)),
				(GateArray_GetGreen(i)),
				(GateArray_GetBlue(i)));
		}
#if (wxVERSION_NUMBER >= 3100)

		Brushes[i] = wxBrush(colour, wxBRUSHSTYLE_SOLID);
#else
		Brushes[i] = wxBrush(colour, wxSOLID);

#endif
		Pens[i] = wxPen(colour);
		Colours[i] = colour;

			}
		}

//void GraphicsEditorWindow::OnSetMode(wxCommandEvent & WXUNUSED(event))
//{
//	CaptureMode();
//}


void GraphicsEditorWindow::OnSetAddress(wxCommandEvent & WXUNUSED(event))
{
	wxString sValue;
	wxString sConfigKey = wxT("graphics_editor/go_to_address");
	wxConfig::Get(false)->Read(sConfigKey, &sValue);

	wxString sMessage = wxT("Please enter an expression to evaluate");
	wxTextEntryDialog dialog(this, sMessage, wxT(""), wxT(""), wxOK | wxCANCEL);
	dialog.SetValue(sValue);
	if (dialog.ShowModal() == wxID_OK)
	{
		wxString str = dialog.GetValue();
		if (str.Length() != 0)
		{
			wxConfig::Get(false)->Write(sConfigKey, str);

			m_nAddress = wxGetApp().ExpressionEvaluate(str);

			m_bRefreshRequired = true;
		}
	}
}

void GraphicsEditorWindow::OnDecreaseWidth(wxCommandEvent & WXUNUSED(event))
{
	DecreaseWidth();
}


void GraphicsEditorWindow::OnIncreaseWidth(wxCommandEvent & WXUNUSED(event))
{
	IncreaseWidth();
}

void GraphicsEditorWindow::OnPageUp(wxCommandEvent & WXUNUSED(event))
{
	PageUp();
}


void GraphicsEditorWindow::OnPageDown(wxCommandEvent & WXUNUSED(event))
{
	PageDown();
}

void GraphicsEditorWindow::OnSetMainPalette(wxCommandEvent & WXUNUSED(event))
{
	m_nPaletteId = 0;
	CapturePalette(m_nPaletteId);
}

void GraphicsEditorWindow::OnSetSpritePalette(wxCommandEvent & WXUNUSED(event))
{
	m_nPaletteId = 1;
	CapturePalette(m_nPaletteId);
}

void GraphicsEditorWindow::IncreaseWidth()
{
	m_nPlotWidth++;
	if (m_nPlotWidth > 256)
	{
		m_nPlotWidth = 256;
	}

	m_bRefreshRequired = true;

}


void GraphicsEditorWindow::DecreaseWidth()
{
	m_nPlotWidth--;
	if (m_nPlotWidth < 1)
	{
		m_nPlotWidth = 1;
	}

	m_bRefreshRequired = true;
}

void GraphicsEditorWindow::PageUp()
{
	int nWidth, nHeight;
	this->GetClientSize(&nWidth, &nHeight);
	int nPage = (nHeight / m_nYRatio)*m_nPlotWidth;

	m_nAddress = IncAddress(m_nAddress, -nPage);

	m_bRefreshRequired = true;
}

void GraphicsEditorWindow::PageDown()
{
	int nWidth, nHeight;
	this->GetClientSize(&nWidth, &nHeight);
	int nPage = (nHeight / m_nYRatio)*m_nPlotWidth;

	m_nAddress = IncAddress(m_nAddress, nPage);
	m_bRefreshRequired = true;
}


int GraphicsEditorWindow::IncAddress(int Address, int Delta)
{
	if (m_nLineMethod == LINE_METHOD_LINEAR)
	{
		Address += Delta;
		Address &= 0x0ffff;
	}
	else
	{
		int nPage = (Address >> 14);
		int nOffset = (Address & 0x07ff);
		int nLines = (Address & 0x03800);

		nOffset += (Delta % 2048);
		nPage += (Delta / 2048);
		nPage &= 0x03;
		nOffset &= 0x07ff;
		Address = (nPage << 14) | nOffset | nLines;
	}
	return Address;
}

void GraphicsEditorWindow::OnKeyDown(wxKeyEvent &event)
{
	switch (event.GetKeyCode())
	{
	default:
		break;

	case WXK_ESCAPE:
		GetParent()->Close();
		break;

	case WXK_NUMPAD_LEFT:
	case WXK_LEFT:
	{
		if (event.GetModifiers() == 0)
		{
			m_nAddress = IncAddress(m_nAddress, -1);

			m_bRefreshRequired = true;
			return;
		}
		else if (event.GetModifiers() == wxMOD_CONTROL)
		{
			DecreaseWidth();
		}
	}
	break;

	case WXK_NUMPAD_ADD:
	{
		if (event.GetModifiers() == 0)
		{
			IncreaseWidth();
			return;
		}
	}
	break;

	case WXK_NUMPAD_SUBTRACT:
	{
		if (event.GetModifiers() == 0)
		{
			DecreaseWidth();
			return;
		}
	}
	break;

	case WXK_NUMPAD_RIGHT:
	case WXK_RIGHT:
	{
		if (event.GetModifiers() == 0)
		{
			m_nAddress = IncAddress(m_nAddress, 1);

			m_bRefreshRequired = true;
			return;
		}
		else if (event.GetModifiers() == wxMOD_CONTROL)
		{
			IncreaseWidth();
		}
	}
	break;

	case WXK_NUMPAD_UP:
	case WXK_UP:
	{
		if (event.GetModifiers() == 0)
		{
			m_nAddress = IncAddress(m_nAddress, -m_nPlotWidth);
			m_bRefreshRequired = true;
			return;
		}
		else
		{
			if (event.GetModifiers() == wxMOD_CONTROL)
			{
				PageUp();
			}
		}
	}
	break;

	case WXK_NUMPAD_DOWN:
	case WXK_DOWN:
	{
		if (event.GetModifiers() == 0)
		{
			m_nAddress = IncAddress(m_nAddress, m_nPlotWidth);
			m_bRefreshRequired = true;
			return;
		}
		else
		{
			if (event.GetModifiers() == wxMOD_CONTROL)
			{
				PageDown();
			}
		}
	}
	break;

	case WXK_NUMPAD_PAGEUP:
	case WXK_PAGEUP:
	{
		if (event.GetModifiers() == 0)
		{
			PageUp();

			return;
		}
	}
	break;

	case WXK_NUMPAD_PAGEDOWN:
	case WXK_PAGEDOWN:
	{
		if (event.GetModifiers()==0)
		{
			PageDown();
			return;
		}
	}
	break;

	}
	event.Skip();

}

#if 0
void GraphicsEditor::OnChoiceChange(wxCommandEvent & WXUNUSED(event))
{
	wxChoice *pChoice;
	GraphicsEditorWindow *pGraphicsEditorWindow;

	pChoice = XRCCTRL(*this, "m_MethodChoice", wxChoice);

	int nMethod = pChoice->GetSelection();

	pGraphicsEditorWindow = XRCCTRL(*this, "m_GraphicsEditorWindow", GraphicsEditorWindow);

	pGraphicsEditorWindow->m_nDisplayMethod = nMethod;
	pGraphicsEditorWindow->m_bRefreshRequired = true;
}

void GraphicsEditor::OnLineChoiceChange(wxCommandEvent & WXUNUSED(event))
{
	wxChoice *pChoice;
	GraphicsEditorWindow *pGraphicsEditorWindow;

	pChoice = XRCCTRL(*this, "m_LineMethodChoice", wxChoice);

	int nMethod = pChoice->GetSelection();

	pGraphicsEditorWindow = XRCCTRL(*this, "m_GraphicsEditorWindow", GraphicsEditorWindow);

	pGraphicsEditorWindow->m_nLineMethod = nMethod;

	if (nMethod==1)
	{
		// we switched to crtc view

		// we adjust the start so it starts on a character boundary; otherwise it becomes impossible to view screen nicely.
		pGraphicsEditorWindow->m_nAddress &= ~0x03800;

	}

	pGraphicsEditorWindow->m_bRefreshRequired = true;
	}

void GraphicsEditor::OnSpinChange(wxSpinEvent &event)
{
	wxWindow *pWindow;
	GraphicsEditorWindow *pGraphicsEditorWindow;

	pGraphicsEditorWindow = XRCCTRL(*this, "m_GraphicsEditorWindow", GraphicsEditorWindow);

	pGraphicsEditorWindow->m_nPlotWidth = event.GetPosition();
	pGraphicsEditorWindow->m_bRefreshRequired = true;
}
#endif

void GraphicsEditorWindow::OnContextMenu(wxContextMenuEvent &event)
{
	wxPoint Position = event.GetPosition();

	// generate a suitable place for the menu
	if (event.GetPosition() == wxDefaultPosition)
	{
		wxPoint ClientPos(0, 0);
		Position = this->ClientToScreen(ClientPos);
	}

	wxMenuBar *pMenuBar = wxXmlResource::Get()->LoadMenuBar(wxT("MB_MENU_GRAPHICS_VIEWER_MENU_BAR"));
	if (pMenuBar)
	{
		wxMenu *pMenu = pMenuBar->Remove(0);
		if (pMenu)
		{

			PopupMenu(pMenu);
			delete pMenu;
		}
		delete pMenuBar;
	}
}


void GraphicsEditorWindow::OnSetMemoryRange(wxCommandEvent & WXUNUSED(event))
{
	wxArrayString sRanges;
	for (int i = 0; i < CPC_GetRegisteredMemoryRangeCount(); i++)
	{
		wxString sDrive;
		const MemoryRange *pRange = CPC_GetRegisteredMemoryRange(i);
		if (pRange)
		{
			if (pRange->sName)
			{
				sDrive = wxString::FromAscii(pRange->sName);
				sRanges.Add(sDrive);
			}
		}
	}

	wxString sMessage;
	wxSingleChoiceDialog dialog(this, wxT("Memory range to use:"), wxT("Choose memory range"), sRanges);
	dialog.SetSelection(0);
	if (dialog.ShowModal() == wxID_OK)
	{
		int nRange = dialog.GetSelection();
		const MemoryRange *pRange = CPC_GetRegisteredMemoryRange(nRange);
		m_pRange = pRange;
	}
	m_bRefreshRequired = true;
}

void GraphicsEditorWindow::CreateImage(int nWidth, int nHeight)
{
	if (m_Image.IsOk())
	{
		m_Image.Destroy();
	}

	// non-scaled image size
	m_nImageWidth = (nWidth + (m_nXRatio - 1)) / m_nXRatio;
	m_nImageHeight = (nHeight + (m_nYRatio - 1)) / m_nYRatio;

	m_Image.Create(m_nImageWidth, m_nImageHeight, false);

	m_bResizeRequired = false;
	m_bRefreshRequired = true;
}

GraphicsEditorWindow::GraphicsEditorWindow() : wxWindow(), m_nDisplayMethod(0), m_nPlotWidth(8), m_nAddress(0), m_nCurrentMode(0), m_nZoom(0), m_nXRatio(1), m_nYRatio(1), m_bReadOnly(false)
{
	m_nPaletteId = 0;
	m_nHeight = -1;
	m_bResizeRequired = true;
	m_bRepeatRefreshPixels = false;
	m_bRepeatRefreshPalette = false;
	m_bShowTransparency = false;
	m_bFlipVertically = false;
	m_bFlipHorizontally = false;
	m_bTransparencyUsesPen = false;
	m_bTransparencyUsesInterleavedMask = false;
	m_nTransparentPen = 0;	// where transparency is based on pen use this
	m_pRange = CPC_GetDefaultMemoryRange();
	m_nXRatio = 8;
	m_nYRatio = 8;
	m_nLineMethod = 0;
	nLinesPerChar = 8;

	m_BackgroundBrushTransparent= new wxBrush(*wxBLACK, wxBRUSHSTYLE_CROSS_HATCH);
	m_BackgroundBrushNoPixels = new wxBrush(*wxBLACK, wxBRUSHSTYLE_FDIAGONAL_HATCH);
	CapturePalette(m_nPaletteId);
}

bool GraphicsEditorWindow::Create(wxWindow* parent, wxWindowID id, const wxString & WXUNUSED(label), const wxPoint& pos, const wxSize& size, long style, const wxValidator & WXUNUSED(validator), const wxString& name)
{
	style |= wxNO_FULL_REPAINT_ON_RESIZE;
	style |= wxCLIP_CHILDREN;
	style |= wxWANTS_CHARS;
	//	bool bResult = wxWindow::Create(parent, id, pos, size, style, name);
	//	if (bResult)
	//   {
	//      SetBackgroundStyle(wxBG_STYLE_CUSTOM);
	// }


	bool bResult = wxWindow::Create(parent, id, pos, size, style, name);
	if (bResult)
	{
		SetBackgroundStyle(wxBG_STYLE_CUSTOM);
	}
	return bResult;
}


GraphicsEditorWindow::~GraphicsEditorWindow()
{
	if (m_Image.IsOk())
	{
		m_Image.Destroy();
	}
	if (m_BackgroundBrushNoPixels)
	{
		delete m_BackgroundBrushNoPixels;
	}
	if (m_BackgroundBrushTransparent)
	{
		delete m_BackgroundBrushTransparent;
	}
}

void GraphicsEditorWindow::OnMouseWheel(wxMouseEvent &event)
{
	int Delta = event.GetWheelDelta();
	if (Delta == 0)
		return;

	int Movement = event.GetWheelRotation();
	if (Movement == 0)
		return;

	int nAmount = Movement / Delta;

	if (event.ControlDown())
	{
		m_nXRatio += nAmount;
		if (m_nXRatio <= 1)
		{
			m_nXRatio = 1;
		}
		else if (m_nXRatio > 16)
		{
			m_nXRatio = 16;
		}
	}
	else
	{
		int nAddressDelta = m_nPlotWidth*nAmount;
		m_nAddress+=nAddressDelta;
		m_nAddress&=0x0ffff;
	}

	this->m_bRefreshRequired = true;
}

//void GraphicsEditorWindow::OnEraseBackground(wxEraseEvent &event)
//{
//
//}

#if 0
for (j = 0; j < 2; j++)
{
	for (i = 0; i < 8; i++)
	{
		//			pSpriteData = ASIC_GetSpriteDataAddr((j<<3)+i);

		for (y = 0; y < 16; y++)
		{
			for (x = 0; x < 16; x++)
			{

				BitmapData[((1 - j)*(pBitmapInfo->bmiHeader.biWidth * 16)) + (i * 16) + (((15 - y)*pBitmapInfo->bmiHeader.biWidth) + x)] = ASIC_GetSpritePixel((j << 3) + i, x, y);
			}
		}
	}
}
#endif
// use current palette and current mode

void GraphicsEditorWindow::Init()
{
	CapturePalette(m_nPaletteId);
}

void GraphicsEditorWindow::DrawBlock(wxAutoBufferedPaintDC &dc, int nPen)
{

	wxPoint pt;
	wxSize sz;
	//printf("x: %d y: %d ratx: %d raty: %d nPen: %d\n", x,y,m_nXRatio, m_nYRatio, nPen);
	pt.x = x;
	pt.y = y;
	sz.x = m_nXRatio;
	sz.y = m_nYRatio;

	// we may need need to decide if the colour has changed
	// may also be better to batch based on pen?
	dc.SetBrush(Brushes[nPen]);
	dc.SetPen(Pens[nPen]);
	dc.DrawRectangle(pt, sz);
	x += m_nXRatio;

}

void GraphicsEditorWindow::SkipBlock()
{
	x += m_nXRatio;
}

void GraphicsEditorWindow::ProcessByte(wxAutoBufferedPaintDC &dc, int CurrentAddress, int nIndex)
{
	int mask = 0;
	int data = 0;
	if (m_bTransparencyUsesInterleavedMask)
	{
		mask = MemoryRange_ReadByte(m_pRange, CurrentAddress);
	}
	data = MemoryRange_ReadByte(m_pRange, CurrentAddress+1);

	switch (m_nDisplayMethod)
	{

	case METHOD_1BPP:                           // mode 2
	{
		for (int i = 0; i < 8; i++)
		{
			// bit 7
			int nMask = (mask >> 7) & 0x01;
			int nPen = (data >> 7) & 0x01;
			if (
				(!(
				(m_bShowTransparency && m_bTransparencyUsesPen && (m_nTransparentPen == nPen)) ||
					(m_bShowTransparency && m_bTransparencyUsesInterleavedMask && (nMask == 0))
					))
				)
			{
				DrawBlock(dc, nPen);
			}
			else
			{
				SkipBlock();
			}
			// shift up data
			data = data << 1;
			mask = mask << 1;
		}
	}
	break;

	case METHOD_2BPP_INTERLEAVED: // mode 1
	{
		for (int i = 0; i < 4; i++)
		{
			int nMask = ((mask >> 7) & 0x01) | (((mask >> 3) << 1) & 0x02);
			int nPen = ((data >> 7) & 0x01) | (((data >> 3) << 1) & 0x02);
			if (
				(!(
				(m_bShowTransparency && m_bTransparencyUsesPen && m_nTransparentPen == nPen) || 
				(m_bShowTransparency && m_bTransparencyUsesInterleavedMask && (nMask==0))
					))
				)
			{
				DrawBlock(dc, nPen);
			}
			else
			{
				SkipBlock();
			}

			data = data << 1;
			mask = mask << 1;
		}

	}
	break;


	case METHOD_2BPP_INTERLEAVED_MODE3: // mode 3
	{
		for (int i = 0; i < 2; i++)
		{
			int nMask = ((mask >> 7) & 0x01) | (((mask >> 3) << 1) & 0x02);
			int nPen = ((data >> 7) & 0x01) | (((data >> 3) << 1) & 0x02);
			if (
				(!(
				(m_bShowTransparency && m_bTransparencyUsesPen && (m_nTransparentPen == nPen)) ||
					(m_bShowTransparency && m_bTransparencyUsesInterleavedMask && (nMask == 0))
					))
				)
			{
				DrawBlock(dc, nPen);
			}
			else
			{
				SkipBlock();
			}

			data = data << 1;
			mask = mask << 1;
		}
	}
	break;

	case METHOD_2BPP_LINEAR: // mode 1
	{
		for (int i = 0; i < 4; i++)
		{
			int nPen = (data >> 6) & 0x03;
			if (!(m_bShowTransparency && m_bTransparencyUsesPen && m_nTransparentPen == nPen))
			{
				DrawBlock(dc, nPen);
			}
			else
			{
				SkipBlock();
			}

			data = data << 2;
		}

	}
	break;


	case METHOD_4BPP_INTERLEAVED: // mode 0
	{
		for (int i = 0; i < 2; i++)
		{
			int nMask = ((mask >> 7) & 0x01) | (((mask >> 3) << 1) & 0x02) | (((mask >> 5) << 2) & 0x04) | (((mask >> 1) << 3) & 0x08);
			int nPen = ((data >> 7) & 0x01) | (((data >> 3) << 1) & 0x02) | (((data >> 5) << 2) & 0x04) | (((data >> 1) << 3) & 0x08);
			if (
				(!(
				(m_bShowTransparency && m_bTransparencyUsesPen && m_nTransparentPen == nPen) ||
					(m_bShowTransparency && m_bTransparencyUsesInterleavedMask && (nMask == 0))
					))
				)
			{
				DrawBlock(dc, nPen);
			}
			else
			{
				SkipBlock();
			}

			data = data << 1;
			mask = mask << 1;
		}

	}
	break;


	case METHOD_4BPP_LINEAR: // mode 0 (not interleaved)
	{
		for (int i = 0; i < 2; i++)
		{
			int nPen = (data >> 4) & 0x0f;
			if (!(m_bShowTransparency && m_bTransparencyUsesPen && m_nTransparentPen == nPen))
			{
				DrawBlock(dc, nPen);
			}
			else
			{
				SkipBlock();
			}

			data = data << 4;
		}

	}
	break;


	case METHOD_4BPP_RENEGADE: // renegade/gryzor special
	{
		if ((nIndex & 1) == 0)
		{

			for (int i = 0; i < 2; i++)
			{

				int nPen = ((data >> 7) & 0x01) | (((data >> 3) << 1) & 0x02) | (((data >> 5) << 2) & 0x04) | (((data >> 1) << 3) & 0x08);
				if (!(m_bShowTransparency && m_bTransparencyUsesPen && m_nTransparentPen == nPen))
				{
					DrawBlock(dc, nPen);
				}
				else
				{
					SkipBlock();
				}

				data = data << 1;
			}



		}
		else
		{

			for (int i = 0; i < 2; i++)
			{
				int nPen = ((data >> 6) & 0x01) | (((data >> 2) << 1) & 0x02) | (((data >> 4) << 2) & 0x04) | (((data >> 0) << 3) & 0x08);
				if (!(m_bShowTransparency && m_bTransparencyUsesPen && m_nTransparentPen == nPen))
				{
					DrawBlock(dc, nPen);
				}
				else
				{
					SkipBlock();
				}

				data = data >> 1;
				//                  data = data<<1;
			}
		}
	}
	break;


	case METHOD_PLUS_SPRITE_PIXELS: 
	{
		int nPen = data & 0x0f;
		if (!(m_bShowTransparency && m_bTransparencyUsesPen && m_nTransparentPen == nPen))
		{
			DrawBlock(dc, nPen);
		}
		else
		{
			SkipBlock();
		}

	}
	break;
	}
}

int GraphicsEditorWindow::PixelsPerByte()
{
	switch (m_nDisplayMethod)
	{

	case METHOD_1BPP:                           // mode 2
		return 8;

	case METHOD_2BPP_INTERLEAVED: // mode 1
		return 4;


	case METHOD_2BPP_INTERLEAVED_MODE3: // mode 3
		return 2;

	case METHOD_2BPP_LINEAR: // mode 1
		return 4;


	case METHOD_4BPP_INTERLEAVED: // mode 0
		return 2;

	case METHOD_4BPP_LINEAR: // mode 0 (not interleaved)
		return 2;


	case METHOD_4BPP_RENEGADE: // renegade/gryzor special
		return 2;


	case METHOD_PLUS_SPRITE_PIXELS:
		return 1;
	}
	return 0;
}


void GraphicsEditorWindow::RefreshImage()
{

	m_Bitmap = wxBitmap(m_Image);

	m_bRefreshRequired = false;
}

void GraphicsEditorWindow::OnPaint(wxPaintEvent & WXUNUSED(event))
{
	if (m_bRepeatRefreshPalette)
	{
		CapturePalette(m_nPaletteId);
	}


	//    printf("Painting graphics window\r\n");

	wxAutoBufferedPaintDC dc(this);

	dc.SetBackground(wxBrush(*wxWHITE, wxBRUSHSTYLE_SOLID));
	dc.Clear();

	int nWidth, nHeight;
	this->GetClientSize(&nWidth, &nHeight);
	
	x = 0;
	y = 0;

	int nCoveredWidth = m_nXRatio*(m_nPlotWidth*PixelsPerByte());
	if (m_bShowTransparency)
	{
		wxPoint pt;
		pt.x = 0;
		pt.y = 0;
		wxSize sz;
		sz.x = nCoveredWidth;
		sz.y = nHeight;

		dc.SetBrush(*m_BackgroundBrushTransparent);
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.DrawRectangle(pt, sz);
	}

	if (nCoveredWidth < nWidth)
	{
		wxPoint pt;
		pt.x = nCoveredWidth;
		pt.y = 0;
		
		wxSize sz;
		sz.x = nWidth - nCoveredWidth;
		sz.y = nHeight;

		dc.SetBrush(*m_BackgroundBrushNoPixels);
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.DrawRectangle(pt, sz);
	}

#if 1
	int nLinesVertically = nHeight / m_nYRatio;
	int nBytesHorizontally = m_nPlotWidth;
	// 15 with this method
	int CurrentAddress = m_bFlipVertically ? this->m_nAddress+(nBytesHorizontally*(nLinesVertically-1)) : this->m_nAddress;
	int StoreAddress = CurrentAddress;
	int R = 0;



	while (y < nHeight)
	{
		CurrentAddress = StoreAddress;
		x = 0;
		int nByte = 0;

		// up to the number of bytes we want to draw and restrict to the width of the window
		while ((nByte < m_nPlotWidth) && (x < nWidth))
		{
			int Inc = m_bTransparencyUsesInterleavedMask ? 2 : 1;
			ProcessByte(dc, CurrentAddress, nByte);
			CurrentAddress = IncAddress(CurrentAddress, Inc);
			++nByte;
		}

		y += m_nYRatio;
		if (m_nLineMethod == LINE_METHOD_LINEAR)
		{
			int Inc = m_bTransparencyUsesInterleavedMask ? m_nPlotWidth << 1 : m_nPlotWidth;
			if (m_bFlipVertically)
			{
				Inc = -Inc;
			}
			StoreAddress = IncAddress(StoreAddress, Inc);
		}
		else
		{
			// todo: flipvertically/flip horizontally
			R++;
			if (R >= nLinesPerChar)
			{
				R = 0;
				StoreAddress = IncAddress(StoreAddress - ((nLinesPerChar - 1) * 0x0800), m_nPlotWidth);
			}
			else
			{
				StoreAddress+=0x0800;
			}
		}
	}

	if (m_nHeight!=-1)
	{
		y = 0;
		while (y<m_nHeight)
		{
			dc.DrawLine(0,y,nWidth, y);


			y += m_nHeight;
			}



		}
#else
	if (m_bResizeRequired)
	{
		CreateImage(nWidth, nHeight);
	}
	if (m_bRefreshRequired)
	{
		RefreshImage();
	}
	dc.SetUserScale(8, 8);
	dc.DrawBitmap(m_Bitmap, 0, 0, false);
#endif
	}


wxObject *GraphicsEditorResourceHandler::DoCreateResource()
{

	XRC_MAKE_INSTANCE(control, GraphicsEditorWindow)


		control->Create(m_parentAsWindow,
		GetID(),
		GetText(wxT("label")),
		GetPosition(), GetSize(),
		GetStyle(),
		wxDefaultValidator,
		GetName());
	if (control)
	{
		SetupWindow(control);
	}
	return control;
}


bool GraphicsEditorResourceHandler::CanHandle(wxXmlNode *node)
{
	return IsOfClass(node, wxT("GraphicsEditor"));
}


GraphicsEditorResourceHandler::GraphicsEditorResourceHandler() : wxXmlResourceHandler()
{


}

GraphicsEditorResourceHandler::~GraphicsEditorResourceHandler()
{

}


GraphicsEditor *GraphicsEditor::m_pInstance = NULL;

// creator
GraphicsEditor *GraphicsEditor::CreateInstance(wxWindow *pParent)
{
	if (m_pInstance == NULL)
	{
		m_pInstance = new GraphicsEditor(pParent);
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



//void GraphicsEditor::Init()
//{
//	Debug_SetGraphicsEditorOpenCallback(OpenWindowCallback);
//}

//void GraphicsEditor::OpenWindowCallback(void)
//{
//	GraphicsEditor::CreateInstance(wxGetApp().GetTopWindow());
//}

GraphicsEditor::~GraphicsEditor()
{

}

BEGIN_EVENT_TABLE(GraphicsEditor, wxDialog)
//EVT_CHOICE(XRCID("m_MethodChoice"), GraphicsEditor::OnChoiceChange)
//EVT_CHOICE(XRCID("m_LineMethodChoice"), GraphicsEditor::OnLineChoiceChange)
//EVT_SPINCTRL(XRCID("m_spinWidth"), GraphicsEditor::OnSpinChange)
EVT_CLOSE(GraphicsEditor::OnClose)
END_EVENT_TABLE()

void GraphicsEditor::OnClose(wxCloseEvent & WXUNUSED(event))
{
	arnguiFrame *pFrame = (arnguiFrame *)GetParent();
	pFrame->DeRegisterWantUpdateFromTimer(this);

	wxGetApp().WriteConfigWindowPosAndSize(wxT("windows/graphics_editor/"), this);
	this->Destroy();

	GraphicsEditor::m_pInstance = NULL;
}

GraphicsEditor::GraphicsEditor(wxWindow *pParent)
{
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("GraphicsDialog"));

	// default size
	//  this->SetSize(800,800);

	wxGetApp().ReadConfigWindowPosAndSize(wxT("windows/graphics_editor/"), this);
	wxGetApp().EnsureWindowVisible(this);

	arnguiFrame *pFrame = (arnguiFrame *)GetParent();
	pFrame->RegisterWantUpdateFromTimer(this);

}

void GraphicsEditor::TimedUpdate()
{
	GraphicsEditorWindow *pGraphicsEditorWindow;

	pGraphicsEditorWindow = XRCCTRL(*this, "m_GraphicsEditorWindow", GraphicsEditorWindow);

	if (pGraphicsEditorWindow != NULL)
	{
		// if timed updates are not enabled AND we are not requesting a refresh
		// don't do it.
		if (
			(!pGraphicsEditorWindow->m_bRepeatRefreshPixels) &&
			(!pGraphicsEditorWindow->m_bRepeatRefreshPalette) &&
			(!pGraphicsEditorWindow->m_bRefreshRequired)
			)
		{
			return;
		}

		pGraphicsEditorWindow->Refresh();
		// we've done the refresh
		pGraphicsEditorWindow->m_bRefreshRequired = false;
	}
}

