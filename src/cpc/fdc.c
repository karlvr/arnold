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
#include "host.h"

/* floppy disc controller */
#include "fdc.h"

/* floppy disc drive */
#include "fdd.h"
/* floppy disc interface */
#include "fdi.h"

#include "cpc.h"
void	FDC_UpdateStateStatus(void);

#define DATA_RATE_US 32


int Gap1;        /* first gap on track */
int Gap3;        /* gap between sector */
int Gap4;       /* last gap on track */

static void FDC_Standby(void);
static void FDC_SetStatus0(void);
static void FDC_ClearStatusRegisters(void);
static int	FDC_GetSectorSize(int);


/* commands */
static void	FDC_Invalid(int);
static void	FDC_ReadATrack(int);
static void	FDC_Specify(int);
static void	FDC_SenseDriveStatus(int);
static void	FDC_SenseInterruptStatus(int);
static void	FDC_WriteData(int);
static void	FDC_ReadData(int);
static void	FDC_Recalibrate(int);
static void	FDC_ReadID(int);
static void	FDC_FormatATrack(int);
static void	FDC_Seek(int);
static void FDC_Scan(int);



/* floppy disc controller */
static NEC765 fdc;

/* normal termination, command completed successfully */
#define FDC_INTERRUPT_CODE_NT						(0x000)
/* abnormal termination, command was executed, but didn't complete
successfully */
#define FDC_INTERRUPT_CODE_AT						(FDC_ST0_INTERRUPT_CODE0)
/* invalid command issued */
#define FDC_INTERRUPT_CODE_IC						(FDC_ST0_INTERRUPT_CODE1)
/* abnormal termination, ready state of drive changed */
#define FDC_INTERRUPT_CODE_AT_READY_CHANGED			(FDC_ST0_INTERRUPT_CODE0 | FDC_ST0_INTERRUPT_CODE1)


#define FDC_ST0_SET_DRIVE_AND_SIDE		\
	fdc.ST0 &= ~(FDC_ST0_HEAD_ADDRESS | FDC_ST0_UNIT_SELECT0 | FDC_ST0_UNIT_SELECT1);	\
	fdc.ST0 |= (fdc.CurrentSide<<2) | fdc.CurrentDrive

#define FDC_ST3_SET_DRIVE_AND_SIDE		\
	FDC_ST3 &= ~(FDC_ST0_HEAD_ADDRESS | FDC_ST0_UNIT_SELECT0 | FDC_ST0_UNIT_SELECT1);	\
	FDC_ST3 |= (fdc.CurrentSide<<2) | fdc.CurrentDrive

#define FDC_ST0_SET_DRIVE				\
	fdc.ST0 &= ~(FDC_ST0_HEAD_ADDRESS | FDC_ST0_UNIT_SELECT0 | FDC_ST0_UNIT_SELECT1);	\
	fdc.ST0 |= fdc.CurrentDrive

#define FDC_STEP_RATE_TO_MICROSECONDS(x)	(x*2000)

// fdc doesn't actually have a data buffer like this. it's done byte by byte
static unsigned char FDC_DataBuffer[32768];
static unsigned char FDC_ScanDataBuffer[32768];

static FDC_COMMAND FDC_CommandTable[]=
{
	{1-1,FDC_Invalid},				/* %00000	** invalid ** */
	{1-1,FDC_Invalid},				/* %00001	** invalid ** */
	{9-1,FDC_ReadATrack},				/* %00010 READ A TRACK */
	{3-1,FDC_Specify},				/* %00011 SPECIFY */
	{2-1,FDC_SenseDriveStatus},		/* %00100 SENSE DRIVE STATUS */
	{9-1,FDC_WriteData},				/* %00101 WRITE DATA */
	{9-1,FDC_ReadData},				/* %00110 READ DATA */
	{2-1,FDC_Recalibrate},			/* %00111 RECALIBRATE */
	{1-1,FDC_SenseInterruptStatus},	/* %01000 SENSE INTERRUPT STATUS */
	{9-1,FDC_WriteData /*FDC_WriteDeletedData*/},		/* %01001 WRITE DELETED DATA */
	{2-1,FDC_ReadID},					/* %01010 READ ID */
	{1-1,FDC_Invalid},				/* %01011 ** invalid ** */
	{9-1,FDC_ReadData},		/* %01100 READ DELETED DATA */
	{6-1,FDC_FormatATrack},			/* %01101 FORMAT A TRACK */
	{1-1,FDC_Invalid},				/* %01110 ** invalid ** */
	{3-1,FDC_Seek},					/* %01111 SEEK */
	{1-1,FDC_Invalid},				/* %10000 ** invalid ** */
	{9-1,FDC_Scan},				/* %10001 SCAN EQUAL */
	{1-1,FDC_Invalid},				/* %10010 ** invalid ** */
	{1-1,FDC_Invalid},				/* %10011 ** invalid ** */
	{1-1,FDC_Invalid},				/* %10100 ** invalid ** */
	{1-1,FDC_Invalid},				/* %10101 ** invalid ** */
	{1-1,FDC_Invalid},				/* %10110 ** invalid ** */
	{1-1,FDC_Invalid},				/* %10111 ** invalid ** */
	{1-1,FDC_Invalid},				/* %11000 ** invalid ** */
	{9-1,FDC_Scan},				/* %11001 SCAN LOW OR EQUAL */
	{1-1,FDC_Invalid},				/* %11010 ** invalid ** */
	{1-1,FDC_Invalid},				/* %11011 ** invalid ** */
	{1-1,FDC_Invalid},				/* %11100 ** invalid ** */
	{9-1,FDC_Scan},		/* %11101 SCAN HIGH OR EQUAL */
	{1-1,FDC_Invalid},				/* %11110 ** invalid ** */
	{1-1,FDC_Invalid},				/* %11111 ** invalid ** */
};

int FDC_GetCurrentCommandLength(void)
{
    return fdc.nCommandBytes;
}

int FDC_GetCommandByte(int Index)
{
    return fdc.CommandBytes[Index];
}

int FDC_GetCurrentCommandResultLength(void)
{
    return fdc.nResultBytes;
}

int FDC_GetResultByte(int Index)
{
    return fdc.ResultBytes[Index];
}


/*-----------------------------------------------------------------------*/
/* set the side output from the fdc. */
static void FDC_SetSideOutput(unsigned long Value)
{
	fdc.CurrentSide = Value;
	/* adjust physical side selection */
	FDI_SetPhysicalSide(Value);
}

/*-----------------------------------------------------------------------*/
/* set the drive output from the fdc */
static void FDC_SetDriveOutput(unsigned long Value)
{
	fdc.CurrentDrive = Value;

	FDI_SetPhysicalDrive(Value);
}

/*-----------------------------------------------------------------------*/
/* get drive and side from command */
static void FDC_GetDriveAndSide(void)
{
	unsigned char DriveAndSide = fdc.CommandBytes[1];

	/* set drive output */
	FDC_SetDriveOutput(DriveAndSide & 0x03);

	/* set side output */
	FDC_SetSideOutput(((DriveAndSide & FDC_COMMAND_HEAD_ADDRESS)>>2));
}

/*-----------------------------------------------------------------------*/
/* get drive from command (side is forced to 0) */
static void FDC_GetDrive(void)
{
	unsigned char DriveAndSide = fdc.CommandBytes[1];

	FDC_SetDriveOutput(DriveAndSide & 0x03);

	FDC_SetSideOutput(0);
}

/*-----------------------------------------------------------------------*/


unsigned char FDC_GetMainStatusRegister(void)
{
	return fdc.MainStatusRegister;
}

void	FDC_SetMainStatusRegister(unsigned char Status)
{
	fdc.MainStatusRegister = Status;
}

unsigned char FDC_GetDataRegister(void)
{
	return 0x0ff;
}

void	FDC_SetDataRegister(unsigned char Data)
{
}


/*-----------------------------------------------------------------------*/
void	FDC_NormalSeekComplete(int DriveIndex)
{
	/* normal termination */
	fdc.Drives[DriveIndex].ST0 = FDC_ST0_SEEK_END | FDC_INTERRUPT_CODE_NT | DriveIndex;

	/* trigger interrupt from this drive */
	fdc.Drives[DriveIndex].Flags |= FDC_DRIVE_INTERRUPT_STATE;

    FDC_RefreshInterrupt();
}

void FDC_ClearAllInterrupts(void)
{
	int i;

	for (i = 0; i < FDC_MAX_DRIVES; i++)
	{
		fdc.Drives[i].Flags &= ~FDC_DRIVE_INTERRUPT_STATE;
	}
}

void FDC_CancelSeek(void)
{
	int i;

	for (i = 0; i < FDC_MAX_DRIVES; i++)
	{
		fdc.Drives[i].Flags &= ~(FDC_DRIVE_RECALIBRATE | FDC_DRIVE_SEEK);
	}
}

void FDC_Poll(void)
{
	int i;
	
	if (!fdc.Flags & NEC765_FLAGS_POLL_ALLOWED)
		return;

	for (i = 0; i < FDC_MAX_DRIVES; i++)
	{
		FDC_SetDriveOutput(i);

		if ((fdc.Drives[i].Flags & FDC_DRIVE_INTERRUPT_STATE) == 0)
		{
			if (FDI_GetCurrentDriveFlags() & FDD_FLAGS_DRIVE_READY)
			{
				/* is now ready */

				/* previously was not ready */
				if ((fdc.Drives[i].Flags & FDC_DRIVE_READY_STATE) == 0)
				{
					/* this drive is interrupting */
					fdc.Drives[i].Flags |= FDC_DRIVE_INTERRUPT_STATE;

					/* store new ready state */
					fdc.Drives[i].Flags |= FDC_DRIVE_READY_STATE;

					/* seek or recalibrate? */
					if ((fdc.Drives[i].Flags & (FDC_DRIVE_RECALIBRATE | FDC_DRIVE_SEEK)) != 0)
					{
						fdc.Drives[i].ST0 = FDC_ST0_INTERRUPT_CODE0 | FDC_ST0_SEEK_END | i;

						/* recalibrate? */
						if ((fdc.Drives[i].Flags & FDC_DRIVE_RECALIBRATE) != 0)
						{
							/* equipment check */
							fdc.Drives[i].ST0 |= FDC_ST0_EQUIPMENT_CHECK;
						}

						/* stop recalibrate or seek */
						fdc.Drives[i].Flags &= ~(FDC_DRIVE_RECALIBRATE | FDC_DRIVE_SEEK);
					}
					else
					{
						/* not recalibrate or seek */
						fdc.Drives[i].ST0 = FDC_ST0_INTERRUPT_CODE1 | FDC_ST0_INTERRUPT_CODE0 | i;
					}


				}
			}
			else
			{
				/* is now not ready */

				/* previously was not ready */
				if ((fdc.Drives[i].Flags & FDC_DRIVE_READY_STATE) != 0)
				{
					/* this drive is interrupting */
					fdc.Drives[i].Flags |= FDC_DRIVE_INTERRUPT_STATE;

					/* store new ready state */
					fdc.Drives[i].Flags &= ~FDC_DRIVE_READY_STATE;

					/* seek or recalibrate? */
					if ((fdc.Drives[i].Flags & (FDC_DRIVE_RECALIBRATE | FDC_DRIVE_SEEK)) != 0)
					{
						fdc.Drives[i].ST0 = FDC_ST0_INTERRUPT_CODE0 | FDC_ST0_NOT_READY | FDC_ST0_SEEK_END | i;

						/* recalibrate? */
						if ((fdc.Drives[i].Flags & FDC_DRIVE_RECALIBRATE) != 0)
						{
							/* equipment check */
							fdc.Drives[i].ST0 |= FDC_ST0_EQUIPMENT_CHECK;
						}

						/* stop recalibrate or seek */
						fdc.Drives[i].Flags &= ~(FDC_DRIVE_RECALIBRATE | FDC_DRIVE_SEEK);
					}
					else
					{
						/* not recalibrate or seek */
						fdc.Drives[i].ST0 = FDC_ST0_INTERRUPT_CODE1 | FDC_ST0_INTERRUPT_CODE0 | FDC_ST0_NOT_READY | i;
					}
				}
			}
		}
	}
    FDC_RefreshInterrupt();
}

void FDC_RefreshInterrupt(void)
{
    int i;

    fdc.InterruptOutput = 0;

	/* TODO: not sense interrupt status or scan or something like that */
    if (fdc.ResultPhase)
    {
        fdc.InterruptOutput=1;
    }
    /* execution phase and data request, non-dma */
    if (((fdc.SpecifyBytes[2]&0x01)!=0) &&
        (fdc.MainStatusRegister & FDC_MSR_EXECUTION_PHASE) &&
        (fdc.MainStatusRegister & FDC_MSR_DATA_REQUEST)
        )
    {
        fdc.InterruptOutput = 1;
    }

    for (i=0; i<FDC_MAX_DRIVES; i++)
	{
		/* this drive interrupting? */
		if (fdc.Drives[i].Flags & FDC_DRIVE_INTERRUPT_STATE)
		{
		    fdc.InterruptOutput = 1;
		    break;
		}
	}
}

int FDC_GetHDLDOutput(void)
{
    return 0;
}

int FDC_GetMFMOutput(void)
{
    /* high for mfm low for fm */
    return 0;
}

int FDC_GetDirectionOutput(void)
{
/*    0 = outward
 1 = inward */
    return 0;
}

int FDC_GetDRQOutput(void)
{
    /* high for a dma transfer */
    return 0;
}

int FDC_GetInterruptOutput(void)
{
    /* update current state */
	FDC_UpdateStateStatus();

    return fdc.InterruptOutput;
}

int FDC_GetDriveOutput(void)
{
    return fdc.CurrentDrive;
}

int FDC_GetSideOutput(void)
{
    /* high for side 1
     0 for side 0 */
    return fdc.CurrentSide;
}

int FDC_GetPCN(int Drive)
{
    return fdc.Drives[Drive].PCN;
}


/*-----------------------------------------------------------------------*/
/* update seek */
void	FDC_CheckSeek(int DriveIndex)
{
	FDC_SetDriveOutput(DriveIndex);

	/* recalibrate? */
	if (fdc.Drives[DriveIndex].Flags & FDC_DRIVE_RECALIBRATE)
	{
		/* at track 0? */
		if (FDI_GetCurrentDriveFlags() & FDD_FLAGS_HEAD_AT_TRACK_0)
		{
			/* clear recalibrate */
			fdc.Drives[DriveIndex].Flags &=~FDC_DRIVE_RECALIBRATE;

			/* seek complete */
			FDC_NormalSeekComplete(DriveIndex);
			return;
		}

		/* not complete after 77 step pulses have been issued? */
		if (fdc.Drives[DriveIndex].StepCount>=77)
		{

			/* clear recalibrate */
			fdc.Drives[DriveIndex].Flags &=~FDC_DRIVE_RECALIBRATE;

			/* abnormal termination */
			fdc.Drives[DriveIndex].ST0 = FDC_ST0_SEEK_END | FDC_ST0_EQUIPMENT_CHECK | FDC_ST0_INTERRUPT_CODE0 | DriveIndex;

			/* trigger interrupt from this drive */
			fdc.Drives[DriveIndex].Flags |= FDC_DRIVE_INTERRUPT_STATE;
			FDC_RefreshInterrupt();
		}
	}
	else
	{
		/* seek? */
		unsigned long PCN;

		/* get PCN */
		PCN = fdc.Drives[DriveIndex].PCN;

		/* is PCN == NCN? */
		if (PCN == fdc.Drives[DriveIndex].NCN)
		{
			/* seek complete */
			fdc.Drives[DriveIndex].Flags &=~FDC_DRIVE_SEEK;
			FDC_NormalSeekComplete(DriveIndex);
			return;
		}
	}

}

/*-----------------------------------------------------------------------*/

void	FDC_BeginSeek(void)
{
	int DriveIndex;

	fdc.Flags |= NEC765_FLAGS_POLL_ALLOWED;

	FDC_GetDrive();

	FDC_ClearStatusRegisters();

	/* get drive index */
	DriveIndex = fdc.CommandBytes[1] & 0x03;

	if ((FDI_GetSelectedDriveFlags() & FDD_FLAGS_DRIVE_READY)==0)
	{
		/* not ready */
		if (fdc.CommandBytes[0]==0x0f)
		{


		}
		else
		{
			fdc.Drives[DriveIndex].PCN = 0;
		}

		fdc.Drives[DriveIndex].ST0 =  FDC_ST0_NOT_READY|FDC_ST0_SEEK_END |FDC_ST0_INTERRUPT_CODE0| DriveIndex;

		/* trigger interrupt from this drive */
		fdc.Drives[DriveIndex].Flags |= FDC_DRIVE_INTERRUPT_STATE;
        FDC_RefreshInterrupt();

		FDC_Standby();
		return;
	}



	/* set seek active bit in main status register */
	fdc.MainStatusRegister|=(1<<DriveIndex);

	/* initialise step count for recalibrate */
	fdc.Drives[DriveIndex].StepCount = 0;

	/* seek? */
	if ((fdc.CommandBytes[0])==0x0f)
	{
		/* set seek */
		fdc.Drives[DriveIndex].Flags |= FDC_DRIVE_SEEK;
		fdc.Drives[DriveIndex].Flags &= ~FDC_DRIVE_RECALIBRATE;

		/* new cylinder number */
		fdc.Drives[DriveIndex].NCN = fdc.CommandBytes[2] & 0x0ff;
	}
	else
	{
		/* set recalibrate */
		fdc.Drives[DriveIndex].Flags |= FDC_DRIVE_RECALIBRATE;
		fdc.Drives[DriveIndex].Flags &= ~FDC_DRIVE_SEEK;

		fdc.Drives[DriveIndex].NCN = 0;
		fdc.Drives[DriveIndex].PCN = 0;
	}

	/* check if seek has completed */
	FDC_CheckSeek(DriveIndex);

	/* can accept attempts to write commands */
	FDC_Standby();

}

/*-----------------------------------------------------------------------*/

void FDC_SetTerminalCount(int State)
{
    /* high to indicate termination */
}

void	FDC_DoSeekOperation(int DriveIndex)
{
	FDC_SetDriveOutput(DriveIndex);

	if (fdc.Drives[DriveIndex].Flags & FDC_DRIVE_RECALIBRATE)
	{
		/* at track 0? */
		if (FDI_GetCurrentDriveFlags() & FDD_FLAGS_HEAD_AT_TRACK_0)
		{

		}
		else
		{
			/* step drive head */
			FDI_PerformStep(DriveIndex, -1);

			/* update step count */
			fdc.Drives[DriveIndex].StepCount++;
		}
	}
	else if (fdc.Drives[DriveIndex].Flags & FDC_DRIVE_SEEK)
	{
		if (fdc.Drives[DriveIndex].PCN != fdc.Drives[DriveIndex].NCN)
		{
			int StepDirection = 0;

			// can pcn go below 0?
			if (fdc.Drives[DriveIndex].PCN > fdc.Drives[DriveIndex].NCN)
			{
				StepDirection = -1;
			}
			else
			{
				StepDirection = +1;
			}
			fdc.Drives[DriveIndex].PCN = (fdc.Drives[DriveIndex].PCN + StepDirection) & 0x0ff;
			
			/* step drive head */
			FDI_PerformStep(DriveIndex,StepDirection);
		}
	}

	/* check seek completed */
	FDC_CheckSeek(DriveIndex);
}
/*-----------------------------------------------------------------------*/

void	FDC_DoSeekOperationOnDrive(int DriveIndex)
{
	/* do step */
	FDC_DoSeekOperation(DriveIndex);
}
/*-----------------------------------------------------------------------*/

void	FDC_UpdateDrives(void)
{
	unsigned long NopCount;
	int i;
	int Difference;

	/* TODO: Join poll timer and step timer together. */
	NopCount = CPC_GetNopCount();
	Difference = NopCount - fdc.PollTimer;
	if (Difference >= fdc.PollTime) 
	{

		if (
			(fdc.LowLevelState == -1) &&
			(fdc.HighLevelState == NEC765_HIGH_LEVEL_STATE_COMMAND_PHASE_FIRST_BYTE)
			)
		{
			FDC_Poll();
		}
		fdc.PollTimer = NopCount - Difference;
	}

	Difference = NopCount - fdc.StepTimer;
	if ((Difference >= fdc.StepTime) && (fdc.StepTime !=0))
	{
		Difference -= fdc.StepTime;
		for (i = 0; i < FDC_MAX_DRIVES; i++)
		{
			if ((fdc.Drives[i].Flags & (FDC_DRIVE_RECALIBRATE | FDC_DRIVE_SEEK)) != 0)
			{
				FDC_DoSeekOperationOnDrive(i);
			}
		}
		fdc.StepTimer = NopCount - Difference;
	}


}

/*-----------------------------------------------------------------------*/

/* clear the low level state */
void	FDC_ClearLowLevelState(void)
{
	fdc.LowLevelState = -1;

}

/* setup a new low level state */
void	FDC_SetLowLevelState(int NewState)
{
	fdc.LowLevelState = NewState;

}

void	FDC_PopHighLevelState(void)
{
	fdc.HighLevelState = fdc.PushedHighLevelState;
}

void	FDC_PushHighLevelState(void)
{
	fdc.PushedHighLevelState = fdc.HighLevelState;
}

void	FDC_SetHighLevelState(int NewState)
{
	fdc.HighLevelState = NewState;


}

/* setup a delay before data request is set */
void	FDC_SetupDataRequestDelay(unsigned long CyclesToDataRequest)
{
	fdc.NopCountOfDataRequestStart = CPC_GetNopCount();
	fdc.CyclesToDataRequest = CyclesToDataRequest;

	FDC_SetLowLevelState(NEC765_LOW_LEVEL_STATE_DELAY);

}

void	FDC_SetupReadExecutionPhase(int DataSize, char *pBuffer)
{
	fdc.ExecutionBufferByteIndex = 0;
	fdc.ExecutionNoOfBytes = DataSize;
	fdc.ExecutionBuffer = pBuffer;

	/* push the high-level state */
	FDC_PushHighLevelState();

	/* clear data request */
	fdc.MainStatusRegister |= FDC_MSR_DATA_FLOW_DIRECTION;

	fdc.MainStatusRegister |= FDC_MSR_DATA_REQUEST;
	FDC_RefreshInterrupt();

	fdc.DataRegister = pBuffer[0];
	fdc.ExecutionNoOfBytes--;
	fdc.ExecutionBufferByteIndex=1;

	/* setup delay before first data request is issued (first byte is ready) */
	FDC_SetupDataRequestDelay(DATA_RATE_US*8);

	/* set high level state for data transfer */
/*	FDC_SetHighLevelState(NEC765_HIGH_LEVEL_STATE_EXECUTION_PHASE_READ_DATA); */

	fdc.HighLevelState = NEC765_HIGH_LEVEL_STATE_EXECUTION_PHASE_READ_DATA;

}

void	FDC_SetupResultPhase(int NoOfStatusBytes)
{
    fdc.nResultBytes = NoOfStatusBytes;
    fdc.ResultPhase = TRUE;
    FDC_RefreshInterrupt();

	/* fdc not in execution phase */
	fdc.MainStatusRegister &= ~(FDC_MSR_EXECUTION_PHASE | FDC_MSR_DATA_REQUEST);
	/* fdc data ready, and direction to cpu */
	fdc.MainStatusRegister |= FDC_MSR_DATA_FLOW_DIRECTION;

    FDC_SetDataRequest();
	fdc.DataRegister = fdc.ResultBytes[0];

	fdc.BytesRemaining = NoOfStatusBytes-1;
	fdc.ByteCount = 1;

	FDC_SetHighLevelState(NEC765_HIGH_LEVEL_STATE_RESULT_PHASE);
}

void FDC_SetDataRequest(void)
{
  	/* request data */
	fdc.MainStatusRegister |= FDC_MSR_DATA_REQUEST;

	FDC_RefreshInterrupt();
}

void FDC_ClearDataRequest(void)
{
    	/* request data */
	fdc.MainStatusRegister &= ~FDC_MSR_DATA_REQUEST;

	FDC_RefreshInterrupt();
}


void	FDC_SetupWriteExecutionPhase(int DataSize, char *pBuffer)
{
	fdc.ExecutionBufferByteIndex = 0;
	fdc.ExecutionNoOfBytes = DataSize;
	fdc.ExecutionBuffer = pBuffer;

	/* push the high-level state */
	FDC_PushHighLevelState();

	/* set data direction */
	fdc.MainStatusRegister &= ~(FDC_MSR_DATA_FLOW_DIRECTION);

	FDC_SetDataRequest();

	/* set high level state for data transfer */
/*	FDC_SetHighLevelState(NEC765_HIGH_LEVEL_STATE_EXECUTION_PHASE_WRITE_DATA); */
	fdc.HighLevelState = NEC765_HIGH_LEVEL_STATE_EXECUTION_PHASE_WRITE_DATA;
}


void	FDC_SetupForExecutionPhase(void)
{
	/* setup initial command state */
	fdc.CommandState = 0;

    FDC_ClearDataRequest();

    /* non dma mode? */
    if (fdc.SpecifyBytes[2] & (1<<0))
    {
        /* signal execution phase */

        /* entering execution phase */
        fdc.MainStatusRegister |= FDC_MSR_EXECUTION_PHASE;
    }

	FDC_SetHighLevelState(NEC765_HIGH_LEVEL_STATE_EXECUTION_PHASE);

	fdc.CommandHandler(fdc.CommandState);
}

void	FDC_UpdateStateStatus(void)
{
	if (fdc.LowLevelState!=-1)
	{
		switch (fdc.LowLevelState)
		{
			/* delay for a specified period, then set data request.
			If data request is already set, then Overrun condition is set */
			case NEC765_LOW_LEVEL_STATE_DELAY:
			{
				unsigned long NopCount;
				unsigned long Difference;

				/* get current nop count */
				NopCount = CPC_GetNopCount();

				/* get difference = number of cycles passed so far */
				Difference = NopCount - fdc.NopCountOfDataRequestStart;

				/* exceeded time to data request? */
				if (Difference>=fdc.CyclesToDataRequest)
				{
					/* clear low level state */
					FDC_ClearLowLevelState();

				}


			}
			break;

			default:
				break;
		}

		return;
	}

	switch (fdc.HighLevelState)
	{
		case NEC765_HIGH_LEVEL_STATE_COMMAND_PHASE_FIRST_BYTE:
		{
#if 0
			/* data has been written */
			if ((fdc.MainStatusRegister & FDC_MSR_DATA_REQUEST)==0)
			{
				unsigned long Data;
				int	CommandIndex;

				Data = fdc.DataRegister;

				/* seek operation active?  */
				if (fdc.Flags & NEC765_FLAGS_SEEK_OPERATION)
				{
					/* invalid */
					Data = 0;
				}

				/* busy set after writing first byte of command  */
				fdc.MainStatusRegister |= FDC_MSR_BUSY;

                fdc.nCommandBytes = 0;
				/* store byte */
				fdc.CommandBytes[fdc.nCommandBytes] = Data;
                fdc.nCommandBytes++;

				/* get command word */
				CommandIndex = Data & FDC_COMMAND_WORD;

				/* set current command */
				fdc.CommandHandler = FDC_CommandTable[CommandIndex].CommandHandler;

				/* set number of bytes left to transfer */
				fdc.BytesRemaining = FDC_CommandTable[CommandIndex].NoOfCommandBytes;

				/* completed command? */
				if (fdc.BytesRemaining==0)
				{
					FDC_SetupForExecutionPhase();
				}
				else
				{
					/* issue a data request */
					fdc.MainStatusRegister |= FDC_MSR_DATA_REQUEST;
					FDC_SetHighLevelState(NEC765_HIGH_LEVEL_STATE_COMMAND_PHASE_REMAINING_BYTES);
				}
			}
#endif
		}
		break;

		case NEC765_HIGH_LEVEL_STATE_COMMAND_PHASE_REMAINING_BYTES:
		{
#if 0
			/* data has been written */
			if ((fdc.MainStatusRegister & FDC_MSR_DATA_REQUEST)==0)
			{
				/* store byte */
				fdc.CommandBytes[fdc.CommandByteIndex] = fdc.DataRegister;
				/* store number of bytes written so far */
				fdc.CommandByteIndex++;
				/* decrement number of bytes to be transfered. */
				fdc.BytesRemaining--;

				/* completed command? */
				if (fdc.BytesRemaining==0)
				{
					/* go to next state */
					FDC_SetupForExecutionPhase();
				}
				else
				{
					/* issue a data request */
					fdc.MainStatusRegister |= FDC_MSR_DATA_REQUEST;

				}
			}
#endif
		}
		break;

		case NEC765_HIGH_LEVEL_STATE_EXECUTION_PHASE_READ_DATA:
		{
			/* data request set? */
			if (fdc.MainStatusRegister & FDC_MSR_DATA_REQUEST)
			{
				/* set overrun condition */
				fdc.ST1 |= FDC_ST1_OVERRUN;
				FDC_ClearDataRequest();
				/* go to high level state */
				FDC_PopHighLevelState();
				return;
			}
		}
		break;

		case NEC765_HIGH_LEVEL_STATE_EXECUTION_PHASE_WRITE_DATA:
		{
#if 0
			/* data request set? */
			if (fdc.MainStatusRegister & FDC_MSR_DATA_REQUEST)
			{
				/* set overrun condition */
				fdc.ST1 |= FDC_ST1_OVERRUN;

                FDC_ClearDataRequest();
				/* go to high level state */
				FDC_PopHighLevelState();
				return;
			}
#endif
		}
		break;

		case NEC765_HIGH_LEVEL_STATE_EXECUTION_PHASE:
		{
			/* execute command handler */
			fdc.CommandHandler(fdc.CommandState);
		}
		break;

		case NEC765_HIGH_LEVEL_STATE_RESULT_PHASE:
		{
			/* data has been read */
			if ((fdc.MainStatusRegister & FDC_MSR_DATA_REQUEST)==0)
			{
				if (fdc.BytesRemaining==0)
				{
					/* go to next state */
					FDC_Standby();
				}
				else
				{
				    /* when first byte is read the interrupt is reset */
				    fdc.ResultPhase = TRUE;

					/* store byte */
					fdc.DataRegister = fdc.ResultBytes[fdc.ByteCount];
					fdc.ByteCount++;

					/* decrement number of bytes to be transfered. */
					fdc.BytesRemaining--;

                    FDC_SetDataRequest();
				}
			}

		}
		break;

	}

}


void	FDC_UpdateStateData(BOOL bWrite)
{

	switch (fdc.HighLevelState)
	{
		case NEC765_HIGH_LEVEL_STATE_COMMAND_PHASE_FIRST_BYTE:
		{
			/* data has been written */
			if (
                (bWrite==TRUE) &&
                ((fdc.MainStatusRegister & FDC_MSR_DATA_FLOW_DIRECTION)==0)
                )

			{
				unsigned char Data;
				int	CommandIndex;


                /* clear data request */
                FDC_ClearDataRequest();

				Data = fdc.DataRegister;

				/* busy set after writing first byte of command  */
				fdc.MainStatusRegister |= FDC_MSR_BUSY;
				fdc.Flags &= ~NEC765_FLAGS_POLL_ALLOWED;

                fdc.nCommandBytes = 0;
				/* store byte */
				fdc.CommandBytes[fdc.nCommandBytes] = Data;
                fdc.nCommandBytes++;


				/* get command word */
				CommandIndex = Data & FDC_COMMAND_WORD;

				/* not fully correct */
				if (CommandIndex == 0x07)
				{
					if (Data != 0x07)
					{
						CommandIndex = 0;
					}
				}
				if (CommandIndex == 0x0f)
				{
					if (Data != 0x0f)
					{
						CommandIndex = 0;
					}
				}
				if (CommandIndex == 0x04)
				{
					if (Data != 0x04)
					{
						CommandIndex = 0;
					}
				}
				if (CommandIndex == 0x08)
				{
					if (Data != 0x08)
					{
						CommandIndex = 0;
					}
				}

				/* seems to be correct */
				if ((CommandIndex != 0x08) && (CommandIndex!=0x0f) && (CommandIndex!=0x07) && (CommandIndex!=0x04) && (CommandIndex!=3))
				{
					if ((fdc.MainStatusRegister & 0x0f) != 0)
					{
						FDC_CancelSeek();
					}
				}

				/* set current command */
				fdc.CommandHandler = FDC_CommandTable[CommandIndex].CommandHandler;

				/* set number of bytes left to transfer */
				fdc.BytesRemaining = FDC_CommandTable[CommandIndex].NoOfCommandBytes;

				/* completed command? */
				if (fdc.BytesRemaining==0)
				{
					FDC_SetupForExecutionPhase();
				}
				else
				{

					/* issue a data request */
					FDC_SetDataRequest();
					FDC_SetHighLevelState(NEC765_HIGH_LEVEL_STATE_COMMAND_PHASE_REMAINING_BYTES);
				}
			}
		}
		break;

		case NEC765_HIGH_LEVEL_STATE_COMMAND_PHASE_REMAINING_BYTES:
		{
			/* data has been written */
			if (
            (bWrite==TRUE) &&
			((fdc.MainStatusRegister & FDC_MSR_DATA_FLOW_DIRECTION)==0)
			)
            {

                /* clear data request */
                FDC_ClearDataRequest();

				/* store byte */
				fdc.CommandBytes[fdc.nCommandBytes] = fdc.DataRegister;
				fdc.nCommandBytes++;
				/* decrement number of bytes to be transfered. */
				fdc.BytesRemaining--;

				/* completed command? */
				if (fdc.BytesRemaining==0)
				{
					/* go to next state */
					FDC_SetupForExecutionPhase();
				}
				else
				{
					/* issue a data request */
					FDC_SetDataRequest();
				}
			}
		}
		break;

		case NEC765_HIGH_LEVEL_STATE_EXECUTION_PHASE_READ_DATA:
		{
			/* a read of the data register will clear the data request */
            if (
            ((fdc.MainStatusRegister & FDC_MSR_DATA_FLOW_DIRECTION)!=0) &&
            (bWrite==FALSE)
            )
            {
                /* overrun condition doesn't apply for last byte */
                if (fdc.ExecutionNoOfBytes==0)
                {
                    FDC_ClearDataRequest();
                    FDC_PopHighLevelState();
                    return;

                }

                FDC_ClearDataRequest();

                /* data request set? */
                if ((fdc.ST1 & FDC_ST1_OVERRUN)!=0)
                {
                    FDC_ClearDataRequest();
                    /* go to high level state */
                    FDC_PopHighLevelState();
                    return;
                }


                {
                    /* overrun not set */

                    /* get byte of data - store in data register */
                    fdc.DataRegister = fdc.ExecutionBuffer[fdc.ExecutionBufferByteIndex];
                    fdc.ExecutionBufferByteIndex++;
                    fdc.ExecutionNoOfBytes--;

                    FDC_SetDataRequest();
                    /* setup a delay before data request is set */
                    FDC_SetupDataRequestDelay(DATA_RATE_US);
                }
            }
		}
		break;

		case NEC765_HIGH_LEVEL_STATE_EXECUTION_PHASE_WRITE_DATA:
		{
			/* overrun not set */
            if (
                ((fdc.MainStatusRegister & FDC_MSR_DATA_FLOW_DIRECTION)==0) && (bWrite)
                )
            {
                /* get data register and store data */
                fdc.ExecutionBuffer[fdc.ExecutionBufferByteIndex] = fdc.DataRegister;
                fdc.ExecutionBufferByteIndex++;
                fdc.ExecutionNoOfBytes--;

                /* any bytes remaining? */
                if (fdc.ExecutionNoOfBytes==0)
                {
                    /* no */
                    FDC_ClearDataRequest();
                    FDC_PopHighLevelState();
                    return;
                }
                else
                {
                    /* yes */
                    /* clear data request */
                    FDC_ClearDataRequest();
#if 0
                    /* data request set? */
                    if ((fdc.ST1 & FDC_ST1_OVERRUN)!=0)
                    {
                        /* set overrun condition */
                        fdc.ST1 |= FDC_ST1_OVERRUN;
                           FDC_ClearDataRequest();

                        /* go to high level state */
                        FDC_PopHighLevelState();
                        return;
                    }
#endif
                    FDC_SetDataRequest();
                    /* setup a delay before data request is set */
                    FDC_SetupDataRequestDelay(DATA_RATE_US);
                }
            }
		}
		break;

		#if 0
		case NEC765_HIGH_LEVEL_STATE_EXECUTION_PHASE:
		{
			/* execute command handler */
			fdc.CommandHandler(fdc.CommandState);
		}
		break;
#endif
		
		case NEC765_HIGH_LEVEL_STATE_RESULT_PHASE:
		{
			/* data has been read */
			if (
			((fdc.MainStatusRegister & FDC_MSR_DATA_FLOW_DIRECTION)!=0) &&
			(bWrite==FALSE)
			)

			{
                /* clear data request */
                FDC_ClearDataRequest();
				if (fdc.BytesRemaining==0)
				{
					/* go to next state */
					FDC_Standby();
				}
				else
				{


					/* store byte */
					fdc.DataRegister = fdc.ResultBytes[fdc.ByteCount];
					/* store number of bytes written so far */
					fdc.ByteCount++;
					/* decrement number of bytes to be transfered. */
					fdc.BytesRemaining--;

                    FDC_SetDataRequest();
				}
			}



		}
		break;

	}
}

void FDC_Cycle(void)
{
	FDC_UpdateDrives();
}
/* TODO: Correct overrun handling. it's stopping writing of data */

/* read from main status register */
unsigned int	FDC_ReadMainStatusRegister(void)
{
	unsigned char Data;

	Data = fdc.MainStatusRegister;

	/* update current state */
	FDC_UpdateStateStatus();

	/* return current status */
	return Data;
}

/* read from data register */
unsigned int	FDC_ReadDataRegister(void)
{
	unsigned char Data;

	/* get data from data register */
	Data = fdc.DataRegister;

	FDC_UpdateStateData(FALSE);

	/* return data */
	return Data;

}

/* - when writing command bytes, data written to port can be read back again */
void	FDC_WriteDataRegister(int Data)
{
	/* set data */
	fdc.DataRegister = Data;

	FDC_UpdateStateData(TRUE);

}

void	FDC_Standby(void)
{
	/* set data flow to accept commands */
	/* set fdc not busy */
	/* set fdc not in execution phase */
	/* set fdc ready to accept data */
	fdc.MainStatusRegister &= (~(FDC_MSR_DATA_FLOW_DIRECTION|FDC_MSR_BUSY|FDC_MSR_EXECUTION_PHASE));
	FDC_SetDataRequest();
	fdc.LowLevelState = -1;
	FDC_SetHighLevelState(NEC765_HIGH_LEVEL_STATE_COMMAND_PHASE_FIRST_BYTE);
    fdc.nCommandBytes = 0;
    fdc.nResultBytes = 0;
	fdc.Flags |= NEC765_FLAGS_POLL_ALLOWED;

	/* disable led */
	FDI_SetCurrentFDDLEDState(FALSE);
}

void	FDC_Power(void)
{
	int i;
	
	/* CONFIRMED: testing indicates all registers cleared to zero which means 
	dma mode is active, slowest step rate. TODO: confirm head load/unload timings */
	fdc.SpecifyBytes[0] = 0;
	fdc.SpecifyBytes[1] = 0;
	fdc.SpecifyBytes[2] = 0;
	/* ensure internal step rate time is updated */
	{

		int SRT = FDC_GetStepRateTime();

		fdc.StepTime = FDC_STEP_RATE_TO_MICROSECONDS(SRT);
	}
	/* on power on it seems the data register is reset to 0 */
	 fdc.DataRegister = 0;

	/* reset PCN for drives. Power on indicates they are reset to 0 */
	for (i = 0; i<FDC_MAX_DRIVES; i++)
	{
		fdc.Drives[i].PCN = 0;
	}
	fdc.Flags = 0;
	fdc.StepTimer = CPC_GetNopCount();
	fdc.PollTimer = CPC_GetNopCount();
	fdc.PollTime = FDC_STEP_RATE_TO_MICROSECONDS(1);
}

void	FDC_Reset(void)
{
	int i;

	fdc.PollTime = FDC_STEP_RATE_TO_MICROSECONDS(1);
	fdc.PollTimer = CPC_GetNopCount();
	fdc.StepTimer = CPC_GetNopCount();
	fdc.Flags = 0;

	/* drive specific stuff */
	for (i = 0; i<FDC_MAX_DRIVES; i++)
	{
		fdc.Drives[i].Flags=0;
		fdc.Drives[i].PCN = 0;		// set to 0 on reset?
		fdc.Drives[i].NCN = 0;
		fdc.Drives[i].ST0 = 0;
		fdc.Drives[i].StepCount = 0;
	}
	/* Places FDC in idle state. Resets output lines to FDD to "0" (low) */
	/* DRQ goes low too, int goes low, read write mode,  */
    fdc.DriveOutput = 0;
    fdc.SideOutput = 0;
    fdc.MainStatusRegister = 0;
    fdc.ResultPhase = FALSE;
    FDC_RefreshInterrupt();
	FDC_Standby();
}

/*-------------------------------------------------------------*/

BOOL	FDC_LastSector(void)
{
	/* next sector */
	fdc.SectorCounter = (fdc.SectorCounter+1)&0x0ff;

	/* is sector offset == EOT?? */
	if ((fdc.SectorCounter & 0x0ff)== fdc.CommandBytes[6])
	{
		/* multi-track? */
		if (fdc.CommandBytes[0] & 0x080)
		{
			if ((fdc.CommandBytes[3]&1)!=0)
				return TRUE;

			/* set H */
			fdc.CommandBytes[3] |= 1;

			/* swap side */
			FDC_SetSideOutput(fdc.CurrentSide^1);

			/* reset sector counter */
			fdc.SectorCounter = 0;

			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}

	return FALSE;
}

/*-------------------------------------------------------------*/

void	FDC_ReadATrack(int State)
{
	BOOL bUpdateState;

	do
	{
		bUpdateState = FALSE;

		switch (fdc.CommandState)
		{
			case 0:
			{
				CHRN	ReadATrack_CHRN;

				/* reset status registers */
				FDC_ClearStatusRegisters();

				/* get current drive and side */
				FDC_GetDriveAndSide();

				FDI_SetDensity(fdc.CommandBytes[0]&0x040);
				fdc.stp = 1;
				
				/* drive ready? */
				if ((FDI_GetSelectedDriveFlags() & FDD_FLAGS_DRIVE_READY)==0)
				{
					fdc.SectorCounter = fdc.CommandBytes[4];

					/* not ready */
					fdc.ST0 = FDC_ST0_NOT_READY;

					/* end command */
					fdc.CommandState = 4;
					bUpdateState = TRUE;
					break;
				}


				/* for now */
				/* setup the initial sector offset */
				fdc.SectorCounter = 0;

				/* If a sector is found which has C,H,R,N matching that
				programmed, this flag will be reset. Otherwise it will
				remain set. Therefore indicating if a sector was found
				or not with ID that matched the programmed ID */
				fdc.ST1 |= FDC_ST1_NO_DATA;

				/* start command execution */
				fdc.CommandState++;
				bUpdateState = TRUE;

				/* wait for index pulse */
				while (FDI_GetNextID(&ReadATrack_CHRN)==0);
			}
			break;

			case 1:
			{
				BOOL bSkip = FALSE;
				BOOL bControlMark = FALSE;
				CHRN	ReadATrack_CHRN;

				do
				{
					unsigned long status;

					status = FDI_GetNextID(&ReadATrack_CHRN);

					/* if seen index pulse and sector counter is 0 */
					if ((status!=0) && (fdc.SectorCounter==0))
					{
						fdc.SectorCounter = fdc.CommandBytes[4];

						/* missing address mark */
						fdc.ST1 = FDC_ST1_MISSING_ADDRESS_MARK;
						/* quit */
						fdc.CommandState=4;
						bUpdateState = TRUE;
						break;
					}

					if (status!=0)
					{
						FDI_GetNextID(&ReadATrack_CHRN);
					}

					/* does programmed ID match this ID? */
					if ((ReadATrack_CHRN.C == fdc.CommandBytes[2]) &&
						(ReadATrack_CHRN.H == fdc.CommandBytes[3]) &&
						(ReadATrack_CHRN.R == fdc.CommandBytes[4]) &&
						(ReadATrack_CHRN.N == fdc.CommandBytes[5]))
					{
						/* C,H,R,N match those programmed */

						/* clear the no data flag */
						fdc.ST1 &= ~FDC_ST1_NO_DATA;
					}

					/* READ DATA */

					/* if deleted data mark is detected, set control mark flag */
					if  (
						((ReadATrack_CHRN.ST2 & FDC_ST2_CONTROL_MARK)!=0)
						)
					{
						/* skip and deleted data mark found */
						bControlMark = TRUE;
					}

					if (bControlMark)
					{
						/* read a "deleted data" using "read data" */
						/* read a "data" using "read deleted data" */
						fdc.ST2 |= FDC_ST2_CONTROL_MARK;
					}

					bSkip = FALSE;

					if (
						/* skip? */
						((fdc.CommandBytes[0] & FDC_COMMAND_SKIP)!=0) &&
						/* deleted data mark? */
						((ReadATrack_CHRN.ST2 & FDC_ST2_CONTROL_MARK)!=0)
						)
					{
						bSkip = TRUE;
					}

					if (bSkip)
					{
						if (FDC_LastSector())
						{
							fdc.ST1 |= FDC_ST1_END_OF_CYLINDER;

							/* current offset = EOT. All sectors transfered.
							Finish command */
							fdc.CommandState = 4;
							bUpdateState = TRUE;
							break;
						}
					}
					else
					{
						/* if N of sector is different to N in command
						then a data error will be reported */
						if (ReadATrack_CHRN.N!=fdc.CommandBytes[5])
						{
							fdc.ST2|=FDC_ST2_DATA_ERROR_IN_DATA_FIELD;
							fdc.ST1|=FDC_ST1_DATA_ERROR;
						}
						else
						{
							/* N specified in read track is same as N specified for sector
							*/

							/* does sector have a data error? */
							if (ReadATrack_CHRN.ST2 & FDC_ST2_DATA_ERROR_IN_DATA_FIELD)
							{
								fdc.ST2|=FDC_ST2_DATA_ERROR_IN_DATA_FIELD;
								fdc.ST1|=FDC_ST1_DATA_ERROR;
							}
						}

						FDI_ReadSector(FDC_DataBuffer);

						/* setup for execution phase */
						fdc.CommandState++;
						/* execution phase 2 */
						FDC_SetupReadExecutionPhase(FDC_GetSectorSize(fdc.CommandBytes[5]),FDC_DataBuffer);
						break;
					}
				}
				while (1==1);
			}
			break;

			case 2:
			{
				if (fdc.ST1 & FDC_ST1_OVERRUN)
				{
					fdc.CommandState = 4;
					bUpdateState = TRUE;
					break;
				}

				/* drive ready? */
				if ((FDI_GetSelectedDriveFlags() & FDD_FLAGS_DRIVE_READY) == 0)
				{
					fdc.SectorCounter = fdc.CommandBytes[4];

					/* not ready */
					fdc.ST0 = FDC_ST0_NOT_READY;

					/* end command */
					fdc.CommandState = 4;
					bUpdateState = TRUE;
					break;
				}

				if (FDC_LastSector())
				{
					fdc.ST1 |= FDC_ST1_END_OF_CYLINDER;

					/* current offset = EOT. All sectors transfered.
					Finish command */
					fdc.CommandState = 4;
					bUpdateState = TRUE;
					break;
				}


				/* go to attempt to read it */
				fdc.CommandState--;
				bUpdateState = TRUE;
			}
			break;

			case 4:
			{
				/* setup result data */
				FDC_SetStatus0();

				fdc.ResultBytes[6] = fdc.CommandBytes[5];
				fdc.ResultBytes[5] = fdc.SectorCounter;
				fdc.ResultBytes[4] = fdc.CommandBytes[3];
				fdc.ResultBytes[3] = fdc.CommandBytes[2];
				fdc.ResultBytes[0] = fdc.ST0;
				fdc.ResultBytes[1] = fdc.ST1;
				fdc.ResultBytes[2] = fdc.ST2;

				FDC_SetupResultPhase(7);

				FDC_ClearAllInterrupts();
				FDC_RefreshInterrupt();

			}
			break;
		}
	}
	while (bUpdateState);
}

/*-------------------------------------------------------------*/


/* RECALIBRATE */
void	FDC_Recalibrate(int State)
{
	switch (State)
	{
		case 0:
		{
			FDC_BeginSeek();

			fdc.CommandState++;
		}
		break;

		case 1:
		{
		}
		break;
	}

}
/*-------------------------------------------------------------*/


void	FDC_Seek(int State)
{
	switch (State)
	{
		case 0:
		{
			FDC_BeginSeek();

			fdc.CommandState++;
		}
		break;

		case 1:
		{
		/*	FDC_BeginSeek(); */
		}
		break;
	}
}

/*-------------------------------------------------------------*/
/* FORMAT A TRACK */
void	FDC_FormatATrack(int State)
{
	BOOL bUpdateState;

	do
	{
		bUpdateState = FALSE;

		switch (fdc.CommandState)
		{
			case 0:
			{

				/* reset status registers */
				FDC_ClearStatusRegisters();

				FDI_SetDensity(fdc.CommandBytes[0]&0x040);
				fdc.stp = 1;

				/* get current drive and side */
				FDC_GetDriveAndSide();

				if ((FDI_GetSelectedDriveFlags() & FDD_FLAGS_DRIVE_READY)==0)
				{
					/* not ready */

					fdc.ST0 = FDC_ST0_NOT_READY;

					fdc.CommandState = 4;
					bUpdateState = TRUE;
					break;
				}

				if ((FDI_GetSelectedDriveFlags() & FDD_FLAGS_WRITE_PROTECTED)!=0)
				{
                    fdc.ST1 = FDC_ST1_NOT_WRITEABLE;
                    fdc.CommandState = 4;
                    bUpdateState = TRUE;
                    break;
				}


				/* initialise sector count */
				fdc.SectorCounter  = 0;

				FDI_EmptyTrack();

				fdc.CommandState++;
				bUpdateState = TRUE;
	#if 0
				/* this is the delay from the point that the ID field is written to
				the end of data for a sector */
				fdc.StoredDelay =
				(
				/* CRC */
				2 +
				/* GAP 2 */
				22 +
				/* Sync (0) */
				12 +
				/* A1 */
				3 +
				/* Data/Deleted Data Address Mark */
				1 +
				/* N */
				((1<<fdc.CommandBytes[2])<<7) +
				/* CRC */
				2 +
				/* GAP 3 */
				fdc.CommandBytes[4]
				) * DATA_RATE_US;

				/* initialise write ptr */
				fdc.pTrackPtr = (unsigned char *)TrackBuffer;
				fdc.pTrackStart = (unsigned char *)TrackBuffer;
				fdc.pTrackEnd = fdc.pTrackStart + TRACK_SIZE;

				/* time to first data request is:
				time_to_next_index + 80 + 12 + 3 + 1 + 50 + 12 + 3
				*/

				NopsToNextIndex = FDC_GetNopsToNextIndex();

				FDC_SetupDataRequestDelay(((80+12+3+1+50+12+3)*DATA_RATE_US)+NopsToNextIndex);
	#endif

			}
			break;

			case 1:
			{
				{
	#if 0
					if (fdc.SectorCounter==0)
					{
						FDD_WriteBytesToTrack(0x04e, 80);
						FDD_WriteBytesToTrack(0x00, 12);
						FDD_WriteBytesToTrack(0x0c2,3);
						FDD_WriteByteToTrack(0x0fc);
						FDD_WriteBytesToTrack(0x04e, 50);
						FDD_WriteBytesToTrack(0x00, 12);
						FDD_WriteBytesToTrack(0x0a1, 3);
					}


					if (fdc.SectorCounter!=0)
					{
						int i;

						/* C,H,R,N of previous track */
						FDD_WriteByteToTrack(FormatATrack_CHRN.C);
						FDD_WriteByteToTrack(FormatATrack_CHRN.H);
						FDD_WriteByteToTrack(FormatATrack_CHRN.R);
						FDD_WriteByteToTrack(FormatATrack_CHRN.N);

						/* write CRC for ID */
						FDD_WriteByteToTrack(fdc.CRC>>8);

						FDD_WriteByteToTrack(fdc.CRC);

						/* GAP 2 */
						FDD_WriteBytesToTrack(0x04e, 22);

						/* SYNC */
						FDD_WriteBytesToTrack(0x00, 12);

						FDD_WriteBytesToTrack(0x0a1, 3);

						/* data mark */
						FDD_WriteByteToTrack(FDC_DATA_MARK);

						/* sector data */
						{
							unsigned char FillerByte = fdc.CommandBytes[5];
							int SectorSize = ((1<<fdc.CommandBytes[2])<<7);

							FDD_WriteBytesToTrack(FillerByte, SectorSize);

						}

						FDD_WriteByteToTrack(fdc.CRC>>8);
						FDD_WriteByteToTrack(fdc.CRC);

						FDD_WriteBytesToTrack(0x04e, fdc.CommandBytes[4]);
					}
	#endif


					/* setup for execution phase */
					fdc.CommandState++;

					/* get ID for this sector C,H,R,N */
					FDC_SetupWriteExecutionPhase(4,FDC_DataBuffer);
				}
			}
			break;

			case 2:
			{
//				if (fdc.ST1 & FDC_ST1_OVERRUN)
//				{
//					fdc.CommandState = 4;
//					bUpdateState = TRUE;
//					break;
//				}
				CHRN format_chrn;

				format_chrn.C = FDC_DataBuffer[0];
				format_chrn.H = FDC_DataBuffer[1];
				format_chrn.R = FDC_DataBuffer[2];
				format_chrn.N = FDC_DataBuffer[3];
				format_chrn.ST1 = 0x0;
				format_chrn.ST2 = 0x0;

				/* write sector */
				/* fdc.CommandBytes[2] = N */
				/* fdc.CommandBytes[3] = SC */
				/* fdc.CommandBytes[4] = GPL */
				/* fdc.CommandBytes[5] = D */

				FDI_AddSector(&format_chrn, fdc.CommandBytes[2],fdc.CommandBytes[5], fdc.CommandBytes[4]);

				/* next sector */
				fdc.SectorCounter++;
				bUpdateState = TRUE;

				/* have we done all sectors -> 0 means 256 */
				if ((fdc.SectorCounter&0x0ff) == (fdc.CommandBytes[3]&0x0ff))
				{


					/* finished transfering ID's */
					fdc.CommandState = 4;
					bUpdateState = TRUE;

					/* last sector */
	/*				FDC_SetupDataRequestDelay(fdc.StoredDelay + NopsToNextIndex); */
					return;
				}
				else
				{
					fdc.CommandState--;
				}
				/* drive ready? */
				if ((FDI_GetSelectedDriveFlags() & FDD_FLAGS_DRIVE_READY) == 0)
				{
					fdc.SectorCounter = fdc.CommandBytes[4];

					/* not ready */
					fdc.ST0 = FDC_ST0_NOT_READY;

					/* end command */
					fdc.CommandState = 4;
					bUpdateState = TRUE;
					break;
				}

	/*			FDC_SetupDataRequestDelay(fdc.StoredDelay + 12 + 3 + 1); */
			}
			break;

			case 4:
			{
				/* setup result data */
				FDC_SetStatus0();

	/*			fdc.CommandBytes[6] = FormatATrack_CHRN.N;
				fdc.CommandBytes[5] = (FormatATrack_CHRN.R+1) & 0x0ff;
				fdc.CommandBytes[4] = FormatATrack_CHRN.H;
				fdc.CommandBytes[3] = FormatATrack_CHRN.C; */
				fdc.ResultBytes[0] = fdc.ST0;
				fdc.ResultBytes[1] = fdc.ST1;
				fdc.ResultBytes[2] = fdc.ST2;

				FDC_SetupResultPhase(7);

				FDC_ClearAllInterrupts();
                FDC_RefreshInterrupt();

			}
			break;
		}
	}
	while (bUpdateState);
}

/*-------------------------------------------------------------*/

BOOL FDC_LocateSector(void)
{
	int IndexCount;
  /*  int Delay;
    int Bytes = 0;
*/
	/* set no data flag */
	fdc.ST1 |= FDC_ST1_MISSING_ADDRESS_MARK;
	fdc.ST1 &=~FDC_ST1_NO_DATA;

	IndexCount = 0;

	do
	{
		unsigned long status;

		status = FDI_GetNextID(&fdc.chrn);

		if (status==0)
		{
			/* got id */
#if 0
			/* minimum we must have read to get id */
            Bytes+=
                12+ /* 12 bytes &00  */
                3+ /* 3 bytes &a1 */
                1+ /* id mark */
                4+ /* c,h,r,n */
                2; /* crc */
#endif
			/* got a id, therefore missing address mark is false */
			fdc.ST1 &=~FDC_ST1_MISSING_ADDRESS_MARK;

			if (fdc.chrn.R == fdc.CommandBytes[4])
			{
				/* found sector with id we want */

				if (fdc.chrn.C == fdc.CommandBytes[2])
				{
					/* C value the same */

					if ((fdc.chrn.H == fdc.CommandBytes[3]) && (fdc.chrn.N == fdc.CommandBytes[5]))
					{
#if 0
					    /* got sector we want, so minimum required to get to sector's data */
					    Bytes +=
                                12+ /* 12 bytes &00  */
                                3+ /* 3 bytes &a1 */
                                1; /* 1 byte data mark */
#endif
						/* found, clear the no data flag */
						return TRUE;
					}
				}
				else
				{
					/* C doesn't match */

					/* C doesn't match and is 0x0ff */
					if (fdc.chrn.C == 0x0ff)
					{
						/* signal bad cylinder */
						fdc.ST2|=FDC_ST2_BAD_CYLINDER;
					}
					else
					{
						/* signal wrong cylinder */
						fdc.ST2 |= FDC_ST2_WRONG_CYLINDER;
					}

					fdc.ST1 |= FDC_ST1_NO_DATA;

					return FALSE;
				}
			}
		}
#if 0
        if (status==0)
        {
            /* found id, but not sector we want */
            /* minimum to get to data field */
		    Bytes +=
                    12+ /* 12 bytes &00  */
                    3+ /* 3 bytes &a1 */
                    1; /* 1 byte data mark */

            /* skip sector's data */
            Bytes += FDC_GetSectorSize(fdc.chrn.N);

            /* crc for sector */
            Bytes += 2;

            /* add on gap 3 for next sector */
            Bytes += Gap3;
        }
#endif
		/* seen index pulse? */
		if (status & 1)
		{
#if 0
		    /* final gap */
		    Bytes -= Gap3;
            /* add on size of last gap */
		    Bytes += Gap4;

		    /* skip gap 1 */
		    Bytes += Gap1;
#endif
			/* update number of times that we have seen the index */
			IndexCount++;
		}

	}
	while (IndexCount!=2);

	/* if id field was found, but encountered index pulse for second time
	then the sector was not matched */
	if ((fdc.ST1 & FDC_ST1_MISSING_ADDRESS_MARK)==0)
	{
		fdc.ST1 |= FDC_ST1_NO_DATA;
	}

	/* error, seen index pulse twice without finding sector */
	return FALSE;
}

/*-------------------------------------------------------------*/

void	FDC_ReadID(int State)
{
	BOOL bUpdateState;

	do
	{
		bUpdateState = FALSE;

		switch (fdc.CommandState)
		{
			case 0:
			{
				/* clear status reg */
				FDC_ClearStatusRegisters();

				/* get drive and side */
				FDC_GetDriveAndSide();

				FDI_SetDensity(fdc.CommandBytes[0]&0x040);
				fdc.stp = 1;

                /* set the LED for this disc drive */
                FDI_SetCurrentFDDLEDState(TRUE);

				/* drive ready ? */
				if ((FDI_GetSelectedDriveFlags() & FDD_FLAGS_DRIVE_READY)!=0)
				{
					int IndexCount;

					IndexCount = 0;

					/* set no data flag */
					fdc.ST1|=FDC_ST1_MISSING_ADDRESS_MARK;

					do
					{
						unsigned long status;


						/* get id and store into internal registers */
						status = FDI_GetNextID(&fdc.chrn);

						if (status==0)
						{
							/* clear missing address mark error */
							fdc.ST1 &=~FDC_ST1_MISSING_ADDRESS_MARK;
							break;
						}

						if (status & 1)
						{
							IndexCount++;

							if (IndexCount==2)
							{
								break;
							}
						}
					}
					while (1==1);
				}
				else
				{
					fdc.ST0 |= FDC_ST0_NOT_READY;
				}
				fdc.CommandState++;
				bUpdateState = TRUE;

				FDC_SetStatus0();
				fdc.ResultBytes[0] = fdc.ST0;
				fdc.ResultBytes[1] = fdc.ST1;
				fdc.ResultBytes[2] = fdc.ST2;
				fdc.ResultBytes[3] = fdc.chrn.C;
				fdc.ResultBytes[4] = fdc.chrn.H;
				fdc.ResultBytes[5] = fdc.chrn.R;
				fdc.ResultBytes[6] = fdc.chrn.N;
			}
			break;

			case 1:
			{
				FDC_SetupResultPhase(7);

				FDC_ClearAllInterrupts();
				FDC_RefreshInterrupt();


			}
			break;


			default:
				break;
			}
	}
	while (bUpdateState);
}


/*-----------------------------------------------------------------------*/

void	FDC_SenseInterruptStatus(int State)
{
	int i;

	/* check drives are interrupting */
	for (i=0; i<FDC_MAX_DRIVES; i++)
	{
		/* this drive interrupting? */
		if (fdc.Drives[i].Flags & FDC_DRIVE_INTERRUPT_STATE)
		{
			fdc.MainStatusRegister &= ~(1 << i);
			/* clear the interrupt */
			fdc.Drives[i].Flags &= ~FDC_DRIVE_INTERRUPT_STATE;
			FDC_RefreshInterrupt();

			/* set st0 */
			fdc.ResultBytes[0] = fdc.Drives[i].ST0;
			/* PCN for drive */
			fdc.ResultBytes[1] = fdc.Drives[i].PCN;
			/* init result phase */
			FDC_SetupResultPhase(2);
			return;
		}
	}

	/* no interrupts active */
	fdc.ResultBytes[0] = 0x080;
	FDC_SetupResultPhase(1);

}



BOOL FDC_UpdateReadID(void)
{
	if (fdc.CommandBytes[4]==fdc.CommandBytes[6])
	{
		/* multi-track */
		if (fdc.CommandBytes[0] & 0x080)
		{
			/* side 1? */
			if ((fdc.CommandBytes[3]&1)!=0)
				return TRUE;

			/* update ID */

			/* set side */
			fdc.CommandBytes[3]|=1;
			/* set id */
			fdc.CommandBytes[4]=1;

			/* swap side */
			FDC_SetSideOutput(fdc.CurrentSide^1);

			/* continue */
			return FALSE;
		}
		else
		{
			/* not multi-track */

			/* done */
			return TRUE;
		}
	}

	/* not done */

	/* increment id */
	fdc.CommandBytes[4]=(fdc.CommandBytes[4]+fdc.stp)&0x0ff;
	return FALSE;
}

/*-------------------------------------------------------------*/

void	FDC_ReadData(int State)
{
	BOOL bUpdateState;

	do
	{
		bUpdateState = FALSE;

		switch (fdc.CommandState)
		{
			case 0:
			{
				FDC_GetDriveAndSide();

				FDC_ClearStatusRegisters();

				FDI_SetDensity(fdc.CommandBytes[0]&0x040);
				fdc.stp = 1;

                /* set the LED for this disc drive */
                FDI_SetCurrentFDDLEDState(TRUE);

				if ((FDI_GetSelectedDriveFlags() & FDD_FLAGS_DRIVE_READY)==0)
				{
					/* not ready */

					fdc.ST0 = FDC_ST0_NOT_READY;

					fdc.CommandState = 4;
					State = 4;
					bUpdateState = TRUE;
					
					break;
				}

				/* allow backtro to work.
				 doesn't expect data request to be set so early! */
				FDC_SetupDataRequestDelay(DATA_RATE_US);

				fdc.CommandState++;
				/*bUpdateState = TRUE; */
			}
			break;

			case 1:
			{
				/* transfer data */
				do
				{

					/* ALL drives? */
					/* drive ready? */
					if ((FDI_GetSelectedDriveFlags() & FDD_FLAGS_DRIVE_READY) == 0)
					{
						/* not ready */
						fdc.ST0 = FDC_ST0_NOT_READY;
						
						/* end command */
						fdc.CommandState = 4;
						bUpdateState = TRUE;
						break;
					}

					/* get index of sector data */
					if (FDC_LocateSector())
					{
						BOOL bControlMark = FALSE;
						BOOL bSkip = FALSE;

						/* TODO: return sector status here */
						FDI_ReadSector(FDC_DataBuffer);

						if ((fdc.CommandBytes[0] & 0x01f)==0x06)
						{
							/* READ DATA */

							/* if deleted data mark is detected, set control mark flag */
							if  (
								((fdc.chrn.ST2 & FDC_ST2_CONTROL_MARK)!=0)
								)
							{
								/* skip and deleted data mark found */
								bControlMark = TRUE;
							}
						}
						else
						{
							/* READ DELETED DATA */

							/* if data mark is detected, set control mark flag */

							if  (
								((fdc.chrn.ST2 & FDC_ST2_CONTROL_MARK)==0)
								)
							{
								/* skip and data mark found */
								bControlMark = TRUE;
							}

						}

						if (bControlMark)
						{
							/* read a "deleted data" using "read data" */
							/* read a "data" using "read deleted data" */
							fdc.ST2 |= FDC_ST2_CONTROL_MARK;
						}

						if  (
							/* skip flag set */
							((fdc.CommandBytes[0] & FDC_COMMAND_SKIP)!=0) &&
							/* this sector will set control mark */
							(bControlMark)
							)
						{
							/* skip and deleted data mark found */
							bSkip = TRUE;
						}


						if (!bSkip)
						{
							/* no skip */
							int ReadSize = 0;
							if (fdc.chrn.N == 0)
							{
								/* fdc in my type 2, DTL: bit 0=0 -> 0x050, else 0x028 */
								/* TODO: GPL involvement */

								ReadSize = ((fdc.CommandBytes[8] & 0x01) == 0) ? 0x050 : 0x028;
							}
							else
							{
								ReadSize = FDC_GetSectorSize(fdc.chrn.N);
							}

							fdc.CommandState++;

							/* transfer sector data to cpu */
							FDC_SetupReadExecutionPhase(ReadSize,FDC_DataBuffer);

							break;
						}
						else
						{

							/* skip! */

							/* any more to read? */
							if (FDC_UpdateReadID())
							{
								/* no */

								/* have read last sector */
								fdc.ST1 |= FDC_ST1_END_OF_CYLINDER;

								fdc.CommandState = 4;

								bUpdateState = TRUE;

                                /* delay been last sector and end of execution phase */
                                /* delay is equal to the time of 2 bytes of CRC */
			                  /*  FDC_SetupDataRequestDelay((DATA_RATE_US<<1)); */

								break;
							}

						}
					}
					else
					{
						/* error finding sector */
						fdc.CommandState = 4;
						bUpdateState = TRUE;
						break;
					}
				}
				while (1==1);

			}
			break;

			case 2:
			{
				BOOL bHalt = FALSE;

				if (fdc.ST1 & FDC_ST1_OVERRUN)
				{
					fdc.CommandState = 4;
					bUpdateState = TRUE;
					break;
				}

				if (
					/* skip not set? */
					((fdc.CommandBytes[0] & FDC_COMMAND_SKIP)==0) &&
					/* control mark set? */
					((fdc.ST2 & FDC_ST2_CONTROL_MARK)!=0)
					)
				{
					bHalt = TRUE;
				}


				if (bHalt)
				{
					fdc.CommandState = 4;
					bUpdateState = TRUE;
					break;
				}


				/* error ? */
				if (
				   (fdc.chrn.ST1 & FDC_ST1_MISSING_ADDRESS_MARK) ||
				   (fdc.chrn.ST2 & FDC_ST2_DATA_ERROR_IN_DATA_FIELD) ||
				   (fdc.chrn.ST2 & FDC_ST2_MISSING_ADDRESS_MARK_IN_DATA_FIELD)
				   )
				{
					/* end of cylinder not set! */

					fdc.ST1 |= FDC_ST1_DATA_ERROR;

					/* data error in data field? */
					if (fdc.chrn.ST2 & FDC_ST2_DATA_ERROR_IN_DATA_FIELD)
					{
						/* set data error */
						fdc.ST2 |= FDC_ST2_DATA_ERROR_IN_DATA_FIELD;
					}

					/* we want to quit */
					fdc.CommandState = 4;
					bUpdateState = TRUE;
					break;
				}

				if (!FDC_UpdateReadID())
				{

					fdc.CommandState = 1;
					bUpdateState = TRUE;
					break;
				}

				/* have read last sector */
				fdc.ST1 |= FDC_ST1_END_OF_CYLINDER;

				fdc.CommandState = 4;
				bUpdateState = TRUE;
			}
			break;



			case 4:
			{
				/* finish whole command */
				FDC_SetStatus0();

				fdc.ResultBytes[6] = fdc.CommandBytes[5];
				fdc.ResultBytes[5] = fdc.CommandBytes[4];
				fdc.ResultBytes[4] = fdc.CommandBytes[3];
				fdc.ResultBytes[3] = fdc.CommandBytes[2];

				fdc.chrn.C = fdc.CommandBytes[3];
				fdc.chrn.H = fdc.CommandBytes[4];
				fdc.chrn.R = fdc.CommandBytes[5];
				fdc.chrn.N = fdc.CommandBytes[6];


				fdc.ResultBytes[0] = fdc.ST0;
				fdc.ResultBytes[1] = fdc.ST1;
				fdc.ResultBytes[2] = fdc.ST2;

				FDC_SetupResultPhase(7);

				FDC_ClearAllInterrupts();
				FDC_RefreshInterrupt();
			}
			break;
		}
	}
	while (bUpdateState);

}

/*-------------------------------------------------------------*/

void	FDC_WriteData(int State)
{
	BOOL bUpdateState;

	do
	{
		bUpdateState = FALSE;

		switch (fdc.CommandState)
		{
			case 0:
			{
				FDC_GetDriveAndSide();

				FDC_ClearStatusRegisters();

				FDI_SetDensity(fdc.CommandBytes[0]&0x040);
				fdc.stp = 1;


                /* set the LED for this disc drive */
                FDI_SetCurrentFDDLEDState(TRUE);

				if ((FDI_GetSelectedDriveFlags() & FDD_FLAGS_DRIVE_READY)==0)
				{
					/* not ready */

					fdc.ST0 = FDC_ST0_NOT_READY;

					fdc.CommandState = 4;
					bUpdateState = TRUE;
					break;
				}


				if ((FDI_GetSelectedDriveFlags() & FDD_FLAGS_WRITE_PROTECTED)!=0)
				{
                    fdc.ST1 = FDC_ST1_NOT_WRITEABLE;
                    fdc.CommandState = 4;
                    bUpdateState = TRUE;
                    break;
				}

				fdc.CommandState++;
				bUpdateState = TRUE;
			}
			break;

			case 1:
			{

				/* ALL drives? */
				/* drive ready? */
				if ((FDI_GetSelectedDriveFlags() & FDD_FLAGS_DRIVE_READY) == 0)
				{
					/* not ready */
					fdc.ST0 = FDC_ST0_NOT_READY;

					/* end command */
					fdc.CommandState = 4;
					bUpdateState = TRUE;
					break;
				}


				/* transfer data */
				if (FDC_LocateSector())
				{
					fdc.CommandState++;

					FDC_SetupWriteExecutionPhase(FDC_GetSectorSize(fdc.chrn.N),FDC_DataBuffer);
					break;
				}
#if 0
				else
				{
					/* error finding sector */
					fdc.CommandState = 4;
					bUpdateState = TRUE;
					break;
				}
#endif

				fdc.CommandState = 4;
				bUpdateState = TRUE;
			}
			break;

			case 2:
			{
				/* to get here, the sector must have been located */

				int Mark;

				/* data or deleted data address mark? */
				if ((fdc.CommandBytes[0]&0x01f)==9)
				{
					/* deleted data */
					Mark = 1;
				}
				else
				{
					/* data */
					Mark = 0;
				}

				FDI_WriteSector(FDC_DataBuffer, Mark);

				/* drive ready? */
				if ((FDI_GetSelectedDriveFlags() & FDD_FLAGS_DRIVE_READY) == 0)
				{
					fdc.SectorCounter = fdc.CommandBytes[4];

					/* not ready */
					fdc.ST0 = FDC_ST0_NOT_READY;

					/* end command */
					fdc.CommandState = 4;
					bUpdateState = TRUE;
					break;
				}


				/* go to next sector */
				if (!FDC_UpdateReadID())
				{
					fdc.CommandState = 1;
					bUpdateState = TRUE;
					break;
				}

				/* set end of cylinder */
				fdc.ST1 |= FDC_ST1_END_OF_CYLINDER;
				fdc.CommandState = 4;
				bUpdateState = TRUE;
			}
			break;

			case 4:
			{
				/* finish whole command */
				FDC_SetStatus0();

				fdc.ResultBytes[6] = fdc.CommandBytes[5];
				fdc.ResultBytes[5] = fdc.CommandBytes[4];
				fdc.ResultBytes[4] = fdc.CommandBytes[3];
				fdc.ResultBytes[3] = fdc.CommandBytes[2];
				fdc.ResultBytes[0] = fdc.ST0;
				fdc.ResultBytes[1] = fdc.ST1;
				fdc.ResultBytes[2] = fdc.ST2;

				FDC_SetupResultPhase(7);

				FDC_ClearAllInterrupts();
				FDC_RefreshInterrupt();

			}
			break;

		}
	}
	while (bUpdateState);

}

/*-------------------------------------------------------------*/

void	FDC_Scan(int State)
{
	BOOL bUpdateState;

	do
	{
		bUpdateState = FALSE;

		switch (fdc.CommandState)
		{
			case 0:
			{
				FDC_GetDriveAndSide();

				FDC_ClearStatusRegisters();

				FDI_SetDensity(fdc.CommandBytes[0]&0x040);

				fdc.stp = fdc.CommandBytes[8];

                /* set the LED for this disc drive */
                FDI_SetCurrentFDDLEDState(TRUE);

				if ((FDI_GetSelectedDriveFlags() & FDD_FLAGS_DRIVE_READY)==0)
				{
					/* not ready */

					fdc.ST0 = FDC_ST0_NOT_READY;
					bUpdateState = TRUE;
					fdc.CommandState = 4;
					break;
				}

				bUpdateState = TRUE;
				fdc.CommandState++;
			}
			break;

			case 1:
			{
				/* transfer data */
				if (FDC_LocateSector())
				{
					fdc.CommandState++;

					FDC_SetupWriteExecutionPhase(FDC_GetSectorSize(fdc.chrn.N), FDC_DataBuffer);
					break;
				}
				/* error finding sector */
				fdc.CommandState = 4;
				bUpdateState = TRUE;
			}
			break;

			case 2:
			{
				int Size = FDC_GetSectorSize(fdc.CommandBytes[5]);
				int i;
				BOOL bMatch = TRUE;
				BOOL bEqual = FALSE;

				FDI_ReadSector(FDC_ScanDataBuffer);

				for (i = 0; i < Size; i++)
				{
					if ((FDC_DataBuffer[i]&0x0ff) == 0xff)
					{
						bEqual = TRUE;
						continue;
					}

					/* scan low or equal */
					if ((fdc.CommandBytes[0] & 0x01f) == 0x019)
					{
						if (FDC_ScanDataBuffer[i] > FDC_DataBuffer[i])
						{
							bMatch = FALSE;
						}
					}
					/* scan high or equal */
					if ((fdc.CommandBytes[0] & 0x01f) == 0x01d)
					{
						if (FDC_ScanDataBuffer[i] < FDC_DataBuffer[i])
						{
							bMatch = FALSE;
						}
					}
					if ((fdc.CommandBytes[0] & 0x01f) == 0x011)
					{
						if (FDC_ScanDataBuffer[i] != FDC_DataBuffer[i])
						{
							bMatch = FALSE;
						}
					}

					if (FDC_ScanDataBuffer[i] == FDC_DataBuffer[i])
					{
						bEqual = TRUE;
					}
				}

				if (bMatch)
				{
					if (bEqual)
					{
						fdc.ST2 |= FDC_ST2_SCAN_EQUAL_HIT;
					}
					/* end command */
					fdc.CommandState = 4;
					bUpdateState = TRUE;
					break;
				}
				else
				{
					fdc.ST2 |= FDC_ST2_SCAN_NOT_SATISFIED;
				}

				/* drive ready? */
				if ((FDI_GetSelectedDriveFlags() & FDD_FLAGS_DRIVE_READY) == 0)
				{
					fdc.SectorCounter = fdc.CommandBytes[4];

					/* not ready */
					fdc.ST0 = FDC_ST0_NOT_READY;

					/* end command */
					fdc.CommandState = 4;
					bUpdateState = TRUE;
					break;
				}

				/* go to next sector */
				if (!FDC_UpdateReadID())
				{
					fdc.CommandState = 1;
					bUpdateState = TRUE;
					break;
				}

				/* go to attempt to write it */
				fdc.CommandState=4;
				bUpdateState = TRUE;
			}
			break;

			case 4:
			{
				/* finish whole command */
				FDC_SetStatus0();

				fdc.ResultBytes[6] = fdc.CommandBytes[5];
				fdc.ResultBytes[5] = fdc.CommandBytes[4];
				fdc.ResultBytes[4] = fdc.CommandBytes[3];
				fdc.ResultBytes[3] = fdc.CommandBytes[2];
				fdc.ResultBytes[0] = fdc.ST0;
				fdc.ResultBytes[1] = fdc.ST1;
				fdc.ResultBytes[2] = fdc.ST2;

				FDC_SetupResultPhase(7);

				FDC_ClearAllInterrupts();
				FDC_RefreshInterrupt();

			}
			break;

		}
	}
	while (bUpdateState);
}

void	FDC_SenseDriveStatus(int State)
{
	int FDC_ST3;
	unsigned long Flags;
	FDC_GetDriveAndSide();

	FDC_ST3 = 0;
	Flags = FDI_GetSelectedDriveFlags();

	if (Flags & FDD_FLAGS_DRIVE_READY)
	{
		FDC_ST3 |= FDC_ST3_READY;
	}

	if (Flags & FDD_FLAGS_WRITE_PROTECTED)
	{
		FDC_ST3 |= FDC_ST3_WRITE_PROTECTED;
	}

	if (Flags & FDD_FLAGS_TWO_SIDE)
	{
		FDC_ST3 |= FDC_ST3_TWO_SIDE;
	}

	if (Flags & FDD_FLAGS_HEAD_AT_TRACK_0)
	{
		FDC_ST3 |= FDC_ST3_TRACK_0;
	}

	/* Sense Drive Status sets drive and side */
	FDC_ST3_SET_DRIVE_AND_SIDE;

	fdc.ResultBytes[0] = FDC_ST3;

	FDC_SetupResultPhase(1);
}


void	FDC_Specify(int State)
{
	int SRT;

	fdc.SpecifyBytes[0] = fdc.CommandBytes[0];
	fdc.SpecifyBytes[1] = fdc.CommandBytes[1];
	fdc.SpecifyBytes[2] = fdc.CommandBytes[2];
    fdc.nResultBytes = 0;

	SRT = FDC_GetStepRateTime();

	fdc.StepTime = FDC_STEP_RATE_TO_MICROSECONDS(SRT);
	fdc.Flags |= NEC765_FLAGS_POLL_ALLOWED;

	FDC_Standby();
}

BOOL FDC_GetDmaMode(void)
{
    return ((fdc.SpecifyBytes[2]&(1<<0))==0);
}

int FDC_GetHeadLoadTime(void)
{
    return (fdc.SpecifyBytes[2]>>1)&0x07f;
}

int FDC_GetHeadUnLoadTime(void)
{
    return (fdc.SpecifyBytes[1]&0x0f);
}

int FDC_GetStepRateTime(void)
{
    return (16-((fdc.SpecifyBytes[1]>>4)&0x0f));
}

void	FDC_Invalid(int State)
{
	fdc.ResultBytes[0] = 0x080;

	FDC_SetupResultPhase(1);
}

void	FDC_ClearStatusRegisters(void)
{
	fdc.ST0 = fdc.ST1 = fdc.ST2 = 0;
}

/* set FDC Status Register Interrupt Code, based on the values
currently stored in ST0, ST1 and ST2 */
void	FDC_SetStatus0(void)
{

	if ((
		/* ST0 bits that specify an error */
		(fdc.ST0 & (FDC_ST0_EQUIPMENT_CHECK |
					FDC_ST0_NOT_READY)) 	|
		/* ST1 bits that specify an error */
		(fdc.ST1 & (FDC_ST1_MISSING_ADDRESS_MARK |
					FDC_ST1_NOT_WRITEABLE |
					FDC_ST1_NO_DATA |
					FDC_ST1_OVERRUN |
					FDC_ST1_DATA_ERROR |
					FDC_ST1_END_OF_CYLINDER))	|
		/* ST2 bits that specify an error */
		(fdc.ST2 & (/*FDC_ST2_CONTROL_MARK | */
					FDC_ST2_DATA_ERROR_IN_DATA_FIELD |
					FDC_ST2_WRONG_CYLINDER |
		//			FDC_ST2_SCAN_EQUAL_HIT |
		//			FDC_ST2_SCAN_NOT_SATISFIED |
					FDC_ST2_BAD_CYLINDER |
					FDC_ST2_MISSING_ADDRESS_MARK_IN_DATA_FIELD))
		)!=0)
	{
		fdc.ST0 |= 0x040;

	}
	else
	{
		fdc.ST0 &= 0x03f;
	}

	/* include drive and side details */
	fdc.ST0 |= (fdc.CurrentDrive | (fdc.CurrentSide<<2));
}

static int	FDC_GetSectorSize(int N)
{
	/* tested on CPC+ and CPC fdc */
	/* 0-> 0x0050 */
	/* 1-> 0x0100 */
	/* 2-> 0x0200 */
	/* 3-> 0x0400 */
	/* 4-> 0x1000 */
	/* 5-> 0x2000 */
	/* 6-> 0x4000 */
	/* 7-> 0x8000 */
	/* 8-> 0x8000 */
	/* >8 -> 0x08000 */
	/* TO BE TESTED; read value depends on dtl and gpl on mfm*/
	if (N==0)
		return 0x0050;
	/* verified */
	if (N>=8)
		return 0x8000;
	/* verified */
	return (1<<N)<<7;
}


