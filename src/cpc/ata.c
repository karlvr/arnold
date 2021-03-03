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
#include "cpcglob.h"
#include "ata.h"

// TODO: Define which version of ATA command set, if packet or Compactflash or similar is supported.

//ftp://ftp.seagate.com/acrobat/reference/111-1c.pdf

void ata_chs_to_lba(ata_device *device)
{
	int Cylinder = (device->CylinderLow & 0x0ff) | ((device->CylinderHigh & 0x0ff) << 8);

	device->CurrentLBASectorIndex = (((Cylinder * device->Heads) + (device->HeadRegister & 0x0f)) * device->SectorsPerTrack) +
		((device->SectorNumberRegister & 0x0ff) - 1);
}

void ata_update_chs(ata_device *device)
{
	device->SectorNumberRegister++;
	if (device->SectorNumberRegister == (device->SectorsPerTrack+1))
	{
		device->SectorNumberRegister = 1;
		
		{
			int Heads = device->HeadRegister & 0x0f;
			Heads++;
			if (Heads == device->Heads)
			{
				int Cylinders = (device->CylinderLow & 0x0ff) | ((device->CylinderHigh & 0x0ff) << 8);

				device->CylinderLow = Cylinders & 0x0ff;
				device->CylinderHigh = (Cylinders >> 8) & 0x0ff;
			}
			else
			{
				device->HeadRegister = (device->HeadRegister & 0x0f0) | Heads;
			}
		}
	}
}


void ata_mark_sector_exists(ata_device *device, int nSector)
{
	int nByte = nSector >> 3;
	int nBit = nSector & 0x07;
	device->SectorExistsMap[nByte] |= (1 << nBit);
}

BOOL ata_sector_exists(ata_device *device, int nSector)
{
	int nByte = nSector >> 3;
	int nBit = nSector & 0x07;
	if (device->SectorExistsMap[nByte] & (1 << nBit))
		return TRUE;

	return FALSE;
}

unsigned char *ata_get_sector_data(ata_device *device, int nSector)
{
	if (!ata_sector_exists(device, nSector))
	{
		/* mark as existing */
		ata_mark_sector_exists(device, nSector);
		/* allocate */
		device->SectorPointers[nSector] = (unsigned char *)malloc(ATA_SECTOR_SIZE);
		if (device->SectorPointers[nSector])
		{
			memset(device->SectorPointers[nSector], ATA_DEFAULT_BYTE, ATA_SECTOR_SIZE);
		}
	}
	return device->SectorPointers[nSector];
}

#if 0
void ata_chs_partition(ata_device *device, unsigned char *pCHS, int Sector)
{
	unsigned short C;
	unsigned char H;
	unsigned char S;
	ata_lba_to_chs(device, Sector, &C, &H, &S);
	pCHS[0] = H;
	pCHS[2] = C & 0x0ff;
	pCHS[1] = (S & 0x03f) | ((C >> 2) & 0x0c0);
}
#endif

#if 0
void ata_setup_partition(ata_device *device, unsigned char *pPartition, int StartSector, int LengthSectors)
{
	pPartition[0] = 0x080;
	ata_chs_partition(device, &pPartition[1], StartSector);
	pPartition[4] = 0x01;
	ata_chs_partition(device, &pPartition[5], StartSector + LengthSectors);
	pPartition[8] = StartSector & 0x0ff;
	pPartition[9] = (StartSector>>8) & 0x0ff;
	pPartition[10] = (StartSector>>16) & 0x0ff;
	pPartition[11] = (StartSector>>24) & 0x00f;
	pPartition[12] = (LengthSectors & 0x0ff);
	pPartition[13] = ((LengthSectors>>8) & 0x0ff);
	pPartition[14] = ((LengthSectors >> 16) & 0x0ff);
	pPartition[15] = ((LengthSectors >> 24) & 0x00f);

}
#endif


BOOL ata_isselected(ata_device *device)
{
	/* this device is selected */
	int nSelectedDevice = (device->HeadRegister >> 4) & 0x01;
	int nThisDevice = device->m_bMaster ? 0 : 1;

	if (!device->m_bEnabled)
		return FALSE;

	return (nSelectedDevice == nThisDevice);
}

BOOL ata_isansweringforotherdevice(ata_device *device)
{
	/* this device is selected */
	int nSelectedDevice = (device->HeadRegister >> 4) & 0x01;

	/* master answering for slave? */
	if (device->m_bMaster && !device->m_bSlaveDetected && (nSelectedDevice == 1))
	{
		/* device 0 answering for device 1 */
		if (device->m_bAnswersForOtherDevice)
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL ata_isresponding(ata_device *device)
{
	if (!device->m_bEnabled)
		return FALSE;

	{
		/* this device is selected */
		int nSelectedDevice = (device->HeadRegister >> 4) & 0x01;
		int nThisDevice = device->m_bMaster ? 0 : 1;

		if (ata_isansweringforotherdevice(device))
			return TRUE;

		return (nSelectedDevice == nThisDevice);
	}
}

#if 0
const char DeviceID[512] =
{
 0x05A,0x004,0x0F4,0x001,0x000,0x000,0x010,0x000,0x000,0x000,0x000,0x002,0x020,0x000,0x003,0x000,
 0x000,0x0E8,0x000,0x000,0x04F,0x044,0x035,0x04D,0x030,0x044,0x030,0x030,0x037,0x031,0x032,0x034,
 0x000,0x039,0x020,0x020,0x020,0x020,0x020,0x020,0x001,0x000,0x001,0x000,0x004,0x000,0x030,0x04E,
 0x030,0x035,0x031,0x033,0x044,0x036,0x051,0x050,0x020,0x049,0x044,0x049,0x020,0x045,0x069,0x044,
 0x06B,0x073,0x06E,0x04F,0x06F,0x04D,0x075,0x064,0x065,0x06C,0x020,0x020,0x020,0x020,0x020,0x020,
 0x020,0x020,0x020,0x020,0x020,0x020,0x020,0x020,0x020,0x020,0x020,0x020,0x020,0x020,0x001,0x000,
 0x000,0x000,0x000,0x00F,0x000,0x000,0x000,0x002,0x000,0x000,0x003,0x000,0x0F4,0x001,0x010,0x000,
 0x020,0x000,0x000,0x0E8,0x003,0x000,0x000,0x001,0x000,0x0E8,0x003,0x000,0x000,0x000,0x007,0x004,
 0x003,0x000,0x078,0x000,0x078,0x000,0x078,0x000,0x078,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
 0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
 0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
 0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
 0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
 0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
 0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
 0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
 0x000,0x000,0x035,0x030,0x033,0x030,0x036,0x031,0x038,0x062,0x000,0x000,0x000,0x000,0x000,0x000,
 0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
 0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
 0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
 0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
 0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
 0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
 0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
 0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
 0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
 0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000
};
#endif

void ata_init(ata_device *device, ATA_JUMPER_SETTING  JumperSetting, BOOL bEnabled, int nCapacity)
{
	switch (JumperSetting)
	{
		default:
		case ATA_JUMPER_MASTER_WITH_SLAVE_PRESENT:
		{
			device->m_bMaster = TRUE;
			device->m_bAnswersForOtherDevice = FALSE;
		}
		break;
		
		case ATA_JUMPER_SLAVE:
		{
			device->m_bMaster = FALSE;
			device->m_bAnswersForOtherDevice = FALSE;
		}
		break;
		
		case ATA_JUMPER_MASTER_SINGLE:
		{
			device->m_bMaster = TRUE;
			device->m_bAnswersForOtherDevice = TRUE;
		}
		break;
	}
			
			
	device->JumperSetting = JumperSetting;
	device->PowerMode = ATA_POWER_MODE_ACTIVE;
	device->DescriptiveErrorCode = 0;
	device->m_bEnabled = bEnabled;
	device->b8BitTransfer = FALSE;
	device->m_bSlaveDetected = FALSE;

	/* need to set drq? */
	device->DeviceMaxSectors = nCapacity / ATA_SECTOR_SIZE;
	//device->DeviceMaxSectors = 0x03e800;
//	device->MaxCylinders = 0x01f4;
//	device->Heads = 0x010;
//	device->SectorsPerTrack = 0x020;
	int SectorExistsMapSize;

	SectorExistsMapSize = device->DeviceMaxSectors >> 3;
	device->SectorExistsMap = (unsigned char *)malloc(SectorExistsMapSize);
	device->SectorPointers = (unsigned char **)malloc(sizeof(unsigned char *)*device->DeviceMaxSectors);
	memset(device->SectorPointers, 0, sizeof(unsigned char *)*device->DeviceMaxSectors);

	memset(device->SectorExistsMap, 0, SectorExistsMapSize);

	device->SectorsPerTrack = 63;
	device->Heads = 16;
	device->MaxCylinders = device->DeviceMaxSectors / (device->SectorsPerTrack*device->Heads);

//	device->Heads = 1;
	//device->SectorsPerTrack = 32;
	memset(device->DeviceIdentification, 0, sizeof(device->DeviceIdentification));
//	memcpy(device->DeviceIdentification, DeviceID, 512);

#if 0
	if (device->DeviceMaxSectors >= MAX_CHS_SECTORS)
	{
		/* set LBA; correct?? */
		device->HeadRegister |= (1 << 6);
		device->HeadRegister |= (1 << 7);
		device->HeadRegister |= (1 << 5);
	}
#endif
	{
		/* TODO fill in remaining */

		/* total number of user addressable sectors (LBA mode) */
		device->DeviceIdentification[0] = (1 << 6);

		int NumberOfCylinders = 16383;
		if (device->DeviceMaxSectors < MAX_CHS_SECTORS)
		{
			NumberOfCylinders = device->MaxCylinders+1;
		}
		device->DeviceIdentification[(1 * 2)] = (NumberOfCylinders &0x0ff);
		device->DeviceIdentification[(1 * 2) + 1] = ((NumberOfCylinders >> 8) &0x0ff);

		int NumberOfHeads = 16;
		if (device->DeviceMaxSectors < MAX_CHS_SECTORS)
		{
			NumberOfHeads = device->Heads;
		}
		device->DeviceIdentification[(3 * 2)] = NumberOfHeads &0x0ff;
		device->DeviceIdentification[(3 * 2) + 1] = (NumberOfHeads >> 8) &0x0ff;

		int NumberOfSectorsPerCylinder = 63;
		if (device->DeviceMaxSectors < MAX_CHS_SECTORS)
		{
			NumberOfSectorsPerCylinder = device->SectorsPerTrack;
		}
		device->DeviceIdentification[(6 * 2)] = NumberOfSectorsPerCylinder &0x0ff;
		device->DeviceIdentification[(6 * 2) + 1] = (NumberOfSectorsPerCylinder >> 8) &0x0ff;

		/* high byte then low byte? */
		device->DeviceIdentification[(49*2)+1] |= (1<<1)|(1<<0); // supports LBA (bit 1) and DMA (bit 0)

		device->DeviceIdentification[(54 * 2)] = device->MaxCylinders &0x0ff;
		device->DeviceIdentification[(54 * 2) + 1] = (device->MaxCylinders >> 8) &0x0ff;
		device->DeviceIdentification[(55 * 2)] = device->Heads &0x0ff;
		device->DeviceIdentification[(56 * 2)] = device->SectorsPerTrack &0x0ff;

		const char *sIdentification = "Emulated IDE Harddisc";
		memset(&device->DeviceIdentification[10 * 2], '0', 20);
		memset(&device->DeviceIdentification[23 * 2], '0', 8);
		memset(&device->DeviceIdentification[27 * 2], ' ', 40);
		memcpy(&device->DeviceIdentification[27 * 2], sIdentification, strlen(sIdentification));
		{
			int i;
			/* bytes appeared to be reversed, at least they appear to be this way on BonnyDOS whereas the other values
			are not reversed */
			for ( i = 0; i < 20; i++)
			{
				unsigned char Byte = device->DeviceIdentification[(27 * 2) + (i << 1)];
				device->DeviceIdentification[(27 * 2) + (i << 1)] = device->DeviceIdentification[(27 * 2) + (i << 1) + 1];
				device->DeviceIdentification[(27 * 2) + (i << 1) + 1] = Byte;
			}
		}
			
		device->DeviceIdentification[(60 * 2) + 0] = device->DeviceMaxSectors &0x0ff;
		device->DeviceIdentification[(60 * 2) + 1] = (device->DeviceMaxSectors >> 8) &0x0ff;
		device->DeviceIdentification[(60 * 2) + 2] = (device->DeviceMaxSectors >> 16) &0x0ff;
		device->DeviceIdentification[(60 * 2) + 3] = (device->DeviceMaxSectors >> 24) &0x0f;
		{
			int VersionsSupported = (1 << 8);
			device->DeviceIdentification[(80 * 2) + 0] = VersionsSupported &0x0ff;
			device->DeviceIdentification[(80 * 2) + 1] = (VersionsSupported>>8) &0x0ff;
		}
	}
//	{
//		unsigned char *pMBR = ata_get_sector_data(device, 0);
//		ata_setup_partition(device, &pMBR[0x01be], 1, device->DeviceMaxSectors - 1);
//	}
}

void ata_finish(ata_device *device)
{
	int i;
	for (i = 0; i < device->DeviceMaxSectors; i++)
	{
		if (device->SectorPointers[i] != NULL)
		{
			free(device->SectorPointers[i]);
			device->SectorPointers[i] = NULL;
		}
	}
	free(device->SectorPointers);
	free(device->SectorExistsMap);
}

void ata_reset(ata_device *device)
{
	device->PowerMode = ATA_POWER_MODE_ACTIVE;
	device->b8BitTransfer = FALSE;
	device->StatusRegister = ATA_STATUS_DSC;
	/* device is ready */
	device->StatusRegister |= ATA_STATUS_DRDY;

	device->SectorsRemaining = 0;
	device->DataSize = 0;

	device->ErrorRegister = 1;
	device->SectorCountRegister = 1;
	device->SectorNumberRegister = 1;
	device->CylinderLow = 0;
	device->CylinderHigh = 0;
	device->HeadRegister = 0;
}

unsigned char ata_get_status_register(ata_device *device)
{
	/* ATA-6 specification describes this */
	if (ata_isansweringforotherdevice(device))
		return 0;

	if (device->StatusRegister & ATA_STATUS_BSY)
	{
		/* all other bits are not valid when BSY is set*/
		/* for now set them all to 1 */
		device->StatusRegister |= ~ATA_STATUS_BSY;
	}
	return device->StatusRegister;
}

int ata_read(ata_device *device, int reg)
{
	if (!device->m_bEnabled)
		return 0x0ff;

	
	
	switch (reg)
	{
	case ATA_ALTERNATE_STATUS_CS | ATA_ALTERNATE_STATUS_DA:
	{
		/* alternative status register */
		return ata_get_status_register(device);
	}
	case ATA_DRIVE_ADDRESS_CS | ATA_DRIVE_ADDRESS_DA:
	{
		int data = 0x083;
		data &= ~(1 << (device->HeadRegister >> 4));
		data |= (device->HeadRegister & 0x0f) ^ 0x0ff;
		return data;
	}
	case ATA_STATUS_CS | ATA_STATUS_DA:
	{
		/* status register */
		return ata_get_status_register(device);
	}


	case ATA_CYLINDER_NUMBER_HIGH_CS | ATA_CYLINDER_NUMBER_HIGH_DA:
	{
		/* ATA specification 4 says this register not valid if BSY set */
		/* On X-mass this returns status when device is busy*/
		if ((device->StatusRegister & ATA_STATUS_BSY)!=0)
		{
			return ata_get_status_register(device);
		}

		/* cylinder high register */
		return device->CylinderHigh;
	}

	case ATA_CYLINDER_NUMBER_LOW_CS | ATA_CYLINDER_NUMBER_LOW_DA:
	{
		/* ATA specification 4 says this register not valid if BSY set */
		/* On X-mass this returns status when device is busy*/
		if ((device->StatusRegister & ATA_STATUS_BSY)!=0)
		{
			return ata_get_status_register(device);
		}
		/* cylinder low register */
		return device->CylinderLow;
	}

	case ATA_DATA_REGISTER_CS | ATA_DATA_REGISTER_DA:
	{
		if ((device->StatusRegister & ATA_STATUS_DRQ) == 0)
			return 0; /* not sure what to return if there is no data request */

		/* On X-mass this returns status when device is busy*/
		if ((device->StatusRegister & ATA_STATUS_BSY)!=0)
		{
			return ata_get_status_register(device);
		}

		{
			unsigned short Data = 0x0;
			if (device->b8BitTransfer)
			{
				Data = device->Buffer[device->DataSize] &0x0ff;
				device->DataSize++;
			}
			else
			{
				Data = (device->Buffer[device->DataSize] &0x0ff)|
				((device->Buffer[device->DataSize+1] &0x0ff) << 8);
				device->DataSize += 2;
			}
			if (device->DataSize == ATA_SECTOR_SIZE)
			{
				device->SectorCountRegister--;
				device->SectorsRemaining--;
				if (device->SectorsRemaining == 0)
				{
					/* no more data */
					device->StatusRegister &= ~ATA_STATUS_DRQ;
					device->StatusRegister &= ~ATA_STATUS_BSY;
					device->DescriptiveErrorCode = ATA_REQUEST_SENSE_NO_ERROR;
				}
				else
				{
					device->DataSize = 0;

					/* LBA? */
					if (device->HeadRegister & ATA_HEAD_LBA)
					{
						device->CurrentLBASectorIndex++;

						/* update registers with LBA of sector written */
						device->HeadRegister = ((device->CurrentLBASectorIndex >> 24) &0x0f) | (device->HeadRegister &0x0f0);
						device->CylinderHigh = (device->CurrentLBASectorIndex >> 16) &0xff;
						device->CylinderLow = (device->CurrentLBASectorIndex >> 8) &0xff;
						device->SectorNumberRegister = device->CurrentLBASectorIndex &0xff;
					}
					else
					{
						/* update chs, convert to lba for access */
						ata_update_chs(device);
						ata_chs_to_lba(device);
					}

					/* if sector is not valid indicate the error */
					if (device->CurrentLBASectorIndex >= device->DeviceMaxSectors)
					{
						device->SectorCountRegister = 0;
						device->StatusRegister &= ~ATA_STATUS_DRQ;
						device->StatusRegister &= ~ATA_STATUS_BSY;
						device->StatusRegister |= ATA_STATUS_ERR;
						device->ErrorRegister |= ATA_ERROR_ABRT;
						device->ErrorRegister |= ATA_ERROR_IDNF;
						device->DescriptiveErrorCode = ATA_REQUEST_SENSE_INVALID_ADDRESS;
					}
					else
					{
						if (ata_sector_exists(device, device->CurrentLBASectorIndex))
						{
							unsigned char *pSector = ata_get_sector_data(device, device->CurrentLBASectorIndex);
							memcpy(device->Buffer, pSector, 512);
						}
						else
						{
							memset(device->Buffer, ATA_DEFAULT_BYTE, 512);
						}
					}
				}
			}
			return Data;
		}
	}
	break;

	case ATA_ERROR_CS | ATA_ERROR_DA:
	{

		/* ATA specification 4 says this register not valid if BSY or DRQ set */
		/* On X-mass this returns status when device is busy*/

		if (!((device->StatusRegister & (ATA_STATUS_BSY | ATA_STATUS_DRQ)) == 0))
		{
			return ata_get_status_register(device);
		}

		return device->ErrorRegister;
	}

	case ATA_SECTOR_COUNT_CS | ATA_SECTOR_COUNT_DA:
	{
		/* ATA specification 4 says this register not valid if BSY or DRQ set */
		/* On X-mass this returns status when device is busy*/
		if (!((device->StatusRegister & (ATA_STATUS_BSY | ATA_STATUS_DRQ)) == 0))
		{
			return ata_get_status_register(device);
		}
		return device->SectorCountRegister;
	}

	case ATA_SECTOR_NUMBER_CS | ATA_SECTOR_NUMBER_DA:
	{
		/* ATA specification 4 says this register not valid if BSY or DRQ set */
		/* On X-mass this returns status when device is busy*/
		if (!((device->StatusRegister & (ATA_STATUS_BSY | ATA_STATUS_DRQ)) == 0))
		{
			return ata_get_status_register(device);
		}
		return device->SectorNumberRegister;
	}

	case ATA_HEAD_CS | ATA_HEAD_DA:
	{
		/* ATA specification 4 says this register not valid if BSY set */
		/* On X-mass this returns status when device is busy*/
		if ((device->StatusRegister & ATA_STATUS_BSY) != 0)
		{
			return ata_get_status_register(device);
		}

		return device->HeadRegister;
	}

	default:
	{
		//printf("Unhandled!");
		break;

	}
	}

	return 0x0;
}

void ata_calc_initial_sector_index(ata_device *device)
{
	if (device->HeadRegister & ATA_HEAD_LBA)
	{
		device->CurrentLBASectorIndex =
			((device->HeadRegister &0x0f) << 24) |
			((device->CylinderHigh &0x0ff) << 16) |
			((device->CylinderLow &0x0ff) << 8) |
			(device->SectorNumberRegister &0x0ff);
	}
	else
	{
		/* update chs, convert to lba for access */
		ata_chs_to_lba(device);
	}
}

void ata_write(ata_device *device, int reg, int data)
{	
	if (!device->m_bEnabled)
		return;


	switch (reg)
	{
		case ATA_CS1_REG_BIT| ATA_DA2_REG_BIT| ATA_DA1_REG_BIT:
		{
			/* Both drives respond */

			/* device control register */
			device->DeviceControl = data;
		}
		break;

		case ATA_COMMAND_CS | ATA_COMMAND_DA:
		{
			/* command register */

			/* clear interrupt */

			/* drq and bsy should be 0 */
			if (data == ATA_COMMAND_EXECUTE_DEVICE_DIAGNOSTIC)
			{
				/* we do this regardless of whether we are selected or not */
				device->CommandRegister = data;
				device->ErrorRegister = 0x01;
				if (device->m_bMaster)
				{
					if (device->m_bAnswersForOtherDevice && !device->m_bSlaveDetected)
					{
						/* returns results for master */
					}
					else
					{

						if (!device->m_bSlaveDetected)
						{
							device->ErrorRegister |= 0x080;
						}
					}
				}
				device->SectorCountRegister = 1;
				device->SectorNumberRegister = 1;
				device->CylinderLow = 0;
				device->CylinderHigh = 0;
				device->HeadRegister = 0;
				device->StatusRegister = ATA_STATUS_DRDY | ATA_STATUS_DSC;
				break;
			}


			/* check we are the selected device by checking DEV bit in head register */
			int nSelectedDevice = (device->HeadRegister >> 4) &0x01;
			int nThisDevice = device->m_bMaster ? 0 : 1;
			if (nSelectedDevice != nThisDevice)
				return;

			/* do not accept if busy set */
			if (device->StatusRegister & ATA_STATUS_BSY)
				return;

			device->CommandRegister = data;
			device->StatusRegister |= ATA_STATUS_BSY;
			device->StatusRegister &= ~ATA_STATUS_ERR;
			device->ErrorRegister = 0;
			
			if (data!=ATA_COMMAND_REQUEST_SENSE)
			{
				device->DescriptiveErrorCode = ATA_REQUEST_SENSE_NO_ERROR;
			}
			
			if ((data &0x0f0) == ATA_COMMAND_RECALIBRATE)
			{
				device->StatusRegister &= ~ATA_STATUS_BSY;
				device->StatusRegister &= ~ATA_STATUS_DRQ;
				break;
			}

			if ((data &0x0f0) == ATA_COMMAND_SEEK)
			{
				if (device->HeadRegister & ATA_HEAD_LBA)
				{
					int SectorIndex =
						((device->HeadRegister &0x0f) << 24) |
						((device->CylinderHigh &0x0ff) << 16) |
						((device->CylinderLow &0x0ff) << 8) |
						(device->SectorNumberRegister &0x0ff);
					
					if (SectorIndex>=device->DeviceMaxSectors)
					{
						device->ErrorRegister = ATA_ERROR_IDNF|ATA_ERROR_ABRT;
						device->StatusRegister |= ATA_STATUS_ERR;
						device->DescriptiveErrorCode = ATA_REQUEST_SENSE_INVALID_ADDRESS;
					}
				}
				else
				{
					if (device->SectorNumberRegister == 0)
					{
						device->ErrorRegister = ATA_ERROR_IDNF | ATA_ERROR_ABRT;
						device->StatusRegister |= ATA_STATUS_ERR;
						device->DescriptiveErrorCode = ATA_REQUEST_SENSE_INVALID_ADDRESS;
					}
					else
					{
						ata_chs_to_lba(device);
						if (device->CurrentLBASectorIndex >= device->DeviceMaxSectors)
						{
							device->ErrorRegister = ATA_ERROR_IDNF | ATA_ERROR_ABRT;
							device->StatusRegister |= ATA_STATUS_ERR;
							device->DescriptiveErrorCode = ATA_REQUEST_SENSE_INVALID_ADDRESS;
						}
					}
				}

				device->StatusRegister &= ~ATA_STATUS_BSY;
				device->StatusRegister &= ~ATA_STATUS_DRQ;
				break;
			}

			switch (device->CommandRegister)
			{
		
			

				case ATA_COMMAND_REQUEST_SENSE:
				{
					/* provides additional error response in error code */
					device->ErrorRegister = device->DescriptiveErrorCode;
					device->StatusRegister &= ~ATA_STATUS_BSY;
					device->StatusRegister &= ~ATA_STATUS_DRQ;
					/* reset it now it's been read */
					device->DescriptiveErrorCode = ATA_REQUEST_SENSE_NO_ERROR;
				}
				break;

			case ATA_COMMAND_WRITE_MULTIPLE:
			case ATA_COMMAND_WRITE_SECTORS:
			case ATA_COMMAND_WRITE_SECTORS_WITHOUT_RETRY:
			case ATA_COMMAND_WRITE_LONG:
			case ATA_COMMAND_WRITE_LONG_WITHOUT_RETRY:
			case ATA_COMMAND_WRITE_VERIFY:
			{
				if ((device->HeadRegister & ATA_HEAD_LBA) == 0)
				{
					if (device->SectorNumberRegister == 0)
					{

						break;
					}
				}

				ata_calc_initial_sector_index(device);
				//printf("Write sector %lu\n", device->CurrentLBASectorIndex);

				if (device->CurrentLBASectorIndex >= device->DeviceMaxSectors)
				{
					device->ErrorRegister |= ATA_ERROR_ABRT;
					device->ErrorRegister |= ATA_ERROR_IDNF;
					device->StatusRegister |= ATA_STATUS_ERR;
					device->StatusRegister &= ~ATA_STATUS_BSY;
				}
				else
				{

					unsigned long nSectors = device->SectorCountRegister;
					if (nSectors == 0)
					{
						nSectors = 256;
					}
					device->DataSize = 0;
					device->SectorsRemaining = nSectors;
					device->StatusRegister |= ATA_STATUS_DRQ;
					device->StatusRegister &= ~ATA_STATUS_BSY;
				}
			}
			break;

			case ATA_COMMAND_READ_VERIFY_SECTORS:
			case ATA_COMMAND_READ_VERIFY_SECTORS_WITHOUT_RETRY:
			{
				// todo timings
				device->StatusRegister &= ~ATA_STATUS_BSY;
				device->StatusRegister &= ~ATA_STATUS_DRQ;
			}
			break;

			case ATA_COMMAND_READ_MULTIPLE:
			case ATA_COMMAND_READ_SECTORS:
			case ATA_COMMAND_READ_SECTORS_WITHOUT_RETRY:
			case ATA_COMMAND_READ_LONG:
			case ATA_COMMAND_READ_LONG_WITHOUT_RETRY:
			{
				if ((device->HeadRegister & ATA_HEAD_LBA) == 0)
				{
					if (device->SectorNumberRegister == 0)
					{
						device->ErrorRegister |= ATA_ERROR_ABRT;
						device->ErrorRegister |= ATA_ERROR_IDNF;
						device->StatusRegister |= ATA_STATUS_ERR;
						device->StatusRegister &= ~ATA_STATUS_BSY;
						device->DescriptiveErrorCode = ATA_REQUEST_SENSE_INVALID_ADDRESS;
						break;
					}
				}
				ata_calc_initial_sector_index(device);
				printf("Read sector %lu\n", device->CurrentLBASectorIndex);
				printf("Device Max Sectors %lu\n", device->DeviceMaxSectors);

				if (device->CurrentLBASectorIndex >= device->DeviceMaxSectors)
				{
					device->ErrorRegister |= ATA_ERROR_ABRT;
					device->ErrorRegister |=  ATA_ERROR_IDNF;
					device->StatusRegister |= ATA_STATUS_ERR;
					device->StatusRegister &= ~ATA_STATUS_BSY;
					device->DescriptiveErrorCode = ATA_REQUEST_SENSE_INVALID_ADDRESS;
				}
				else
				{

					unsigned long nSectors = device->SectorCountRegister;
					if (nSectors == 0)
					{
						nSectors = 256;
					}
					device->DataSize = 0;
					device->SectorsRemaining = nSectors;
					device->StatusRegister |= ATA_STATUS_DRQ;
					device->StatusRegister &= ~ATA_STATUS_BSY;

					if (ata_sector_exists(device, device->CurrentLBASectorIndex))
					{
						unsigned char *pSector = ata_get_sector_data(device, device->CurrentLBASectorIndex);
						memcpy(device->Buffer, pSector, 512);
					}
					else
					{
						memset(device->Buffer, ATA_DEFAULT_BYTE, 512);
					}
				}
			}
			break;

			case ATA_COMMAND_IDENTIFY_DEVICE:
			{
				memcpy(device->Buffer, device->DeviceIdentification, 512);
				device->DataSize = 0;
				device->StatusRegister |= ATA_STATUS_DRQ;
				device->StatusRegister &= ~ATA_STATUS_BSY;
				device->SectorsRemaining = 1;
				device->ErrorRegister = 0;
				device->SectorCountRegister = 1; /* should be 0 at end */
				
#if 0
				if (device->DeviceMaxSectors >= MAX_CHS_SECTORS)
				{
					/* set LBA; correct?? */
					device->HeadRegister |= (1 << 6);
					device->HeadRegister |= (1 << 7);
					device->HeadRegister |= (1 << 5);
				}
#endif
			}
			break;

			case ATA_COMMAND_WRITE_BUFFER:
			{
				device->DataSize = 0;
				device->SectorsRemaining = 1;
				device->StatusRegister |= ATA_STATUS_DRQ;
				device->StatusRegister &= ~ATA_STATUS_BSY;
				device->ErrorRegister = 0;
				device->SectorCountRegister = 1; /* should be 0 at end */
			}
			break;

			case ATA_COMMAND_READ_BUFFER:
			{
				device->DataSize = 0;
				device->StatusRegister |= ATA_STATUS_DRQ;
				device->StatusRegister &= ~ATA_STATUS_BSY;
				device->SectorsRemaining = 1;
				device->ErrorRegister = 0;
				device->SectorCountRegister = 1; /* should be 0 at end */

			}
			break;

			case ATA_COMMAND_INIT_DRIVE_PARAMETERS:
			{
				/* NEEDS FIXING */
				int CurrentHeads = (device->HeadRegister & 0x0f) + 1;
				int CurrentSectorsPerTrack = (device->SectorNumberRegister & 0x0ff);
				int CurrentTracks;
				int CurrentTotalSectors;
				if (CurrentSectorsPerTrack>=31)
				{
					CurrentSectorsPerTrack = 30;
				}
				CurrentTracks = device->DeviceMaxSectors / (CurrentHeads*CurrentSectorsPerTrack);
				CurrentTotalSectors = CurrentTracks*CurrentSectorsPerTrack*CurrentHeads;
				//printf("Current Heads: %02x\n",CurrentHeads);
			//	printf("Current SPT: %02x\n",CurrentSectorsPerTrack);
			//	printf("Current Tracks: %02x\n",CurrentTracks);
			//	printf("Current Total sectors: %04x\n",CurrentTotalSectors);
				
				device->DeviceIdentification[(54 * 2)] = (CurrentTracks&0x0ff);
				device->DeviceIdentification[(54 * 2)+1] = ((CurrentTracks>>8)&0x0ff);

				device->DeviceIdentification[(55 * 2)] = (CurrentHeads&0x0ff);
				device->DeviceIdentification[(55 * 2) + 1] = ((CurrentHeads>>8)&0x0ff);

				device->DeviceIdentification[(56 * 2)] = (CurrentSectorsPerTrack&0x0ff);
				device->DeviceIdentification[(56 * 2) + 1] = ((CurrentSectorsPerTrack>>8)&0x0ff);

				device->DeviceIdentification[(57 * 2)] = CurrentTotalSectors;
				device->DeviceIdentification[(57 * 2) + 1] = (CurrentTotalSectors>>8)&0x0ff;
				device->DeviceIdentification[(58 * 2)] = (CurrentTotalSectors >> 16) & 0x0ff;
				device->DeviceIdentification[(58 * 2) + 1] = (CurrentTotalSectors >> 24) & 0x0ff;

				device->StatusRegister &= ~ATA_STATUS_BSY;
				device->StatusRegister &= ~ATA_STATUS_DRQ;
				device->ErrorRegister = 0;
			}
			break;

#if 0
			case ATA_COMMAND_READ_NATIVE_MAX_ADDRESS:
			{
				device->StatusRegister &= ~ATA_ERROR_BSY;
				device->StatusRegister &= ~ATA_ERROR_DRQ;
				device->ErrorRegister = 0;

				/* return the maximum sector address as LBA in the registers */
				device->SectorNumberRegister = device->DeviceMaxSectors &0x0ff;
				device->CylinderLow = (device->DeviceMaxSectors>>8) &0x0ff;
				device->CylinderHigh = (device->DeviceMaxSectors>>16) &0x0ff;
				device->HeadRegister &= ~0x0f;
				device->HeadRegister |= (1 << 6);	// LBA
				device->HeadRegister |= (device->DeviceMaxSectors >> 24) &0x0f;
			}
			break;
#endif

			case ATA_COMMAND_DEVICE_RESET:
			case ATA_COMMAND_NOP:
				device->StatusRegister |= ATA_STATUS_ERR;
				device->StatusRegister &= ~ATA_STATUS_BSY;
				device->StatusRegister &= ~ATA_STATUS_DRQ;
				device->ErrorRegister |= ATA_ERROR_ABRT;
				device->DescriptiveErrorCode = ATA_REQUEST_SENSE_INVALID_COMMAND;
				break;
			
			case ATA_COMMAND_IDLE:
			case ATA_COMMAND_IDLE_IMMEDIATE:
			case ATA_COMMAND_IDLE_IMMEDIATE_E1:
			case ATA_COMMAND_IDLE_E3:
			{
				device->PowerMode = ATA_POWER_MODE_IDLE;
				device->StatusRegister &= ~ATA_STATUS_BSY;
				device->StatusRegister &= ~ATA_STATUS_DRQ;
			}
			break;


			case ATA_COMMAND_STANDBY:
			case ATA_COMMAND_STANDBY_IMMEDIATE:
			case ATA_COMMAND_STANDBY_IMMEDIATE_E0:
			case ATA_COMMAND_STANDBY_E2:
			{
				device->PowerMode = ATA_POWER_MODE_STANDBY;
				device->StatusRegister &= ~ATA_STATUS_BSY;
				device->StatusRegister &= ~ATA_STATUS_DRQ;
			}
			break;


			case ATA_COMMAND_SLEEP:
			case ATA_COMMAND_SLEEP_E6:
			{		
				// needs reset to get out of sleep
				device->PowerMode = ATA_COMMAND_SLEEP;
				device->StatusRegister &= ~ATA_STATUS_BSY;
				device->StatusRegister &= ~ATA_STATUS_DRQ;
			}
			break;

			case ATA_COMMAND_CHECK_POWER_MODE:
			case ATA_COMMAND_CHECK_POWER_MODE_E5:
			{
				if (device->PowerMode == ATA_POWER_MODE_IDLE)
				{
					device->SectorCountRegister = 0x0ff;
				}
				else if (device->PowerMode == ATA_POWER_MODE_STANDBY)
				{
					device->SectorCountRegister = 0x00;
				}
				device->StatusRegister &= ~ATA_STATUS_BSY;
				device->StatusRegister &= ~ATA_STATUS_DRQ;
			}
			break;
				
			case ATA_COMMAND_SET_FEATURES:
			{
				/* TODO: If device doesn't support 8-bit transfer we can error */
				if (device->FeaturesRegister == 0x081)
				{
					device->b8BitTransfer = FALSE;
				}
				if (device->FeaturesRegister == 0x01)
				{
					device->b8BitTransfer = TRUE;
				}
				device->StatusRegister &= ~ATA_STATUS_BSY;
				device->StatusRegister &= ~ATA_STATUS_DRQ;
			}
			break;

			default:
				device->StatusRegister |= ATA_STATUS_ERR;
				device->StatusRegister &= ~ATA_STATUS_BSY;
				device->StatusRegister &= ~ATA_STATUS_DRQ;
				device->ErrorRegister |= ATA_ERROR_ABRT;
				device->DescriptiveErrorCode = ATA_REQUEST_SENSE_INVALID_COMMAND;
				break;
			}
		}
		break;

		case ATA_CYLINDER_NUMBER_HIGH_CS | ATA_CYLINDER_NUMBER_HIGH_DA:
		{
			/* drq and bsy should be 0 */
			/* cylinder high register */
			device->CylinderHigh = data;
		}
		break;

		case ATA_CYLINDER_NUMBER_LOW_CS | ATA_CYLINDER_NUMBER_LOW_DA:
		{
			/* drq and bsy should be 0 */
			/* cylinder high register */
			device->CylinderLow = data;
		}
		break;


		case ATA_DATA_REGISTER_CS | ATA_DATA_REGISTER_DA:
		{
			if (!ata_isresponding(device))
			{
				break;
			}

			if ((device->StatusRegister & ATA_STATUS_DRQ) == 0)
				break;

			if (device->b8BitTransfer)
			{
				device->Buffer[device->DataSize] = (data &0x0ff);
				device->DataSize ++;
			}
			else
			{
				device->Buffer[device->DataSize] = (data &0x0ff);
				device->Buffer[device->DataSize + 1] = ((data >> 8) &0x0ff);
				device->DataSize += 2;
			}
			/* has data in sector remaining? */
			if (device->DataSize == ATA_SECTOR_SIZE)
			{
				/* write to sector */
				unsigned char *pData = ata_get_sector_data(device, device->CurrentLBASectorIndex);
				memcpy(pData, device->Buffer, 512);

				device->SectorCountRegister--;
				device->SectorsRemaining--;
				if (device->SectorsRemaining == 0)
				{
					/* no more data */
					device->StatusRegister &= ~ATA_STATUS_DRQ;
					/* clear busy */
					device->StatusRegister &= ~ATA_STATUS_BSY;
				}
				else
				{
					device->DataSize = 0;
					device->CurrentLBASectorIndex++;
					if (device->HeadRegister & ATA_HEAD_LBA)
					{
						/* update registers with LBA of sector written */
						device->HeadRegister = ((device->CurrentLBASectorIndex >> 24) &0x0f) | (device->HeadRegister &0x0f0);
						device->CylinderHigh = (device->CurrentLBASectorIndex >> 16) &0xff;
						device->CylinderLow = (device->CurrentLBASectorIndex >> 8) &0xff;
						device->SectorNumberRegister = device->CurrentLBASectorIndex &0xff;
					}
					else
					{
						ata_update_chs(device);
						ata_chs_to_lba(device);
					}

					/* if sector is not valid indicate the error */
					if (device->CurrentLBASectorIndex >= device->DeviceMaxSectors)
					{
						device->SectorCountRegister = 0;
						device->ErrorRegister |= ATA_ERROR_ABRT;
						device->ErrorRegister |= ATA_ERROR_IDNF;
						device->StatusRegister |= ATA_STATUS_ERR;
						device->StatusRegister &= ~ATA_STATUS_BSY;
						device->DescriptiveErrorCode = ATA_REQUEST_SENSE_INVALID_ADDRESS;
					}

				}
			}
		}
		break;

		case ATA_HEAD_CS | ATA_HEAD_DA:
		{
			/* Both drives respond */

			/* bit 6 = LBA */
			/* bit 4 = device */
			device->HeadRegister = data;
		}
		break;



		case ATA_FEATURES_CS | ATA_FEATURES_DA:
		{
			device->FeaturesRegister = data;
		}
		break;


		case ATA_SECTOR_COUNT_CS | ATA_SECTOR_COUNT_DA:
		{
			device->SectorCountRegister = data;
		}
		break;


		case ATA_SECTOR_NUMBER_CS | ATA_SECTOR_NUMBER_DA:
		{
			device->SectorNumberRegister = data;
		}
		break;
		
		default:
		{
			break;

		}

	}
}

