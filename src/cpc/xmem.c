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
#include "emudevice.h"
/* NEEDS work! */
extern unsigned char *Z80MemoryBase;

int XMemRomSelect = 0;
int XMemRamSelect = 0;
unsigned char XMemRam[512*1024];
unsigned char XMemFlashRom[512*1024];

BOOL XMemRamSwitchState = TRUE;
BOOL XMemBootSwitchState = TRUE;
BOOL XMemRomLockSwitchState = FALSE;
BOOL XMemReadRomSwitchState = TRUE;
ExpansionRomData m_XMemRoms;

BOOL XMem_GetRamSwitch(void)
{
	return XMemRamSwitchState;
}
void XMem_SetRamSwitch(BOOL bState)
{
	XMemRamSwitchState = bState;
}


BOOL XMem_GetBootSwitch(void)
{
	return XMemBootSwitchState;
}

void XMem_SetBootSwitch(BOOL bState)
{
	XMemBootSwitchState = bState;
}


BOOL XMem_GetRomProtectSwitch(void)
{
	return XMemRomLockSwitchState;
}

void XMem_SetRomProtectSwitch(BOOL bState)
{
	XMemRomLockSwitchState = bState;
}



BOOL XMem_GetRomEnabledSwitch(void)
{
	return XMemReadRomSwitchState;
}

void XMem_SetRomEnabledSwitch(BOOL bState)
{
	XMemReadRomSwitchState = bState;
}

void XMem_Reset(void)
{
	XMemRamSelect = 0;
	XMemRomSelect = 0;
}

static EmuDeviceSwitch XMemSwitches[4]=
{
  {
      "Ram switch (On=6128, Off=464/664)",
	  "RamSwitch",
      XMem_GetRamSwitch,
      XMem_SetRamSwitch
  },
  {
      "Boot (On=X-Mem, Off=CPC)",
	  "Boot",
      XMem_GetBootSwitch,
      XMem_SetBootSwitch
  },
  {
      "Rom lock/protect (On=Yes, Off=No)",
	  "RomProtect",
      XMem_GetRomProtectSwitch,
      XMem_SetRomProtectSwitch
  },
  {
      "Rom enabled (On=Yes, Off=No)",
	  "RomEnable",
      XMem_GetRomEnabledSwitch,
      XMem_SetRomEnabledSwitch
  }
};


void	XMem_ROM_Write(Z80_WORD Port, Z80_BYTE Data)
{
	XMemRomSelect = Data;
	
    Computer_RethinkMemory();	
}

void XMem_RAM_RestoreFromSnapshot(int Config)
{
	XMemRamSelect = Config;
}


void	XMem_RAM_Write(Z80_WORD Port, Z80_BYTE Data)
{
	XMemRamSelect = Data;
	
    Computer_RethinkMemory();	
}


void XMem_MemoryRethink(MemoryData *pData)
{
	int XMemRamBank = ((XMemRamSelect >> 3) & 0x07);

	switch (XMemRamSelect & 0x07)
	{
		/* nothing */
	case 0:
		break;

	case 2:
	{
		int i;
		/* complete switch */
		for (i = 0; i < 4; i++)
		{
			int nPage = i;
			int nRamOffset = (i << 14);
			int nOffset = nRamOffset - (nPage << 14);
			pData->pWritePtr[(i << 1) + 0] = XMemRam + (XMemRamBank << 16) - nOffset;
			pData->pWritePtr[(i << 1) + 1] = pData->pWritePtr[(i << 1) + 0];
			if (!pData->bRomEnable[(i << 1) + 0])
			{
				pData->pReadPtr[(i << 1) + 0] = pData->pWritePtr[(i << 1) + 0];
			}

			if (!pData->bRomEnable[(i << 1) + 1])
			{
				pData->pReadPtr[(i << 1) + 1] = pData->pWritePtr[(i << 1) + 1];
			}
			pData->bRamDisable[(i << 1) + 0] = TRUE;
			pData->bRamDisable[(i << 1) + 1] = TRUE;
		}
	}
	break;

	case 1:
	{
		pData->pWritePtr[6] = (XMemRam + (XMemRamBank << 16)) + (3 << 14) - 0x0c000;
		pData->pWritePtr[7] = pData->pWritePtr[6];
		if (!pData->bRomEnable[7])
		{
			pData->pReadPtr[6] = pData->pWritePtr[6];
		}
		if (!pData->bRomEnable[6])
		{
			pData->pReadPtr[7] = pData->pWritePtr[7];
		}
		pData->bRamDisable[6] = TRUE;
		pData->bRamDisable[7] = TRUE;
	}
	break;

	case 3:
	{
		/* pal can do this because it swaps a14/a15 to ram */

		/* page 7 appears at position 3 */
		pData->pWritePtr[7] = XMemRam + (XMemRamBank << 16) + (3 << 14) - 0x0c000;
		pData->pWritePtr[6] = pData->pWritePtr[7];
		if (!pData->bRomEnable[7])
		{
			pData->pReadPtr[7] = pData->pWritePtr[7];
		}
		if (!pData->bRomEnable[6])
		{
			pData->pReadPtr[6] = pData->pWritePtr[6];
		}
		pData->bRamDisable[7] = TRUE;
		pData->bRamDisable[6] = TRUE;

		/* normal page 3 ram appears in range &4000-&7fff */
		pData->pWritePtr[2] = Z80MemoryBase + (3 << 14) - 0x04000;
		pData->pWritePtr[3] = pData->pWritePtr[2];

		pData->pReadPtr[2] = pData->pWritePtr[2];
		pData->pReadPtr[3] = pData->pWritePtr[3];
		pData->bRamDisable[2] = TRUE;
		pData->bRamDisable[3] = TRUE;
	}
	break;

	case 4:
	case 5:
	case 6:
	case 7:
	{
		/* 4000-7fff only */
		pData->pWritePtr[2] = XMemRam + (XMemRamBank << 16) + ((XMemRamSelect & 0x03) << 14) - 0x04000;
		pData->pWritePtr[3] = pData->pWritePtr[2];

		pData->pReadPtr[2] = pData->pWritePtr[2];
		pData->pReadPtr[3] = pData->pWritePtr[3];
		pData->bRamDisable[2] = TRUE;
		pData->bRamDisable[3] = TRUE;
	}
	break;
	}


	/* boot from x-mem and rom enabled? */
	/* TODO: Check, is rom 0 also disabled? */
	if (XMemReadRomSwitchState && XMemBootSwitchState)
	{
		/* firmware is visible */
		const unsigned char *pRomData = &XMemFlashRom[(7 << 14)];
		
		pData->pReadPtr[0] = pRomData;
		pData->pReadPtr[1] = pRomData;
		pData->bRomDisable[1] = TRUE;
		pData->bRomDisable[0] = TRUE;
	}
	
	/* ignore rom? x-mem ignores rom 7 but allows all others */
    if (XMemReadRomSwitchState && ((XMemRomSelect & 0x0c0)==0x0) && ((XMemRomSelect&0x01f)!=7))
    {
		const unsigned char *pRomData = &XMemFlashRom[(XMemRomSelect & 0x01f) << 14]-0x0c000;
		
		if (pData->bRomEnable[6] && !pData->bRomDisable[6])
		{
			pData->bRomDisable[6] = TRUE;
			pData->pReadPtr[6] = pRomData;
		}
		if (pData->bRomEnable[7] && !pData->bRomDisable[7])
		{
			pData->bRomDisable[7] = TRUE;
			pData->pReadPtr[7] = pRomData;
		}
    }
}
/* TotO's comments in cpcwiki:

ODD ($7Fxx) = X-MEM
EVEN ($7Exx) = CPC 6128 Expansion or Y-MEM
*/

CPCPortWrite XMemPortWrite[2]=
{
	{
	0x8100,            /* and */
	0x0100,            /* compare */
	XMem_RAM_Write
	},
	{
	0x02000,            /* and */
	0x00000,            /* compare */
	XMem_ROM_Write
	}
};


void XMem_ClearExpansionRom(int RomIndex)
{
	memset(XMemFlashRom + (RomIndex << 14), 0x0ff, 16384);
}

BOOL XMem_SetExpansionRom(int RomIndex, const unsigned char *pData, unsigned long Length)
{
	EmuDevice_CopyRomData(XMemFlashRom + (RomIndex << 14), 16384, pData, Length);
	return TRUE;
}


static DkRamData XMemRamData;

void XMem_InitDevice(void)
{
	int i;

	memset(XMemFlashRom, 0x0ff, sizeof(XMemFlashRom));
	memset(XMemRam, 0x0ff, sizeof(XMemRam));

	ExpansionRom_Init(&m_XMemRoms, XMem_ClearExpansionRom, XMem_SetExpansionRom);

	for (i = 0; i < 32; i++)
	{
		ExpansionRom_SetAvailableState(&m_XMemRoms, i, TRUE);
	}
	for (i = 0; i < MAX_DKRAM_PAGES; i++)
	{
		XMemRamData.PageAvailable[i] = TRUE;
		XMemRamData.Pages[i] = XMemRam + (i << 14);
	}

	
}

static EmuDevice XMemDevice=
{
	NULL,
	XMem_InitDevice,
	NULL,
	"XMEM",
	"XMem",
	"ToTo's X-Mem",
    CONNECTION_EXPANSION,   /* connects to expansion */
	DEVICE_FLAGS_HAS_DKTRONICS_RAM,                   
    0,                /* no read ports */
  NULL,
  sizeof(XMemPortWrite)/sizeof(XMemPortWrite[0]),                    /* 1 write ports */
  XMemPortWrite,
  0,                /* no memory read*/
  NULL,
  0,                /* no memory write */
  NULL,
  XMem_Reset, /* no reset function */
  XMem_MemoryRethink, /* no rethink function */
  XMem_Reset, /* no power function */
	sizeof(XMemSwitches)/sizeof(XMemSwitches[0]),                      /* no switches */
	XMemSwitches,
    0,                      /* no buttons */
    NULL,
    0,                      /* no onboard roms */
    NULL,
	NULL,	/* no cursor update function */
	&m_XMemRoms, /* rom slots */
	NULL,	/* printer */
	NULL, /* joystick */
	  0,
	  NULL,
	  NULL, /* sound */
	  NULL, /* lpen */
	  NULL, /* reti */
	  NULL, /* ack maskable interrupt */
	  &XMemRamData, /* dkram data */
	  NULL, /* device ram */
	  NULL, /* device backup */
	  XMem_RAM_RestoreFromSnapshot
};

void XMem_Init(void)
{
	RegisterDevice(&XMemDevice);
}










