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
#ifndef ARNGUIAPP_H
#define ARNGUIAPP_H

// we want this on, but we can test SDL/SDL2 resizing if we turn this off
#if defined(__WXMSW__)
#ifndef WIN_EMBED_WINDOW
#define WIN_EMBED_WINDOW
#endif
#endif


typedef struct
{
	int m_nWindowWidth;
	int m_nWindowHeight;
	int m_nRenderWidth;
	int m_nRenderHeight;
	bool m_bUseHardwareScaling;	// sdl2 switches on hardware scaling
} WindowedRenderSettings;

typedef struct
{
	int m_nDisplayWidth;
	int m_nDisplayHeight;
	int m_nRenderWidth;
	int m_nRenderHeight;
	bool m_bUseHardwareScaling;	// sdl2 switches on hardware scaling
} FullscreenRenderSettings;

#include <wx/app.h>
#include <wx/cmdline.h>
#include <wx/config.h>
#include <wx/wfstream.h>
#include <wx/snglinst.h>
#include <wx/arrstr.h>
#include <wx/menu.h>

#ifndef wxOVERRIDE
#define wxOVERRIDE
#endif

#define BEGIN_TIMING(x) \
    wxStopWatch sw; \
    printf(x " timing begin:\n")

#define END_TIMING(x) \
  printf(x "took %lu\n", sw.Time());

#ifdef USE_SDL2
#include "sdl2/SDL2PlatformSpecific.h"
#endif

typedef void(*VOID_PTR);

#ifdef USE_SDL
#include "sdl/SDLPlatformSpecific.h"
#endif



#if 0

class PlayList
{
public:
	void Next();
	void Prev();



};

#endif // 0

// we could have one of these per game..?
// so they can continue from where they left off?
class SavePoint
{
public:
	SavePoint()
	{
		m_nCurrentSavePoint = 0;
}

	void AddSavepoint();        // add a new savepoint. all after are cleared

	void GoForwards();
	void ClearSavePoints();
	void ClearSavePointsAfter(int nPos);
	void GoBack();
	void LoadSavePoint(size_t nPos);
private:
	// one we have currently loaded
	int m_nCurrentSavePoint;
	wxArrayString m_SavePoints;
};

class arnguiFrame;
class wxListCtrl;

#include "Media.h"

WX_DEFINE_ARRAY(Media *, MediaArray);

class arnguiConfig
{
public:
	wxString m_sName;
	wxString m_sFilename;
};


typedef enum
{
	ACTION_CPC_NONE = 0,
	ACTION_CPC_RESET,               // reset with question
	ACTION_CPC_RESET_IMMEDIATE,		// reset immediate
	ACTION_TOGGLE_FULLSCREEN,       // toggle full screen
	ACTION_DECREASE_WARP_FACTOR,
	ACTION_INCREASE_WARP_FACTOR,
	ACTION_ZOOM_IN,
	ACTION_ZOOM_OUT,
	ACTION_INSERT_DISK_DRIVE_A,
	ACTION_INSERT_DISK_DRIVE_B,
	ACTION_INSERT_CARTRIDGE,
	ACTION_LOAD_SNAPSHOT,
	ACTION_SAVE_SNAPSHOT,
	ACTION_INSERT_TAPE,
	ACTION_SAVE_SCREENSHOT,
	ACTION_NEXT_PLAYLIST,
	ACTION_PREV_PLAYLIST,
	ACTION_PREV_SAVESTATE,
	ACTION_NEXT_SAVESTATE,
	ACTION_SAVE_SAVESTATE,
	ACTION_QUIT,				// quit with question
	ACTION_QUIT_IMMEDIATE,		// quit immediately no questions
	ACTION_SHOW_OSD,
	ACTION_TOGGLE_KEYSTICK_ACTIVE,

	// keep last
	ACTION_CODE_NUM
} ACTION_CODE;


typedef enum
{
	BUILTIN_MEDIA_UNKNOWN,
	BUILTIN_MEDIA_ROM, /* rom etc */
	BUILTIN_MEDIA_CART, /* cpr, etc */
	BUILTIN_MEDIA_DISK,	/* dsk, etc */
	BUILTIN_MEDIA_TAPE, /* cdt etc */
	BUILTIN_MEDIA_SNAPSHOT
} BUILTIN_MEDIA_TYPE;


typedef struct
{
	unsigned int m_nKeyCode;            // host keycode
	bool m_bShift;                      // true if shift should be pressed on host
	bool m_bControl;                    // true if control should be pressed on host
	bool m_bAlt;                        // true if alt should bre pressed on host
	ACTION_CODE m_Action;               // the action to perform
} ActionKey;


WX_DEFINE_ARRAY(arnguiConfig *, arnguiConfigArray);

class arnguiApp : public wxApp
{
private:
	wxString m_sConfig;
	int m_nDefaultCartridge;
	wxConfig *m_pConfig;
	static const wxCmdLineEntryDesc m_CommandLineDesc[];


	arnguiFrame* frame;
	void ScanConfigs(const wxString &sDirectory, arnguiConfigArray &configList);
	void ScanConfigsAppThenLocal(const wxString &sConfig, arnguiConfigArray &configList);

	void ReadConfig(const wxString &sFilename);

	arnguiConfigArray m_ComputerConfigs;
	arnguiConfigArray m_KeyboardConfigs;
	arnguiConfigArray m_JoystickConfigs;


public:
#if ((defined(GTK2_EMBED_WINDOW) && defined(__WXGTK__)) || (defined(MAC_EMBED_WINDOW) && defined(__WXMAC__)))
	int *GetKeySetPtr(int nSet);
#endif
	arnguiApp();
	PlatformSpecific m_PlatformSpecific;
	void ReadPositionalKeyboardConfiguration();
	void WritePositionalKeyboardConfiguration();

	void Log(const char *message, ...);

	wxFileOutputStream *m_pPrinterOutputStream;
	int ExpressionEvaluate(const wxString &str);
	virtual void OnInitCmdLine(wxCmdLineParser& parser) wxOVERRIDE;
	virtual bool OnCmdLineParsed(wxCmdLineParser& parser) wxOVERRIDE;
	const char *GetActionCodeName(ACTION_CODE nAction);

	void NextSaveState();
	void PrevSaveState();
	void SaveSaveState();

	void RestartPower(bool bShowMessage, bool bSetBreakpoint);
	void RestartReset(bool bShowMessage, bool bSetBreakpoint);

	MediaArray m_Media;
	DiskMedia *m_Drives[4];
	CassetteMedia *m_Cassette;
	CartridgeMedia *m_Cartridge;
	SnapshotMedia *m_Snapshot;
	wxMessageOutput *m_OldMessageOutput;

	int GetAudioFrequency();
	int GetAudioBits();
	int GetAudioChannels();

	void SetAudioFrequency(int nFrequency);
	void SetAudioBits(int nBits);
	void SetAudioChannels(int nChannels);

	int *GetKeySet();
	void SetKeySet(int nSet);

	int GetNumBuiltinMedia();
	const wxChar *GetBuiltinMediaDisplayName(int Index);
	BUILTIN_MEDIA_TYPE GetBuiltinMediaType(int Index);

	void GetBuiltinMediaDataByIndex(int Index, unsigned char **ppData, unsigned long *ppLength);
	void GetBuiltinMediaDataByPath(const wxString &sPath, unsigned char **ppData, unsigned long *ppLength);
	void GetBuiltinMediaDataByID(int nID, unsigned char **ppData, unsigned long *ppLength);
	bool IsBuiltinMediaPath(const wxString &sPath);
	wxString GetBuiltinMediaPath(int Index);

	void SetColumnSize(wxListCtrl *pListCtrl, int nColumn);


	void OnQueryEndSession(wxCloseEvent &event);

	void Config_CPC464EN();
	void Config_CPC464ES();
	void Config_CPC464FR();
	void Config_CPC464DK();
	void Config_CPC6128DK();
	void Config_CPC664EN();
	void Config_464Plus();
	void Config_GX4000();
	void Config_CSD();
	void Config_6128Plus();
	void Config_6128Plus_Cassette();
	void Config_CPC6128EN();
	void Config_CPC6128ENParados();
	void Config_CPC6128ES();
	void Config_CPC6128W();
	void Config_CPC6128FR();
	void Config_KCC();
	void Config_KCCWithDDI1();
	void Config_Aleste();
	void Config_CPC464EN_DDI1();

	virtual bool OnInit() wxOVERRIDE;
	virtual int OnExit() wxOVERRIDE;

	void InsertDefaultCartridge(int nID);
	int DefaultCartridge();

	const wxString &CurrentConfig();
	void ChooseConfig(const wxString &);
	void RefreshJoystick();
	void CloseJoysticks();
	void WarnDrive(int nDrive);


	void InsertOrAutoStartMedia(const wxString &sFilename, bool bAutoStart);

	//   void InsertCartridge(const wxString &sFilename);

	//   void LoadSnapshot(const wxString &sFilename);
	bool SaveSnapshot(const wxString &sFilename, SNAPSHOT_OPTIONS *pOptions, SNAPSHOT_MEMORY_BLOCKS *pBlocks);
	bool LoadSnapshot(const wxString &sFilename);

	bool LoadLocalFile(const wxString &sFilename, unsigned char **ppPtr, unsigned long *pLength);

	bool CanSave(const wxString &sFilename);
	bool SaveLocalFile(const wxString &sFilename, unsigned char *pPtr, unsigned long nLength);
	bool SaveLocalFile(const wxString &sFilename, const wxString &sText);
	void WriteDisk(int nDrive);

	// save a thumbnail
	void SaveThumbnail(const wxString &sFilename);

	bool SaveScreenshot(const wxString &sFilename, bool bAsThumbnail);

	//   void InsertDisk(int nDrive, const wxString &sFilename);
	//   bool RemoveDisk(int nDrive);

	static wxString GetAppPath();
	static wxString GetROMFullPath();
	static wxString GetGUIFullPath();
	static wxString GetIconsFullPath();

	wxSingleInstanceChecker *m_checker;


	bool ReadConfigColour(const wxString &sName, wxColour &Colour, const wxColour &Default);
	void WriteConfigColour(const wxString &sName, const wxColour &Colour);

	bool ReadConfigWindowPosAndSize(const wxString &sName, wxWindow *pWindow);
	void WriteConfigWindowPosAndSize(const wxString &sName, const wxWindow *pWindow);

	bool ReadConfigWindowPos(const wxString &sName, wxWindow *pWindow);
	void WriteConfigWindowPos(const wxString &sName, const wxWindow *pWindow);

	void ReadConfigListCtrl(const wxString &sName, wxListCtrl *pWindow);
	void WriteConfigListCtrl(const wxString &sName, const wxListCtrl *pWindow);

	void OpenPrinterFile(const wxString &sFilename);
	void ClosePrinterFile(void);

	void ProcessAction(ACTION_CODE nAction);
	void SetTranslatedKey(const char *sUTF8String);

	bool ReadConfigWindowSize(const wxString &sName, wxWindow *pWindow);
	void WriteConfigWindowSize(const wxString &sName, const wxWindow *pWindow);

	void CreateUserDataDir();

	int FindBuiltinMediaByPath(const wxString &sPath);

	bool PickBuiltInMedia(BUILTIN_MEDIA_TYPE nType, const wxString &sROMName, wxString &sId, unsigned char **ppData, unsigned long *pLength);

	void EnsureWindowVisible(wxWindow *pWindow);

	int GetFullScreenMode();
	void SetFullScreenMode(int nMode);

	void SetKeyboardMode(int KeyboardMode);

	void ProcessCommandLineFromString(const wxString &str);

	void HandleDropFile(wxString sFile);

	static void RefreshSingleItemState(wxMenuBar *pMenuBar, long id, BOOL bState);

private:
	void CommandLineParsed(wxCmdLineParser &);
	void ExecuteCommandLine();

	bool m_bDisplaySplash;
	bool m_bAudio;
	bool m_bFullScreen;
	bool m_bJoystick;
	wxString m_sConfigRequested;
	wxString m_sTapeFile;
	wxString m_sCartridgeFile;
	wxString m_sSnapshotFile;
	wxString m_sDiskFile[4];
	wxString m_sAutoType;
	wxArrayString m_sLabelSets;
	wxString m_sAutoStart;
	wxString m_sDiskFileNoDrive;
	wxArrayString m_sMedia;
	long m_nCRTCType;
	int m_nFullScreenMode;
	int m_nAudioFrequency;
	int m_nAudioBits;
	int m_nAudioChannels;
	int m_nThumbnailWidth;
	int m_nThumbnailHeight;
	SavePoint m_SavePoint;


#if defined(__WXMAC__)
	virtual void MacNewFile() wxOVERRIDE;
	virtual void MacOpenFile(const wxString &sFilename) wxOVERRIDE;
	virtual void MacOpenFiles(const wxArrayString &fileNames) wxOVERRIDE;
	virtual void MacOpenURL(const wxString &url) wxOVERRIDE;
	virtual void MacReopenApp() wxOVERRIDE;
#endif	
	void RestoreJoystickConfig(const wxString &sConfigPrefix, int id, const char *s);

	DECLARE_EVENT_TABLE()
};

DECLARE_APP(arnguiApp)

#endif // ARNGUIAPP_H
