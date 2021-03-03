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
/* good:
- RAM
- ROM
- I/o decoding of RAM 
- I/O decoding of ROM
- ide

to improve:
- rtc
- i/o decoding of symbiface 2 registers
*/
#include "cpc.h"
#include "emudevice.h"
#include "mouse.h"
#include "ata.h"

enum
{
    SYMBIMOUSE_MODE_NONE = 0,
    SYMBIMOUSE_MODE_X_MOVEMENT,
    SYMBIMOUSE_MODE_Y_MOVEMENT,
    SYMBIMOUSE_MODE_BUTTONS
};


typedef struct 
{
	unsigned char Registers[128];
	unsigned char Address;
	unsigned char UpdateStatus;
	unsigned char InterruptFlags;
} ds12887;

/* TODO: Master/slave device */
ds12887 symbiface_rtc;
ata_device symbiface_master_ata;
ata_device symbiface_slave_ata;

ExpansionRomData symbiface_roms;

void ds12887_reset(ds12887 *ds)
{
	ds->UpdateStatus  = 0;
	/* reset periodic interrupt enable,
		reset alaram interrupt enable,
		reset update ended interrupt enable,
		square wave enable reset */
	ds->Registers[0x0b] &=~((1<<6)|(1<<5)|(1<<4)|(1<<3));

	ds->InterruptFlags = 0;
}

void ds12887_update_ints(ds12887 *ds)
{
	ds->InterruptFlags &=~(1<<7);
	
	/* if PF=PIE=1, AF=AIE=1, UF=UIE=1 set IRQF */
	if (ds->InterruptFlags & ds->Registers[0x0b] & ((1<<6)|(1<<5)|(1<<4)))
	{
		ds->InterruptFlags |= (1<<7);
	}
}

unsigned char ds12887_read_data(ds12887 *ds)
{
//printf("Read ds12887 Addr %02x\n", ds->Address);

if (ds->Address == 0x0c)
return ds->InterruptFlags;
if (ds->Address == 0x0d)
return (1 << 7);	/* indicates valid ram and time */

{
	unsigned char Data = ds->Registers[ds->Address];
	if (ds->Address == 0x0a)
	{
		Data = Data & 0x07f;
		Data = Data | ds->UpdateStatus;
	}

	return Data;
}
}

void ds12887_write_addr(ds12887 *ds, unsigned char Data)
{
	ds->Address = Data & 0x07f;
	//printf("Select ds12887 Addr %02x\n", ds->Address);
}


void ds12887_write_data(ds12887 *ds, unsigned char Data)
{
	//printf("Write ds12887 Addr %02x Data %02x\n", ds->Address, Data);
	ds->Registers[ds->Address] = Data;
}

typedef struct
{
	BOOL bStoredMouseValid;
	BOOL bStoreMouse;
	signed int StoredDeltaX;
	signed int StoredDeltaY;
	int StoredMouseX;
	int StoredMouseY;
	int StoredMouseButtons;
	int StoredHDData;
	int MouseX;
	int MouseY;
	int SymbifaceReport;
	BOOL DataByteHigh;
	BOOL DataRead;
	unsigned short IDEData;
	unsigned char ROMStaticRAM[512 * 1024]; /* symbiface 2 has a static ram for it's rom that is battery backed */
	unsigned char RAMStaticRAM[512 * 1024]; /* symbiface 2 has a static ram for it's ram */
	BOOL RomEnables[8]; /* PCB has 8 dip switches */
	BOOL J2; /* switch allows 8-31 */
	BOOL RomEnable;
	int CurrentRom;
	BOOL RomEnable4000;
	int RamConfig;
} symbiface2;

static symbiface2 sym2;


#define SYMBIFACE_REPORT_X (1<<0)
#define SYMBIFACE_REPORT_Y (1<<1)
#define SYMBIFACE_REPORT_BUTTONS (1<<2)


#define SYMBIMOUSE_BUTTON_SCROLL_WHEEL_STATE (1<<5)

#define SYMBIMOUSE_BUTTON_LEFT (1<<0)
#define SYMBIMOUSE_BUTTON_RIGHT (1<<1)
#define SYMBIMOUSE_BUTTON_MIDDLE (1<<2)
#define SYMBIMOUSE_BUTTON_FORWARD (1<<3)
#define SYMBIMOUSE_BUTTON_BACKWARD (1<<4)

/*
// fd06-fd0f for ide

// fd10 is mouse (write only)
// fd11 enable/disable roms
// fd14 is RTC data (read/write)
// fd15 is RTC register (write only)
// fd17 show rom or not
*/

char intto6bitsigned(int x)
{
	char ax5 = ((char)abs(x)) & 0x1f;
	if (x < 0)
	{
		return ((ax5 ^ 0x3f) + 1);
	}
	else
	{
		return (ax5);
	}
}

BOOL SymbifaceRead(Z80_WORD Port, Z80_BYTE *pDeviceData)
{

	/* ide reg? */
	if (((Port & 0x01f) >= 6) && ((Port & 0x01f) <= 0x0f))
	{
		/* read high byte? */
		if (sym2.DataByteHigh)
		{
			/* reset back */
			sym2.DataByteHigh = FALSE;

			/* return high byte of data */
			*pDeviceData =  (sym2.IDEData >> 8) & 0x0ff;
			return TRUE;
		}
	}


//	//printf("Symbiface read %04x\n", Port);
    switch (Port & 0x01f)
    {
		case 6:
		{
			*pDeviceData = 0x0ff;
			if (ata_isresponding(&symbiface_master_ata))
			{
				*pDeviceData =  ata_read(&symbiface_master_ata,ATA_ALTERNATE_STATUS_CS | ATA_ALTERNATE_STATUS_DA);
			}
			else if (ata_isresponding(&symbiface_slave_ata))
			{
				*pDeviceData =  ata_read(&symbiface_slave_ata,ATA_ALTERNATE_STATUS_CS | ATA_ALTERNATE_STATUS_DA);
			}
			return TRUE;
		}
		break;
		
		case 7:
		{
			*pDeviceData = 0x0ff;
			if (ata_isresponding(&symbiface_master_ata))
			{
				*pDeviceData = ata_read(&symbiface_master_ata, ATA_DRIVE_ADDRESS_CS | ATA_DRIVE_ADDRESS_DA);
			}
			else if (ata_isresponding(&symbiface_slave_ata))
			{
				*pDeviceData = ata_read(&symbiface_slave_ata, ATA_DRIVE_ADDRESS_CS | ATA_DRIVE_ADDRESS_DA);
			}
			return TRUE;
		}
		break;

		/* IDE data register */
		case 0x8:
		{
			sym2.DataRead = TRUE; // is this correct?
			sym2.DataByteHigh = TRUE;
			sym2.IDEData = 0x0ffff;

			/* Data Byte High is false, read data */
			if (ata_isresponding(&symbiface_master_ata))
			{
				sym2.IDEData = ata_read(&symbiface_master_ata, ATA_DATA_REGISTER_DA | ATA_DATA_REGISTER_CS);
			}
			else if (ata_isresponding(&symbiface_slave_ata))
			{
				sym2.IDEData = ata_read(&symbiface_slave_ata, ATA_DATA_REGISTER_DA | ATA_DATA_REGISTER_CS);
			}
			*pDeviceData =   sym2.IDEData & 0x0ff;
		}
		return TRUE;

		/* IDE Error register */
		case 0x9:
		{
			*pDeviceData = 0x0ff;
			if (ata_isresponding(&symbiface_master_ata))
			{
				*pDeviceData = ata_read(&symbiface_master_ata,ATA_ERROR_DA | ATA_ERROR_CS) & 0x0ff;
			}
			else if (ata_isresponding(&symbiface_slave_ata))
			{
				*pDeviceData = ata_read(&symbiface_slave_ata,ATA_ERROR_DA | ATA_ERROR_CS) & 0x0ff;
			}
		}
		return TRUE;
			
		/* IDE Sector count */
		case 0x0A:
		{
			*pDeviceData = 0x0ff;
			if (ata_isresponding(&symbiface_master_ata))
			{
				*pDeviceData =  ata_read(&symbiface_master_ata,ATA_SECTOR_COUNT_DA | ATA_SECTOR_COUNT_CS);
			}
			else if (ata_isresponding(&symbiface_slave_ata))
			{
				*pDeviceData =  ata_read(&symbiface_slave_ata,ATA_SECTOR_COUNT_DA | ATA_SECTOR_COUNT_CS);
			}
		}
		return TRUE;
		
			
		/* IDE Sector number */
		case 0x0b:
		{			
			*pDeviceData = 0x0ff;

			if (ata_isresponding(&symbiface_master_ata))
			{
				*pDeviceData =  ata_read(&symbiface_master_ata,ATA_SECTOR_NUMBER_DA | ATA_SECTOR_NUMBER_CS);
			}
			else if (ata_isresponding(&symbiface_slave_ata))
			{
				*pDeviceData =  ata_read(&symbiface_slave_ata,ATA_SECTOR_NUMBER_DA | ATA_SECTOR_NUMBER_CS);
			}
		}
		return TRUE;
		
		/* IDE Cylinder number low */
		case 0x0c:
		{
			*pDeviceData = 0x0ff;

			if (ata_isresponding(&symbiface_master_ata))
			{
				*pDeviceData =  ata_read(&symbiface_master_ata,ATA_CYLINDER_NUMBER_LOW_DA | ATA_CYLINDER_NUMBER_LOW_CS);
			}
			else if (ata_isresponding(&symbiface_slave_ata))
			{
				*pDeviceData =  ata_read(&symbiface_slave_ata,ATA_CYLINDER_NUMBER_LOW_DA | ATA_CYLINDER_NUMBER_LOW_CS);
			}
		}
		return TRUE;
			
		/* IDE Cylinder number high */
		case 0x0d:
		{
			*pDeviceData = 0x0ff;

			if (ata_isresponding(&symbiface_master_ata))
			{
				*pDeviceData =  ata_read(&symbiface_master_ata,ATA_CYLINDER_NUMBER_HIGH_DA | ATA_CYLINDER_NUMBER_HIGH_CS);
			}
			else if (ata_isresponding(&symbiface_slave_ata))
			{
				*pDeviceData =  ata_read(&symbiface_slave_ata,ATA_CYLINDER_NUMBER_HIGH_DA | ATA_CYLINDER_NUMBER_HIGH_CS);
			}
		}
		return TRUE;
				
				
		/* IDE head */
		case 0x0e:
		{
			*pDeviceData = 0x0ff;
			if (ata_isresponding(&symbiface_master_ata))
			{
				*pDeviceData =  ata_read(&symbiface_master_ata,ATA_HEAD_DA | ATA_HEAD_CS);
			}
			else  if (ata_isresponding(&symbiface_slave_ata))
			{
				*pDeviceData =  ata_read(&symbiface_slave_ata,ATA_HEAD_DA | ATA_HEAD_CS);
			}
		}
		return TRUE;
		
		/* IDE Status */
		case 0x0f:
		{
			*pDeviceData = 0x0ff;
			if (ata_isresponding(&symbiface_master_ata))
			{
				*pDeviceData =  ata_read(&symbiface_master_ata,ATA_STATUS_DA | ATA_STATUS_CS);
			}
			else if (ata_isresponding(&symbiface_slave_ata))
			{
				*pDeviceData = ata_read(&symbiface_slave_ata,ATA_STATUS_DA | ATA_STATUS_CS);
			}
			
		}
		return TRUE;
		
			
				case 0x010:
	{
		Z80_BYTE Data = 0x00;
		/* PS/2 protocol:

		mouse can send packet
		or host can choose to read packet

		symbiface must store it; or add it to it's values
		because it doesn't know when CPC will read it. CPC may choose to read
		it at any time.

		when read, it returns the value and then resets it.

		then it can know if value should be passed to CPC or not.

		*/
		if (sym2.SymbifaceReport == 0)
		{
			signed int MouseX = Mouse_GetX();
			signed int MouseY = Mouse_GetY();
			int MouseButtons = Mouse_GetButtons();

			if ( !sym2.bStoredMouseValid )
			{
				sym2.bStoredMouseValid = TRUE;
				sym2.StoredMouseX = MouseX;
				sym2.StoredMouseY = MouseY;
				sym2.StoredMouseButtons = MouseButtons;
			}


			sym2.StoredDeltaX = MouseX - sym2.StoredMouseX;
			sym2.StoredDeltaY = -( MouseY - sym2.StoredMouseY );
			sym2.StoredMouseX = MouseX;
			sym2.StoredMouseY = MouseY;
			if ( sym2.StoredDeltaX )
			{
				sym2.SymbifaceReport |= SYMBIFACE_REPORT_X;
			}
			if ( sym2.StoredDeltaY )
			{
				sym2.SymbifaceReport |= SYMBIFACE_REPORT_Y;
			}
			if ( sym2.StoredMouseButtons^MouseButtons )
			{
				sym2.StoredMouseButtons = MouseButtons;
				sym2.SymbifaceReport |= SYMBIFACE_REPORT_BUTTONS;
			}
		}

		if ( sym2.SymbifaceReport & SYMBIFACE_REPORT_X )
		{
			Data = ( SYMBIMOUSE_MODE_X_MOVEMENT << 6 ) | intto6bitsigned( sym2.StoredDeltaX );
			sym2.SymbifaceReport &= ~SYMBIFACE_REPORT_X;
		}
		else if ( sym2.SymbifaceReport & SYMBIFACE_REPORT_Y )
		{
			Data = ( SYMBIMOUSE_MODE_Y_MOVEMENT << 6 ) | intto6bitsigned( sym2.StoredDeltaY );
			sym2.SymbifaceReport &= ~SYMBIFACE_REPORT_Y;
		}
		else if ( sym2.SymbifaceReport & SYMBIFACE_REPORT_BUTTONS )
		{
			Data = ( SYMBIMOUSE_MODE_BUTTONS << 6 ) | sym2.StoredMouseButtons;
			sym2.SymbifaceReport &= ~SYMBIFACE_REPORT_BUTTONS;
		}

		*pDeviceData = Data;
	}
    return TRUE;

				/* enables roms */
				case 0x011:
				{
					sym2.RomEnable = TRUE;

					Computer_RethinkMemory();
				}
				return FALSE; /* no data returned I think */


				case 0x014:
				{
					/* rtc data */
					unsigned char Data = ds12887_read_data(&symbiface_rtc);
					//printf("Read ds12887 %02x\n", Data);
					*pDeviceData = Data;
				}
				return TRUE; 

				/* enable rom in &4000-&7fff */
				case 0x017:
				{
					sym2.RomEnable4000 = TRUE;
					Computer_RethinkMemory();
				}
				return FALSE; /* no data returned I think */

		default:
			//printf("Symbiface read not supported %04x", Port);
			break;

    }

//    // TODO
//	if (((Port & 0x01f) >= 6) && ((Port & 0x01f) <= 0x0f))
//	{
//		sym2.DataByteHigh = 0;
//	}
  
	return FALSE;
}

void SymbifaceRomWrite(Z80_WORD Port, Z80_BYTE Data)
{
	sym2.CurrentRom = Data;
	Computer_RethinkMemory();
}

void SymbifaceRamWrite(Z80_WORD Port, Z80_BYTE Data)
{
	if ((Data & 0x0c0)==0x0c0)
	{
		sym2.RamConfig = Data;
		Computer_RethinkMemory();
	}
}

BOOL Symbiface_Rom0Enabled(void)
{
	return sym2.RomEnables[0];
}

void Symbiface_Rom0Enable(BOOL bState)
{
	sym2.RomEnables[0] = bState;
}

BOOL Symbiface_Rom1Enabled(void)
{
	return sym2.RomEnables[1];
}

void Symbiface_Rom1Enable(BOOL bState)
{
	sym2.RomEnables[1] = bState;
}

BOOL Symbiface_Rom2Enabled(void)
{
	return sym2.RomEnables[2];
}

void Symbiface_Rom2Enable(BOOL bState)
{
	sym2.RomEnables[2] = bState;
}

BOOL Symbiface_Rom3Enabled(void)
{
	return sym2.RomEnables[3];
}

void Symbiface_Rom3Enable(BOOL bState)
{
	sym2.RomEnables[3] = bState;
}

BOOL Symbiface_Rom4Enabled(void)
{
	return sym2.RomEnables[4];
}

void Symbiface_Rom4Enable(BOOL bState)
{
	sym2.RomEnables[4] = bState;
}

BOOL Symbiface_Rom5Enabled(void)
{
	return sym2.RomEnables[5];
}

void Symbiface_Rom5Enable(BOOL bState)
{
	sym2.RomEnables[5] = bState;
}

BOOL Symbiface_Rom6Enabled(void)
{
	return sym2.RomEnables[6];
}

void Symbiface_Rom6Enable(BOOL bState)
{
	sym2.RomEnables[6] = bState;
}

BOOL Symbiface_Rom7Enabled(void)
{
	return sym2.RomEnables[7];
}

void Symbiface_Rom7Enable(BOOL bState)
{
	sym2.RomEnables[7] = bState;
}

BOOL Symbiface_J2Enabled(void)
{
	return sym2.J2;
}

void Symbiface_J2Enable(BOOL bState)
{
	sym2.J2 = bState;
}




static EmuDeviceSwitch SymbifaceSwitches[9] =
{
	{
		"ROM 0 Enable",          /* the switch on the top */
		"Rom0Enable",
		Symbiface_Rom0Enabled,
		Symbiface_Rom0Enable
	},
	{
		"ROM 1 Enable",          /* the switch on the top */
		"Rom1Enable",
		Symbiface_Rom1Enabled,
		Symbiface_Rom1Enable
	},
	{
		"ROM 2 Enable",          /* the switch on the top */
		"Rom2Enable",
		Symbiface_Rom2Enabled,
		Symbiface_Rom2Enable
	},
	{
		"ROM 3 Enable",          /* the switch on the top */
		"Rom3Enable",
		Symbiface_Rom3Enabled,
		Symbiface_Rom3Enable
	},
	{
		"ROM 4 Enable",          /* the switch on the top */
		"Rom4Enable",
		Symbiface_Rom4Enabled,
		Symbiface_Rom4Enable
	},
	{
		"ROM 5 Enable",          /* the switch on the top */
		"Rom5Enable",
		Symbiface_Rom5Enabled,
		Symbiface_Rom5Enable
	},
	{
		"ROM 6 Enable",          /* the switch on the top */
		"Rom6Enable",
		Symbiface_Rom6Enabled,
		Symbiface_Rom6Enable
	},
	{
		"ROM 7 Enable",          /* the switch on the top */
		"Rom7Enable",
		Symbiface_Rom7Enabled,
		Symbiface_Rom7Enable
	},
	{
		"J2 - (On = Disable ROMs 8-31,Off = Allow 0-31)",          /* the switch on the top */
		"J2",
		Symbiface_J2Enabled,
		Symbiface_J2Enable
	},
};

void Symbiface_MemoryRethink(MemoryData *pData)
{
	int Bank = (sym2.RamConfig>>3) & 0x07;
	int RomIndex = sym2.CurrentRom & 0x01f;

	switch (sym2.RamConfig & 0x07)
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
			pData->pWritePtr[(i << 1) + 0] = sym2.RAMStaticRAM + (Bank << 16);
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

	/* 3 will look correct on a 6128 because the PAL16L8 inside the CPC moves the block to &4000. The ram expansion
	then provides &c000-&ffff. */
	case 1:
	case 3:
	{
		pData->pWritePtr[6] = (sym2.RAMStaticRAM + (Bank << 16)) + (3 << 14) - 0x0c000;
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


	case 4:
	case 5:
	case 6:
	case 7:
	{
		/* 4000-7fff only */
		pData->pWritePtr[2] = sym2.RAMStaticRAM + (Bank << 16) + ((sym2.RamConfig & 0x03) << 14) - 0x04000;
		pData->pWritePtr[3] = pData->pWritePtr[2];

		pData->pReadPtr[2] = pData->pWritePtr[2];
		pData->pReadPtr[3] = pData->pWritePtr[3];
		pData->bRamDisable[2] = TRUE;
		pData->bRamDisable[3] = TRUE;
	}
	break;
	}
	
	if (/* software switch to enable */
		sym2.RomEnable && 
		/* not enabled at 4000-7fff */
		!sym2.RomEnable4000 &&
		/* rom slot is 0-7 and dip switch is enabled */
		(((RomIndex<8) && sym2.RomEnables[RomIndex]) || 
		/* rom slot is 8-31 and J2 is not active */
		((RomIndex>=8) && !sym2.J2))
		)
	{
		const unsigned char *pRomData = &sym2.ROMStaticRAM[RomIndex << 14] - 0x0c000;

		/* unknown if uses ROMEN */
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

	/* current ROM is visible in the range &4000-&7fff? */
	/* map it as RAM */
	if (sym2.RomEnable4000)
	{
		unsigned char *pRomData = &sym2.ROMStaticRAM[RomIndex << 14] - 0x04000;

		pData->bRamDisable[2] = TRUE;
		pData->bRamDisable[3] = TRUE;
		pData->pReadPtr[2] = pRomData;
		pData->pReadPtr[3] = pRomData;
		pData->pWritePtr[2] = pRomData;
		pData->pWritePtr[3] = pRomData;
	}

	
}


void SymbifaceWrite(Z80_WORD Port, Z80_BYTE Data)
{
//	//printf("Symbiface write %04x %02x\n", Port, Data);
/* ide reg? */
	if (((Port & 0x01f) >= 6) && ((Port & 0x01f) <= 0x0f))
	{
		/* write high byte? */
		if (sym2.DataByteHigh)
		{
			/* reset back */
			sym2.DataByteHigh = FALSE;
			sym2.IDEData = sym2.IDEData & 0x0ff;
			sym2.IDEData |= (Data & 0x0ff) << 8;

			if (!sym2.DataRead)
			{
				ata_write(&symbiface_master_ata, ATA_DATA_REGISTER_DA | ATA_DATA_REGISTER_CS, sym2.IDEData);
				ata_write(&symbiface_slave_ata, ATA_DATA_REGISTER_DA | ATA_DATA_REGISTER_CS, sym2.IDEData);
			}
			return;
		}
	}

	switch (Port & 0x01f)
	{
	
	case 8:
	{
		sym2.IDEData = Data & 0x0ff;
		sym2.DataByteHigh = TRUE;
		sym2.DataRead = FALSE; // is this correct?

		return;
	}

	case 0x9:
	{
		ata_write(&symbiface_master_ata,ATA_FEATURES_DA | ATA_FEATURES_CS, Data);
		ata_write(&symbiface_slave_ata, ATA_FEATURES_DA | ATA_FEATURES_CS, Data);
	}
	break;

	case 0x0A:
	{
		ata_write(&symbiface_master_ata,ATA_SECTOR_COUNT_DA | ATA_SECTOR_COUNT_CS, Data);
		ata_write(&symbiface_slave_ata, ATA_SECTOR_COUNT_DA | ATA_SECTOR_COUNT_CS, Data);
	}
	break;

	case 0x0b:
	{
		ata_write(&symbiface_master_ata,ATA_SECTOR_NUMBER_DA | ATA_SECTOR_NUMBER_CS, Data);
		ata_write(&symbiface_slave_ata, ATA_SECTOR_NUMBER_DA | ATA_SECTOR_NUMBER_CS, Data);
	}
	break;

	case 0x0c:
	{
		ata_write(&symbiface_master_ata,ATA_CYLINDER_NUMBER_LOW_DA | ATA_CYLINDER_NUMBER_LOW_CS, Data);
		ata_write(&symbiface_slave_ata, ATA_CYLINDER_NUMBER_LOW_DA | ATA_CYLINDER_NUMBER_LOW_CS, Data);
	}
	break;

	case 0x0d:
	{
		ata_write(&symbiface_master_ata,ATA_CYLINDER_NUMBER_HIGH_DA | ATA_CYLINDER_NUMBER_HIGH_CS, Data);
		ata_write(&symbiface_slave_ata, ATA_CYLINDER_NUMBER_HIGH_DA | ATA_CYLINDER_NUMBER_HIGH_CS, Data);
	}
	break;

	case 0x0e:
	{
		ata_write(&symbiface_master_ata,ATA_HEAD_DA | ATA_HEAD_CS, Data);
		ata_write(&symbiface_slave_ata, ATA_HEAD_DA | ATA_HEAD_CS, Data);
	}
	break;

	case 0x0f:
	{
		ata_write(&symbiface_master_ata,ATA_COMMAND_DA | ATA_COMMAND_CS, Data);
		ata_write(&symbiface_slave_ata, ATA_COMMAND_DA | ATA_COMMAND_CS, Data);
	}
	break;

	/* disable roms */
	case 0x011:
	{
		sym2.RomEnable = FALSE;
		Computer_RethinkMemory();
	}

	case 0x014:
	{
		ds12887_write_data(&symbiface_rtc, Data);
	}
	break;
	
	case 0x015:
	{
		ds12887_write_addr(&symbiface_rtc, Data);
	}
	break;

	/* disable rom &4000-&7fff */
	case 0x017:
	{
		sym2.RomEnable4000 = FALSE;
		Computer_RethinkMemory();
	}
	break;
	
	
	default:
	{

		//printf("Symbiface write not supported %04x %02x", Port, Data);
	}
	break;

	}

//	if (((Port & 0x01f) >= 6) && ((Port & 0x01f) <= 0x0f))
//	{
//		sym2.DataByteHigh = 0;
//	}

}

void Symbiface_Reset(void)
{
	sym2.RamConfig = 0;
	sym2.DataByteHigh = FALSE;
	sym2.IDEData = 0;
	ds12887_reset(&symbiface_rtc);
	ata_reset(&symbiface_master_ata);
	ata_reset(&symbiface_slave_ata);
}


static CPCPortRead symbifacePortRead[1]=
{
	{
		0x0ff00, /* and */
		0x0fd00, /* cmp */
		SymbifaceRead
	}
};

static CPCPortWrite symbifacePortWrite[3]=
{
	/* seems to just decode a15,a14?? */
	/* not confirmed yet */
	{
		0x0ff00, /* and */
		0x0fd00, /* cmp */
		SymbifaceWrite
	},
	{
		0x0ff00, /* confirmed through testing */
		0x0df00,
		SymbifaceRomWrite
	},
	{
		0xff00, /* confirmed through testing */
		0x7f00,
		SymbifaceRamWrite
	}
};

void Symbiface_ClearExpansionRom(int RomIndex)
{
	memset(sym2.ROMStaticRAM+(RomIndex<<14), 0x0ff, 16384);
}

BOOL Symbiface_SetExpansionRom(int RomIndex, const unsigned char *pData, unsigned long Length)
{
	EmuDevice_CopyRomData(sym2.ROMStaticRAM + (RomIndex << 14), 16384, pData, Length);
	return TRUE;
}

void Symbiface_InitDevice(void)
{
	int i;
	// master/single and slave
	ata_init(&symbiface_master_ata, ATA_JUMPER_MASTER_SINGLE, TRUE, 20 * 1024 * 1024);
	ata_init(&symbiface_slave_ata, ATA_JUMPER_SLAVE, TRUE, 128 * 1024 * 1024);

	// master and slave
//	ata_init(&symbiface_master_ata, ATA_JUMPER_MASTER_WITH_SLAVE_PRESENT, TRUE, 20 * 1024 * 1024);
//	ata_init(&symbiface_slave_ata, ATA_JUMPER_SLAVE, TRUE, 128 * 1024 * 1024);
	// master single
//	ata_init(&symbiface_master_ata, ATA_JUMPER_MASTER_SINGLE, TRUE, 20 * 1024 * 1024);
//	ata_init(&symbiface_slave_ata, ATA_JUMPER_SLAVE, FALSE, 128 * 1024 * 1024);
	// master and slave - no slave
//	ata_init(&symbiface_master_ata, ATA_JUMPER_MASTER_WITH_SLAVE_PRESENT, TRUE, 20 * 1024 * 1024);
//	ata_init(&symbiface_slave_ata, ATA_JUMPER_SLAVE, FALSE, 128 * 1024 * 1024);
// slave
//	ata_init(&symbiface_master_ata, ATA_JUMPER_MASTER_WITH_SLAVE_PRESENT, FALSE, 20 * 1024 * 1024);
//	ata_init(&symbiface_slave_ata, ATA_JUMPER_SLAVE, TRUE, 128 * 1024 * 1024);
	symbiface_master_ata.m_bSlaveDetected = symbiface_master_ata.m_bEnabled && symbiface_slave_ata.m_bEnabled;

	memset(sym2.ROMStaticRAM, 0x0ff, 512 * 1024);
	memset(sym2.RAMStaticRAM, 0x0ff, 512 * 1024);

	ExpansionRom_Init(&symbiface_roms, Symbiface_ClearExpansionRom, Symbiface_SetExpansionRom);
	for (i = 0; i < 32; i++)
	{
		ExpansionRom_SetAvailableState(&symbiface_roms, i, TRUE);
	}
	for (i = 0; i < 8; i++)
	{
		sym2.RomEnables[i] = FALSE;
	}
	sym2.RamConfig = 0;
}

void Symbiface_FinishDevice(void)
{
	ata_finish(&symbiface_master_ata);
	ata_finish(&symbiface_slave_ata);
}

static EmuDevice SymbifaceDevice=
{
	NULL,
	Symbiface_InitDevice,
	Symbiface_FinishDevice,
	"SYMBIFACE",
	"Symbiface2",
	"Symbiface 2",
	CONNECTION_EXPANSION,   /* connected to expansion */
	DEVICE_FLAGS_TESTED| DEVICE_WORKING,
	sizeof(symbifacePortRead) / sizeof(symbifacePortRead[0]),                /* 1 read port */
  symbifacePortRead,
  sizeof(symbifacePortWrite)/sizeof(symbifacePortWrite[0]),                    /* 1 write ports */
  symbifacePortWrite, 
  0,                /* no memory read*/
  NULL,
  0,                /* no memory write */
  NULL,
  Symbiface_Reset, /* reset function */
  Symbiface_MemoryRethink, /* memory rethink */
  Symbiface_Reset, /* power function*/
	sizeof(SymbifaceSwitches)/sizeof(SymbifaceSwitches[0]),      /* no switches*/
	SymbifaceSwitches,
    0,                      /* no buttons */
    NULL,
    0,                      /* no onboard roms */
    NULL,
      NULL,                   /* no cursor function */
	  &symbiface_roms, /* rom slots */
	  NULL,
	  NULL,
	  0,
	  NULL,
	  NULL, /* sound */
	  NULL, /* lpen */
	  NULL, /* reti */
	  NULL, /* ack maskable interrupt */
	  NULL, /* dkram data */
	  NULL, /* device ram */
	  NULL, /* device backup */
	  NULL
};

void Symbiface_Init(void)
{
	memset( &sym2, 0, sizeof( symbiface2 ) );
	sym2.bStoredMouseValid = FALSE;
	sym2.bStoreMouse = TRUE;
	sym2.StoredDeltaX = 0;
	sym2.StoredDeltaY = 0;
	sym2.StoredMouseX = 0;
	sym2.StoredMouseY = 0;
	sym2.StoredMouseButtons = 0;
	sym2.StoredHDData = 0;
	sym2.MouseX = 0;
	sym2.MouseY = 0;
	sym2.SymbifaceReport = 0;
	sym2.CurrentRom = 0;
	sym2.RomEnable = TRUE;
	sym2.RomEnable4000 = FALSE;
	sym2.RamConfig = 0;
	RegisterDevice(&SymbifaceDevice);
}
