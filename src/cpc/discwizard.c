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

/* THIS EMULATION IS INCOMPLETE. MORE TESTING IS NEEDED ON A REAL DEVICE */

/* TODO: Need to emulate PAL inside disc wizard*/
/* Disc Wizard emulation */
/* NOT WORKING */
#include "cpc.h"
#include "emudevice.h"

/* ROMDIS??? */

static unsigned char DiscWizardRomData[8192];
static BOOL DiscWizard_LoadSaveSwitchEnable = FALSE;
static BOOL DiscWizard_OneTwoSwitchEnable = FALSE;
/* bit 0 is rom enable/disable state */
static unsigned char DiscWizardRomState=0;

void DiscWizard_SetROM(const unsigned char *pROM, unsigned long RomLength)
{
	EmuDevice_CopyRomData(DiscWizardRomData, sizeof(DiscWizardRomData), pROM, RomLength);
}


void DiscWizard_ClearROM(void)
{
	EmuDevice_ClearRomData(DiscWizardRomData, sizeof(DiscWizardRomData));
}

void    DiscWizard_SetLoadSaveSwitch(BOOL bState)
{
	DiscWizard_LoadSaveSwitchEnable = bState;

}


BOOL    DiscWizard_IsLoadSaveSwitchEnabled(void)
{
	return DiscWizard_LoadSaveSwitchEnable;
}

BOOL    DiscWizard_IsOneTwoSwitchEnabled(void)
{
	return DiscWizard_OneTwoSwitchEnable;
}


void    DiscWizard_SetOneTwoSwitch(BOOL bState)
{
	DiscWizard_OneTwoSwitchEnable = bState;

}


/* a12, a14, a15,mreq */
/* nmi, iorq */

/* rd, mreq,  output of pal, nmi, a13, load/save */

void DiscWizard_MemoryRethink(MemoryData *pData)
{
    if ((DiscWizardRomState & (1<<0))!=0)
    {
        unsigned char *pRomPtr = NULL;
        int nIndex = 0;
		/* no effect on ramdis/romdis */
        if (DiscWizard_IsOneTwoSwitchEnabled())
        {
            nIndex |= 2;
        }
        if (DiscWizard_IsLoadSaveSwitchEnabled())
        {
            nIndex |= 1;
        }

        pRomPtr = DiscWizardRomData + (nIndex<<11);

        /* read only */
        pData->pReadPtr[1] = pRomPtr-8192; /* repeats */
        pData->pReadPtr[0] = pRomPtr;

		pData->bRomDisable[0] = TRUE;
		pData->bRomDisable[1] = TRUE;

		/* RAMDIS */
		pData->pWritePtr[1] = GetDummyWriteRam();
		pData->pWritePtr[0] = GetDummyWriteRam();
		/* we disable ram */
		pData->bRamDisable[0] = TRUE;
		pData->bRamDisable[1] = TRUE;

    }
}

void DiscWizard_Reset(void)
{
    /* assumption, but seems to hold true */
    DiscWizardRomState = 0;
	Computer_RethinkMemory();

}

void    DiscWizard_Stop(void)
{
    DiscWizardRomState = 1;

/* not going to work.... */
    CPU_SetNMIState(FALSE);
    CPU_SetNMIState(TRUE);
    CPU_SetNMIState(FALSE);

	Computer_RethinkMemory();
}


void	DiscWizard_Write(Z80_WORD Port, Z80_BYTE Data)
{
    /* no idea about how the data is decoded */
    DiscWizardRomState = Data;
	Computer_RethinkMemory();

}


CPCPortWrite discWizardPortWrite[1]=
{
	{
	0x0ffff,            /* and */           /* not correct */
	0x0f0e0,            /* compare */       /* correct */
	DiscWizard_Write
	}	
};

static EmuDeviceRom DiscWizardRom[1]=
{
	{
	"Disc Wizard System ROM",
	"SystemRom",
	DiscWizard_SetROM,
	DiscWizard_ClearROM,
  sizeof(DiscWizardRomData),
	0   /* ROM CRC - todo */
	}
};

static EmuDeviceSwitch DiscWizardSwitches[2]=
{
  {
    "Load/Save Switch (OFF=Save)",
	"LoadSave",
    DiscWizard_IsLoadSaveSwitchEnabled,
    DiscWizard_SetLoadSaveSwitch
},
{
    "1/2 Switch (OFF = 1)",
	"1_2",
    DiscWizard_IsOneTwoSwitchEnabled,
    DiscWizard_SetOneTwoSwitch
}
};

static EmuDeviceButton DiscWizardStopButton[1]=
{
	{
		"Stop Button",
		DiscWizard_Stop
	}
};

static EmuDevice DiscWizardDevice=
{
	NULL,
	NULL,
	NULL,
	"DISCWIZARD",
	"DiscWizard",
	"Disc Wizard",
	CONNECTION_EXPANSION,   /* connects to expansion */
	0,
  0,
  NULL,
  sizeof(discWizardPortWrite)/sizeof(discWizardPortWrite[1]),
 discWizardPortWrite,
 0,                /* no memory read*/
 NULL,
 0,                /* no memory write */
 NULL,
 DiscWizard_Reset,
  DiscWizard_MemoryRethink,
  DiscWizard_Reset,
	sizeof(DiscWizardSwitches)/sizeof(DiscWizardSwitches[0]),                      /* no switches */
	DiscWizardSwitches,
	sizeof(DiscWizardStopButton) / sizeof(DiscWizardStopButton[0]),                      /* no buttons */
    DiscWizardStopButton,
	sizeof(DiscWizardRom) / sizeof(DiscWizardRom[0]),                      
    DiscWizardRom,
    NULL,                   /* no cursor function */
    NULL,                   /* no generic roms */
	NULL,					/* printer */
	NULL,					/* joystick */
	0,
	NULL,					/* memory ranges */
	NULL,	/* no sound */
	NULL,	/* lpen */
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	NULL, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
	NULL,

};

void DiscWizard_Init(void)
{
	RegisterDevice(&DiscWizardDevice);
}










