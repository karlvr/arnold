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
*
* Emulation of Jon Bradbury's uIDE16 Universal IDE Adapter. 
* Made with permission from Jon.
*/
#include "cpc.h"
#include "emudevice.h"
#include "ata.h"

static unsigned short uide_AddrSwitches = 0x0fef0;

static ata_device uide_ata;


BOOL uide_JP1Enabled(void)
{
	return (((uide_AddrSwitches>>15)&0x01)!=0);
}
	
void uide_JP1Enable(BOOL bState)
{
	uide_AddrSwitches &= ~(1<<15);
	if (bState)
	{
		uide_AddrSwitches |= (1<<15);
	}
}

BOOL uide_JP2Enabled(void)
{
	return (((uide_AddrSwitches>>14)&0x01)!=0);
}
	
void uide_JP2Enable(BOOL bState)
{
	uide_AddrSwitches &= ~(1<<14);
	if (bState)
	{
		uide_AddrSwitches |= (1<<14);
	}
}

BOOL uide_JP3Enabled(void)
{
	return (((uide_AddrSwitches>>13)&0x01)!=0);
}
	
void uide_JP3Enable(BOOL bState)
{
	uide_AddrSwitches &= ~(1<<13);
	if (bState)
	{
		uide_AddrSwitches |= (1<<13);
	}
}

BOOL uide_JP4Enabled(void)
{
	return (((uide_AddrSwitches>>12)&0x01)!=0);
}
	
void uide_JP4Enable(BOOL bState)
{
	uide_AddrSwitches &= ~(1<<12);
	if (bState)
	{
		uide_AddrSwitches |= (1<<12);
	}
}

BOOL uide_JP5Enabled(void)
{
	return (((uide_AddrSwitches>>11)&0x01)!=0);
}
	
void uide_JP5Enable(BOOL bState)
{
	uide_AddrSwitches &= ~(1<<11);
	if (bState)
	{
		uide_AddrSwitches |= (1<<11);
	}
}

BOOL uide_JP6Enabled(void)
{
	return (((uide_AddrSwitches>>10)&0x01)!=0);
}
	
void uide_JP6Enable(BOOL bState)
{
	uide_AddrSwitches &= ~(1<<10);
	if (bState)
	{
		uide_AddrSwitches |= (1<<10);
	}
}

BOOL uide_JP7Enabled(void)
{
	return (((uide_AddrSwitches>>9)&0x01)!=0);
}
	
void uide_JP7Enable(BOOL bState)
{
	uide_AddrSwitches &= ~(1<<9);
	if (bState)
	{
		uide_AddrSwitches |= (1<<9);
	}
}

BOOL uide_JP8Enabled(void)
{
	return (((uide_AddrSwitches>>8)&0x01)!=0);
}
	
void uide_JP8Enable(BOOL bState)
{
	uide_AddrSwitches &= ~(1<<8);
	if (bState)
	{
		uide_AddrSwitches |= (1<<8);
	}
}

BOOL uide_JP9Enabled(void)
{
	return (((uide_AddrSwitches>>7)&0x01)!=0);
}
	
void uide_JP9Enable(BOOL bState)
{
	uide_AddrSwitches &= ~(1<<7);
	if (bState)
	{
		uide_AddrSwitches |= (1<<7);
	}
}

BOOL uide_JP10Enabled(void)
{
	return (((uide_AddrSwitches>>6)&0x01)!=0);
}
	
void uide_JP10Enable(BOOL bState)
{
	uide_AddrSwitches &= ~(1<<6);
	if (bState)
	{
		uide_AddrSwitches |= (1<<6);
	}
}

BOOL uide_JP11Enabled(void)
{
	return (((uide_AddrSwitches>>5)&0x01)!=0);
}
	
void uide_JP11Enable(BOOL bState)
{
	uide_AddrSwitches &= ~(1<<5);
	if (bState)
	{
		uide_AddrSwitches |= (1<<5);
	}
}

BOOL uide_JP12Enabled(void)
{
	return (((uide_AddrSwitches>>4)&0x01)!=0);
}
	
void uide_JP12Enable(BOOL bState)
{
	uide_AddrSwitches &= ~(1<<4);
	if (bState)
	{
		uide_AddrSwitches |= (1<<4);
	}
}

BOOL uide_JP13Enabled(void)
{
	return (((uide_AddrSwitches>>3)&0x01)!=0);
}
	
void uide_JP13Enable(BOOL bState)
{
	uide_AddrSwitches &= ~(1<<3);
	if (bState)
	{
		uide_AddrSwitches |= (1<<3);
	}
}

static EmuDeviceSwitch uideSwitches[13] =
{
	{
		"JP1 - A15",          
		"JP1",
		uide_JP1Enabled,
		uide_JP1Enable
	},
	{
		"JP2 - A14",          
		"JP2",
		uide_JP2Enabled,
		uide_JP2Enable
	},
	{
		"JP3 - A13",          
		"JP3",
		uide_JP3Enabled,
		uide_JP3Enable
	},
	{
		"JP4 - A12",          
		"JP4",
		uide_JP4Enabled,
		uide_JP4Enable
	},
	{
		"JP5 - A11",          
		"JP5",
		uide_JP5Enabled,
		uide_JP5Enable
	},
	{
		"JP6 - A10",          
		"JP6",
		uide_JP6Enabled,
		uide_JP6Enable
	},
	{
		"JP7 - A9",          
		"JP7",
		uide_JP7Enabled,
		uide_JP7Enable
	},
	{
		"JP8 - A8",          
		"JP8",
		uide_JP8Enabled,
		uide_JP8Enable
	},
	{
		"JP9 - A7",          
		"JP9",
		uide_JP9Enabled,
		uide_JP9Enable
	},
	{
		"JP10 - A6",          
		"JP10",
		uide_JP10Enabled,
		uide_JP10Enable
	},
	{
		"JP11 - A5",          
		"JP11",
		uide_JP11Enabled,
		uide_JP11Enable
	},
	{
		"JP12 - A4",          
		"JP12",
		uide_JP12Enabled,
		uide_JP12Enable
	},
	{
		"JP13 - A3",          
		"JP13",
		uide_JP13Enabled,
		uide_JP13Enable
	},

};

BOOL uide_Read(Z80_WORD Port, Z80_BYTE *pDeviceData)
{
	if ((Port&0xfff8)==uide_AddrSwitches)
	{
			
		switch (Port & 0x07)
		{
			/* IDE data register */
		case 0x0:
		{
			*pDeviceData = 0x0ff;
			if (ata_isresponding(&uide_ata))
			{
				*pDeviceData =  ata_read(&uide_ata, ATA_DATA_REGISTER_DA | ATA_DATA_REGISTER_CS) & 0x0ff;
			}
		}
		return TRUE;



			/* IDE Error register */
			case 0x1:
			{
			*pDeviceData = 0x0ff;
				if (ata_isresponding(&uide_ata))
				{
					*pDeviceData =  ata_read(&uide_ata, ATA_ERROR_DA | ATA_ERROR_CS) & 0x0ff;
				}
			}
			return TRUE;



			/* IDE Sector count */
			case 0x2:
			{
			*pDeviceData = 0x0ff;
				if (ata_isresponding(&uide_ata))
				{
					*pDeviceData =  ata_read(&uide_ata, ATA_SECTOR_COUNT_DA | ATA_SECTOR_COUNT_CS);
				}
			}
			return TRUE;

			/* IDE Sector number */
			case 0x3:
			{
			*pDeviceData = 0x0ff;
				if (ata_isresponding(&uide_ata))
				{
					*pDeviceData =  ata_read(&uide_ata, ATA_SECTOR_NUMBER_DA | ATA_SECTOR_NUMBER_CS);
				}
			}
			return TRUE;

			/* IDE Cylinder number low */
			case 0x4:
			{
			*pDeviceData = 0x0ff;
				if (ata_isresponding(&uide_ata))
				{
					*pDeviceData =  ata_read(&uide_ata, ATA_CYLINDER_NUMBER_LOW_DA | ATA_CYLINDER_NUMBER_LOW_CS);
				}
			}
			return TRUE;

			/* IDE Cylinder number high */
			case 0x5:
			{
			*pDeviceData = 0x0ff;
				if (ata_isresponding(&uide_ata))
				{
					*pDeviceData =  ata_read(&uide_ata, ATA_CYLINDER_NUMBER_HIGH_DA | ATA_CYLINDER_NUMBER_HIGH_CS);
				}
			}
			return TRUE;

			/* IDE head */
			case 0x6:
			{
			*pDeviceData = 0x0ff;
				if (ata_isresponding(&uide_ata))
				{
					*pDeviceData =  ata_read(&uide_ata, ATA_HEAD_DA | ATA_HEAD_CS);
				}
			}
			return TRUE;

			/* IDE Status */
			case 0x7:
			{
			*pDeviceData = 0x0ff;
				if (ata_isresponding(&uide_ata))
				{
					*pDeviceData =  ata_read(&uide_ata, ATA_STATUS_DA | ATA_STATUS_CS);
				}
			}
			return TRUE;

			default:
				break;
		}
	}	
	return FALSE;
}

void uide_Write(Z80_WORD Port, Z80_BYTE Data)
{
	if ((Port&0xfff8)==uide_AddrSwitches)
	{
		
	switch (Port & 0x07)
	{
	
	case 0:
	{
		ata_write(&uide_ata,ATA_DATA_REGISTER_DA | ATA_DATA_REGISTER_CS, Data & 0x0ff);
	}
	break;

	case 1:
	{
		ata_write(&uide_ata,ATA_FEATURES_DA | ATA_FEATURES_CS, Data);
	}
	break;

	case 2:
	{
		ata_write(&uide_ata,ATA_SECTOR_COUNT_DA | ATA_SECTOR_COUNT_CS, Data);
	}
	break;

	case 3:
	{
		ata_write(&uide_ata,ATA_SECTOR_NUMBER_DA | ATA_SECTOR_NUMBER_CS, Data);
	}
	break;

	case 4:
	{
		ata_write(&uide_ata,ATA_CYLINDER_NUMBER_LOW_DA | ATA_CYLINDER_NUMBER_LOW_CS, Data);
	}
	break;

	case 5:
	{
		ata_write(&uide_ata,ATA_CYLINDER_NUMBER_HIGH_DA | ATA_CYLINDER_NUMBER_HIGH_CS, Data);
	}
	break;

	case 6:
	{
		ata_write(&uide_ata,ATA_HEAD_DA | ATA_HEAD_CS, Data);
	}
	break;

	case 7:
	{
		ata_write(&uide_ata,ATA_COMMAND_DA | ATA_COMMAND_CS, Data);
	}
	break;
	
	default:
	{
	}
	break;

	}
}
}


CPCPortRead uidePortRead[1]=
{
	{
	0x0000,            /* and */
	0x0000,            /* compare */
	uide_Read
	},
};

CPCPortWrite uidePortWrite[1]=
{
	{
	0x0000,            /* and */
	0x0000,            /* compare */
	uide_Write
	},
};

void uide_Reset(void)
{
	ata_reset(&uide_ata);
}

void uide_InitDevice(void)
{
	ata_init(&uide_ata, ATA_JUMPER_MASTER_WITH_SLAVE_PRESENT, TRUE,128 * 1024 * 1024);
	uide_ata.m_bSlaveDetected = FALSE; // through device enable!
}

void uide_FinishDevice(void)
{
	ata_finish(&uide_ata);
}

static EmuDevice uideDevice=
{
	NULL,
	uide_InitDevice,
	uide_FinishDevice,
	"uide16",
	"uide16",
	"Jon Bradbury's uIDE 16",
    CONNECTION_EXPANSION,   /* connects to expansion */
	DEVICE_WORKING| DEVICE_FLAGS_FROM_SPECIFICATION,
    sizeof(uidePortRead)/sizeof(uidePortRead[0]),                /* 1 read port */
  uidePortRead,
  sizeof(uidePortWrite)/sizeof(uidePortWrite[0]),                    /* 1 write ports */
  uidePortWrite,
  0,                /* no memory read*/
  NULL,
  0,                /* no memory write */
  NULL,
  uide_Reset, /* no reset function */
  NULL, /* no rethink function */
  uide_Reset, /* no power function */
	sizeof(uideSwitches)/sizeof(uideSwitches[0]),      /* no switches*/
	uideSwitches,
    0,                      /* no buttons */
    NULL,
    0,                      /* no onboard roms */
    NULL,
	NULL,	/* no cursor update function */
	NULL, /* rom slots */
	NULL,	/* printer */
	NULL, /* joystick */
	0,
	NULL,
	NULL,
	NULL,
	NULL
};

void uide_Init(void)
{
	RegisterDevice(&uideDevice);
}










