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
#ifndef __DEBUGGER_DIALOG_HEADER_INCLUDED__
#define __DEBUGGER_DIALOG_HEADER_INCLUDED__


#include <wx/dialog.h>
#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/dcbuffer.h>
#include "EmuFileType.h"
#include "UpdatableDialog.h"
//#include <wx/dynarray.h>
#include <wx/listctrl.h>
#include <wx/xrc/xmlres.h>

#ifndef wxOVERRIDE
#define wxOVERRIDE
#endif

#include "ItemData.h"

extern "C"
{

#include "../cpc/debugger/memdump.h"
#include "../cpc/debugger/dissasm.h"
#include "../cpc/debugger/stack.h"
#include "../cpc/debugger/breakpt.h"
#include "../cpc/debugger/gdebug.h"
}

class ConvertAddressToolDialog : public wxDialog
{
public:
	ConvertAddressToolDialog(wxWindow *pParent);
	~ConvertAddressToolDialog();

	virtual bool TransferDataFromWindow() wxOVERRIDE;
	virtual bool TransferDataToWindow() wxOVERRIDE;

	// creator
	static ConvertAddressToolDialog *CreateInstance(wxWindow *pParent);


private:

	static void Init();
	static ConvertAddressToolDialog *m_pInstance;

	static void OpenWindowCallback(void);

	void OnClose(wxCloseEvent &event);
	void OnConvertRange1ToRange2(wxCommandEvent &event);
	void OnConvertRange2ToRange1(wxCommandEvent &event);
	void ConvertRangeToRange(long RangeFromStartId, long RangeFromEndId, long RangeToStartId, long RangeToEndId);

	DECLARE_EVENT_TABLE()
};

class BreakpointDialog : public wxDialog
{
public:
	BreakpointDialog(BREAKPOINT_MAJOR_TYPE nMajorType, int Address,wxWindow *pParent);
	~BreakpointDialog();
	void SetBreakpoint(BREAKPOINT *pBreakpoint);

	virtual bool TransferDataFromWindow() wxOVERRIDE;
	virtual bool TransferDataToWindow() wxOVERRIDE;

	void OnChoiceChange(wxCommandEvent &event);

	void UpdateDialogFromData();

	BREAKPOINT_MAJOR_TYPE m_nMajorType;
	int m_nCount;
	BREAKPOINT_TYPE m_nType;
	int m_nAddress;
	bool m_bEnabled;
	bool m_bUseDataMask;
	bool m_bUseAddressMask;
	bool m_bUseData;
	int m_nDataMask;
	int m_nData;
	int m_nAddressMask;
	bool m_bWrite;
private:

	DECLARE_EVENT_TABLE()
};

class ItemClientData : public wxClientData
{
protected:
	ItemData m_Data;
public:
	ItemClientData() : wxClientData() {}
	ItemClientData(const ItemData &Data) : m_Data(Data) { };
	ItemData &GetData() { return m_Data; };
	void SetData(const ItemData &Data) { m_Data = Data; };
};

class HardwareDetailsItem
{
public:
	HardwareDetailsItem(const HardwareDetailsItem &src)
	{
		(*this) = src;
	}

	HardwareDetailsItem &operator=(const HardwareDetailsItem &src)
	{
		this->m_Label = src.m_Label;
		this->m_Data = src.m_Data;
		this->m_CachedValue = src.m_CachedValue;
		this->m_bChanged = src.m_bChanged;
		return (*this);
	}
	HardwareDetailsItem()
	{
		m_bChanged = false;
	}

	wxString m_Label;
	ItemData m_Data;
	wxString m_CachedValue;
	bool m_bChanged;

};

//#include <wx/dynarray.h>
//WX_DECLARE_OBJARRAY(HardwareDetailsItem,ArrayOfHardwareDetailsItem);

class OurTextWindow : public wxWindow
{
public:
	OurTextWindow();
	~OurTextWindow();

	bool Create(wxWindow* parent, wxWindowID id, const wxString &label, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxValidator &validator = wxDefaultValidator, const wxString& name = wxPanelNameStr);



protected:
	virtual wxSize 	DoGetBestClientSize() const  wxOVERRIDE{ return wxSize(100, 100); }

	bool m_bHasMargin;
	wxString m_sContextMenuName;
	wxPoint TextOrigin;
	wxPoint ClientOrigin;
	wxSize TextSize;
	wxSize ClientSize;
	wxCoord m_nFontWidth;
	wxCoord  m_nFontHeight;
	int m_nCharsWidth;      // width of window in chars
	int m_nCharsHeight;      // height of window in chars


	virtual void DoOnPaint(wxAutoBufferedPaintDC  &dc) = 0;
	void DrawCursor(wxAutoBufferedPaintDC  &dc, int x, int y, int width, int height);

	void DrawCurrentMarker(wxAutoBufferedPaintDC  &dc, int y);
	void DrawBreakpointMarker(wxAutoBufferedPaintDC  &dc, int y, bool bEnabled);

	void DrawMarkerBitmap(wxAutoBufferedPaintDC  &dc, wxBitmap &Bitmap, int y);
	void SendDataToRegister(int nData);
	void SendDataToStack(int nData);
	void SendDataToMemoryDump(int nData);
	void SendDataToDissassembly(int nData);

	void PixelToChar(int PixelX, int PixelY, int *pCharX, int *pCharY);

private:
	bool m_bHasFocus;

	DECLARE_EVENT_TABLE()

	void OnPaint(wxPaintEvent &event);
	void OnSize(wxSizeEvent &event);
	void OnKeyDown(wxKeyEvent &event);
	void OnMouseWheel(wxMouseEvent &event);
	void OnMouseLeftButtonDown(wxMouseEvent &event);
	void OnContextMenu(wxContextMenuEvent &event);
	void OnSetFocus(wxFocusEvent &event);
	void OnKillFocus(wxFocusEvent &event);
	void OnEraseBackground(wxEraseEvent &event);
	bool GetGotoAddress(int *Address);
	bool GetGoToLabel(int *nAddress);
	virtual void DoTab(bool bForwards);
	virtual void DoCursorUp();
	virtual void DoCursorDown();
	virtual void DoCursorLeft();
	virtual void DoCursorRight();
	virtual void DoPageUp();
	virtual void DoPageDown();
	virtual void DoSelect(int x, int y);
	virtual void DoClickMargin(int x, int y);
	virtual void UpdateContextMenuState(wxMenuBar *);

};

class MemDumpWindow : public OurTextWindow
{
protected:
	MEMDUMP_WINDOW *pMemdumpWindow;
	SEARCH_DATA SearchData;

	EmuFileType m_FileType;
public:
	MemDumpWindow();
	~MemDumpWindow();
	bool Create(wxWindow* parent, wxWindowID id, const wxString &label, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxValidator &validator = wxDefaultValidator, const wxString& name = wxPanelNameStr);
	MEMDUMP_WINDOW *GetStruct() { return pMemdumpWindow; }

protected:
	virtual void DoOnPaint(wxAutoBufferedPaintDC &dc) wxOVERRIDE;
	void DoFind();

	virtual void DoTab(bool bForwards) wxOVERRIDE;
	virtual void DoCursorUp() wxOVERRIDE;
	virtual void DoCursorDown() wxOVERRIDE;
	virtual void DoCursorLeft() wxOVERRIDE;
	virtual void DoCursorRight() wxOVERRIDE;
	virtual void DoPageUp() wxOVERRIDE;
	virtual void DoPageDown() wxOVERRIDE;
	virtual void DoSelect(int x, int y) wxOVERRIDE;
	virtual void UpdateContextMenuState(wxMenuBar *) wxOVERRIDE;

	void OnSendCursorAddressToDissassembly(wxCommandEvent &event);
	void OnSendCursorAddressToRegister(wxCommandEvent &event);
	void OnSendCursorAddressToStack(wxCommandEvent &event);
	void OnGotoAddress(wxCommandEvent &event);
	void OnGotoLabel(wxCommandEvent & WXUNUSED(event));
	void OnLoadData(wxCommandEvent &event);
	void OnSaveData(wxCommandEvent &event);
	void OnFillMemory(wxCommandEvent &event);
	void OnSearchForData(wxCommandEvent &event);
	void OnSearchNext(wxCommandEvent &event);
	void PerformSearch();
	void OnKeyDown(wxKeyEvent &event);

	void OnByteDataSize(wxCommandEvent &event);
	void OnWordDataSize(wxCommandEvent &event);
	void OnChar(wxKeyEvent &event);
	void OnSetMemoryRange(wxCommandEvent &event);
	void OnBreakpoint(wxCommandEvent &event);
	void OnSevenBitASCII(wxCommandEvent &event);
	void OnKillFocus(wxFocusEvent &event);
	void UpdateDissassembly();

	DECLARE_EVENT_TABLE()
};


class MemdumpSearchDialog : public wxDialog
{
public:
	MemdumpSearchDialog(wxWindow *pParent, SEARCH_DATA *pSearchData);
	~MemdumpSearchDialog();

	virtual bool TransferDataFromWindow() wxOVERRIDE;
	virtual bool TransferDataToWindow() wxOVERRIDE;


private:
	SEARCH_DATA *m_pSearchData;
	DECLARE_EVENT_TABLE()
};

class FillMemoryDialog : public wxDialog
{
public:
	FillMemoryDialog(wxWindow *pParent);
	~FillMemoryDialog();

	virtual bool TransferDataFromWindow() wxOVERRIDE;
	virtual bool TransferDataToWindow() wxOVERRIDE;


	int m_nStart;
	int m_nEnd;
	int FillStringCount;
	char *pFillString;
	unsigned char *pFillStringMask;

private:
	DECLARE_EVENT_TABLE()
};


class DissassembleWindow : public OurTextWindow
{
protected:
	DISSASSEMBLE_WINDOW *pDissassembleWindow;
	EmuFileType m_FileType;

public:
	DissassembleWindow();
	~DissassembleWindow();
	bool Create(wxWindow* parent, wxWindowID id, const wxString &label, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxValidator &validator = wxDefaultValidator, const wxString& name = wxPanelNameStr);
	DISSASSEMBLE_WINDOW *GetStruct() { return pDissassembleWindow; }
	void RefreshIfInRange(int Start, int End);
protected:
	virtual void DoOnPaint(wxAutoBufferedPaintDC &dc) wxOVERRIDE;

	DECLARE_EVENT_TABLE()
	void OnKeyDown(wxKeyEvent &event);
	void OnResetCycleCounter(wxCommandEvent &event);

	virtual void DoTab(bool bForwards) wxOVERRIDE;
	virtual void DoCursorUp()  wxOVERRIDE;
	virtual void DoCursorDown()  wxOVERRIDE;
	virtual void DoCursorRight() wxOVERRIDE;
	virtual void DoPageUp() wxOVERRIDE;
	virtual void DoPageDown() wxOVERRIDE;
	virtual void DoCursorLeft() wxOVERRIDE;
	virtual void UpdateContextMenuState(wxMenuBar *) wxOVERRIDE;
	void OnToggleOpcodes(wxCommandEvent &event);
	void OnLabelManager(wxCommandEvent & WXUNUSED(event));
	void AddLabelAtCursor(wxCommandEvent & WXUNUSED(event));
	void OnGotoLabel(wxCommandEvent & WXUNUSED(event));

	void OnToggleBreakOpcode(wxCommandEvent &event);
	
	void OnLoadData(wxCommandEvent &event);
	void OnSaveData(wxCommandEvent &event);


	void ToggleACursorBreakpoint();

	void OnToggleAscii(wxCommandEvent &event);


	void OnToggleLabels(wxCommandEvent &event);


	void OnToggleTimings(wxCommandEvent &event);


	void OnFollowPC(wxCommandEvent &event);

	void OnBreak(wxCommandEvent &event);
	void OnContinue(wxCommandEvent &event);
	void OnStepOver(wxCommandEvent &event);
	void OnStepInto(wxCommandEvent &event);
	void OnSetPCToAddress(wxCommandEvent &event);
	void OnReturnToPC(wxCommandEvent &event);
	void OnRunToAddress(wxCommandEvent &event);
	void OnToggleBreakpoint(wxCommandEvent &event);

	void OnSendCursorAddressToMemoryDump(wxCommandEvent &event);
	void OnSendCursorAddressToRegister(wxCommandEvent &event);
	void OnSendCursorAddressToStack(wxCommandEvent &event);

	void OnConvertAddressTool(wxCommandEvent &event);
	void OnGotoAddress(wxCommandEvent &event);

	void OnDissassembleToClipboard(wxCommandEvent &event);
	void OnDissassembleToFile(wxCommandEvent &event);

	void DissassembleRange(wxString &sString, int nStart, int nEnd, bool bBytes, bool bASCII, bool bLabels);

	void ToggleABreakpoint(int nAddr);
	void RunToAddress();
	void SetPCToAddress();
	void DoStepOverInstruction();
	void DoStepIntoInstruction();


	void OnSetType(wxCommandEvent &event);
	int MenuIdToTypeId(int nId);

	virtual void DoSelect(int x, int y) wxOVERRIDE;

	void OnSetMemoryRange(wxCommandEvent &event);
	virtual void DoClickMargin(int x, int y) wxOVERRIDE;

	void OnSevenBitASCII(wxCommandEvent &event);

};

class DissassemblyRangeDialog : public wxDialog
{
public:
	DissassemblyRangeDialog(wxWindow *pParent);
	~DissassemblyRangeDialog();

	int m_nStart;
	int m_nEnd;
	bool m_bBytes;
	bool m_bASCII;
	bool m_bLabels;

	virtual bool TransferDataFromWindow() wxOVERRIDE;
	virtual bool TransferDataToWindow() wxOVERRIDE;


private:

	DECLARE_EVENT_TABLE()
};

class StackWindow : public OurTextWindow
{
protected:
	STACK_WINDOW *pStackWindow;
public:
	StackWindow();
	~StackWindow();
	bool Create(wxWindow* parent, wxWindowID id, const wxString &label, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxValidator &validator = wxDefaultValidator, const wxString& name = wxPanelNameStr);
	STACK_WINDOW *GetStruct() { return pStackWindow; }
protected:
	virtual void DoOnPaint(wxAutoBufferedPaintDC &dc) wxOVERRIDE;
	void SetSPToCursor();
	void RunToCursor();
	void OnKeyDown(wxKeyEvent &event);
	void OnRunToCursor(wxCommandEvent & event);
	void OnToggleBreakpoint(wxCommandEvent &event);
	virtual void DoClickMargin(int x, int y) wxOVERRIDE;

	virtual void DoSelect(int x, int y) wxOVERRIDE;
	virtual void DoTab(bool bForwards) wxOVERRIDE;
	virtual void DoCursorUp() wxOVERRIDE;
	virtual void DoCursorDown() wxOVERRIDE;
	virtual void DoCursorLeft() wxOVERRIDE;
	virtual void DoCursorRight() wxOVERRIDE;
	virtual void DoPageUp() wxOVERRIDE;
	virtual void DoPageDown() wxOVERRIDE;
	virtual void UpdateContextMenuState(wxMenuBar *) wxOVERRIDE;
	void OnFollowSP(wxCommandEvent &event);
	void OnShowAddressOffset(wxCommandEvent &event);
	void OnSetSPToAddress(wxCommandEvent &event);
	void OnReturnToSP(wxCommandEvent &event);



	void ToggleACursorBreakpoint();
	void ToggleABreakpoint(int nAddr);

	void OnSendDataToMemoryDump(wxCommandEvent &event);
	void OnSendDataToRegister(wxCommandEvent &event);
	void OnSendDataToDissassembly(wxCommandEvent &event);

	void OnGotoAddress(wxCommandEvent &event);
	void OnGotoLabel(wxCommandEvent & WXUNUSED(event));

	DECLARE_EVENT_TABLE()
};


class HardwareDetailsDisplay
{
public:
	HardwareDetailsDisplay();

	bool m_bDisplayPrinter;

	bool m_bDisplayKeyboard;

	bool m_bDisplayROMSelect;

	bool m_bDisplayPPI;

	bool m_bDisplayZ80;
	bool m_bDisplayZ80Registers;
	bool m_bDisplayZ80Inputs;

	bool m_bDisplayASIC;
	bool m_bDisplayASICPalette;
	bool m_bDisplayASICSprites;
	bool m_bDisplayASICDMA;
	bool m_bDisplayASICAnalogueInputs;

	bool m_bDisplayCRTC;
	bool m_bDisplayCRTCRegisters;
	bool m_bDisplayCRTCOutputs;

	bool m_bDisplayAY;
	bool m_bDisplayAYRegisters;

	bool m_bDisplayFDC;
	bool m_bDisplayGA;
	bool m_bDisplayKCC;
	bool m_bDisplayAleste;
	bool m_bDisplayFDI;


	bool m_bDisplayMonitor;
};

class DebuggerDisplayState
{
public:
	DebuggerDisplayState();

	bool DissassembleSettingValid[4];
	DISSASSEMBLE_SETTINGS DissassembleSettings[4];
	bool MemoryDumpSettingValid[4];
	MEMDUMP_SETTINGS MemoryDumpSettings[4];
	bool StackSettingValid;
	STACK_SETTINGS StackSettings;
};

class HardwareDetails : public wxListCtrl
{
protected:
public:
	HardwareDetails();
	~HardwareDetails();
	wxString m_sChangedText;

	void PerformRefresh();
	void PerformRefreshShown();

	void AddItem(const wxString &sValueName, ItemData &);
	//class ItemClientData *);

	void AddStringIDItem(const wxString &sValueName, int nCode, int nIndex, const wxChar **pStringTable, int nStringTableLength);
	void AddStringIDItem(const wxString &sValueName, int nCode, int nIndex, int nMask, int nShift, const wxChar **pStringTable, int nStringTableLength);

	void Add8BitItem(const wxString &sValueName, int nCode, int nIndex);
	void AddMasked8BitItem(const wxString &sValueName, int nCode, int nIndex, int nMask);
	void Add16BitItem(const wxString &sValueName, int nCode, int nIndex);

	// specific comparison
	void AddBoolItem(const wxString &sValueName, int nCode, int nIndex, int nMask, int nComparison);
	// true if value set
	void AddBoolItem(const wxString &sValueName, int nCode, int nIndex, int nMask, bool bSet);

	void AddBoolItem(const wxString &sValueName, int nCode, int nIndex, bool bSet);

	//    void RefreshItem(int nItem);
	//    void AddItem(const wxString &sValueName, const wxString &sValue);
	//  void Add16BitNumber(const wxString &sValueName, unsigned short nValue);
	//void Add8BitNumber(const wxString &sValueName, unsigned char nValue);
	// void Add16BitSignedNumber(const wxString &sValueName, signed short nValue);
	// void Add32BitNumber(const wxString &sValueName, unsigned long nValue);
	// void Add8BitSignedNumber(const wxString &sValueName, signed char nValue);
	// void AddBool(const wxString &sValueName, bool bValue);
	void ClearItems();
	void OnCreate(wxWindowCreateEvent &event);

protected:
	long m_nCachedTopItem;
	long m_nCachedBottomItem;
	long GetBottomItem();
	void OnClearChanged(wxCommandEvent &event);

	wxArrayPtrVoid m_Items;
	//ArrayOfHardwareDetailsItem m_Items;
	virtual wxString OnGetItemText(long item, long column) const wxOVERRIDE;
	void OnContextMenu(wxContextMenuEvent &event);

	void OnKeyDown(wxKeyEvent &event);

	void OnDisplay(wxCommandEvent &event);
	int GetDisplayIdFromMenuId(int nMenuId);

	void OnShowZ80(wxCommandEvent &event);
	void OnShowZ80Registers(wxCommandEvent &event);
	void OnShowZ80Inputs(wxCommandEvent &event);

	void OnShowCRTC(wxCommandEvent &event);
	void OnShowCRTCRegisters(wxCommandEvent &event);
	void OnShowCRTCOutputs(wxCommandEvent &event);


	void OnShowKeyboard(wxCommandEvent & WXUNUSED(event));

	void OnShowAY(wxCommandEvent &event);
	void OnShowAYRegisters(wxCommandEvent &event);

	void OnShowASIC(wxCommandEvent &event);
	void OnShowASICPalette(wxCommandEvent &event);
	void OnShowASICDMA(wxCommandEvent &event);
	void OnShowASICAnalogueInputs(wxCommandEvent &event);
	void OnShowASICSprites(wxCommandEvent &event);

	void OnShowFDC(wxCommandEvent &event);
	void OnShowFDI(wxCommandEvent &event);

	void OnShowGA(wxCommandEvent &event);

	void OnShowKCCompact(wxCommandEvent &event);

	void OnShowAleste(wxCommandEvent &event);

	void OnShowMonitor(wxCommandEvent &event);

	void OnShowPrinter(wxCommandEvent &event);

	void OnShowPPI(wxCommandEvent &event);

	void OnCacheHint(wxListEvent &event);

	void OnShowROMSelect(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
	DECLARE_DYNAMIC_CLASS(HardwareDetails)
};



class Breakpoints : public wxListCtrl
{
protected:
public:
	Breakpoints();
	~Breakpoints();
	virtual void PerformRefresh();

	static void AddBreakpointI(BREAKPOINT_MAJOR_TYPE nType, int Address,wxWindow *pParent);

	void OnCreate(wxWindowCreateEvent &event);
protected:
	void PerformInitialSetup();

	void AddBreakpoint(BREAKPOINT_MAJOR_TYPE);
	void RemoveBreakpoints();
	long m_nCachedTopItem;
	long m_nCachedBottomItem;
	long GetBottomItem();
	void PerformRefreshShown();

	void PerformRefreshItem(BOOL bState);
	void SetEnableState(BOOL bState);
	void SetEnableStateAll(BOOL bState);

	void GoToBreakpoint(long item);
	void OnBreakEvent(wxCommandEvent &event);
	int IdToMask(int id);

	void OnContextMenu(wxContextMenuEvent &event);
	void OnGotoBreakpoint(wxCommandEvent &event);
	void OnBreakpointActivated(wxListEvent& event);
	void OnBreakpointRegisterAdd(wxCommandEvent &event);
	void OnBreakpointMemoryAdd(wxCommandEvent &event);
	void OnBreakpointIOAdd(wxCommandEvent &event);
	void OnBreakpointRemove(wxCommandEvent &event);
	void OnBreakpointEnable(wxCommandEvent &event);
	void OnBreakpointDisable(wxCommandEvent &event);
	void OnBreakpointEnableAll(wxCommandEvent &event);
	void OnBreakpointDisableAll(wxCommandEvent &event);
	void OnBreakpointEdit(wxCommandEvent &event);
	void OnKeyDown(wxKeyEvent &event);
	void OnCacheHint(wxListEvent &event);

	DECLARE_EVENT_TABLE()
	DECLARE_DYNAMIC_CLASS(Breakpoints)
};

WX_DEFINE_ARRAY(DissassembleWindow *, DissassemblyWindowArray);
WX_DEFINE_ARRAY(MemDumpWindow *, MemdumpWindowArray);
WX_DEFINE_ARRAY(StackWindow *, StackWindowArray);

class DebuggerWindow : public wxDialog, public UpdatableDialog
{
	DebuggerWindow(wxWindow *pParent);
	~DebuggerWindow();

	static DebuggerWindow *m_pInstance;

	static void OpenWindowCallback(void);

public:
	virtual void TimedUpdate() wxOVERRIDE;
	void InitialUpdate();
	void OnShow(wxShowEvent &event);
	static void Init();



	// creator
	static DebuggerWindow *CreateInstance(wxWindow *pParent);

	// virtual bool TransferDataToWindow();

	static void RegisterDissassemblyWindow(DissassembleWindow *pWindow);
	static void RegisterMemdumpWindow(MemDumpWindow *pWindow);
	static void RegisterStackWindow(StackWindow *pWindow);
	static void RegisterHardwareDetails(HardwareDetails *pWindow);
	static void RegisterBreakpoints(Breakpoints *pWindow);

	static void DeRegisterDissassemblyWindow(DissassembleWindow *pWindow);
	static void DeRegisterMemdumpWindow(MemDumpWindow *pWindow);
	static void DeRegisterStackWindow(StackWindow *pWindow);
	static void DeRegisterHardwareDetails(HardwareDetails *pWindow);
	static void DeRegisterBreakpoints(Breakpoints *pWindow);

	static int GetDissassemblyWindowCount();
	static DissassembleWindow *GetDissassemblyWindowByIndex(size_t nIndex);

	static int GetMemdumpWindowCount();
	static MemDumpWindow *GetMemdumpWindowByIndex(size_t nIndex);

	static int GetStackWindowCount();
	static StackWindow *GetStackWindowByIndex(size_t nIndex);

	static Breakpoints *GetBreakpointWindow();

protected:
	static HardwareDetails *m_HardwareDetails;
	static DissassemblyWindowArray m_DissassemblyWindows;
	static MemdumpWindowArray m_MemdumpWindows;
	static StackWindowArray m_StackWindows;
	static Breakpoints *m_Breakpoints;

	void OnClose(wxCloseEvent &event);
	DECLARE_EVENT_TABLE()

};

class DissassembleWindowResourceHandler : public wxXmlResourceHandler
{
public:
	DissassembleWindowResourceHandler();
	~DissassembleWindowResourceHandler();

	virtual wxObject *DoCreateResource() wxOVERRIDE;
	virtual bool CanHandle(wxXmlNode *node) wxOVERRIDE;
};

class MemDumpWindowResourceHandler : public wxXmlResourceHandler
{
public:
	MemDumpWindowResourceHandler();
	~MemDumpWindowResourceHandler();

	virtual wxObject *DoCreateResource() wxOVERRIDE;
	virtual bool CanHandle(wxXmlNode *node) wxOVERRIDE;
};


class StackWindowResourceHandler : public wxXmlResourceHandler
{
public:
	StackWindowResourceHandler();
	~StackWindowResourceHandler();

	virtual wxObject *DoCreateResource() wxOVERRIDE;
	virtual bool CanHandle(wxXmlNode *node) wxOVERRIDE;
};



class HardwareDetailsResourceHandler : public wxXmlResourceHandler
{
public:
	HardwareDetailsResourceHandler();
	~HardwareDetailsResourceHandler();

	virtual wxObject *DoCreateResource();
	virtual bool CanHandle(wxXmlNode *node);
};



class BreakpointsResourceHandler : public wxXmlResourceHandler
{
public:
	BreakpointsResourceHandler();
	~BreakpointsResourceHandler();

	virtual wxObject *DoCreateResource();
	virtual bool CanHandle(wxXmlNode *node);
};


class OurTextWindowProperties
{
public:
	void Init();

	int m_nBorderWidth;

	wxFont m_Font;

	// normal text colour
	wxColour m_TextColour;

	// normal modified colour
	wxColour m_TextModifiedColour;

	// colour of the cursor
	wxColour m_CursorColour;

	// colour of current line
	wxColour m_SelectedLineColour;

	// background colour
	wxColour m_BackgroundColour;

	wxColour m_ValueChangedColour;

	wxColour m_MarginColour;

	wxColour m_TextUnderCursorColour;

	wxColour m_SelectedBorderColour;
	wxColour m_UnSelectedBorderColour;

	wxBitmap m_BreakpointBitmap;
	wxBitmap m_DisabledBreakpointBitmap;
	wxBitmap m_CursorBitmap;

	wxBrush m_BackgroundBrush;
	wxPen m_MarginPen;
	wxBrush m_MarginBrush;
	wxBrush m_CursorBrush;
	wxPen m_CursorPen;
	wxPen m_SelectedBorderPen;
	wxPen m_UnSelectedBorderPen;

protected:
	int m_nPointSize;
	wxString m_sFaceName;
};


extern OurTextWindowProperties g_TextWindowProperties;

#endif

