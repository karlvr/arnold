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
#include "ata.h"
/* TODO: Slave/Master. */
ata_device xmass_ata;

BOOL XMass_Read(Z80_WORD Port, Z80_BYTE *pDeviceData)
{

	switch (Port & 0x07)
	{
		/* IDE data register */
	case 0x0:
	{
		*pDeviceData = 0x0ff;
		if (ata_isresponding(&xmass_ata))
		{
			*pDeviceData =  ata_read(&xmass_ata, ATA_DATA_REGISTER_DA | ATA_DATA_REGISTER_CS) & 0x0ff;
		}
	}
	return TRUE;



		/* IDE Error register */
		case 0x1:
		{
		*pDeviceData = 0x0ff;
			if (ata_isresponding(&xmass_ata))
			{
				*pDeviceData =  ata_read(&xmass_ata, ATA_ERROR_DA | ATA_ERROR_CS) & 0x0ff;
			}
		}
		return TRUE;



		/* IDE Sector count */
		case 0x2:
		{
		*pDeviceData = 0x0ff;
			if (ata_isresponding(&xmass_ata))
			{
				*pDeviceData =  ata_read(&xmass_ata, ATA_SECTOR_COUNT_DA | ATA_SECTOR_COUNT_CS);
			}
		}
		return TRUE;

		/* IDE Sector number */
		case 0x3:
		{
			*pDeviceData = 0x0ff;
		if (ata_isresponding(&xmass_ata))
			{
				*pDeviceData =  ata_read(&xmass_ata, ATA_SECTOR_NUMBER_DA | ATA_SECTOR_NUMBER_CS);
			}
		}
		return TRUE;

		/* IDE Cylinder number low */
		case 0x4:
		{
			*pDeviceData = 0x0ff;
		if (ata_isresponding(&xmass_ata))
			{
				*pDeviceData =  ata_read(&xmass_ata, ATA_CYLINDER_NUMBER_LOW_DA | ATA_CYLINDER_NUMBER_LOW_CS);
			}
		}
		return TRUE;

		/* IDE Cylinder number high */
		case 0x5:
		{
		*pDeviceData = 0x0ff;
			if (ata_isresponding(&xmass_ata))
			{
				*pDeviceData =  ata_read(&xmass_ata, ATA_CYLINDER_NUMBER_HIGH_DA | ATA_CYLINDER_NUMBER_HIGH_CS);
			}
		}
		return TRUE;

		/* IDE head */
		case 0x6:
		{
		*pDeviceData = 0x0ff;
			if (ata_isresponding(&xmass_ata))
			{
				*pDeviceData =  ata_read(&xmass_ata, ATA_HEAD_DA | ATA_HEAD_CS);
			}
		}
		return TRUE;

		/* IDE Status */
		case 0x7:
		{
		*pDeviceData = 0x0ff;
			if (ata_isresponding(&xmass_ata))
			{
				*pDeviceData =  ata_read(&xmass_ata, ATA_STATUS_DA | ATA_STATUS_CS);
			}
		}
		return TRUE;

		default:
			break;
	}
	
	return FALSE;
}

void XMass_Write(Z80_WORD Port, Z80_BYTE Data)
{
	switch (Port & 0x07)
	{
	
	case 0:
	{
		ata_write(&xmass_ata,ATA_DATA_REGISTER_DA | ATA_DATA_REGISTER_CS, Data & 0x0ff);
	}
	break;

	case 1:
	{
		ata_write(&xmass_ata,ATA_FEATURES_DA | ATA_FEATURES_CS, Data);
	}
	break;

	case 2:
	{
		ata_write(&xmass_ata,ATA_SECTOR_COUNT_DA | ATA_SECTOR_COUNT_CS, Data);
	}
	break;

	case 3:
	{
		ata_write(&xmass_ata,ATA_SECTOR_NUMBER_DA | ATA_SECTOR_NUMBER_CS, Data);
	}
	break;

	case 4:
	{
		ata_write(&xmass_ata,ATA_CYLINDER_NUMBER_LOW_DA | ATA_CYLINDER_NUMBER_LOW_CS, Data);
	}
	break;

	case 5:
	{
		ata_write(&xmass_ata,ATA_CYLINDER_NUMBER_HIGH_DA | ATA_CYLINDER_NUMBER_HIGH_CS, Data);
	}
	break;

	case 6:
	{
		ata_write(&xmass_ata,ATA_HEAD_DA | ATA_HEAD_CS, Data);
	}
	break;

	case 7:
	{
		ata_write(&xmass_ata,ATA_COMMAND_DA | ATA_COMMAND_CS, Data);
	}
	break;
	
	default:
	{
	}
	break;

	}
}


CPCPortRead XMassPortRead[1]=
{
	{
	0xfff8,            /* and */
	0xfd08,            /* compare */
	XMass_Read
	},
};

CPCPortWrite XMassPortWrite[1]=
{
	{
	0xfff8,            /* and */
	0xfd08,            /* compare */
	XMass_Write
	},
};

void XMass_Reset(void)
{
	ata_reset(&xmass_ata);
	xmass_ata.b8BitTransfer = TRUE;
}

void XMass_InitDevice(void)
{
	ata_init(&xmass_ata, ATA_JUMPER_MASTER_WITH_SLAVE_PRESENT, TRUE,128 * 1024 * 1024);
	xmass_ata.m_bSlaveDetected = FALSE; // through device enable!
		
	// TODO: X-Mass does this in h/w and holds CPC at reset. Fake it here
	xmass_ata.b8BitTransfer = TRUE;
}

void XMass_FinishDevice(void)
{
	ata_finish(&xmass_ata);
}

static EmuDevice XMassDevice=
{
	NULL,
	XMass_InitDevice,
	XMass_FinishDevice,
	"XMASS",
	"XMass",
	"ToTo's X-Mass",
    CONNECTION_EXPANSION,   /* connects to expansion */
	DEVICE_FLAGS_TESTED| DEVICE_WORKING,
    sizeof(XMassPortWrite)/sizeof(XMassPortWrite[0]),                /* 1 read port */
  XMassPortRead,
  sizeof(XMassPortWrite)/sizeof(XMassPortWrite[0]),                    /* 1 write ports */
  XMassPortWrite,
  0,                /* no memory read*/
  NULL,
  0,                /* no memory write */
  NULL,
  XMass_Reset, /* no reset function */
  NULL, /* no rethink function */
  XMass_Reset, /* no power function */
	0,                       /* no switches */
	NULL,
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

void XMass_Init(void)
{
	RegisterDevice(&XMassDevice);
}










