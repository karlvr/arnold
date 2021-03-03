/***************************************************************
 * Name:      arnguiApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Kevin Thacker (kev@arnoldemu.freeserve.co.uk)
 * Created:   2010-11-08
 * Copyright: Kevin Thacker (arnold.cpc-live.com)
 * License:
 **************************************************************/


//#ifdef WX_PRECOMP
//#include "wx_pch.h"
//#endif

//#ifdef __BORLANDC__
//#pragma hdrstop
//#endif //__BORLANDC__

#include "arnguiApp.h"
#include "arnguiMain.h"
#include "EmuWindow.h"
#include "LoadBinaryDialog.h"
#include "DebuggerDialog.h"
#include "GraphicsEditor.h"

IMPLEMENT_APP(arnguiApp)

//#ifdef __WXMSW__
//#include <wx/msw/private.h>
//#endif
#include <wx/xrc/xmlres.h>
#include <wx/stdpaths.h>
#include <wx/choicdlg.h>
#include <wx/ipc.h>
#include <wx/listctrl.h>
#include <wx/wfstream.h>
#include <locale.h>
#include <wx/tarstrm.h>
//#include "roms.cpp"
#include <wx/filesys.h>
#include <wx/fs_zip.h>
#include <wx/fs_arc.h>
#include <wx/fs_filter.h>
#include <wx/splash.h>
#include <wx/dir.h>
#include <wx/fileconf.h>
#include <wx/msgout.h>
#include <wx/tokenzr.h>


//#inclide <wx/dir.h>
#ifdef WIN32
#include <tchar.h>
#endif

#ifndef min
#define min(a,b) ((a)<(b)) ? (a) : (b)
#endif

extern "C"
{
//#ifdef __WXMSW__
//   #include "./directx/graphlib.h"
    //  #include "./directx/dd.h"
    //  #include "./directx/di.h"
//   #include "./directx/ds.h"
//#endif

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
#include "../cpc/audioevent.h"
#include "../cpc/sdlsound.h"

#include "../cpc/tzx.h"
#include "../cpc/autotype.h"
#include "../cpc/autorunfile.h"
#include "../cpc/labelset.h"
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

extern void DiscWizard_Init();
//extern void HackIt_Init();
extern void Symbiface_Init();

extern void TransTape_Init();
}

//for Visual Leak Detector
#include <vld.h>

//#include <wx/arrimpl.cpp>
//WX_DEFINE_OBJARRAY(ArrayOfarnguiConfig)

// for older wx2.8 releases including first OS release on raspberry pi
#ifndef wxT_2
#define wxT_2(x) wxT(x)
#endif

int arnguiApp::ExpressionEvaluate(const wxString &sValue)
{
#if (wxVERSION_NUMBER >= 2900)
    // 2.9
    const wxCharBuffer tmp_buf = wxConvCurrent->cWX2MB(sValue.wc_str());
#else
    // 2.8
    const wxWX2MBbuf tmp_buf = wxConvCurrent->cWX2MB(sValue);
#endif
    const char *pString = (const char*) tmp_buf;

    int nResult;
    EvaluateExpression(pString,&nResult);
    return nResult;
}


void TraceMessage(const wxString &sMessage)
{
//wxMessageOutputDebug().Printf(wxT("%s"),sMessage.GetData());
    wxMessageOutput::Get()->Printf(wxT("%s"),sMessage.GetData());
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

    /* some built in roms */
    ROM_ID_PARADOS,
    ROM_ID_QCMD,

    // keep last
    ROM_ID_LAST
};

struct RomDetails
{
    //  int nID;
    /* a tag used to identify rom e.g. within UI */
    wxString sTag;
    /* filename of rom to load */
    //  wxString sFilename;
    /* address to hold data */
    unsigned char *pData;
    /* length of data */
    unsigned long nLength;
};

static RomDetails Roms[ROM_ID_LAST-ROM_ID_NULL];


int arnguiApp::GetNumBuiltinRoms()
{

    return sizeof(Roms)/sizeof(Roms[0]);
}

wxString arnguiApp::GetBuiltinRomName(int Index)
{
    return Roms[Index].sTag;
}

void arnguiApp::GetBuiltinRom(int Index, unsigned char **ppData, unsigned long *ppLength)
{
    *ppData = Roms[Index].pData;
    *ppLength = Roms[Index].nLength;
}
struct RomLoad
{
    int nID;
    wxString sFilename;
};

RomLoad RomLoadList[]=
{
    // cpc464
    {
        ROM_ID_CPC464E_OS,
        wxT("roms/cpc464e/os.rom")
    },
    {
        ROM_ID_CPC464E_BASIC,
        wxT("roms/cpc464e/basic.rom")
    },
    {
        ROM_ID_CPC464D_OS,
        wxT("roms/cpc464d/os.rom")
    },
    {
        ROM_ID_CPC464D_BASIC,
        wxT("roms/cpc464d/basic.rom")
    },
    {
        ROM_ID_CPC464F_OS,
        wxT("roms/cpc464f/os.bin")
    },
    {
        ROM_ID_CPC464F_BASIC,
        wxT("roms/cpc464f/basic.bin")
    },
    {
        ROM_ID_CPC464S_OS,
        wxT("roms/cpc464s/os.rom")
    },
    // cpc664
    {
        ROM_ID_CPC664E_OS,
        wxT( "roms/cpc664e/os.rom")
    },
    {
        ROM_ID_CPC664E_BASIC,
        wxT("roms/cpc664e/basic.rom")
    },
    // cpc6128
    {
        ROM_ID_CPC6128E_OS,
        wxT("roms/cpc6128e/os.rom")
    },
    {
        ROM_ID_CPC6128E_BASIC,
        wxT("roms/cpc6128e/basic.rom")
    },
    {
        ROM_ID_CPC6128F_OS,
        wxT("roms/cpc6128f/os.rom")
    },
    {
        ROM_ID_CPC6128F_BASIC,
        wxT("roms/cpc6128f/basic.rom")
    },
    {
        ROM_ID_CPC6128S_OS,
        wxT("roms/cpc6128s/os.rom")
    },
    {
        ROM_ID_CPC6128S_BASIC,
        wxT("roms/cpc6128s/basic.rom")
    },
    {
        ROM_ID_CPC6128W_OS,
        wxT("roms/cpc6128w/os.rom")
    },
    // amsdos
    {
        ROM_ID_AMSDOSE,
        wxT( "roms/amsdose/amsdos.rom")
    },
    {
        ROM_ID_SYSTEM_CART_EN,
        wxT("roms/cpcplus/system_en.cpr")
    },
    {
        ROM_ID_SYSTEM_CART_FR,
        wxT("roms/cpcplus/system_fr.cpr")
    },
    {
        ROM_ID_SYSTEM_CART_FR2,
        wxT("roms/cpcplus/system_fr2.cpr")
    },
    {
        ROM_ID_SYSTEM_CART_ES,
        wxT("roms/cpcplus/system_es.cpr")
    },
    // csd
    {
        ROM_ID_SYSTEM_CART_CSD,
        wxT("roms/cpcplus/system_csd.cpr")
    },
    // kcc
    {
        ROM_ID_KCC_BASIC,
        wxT("roms/kcc/kccbas.rom")
    },
    {
        ROM_ID_KCC_COLOUR,
        wxT("roms/kcc/FARBEN.ROM")
    },
    {
        ROM_ID_KCC_OS,
        wxT("roms/kcc/kccos.rom")
    },
    // aleste 520ex
    {
        ROM_ID_ALESTE_AF,
        wxT("roms/aleste/AF.BIN")
    },
    {
        ROM_ID_ALESTE_AL512,
        wxT("roms/aleste/AL512.BIN")
    },
    {
        ROM_ID_ALESTE_MAPPER,
        wxT("roms/aleste/MAPPER.BIN")
    },
    {
        ROM_ID_ALESTE_RFCOLDAT,
        wxT("roms/aleste/RFCOLDAT.BIN")
    },
    {
        ROM_ID_ALESTE_RFVDKEY,
        wxT("roms/aleste/RFVDKEY.BIN")
    },
    {
        ROM_ID_ALESTE_ROMRAM,
        wxT("roms/aleste/ROMRAM.BIN")
    },
    {
        ROM_ID_PARADOS,
        wxT("roms/parados.rom")
    },
       {
        ROM_ID_QCMD,
        wxT("roms/qcmd.rom")
    },
};

void GetRomByID(int nID, unsigned char **ppPtr, unsigned long *pLength)
{
    *ppPtr = Roms[nID].pData;
    *pLength = Roms[nID].nLength;
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
        GetRomByID(ROM_ID_SYSTEM_CART_EN, &pCart, &nCartLength);
    }
    break;

    case 1:
    {
        GetRomByID(ROM_ID_SYSTEM_CART_FR, &pCart, &nCartLength);
    }
    break;

    case 2:
    {
        GetRomByID(ROM_ID_SYSTEM_CART_FR2, &pCart, &nCartLength);
    }
    break;

    case 3:
    {
        GetRomByID(ROM_ID_SYSTEM_CART_ES, &pCart, &nCartLength);
    }
    break;
    }

    Cartridge_SetDefault(pCart, nCartLength);
}


void Config_CPC464EN()
{
    unsigned char *pOS;
    unsigned char *pBasic;
    unsigned long RomLength;

    GetRomByID(ROM_ID_CPC464E_OS, &pOS, &RomLength);
    GetRomByID(ROM_ID_CPC464E_BASIC, &pBasic, &RomLength);

    CPC_SetExpLow(FALSE);
    CPC_SetOSRom(pOS);
    CPC_SetBASICRom(pBasic);
    CPC_SetHardware(CPC_HW_CPC);
}


void Config_CPC464ES()
{
    unsigned char *pOS;
    unsigned char *pBasic;
    unsigned long RomLength;

    GetRomByID(ROM_ID_CPC464S_OS, &pOS, &RomLength);
    GetRomByID(ROM_ID_CPC464E_BASIC, &pBasic, &RomLength);

    CPC_SetExpLow(FALSE);
    CPC_SetOSRom(pOS);
    CPC_SetBASICRom(pBasic);
    CPC_SetHardware(CPC_HW_CPC);
}

void Config_CPC464FR()
{
    unsigned char *pOS;
    unsigned char *pBasic;
    unsigned long RomLength;

    GetRomByID(ROM_ID_CPC464F_OS, &pOS, &RomLength);
    GetRomByID(ROM_ID_CPC464F_BASIC, &pBasic, &RomLength);

    CPC_SetExpLow(FALSE);
    CPC_SetOSRom(pOS);
    CPC_SetBASICRom(pBasic);
    CPC_SetHardware(CPC_HW_CPC);
}


void Config_CPC464DK()
{
    unsigned char *pOS;
    unsigned char *pBasic;
    unsigned long RomLength;

    GetRomByID(ROM_ID_CPC464D_OS, &pOS, &RomLength);
    GetRomByID(ROM_ID_CPC464D_BASIC, &pBasic, &RomLength);

    CPC_SetExpLow(FALSE);
    CPC_SetOSRom(pOS);
    CPC_SetBASICRom(pBasic);
    CPC_SetHardware(CPC_HW_CPC);
}

void Config_CPC664EN()
{
    unsigned char *pOS;
    unsigned char *pBasic;
    unsigned char *pAmsdos;
    unsigned long RomLength;

    GetRomByID(ROM_ID_CPC664E_OS, &pOS, &RomLength);
    GetRomByID(ROM_ID_CPC664E_BASIC, &pBasic, &RomLength);
    GetRomByID(ROM_ID_AMSDOSE, &pAmsdos, &RomLength);

    CPC_SetExpLow(TRUE);
    CPC_SetOSRom(pOS);
    CPC_SetBASICRom(pBasic);
    Amstrad_DiscInterface_SetRom(pAmsdos);
    Amstrad_DiscInterface_Install();

    CPC_SetHardware(CPC_HW_CPC);
}

void Config_464Plus()
{
    ASIC_SetGX4000(FALSE);
    CPC_SetExpLow(TRUE);
    CPC_SetHardware(CPC_HW_CPCPLUS);

    /* 128k ram */
    ASIC_SetR128(FALSE);
    /* disc interface */
    ASIC_SetR129(FALSE);

    Cartridge_InsertDefault();
   
}

void arnguiApp::InsertMedia(const wxString &sFilename)
{
    unsigned char *pLocation;
    unsigned long Length;

    LoadLocalFile(sFilename, &pLocation, &Length);

    if (pLocation==NULL)
        return;

    if (Cartridge_AttemptInsert(pLocation, Length)==ARNOLD_STATUS_OK)
    {
        free(pLocation);
        return;
    }

    if (Snapshot_Insert(pLocation, Length)==ARNOLD_STATUS_OK)
    {
        free(pLocation);
        return;
    }

    if (TapeImage_Insert(pLocation, Length)==ARNOLD_STATUS_OK)
    {
        free(pLocation);
        return;
    }

    if (DiskImage_Recognised(pLocation, Length)==ARNOLD_STATUS_OK)
    {
        // choose which disk!
        wxArrayString sDrives;
        for (int i=0; i<4; i++)
        {
            wxString sDrive;
            sDrive.Printf(wxT("Drive %c"),'A'+i);
            sDrives.Add(sDrive);
        }

        wxString sMessage;
        sMessage = wxT("Insert disk image file:\n\"")+sFilename+wxT("\"\ninto which drive?\n");
        wxSingleChoiceDialog *pDialog = new wxSingleChoiceDialog(GetTopWindow(), sMessage, wxT("Choose drive"), sDrives);
        pDialog->SetSelection(0);
        if (pDialog->ShowModal()==wxID_OK)
        {
            int nDrive = pDialog->GetSelection();

            m_Drives[nDrive]->LoadWithSaveModified(sFilename);
        }
        pDialog->Destroy();	//pDialog->Destroy();
    }

    free(pLocation);
}



void arnguiApp::AutoStartMedia(const wxString &sFilename)
{
    unsigned char *pFileData;
    unsigned long FileDataSize;

    /* turn off autotype */
    AutoType_Finish();

    /* clean up previous auto run */
    AutoRunFile_Finish();

    /* try to load it */
    LoadLocalFile(sFilename, &pFileData, &FileDataSize);

    if (pFileData!=NULL)
    {
        int nStatus;

        // try cartridge...
        if (Cartridge_AttemptInsert(pFileData, FileDataSize)==ARNOLD_STATUS_OK)
        {
            free(pFileData);
            return;
        }

        // try snapshot
        if (Snapshot_Insert(pFileData, FileDataSize)==ARNOLD_STATUS_OK)
        {
            free(pFileData);
            return;
        }

        /* boot from zero, unless drive switch is in action */
        int nAutoStartDrive = FDI_GetDriveToAccess(0);
        int nAutoStartSide = FDI_GetSideToAccess(nAutoStartDrive,0);

        /* try disk */
        nStatus = DiskImage_InsertDisk(nAutoStartDrive, pFileData, FileDataSize);
        if (nStatus==ARNOLD_STATUS_OK)
        {
            unsigned char *pBuffer;

            /* autostart? */
            /* this may not work on large directories like
            romdos etc */
            pBuffer = (unsigned char *)malloc(512*5);
            if (pBuffer)
            {
                const char *pAutoTypeString;

                /* try auto-start */
                int nAutoRunResult = AMSDOS_GenerateAutorunCommand(nAutoStartDrive, nAutoStartSide, pBuffer,&pAutoTypeString);

                if (nAutoRunResult == AUTORUN_OK)
                {
                    AutoType_SetString(pAutoTypeString, FALSE,TRUE, TRUE);
                }
				else if (nAutoRunResult==AUTORUN_TOO_MANY_POSSIBILITIES)
				{
            printf("Unable to auto-run there were too many possibilities\n");
					//    MessageBox(GetDesktopWindow(), _T("Too many files qualify for auto-run. Unable to auto-start this disc"),_T("Arnold"), MB_OK);
				}
				else if (nAutoRunResult==AUTORUN_NOT_POSSIBLE)
				{
            printf("Unable to auto-start this disc\n");
					//   MessageBox(GetDesktopWindow(), _T("Unable to auto-start this disc."),_T("Arnold"), MB_OK);
				}
				else if (nAutoRunResult==AUTORUN_NO_FILES_QUALIFY)
				{
            printf("Can't find any files to auto-run. Unable to auto-start this disc\n");
					//    MessageBox(GetDesktopWindow(), _T("Can't find any files to auto-run. Unable to auto-start this disc"),_T("Arnold"), MB_OK);
				}
				else
				{
            printf("Unable to auto-start this disc\n");
					//    MessageBox(GetDesktopWindow(), _T("Unable to auto-start this disc"),_T("Arnold"), MB_OK);
				}

                free(pBuffer);
            }

            free(pFileData);

            return;
        }

        // try tape image
        if (TapeImage_Insert(pFileData, FileDataSize)==ARNOLD_STATUS_OK)
        {
            AutoType_SetString(sAutoStringTape, FALSE,TRUE, TRUE);

            free(pFileData);

            return;

        }

        // try sample
        /*if (Sample_Load(pOpenFilename))
        {
            AutoType_SetString(sAutoStringTape, FALSE,TRUE, TRUE);

            free(pFileData);
            return;
        }
        */


        {
            bool bLoading = false;
            FILE_HEADER FileHeader;

            GetHeaderDataFromBuffer((const char *)pFileData, FileDataSize, &FileHeader);

            if (!FileHeader.bHasHeader)
            {
                wxMessageBox(wxT("Unable to auto-start file because it doesn't have a header"),
                             wxGetApp().GetAppName(), wxICON_INFORMATION|wxOK);


                FileHeader.HeaderStartAddress = 0x0000;
                FileHeader.HeaderLength = FileDataSize;
                FileHeader.HeaderExecutionAddress = FileHeader.HeaderStartAddress;
                // suggest binary
                FileHeader.HeaderFileType = 2;
            }

            // show this if SHIFT is pressed?
            LoadBinaryDialog *pDialog = new LoadBinaryDialog(GetTopWindow(), &FileHeader);
            if (pDialog!=NULL)
            {
                if (pDialog->ShowModal()==wxID_OK)
                {
                    if (pDialog->m_bSetBreakpoint)
                    {
                        if (!Breakpoints_IsAVisibleBreakpoint(BREAKPOINT_TYPE_PC, pDialog->m_BreakpointAddress))
                        {
                            Breakpoints_AddBreakpoint(BREAKPOINT_TYPE_PC, pDialog->m_BreakpointAddress);
                        }
                    }

                    unsigned long Length;
                    FileHeader.MemoryStart = FileHeader.HeaderStartAddress;
                    Length = min(FileHeader.HeaderLength, FileDataSize);
                    FileHeader.MemoryEnd = FileHeader.MemoryStart+Length;

                    /* reload system cartridge if running in cpc+ mode */
                    if (CPC_GetHardware()==CPC_HW_CPCPLUS)
                    {
                        //CPC_ReloadSystemCartridge();

                        // need to ensure the default is a system one!
                        Cartridge_InsertDefault();

                        Cartridge_Autostart();

                    }

                    AutoRunFile_SetData((const char *)pFileData, FileDataSize, TRUE, TRUE, &FileHeader, TRUE);
                    bLoading = true;
                }
                pDialog->Destroy();	//pDialog->Destroy();
            }

            if (!bLoading)
            {
                free(pFileData);
            }
        }
    }
}

void Config_GX4000()
{
    ASIC_SetGX4000(TRUE);
    CPC_SetExpLow(FALSE);
    CPC_SetHardware(CPC_HW_CPCPLUS);
    Cartridge_InsertDefault();
//    CPC_InsertSystemCartridge();

}


void Config_CSD()
{
    CPC_SetHardware(CPC_HW_CPCPLUS);
    Jukebox_Enable(TRUE);

}

void Config_6128Plus()
{
    ASIC_SetGX4000(FALSE);
    CPC_SetExpLow(TRUE);
    CPC_SetHardware(CPC_HW_CPCPLUS);
    Cartridge_InsertDefault();
    /* 128k ram */
    ASIC_SetR128(TRUE);
    /* disc interface */
    ASIC_SetR129(TRUE);
}

void Config_CPC6128EN()
{
    unsigned char *pOS;
    unsigned char *pBasic;
    unsigned char *pAmsdos;
    unsigned long RomLength;

    GetRomByID(ROM_ID_CPC6128E_OS, &pOS, &RomLength);
    GetRomByID(ROM_ID_CPC6128E_BASIC, &pBasic, &RomLength);
    GetRomByID(ROM_ID_AMSDOSE, &pAmsdos, &RomLength);

    CPC_SetExpLow(TRUE);
    CPC_SetOSRom(pOS);
    CPC_SetBASICRom(pBasic);
    Amstrad_DiscInterface_SetRom(pAmsdos);
    Amstrad_DiscInterface_Install();
    //Amstrad_RAM_Install();
    PAL_Install();
    CPC_SetHardware(CPC_HW_CPC);
}

void Config_CPC6128ENParados()
{
    unsigned char *pOS;
    unsigned char *pBasic;
    unsigned char *pParados;
    unsigned long RomLength;

    GetRomByID(ROM_ID_CPC6128E_OS, &pOS, &RomLength);
    GetRomByID(ROM_ID_CPC6128E_BASIC, &pBasic, &RomLength);
    GetRomByID(ROM_ID_PARADOS, &pParados, &RomLength);

    CPC_SetExpLow(TRUE);
    CPC_SetOSRom(pOS);
    CPC_SetBASICRom(pBasic);

    Amstrad_DiscInterface_SetRom(pParados);
    Amstrad_DiscInterface_Install();
    PAL_Install();
    CPC_SetHardware(CPC_HW_CPC);
}


void Config_CPC6128ES()
{
    unsigned char *pOS;
    unsigned char *pBasic;
    unsigned char *pAmsdos;
    unsigned long RomLength;

    GetRomByID(ROM_ID_CPC6128S_OS, &pOS, &RomLength);
    GetRomByID(ROM_ID_CPC6128S_BASIC, &pBasic, &RomLength);
    GetRomByID(ROM_ID_AMSDOSE, &pAmsdos, &RomLength);

    CPC_SetExpLow(TRUE);
    CPC_SetOSRom(pOS);
    CPC_SetBASICRom(pBasic);


    Amstrad_DiscInterface_SetRom(pAmsdos);
    Amstrad_DiscInterface_Install();
    PAL_Install();
    CPC_SetHardware(CPC_HW_CPC);
}


void Config_CPC6128W()
{
    unsigned char *pOS;
    unsigned char *pBasic;
    unsigned char *pAmsdos;
    unsigned long RomLength;

    GetRomByID(ROM_ID_CPC6128W_OS, &pOS, &RomLength);
    GetRomByID(ROM_ID_CPC6128E_BASIC, &pBasic, &RomLength);
    GetRomByID(ROM_ID_AMSDOSE, &pAmsdos, &RomLength);

    CPC_SetExpLow(TRUE);
    CPC_SetOSRom(pOS);
    CPC_SetBASICRom(pBasic);


    Amstrad_DiscInterface_SetRom(pAmsdos);
    Amstrad_DiscInterface_Install();
    PAL_Install();
    CPC_SetHardware(CPC_HW_CPC);
}

void Config_CPC6128FR()
{
    unsigned char *pOS;
    unsigned char *pBasic;
    unsigned char *pAmsdos;
    unsigned long RomLength;

    GetRomByID(ROM_ID_CPC6128F_OS, &pOS, &RomLength);
    GetRomByID(ROM_ID_CPC6128F_BASIC, &pBasic, &RomLength);
    GetRomByID(ROM_ID_AMSDOSE, &pAmsdos, &RomLength);

    CPC_SetExpLow(TRUE);
    CPC_SetOSRom(pOS);
    CPC_SetBASICRom(pBasic);


    Amstrad_DiscInterface_SetRom(pAmsdos);
    Amstrad_DiscInterface_Install();
    PAL_Install();
    CPC_SetHardware(CPC_HW_CPC);
}

void Config_KCC()
{
    unsigned char *pOS;
    unsigned char *pBasic;
    unsigned char *pColour;
    unsigned long RomLength;

    GetRomByID(ROM_ID_KCC_OS, &pOS, &RomLength);
    GetRomByID(ROM_ID_KCC_BASIC, &pBasic, &RomLength);
    GetRomByID(ROM_ID_KCC_COLOUR, &pColour, &RomLength);

    KCC_SetOSRom(pOS);
    KCC_SetBASICRom(pBasic);
    KCC_SetColorROM(pColour);

    CPC_SetHardware(CPC_HW_KCCOMPACT);
}


void Config_KCCWithDDI1()
{
    unsigned char *pOS;
    unsigned char *pBasic;
    unsigned char *pColour;
    unsigned char *pAmsdos;
    unsigned long RomLength;

    GetRomByID(ROM_ID_KCC_OS, &pOS, &RomLength);
    GetRomByID(ROM_ID_KCC_BASIC, &pBasic, &RomLength);
    GetRomByID(ROM_ID_KCC_COLOUR, &pColour, &RomLength);
    GetRomByID(ROM_ID_AMSDOSE, &pAmsdos, &RomLength);



    KCC_SetOSRom(pOS);
    KCC_SetBASICRom(pBasic);
    KCC_SetColorROM(pColour);

    Amstrad_DiscInterface_SetRom(pAmsdos);
    Amstrad_DiscInterface_Install();


    CPC_SetHardware(CPC_HW_KCCOMPACT);
}

void Config_Aleste()
{
    unsigned char *pMapper;
    unsigned char *pRFColCat;
    unsigned char *pROMRam;
    unsigned char *pAL512;
    unsigned long RomLength;

    GetRomByID(ROM_ID_ALESTE_MAPPER, &pMapper, &RomLength);
    GetRomByID(ROM_ID_ALESTE_RFCOLDAT, &pRFColCat, &RomLength);
    GetRomByID(ROM_ID_ALESTE_ROMRAM, &pROMRam, &RomLength);
    GetRomByID(ROM_ID_ALESTE_AL512, &pAL512, &RomLength);

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
    printf("Chosen config: %s\r\n", buf.data());


    Amstrad_DiscInterface_Uninstall();
    Amstrad_RAM_Uninstall();
    Jukebox_Enable(FALSE);

  m_sConfig = sName;

  if (sName.CmpNoCase(wxT("cpc464en"))==0)
  {
    Config_CPC464EN();
    }
  else
  if (sName.CmpNoCase(wxT("cpc464es"))==0)
  {
    Config_CPC464ES();
    }
  else
  if (sName.CmpNoCase(wxT("cpc464fr"))==0)
  {
        Config_CPC464FR();
    }
    else
      if (sName.CmpNoCase(wxT("cpc464dk"))==0)
      {
        Config_CPC464DK();
    }
  else if (sName.CmpNoCase(wxT("cpc664en"))==0)
  {
        Config_CPC664EN();
    }
    else if (sName.CmpNoCase(wxT("cpc6128en"))==0)
    {
        Config_CPC6128EN();
    }
  else if (sName.CmpNoCase(wxT("cpc6128fr"))==0)
  {
        Config_CPC6128FR();
    }
  else if (sName.CmpNoCase(wxT("cpc6128es"))==0)
    {
        Config_CPC6128ES();
    }
    else if (sName.CmpNoCase(wxT("464plus"))==0)
    {
        Config_464Plus();
    }
    else if (sName.CmpNoCase(wxT("6128plus"))==0)
    {
        Config_6128Plus();
    }
    else if (sName.CmpNoCase(wxT("gx4000"))==0)
    {

        Config_GX4000();
    }
    else if (sName.CmpNoCase(wxT("kccompact"))==0)
    {

        Config_KCC();
    }
    else if (sName.CmpNoCase(wxT("aleste"))==0)
    {

        Config_Aleste();
    }
    else if (sName.CmpNoCase(wxT("csd"))==0)
    {

        Config_CSD();
    }
    else if (sName.CmpNoCase(wxT("6128enparados"))==0)
    {

        Config_CPC6128ENParados();
    }
    else if (sName.CmpNoCase(wxT("cpc6128w"))==0)
    {
        Config_CPC6128W();
    }
    else
    {
        Config_CPC6128EN();
    }
    printf("Done Choose config\n");

}

wxConnectionBase *stServer::OnAcceptConnection(const wxString& topic)
{
    if (topic.Lower() == wxT("arnold"))
    {
// Check that there are no modal dialogs active
        wxWindowList::compatibility_iterator node = wxTopLevelWindows.GetFirst();
        while (node)
        {
            wxDialog* dialog = wxDynamicCast(node->GetData(), wxDialog);
            if (dialog && dialog->IsModal())
            {
                return NULL;
            }
            node = node->GetNext();
        }
        return new stConnection();
    }
    else
        return NULL;
}

// Opens a file passed from another instance
bool stConnection::OnExecute(const wxString& WXUNUSED(topic),
                             wxChar *data,
                             int WXUNUSED(size),
                             wxIPCFormat WXUNUSED(format))
{
    wxString filename(data);

#if 0

    stMainFrame* frame = wxDynamicCast(wxGetApp().GetTopWindow(),
                                       stMainFrame);
    wxString filename(data);
    if (filename.IsEmpty())
    {
// Just raise the main window
        if (frame)
            frame->Raise();
    }
    else
    {
// Check if the filename is already open,
// and raise that instead.
        wxNode* node = wxGetApp().GetDocManager()-
                       >GetDocuments().GetFirst();
        while (node)
        {
            MyDocument* doc = wxDynamicCast(node->GetData(),MyDocument);
            if (doc && doc->GetFilename() == filename)
            {
                if (doc->GetFrame())
                    doc->GetFrame()->Raise();
                return true;
            }
            node = node->GetNext();
        }
        wxGetApp().GetDocManager()->CreateDocument(
            filename, wxDOC_SILENT);
    }
#endif
    return true;
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


void arnguiApp::ProcessAction(ACTION_CODE nAction)
{
	if (frame)
		frame->ProcessAction(nAction);
}


void arnguiApp::SetTranslatedKey(int ch)
{
	AutoType_KeyPress(ch);
}


int arnguiApp::OnExit()
{
  // set old one and delete our one
  delete wxMessageOutput::Set(m_OldMessageOutput);

  m_ComputerConfigs.Empty();
  m_KeyboardConfigs.Empty();
  m_JoystickConfigs.Empty();

    ClosePrinterFile();
  //  if (pBrunwordMK2_ROM)
   // {
    //    free(pBrunwordMK2_ROM);
    //}
    //if (pBrunwordMK4_ROM)
   // {
    //    free(pBrunwordMK4_ROM);
   // }

    YMOutput_Finish();
//	frame->Destroy();
    // delete the instance checker
    delete m_checker;
    delete m_server;
//    delete frame;

    for (unsigned int i=0; i!=m_Media.GetCount(); i++)
    {
        m_Media[i]->Unload();
      delete m_Media[i];
      m_Media[i] = NULL;
    }
    m_Media.Clear();


    // free roms
    for (int i=ROM_ID_NULL+1; i<ROM_ID_LAST; i++)
    {
        Roms[i].nLength = 0;
        if (Roms[i].pData!=NULL)
        {
            free(Roms[i].pData);
        }
    }
  wxXmlResource::Get()->ClearHandlers();
    wxXmlResource::Get()->Unload(GetGUIFullPath());
    // try to deallocate it.
    wxXmlResource::Set(NULL);

    labelsets_free();

    Multiface_Finish();
    /* close CPC emulation */
    CPC_Finish();

	sdl_close_audio();

    AutoType_Finish();
    AutoRunFile_Finish();

    WinapePokeDatabase_Free();

    if (m_pConfig)
    {
         wxConfigBase::Set(NULL);

        delete m_pConfig;
    }
    
    m_PlatformSpecific.Shutdown();
    
    return wxApp::OnExit();

}

const wxCmdLineEntryDesc arnguiApp::m_CommandLineDesc[] =
{

    { wxCMD_LINE_OPTION, wxT_2("t"), wxT_2("tape"),  wxT_2("specify tape file"), wxCMD_LINE_VAL_STRING },
    { wxCMD_LINE_OPTION, wxT_2("d"), wxT_2("disk"),   wxT_2("specify disc for any drive"), wxCMD_LINE_VAL_STRING },
    { wxCMD_LINE_OPTION, wxT_2("a"), wxT_2("drivea"),   wxT_2("specify disc for drive A"), wxCMD_LINE_VAL_STRING },
    { wxCMD_LINE_OPTION, wxT_2("b"), wxT_2("driveb"),   wxT_2("specify disc for drive B"), wxCMD_LINE_VAL_STRING },
    { wxCMD_LINE_OPTION, wxT_2("da"), wxT_2("diska"),   wxT_2("specify disc for drive A"), wxCMD_LINE_VAL_STRING },
    { wxCMD_LINE_OPTION, wxT_2("db"), wxT_2("diskb"),   wxT_2("specify disc for drive B"), wxCMD_LINE_VAL_STRING },
    { wxCMD_LINE_OPTION, wxT_2("dc"), wxT_2("diskc"),   wxT_2("specify disc for drive C"), wxCMD_LINE_VAL_STRING },
    { wxCMD_LINE_OPTION, wxT_2("dd"), wxT_2("diskd"),   wxT_2("specify disc for drive D"), wxCMD_LINE_VAL_STRING },
    { wxCMD_LINE_OPTION, wxT_2("c"), wxT_2("cart"),   wxT_2("specify cartridge"), wxCMD_LINE_VAL_STRING },
    { wxCMD_LINE_OPTION, wxT_2("s"), wxT_2("snapshot"),    wxT_2("specify memory snapshot"), wxCMD_LINE_VAL_STRING },
    { wxCMD_LINE_OPTION, wxT_2("at"), wxT_2("autotype"),    wxT_2("auto type string"), wxCMD_LINE_VAL_STRING },
    { wxCMD_LINE_OPTION, wxT_2("cr"), wxT_2("crtctype"),    wxT_2("crtc type"), wxCMD_LINE_VAL_NUMBER},
    { wxCMD_LINE_OPTION, wxT_2("cfg"), wxT_2("config"),    wxT_2("set name of configuration to use"), wxCMD_LINE_VAL_STRING},
    { wxCMD_LINE_OPTION, wxT_2("r"), wxT_2("rom"),  wxT_2("specify ROM file"), wxCMD_LINE_VAL_STRING },

  { wxCMD_LINE_SWITCH, wxT_2("ns"), wxT_2("nosplash"),    wxT_2("do not display splash screen"), wxCMD_LINE_VAL_NONE},
    { wxCMD_LINE_SWITCH, wxT_2("na"), wxT_2("noaudio"),    wxT_2("do not initialise host audio"), wxCMD_LINE_VAL_NONE},
   { wxCMD_LINE_SWITCH, wxT_2("nj"), wxT_2("nojoystick"),    wxT_2("do not initialise host joystick/joypad"), wxCMD_LINE_VAL_NONE},
   { wxCMD_LINE_SWITCH, wxT_2("f"), wxT_2("fullscreen"),    wxT_2("fullscreen display"), wxCMD_LINE_VAL_NONE},

    // to allow files of any type
    { wxCMD_LINE_PARAM,  NULL, NULL, wxT_2("file"),       wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL|wxCMD_LINE_PARAM_MULTIPLE },
    { wxCMD_LINE_NONE }
};

void arnguiApp::OnInitCmdLine(wxCmdLineParser& parser)
{
    wxApp::OnInitCmdLine(parser);
    parser.SetDesc(m_CommandLineDesc);
}

bool arnguiApp::OnCmdLineParsed(wxCmdLineParser& parser)
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

    if (parser.Found(wxT("fullscreen")))
    {
      m_bFullScreen = true;
    }

    if (parser.Found(wxT("config"), &sValue))
    {
      m_sConfigRequested = sValue;
    }

    if (parser.Found(wxT("tape"), &sValue))
    {
      m_sTapeFile = sValue;
    }

    if (parser.Found(wxT("rom"), &sValue))
    {
      m_sROMFile = sValue;
    }

    if (parser.Found(wxT("cart"), &sValue))
    {
      m_sCartridgeFile = sValue;
    }

    if (parser.Found(wxT("snapshot"), &sValue))
    {
      m_sSnapshotFile = sValue;
    }

    if (parser.Found(wxT("diska"), &sValue) || parser.Found(wxT("drivea"),&sValue))
    {
      m_sDiskFile[0] = sValue;
    }

    if (parser.Found(wxT("diskb"), &sValue) || parser.Found(wxT("driveb"),&sValue))
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

        if (parser.Found(wxT("disk"), &sValue))
    {
      m_sDiskFileNoDrive = sValue;
    }

      long nValue;
    if (parser.Found(wxT("crtctype"), &nValue))
    {
      m_nCRTCType = nValue;
    }


    for (unsigned int i=0; i!=parser.GetParamCount(); i++)
    {
        wxString sParam = parser.GetParam(i);

      m_sMedia.Add(sParam);
    }
    return true;
}

const wxChar *sAuthor=wxT("Kevin Thacker");
const wxChar *sAppName=wxT("Arnold");
const wxChar *sAppDisplayName=wxT("Arnold Emulator");


arnguiApp::arnguiApp() : wxApp()
{
  m_nCRTCType = -1;   // no crtc type set by default
  m_bDisplaySplash = true; // enabled by default
  m_bJoystick = true;   // enabled by default
  m_bAudio = true;      // enabled by default
  m_bFullScreen = false; // windowed by default

  m_nFullScreenMode = -1;


  // default settings for audio
  m_nAudioFrequency = 44100;
  m_nAudioBits = 8;
  m_nAudioChannels = 2;
}


bool arnguiApp::OnInit()
{
  if (!wxApp::OnInit())
    return false;


   m_OldMessageOutput = wxMessageOutput::Set(new wxMessageOutputStderr);


    if (::wxDisplayDepth ()<=8)
    {
        wxMessageDialog *pDialog = new wxMessageDialog(NULL, wxT("Please use 16-bit, 24-bit or 32-bit display mode."),wxT(""),wxOK);
        pDialog->ShowModal();
        pDialog->Destroy();

        return false;
    }

    m_pPrinterOutputStream = NULL;
    m_PrinterData = 0;
    m_bPrinterStrobe = true;

#if (wxVERSION_NUMBER < 2900)
    wxSetlocale(LC_ALL, wxEmptyString);
#endif
    SetVendorName(sAuthor);
    SetAppName(sAppName);
//	SetAppDisplayName(sAppDisplayName);
    const wxString name = wxString::Format(wxT("%s-%s"),wxGetApp().GetAppName().c_str(), wxGetUserId().c_str());
    m_server = NULL;
    m_checker = new wxSingleInstanceChecker(name);
    if ( m_checker->IsAnotherRunning() )
    {
        bool bQuit = true;

        // top level window needed

        // run another?
        wxMessageDialog *pDialog = new wxMessageDialog(NULL, wxT("Another instance of Arnold is already running.\nDo you wish to run another?"), wxT(""),wxYES_NO);
        if (pDialog->ShowModal()==wxID_YES)
        {
            // if we say yes, then we don't quit and we continue
            bQuit = false;
        }
        pDialog->Destroy();	//pDialog->Destroy();

        if (bQuit)
        {
            // OK, there IS another one running, so try to connect to it
            // and send it any filename before exiting.
            stClient* client = new stClient;
            // ignored under DDE, host name in TCP/IP based classes
            wxString hostName = wxT("localhost");
//            wxGetHostName(hostName);
            // Create the connection
            wxConnectionBase* connection =
                client->MakeConnection(hostName, wxGetApp().GetAppName(), wxGetApp().GetAppName());
            if (connection)
            {
                wxString sCommandLine;
                sCommandLine+= wxT("\"");
                sCommandLine+=wxGetApp().GetAppName();
                sCommandLine+= wxT("\"");

                for (int i=0; i<wxApp::argc; i++)
                {
                    sCommandLine += wxT(" \"");
                    sCommandLine += wxString(wxApp::argv[i]);
                    sCommandLine += wxT("\"");
                }
                // Ask the other instance to open a file or raise itself
                connection->Execute(sCommandLine);
                connection->Disconnect();
                delete connection;
            }
            else
            {
                wxMessageBox(wxT("Sorry, the existing instance may be too busy too respond.\nPlease close any open dialogs and try again"),
                             wxGetApp().GetAppName(), wxICON_INFORMATION|wxOK);
            }
            delete client;

            return false;
        }
    }
    else
    {
        // Create a new server
        m_server = new stServer;
      // valgrind will throw an error because the port is not initialised.
      // wxWidgets allows named service or port number here.
      // I chose to use the app name.
        m_server->Create(wxGetApp().GetAppName() );
    }


    wxSplashScreen* splash = NULL;
    wxFileSystem::AddHandler(new wxArchiveFSHandler);
    wxFileSystem::AddHandler(new wxFilterFSHandler);
    wxFileSystem::AddHandler(new wxZipFSHandler);



    wxImage::AddHandler(new wxPNGHandler);
#ifdef WIN32
    wxImage::AddHandler(new wxBMPHandler);
#endif

  if (m_bDisplaySplash)
  {

    wxBitmap bitmap;

    wxFileName SplashLogoPath(GetAppPath(),  wxT("arnlogo.png"));
    wxString sSplashLogoPath = SplashLogoPath.GetFullPath();
    if (bitmap.LoadFile(sSplashLogoPath, wxBITMAP_TYPE_PNG))
    {
        splash = new wxSplashScreen(bitmap,
                                    wxSPLASH_CENTRE_ON_SCREEN|wxSPLASH_NO_TIMEOUT,
                                    -1, NULL, -1, wxDefaultPosition, wxDefaultSize,
                                    wxBORDER_SIMPLE|wxSTAY_ON_TOP);

    }
  }

    // create config
    m_pConfig = new wxConfig(wxGetApp().GetAppName(), wxEmptyString, wxEmptyString, wxEmptyString, wxCONFIG_USE_SUBDIR|wxCONFIG_USE_LOCAL_FILE);
    if (m_pConfig!=NULL)
    {
        // set this as current
        wxConfigBase::Set(m_pConfig);
    }
    else
    {
        wxMessageBox(wxT("Did not open config file. No settings will be saved."),
                    wxGetApp().GetAppName(), wxICON_INFORMATION|wxOK);
    }


    wxXmlResource::Get()->InitAllHandlers();
    wxXmlResource::Get()->AddHandler(new DissassembleWindowResourceHandler());
    wxXmlResource::Get()->AddHandler(new MemDumpWindowResourceHandler());
    wxXmlResource::Get()->AddHandler(new StackWindowResourceHandler());
    wxXmlResource::Get()->AddHandler(new HardwareDetailsResourceHandler());
    wxXmlResource::Get()->AddHandler(new BreakpointsResourceHandler());
    wxXmlResource::Get()->AddHandler(new GraphicsEditorResourceHandler());
    wxXmlResource::Get()->Load(GetGUIFullPath());


    // unfinished
    ScanConfigsAppThenLocal(wxT("computer"), m_ComputerConfigs);
    ScanConfigsAppThenLocal(wxT("keyboard"), m_KeyboardConfigs);
    ScanConfigsAppThenLocal(wxT("joystick"), m_JoystickConfigs);

    for (int i=0; i<4; i++)
    {
        DiskMedia *pDiskMedia = new DiskMedia(i);
        m_Drives[i] = pDiskMedia;
        m_Media.Add(pDiskMedia);
    }

    {
        CassetteMedia *pCassetteMedia = new CassetteMedia();
        m_Cassette = pCassetteMedia;
        m_Media.Add(pCassetteMedia);
    }

    {
        CartridgeMedia *pCartridgeMedia = new CartridgeMedia();
        m_Cartridge = pCartridgeMedia;
        m_Media.Add(pCartridgeMedia);
    }

    {
        SnapshotMedia *pSnapshotMedia = new SnapshotMedia();
        m_Snapshot = pSnapshotMedia;
        m_Media.Add(pSnapshotMedia);
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
        for (int i=ROM_ID_NULL+1; i<ROM_ID_LAST; i++)
        {
            Roms[i].nLength = 0;
            Roms[i].pData = NULL;
        }

        // now load
        for (unsigned int i=0; i!=sizeof(RomLoadList)/sizeof(RomLoadList[0]); i++)
        {
            unsigned char *pRomData;
            unsigned long RomLength;

            int nID = RomLoadList[i].nID;

            // if this rom is already loaded then assert
            //      wxASSERT_MSG((Roms[nID].pData!=NULL), "Rom already has data");

            wxString sFilename = GetROMFullPath() + RomLoadList[i].sFilename;
            if (LoadLocalFile(sFilename, &pRomData, &RomLength))
            {
                Roms[nID].sTag = RomLoadList[i].sFilename;
                Roms[nID].pData = pRomData;
                Roms[nID].nLength = RomLength;
            }
            else
            {
                wxCharBuffer buf = sFilename.mb_str();
				printf("Failed to load ROM %s\n", buf.data());

            }
        }
    }
    AutoType_Init();
    AutoRunFile_Init();
	
    /* initialise cpc hardware */
    CPC_Initialise();


    YMOutput_Init();

//    Multiface2_Init();

    // setup devices which are accessible in the devices window.
    // this is now the correct way to implement devices in a plug and play type
    // way

	// setup amdrum device
	Amdrum_Init();
	// amram 2 device
	Amram2_Init();
	// setup megarom device
	MegaROM_Init();
	// setup symbiface device
	Symbiface_Init();
	// brunword mk2
	BrunWordMk2_Init();
	// brunword mk4
	BrunWordMk4_Init();
	// CPC-AY
	CTCAY_Init();
	// Multiface2
	Multiface2_Init();
    // dk'tronics speech
    DkTronicsSpeech_Init();
    // ssa-1 speech
    SSA1SpeechDevice_Init();
    // kempston mouse
    KempstonMouse_Init();
    // magnum light phaser
    MagnumLightPhaserDevice_Init();
    // disc wizard
    DiscWizard_Init();
	// trans tape
	TransTape_Init();
	// hack it/le hackeur
    //HackIt_Init();
    int nValue;
    bool bValue;
    wxString sValue;

    // setup devices..
    for (int i=0; i<EmuDevice_GetNumDevices(); i++)
    {
        wxString sDeviceName(EmuDevice_GetName(i), wxConvUTF8);
        wxString sKey;


        // device is enabled?
        sKey.sprintf(wxT("devices/%s/enabled"), sDeviceName.c_str());
        wxConfig::Get(false)->Read(sKey, &bValue, false);
        EmuDevice_Enable(i,bValue);
        printf("Device %s is %s\n", EmuDevice_GetName(i), bValue ? "enabled" : "disabled");
      
        // setup switches
        for (int j=0; j<EmuDevice_GetNumSwitches(i); j++)
        {
            wxString sSwitchName(EmuDevice_GetSwitchName(i, j), wxConvUTF8);

            sKey.sprintf(wxT("devices/%s/switches/%s/state"), sDeviceName.c_str(), sSwitchName.c_str());
            wxConfig::Get(false)->Read(sKey, &bValue, false);

            EmuDevice_SetSwitchState(i, j, bValue);
          printf("Device %s, switch %s: %s\n", EmuDevice_GetName(i), EmuDevice_GetSwitchName(i,j), bValue ? "on" : "off");
        }


        // setup roms
        for (int j=0; j<EmuDevice_GetNumRoms(i); j++)
        {
            wxString sRomName(EmuDevice_GetRomName(i, j), wxConvUTF8);

            sKey.sprintf(wxT("devices/%s/roms/%s/filename"), sDeviceName.c_str(), sRomName.c_str());
            wxConfig::Get(false)->Read(sKey, &sValue, wxEmptyString);

            if (sValue.IsEmpty())
            {
               printf("Device %s, rom %s: no file picked for ROM\n", EmuDevice_GetName(i), EmuDevice_GetRomName(i,j));
            }
            else
            {
                unsigned char *pData = NULL;
                unsigned long nLength = 0;
                if (!wxGetApp().LoadLocalFile(sValue, &pData, &nLength))
                {
                    wxCharBuffer buf = sValue.mb_str();
                    printf("Failed to load ROM: Device %s, rom %s, filename %s\n", EmuDevice_GetName(i), EmuDevice_GetRomName(i,j),buf.data());

                }
                else
                {
                    wxCharBuffer buf = sValue.mb_str();
                    EmuDevice_SetRom(i, j, pData, nLength);
                    free(pData);
                    printf("Set ROM data for Device %s, rom %s, filename %s\n", EmuDevice_GetName(i), EmuDevice_GetRomName(i,j), buf.data());
                }
            }

        }
    }

    {
        unsigned char *pCart;
        unsigned long CartLength;

        GetRomByID(ROM_ID_SYSTEM_CART_CSD, &pCart, &CartLength);

        Jukebox_InsertSystemCartridge(pCart, CartLength);
    }
    Joystick_InitDefaultSettings();



	


    DebuggerWindow::Init();

    /* CPC initialised, so set default setup */
//	CPCEmulation_InitialiseDefaultSetup();

       wxConfig::Get(false)->Read(wxT("DefaultCartridgeID"), &nValue, 0);
       InsertDefaultCartridge(nValue);

       wxConfig::Get(false)->Read(wxT("ConfigName"), &sValue, wxT("cpc6128en"));
       ChooseConfig(sValue);

 
    wxConfig::Get(false)->Read(wxT("fullscreen/mode"), &nValue, -1);
    m_nFullScreenMode = nValue;

    wxConfig::Get(false)->Read(wxT("audio/frequency"), &nValue, 44100);
      SetAudioFrequency(nValue);
  	printf("Audio frequency: %d\n", nValue);

    wxConfig::Get(false)->Read(wxT("audio/bits"), &nValue, 8);
     SetAudioBits(nValue);
  	printf("Audio bits per sample: %d\n", nValue);

    wxConfig::Get(false)->Read(wxT("audio/channels"), &nValue, 2);
     SetAudioChannels(nValue);
  	printf("Audio channels: %d\n", nValue);

	wxConfig::Get(false)->Read(wxT("audio/output"), &nValue, 0);
    Audio_SetOutput(nValue);

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


  	printf("Audio output: %d\n", nValue);
#endif


  if (!m_PlatformSpecific.Init(m_bAudio, m_bJoystick))
  {
      return false;
  }

      m_PlatformSpecific.AutoConfigureJoystick();
	//  m_PlatformSpecific.ConfigureAudio();<<<<CRASH HERE
  
    frame = new arnguiFrame(m_bFullScreen,0L);
    SetTopWindow(frame);
#ifdef __WXMSW__
    frame->SetIcon(wxICON(aaaa)); // To Set App Icon
#endif
    if (splash)
    {
      // delete the splash screen now the window has shown
      splash->Destroy();
    }
    frame->Show();


    wxConfig::Get(false)->Read(wxT("cassette/negate_polarity"), &bValue, false);
    Cassette_NegatePolarity(bValue);
    printf("Tape negate polarity of signal: %s\n", bValue ? "yes" : "no");
    

    wxConfig::Get(false)->Read(wxT("csd/auto_timer_enabled"), &bValue, false);
    Jukebox_SetTimerEnabled(bValue);
  printf("CSD auto timer enabled: %s\n", bValue ? "yes" : "no");

    wxConfig::Get(false)->Read(wxT("csd/auto_timer_selection"), &nValue, 3);
    Jukebox_SetTimerSelection(nValue);
  printf("CSD auto timer selection: %d\n", nValue);

    wxConfig::Get(false)->Read(wxT("csd/manual_timer_selection"), &nValue, 7);
    Jukebox_SetManualTimerSelection(nValue);
  printf("CSD manual timer selection: %d\n", nValue);

    for (int i=0; i<13; i++)
    {
        wxString sConfigPrefix;

        sConfigPrefix.Printf(wxT("csd/cartridge%d/"),i);

        // system cartridge is enabled by default for csd. All others are disabled by default.
        bool bDefault = false;
        if (i==0)
        {
            bDefault = true;
        }
        wxConfig::Get(false)->Read(sConfigPrefix+wxT("enabled"), &bValue, bDefault);
        Jukebox_SetCartridgeEnable(i, bValue);
	printf("CSD cartridge %d: %s\n", i, Jukebox_IsCartridgeEnabled(i) ? "enabled" : "disabled");

        // TODO: read cartridge filename
        wxConfig::Get(false)->Read(sConfigPrefix+wxT("filename"), &sValue, wxEmptyString);
    }


    for (int i=0; i<16; i++)
    {
        wxString sConfigPrefix;

        sConfigPrefix.Printf(wxT("rom/rom%d/"),i);

        wxConfig::Get(false)->Read(sConfigPrefix+wxT("enabled"), &bValue, false);
//        ExpansionRom_SetActiveState(i, bValue);

        // read cartridge filename
        wxConfig::Get(false)->Read(sConfigPrefix+wxT("filename"), &sValue, wxEmptyString);
        // TODO: set in dialog
    }

    wxConfig::Get(false)->Read(wxT("cassette/ignore_motor"), &bValue, false);
    Computer_SetIgnoreMotor(bValue);
    printf("Tape hardware ignores motor on/off request: %s\n", bValue ? "yes" : "no");

    //  wxConfig::Get(false)->Read(wxT("disc/side_switch_active"), &bValue, false);
    // FDI_SetSwapSides(bValue);

    wxConfig::Get(false)->Read(wxT("disc/force_side_1"), &bValue, false);
    FDI_SetForceSide1(bValue);
  printf("FDI force side 1 (side switch): %s\n", bValue ? "yes" : "no");

    wxConfig::Get(false)->Read(wxT("disc/force_disc_rom_off"), &bValue, false);
    FDI_SetForceDiscRomOff(bValue);
  printf("FDI force disc rom off : %s\n", bValue ? "yes" : "no");

    wxConfig::Get(false)->Read(wxT("disc/drive_switch_active"), &bValue, false);
    FDI_SetSwapDrives(bValue);
  printf("FDI drive switch: %s\n", bValue ? "A=B, B=A" : "A=A, B=B");

    wxConfig::Get(false)->Read(wxT("disc/4_drives_enable"), &bValue, false);
    FDI_Set4Drives(bValue);
  printf("FDI use 4 drives: %s\n", bValue ? "yes" : "no");

    wxConfig::Get(false)->Read(wxT("links/exp_link"), &bValue, true);
    CPC_SetExpLow(bValue);
  printf("/EXP input to CPC is LOW: %s\n", bValue ? "yes" : "no");

    wxConfig::Get(false)->Read(wxT("links/50hz_link"), &bValue, true);
    CPC_Set50Hz(bValue);
  printf("CPC 50Hz setting: %s\n", bValue ? "yes" : "no");

    wxConfig::Get(false)->Read(wxT("links/computer_name_link"), &nValue, (int)PPI_COMPUTER_NAME_AMSTRAD);
    CPC_SetComputerNameIndex(nValue);
  printf("CPC Computer name: %d\n", nValue);

    wxConfig::Get(false)->Read(wxT("monitor/monitor_type"), &nValue, (int)CPC_MONITOR_COLOUR);
    CPC_SetMonitorType((CPC_MONITOR_TYPE_ID)nValue);
printf("CPC Monitor Type: %d\n", nValue);

    wxConfig::Get(false)->Read(wxT("crtc/crtc_type"), &nValue, (int)0);
    CPC_SetCRTCType(nValue);
printf("CPC CRTC Type: %d\n", nValue);

    wxConfig::Get(false)->Read(wxT("ram/ram_64k_ram_enabled"), &bValue, true);
    CPC_EnableRamConfig(bValue, CPC_RAM_CONFIG_64K_RAM);
printf("64K Dk'Tronics External RAM enabled: %s\n", bValue ? "yes" : "no");

    wxConfig::Get(false)->Read(wxT("ram/ram_256k_ram_enabled"), &bValue, false);
    CPC_EnableRamConfig(bValue, CPC_RAM_CONFIG_256K_RAM);
printf("256K Dk'Tronics External RAM enabled: %s\n", bValue ? "yes" : "no");

    wxConfig::Get(false)->Read(wxT("ram/ram_256k_silicon_disk_enabled"), &bValue, false);
    CPC_EnableRamConfig(bValue, CPC_RAM_CONFIG_256K_SILICON_DISK);
printf("256K Dk'Tronics Silicon Disk RAM enabled: %s\n", bValue ? "yes" : "no");

    // ym
    wxConfig::Get(false)->Read(wxT("ym/record/version"), &nValue, YMOUTPUT_VERSION_5);
    YMOutput_SetVersion(nValue);
  printf("YM recording version %d\n", nValue);
  
    wxConfig::Get(false)->Read(wxT("ym/record/start_record_when_silence_ends"), &bValue, true);
    YMOutput_SetRecordWhenSilenceEnds(bValue);
printf("YM start recording when silence ends: %s\n", bValue ? "yes" : "no");
  
    wxConfig::Get(false)->Read(wxT("ym/record/stop_record_when_silence_begins"), &bValue, true);
    YMOutput_SetStopRecordwhenSilenceBegins(bValue);
printf("YM stop recording when silence begins: %s\n", bValue ? "yes" : "no");

    // drive settings
    for (int i=0; i<4; i++)
    {
        wxString sConfigPrefix;

        sConfigPrefix.Printf(wxT("drive/%c/"),i+'a');

      // by default don't turn to side B
        wxConfig::Get(false)->Read(sConfigPrefix+wxT("turn_disk_to_side_b"), &bValue, false);
        FDD_TurnDiskToSideB(i, bValue);
        if (bValue)
        {
          printf("Drive %d is turned to side B\n", i);
        }
        else
        {
          printf("Drive %d is not turned over to side B\n",i);
        }

        // by default just enable drive 0 and 1
        bool bEnableDrive = false;
        if ((i==0) || (i==1))
        {
          bEnableDrive = true;
        }
        wxConfig::Get(false)->Read(sConfigPrefix+wxT("enable"), &bValue, bEnableDrive);
        FDD_Enable(i, bValue);
        printf("Drive %d is %s.\n", i, bValue ? "enabled" : "disabled" );

          // by default don't force write protect
        wxConfig::Get(false)->Read(sConfigPrefix+wxT("force_write_protect"), &bValue, false);
        FDD_SetAlwaysWriteProtected(i, bValue);
        if (bValue)
        {
          printf("Drive %d is always write protected.\n",i);
        }
        else
        {
          printf("Drive %d is read/write.\n", i);
        }

        // by default don't force ready
        wxConfig::Get(false)->Read(sConfigPrefix+wxT("force_ready"), &bValue, false);
        FDD_SetAlwaysReady(i, bValue);
      if (bValue)
        {
          printf("Drive %d is always ready.\n",i );
        }
        else
        {
          printf("Drive %d has correct ready signal operation.\n",i);
        }

        // drive A defaults to single sided, all others are double sided
        bool bDoubleSided = true;
        if (i==0)
        {
            bDoubleSided = false;
        }
        wxConfig::Get(false)->Read(sConfigPrefix+wxT("double_sided"), &bValue, bDoubleSided);
        FDD_SetDoubleSided(i, bValue);
        printf("Drive %d is %s sided.\n", i, bValue ? "double" : "single" );

        // drive A defaults to single sided 40 track
        int nTracks = MAX_TRACKS_80_TRACK;
        if (i==0)
        {
            nTracks = MAX_TRACKS_40_TRACK;
        }
        wxConfig::Get(false)->Read(sConfigPrefix+wxT("max_tracks"), &nValue, nTracks);
        FDD_SetTracks(i, nValue);
        printf("Drive %d has %d tracks.\n", i, nValue);
    }



    for (int i=0; i<2; i++)
    {
        wxString sConfigPrefix;

        sConfigPrefix.Printf(wxT("joystick/digital%d/"),i);

        // by default joysticks are not enabled.. or should we just enable joystick 0?
        wxConfig::Get(false)->Read(sConfigPrefix+wxT("enable"), &bValue, false);
        Joystick_Activate(CPC_DIGITAL_JOYSTICK0+i, bValue);
        printf("Digital Joystick %d is %s.\n", i,bValue ? "active" : "inactive" );

      wxConfig::Get(false)->Read(sConfigPrefix+wxT("type"), &nValue, JOYSTICK_TYPE_UNKNOWN);
        Joystick_SetType(CPC_DIGITAL_JOYSTICK0+i, nValue);

          printf("Digital Joystick %d is ",i);
      switch (nValue)
        {
            case JOYSTICK_TYPE_SIMULATED_BY_KEYBOARD:
            {
                printf("simulated by keyboard.\n");
            }
            break;

            case JOYSTICK_TYPE_SIMULATED_BY_MOUSE:
            {
                printf("simulated by mouse.\n");
            }
            break;

            case JOYSTICK_TYPE_REAL:
            {
                printf("a physical joystick/gamepad.\n");
            }
            break;


            default:
            {
                printf("controlled by unknown.\n");
            }
            break;
          }

        wxConfig::Get(false)->Read(sConfigPrefix+wxT("keyset"), &nValue, JOYSTICK_KEYSET_UNKNOWN);
        Joystick_SetKeySet(CPC_DIGITAL_JOYSTICK0+i, nValue);
           printf("Digital Joystick %d uses keyset %d\n",i, nValue);

        wxConfig::Get(false)->Read(sConfigPrefix+wxT("joystickID"), &nValue, -1);
        Joystick_SetPhysical(CPC_DIGITAL_JOYSTICK0+i, nValue);
      printf("Digital Joystick %d uses physical joystick %d\n",i, nValue);

        for (int j=0; j<JOYSTICK_SIMULATED_KEYID_LAST; j++)
        {
            wxString sKeyID;
            sKeyID.Printf(wxT("keyID%d"),j);
            wxConfig::Get(false)->Read(sConfigPrefix+sKeyID, &nValue, 0);
            Joystick_SetSimulatedKeyID(CPC_DIGITAL_JOYSTICK0+i, j, nValue);
        }

    }

    for (int i=0; i<2; i++)
    {
        wxString sConfigPrefix;

        sConfigPrefix.Printf(wxT("joystick/analogue%d/"),i);

    // by default analogue joysticks are not enabled
        wxConfig::Get(false)->Read(sConfigPrefix+wxT("enable"), &bValue, false);
        Joystick_Activate(CPC_ANALOGUE_JOYSTICK0+i, bValue);
        printf("Analogue Joystick %d is %s.\n", i,bValue ? "active" : "inactive" );

        wxConfig::Get(false)->Read(sConfigPrefix+wxT("type"), &nValue, JOYSTICK_TYPE_UNKNOWN);
        Joystick_SetType(CPC_ANALOGUE_JOYSTICK0+i, nValue);
  printf("Analogue Joystick %d is ",i);
      switch (nValue)
        {
            case JOYSTICK_TYPE_SIMULATED_BY_KEYBOARD:
            {
                printf("simulated by keyboard.\n");
            }
            break;

            case JOYSTICK_TYPE_SIMULATED_BY_MOUSE:
            {
                printf("simulated by mouse.\n");
            }
            break;

            case JOYSTICK_TYPE_REAL:
            {
                printf("a physical joystick/gamepad.\n");
            }
            break;

            default:
            {
                printf("controlled by unknown.\n");
            }
            break;

          }
        wxConfig::Get(false)->Read(sConfigPrefix+wxT("keyset"), &nValue, JOYSTICK_KEYSET_UNKNOWN);
        Joystick_SetKeySet(CPC_ANALOGUE_JOYSTICK0+i, nValue);
     printf("Analogue Joystick %d uses keyset %d\n",i, nValue);

        wxConfig::Get(false)->Read(sConfigPrefix+wxT("joystickID"), &nValue, -1);
        Joystick_SetPhysical(CPC_ANALOGUE_JOYSTICK0+i, nValue);
  printf("Analogue Joystick %d uses physical joystick %d\n",i, nValue);

        for (int j=0; j<JOYSTICK_SIMULATED_KEYID_LAST; j++)
        {
            wxString sKeyID;
            sKeyID.Printf(wxT("keyID%d"),j);
            wxConfig::Get(false)->Read(sConfigPrefix+sKeyID, &nValue, 0);
            Joystick_SetSimulatedKeyID(CPC_ANALOGUE_JOYSTICK0+i, j, nValue);
        }


    }

 //   wxConfig::Get(false)->Read(wxT("multiface/enabled"), &bValue, false);
  //  Multiface_EnableEmulation(bValue);
// printf("Multiface emulation is %s\n", bValue ? "enabled" : "disabled");

//   wxConfig::Get(false)->Read(wxT("multiface/rom/cpc"), &sValue, wxEmptyString);
//   wxConfig::Get(false)->Read(wxT("multiface/rom/plus"), &sValue, wxEmptyString);


    wxConfig::Get(false)->Read(wxT("snapshot/write_version"), &nValue, 3);
    SnapshotSettings_SetVersion(nValue);
printf("Will write snapshot version %d\n", nValue);

    wxConfig::Get(false)->Read(wxT("snapshot/compressed"), &bValue, true);
    SnapshotSettings_SetCompressed(bValue);
  printf("Will write snapshot 3: %s\n", bValue ? "compressed" : "uncompressed");

    //  wxConfig::Get(false)->Read(wxT("debug/dissassemble/show_opcodes"), &bValue, true);

    //  wxConfig::Get(false)->Read(wxT("debug/dissassemble/show_ascii"), &bValue, true);

    wxConfig::Get(false)->Read(wxT("display/scanlines"), &bValue, true);
    Render_SetScanlines(bValue);

    wxConfig::Get(false)->Read(wxT("keyboard/mode"), &nValue, 0);
    SetKeyboardMode(nValue);
  printf("Keyboard mode: %d\n", nValue);
  
    wxConfig::Get(false)->Read(wxT("keyboard/language"), &nValue, KEYBOARD_LANGUAGE_ID_ENGLISH);
    Keyboard_SetLanguage(nValue);
      printf("Keyboard language: %d\n", nValue);

    wxConfig::Get(false)->Read(wxT("keyboard/positional/set"), &nValue, 0);
    Keyboard_SetPositionalSet(nValue);
    SetKeySet(nValue);
      printf("Keyboard positional set: %d\n", nValue);

    wxConfig::Get(false)->Read(wxT("keyboard/hardware"), &nValue, 1);
    Keyboard_EnableClash(nValue==1 ? TRUE : FALSE);

    g_TextWindowProperties.Init();

    RefreshJoystick();


      printf("Handling command line parameters\n");

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

        if (!m_Cassette->LoadWithSaveModified(m_sTapeFile))
        {
            wxCharBuffer buf = m_sTapeFile.mb_str();
            printf("Failed to load Tape %s\n", buf.data());


        }

    }

    // TODO: Handle ROM

    if (!m_sCartridgeFile.IsEmpty())
    {
    wxString sMessage;
      sMessage.Format(wxT("Command-line: Cartridge: %s\r\n"), m_sCartridgeFile.GetData());
      TraceMessage(sMessage);
        if (!m_Cartridge->LoadWithSaveModified(m_sCartridgeFile))
        {
            wxCharBuffer buf = m_sCartridgeFile.mb_str();
            printf("Failed to load Cartridge %s\n", buf.data());
        }
    }

    if (!m_sSnapshotFile.IsEmpty())
    {
       wxString sMessage;
      sMessage.Format(wxT("Command-line: Snapshot: %s\r\n"), m_sSnapshotFile.GetData());
      TraceMessage(sMessage);
        if (!m_Snapshot->LoadWithSaveModified(m_sSnapshotFile))
        {
            wxCharBuffer buf = m_sSnapshotFile.mb_str();
            printf("Failed to load Snapshot %s\n", buf.data());
        }
    }

    for (size_t i=0; i<4; i++)
    {
        if (!m_sDiskFile[i].IsEmpty())
        {
             wxString sMessage;
        sMessage.Format(wxT("Command-line: Disk insert into drive %c: %s\r\n"), 'A'+i, m_sDiskFile[i].GetData());
        TraceMessage(sMessage);
          if (!m_Drives[i]->LoadWithSaveModified(m_sDiskFile[i]))
         {
            wxCharBuffer buf = m_sDiskFile[i].mb_str();
            printf("Failed to load Disk  %s\n", buf.data());
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
        for (int i=0; i<4; i++)
        {
            wxString sDrive;
            sDrive.Printf(wxT("Drive %c"),'A'+i);
            sDrives.Add(sDrive);
        }

        sMessage = wxT("Insert disk image file:\n\"")+m_sDiskFileNoDrive+wxT("\"\ninto which drive?\n");
        wxSingleChoiceDialog *pDialog = new wxSingleChoiceDialog(GetTopWindow(), sMessage, wxT("Choose drive"), sDrives);
        pDialog->SetSelection(0);
        if (pDialog->ShowModal()==wxID_OK)
        {
            int nDrive = pDialog->GetSelection();

            if (!m_Drives[nDrive]->LoadWithSaveModified(m_sDiskFileNoDrive))
            {
                wxCharBuffer buf = m_sDiskFileNoDrive.mb_str();
                printf("Failed to load Disk  %s\n", buf.data());


            }
        }
        pDialog->Destroy();	//pDialog->Destroy();
    }

    // autotype, allow C style \r\n etc
    if (!m_sAutoType.IsEmpty())
    {
       wxString sMessage;
       sMessage.sprintf(wxT("Command-line: Autotype text: %s\r\n"), m_sAutoType.c_str());
      TraceMessage(sMessage);

        /*   wxString sMessage;
           sMessage.Printf(wxT("Autotype string: %s"), sAutoType.c_str());
            wxMessageDialog *pMessage = new wxMessageDialog(frame,sMessage,wxEmptyString, wxYES_NO|wxICON_EXCLAMATION);;
           if (pMessage!=NULL)
           {
               if (pMessage->ShowModal()==wxID_YES)
               {

               }
           }
           delete pMessage;
        */
        // unescape eol and tab
        m_sAutoType.Replace(wxT("\\r"), wxT("\r"));
        m_sAutoType.Replace(wxT("&quot;"), wxT("\""));
        m_sAutoType.Replace(wxT("\\n"), wxT("\n"));
        m_sAutoType.Replace(wxT("\\t"), wxT("\t"));
        m_sAutoType.Replace(wxT("\\\""), wxT("\""));

        const wxCharBuffer buffer =m_sAutoType.utf8_str();

        /* wait for input, reset cpc */
        AutoType_SetString(buffer.data(),TRUE, TRUE, TRUE);
        buffer.release();
    }

    if (m_nCRTCType!=-1)
     {
             wxString sMessage;
      sMessage.Format(wxT("Command-line: CRTC type: %d\r\n"), (int)m_nCRTCType);
      TraceMessage(sMessage);

      CPC_SetCRTCType(m_nCRTCType);
    }

    for (unsigned int i=0; i!=m_sMedia.GetCount(); i++)
    {
        wxString sParam = m_sMedia[i];

       wxString sMessage;
      sMessage.Format(wxT("Command-line: Insert Media: %s\r\n"), sParam.GetData());
      TraceMessage(sMessage);

        InsertMedia(sParam);

    }

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

            Keyboard_SetPositionalSet(2);

        }
        else
        if (_tcsicmp(sKeyboardLayoutName,_T("0000040a"))==0)
        {
            wxString sMessage;
          sMessage.Format(wxT("Keyboard layout detected as Spanish Traditional Sort"));
          TraceMessage(sMessage);

            Keyboard_SetPositionalSet(3);

        }
           else
        if (_tcsicmp(sKeyboardLayoutName,_T("00000407"))==0)
        {
            wxString sMessage;
          sMessage.Format(wxT("Keyboard layout detected as German Standard"));
          TraceMessage(sMessage);

            Keyboard_SetPositionalSet(1);

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
//    wxString sPrinterFilename(wxT("./printer.txt"));
 // 	OpenPrinterFile( sPrinterFilename );
  printf("Starting main loop\n");

    // start the main loop
    return true;
}

void arnguiApp::WriteConfigWindowPos(const wxString &sName, const wxWindow *pWindow)
{
    int nX, nY;

    pWindow->GetPosition(&nX, &nY);

    wxConfig::Get(false)->Write(sName+wxT("position/x"), nX);
    wxConfig::Get(false)->Write(sName+wxT("position/y"), nY);
}


bool arnguiApp::ReadConfigWindowPos(const wxString &sName, wxWindow *pWindow)
{
    int nX, nY;

    if (wxConfig::Get(false)->Read(sName+wxT("position/x"), &nX))
    {
        if (wxConfig::Get(false)->Read(sName+wxT("position/y"), &nY))
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

    wxConfig::Get(false)->Write(sName+wxT("dimensions/width"), nWidth);
    wxConfig::Get(false)->Write(sName+wxT("dimensions/height"), nHeight);
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
    if ((windowX+windowWidth)<clientX)
    {
        windowX = clientX;
        bSetWindowPosition = true;
    }

    if ((windowY+windowHeight)<clientY)
    {
        windowY = clientY;
        bSetWindowPosition = true;
    }

    if (windowX>(clientX+clientWidth))
    {
        windowX = (clientX+clientWidth)-windowWidth;
        bSetWindowPosition = true;
    }

    if (windowY>(clientY+clientHeight))
    {
        windowY = (clientY+clientHeight)-windowHeight;
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

    if (wxConfig::Get(false)->Read(sName+wxT("dimensions/width"), &nWidth,-1))
    {

        if (wxConfig::Get(false)->Read(sName+wxT("dimensions/height"), &nHeight,-1))
        {
            if ((nWidth>0) && (nHeight>0))
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
    bState = bState || ReadConfigWindowSize(sName, pWindow);
	return bState;
}

void arnguiApp::ReadConfigListCtrl(const wxString &sName, wxListCtrl *pListCtrl)
{
    // number of columns
    int nColumnCount;
    wxConfig::Get(false)->Read(sName+wxT("NumColumns"), &nColumnCount,0);

    for (int i=0; i<nColumnCount; i++)
    {
        wxString sColName;
        sColName.Printf(wxT("Column%d/"), i);

        int nWidth;
        // get width
        wxConfig::Get(false)->Read(sName+sColName+wxT("Width"), &nWidth, 0);

        wxString sColumnName;
        // get name
        wxConfig::Get(false)->Read(sName+sColName+wxT("Name"), &sColumnName, wxEmptyString);

        if (sName!=wxEmptyString)
        {
            if (nWidth!=0)
            {
                for (int j=0; j<pListCtrl->GetColumnCount(); j++)
                {
                    // find column by name
                    wxListItem Item;
                    Item.SetId(j);
                    Item.SetMask(wxLIST_MASK_TEXT);
                    if (pListCtrl->GetColumn(j, Item))
                    {
                        if (Item.GetText()==sColumnName)
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


void arnguiApp::WriteConfigListCtrl(const wxString &sName, const wxListCtrl *pListCtrl)
{
    // number of columns
    wxConfig::Get(false)->Write(sName+wxT("NumColumns"), pListCtrl->GetColumnCount());

    // name and width of each column
    for (int i=0; i<pListCtrl->GetColumnCount(); i++)
    {
        wxListItem Item;
        Item.SetId(i);
        Item.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_WIDTH);
        pListCtrl->GetColumn(i, Item);

        wxString sColName;
        sColName.Printf(wxT("Column%d/"), i);

        wxConfig::Get(false)->Write(sName+sColName+wxT("Width"), Item.GetWidth());
        wxConfig::Get(false)->Write(sName+sColName+wxT("Name"), Item.GetText());

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
    if (!wxFileName::DirExists(sUserDataDir))
    {
        wxFileName::Mkdir(sUserDataDir, 0777, wxPATH_MKDIR_FULL);
    }
}


void arnguiApp::WriteConfigColour(const wxString &sName, const wxColour &Colour)
{

    wxString sValue;
    sValue.Printf(wxT("%02x%02x%02x"), Colour.Red(), Colour.Green(), Colour.Blue());

    wxConfig::Get(false)->Write(sName, sValue);
}
#if 0
void arnguiApp::InsertCartridge(const wxString &sFilename)
{
    unsigned char *pCartridgeData;
    unsigned long CartridgeLength;

    /* try to load it */
    LoadLocalFile(sFilename, &pCartridgeData, &CartridgeLength);

    if (pCartridgeData!=NULL)
    {
        int nStatus = Cartridge_AttemptInsert(pCartridgeData, CartridgeLength);

        if (nStatus==ARNOLD_STATUS_OK)
        {
        }

        free(pCartridgeData);
    }
}
#endif

extern "C"
{

    extern int PIXEL_STEP_SHIFT;
    extern int ScanLines;
    extern int FillScanLines;
}

void arnguiApp::SaveScreenshot(const wxString &sFilename)
{
    int ScreenWidth = (X_CRTC_CHAR_WIDTH<<(1+3));
    int ScreenHeight = Y_CRTC_LINE_HEIGHT;
    int x;

    ScreenWidth = ScreenWidth>>PIXEL_STEP_SHIFT;

    if ((ScanLines) || (FillScanLines))
    {
        ScreenHeight = ScreenHeight<<1;
    }

    unsigned char *pixels = new unsigned char[ScreenWidth*ScreenHeight*3];

    // scanlines, fill scanlines?
    for ( int y = 0; y < ScreenHeight; ++y )
    {
        int Offset = y*ScreenWidth*3;

        if ((ScanLines) && (!FillScanLines) && ((y & 1)!=0))
        {
            for (x=0; x<ScreenWidth; x++)
            {
                pixels[Offset+(x*3)+0] = 0;
                pixels[Offset+(x*3)+1] = 0;
                pixels[Offset+(x*3)+2] = 0;
            }
        }
        else
        {
            for (x=0; x<ScreenWidth; x++)
            {
                unsigned char r,g,b;

                /* get pixel from graphics buffer */
                if ((FillScanLines) || (ScanLines))
                {
                    Render_GetPixelRGBAtXY(x,(y>>1), &r, &g, &b);
                }
                else
                {
                    Render_GetPixelRGBAtXY(x,y, &r, &g, &b);
                }
                pixels[Offset+(x*3)+0] = r;
                pixels[Offset+(x*3)+1] = g;
                pixels[Offset+(x*3)+2] = b;
            }
        }
    }
    wxImage image(ScreenWidth, ScreenHeight);
    image.SetData(pixels, true);
    image.SaveFile(sFilename);
    delete [] pixels;
}
#if 0
void arnguiApp::LoadSnapshot(const wxString &sFilename)
{
    unsigned char *pSnapshotData;
    unsigned long SnapshotLength;

    /* try to load it */
    LoadLocalFile(sFilename, &pSnapshotData, &SnapshotLength);

    if (pSnapshotData!=NULL)
    {
        if (Snapshot_Insert(pSnapshotData, SnapshotLength)==ARNOLD_STATUS_OK)
        {
        }

        free(pSnapshotData);
    }
}
#endif


void arnguiApp::SaveSnapshot(const wxString &sFilename)
{
    unsigned long nLength;
    unsigned char *pSnapshotData = NULL;
    SNAPSHOT_OPTIONS SnapshotOptions;

    SnapshotOptions.Version = SnapshotSettings_GetVersion();
    SnapshotOptions.bCompressed = SnapshotSettings_GetCompressed();

    nLength = Snapshot_CalculateEstimatedOutputSize(&SnapshotOptions);

    pSnapshotData = (unsigned char *)malloc(nLength);

    if (pSnapshotData!=NULL)
    {
        unsigned long nActualLength = Snapshot_GenerateOutputData(pSnapshotData, &SnapshotOptions);

        SaveLocalFile(sFilename, pSnapshotData, nActualLength);

        free(pSnapshotData);
    }
}


#include <wx/file.h>
#include <wx/filedlg.h>
#include <wx/mstream.h>

bool arnguiApp::LoadLocalFile(const wxString &sFilename, unsigned char **ppPtr, unsigned long *pLength)
{
    *ppPtr = NULL;
    *pLength = 0;

    bool bState = false;
    wxFileSystem* fileSystem = new wxFileSystem;
    if (fileSystem!=NULL)
    {
        wxFSFile* file = fileSystem->OpenFile(sFilename);

        if (file)
        {

            wxInputStream* pInputStream = file->GetStream();

#if 0
            wxMemoryOutputStream *pMemoryOutputStream = new wxMemoryOutputStream();
            pInputStream->Read(pMemoryOutputStream);

            size_t nLength = pMemoryOutputStream->GetLength();

            unsigned char *pData = (unsigned char *)malloc(nLength);
            if (pData!=NULL)
            {
                pMemoryOutputStream->CopyTo(pData, nLength);
                *ppPtr = pData;
                *pLength = nLength;
                bSuccess = true;
            }

            delete pMemoryOutputStream;
#endif

            size_t nSize = pInputStream->GetSize();
            if (nSize!=0)
            {
                unsigned char *pData = (unsigned char *)malloc(nSize);
                if (pData!=NULL)
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
    if (pFile->Open(sFilename, wxFile::read))
    {

        wxFileOffset Length = pFile->Length();
        unsigned char *pPtr = (unsigned char *)malloc(Length);
        if (pPtr!=NULL)
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
    wxFileName PotentialDir(Filename.GetFullName()+Filename.GetPathSeparator());
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
    // file already exists?
    if (::wxFileExists(sFilename))
    {
        // generate name of backup file
        wxString sNewFilename;
        sNewFilename = sFilename+wxT(".bak");
        // create backup
        ::wxCopyFile(sFilename, sNewFilename, true);
    }

    bool bSuccess = false;
    wxTempFile TempFile;
    if (TempFile.Open(sFilename))
    {
        if (TempFile.Write(pPtr, nLength))
        {

            if (TempFile.Commit())
            {
                bSuccess = true;

            }
        }
    }

    return bSuccess;

#if 0
    // wxTempFileOutputStream creates a temporary file, when writing is complete
    // it then overwrites existing
    wxTempFileOutputStream *pTempFileOutputStream = new wxTempFileOutputStream(sFilename);

    if (!pTempFileOutputStream->IsOK())
    {
        pTempFileOutputStream->Discard();
        delete pTempFileOutputStream;
        return false;
    }

    bool bSuccess = false;
    wxTempFileOutputStream &Result = pTempFileOutputStream->Write(pPtr, nLength);
    if (Result.LastWrite()==nLength)
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
    // file already exists?
    if (::wxFileExists(sFilename))
    {
        // generate name of backup file
        wxString sNewFilename;
        sNewFilename = sFilename+wxT(".bak");
        // create backup
        ::wxCopyFile(sFilename, sNewFilename, true);
    }

    bool bSuccess = false;
    wxTempFile TempFile;
    if (TempFile.Open(sFilename))
    {
        if (TempFile.Write(sText))
        {

            if (TempFile.Commit())
            {
                bSuccess = true;

            }
        }
    }

    return bSuccess;
}


#if 0
void arnguiApp::InsertTape(const wxString &sFilename)
{

    unsigned char *pTapeImageData = NULL;
    unsigned long TapeImageLength = 0;

    /* try to load it */
    LoadLocalFile(sFilename, &pTapeImageData, &TapeImageLength);

    if (pTapeImageData!=NULL)
    {
        if (TapeImage_Insert(pTapeImageData, TapeImageLength)==ARNOLD_STATUS_OK)
        {
            free(pTapeImageData);
        }
        else
        {
            free(pTapeImageData);

            //      if (Sample_Load(sFilename.c_str()))
            //    {
            //  }
        }
    }
}

bool arnguiApp::RemoveTape()
{
    Tape_Remove();
    return true;
}
#endif

void arnguiApp::CloseJoysticks()
{
  m_PlatformSpecific.CloseJoysticks();
}

void arnguiApp::RefreshJoystick()
{
  m_PlatformSpecific.RefreshJoysticks();

    for (int j=0; j<4; j++)
    {
        if (Joystick_GetType(j)==JOYSTICK_TYPE_SIMULATED_BY_KEYBOARD)
        {
            int nKeySet = Joystick_GetKeySet(j);

          m_PlatformSpecific.ConfigureJoystickKeySet(j,nKeySet);

        }
        else if (Joystick_GetType(j)==JOYSTICK_TYPE_SIMULATED_BY_MOUSE)
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

// get the location of the roms that come with the app are read from
wxString arnguiApp::GetROMFullPath()
{
    wxFileName ExeData(GetAppPath(),  wxT("roms.zip"));
    wxString sGUIFullPath = ExeData.GetFullPath();
    sGUIFullPath += wxT("#zip:");
    return sGUIFullPath;
}

// get the directory path of the app (with trailing separator)
// so we can easily add on another path or filename
wxString arnguiApp::GetAppPath()
{
    wxString sPath = wxStandardPaths::Get().GetExecutablePath();
    wxFileName Path(sPath);
    return Path.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR);
}

// get the location of the icons that are associated with file extensions (Windows)
// and only if we are defined to be the default for that association
wxString arnguiApp::GetIconsFullPath()
{
    // get location of executable
    wxFileName IconPath(GetAppPath(), wxT("icons.dll"));
    return IconPath.GetFullPath();
}

Media::Media() : m_nType(MEDIA_TYPE_UNKNOWN), m_bMediaInserted(false), m_nUnit(0)
{
    m_sCurrentPath = wxEmptyString;
}

wxString Media::GetRecentPath() const
{
    if (m_History.GetCount()==0)
        return wxEmptyString;

    return m_History.Item(0);
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



}

void Media::ForceSave()
{
    SaveModified(true);
}

bool Media::LoadWithSaveModified(const wxString &sFilename)
{
    SaveModified(false);
    Remove();

    return Load(sFilename);
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
            wxMessageDialog *pDialog = new wxMessageDialog(wxGetApp().GetTopWindow(), sMessage, wxT(""),wxYES_NO);
            int nResult = pDialog->ShowModal();
            pDialog->Destroy();

            // we are happy to reload.
            if (nResult==wxID_OK)
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

    return Load(sPath);
}

#if 0
void Media::SaveLoop()
{

        // loop until quit or we saved successfully
              while (!bDone)
              {
                  // if we're not requesting a name... can we save at this path?
                  if (!bRequestName && !wxGetApp().CanSave(sSaveName))
                  {
                      wxString sMessage;
                      sMessage.Printf(wxT("Media can't be saved to \"%s\".\nPlease pick a new filename."), m_sCurrentPath.c_str());

                      // indicate we can't save here
                      wxMessageDialog *pDialog = new wxMessageDialog(wxGetApp().GetTopWindow(), sMessage, wxT(""),wxOK|wxCANCEL);
                      int nResult = pDialog->ShowModal();
                      pDialog->Destroy();
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

                  if (!bDone)
                  {
                      // go for a save..
                        if (Save(sSaveName))
                      {
                        printf("Saved successfully\n");

                        // and reload it.
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
                wxMessageDialog *pDialog = new wxMessageDialog(wxGetApp().GetTopWindow(), sMessage, wxT(""),wxYES_NO);
                int nResult = pDialog->ShowModal();
                pDialog->Destroy();

                if (nResult==wxID_NO)
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
                      if (m_sCurrentPath==wxEmptyString)
                      {
                  // tell the user and give them the choice to quit, or to continue
                          wxMessageDialog *pDialog = new wxMessageDialog(wxGetApp().GetTopWindow(), wxT("No filename is set for media\nPlease pick a filename."), wxT(""),wxOK|wxCANCEL);
                          int nResult = pDialog->ShowModal();
                          pDialog->Destroy();

                          if (nResult==wxID_CANCEL)
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
                              wxString sMessage;
                              sMessage.Printf(wxT("Media can't be saved to \"%s\".\nPlease pick a new filename."), m_sCurrentPath.c_str());

                              // indicate we can't save here
                              wxMessageDialog *pDialog = new wxMessageDialog(wxGetApp().GetTopWindow(), sMessage, wxT(""),wxOK|wxCANCEL);
                              int nResult = pDialog->ShowModal();
                              pDialog->Destroy();
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

                      if (!bDone)
                      { 
                      // go for a save..
                        if (Save(sSaveName))
                      {
                        printf("Saved successfully\n");

                        // and reload it.
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
    m_sCurrentPath= sFilename;
    m_bMediaInserted = true;

    if (m_sCurrentPath!=wxEmptyString)
    {

        // if it is already in the list remove it
        for (unsigned int i=0; i!=m_History.GetCount(); i++)
        {
            if (m_History.Item(i)==sFilename)
            {
                m_History.RemoveAt(i);
                break;
            }
        }

        // insert at beginning
        m_History.Insert(sFilename, 0);
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

bool Media::Load(const wxString &sFilename)
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

bool DiskMedia::Load(const wxString &sFilename)
{
    unsigned char *pDiskImageData = NULL;
    unsigned long DiskImageLength = 0;

    /* try to load it */
    wxGetApp().LoadLocalFile(sFilename, &pDiskImageData, &DiskImageLength);

    if (pDiskImageData!=NULL)
    {
        int nStatus = DiskImage_InsertDisk(m_nUnit, pDiskImageData, DiskImageLength);

        if (nStatus==ARNOLD_STATUS_OK)
        {
            LoadI(sFilename);
        }

        free(pDiskImageData);
    }
    return false;

}

bool DiskMedia::Save(const wxString &sFilename)
{
    unsigned long nDiskImage = DiskImage_CalculateOutputSize(m_nUnit, FALSE);

    unsigned char *pDiskImage = (unsigned char *)malloc(nDiskImage);

    if (pDiskImage)
    {
        DiskImage_GenerateOutputData(pDiskImage,m_nUnit, FALSE);

        wxGetApp().SaveLocalFile(sFilename, pDiskImage, nDiskImage);

        DiskImage_ResetDirty(m_nUnit);

        free(pDiskImage);
        return true;
    }

    return false;
}


void DiskMedia::Remove()
{
    DiskImage_RemoveDisk(m_nUnit);
    RemoveI();
}

bool DiskMedia::IsModified() const
{
    return DiskImage_IsImageDirty(m_nUnit);
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

bool CassetteMedia::Load(const wxString &sFilename)
{
    unsigned char *pTapeImageData = NULL;
    unsigned long TapeImageLength = 0;

    /* try to load it */
    wxGetApp().LoadLocalFile(sFilename, &pTapeImageData, &TapeImageLength);

    if (pTapeImageData!=NULL)
    {
        if (TapeImage_Insert(pTapeImageData, TapeImageLength)==ARNOLD_STATUS_OK)
        {
            LoadI(sFilename);

            free(pTapeImageData);
        }
        else
        {
            free(pTapeImageData);

            //      if (Sample_Load(sFilename.c_str()))
            //    {
            //  }
        }
    }
    return false;

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

bool CartridgeMedia::Load(const wxString &sFilename)
{
    bool bSuccess = false;
    unsigned char *pCartridgeData;
    unsigned long CartridgeLength;

    /* try to load it */
    wxGetApp().LoadLocalFile(sFilename, &pCartridgeData, &CartridgeLength);

    if (pCartridgeData!=NULL)
    {
        int nStatus = Cartridge_AttemptInsert(pCartridgeData, CartridgeLength);

        if (nStatus==ARNOLD_STATUS_OK)
        {
            LoadI(sFilename);
        }
        else
        {
            Cartridge_InsertBinary(pCartridgeData, CartridgeLength);

            /* if cartridge was inserted ok, then auto-start it */
            Cartridge_Autostart();

            LoadI(sFilename);
        }

        bSuccess = true;
        free(pCartridgeData);
    }
    return bSuccess;
}

void CartridgeMedia::Remove()
{
    Cartridge_Remove();
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

bool SnapshotMedia::Load(const wxString &sFilename)
{
    unsigned char *pSnapshotData;
    unsigned long SnapshotLength;

    /* try to load it */
    wxGetApp().LoadLocalFile(sFilename, &pSnapshotData, &SnapshotLength);

    if (pSnapshotData!=NULL)
    {
        if (Snapshot_Insert(pSnapshotData, SnapshotLength)==ARNOLD_STATUS_OK)
        {
            LoadI(sFilename);
        }

        free(pSnapshotData);
    }

    return false;
}

void arnguiApp::OpenPrinterFile(const wxString &sFilename)
{
    m_pPrinterOutputStream = new wxFileOutputStream(sFilename);
    if (m_pPrinterOutputStream!=NULL)
    {
        if (m_pPrinterOutputStream->IsOk())
        {
            Printer_SetBusyInput(FALSE);
            return;
        }
        delete m_pPrinterOutputStream;
        m_pPrinterOutputStream = NULL;
    }
}

//unsigned char PrinterStatus
//unsigned char PrinterStatusShift;

//insert digiblaster here
void AppPrinter_RefreshOutputs()
{

	//force digiblaster
	if (TRUE) Audio_Digiblaster_Write(Printer_Get8BitData());

    if (wxGetApp().m_pPrinterOutputStream!=NULL)
    {
        /* high to low causes data to be written */
        wxGetApp().m_PrinterData = Printer_Get8BitData();

       Printer_SetBusyInput(FALSE);
      #if 0
      switch ((wxGetApp().m_PrinterData & ((1<<2)|(1<<1))))
      {
        /* 2 low, 1 low */
          case 0:
          {
              PrinterStatusShift = 0;
          }
          break;

         case (1<<1):
         {
           printf("Set busy to true\n");
           /* indicates paper out */
            /* 2 is 0, 1 is 0 */
           Printer_SetBusyInput(TRUE);
         }
         break;

         case (1<<2)|(1<<1):
         {
            printf("other...");
         }
         break;

         default:
           printf("Set busy to false\n");
           Printer_SetBusyInput(FALSE);
         break;
       }
      #endif

      if (wxGetApp().m_bPrinterStrobe)
        {
            if (!Printer_GetStrobeOutputState())
            {
                wxGetApp().m_pPrinterOutputStream->PutC(wxGetApp().m_PrinterData);
                wxGetApp().m_bPrinterStrobe = false;
            }
        }
        else
        {
            if (Printer_GetStrobeOutputState())
            {
                wxGetApp().m_bPrinterStrobe = true;
            }
        }
    }

}

extern "C"
{
    //void Printer_RefreshOutputs()
    //{
    //    AppPrinter_RefreshOutputs();
    //}
}

void arnguiApp::ClosePrinterFile(void)
{
    if (m_pPrinterOutputStream!=NULL)
    {
        m_pPrinterOutputStream->Close();
        delete m_pPrinterOutputStream;
        m_pPrinterOutputStream = NULL;
        Printer_SetBusyInput(TRUE);
    }
}


void SnapshotMedia::Remove()
{
    RemoveI();
}

void arnguiApp::ScanConfigsAppThenLocal(const wxString &sConfig, wxArrayPtrVoid &configList)
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


void arnguiApp::ScanConfigs(const wxString &sDirectory, wxArrayPtrVoid &configList)
{
    if (!wxDir::Exists(sDirectory))
        return;

    wxDir dir(sDirectory);
    if ( !dir.IsOpened() )
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
    if (fileSystem!=NULL)
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
		wxMessageDialog *pMessage = new wxMessageDialog(GetTopWindow(), wxT("Are you sure you want to restart (Power method)?\nAll unsaved RAM contents will be lost."),wxApp::GetAppName(), wxYES_NO|wxNO_DEFAULT|wxICON_EXCLAMATION);;
		if (pMessage!=NULL)
		{
			if (pMessage->ShowModal()==wxID_YES)
			{
				bPerformRestart = true;
			}
			pMessage->Destroy();
		}
	}

	if (bPerformRestart)
	{
		/* clear autotype */
		AutoType_Finish();
		/* clear auto run */
		AutoRunFile_Finish();

		/* do reset */
		Computer_RestartPower();

        if (bSetBreakpoint)
        {
            Breakpoints_AddBreakpoint(BREAKPOINT_TYPE_PC,0);
        }
	}
}

void arnguiApp::RestartReset(bool bShowMessage, bool bSetBreakpoint)
{
    bool bPerformRestart = true;

	if (bShowMessage)
	{
		bPerformRestart = false;
		wxMessageDialog *pMessage = new wxMessageDialog(GetTopWindow(), wxT("Are you sure you want to restart (Reset method)?\nAll unsaved RAM contents will be lost."),wxApp::GetAppName(), wxYES_NO|wxNO_DEFAULT|wxICON_EXCLAMATION);;
		if (pMessage!=NULL)
		{
			if (pMessage->ShowModal()==wxID_YES)
			{
				bPerformRestart = true;
			}
			pMessage->Destroy();
		}
	}

	if (bPerformRestart)
	{
		/* clear autotype */
		AutoType_Finish();
		/* clear auto run */
		AutoRunFile_Finish();

		/* do reset */
		Computer_RestartReset();


        if (bSetBreakpoint)
        {
            Breakpoints_AddBreakpoint(BREAKPOINT_TYPE_PC,0);
        }
	}




}


bool arnguiApp::PickBuiltInRom(const wxString &sROMName, unsigned char **ppData, unsigned long *pLength)
{
    bool bPicked = false;

    // get list of build in roms
	wxArrayString sROMs;
	for (int i=0; i<wxGetApp().GetNumBuiltinRoms(); i++)
	{
		sROMs.Add(wxGetApp().GetBuiltinRomName(i));
	}

	wxString sMessage;
	sMessage = wxT("Choose built-in ROM for ") + sROMName;
	wxSingleChoiceDialog *pDialog = new wxSingleChoiceDialog(NULL, sMessage, wxT("Choose built-in ROM"), sROMs);
	pDialog->SetSelection(0);
	if (pDialog->ShowModal()==wxID_OK)
	{
		int nBuiltinRom = pDialog->GetSelection();
        bPicked = true;

		GetBuiltinRom(nBuiltinRom, ppData, pLength);
	}
	pDialog->Destroy();
    return bPicked;
}

void arnguiApp::SetKeyboardMode(int KeyboardMode)
{
    if ((KeyboardMode<0) || (KeyboardMode>1))
    {
      KeyboardMode = 0;
    }

    printf("Keyboard mode %s\n",KeyboardMode==0 ? "Positional" : "Translated");

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
  printf("Audio channels: %d\n", nChannels);
}

void arnguiApp::SetAudioFrequency(int nFrequency)
{
    m_nAudioFrequency = nFrequency;
  printf("Audio frequency: %d\n", nFrequency);
}

void arnguiApp::SetAudioBits(int nBits)
{
    m_nAudioBits = nBits;
  printf("Audio bits per sample: %d\n", nBits);
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
	  //m_SavePoint.GoForwards();
  }
  
  void arnguiApp::PrevSaveState()
  {
	  //m_SavePoint.GoBack();
  }
  
  void arnguiApp::SaveSaveState()
  {
	//m_SavePoint.AddSavepoint();
  }