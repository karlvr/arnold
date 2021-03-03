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

/*
All versions of wxwidgets seem to have memory leaks. including 3.0.
I need to find a way around them.

check to ensure all variables are initialised in all classes.

check sdl2 keyboard feedback is working, same for normal sdl after it was split.



*/

#include "arnguiApp.h"
#include "arnguiMain.h"
#include "EmuWindow.h"
#include "LoadBinaryDialog.h"
#include "DebuggerDialog.h"
#include "GraphicsEditor.h"
#include "PokeMemoryDialog.h"
#include "KeyJoyDialog.h"
#include "connection.h"
#include "IntClientData.h"
#include "SnapshotLoadDialog.h"
//#include "ArchiveDialog.h"

#if defined(__WXMAC__)
// for linking on Mac. Works with Linux
#ifdef main
#undef main
#endif //main
#endif

int PALDeviceIndex = -1;

#if defined(__WXGTK__)
#ifdef main
#undef main
#endif //main
IMPLEMENT_APP_NO_MAIN(arnguiApp)
#else
IMPLEMENT_APP(arnguiApp)
#endif

#include <wx/xrc/xmlres.h>
#include <wx/stdpaths.h>
#include <wx/choicdlg.h>
#include <wx/ipc.h>
#include <wx/listctrl.h>
#include <wx/wfstream.h>
#include <locale.h>
#include <wx/tarstrm.h>
#include <wx/filesys.h>
#include <wx/fs_zip.h>
#include <wx/fs_arc.h>
#include <wx/fs_filter.h>
#include <wx/splash.h>
#include <wx/dir.h>
#include <wx/fileconf.h>
#include <wx/msgout.h>
#include <wx/tokenzr.h>
#include <wx/sysopt.h>

#ifndef min
#define min(a,b) ((a)<(b)) ? (a) : (b)
#endif

extern "C"
{
#include "../cpc/cpc.h"
#include "../cpc/asic.h"
#include "../cpc/diskimage/diskimg.h"
#include "../cpc/multface.h"
#include "../cpc/winape_poke_database.h"
#include "../cpc/debugger/gdebug.h"
#include "../cpc/jukebox.h"
#include "../cpc/joystick.h"
#include "../cpc/aleste.h"
#include "../cpc/kcc.h"
#include "../cpc/wav.h"
	//#include "../cpc/audioevent.h"

#include "../cpc/tzx.h"
#include "../cpc/autotype.h"
#include "../cpc/autorunfile.h"
#include "../cpc/debugger/labelset.h"
#include "../cpc/dumpym.h"
#include "../cpc/megarom.h"
#include "../cpc/brunword.h"
#include "../cpc/debugger/memdump.h"
#include "../cpc/debugger/dissasm.h"
#include "../cpc/debugger/stack.h"
#include "../cpc/debugger/breakpt.h"
#include "../cpc/debugger/parse.h"
#include "../cpc/pal.h"
#include "../cpc/amdrum.h"
#include "../cpc/amram2.h"
#include "../cpc/megarom.h"
#include "../cpc/brunword.h"
#include "../cpc/cpcay.h"
#include "../cpc/speech.h"
#include "../cpc/kempston.h"
#include "../cpc/magnum.h"
#include "../cpc/emudevice.h"
#include "../cpc/snapshot.h"
#include "../cpc/fdi.h"
#include "../cpc/fdd.h"
#include "../cpc/cassette.h"
#include "../cpc/printer.h"
#include "../cpc/render.h"
#include "../cpc/debugger/breakpt.h"
#include "../cpc/keyjoy.h"
#include "../cpc/ddi.h"
	extern void rombo_Init();
	extern void MHT64K_Init();
	extern void VortexRAM_Init();
	extern void DiscWizard_Init();
	extern void HackIt_Init();
	extern void hexam_Init();
	extern void Symbiface_Init();
	extern void uide_Init();
	extern void DobbertinHD20_Init();
	extern void Vortexwd20_Init();
	extern void TransTape_Init();
#include "../cpc/ramrom.h"
	extern void MirageImager_Init();
	extern void Westphaser_Init();
	extern void Gunstick_Init();
	extern void AmxMouse_Init();
	extern void TrojanLightPhaserDevice_Init();
	extern int DigiblasterDevice_Init();
	extern void PrintToFileDevice_Init();
	extern void InicronRAM_Init();
	extern void CTRAM_Init();
	extern void Multiplay_Init();
	extern void KCCFloppy_Init();
	extern void LowerROM_Init();
	extern void hackit_Init();
	extern void WinapePokeDatabase_Free();
	extern void DkTronics256KBRam_Init();
	extern void DkTronics256KBSiliconDisk_Init();
	extern void XMass_Init();
	extern void XMem_Init();


}

#if defined(__WXGTK__)
int main(int argc, char** argv)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        return 1;
    }

    return wxEntry(argc, argv);
}
#endif

#if defined(__WXMAC__)
void arnguiApp::MacOpenFile(const wxString &sFilename)
{
	InsertOrAutoStartMedia(sFilename,false);
}

void arnguiApp::MacNewFile()
{
}

void arnguiApp::MacOpenFiles(const wxArrayString &fileNames)
{
	for (int i = 0; i < fileNames.GetCount(); i++)
	{
		InsertOrAutoStartMedia(fileNames[i],false);
	}
}

void arnguiApp::MacOpenURL(const wxString &url)
{
	InsertOrAutoStartMedia(url, false);
}

void arnguiApp::MacReopenApp()
{
	wxApp::MacReopenApp();
}
#endif

#ifdef AELISS
//for Visual Leak Detector
#include <vld.h>
#endif

//#include <wx/arrimpl.cpp>
//WX_DEFINE_OBJARRAY(ArrayOfarnguiConfig)

// for older wx2.8 releases including first OS release on raspberry pi
#ifndef wxT_2
#define wxT_2(x) wxT(x)
#endif

BEGIN_EVENT_TABLE(arnguiApp, wxApp)
EVT_QUERY_END_SESSION(arnguiApp::OnQueryEndSession)
END_EVENT_TABLE()

void arnguiApp::Log(const char *message, ...)
{
}


void arnguiApp::OnQueryEndSession(wxCloseEvent & event)
{
	/* do not veto, just do it */
	event.Skip();
}

int arnguiApp::ExpressionEvaluate(const wxString &sValue)
{
#if (wxVERSION_NUMBER >= 2900)
	// 2.9
	const wxCharBuffer tmp_buf = wxConvCurrent->cWX2MB(sValue.wc_str());
#else
	// 2.8
	const wxWX2MBbuf tmp_buf = wxConvCurrent->cWX2MB(sValue);
#endif
	const char *pString = (const char*)tmp_buf;

	int nResult;
	EvaluateExpression(pString, &nResult);
	return nResult;
}


void TraceMessage(const wxString &sMessage)
{
	//wxMessageOutputDebug().Printf(wxT("%s"),sMessage.GetData());
	//wxMessageOutput::Get()->Log(wxT("%s"), sMessage.GetData());
}
enum
{
	// keep first
	ROM_ID_NULL = 0,

	// cpc464
	ROM_ID_CPC464E_OS,      // english
	ROM_ID_CPC464E_BASIC,

	ROM_ID_CPC464D_OS,    // danish
	ROM_ID_CPC464D_BASIC,

	ROM_ID_CPC464F_OS,    // french
	ROM_ID_CPC464F_BASIC,

	ROM_ID_CPC464S_OS,    // spanish

	// cpc472

	// cpc664
	ROM_ID_CPC664E_OS,        // english
	ROM_ID_CPC664E_BASIC,

	// cpc6128

	ROM_ID_CPC6128E_OS,     // english
	ROM_ID_CPC6128E_BASIC,

	ROM_ID_CPC6128F_OS,     // french
	ROM_ID_CPC6128F_BASIC,

	ROM_ID_CPC6128S_OS,     // spanish
	ROM_ID_CPC6128S_BASIC,

	ROM_ID_CPC6128W_OS,   // swedish/finnish

	ROM_ID_CPC6128D_OS,     // danish
	ROM_ID_CPC6128D_BASIC,  // danish

	// amsdos
	ROM_ID_AMSDOSE,

	// plus
	ROM_ID_SYSTEM_CART_EN,
	ROM_ID_SYSTEM_CART_FR,
	ROM_ID_SYSTEM_CART_FR2,
	ROM_ID_SYSTEM_CART_ES,

	// csd
	ROM_ID_SYSTEM_CART_CSD,

	// kcc
	ROM_ID_KCC_BASIC,
	ROM_ID_KCC_COLOUR,
	ROM_ID_KCC_OS,

	// aleste 520ex
	ROM_ID_ALESTE_AF,
	ROM_ID_ALESTE_AL512,
	ROM_ID_ALESTE_MAPPER,
	ROM_ID_ALESTE_RFCOLDAT,
	ROM_ID_ALESTE_RFVDKEY,
	ROM_ID_ALESTE_ROMRAM,

	/* parados dos replacement */
	ROM_ID_PARADOS,

	/* qcmd */
	ROM_ID_QCMD,

	/* firmware 3.12 */
	ROM_ID_FW31_EN,
	ROM_ID_FW31_ES,
	ROM_ID_FW31_FR,

	/* firmware 3.16 */
	ROM_ID_FW316_EN,
	ROM_ID_FW316_EXP_EN,


	// keep last
	ROM_ID_LAST
};

struct BuiltinMediaDetails
{
	BUILTIN_MEDIA_TYPE Type;

	/* ID */
	int nID;

	/* filename within roms.zip */
	const wxChar *sFilename;
	/* display name to show in GUI */
	const wxChar *sDisplayName;
	/* address to hold data */
	unsigned char *pData;
	/* length of data */
	unsigned long nLength;
};

BuiltinMediaDetails BuiltinMedia[] =
{
	// cpc464
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_CPC464E_OS,
		wxT("roms/cpc464e/os.rom"),
		wxT("CPC464 English OS ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_CPC464E_BASIC,
		wxT("roms/cpc464e/basic.rom"),
		wxT("CPC464 English BASIC ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_CPC464D_OS,
		wxT("roms/cpc464d/os.rom"),
		wxT("CPC464 Danish OS ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_CPC464D_BASIC,
		wxT("roms/cpc464d/basic.rom"),
		wxT("CPC464 Danish BASIC ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_CPC464F_OS,
		wxT("roms/cpc464f/os.bin"),
		wxT("CPC464 French OS ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_CPC464F_BASIC,
		wxT("roms/cpc464f/basic.bin"),
		wxT("CPC464 French BASIC ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_CPC464S_OS,
		wxT("roms/cpc464s/os.rom"),
		wxT("CPC464 Spanish OS ROM"),
		NULL,
		0
	},
	// cpc664
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_CPC664E_OS,
		wxT("roms/cpc664e/os.rom"),
		wxT("CPC664 English OS ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_CPC664E_BASIC,
		wxT("roms/cpc664e/basic.rom"),
		wxT("CPC664 English BASIC ROM"),
		NULL,
		0
	},
	// cpc6128
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_CPC6128E_OS,
		wxT("roms/cpc6128e/os.rom"),
		wxT("CPC6128 English OS ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_CPC6128E_BASIC,
		wxT("roms/cpc6128e/basic.rom"),
		wxT("CPC6128 English BASIC ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_CPC6128F_OS,
		wxT("roms/cpc6128f/os.rom"),
		wxT("CPC6128 French OS ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_CPC6128F_BASIC,
		wxT("roms/cpc6128f/basic.rom"),
		wxT("CPC6128 French BASIC ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_CPC6128S_OS,
		wxT("roms/cpc6128s/os.rom"),
		wxT("CPC6128 Spanish OS ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_CPC6128S_BASIC,
		wxT("roms/cpc6128s/basic.rom"),
		wxT("CPC6128 Spanish BASIC ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_CPC6128W_OS,
		wxT("roms/cpc6128w/os.rom"),
		wxT("CPC6128 Swedish/Finish OS ROM")
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_CPC6128D_OS,
		wxT("roms/cpc6128d/os.rom"),
		wxT("CPC6128 Danish OS ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_CPC6128D_BASIC,
		wxT("roms/cpc6128d/basic.rom"),
		wxT("CPC6128 Danish BASIC ROM"),
		NULL,
		0
	},
	// amsdos
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_AMSDOSE,
		wxT("roms/amsdose/amsdos.rom"),
		wxT("AMSDOS v0.7 ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_CART,
		ROM_ID_SYSTEM_CART_EN,
		wxT("roms/cpcplus/system_en.cpr"),
		wxT("Plus/GX4000 System Cartridge English"),
		NULL,
		0,
	},
	{
		BUILTIN_MEDIA_CART,
		ROM_ID_SYSTEM_CART_FR,
		wxT("roms/cpcplus/system_fr.cpr"),
		wxT("Plus/GX4000 System Cartridge French"),
		NULL,
		0,
	},
	{
		BUILTIN_MEDIA_CART,
		ROM_ID_SYSTEM_CART_FR2,
		wxT("roms/cpcplus/system_fr2.cpr"),
		wxT("Plus/GX4000 System Cartridge French (CPC6128)"),
		NULL,
		0,
	},
	{
		BUILTIN_MEDIA_CART,
		ROM_ID_SYSTEM_CART_ES,
		wxT("roms/cpcplus/system_es.cpr"),
		wxT("Plus/GX4000 System Cartridge Spanish"),
		NULL,
		0,
	},
	// csd
	{
		BUILTIN_MEDIA_CART,
		ROM_ID_SYSTEM_CART_CSD,
		wxT("roms/cpcplus/system_csd.cpr"),
		wxT("CSD System Cartridge"),
		NULL,
		0,
	},
	// kcc
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_KCC_BASIC,
		wxT("roms/kcc/kccbas.rom"),
		wxT("KCC System BASIC ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_KCC_COLOUR,
		wxT("roms/kcc/FARBEN.ROM"),
		wxT("KCC Colour ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_KCC_OS,
		wxT("roms/kcc/kccos.rom"),
		wxT("KCC System OS ROM"),
		NULL,
		0
	},
	// aleste 520ex
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_ALESTE_AF,
		wxT("roms/aleste/AF.BIN"),
		wxT("Aleste AF System ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_ALESTE_AL512,
		wxT("roms/aleste/AL512.BIN"),
		wxT("Aleste AL512 System ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_ALESTE_MAPPER,
		wxT("roms/aleste/MAPPER.BIN"),
		wxT("Aleste MAPPER System ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_ALESTE_RFCOLDAT,
		wxT("roms/aleste/RFCOLDAT.BIN"),
		wxT("Aleste RFCOLDAT System ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_ALESTE_RFVDKEY,
		wxT("roms/aleste/RFVDKEY.BIN"),
		wxT("Aleste RFVDKEY System ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_ALESTE_ROMRAM,
		wxT("roms/aleste/ROMRAM.BIN"),
		wxT("Aleste ROMRAM System ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_PARADOS,
		wxT("roms/parados.rom"),
		wxT("Parados v1.1 ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_QCMD,
		wxT("roms/qcmd.rom"),
		wxT("QCMD"),
		NULL,
		0

	},
	// fw3.1
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_FW316_EN,
		wxT("roms/fw3.1/FW316.ROM"),
		wxT("Firmware 3.16 English ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_FW316_EXP_EN,
		wxT("roms/fw3.1/FW316EXP.ROM"),
		wxT("Firmware 3.16 English Expansion ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_FW31_EN,
		wxT("roms/fw3.1/fw312en.rom"),
		wxT("Firmware 3.12 English ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_FW31_ES,
		wxT("roms/fw3.1/fw312es.rom"),
		wxT("Firmware 3.12 Spanish ROM"),
		NULL,
		0
	},
	{
		BUILTIN_MEDIA_ROM,
		ROM_ID_FW31_FR,
		wxT("roms/fw3.1/fw312fr.rom"),
		wxT("Firmware 3.12 French ROM"),
		NULL,
		0
	},


};

int arnguiApp::GetNumBuiltinMedia()
{
	return sizeof(BuiltinMedia) / sizeof(BuiltinMedia[0]);
}

const wxChar *arnguiApp::GetBuiltinMediaDisplayName(int Index)
{
	if (Index >= GetNumBuiltinMedia())
		return NULL;

	return BuiltinMedia[Index].sDisplayName;
}

BUILTIN_MEDIA_TYPE arnguiApp::GetBuiltinMediaType(int Index)
{
	if (Index >= GetNumBuiltinMedia())
		return BUILTIN_MEDIA_UNKNOWN;

	return BuiltinMedia[Index].Type;
}


wxString arnguiApp::GetBuiltinMediaPath(int Index)
{
	if (Index >= GetNumBuiltinMedia())
	{
		return wxEmptyString;
	}

	wxString sPath(wxT("builtin:"));
	sPath += BuiltinMedia[Index].sFilename;
	return sPath;
}

bool arnguiApp::IsBuiltinMediaPath(const wxString &sPath)
{
	return sPath.StartsWith(wxT("builtin:"));
}

int arnguiApp::FindBuiltinMediaByPath(const wxString &sPath)
{
	wxString sComparePath = sPath.SubString(8, sPath.Length());

	for (int i = 0; i < GetNumBuiltinMedia(); i++)
	{
		wxString sFilename(BuiltinMedia[i].sFilename);

		if (sFilename.CmpNoCase(sComparePath) == 0)
		{
			return i;
		}
	}

	return 0;
}

void arnguiApp::GetBuiltinMediaDataByIndex(int Index, unsigned char **ppData, unsigned long *ppLength)
{
	if (Index >= GetNumBuiltinMedia())
	{
		*ppData = NULL;
		*ppLength = 0;
	}
	else
	{
		*ppData = BuiltinMedia[Index].pData;
		*ppLength = BuiltinMedia[Index].nLength;
	}
}


void arnguiApp::GetBuiltinMediaDataByID(int nID, unsigned char **ppPtr, unsigned long *pLength)
{
	*ppPtr = NULL;
	*pLength = 0;
	for (int i = 0; i < GetNumBuiltinMedia(); i++)
	{
		if (BuiltinMedia[i].nID == nID)
		{
			*ppPtr = BuiltinMedia[i].pData;
			*pLength = BuiltinMedia[i].nLength;
			break;
		}
	}
}

int arnguiApp::DefaultCartridge()
{

	return m_nDefaultCartridge;
}

void arnguiApp::InsertDefaultCartridge(int nID)
{
	unsigned char *pCart = NULL;
	unsigned long nCartLength = 0;

	m_nDefaultCartridge = nID;
	switch (nID)
	{
	case 0:
	{
		GetBuiltinMediaDataByID(ROM_ID_SYSTEM_CART_EN, &pCart, &nCartLength);
	}
	break;

	case 1:
	{
		GetBuiltinMediaDataByID(ROM_ID_SYSTEM_CART_FR, &pCart, &nCartLength);
	}
	break;

	case 2:
	{
		GetBuiltinMediaDataByID(ROM_ID_SYSTEM_CART_FR2, &pCart, &nCartLength);
	}
	break;

	case 3:
	{
		GetBuiltinMediaDataByID(ROM_ID_SYSTEM_CART_ES, &pCart, &nCartLength);
	}
	break;
	}
	if (pCart != NULL)
	{
		Cartridge_SetDefault(pCart, nCartLength);
	}
	else
	{
		Log("unable to set default cartridge\n");
	}
}


void arnguiApp::Config_CPC464EN()
{
	unsigned char *pOS;
	unsigned char *pBasic;
	unsigned long RomLength;

	GetBuiltinMediaDataByID(ROM_ID_CPC464E_OS, &pOS, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_CPC464E_BASIC, &pBasic, &RomLength);

	CPC_SetExpLow(FALSE);
	CPC_SetOSRom(pOS);
	CPC_SetBASICRom(pBasic);
	CPC_SetHardware(CPC_HW_CPC);
}


void arnguiApp::Config_CPC464EN_DDI1()
{
	unsigned char *pOS;
	unsigned char *pBasic;
	unsigned char *pAmsdos;
	unsigned long RomLength;


	GetBuiltinMediaDataByID(ROM_ID_CPC464E_OS, &pOS, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_CPC464E_BASIC, &pBasic, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_AMSDOSE, &pAmsdos, &RomLength);

	CPC_SetExpLow(TRUE);
	CPC_SetOSRom(pOS);
	CPC_SetBASICRom(pBasic);


	Amstrad_DiscInterface_SetRom(pAmsdos);
	Amstrad_DiscInterface_Install();

	CPC_SetHardware(CPC_HW_CPC);
}



void arnguiApp::Config_CPC464ES()
{
	unsigned char *pOS;
	unsigned char *pBasic;
	unsigned long RomLength;

	GetBuiltinMediaDataByID(ROM_ID_CPC464S_OS, &pOS, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_CPC464E_BASIC, &pBasic, &RomLength);

	CPC_SetExpLow(FALSE);
	CPC_SetOSRom(pOS);
	CPC_SetBASICRom(pBasic);
	CPC_SetHardware(CPC_HW_CPC);
}

void arnguiApp::Config_CPC464FR()
{
	unsigned char *pOS;
	unsigned char *pBasic;
	unsigned long RomLength;

	GetBuiltinMediaDataByID(ROM_ID_CPC464F_OS, &pOS, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_CPC464F_BASIC, &pBasic, &RomLength);

	CPC_SetExpLow(FALSE);
	CPC_SetOSRom(pOS);
	CPC_SetBASICRom(pBasic);
	CPC_SetHardware(CPC_HW_CPC);
}


void arnguiApp::Config_CPC464DK()
{
	unsigned char *pOS;
	unsigned char *pBasic;
	unsigned long RomLength;

	GetBuiltinMediaDataByID(ROM_ID_CPC464D_OS, &pOS, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_CPC464D_BASIC, &pBasic, &RomLength);

	CPC_SetExpLow(FALSE);
	CPC_SetOSRom(pOS);
	CPC_SetBASICRom(pBasic);
	CPC_SetHardware(CPC_HW_CPC);
}


void arnguiApp::Config_CPC6128DK()
{
	unsigned char *pOS;
	unsigned char *pBasic;
	unsigned long RomLength;
	unsigned char *pAmsdos;

	GetBuiltinMediaDataByID(ROM_ID_CPC6128D_OS, &pOS, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_CPC6128D_BASIC, &pBasic, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_AMSDOSE, &pAmsdos, &RomLength);

	CPC_SetExpLow(TRUE);
	CPC_SetOSRom(pOS);
	CPC_SetBASICRom(pBasic);


	Amstrad_DiscInterface_SetRom(pAmsdos);
	Amstrad_DiscInterface_Install();
	EmuDevice_Enable(PALDeviceIndex, TRUE);
	CPC_SetHardware(CPC_HW_CPC);
}

void arnguiApp::Config_CPC664EN()
{
	unsigned char *pOS;
	unsigned char *pBasic;
	unsigned char *pAmsdos;
	unsigned long RomLength;

	GetBuiltinMediaDataByID(ROM_ID_CPC664E_OS, &pOS, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_CPC664E_BASIC, &pBasic, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_AMSDOSE, &pAmsdos, &RomLength);

	CPC_SetExpLow(TRUE);
	CPC_SetOSRom(pOS);
	CPC_SetBASICRom(pBasic);
	Amstrad_DiscInterface_SetRom(pAmsdos);
	Amstrad_DiscInterface_Install();

	CPC_SetHardware(CPC_HW_CPC);
}

void arnguiApp::Config_464Plus()
{
	ASIC_SetGX4000(FALSE);
	CPC_SetExpLow(TRUE);
	CPC_SetHardware(CPC_HW_CPCPLUS);

	/* 128k ram */
	ASIC_SetR128(FALSE);
	/* disc interface */
	ASIC_SetR129(FALSE);

	Cartridge_InsertDefault();

	ASIC_SetHasCassetteInterface(TRUE);

}

// these can be used when the switches are turned as well as when a disk is inserted.
void arnguiApp::WarnDrive(int nDrive)
{
	char chDrive = (char)(nDrive + 'A');
	wxString sMessage;

	if (!DiskImage_HasSide(nDrive, 1))
	{
		// these can also be warned when we do this for each drive.
		if (FDI_GetForceSide1())
		{
			wxString sMessagePart;
			sMessagePart.Printf(wxT("Side 1 is forced, but the disk inserted into drive %c is single sided.\n"), chDrive);
			sMessage += sMessagePart;
		}

		if (FDD_IsTurnDiskToSideB(nDrive))
		{
			wxString sMessagePart;
			sMessagePart.Printf(wxT("Disk is turned to side 1 on drive %c but the disk is single sided.\n"), chDrive);
			sMessage += sMessagePart;
		}
	}

	// is this drive enabled? if not warn
	if (!FDD_IsEnabled(nDrive))
	{
		wxString sMessagePart;
		sMessagePart.Printf(wxT("Drive %c is not enabled.\n"), chDrive);
		sMessage += sMessagePart;
	}
	// display if there are warnings
	if (!sMessage.IsEmpty())
	{
		wxMessageBox(sMessage, GetAppName(), wxICON_WARNING | wxOK);
	}
}



void arnguiApp::InsertOrAutoStartMedia(const wxString &sFilename, bool bAutoStart)
{

	/* turn off autotype */
	AutoType_Finish();

	/* clean up previous auto run */
	AutoRunFile_Finish();

	// try cartridge...
	if (m_Cartridge->LoadWithSaveModified(sFilename, false))
	{
		return;
	}

	if (bAutoStart)
	{
		/* if plus, insert default in the case we have a cartridge already inserted which is not default */
		if (CPC_GetHardware() == CPC_HW_CPCPLUS)
		{
			/* if not cartridge, insert default */
			Cartridge_InsertDefault();
		}
	}

	// try snapshot
	if (m_Snapshot->LoadWithSaveModified(sFilename, false))
	{
		return;
	}


	// try tape image
	if (m_Cassette->LoadWithSaveModified(sFilename, false))
	{
		if (bAutoStart)
		{
			AutoType_SetString(sAutoStringTape, FALSE, TRUE, TRUE);

			/* if play/pause are acknowledged, then set them accordingly so tape auto starts */
			Cassette_PressPlay(TRUE);
			Cassette_PressPause(FALSE);
		}
		return;
	}

	// try sample
	/*if (Sample_Load(pOpenFilename))
	{
	AutoType_SetString(sAutoStringTape, FALSE,TRUE, TRUE);
	return;
	}*/

	if (!bAutoStart)
	{
		unsigned char *pLocation;
		unsigned long Length;

		LoadLocalFile(sFilename, &pLocation, &Length);

		if (pLocation == NULL)
			return;

		if (DiskImage_Recognised(pLocation, Length) == ARNOLD_STATUS_OK)
		{
			// choose which disk!
			wxArrayString sDrives;
			for (int i = 0; i < CPC_MAX_DRIVES; i++)
			{
				wxString sDrive;
				sDrive.Printf(wxT("Drive %c"), (char)('A' + i));
				sDrives.Add(sDrive);
			}

			wxString sMessage;
			sMessage = wxT("Insert disk image file:\n\"") + sFilename + wxT("\"\ninto which drive?\n");
			wxSingleChoiceDialog dialog(GetTopWindow(), sMessage, wxT("Choose drive"), sDrives);
			dialog.SetSelection(0);
			if (dialog.ShowModal() == wxID_OK)
			{
				int nDrive = dialog.GetSelection();
				WarnDrive(nDrive);
				m_Drives[nDrive]->LoadWithSaveModified(sFilename, false);
			}
		}
		free(pLocation);
	}
	else
	{
		/* boot from zero, unless drive switch is in action */
		int nAutoStartDrive = FDI_GetDriveToAccess(0);
		int nAutoStartSide = FDI_GetSideToAccess(nAutoStartDrive, 0);

		/* try disk */
		if (m_Drives[nAutoStartDrive]->LoadWithSaveModified(sFilename, false))
		{
			unsigned char *pBuffer;

			/* enable drive */
			if (!FDD_IsEnabled(nAutoStartDrive))
			{
				FDD_Enable(nAutoStartDrive, TRUE);
			}

			/* if inserted disk doesn't have two sides and we want to auto start on side 2 */
			if (!DiskImage_HasSide(nAutoStartDrive, 1) && (nAutoStartSide == 1))
			{

				/* forcing side 1? */
				if (FDI_GetForceSide1())
				{
					/* turn back to side 0 */
					FDI_SetForceSide1(FALSE);
				}

				/* disk turned to side B? */
				if (FDD_IsTurnDiskToSideB(nAutoStartDrive))
				{
					/* turn back to side A */
					FDD_TurnDiskToSideB(nAutoStartDrive, FALSE);
				}

				/* and auto start side 0 */
				nAutoStartSide = 0;
			}

			// autostart ?
			// this may not work on large directories like romdos etc
			pBuffer = (unsigned char *)malloc(512 * 5);
			if (pBuffer)
			{
				const char *pAutoTypeString;

				/* try auto-start */
				int nAutoRunResult;

				memset(pBuffer, 0x0, 512 * 5);

				nAutoRunResult = AMSDOS_GenerateAutorunCommand(nAutoStartDrive, nAutoStartSide, pBuffer, &pAutoTypeString);

				if (nAutoRunResult == AUTORUN_OK)
				{
					AutoType_SetString(pAutoTypeString, FALSE, TRUE, TRUE);
				}
				else
				{
					if (nAutoRunResult == AUTORUN_TOO_MANY_POSSIBILITIES)
					{
						wxMessageBox(wxT("Too many files qualify for auto-run. Unable to auto-start this disc"), GetAppName(), wxICON_INFORMATION | wxOK);
					}
					else if (nAutoRunResult == AUTORUN_NOT_POSSIBLE)
					{
						wxMessageBox(wxT("Unable to auto-start this disc."), GetAppName(), wxICON_INFORMATION | wxOK);
					}
					else if (nAutoRunResult == AUTORUN_NO_FILES_QUALIFY)
					{
						wxMessageBox(wxT("Can't find any files to auto-run. Unable to auto-start this disc"), GetAppName(), wxICON_INFORMATION | wxOK);
					}
					else
					{
						wxMessageBox(wxT("Unable to auto-start this disc."), GetAppName(), wxICON_INFORMATION | wxOK);
					}

					/* do CAT */
					AutoType_SetString(sAutoStringCat, FALSE, TRUE, TRUE);
				}

				free(pBuffer);
			}
			return;
		}
	}

	if (bAutoStart)
	{
		FILE_HEADER FileHeader;
		unsigned char *pFileData;
		unsigned long FileDataSize;
		LoadLocalFile(sFilename, &pFileData, &FileDataSize);

		if (pFileData != NULL)
		{
			/* Insert file into a disk and run it from there */

			GetHeaderDataFromBuffer((const unsigned char *)pFileData, FileDataSize, &FileHeader);

			if (!FileHeader.bHasHeader)
			{
				wxMessageBox(wxT("Unable to auto-start file because it doesn't have a header"), GetAppName(), wxICON_INFORMATION | wxOK);

				FileHeader.HeaderStartAddress = 0x0000;
				FileHeader.HeaderLength = FileDataSize;
				FileHeader.HeaderExecutionAddress = FileHeader.HeaderStartAddress;
				// suggest binary
				FileHeader.HeaderFileType = 2;
			}

			// show this if SHIFT is pressed?
			LoadBinaryDialog dialog(GetTopWindow(), &FileHeader);
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
				Length = min(FileHeader.HeaderLength, FileDataSize);
				FileHeader.MemoryEnd = FileHeader.MemoryStart + Length;

				AutoRunFile_SetData((const char *)pFileData, FileDataSize, TRUE, TRUE, &FileHeader);
			}

			free(pFileData);
		}
	}
}


void arnguiApp::Config_GX4000()
{
	ASIC_SetGX4000(TRUE);
	CPC_SetExpLow(FALSE);
	CPC_SetHardware(CPC_HW_CPCPLUS);
	Cartridge_InsertDefault();
	//    CPC_InsertSystemCartridge();
	ASIC_SetHasCassetteInterface(FALSE);
	/* 128k ram */
	ASIC_SetR128(FALSE);
	/* disc interface */
	ASIC_SetR129(FALSE);
}


void arnguiApp::Config_CSD()
{
	CPC_SetHardware(CPC_HW_CPCPLUS);
	Jukebox_Enable(TRUE);

	/* it's close to a 464 or 6128 */
}

void arnguiApp::Config_6128Plus()
{
	//Log("setting up 6128 plus\n");
	ASIC_SetGX4000(FALSE);
	CPC_SetExpLow(TRUE);
	CPC_SetHardware(CPC_HW_CPCPLUS);
	Cartridge_InsertDefault();
	/* 128k ram */
	ASIC_SetR128(TRUE);
	/* disc interface */
	ASIC_SetR129(TRUE);
	/* no cassette interface */
	ASIC_SetHasCassetteInterface(FALSE);
}


void arnguiApp::Config_6128Plus_Cassette()
{
	//Log("setting up 6128 plus with cassette\n");
	ASIC_SetGX4000(FALSE);
	CPC_SetExpLow(TRUE);
	CPC_SetHardware(CPC_HW_CPCPLUS);
	Cartridge_InsertDefault();
	/* 128k ram */
	ASIC_SetR128(TRUE);
	/* disc interface */
	ASIC_SetR129(TRUE);
	/* no cassette interface */
	ASIC_SetHasCassetteInterface(TRUE);
}

void arnguiApp::Config_CPC6128EN()
{
	unsigned char *pOS;
	unsigned char *pBasic;
	unsigned char *pAmsdos;
	unsigned long RomLength;

	GetBuiltinMediaDataByID(ROM_ID_CPC6128E_OS, &pOS, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_CPC6128E_BASIC, &pBasic, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_AMSDOSE, &pAmsdos, &RomLength);

	CPC_SetExpLow(TRUE);
	CPC_SetOSRom(pOS);
	CPC_SetBASICRom(pBasic);
	Amstrad_DiscInterface_SetRom(pAmsdos);
	Amstrad_DiscInterface_Install();
	//Amstrad_RAM_Install();
	EmuDevice_Enable(PALDeviceIndex, TRUE);
	CPC_SetHardware(CPC_HW_CPC);
}

void arnguiApp::Config_CPC6128ENParados()
{
	unsigned char *pOS;
	unsigned char *pBasic;
	unsigned char *pParados;
	unsigned long RomLength;

	GetBuiltinMediaDataByID(ROM_ID_CPC6128E_OS, &pOS, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_CPC6128E_BASIC, &pBasic, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_PARADOS, &pParados, &RomLength);

	CPC_SetExpLow(TRUE);
	CPC_SetOSRom(pOS);
	CPC_SetBASICRom(pBasic);

	Amstrad_DiscInterface_SetRom(pParados);
	Amstrad_DiscInterface_Install();
	EmuDevice_Enable(PALDeviceIndex, TRUE);
	CPC_SetHardware(CPC_HW_CPC);
}


void arnguiApp::Config_CPC6128ES()
{
	unsigned char *pOS;
	unsigned char *pBasic;
	unsigned char *pAmsdos;
	unsigned long RomLength;

	GetBuiltinMediaDataByID(ROM_ID_CPC6128S_OS, &pOS, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_CPC6128S_BASIC, &pBasic, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_AMSDOSE, &pAmsdos, &RomLength);

	CPC_SetExpLow(TRUE);
	CPC_SetOSRom(pOS);
	CPC_SetBASICRom(pBasic);


	Amstrad_DiscInterface_SetRom(pAmsdos);
	Amstrad_DiscInterface_Install();
	EmuDevice_Enable(PALDeviceIndex, TRUE);
	CPC_SetHardware(CPC_HW_CPC);
}


void arnguiApp::Config_CPC6128W()
{
	unsigned char *pOS;
	unsigned char *pBasic;
	unsigned char *pAmsdos;
	unsigned long RomLength;

	GetBuiltinMediaDataByID(ROM_ID_CPC6128W_OS, &pOS, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_CPC6128E_BASIC, &pBasic, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_AMSDOSE, &pAmsdos, &RomLength);

	CPC_SetExpLow(TRUE);
	CPC_SetOSRom(pOS);
	CPC_SetBASICRom(pBasic);


	Amstrad_DiscInterface_SetRom(pAmsdos);
	Amstrad_DiscInterface_Install();
	EmuDevice_Enable(PALDeviceIndex, TRUE);
	CPC_SetHardware(CPC_HW_CPC);
}

void arnguiApp::Config_CPC6128FR()
{
	unsigned char *pOS;
	unsigned char *pBasic;
	unsigned char *pAmsdos;
	unsigned long RomLength;

	GetBuiltinMediaDataByID(ROM_ID_CPC6128F_OS, &pOS, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_CPC6128F_BASIC, &pBasic, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_AMSDOSE, &pAmsdos, &RomLength);

	CPC_SetExpLow(TRUE);
	CPC_SetOSRom(pOS);
	CPC_SetBASICRom(pBasic);


	Amstrad_DiscInterface_SetRom(pAmsdos);
	Amstrad_DiscInterface_Install();
	EmuDevice_Enable(PALDeviceIndex, TRUE);
	CPC_SetHardware(CPC_HW_CPC);

}

void arnguiApp::Config_KCC()
{
	unsigned char *pOS;
	unsigned char *pBasic;
	unsigned char *pColour;
	unsigned long RomLength;

	GetBuiltinMediaDataByID(ROM_ID_KCC_OS, &pOS, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_KCC_BASIC, &pBasic, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_KCC_COLOUR, &pColour, &RomLength);

	KCC_SetOSRom(pOS);
	KCC_SetBASICRom(pBasic);
	KCC_SetColorROM(pColour);

	CPC_SetHardware(CPC_HW_KCCOMPACT);
}


void arnguiApp::Config_KCCWithDDI1()
{
	unsigned char *pOS;
	unsigned char *pBasic;
	unsigned char *pColour;
	unsigned char *pAmsdos;
	unsigned long RomLength;

	GetBuiltinMediaDataByID(ROM_ID_KCC_OS, &pOS, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_KCC_BASIC, &pBasic, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_KCC_COLOUR, &pColour, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_AMSDOSE, &pAmsdos, &RomLength);



	KCC_SetOSRom(pOS);
	KCC_SetBASICRom(pBasic);
	KCC_SetColorROM(pColour);

	Amstrad_DiscInterface_SetRom(pAmsdos);
	Amstrad_DiscInterface_Install();


	CPC_SetHardware(CPC_HW_KCCOMPACT);
}

void arnguiApp::Config_Aleste()
{
	unsigned char *pMapper;
	unsigned char *pRFColCat;
	unsigned char *pROMRam;
	unsigned char *pAL512;
	unsigned long RomLength;

	GetBuiltinMediaDataByID(ROM_ID_ALESTE_MAPPER, &pMapper, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_ALESTE_RFCOLDAT, &pRFColCat, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_ALESTE_ROMRAM, &pROMRam, &RomLength);
	GetBuiltinMediaDataByID(ROM_ID_ALESTE_AL512, &pAL512, &RomLength);

	CPC_SetHardware(CPC_HW_ALESTE);

	Aleste_SetMapperROM(pMapper);
	Aleste_SetRfColDatROM(pRFColCat);
	Aleste_SetRomRamROM(pROMRam);
	Aleste_SetAl512(pAL512);
}


const wxString & arnguiApp::CurrentConfig()
{
	return m_sConfig;
}

void arnguiApp::ChooseConfig(const wxString &sName)
{
	wxString sMessage;
	sMessage.Format(wxT("Chosen config: %s\r\n"), sName.GetData());
	TraceMessage(sMessage);

	wxCharBuffer buf = sName.mb_str();
	//Log("Chosen config: %s\r\n", buf.data());

	EmuDevice_Enable(PALDeviceIndex, FALSE);
	Amstrad_DiscInterface_Uninstall();
	//   Amstrad_RAM_Uninstall();
	Jukebox_Enable(FALSE);

	m_sConfig = sName;

	if (sName.CmpNoCase(wxT("cpc464en")) == 0)
	{
		Config_CPC464EN();
	}
	else
		if (sName.CmpNoCase(wxT("cpc464en_ddi1")) == 0)
		{
			Config_CPC464EN_DDI1();
		}
		else
			if (sName.CmpNoCase(wxT("cpc464es")) == 0)
	{
		Config_CPC464ES();
	}
	else
	if (sName.CmpNoCase(wxT("cpc464fr")) == 0)
	{
		Config_CPC464FR();
	}
	else
	if (sName.CmpNoCase(wxT("cpc464dk")) == 0)
	{
		Config_CPC464DK();
	}
	else if (sName.CmpNoCase(wxT("cpc664en")) == 0)
	{
		Config_CPC664EN();
	}
	else if (sName.CmpNoCase(wxT("cpc6128en")) == 0)
	{
		Config_CPC6128EN();
	}
	else if (sName.CmpNoCase(wxT("cpc6128fr")) == 0)
	{
		Config_CPC6128FR();
	}
	else if (sName.CmpNoCase(wxT("cpc6128es")) == 0)
	{
		Config_CPC6128ES();
	}
	else if (sName.CmpNoCase(wxT("cpc6128dk")) == 0)
	{
		Config_CPC6128DK();
	}
	else if (sName.CmpNoCase(wxT("464plus")) == 0)
	{
		Config_464Plus();
	}
	else if (sName.CmpNoCase(wxT("6128plus")) == 0)
	{
		Config_6128Plus();
	}
	else if (sName.CmpNoCase(wxT("6128pluscas")) == 0)
	{
		Config_6128Plus_Cassette();
	}
	else if (sName.CmpNoCase(wxT("gx4000")) == 0)
	{

		Config_GX4000();
	}
	else if (sName.CmpNoCase(wxT("kccompact")) == 0)
	{

		Config_KCC();
	}
	else if (sName.CmpNoCase(wxT("aleste")) == 0)
	{

		Config_Aleste();
	}
	else if (sName.CmpNoCase(wxT("csd")) == 0)
	{

		Config_CSD();
	}
	else if (sName.CmpNoCase(wxT("6128enparados")) == 0)
	{

		Config_CPC6128ENParados();
	}
	else if (sName.CmpNoCase(wxT("cpc6128w")) == 0)
	{
		Config_CPC6128W();
	}
	else
	{
		Config_CPC6128EN();
	}
	//Log("Done Choose config\n");

	CPC_SetMonitorType(CPC_GetMonitorType());
	Keyboard_DetectLanguage();
}


#if 0
void CPC_InsertSystemCartridge(void)
{
	switch (CPC_GetSysLang())
	{
	case SYS_LANG_EN:
	{
		Cartridge_Insert(binary_cpcplus_en, sizeof(binary_cpcplus_en));
	}
	break;

	case SYS_LANG_ES:
	{
		Cartridge_Insert(binary_cpcplus_es, sizeof(binary_cpcplus_es));
	}
	break;

	case SYS_LANG_FR:
	{
		Cartridge_Insert(binary_cpcplus_fr, sizeof(binary_cpcplus_fr));
	}
	break;

	case SYS_LANG_FR2:
	{
		Cartridge_Insert(binary_cpcplus_fr2, sizeof(binary_cpcplus_fr2));
	}
	break;
	}
}



void    CPC_ReloadSystemCartridge(void)
{
	CPC_InsertSystemCartridge();

	Cartridge_Autostart();
}
#endif

extern "C"
{
	extern void CRTC_LogFinish();
}

int arnguiApp::OnExit()
{
	CRTC_LogFinish();

	ShutdownServer();

	//Log("Exiting\n");

	// free all breakpoints that were added
	Breakpoints_Free();

	WX_CLEAR_ARRAY(m_ComputerConfigs);
	WX_CLEAR_ARRAY(m_KeyboardConfigs);
	WX_CLEAR_ARRAY(m_JoystickConfigs);

	// set old one and delete our one
	delete wxMessageOutput::Set(m_OldMessageOutput);

	ClosePrinterFile();
	labelsets_free();

	YMOutput_Finish();

	UnRegisterAllDevices();

	delete m_checker;

	//Log("freeing media\n");
	for (unsigned int i = 0; i != m_Media.GetCount(); i++)
	{
		// save the settings
		m_Media[i]->SaveConfig();

		m_Media[i]->Unload();
		delete m_Media[i];
		m_Media[i] = NULL;
	}
	m_Media.Clear();


	//Log("freeing built in media\n");
	for (int i = 0; i < GetNumBuiltinMedia(); i++)
	{
		BuiltinMedia[i].nLength = 0;
		if (BuiltinMedia[i].pData != NULL)
		{
			free(BuiltinMedia[i].pData);
		}
	}
	//Log("wxXmlResource shutdown\n");

	wxXmlResource::Get()->ClearHandlers();
	wxXmlResource::Get()->Unload(GetGUIFullPath());

	wxXmlResource *pResource = wxXmlResource::Get();
	// try to deallocate it.
	wxXmlResource::Set(NULL);
	delete pResource;

	//Multiface_Finish();
	/* close CPC emulation */
	Log("CPC  shutdown\n");
	CPC_Finish();

	//sdl_close_audio();
	Audio_Finish();

	AutoType_Finish();
	AutoRunFile_Finish();

	WinapePokeDatabase_Free();

	Log("wxConfig shutdown\n");
	if (m_pConfig)
	{
		wxConfigBase::Set(NULL);

		delete m_pConfig;
	}
	Log("Platform shutdown\n");

#if defined(GTK2_EMBED_WINDOW) || defined(MAC_EMBED_WINDOW) 
	// if linux embed window or mac embed window, shutdown elsewhere.
#else
	m_PlatformSpecific.Shutdown();
#endif
	Log("finished clean up exiting app\n");

#ifdef _DEBUG
#ifdef WIN32
	FreeConsole();
#endif
#endif
	return wxApp::OnExit();

}

const wxCmdLineEntryDesc arnguiApp::m_CommandLineDesc[] =
{

	{ wxCMD_LINE_OPTION, wxT_2("t"), wxT_2("tape"), wxT_2("specify tape file"), wxCMD_LINE_VAL_STRING,0 },
	{ wxCMD_LINE_OPTION, wxT_2("d"), wxT_2("disk"), wxT_2("specify disc for any drive"), wxCMD_LINE_VAL_STRING,0 },
	{ wxCMD_LINE_OPTION, wxT_2("a"), wxT_2("drivea"), wxT_2("specify disc for drive A"), wxCMD_LINE_VAL_STRING,0 },
	{ wxCMD_LINE_OPTION, wxT_2("b"), wxT_2("driveb"), wxT_2("specify disc for drive B"), wxCMD_LINE_VAL_STRING,0 },
	{ wxCMD_LINE_OPTION, wxT_2("da"), wxT_2("diska"), wxT_2("specify disc for drive A"), wxCMD_LINE_VAL_STRING,0 },
	{ wxCMD_LINE_OPTION, wxT_2("db"), wxT_2("diskb"), wxT_2("specify disc for drive B"), wxCMD_LINE_VAL_STRING,0 },
	{ wxCMD_LINE_OPTION, wxT_2("dc"), wxT_2("diskc"), wxT_2("specify disc for drive C"), wxCMD_LINE_VAL_STRING,0 },
	{ wxCMD_LINE_OPTION, wxT_2("dd"), wxT_2("diskd"), wxT_2("specify disc for drive D"), wxCMD_LINE_VAL_STRING,0 },
	{ wxCMD_LINE_OPTION, wxT_2("c"), wxT_2("cart"), wxT_2("specify cartridge"), wxCMD_LINE_VAL_STRING,0 },
	{ wxCMD_LINE_OPTION, wxT_2("s"), wxT_2("snapshot"), wxT_2("specify memory snapshot"), wxCMD_LINE_VAL_STRING,0 },
	{ wxCMD_LINE_OPTION, wxT_2("at"), wxT_2("autotype"), wxT_2("specify string to auto type (use HTML characters to escape special characters)"), wxCMD_LINE_VAL_STRING,0 },
	{ wxCMD_LINE_OPTION, wxT_2("as"), wxT_2("autostart"), wxT_2("specify media to auto start"), wxCMD_LINE_VAL_STRING,0 },
	{ wxCMD_LINE_OPTION, wxT_2("ls"), wxT_2("labelset"), wxT_2("load a collection of labels for use in the debugger"), wxCMD_LINE_VAL_STRING,0},
	{ wxCMD_LINE_OPTION, wxT_2("cr"), wxT_2("crtctype"), wxT_2("crtc type"), wxCMD_LINE_VAL_NUMBER,0 },
	{ wxCMD_LINE_OPTION, wxT_2("cfg"), wxT_2("config"), wxT_2("set name of configuration to use"), wxCMD_LINE_VAL_STRING,0 },

	{ wxCMD_LINE_SWITCH, wxT_2("ns"), wxT_2("nosplash"), wxT_2("do not display splash screen"), wxCMD_LINE_VAL_NONE,0 },
	{ wxCMD_LINE_SWITCH, wxT_2("na"), wxT_2("noaudio"), wxT_2("do not initialise host audio"), wxCMD_LINE_VAL_NONE,0 },
	{ wxCMD_LINE_SWITCH, wxT_2("nj"), wxT_2("nojoystick"), wxT_2("do not initialise host joystick/joypad"), wxCMD_LINE_VAL_NONE,0 },
	//   { wxCMD_LINE_SWITCH, wxT_2("f"), wxT_2("fullscreen"),    wxT_2("use fullscreen display on start"), wxCMD_LINE_VAL_NONE},

	// to allow files of any type
	{ wxCMD_LINE_PARAM, NULL, NULL, wxT_2("file"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE },
	{ wxCMD_LINE_NONE }
};

void arnguiApp::ProcessCommandLineFromString(const wxString &str)
{
	wxCmdLineParser parser(m_CommandLineDesc,str);
	if (parser.Parse(false) == 0)
	{
		CommandLineParsed(parser);
		ExecuteCommandLine();
	}
}

void arnguiApp::ExecuteCommandLine()
{
	Log("Handling command line parameters\n");

	//    Computer_RestartPower();

	// autotype, allow C style \r\n etc

	if (!m_sConfigRequested.IsEmpty())
	{
		wxString sMessage;
		sMessage.Format(wxT("Command-line: Configuration: %s\r\n"), m_sConfigRequested.GetData());
		TraceMessage(sMessage);

		ChooseConfig(m_sConfigRequested);
	}


	RestartPower(false, true);

	if (!m_sTapeFile.IsEmpty())
	{
		wxString sMessage;
		sMessage.Format(wxT("Command-line: Tape: %s\r\n"), m_sTapeFile.GetData());
		TraceMessage(sMessage);



		if (!m_Cassette->LoadWithSaveModified(m_sTapeFile, true))
		{
			wxCharBuffer buf = m_sTapeFile.mb_str();
			Log("Failed to load Tape %s\n", buf.data());


		}

	}

	// TODO: Handle ROM

	if (!m_sCartridgeFile.IsEmpty())
	{
		wxString sMessage;
		sMessage.Format(wxT("Command-line: Cartridge: %s\r\n"), m_sCartridgeFile.GetData());
		TraceMessage(sMessage);
		if (!m_Cartridge->LoadWithSaveModified(m_sCartridgeFile, true))
		{
			wxCharBuffer buf = m_sCartridgeFile.mb_str();
			Log("Failed to load Cartridge %s\n", buf.data());
		}
	}

	if (!m_sSnapshotFile.IsEmpty())
	{
		wxString sMessage;
		sMessage.Format(wxT("Command-line: Snapshot: %s\r\n"), m_sSnapshotFile.GetData());
		TraceMessage(sMessage);
		if (!m_Snapshot->LoadWithSaveModified(m_sSnapshotFile, true))
		{
			wxCharBuffer buf = m_sSnapshotFile.mb_str();
			Log("Failed to load Snapshot %s\n", buf.data());
		}
	}

	for (size_t i = 0; i < CPC_MAX_DRIVES; i++)
	{
		if (!m_sDiskFile[i].IsEmpty())
		{
			wxString sMessage;
			sMessage.Format(wxT("Command-line: Disk insert into drive %c: %s\r\n"), (char)('A' + i), m_sDiskFile[i].GetData());
			TraceMessage(sMessage);
			WarnDrive(i);

			if (!m_Drives[i]->LoadWithSaveModified(m_sDiskFile[i], true))
			{
				wxCharBuffer buf = m_sDiskFile[i].mb_str();
				Log("Failed to load Disk  %s\n", buf.data());
			}

		}
	}

	if (!m_sDiskFileNoDrive.IsEmpty())
	{
		wxString sMessage;
		sMessage.Format(wxT("Command-line: Disk insert into drive: %s\r\n"), m_sDiskFileNoDrive.GetData());
		TraceMessage(sMessage);

		// choose which disk!
		wxArrayString sDrives;
		for (int i = 0; i < CPC_MAX_DRIVES; i++)
		{
			wxString sDrive;
			sDrive.Printf(wxT("Drive %c"), (char)('A' + i));
			sDrives.Add(sDrive);
		}

		sMessage = wxT("Insert disk image file:\n\"") + m_sDiskFileNoDrive + wxT("\"\ninto which drive?\n");
		wxSingleChoiceDialog dialog(GetTopWindow(), sMessage, wxT("Choose drive"), sDrives);
		dialog.SetSelection(0);
		if (dialog.ShowModal() == wxID_OK)
		{
			int nDrive = dialog.GetSelection();
			WarnDrive(nDrive);

			if (!m_Drives[nDrive]->LoadWithSaveModified(m_sDiskFileNoDrive, true))
			{
				wxCharBuffer buf = m_sDiskFileNoDrive.mb_str();
				Log("Failed to load Disk  %s\n", buf.data());


			}
		}
	}

	// autotype, allow C style \r\n etc
	if (!m_sAutoType.IsEmpty())
	{

		wxString sMessage;
		sMessage.Printf(wxT("Command-line: Autotype text: %s\r\n"), m_sAutoType.c_str());
		TraceMessage(sMessage);

		/*   wxString sMessage;
		sMessage.Printf(wxT("Autotype string: %s"), sAutoType.c_str());
		wxMessageDialog dialog(frame,sMessage,wxEmptyString, wxYES_NO|wxICON_EXCLAMATION);;
		if (dialog.ShowModal()==wxID_YES)
		{

		}
		*/
		// unescape eol and tab
		m_sAutoType.Replace(wxT("\\r"), wxT("\r"));
		m_sAutoType.Replace(wxT("&quot;"), wxT("\""));
		m_sAutoType.Replace(wxT("\\n"), wxT("\n"));
		m_sAutoType.Replace(wxT("\\t"), wxT("\t"));
		m_sAutoType.Replace(wxT("\\\""), wxT("\""));

		const wxCharBuffer buffer = m_sAutoType.utf8_str();

		/* wait for input, no need to reset cpc */
		AutoType_SetString(buffer.data(), TRUE, TRUE, TRUE);
		buffer.release();
	}

#if 0
	for (unsigned int i = 0; i != m_sLabelSets.GetCount(); i++)
	{
		wxString sParam = m_sLabelSets[i];

		wxString sMessage;
		sMessage.Format(wxT("Command-line: Label set: %s\r\n"), m_LabelSets.GetData());
		TraceMessage(sMessage);

		InsertMedia(sParam);

	}
#endif


	if (!m_sAutoStart.IsEmpty())
	{
		wxString sMessage;
		sMessage.Printf(wxT("Command-line: Autostart: %s\r\n"), m_sAutoStart.c_str());
		TraceMessage(sMessage);

		InsertOrAutoStartMedia(m_sAutoStart,true);
	}

	if (m_nCRTCType != -1)
	{
		wxString sMessage;
		sMessage.Format(wxT("Command-line: CRTC type: %d\r\n"), (int)m_nCRTCType);
		TraceMessage(sMessage);

		CPC_SetCRTCType(m_nCRTCType);
	}

	for (unsigned int i = 0; i != m_sMedia.GetCount(); i++)
	{
		wxString sParam = m_sMedia[i];

		wxString sMessage;
		sMessage.Format(wxT("Command-line: Insert Media: %s\r\n"), sParam.GetData());
		TraceMessage(sMessage);

		InsertOrAutoStartMedia(sParam,false);

	}
}

void arnguiApp::OnInitCmdLine(wxCmdLineParser& parser)
{
	wxApp::OnInitCmdLine(parser);
	parser.SetDesc(m_CommandLineDesc);
}

void arnguiApp::CommandLineParsed(wxCmdLineParser &parser)
{
	wxString sValue;


	if (parser.Found(wxT("nosplash")))
	{
		m_bDisplaySplash = false;
	}

	if (parser.Found(wxT("noaudio")))
	{
		m_bAudio = false;
	}

	if (parser.Found(wxT("nojoystick")))
	{
		m_bJoystick = false;
	}

	//    if (parser.Found(wxT("fullscreen")))
	//   {
	//    m_bFullScreen = true;
	// }

	if (parser.Found(wxT("config"), &sValue))
	{
		m_sConfigRequested = sValue;
	}

	if (parser.Found(wxT("tape"), &sValue))
	{
		m_sTapeFile = sValue;
	}

	if (parser.Found(wxT("cart"), &sValue))
	{
		m_sCartridgeFile = sValue;
	}

	if (parser.Found(wxT("snapshot"), &sValue))
	{
		m_sSnapshotFile = sValue;
	}

	if (parser.Found(wxT("diska"), &sValue) || parser.Found(wxT("drivea"), &sValue))
	{
		m_sDiskFile[0] = sValue;
	}

	if (parser.Found(wxT("diskb"), &sValue) || parser.Found(wxT("driveb"), &sValue))
	{
		m_sDiskFile[1] = sValue;
	}

	if (parser.Found(wxT("diskc"), &sValue))
	{
		m_sDiskFile[2] = sValue;
	}

	if (parser.Found(wxT("diskd"), &sValue))
	{
		m_sDiskFile[3] = sValue;
	}

	if (parser.Found(wxT("autotype"), &sValue))
	{
		m_sAutoType = sValue;
	}

#if 0
	if (parser.Found(wxT("labelset"), &sValue))
	{
		m_sLabelSet = sValue;
	}
#endif

	if (parser.Found(wxT("autostart"), &sValue))
	{
		m_sAutoStart = sValue;
	}

	if (parser.Found(wxT("disk"), &sValue))
	{
		m_sDiskFileNoDrive = sValue;
	}

	long nValue;
	if (parser.Found(wxT("crtctype"), &nValue))
	{
		m_nCRTCType = nValue;
	}


	for (unsigned int i = 0; i != parser.GetParamCount(); i++)
	{
		wxString sParam = parser.GetParam(i);
		wxString sFilename = sParam;
		m_sMedia.Add(sFilename);
	}
}

bool arnguiApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
	CommandLineParsed(parser);
	return true;
}

const wxChar *sAuthor = wxT("Kevin Thacker");
const wxChar *sAppName = wxT("Arnold");
const wxChar *sAppDisplayName = wxT("Arnold Emulator");


arnguiApp::arnguiApp() : wxApp()
{
	/* Enable memory leak checks */
#if defined(_DEBUG) && defined(__WXMSW__)
	//_CrtSetBreakAlloc(337888);

	int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	// Enable one of the following lines depending on the level (and performance) of checking required:
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;	// Minimal memory checking on exit of app.
	//tmpFlag |= _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_CHECK_CRT_DF | _CRTDBG_DELAY_FREE_MEM_DF; // Comprehensive (and slow!) checking
	_CrtSetDbgFlag(tmpFlag);
#endif
	m_nThumbnailWidth = 160;
	m_nThumbnailHeight = 100;

	m_nCRTCType = -1;   // no crtc type set by default
	m_bDisplaySplash = true; // enabled by default
	m_bJoystick = true;   // enabled by default
	m_bAudio = true;      // enabled by default
	m_bFullScreen = false; // windowed by default
	m_nDefaultCartridge = 0;
	m_nFullScreenMode = -1;
	frame = NULL;
	m_Cassette = NULL;
	m_Cartridge = NULL;
	m_Snapshot = NULL;
	for (int i = 0; i < CPC_MAX_DRIVES; i++)
	{
		m_Drives[i] = NULL;
	}

	// default settings for audio
	m_nAudioFrequency = 44100;
	m_nAudioBits = 8;
	m_nAudioChannels = 2;


	m_pPrinterOutputStream = NULL;
	m_checker = NULL;
	m_pConfig = NULL;
	m_OldMessageOutput = NULL;
	m_nCRTCType = 0;
}

const char *ActionCodeText[ACTION_CODE_NUM] =
{
	"None",
	"Reset",
	"Reset without question",
	"Toggle Fullscreen",
	"Decrease Warp Factor",
	"Increase Warp Factor",
	"Zoom In",
	"Zoom Out",
	"Insert Disk into drive A",
	"Insert Disk into drive B",
	"Insert Cartridge",
	"Load Snapshot",
	"Save Snapshot",
	"Insert Tape",
	"Save Screenshot",
	"Next in playlist",
	"Prev in playlist",
	"Prev save state",
	"Next save state",
	"Save save satte",
	"Quit",
	"Quit without question",
	"Show on-screen display",
	"Toggle KeyStick enable"
};


const char *arnguiApp::GetActionCodeName(ACTION_CODE nAction)
{
	if (nAction < ACTION_CODE_NUM)
	{
		return ActionCodeText[nAction];
	}

	return "";
}


void arnguiApp::ProcessAction(ACTION_CODE nAction)
{
	if (frame)
		frame->ProcessAction(nAction);
}


void arnguiApp::SetTranslatedKey(const char *sUTF8String)
{
	//const char sUTF8StringTest[5] = { 0x020,0x0,0x0,0x0,0x0 };
//const char sUTF8StringTest[5] = { 0x07f,0x0,0x0,0x0,0x0 };
//	const char sUTF8StringTest[5] = { 0x0c0+0x010,0x080+0x020,0x0,0x0,0x0 };
///	const char sUTF8String[5] = { 0x0e0+0x0f,0x080+0x020,0x080+0x020,0x0,0x0,0x0 };
//	const char sUTF8String[5] = { 0x0f0+0x07,0x080+0x020,0x080+0x020,0x080+0x020,0x0};

	/* convert utf-8 */
	AutoType_AppendTypedString(sUTF8String);
}

extern "C"
{
	extern void CRTC_LogInit();
	extern void CRTC_LogFinish();
}

bool arnguiApp::OnInit()
{
#ifdef _DEBUG
#ifdef WIN32
	AllocConsole();
	freopen("conin$", "r", stdin);
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);
	Log("Debugging Window:\n");
#endif
#endif
	if (!wxApp::OnInit())
		return false;

	CRTC_LogInit();


	m_OldMessageOutput = wxMessageOutput::Set(new wxMessageOutputStderr);


	if (::wxDisplayDepth() <= 8)
	{
		wxMessageDialog dialog(NULL, wxT("Please use 16-bit, 24-bit or 32-bit display mode."), wxT(""), wxOK);
		dialog.ShowModal();

		return false;
	}

	//wxSystemOptions is slow, if a value is not set, it keeps querying the environment
	//each and every time...
	wxSystemOptions::SetOption(_T("filesys.no-mimetypesmanager"), 0);
	wxSystemOptions::SetOption(_T("window-default-variant"), _T(""));
#if defined(__WXMSW__)
	wxSystemOptions::SetOption(_T("no-maskblt"), 0);
	wxSystemOptions::SetOption(_T("msw.window.no-clip-children"), 0);
	wxSystemOptions::SetOption(_T("msw.font.no-proof-quality"), 0);
#endif
#if defined(__WXMAC__)
	// Disable window animation
	wxSystemOptions::SetOption(wxMAC_WINDOW_PLAIN_TRANSITION, 1);
#endif

	// stop all windows getting idle events
	wxIdleEvent::SetMode(wxIDLE_PROCESS_SPECIFIED);
	
	// stop all windows getting update events; update events
	// do not occur for the main menu if running under Ubunty with Unity
	// window manager.
	wxUpdateUIEvent::SetMode(wxUPDATE_UI_PROCESS_SPECIFIED);

#if (wxVERSION_NUMBER < 2900)
	wxSetlocale(LC_ALL, wxEmptyString);
#endif
	SetVendorName(sAuthor);
	SetAppName(sAppName);
	//	SetAppDisplayName(sAppDisplayName);
	const wxString name = GetAppName()+wxT("-")+wxGetUserId();

	m_checker = new wxSingleInstanceChecker(name);
	if (m_checker->IsAnotherRunning())
	{
#if defined(__WXMAC__)
		// on mac do not show this dialog, send data to app and shut down
		SendDataToOtherInstance();
		return false;
#else
		bool bQuit = true;

		// top level window needed

		// run another?
		wxMessageDialog dialog(NULL, wxT("Another instance of Arnold is already running.\nDo you wish to run another?"), wxT(""), wxYES_NO);
		if (dialog.ShowModal() == wxID_YES)
		{
			// if we say yes, then we don't quit and we continue
			bQuit = false;
		}

		if (bQuit)
		{
			SendDataToOtherInstance();

			return false;
		}
#endif
	}

	InitServer();

	wxSplashScreen* splash = NULL;
	wxFileSystem::AddHandler(new wxArchiveFSHandler);
	wxFileSystem::AddHandler(new wxFilterFSHandler);
	wxFileSystem::AddHandler(new wxZipFSHandler);



	wxImage::AddHandler(new wxPNGHandler);
	//#ifndef WIN32
	//    wxImage::AddHandler(new wxBMPHandler);
	//#endif

	if (m_bDisplaySplash)
	{

		wxBitmap bitmap;

		wxFileName SplashLogoPath(GetAppPath(), wxT("arnlogo.png"));
		wxString sSplashLogoPath = SplashLogoPath.GetFullPath();
		if (bitmap.LoadFile(sSplashLogoPath, wxBITMAP_TYPE_PNG))
		{
			splash = new wxSplashScreen(bitmap,
				wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_NO_TIMEOUT,
				0, NULL, wxID_ANY, wxDefaultPosition, wxDefaultSize,
				wxBORDER_SIMPLE | wxSTAY_ON_TOP);

		}
	}

	// on MacOSX:
	// 
	// ~/Library/Application Support/Arnold
	//
	// additional data here:
	// ~/Library/Saved Application State/com.thacker.arnold.savedState
	// 
	// On Linux:
	// 
	// ~/.Arnold
	//
	// On Windows:
	//
	// In registry 

	// create config
	CreateUserDataDir();
	m_pConfig = new wxConfig(GetAppName(), wxEmptyString, wxEmptyString, wxEmptyString, wxCONFIG_USE_SUBDIR | wxCONFIG_USE_LOCAL_FILE);
	if (m_pConfig != NULL)
	{
		wxString sConfigPath = m_pConfig->GetPath();
		wxCharBuffer buf = sConfigPath.mb_str();
		Log("Config file: '%s'\n", buf.data());

		// set this as current
		wxConfigBase::Set(m_pConfig);
	}
	else
	{
		wxMessageBox(wxT("Did not open config file. No settings will be saved."),
			GetAppName(), wxICON_INFORMATION | wxOK);
	}

	{
		wxString sCurrentDir = ::wxGetCwd();
		wxCharBuffer buf = sCurrentDir.mb_str();
		Log("Current dir: '%s'\n", buf.data());
	}

	wxXmlResource::Get()->InitAllHandlers();
	wxXmlResource::Get()->AddHandler(new DissassembleWindowResourceHandler());
	wxXmlResource::Get()->AddHandler(new MemDumpWindowResourceHandler());
	wxXmlResource::Get()->AddHandler(new StackWindowResourceHandler());
	//	wxXmlResource::Get()->AddHandler(new HardwareDetailsResourceHandler());
	//	wxXmlResource::Get()->AddHandler(new BreakpointsResourceHandler());
	wxXmlResource::Get()->AddHandler(new GraphicsEditorResourceHandler());
	//	wxXmlResource::Get()->AddHandler(new ArchiveListCtrlResourceHandler());
	// something around here is causing a 40 byte alloc when xrc can't be found
	wxXmlResource::Get()->Load(GetGUIFullPath());

	// initialise defaults
	SnapshotSettings_Init(SnapshotSettings_GetDefault());

	// unfinished
	ScanConfigsAppThenLocal(wxT("computer"), m_ComputerConfigs);
	ScanConfigsAppThenLocal(wxT("keyboard"), m_KeyboardConfigs);
	ScanConfigsAppThenLocal(wxT("joystick"), m_JoystickConfigs);

	for (int i = 0; i < CPC_MAX_DRIVES; i++)
	{
		DiskMedia *pDiskMedia = new DiskMedia(i);
		m_Drives[i] = pDiskMedia;
		m_Media.Add(pDiskMedia);
		pDiskMedia->LoadConfig();
	}

	{
		CassetteMedia *pCassetteMedia = new CassetteMedia();
		m_Cassette = pCassetteMedia;
		m_Media.Add(pCassetteMedia);
		pCassetteMedia->LoadConfig();
	}

	{
		CartridgeMedia *pCartridgeMedia = new CartridgeMedia();
		m_Cartridge = pCartridgeMedia;
		m_Media.Add(pCartridgeMedia);
		pCartridgeMedia->LoadConfig();
	}

	{
		SnapshotMedia *pSnapshotMedia = new SnapshotMedia();
		m_Snapshot = pSnapshotMedia;
		m_Media.Add(pSnapshotMedia);
		pSnapshotMedia->LoadConfig();
	}

#if 0
	{
		RomMedia *pRomMedia = new RomMedia();
		m_Rom = pRomMedia;
		m_Media.Add(pRomMedia);
	}
#endif

	{
		// init roms
		for (int i = 0; i < GetNumBuiltinMedia(); i++)
		{
			BuiltinMedia[i].nLength = 0;
			BuiltinMedia[i].pData = NULL;
		}

		// now load
		for (size_t i = 0; i < GetNumBuiltinMedia(); i++)
		{
			unsigned char *pRomData;
			unsigned long RomLength;

			wxString sFilename = GetROMFullPath() + BuiltinMedia[i].sFilename;
			
			if (LoadLocalFile(sFilename, &pRomData, &RomLength))
			{
				BuiltinMedia[i].pData = pRomData;
				BuiltinMedia[i].nLength = RomLength;
			}
			else
			{
				wxCharBuffer buf = sFilename.mb_str();
				Log("Failed to load ROM %s\n", buf.data());

			}
		}
	}
	AutoType_Init();
	AutoRunFile_Init();

	Log("CPC_Initialise\n");
	/* initialise cpc hardware */
	CPC_Initialise();

	labelsets_init();


	YMOutput_Init();

	//    Multiface2_Init();

	// setup devices which are accessible in the devices window.
	// this is now the correct way to implement devices in a plug and play type
	// way
	Log("Device Init\n");

	// expansion devices

	// setup amdrum device
	// disabled - audio needs hooking up
	Amdrum_Init();
	// amram 2 device
	Amram2_Init(); // tested and working (note; if we see garbage on screen when programming amram this indicates ram is still active)
	// setup megarom device
	MegaROM_Init();			// tested and working
	LowerROM_Init();		// tested and working
	rombo_Init();			// tested and working
	// setup symbiface device
	Symbiface_Init();		// working but incomplete	
	DobbertinHD20_Init();	//working but incomplete
	Vortexwd20_Init();
	uide_Init();
	// brunword mk2
	BrunWordMk2_Init();	 // tested and working
	// brunword mk4
	BrunWordMk4_Init(); // tested and working
	// CPC-AY
	// not fully working
	CTCAY_Init();
	// Multiface2
	// multiface needs to be checked
	Multiface2_Init();
	// dk'tronics speech
	// disabled audio needs hooking up
	DkTronicsSpeech_Init();
	// ssa-1 speech
	// disabled audio needs hooking up
	SSA1SpeechDevice_Init();
	// kempston mouse
	// mouse needs to be hooked up
	KempstonMouse_Init();
	// magnum light phaser
	// disabled lightpen needs hooking up
	MagnumLightPhaserDevice_Init();
	// disabled lightpen needs hooking up
	TrojanLightPhaserDevice_Init();
	// disabled audio needs hooking up
	//DigiblasterDevice_Init();
	//hack to force the digiblaster
	DigiblasterDevice_Init();

	// disc wizard
	// incomplete
	DiscWizard_Init();
	// trans tape
	// incomplete
	TransTape_Init();
	// Inicron RAM-ROM
	RamRom_Init();			// needs testing
	// disabled not complete
	MHT64K_Init();
	// disabled not complete
	MirageImager_Init();
	PALDeviceIndex = PAL16L8_Init();
	Yarek4MB_Init();
	VortexRAM_Init();
	InicronRAM_Init();		// not accurate but works
	DkTronics256KBRam_Init();
	DkTronics256KBSiliconDisk_Init();
	//CTRAM_Init();

	// joystick devices
	// disabled lightpen needs hooking up
	Westphaser_Init();
	// disabled lightpen needs hooking up
	Gunstick_Init();
	// needs hooking into mouse again
	AmxMouse_Init();
	hackit_Init();	// tested and working fine
	hexam_Init();	// tested and working fine

	KCCFloppy_Init();

	// printer devices 
	// disabled needs audio hooking up
	//DigiblasterDevice_Init();

	PrintToFileDevice_Init();
	XMass_Init();
	XMem_Init();
	Multiplay_Init(); // tested and works fine

	int nValue;
	bool bValue;
	wxString sValue;

	// setup devices..
	for (int i = 0; i < EmuDevice_GetNumDevices(); i++)
	{
		wxString sDeviceName(EmuDevice_GetSaveName(i), wxConvUTF8);
		wxString sKey;


		// should we restore data?

		// device is enabled?
		sKey.Printf(wxT("devices/%s/enabled"), sDeviceName.c_str());
		if (wxConfig::Get(false)->Read(sKey, &bValue, false))
		{
			EmuDevice_Enable(i, bValue ? TRUE : FALSE);
			Log("Device %s is %s\n", EmuDevice_GetName(i), bValue ? "enabled" : "disabled");
		}

		// setup switches
		for (int j = 0; j < EmuDevice_GetNumSwitches(i); j++)
		{
			wxString sSwitchName(EmuDevice_GetSwitchSaveName(i, j), wxConvUTF8);

			sKey.Printf(wxT("devices/%s/switches/%s/state"), sDeviceName.c_str(), sSwitchName.c_str());
			// If the setting doesn't exist
			if (wxConfig::Get(false)->Read(sKey, &bValue, false))
			{
				EmuDevice_SetSwitchState(i, j, bValue ? TRUE : FALSE);
				Log("Device %s, switch %s: %s\n", EmuDevice_GetName(i), EmuDevice_GetSwitchName(i, j), bValue ? "on" : "off");
			}
		}


		// setup roms; these are device on-board roms
		for (int j = 0; j < EmuDevice_GetNumRoms(i); j++)
		{
			wxString sRomName(EmuDevice_GetRomSaveName(i, j), wxConvUTF8);

			sKey.Printf(wxT("devices/%s/roms/%s/filename"), sDeviceName.c_str(), sRomName.c_str());
			if (wxConfig::Get(false)->Read(sKey, &sValue, wxEmptyString))
			{
				if (sValue.IsEmpty())
				{
					Log("Device %s, rom %s: no file picked for ROM\n", EmuDevice_GetName(i), EmuDevice_GetRomName(i, j));
				}
				else if (IsBuiltinMediaPath(sValue))
				{
					int nBuiltinRom = FindBuiltinMediaByPath(sValue);
					unsigned char *pData = NULL;
					unsigned long nLength = 0;
					GetBuiltinMediaDataByIndex(nBuiltinRom, &pData, &nLength);
					EmuDevice_SetRom(i, j, pData, nLength);

					Log("Device %s, rom %s using builtin media\n", EmuDevice_GetName(i), EmuDevice_GetRomName(i, j));
				}
				else
				{
					unsigned char *pData = NULL;
					unsigned long nLength = 0;
					if (!LoadLocalFile(sValue, &pData, &nLength))
					{
						wxCharBuffer buf = sValue.mb_str();
						Log("Failed to load ROM: Device %s, rom %s, filename %s\n", EmuDevice_GetName(i), EmuDevice_GetRomName(i, j), buf.data());

					}
					else
					{
						wxCharBuffer buf = sValue.mb_str();
						EmuDevice_SetRom(i, j, pData, nLength);
						free(pData);
						Log("Set ROM data for Device %s, rom %s, filename %s\n", EmuDevice_GetName(i), EmuDevice_GetRomName(i, j), buf.data());
					}
				}
			}
		}

		ExpansionRomData *pExpansionRomData = EmuDevice_GetExpansionRomData(i);
		if (pExpansionRomData != NULL)
		{
			for (int j = 0; j < 256; j++)
			{
				/* device exposes this rom? */
				if (ExpansionRom_IsAvailable(pExpansionRomData, j))
				{
					wxString sConfigPrefix;

					sConfigPrefix.Printf(wxT("devices/%s/ExpansionRoms/ExpansionRom%d/"), sDeviceName.c_str(), j);

					if (wxConfig::Get(false)->Read(sConfigPrefix + wxT("enabled"), &bValue, false))
					{
						ExpansionRom_SetActiveState(pExpansionRomData, j, bValue ? TRUE : FALSE);
					}
					sKey.Printf(wxT("devices/%s/ExpansionRoms/ExpansionRom%d/filename"), sDeviceName.c_str(), j);
					if (wxConfig::Get(false)->Read(sKey, &sValue, wxEmptyString))
					{

						if (!sValue.IsEmpty())
						{
							if (IsBuiltinMediaPath(sValue))
							{
								int nBuiltinRom = FindBuiltinMediaByPath(sValue);
								unsigned char *pData = NULL;
								unsigned long nLength = 0;
								GetBuiltinMediaDataByIndex(nBuiltinRom, &pData, &nLength);

								int nResult = ExpansionRom_SetRomData(pExpansionRomData, pData, nLength, j);

								if (nResult == ARNOLD_STATUS_OK)
								{
									wxCharBuffer buf = sValue.mb_str();
									Log("Set ROM data for Device %s, Expansion rom %d, using built in %s\n", EmuDevice_GetName(i), j, buf.data());
								}
								else
								{
									wxCharBuffer buf = sValue.mb_str();
									Log("Failed to set expansion ROM: Device %s, expansion rom %d, using built in %s\n", EmuDevice_GetName(i), j, buf.data());
								}
							}
							else
							{

								unsigned char *pData = NULL;
								unsigned long nLength = 0;
								if (!LoadLocalFile(sValue, &pData, &nLength))
								{
									wxCharBuffer buf = sValue.mb_str();
									Log("Failed to load expansion ROM: Device %s, expansion rom %d, filename %s\n", EmuDevice_GetName(i), j, buf.data());

								}
								else
								{
									wxCharBuffer buf = sValue.mb_str();
									int nResult = ExpansionRom_SetRomData(pExpansionRomData, pData, nLength, j);

									if (nResult == ARNOLD_STATUS_OK)
									{
										Log("Set ROM data for Device %s, Expansion rom %d, filename %s\n", EmuDevice_GetName(i), j, buf.data());
									}
									else
									{
										Log("Failed to set expansion ROM: Device %s, expansion rom %d, filename %s\n", EmuDevice_GetName(i), j, buf.data());
									}
									free(pData);
								}
							}

						}
					}
				}
			}
		}
	}

	{
		unsigned char *pCart;
		unsigned long CartLength;

		GetBuiltinMediaDataByID(ROM_ID_SYSTEM_CART_CSD, &pCart, &CartLength);

		Jukebox_InsertSystemCartridge(pCart, CartLength);
	}
	Joystick_InitDefaultSettings();






	DebuggerWindow::Init();

	/* CPC initialised, so set default setup */
	//	CPCEmulation_InitialiseDefaultSetup();

	wxConfig::Get(false)->Read(wxT("DefaultCartridgeID"), &nValue, 0);
	InsertDefaultCartridge(nValue);

	wxConfig::Get(false)->Read(wxT("ConfigName"), &sValue, wxT("cpc6128en"));
	{
		wxCharBuffer buf = sValue.mb_str();
		Log("Config wanted: '%s'\n", buf.data());

		ChooseConfig(sValue);
	}

	if (wxConfig::Get(false)->Read(wxT("fullscreen/mode"), &nValue, -1))
	{
		m_nFullScreenMode = nValue;
	}

	if (wxConfig::Get(false)->Read(wxT("audio/frequency"), &nValue, 44100))
	{
		SetAudioFrequency(nValue);
		Log("Audio frequency: %d\n", nValue);
	}

	if (wxConfig::Get(false)->Read(wxT("audio/bits"), &nValue, 8))
	{
		SetAudioBits(nValue);
		Log("Audio bits per sample: %d\n", nValue);
	}

	if (wxConfig::Get(false)->Read(wxT("audio/channels"), &nValue, 2))
	{
		SetAudioChannels(nValue);
		Log("Audio channels: %d\n", nValue);
	}

	if (wxConfig::Get(false)->Read(wxT("audio/output"), &nValue, (int)CPC_AUDIO_OUTPUT_MONO_SPEAKER))
	{
		Audio_SetOutput((CPC_AUDIO_OUTPUT_TYPE)nValue);
	}

	if (wxConfig::Get(false)->Read(wxT("audio/speaker_volume"), &nValue, SPEAKER_VOLUME_MAX))
	{
		CPC_SetSpeakerVolume(nValue);
	}

	// display...

	// 0 = unlimited. 100 = normal
	if (wxConfig::Get(false)->Read(wxT("window_scale/percent"), &nValue, 100))
	{
		CPC_SetWindowScale(nValue);
		Log("Window Scale: %d%%\n", CPC_GetWindowScale());
	}

	if (wxConfig::Get(false)->Read(wxT("display/fillscanlines"), &bValue, true))
	{
		Render_SetFillScanlines(bValue ? TRUE : FALSE);
	}
	if (wxConfig::Get(false)->Read(wxT("display/fulldisplay"), &bValue, false))
	{
		Render_SetFullDisplay(bValue ? TRUE : FALSE);
	}

	if (wxConfig::Get(false)->Read(wxT("display/double_height"), &bValue, true))
	{
		Render_SetScanlines(bValue ? TRUE : FALSE);
	}


#if 0
	const char *sAudio = "Unknown";
	switch (nValue)
	{
	case 0:
	{
		sAudio = "Speaker";
	}
	break;
	case 1:
	{


	}
	break;


	Log("Audio output: %d\n", nValue);
#endif

	if (!m_PlatformSpecific.Init(m_bAudio, m_bJoystick))
	{
		return false;
	}
	m_PlatformSpecific.AutoConfigureTouch();

	m_PlatformSpecific.AutoConfigureJoystick();
	m_PlatformSpecific.ConfigureAudio();
#if 0
#ifdef USE_SDL2
	// sdl2 on linux; works ok, window size is small!
	if (!m_PlatformSpecific.Init(m_bAudio, m_bJoysticConfigurek))
	{
		return false;
	}
	m_PlatformSpecific.AutoConfigureTouch();

	m_PlatformSpecific.AutoConfigureJoystick();
	m_PlatformSpecific.ConfigureAudio();
#endif
#endif
	frame = new arnguiFrame(m_bFullScreen, 0L);
	SetTopWindow(frame);
	if (splash)
	{
		// delete the splash screen now the window has shown
		splash->Destroy();
	}
	frame->Show();


	if (wxConfig::Get(false)->Read(wxT("cassette/negate_polarity"), &bValue, false))
	{
		Cassette_NegatePolarity(bValue ? TRUE : FALSE);
		Log("Tape negate polarity of signal: %s\n", bValue ? "yes" : "no");
	}


	if (wxConfig::Get(false)->Read(wxT("csd/auto_timer_enabled"), &bValue, false))
	{
		Jukebox_SetTimerEnabled(bValue ? TRUE : FALSE);
		Log("CSD auto timer enabled: %s\n", bValue ? "yes" : "no");
	}
	if (wxConfig::Get(false)->Read(wxT("csd/auto_timer_selection"), &nValue, 3))
	{
		Jukebox_SetTimerSelection(nValue);
		Log("CSD auto timer selection: %d\n", nValue);
	}

	if (wxConfig::Get(false)->Read(wxT("csd/manual_timer_selection"), &nValue, 7))
	{
		Jukebox_SetManualTimerSelection(nValue);
		Log("CSD manual timer selection: %d\n", nValue);
	}

	for (int i = 0; i < 13; i++)
	{
		wxString sConfigPrefix;

		sConfigPrefix.Printf(wxT("csd/cartridge%d/"), i);

		// system cartridge is enabled by default for csd. All others are disabled by default.
		bool bDefault = false;
		if (i == 0)
		{
			bDefault = true;
		}
		if (wxConfig::Get(false)->Read(sConfigPrefix + wxT("enabled"), &bValue, bDefault))
		{
			Jukebox_SetCartridgeEnable(i, bValue ? TRUE : FALSE);
			Log("CSD cartridge %d: %s\n", i, Jukebox_IsCartridgeEnabled(i) ? "enabled" : "disabled");
		}

		if (wxConfig::Get(false)->Read(sConfigPrefix + wxT("filename"), &sValue, wxEmptyString))
		{
			if (!sValue.IsEmpty())
			{
				if (IsBuiltinMediaPath(sValue))
				{
					int nBuiltinRom = FindBuiltinMediaByPath(sValue);
					unsigned char *pData = NULL;
					unsigned long nLength = 0;
					GetBuiltinMediaDataByIndex(nBuiltinRom, &pData, &nLength);
					Jukebox_CartridgeInsert(i, pData, nLength);

					Log("CSD cartridge %d uses builtin media\n", i);
				}
				else
				{
					unsigned char *pData = NULL;
					unsigned long nLength = 0;
					if (!LoadLocalFile(sValue, &pData, &nLength))
					{
						wxCharBuffer buf = sValue.mb_str();
						Log("Failed to load CSD cartridge %d: %s\n", i, buf.data());

					}
					else
					{
						wxCharBuffer buf = sValue.mb_str();

						// load it
						Jukebox_CartridgeInsert(i, pData, nLength);
						Log("Loaded CSD cartridge %d: %s\n", i, buf.data());
						free(pData);
					}
				}
			}
		}
	}

	if (wxConfig::Get(false)->Read(wxT("cassette/ignore_motor"), &bValue, false))
	{
		Cassette_SetIgnoreRelay(bValue ? TRUE : FALSE);
		Log("Tape player ignores motor/relay on/off request: %s\n", bValue ? "yes" : "no");
	}

	//  wxConfig::Get(false)->Read(wxT("disc/side_switch_active"), &bValue, false);
	// FDI_SetSwapSides(bValue);

	if (wxConfig::Get(false)->Read(wxT("disc/force_side_1"), &bValue, false))
	{
		FDI_SetForceSide1(bValue ? TRUE : FALSE);
		Log("FDI force side 1 (side switch): %s\n", bValue ? "yes" : "no");
	}

	if (wxConfig::Get(false)->Read(wxT("disc/force_disc_rom_off"), &bValue, false))
	{
		FDI_SetForceDiscRomOff(bValue ? TRUE : FALSE);
		Log("FDI force disc rom off : %s\n", bValue ? "yes" : "no");
	}

	if (wxConfig::Get(false)->Read(wxT("disc/drive_switch_active"), &bValue, false))
	{
		FDI_SetSwapDrives(bValue ? TRUE : FALSE);
		Log("FDI drive switch: %s\n", bValue ? "A=B, B=A" : "A=A, B=B");
	}

	if (wxConfig::Get(false)->Read(wxT("disc/4_drives_enable"), &bValue, false))
	{
		FDI_Set4Drives(bValue ? TRUE : FALSE);
		Log("FDI use 4 drives: %s\n", bValue ? "yes" : "no");
	}

	if (wxConfig::Get(false)->Read(wxT("links/exp_link"), &bValue, true))
	{
		CPC_SetExpLow(bValue ? TRUE : FALSE);
		Log("/EXP input to CPC is LOW: %s\n", bValue ? "yes" : "no");
	}

	if (wxConfig::Get(false)->Read(wxT("links/50hz_link"), &bValue, true))
	{
		CPC_Set50Hz(bValue ? TRUE : FALSE);
		Log("CPC 50Hz setting: %s\n", bValue ? "yes" : "no");
	}

	if (wxConfig::Get(false)->Read(wxT("links/computer_name_link"), &nValue, (int)PPI_COMPUTER_NAME_AMSTRAD))
	{
		CPC_SetComputerNameIndex(nValue);
		Log("CPC Computer name: %d\n", nValue);
	}
	if (wxConfig::Get(false)->Read(wxT("monitor/monitor_type"), &nValue, (int)CPC_MONITOR_COLOUR))
	{
		CPC_SetMonitorType((CPC_MONITOR_TYPE_ID)nValue);
		Log("CPC Monitor Type: %d\n", nValue);
	}
	if (wxConfig::Get(false)->Read(wxT("crtc/crtc_type"), &nValue, (int)0))
	{
		CPC_SetCRTCType(nValue);
		Log("CPC CRTC Type: %d\n", nValue);
	}

	// ym
	if (wxConfig::Get(false)->Read(wxT("ym/record/version"), &nValue, YMOUTPUT_VERSION_5))
	{
		YMOutput_SetVersion(nValue);
		Log("YM recording version %d\n", nValue);
	}

	if (wxConfig::Get(false)->Read(wxT("ym/record/start_record_when_silence_ends"), &bValue, true))
	{
		YMOutput_SetRecordWhenSilenceEnds(bValue ? TRUE : FALSE);
		Log("YM start recording when silence ends: %s\n", bValue ? "yes" : "no");
	}

	if (wxConfig::Get(false)->Read(wxT("ym/record/stop_record_when_silence_begins"), &bValue, true))
	{
		YMOutput_SetStopRecordwhenSilenceBegins(bValue ? TRUE : FALSE);
		Log("YM stop recording when silence begins: %s\n", bValue ? "yes" : "no");
	}
	if (wxConfig::Get(false)->Read(wxT("onboard_rom_override/os/enable"), &bValue, false))
	{
		CPC_SetOSOverrideROMEnable(bValue ? TRUE : FALSE);
		Log("On-board OS ROM Override: %s\n", bValue ? "yes" : "no");
	}
	if (wxConfig::Get(false)->Read(wxT("onboard_rom_override/os/filename"), &sValue, wxEmptyString))
	{
		if (!sValue.IsEmpty())
		{
			if (IsBuiltinMediaPath(sValue))
			{
				int nBuiltinRom = FindBuiltinMediaByPath(sValue);
				unsigned char *pData = NULL;
				unsigned long nLength = 0;
				GetBuiltinMediaDataByIndex(nBuiltinRom, &pData, &nLength);
				CPC_SetOSOverrideROM(pData, nLength);

				Log("On-board OS ROM override with builtin media\n");
			}
			else
			{
				unsigned char *pData = NULL;
				unsigned long nLength = 0;
				if (!LoadLocalFile(sValue, &pData, &nLength))
				{
					wxCharBuffer buf = sValue.mb_str();
					Log("Failed to load os override rom %s\n", buf.data());

					CPC_SetOSOverrideROMEnable(FALSE);
				}
				else
				{
					wxCharBuffer buf = sValue.mb_str();

					// todo length
					CPC_SetOSOverrideROM(pData, nLength);
					Log("Set os override rom %s\n", buf.data());
					free(pData);
				}
			}
		}
		else
		{
			Log("OS override ROM set but no file is defined.\n");
			CPC_SetOSOverrideROMEnable(FALSE);
		}
	}
	if (wxConfig::Get(false)->Read(wxT("onboard_rom_override/basic/enable"), &bValue, false))
	{
		CPC_SetBASICOverrideROMEnable(bValue ? TRUE : FALSE);
		Log("On-board BASIC ROM Override: %s\n", bValue ? "yes" : "no");
	}

	if (wxConfig::Get(false)->Read(wxT("onboard_rom_override/basic/filename"), &sValue, wxEmptyString))
	{
		if (!sValue.IsEmpty())
		{
			if (IsBuiltinMediaPath(sValue))
			{
				int nBuiltinRom = FindBuiltinMediaByPath(sValue);
				unsigned char *pData = NULL;
				unsigned long nLength = 0;
				GetBuiltinMediaDataByIndex(nBuiltinRom, &pData, &nLength);
				CPC_SetBASICOverrideROM(pData, nLength);

				Log("On-board BASIC ROM override with builtin media\n");
			}
			else
			{

				unsigned char *pData = NULL;
				unsigned long nLength = 0;
				if (!LoadLocalFile(sValue, &pData, &nLength))
				{
					wxCharBuffer buf = sValue.mb_str();
					Log("Failed to load basic override rom %s\n", buf.data());
					CPC_SetBASICOverrideROMEnable(FALSE);
				}
				else
				{
					wxCharBuffer buf = sValue.mb_str();

					// todo length
					CPC_SetBASICOverrideROM(pData, nLength);
					Log("Set basic override rom %s\n", buf.data());
					free(pData);
				}
			}

		}
		else
		{
			Log("Basic override ROM set but no file is defined.\n");
			CPC_SetBASICOverrideROMEnable(FALSE);
		}
	}
	if (wxConfig::Get(false)->Read(wxT("onboard_rom_override/amsdos/enable"), &bValue, false))
	{
		CPC_SetAmsdosOverrideROMEnable(bValue ? TRUE : FALSE);
		Log("On-board AMSDOS ROM Override: %s\n", bValue ? "yes" : "no");
	}

	if (wxConfig::Get(false)->Read(wxT("onboard_rom_override/amsdos/filename"), &sValue, wxEmptyString))
	{
		if (!sValue.IsEmpty())
		{
			if (IsBuiltinMediaPath(sValue))
			{
				int nBuiltinRom = FindBuiltinMediaByPath(sValue);
				unsigned char *pData = NULL;
				unsigned long nLength = 0;
				GetBuiltinMediaDataByIndex(nBuiltinRom, &pData, &nLength);
				CPC_SetAmsdosOverrideROM(pData, nLength);

				Log("On-board AMSDOS ROM override with builtin media\n");
			}
			else
			{

				unsigned char *pData = NULL;
				unsigned long nLength = 0;
				if (!LoadLocalFile(sValue, &pData, &nLength))
				{
					wxCharBuffer buf = sValue.mb_str();
					Log("Failed to load amsdos override rom %s\n", buf.data());
					CPC_SetAmsdosOverrideROMEnable(FALSE);
				}
				else
				{
					wxCharBuffer buf = sValue.mb_str();

					// todo length
					CPC_SetAmsdosOverrideROM(pData, nLength);
					Log("Set amsdos override rom %s\n", buf.data());
					free(pData);
				}
			}
		}
		else
		{
			Log("Amsdos override ROM set but no file is defined.\n");
			CPC_SetAmsdosOverrideROMEnable(FALSE);
		}
	}
	// ym
	if (wxConfig::Get(false)->Read(wxT("ym/record/version"), &nValue, YMOUTPUT_VERSION_5))
	{
		YMOutput_SetVersion(nValue);
		Log("YM recording version %d\n", nValue);
	}

	if (wxConfig::Get(false)->Read(wxT("ym/record/start_record_when_silence_ends"), &bValue, true))
	{
		YMOutput_SetRecordWhenSilenceEnds(bValue ? TRUE : FALSE);
		Log("YM start recording when silence ends: %s\n", bValue ? "yes" : "no");
	}
	if (wxConfig::Get(false)->Read(wxT("ym/record/stop_record_when_silence_begins"), &bValue, true))
	{
		YMOutput_SetStopRecordwhenSilenceBegins(bValue ? TRUE : FALSE);
		Log("YM stop recording when silence begins: %s\n", bValue ? "yes" : "no");
	}

	// drive settings
	for (int i = 0; i < CPC_MAX_DRIVES; i++)
	{
		wxString sConfigPrefix;

		sConfigPrefix.Printf(wxT("drive/%c/"), (char)(i + 'a'));

		// by default don't turn to side B
		if (wxConfig::Get(false)->Read(sConfigPrefix + wxT("turn_disk_to_side_b"), &bValue, false))
		{
			FDD_TurnDiskToSideB(i, bValue ? TRUE : FALSE);
			if (bValue)
			{
				Log("Drive %d is turned to side B\n", i);
			}
			else
			{
				Log("Drive %d is not turned over to side B\n", i);
			}
		}
		// by default just enable drive 0 and 1
		bool bEnableDrive = false;
		if ((i == 0) || (i == 1))
		{
			bEnableDrive = true;
		}
		if (wxConfig::Get(false)->Read(sConfigPrefix + wxT("enable"), &bValue, bEnableDrive))
		{
			FDD_Enable(i, bValue ? TRUE : FALSE);
			Log("Drive %d is %s.\n", i, bValue ? "enabled" : "disabled");
		}

		// by default don't force write protect
		if (wxConfig::Get(false)->Read(sConfigPrefix + wxT("force_write_protect"), &bValue, false))
		{
			FDD_SetAlwaysWriteProtected(i, bValue ? TRUE : FALSE);
			if (bValue)
			{
				Log("Drive %d is always write protected.\n", i);
			}
			else
			{
				Log("Drive %d is read/write.\n", i);
			}
		}

		// by default don't force ready
		if (wxConfig::Get(false)->Read(sConfigPrefix + wxT("force_ready"), &bValue, false))
		{
			FDD_SetAlwaysReady(i, bValue ? TRUE : FALSE);
			if (bValue)
			{
				Log("Drive %d is always ready.\n", i);
			}
			else
			{
				Log("Drive %d has correct ready signal operation.\n", i);
			}
		}

		// drive A defaults to single sided, all others are double sided
		bool bDoubleSided = true;
		if (i == 0)
		{
			bDoubleSided = false;
		}
		if (wxConfig::Get(false)->Read(sConfigPrefix + wxT("double_sided"), &bValue, bDoubleSided))
		{
			FDD_SetDoubleSided(i, bValue ? TRUE : FALSE);
			Log("Drive %d is %s sided.\n", i, bValue ? "double" : "single");
		}

		// drive A defaults to single sided 40 track
		int nTracks = MAX_TRACKS_80_TRACK;
		if (i == 0)
		{
			nTracks = MAX_TRACKS_40_TRACK;
		}
		if (wxConfig::Get(false)->Read(sConfigPrefix + wxT("max_tracks"), &nValue, nTracks))
		{
			FDD_SetTracks(i, nValue);
			Log("Drive %d has %d tracks.\n", i, nValue);
		}
	}


	for (int i = 0; i < CPC_NUM_DIGITAL_JOYSTICKS; i++)
	{
		wxString sConfigPrefix;

		sConfigPrefix.Printf(wxT("joystick/digital%d/"), i);

		RestoreJoystickConfig(sConfigPrefix, CPC_DIGITAL_JOYSTICK0 + i, "Digital");
	}


	for (int i = 0; i < CPC_NUM_ANALOGUE_JOYSTICKS; i++)
	{
		wxString sConfigPrefix;

		sConfigPrefix.Printf(wxT("joystick/analogue%d/"), i);

		RestoreJoystickConfig(sConfigPrefix, CPC_ANALOGUE_JOYSTICK0 + i, "Analogue");
	}

	for (int i = 0; i < MULTIPLAY_NUM_JOYSTICKS; i++)
	{
		wxString sConfigPrefix;

		sConfigPrefix.Printf(wxT("joystick/multiplay%d/"), i);

		RestoreJoystickConfig(sConfigPrefix, MULTIPLAY_JOYSTICK0 + i, "Multiplay");
	}

	if (wxConfig::Get(false)->Read(wxT("keystick/active"), &bValue, true))
	{
		Log("KeyStick is : %s\n", bValue ? "enabled" : "disabled");
		Joystick_KeyStickActive(bValue ? TRUE : FALSE);
	}

	//Joy to Key part
	if (wxConfig::Get(false)->Read(wxT("keyjoy/enabled"), &bValue, false))
	{
		Log("KeyJoy is : %s\n", bValue ? "enabled" : "disabled");
		KeyJoy_SetActive(bValue ? TRUE : FALSE);
	}

	/*
	wxConfig::Get(false)->Read(wxT("keyjoy/set"), &nValue, 0);
	Log("KeyJoy set : %d\n", nValue);
	KeyJoy_InitSet(nValue);
	*/

	if (wxConfig::Get(false)->Read(wxT("keyjoy/id"), &sValue, wxEmptyString))
	{
		int nID = m_PlatformSpecific.GetJoystickIdFromString(sValue);
		KeyJoy_SetPhysical(nID);

		wxCharBuffer buf = sValue.mb_str();
		Log("KeyJoy uses physical joystick %s\n", buf.data());
	}



	wxString sKeyID;
	for (int i = 0; i < MAXREDEFBUTTON; i++)
	{
		sKeyID.Printf(wxT("ButtonID%d"), i);
		if (wxConfig::Get(false)->Read(wxT("keyjoy/Remap/") + sKeyID, &nValue, -1))
		{
			KeyJoy_SetMappingJoyToKeyButton(i, nValue);
			if (nValue != -1)
			{
				Log("Keyjoy: Button %d is mapped to ", i);
				if (nValue < SPECIALACTIONID)
				{
					Log("%s\n", CPC_GetKeyName((CPC_KEY_ID)nValue));
				}
				else
				{
					Log("%s\n", GetActionCodeName((ACTION_CODE)(nValue - SPECIALACTIONID)));
				}
			}
		}
	}

	for (int i = 0; i < MAXREDEFAXIS; i++)
	{
		sKeyID.Printf(wxT("AxisID%dMin"), i);
		if (wxConfig::Get(false)->Read(wxT("keyjoy/Remap/") + sKeyID, &nValue, -1))
		{
			KeyJoy_SetMappingJoyToKeyAxis(i, KEYJOY_AXIS_MIN, nValue);
			if (nValue != -1)
			{
				Log("Keyjoy: Axis %d (Min) is mapped to ", i);

				if (nValue < SPECIALACTIONID)
				{
					Log("%s\n", CPC_GetKeyName((CPC_KEY_ID)nValue));
				}
				else
				{
					Log("%s\n", GetActionCodeName((ACTION_CODE)(nValue - SPECIALACTIONID)));
				}
			}
		}
		sKeyID.Printf(wxT("AxisID%dMax"), i);
		if (wxConfig::Get(false)->Read(wxT("keyjoy/Remap/") + sKeyID, &nValue, -1))
		{
			KeyJoy_SetMappingJoyToKeyAxis(i, KEYJOY_AXIS_MAX, nValue);
			if (nValue != -1)
			{
				Log("Keyjoy: Axis %d (Max) is mapped to ", i);

				if (nValue < SPECIALACTIONID)
				{
					Log("%s\n", CPC_GetKeyName((CPC_KEY_ID)nValue));
				}
				else
				{
					Log("%s\n", GetActionCodeName((ACTION_CODE)(nValue - SPECIALACTIONID)));
				}
			}
		}
	}


	for (int i = 0; i < MAXREDEFHAT; i++)
	{
		sKeyID.Printf(wxT("HatID%dXMin"), i);
		if (wxConfig::Get(false)->Read(wxT("keyjoy/Remap/") + sKeyID, &nValue, -1))
		{
			KeyJoy_SetMappingJoyToKeyHat(i, 0, KEYJOY_AXIS_MIN, nValue);
			if (nValue != -1)
			{
				Log("Keyjoy: Hat %d (Left) is mapped to ", i);

				if (nValue < SPECIALACTIONID)
				{
					Log("%s\n", CPC_GetKeyName((CPC_KEY_ID)nValue));
				}
				else
				{
					Log("%s\n", GetActionCodeName((ACTION_CODE)(nValue - SPECIALACTIONID)));
				}
			}
		}
		sKeyID.Printf(wxT("HatID%dXMax"), i);
		if (wxConfig::Get(false)->Read(wxT("keyjoy/Remap/") + sKeyID, &nValue, -1))
		{
			KeyJoy_SetMappingJoyToKeyHat(i, 0, KEYJOY_AXIS_MAX, nValue);
			if (nValue != -1)
			{
				Log("Keyjoy: Hat %d (Right) is mapped to ", i);

				if (nValue < SPECIALACTIONID)
				{
					Log("%s\n", CPC_GetKeyName((CPC_KEY_ID)nValue));
				}
				else
				{
					Log("%s\n", GetActionCodeName((ACTION_CODE)(nValue - SPECIALACTIONID)));
				}
			}
		}
		sKeyID.Printf(wxT("HatID%dYMin"), i);
		if (wxConfig::Get(false)->Read(wxT("keyjoy/Remap/") + sKeyID, &nValue, -1))
		{
			KeyJoy_SetMappingJoyToKeyHat(i, 1, KEYJOY_AXIS_MIN, nValue);
			if (nValue != -1)
			{
				Log("Keyjoy: Hat %d (Up) is mapped to ", i);

				if (nValue < SPECIALACTIONID)
				{
					Log("%s\n", CPC_GetKeyName((CPC_KEY_ID)nValue));
				}
				else
				{
					Log("%s\n", GetActionCodeName((ACTION_CODE)(nValue - SPECIALACTIONID)));
				}
			}
		}

		sKeyID.Printf(wxT("HatID%dYMax"), i);
		if (wxConfig::Get(false)->Read(wxT("keyjoy/Remap/") + sKeyID, &nValue, -1))
		{
			KeyJoy_SetMappingJoyToKeyHat(i, 1, KEYJOY_AXIS_MAX, nValue);
			if (nValue != -1)
			{
				Log("Keyjoy: Hat %d (Down) is mapped to ", i);

				if (nValue < SPECIALACTIONID)
				{
					Log("%s\n", CPC_GetKeyName((CPC_KEY_ID)nValue));
				}
				else
				{
					Log("%s\n", GetActionCodeName((ACTION_CODE)(nValue - SPECIALACTIONID)));
				}
			}
		}
	}

	if (wxConfig::Get(false)->Read(wxT("keyboard/positional/set"), &nValue, 0))
	{
		Keyboard_SetPositionalSet(nValue);
		SetKeySet(nValue);
		Log("Keyboard positional set: %d\n", nValue);
	}

	ReadPositionalKeyboardConfiguration();



	//snapshot
	if (wxConfig::Get(false)->Read(wxT("snapshot/write_version"), &nValue, 3))
	{
		SnapshotSettings_SetVersion(SnapshotSettings_GetDefault(), nValue);
		Log("Will write snapshot version %d\n", nValue);
	}

	if (wxConfig::Get(false)->Read(wxT("snapshot/compressed"), &bValue, true))
	{
		SnapshotSettings_SetCompressed(SnapshotSettings_GetDefault(), bValue ? TRUE : FALSE);
		Log("Will write snapshot 3: %s\n", bValue ? "compressed" : "uncompressed");
	}
	

	//  wxConfig::Get(false)->Read(wxT("debug/dissassemble/show_opcodes"), &bValue, true);

	// 0 = unlimited. 100 = normal
	if (wxConfig::Get(false)->Read(wxT("emulation_speed/percent"), &nValue, 100))
	{
		CPC_SetEmuSpeedPercent(nValue);
		if (CPC_GetEmuSpeedPercent() == 0)
		{
			Log("Emulation speed: Unlimited\n");
		}
		else
		{
			Log("Emulation speed: %d%%\n", CPC_GetEmuSpeedPercent());
		}
	}

	// this needs to be done always to setup host correctly
	wxConfig::Get(false)->Read(wxT("keyboard/mode"), &nValue, 0);
	{
		SetKeyboardMode(nValue);
		Log("Keyboard mode: %d\n", nValue);
	}

	if (wxConfig::Get(false)->Read(wxT("keyboard/language_detect"), &bValue, true))
	{
		Keyboard_SetAutoDetectLanguage(bValue ? true : false);
		Log("Keyboard language auto-detect: %s\n", bValue ? "yes" : "no");
	}

	if (wxConfig::Get(false)->Read(wxT("keyboard/language"), &nValue, KEYBOARD_LANGUAGE_ID_ENGLISH))
	{
		Keyboard_SetLanguage(nValue);
		Log("Keyboard language: %d\n", nValue);
	}

	if (wxConfig::Get(false)->Read(wxT("keyboard/hardware"), &nValue, 1))
	{
		Keyboard_EnableClash(nValue == 1 ? TRUE : FALSE);
	}

	if (wxConfig::Get(false)->Read(wxT("firmware/16_roms/supported"), &bValue, true))
	{
		Firmware_SetSupports16Roms(bValue ? true : false);
		Log("firmware auto-detect supports 16 ROMs: %s\n", bValue ? "yes" : "no");
	}

	if (wxConfig::Get(false)->Read(wxT("firmware/16_roms/auto_detect"), &bValue, true))
	{
		Firmware_SetAutoDetect16Roms(bValue ? true : false);
		Log("force firmware supports 16 ROMs: %s\n", bValue ? "yes" : "no");
	}

	g_TextWindowProperties.Init();

	RefreshJoystick();


	ExecuteCommandLine();

#if 0
#ifdef WIN32
	//http://msdn.microsoft.com/en-US/goglobal/bb895996.aspx

	TCHAR sKeyboardLayoutName[KL_NAMELENGTH];
	if (GetKeyboardLayoutName(sKeyboardLayoutName))
	{
		// English - 409 (us)
		// English - 809 (uk)
		// Finnish - 40b
		// Swedish - 41d
		// German - 407
		// Italian - 410
		// French - 40c
		// Spanish - 40a

		if (_tcsicmp(sKeyboardLayoutName,_T("00000409"))==0)
		{
			wxString sMessage;
			sMessage.Format(wxT("Keyboard layout detected as English (US)"));
			TraceMessage(sMessage);

			Keyboard_SetPositionalSet(0);

		}
		else
			if (_tcsicmp(sKeyboardLayoutName,_T("00000809"))==0)
			{
				wxString sMessage;
				sMessage.Format(wxT("Keyboard layout detected as English (UK)"));
				TraceMessage(sMessage);

				Keyboard_SetPositionalSet(0);

			}
			else
				if (_tcsicmp(sKeyboardLayoutName,_T("0000040c"))==0)
				{
					wxString sMessage;
					sMessage.Format(wxT("Keyboard layout detected as French Standard"));
					TraceMessage(sMessage);

					Keyboard_SetPositionalSet( 2 );

				}
				else
					if ( _tcsicmp( sKeyboardLayoutName, _T( "0000040a" ) ) == 0 )
					{
						wxString sMessage;
						sMessage.Format( wxT( "Keyboard layout detected as Spanish Traditional Sort" ) );
						TraceMessage( sMessage );

						Keyboard_SetPositionalSet( 3 );

					}
					else
						if ( _tcsicmp( sKeyboardLayoutName, _T( "00000407" ) ) == 0 )
						{
							wxString sMessage;
							sMessage.Format( wxT( "Keyboard layout detected as German Standard" ) );
							TraceMessage( sMessage );

							Keyboard_SetPositionalSet( 1 );

						}
						else
						{
							wxString sMessage;
							sMessage.Format(wxT("Keyboard layout not detected. Code: %s"), sKeyboardLayoutName);
							TraceMessage(sMessage);
						}
	}

#endif // WIN32
#endif //
	Log("Starting main loop\n");

	Keyboard_DetectLanguage();

	frame->UpdateMenuStates();


	// start the main loop
	return true;
}

	void arnguiApp::RestoreJoystickConfig(const wxString &sConfigPrefix, int id, const char *sType)
	{
		bool bValue;
		int nValue;
		int index = id % 2;
		wxString sValue;

		// by default joysticks are not enabled.. or should we just enable joystick 0?
		if (wxConfig::Get(false)->Read(sConfigPrefix + wxT("enable"), &bValue, false))
		{
			Joystick_Activate(id, bValue ? TRUE : FALSE);
			Log("%s Joystick %d is %s.\n", sType, index, bValue ? "active" : "inactive");
		}
		if (wxConfig::Get(false)->Read(sConfigPrefix + wxT("autofire/enable"), &bValue, false))
		{
			JoystickAF_Activate(id, bValue ? TRUE : FALSE);
			Log("%s Joystick %d autofire is %s.\n", sType, index, bValue ? "active" : "inactive");
		}

		if (wxConfig::Get(false)->Read(sConfigPrefix + wxT("autofire/rate"), &nValue, MAXAUTOFIRERATE))
		{
			JoystickAF_SetRate(id, nValue);
			Log("%s Joystick %d autofire rate %d.\n", sType, index, JoystickAF_GetRate(id));
		}
		// choose a default so that the radio buttons in the dialog show something sensible.
		if (wxConfig::Get(false)->Read(sConfigPrefix + wxT("type"), &nValue, JOYSTICK_TYPE_SIMULATED_BY_KEYBOARD))
		{
			// we need to set something to make the UI work.
			if (nValue == JOYSTICK_TYPE_UNKNOWN)
			{
				nValue = JOYSTICK_TYPE_SIMULATED_BY_KEYBOARD;
			}
			Joystick_SetType(id, nValue);

			Log("%s Joystick %d is ", sType, index);
			switch (nValue)
			{
			case JOYSTICK_TYPE_SIMULATED_BY_KEYBOARD:
			{
				Log("simulated by keyboard.\n");
			}
			break;

			case JOYSTICK_TYPE_SIMULATED_BY_MOUSE:
			{
				Log("simulated by mouse.\n");
			}
			break;

			case JOYSTICK_TYPE_REAL:
			{
				Log("a physical joystick/gamepad.\n");
			}
			break;


			default:
			{
				Log("controlled by unknown.\n");
			}
			break;
			}
		}

		// choose a default
		if (wxConfig::Get(false)->Read(sConfigPrefix + wxT("keyset"), &nValue, 0))
		{
			Joystick_SetKeySet(id, nValue);
			Log("%s Joystick %d uses keyset %d\n", sType, index, nValue);
		}

		if (wxConfig::Get(false)->Read(sConfigPrefix + wxT("id"), &sValue, wxEmptyString))
		{
			int nID = m_PlatformSpecific.GetJoystickIdFromString(sValue);
			Joystick_SetPhysical(id, nID);

			wxCharBuffer buf = sValue.mb_str();
			Log("%s Joystick %d uses physical joystick %s\n", sType, index, buf.data());
		}

		for (int j = 0; j < CPC_JOYSTICK_NUM_BUTTONS; j++)
		{
			wxString sKeyID;
			sKeyID.Printf(wxT("ButtonID%d"), j);


			if (wxConfig::Get(false)->Read(sConfigPrefix + wxT("redefinition/") + sKeyID, &nValue, -1))
			{
				Log("CPC joystick fire %d is mapped to PC joypad button %d\n", j, nValue);
				Joystick_SetButtonMappingCPC(id, j, nValue);
			}
		}

		for (int j = 0; j < CPC_JOYSTICK_NUM_AXES; j++)
		{
			wxString sKeyID;
			sKeyID.Printf(wxT("AxisID%d"), j);

			if (wxConfig::Get(false)->Read(sConfigPrefix + wxT("redefinition/") + sKeyID, &nValue, -1))
			{
				Log("CPC joystick %s is mapped to PC joypad axis %d\n", (j == 0) ? "X" : "Y", nValue);
				Joystick_SetAxisMappingPhysical(id, j, nValue);
			}
		}
#if 0           
		for ( int j = 0; j < MAXREDEFHAT; j++ )
		{
			wxString sKeyID;
			sKeyID.Printf(wxT("HatID%d"), j);

			//defaut values are set in joystick.c
			wxConfig::Get(false)->Read(sConfigPrefix + wxT("redefinition/") + sKeyID, &nValue, -1);
			if (nValue != -1)
			{
				Log("PC Joypad Hat/POV %d is mapped to CPC joystick %d X/Y", j, nValue);
				Joystick_SetHatMappingPhysical(id, j, nValue);
			}
		}
#endif

		//reload joystick simulated key
		for (int j = 0; j < JOYSTICK_SIMULATED_KEYID_LAST; j++)
		{
			wxString sKeyID;
			sKeyID.Printf(wxT("Simulated/keyID%d"), j);
			if (wxConfig::Get(false)->Read(sConfigPrefix + sKeyID, &nValue, 0))
			{
				Joystick_SetSimulatedKeyID(id, j, nValue);
			}
		}
	}

	void arnguiApp::WriteConfigWindowPos(const wxString &sName, const wxWindow *pWindow)
	{
		int nX, nY;

		pWindow->GetPosition(&nX, &nY);

		wxConfig::Get(false)->Write(sName + wxT("position/x"), nX);
		wxConfig::Get(false)->Write(sName + wxT("position/y"), nY);
	}


	bool arnguiApp::ReadConfigWindowPos(const wxString &sName, wxWindow *pWindow)
	{
		int nX, nY;

		if (wxConfig::Get(false)->Read(sName + wxT("position/x"), &nX))
		{
			if (wxConfig::Get(false)->Read(sName + wxT("position/y"), &nY))
			{


				pWindow->SetPosition(wxPoint(nX, nY));
				return true;
			}
		}

		return false;
	}

	void arnguiApp::WriteConfigWindowSize(const wxString &sName, const wxWindow *pWindow)
	{
		int nWidth, nHeight;

		pWindow->GetSize(&nWidth, &nHeight);

		wxConfig::Get(false)->Write(sName + wxT("dimensions/width"), nWidth);
		wxConfig::Get(false)->Write(sName + wxT("dimensions/height"), nHeight);
	}

	void arnguiApp::EnsureWindowVisible(wxWindow *pWindow)
	{
		int windowX, windowY;
		int windowHeight, windowWidth;

		pWindow->GetPosition(&windowX, &windowY);
		pWindow->GetSize(&windowWidth, &windowHeight);

		int clientX;
		int clientY;
		int clientWidth;
		int clientHeight;

		// get desktop size...
		::wxClientDisplayRect(&clientX, &clientY, &clientWidth, &clientHeight);

		bool bSetWindowPosition = false;
		if ((windowX + windowWidth) < clientX)
		{
			windowX = clientX;
			bSetWindowPosition = true;
		}

		if ((windowY + windowHeight) < clientY)
		{
			windowY = clientY;
			bSetWindowPosition = true;
		}

		if (windowX > (clientX + clientWidth))
		{
			windowX = (clientX + clientWidth) - windowWidth;
			bSetWindowPosition = true;
		}

		if (windowY > (clientY + clientHeight))
		{
			windowY = (clientY + clientHeight) - windowHeight;
			bSetWindowPosition = true;
		}

		if (bSetWindowPosition)
		{
			pWindow->SetPosition(wxPoint(windowX, windowY));
		}
	}

	void arnguiApp::WriteConfigWindowPosAndSize(const wxString &sName, const wxWindow *pWindow)
	{
		WriteConfigWindowSize(sName, pWindow);
		WriteConfigWindowPos(sName, pWindow);
	}

	bool arnguiApp::ReadConfigWindowSize(const wxString &sName, wxWindow *pWindow)
	{
		int nWidth, nHeight;

		if (wxConfig::Get(false)->Read(sName + wxT("dimensions/width"), &nWidth, -1))
		{

			if (wxConfig::Get(false)->Read(sName + wxT("dimensions/height"), &nHeight, -1))
			{
				if ((nWidth > 0) && (nHeight > 0))
				{

					pWindow->SetSize(nWidth, nHeight);
					return true;
				}
			}
		}

		return false;
	}

	bool arnguiApp::ReadConfigWindowPosAndSize(const wxString &sName, wxWindow *pWindow)
	{
		bool bState = ReadConfigWindowPos(sName, pWindow);
		if (bState)
		{
			bState = ReadConfigWindowSize(sName, pWindow);
		}
		return bState;
	}

	void arnguiApp::ReadConfigListCtrl(const wxString &sName, wxListCtrl *pListCtrl)
	{
		// number of columns
		int nColumnCount;
		if (wxConfig::Get(false)->Read(sName + wxT("NumColumns"), &nColumnCount, 0))
		{
			for (int i = 0; i < nColumnCount; i++)
			{
				wxString sColName;
				sColName.Printf(wxT("Column%d/"), i);

				int nWidth;
				// get width
				if (wxConfig::Get(false)->Read(sName + sColName + wxT("Width"), &nWidth, 0))
				{
					wxString sColumnName;
					// get name
					if (wxConfig::Get(false)->Read(sName + sColName + wxT("Name"), &sColumnName, wxEmptyString))
					{
						if (sName != wxEmptyString)
						{
							if (nWidth != 0)
							{
								for (int j = 0; j < pListCtrl->GetColumnCount(); j++)
								{
									// find column by name
									wxListItem Item;
									Item.SetId(j);
									Item.SetMask(wxLIST_MASK_TEXT);
									if (pListCtrl->GetColumn(j, Item))
									{
										if (Item.GetText() == sColumnName)
										{
											// set it's width
											pListCtrl->SetColumnWidth(j, nWidth);
											break;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}


	void arnguiApp::WriteConfigListCtrl(const wxString &sName, const wxListCtrl *pListCtrl)
	{
		// number of columns
		wxConfig::Get(false)->Write(sName + wxT("NumColumns"), pListCtrl->GetColumnCount());

		// name and width of each column
		for (int i = 0; i < pListCtrl->GetColumnCount(); i++)
		{
			wxListItem Item;
			Item.SetId(i);
			Item.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_WIDTH);
			pListCtrl->GetColumn(i, Item);

			wxString sColName;
			sColName.Printf(wxT("Column%d/"), i);

			wxConfig::Get(false)->Write(sName + sColName + wxT("Width"), Item.GetWidth());
			wxConfig::Get(false)->Write(sName + sColName + wxT("Name"), Item.GetText());

		}
	}



	bool arnguiApp::ReadConfigColour(const wxString &sName, wxColour &Colour, const wxColour &Default)
	{
		wxString sValue;
		if (wxConfig::Get(false)->Read(sName, &sValue, wxEmptyString))
		{
			wxString sElement;
			long nR, nG, nB;

			sElement = sValue.Mid(0, 2);
			sElement.ToLong(&nR, 16);

			sElement = sValue.Mid(2, 2);
			sElement.ToLong(&nG, 16);


			sElement = sValue.Mid(4, 2);
			sElement.ToLong(&nB, 16);

			Colour.Set(nR, nG, nB);

			return true;
		}

		Colour = Default;
		return false;
	}

	void arnguiApp::CreateUserDataDir()
	{
		// get user data dir
		wxString sUserDataDir = wxStandardPaths::Get().GetUserDataDir();

		// does it exist?
		if (!wxDir::Exists(sUserDataDir))
		{
			wxFileName::Mkdir(sUserDataDir);
		}
	}


	void arnguiApp::WriteConfigColour(const wxString &sName, const wxColour &Colour)
	{

		wxString sValue;
		sValue.Printf(wxT("%02x%02x%02x"), Colour.Red(), Colour.Green(), Colour.Blue());

		wxConfig::Get(false)->Write(sName, sValue);
	}

	bool arnguiApp::SaveScreenshot(const wxString &sFilename, bool bAsThumbnail)
	{
		int ScreenWidth, ScreenHeight;

		int ratio = 1;
		if (Render_GetScanlines())
		{
			ratio = 2;
		}

		/* the dimensions include scanlines */
		Render_GetWindowedDisplayDimensions(&ScreenWidth, &ScreenHeight);

		int HeightToRead = ScreenHeight;
		int LineLength = ScreenWidth * 3;
		int DataSize = LineLength*HeightToRead;

		unsigned char *pixels = new unsigned char[DataSize];
		if (pixels != NULL)
		{
			memset(pixels, 0, DataSize);

			for (int y = 0; y < HeightToRead; ++y)
			{
				int Offset = y*LineLength;

				for (int x = 0; x < ScreenWidth; x++)
				{
					unsigned char r, g, b;

					Render_GetPixelRGBAtXY(x, (y/ratio), &r, &g, &b);

					pixels[Offset + (x * 3) + 0] = r;
					pixels[Offset + (x * 3) + 1] = g;
					pixels[Offset + (x * 3) + 2] = b;
				}

			}
			wxImage image(ScreenWidth, ScreenHeight);
			image.SetData(pixels, true);

			//make thumb for snapshoot
			if (bAsThumbnail) image.Rescale(m_nThumbnailWidth, m_nThumbnailHeight);

			image.SaveFile(sFilename);
			delete[] pixels;
		}

		return true;
	}

	bool arnguiApp::LoadSnapshot(const wxString &sFilename)
	{
		return m_Snapshot->LoadWithSaveModified(sFilename, false);
	}

	// filename is media file which is being saved
	// we replace the extension and write out a thumbnail.
	void arnguiApp::SaveThumbnail(const wxString &sFilename)
	{
		// this code will get the location of filename, change it's extension and write thumbnail
		// here, however if the source is a readonly file system this WILL fail.
		// So we need to have a secondary writable location for thumbnails.
		wxFileName File(sFilename);
		File.SetExt(wxT("png"));
		SaveScreenshot(File.GetFullPath(), true);
	}

	bool arnguiApp::SaveSnapshot(const wxString &sFilename, SNAPSHOT_OPTIONS *pOptions, SNAPSHOT_MEMORY_BLOCKS *pMemoryBlocks)
	{
		unsigned long nLength;
		unsigned char *pSnapshotData = NULL;

		nLength = Snapshot_CalculateEstimatedOutputSize(pOptions, pMemoryBlocks);

		pSnapshotData = (unsigned char *)malloc(nLength);

		if (pSnapshotData != NULL)
		{
			unsigned long nActualLength = Snapshot_GenerateOutputData(pSnapshotData, pOptions, pMemoryBlocks);

			SaveLocalFile(sFilename, pSnapshotData, nActualLength);

			free(pSnapshotData);

			SaveThumbnail(sFilename);

			return true;
		}

		return false;
	}


#include <wx/file.h>
#include <wx/filedlg.h>
#include <wx/mstream.h>

	bool arnguiApp::LoadLocalFile(const wxString &sFilename, unsigned char **ppPtr, unsigned long *pLength)
	{
		*ppPtr = NULL;
		*pLength = 0;

		bool bState = false;
		wxFileSystem* fileSystem = new wxFileSystem();
		if (fileSystem != NULL)
		{
			// will fail if there is no extension
			wxFSFile* file = fileSystem->OpenFile(sFilename);

			if (file)
			{

				wxInputStream* pInputStream = file->GetStream();

#if 0
				wxMemoryOutputStream *pMemoryOutputStream = new wxMemoryOutputStream();
				pInputStream->Read( pMemoryOutputStream );

				size_t nLength = pMemoryOutputStream->GetLength();

				unsigned char *pData = (unsigned char *)malloc(nLength);
				if (pData != NULL)
				{
					pMemoryOutputStream->CopyTo(pData, nLength);
					*ppPtr = pData;
					*pLength = nLength;
					bSuccess = true;
				}

				delete pMemoryOutputStream;
#endif

				size_t nSize = pInputStream->GetSize();
				if (nSize != 0)
				{
					unsigned char *pData = (unsigned char *)malloc(nSize);
					if (pData != NULL)
					{
						pInputStream->Read(pData, nSize);
						*ppPtr = pData;
						*pLength = nSize;
						bState = true;
					}
				}
				// delete pInputStream;

				delete file;
			}

			delete fileSystem;
		}
#if 0

		bool bState = false;
		*ppPtr = NULL;
		*pLength = 0;

		wxFile *pFile = new wxFile();
		if ( pFile->Open( sFilename, wxFile::read ) )
		{

			wxFileOffset Length = pFile->Length();
			unsigned char *pPtr = (unsigned char *)malloc(Length);
			if (pPtr != NULL)
			{
				pFile->Read(pPtr, Length);
				*ppPtr = pPtr;
				*pLength = Length;
				bState = true;
			}
			pFile->Close();
		}
		delete pFile;
#endif

		return bState;
	}


	bool arnguiApp::CanSave(const wxString &sFilename)
	{
		wxFileName Filename(sFilename);

		// invalid path?
		if (!Filename.IsOk())
			return false;

		// represents a directory (it has a slash on the end)
		if (Filename.IsDir())
			return false;

		// now it has the potential to be a file, but construct a dir and see if it exists like this
		wxFileName PotentialDir(Filename.GetFullName() + Filename.GetPathSeparator());
		if (PotentialDir.DirExists())
			return false;

		// at this point we determine this is a file

		// if this represents a file
		// DirExists will check the dir the file
		// is in exists.
		if (!Filename.DirExists())
			return false;

		if (!Filename.IsDirWritable())
			return false;

		// if this represents a file
		// check it exists
		if (Filename.FileExists())
		{
			// then check we can write to it
			if (!Filename.IsFileWritable())
				return false;
		}

		// need to check if it has a zip extension!

		return true;
	}

	bool arnguiApp::SaveLocalFile(const wxString &sFilename, unsigned char *pPtr, unsigned long nLength)
	{
		bool bSuccess = false;
		// file already exists?
		if (::wxFileExists(sFilename))
		{
			// generate name of backup file
			wxString sNewFilename;
			sNewFilename = sFilename + wxT(".bak");
			// create backup
			::wxCopyFile(sFilename, sNewFilename, true);

			wxTempFile TempFile;
			if (TempFile.Open(sFilename))
			{
				if (TempFile.Write(pPtr, nLength))
				{
			//		TempFile.Flush();
					if (TempFile.Commit())
					{
						bSuccess = true;

					}
				}
			}
		}
		else
		{
			wxFile file(sFilename, wxFile::write);
			if (file.IsOpened())
			{
				file.Write(pPtr, nLength);
				//file.Flush();
				file.Close();
				bSuccess = true;
			}
		}
		return bSuccess;

#if 0
		// wxTempFileOutputStream creates a temporary file, when writing is complete
		// it then overwrites existing
		wxTempFileOutputStream *pTempFileOutputStream = new wxTempFileOutputStream( sFilename );

		if ( !pTempFileOutputStream->IsOK() )
		{
			pTempFileOutputStream->Discard();
			delete pTempFileOutputStream;
			return false;
		}

		bool bSuccess = false;
		wxTempFileOutputStream &Result = pTempFileOutputStream->Write( pPtr, nLength );
		if ( Result.LastWrite() == nLength )
		{
			bSuccess = true;
			pTempFileOutputStream->Commit();
		}
		else
		{
			pTempFileOutputStream->Discard();
		}

		delete pTempFileOutputStream;
		return bSuccess;
#endif
	}


	bool arnguiApp::SaveLocalFile(const wxString &sFilename, const wxString &sText)
	{
		bool bSuccess = false;
		// file already exists?
		if (::wxFileExists(sFilename))
		{
			// generate name of backup file
			wxString sNewFilename;
			sNewFilename = sFilename + wxT(".bak");
			// create backup
			::wxCopyFile(sFilename, sNewFilename, true);

			wxTempFile TempFile;
			if (TempFile.Open(sFilename))
			{
				if (TempFile.Write(sText))
				{
//					TempFile.Flush();

					if (TempFile.Commit())
					{
						bSuccess = true;

					}
				}
			}
		}
		else
		{
			wxFile file(sFilename, wxFile::write);
			if (file.IsOpened())
			{
				file.Write(sText);
	//			file.Flush();
				file.Close();
				bSuccess = true;
			}
		}
		return bSuccess;
	}



	void arnguiApp::CloseJoysticks()
	{

		m_PlatformSpecific.CloseJoysticks();
	}

	void arnguiApp::RefreshJoystick()
	{
		m_PlatformSpecific.RefreshJoysticks();

		for (int j = 0; j < CPC_NUM_JOYSTICKS; j++)
		{
			if (Joystick_GetType(j) == JOYSTICK_TYPE_SIMULATED_BY_KEYBOARD)
			{
				int nKeySet = Joystick_GetKeySet(j);
				m_PlatformSpecific.ConfigureJoystickKeySet(j, nKeySet);

			}
			else if (Joystick_GetType(j) == JOYSTICK_TYPE_SIMULATED_BY_MOUSE)
			{
				m_PlatformSpecific.ConfigureJoystickAsMouse(j);
			}
		}
	}


	void arnguiApp::SetKeySet(int nSet)
	{
		m_PlatformSpecific.SetKeySet(nSet);
	}


	// get the location of the GUI that come with the app are read from
	wxString arnguiApp::GetGUIFullPath()
	{
		//#ifndef __WXDEBUG__
		//    // for release
		//    wxFileName ExeData(GetAppPath(),  wxT("arngui.zip"));
		//    wxString sGUIFullPath = ExeData.GetFullPath();
		//    sGUIFullPath += wxT("#zip:GUIFrame.xrc");
		//    return sGUIFullPath;
		//#else
		// for debugging
		wxFileName GUIPath(GetAppPath(), wxT("GUIFrame.xrc"));
		return GUIPath.GetFullPath();
		//#endif
	}

	wxString strEscape(wxT("%:?#[]@!$&'()*+,;= %"));

	void Escape(wxString &str)
	{
		wxString strOut;

		for (size_t i = 0; i<str.Length(); i++)
		{
			bool bFound = false;

			wxChar ch = str[i];
			//	Log("%d (%c)\n", ch, ch);
			for (size_t j = 0; j<strEscape.Length(); j++)
			{
				//	Log("s: %d %c\n", strEscape[j], strEscape[j]);
				if ((ch == strEscape[j]) || (ch<20) || (ch>127))
				{
					wxString sAppend;
					sAppend.Printf(wxT("%%%02x"), ch);
					const wxCharBuffer buffer = sAppend.utf8_str();
					//		Log("%s\n", buffer.data());

					strOut += sAppend;

					bFound = true;
					break;
				}
			}

			if (!bFound)
			{
				strOut += ch;
			}
		}
		str = strOut;
		//const wxCharBuffer buffer = str.utf8_str();
		//	Log("%s\n", buffer.data());

	}

	// get the location of the roms that come with the app are read from
	wxString arnguiApp::GetROMFullPath()
	{
		wxFileName ExeData(GetAppPath(), wxT("roms.zip"));
		wxString sGUIFullPath = ExeData.GetFullPath();
		Escape(sGUIFullPath);
		sGUIFullPath += wxT("#zip:");
		return sGUIFullPath;
	}

	// get the directory path of the app (with trailing separator)
	// so we can easily add on another path or filename
	wxString arnguiApp::GetAppPath()
	{
		wxString sPath = wxStandardPaths::Get().GetExecutablePath();
		wxFileName Path(sPath);
// on mac the files are stored within the resources directory
#if defined(__WXMAC__)
		Path.Normalize();
		Path.RemoveLastDir();
		Path.AppendDir(wxT("Resources"));
#endif
		return Path.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
	}

	// get the location of the icons that are associated with file extensions (Windows)
	// and only if we are defined to be the default for that association
	wxString arnguiApp::GetIconsFullPath()
	{
		// get location of executable
		wxFileName IconPath(GetAppPath(), wxT("icons.dll"));
		return IconPath.GetFullPath();
	}

	Media::Media() : m_nType(MEDIA_TYPE_UNKNOWN), m_bMediaInserted(false), m_History(FILE_HISTORY_SIZE), m_nUnit(0)
	{
		m_nBaseID = 0;
		m_sCurrentPath = wxEmptyString;
	}

	wxString Media::GetRecentPath() const
	{
		if (m_History.GetCount() == 0)
			return wxEmptyString;

		return m_History.GetHistoryFile(0);
	}


	Media::~Media()
	{

	}

	void Media::Unload(bool bForce)
	{
		// if we force it, don't attempt to save
		if (!bForce)
		{
			// ask if the user wants to save..
			SaveModified(false);
		}
		Remove();
	}

	void Media::SaveAs()
	{
		//TODO:


	}

	wxString Media::GetSettingsKey()
	{
		const wxChar *sType = wxT("unknown");
		switch (m_nType)
		{
		case MEDIA_TYPE_CARTRIDGE:
		{
			sType = wxT("Cartridge");
		}
		break;

		case MEDIA_TYPE_DISK:
		{
			sType = wxT("Disk");
		}
		break;

		case MEDIA_TYPE_CASSETTE:
		{
			sType = wxT("Cassette");
		}
		break;

		case MEDIA_TYPE_SNAPSHOT:
		{
			sType = wxT("Snapshot");
		}
		break;

		default:
			break;
		}
		wxString sKey;
		sKey.Printf(wxT("media/%s%d/"), sType, m_nUnit);
		return sKey;
	}

	void Media::SaveConfig()
	{
		SaveHistory();
	}

	bool Media::RemoveHistoryItem(size_t nID)
	{
		if (!IsOurHistoryItem(nID))
			return false;

		// get the index
		size_t nIndex = nID - m_nBaseID;

		// remove it
		m_History.RemoveFileFromHistory(nIndex);

		SaveHistory();

		return true;
	}

	bool Media::IsOurHistoryItem(size_t nID)
	{
		// check ID is in our range
		if ((nID < m_nBaseID) || (nID >= (m_nBaseID + FILE_HISTORY_SIZE)))
			return false;

		// get the index
		size_t nIndex = nID - m_nBaseID;

		// check it doesn't exceed the size of the history
		if (nIndex >= m_History.GetCount())
			return false;

		return true;
	}

	bool Media::LoadHistoryItem(size_t nID)
	{
		if (!IsOurHistoryItem(nID))
			return false;

		// get the index
		size_t nIndex = nID - m_nBaseID;

		// now load it.
		if (!LoadWithSaveModified(m_History.GetHistoryFile(nIndex), true))
		{
			// ask if we want to save changes?
			wxString sMessage;
			sMessage.Printf(wxT("Failed to load %s. Remove from menu?"), m_History.GetHistoryFile(nIndex).c_str());
			wxMessageDialog dialog(wxGetApp().GetTopWindow(), sMessage, wxT(""), wxYES_NO);
			int nResult = dialog.ShowModal();

			if (nResult == wxID_YES)
			{
				wxGetApp().Log("Removing file\n");
				m_History.RemoveFileFromHistory(nIndex);
				SaveHistory();
			}
			return false;
		}

		return true;
	}


	void Media::ClearHistory()
	{
		for (size_t nItem = m_History.GetCount(); nItem != 0; nItem--)
		{
			m_History.RemoveFileFromHistory(nItem - 1);
		}
		SaveHistory();
	}

	void Media::LoadConfig()
	{
		wxString sPath = wxConfig::Get(false)->GetPath();

		wxConfig::Get(false)->SetPath(GetSettingsKey());
		m_History.Load(*wxConfig::Get(false));
		wxConfig::Get(false)->SetPath(sPath);

		wxString sKey = GetSettingsKey();
		wxString sEmpty = wxEmptyString;

		// read the current item
		wxConfig::Get(false)->Read(sKey + wxT("Current"), &m_sCurrentPath, sEmpty);
	}

	void Media::SetHistoryMenu(wxMenu *pMenu, size_t nID)
	{
		m_nBaseID = nID;
		m_History.UseMenu(pMenu);
		m_History.SetBaseId(m_nBaseID);
		m_History.AddFilesToMenu();
	}

	void Media::SaveHistory()
	{
		wxString sPath = wxConfig::Get(false)->GetPath();

		wxConfig::Get(false)->SetPath(GetSettingsKey());
		m_History.Save(*wxConfig::Get(false));
		wxConfig::Get(false)->SetPath(sPath);

		wxString sKey = GetSettingsKey();
		sKey += wxT("history/");

		// read the current item
		wxConfig::Get(false)->Write(sKey + wxT("Current"), m_sCurrentPath);

	}


	void Media::ForceSave()
	{
		SaveModified(true);
	}

	bool Media::LoadWithSaveModified(const wxString &sFilename, bool bAllowFallback)
	{
		SaveModified(false);
		Remove();

		return Load(sFilename, bAllowFallback);
	}


	bool Media::IgnoreModified()
	{
		if (!GetMediaInserted())
			return true;

		// can this media be modified?
		if (CanBeModified())
		{
			// is it modified?
			if (IsModified())
			{
				// ask if we want to save changes?
				wxString sMessage;
				sMessage.Printf(wxT("Media %s has been modified.\nIf you reload you will loose these changes. Continue?"), GetName().GetData());
				wxMessageDialog dialog(wxGetApp().GetTopWindow(), sMessage, wxT(""), wxYES_NO);
				int nResult = dialog.ShowModal();
				
				// we are happy to reload.
				if (nResult == wxID_YES)
					return true;

				// we will not reload
				return false;
			}
		}
		return true;
	}

	bool Media::Reload(bool bForce)
	{
		if (!GetMediaInserted())
			return true;

		if (!bForce)
		{
			// don't force reload - check we are sure we want to reload
			if (!IgnoreModified())
				return false;
		}
		wxString sPath = m_sCurrentPath;

		Remove();

		return Load(sPath, true);
	}

#if 0
	void Media::SaveLoop()
	{

		// loop until quit or we saved successfully
		while (!bDone)
		{
			// if we're not requesting a name... can we save at this path?
			if (!bRequestName && !CanSave(sSaveName))
			{
				wxString sMessage;
				sMessage.Printf(wxT("Media can't be saved to \"%s\".\nPlease pick a new filename."), m_sCurrentPath.c_str());

				// indicate we can't save here
				wxMessageDialog dialog(GetTopWindow(), sMessage, wxT(""),wxOK|wxCANCEL);
				int nResult = pDialog->ShowModal();
				
				if (nResult==wxID_CANCEL)
				{
					// can''t save, but we don't want to enter a name
					bDone = true;
				}
				else
				{
					// can't save, but we will enter a name
					bRequestName = true;
				}
			}


			if (!bDone && bRequestName)
			{
				wxFileName RecentPath(sSaveName);

				wxFileDialog openFileDialog(
					NULL,
					wxT(""),      // title
					RecentPath.GetPath(),         // default dir
					RecentPath.GetFullName(),     // default file
					wxT(""),
					wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

				if (openFileDialog.ShowModal() == wxID_CANCEL)
				{
					// user canceled picking of new file. we're done
					bDone = true;
				}
				else
				{
					// dont requesting a name
					bRequestName = false;
					// user picked a filename
					sSaveName = openFileDialog.GetFilename();
				}
			}

			if ( !bDone )
			{
				// go for a save..
				if ( Save( sSaveName ) )
				{
					Log( "Saved successfully\n" );

					// and reload it.
					LoadI( sSaveName );

					// we're done
					bDone = true;
				}
				else
				{
					// go back to start of loop
				}
			}
		}
	}
#endif

	bool Media::SaveModified(bool bForce)
	{
		bool bSaved = false;
		bool bDone = false;

		if (!GetMediaInserted())
			return true;

		// can this media be modified?
		if (CanBeModified())
		{
			// is it modified?
			if (IsModified() || bForce)
			{
				// ask if we want to save changes?
				wxString sMessage;
				sMessage.Printf(wxT("Media %s has been modified.\nDo you want to save changes?"), GetName().GetData());
				wxMessageDialog mediaModifiedDialog(wxGetApp().GetTopWindow(), sMessage, wxT(""), wxYES_NO);
				int nResult = mediaModifiedDialog.ShowModal();
				
				if (nResult == wxID_NO)
				{
					// wan't to quit
					bDone = true;
				}
				else
				{
					// we did not quit

					bool bRequestName = false;
					wxString sSaveName = m_sCurrentPath;

					// is path empty?
					if (m_sCurrentPath == wxEmptyString)
					{
						// tell the user and give them the choice to quit, or to continue
						wxMessageDialog dialog(wxGetApp().GetTopWindow(), wxT("No filename is set for media\nPlease pick a filename."), wxT(""), wxOK | wxCANCEL);
						nResult = dialog.ShowModal();

						if (nResult == wxID_CANCEL)
						{
							// we wanted to save, but we didn't want to enter a filename
							bDone = true;
						}
						else
						{
							// they continued, so suggest a name...
							// but go to file picker anyway

							// here we want to suggest a good default name

							sSaveName = wxT("new.dsk");

							// suggest a starting point, but we want to pick a name
							// sSaveName = GetRecentPath();
							bRequestName = true;
						}
					}

					//SaveLoop(sSaveName);



					// loop until quit or we saved successfully
					while (!bDone)
					{
						// if we're not requesting a name... can we save at this path?
						if (!bRequestName && !wxGetApp().CanSave(sSaveName))
						{
							sMessage.Printf(wxT("Media can't be saved to \"%s\".\nPlease pick a new filename."), m_sCurrentPath.c_str());

							// indicate we can't save here
							wxMessageDialog dialog(wxGetApp().GetTopWindow(), sMessage, wxT(""), wxOK | wxCANCEL);
							nResult = dialog.ShowModal();
							
							if (nResult == wxID_CANCEL)
							{
								// can''t save, but we don't want to enter a name
								bDone = true;
							}
							else
							{
								// can't save, but we will enter a name
								bRequestName = true;
							}
						}

						if (!bDone && bRequestName)
						{
							wxFileName RecentPath(sSaveName);

							wxFileDialog openFileDialog(
								NULL,
								wxT(""),      // title
								RecentPath.GetPath(),         // default dir
								RecentPath.GetFullName(),     // default file
								wxT(""),
								wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

							if (openFileDialog.ShowModal() == wxID_CANCEL)
							{
								// user canceled picking of new file. we're done
								bDone = true;
							}
							else
							{
								// dont requesting a name
								bRequestName = false;
								// user picked a filename
								sSaveName = openFileDialog.GetPath();
							}
						}

						if (!bDone)
						{
							// go for a save..
							if (Save(sSaveName))
							{
								wxGetApp().Log("Saved successfully\n");

								// and reload it?
								LoadI(sSaveName);

								// we're done
								bDone = true;
							}
							else
							{
								// go back to start of loop
							}
						}
					}
				}
			}
		}
		return bSaved;

	}


	void Media::RemoveI()
	{
		m_sCurrentPath = wxEmptyString;
		m_bMediaInserted = false;
	}

	bool Media::Save(const wxString &sFilename)
	{
		//???
		return true;
	}

	void Media::LoadI(const wxString &sFilename)
	{
		m_sCurrentPath = sFilename;
		m_bMediaInserted = true;

		if (m_sCurrentPath != wxEmptyString)
		{
			// CRASHES HERE WHEN SAVING ON EXIT.
			m_History.AddFileToHistory(m_sCurrentPath);
			SaveHistory();
		}
	}


	bool Media::IsModified() const
	{
		return false;
	}

	bool Media::CanBeModified() const
	{
		return false;
	}

	bool Media::Load(const wxString &sFilename, bool bAllowFallback)
	{
		return false;
	}

	void Media::Remove()
	{

	}

	void Media::SetName(const wxString &sName)
	{
		m_sName = sName;
	}

	void Media::SetUnit(int nUnit)
	{
		m_nUnit = nUnit;
	}

	void Media::SetType(int nType)
	{
		m_nType = nType;
	}

	DiskMedia::DiskMedia(int nUnit) : Media()
	{
		wxString sName;
		sName.Printf(wxT("FDD %d"), nUnit);
		SetName(sName);
		SetUnit(nUnit);
		SetType(MEDIA_TYPE_DISK);
	}

	bool DiskMedia::CanBeModified() const
	{
		return true;
	}


	DiskMedia::~DiskMedia()
	{

	}

	void DiskMedia::InsertUninitialised()
	{
		SaveModified(false);
		Remove();
		DiskImage_InsertUnformattedDisk(m_nUnit);
		m_bMediaInserted = true;
		m_sCurrentPath = wxEmptyString;

	}

	void DiskMedia::InsertInitialised(int nCode)
	{
		SaveModified(false);
		Remove();
		DiskImage_InsertFormattedDisk(m_nUnit, nCode);
		m_bMediaInserted = true;
		m_sCurrentPath = wxEmptyString;

	}


	bool DiskMedia::Load(const wxString &sFilename, bool bAllowFallback)
	{
		unsigned char *pDiskImageData = NULL;
		unsigned long DiskImageLength = 0;

		bool bSuccess = false;


		{
			wxCharBuffer buf = sFilename.mb_str();
			wxGetApp().Log("Disk: '%s'\n", buf.data());
		}

		/* try to load it */
		wxGetApp().LoadLocalFile(sFilename, &pDiskImageData, &DiskImageLength);

		if (pDiskImageData != NULL)
		{
			int nStatus = DiskImage_InsertDisk(m_nUnit, pDiskImageData, DiskImageLength);

			if (nStatus == ARNOLD_STATUS_OK)
			{
				LoadI(sFilename);
				bSuccess = true;
			}

			free(pDiskImageData);
		}

		return bSuccess;
	}

	bool DiskMedia::Save(const wxString &sFilename)
	{
		bool success = false;
		unsigned long nDiskImage = DiskImage_CalculateOutputSize(m_nUnit, FALSE);

		unsigned char *pDiskImage = (unsigned char *)malloc(nDiskImage);

		if (pDiskImage)
		{
			DiskImage_GenerateOutputData(pDiskImage, m_nUnit, FALSE);

			if (wxGetApp().SaveLocalFile(sFilename, pDiskImage, nDiskImage))
			{
				DiskImage_ResetDirty(m_nUnit);
				success = true;
			}

			free(pDiskImage);
		}
		return success;
	}


	void DiskMedia::Remove()
	{
		DiskImage_RemoveDisk(m_nUnit);
		RemoveI();
	}

	bool DiskMedia::IsModified() const
	{
		return DiskImage_IsImageDirty(m_nUnit) ? true : false;
	}



	CassetteMedia::CassetteMedia() : Media()
	{
		SetName(wxT("Cassette"));
		SetUnit(0);
		SetType(MEDIA_TYPE_CASSETTE);

	}

	bool CassetteMedia::CanBeModified() const
	{
		return true;
	}


	CassetteMedia::~CassetteMedia()
	{

	}

	bool CassetteMedia::Load(const wxString &sFilename, bool bAllowFallback)
	{
		unsigned char *pTapeImageData = NULL;
		unsigned long TapeImageLength = 0;
		bool bSuccess = false;

		{
			wxCharBuffer buf = sFilename.mb_str();
			wxGetApp().Log("Cassette: '%s'\n", buf.data());
		}
		/* try to load it */
		wxGetApp().LoadLocalFile(sFilename, &pTapeImageData, &TapeImageLength);

		if (pTapeImageData != NULL)
		{
			if (TapeImage_Insert(pTapeImageData, TapeImageLength) == ARNOLD_STATUS_OK)
			{
				LoadI(sFilename);

				free(pTapeImageData);

				bSuccess = true;

			}
			else
			{
				free(pTapeImageData);
				wxCharBuffer buffer = sFilename.ToUTF8();
				if (Sample_Load(buffer.data()))
				{
					bSuccess = true;
				}
			}
		}
		return bSuccess;

	}

	void CassetteMedia::Remove()
	{
		TapeImage_Remove();
		RemoveI();
	}

	bool CassetteMedia::IsModified() const
	{
		return false;
	}



	CartridgeMedia::CartridgeMedia()
	{
		SetName(wxT("Cartridge"));
		SetUnit(0);
		SetType(MEDIA_TYPE_CARTRIDGE);
	}

	CartridgeMedia::~CartridgeMedia()
	{

	}

	bool CartridgeMedia::Load(const wxString &sFilename, bool bAllowFallback)
	{
		bool bSuccess = false;
		unsigned char *pCartridgeData;
		unsigned long CartridgeLength;
		{
			wxCharBuffer buf = sFilename.mb_str();
			wxGetApp().Log("Cartridge: '%s'\n", buf.data());
		}

		/* try to load it */
		wxGetApp().LoadLocalFile(sFilename, &pCartridgeData, &CartridgeLength);

		if (pCartridgeData != NULL)
		{
			int nStatus = Cartridge_AttemptInsert(pCartridgeData, CartridgeLength);

			if (nStatus == ARNOLD_STATUS_OK)
			{
				LoadI(sFilename);
				bSuccess = true;
			}
			else
			{
				if (bAllowFallback)
				{
					Cartridge_InsertBinary(pCartridgeData, CartridgeLength);

					// if cartridge was inserted ok, then auto-start it
					Cartridge_Autostart();

					LoadI(sFilename);

					bSuccess = true;
				}
			}

			free(pCartridgeData);
		}
		return bSuccess;
	}

	void CartridgeMedia::Remove()
	{
		//to prevent removing of system cartridge
		if (GetMediaInserted())
		{
			Cartridge_Remove();
		}
		RemoveI();
	}





	SnapshotMedia::SnapshotMedia()
	{
		SetName(wxT("Snapshot"));
		SetUnit(0);
		SetType(MEDIA_TYPE_SNAPSHOT);

	}

	SnapshotMedia::~SnapshotMedia()
	{

	}

	bool SnapshotMedia::Load(const wxString &sFilename, bool bAllowFallback)
	{
		unsigned char *pSnapshotData;
		unsigned long SnapshotLength;
		bool bLoaded = false;
		{
			wxCharBuffer buf = sFilename.mb_str();
			wxGetApp().Log("Snapshot: '%s'\n", buf.data());
		}

		/* try to load it */
		wxGetApp().LoadLocalFile(sFilename, &pSnapshotData, &SnapshotLength);

		if (pSnapshotData != NULL)
		{
			SNAPSHOT_REQUIREMENTS Requirements;
			if (Snapshot_IsValid(pSnapshotData, SnapshotLength, &Requirements) == ARNOLD_STATUS_OK)
			{
				SNAPSHOT_MEMORY_BLOCKS SnapshotMemoryBlocks;
				Snapshot_CollectMemory(&SnapshotMemoryBlocks, SnapshotSettings_GetDefault(), TRUE);

				bool bLoad = false;

				/* are requirements met? */
				if (!Snapshot_AreRequirementsMet(&Requirements, &SnapshotMemoryBlocks))
				{
					SnapshotLoadSettingsDialog dialog(wxGetApp().GetTopWindow(), &Requirements, &SnapshotMemoryBlocks);
					if (dialog.ShowModal() == wxID_OK)
					{
						bLoad = true;
					}
				}
				else
				{
					bLoad = true;
				}

				if (bLoad)
				{
					if (Snapshot_Insert(pSnapshotData, SnapshotLength, &SnapshotMemoryBlocks) == ARNOLD_STATUS_OK)
					{

						LoadI(sFilename);
						bLoaded = true;
					}
					else
					{
						wxGetApp().Log("Failed snapshot insert\n");
					}
				}
			}

			free(pSnapshotData);
		}
		else
		{
			wxGetApp().Log("Failed snapshot load\n");
		}
		return bLoaded;
	}

	void arnguiApp::OpenPrinterFile(const wxString &sFilename)
	{
		m_pPrinterOutputStream = new wxFileOutputStream(sFilename);
		if (m_pPrinterOutputStream != NULL)
		{
			if (m_pPrinterOutputStream->IsOk())
			{

				// this should be handled by print to file really.
				Printer_SetBusyInput(FALSE);
				return;
			}
			else
			{
				wxGetApp().Log("printer output stream not ok\n");
			}
			delete m_pPrinterOutputStream;
			m_pPrinterOutputStream = NULL;
		}
	}

#if 0
	switch ( ( m_PrinterData & ( ( 1 << 2 ) | ( 1 << 1 ) ) ) )
	{
		/* 2 low, 1 low */
	case 0:
	{
		PrinterStatusShift = 0;
	}
	break;

	case ( 1 << 1 ) :
	{
		wxGetApp().Log( "Set busy to true\n" );
		/* indicates paper out */
		/* 2 is 0, 1 is 0 */
		Printer_SetBusyInput( TRUE );
	}
					break;

	case (1 << 2) | (1 << 1) :
	{
		wxGetApp().Log("other...");
	}
							 break;

	default:
		wxGetApp().Log("Set busy to false\n");
		Printer_SetBusyInput(FALSE);
		break;
	}
#endif



	void CPC_PrintToFile(char ch)
	{
		if (wxGetApp().m_pPrinterOutputStream != NULL)
		{
			wxGetApp().m_pPrinterOutputStream->PutC(ch);
		}
	}


	void arnguiApp::ClosePrinterFile(void)
	{
		if (m_pPrinterOutputStream != NULL)
		{
			m_pPrinterOutputStream->Close();
			delete m_pPrinterOutputStream;
			m_pPrinterOutputStream = NULL;

			// this should be handled by print to file.
			Printer_SetBusyInput(TRUE);
		}
	}


	void SnapshotMedia::Remove()
	{
		RemoveI();
	}

	void arnguiApp::ScanConfigsAppThenLocal(const wxString &sConfig, arnguiConfigArray &configList)
	{
		// path for app configs
		wxFileName sAppConfigPath(GetAppPath());
		sAppConfigPath.AppendDir(wxT("config"));
		sAppConfigPath.AppendDir(sConfig);
		ScanConfigs(sAppConfigPath.GetFullPath(), configList);

		// doesn't seem to append correctly putting "Arnold" on the end??
		// path for user configs
		wxFileName sUserConfigPath(wxStandardPaths::Get().GetUserDataDir());
		sUserConfigPath.AppendDir(wxT("config"));
		sUserConfigPath.AppendDir(sConfig);
		ScanConfigs(sUserConfigPath.GetFullPath(), configList);
	}



	void arnguiApp::ScanConfigs(const wxString &sDirectory, arnguiConfigArray &configList)
	{
		if (!wxDir::Exists(sDirectory))
			return;

		wxDir dir(sDirectory);
		if (!dir.IsOpened())
		{
			return;
		}

		wxString sFilename;

		bool bContinue = dir.GetFirst(&sFilename, wxT("*.cfg"));
		while (bContinue)
		{
			arnguiConfig *config = new arnguiConfig();
			config->m_sName = sFilename;
			config->m_sFilename = sFilename;
			configList.Add(config);

			bContinue = dir.GetNext(&sFilename);
		}
	}

	void arnguiApp::ReadConfig(const wxString &sFilename)
	{
		wxFileSystem* fileSystem = new wxFileSystem;
		if (fileSystem != NULL)
		{
			wxFSFile* file = fileSystem->OpenFile(sFilename);

			if (file)
			{

				wxInputStream* pInputStream = file->GetStream();
				wxFileConfig fileConfig((*pInputStream));

				// need display name


				//  file->Close();
			}
			delete fileSystem;
		}
	}

	void arnguiApp::RestartPower(bool bShowMessage, bool bSetBreakpoint)
	{
		bool bPerformRestart = true;

		if (bShowMessage)
		{
			bPerformRestart = false;
			wxMessageDialog dialog(GetTopWindow(), wxT("Are you sure you want to restart (Power method)?\nAll unsaved RAM contents will be lost."), wxApp::GetAppName(), wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION);;
			if (dialog.ShowModal() == wxID_YES)
			{
				bPerformRestart = true;
			}
		}

		if (bPerformRestart)
		{
			/* do here, because reset is called in other places */
			/* clear autotype */
			AutoType_Finish();
			/* clear auto run */
			AutoRunFile_Finish();

			/* do reset */
			Computer_RestartPower();

			//     if (bSetBreakpoint)
			//    {
			//       Breakpoints_AddBreakpoint(BREAKPOINT_TYPE_PC,0);
			//  }
		}
	}

	void arnguiApp::RestartReset(bool bShowMessage, bool bSetBreakpoint)
	{
		bool bPerformRestart = true;

		if (bShowMessage)
		{
			bPerformRestart = false;
			wxMessageDialog dialog(GetTopWindow(), wxT("Are you sure you want to restart (Reset method)?\nAll unsaved RAM contents will be lost."), wxApp::GetAppName(), wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION);;
			if (dialog.ShowModal() == wxID_YES)
			{
				bPerformRestart = true;
			}
		}

		if (bPerformRestart)
		{
			/* do here, because reset is called in other places */
			/* clear autotype */
			AutoType_Finish();
			/* clear auto run */
			AutoRunFile_Finish();

			/* do reset */
			Computer_RestartReset();


			//     if (bSetBreakpoint)
			//    {
			//       Breakpoints_AddBreakpoint(BREAKPOINT_TYPE_PC,0);
			//  }
		}




	}


	bool arnguiApp::PickBuiltInMedia(BUILTIN_MEDIA_TYPE nType, const wxString &sROMName, wxString &sId, unsigned char **ppData, unsigned long *pLength)
	{
		bool bPicked = false;

		int nRoms = 0;
		for (int i = 0; i < GetNumBuiltinMedia(); i++)
		{
			if (GetBuiltinMediaType(i) == nType)
			{
				nRoms++;
			}
		}

		wxArrayString sROMs;
		VOID_PTR *DataArray = new VOID_PTR[nRoms];

		nRoms = 0;
		for (int i = 0; i < GetNumBuiltinMedia(); i++)
		{
			if (GetBuiltinMediaType(i) == nType)
			{
				sROMs.Add(GetBuiltinMediaDisplayName(i));

				IntClientData *pClientData = new IntClientData(i);
				DataArray[nRoms] = (VOID_PTR)pClientData;
				++nRoms;
			}
		}

		wxString sMessage;
		sMessage = wxT("Choose built-in ROM for ") + sROMName;

#if (wxVERSION_NUMBER >= 2900)
		// 2.9
		wxSingleChoiceDialog dialog(GetTopWindow(), sMessage, wxT("Choose built-in ROM"), sROMs, (void **)DataArray);
#else
		// 2.8
		wxSingleChoiceDialog dialog(GetTopWindow(), sMessage, wxT("Choose built-in ROM"), sROMs, (char **)DataArray);
#endif
		dialog.SetSelection(0);
		if (dialog.ShowModal() == wxID_OK)
		{
#if (wxVERSION_NUMBER >= 2900)
			IntClientData *pClientData = (IntClientData *)dialog.GetSelectionData();
#else
			IntClientData *pClientData = (IntClientData *)dialog.GetSelectionClientData();
#endif
			int nBuiltinRom = pClientData->GetData();
			bPicked = true;

			/* path for storing */
			sId = GetBuiltinMediaPath(nBuiltinRom);

			GetBuiltinMediaDataByIndex(nBuiltinRom, ppData, pLength);
		}
	
		for (int i = 0; i < nRoms; i++)
		{
			delete (IntClientData *)DataArray[i];
		}
		delete[](IntClientData **)DataArray;

		return bPicked;
	}

	void arnguiApp::SetKeyboardMode(int KeyboardMode)
	{
		if ((KeyboardMode < 0) || (KeyboardMode>1))
		{
			KeyboardMode = 0;
		}

		Log("Keyboard mode %s\n", KeyboardMode == 0 ? "Positional" : "Translated");

		Keyboard_SetMode(KeyboardMode);
		m_PlatformSpecific.ConfigureKeyboardMode();
	}

	int arnguiApp::GetAudioFrequency()
	{
		return m_nAudioFrequency;
	}

	int arnguiApp::GetAudioBits()
	{
		return m_nAudioBits;
	}

	int arnguiApp::GetAudioChannels()
	{
		return m_nAudioChannels;
	}

	void arnguiApp::SetAudioChannels(int nChannels)
	{
		m_nAudioChannels = nChannels;
		Log("Audio channels: %d\n", nChannels);
	}

	void arnguiApp::SetAudioFrequency(int nFrequency)
	{
		m_nAudioFrequency = nFrequency;
		Log("Audio frequency: %d\n", nFrequency);
	}

	void arnguiApp::SetAudioBits(int nBits)
	{
		m_nAudioBits = nBits;
		Log("Audio bits per sample: %d\n", nBits);
	}

	void arnguiApp::SetFullScreenMode(int nMode)
	{
		m_nFullScreenMode = nMode;
	}

	int arnguiApp::GetFullScreenMode()
	{
		return m_nFullScreenMode;
	}

	void arnguiApp::NextSaveState()
	{
		m_SavePoint.GoForwards();
	}

	void arnguiApp::PrevSaveState()
	{
		m_SavePoint.GoBack();
	}

	void arnguiApp::SaveSaveState()
	{
		m_SavePoint.AddSavepoint();
	}

	void SavePoint::AddSavepoint()
	{
		// need to clear after
		ClearSavePointsAfter(m_nCurrentSavePoint);

		// get user data dir
		wxString sUserDataDir = wxStandardPaths::Get().GetUserDataDir();

		wxString sSavePointName;
		sSavePointName.sprintf(wxT("SavePoint%d"), m_nCurrentSavePoint);
		wxFileName SavePoint(sUserDataDir, sSavePointName);
		wxString sSavePointFilename = SavePoint.GetFullPath();

		// TODO
		SNAPSHOT_OPTIONS Options;
		SNAPSHOT_MEMORY_BLOCKS MemoryBlocks;
		Snapshot_CollectMemory(&MemoryBlocks, &Options, TRUE);

		wxGetApp().SaveSnapshot(sSavePointFilename, &Options, &MemoryBlocks);
		m_SavePoints.Add(sSavePointFilename);
		m_nCurrentSavePoint = m_SavePoints.Count() - 1;
		wxGetApp().Log("Saving save point\n");
		wxGetApp().Log("Current Save Point: %d\n", m_nCurrentSavePoint);

	}

	void SavePoint::GoForwards()
	{
		// load the next
		if ((int)m_nCurrentSavePoint != ((int)m_SavePoints.GetCount() - 1))
		{
			m_nCurrentSavePoint++;
			LoadSavePoint(m_nCurrentSavePoint);
			wxGetApp().Log("Current Save Point: %d\n", m_nCurrentSavePoint);
		}
		else
		{
			wxGetApp().Log("Can't go forwards at end of save points\n");
		}

	}
	void SavePoint::ClearSavePoints()
	{
		m_SavePoints.clear();
	}

	void SavePoint::ClearSavePointsAfter(int nPos)
	{
		for (int i = m_SavePoints.GetCount() - 1; i > nPos; i--)
		{
			m_SavePoints.RemoveAt(i);
		}
	}

	void SavePoint::GoBack()
	{
		// load the prev
		if (m_nCurrentSavePoint != 0)
		{
			m_nCurrentSavePoint--;
			LoadSavePoint(m_nCurrentSavePoint);
			wxGetApp().Log("Current Save point: %d\n", m_nCurrentSavePoint);
		}
		else
		{
			wxGetApp().Log("Can't go backwards at start of save points\n");
		}


	}

	void SavePoint::LoadSavePoint(size_t nPos)
	{
		if ((nPos < m_SavePoints.GetCount()))
		{
			wxGetApp().Log("Loading Save point: %u\n", nPos);

			wxString sSavePointName = m_SavePoints[nPos];
			wxGetApp().LoadSnapshot(sSavePointName);
		}
	}

	void arnguiApp::ReadPositionalKeyboardConfiguration()
	{
		wxString sConfigPrefix = wxT("keyboard/positional/Custom/");

		m_PlatformSpecific.ClearHostKeys();

		for (int j = 0; j < CPC_KEY_NUM_KEYS; j++)
		{
			if ((j >= CPC_KEY_JOY_UP) && (j < CPC_KEY_DEL))
				continue;

			int HostKeys[MaxNumKeys];
			int nHostKeysSet = 0;
			wxString sKeyID;
			sKeyID.Printf(wxT("CPCKey%d/"), j);

			for (size_t i = 0; i < MaxNumKeys; i++)
			{
				HostKeys[i] = CPC_KEY_DEF_UNSET_KEY;
				wxString sSubKey;
				sSubKey.Printf(wxT("HostKey%d"), (int)i);
				int nValue;
				if (wxConfig::Get(false)->Read(sConfigPrefix + sKeyID + sSubKey, &nValue, (int)CPC_KEY_DEF_UNSET_KEY))
				{
					HostKeys[i] = nValue;
					nHostKeysSet++;
				}
			}

			if (nHostKeysSet == MaxNumKeys)
			{
				m_PlatformSpecific.SetHostKey((CPC_KEY_ID)j, HostKeys);
			}
		}
	}

	void arnguiApp::WritePositionalKeyboardConfiguration()
	{
		wxString sConfigPrefix = wxT("keyboard/positional/Custom/");
		wxConfig::Get(false)->Write(sConfigPrefix + wxT("name"), wxT("custom"));

		for (int j = 0; j < CPC_KEY_NUM_KEYS; j++)
		{
			if ((j >= CPC_KEY_JOY_UP) && (j < CPC_KEY_DEL))
				continue;

			int HostKeys[MaxNumKeys];
			m_PlatformSpecific.GetHostKey((CPC_KEY_ID)j, HostKeys);

			wxString sKeyID;
			sKeyID.Printf(wxT("CPCKey%d/"), j);

			for (size_t i = 0; i < MaxNumKeys; i++)
			{
				wxString sSubKey;
				sSubKey.Printf(wxT("HostKey%d"), i);
				wxConfig::Get(false)->Write(sConfigPrefix + sKeyID + sSubKey, HostKeys[i]);
			}
		}
	}

#if ((defined(GTK2_EMBED_WINDOW) && defined(__WXGTK__)) || (defined(MAC_EMBED_WINDOW) && defined(__WXMAC__)))

	//=====================================================================
	// Key To int part / KeyStick
	//=====================================================================

	static int KeySetCursors[JOYSTICK_SIMULATED_KEYID_LAST]=
	{
		WXK_UP,
		WXK_DOWN,
		WXK_LEFT,
		WXK_RIGHT,
		WXK_SPACE,
		WXK_SHIFT,
		WXK_CONTROL,
		0,
		0,
		0,
		0
	};

	static int KeySetWASD[JOYSTICK_SIMULATED_KEYID_LAST] =
	{
		(wxChar)'W',
		(wxChar)'S',
		(wxChar)'A',
		(wxChar)'D',
		WXK_SPACE,
		WXK_SHIFT,
		WXK_CONTROL,
		0,
		0,
		0,
		0
	};


	static int KeySetNumPad[JOYSTICK_SIMULATED_KEYID_LAST]=
	{
		WXK_NUMPAD8,
		WXK_NUMPAD2,
		WXK_NUMPAD4,
		WXK_NUMPAD6,
		WXK_NUMPAD5,
		WXK_NUMPAD0,
		WXK_NUMPAD_DECIMAL,
		WXK_NUMPAD7,
		WXK_NUMPAD9,
		WXK_NUMPAD1,
		WXK_NUMPAD3
	};


	static int KeySetHome[ JOYSTICK_SIMULATED_KEYID_LAST ] =
	{
		WXK_PAGEDOWN,
		WXK_PAGEUP,
		WXK_HOME,
		WXK_END,
		WXK_DELETE,
		WXK_INSERT,
		WXK_CONTROL,
		0,
		0,
		0,
		0
	};

	static int KeySetCustom[ JOYSTICK_SIMULATED_KEYID_LAST ] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


	int *arnguiApp::GetKeySetPtr(int nKeySet)
	{
		if (nKeySet == JOYSTICK_KEYSET_CURSORS) return KeySetCursors;
		else if (nKeySet == JOYSTICK_KEYSET_WASD) return KeySetWASD;
		else if (nKeySet == JOYSTICK_KEYSET_NUMPAD) return KeySetNumPad;
		else  if (nKeySet == JOYSTICK_KEYSET_INSERT_HOME) return KeySetHome;
		else  if (nKeySet == JOYSTICK_KEYSET_CUSTOM) return KeySetCustom;
		return NULL;
	}
#endif

	void arnguiApp::SetColumnSize(wxListCtrl *pListCtrl, int nColumn)
	{
		/* from wxwidgets forum */
		pListCtrl->SetColumnWidth(nColumn, wxLIST_AUTOSIZE_USEHEADER);
		int nWidthHeader = pListCtrl->GetColumnWidth(nColumn);
		pListCtrl->SetColumnWidth(nColumn, wxLIST_AUTOSIZE);
		int nWidthColumn = pListCtrl->GetColumnWidth(nColumn);
		if (nWidthHeader > nWidthColumn)
		{
			pListCtrl->SetColumnWidth(nColumn, wxLIST_AUTOSIZE_USEHEADER);
		}
	}

#include "ArchiveDialog.h"

void arnguiApp::HandleDropFile(wxString sFile)
{
	// addition from Aeliss, drag and drop of archives

	// dropped an archive?
	if (ArchiveDialog::IsArchive(sFile))
	{
		// pick from the archive
		wxString sPickedFilename;

		if (ArchiveDialog::DoPicking(wxGetApp().GetTopWindow(), sFile, wxEmptyString, sPickedFilename))
		{
			if (sPickedFilename.IsEmpty())
			{
				// user backed out of archive
				return;
			}

			// use the picked filename
			sFile = sPickedFilename;
		}
		else
		{
			// user cancelled archive..
			return;
		}
	}

	wxGetApp().InsertOrAutoStartMedia(sFile, false);
}

void arnguiApp::RefreshSingleItemState(wxMenuBar *pMenuBar, long id, BOOL bState)
{
	wxMenuItem *pMenuItem = pMenuBar->FindItem(id);
	if (pMenuItem != NULL)
	{
		// ensure it's selected
		pMenuItem->Enable(true);

		bool bChecked = bState ? true : false;

		// if the value is the chosen one, check it else clear the check
		pMenuItem->Check(bChecked);
	}
}

