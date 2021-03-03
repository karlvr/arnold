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
#include "ramrom.h"
#include <memory.h>
#include "emudevice.h"

/*************************/
/**** INICRON RAM-ROM ****/

/* buffer to hold ram contents; split into banks of 16k */
static unsigned char	*RAM_ROM_RAM = NULL;
/* an array of bits, a bit will be 1 if the corresponding bank is enabled,
and 0 if the corresponding bank is disabled */
static unsigned char	*RAM_ROM_BankEnables = NULL;
/* rom select index for eprom onboard ram-rom (not emulated) */
static unsigned char	RAM_ROM_EPROM_Bank = 0;
/* flags about the ram-rom status */
static unsigned char	RAM_ROM_Flags = 0;
/* number of banks the ram-rom holds */
static unsigned long	RAM_ROM_NumBlocks;
/* the mask will be 0 if ram is write disabled and 0x0ffffffff if the ram is write enabled */
static BOOL				RAM_ROM_WriteEnabled;
static BOOL             RAM_ROM_Active = FALSE;
static unsigned long RAM_ROM_RomSelected;

/* TODO: Need common expansion rom support allowing you to write into the segments

TODO: Eprom support.
TODO: Eprom Segment support for 8 to 64k EPROM
TODO: RAM-ROM enables for 7-15. DONE.
*/
BOOL	RAM_ROM_IsRamOn(void)
{
	return ((RAM_ROM_Flags & RAM_ROM_FLAGS_RAM_ON)!=0);
}

/* set if ram-rom ram is enabled and therefore if roms can be seen */
void	RAM_ROM_SetRamOnState(BOOL State)
{
	if (State)
	{
		RAM_ROM_Flags |= RAM_ROM_FLAGS_RAM_ON;
	}
	else
	{
		RAM_ROM_Flags &= ~RAM_ROM_FLAGS_RAM_ON;
	}

}

/* set read/write state for whole ram */
void	RAM_ROM_SetRamWriteEnableState(BOOL State)
{
	if (State)
	{
		RAM_ROM_Flags |= RAM_ROM_FLAGS_RAM_WRITE_ENABLE;
	}
	else
	{
		RAM_ROM_Flags &= ~RAM_ROM_FLAGS_RAM_WRITE_ENABLE;
	}
}

/* set if eprom on ram-rom is visible */
void	RAM_ROM_SetEPROMOnState(BOOL State)
{
	if (State)
	{
		RAM_ROM_Flags |= RAM_ROM_FLAGS_EPROM_ON;
	}
	else
	{
		RAM_ROM_Flags &= ~RAM_ROM_FLAGS_EPROM_ON;
	}

}

/* true if ram is write enabled, false if ram is write disabled */
BOOL	RAM_ROM_IsRamWriteEnabled(void)
{
	return ((RAM_ROM_Flags & RAM_ROM_FLAGS_RAM_WRITE_ENABLE)!=0);
}

/* true if rom is on and visible, false if off */
BOOL	RAM_ROM_IsEPROMOn(void)
{
	return ((RAM_ROM_Flags & RAM_ROM_FLAGS_EPROM_ON)!=0);
}

/* get selection value for rom */
int		RAM_ROM_GetEPROMBank(void)
{
	return RAM_ROM_EPROM_Bank;
}

/* initialise, allocate memory and setup */
void	RAM_ROM_Initialise(int NumBlocks)
{
	int Size;

	RAM_ROM_NumBlocks = NumBlocks;

	Size = NumBlocks*16*1024;

	RAM_ROM_RAM = (unsigned char *)malloc(Size);

	if (RAM_ROM_RAM!=NULL)
	{
		memset(RAM_ROM_RAM, 0x0ff, Size);
	}

	Size = (NumBlocks+7)>>3;


	RAM_ROM_BankEnables = (unsigned char *)malloc(Size);

	if (RAM_ROM_BankEnables!=NULL)
	{
		memset(RAM_ROM_BankEnables, 0x0ff, Size);
	}

	RAM_ROM_WriteEnabled = FALSE;
	RAM_ROM_Flags = 0;

}

void	RAM_ROM_Finish(void)
{
	if (RAM_ROM_RAM!=NULL)
	{
		free(RAM_ROM_RAM);
		RAM_ROM_RAM = NULL;
	}

	if (RAM_ROM_BankEnables!=NULL)
	{
		free(RAM_ROM_BankEnables);
		RAM_ROM_BankEnables = NULL;
	}
}

/* set bank enabled  state - if bank is enabled it is visible */
void	RAM_ROM_SetBankEnable(int Bank, BOOL State)
{
	unsigned long BankByte;
	unsigned char BankBit;

	if (RAM_ROM_BankEnables==NULL)
		return;

	BankByte = Bank>>3;
	BankBit = Bank & 0x07;

	if (State)
	{
		RAM_ROM_BankEnables[BankByte] |= (1<<BankBit);
	}
	else
	{
		RAM_ROM_BankEnables[BankByte] &= ~(1<<BankBit);
	}
}

BOOL RAM_ROM_IsBank0Enabled(void)
{
    return RAM_ROM_GetBankEnableState(0);
}
BOOL RAM_ROM_IsBank1Enabled(void)
{
    return RAM_ROM_GetBankEnableState(1);
}
BOOL RAM_ROM_IsBank2Enabled(void)
{
    return RAM_ROM_GetBankEnableState(2);
}
BOOL RAM_ROM_IsBank3Enabled(void)
{
    return RAM_ROM_GetBankEnableState(3);
}
BOOL RAM_ROM_IsBank4Enabled(void)
{
    return RAM_ROM_GetBankEnableState(4);
}
BOOL RAM_ROM_IsBank5Enabled(void)
{
    return RAM_ROM_GetBankEnableState(5);
}
BOOL RAM_ROM_IsBank6Enabled(void)
{
    return RAM_ROM_GetBankEnableState(6);
}
BOOL RAM_ROM_IsBank7Enabled(void)
{
    return RAM_ROM_GetBankEnableState(7);
}

void RAM_ROM_SetBank0Enable(BOOL bState)
{
    RAM_ROM_SetBankEnable(0, bState);
    RAM_ROM_SetBankEnable(8, bState);
}

void RAM_ROM_SetBank1Enable(BOOL bState)
{
    RAM_ROM_SetBankEnable(1, bState);
    RAM_ROM_SetBankEnable(9, bState);
}
void RAM_ROM_SetBank2Enable(BOOL bState)
{
    RAM_ROM_SetBankEnable(2, bState);
    RAM_ROM_SetBankEnable(10, bState);
}
void RAM_ROM_SetBank3Enable(BOOL bState)
{
    RAM_ROM_SetBankEnable(3, bState);
    RAM_ROM_SetBankEnable(11, bState);
}
void RAM_ROM_SetBank4Enable(BOOL bState)
{
    RAM_ROM_SetBankEnable(4, bState);
    RAM_ROM_SetBankEnable(12, bState);
}
void RAM_ROM_SetBank5Enable(BOOL bState)
{
    RAM_ROM_SetBankEnable(5, bState);
    RAM_ROM_SetBankEnable(13, bState);
}
void RAM_ROM_SetBank6Enable(BOOL bState)
{
    RAM_ROM_SetBankEnable(6, bState);
    RAM_ROM_SetBankEnable(14, bState);
}
void RAM_ROM_SetBank7Enable(BOOL bState)
{
    RAM_ROM_SetBankEnable(7, bState);
    RAM_ROM_SetBankEnable(15, bState);
}


/* true if bank is enabled, false otherwise */
BOOL	RAM_ROM_GetBankEnableState(int Bank)
{
	unsigned long BankByte;
	unsigned char BankBit;

	if (RAM_ROM_BankEnables==NULL)
		return FALSE;

	BankByte = Bank>>3;
	BankBit = Bank & 0x07;

	return ((RAM_ROM_BankEnables[BankByte] & (1<<BankBit))!=0);
}

void	RAM_ROM_RethinkMemory(MemoryData *pData)
{
    if (RAM_ROM_Active)
    {
        unsigned char *pRamPtr = RAM_ROM_RAM+((RAM_ROM_RomSelected-1)<<14)-0x0c000;
        pData->bRomDisable[7] = TRUE;
        pData->bRomDisable[6] = TRUE;
        pData->pReadPtr[7] = pRamPtr;
        pData->pReadPtr[6] = pRamPtr;
        if (RAM_ROM_WriteEnabled)
        {
            pData->pWritePtr[7] = pRamPtr;
            pData->pWritePtr[6] = pRamPtr;
            pData->bRamDisable[7] = TRUE;
            pData->bRamDisable[6] = TRUE;
        }
    }
}

void	RAM_ROM_Install(void)
{
	RAM_ROM_Initialise(16);
}

void	RamRom_Write(Z80_WORD Port, Z80_BYTE Data)
{

    /* only handles ram currently not the rom */

    int BankSelected= Data&0x0f;
    int BankByte;
    int BankBit;

    RAM_ROM_Active = FALSE;

   	RAM_ROM_WriteEnabled = FALSE;

	if (RAM_ROM_RAM==NULL)
		return;

	if ((RAM_ROM_Flags & RAM_ROM_FLAGS_RAM_ON)==0)
		return;

	if (RAM_ROM_Flags & RAM_ROM_FLAGS_RAM_WRITE_ENABLE)
	{
		RAM_ROM_WriteEnabled = TRUE;
	}

    BankByte = BankSelected>>3;
    BankBit = BankSelected & 0x07;

    /* if enabled... */
    if (RAM_ROM_BankEnables[BankByte] & (1<<BankBit))
    {
        RAM_ROM_Active = TRUE;
        RAM_ROM_RomSelected = BankSelected;
    }

    Computer_RethinkMemory();
}


static EmuDeviceSwitch RamRomSwitches[11]=
{
  {
      "Write Enable",                   /* write enable to sram */
	  "WriteEnable",
      RAM_ROM_IsRamWriteEnabled,
      RAM_ROM_SetRamWriteEnableState
  },
  {
      "Ram active (socket 0-15)",                     /* if it's active */
	  "RamActive",
      RAM_ROM_IsRamOn,
      RAM_ROM_SetRamOnState
  },
  {
      "EPROM is enabled ",              /* if eprom is enabled */
	  "EpromEnabled",
      RAM_ROM_IsEPROMOn,
      RAM_ROM_SetEPROMOnState
  },
   {
      "Socket 0/7 enabled",
	  "Socket_0_7_Enabled",
      RAM_ROM_IsBank0Enabled,
      RAM_ROM_SetBank0Enable
  },
   {
      "Socket 1/8 enabled",
	  "Socket_1_8_Enabled",
	  RAM_ROM_IsBank1Enabled,
      RAM_ROM_SetBank1Enable
  },
   {
      "Socket 2/9 enabled",
	  "Socket_2_9_Enabled",
	  RAM_ROM_IsBank2Enabled,
      RAM_ROM_SetBank2Enable
  },
   {
      "Socket 3/10 enabled",
	  "Socket_3_10_Enabled",
	  RAM_ROM_IsBank3Enabled,
      RAM_ROM_SetBank3Enable
  },
   {
      "Socket 4/11 enabled",
	  "Socket_4_11_Enabled",
	  RAM_ROM_IsBank4Enabled,
      RAM_ROM_SetBank4Enable
  },
   {
      "Socket 5/12 enabled",
	  "Socket_5_12_Enabled",
	  RAM_ROM_IsBank5Enabled,
      RAM_ROM_SetBank5Enable
  },
   {
      "Socket 6/13 enabled",
	  "Socket_6_13_Enabled",
	  RAM_ROM_IsBank6Enabled,
      RAM_ROM_SetBank6Enable
  },
   {
      "Socket 7/14 enabled",
	  "Socket_7_14_Enabled",
	  RAM_ROM_IsBank7Enabled,
      RAM_ROM_SetBank7Enable
  },
};

/* TODO: Verify */
CPCPortWrite RamRomPortWrite[1]=
{
	{
		0x02000,            /* and */
		0x00000,            /* compare */
		RamRom_Write
	}
};

static EmuDevice RamRomDevice=
{
	NULL,
	RAM_ROM_Install,
	RAM_ROM_Finish,
	"RAMROM",
	"InicronRamRom",
	"Inicron RAM-ROM",
    CONNECTION_EXPANSION,   /* connects to expansion */
	DEVICE_FLAGS_HAS_EXPANSION_ROMS| DEVICE_FLAGS_FROM_SPECIFICATION, /* has expansion roms */
    0,                /* no read ports */
  NULL,
  1,                    /* 1 write ports */
	RamRomPortWrite,
	0,                /* no memory read*/
	NULL,
	0,                /* no memory write */
	NULL,
	NULL, /* todo: reset?? no reset function */
  RAM_ROM_RethinkMemory, 
  NULL, /* todo: no power function */
	sizeof(RamRomSwitches)/sizeof(RamRomSwitches[0]),                      /* no switches */
	RamRomSwitches,
    0,                      /* no buttons */
    NULL,
    0,                      /* no onboard roms */
    NULL,
	NULL,	/* no cursor update function */
	NULL,
	NULL,	/* printer */
	NULL,	/* joystick */
	0,
	NULL,	/* memory ranges */
	NULL, /* sound */
	NULL, /* lpen */
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	NULL, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
	NULL
};

void RamRom_Init(void)
{
 	RegisterDevice(&RamRomDevice);
}
