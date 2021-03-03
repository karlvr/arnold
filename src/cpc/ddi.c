#include "cpcglob.h"
#include "cpc.h"
#include "ddi.h"
#include "fdi.h"
#include "fdc.h"

static BOOL Amstrad_DiscInterface_Enabled = FALSE;

/*
//void Amstrad_DiscInterface_Install(void)
//{
//	Amstrad_DiscInterface_Enabled = TRUE;
//}

//void Amstrad_DiscInterface_DeInstall(void)
//{
//	Amstrad_DiscInterface_Enabled = FALSE;
//}
*/

void Amstrad_DiscInterface_Enable(BOOL bState)
{
	Amstrad_DiscInterface_Enabled = bState;
}

/*
//void Amstrad_DiscInterface_DeInstall(void)
//{
//	Amstrad_DiscInterface_Enabled = FALSE;
//}
*/


/* dos rom */
static const unsigned char *Amstrad_DiscInterface_pROM;


void Amstrad_DiscInterface_PortWrite(Z80_WORD Port, Z80_BYTE Data)
{
	unsigned int            Index;

	Index = ((Port & 0x0100) >> (8 - 1)) | (Port & 0x01);

	switch (Index)
	{
	case 0:
	case 1:
	{
		FDI_SetMotorState(Data);
	}
	break;

	case 2:
	case 3:
	{
		FDC_WriteDataRegister(Data);
	}
	break;

	default
		:
			break;
	}
}

BOOL Amstrad_DiscInterface_PortRead(Z80_WORD Port, Z80_BYTE *pDeviceData)
{
	unsigned int            Index;

	Index = ((Port & 0x0100) >> (8 - 1)) | (Port & 0x01);

	switch (Index)
	{
	case 2:
	{
		*pDeviceData = FDC_ReadMainStatusRegister();
	}
	return TRUE;

	case 3:
	{
		*pDeviceData =  FDC_ReadDataRegister();
	}
	return TRUE;

	default
		:
			break;
	}

	return FALSE;
}


void Amstrad_DiscInterface_SetRom(const unsigned char *pRom)
{
	Amstrad_DiscInterface_pROM = pRom;
}


static BOOL Amstrad_DiscInterface_RomEnabled = FALSE;

void AmstradDiscInterface_ROMSelect(Z80_WORD Addr, Z80_BYTE Data)
{
	Amstrad_DiscInterface_RomEnabled = FALSE;

	if (FDI_GetForceDiscRomOff())
	{
		return;
	}

	if (CPC_GetExpLow())
	{
		if (Data == 7)
		{
			Amstrad_DiscInterface_RomEnabled = TRUE;
		}
	}
	else
	{
		if (Data == 0)
		{
			Amstrad_DiscInterface_RomEnabled = TRUE;
		}
	}
	Computer_RethinkMemory();
}

CPCPortWrite AmstradDiscInterfaceROMSelect =
{
	0x02000,
	0x00000,
	AmstradDiscInterface_ROMSelect
};

CPCPortWrite AmstradDiscInterfacePortWrite =
{
	0x0480,
	0x0000,
	Amstrad_DiscInterface_PortWrite
};


CPCPortRead AmstradDiscInterfacePortRead =
{
	0x0480,
	0x0000,
	Amstrad_DiscInterface_PortRead
};

void Amstrad_DiscInterface_MemoryRethink(MemoryData *pData)
{
	const unsigned char *pRom = Amstrad_DiscInterface_pROM;
	if (CPC_GetAmsdosOverrideROMEnable())
	{
		/* override rom set? */
		pRom = CPC_GetAmsdosOverrideROM();
	}

	/* uses ROMEN */
	/* TODO change order so that disc rom is not overridden */
	/* if rom is enabled, and disc rom is selected */
	if (pData->bRomEnable[6] && Amstrad_DiscInterface_RomEnabled && (pRom != NULL))
	{
		/* Disable internal rom */
		pData->bRomDisable[7] = TRUE;
		pData->bRomDisable[6] = TRUE;

		/* set disc rom readable in memory */
		pData->pReadPtr[7] = pRom - 0x0c000;
		pData->pReadPtr[6] = pRom - 0x0c000;
	}
}

void Amstrad_DiscInterface_Install(void)
{
	CPC_InstallWritePort(&AmstradDiscInterfaceROMSelect);
	CPC_InstallWritePort(&AmstradDiscInterfacePortWrite);
	CPC_InstallMemoryRethinkHandler(Amstrad_DiscInterface_MemoryRethink);
	CPC_InstallReadPort(&AmstradDiscInterfacePortRead);
}

void Amstrad_DiscInterface_Uninstall(void)
{
	CPC_UninstallWritePort(&AmstradDiscInterfaceROMSelect);
	CPC_UninstallWritePort(&AmstradDiscInterfacePortWrite);
	CPC_UninstallReadPort(&AmstradDiscInterfacePortRead);
	CPC_UnInstallMemoryRethinkHandler(Amstrad_DiscInterface_MemoryRethink);
}
