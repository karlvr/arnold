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
#ifndef __ATA_HEADER_INCLUDED__
#define __ATA_HEADER_INCLUDED__

#include "cpcglob.h"

#define ATA_VER_1 (1<<1)
#define ATA_VER_2 (1<<2)
#define ATA_VER_3 (1<<3)
#define ATA_VER_4 (1<<4)
#define ATA_VER_5 (1<<5)
#define ATA_VER_6 (1<<6)
#define ATA_CF (1<<7)
#define ATA_PACKET (1<<8)

#define ATA_CS1_REG_BIT (1<<4)
#define ATA_CS0_REG_BIT (1<<3)
#define ATA_DA2_REG_BIT (1<<2)
#define ATA_DA1_REG_BIT (1<<1)
#define ATA_DA0_REG_BIT (1<<0)

#define ATA_SRST (1<<2)
#define ATA_INTEN (1<<1)

#define ATA_SECTOR_SIZE 512
#define MAX_CHS_SECTORS 16515072

#define ATA_STATUS_BSY (1<<7) /* busy, other bits are not expected to be valid; but in reality do appear to be */
#define ATA_STATUS_DRDY (1<<6)
/* Bit 5 */
#define ATA_STATUS_DSC (1<<4)	/* seek complete */
#define ATA_STATUS_DRQ (1<<3) /* data request */
/* Bit 2 */
/* Bit 1 */
#define ATA_STATUS_ERR (1<<0)/* error */

/* Bit 7 */
/* Bit 6 */
/* Bit 5 */
#define ATA_ERROR_IDNF (1<<4)
/* Bit 3 */
#define ATA_ERROR_ABRT (1<<2)
/* Bit 1 */
/* Bit 0 */

/* bit 6 */
#define ATA_HEAD_LBA (1<<6)



typedef enum
{
	ATA_REQUEST_SENSE_NO_ERROR = 0x0,
	ATA_REQUEST_SENSE_SELF_TEST_OK = 0x01,
	ATA_REQUEST_SENSE_MISC_ERR = 0x09,
	ATA_REQUEST_SENSE_IDNF_10 = 0x010,
	ATA_REQUEST_SENSE_IDNF_14 = 0x014,
	ATA_REQUEST_SENSE_INVALID_COMMAND = 0x020,
	ATA_REQUEST_SENSE_INVALID_ADDRESS = 0x021,
	ATA_REQUEST_SENSE_ADDRESS_OVERFLOW = 0x02f,
} ATA_REQUEST_SENSE_CODE;

typedef enum
{
	ATA_POWER_MODE_SLEEP,
	ATA_POWER_MODE_STANDBY,
	ATA_POWER_MODE_IDLE,
	ATA_POWER_MODE_ACTIVE
} ATA_POWER_MODE;

typedef enum
{
	ATA_JUMPER_MASTER_SINGLE,
	ATA_JUMPER_MASTER_WITH_SLAVE_PRESENT,
	ATA_JUMPER_SLAVE
} ATA_JUMPER_SETTING;

typedef struct
{
	BOOL m_bEnabled; /* if TRUE device is present */
	BOOL m_bAnswersForOtherDevice; /* if TRUE, will answer for other device. e.g. master answers for slave, or slave answers for master */
	BOOL m_bMaster; /* if TRUE device responds to device 0 */
	BOOL m_bSlaveDetected; /* this corresponds to the signal asserted by the slave when it's initialised */

	ATA_JUMPER_SETTING JumperSetting;
	ATA_POWER_MODE PowerMode;
	unsigned char CommandRegister;
	unsigned char CylinderHigh;
	unsigned char CylinderLow;
	unsigned char HeadRegister;
	unsigned char StatusRegister;
	unsigned char DeviceControl;
	unsigned char FeaturesRegister;
	unsigned char ErrorRegister;
	unsigned char SectorCountRegister;
	unsigned char SectorNumberRegister;
	ATA_REQUEST_SENSE_CODE DescriptiveErrorCode;
	/* 4GB = 4096*1024/512 */
	unsigned long DeviceMaxSectors;
	unsigned int MaxCylinders;
	unsigned char Heads;
	unsigned char SectorsPerTrack;

	unsigned char *SectorExistsMap;
	unsigned char **SectorPointers;
	unsigned long DataSize;
	unsigned long SectorsRemaining;
	unsigned long CurrentLBASectorIndex;
	unsigned char DeviceIdentification[512];
	unsigned char Buffer[512];
	BOOL b8BitTransfer; /* TRUE if device is using 8-bit transfer */
} ata_device;

/* TODO: Device 0 answering for device 1 */

BOOL ata_isresponding(ata_device *device);

void ata_reset(ata_device *device);
void ata_init(ata_device *device, ATA_JUMPER_SETTING JumperSetting, BOOL bEnable, int nCapacity);
void ata_finish(ata_device *device);

#define ATA_ALTERNATE_STATUS_CS ATA_CS1_REG_BIT
#define ATA_ALTERNATE_STATUS_DA ATA_DA2_REG_BIT | ATA_DA1_REG_BIT

#define ATA_DRIVE_ADDRESS_CS ATA_CS1_REG_BIT
#define ATA_DRIVE_ADDRESS_DA ATA_DA2_REG_BIT | ATA_DA1_REG_BIT | ATA_DA0_REG_BIT

#define ATA_DATA_REGISTER_CS ATA_CS0_REG_BIT
#define ATA_DATA_REGISTER_DA 0

// error/features use same select
#define ATA_ERROR_CS ATA_CS0_REG_BIT
#define ATA_ERROR_DA ATA_DA0_REG_BIT

#define ATA_FEATURES_CS ATA_CS0_REG_BIT
#define ATA_FEATURES_DA ATA_DA0_REG_BIT

#define ATA_SECTOR_COUNT_CS ATA_CS0_REG_BIT
#define ATA_SECTOR_COUNT_DA ATA_DA1_REG_BIT

#define ATA_SECTOR_NUMBER_CS ATA_CS0_REG_BIT
#define ATA_SECTOR_NUMBER_DA ATA_DA1_REG_BIT | ATA_DA0_REG_BIT

#define ATA_CYLINDER_NUMBER_LOW_CS ATA_CS0_REG_BIT
#define ATA_CYLINDER_NUMBER_LOW_DA ATA_DA2_REG_BIT

#define ATA_CYLINDER_NUMBER_HIGH_CS ATA_CS0_REG_BIT
#define ATA_CYLINDER_NUMBER_HIGH_DA ATA_DA2_REG_BIT | ATA_DA0_REG_BIT

#define ATA_HEAD_CS ATA_CS0_REG_BIT
#define ATA_HEAD_DA ATA_DA2_REG_BIT | ATA_DA1_REG_BIT

// command/status use same select
#define ATA_COMMAND_CS ATA_CS0_REG_BIT
#define ATA_COMMAND_DA ATA_DA2_REG_BIT | ATA_DA1_REG_BIT | ATA_DA0_REG_BIT

#define ATA_STATUS_CS ATA_CS0_REG_BIT
#define ATA_STATUS_DA ATA_DA2_REG_BIT | ATA_DA1_REG_BIT | ATA_DA0_REG_BIT

#define ATA_DEFAULT_BYTE 0x0

int ata_read(ata_device *, int reg);
void ata_write(ata_device *, int reg, int data);
//#define		COMMAND_DEVICE_RESET 0x08

#define ATA_COMMAND_NOP 0x00
#define ATA_COMMAND_REQUEST_SENSE 0x03
#define ATA_COMMAND_DEVICE_RESET 0x08
#define	ATA_COMMAND_RECALIBRATE 0x10
#define ATA_COMMAND_READ_SECTORS 0x020
#define ATA_COMMAND_READ_SECTORS_WITHOUT_RETRY 0x021
#define ATA_COMMAND_READ_LONG 0x022
#define ATA_COMMAND_READ_LONG_WITHOUT_RETRY 0x023
#define ATA_COMMAND_WRITE_SECTORS 0x030
#define ATA_COMMAND_WRITE_SECTORS_WITHOUT_RETRY 0x031
#define ATA_COMMAND_WRITE_LONG 0x032
#define ATA_COMMAND_WRITE_LONG_WITHOUT_RETRY 0x033
#define ATA_COMMAND_WRITE_VERIFY 0x03C
#define ATA_COMMAND_READ_VERIFY_SECTORS 0x040
#define ATA_COMMAND_READ_VERIFY_SECTORS_WITHOUT_RETRY 0x041
#define ATA_COMMAND_FORMAT_TRACK 0x050
#define ATA_COMMAND_SEEK 0x070
#define ATA_COMMAND_EXECUTE_DEVICE_DIAGNOSTICS 0x090
#define ATA_COMMAND_INIT_DRIVE_PARAMETERS 0x091
#define ATA_COMMAND_STANDBY_IMMEDIATE 0x094
#define ATA_COMMAND_IDLE_IMMEDIATE 0x095
#define ATA_COMMAND_STANDBY 0x096
#define ATA_COMMAND_IDLE 0x097
#define ATA_COMMAND_CHECK_POWER_MODE 0x098
#define ATA_COMMAND_SLEEP 0x099
#define ATA_COMMAND_READ_MULTIPLE 0x0c4
#define ATA_COMMAND_WRITE_MULTIPLE 0x0c5
#define ATA_COMMAND_SET_MULTIPLE_MODE 0x0c6
#define ATA_COMMAND_READ_DMA_WITH_RETRY 0x0c8
#define ATA_COMMAND_READ_DMA_WITHOUT_RETRY 0x0c9
#define ATA_COMMAND_WRITE_DMA_WITH_RETRY 0x0ca
#define ATA_COMMAND_WRITE_DMA_WITHOUT_RETRY 0x0cb
#define ATA_COMMAND_ACKNOWLEDGE_MEDIA_CHANGE 0x0dc
#define ATA_COMMAND_STANDBY_IMMEDIATE_E0 0x0e0
#define ATA_COMMAND_IDLE_IMMEDIATE_E1 0x0e1
#define ATA_COMMAND_STANDBY_E2 0x0e2
#define ATA_COMMAND_IDLE_E3 0x0e3
#define ATA_COMMAND_READ_BUFFER 0x0e4
#define ATA_COMMAND_CHECK_POWER_MODE_E5 0x0e5
#define ATA_COMMAND_SLEEP_E6 0x0e6
#define ATA_COMMAND_WRITE_BUFFER 0x0e8
#define ATA_COMMAND_WRITE_SAME 0x0e9
#define ATA_COMMAND_IDENTIFY_DEVICE 0x0ec
#define ATA_COMMAND_READ_NATIVE_MAX_ADDRESS 0x0f8
#define ATA_COMMAND_SET_FEATURES 0x0ef
#define ATA_COMMAND_EXECUTE_DEVICE_DIAGNOSTIC 0x090
#define ATA_COMMAND_INIT_DRIVE_PARAMETERS 0x091


#endif
