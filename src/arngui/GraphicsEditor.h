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
#ifndef __GRAPHICS_DIALOG_HEADER_INCLUDED__
#define __GRAPHICS_DIALOG_HEADER_INCLUDED__

#include <wx/dialog.h>
//#include <wx/listctrl.h>
#include <wx/dcbuffer.h>
//#include "EmuFileType.h"
#include "UpdatableDialog.h"
//#include <wx/dynarray.h>
//#include <wx/listctrl.h>
#include <wx/xrc/xmlres.h>
//#include "ItemData.h"
#include <wx/scrolwin.h>
#include <wx/spinctrl.h>
#include <wx/image.h>
#include <wx/bitmap.h>


#ifndef wxOVERRIDE
#define wxOVERRIDE
#endif

extern "C"
{
#include "../cpc/memrange.h"
#include "../cpc/cpc.h"
}

// TODO: Capture current palette and retain it so colours don't change

enum
{
	METHOD_1BPP,                           // mode 2
	METHOD_2BPP_INTERLEAVED,    //  mode 1
	METHOD_2BPP_INTERLEAVED_MODE3, // mode 3
	METHOD_2BPP_LINEAR,
	METHOD_4BPP_INTERLEAVED,    //  mode 0
	METHOD_4BPP_LINEAR,             // can be compressed plus sprite pixels
	METHOD_PLUS_SPRITE_PIXELS,
	METHOD_4BPP_RENEGADE
};

enum
{
	LINE_METHOD_LINEAR,
	LINE_METHOD_CRTC
};
class GraphicsEditorWindow : public wxWindow
{
public:
	GraphicsEditorWindow();
	~GraphicsEditorWindow();
	bool m_bRepeatRefreshPixels;
	bool m_bRepeatRefreshPalette;
	bool    m_bRefreshRequired;
	bool m_bShowTransparency;
	bool m_bTransparencyUsesPen;
	bool m_bTransparencyUsesInterleavedMask;
	bool m_bFlipVertically;
	bool m_bFlipHorizontally;
	virtual bool Create(wxWindow* parent, wxWindowID id, const wxString &label, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxValidator &validator = wxDefaultValidator, const wxString& name = wxPanelNameStr);
	void Init();
	// display method
	int m_nDisplayMethod;
	int m_nLineMethod;
	int m_nPlotWidth;
	int m_nAddress;
	int m_nTransparentPen;
	int nLinesPerChar;
protected:
	void ProcessByte(wxAutoBufferedPaintDC &dc, int Address, int nByte);
	int IncAddress(int nAddress, int nDelta);
	void RefreshImage();
	void CreateImage(int nWidth, int nHeight);
	wxBrush Brushes[16];
	wxPen Pens[16];
	wxColour Colours[16];
	void OnSize(wxSizeEvent &event);

	void DrawBlock(wxAutoBufferedPaintDC &dc, int nPen);
	void SkipBlock();
	const MemoryRange *m_pRange;
	int x;
	int y;
	// mode used for display
	int m_nCurrentMode;
	// zoom
	int m_nZoom;

	int m_nXRatio;
	int m_nYRatio;
	int m_nHeight;
	int m_nPaletteId;

	// true if not editable
	bool m_bReadOnly;
	void OnSetMemoryRange(wxCommandEvent &event);
	void OnSelectConfig(wxCommandEvent &event);
	void OnContextMenu(wxContextMenuEvent &event);
	void OnKeyDown(wxKeyEvent &event);
	void OnSetMainPalette(wxCommandEvent &event);
	void OnSetSpritePalette(wxCommandEvent &event);
	void OnSetRepeatedRefreshPixels(wxCommandEvent &event);
	void OnSetRepeatedRefreshPalette(wxCommandEvent &event);
	void OnFlipVertically(wxCommandEvent &event);
	void OnSetAddress(wxCommandEvent &event);
	void OnIncreaseWidth(wxCommandEvent &event);
	void OnDecreaseWidth(wxCommandEvent &event);
	void OnPageUp(wxCommandEvent &event);
	void OnPageDown(wxCommandEvent &event);
	void OnSetShowTransparency(wxCommandEvent & WXUNUSED(event));
	void OnUpdateShowTransparency(wxUpdateUIEvent &event);
	void IncreaseWidth();
	void DecreaseWidth();
	void PageUp();
	void PageDown();

	//void OnSetMode(wxCommandEvent & event);
	//void OnSetScreenBase(wxCommandEvent & event);
	void OnUpdateRepeatedRefreshPixels(wxUpdateUIEvent &event);
	void OnUpdateRepeatedRefreshPalette(wxUpdateUIEvent &event);
	void OnUpdateFlipVertically(wxUpdateUIEvent &event);
	void OnSetPreset(int);
	void OnPaint(wxPaintEvent &event);
	//   void OnSize(wxSizeEvent &event);
	//  void OnKeyDown(wxKeyEvent &event);
	void OnMouseWheel(wxMouseEvent &event);

	//   void OnMouseLeftButtonDown(wxMouseEvent &event);
	//  void OnContextMenu(wxContextMenuEvent &event);
	//void OnSetFocus(wxFocusEvent &event);
	//void OnKillFocus(wxFocusEvent &event);
	//void OnEraseBackground(wxEraseEvent &event);
	DECLARE_EVENT_TABLE()
	void CaptureMode();
	void CaptureScreenBase();
	void CaptureCRTC();
	void CapturePalette(int);
	int PixelsPerByte();

	wxBitmap m_Bitmap;
	wxImage m_Image;
	bool    m_bResizeRequired;
	int m_nImageWidth;
	int m_nImageHeight;
	int m_nWidth;
	wxBrush *m_BackgroundBrushNoPixels;
	wxBrush *m_BackgroundBrushTransparent;
};


class GraphicsEditorResourceHandler : public wxXmlResourceHandler
{
public:
	GraphicsEditorResourceHandler();
	~GraphicsEditorResourceHandler();

	virtual wxObject *DoCreateResource();
	virtual bool CanHandle(wxXmlNode *node);
};
class GraphicsEditor : public wxDialog, public UpdatableDialog
{
	GraphicsEditor(wxWindow *pParent);
	~GraphicsEditor();

	static GraphicsEditor *m_pInstance;

	static void OpenWindowCallback(void);

public:
	virtual void TimedUpdate() wxOVERRIDE;

	static void Init();

	// creator
	static GraphicsEditor *CreateInstance(wxWindow *pParent);

protected:
#if 0
	void OnSpinChange(wxSpinEvent &event);
	void OnChoiceChange(wxCommandEvent &event);
	void OnLineChoiceChange(wxCommandEvent &event);
#endif
	void OnClose(wxCloseEvent &event);
	DECLARE_EVENT_TABLE()

};

#endif
