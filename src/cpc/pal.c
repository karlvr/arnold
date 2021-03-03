/* FIX INTERNAL VS EXTERNAL */

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
#include "cpc.h"
#include "pal.h"
#include "emudevice.h"
#include "memrange.h"
#include "riff.h"
extern unsigned char *Z80MemoryBase;


void PALData_Reset(PAL16L8Data *pData)
{
	pData->RamConfig = 0;
	Computer_RethinkMemory();
}
	
void PALData_Update(MemoryData *pData, PAL16L8Data *pPALData)
{
	/* note doesn't really use ROMEN, uses RAMDIS, A14, A15 */
	switch (pPALData->RamConfig & 0x07)
	{
		/* nothing */
	case 0:
		break;

	case 2:
	{
		int i;
		unsigned char *pAddr = pPALData->pRamPtr + (pPALData->Bank << 16);

		/* complete switch */
		for (i = 0; i < 8; i++)
		{
			if (!pData->bRamDisable[i])
			{
				pData->pWritePtr[i] = pAddr;
				if (!pData->bRomEnable[i])
				{
					pData->pReadPtr[i] = pData->pWritePtr[i];
				}
				pData->bRamDisable[i] = TRUE;
			}
		}

	}
	break;

	case 1:
	{
		if (!pData->bRamDisable[7])
		{
			pData->pWritePtr[7] = (pPALData->pRamPtr + (pPALData->Bank << 16)) + (3 << 14) - 0x0c000;
			if (!pData->bRomEnable[7])
			{
				pData->pReadPtr[7] = pData->pWritePtr[7];
			}
			pData->bRamDisable[7] = TRUE;
		}

		if (!pData->bRamDisable[6])
		{
			pData->pWritePtr[6] = (pPALData->pRamPtr + (pPALData->Bank << 16)) + (3 << 14) - 0x0c000;
			if (!pData->bRomEnable[6])
			{
				pData->pReadPtr[6] = pData->pWritePtr[6];
			}
			pData->bRamDisable[6] = TRUE;
		}
	}
	break;

	case 3:
	{
		/* pal can do this because it swaps a14/a15 to ram */

		/* page 7 appears at position 3 */
		if (!pData->bRamDisable[7])
		{
			pData->pWritePtr[7] = pPALData->pRamPtr + (pPALData->Bank << 16) + (3 << 14) - 0x0c000;
			if (!pData->bRomEnable[7])
			{
				pData->pReadPtr[7] = pData->pWritePtr[7];
			}
			pData->bRamDisable[7] = TRUE;
		}

		if (!pData->bRamDisable[6])
		{
			pData->pWritePtr[6] = pPALData->pRamPtr + (pPALData->Bank << 16) + (3 << 14) - 0x0c000;
			if (!pData->bRomEnable[6])
			{
				pData->pReadPtr[6] = pData->pWritePtr[6];
			}
			pData->bRamDisable[6] = TRUE;
		}

		if (!pData->bRamDisable[3])
		{
			pData->pWritePtr[3] = Z80MemoryBase + (3 << 14) - 0x04000;
			pData->pReadPtr[3] = pData->pWritePtr[3];
			pData->bRamDisable[3] = TRUE;
		}

		if (!pData->bRamDisable[2])
		{
			pData->pWritePtr[2] = Z80MemoryBase + (3 << 14) - 0x04000;
			pData->pReadPtr[2] = pData->pWritePtr[2];
			pData->bRamDisable[2] = TRUE;
		}
	}
	break;

	case 4:
	case 5:
	case 6:
	case 7:
	{
		if (!pData->bRamDisable[2])
		{
			/* 4000-7fff only */
			pData->pWritePtr[2] = pPALData->pRamPtr + (pPALData->Bank << 16) + ((pPALData->RamConfig & 0x03) << 14) - 0x04000;
			pData->pReadPtr[2] = pData->pWritePtr[2];
			pData->bRamDisable[2] = TRUE;
		}

		if (!pData->bRamDisable[3])
		{
			pData->pWritePtr[3] = pPALData->pRamPtr + (pPALData->Bank << 16) + ((pPALData->RamConfig & 0x03) << 14) - 0x04000;
			pData->pReadPtr[3] = pData->pWritePtr[3];
			pData->bRamDisable[3] = TRUE;
		}
	}
	break;
	}
}

static PAL16L8Data PAL16L8;

const char *sPAL16L8MemoryRangeNames[65536 / 16384] =
{
	"PAL16L8 Page 0",
	"PAL16L8 Page 1",
	"PAL16L8 Page 2",
	"PAL16L8 Page 3",
};



void PAL16L8_Reset(void)
{
	PALData_Reset(&PAL16L8);
}

void PAL16L8_MemoryRethink(MemoryData *pData)
{
	PALData_Update(pData, &PAL16L8);
}

static void PAL16L8_PortWrite(Z80_WORD Port, Z80_BYTE Data)
{
	/* TODO: Confirm */
	if ((Data & 0x0c0) == 0x0c0)
	{
		PAL16L8.Bank = 0;
		PAL16L8.RamConfig = Data;

		Computer_RethinkMemory();
	}
}



/* TODO: Confirm */
CPCPortWrite PAL16L8PortWrite[1]=
{
	{
		0x08000,
		0x00000,
		PAL16L8_PortWrite
	}
};

void PAL16L8_RestoreFromSnapshot(int Config)
{
	PAL16L8.Bank = 0;
	PAL16L8.RamConfig = (Config & 0x07);
}

static MemoryRange PAL16L8_MemoryRanges[65536 / 16384];
static DkRamData PAL16L8DkRamData;

void PAL16L8Device_Init(void)
{
	int i;
	PAL16L8.Bank = 0;
	PAL16L8.pRamPtr = (unsigned char *)malloc(65536);
	memset(PAL16L8.pRamPtr, 0x0ff, 65536);
	for (i = 0; i < (65536 / 16384); i++)
	{
		PAL16L8_MemoryRanges[i].sName = sPAL16L8MemoryRangeNames[i];
		PAL16L8_MemoryRanges[i].m_nID = RIFF_FOURCC_CODE('P', 'A', 'L', '0' + i);
		PAL16L8_MemoryRanges[i].m_bCPU = FALSE;
		PAL16L8_MemoryRanges[i].m_bReadOnly = FALSE;
		PAL16L8_MemoryRanges[i].pBase = PAL16L8.pRamPtr + (i * 16384);
		PAL16L8_MemoryRanges[i].m_nLength = 16384;
	}
	for (i = 0; i < MAX_DKRAM_PAGES; i++)
	{
		BOOL bEnabled = (i < 4);
		PAL16L8DkRamData.PageAvailable[i] = bEnabled;
		if (bEnabled)
		{
			PAL16L8DkRamData.Pages[i] = PAL16L8.pRamPtr + (i << 14);
		}
	}

}

void PAL16L8Device_Shutdown(void)
{
	if (PAL16L8.pRamPtr)
	{
		free(PAL16L8.pRamPtr);
	}
}



static EmuDevice PAL16L8Device =
{
	NULL,
	PAL16L8Device_Init,
	PAL16L8Device_Shutdown,
	"PAL16L8",
	"Pal16l8",
	"PAL16L8 (CPC6128 extra 64KB RAM)",
	CONNECTION_INTERNAL,   /* connects to expansion */
	DEVICE_FLAGS_HAS_DKTRONICS_RAM | DEVICE_FLAGS_TESTED | DEVICE_WORKING,
	0,
	NULL,					/* no read ports */
	1,
	PAL16L8PortWrite,			/* 1 write port */
	0,                /* no memory read*/
	NULL,
	0,                /* no memory write */
	NULL, 
	PAL16L8_Reset,
	PAL16L8_MemoryRethink,
	PAL16L8_Reset,
	0,                      /* no switches */
	NULL,
	0,                      /* 2 buttons */
	NULL,
	0,                      /* 1 onboard roms */
	NULL,
	NULL,                   /* no cursor function */
	NULL,                   /* no generic roms */
	NULL,					/* printer */
	NULL,					/* joystick */
	sizeof(PAL16L8_MemoryRanges) / sizeof(PAL16L8_MemoryRanges[0]),
	PAL16L8_MemoryRanges,
	NULL, /* sound */
	NULL, /* lpen */
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	&PAL16L8DkRamData, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
	PAL16L8_RestoreFromSnapshot
};

int PAL16L8_Init(void)
{
	return RegisterDevice(&PAL16L8Device);
}

MemoryRange Yarek4MB_MemoryRanges[(4096*1024) / 16384];
static PAL16L8Data Yarek4MB;

static void Yarek4MB_PortWrite(Z80_WORD Port, Z80_BYTE Data)
{
	if ((Data & 0x0c0) == 0x0c0)
	{
		Yarek4MB.Bank = (((Port >> 8) & 0x07) << 3) | ((Data >> 3) & 0x07);
		Yarek4MB.RamConfig = Data & 0x07;


		Computer_RethinkMemory();
	}
}


CPCPortWrite Yarek4MBPortWrite[1] =
{
	{
	/* confirmed by TFM */
	0x08000,
	0x00000,
	Yarek4MB_PortWrite
	}
};

void Yarek4MB_Reset(void)
{
	PALData_Reset(&Yarek4MB);
	/* bank reset? */
}

void Yarek4MB_MemoryRethink(MemoryData *pData)
{
	PALData_Update(pData, &Yarek4MB);
}

static DkRamData Yarek4MBRamData;

void Yarek4MBDevice_Init(void)
{
	int i;

	Yarek4MB.pRamPtr = (unsigned char *)malloc(4096 * 1024);
	memset(Yarek4MB.pRamPtr, 0x0ff, 4096 * 1024);

	for (i = 0; i < (4096 * 1024) / 16384; i++)
	{
		Yarek4MB_MemoryRanges[i].sName = "Yarek 4MB Page";
		Yarek4MB_MemoryRanges[i].m_nID = RIFF_FOURCC_CODE('Y', '4', 'A' + (i / 16), 'A' + (i & 0x0f));
		Yarek4MB_MemoryRanges[i].m_bCPU = FALSE;
		Yarek4MB_MemoryRanges[i].m_bReadOnly = FALSE;
		Yarek4MB_MemoryRanges[i].pBase = Yarek4MB.pRamPtr + (i * 16384);
		Yarek4MB_MemoryRanges[i].m_nLength = 16384;
	}
	
	for (i = 0; i < MAX_DKRAM_PAGES; i++)
	{
		Yarek4MBRamData.PageAvailable[i] = TRUE;
		Yarek4MBRamData.Pages[i] = Yarek4MB.pRamPtr + (i << 14);
	}
}

void Yarek4MBDevice_Shutdown(void)
{
	if (Yarek4MB.pRamPtr)
	{
		free(Yarek4MB.pRamPtr);
	}
}

static EmuDevice Yarek4MBDevice =
{
	NULL,
	Yarek4MBDevice_Init,
	Yarek4MBDevice_Shutdown,
	"YAREK4MB",
	"YarekInternal4Mb",
	"Yarek's 4MB Internal Ram Expansion (CPC6128)",
	CONNECTION_INTERNAL,   /* connects to expansion */
	DEVICE_FLAGS_HAS_DKTRONICS_RAM|DEVICE_FLAGS_TESTED|DEVICE_WORKING,
	0,
	NULL,					/* no read ports */
	sizeof(Yarek4MBPortWrite)/sizeof(Yarek4MBPortWrite[0]),
	Yarek4MBPortWrite,			/* 1 write port */
	0,                /* no memory read*/
	NULL,
	0,                /* no memory write */
	NULL, 
	Yarek4MB_Reset,
	Yarek4MB_MemoryRethink,
	Yarek4MB_Reset,
	0,                      /* no switches */
	NULL,
	0,                      /* 2 buttons */
	NULL,
	0,                      /* 1 onboard roms */
	NULL,
	NULL,                   /* no cursor function */
	NULL,                   /* no generic roms */
	NULL,					/* printer */
	NULL,					/* joystick */
	sizeof(Yarek4MB_MemoryRanges) / sizeof(Yarek4MB_MemoryRanges[0]),
	Yarek4MB_MemoryRanges,
	NULL, /* sound */
	NULL, /* lpen */
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	&Yarek4MBRamData, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
	NULL,
};

void Yarek4MB_Init(void)
{

	RegisterDevice(&Yarek4MBDevice);
}

static PAL16L8Data DkTronics256KBRamPAL;

void DkTronics256KBRam_RestoreFromSnapshot(int Config)
{
	DkTronics256KBRamPAL.Bank = (Config>>3)&0x03;
	DkTronics256KBRamPAL.RamConfig = (Config & 0x07);
}

void DkTronics256KBRam_Reset(void)
{
	PALData_Reset(&DkTronics256KBRamPAL);
}

void DkTronics256KBRam_MemoryRethink(MemoryData *pData)
{
	PALData_Update(pData, &DkTronics256KBRamPAL);
}

static void DkTronics256KBRam_PortWrite(Z80_WORD Port, Z80_BYTE Data)
{
	/* TODO: Confirm */
	if ((Data & 0x0c0) == 0x0c0)
	{
		DkTronics256KBRamPAL.Bank = (Data>>3)&0x03;
		DkTronics256KBRamPAL.RamConfig = Data&0x07;

		Computer_RethinkMemory();
	}
}



/* TODO: Confirm */
CPCPortWrite DkTronics256KBRamPortWrite[1] =
{
	{
		0x08000,
		0x00000,
		DkTronics256KBRam_PortWrite
	}
};

static DkRamData DkTronics256KBRamData;


void DkTronics256KBRamDevice_Init(void)
{
	int i;
	DkTronics256KBRamPAL.Bank = 0;
	DkTronics256KBRamPAL.pRamPtr = (unsigned char *)malloc(256*1024);
	memset(DkTronics256KBRamPAL.pRamPtr, 0x0ff, 256*1024);

	#if 0
	for (i = 0; i < (65536 / 16384); i++)
	{
		DkTronics256KBRam_MemoryRanges[i].sName = sPAL16L8MemoryRangeNames[i];
		DkTronics256KBRam_MemoryRanges[i].m_nID = RIFF_FOURCC_CODE('P', 'A', 'L', '0' + i);
		DkTronics256KBRam_MemoryRanges[i].m_bCPU = FALSE;
		DkTronics256KBRam_MemoryRanges[i].m_bReadOnly = FALSE;
		DkTronics256KBRam_MemoryRanges[i].pBase = DkTronics256KBRamPAL.pRamPtr + (i * 16384);
		DkTronics256KBRam_MemoryRanges[i].m_nLength = 16384;
	}
#endif
	for (i = 0; i < MAX_DKRAM_PAGES; i++)
	{
		BOOL bEnabled = (i < 16);
		DkTronics256KBRamData.PageAvailable[i] = bEnabled;
		if (bEnabled)
		{
			DkTronics256KBRamData.Pages[i] = DkTronics256KBRamPAL.pRamPtr + (i << 14);
		}
	}
}

void DkTronics256KBRamDevice_Shutdown(void)
{
	if (DkTronics256KBRamPAL.pRamPtr)
	{
		free(DkTronics256KBRamPAL.pRamPtr);
	}
}

static EmuDevice DkTronics256KBRamDevice =
{
	NULL,
	DkTronics256KBRamDevice_Init,
	DkTronics256KBRamDevice_Shutdown,
	"DK256KBRAM",
	"DK256KBRAM",
	"Dk'Tronics 256KB RAM",
	CONNECTION_EXPANSION,   /* connects to expansion */
	DEVICE_FLAGS_HAS_DKTRONICS_RAM,
	0,
	NULL,					/* no read ports */
	1,
	DkTronics256KBRamPortWrite,			/* 1 write port */
	0,                /* no memory read*/
	NULL,
	0,                /* no memory write */
	NULL,
	DkTronics256KBRam_Reset,
	DkTronics256KBRam_MemoryRethink,
	DkTronics256KBRam_Reset,
	0,                      /* no switches */
	NULL,
	0,                      /* 2 buttons */
	NULL,
	0,                      /* 1 onboard roms */
	NULL,
	NULL,                   /* no cursor function */
	NULL,                   /* no generic roms */
	NULL,					/* printer */
	NULL,					/* joystick */
	0,
	NULL,
	NULL, /* sound */
	NULL, /* lpen */
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	&DkTronics256KBRamData, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
	DkTronics256KBRam_RestoreFromSnapshot
};

int DkTronics256KBRam_Init(void)
{
	return RegisterDevice(&DkTronics256KBRamDevice);
}

static PAL16L8Data DkTronics256KBSiliconDiskPAL;

void DkTronics256KBSiliconDisk_Reset(void)
{
	PALData_Reset(&DkTronics256KBSiliconDiskPAL);
}

void DkTronics256KBSiliconDisk_MemoryRethink(MemoryData *pData)
{
	PALData_Update(pData, &DkTronics256KBSiliconDiskPAL);
}

static void DkTronics256KBSiliconDisk_PortWrite(Z80_WORD Port, Z80_BYTE Data)
{
	/* TODO: Confirm */
	if ((Data & 0x0c0) == 0x0c0)
	{
		DkTronics256KBSiliconDiskPAL.Bank = (Data >> 4) & 0x03;
		DkTronics256KBSiliconDiskPAL.RamConfig = Data&0x07;

		Computer_RethinkMemory();
	}
}



/* TODO: Confirm */
CPCPortWrite DkTronics256KBSiliconDiskPortWrite[1] =
{
	{
		0x08000,
		0x00000,
		DkTronics256KBSiliconDisk_PortWrite
	}
};

static DkRamData DkTronics256KBSiliconDiskRamData;

void DkTronics256KBSiliconDisk_RestoreFromSnapshot(int Config)
{
	DkTronics256KBSiliconDiskPAL.Bank = (Config >> 4) & 0x03;
	DkTronics256KBSiliconDiskPAL.RamConfig = (Config & 0x07);
}

void DkTronics256KBSiliconDiskDevice_Init(void)
{
	int i;
	DkTronics256KBSiliconDiskPAL.Bank = 0;
	DkTronics256KBSiliconDiskPAL.pRamPtr = (unsigned char *)malloc(256 * 1024);
	memset(DkTronics256KBSiliconDiskPAL.pRamPtr, 0x0ff, 256 * 1024);
	for (i = 0; i < MAX_DKRAM_PAGES; i++)
	{
		BOOL bEnabled = (i > 16);
		DkTronics256KBSiliconDiskRamData.PageAvailable[i] = bEnabled;
		if (bEnabled)
		{
			DkTronics256KBSiliconDiskRamData.Pages[i] = DkTronics256KBSiliconDiskPAL.pRamPtr + (i << 14);
		}
	}
}

void DkTronics256KBSiliconDiskDevice_Shutdown(void)
{
	if (DkTronics256KBSiliconDiskPAL.pRamPtr)
	{
		free(DkTronics256KBSiliconDiskPAL.pRamPtr);
	}
}

static EmuDevice DkTronics256KBSiliconDiskDevice =
{
	NULL,
	DkTronics256KBSiliconDiskDevice_Init,
	DkTronics256KBSiliconDiskDevice_Shutdown,
	"DK256KBSILICONDISK",
	"DK256KBSILICONDISK",
	"Dk'Tronics 256KB Silicon Disk",
	CONNECTION_EXPANSION,   /* connects to expansion */
	DEVICE_FLAGS_HAS_DKTRONICS_RAM,
	0,
	NULL,					/* no read ports */
	1,
	DkTronics256KBSiliconDiskPortWrite,			/* 1 write port */
	0,                /* no memory read*/
	NULL,
	0,                /* no memory write */
	NULL,
	DkTronics256KBSiliconDisk_Reset,
	DkTronics256KBSiliconDisk_MemoryRethink,
	DkTronics256KBSiliconDisk_Reset,
	0,                      /* no switches */
	NULL,
	0,                      /* 2 buttons */
	NULL,
	0,                      /* 1 onboard roms */
	NULL,
	NULL,                   /* no cursor function */
	NULL,                   /* no generic roms */
	NULL,					/* printer */
	NULL,					/* joystick */
	0,
	NULL,
	NULL, /* sound */
	NULL, /* lpen */
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	&DkTronics256KBSiliconDiskRamData, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
	DkTronics256KBSiliconDisk_RestoreFromSnapshot
};

int DkTronics256KBSiliconDisk_Init(void)
{
	return RegisterDevice(&DkTronics256KBSiliconDiskDevice);
}
