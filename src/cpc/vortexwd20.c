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

unsigned char *VortexHardDisc = NULL;

typedef struct
{
	unsigned char Status;
	unsigned char SDH;
	unsigned char Precomp;
	unsigned char SectorCount;
	unsigned char SectorNumber;
	unsigned char ErrorRegister;
	unsigned char CylinderLow;
	unsigned char CylinderHigh;
	int BufferRemaining;
	int SectorSize;

	int nBufferPos;
	int CurrentCylinder;
	int SectorsPerTrack;
	int NumHeads;
	int CurrentHead;
	int BlockCount;
	int CurrentSector;
	unsigned char Buffer[ 256 ];
} wd1010;

wd1010 winchester;

unsigned char wd1010_read_status_register(wd1010 *wd)
{
	return wd->Status;
}


unsigned long wd1010_get_sector_offset( wd1010 *wd )
{
	unsigned long SectorOffset = ( wd->CurrentCylinder*wd->SectorsPerTrack*wd->NumHeads ) + ( wd->SectorsPerTrack * wd->CurrentHead ) + wd->CurrentSector;
	return SectorOffset;
}


void wd1010_transfer_sector_start(wd1010 *wd, int read)
{
	if (read)
	{
		unsigned long SectorOffset = wd1010_get_sector_offset(wd);
		unsigned long Offset = SectorOffset*256;
		memcpy(wd->Buffer, &VortexHardDisc[Offset], 256);
	}
	wd->nBufferPos = 0;
	wd->BufferRemaining= 256;
}

void wd1010_transfer_sector(wd1010 *wd, int read)
{

	if (!read)
	{
		unsigned long SectorOffset = wd1010_get_sector_offset(wd);
		unsigned long Offset = SectorOffset*256;
		memcpy( &VortexHardDisc[ Offset ], wd->Buffer, 256 );
	}

	wd->BlockCount--;
	if ( wd->BlockCount != 0 )
	{
		wd->CurrentSector++;
		if ( wd->CurrentSector == wd->SectorsPerTrack )
		{
			wd->CurrentSector = 0;
			wd->CurrentHead++;
			if ( wd->CurrentHead == wd->NumHeads )
			{
				wd->CurrentHead = 0;
				wd->CurrentCylinder++;
			}
		}

		wd1010_transfer_sector_start(wd, read);
	}
	else
	{
		wd->Status &= ~(1 << 7);
		/* no more data request */
		wd->Status &= ~(1 << 3);
	}
}


unsigned char wd1010_read_data_register(wd1010 *wd)
{
	unsigned char Data = wd->Buffer[wd->nBufferPos];
	wd->nBufferPos++;
	wd->BufferRemaining--;
	if (wd->BufferRemaining == 0)
	{
		wd1010_transfer_sector(wd, 1);
	}
	return Data;
}
void wd1010_write_data_register(wd1010 *wd, int Data)
{
}

void wd1010_reset(wd1010 *wd)
{
	memset( wd, 0, sizeof( wd1010 ) );

	wd->Status = 0x0;
	wd->ErrorRegister = 0x0;
	wd->SectorsPerTrack = 32;
	wd->NumHeads = 4;
}

void wd1010_write_sector_count_register(wd1010 *wd, int Data)
{
	wd->SectorCount = Data;
	//printf("Sector Count %d\n", Data);
}
int wd1010_read_sector_count_register(wd1010 *wd)
{
	return wd->SectorCount ;
}

void wd1010_write_precomp_register(wd1010 *wd, int Data)
{
	wd->Precomp = Data;
	//printf("Precomp %d\n", Data);
}

void wd1010_write_cylinder_low_register(wd1010 *wd, int Data)
{
	wd->CylinderLow= Data;
//	printf("Cylinder low %d\n", Data);
}

int wd1010_read_cylinder_low_register(wd1010 *wd)
{
	return wd->CylinderLow;

}
void wd1010_write_cylinder_high_register(wd1010 *wd, int Data)
{
	wd->CylinderHigh = Data;
	//printf("Cylinder high %d\n", Data);
}
int wd1010_read_cylinder_high_register(wd1010 *wd)
{
	return wd->CylinderHigh;

}

int wd1010_read_error_register(wd1010 *wd)
{
	return wd->ErrorRegister;

}

void wd1010_write_sector_number_register(wd1010 *wd, int Data)
{
	wd->SectorNumber = Data;
	//printf("Sector number %d\n", Data);
}
int wd1010_read_sector_number_register(wd1010 *wd)
{
	return wd->SectorNumber;
}

void write_drive_select(wd1010 *wd, int Data)
{
	/* drive select? */
	wd->Status |= (1 << 6);
}

void wd1010_write_sdh_register(wd1010 *wd, int Data)
{

	wd->SDH = Data;
//	printf("Drive %d\n", (wd->SDH >> 3)&0x03);
//	printf("Head %d\n", (wd->SDH & 0x07));
	switch (wd->SDH & ((1 << 5) | (1 << 6)))
	{
	case 0:
	{
//		printf("Sector size: 256\n");
		wd->SectorSize = 256;
	}
	break;
	case (1 << 5):
	{
	//	printf("Sector size: 512\n");
		wd->SectorSize = 512;
	}
	break;
	case (1<<6):
	{
//		printf("Sector size: 1024\n");
		wd->SectorSize = 1024;
	}
	break;
	case (1<<5)|(1<<6):
	{
//		printf("Sector size: 128\n");
		wd->SectorSize = 128;
	}
	break;

	}
}

int wd1010_read_sdh_register(wd1010 *wd)
{

	return wd->SDH;
}


void wd1010_write_command_register(wd1010 *wd, int Data)
{
	//printf("WD1010 command %02x\n", Data);

	/* clear error */
	wd->Status &= ~(1 << 0);
	/* set command in progress */
	wd->Status |= (1 << 1);
	/* set busy */
	//wd->Status |= (1 << 7);

	
	switch ((Data >> 4) & 0x0f)
	{
		/* restore */
	case 0x1:
	{
		/* restore */
		/* TODO: Perform seek, error messages etc */
		/* reset busy */
		/* set busy */
		wd->Status |= (1 << 6);
		wd->Status &= ~(1 << 7);
		/* reset command in progress */
		wd->Status &= ~(1 << 1);

	}
	break;
	case 0x02:
	{
		/* read sector */
	
		/* ready */
		wd->Status |= (1 << 6);
		/* seek complete */
		wd->Status |= (1 << 4);
		/* drq */
		wd->Status |= (1 << 3);
		wd->BufferRemaining = wd->SectorSize;

		wd->CurrentCylinder = ( wd->CylinderLow & 0x0ff ) | ( ( wd->CylinderHigh & 0x03 ) << 8 );
		wd->CurrentSector = wd->SectorNumber & 0x0ff;
		wd->CurrentHead = wd->SDH & 0x0f;
		wd->BlockCount = wd->SectorCount;
		if ( wd->BlockCount == 0 )
		{
			wd->BlockCount = 256;
		}
		wd1010_transfer_sector_start(wd,1);

	}
	break;
	case 0x07:
	{
		/* restore */
		/* TODO: Perform seek, error messages etc */
		/* reset busy */
		/* set busy */
		wd->Status |= (1 << 6);
		wd->Status &= ~(1 << 7);
		/* reset command in progress */
		wd->Status &= ~(1 << 1);
	}
	break;

	default:
		break;

		
	}
}

BOOL Vortex_wd20_read(Z80_WORD Addr, Z80_BYTE *pDeviceData)
{
	int Register = ((Addr >> (9 - 2) & (1 << 2)) | (Addr & 0x03));
	switch (Register)
	{
	case 0:
	{
		*pDeviceData = wd1010_read_data_register(&winchester);
	}
	return TRUE;
	case 1:
	{
		*pDeviceData =  wd1010_read_error_register(&winchester);
	}
	return TRUE;
	case 2:
	{
		*pDeviceData =  wd1010_read_sector_count_register(&winchester);
	}
	return TRUE;
	case 3:
	{
		*pDeviceData =  wd1010_read_sector_number_register(&winchester);
	}
	return TRUE;
	case 4:
	{
		*pDeviceData =  wd1010_read_cylinder_low_register(&winchester);
	}
	return TRUE;
	case 5:
	{
		*pDeviceData =  wd1010_read_cylinder_high_register(&winchester);
	}
	return TRUE;
	case 6:
	{
		*pDeviceData =  wd1010_read_sdh_register(&winchester);
	}
	return TRUE;
	case 7:
	{
		*pDeviceData =  wd1010_read_status_register(&winchester);
	}
	return TRUE;
	}

	return FALSE;
}

void Vortex_wd20_write(Z80_WORD Addr, Z80_BYTE Data)
{
	int Register = ((Addr >> (9 - 2) & (1 << 2)) | (Addr & 0x03));
	switch (Register)
	{
	case 0:
	{
		wd1010_write_data_register(&winchester, Data);
	}
	break;
	case 1:
	{
		wd1010_write_precomp_register(&winchester, Data);
	}
	break;
	case 2:
	{
		wd1010_write_sector_count_register(&winchester, Data);
	}
	break;
	case 3:
	{
		wd1010_write_sector_number_register(&winchester, Data);
	}
	break;
	case 4:
	{
		wd1010_write_cylinder_low_register(&winchester, Data);
	}
	break;
	case 5:
	{
		wd1010_write_cylinder_high_register(&winchester, Data);
	}
	break;
	case 6:
	{
		wd1010_write_sdh_register(&winchester, Data);
		write_drive_select(&winchester, Data);
	}
	break;
	case 7:
	{
		wd1010_write_command_register(&winchester, Data);
	}
	break;
	}

}


static CPCPortRead VortexRead[1]=
{
	{
		0x0f8f8,
		0x0f8f8,
		Vortex_wd20_read
	}
};


static CPCPortWrite VortexWrite[1]=
{
	{
		0x0f8f8,
		0x0f8f8,
		Vortex_wd20_write
	}
};

void Vortexwd20Device_Init(void)
{
	wd1010_reset(&winchester);

	VortexHardDisc = (unsigned char *)malloc(20 * 1024 * 1024);
	memset(VortexHardDisc, 0xe5, 20 * 1024 * 1024);

	// init format for vortex
	{
		int i;
		const char *pPassword = "CPC";
		unsigned long SectorOffset = 0;	// wd1010_get_sector_offset();
		unsigned long Offset = SectorOffset * 256;
		unsigned char *pSector = &VortexHardDisc[Offset];
		memset(pSector, 0, 256);
		pSector[240] = 0x0ff;
		pSector[241] = strlen(pPassword);		// password length
		memcpy(&pSector[242], pPassword, strlen(pPassword));
		for (i = 0; i < strlen(pPassword); i++)
		{
			pSector[242 + i] ^= 0x0ff;
		}
		pSector[1] = 0;		// be16, !=0 = be18
		pSector[252] = 1;
		pSector[7] = 8;
		pSector[8] = 2;
		for (i = 9; i < 239; i++)
		{
			pSector[i] = 8;
		}
		for (i = 2; i < 7; i++)
		{
			pSector[i] = 4;
		}
		// byte 0: checksum of bytes 1 to 255 in sector
		// byte 7: number of partitions
		// byte 8: ??
		// byte 240: !=0 => no password, 0 => password
		// byte 241: password length
		// byte 242: password characters CPLed (up to 10 characters?)
		// byte 252: drive id (0=a, 1=b, 2=c etc)
		
		{
			// init checksum
			int Checksum = 0;
			for (i = 1; i < 256; i++)
			{
				Checksum += VortexHardDisc[Offset + i];
			}
			VortexHardDisc[Offset] = (Checksum&0x0ff);
		}

	}

}
void Vortexwd20Device_Exit(void)
{
	free(VortexHardDisc);
}
static EmuDevice Vortexwd20Device =
{
	NULL,
	Vortexwd20Device_Init,
	Vortexwd20Device_Exit,
	"VTXWD20",
	"VortexWD20",
	"Vortex wd20",
	CONNECTION_EXPANSION,   /* connected to expansion */
	0,
	sizeof(VortexRead)/sizeof(VortexRead[0]),                /* 1 read port */
	VortexRead,
	sizeof(VortexWrite) / sizeof(VortexWrite[0]),                /* 1 read port */
	VortexWrite,
	0,                /* no memory read*/
	NULL,
	0,                /* no memory write */
	NULL, 
	NULL, /* reset function */
	NULL, /* memory rethink */
	NULL, /* power function*/
	0,      /* no switches*/
	NULL,
	0,                      /* no buttons */
	NULL,
	0,                      /* no onboard roms */
	NULL,
	NULL,                   /* no cursor function */
	NULL,                 /* no generic roms */
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
	NULL,

};

void Vortexwd20_Init(void)
{
	RegisterDevice(&Vortexwd20Device);
}

