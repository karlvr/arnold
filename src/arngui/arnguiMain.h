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
#ifndef ARNGUIMAIN_H
#define ARNGUIMAIN_H


#include "UpdatableDialog.h"
#include "arnguiApp.h"

#include <wx/dnd.h>
#include <wx/frame.h>

#ifndef wxOVERRIDE
#define wxOVERRIDE
#endif

#include "EmuFileType.h"

class arnguiFileDropTarget : public wxFileDropTarget
{
public:
	arnguiFileDropTarget();
	virtual ~arnguiFileDropTarget();
private:

	virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &sFilenames)  wxOVERRIDE;
};


#if 0
class OurStatusBar : public wxStatusBar
{
public:
	bool Create(wxWindow *  parent,
		wxWindowID  id = wxID_ANY,
		long  style = 0,
		const wxString &  name = wxStatusBarNameStr
		);
};
#endif

typedef BOOL(*ENABLED_FUNCTION_INT_PARAM)(int);
typedef BOOL(*ENABLED_FUNCTION_NO_PARAM)();

typedef struct
{
	int nID;
	wxObjectEventFunction commandHandler;
} EventHandlerSetup;

typedef struct
{
	int id;
	int nValue;
} XRCID_Map;

class EmulationWindow;

class arnguiFrame : public wxFrame
{
public:
	void UpdateMenuStates();

	arnguiFrame(bool bFullScreen, wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Arnold"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(481, 466), long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);
	//        arnguiFrame(wxFrame *frame);
	virtual ~arnguiFrame();
	void EnsureWindowed();
	void EnsureFullscreen();

	void ProcessAction(ACTION_CODE nCode);
	//    void LoadSnapshot(const wxString &sFilename);
	//   void SaveSnapshot(const wxString &sFilename, int SnapshotVersion);

	void UpdateStatusBars();

	void RegisterWantUpdateFromTimer(UpdatableDialog *pUpdatable);
	void DeRegisterWantUpdateFromTimer(UpdatableDialog *pUpdatable);

	bool PerformExit();

	/* ROM */
	bool OpenROM(unsigned char **pData, unsigned long *pLength, wxString &sFilename, const wxString &sTitleSuffix);
private:
	UpdatableDialogArray m_UpdatableDialog;
	wxToolBar *m_ToolBar;
	//     OurStatusBar *m_StatusBar;
	static EventHandlerSetup m_EventHandlers[];

	typedef void (arnguiFrame::*UPDATE_FUNCTION)();

	static arnguiFrame::UPDATE_FUNCTION m_UpdateHandlers[];

	wxIcon *m_icon;
	// our emulation window which covers the client area of the frame
	EmulationWindow *m_pEmulationWindow;

	EmuFileType ScreenshotFileType;
	EmuFileType SnapshotFileType;
	EmuFileType TapeFileType;
	EmuFileType DiskFileType;
	EmuFileType CartridgeFileType;
	EmuFileType MediaFileType;
	EmuFileType ROMFileType;

	wxMenuBar *m_MenuBar;

	// a drop target that accepts files
	arnguiFileDropTarget *m_pFileDropTarget;

	// the idle event handle, this is setup after the emulation window has been created
	void OnIdle(wxIdleEvent &event);

	int Value_From_XRCID(const XRCID_Map *pMap, int nItems, int id);

	// used where a menu contains multiple items of which only one is selected. Make a selection and others are cleared.
	// we pass in the value. The items are all related.
	void RefreshMultipleItemStatesRadio(const XRCID_Map *pMap, int nItems, int nValueChosen);

	// used where multiple items can be enabled/disabled indepedently but whose state depends on calling a function with an int
	// parameter (e.g. all disc drive enabled menu items)
	void RefreshMultipleItemStates(const XRCID_Map *pMap, int nItems, ENABLED_FUNCTION_INT_PARAM pEnabledFunction);

	void OnCPCSaveScreenshot(wxCommandEvent &event);
	void OnMRUFile(wxCommandEvent& event);


	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);

	void OnComputerRestartReset(wxCommandEvent& event);
	void OnComputerRestartPower(wxCommandEvent& event);

	// cartridge handlers
	void OnCPCInsertCartridge(wxCommandEvent &event);
	void OnCPCRemoveCartridge(wxCommandEvent &event);
	void OnCPCInsertDefaultCartridge(wxCommandEvent &event);

	void OnSavePointSave(wxCommandEvent &event);
	void OnSavePointPrev(wxCommandEvent &event);
	void OnSavePointNext(wxCommandEvent &event);

	void OnAutoStartMedia(wxCommandEvent &event);

	int GetDriveIdFromSettingsId(int nMenuId);
	void OnCPCDriveSettings(wxCommandEvent &event);

	int GetDriveIdFromNewDiskId(int nMenuId);
	void OnCPCNewDisk(wxCommandEvent &event);

	int GetDriveIdFromWriteChangesDiskId(int nMenuId);
	void OnCPCDiskWriteChanges(wxCommandEvent &event);

	void ShowEmulationWindow(bool bFullScreen, bool bAudio, bool bJoystick);

	void OnMonitorSettings(wxCommandEvent & event);

	void OnEnableFirmwareSupports16RomsUpdateUI();
	void OnEnableAutodetectFirmwareSupports16RomsUpdateUI();
	void OnEnableFirmwareSupports16Roms(wxCommandEvent &event);
	void OnEnableAutodetectFirmwareSupports16Roms(wxCommandEvent &event);


	void OnClearRecentFilesDriveA(wxCommandEvent &event);
	void OnClearRecentFilesDriveB(wxCommandEvent &event);
	void OnClearRecentFilesDriveC(wxCommandEvent &event);
	void OnClearRecentFilesDriveD(wxCommandEvent &event);
	void OnClearRecentFilesCartridge(wxCommandEvent &event);
	void OnClearRecentFilesSnapshot(wxCommandEvent &event);
	void OnClearRecentFilesTape(wxCommandEvent &event);


	void OnKeyJoy(wxCommandEvent &event);
	void OnKeyboardPositionalConfig(wxCommandEvent &event);
	void OnCPCCSDCartridges(wxCommandEvent &event);
	void OnCPCAutoType(wxCommandEvent &event);
	//void OnCPCExpansionRoms(wxCommandEvent &event);
	void OnCPCOnBoardRoms(wxCommandEvent &event);
	void OnPokeDatabase(wxCommandEvent &event);
	void OnCPCYMRecording(wxCommandEvent &event);
	//    void OnCPCDebuggerDisplay(wxCommandEvent &event);
	void OnPrinterSetFile(wxCommandEvent &event);
	void OnPrinterCloseFile(wxCommandEvent &event);

	void OnCPCJoystick(wxCommandEvent &event);
	CPC_JOYSTICK_ID GetJoystickIdFromMenuId(int nMenuId);

	void OnRegisterFileTypes(wxCommandEvent &event);

	void OnCPCFullScreen(wxCommandEvent &event);


	void OnCPCDriveStatus(wxCommandEvent &event);
	void OnCPCMediaStatus(wxCommandEvent &event);
	// void OnCPCDissassembly(wxCommandEvent &event);
	//   void OnCPCMemdump(wxCommandEvent &event);
	//    void OnCPCStack(wxCommandEvent &event);
	void OnCPCDebugger(wxCommandEvent &event);
	void OnCPCGraphicsEditor(wxCommandEvent &event);

	void OnFullScreenSettings(wxCommandEvent &event);
	void OnAudioSettings(wxCommandEvent &event);

	// snapshot handlers
	void OnCPCLoadSnapshot(wxCommandEvent &event);
	void OnCPCSaveSnapshot(wxCommandEvent &event);

	void OnCPCInsertTape(wxCommandEvent &event);
	void OnCPCRemoveTape(wxCommandEvent &event);

	void OnCPCIgnoreTapeMotor(wxCommandEvent &event);
	void OnCPCIgnoreTapeMotorUpdateUI();
	void OnCPCTapePlay(wxCommandEvent & event);

	void OnCPCTapePlayUpdateUI();
	void OnCPCTapePause(wxCommandEvent & event);
	void OnCPCTapePauseUpdateUI();

	void OnKeyStickActive(wxCommandEvent &event);
	void OnKeyStickActiveUpdateUI();

	void OnCPCNegateTapePolarity(wxCommandEvent &event);
	void OnCPCNegateTapePolarityUpdateUI();

	void OnFillScanlines(wxCommandEvent &event);
	void OnFillScanlinesUpdateUI();
	void OnEntireDisplay(wxCommandEvent &event);
	void OnEntireDisplayUpdateUI();

	void OnDoubleHeight(wxCommandEvent &event);
	void OnDoubleHeightUpdateUI();

	void OnEnableScreenSplit(wxCommandEvent & WXUNUSED(event));
	void OnEnableScreenSplitUpdateUI();

	void OnEnablePRIInterrupt(wxCommandEvent & WXUNUSED(event));
	void OnEnablePRIInterruptUpdateUI();

	void OnEnableHorizontalScroll(wxCommandEvent & WXUNUSED(event));
	void OnEnableHorizontalScrollUpdateUI();

	void OnEnableVerticalScroll(wxCommandEvent & WXUNUSED(event));
	void OnEnableVerticalScrollUpdateUI();

	void OnEnableScrollBorder(wxCommandEvent & WXUNUSED(event));
	void OnEnableScrollBorderUpdateUI();


	void OnDrawSync(wxCommandEvent &event);
	void OnDrawSyncUpdateUI();

	void OnDrawBlanking(wxCommandEvent &event);
	void OnDrawBlankingUpdateUI();

	void OnDrawBorder(wxCommandEvent &event);
	void OnDrawBorderUpdateUI();


	void OnCPCAudioOutput(wxCommandEvent &event);
	void OnCPCAudioOutputUpdateUI();

	void OnCPCAudioSpeaker(wxCommandEvent &event);
	void OnCPCAudioSpeakerUpdateUI();

	void OnCPCKeyboardMode(wxCommandEvent &event);
	void OnCPCKeyboardModeUpdateUI();

	void OnEnableSprite(wxCommandEvent &event);
	void OnEnableSpriteUpdateUI();

	void OnCPCKeyboardHardware(wxCommandEvent &event);
	void OnCPCKeyboardHardwareUpdateUI();


	void OnSpeed(wxCommandEvent &event);
	void OnSpeedUpdateUI();

	void OnPositionalKeyboardSet(wxCommandEvent & event);
	void OnPositionalKeyboardSetUpdateUI();

	void OnWindowScale(wxCommandEvent &event);
	void OnWindowScaleUpdateUI();

	void OnAYEnableUpdateUI();


	void OnCPCKeyboardLanguage(wxCommandEvent &event);
	void OnCPCKeyboardLanguageUpdateUI();

	void OnCPCKeyboardAutoDetectLanguage(wxCommandEvent &event);
	void OnCPCKeyboardAutoDetectLanguageUpdateUI();

	//    int MenuIdToKeyboardPositionalSetId(int MenuId);
	//   void OnCPCPositionalSet(wxCommandEvent &event);
	// void OnCPCPositionalSetUpdateUI();

	int MenuIdToInsertDriveId(int nMenuId);
	int MenuIdToReloadDriveId(int nMenuId);
	int MenuIdToRemoveDriveId(int nMenuId);

	void OnCPCSetDefaultCartridge(wxCommandEvent &event);
	void OnCPCSetDefaultCartridgeUpdateUI();

	void PerformCleanup();

	// to be improved
	const wxChar *MenuIdToConfigId(int nMenuId);
	void ChooseConfig(wxCommandEvent &event);
	void ChooseConfigUpdateUI();

	//  int MenuIdToWriteProtectDriveId(int nMenuId);
	//    void OnCPCForceWriteProtectDiskDrive(wxCommandEvent &event);
	//    void OnCPCForceWriteProtectDiskDriveUpdateUI();

	//  int MenuIdToReadyDriveId(int nMenuId);
	//   void OnCPCForceReadyDiskDrive(wxCommandEvent &event);
	//    void OnCPCForceReadyDiskDriveUpdateUI();

	void WriteJoystickSettings(const wxString &sConfigPrefix, int id);

	void InsertDiskGUI(int nDrive);
	void ReloadDiskGUI(int nDrive);

	void OnCPCDriveEnable(wxCommandEvent &event);
	void OnCPCDriveEnableUpdateUI();
	int MenuIdToDriveEnableId(int nMenuId);

	void OnCPCForceDiscRomOff(wxCommandEvent &event);
	void OnCPCForceDiscRomOffUpdateUI();

	void OnCPCSwapDrives(wxCommandEvent &event);
	void OnCPCSwapDrivesUpdateUI();

	void OnCPCSwapSides(wxCommandEvent &event);
	void OnCPCSwapSidesUpdateUI();

	void OnCPCEnableFourDrives(wxCommandEvent &event);
	void OnCPCEnableFourDrivesUpdateUI();

	void OnCPCDiscInterfaceLink(wxCommandEvent &event);
	void OnCPCDiscInterfaceLinkUpdateUI();

	void OnCPCInsertDisk(wxCommandEvent &event);

	void OnCPCReloadDisk(wxCommandEvent &event);


	void OnCPCRemoveDisk(wxCommandEvent &event);

	void OnCPCTurnDiskToSideBInDrive(wxCommandEvent &event);
	void OnCPCTurnDiskToSideBInDriveUpdateUI();

	void OnCPCHzLink(wxCommandEvent &event);
	void OnCPCHzLinkUpdateUI();


	void OnCPCComputerName(wxCommandEvent &event);
	void OnCPCComputerNameUpdateUI();

	void OnCPCMonitor(wxCommandEvent &event);
	void OnCPCMonitorUpdateUI();

	void OnCPCCRTCType(wxCommandEvent &event);
	void OnCPCCRTCTypeUpdateUI();

	void OnExpansionDevices(wxCommandEvent& event);
	void OnPrinterDevices(wxCommandEvent& event);
	void OnInternalDevices(wxCommandEvent& event);
	void OnJoystickDevices(wxCommandEvent& event);

	void OnCPCAYEnableChannelA(wxCommandEvent &event);
	void OnCPCAYEnableChannelB(wxCommandEvent &event);
	void OnCPCAYEnableChannelC(wxCommandEvent &event);
	void OnCPCAYEnableNoise(wxCommandEvent &event);
	void OnCPCAYEnableHardwareEnvelope(wxCommandEvent &event);

	wxLongLong m_PreviousUpdateTime;

	bool m_bAllowIdle;
	bool m_bFullScreen;

	DECLARE_EVENT_TABLE()

};


#endif // ARNGUIMAIN_H
