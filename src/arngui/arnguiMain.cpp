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

#include "arnguiMain.h"
#include "arnguiApp.h"
#include "EmuWindow.h"
#include "AboutDialog.h"
#include "ArchiveDialog.h"
//#include <wx/xrc/xmlres.h>
//#include <wx/dialog.h>
//#include <wx/listctrl.h>
//#include <wx/listbox.h>
//#include <wx/arrstr.h>
//#include <wx/clntdata.h>
//#include <wx/filename.h>
//#include <wx/font.h>
//#include <wx/fontenum.h>
//#include <wx/stdpaths.h>
//#include <wx/textfile.h>
//#include <wx/archive.h>
//#include <wx/wfstream.h>
//#include <wx/textdlg.h>
//#include <wx/notebook.h>
//#include <wx/clipbrd.h>
#include "CSDDialog.h"
//#include "ExpansionRomDialog.h"
#include "OnBoardRomDialog.h"
#include "AutoTypeDialog.h"
#include "DriveSettingsDialog.h"
#include "YMDialogs.h"
#include "JoystickDialog.h"
#include "SnapshotSettingsDialog.h"
#include "DriveStatusDialog.h"
#include "MediaStatusDialog.h"
#include "DebuggerDialog.h"
#include "IntClientData.h"
//#include "PokeMemoryDialog.h"
#include "PositionalKeyboardDialog.h"
#include "GraphicsEditor.h"
//#include "ArchiveDialog.h"
#include "KeyJoyDialog.h"
//#include <wx/arrimpl.cpp>
//#include <wx/stdpaths.h>
#include "DevicesDialog.h"
#include "FullScreenDialog.h"
#include "AudioSettingsDialog.h"
#include "KeyJoyDialog.h"
#include "PokeMemoryDialog.h"
#include "MonitorSettingsDialog.h"
#include "sdlcommon/sound.h"
#include <wx/evtloop.h>

extern "C"
{

	//   #include "./directx/graphlib.h"
	// #include "./directx/dd.h"
	//  #include "./directx/di.h"
	//  #include "./directx/ds.h"

#include "../cpc/cpc.h"
#include "../cpc/diskimage/diskimg.h"
#include "../cpc/multface.h"
#include "../cpc/winape_poke_database.h"
	//   #include "../cpc/debugger/gdebug.h"
	//   #include "../cpc/jukebox.h"
#include "../cpc/joystick.h"
#include "../cpc/aleste.h"
#include "../cpc/kcc.h"
#include "../cpc/wav.h"
#include "../cpc/tzx.h"
#include "../cpc/autotype.h"
#include "../cpc/autorunfile.h"
#include "../cpc/debugger/labelset.h"
#include "../cpc/monitor.h"
	//  #include "../cpc/debugger/memdump.h"
	//#include "../cpc/debugger/dissasm.h"
	//#include "../cpc/debugger/stack.h"
	//#include "../cpc/debugger/breakpt.h"
#include "../cpc/asic.h"
#include "../cpc/i8255.h"
#include "../cpc/cassette.h"
#include "../cpc/fdi.h"
#include "../cpc/fdd.h"
#include "../cpc/render.h"
#include "../cpc/psgplay.h"
}



BEGIN_EVENT_TABLE(arnguiFrame, wxFrame)
END_EVENT_TABLE()

#define ARRAY_ELEMENT_COUNT(array) \
	sizeof(array)/sizeof(array[0])

arnguiFrame::UPDATE_FUNCTION arnguiFrame::m_UpdateHandlers[] =
{
	&arnguiFrame::ChooseConfigUpdateUI,
	&arnguiFrame::OnSpeedUpdateUI,

	&arnguiFrame::OnWindowScaleUpdateUI,


	&arnguiFrame::OnFillScanlinesUpdateUI,
	&arnguiFrame::OnEntireDisplayUpdateUI,
	&arnguiFrame::OnDoubleHeightUpdateUI,
	&arnguiFrame::OnDrawSyncUpdateUI,
	&arnguiFrame::OnDrawBlankingUpdateUI,
	&arnguiFrame::OnDrawBorderUpdateUI,

	&arnguiFrame::OnCPCKeyboardAutoDetectLanguageUpdateUI,

	&arnguiFrame::OnCPCSetDefaultCartridgeUpdateUI,

	&arnguiFrame::OnCPCKeyboardHardwareUpdateUI,


	&arnguiFrame::OnCPCKeyboardModeUpdateUI,

	&arnguiFrame::OnCPCKeyboardLanguageUpdateUI,


	&arnguiFrame::OnCPCNegateTapePolarityUpdateUI,
	&arnguiFrame::OnCPCTapePlayUpdateUI,
	&arnguiFrame::OnCPCTapePauseUpdateUI,

	&arnguiFrame::OnCPCIgnoreTapeMotorUpdateUI,

	&arnguiFrame::OnCPCDriveEnableUpdateUI,

	&arnguiFrame::OnCPCSwapDrivesUpdateUI,
	&arnguiFrame::OnCPCSwapSidesUpdateUI,
	&arnguiFrame::OnCPCForceDiscRomOffUpdateUI,

	&arnguiFrame::OnCPCEnableFourDrivesUpdateUI,
	&arnguiFrame::OnCPCDiscInterfaceLinkUpdateUI,


	&arnguiFrame::OnCPCTurnDiskToSideBInDriveUpdateUI,

	&arnguiFrame::OnCPCHzLinkUpdateUI,

	&arnguiFrame::OnCPCAudioOutputUpdateUI,
	&arnguiFrame::OnCPCAudioSpeakerUpdateUI,

	&arnguiFrame::OnCPCComputerNameUpdateUI,
	&arnguiFrame::OnCPCMonitorUpdateUI,

	&arnguiFrame::OnCPCCRTCTypeUpdateUI,

	&arnguiFrame::OnKeyStickActiveUpdateUI,
	&arnguiFrame::OnAYEnableUpdateUI,

	&arnguiFrame::OnEnableSpriteUpdateUI,

	&arnguiFrame::OnEnableScreenSplitUpdateUI,

	&arnguiFrame::OnEnablePRIInterruptUpdateUI,

	&arnguiFrame::OnEnableHorizontalScrollUpdateUI,
	&arnguiFrame::OnEnableVerticalScrollUpdateUI,
	&arnguiFrame::OnEnableScrollBorderUpdateUI,
	&arnguiFrame::OnEnableFirmwareSupports16RomsUpdateUI,
	&arnguiFrame::OnEnableAutodetectFirmwareSupports16RomsUpdateUI,

	&arnguiFrame::OnPositionalKeyboardSetUpdateUI,

};

// put handlers here so they can be connected and disconnected cleanly
EventHandlerSetup arnguiFrame::m_EventHandlers[] =
{
	{ XRCID("FirmwareAutodetectSupports16Roms"), wxCommandEventHandler(arnguiFrame::OnEnableAutodetectFirmwareSupports16Roms) },
	{ XRCID("FirmwareSupports16Roms"), wxCommandEventHandler(arnguiFrame::OnEnableFirmwareSupports16Roms) },

	{ XRCID("EnableSprite0"), wxCommandEventHandler(arnguiFrame::OnEnableSprite) },
	{ XRCID("EnableSprite1"), wxCommandEventHandler(arnguiFrame::OnEnableSprite) },
	{ XRCID("EnableSprite2"), wxCommandEventHandler(arnguiFrame::OnEnableSprite) },
	{ XRCID("EnableSprite3"), wxCommandEventHandler(arnguiFrame::OnEnableSprite) },
	{ XRCID("EnableSprite4"), wxCommandEventHandler(arnguiFrame::OnEnableSprite) },
	{ XRCID("EnableSprite5"), wxCommandEventHandler(arnguiFrame::OnEnableSprite) },
	{ XRCID("EnableSprite6"), wxCommandEventHandler(arnguiFrame::OnEnableSprite) },
	{ XRCID("EnableSprite7"), wxCommandEventHandler(arnguiFrame::OnEnableSprite) },
	{ XRCID("EnableSprite8"), wxCommandEventHandler(arnguiFrame::OnEnableSprite) },
	{ XRCID("EnableSprite9"), wxCommandEventHandler(arnguiFrame::OnEnableSprite) },
	{ XRCID("EnableSprite10"), wxCommandEventHandler(arnguiFrame::OnEnableSprite) },
	{ XRCID("EnableSprite11"), wxCommandEventHandler(arnguiFrame::OnEnableSprite) },
	{ XRCID("EnableSprite12"), wxCommandEventHandler(arnguiFrame::OnEnableSprite) },
	{ XRCID("EnableSprite13"), wxCommandEventHandler(arnguiFrame::OnEnableSprite) },
	{ XRCID("EnableSprite14"), wxCommandEventHandler(arnguiFrame::OnEnableSprite) },
	{ XRCID("EnableSprite15"), wxCommandEventHandler(arnguiFrame::OnEnableSprite) },
	//	{ XRCID("EnableAllSprites"), wxCommandEventHandler(arnguiFrame::OnEnableSprite) },
	//	{ XRCID("DisableAllSprites"), wxCommandEventHandler(arnguiFrame::OnEnableSprite) },

	{ XRCID("EnableScreenSplit"), wxCommandEventHandler(arnguiFrame::OnEnableScreenSplit) },
	{ XRCID("EnablePRIInterrupt"), wxCommandEventHandler(arnguiFrame::OnEnablePRIInterrupt) },
	{ XRCID("EnableHorizontalScroll"), wxCommandEventHandler(arnguiFrame::OnEnableHorizontalScroll) },
	{ XRCID("EnableVerticalScroll"), wxCommandEventHandler(arnguiFrame::OnEnableVerticalScroll) },
	{ XRCID("EnableScrollBorder"), wxCommandEventHandler(arnguiFrame::OnEnableScrollBorder) },

	{ XRCID("menu_recentfiles_drivea_clear"), wxCommandEventHandler(arnguiFrame::OnClearRecentFilesDriveA) },
	{ XRCID("menu_recentfiles_driveb_clear"), wxCommandEventHandler(arnguiFrame::OnClearRecentFilesDriveB) },
	{ XRCID("menu_recentfiles_drivec_clear"), wxCommandEventHandler(arnguiFrame::OnClearRecentFilesDriveC) },
	{ XRCID("menu_recentfiles_drived_clear"), wxCommandEventHandler(arnguiFrame::OnClearRecentFilesDriveD) },
	{ XRCID("menu_recentfiles_cartridge"), wxCommandEventHandler(arnguiFrame::OnClearRecentFilesCartridge) },
	{ XRCID("menu_recentfiles_snapshot"), wxCommandEventHandler(arnguiFrame::OnClearRecentFilesSnapshot) },
	{ XRCID("menu_recentfiles_tape"), wxCommandEventHandler(arnguiFrame::OnClearRecentFilesTape) },

	{ XRCID("ID_CPCTYPE_CPC464_EN"), wxCommandEventHandler(arnguiFrame::ChooseConfig) },
	{ XRCID("ID_CPCTYPE_CPC464_EN_DDI1"), wxCommandEventHandler(arnguiFrame::ChooseConfig) },
	{ XRCID("ID_CPCTYPE_CPC464_FR"), wxCommandEventHandler(arnguiFrame::ChooseConfig) },
	{ XRCID("ID_CPCTYPE_CPC464_ES"), wxCommandEventHandler(arnguiFrame::ChooseConfig) },
	{ XRCID("ID_CPCTYPE_CPC464_DK"), wxCommandEventHandler(arnguiFrame::ChooseConfig) },
	{ XRCID("ID_CPCTYPE_CPC664_EN"), wxCommandEventHandler(arnguiFrame::ChooseConfig) },
	{ XRCID("ID_CPCTYPE_CPC6128_EN"), wxCommandEventHandler(arnguiFrame::ChooseConfig) },
	{ XRCID("ID_CPCTYPE_CPC6128_FN"), wxCommandEventHandler(arnguiFrame::ChooseConfig) },
	{ XRCID("ID_CPCTYPE_CPC6128_PARADOS_EN"), wxCommandEventHandler(arnguiFrame::ChooseConfig) },
	{ XRCID("ID_CPCTYPE_CPC6128_FR"), wxCommandEventHandler(arnguiFrame::ChooseConfig) },
	{ XRCID("ID_CPCTYPE_CPC6128_ES"), wxCommandEventHandler(arnguiFrame::ChooseConfig) },
	{ XRCID("ID_CPCTYPE_CPC6128_DK"), wxCommandEventHandler(arnguiFrame::ChooseConfig) },
	{ XRCID("ID_CPCTYPE_464PLUS"), wxCommandEventHandler(arnguiFrame::ChooseConfig) },
	{ XRCID("ID_CPCTYPE_6128PLUS"), wxCommandEventHandler(arnguiFrame::ChooseConfig) },
	{ XRCID("ID_CPCTYPE_6128PLUS_CASSETTE"), wxCommandEventHandler(arnguiFrame::ChooseConfig) },
	{ XRCID("ID_CPCTYPE_GX4000"), wxCommandEventHandler(arnguiFrame::ChooseConfig) },
	{ XRCID("ID_CPCTYPE_KCCOMPACT"), wxCommandEventHandler(arnguiFrame::ChooseConfig) },
	{ XRCID("ID_CPCTYPE_ALESTE"), wxCommandEventHandler(arnguiFrame::ChooseConfig) },
	{ XRCID("ID_CHANGECONFIGURATION_CSD"), wxCommandEventHandler(arnguiFrame::ChooseConfig) },



	{ XRCID("EmulationSpeed200"), wxCommandEventHandler(arnguiFrame::OnSpeed) },
	{ XRCID("EmulationSpeed100"), wxCommandEventHandler(arnguiFrame::OnSpeed) },
	{ XRCID("EmulationSpeed50"), wxCommandEventHandler(arnguiFrame::OnSpeed) },
	{ XRCID("EmulationSpeed25"), wxCommandEventHandler(arnguiFrame::OnSpeed) },
	{ XRCID("EmulationSpeedUnlimited"), wxCommandEventHandler(arnguiFrame::OnSpeed) },

	{ XRCID("m_PositionalSetInternational"), wxCommandEventHandler(arnguiFrame::OnPositionalKeyboardSet) },
	{ XRCID("m_PositionalAzerty"), wxCommandEventHandler(arnguiFrame::OnPositionalKeyboardSet) },
	{ XRCID("m_PositionalQwertz"), wxCommandEventHandler(arnguiFrame::OnPositionalKeyboardSet) },
	{ XRCID("m_PositionalSetCustom"), wxCommandEventHandler(arnguiFrame::OnPositionalKeyboardSet) },

	{ XRCID("SpeakerVol0"), wxCommandEventHandler(arnguiFrame::OnCPCAudioSpeaker) },
	{ XRCID("SpeakerVol50"), wxCommandEventHandler(arnguiFrame::OnCPCAudioSpeaker) },
	{ XRCID("SpeakerVol100"), wxCommandEventHandler(arnguiFrame::OnCPCAudioSpeaker) },


	{ XRCID("WindowScale100"), wxCommandEventHandler(arnguiFrame::OnWindowScale) },
	{ XRCID("WindowScale200"), wxCommandEventHandler(arnguiFrame::OnWindowScale) },
	{ XRCID("WindowScale300"), wxCommandEventHandler(arnguiFrame::OnWindowScale) },
	{ XRCID("WindowScale400"), wxCommandEventHandler(arnguiFrame::OnWindowScale) },



	{ XRCID("FillScanlines"), wxCommandEventHandler(arnguiFrame::OnFillScanlines) },
	{ XRCID("EntireDisplay"), wxCommandEventHandler(arnguiFrame::OnEntireDisplay) },
	{ XRCID("DoubleHeight"), wxCommandEventHandler(arnguiFrame::OnDoubleHeight) },
	{ XRCID("DrawSync"), wxCommandEventHandler(arnguiFrame::OnDrawSync) },
	{ XRCID("DrawBlanking"), wxCommandEventHandler(arnguiFrame::OnDrawBlanking) },
	{ XRCID("DrawBorder"), wxCommandEventHandler(arnguiFrame::OnDrawBorder) },
	{ XRCID("MonitorSettings"), wxCommandEventHandler(arnguiFrame::OnMonitorSettings) },

	// Connect Events
	{ XRCID("m_FullScreenSettings"), wxCommandEventHandler(arnguiFrame::OnFullScreenSettings) },
	{ XRCID("m_menuAudioSettings"), wxCommandEventHandler(arnguiFrame::OnAudioSettings) },
	{ XRCID("wxID_EXIT"), wxCommandEventHandler(arnguiFrame::OnExit) },
	{ XRCID("menu_insert_cartridge"), wxCommandEventHandler(arnguiFrame::OnCPCInsertCartridge) },
	{ XRCID("menu_remove_cartridge"), wxCommandEventHandler(arnguiFrame::OnCPCRemoveCartridge) },
	{ XRCID("menu_insert_system_cartridge"), wxCommandEventHandler(arnguiFrame::OnCPCInsertDefaultCartridge) },
	{ XRCID("SaveScreenshot"), wxCommandEventHandler(arnguiFrame::OnCPCSaveScreenshot) },

	{ XRCID("menu_autostart"), wxCommandEventHandler(arnguiFrame::OnAutoStartMedia) },

	{ XRCID("m_SavePointSave"), wxCommandEventHandler(arnguiFrame::OnSavePointSave) },
	{ XRCID("m_SavePointNext"), wxCommandEventHandler(arnguiFrame::OnSavePointNext) },
	{ XRCID("m_SavePointPrev"), wxCommandEventHandler(arnguiFrame::OnSavePointPrev) },

	{ XRCID("ID_JOYSTICK_DIGITALJOYSTICK0"), wxCommandEventHandler(arnguiFrame::OnCPCJoystick) },
	{ XRCID("ID_JOYSTICK_DIGITALJOYSTICK1"), wxCommandEventHandler(arnguiFrame::OnCPCJoystick) },
	{ XRCID("ID_JOYSTICK_ANALOGUEJOYSTICK0"), wxCommandEventHandler(arnguiFrame::OnCPCJoystick) },
	{ XRCID("ID_JOYSTICK_ANALOGUEJOYSTICK1"), wxCommandEventHandler(arnguiFrame::OnCPCJoystick) },
	{ XRCID("ID_JOYSTICK_MULTIPLAY0"), wxCommandEventHandler(arnguiFrame::OnCPCJoystick) },
	{ XRCID("ID_JOYSTICK_MULTIPLAY1"), wxCommandEventHandler(arnguiFrame::OnCPCJoystick) },

	{ XRCID("ID_SETTINGS_REGISTERFILETYPES"), wxCommandEventHandler(arnguiFrame::OnRegisterFileTypes) },

	{ XRCID("menu_system_cart_en"), wxCommandEventHandler(arnguiFrame::OnCPCSetDefaultCartridge) },
	{ XRCID("menu_system_cart_fr"), wxCommandEventHandler(arnguiFrame::OnCPCSetDefaultCartridge) },
	{ XRCID("menu_system_cart_fr2"), wxCommandEventHandler(arnguiFrame::OnCPCSetDefaultCartridge) },
	{ XRCID("menu_system_cart_es"), wxCommandEventHandler(arnguiFrame::OnCPCSetDefaultCartridge) },

	{ XRCID("KeyboardHardwareCPC664"), wxCommandEventHandler(arnguiFrame::OnCPCKeyboardHardware) },
	{ XRCID("KeyboardHardwareCPC6128"), wxCommandEventHandler(arnguiFrame::OnCPCKeyboardHardware) },


	{ XRCID("KeyboardModeTranslated"), wxCommandEventHandler(arnguiFrame::OnCPCKeyboardMode) },
	{ XRCID("KeyboardModePositional"), wxCommandEventHandler(arnguiFrame::OnCPCKeyboardMode) },

	{ XRCID("KeyboardLanguageEnglish"), wxCommandEventHandler(arnguiFrame::OnCPCKeyboardAutoDetectLanguage) },
	{ XRCID("KeyboardLanguageFrench"), wxCommandEventHandler(arnguiFrame::OnCPCKeyboardLanguage) },
	{ XRCID("KeyboardLanguageSpanish"), wxCommandEventHandler(arnguiFrame::OnCPCKeyboardLanguage) },
	{ XRCID("KeyboardLanguageDanish"), wxCommandEventHandler(arnguiFrame::OnCPCKeyboardLanguage) },


	//  {XRCID("PositionalSetQwerty"),wxCommandEventHandler(arnguiFrame::OnCPCPositionalSet),arnguiFrame::OnCPCPositionalSetUpdateUI)},
	//{XRCID("PositionalSetQwertz"),wxCommandEventHandler(arnguiFrame::OnCPCPositionalSet),arnguiFrame::OnCPCPositionalSetUpdateUI)},
	//{XRCID("PositionalSetAzerty"),wxCommandEventHandler(arnguiFrame::OnCPCPositionalSet),arnguiFrame::OnCPCPositionalSetUpdateUI)},
	//{XRCID("PositionalSetSpanish"),wxCommandEventHandler(arnguiFrame::OnCPCPositionalSet),arnguiFrame::OnCPCPositionalSetUpdateUI)},


	{ XRCID("menu_csd_cartridges"), wxCommandEventHandler(arnguiFrame::OnCPCCSDCartridges) },

	{ XRCID("m_PositionalConfiguration"), wxCommandEventHandler(arnguiFrame::OnKeyboardPositionalConfig) },
	{ XRCID("m_KeyJoy"), wxCommandEventHandler(arnguiFrame::OnKeyJoy) },
	{ XRCID("KeyStickActive"), wxCommandEventHandler(arnguiFrame::OnKeyStickActive) },




	{ XRCID("menu_tape_insert"), wxCommandEventHandler(arnguiFrame::OnCPCInsertTape) },
	{ XRCID("menu_tape_remove"), wxCommandEventHandler(arnguiFrame::OnCPCRemoveTape) },

	{ XRCID("menu_tape_negate_polarity"), wxCommandEventHandler(arnguiFrame::OnCPCNegateTapePolarity) },
	{ XRCID("menu_tape_play"), wxCommandEventHandler(arnguiFrame::OnCPCTapePlay) },
	{ XRCID("menu_tape_pause"), wxCommandEventHandler(arnguiFrame::OnCPCTapePause) },

	{ XRCID("m_IgnoreMotorControl"), wxCommandEventHandler(arnguiFrame::OnCPCIgnoreTapeMotor) },

	{ XRCID("ID_TOOLS_AUTOTYPE"), wxCommandEventHandler(arnguiFrame::OnCPCAutoType) },
	{ XRCID("ID_TOOLS_WINAPE_POKE_DATABASE"), wxCommandEventHandler(arnguiFrame::OnPokeDatabase) },
	{ XRCID("ID_TOOLS_YMRECORDING"), wxCommandEventHandler(arnguiFrame::OnCPCYMRecording) },
	//  { XRCID("ID_DEBUG_DEBUGDISPLAY"), wxCommandEventHandler( arnguiFrame::OnCPCDebuggerDisplay )},
	//{ XRCID("m_ExternalRoms"), wxCommandEventHandler( arnguiFrame::OnCPCExpansionRoms ) },
	{ XRCID("m_OnBoardRoms"), wxCommandEventHandler(arnguiFrame::OnCPCOnBoardRoms) },

	{ XRCID("menu_view_fullscreen"), wxCommandEventHandler(arnguiFrame::OnCPCFullScreen) },

	{ XRCID("m_drive_status"), wxCommandEventHandler(arnguiFrame::OnCPCDriveStatus) },
	{ XRCID("m_media_status"), wxCommandEventHandler(arnguiFrame::OnCPCMediaStatus) },
	//{ XRCID("m_menuItem143"), wxCommandEventHandler( arnguiFrame::OnCPCMemdump ) },
	//{ XRCID("m_menuItem142"), wxCommandEventHandler( arnguiFrame::OnCPCDissassembly ) },
	//{ XRCID("m_menuItem144"), wxCommandEventHandler( arnguiFrame::OnCPCStack ) },
	{ XRCID("m_Debugger"), wxCommandEventHandler(arnguiFrame::OnCPCDebugger) },
	{ XRCID("m_GraphicsEditor"), wxCommandEventHandler(arnguiFrame::OnCPCGraphicsEditor) },

	{ XRCID("PrinterSetFile"), wxCommandEventHandler(arnguiFrame::OnPrinterSetFile) },
	{ XRCID("PrinterCloseFile"), wxCommandEventHandler(arnguiFrame::OnPrinterCloseFile) },


	{ XRCID("menu_snapshot_load"), wxCommandEventHandler(arnguiFrame::OnCPCLoadSnapshot) },
	{ XRCID("menu_snapshot_save"), wxCommandEventHandler(arnguiFrame::OnCPCSaveSnapshot) },

	{ XRCID("menu_drivea_enable"), wxCommandEventHandler(arnguiFrame::OnCPCDriveEnable) },
	{ XRCID("menu_driveb_enable"), wxCommandEventHandler(arnguiFrame::OnCPCDriveEnable) },
	{ XRCID("menu_drivec_enable"), wxCommandEventHandler(arnguiFrame::OnCPCDriveEnable) },
	{ XRCID("menu_drived_enable"), wxCommandEventHandler(arnguiFrame::OnCPCDriveEnable) },

	{ XRCID("menu_swap_drives"), wxCommandEventHandler(arnguiFrame::OnCPCSwapDrives) },
	{ XRCID("menu_force_side_1"), wxCommandEventHandler(arnguiFrame::OnCPCSwapSides) },
	{ XRCID("menu_force_disc_rom_off"), wxCommandEventHandler(arnguiFrame::OnCPCForceDiscRomOff) },

	{ XRCID("menu_enable_four_drives"), wxCommandEventHandler(arnguiFrame::OnCPCEnableFourDrives) },
	{ XRCID("menu_disc_interface_link"), wxCommandEventHandler(arnguiFrame::OnCPCDiscInterfaceLink) },

	{ XRCID("menu_insert_disk_drivea"), wxCommandEventHandler(arnguiFrame::OnCPCInsertDisk) },
	{ XRCID("menu_reload_disk_drivea"), wxCommandEventHandler(arnguiFrame::OnCPCReloadDisk) },
	{ XRCID("menu_remove_disk_drivea"), wxCommandEventHandler(arnguiFrame::OnCPCRemoveDisk) },

	{ XRCID("menu_writechanges_drivea"), wxCommandEventHandler(arnguiFrame::OnCPCDiskWriteChanges) },
	{ XRCID("menu_writechanges_driveb"), wxCommandEventHandler(arnguiFrame::OnCPCDiskWriteChanges) },
	{ XRCID("menu_writechanges_drivec"), wxCommandEventHandler(arnguiFrame::OnCPCDiskWriteChanges) },
	{ XRCID("menu_writechanges_drived"), wxCommandEventHandler(arnguiFrame::OnCPCDiskWriteChanges) },

	{ XRCID("menu_settings_drivea"), wxCommandEventHandler(arnguiFrame::OnCPCDriveSettings) },
	{ XRCID("menu_settings_driveb"), wxCommandEventHandler(arnguiFrame::OnCPCDriveSettings) },
	{ XRCID("menu_settings_drivec"), wxCommandEventHandler(arnguiFrame::OnCPCDriveSettings) },
	{ XRCID("menu_settings_drived"), wxCommandEventHandler(arnguiFrame::OnCPCDriveSettings) },

	{ XRCID("menu_new_disk_drivea"), wxCommandEventHandler(arnguiFrame::OnCPCNewDisk) },
	{ XRCID("menu_new_disk_driveb"), wxCommandEventHandler(arnguiFrame::OnCPCNewDisk) },
	{ XRCID("menu_new_disk_drivec"), wxCommandEventHandler(arnguiFrame::OnCPCNewDisk) },
	{ XRCID("menu_new_disk_drived"), wxCommandEventHandler(arnguiFrame::OnCPCNewDisk) },

	{ XRCID("menu_turnoverdisk_drivea"), wxCommandEventHandler(arnguiFrame::OnCPCTurnDiskToSideBInDrive) },


	{ XRCID("menu_insert_disk_driveb"), wxCommandEventHandler(arnguiFrame::OnCPCInsertDisk) },
	{ XRCID("menu_reload_disk_driveb"), wxCommandEventHandler(arnguiFrame::OnCPCReloadDisk) },
	{ XRCID("menu_remove_disk_driveb"), wxCommandEventHandler(arnguiFrame::OnCPCRemoveDisk) },

	{ XRCID("menu_turnoverdisk_driveb"), wxCommandEventHandler(arnguiFrame::OnCPCTurnDiskToSideBInDrive) },

	{ XRCID("menu_insert_disk_drivec"), wxCommandEventHandler(arnguiFrame::OnCPCInsertDisk) },
	{ XRCID("menu_reload_disk_drivec"), wxCommandEventHandler(arnguiFrame::OnCPCReloadDisk) },
	{ XRCID("menu_remove_disk_drivec"), wxCommandEventHandler(arnguiFrame::OnCPCRemoveDisk) },

	{ XRCID("menu_turnoverdisk_drivec"), wxCommandEventHandler(arnguiFrame::OnCPCTurnDiskToSideBInDrive) },

	{ XRCID("menu_insert_disk_drived"), wxCommandEventHandler(arnguiFrame::OnCPCInsertDisk) },
	{ XRCID("menu_reload_disk_drived"), wxCommandEventHandler(arnguiFrame::OnCPCReloadDisk) },
	{ XRCID("menu_remove_disk_drived"), wxCommandEventHandler(arnguiFrame::OnCPCRemoveDisk) },

	{ XRCID("menu_turnoverdisk_drived"), wxCommandEventHandler(arnguiFrame::OnCPCTurnDiskToSideBInDrive) },


	{ XRCID("m_RestartReset"), wxCommandEventHandler(arnguiFrame::OnComputerRestartReset) },
	{ XRCID("m_RestartPower"), wxCommandEventHandler(arnguiFrame::OnComputerRestartPower) },


	{ XRCID("menu_50hz_link"), wxCommandEventHandler(arnguiFrame::OnCPCHzLink) },
	{ XRCID("menu_60hz_link"), wxCommandEventHandler(arnguiFrame::OnCPCHzLink) },

	{ XRCID("m_AudioSpeaker"), wxCommandEventHandler(arnguiFrame::OnCPCAudioOutput) },
	{ XRCID("m_AudioStereoConnector"), wxCommandEventHandler(arnguiFrame::OnCPCAudioOutput) },
	{ XRCID("m_AudioExpansion"), wxCommandEventHandler(arnguiFrame::OnCPCAudioOutput) },


	{ XRCID("menu_computername_isp"), wxCommandEventHandler(arnguiFrame::OnCPCComputerName) },
	{ XRCID("menu_computername_triumph"), wxCommandEventHandler(arnguiFrame::OnCPCComputerName) },
	{ XRCID("menu_computername_saisho"), wxCommandEventHandler(arnguiFrame::OnCPCComputerName) },
	{ XRCID("menu_computername_solavox"), wxCommandEventHandler(arnguiFrame::OnCPCComputerName) },
	{ XRCID("menu_computername_awa"), wxCommandEventHandler(arnguiFrame::OnCPCComputerName) },
	{ XRCID("menu_computername_schneider"), wxCommandEventHandler(arnguiFrame::OnCPCComputerName) },
	{ XRCID("menu_computername_orion"), wxCommandEventHandler(arnguiFrame::OnCPCComputerName) },
	{ XRCID("menu_computername_amstrad"), wxCommandEventHandler(arnguiFrame::OnCPCComputerName) },

	{ XRCID("menu_monitor_colour"), wxCommandEventHandler(arnguiFrame::OnCPCMonitor) },
	{ XRCID("menu_monitor_green_screen"), wxCommandEventHandler(arnguiFrame::OnCPCMonitor) },
	{ XRCID("menu_monitor_greyscale"), wxCommandEventHandler(arnguiFrame::OnCPCMonitor) },

	{ XRCID("menu_crtctype_0"), wxCommandEventHandler(arnguiFrame::OnCPCCRTCType) },
	{ XRCID("menu_crtctype_1"), wxCommandEventHandler(arnguiFrame::OnCPCCRTCType) },
	{ XRCID("menu_crtctype_2"), wxCommandEventHandler(arnguiFrame::OnCPCCRTCType) },
	{ XRCID("menu_crtctype_3"), wxCommandEventHandler(arnguiFrame::OnCPCCRTCType) },
	{ XRCID("menu_crtctype_4"), wxCommandEventHandler(arnguiFrame::OnCPCCRTCType) },
	{ XRCID("menu_crtctype_5"), wxCommandEventHandler(arnguiFrame::OnCPCCRTCType) },

	{ XRCID("EnableChannelA"), wxCommandEventHandler(arnguiFrame::OnCPCAYEnableChannelA) },
	{ XRCID("EnableChannelB"), wxCommandEventHandler(arnguiFrame::OnCPCAYEnableChannelB) },
	{ XRCID("EnableChannelC"), wxCommandEventHandler(arnguiFrame::OnCPCAYEnableChannelC) },
	{ XRCID("EnableNoise"), wxCommandEventHandler(arnguiFrame::OnCPCAYEnableNoise) },
	{ XRCID("EnableHardwareEnvelope"), wxCommandEventHandler(arnguiFrame::OnCPCAYEnableHardwareEnvelope) },

	{ XRCID("m_Devices"), wxCommandEventHandler(arnguiFrame::OnExpansionDevices) },
	{ XRCID("m_PrinterDevices"), wxCommandEventHandler(arnguiFrame::OnPrinterDevices) },
	{ XRCID("m_JoystickDevices"), wxCommandEventHandler(arnguiFrame::OnJoystickDevices) },
	{ XRCID("m_InternalDevices"), wxCommandEventHandler(arnguiFrame::OnInternalDevices) },

	//	{ m_Reset->GetId(), wxCommandEventHandler( GUIFrame::OnCPCReset ) },
	//	{ menuHelpAbout->GetId(), wxCommandEventHandler( GUIFrame::OnAbout ) },
	{ XRCID("wxID_ABOUT"), wxCommandEventHandler(arnguiFrame::OnAbout) },


};

#if 0
class LEDIndicator : public wxWindow
{
private:
	int m_nDrive;
public:
	void

		FDD *pDrive = FDD_GetDrive(m_nDrive);

	wxString sLabel;
	sLabel.Printf(wxT("%c: %02d", m_nDrive, pDrive->CurrentTrack);




};;

bool OurStatusBar::Create(wxWindow *  parent,
	wxWindowID  id,
	long  style,
	const wxString &  name
	)
{
	bool bStatus = wxStatusBar::Create(parent, id, style, name);
	if (bStatus)
	{
		this->SetFieldsCount(5);

		wxString sLabel;
		sLabel.Printf(wxT("%c: %02d", m_nDrive, FDD_Get))

			// we need drive id, track and then led
			wxRect Rect;
		this->GetFieldRect(0, Rect);



	}
	return bStatus;
}
#endif

#if defined(__WXGTK__) || defined(__WXX__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#include "arnold.xpm"
#pragma GCC diagnostic pop
#endif

// for 2.8 compatibility, setup the xrcids in a table and look them up
int driveIds[4] = {
	XRCID("menu_recentfiles_drivea"),
	XRCID("menu_recentfiles_driveb"),
	XRCID("menu_recentfiles_drivec"),
	XRCID("menu_recentfiles_drived")
};

arnguiFrame::arnguiFrame(bool bFullScreen, wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxFrame(parent, id, title, pos, size, (style& ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX)) | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE),
m_ToolBar(NULL), m_icon(NULL), m_pEmulationWindow(NULL),
ScreenshotFileType(wxT("Open Screenshot"), wxT("Save Screenshot")),
SnapshotFileType(wxT("Open Snapshot"), wxT("Save Snapshot")),
TapeFileType(wxT("Open Tape"), wxT("Save Tape")),
DiskFileType(wxT("Open Disk"), wxT("Save Disk")),
CartridgeFileType(wxT("Open Cartridge"), wxT("Save Cartridge")),
MediaFileType(wxT("Open Media"), wxT("Save Media")),
ROMFileType(wxT("Open ROM"), wxT("Save ROM")),
m_MenuBar(NULL), m_pFileDropTarget(NULL), m_bAllowIdle(false), m_bFullScreen(false)
{
#if defined(__WXMSW__)
	wxString sName(wxT("arnold_icon"));
	m_icon = new wxIcon(sName);
#endif
#if defined(__WXGTK__) || defined(__WXX__)
	// this icon appears in linux (and on the side in unity on ubuntu)
	// should we work out which icon to show based on the theme???
	// so we can have higher resolution icons???
	m_icon = new wxIcon(arnold_xpm);
#endif
	if (m_icon != NULL)
	{
		SetIcon((*m_icon));
	}
	SetExtraStyle(GetExtraStyle() | wxWS_EX_PROCESS_IDLE);

	// binary
	FileFilter BinaryFilter = { wxT("Binary"), wxT("bin") };
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	// case sensitive filesystems
	FileFilter BinaryFilterAlt = { wxT("Binary"), wxT("BIN") };
#endif
	FileFilter BasicFilter = { wxT("Amstrad Basic"), wxT("bas") };
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	// case sensitive filesystems
	FileFilter BasicFilterAlt = { wxT("Amstrad Basic"), wxT("BAS") };
#endif



	// rom
	FileFilter ROMFilter = { wxT("Amstrad ROM"), wxT("rom") };
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	FileFilter ROMFilterAlt = { wxT("Amstrad ROM"), wxT("ROM") };
#endif

	// snapshot
	FileFilter SnapshotFilter = { wxT("Snapshot"), wxT("sna") };
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	FileFilter SnapshotFilterAlt = { wxT("Snapshot"), wxT("SNA") };
#endif

	// tape
	FileFilter TZXFilter = { wxT("ZX Tape Image"), wxT("tzx") };
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	FileFilter TZXFilterAlt = { wxT("ZX Tape Image"), wxT("TZX") };
#endif
	FileFilter CDTFilter = { wxT("CPC Tape Image"), wxT("cdt") };
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	FileFilter CDTFilterAlt = { wxT("CPC Tape Image"), wxT("cdt") };
#endif
	FileFilter CSWFilter = { wxT("Compressed Square Wave"), wxT("csw") };
	FileFilter WAVFilter = { wxT("Windows Audio file"), wxT("wav") };
	FileFilter VOCFilter = { wxT("VOC Audio file"), wxT("voc") };

	FileFilter DSKFilter = { wxT("CPC Disk Image"), wxT("dsk") };
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	FileFilter DSKFilterAlt = { wxT("CPC Disk Image"), wxT("DSK") };
#endif
	FileFilter CPRFilter = { wxT("Cartridge Image"), wxT("cpr") };
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	FileFilter CPRFilterAlt = { wxT("Cartridge Image"), wxT("CPR") };
#endif
	FileFilter CRTFilter = { wxT("Cartridge Image"), wxT("crt") };
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	FileFilter CRTFilterAlt = { wxT("Cartridge Image"), wxT("CRT") };
#endif

	FileFilter PNGFilter = { wxT("Portable Network Graphics Image"), wxT("png") };
	FileFilter BMPFilter = { wxT("Windows Bitmap"), wxT("bmp") };

	// zip
	FileFilter ZipFilter = { wxT("ZIP Archive"), wxT("zip") };
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	FileFilter ZipFilterAlt = { wxT("ZIP Archive"), wxT("ZIP") };
#endif

	FileFilter TarFilter = { wxT("TAR Archive"), wxT("tar") };

	FileFilter GZipFilter = { wxT("GZIP Archive"), wxT("gz") };

	SnapshotFileType.AddReadFilter(SnapshotFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	SnapshotFileType.AddReadFilter(SnapshotFilterAlt);
#endif
	SnapshotFileType.AddReadFilter(ZipFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	SnapshotFileType.AddReadFilter(ZipFilterAlt);
#endif
	SnapshotFileType.AddReadFilter(TarFilter);
	SnapshotFileType.AddReadFilter(GZipFilter);

	SnapshotFileType.AddWriteFilter(SnapshotFilter);

	TapeFileType.AddReadFilter(CDTFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	TapeFileType.AddReadFilter(CDTFilterAlt);
#endif
	TapeFileType.AddReadFilter(TZXFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	TapeFileType.AddReadFilter(TZXFilterAlt);
#endif
	TapeFileType.AddReadFilter(CSWFilter);
	TapeFileType.AddReadFilter(WAVFilter);
	TapeFileType.AddReadFilter(VOCFilter);
	TapeFileType.AddReadFilter(ZipFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	TapeFileType.AddReadFilter(ZipFilterAlt);
#endif
	TapeFileType.AddReadFilter(TarFilter);
	TapeFileType.AddReadFilter(GZipFilter);

	DiskFileType.AddReadFilter(DSKFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	DiskFileType.AddReadFilter(DSKFilterAlt);
#endif
	DiskFileType.AddReadFilter(ZipFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	DiskFileType.AddReadFilter(ZipFilterAlt);
#endif
	DiskFileType.AddReadFilter(TarFilter);
	DiskFileType.AddReadFilter(GZipFilter);
	DiskFileType.AddWriteFilter(DSKFilter);


	ROMFileType.AddReadFilter(ROMFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	ROMFileType.AddReadFilter(ROMFilterAlt);
#endif
	ROMFileType.AddReadFilter(BinaryFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	ROMFileType.AddReadFilter(BinaryFilterAlt);
#endif
	ROMFileType.AddReadFilter(ZipFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	ROMFileType.AddReadFilter(ZipFilterAlt);
#endif
	ROMFileType.AddReadFilter(TarFilter);
	ROMFileType.AddReadFilter(GZipFilter);

	CartridgeFileType.AddReadFilter(CPRFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	CartridgeFileType.AddReadFilter(CPRFilterAlt);
#endif
	CartridgeFileType.AddReadFilter(CRTFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	CartridgeFileType.AddReadFilter(CRTFilterAlt);
#endif
	CartridgeFileType.AddReadFilter(BinaryFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	CartridgeFileType.AddReadFilter(BinaryFilterAlt);
#endif
	CartridgeFileType.AddReadFilter(ZipFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	CartridgeFileType.AddReadFilter(ZipFilterAlt);
#endif
	CartridgeFileType.AddReadFilter(TarFilter);
	CartridgeFileType.AddReadFilter(GZipFilter);

	ScreenshotFileType.AddWriteFilter(PNGFilter);
	ScreenshotFileType.AddWriteFilter(BMPFilter);
	SnapshotFileType.AddReadFilter(ZipFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	SnapshotFileType.AddReadFilter(ZipFilterAlt);
#endif
	SnapshotFileType.AddReadFilter(TarFilter);
	SnapshotFileType.AddReadFilter(GZipFilter);

	// tape
	MediaFileType.AddReadFilter(CDTFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	MediaFileType.AddReadFilter(CDTFilterAlt);
#endif
	MediaFileType.AddReadFilter(TZXFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	MediaFileType.AddReadFilter(TZXFilterAlt);
#endif
	MediaFileType.AddReadFilter(CSWFilter);
	MediaFileType.AddReadFilter(WAVFilter);
	MediaFileType.AddReadFilter(VOCFilter);
	// disk
	MediaFileType.AddReadFilter(DSKFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	MediaFileType.AddReadFilter(DSKFilterAlt);
#endif
	// cart
	MediaFileType.AddReadFilter(CPRFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	MediaFileType.AddReadFilter(CPRFilterAlt);
#endif
	MediaFileType.AddReadFilter(CRTFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	MediaFileType.AddReadFilter(CRTFilterAlt);
#endif
	// snapshot
	MediaFileType.AddReadFilter(SnapshotFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	MediaFileType.AddReadFilter(SnapshotFilterAlt);
#endif
	// file
	MediaFileType.AddReadFilter(BinaryFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	MediaFileType.AddReadFilter(BinaryFilterAlt);
#endif
	MediaFileType.AddReadFilter(BasicFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	MediaFileType.AddReadFilter(BasicFilterAlt);
#endif
	// archives
	MediaFileType.AddReadFilter(ZipFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	MediaFileType.AddReadFilter(ZipFilterAlt);
#endif
	MediaFileType.AddReadFilter(TarFilter);
	MediaFileType.AddReadFilter(GZipFilter);
	// ROM
	MediaFileType.AddReadFilter(ROMFilter);
#if !defined(__WXMAC__) && !defined(__WXMSW__)
	MediaFileType.AddReadFilter(ROMFilterAlt);
#endif

	//set all initial path
	SnapshotFileType.SetInitialPath(wxGetApp().m_Snapshot->GetRecentPath());
	// disk is more of a problem, we have a recent path per drive.
	// but only one file type setup.
	// the reason I split it per drive is if you are using multiple drives it's nice to remember what each drive had
	// but also it's nice to know what all drives had.
	// so I need to think of a nice way to implement that.
	//DiskFileType.SetInitialPath(wxGetApp().m_Drives[0]->GetRecentPath());
	CartridgeFileType.SetInitialPath(wxGetApp().m_Cartridge->GetRecentPath());
	//ROMFileType.SetInitialPath(wxGetApp().m_Rom->GetRecentPath());
	//ScreenshotFileType.SetInitialPath(wxGetApp().m_Snapshot->GetRecentPath());
	TapeFileType.SetInitialPath(wxGetApp().m_Cassette->GetRecentPath());

	m_pEmulationWindow = NULL;

	//this part causes resize events so better to use it before the screen setting, and less problem for OSD
	m_MenuBar = wxXmlResource::Get()->LoadMenuBar(this, wxT("main_menu"));
	if (m_MenuBar == NULL)
	{
		printf("Failed to get menu bar\n");
	}
	else
	{
#if !defined(__WXMSW__)
		wxMenu *pMenu = NULL;
		wxMenuItem *pFileTypeItem = m_MenuBar->FindItem(XRCID("ID_SETTINGS_REGISTERFILETYPES"), &pMenu);
		if ((pFileTypeItem != NULL) && (pMenu != NULL))
		{
			pMenu->Delete(pFileTypeItem);
		}
#endif


		this->SetMenuBar(m_MenuBar);

		// base id
		size_t nID = wxID_HIGHEST + 1;
		size_t nRecentIDFirst = nID;

		wxMenuItem *pMenuItem;


		// recent files for each of the drives
		for (int i = 0; i < CPC_MAX_DRIVES; i++)
		{
			pMenuItem = m_MenuBar->FindItem(driveIds[i]);
			if (pMenuItem)
			{
				if (pMenuItem->IsSubMenu())
				{
					wxMenu *pMenu = pMenuItem->GetSubMenu();
					wxGetApp().m_Drives[i]->SetHistoryMenu(pMenu, nID);
				}
			}

			nID += FILE_HISTORY_SIZE;
		}

		pMenuItem = m_MenuBar->FindItem(XRCID("menu_recentfiles_cartridge"));
		if (pMenuItem)
		{
			if (pMenuItem->IsSubMenu())
			{
				wxMenu *pMenu = pMenuItem->GetSubMenu();
				wxGetApp().m_Cartridge->SetHistoryMenu(pMenu, nID);
			}
		}
		nID += FILE_HISTORY_SIZE;

		pMenuItem = m_MenuBar->FindItem(XRCID("menu_recentfiles_snapshot"));
		if (pMenuItem)
		{
			if (pMenuItem->IsSubMenu())
			{
				wxMenu *pMenu = pMenuItem->GetSubMenu();
				wxGetApp().m_Snapshot->SetHistoryMenu(pMenu, nID);
			}
		}
		nID += FILE_HISTORY_SIZE;

		pMenuItem = m_MenuBar->FindItem(XRCID("menu_recentfiles_tape"));
		if (pMenuItem)
		{
			if (pMenuItem->IsSubMenu())
			{
				wxMenu *pMenu = pMenuItem->GetSubMenu();
				wxGetApp().m_Cassette->SetHistoryMenu(pMenu, nID);
			}
		}
		nID += FILE_HISTORY_SIZE;

		//	printf("%d -> %d recent ids\n", nRecentIDFirst, nID);
		// connect all recent ids
		Connect(nRecentIDFirst, nID, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(arnguiFrame::OnMRUFile));
	}

	// toolbar doesn't seem appropiate on mac
#ifndef __WXMAC__
	m_ToolBar = wxXmlResource::Get()->LoadToolBar(this, wxT("main_toolbar"));
	if (m_ToolBar != NULL)
	{
		// if we can check the buttons have been loaded ok
		// we can then enable the toolbar...

		// check all buttons are valid?
		for (size_t i = 0; i < m_ToolBar->GetToolsCount(); i++)
		{
		}

		this->SetToolBar(m_ToolBar);
	}
#endif

	//this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	printf("Create emulation window\r\n");

	// create the emulation window; may be better to set up fullscreen or not seperately!
	m_pEmulationWindow = new EmulationWindow(bFullScreen, this, wxID_ANY, this->GetClientAreaOrigin(), this->GetClientSize(), wxBORDER_NONE);


	if (m_pEmulationWindow != NULL)
	{
		// show the emulation window
		printf("Show emulation window\r\n");
		m_pEmulationWindow->Show(true);

		// record time; used to ensure debugger updates are not too frequent
		m_PreviousUpdateTime = ::wxGetLocalTimeMillis();

		{

			wxIdleEvent event;
			event.SetEventObject(m_pEmulationWindow);
			m_pEmulationWindow->GetEventHandler()->AddPendingEvent(event);


			m_bAllowIdle = true;
			// connect the idle event
			Connect(wxEVT_IDLE, wxIdleEventHandler(arnguiFrame::OnIdle));

			// set drop target for drag-drop from explorer
			m_pFileDropTarget = new arnguiFileDropTarget();
			m_pEmulationWindow->SetDropTarget(m_pFileDropTarget);
			printf("Done init drag and drop\r\n");

		}
	}


	for (size_t i = 0; i < sizeof(m_EventHandlers) / sizeof(m_EventHandlers[0]); i++)
	{
		// connect handler
		if (m_EventHandlers[i].commandHandler != NULL)
		{
			this->Connect(m_EventHandlers[i].nID, wxEVT_COMMAND_MENU_SELECTED, m_EventHandlers[i].commandHandler);
		}
	}



#if 0
  {
	  wxBitmap Bitmap;
	  wxFileName BitmapPath(wxGetApp().GetAppPath(), wxT("autostart.png"));
	  wxString sFilename = BitmapPath.GetFullPath();
	  if (Bitmap.LoadFile(sFilename, wxBITMAP_TYPE_PNG))
	  {
		  wxMask *bitmapMask = new wxMask(Bitmap, wxColour(255, 0, 255));
		  if (bitmapMask != NULL)
		  {
			  Bitmap.SetMask(bitmapMask);
		  }
	  }
	  m_ToolBar->SetToolNormalBitmap(0, Bitmap);
  }
}
#endif


//    m_StatusBar = new OurStatusBar();
//  m_StatusBar->Create(this);
// this->SetStatusBar(m_StatusBar);
//    m_StatusBar = this->CreateStatusBar(4);
//  m_StatusBar->SetFieldsCount(4);
//this->SetStatusBar(m_StatusBar);



printf("Restoring window\r\n");

wxGetApp().ReadConfigWindowPos(wxT("windows/main/"), this);
wxGetApp().EnsureWindowVisible(this);

printf("Done restoring window\r\n");

}

void arnguiFrame::OnClearRecentFilesDriveA(wxCommandEvent & WXUNUSED(event))
{
	wxGetApp().m_Drives[0]->ClearHistory();
}

void arnguiFrame::OnClearRecentFilesDriveB(wxCommandEvent & WXUNUSED(event))
{
	wxGetApp().m_Drives[1]->ClearHistory();
}

void arnguiFrame::OnClearRecentFilesDriveC(wxCommandEvent & WXUNUSED(event))
{
	wxGetApp().m_Drives[2]->ClearHistory();
}

void arnguiFrame::OnClearRecentFilesDriveD(wxCommandEvent & WXUNUSED(event))
{
	wxGetApp().m_Drives[3]->ClearHistory();
}

void arnguiFrame::OnClearRecentFilesCartridge(wxCommandEvent & WXUNUSED(event))
{
	wxGetApp().m_Cartridge->ClearHistory();
}

void arnguiFrame::OnClearRecentFilesSnapshot(wxCommandEvent & WXUNUSED(event))
{
	wxGetApp().m_Snapshot->ClearHistory();
}

void arnguiFrame::OnClearRecentFilesTape(wxCommandEvent & WXUNUSED(event))
{
	wxGetApp().m_Cassette->ClearHistory();
}

void arnguiFrame::UpdateMenuStates()
{
	for (size_t i = 0; i < sizeof(m_UpdateHandlers) / sizeof(m_UpdateHandlers[0]); i++)
	{
		arnguiFrame &frame = (*this);

		(frame.*(m_UpdateHandlers[i]))();
	}

}

void arnguiFrame::OnMRUFile(wxCommandEvent& event)
{

	for (size_t nDrive = 0; nDrive < CPC_MAX_DRIVES; nDrive++)
	{
		if (wxGetApp().m_Drives[nDrive]->IsOurHistoryItem(event.GetId()))
		{
			if (!wxGetApp().m_Drives[nDrive]->LoadHistoryItem(event.GetId()))
			{
				// ask if they want the item removed or not...
				return;
			}

			return;
		}
	}

	if (wxGetApp().m_Cartridge->IsOurHistoryItem(event.GetId()))
	{
		if (!wxGetApp().m_Cartridge->LoadHistoryItem(event.GetId()))
		{
			// ask if they want the item removed or not...
			return;
		}

		return;
	}

	if (wxGetApp().m_Cassette->IsOurHistoryItem(event.GetId()))
	{
		if (!wxGetApp().m_Cassette->LoadHistoryItem(event.GetId()))
		{
			// ask if they want the item removed or not...
			return;
		}

		return;
	}

	if (wxGetApp().m_Snapshot->IsOurHistoryItem(event.GetId()))
	{
		if (!wxGetApp().m_Snapshot->LoadHistoryItem(event.GetId()))
		{
			// ask if they want the item removed or not...
			return;
		}

		return;
	}

	//	wxString fname = m_FilesHistory.GetHistoryFile(event.GetId() - wxID_FILE1);
	//	-    Open(fname, true);
	//	+    if (!Open(fname, true))
	//		+        m_FilesHistory.RemoveFileFromHistory(event.GetId() - wxID_FILE1); // Remove files that cannot be found

}

void arnguiFrame::UpdateStatusBars()
{
#if 0
	for (int i = 0; i < CPC_MAX_DRIVES; i++)
	{
		FDD		*pDrive = FDD_GetDrive(i);
		wxString sText;
		sText.Printf(wxT("%d"), pDrive->CurrentTrack);
		m_StatusBar->SetStatusText(sText, i);
	}
#endif

}

void arnguiFrame::OnComputerRestartReset(wxCommandEvent& WXUNUSED(event))
{
	wxGetApp().RestartReset(true, true);

}

void arnguiFrame::OnComputerRestartPower(wxCommandEvent& WXUNUSED(event))
{
	wxGetApp().RestartPower(true, true);
}




void arnguiFrame::OnCPCInsertTape(wxCommandEvent & WXUNUSED(event))
{
	wxString sFilename;
	wxString sTitleSuffix;

	if (TapeFileType.Open(this, sFilename, sTitleSuffix))
	{
		wxGetApp().m_Cassette->LoadWithSaveModified(sFilename, true);
	}
}

void arnguiFrame::OnCPCRemoveTape(wxCommandEvent & WXUNUSED(event))
{
	wxGetApp().m_Cassette->Remove();
}

void arnguiFrame::OnCPCTapePlay(wxCommandEvent & WXUNUSED(event))
{
	Cassette_PressPlay(!Cassette_GetPlay());

	OnCPCTapePlayUpdateUI();
}

void arnguiFrame::OnCPCTapePlayUpdateUI()
{
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("menu_tape_play"), Cassette_GetPlay());
}


void arnguiFrame::OnCPCTapePause(wxCommandEvent & WXUNUSED(event))
{
	Cassette_PressPause(!Cassette_GetPause());

	OnCPCTapePauseUpdateUI();
}

void arnguiFrame::OnCPCTapePauseUpdateUI()
{
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("menu_tape_pause"), Cassette_GetPause());
}


void arnguiFrame::OnCPCNegateTapePolarity(wxCommandEvent & WXUNUSED(event))
{
	Cassette_NegatePolarity(!Cassette_IsPolarityNegated());

	wxConfig::Get(false)->Write(wxT("cassette/negate_polarity"), (Cassette_IsPolarityNegated() == TRUE));

	OnCPCNegateTapePolarityUpdateUI();
}

void arnguiFrame::OnCPCNegateTapePolarityUpdateUI()
{
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("menu_tape_negate_polarity"), Cassette_IsPolarityNegated());
}



void arnguiFrame::OnEnableScreenSplit(wxCommandEvent & WXUNUSED(event))
{
	ASIC_SetScreenSplitOverride(!ASIC_GetScreenSplitOverride());

	OnEnableScreenSplitUpdateUI();
}

void arnguiFrame::OnEnableScreenSplitUpdateUI()
{
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("EnableScreenSplit"), ASIC_GetScreenSplitOverride());
}


void arnguiFrame::OnEnablePRIInterrupt(wxCommandEvent & WXUNUSED(event))
{
	ASIC_SetPRIInterruptOverride(!ASIC_GetPRIInterruptOverride());

	OnEnablePRIInterruptUpdateUI();
}

void arnguiFrame::OnEnablePRIInterruptUpdateUI()
{
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("EnablePRIInterrupt"), ASIC_GetPRIInterruptOverride());
}


void arnguiFrame::OnEnableHorizontalScroll(wxCommandEvent & WXUNUSED(event))
{
	ASIC_SetHorizontalScrollOverride(!ASIC_GetHorizontalScrollOverride());

	OnEnableHorizontalScrollUpdateUI();
}

void arnguiFrame::OnEnableHorizontalScrollUpdateUI()
{
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("EnableHorizontalScroll"), ASIC_GetHorizontalScrollOverride());
}

void arnguiFrame::OnEnableVerticalScroll(wxCommandEvent & WXUNUSED(event))
{
	ASIC_SetVerticalScrollOverride(!ASIC_GetVerticalScrollOverride());

	OnEnableVerticalScrollUpdateUI();
}

void arnguiFrame::OnEnableVerticalScrollUpdateUI()
{
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("EnableVerticalScroll"), ASIC_GetVerticalScrollOverride());
}


void arnguiFrame::OnEnableScrollBorder(wxCommandEvent & WXUNUSED(event))
{
	ASIC_SetScrollBorderOverride(!ASIC_GetScrollBorderOverride());

	OnEnableScrollBorderUpdateUI();
}

void arnguiFrame::OnEnableScrollBorderUpdateUI()
{
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("EnableScrollBorder"), ASIC_GetScrollBorderOverride());
}





void arnguiFrame::OnCPCIgnoreTapeMotor(wxCommandEvent & WXUNUSED(event))
{
	Cassette_SetIgnoreRelay(!Cassette_GetIgnoreRelay());

	wxConfig::Get(false)->Write(wxT("cassette/ignore_motor"), (Cassette_GetIgnoreRelay() == TRUE));

	OnCPCIgnoreTapeMotorUpdateUI();
}

void arnguiFrame::OnCPCIgnoreTapeMotorUpdateUI()
{
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("m_IgnoreMotorControl"), Cassette_GetIgnoreRelay());
}

void arnguiFrame::OnKeyStickActive(wxCommandEvent & WXUNUSED(event))
{
	Joystick_KeyStickActive(!Joystick_IsKeyStickActive());

	wxConfig::Get(false)->Write(wxT("keystick/active"), (Joystick_IsKeyStickActive() == TRUE));
	printf("KeyStick is : %s\n", Joystick_IsKeyStickActive() ? "enabled" : "disabled");

	OnKeyStickActiveUpdateUI();
}

void arnguiFrame::OnKeyStickActiveUpdateUI()
{
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("KeyStickActive"), Joystick_IsKeyStickActive());
}

void arnguiFrame::OnCPCInsertCartridge(wxCommandEvent & WXUNUSED(event))
{
	wxString sFilename;
	wxString sTitleSuffix;

	if (CartridgeFileType.Open(this, sFilename, sTitleSuffix))
	{
		wxGetApp().m_Cartridge->LoadWithSaveModified(sFilename, true);

		OnCPCKeyboardLanguageUpdateUI();
	}
}

void arnguiFrame::OnAutoStartMedia(wxCommandEvent & WXUNUSED(event))
{
	wxString sFilename;
	wxString sTitleSuffix;

	if (MediaFileType.Open(this, sFilename, sTitleSuffix))
	{
		wxGetApp().InsertOrAutoStartMedia(sFilename, true);
	}
}


void arnguiFrame::OnCPCLoadSnapshot(wxCommandEvent & WXUNUSED(event))
{
	wxString sFilename;
	wxString sTitleSuffix;

	if (SnapshotFileType.Open(this, sFilename, sTitleSuffix))
	{
		wxGetApp().m_Snapshot->LoadWithSaveModified(sFilename, true);

		// enable through a dialog, allow the user to breakpoint when a snapshot starts
#if 0
		// for breakpointing snapshots where they start		
		{
			if (!Breakpoints_IsAVisibleBreakpoint(BREAKPOINT_TYPE_PC, CPU_GetPC()))
			{
				Breakpoints_AddBreakpoint(BREAKPOINT_TYPE_PC, CPU_GetPC());
			}
		}
#endif		
	}
}


void arnguiFrame::OnCPCCSDCartridges(wxCommandEvent & WXUNUSED(event))
{
	CSDDialog dialog(this);
	dialog.ShowModal();
}

void arnguiFrame::OnKeyboardPositionalConfig(wxCommandEvent & WXUNUSED(event))
{

	PositionalKeyboardDialog dialog(this);
	if (dialog.ShowModal() == wxID_OK)
	{
		wxGetApp().WritePositionalKeyboardConfiguration();
	}
}



void arnguiFrame::OnKeyJoy(wxCommandEvent & WXUNUSED(event))
{

	KeyJoyDialog dialog(this);
	dialog.ShowModal();

	wxGetApp().RefreshJoystick();


}

#if 0
void arnguiFrame::OnCPCExpansionRoms(wxCommandEvent & WXUNUSED(event))
{
	ExpansionRomsDialog dialog(this);
	dialog.ShowModal();
}
#endif


void arnguiFrame::OnCPCOnBoardRoms(wxCommandEvent & WXUNUSED(event))
{
	OnBoardRomDialog dialog(this);
	dialog.ShowModal();

	OnCPCKeyboardLanguageUpdateUI();
}

int arnguiFrame::GetDriveIdFromSettingsId(int nMenuId)
{
	if (nMenuId == XRCID("menu_settings_drivea"))
	{
		return (int)0;
	}

	if (nMenuId == XRCID("menu_settings_driveb"))
	{
		return (int)1;
	}

	if (nMenuId == XRCID("menu_settings_drivec"))
	{
		return (int)2;
	}

	if (nMenuId == XRCID("menu_settings_drived"))
	{
		return (int)3;
	}

	return (int)0;
}



void arnguiFrame::OnCPCDriveSettings(wxCommandEvent &event)
{
	int DriveId = GetDriveIdFromSettingsId(event.GetId());

	DriveSettingsDialog dialog(this, DriveId);
	dialog.ShowModal();
}

int arnguiFrame::GetDriveIdFromNewDiskId(int nMenuId)
{
	if (nMenuId == XRCID("menu_new_disk_drivea"))
	{
		return (int)0;
	}

	if (nMenuId == XRCID("menu_new_disk_driveb"))
	{
		return (int)1;
	}

	if (nMenuId == XRCID("menu_new_disk_drivec"))
	{
		return (int)2;
	}

	if (nMenuId == XRCID("menu_new_disk_drived"))
	{
		return (int)3;
	}

	return (int)0;
}

void arnguiFrame::OnCPCNewDisk(wxCommandEvent &event)
{
	int DriveId = GetDriveIdFromNewDiskId(event.GetId());
	char chDrive = (char)('A' + DriveId);

	wxString sMessage;
	sMessage.Printf(wxT("Select format for new disk in Drive %c"), chDrive);

	wxArrayString sFormats;
	IntClientData *pClientData;

	/* +1 to include unformatted */
	VOID_PTR *DataArray = new VOID_PTR[GetNumFormatDescriptions() + 1];

	int nItems = 0;
	int nMaxTracks = FDD_GetTracks(DriveId);
	int nMaxSides = FDD_GetDoubleSided(DriveId) ? 2 : 1;

	sFormats.Add(wxT("Unformatted"));
	pClientData = new IntClientData(0);
	DataArray[nItems] = (VOID_PTR)pClientData;
	nItems++;

	for (int i = 0; i < GetNumFormatDescriptions(); i++)
	{
		const FORMAT_DESCRIPTION *pDescription = GetFormatDescription(i);

		if (
			(pDescription->nTracks <= nMaxTracks) &&
			(pDescription->nSides <= nMaxSides)
			)
		{
			wxString sDescription(pDescription->sDescription, wxConvLocal);
			sFormats.Add(sDescription);

			pClientData = new IntClientData(i + 1);
			DataArray[nItems] = (VOID_PTR)pClientData;
			nItems++;
		}
	}

	wxString sTitle;
	sTitle.sprintf(wxT("New Disk - Drive %c"), chDrive);

#if (wxVERSION_NUMBER >= 2900)
	// 2.9
	wxSingleChoiceDialog dialog(this, sMessage, sTitle, sFormats, (void **)DataArray);
#else
	// 2.8
	wxSingleChoiceDialog dialog(this, sMessage, sTitle, sFormats, (char **)DataArray);
#endif

	if (dialog.ShowModal() == wxID_OK)
	{
		// we confirmed format.

		// remove existing
		wxGetApp().m_Drives[DriveId]->Unload();

#if (wxVERSION_NUMBER >= 2900)
		const IntClientData *pSelectedClientData = (const IntClientData *)dialog.GetSelectionData();
#else
		const IntClientData *pSelectedClientData = (const IntClientData *)dialog.GetSelectionClientData();
#endif
		int nFormatId = pSelectedClientData->GetData();

		if (nFormatId == 0)
		{
			wxGetApp().m_Drives[DriveId]->InsertUninitialised();
		}
		else
		{
			wxGetApp().m_Drives[DriveId]->InsertInitialised(nFormatId - 1);
		}
	}
	for (int i = 0; i < nItems; i++)
	{
		delete (IntClientData *)DataArray[i];
	}
	delete[](IntClientData **)DataArray;
}


int arnguiFrame::GetDriveIdFromWriteChangesDiskId(int nMenuId)
{
	if (nMenuId == XRCID("menu_writechanges_drivea"))
	{
		return (int)0;
	}

	if (nMenuId == XRCID("menu_writechanges_driveb"))
	{
		return (int)1;
	}

	if (nMenuId == XRCID("menu_writechanges_drivec"))
	{
		return (int)2;
	}

	if (nMenuId == XRCID("menu_writechanges_drived"))
	{
		return (int)3;
	}

	return (int)0;
}

void arnguiFrame::OnCPCDiskWriteChanges(wxCommandEvent &event)
{
	int DriveId = GetDriveIdFromWriteChangesDiskId(event.GetId());

	wxGetApp().m_Drives[DriveId]->ForceSave();
}

#if 0
void arnguiFrame::OnCPCPokeMemory(wxCommandEvent &event)
{
	PokeMemoryDialog dialog(this);
	dialog.ShowModal();
}
#endif


void arnguiFrame::OnAYEnableUpdateUI()
{
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("EnableChannelA"), AY_IsChannelAEnabled());
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("EnableChannelB"), AY_IsChannelBEnabled());
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("EnableChannelC"), AY_IsChannelCEnabled());
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("EnableNoise"), AY_IsNoiseEnabled());
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("EnableHardwareEnvelope"), AY_IsHardwareEnvelopeEnabled());
}

void arnguiFrame::OnCPCAYEnableChannelA(wxCommandEvent & WXUNUSED(event))
{
	AY_SetChannelAEnabled(!AY_IsChannelAEnabled());

	OnAYEnableUpdateUI();
}

void arnguiFrame::OnCPCAYEnableChannelB(wxCommandEvent & WXUNUSED(event))
{
	AY_SetChannelBEnabled(!AY_IsChannelBEnabled());

	OnAYEnableUpdateUI();
}
void arnguiFrame::OnCPCAYEnableChannelC(wxCommandEvent & WXUNUSED(event))
{
	AY_SetChannelCEnabled(!AY_IsChannelCEnabled());

	OnAYEnableUpdateUI();
}
void arnguiFrame::OnCPCAYEnableNoise(wxCommandEvent & WXUNUSED(event))
{
	AY_SetNoiseEnabled(!AY_IsNoiseEnabled());

	OnAYEnableUpdateUI();
}
void arnguiFrame::OnCPCAYEnableHardwareEnvelope(wxCommandEvent & WXUNUSED(event))
{
	AY_SetHardwareEnvelopeEnabled(!AY_IsHardwareEnvelopeEnabled());

	OnAYEnableUpdateUI();
}


void arnguiFrame::OnCPCYMRecording(wxCommandEvent & WXUNUSED(event))
{
	YmRecordingDialog::CreateInstance(this);
}

CPC_JOYSTICK_ID arnguiFrame::GetJoystickIdFromMenuId(int nMenuId)
{
	if (nMenuId == XRCID("ID_JOYSTICK_DIGITALJOYSTICK0"))
	{
		return CPC_DIGITAL_JOYSTICK0;
	}

	if (nMenuId == XRCID("ID_JOYSTICK_DIGITALJOYSTICK1"))
	{
		return CPC_DIGITAL_JOYSTICK1;
	}

	if (nMenuId == XRCID("ID_JOYSTICK_ANALOGUEJOYSTICK0"))
	{
		return CPC_ANALOGUE_JOYSTICK0;
	}

	if (nMenuId == XRCID("ID_JOYSTICK_ANALOGUEJOYSTICK1"))
	{
		return CPC_ANALOGUE_JOYSTICK1;
	}
	if (nMenuId == XRCID("ID_JOYSTICK_MULTIPLAY0"))
	{
		return MULTIPLAY_JOYSTICK0;
	}

	if (nMenuId == XRCID("ID_JOYSTICK_MULTIPLAY1"))
	{
		return MULTIPLAY_JOYSTICK1;
	}

	return CPC_DIGITAL_JOYSTICK0;
}

void arnguiFrame::WriteJoystickSettings(const wxString &sConfigPrefix, int id)
{

	wxConfig::Get(false)->Write(sConfigPrefix + wxT("enable"), Joystick_IsActive(id));

	wxConfig::Get(false)->Write(sConfigPrefix + wxT("autofire/enable"), JoystickAF_IsActive(id));

	wxConfig::Get(false)->Write(sConfigPrefix + wxT("autofire/rate"), JoystickAF_GetRate(id));

	wxConfig::Get(false)->Write(sConfigPrefix + wxT("type"), Joystick_GetType(id));

	wxConfig::Get(false)->Write(sConfigPrefix + wxT("keyset"), Joystick_GetKeySet(id));

	wxString sId = wxGetApp().m_PlatformSpecific.GetJoystickIdString(Joystick_GetPhysical(id));
	wxConfig::Get(false)->Write(sConfigPrefix + wxT("id"), sId);

	//Save joystick button redefinition
	for (int j = 0; j < CPC_JOYSTICK_NUM_BUTTONS; j++)
	{
		wxString sKeyID;
		sKeyID.Printf(wxT("ButtonID%d"), j);
		wxConfig::Get(false)->Write(sConfigPrefix + wxT("redefinition/") + sKeyID, Joystick_GetButtonMappingCPC(id, j));
	}

	//Save joystick axis redefinition
	for (int j = 0; j < CPC_JOYSTICK_NUM_AXES; j++)
	{
		wxString sKeyID;
		sKeyID.Printf(wxT("AxisID%d"), j);
		wxConfig::Get(false)->Write(sConfigPrefix + wxT("redefinition/") + sKeyID, Joystick_GetAxisMappingPhysical(id, j));
	}

	//Save joystick simulated key
	for (int j = 0; j < JOYSTICK_SIMULATED_KEYID_LAST; j++)
	{
		wxString sKeyID;
		sKeyID.Printf(wxT("Simulated/keyID%d"), j);
		wxConfig::Get(false)->Write(sConfigPrefix + sKeyID, Joystick_GetSimulatedKeyID(id, j));
	}
}


void arnguiFrame::OnCPCJoystick(wxCommandEvent &event)
{
	CPC_JOYSTICK_ID CPCJoystickID = GetJoystickIdFromMenuId(event.GetId());

	JoystickDialog dialog(this, CPCJoystickID);
	dialog.ShowModal();

	if ((CPCJoystickID == MULTIPLAY_JOYSTICK0) || (CPCJoystickID == MULTIPLAY_JOYSTICK1))
	{
		int MultiplayDevice = EmuDevice_GetDeviceByName("Multiplay");
		if (MultiplayDevice == -1)
		{
			wxMessageBox(wxT("Multiplay device is not registered. You will not be able to use multiplay joysticks"));
		}
		else
		{
			if (!EmuDevice_IsEnabled(MultiplayDevice))
			{
				wxMessageBox(wxT("Please enable the Multiplay device to use Multiplay joysticks"));
			}
		}
	}

	bool bAnyKeyStick = false;

	wxGetApp().RefreshJoystick();

	for (int i = 0; i < CPC_NUM_DIGITAL_JOYSTICKS; i++)
	{
		wxString sConfigPrefix;

		if (
			(Joystick_IsActive(CPC_DIGITAL_JOYSTICK0 + i)) &&
			(Joystick_GetType(CPC_DIGITAL_JOYSTICK0 + i) == JOYSTICK_TYPE_SIMULATED_BY_KEYBOARD)
			)
		{
			bAnyKeyStick = true;
		}


		sConfigPrefix.Printf(wxT("joystick/digital%d/"), i);

		WriteJoystickSettings(sConfigPrefix, CPC_DIGITAL_JOYSTICK0 + i);
	}

	for (int i = 0; i < CPC_NUM_ANALOGUE_JOYSTICKS; i++)
	{
		wxString sConfigPrefix;

		if (
			(Joystick_IsActive(CPC_ANALOGUE_JOYSTICK0 + i)) &&
			(Joystick_GetType(CPC_ANALOGUE_JOYSTICK0 + i) == JOYSTICK_TYPE_SIMULATED_BY_KEYBOARD)
			)
		{
			bAnyKeyStick = true;
		}


		sConfigPrefix.Printf(wxT("joystick/analogue%d/"), i);

		WriteJoystickSettings(sConfigPrefix, CPC_ANALOGUE_JOYSTICK0 + i);
	}

	for (int i = 0; i < MULTIPLAY_NUM_JOYSTICKS; i++)
	{
		wxString sConfigPrefix;

		sConfigPrefix.Printf(wxT("joystick/multiplay%d/"), i);

		if (
			(Joystick_IsActive(MULTIPLAY_JOYSTICK0 + i)) &&
			(Joystick_GetType(MULTIPLAY_JOYSTICK0 + i) == JOYSTICK_TYPE_SIMULATED_BY_KEYBOARD)
			)
		{
			bAnyKeyStick = true;
		}


		WriteJoystickSettings(sConfigPrefix, MULTIPLAY_JOYSTICK0 + i);
	}


	if (bAnyKeyStick)
	{
		if (!Joystick_IsKeyStickActive())
		{
			wxString sMessage;
			sMessage.Printf(wxT("One of the joysticks is simulated by keyboard.\nPlease use the 'Keystick override' when you want to use it."));
			wxMessageBox(sMessage);
		}
	}
}

void arnguiFrame::OnCPCDriveStatus(wxCommandEvent & WXUNUSED(event))
{
	DriveStatusDialog::CreateInstance(this);
}

void arnguiFrame::OnCPCAutoType(wxCommandEvent & WXUNUSED(event))
{
	CAutoTypeDialog::CreateInstance(this);
}

arnguiFrame::~arnguiFrame()
{
	PerformCleanup();
}



void arnguiFrame::PerformCleanup()
{
	delete m_icon;
	this->SetMenuBar(NULL);
	this->SetToolBar(NULL);
	delete m_ToolBar;
	delete m_MenuBar;

	m_bAllowIdle = false;

	wxGetApp().WriteConfigWindowPos(wxT("windows/main/"), this);


	if (m_pEmulationWindow != NULL)
	{
		//    m_pEmulationWindow->SetDropTarget(NULL);

		// disconnect the idle event
		this->Disconnect(wxEVT_IDLE, wxIdleEventHandler(arnguiFrame::OnIdle));

		// destroy the emulation window
		m_pEmulationWindow->Destroy();
		//        delete m_pEmulationWindow;
		m_pEmulationWindow = NULL;
	}
	printf("Cleaning up event handlers\n");
	for (size_t i = 0; i < sizeof(m_EventHandlers) / sizeof(m_EventHandlers[0]); i++)
	{
		if (m_EventHandlers[i].commandHandler != NULL)
		{
			this->Disconnect(m_EventHandlers[i].nID, wxEVT_COMMAND_MENU_SELECTED, m_EventHandlers[i].commandHandler);
		}
	}
	printf("Done Cleaning up event handlers\n");

	//   delete m_pFileDropTarget;

	//   delete this->m_pFileDropTarget;

#if defined(GTK2_EMBED_WINDOW) || defined(MAC_EMBED_WINDOW) 
	// if embed mac or embed linux shutdown here to avoid crash on exit
	wxGetApp().m_PlatformSpecific.Shutdown();
#endif


}

bool arnguiFrame::PerformExit()
{
	m_bAllowIdle = false;

	bool bQuit = false;
	wxMessageDialog dialog(this, wxT("Are you sure you want to exit Arnold?"), wxGetApp().GetAppName(), wxYES_NO | wxNO_DEFAULT);
	if (dialog.ShowModal() == wxID_YES)
	{
		bQuit = true;
	}

	if (!bQuit)
	{
		return false;
	}

	return true;
}

void arnguiFrame::ProcessAction(ACTION_CODE nCode)
{

	switch (nCode)
	{
	default:
		break;

	case ACTION_TOGGLE_KEYSTICK_ACTIVE:
	{
		Joystick_KeyStickActive(!Joystick_IsKeyStickActive());
		OnKeyStickActiveUpdateUI();
		printf("KeyStick is : %s\n", Joystick_IsKeyStickActive() ? "enabled" : "disabled");
	}
	break;



	case ACTION_CPC_RESET:
	{
		wxGetApp().RestartReset(true, false);
	}
	break;

	case ACTION_CPC_RESET_IMMEDIATE:
	{
		wxGetApp().RestartReset(false, false);
	}
	break;

	case ACTION_NEXT_PLAYLIST:
	{
	}
	break;

	case ACTION_PREV_PLAYLIST:
	{
	}
	break;

	case ACTION_NEXT_SAVESTATE:
	{
		wxGetApp().NextSaveState();
	}
	break;

	case ACTION_PREV_SAVESTATE:
	{
		wxGetApp().PrevSaveState();
	}
	break;

	case ACTION_SAVE_SAVESTATE:
	{
		wxGetApp().SaveSaveState();
	}
	break;

	case ACTION_TOGGLE_FULLSCREEN:
	{
		// if we are full-screen then go windowed
		if (m_bFullScreen)
		{
			EnsureWindowed();
		}
		else
		{
			EnsureFullscreen();
		}
	}
	break;

	case ACTION_QUIT:
	{
		EnsureWindowed();
		this->Close(false);
	}
	break;

	case ACTION_QUIT_IMMEDIATE:
	{
		EnsureWindowed();
		this->Close(true);
	}
	break;

	case ACTION_INSERT_DISK_DRIVE_A:
	{
		EnsureWindowed();
		InsertDiskGUI(0);
	}
	break;

	case ACTION_INSERT_DISK_DRIVE_B:
	{
		EnsureWindowed();
		InsertDiskGUI(1);
	}
	break;
	}
}

void arnguiFrame::OnExit(wxCommandEvent & WXUNUSED(event))
{
	this->Close(false);
}

void arnguiFrame::OnAbout(wxCommandEvent & WXUNUSED(event))
{
	CAboutDialog dlg(this);
	dlg.ShowModal();
}


arnguiFileDropTarget::arnguiFileDropTarget() : wxFileDropTarget()
{
}

arnguiFileDropTarget::~arnguiFileDropTarget()
{
}

bool arnguiFileDropTarget::OnDropFiles(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y), const wxArrayString &sFilenames)
{
	for (unsigned int i = 0; i != sFilenames.GetCount(); i++)
	{
		wxString sFile = sFilenames[i];

		wxGetApp().HandleDropFile(sFile);
	}
	return true;
}


void arnguiFrame::RegisterWantUpdateFromTimer(UpdatableDialog *pUpdatable)
{
	for (unsigned int i = 0; i != m_UpdatableDialog.GetCount(); i++)
	{
		if (m_UpdatableDialog.Item(i) == pUpdatable)
		{
			return;
		}
	}

	m_UpdatableDialog.Add(pUpdatable);

}

void arnguiFrame::DeRegisterWantUpdateFromTimer(UpdatableDialog *pUpdatable)
{
	for (unsigned int i = 0; i != m_UpdatableDialog.GetCount(); i++)
	{
		if (m_UpdatableDialog.Item(i) == pUpdatable)
		{
			m_UpdatableDialog.RemoveAt(i);
			return;
		}
	}


}


XRCID_Map HzIDs_Map[] =
{
	{ XRCID("menu_50hz_link"), 0 },
	{ XRCID("menu_60hz_link"), 1 },
};

void arnguiFrame::OnCPCHzLink(wxCommandEvent &event)
{
	int nID = Value_From_XRCID(HzIDs_Map, ARRAY_ELEMENT_COUNT(HzIDs_Map), event.GetId());
	CPC_Set50Hz((nID == 0) ? TRUE : FALSE);

	wxConfig::Get(false)->Write(wxT("links/50hz_link"), CPC_Get50Hz());
	printf("CPC 50Hz setting: %s\n", CPC_Get50Hz() ? "yes" : "no");

	OnCPCHzLinkUpdateUI();

}

void arnguiFrame::OnCPCHzLinkUpdateUI()
{
	RefreshMultipleItemStatesRadio(HzIDs_Map, ARRAY_ELEMENT_COUNT(HzIDs_Map), CPC_Get50Hz() ? 0 : 1);
}

XRCID_Map ComputerIDs_Map[] =
{
	{ XRCID("menu_computername_isp"), PPI_COMPUTER_NAME_ISP },
	{ XRCID("menu_computername_triumph"), PPI_COMPUTER_NAME_TRIUMPH },
	{ XRCID("menu_computername_saisho"), PPI_COMPUTER_NAME_SAISHO },
	{ XRCID("menu_computername_solavox"), PPI_COMPUTER_NAME_SOLAVOX },
	{ XRCID("menu_computername_awa"), PPI_COMPUTER_NAME_AWA },
	{ XRCID("menu_computername_schneider"), PPI_COMPUTER_NAME_SCHNEIDER },
	{ XRCID("menu_computername_orion"), PPI_COMPUTER_NAME_ORION },
	{ XRCID("menu_computername_amstrad"), PPI_COMPUTER_NAME_AMSTRAD }
};


int arnguiFrame::Value_From_XRCID(const XRCID_Map *pMap, int nItems, int id)
{
	for (int i = 0; i < nItems; i++)
	{
		if (pMap[i].id == id)
		{
			return pMap[i].nValue;
		}
	}
	return pMap[0].nValue;
}


void arnguiFrame::RefreshMultipleItemStatesRadio(const XRCID_Map *pMap, int nItems, int nValueChosen)
{

	for (int i = 0; i < nItems; i++)
	{
		wxMenuItem *pMenuItem = m_MenuBar->FindItem(pMap[i].id);
		if (pMenuItem != NULL)
		{
			// ensure it's selected
			pMenuItem->Enable(true);

			bool bCheck = (pMap[i].nValue == nValueChosen);
			// if the value is the chosen one, check it else clear the check
			pMenuItem->Check(bCheck);

		}
	}
}
void arnguiFrame::RefreshMultipleItemStates(const XRCID_Map *pMap, int nItems, ENABLED_FUNCTION_INT_PARAM pEnabledFunction)
{
	for (int i = 0; i < nItems; i++)
	{
		wxMenuItem *pMenuItem = m_MenuBar->FindItem(pMap[i].id);
		if (pMenuItem != NULL)
		{
			// ensure it's selected
			pMenuItem->Enable(true);

			bool bChecked = pEnabledFunction(pMap[i].nValue) ? true : false;

			// if the value is the chosen one, check it else clear the check
			pMenuItem->Check(bChecked);
		}
	}
}

void arnguiFrame::OnCPCComputerName(wxCommandEvent &event)
{
	int nComputerId = Value_From_XRCID(ComputerIDs_Map, ARRAY_ELEMENT_COUNT(ComputerIDs_Map), event.GetId());
	CPC_SetComputerNameIndex(nComputerId);

	wxConfig::Get(false)->Write(wxT("links/computer_name_link"), CPC_GetComputerNameIndex());

	printf("CPC Computer name: %d\n", CPC_GetComputerNameIndex());

	OnCPCComputerNameUpdateUI();

}

void arnguiFrame::OnCPCComputerNameUpdateUI()
{
	RefreshMultipleItemStatesRadio(ComputerIDs_Map, ARRAY_ELEMENT_COUNT(ComputerIDs_Map), CPC_GetComputerNameIndex());
}


XRCID_Map MonitorIDs_Map[] =
{
	{ XRCID("menu_monitor_colour"), CPC_MONITOR_COLOUR },
	{ XRCID("menu_monitor_green_screen"), CPC_MONITOR_GT64 },
	{ XRCID("menu_monitor_greyscale"), CPC_MONITOR_MM12 },
};

void arnguiFrame::OnCPCMonitor(wxCommandEvent &event)
{
	int nMonitorID = Value_From_XRCID(MonitorIDs_Map, ARRAY_ELEMENT_COUNT(MonitorIDs_Map), event.GetId());
	CPC_SetMonitorType((CPC_MONITOR_TYPE_ID)nMonitorID);
	wxConfig::Get(false)->Write(wxT("monitor/monitor_type"), (int)CPC_GetMonitorType());
	printf("CPC Monitor Type: %d\n", (int)CPC_GetMonitorType());

	OnCPCMonitorUpdateUI();

}

void arnguiFrame::OnCPCMonitorUpdateUI()
{

	RefreshMultipleItemStatesRadio(MonitorIDs_Map, ARRAY_ELEMENT_COUNT(MonitorIDs_Map), CPC_GetMonitorType());

}


XRCID_Map CRTCIDs_Map[] =
{
	{ XRCID("menu_crtctype_0"), 0 },
	{ XRCID("menu_crtctype_1"), 1 },
	{ XRCID("menu_crtctype_2"), 2 },
	{ XRCID("menu_crtctype_3"), 3 },
	{ XRCID("menu_crtctype_4"), 4 },
	{ XRCID("menu_crtctype_5"), 5 },
};

void arnguiFrame::OnCPCCRTCType(wxCommandEvent &event)
{
	int nCRTCTypeID = Value_From_XRCID(CRTCIDs_Map, ARRAY_ELEMENT_COUNT(CRTCIDs_Map), event.GetId());
	CPC_SetCRTCType(nCRTCTypeID);
	wxConfig::Get(false)->Write(wxT("crtc/crtc_type"), (int)CPC_GetCRTCType());
	printf("CPC CRTC Type: %d\n", CPC_GetCRTCType());
	OnCPCCRTCTypeUpdateUI();

}

void arnguiFrame::OnCPCCRTCTypeUpdateUI()
{
	RefreshMultipleItemStatesRadio(CRTCIDs_Map, ARRAY_ELEMENT_COUNT(CRTCIDs_Map), CPC_GetCRTCType());
}

void arnguiFrame::InsertDiskGUI(int nDrive)
{
	wxString sFilename;
	wxString sTitleSuffix;
	sTitleSuffix.sprintf(wxT("Drive %c"), 'A' + nDrive);

	if (DiskFileType.Open(this, sFilename, sTitleSuffix))
	{
		wxGetApp().WarnDrive(nDrive);
		wxGetApp().m_Drives[nDrive]->LoadWithSaveModified(sFilename, true);
	}
}

void arnguiFrame::ReloadDiskGUI(int nDrive)
{
	if (!wxGetApp().m_Drives[nDrive]->GetMediaInserted())
	{
		wxString sMessage;
		sMessage.Printf(wxT("No disc is inserted into drive %c."), (char)(nDrive + 'A'));
		wxMessageBox(sMessage);
	}
	else
	{
		wxGetApp().m_Drives[nDrive]->Reload();
	}
}


void arnguiFrame::OnCPCRemoveCartridge(wxCommandEvent & WXUNUSED(event))
{
	Cartridge_Remove();
}

void arnguiFrame::OnCPCInsertDefaultCartridge(wxCommandEvent & WXUNUSED(event))
{
	// should this be insert with modified?
	Cartridge_InsertDefault();

	OnCPCKeyboardLanguageUpdateUI();
}


void arnguiFrame::OnCPCSaveSnapshot(wxCommandEvent & WXUNUSED(event))
{
	if (!Snapshot_CanSaveAccurately())
	{
		wxString sMessage;
		sMessage.Printf(wxT("The current emulation configuration can't be saved accurately with this type of snapshot file."));
		wxMessageBox(sMessage);
	}

	SnapshotSaveSettingsDialog dialog(this);
	if (dialog.ShowModal() == wxID_OK)
	{
		wxString sFilename;
		wxString sTitleSuffix;
		if (SnapshotFileType.OpenForWrite(this, sFilename, sTitleSuffix))
		{
			wxGetApp().SaveSnapshot(sFilename, &dialog.Options, &dialog.SnapshotMemoryBlocks);
		}
	}
}


void arnguiFrame::OnCPCSaveScreenshot(wxCommandEvent & WXUNUSED(event))
{
	wxString sFilename;
	wxString sTitleSuffix;
	if (ScreenshotFileType.OpenForWrite(this, sFilename, sTitleSuffix))
	{
		wxGetApp().SaveScreenshot(sFilename, false);
	}
}


void arnguiFrame::OnCPCForceDiscRomOff(wxCommandEvent & WXUNUSED(event))
{
	FDI_SetForceDiscRomOff(!FDI_GetForceDiscRomOff());
	wxConfig::Get(false)->Write(wxT("disc/force_disc_rom_off"), FDI_GetForceDiscRomOff());
	printf("FDI force disc rom off : %s\n", FDI_GetForceDiscRomOff() ? "yes" : "no");

	OnCPCForceDiscRomOffUpdateUI();
}

void arnguiFrame::OnCPCForceDiscRomOffUpdateUI()
{
	arnguiApp::RefreshSingleItemState(m_MenuBar,XRCID("menu_force_disc_rom_off"), FDI_GetForceDiscRomOff());
}

void arnguiFrame::OnCPCSwapDrives(wxCommandEvent & WXUNUSED(event))
{
	FDI_SwapDrives();
	wxConfig::Get(false)->Write(wxT("disc/drive_switch_active"), FDI_GetSwapDrives());
	printf("FDI drive switch: %s\n", FDI_GetSwapDrives() ? "A=B, B=A" : "A=A, B=B");

	OnCPCSwapDrivesUpdateUI();
}

void arnguiFrame::OnCPCSwapDrivesUpdateUI()
{
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("menu_swap_drives"), FDI_GetSwapDrives());

}

void arnguiFrame::OnCPCSwapSides(wxCommandEvent & WXUNUSED(event))
{
	FDI_SetForceSide1(!FDI_GetForceSide1());
	wxConfig::Get(false)->Write(wxT("disc/force_side_1"), FDI_GetForceSide1());

	printf("FDI force side 1 (side switch): %s\n", FDI_GetForceSide1() ? "yes" : "no");
}

void arnguiFrame::OnCPCSwapSidesUpdateUI()
{
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("menu_force_side_1"), FDI_GetForceSide1());
}


void arnguiFrame::OnCPCEnableFourDrives(wxCommandEvent & WXUNUSED(event))
{
	FDI_Set4Drives(!FDI_Get4Drives());
	wxConfig::Get(false)->Write(wxT("disc/4_drives_enable"), FDI_Get4Drives());
	printf("FDI use 4 drives: %s\n", FDI_Get4Drives() ? "yes" : "no");

	OnCPCEnableFourDrivesUpdateUI();
}

void arnguiFrame::OnCPCEnableFourDrivesUpdateUI()
{
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("menu_enable_four_drives"), FDI_Get4Drives());
}

void arnguiFrame::OnCPCDiscInterfaceLink(wxCommandEvent & WXUNUSED(event))
{
	CPC_SetExpLow(!CPC_GetExpLow());
	wxConfig::Get(false)->Write(wxT("links/exp_link"), CPC_GetExpLow());
	printf("/EXP input to CPC is LOW: %s\n", CPC_GetExpLow() ? "yes" : "no");

	OnCPCDiscInterfaceLinkUpdateUI();
}


void arnguiFrame::OnEnableFirmwareSupports16RomsUpdateUI()
{
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("FirmwareSupports16Roms"), Firmware_GetSupports16Roms());
}

void arnguiFrame::OnEnableAutodetectFirmwareSupports16RomsUpdateUI()
{
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("FirmwareAutodetectSupports16Roms"), Firmware_GetAutoDetect16Roms());
}

void arnguiFrame::OnEnableFirmwareSupports16Roms(wxCommandEvent & WXUNUSED(event))
{
	Firmware_SetSupports16Roms(!Firmware_GetSupports16Roms());
	OnEnableFirmwareSupports16RomsUpdateUI();

	wxConfig::Get(false)->Write(wxT("firmware/16_roms/supported"), Firmware_GetSupports16Roms());
}

void arnguiFrame::OnEnableAutodetectFirmwareSupports16Roms(wxCommandEvent & WXUNUSED(event))
{
	Firmware_SetAutoDetect16Roms(!Firmware_GetAutoDetect16Roms());
	OnEnableAutodetectFirmwareSupports16RomsUpdateUI();

	wxConfig::Get(false)->Write(wxT("firmware/16_roms/auto_detect"), Firmware_GetAutoDetect16Roms());
}

void arnguiFrame::OnCPCDiscInterfaceLinkUpdateUI()
{
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("menu_disc_interface_link"), !CPC_GetExpLow());
}

const int WindowUpdateMilliseconds = 40;

void arnguiFrame::OnIdle(wxIdleEvent &event)
{
	wxEventLoopBase *pActiveEventLoop = wxEventLoopBase::GetActive();
	if (pActiveEventLoop && !pActiveEventLoop->IsMain())
	{
		event.RequestMore(true);

		return;
	}

	//if (!m_bAllowIdle)
	//	return;
	/* do not display FPS if debugger is stopped; no point */
	/* also do not update if debugger is stopped; I think this is handled seperately */
	/* if we are stepping we would also get a bad fps value, only if it's been actively running
	for a while we should show fps */
	//	if (!Debug_IsStopped())
	//	{
//			int FPS = wxGetApp().m_PlatformSpecific.GetAverageFPS();
	//	printf("%d%%\n", FPS);
	//	}
///	wxString sFPS;
//	sFPS.Printf(wxT("%d%%"), FPS);
//	SetTitle(sFPS);

	if (m_pEmulationWindow != NULL)
	{

		// update emulation window on idle
		m_pEmulationWindow->OnIdle(event);
	}


	// we capture the current and previous time we recorded and
	// only update if the time is greater than our interval. This ensures	
	// updates are not too frequent

	wxLongLong MillisecondsCurrent = ::wxGetLocalTimeMillis();
	wxLongLong Diff = MillisecondsCurrent - m_PreviousUpdateTime;
	if (Diff >= WindowUpdateMilliseconds)
	{

		// update all
		for (unsigned int i = 0; i != m_UpdatableDialog.GetCount(); i++)
		{
			UpdatableDialog *pDialog = m_UpdatableDialog.Item(i);
			pDialog->TimedUpdate();
		}
		m_PreviousUpdateTime = ::wxGetLocalTimeMillis();
	}


	event.RequestMore(true);

}


XRCID_Map SystemCartIDs_Map[] =
{
	{ XRCID("menu_system_cart_en"), 0 },
	{ XRCID("menu_system_cart_fr"), 1 },
	{ XRCID("menu_system_cart_fr2"), 2 },
	{ XRCID("menu_system_cart_es"), 3 },
};

void arnguiFrame::OnCPCSetDefaultCartridge(wxCommandEvent &event)
{
	int DefaultCartridgeID = Value_From_XRCID(SystemCartIDs_Map, ARRAY_ELEMENT_COUNT(SystemCartIDs_Map), event.GetId());
	wxGetApp().InsertDefaultCartridge(DefaultCartridgeID);
	wxConfig::Get(false)->Write(wxT("DefaultCartridgeId"), DefaultCartridgeID);

	OnCPCSetDefaultCartridgeUpdateUI();
}

void arnguiFrame::OnCPCSetDefaultCartridgeUpdateUI()
{
	RefreshMultipleItemStatesRadio(SystemCartIDs_Map, ARRAY_ELEMENT_COUNT(SystemCartIDs_Map), wxGetApp().DefaultCartridge());
}


XRCID_Map KeyboardModeIDs_Map[] =
{
	{ XRCID("KeyboardModePositional"), 0 },
	{ XRCID("KeyboardModeTranslated"), 1 },
};

void arnguiFrame::OnCPCKeyboardMode(wxCommandEvent &event)
{
	int KeyboardMode = Value_From_XRCID(KeyboardModeIDs_Map, ARRAY_ELEMENT_COUNT(KeyboardModeIDs_Map), event.GetId());
	wxGetApp().SetKeyboardMode(KeyboardMode);
	wxConfig::Get(false)->Write(wxT("keyboard/mode"), KeyboardMode);

	OnCPCKeyboardModeUpdateUI();
}

void arnguiFrame::OnCPCKeyboardModeUpdateUI()
{
	RefreshMultipleItemStatesRadio(KeyboardModeIDs_Map, ARRAY_ELEMENT_COUNT(KeyboardModeIDs_Map), Keyboard_GetMode());
}


XRCID_Map KeyboardHardwareIDs_Map[] =
{
	{ XRCID("KeyboardHardwareCPC664"), 0 },
	{ XRCID("KeyboardHardwareCPC6128"), 1 },
};

void arnguiFrame::OnCPCKeyboardHardware(wxCommandEvent &event)
{
	int KeyboardHardware = Value_From_XRCID(KeyboardHardwareIDs_Map, ARRAY_ELEMENT_COUNT(KeyboardHardwareIDs_Map), event.GetId());
	if (KeyboardHardware == 0)
	{
		Keyboard_EnableClash(FALSE);
	}
	else
	{
		Keyboard_EnableClash(TRUE);
	}

	wxConfig::Get(false)->Write(wxT("keyboard/hardware"), KeyboardHardware);

	OnCPCKeyboardHardwareUpdateUI();
}

void arnguiFrame::OnCPCKeyboardHardwareUpdateUI()
{
	RefreshMultipleItemStatesRadio(KeyboardHardwareIDs_Map, ARRAY_ELEMENT_COUNT(KeyboardHardwareIDs_Map), Keyboard_IsClashEnabled() ? 1 : 0);
}


XRCID_Map KeyboardLanguageIDs_Map[] =
{
	{ XRCID("KeyboardLanguageEnglish"), KEYBOARD_LANGUAGE_ID_ENGLISH },
	{ XRCID("KeyboardLanguageFrench"), KEYBOARD_LANGUAGE_ID_FRENCH },
	{ XRCID("KeyboardLanguageSpanish"), KEYBOARD_LANGUAGE_ID_SPANISH },
	{ XRCID("KeyboardLanguageDanish"), KEYBOARD_LANGUAGE_ID_DANISH },
};

void arnguiFrame::OnCPCKeyboardLanguage(wxCommandEvent &event)
{
	int KeyboardLanguage = Value_From_XRCID(KeyboardLanguageIDs_Map, ARRAY_ELEMENT_COUNT(KeyboardLanguageIDs_Map), event.GetId());
	Keyboard_SetLanguage(KeyboardLanguage);
	wxConfig::Get(false)->Write(wxT("keyboard/language"), KeyboardLanguage);
	printf("Keyboard language: %d\n", KeyboardLanguage);

	OnCPCKeyboardLanguageUpdateUI();
}

void arnguiFrame::OnCPCKeyboardLanguageUpdateUI()
{
	RefreshMultipleItemStatesRadio(KeyboardLanguageIDs_Map, ARRAY_ELEMENT_COUNT(KeyboardLanguageIDs_Map), Keyboard_GetLanguage());
}

void arnguiFrame::OnCPCKeyboardAutoDetectLanguage(wxCommandEvent & WXUNUSED(event))
{
	Keyboard_SetAutoDetectLanguage(!Keyboard_GetAutoDetectLanguage());

	Keyboard_DetectLanguage();

	wxConfig::Get(false)->Write(wxT("keyboard/language_detect"), (Keyboard_GetAutoDetectLanguage() == TRUE));

	OnCPCKeyboardLanguageUpdateUI();
	OnCPCKeyboardAutoDetectLanguageUpdateUI();
}

void arnguiFrame::OnCPCKeyboardAutoDetectLanguageUpdateUI()
{
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("AutoDetectLanguage"), Render_GetScanlines());
}

#if 0
// to be improved
int arnguiFrame::MenuIdToKeyboardPositionalSetId(int nMenuId)
{
	if (nMenuId == XRCID("PositionalSetQwerty"))
	{
		return 0;
	}
	if (nMenuId == XRCID("PositionalSetQwertz"))
	{
		return 1;
	}
	if (nMenuId == XRCID("PositionalSetAzerty"))
	{
		return 2;
	}
	if (nMenuId == XRCID("PositionalSetSpanish"))
	{
		return 3;
	}
	return 0;
}

void arnguiFrame::OnCPCPositionalSet(wxCommandEvent &event)
{
	int Keyset = MenuIdToKeyboardPositionalSetId(event.GetId());
	Keyboard_SetPositionalSet(Keyset);
	wxConfig::Get(false)->Write(wxT("keyboard/positional/set"), Keyset);
	wxGetApp().SetKeySet(Keyset);
}

void arnguiFrame::OnCPCPositionalSetUpdateUI()
{
	int Keyset = MenuIdToKeyboardPositionalSetId(event.GetId());
	bool bCheck = false;
	if (Keyset == Keyboard_GetPositionalSet())
	{
		bCheck = true;
	}
	event.Check(bCheck);
}
#endif


XRCID_Map AudioOutputIDs_Map[] =
{
	{ XRCID("m_AudioSpeaker"), CPC_AUDIO_OUTPUT_MONO_SPEAKER },
	{ XRCID("m_AudioStereoConnector"), CPC_AUDIO_OUTPUT_STEREO },
	{ XRCID("m_AudioExpansion"), CPC_AUDIO_OUTPUT_MONO_EXPANSION },
};

void arnguiFrame::OnCPCAudioOutput(wxCommandEvent &event)
{
	int  OutputID = Value_From_XRCID(AudioOutputIDs_Map, ARRAY_ELEMENT_COUNT(AudioOutputIDs_Map), event.GetId());
	wxConfig::Get(false)->Write(wxT("audio/output"), OutputID);
	Audio_SetOutput((CPC_AUDIO_OUTPUT_TYPE)OutputID);
	printf("Audio output: %d\n", (int)OutputID);

	if ((Audio_GetOutput() == CPC_AUDIO_OUTPUT_STEREO) && (wxGetApp().GetAudioChannels() == 1))
	{
		wxString sMessage;
		sMessage.Printf(wxT("Stereo sound output of the CPC/Plus/GX4000 will not be heard as good as it should be because you have selected mono sound output from your computer"));
		wxMessageBox(sMessage);
	}

	if ((Audio_GetOutput() == 1) && (wxGetApp().GetAudioChannels() == 1))
	{
		wxString sMessage;
		sMessage.Printf(wxT("Stereo sound output of the CPC/Plus/GX4000 will not be heard as good as it should be because you have selected mono sound output from your computer"));
		wxMessageBox(sMessage);
	}

	OnCPCAudioOutputUpdateUI();

}

void arnguiFrame::OnCPCAudioOutputUpdateUI()
{
	RefreshMultipleItemStatesRadio(AudioOutputIDs_Map, ARRAY_ELEMENT_COUNT(AudioOutputIDs_Map), Audio_GetOutput());
}


// to be improved; getting closer
const wxChar *arnguiFrame::MenuIdToConfigId(int nMenuId)
{
	if (nMenuId == XRCID("ID_CPCTYPE_CPC464_EN"))
	{
		return wxT("cpc464en");
	}
	if (nMenuId == XRCID("ID_CPCTYPE_CPC464_EN_DDI1"))
	{
		return wxT("cpc464en_ddi1");
	}
	if (nMenuId == XRCID("ID_CPCTYPE_CPC464_FR"))
	{
		return wxT("cpc464fr");
	}
	if (nMenuId == XRCID("ID_CPCTYPE_CPC464_ES"))
	{
		return wxT("cpc464es");
	}
	if (nMenuId == XRCID("ID_CPCTYPE_CPC464_DK"))
	{
		return wxT("cpc464dk");
	}
	if (nMenuId == XRCID("ID_CPCTYPE_CPC664_EN"))
	{
		return wxT("cpc664en");
	}
	if (nMenuId == XRCID("ID_CPCTYPE_CPC6128_EN"))
	{
		return wxT("cpc6128en");
	}
	if (nMenuId == XRCID("ID_CPCTYPE_CPC6128_FN"))
	{
		return wxT("cpc6128w");
	}
	if (nMenuId == XRCID("ID_CPCTYPE_CPC6128_FR"))
	{
		return wxT("cpc6128fr");
	}
	if (nMenuId == XRCID("ID_CPCTYPE_CPC6128_ES"))
	{
		return wxT("cpc6128es");
	}
	if (nMenuId == XRCID("ID_CPCTYPE_CPC6128_DK"))
	{
		return wxT("cpc6128dk");
	}
	if (nMenuId == XRCID("ID_CPCTYPE_464PLUS"))
	{
		return wxT("464plus");
	}
	if (nMenuId == XRCID("ID_CPCTYPE_6128PLUS"))
	{
		return wxT("6128plus");
	}
	if (nMenuId == XRCID("ID_CPCTYPE_6128PLUS_CASSETTE"))
	{
		return wxT("6128pluscas");
	}
	if (nMenuId == XRCID("ID_CPCTYPE_GX4000"))
	{
		return wxT("gx4000");
	}
	if (nMenuId == XRCID("ID_CPCTYPE_KCCOMPACT"))
	{
		return wxT("kccompact");
	}
	if (nMenuId == XRCID("ID_CPCTYPE_ALESTE"))
	{
		return wxT("aleste");
	}
	if (nMenuId == XRCID("ID_CHANGECONFIGURATION_CSD"))
	{
		return wxT("csd");
	}
	if (nMenuId == XRCID("ID_CPCTYPE_CPC6128_PARADOS_EN"))
	{
		return wxT("cpc6128paradosen");
	}

	return wxT("cpc6128");
}

void arnguiFrame::ChooseConfig(wxCommandEvent &event)
{
	const wxChar *sConfigID = MenuIdToConfigId(event.GetId());
	wxGetApp().ChooseConfig(sConfigID);

	/* clear autotype */
	AutoType_Finish();
	/* clear auto run */
	AutoRunFile_Finish();

	Computer_RestartPower();
	wxConfig::Get(false)->Write(wxT("ConfigName"), sConfigID);

	OnCPCKeyboardLanguageUpdateUI();
}

void arnguiFrame::ChooseConfigUpdateUI()
{
	//	const wxChar *sConfigID = MenuIdToConfigId(event.GetId());
	//	bool bCheck = false;
	//	if (sConfigID==wxGetApp().CurrentConfig())
	//	{
	//		bCheck = true;
	//	}
	//	event.Check(bCheck);
}



void arnguiFrame::OnCPCFullScreen(wxCommandEvent & WXUNUSED(event))
{
	EnsureFullscreen();
}

void arnguiFrame::EnsureFullscreen()
{
	if (!IsFullScreen())
	{
		ShowFullScreen(true);
	}
	// without this works well in windows switching back and forth.
	// (e.g. works with alt-tab etc)
	// but we can't choose the resolution in fullscreen
	//	m_pEmulationWindow->GoFullScreen();
	m_bFullScreen = true;
}

void arnguiFrame::EnsureWindowed()
{
	if (IsFullScreen())
	{
		ShowFullScreen(false);
	}

	// without this works well in windows switching back and forth.

	//	m_pEmulationWindow->GoWindowed();
	m_bFullScreen = false;
}


XRCID_Map EnableDriveIDs_Map[] =
{
	{ XRCID("menu_drivea_enable"), 0 },
	{ XRCID("menu_driveb_enable"), 1 },
	{ XRCID("menu_drivec_enable"), 2 },
	{ XRCID("menu_drived_enable"), 3 },
};

void arnguiFrame::OnCPCDriveEnable(wxCommandEvent &event)
{
	int nDriveID = Value_From_XRCID(EnableDriveIDs_Map, ARRAY_ELEMENT_COUNT(EnableDriveIDs_Map), event.GetId());
	FDD_Enable(nDriveID, !FDD_IsEnabled(nDriveID));
	wxString sConfig;
	sConfig.Printf(wxT("drive/%c/enable"), nDriveID + 'a');
	wxConfig::Get(false)->Write(sConfig, FDD_IsEnabled(nDriveID));

	OnCPCDriveEnableUpdateUI();
}

void arnguiFrame::OnCPCDriveEnableUpdateUI()
{
	RefreshMultipleItemStates(EnableDriveIDs_Map, ARRAY_ELEMENT_COUNT(EnableDriveIDs_Map), FDD_IsEnabled);
}


int arnguiFrame::MenuIdToInsertDriveId(int nMenuId)
{
	if (nMenuId == XRCID("menu_insert_disk_drivea"))
	{
		return 0;
	}
	if (nMenuId == XRCID("menu_insert_disk_driveb"))
	{
		return 1;
	}
	if (nMenuId == XRCID("menu_insert_disk_drivec"))
	{
		return 2;
	}
	if (nMenuId == XRCID("menu_insert_disk_drived"))
	{
		return 3;
	}

	return 0;
}

void arnguiFrame::OnCPCInsertDisk(wxCommandEvent &event)
{
	int nDriveID = MenuIdToInsertDriveId(event.GetId());


	InsertDiskGUI(nDriveID);
}

int arnguiFrame::MenuIdToReloadDriveId(int nMenuId)
{
	if (nMenuId == XRCID("menu_reload_disk_drivea"))
	{
		return 0;
	}
	if (nMenuId == XRCID("menu_reload_disk_driveb"))
	{
		return 1;
	}
	if (nMenuId == XRCID("menu_reload_disk_drivec"))
	{
		return 2;
	}
	if (nMenuId == XRCID("menu_reload_disk_drived"))
	{
		return 3;
	}

	return 0;
}

void arnguiFrame::OnCPCReloadDisk(wxCommandEvent &event)
{
	int nDriveID = MenuIdToReloadDriveId(event.GetId());
	ReloadDiskGUI(nDriveID);
}


int arnguiFrame::MenuIdToRemoveDriveId(int nMenuId)
{
	if (nMenuId == XRCID("menu_remove_disk_drivea"))
	{
		return 0;
	}
	if (nMenuId == XRCID("menu_remove_disk_driveb"))
	{
		return 1;
	}
	if (nMenuId == XRCID("menu_remove_disk_drivec"))
	{
		return 2;
	}
	if (nMenuId == XRCID("menu_remove_disk_drived"))
	{
		return 3;
	}

	return 0;
}

void arnguiFrame::OnCPCRemoveDisk(wxCommandEvent &event)
{
	int nDriveID = MenuIdToRemoveDriveId(event.GetId());
	char chDrive = (char)(nDriveID + 'A');

	if (!wxGetApp().m_Drives[nDriveID]->GetMediaInserted())
	{
		wxString sMessage;
		sMessage.Printf(wxT("No disc is inserted into drive %c."), chDrive);
		wxMessageBox(sMessage);
	}
	else
	{

		wxString sMessage;
		sMessage.Printf(wxT("Are you sure you want to remove the disk from drive %c?"), chDrive);
		wxMessageDialog dialog(this, sMessage, wxGetApp().GetAppName(), wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION);;
		if (dialog.ShowModal() == wxID_YES)
		{
			wxGetApp().m_Drives[nDriveID]->Unload();
		}
	}
}

XRCID_Map TurnDiskIDs_Map[] =
{
	{ XRCID("menu_turnoverdisk_drivea"), 0 },
	{ XRCID("menu_turnoverdisk_driveb"), 1 },
	{ XRCID("menu_turnoverdisk_drivec"), 2 },
	{ XRCID("menu_turnoverdisk_drived"), 3 },
};

void arnguiFrame::OnCPCTurnDiskToSideBInDrive(wxCommandEvent &event)
{
	int nDriveID = Value_From_XRCID(TurnDiskIDs_Map, ARRAY_ELEMENT_COUNT(TurnDiskIDs_Map), event.GetId());
	FDD_TurnDiskToSideB(nDriveID, !FDD_IsTurnDiskToSideB(nDriveID));

	wxString sConfig;
	sConfig.Printf(wxT("drive/%c/turn_disk_to_side_b"), (char)(nDriveID + 'a'));
	wxConfig::Get(false)->Write(sConfig, FDD_IsTurnDiskToSideB(nDriveID) ? true : false);

	OnCPCTurnDiskToSideBInDriveUpdateUI();
}

void arnguiFrame::OnCPCTurnDiskToSideBInDriveUpdateUI()
{
	RefreshMultipleItemStates(TurnDiskIDs_Map, ARRAY_ELEMENT_COUNT(TurnDiskIDs_Map), FDD_IsTurnDiskToSideB);
}

void arnguiFrame::OnCPCDebugger(wxCommandEvent & WXUNUSED(event))
{
	DebuggerWindow::CreateInstance(this);
}

void arnguiFrame::OnCPCGraphicsEditor(wxCommandEvent & WXUNUSED(event))
{
	GraphicsEditor::CreateInstance(this);
}

void arnguiFrame::OnFillScanlines(wxCommandEvent & WXUNUSED(event))
{
	Render_SetFillScanlines(!Render_GetFillScanlines());

	wxConfig::Get(false)->Write(wxT("display/fillscanlines"), (Render_GetFillScanlines() == TRUE));

	OnFillScanlinesUpdateUI();
}

void arnguiFrame::OnFillScanlinesUpdateUI()
{
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("FillScanlines"), Render_GetFillScanlines());
}


void arnguiFrame::OnEntireDisplay(wxCommandEvent & WXUNUSED(event))
{
	Render_SetFullDisplay(!Render_GetFullDisplay());

	wxConfig::Get(false)->Write(wxT("display/fulldisplay"), (Render_GetFullDisplay() == TRUE));

	m_pEmulationWindow->UpdateSize(m_bFullScreen);

	OnEntireDisplayUpdateUI();
}

void arnguiFrame::OnEntireDisplayUpdateUI()
{
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("EntireDisplay"), Render_GetFullDisplay());
}


void arnguiFrame::OnDoubleHeight(wxCommandEvent & WXUNUSED(event))
{
	Render_SetScanlines(!Render_GetScanlines());

	m_pEmulationWindow->UpdateSize(m_bFullScreen);

	wxConfig::Get(false)->Write(wxT("display/double_height"), (Render_GetScanlines() == TRUE));

	OnDoubleHeightUpdateUI();
}

void arnguiFrame::OnDoubleHeightUpdateUI()
{
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("DoubleHeight"), Render_GetScanlines());
}

void arnguiFrame::OnMonitorSettings(wxCommandEvent & WXUNUSED(event))
{
	CMonitorSettingsDialog::CreateInstance(this);

}

void arnguiFrame::OnDrawSync(wxCommandEvent & WXUNUSED(event))
{
	Monitor_EnableDrawSync(!Monitor_GetDrawSync());

	wxConfig::Get(false)->Write(wxT("display/sync"), (Monitor_GetDrawSync() == TRUE));

	OnDrawSyncUpdateUI();
}

void arnguiFrame::OnDrawSyncUpdateUI()
{
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("DrawSync"), Monitor_GetDrawSync());
}

void arnguiFrame::OnDrawBlanking(wxCommandEvent & WXUNUSED(event))
{
	Computer_SetDrawBlanking(!Computer_GetDrawBlanking());

	wxConfig::Get(false)->Write(wxT("display/blanking"), (Computer_GetDrawBlanking() == TRUE));

	OnDrawBlankingUpdateUI();
}

void arnguiFrame::OnDrawBlankingUpdateUI()
{
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("DrawBlanking"), Computer_GetDrawBlanking());
}


void arnguiFrame::OnDrawBorder(wxCommandEvent & WXUNUSED(event))
{
	Computer_SetDrawBorder(!Computer_GetDrawBorder());

	wxConfig::Get(false)->Write(wxT("display/border"), (Computer_GetDrawBorder() == TRUE));

	OnDrawBorderUpdateUI();
}

void arnguiFrame::OnDrawBorderUpdateUI()
{
	arnguiApp::RefreshSingleItemState(m_MenuBar, XRCID("DrawBorder"), Computer_GetDrawBorder());
}


XRCID_Map SpeedIDs_Map[] =
{
	{ XRCID("EmulationSpeed200"), 200 },
	{ XRCID("EmulationSpeed100"), 100 },
	{ XRCID("EmulationSpeed50"), 50 },
	{ XRCID("EmulationSpeed25"), 25 },
	{ XRCID("EmulationSpeedUnlimited"), 0 },
};

void arnguiFrame::OnSpeed(wxCommandEvent & event)
{
	int nPercent = Value_From_XRCID(SpeedIDs_Map, ARRAY_ELEMENT_COUNT(SpeedIDs_Map), event.GetId());

	CPC_SetEmuSpeedPercent(nPercent);

	wxConfig::Get(false)->Write(wxT("emulation_speed/percent"), CPC_GetEmuSpeedPercent());
	printf("Emulation speed: %d%%\n", CPC_GetEmuSpeedPercent());

	OnSpeedUpdateUI();

	wxGetApp().m_PlatformSpecific.ResetTiming();

}

void arnguiFrame::OnSpeedUpdateUI()
{
	RefreshMultipleItemStatesRadio(SpeedIDs_Map, ARRAY_ELEMENT_COUNT(SpeedIDs_Map), CPC_GetEmuSpeedPercent());
}


XRCID_Map PositionalKeyboardSetIds_Map[] =
{
	{ XRCID("m_PositionalSetInternational"), 0},
	{ XRCID("m_PositionalAzerty"), 1 },
	{ XRCID("m_PositionalQwertz"), 2 },
	{ XRCID("m_PositionalSetCustom"), 3 },
};




void arnguiFrame::OnPositionalKeyboardSet(wxCommandEvent & event)
{
	int nSet= Value_From_XRCID(PositionalKeyboardSetIds_Map, ARRAY_ELEMENT_COUNT(PositionalKeyboardSetIds_Map), event.GetId());

	wxGetApp().m_PlatformSpecific.SetKeySet(nSet);

	wxConfig::Get(false)->Write(wxT("keyboard/positional/set"), nSet);
	printf("Positional keyboard set: %d%%\n", nSet);

	OnPositionalKeyboardSetUpdateUI();

	wxGetApp().m_PlatformSpecific.ResetTiming();

}

void arnguiFrame::OnPositionalKeyboardSetUpdateUI()
{
	int nSet = 0;
	wxConfig::Get(false)->Read(wxT("keyboard/positional/set"), &nSet, 0 );

	RefreshMultipleItemStatesRadio(PositionalKeyboardSetIds_Map, ARRAY_ELEMENT_COUNT(PositionalKeyboardSetIds_Map), nSet);
}


XRCID_Map SpriteIDs_Map[] =
{
	{ XRCID("EnableSprite0"), (1 << 0) },
	{ XRCID("EnableSprite1"), (1 << 1) },
	{ XRCID("EnableSprite2"), (1 << 2) },
	{ XRCID("EnableSprite3"), (1 << 3) },
	{ XRCID("EnableSprite4"), (1 << 4) },
	{ XRCID("EnableSprite5"), (1 << 5) },
	{ XRCID("EnableSprite6"), (1 << 6) },
	{ XRCID("EnableSprite7"), (1 << 7) },
	{ XRCID("EnableSprite8"), (1 << 8) },
	{ XRCID("EnableSprite9"), (1 << 9) },
	{ XRCID("EnableSprite10"), (1 << 10) },
	{ XRCID("EnableSprite11"), (1 << 11) },
	{ XRCID("EnableSprite12"), (1 << 12) },
	{ XRCID("EnableSprite13"), (1 << 13) },
	{ XRCID("EnableSprite14"), (1 << 14) },
	{ XRCID("EnableSprite15"), (1 << 15) },
};

void arnguiFrame::OnEnableSprite(wxCommandEvent & event)
{
	int nMask = Value_From_XRCID(SpriteIDs_Map, ARRAY_ELEMENT_COUNT(SpriteIDs_Map), event.GetId());

	ASIC_SetSpriteOverride(ASIC_GetSpriteOverride() ^ nMask);

	OnEnableSpriteUpdateUI();
}

void arnguiFrame::OnEnableSpriteUpdateUI()
{

	for (size_t i = 0; i != sizeof(SpriteIDs_Map) / sizeof(SpriteIDs_Map[0]); i++)
	{
		wxMenuItem *pMenuItem = m_MenuBar->FindItem(SpriteIDs_Map[i].id);
		if (pMenuItem != NULL)
		{
			// ensure it's selected
			pMenuItem->Enable(true);

			bool bCheck = ((ASIC_GetSpriteOverride() & SpriteIDs_Map[i].nValue) != 0);
			// if the value is the chosen one, check it else clear the check
			pMenuItem->Check(bCheck);

		}
	}

	//RefreshMultipleItemStatesRadio(SpriteIDs_Map, ARRAY_ELEMENT_COUNT(SpriteIDs_Map), CPC_GetEmuSpeedPercent());
}



XRCID_Map SpeakerIDs_Map[] =
{
	{ XRCID("SpeakerVol0"), 0 },
	{ XRCID("SpeakerVol50"), SPEAKER_VOLUME_MAX / 2 },
	{ XRCID("SpeakerVol100"), SPEAKER_VOLUME_MAX },
};

void arnguiFrame::OnCPCAudioSpeaker(wxCommandEvent & event)
{
	int nVolume = Value_From_XRCID(SpeakerIDs_Map, ARRAY_ELEMENT_COUNT(SpeakerIDs_Map), event.GetId());

	CPC_SetSpeakerVolume(nVolume);

	wxConfig::Get(false)->Write(wxT("audio/speaker_volume"), CPC_GetSpeakerVolume());
	printf("Speaker Volume: %d%%\n", CPC_GetSpeakerVolume());

	OnCPCAudioSpeakerUpdateUI();
}

void arnguiFrame::OnCPCAudioSpeakerUpdateUI()
{
	RefreshMultipleItemStatesRadio(SpeakerIDs_Map, ARRAY_ELEMENT_COUNT(SpeakerIDs_Map), CPC_GetSpeakerVolume());
}



XRCID_Map ScaleIDs_Map[] =
{
	{ XRCID("WindowScale100"), 100 },
	{ XRCID("WindowScale200"), 200 },
	{ XRCID("WindowScale300"), 300 },
	{ XRCID("WindowScale400"), 400 },
};

void arnguiFrame::OnWindowScale(wxCommandEvent & event)
{
	int nPercent = Value_From_XRCID(ScaleIDs_Map, ARRAY_ELEMENT_COUNT(ScaleIDs_Map), event.GetId());

	CPC_SetWindowScale(nPercent);

	wxConfig::Get(false)->Write(wxT("window_scale/percent"), CPC_GetWindowScale());
	printf("Window scale: %d%%\n", CPC_GetWindowScale());

	m_pEmulationWindow->UpdateSize(m_bFullScreen);

	OnWindowScaleUpdateUI();
}

void arnguiFrame::OnWindowScaleUpdateUI()
{
	RefreshMultipleItemStatesRadio(ScaleIDs_Map, ARRAY_ELEMENT_COUNT(ScaleIDs_Map), CPC_GetWindowScale());
}




void arnguiFrame::OnCPCMediaStatus(wxCommandEvent & WXUNUSED(event))
{
	MediaStatusDialog::CreateInstance(this);
}


void arnguiFrame::OnExpansionDevices(wxCommandEvent & WXUNUSED(event))
{
	DevicesDialog dialog(this, CONNECTION_EXPANSION);
	dialog.ShowModal();
}


void arnguiFrame::OnJoystickDevices(wxCommandEvent & WXUNUSED(event))
{
	DevicesDialog dialog(this, CONNECTION_JOYSTICK);
	dialog.ShowModal();
}

void arnguiFrame::OnInternalDevices(wxCommandEvent & WXUNUSED(event))
{
	DevicesDialog dialog(this, CONNECTION_INTERNAL);
	dialog.ShowModal();
}

void arnguiFrame::OnPrinterDevices(wxCommandEvent & WXUNUSED(event))
{
	DevicesDialog dialog(this, CONNECTION_PRINTER);
	dialog.ShowModal();
}

void arnguiFrame::OnPrinterSetFile(wxCommandEvent & WXUNUSED(event))
{
	wxFileDialog openFileDialog(
		NULL,
		wxT("Save Printer Text To"),      // title
		wxT(""),
		wxT(""),
		wxT("Text Files (*.txt)|*.txt||"),
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if (openFileDialog.ShowModal() == wxID_CANCEL)
	{
		return;
	}
	printf("opening printer file\n");
	wxGetApp().OpenPrinterFile(openFileDialog.GetPath());
}

void arnguiFrame::OnPrinterCloseFile(wxCommandEvent & WXUNUSED(event))
{
	wxGetApp().ClosePrinterFile();
}

#if !defined(__WXMSW__)
void arnguiFrame::OnRegisterFileTypes(wxCommandEvent &event)
{
	wxMessageBox(wxT("This functionality is only available in the Windows build."),
		wxGetApp().GetAppName(), wxICON_WARNING | wxOK);
}
#endif

bool arnguiFrame::OpenROM(unsigned char **pData, unsigned long *pLength, wxString &sFilename, const wxString &sTitleSuffix)
{
	*pData = NULL;
	*pLength = 0;

	if (ROMFileType.Open(this, sFilename, sTitleSuffix))
	{
		/* try to load it */
		wxGetApp().LoadLocalFile(sFilename, pData, pLength);
		return true;
	}
	return false;
}


void arnguiFrame::OnFullScreenSettings(wxCommandEvent & WXUNUSED(event))
{
	FullScreenDialog dialog(this);
	dialog.ShowModal();
}




void arnguiFrame::OnAudioSettings(wxCommandEvent & WXUNUSED(event))
{
	AudioSettingsDialog dialog(this);
	if (dialog.ShowModal() == wxID_OK)
	{

		if (wxGetApp().GetAudioChannels() == 1)
		{
			wxString sMessage;
			sMessage.Printf(wxT("You have chosen a mono sound output.\n\nPlease choose stereo output to appreciate the stereo sound output of the CPC/Plus/GX4000"));
			wxMessageBox(sMessage);
		}


		wxGetApp().m_PlatformSpecific.ConfigureAudio();

		if (
			(wxGetApp().GetAudioFrequency() != SDLCommon::m_nActualFrequency) ||
			(wxGetApp().GetAudioChannels() != SDLCommon::m_nActualChannels) ||
			(wxGetApp().GetAudioBits() != SDLCommon::m_nActualBitsPerSample)
			)
		{
			wxString sMessage;
			sMessage.Printf(wxT("Unable to playback with the requested settings.\n\nRequested:\nFrequency: %d Hz\nChannels: %d\nBits Per Sample: %d\n\nObtained:\nFrequency: %d\nChannels: %d\nBits Per Sample: %d\n"),
				wxGetApp().GetAudioFrequency(), wxGetApp().GetAudioChannels(), wxGetApp().GetAudioBits(),
				SDLCommon::m_nActualFrequency, SDLCommon::m_nActualChannels, SDLCommon::m_nActualBitsPerSample);
			wxMessageBox(sMessage);
		}

	}
}

void arnguiFrame::OnPokeDatabase(wxCommandEvent & WXUNUSED(event))
{
	Winape_poke_DataBaseDialog dialog(this);
	dialog.ShowModal();
}


void arnguiFrame::OnSavePointSave(wxCommandEvent & WXUNUSED(event))
{
	wxGetApp().SaveSaveState();
}

void arnguiFrame::OnSavePointPrev(wxCommandEvent & WXUNUSED(event))
{
	wxGetApp().PrevSaveState();
}

void arnguiFrame::OnSavePointNext(wxCommandEvent & WXUNUSED(event))
{
	wxGetApp().NextSaveState();
}



