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
#ifndef __FDC_HEADER_INCLUDED__
#define __FDC_HEADER_INCLUDED__

#define FDC_MAX_DRIVES 4


#include "cpcglob.h"
#include "device.h"


/* FDC defines */

#define FDC_COMMAND_MULTI_TRACK						0x080
#define FDC_COMMAND_MFM								0x040
#define FDC_COMMAND_SKIP							0x020
#define FDC_COMMAND_WORD							0x01f
#define FDC_COMMAND_HEAD_ADDRESS					0x004
#define FDC_COMMAND_UNIT_STANDARD					0x003
#define FDC_COMMAND_UNIT_CPC						0x001

/* Main Status Register defines */
#define FDC_MSR_DATA_REQUEST						0x080
#define FDC_MSR_DATA_FLOW_DIRECTION					0x040
#define FDC_MSR_EXECUTION_PHASE						0x020
#define FDC_MSR_BUSY								0x010
#define FDC_MSR_FDD_3_BUSY							0x008
#define FDC_MSR_FDD_2_BUSY							0x004
#define FDC_MSR_FDD_1_BUSY							0x002
#define FDC_MSR_FDD_0_BUSY							0x001


/* Status Register 0 (ST0) defines */
#define FDC_ST0_INTERRUPT_CODE1						0x080
#define FDC_ST0_INTERRUPT_CODE0						0x040
#define FDC_ST0_SEEK_END							0x020
#define FDC_ST0_EQUIPMENT_CHECK						0x010
#define FDC_ST0_NOT_READY							0x008
#define FDC_ST0_HEAD_ADDRESS						0x004
#define FDC_ST0_UNIT_SELECT1						0x002
#define FDC_ST0_UNIT_SELECT0						0x001

/* Status Register 1 (ST1) defines */
#define FDC_ST1_END_OF_CYLINDER						0x080
#define FDC_ST1_UNUSED_BIT6							0x040
#define FDC_ST1_DATA_ERROR							0x020
#define FDC_ST1_OVERRUN								0x010
#define FDC_ST1_UNUSED_BIT3							0x008
#define FDC_ST1_NO_DATA								0x004
#define FDC_ST1_NOT_WRITEABLE						0x002
#define FDC_ST1_MISSING_ADDRESS_MARK				0x001

/* Status Register 2 (ST2) defines */
#define FDC_ST2_UNUSED_BIT7							0x080
#define FDC_ST2_CONTROL_MARK						0x040
#define FDC_ST2_DATA_ERROR_IN_DATA_FIELD			0x020
#define FDC_ST2_WRONG_CYLINDER						0x010
#define FDC_ST2_SCAN_EQUAL_HIT						0x008
#define FDC_ST2_SCAN_NOT_SATISFIED					0x004
#define FDC_ST2_BAD_CYLINDER						0x002
#define FDC_ST2_MISSING_ADDRESS_MARK_IN_DATA_FIELD	0x001

/* Status Register 3 (ST3) defines */
#define FDC_ST3_FAULT								0x080
#define FDC_ST3_WRITE_PROTECTED						0x040
#define FDC_ST3_READY								0x020
#define FDC_ST3_TRACK_0								0x010
#define FDC_ST3_TWO_SIDE							0x008
#define FDC_ST3_HEAD_ADDRESS						0x004
#define FDC_ST3_UNIT_SELECT1						0x002
#define FDC_ST3_UNIT_SELECT0						0x001


/* FDC functions */
void				FDC_Reset(void);
void				FDC_Power(void);

void				FDC_WriteDataRegister(int);
unsigned int		FDC_ReadDataRegister(void);
unsigned int		FDC_ReadMainStatusRegister(void);

#define NEC765_FLAGS_DATA_TRANSFER 0x08
#define NEC765_FLAGS_POLL_ALLOWED 0x010
enum
{
	NEC765_LOW_LEVEL_STATE_DELAY = 0,

	NEC765_HIGH_LEVEL_STATE_EXECUTION_PHASE_READ_DATA,
	NEC765_HIGH_LEVEL_STATE_EXECUTION_PHASE_WRITE_DATA,
	NEC765_HIGH_LEVEL_STATE_EXECUTION_PHASE,
	NEC765_HIGH_LEVEL_STATE_COMMAND_PHASE_FIRST_BYTE,
	NEC765_HIGH_LEVEL_STATE_COMMAND_PHASE_REMAINING_BYTES,
	NEC765_HIGH_LEVEL_STATE_RESULT_PHASE
};

/* drive flags */
#define FDC_DRIVE_READY_STATE 0x01
#define FDC_DRIVE_INTERRUPT_STATE 0x02
#define FDC_DRIVE_RECALIBRATE 0x04
#define FDC_DRIVE_SEEK 0x08

typedef struct
{
	unsigned long Flags;
	unsigned long ST0;
	unsigned long PCN;
	unsigned long NCN;
	unsigned long StepCount;
//	unsigned long StepTimeForDrive;
} NEC765_DRIVE;


typedef struct
{
	unsigned long Flags;

	unsigned char ST0;
	unsigned char ST1;
	unsigned char ST2;
	unsigned char ST3;

	CHRN chrn;

    BOOL ResultPhase;

	/* drive output from FDC */
	unsigned long CurrentDrive;
	/* side output from FDC */
	unsigned long CurrentSide;


/*	unsigned long MainStatusRegister; */

    unsigned char SpecifyBytes[3];

	NEC765_DRIVE Drives[FDC_MAX_DRIVES];
	unsigned long StepTime;
	/* fdc state */
	int LowLevelState;

	int PollTimer;
	int PollTime;
	int StepTimer;
	int HighLevelState;
	int PushedHighLevelState;

	void	(*CommandHandler)(int);

	/* command state */
	unsigned long CommandState;

	/* state to go to when complete */
	unsigned long NextCommandState;

	/* number of cycles before a data request is issued */
	unsigned long CyclesToDataRequest;
	/* nop count when we setup the data request delay */
	unsigned long NopCountOfDataRequestStart;

	unsigned long ExecutionBufferByteIndex;
	unsigned long ExecutionNoOfBytes;
	char *ExecutionBuffer;

    unsigned long ByteCount;
	unsigned long BytesRemaining;

	unsigned long StoredDelay;


	unsigned short CRC;

	unsigned char MainStatusRegister;
	unsigned char DataRegister;

	unsigned char SectorCounter;

    /* number of actual bytes stored */
	int nCommandBytes;
	/* the stored bytes */
	unsigned char CommandBytes[12];

    /* number of actual bytes stored */
	int nResultBytes;
	/* the stored bytes */
	unsigned char ResultBytes[12];

	int InterruptOutput;
	int DriveOutput;
	int SideOutput;
	int stp;
} NEC765;

int FDC_GetCommandByte(int Index);
int FDC_GetCurrentCommandLength(void);

int FDC_GetResultByte(int Index);
int FDC_GetCurrentCommandResultLength(void);

int FDC_GetInterruptOutput(void);
int FDC_GetDriveOutput(void);
int FDC_GetSideOutput(void);
int FDC_GetPCN(int Drive);
int FDC_GetDirectionOutput(void);
void FDC_SetTerminalCount(int State);
int FDC_GetDRQOutput(void);
int FDC_GetMFMOutput(void);
int FDC_GetHDLDOutput(void);
void FDC_RefreshInterrupt(void);
void FDC_SetDataRequest(void);
void FDC_ClearDataRequest(void);
BOOL FDC_GetDmaMode(void);
int FDC_GetHeadLoadTime(void);
int FDC_GetHeadUnLoadTime(void);
int FDC_GetStepRateTime(void);

typedef void (*FDC_COMMAND_FUNCTION)(int);

typedef struct
{
	int			NoOfCommandBytes;
	FDC_COMMAND_FUNCTION	CommandHandler;
} FDC_COMMAND;

/* snapshot functions */
unsigned char FDC_GetMainStatusRegister(void);
unsigned char FDC_GetDataRegister(void);
void	FDC_SetMainStatusRegister(unsigned char);
void	FDC_SetDataRegister(unsigned char);
void FDC_Poll(void);
void FDC_Cycle(void);

typedef enum
{
	FDC_WRITE_FIRST_COMMAND_BYTE = 0,
	FDC_WRITE_COMMAND_BYTES,
	FDC_WRITE_EXECUTION_PHASE,
	FDC_READ_EXECUTION_PHASE,
	FDC_READ_RESULT_PHASE
} FDC_STATE;

#endif

