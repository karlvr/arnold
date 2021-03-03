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
#include "fdi.h"
#include "fdd.h"
#include "diskimage/diskimg.h"

/* floppy disc interface sits between the fdc and the
fdd */

static FDI fdi;

/* ABBA drive switch; use to swap drives */
static unsigned long FDI_DriveSwitch = 0;
/* side switch; used to swap sides */
static unsigned long FDI_SideSwitch = 0;
/* disc motor output from floppy disc interface */
static int FDI_MotorState = 0;
static unsigned long FDI_DriveMask = 0x01;

static BOOL FDI_ForceSide1 = FALSE;
static BOOL FDI_ForceDiscRomOff = FALSE;

/* poor man's ABBA switch forces drive B to always be selected */
/* not easy to simulate in emu */

/* swap drives */
void FDI_SwapDrives(void)
{
	FDI_DriveSwitch^=0x01;
}

void FDI_SetSwapDrives(BOOL bSwap)
{

	if (bSwap)
	{
		FDI_DriveSwitch|=0x01;
	}
	else
	{
		FDI_DriveSwitch&=~0x01;
	}

}

/*-----------------------------------------------------------------------*/
/* get state of motor output from floppy disc interface */
BOOL FDI_GetMotorState(void)
{
	if ((FDI_MotorState & 0x01)==0x01)
	{
		return TRUE;
	}

	return FALSE;
}

/*-----------------------------------------------------------------------*/
/* set state of motor output from floppy disc interface */
void	FDI_SetMotorState(int MotorStatus)
{
	int i;

	FDI_MotorState = MotorStatus & 0x01;

	for (i = 0; i<CPC_MAX_DRIVES; i++)
	{
		FDD_RefreshReadyState(i);
	}

}

BOOL FDI_Get4Drives()
{
	return (FDI_DriveMask==0x03);
}

void FDI_Set4Drives(BOOL bFourDrives)
{
	if (bFourDrives)
	{
		FDI_DriveMask = 0x03;
		FDD_Enable(2, TRUE);
		FDD_Enable(3, TRUE);

	}
	else
	{
		FDI_DriveMask = 0x01;
		FDD_Enable(2, FALSE);
		FDD_Enable(3, FALSE);
	}
}

BOOL FDI_GetSwapDrives(void)
{
	return (FDI_DriveSwitch!=0);
}

BOOL FDI_GetSwapSides(void)
{
	return (FDI_SideSwitch!=0);
}

void FDI_SetSwapSides(BOOL bState)
{
	if (bState)
	{
		FDI_SideSwitch |=0x01;
	}
	else
	{
		FDI_SideSwitch &= ~0x01;
	}
}

BOOL FDI_GetForceSide1()
{
	return FDI_ForceSide1;
}

void FDI_SetForceSide1(BOOL bState)
{
	FDI_ForceSide1 = bState;
}

BOOL FDI_GetForceDiscRomOff()
{
	return FDI_ForceDiscRomOff;
}

void FDI_SetForceDiscRomOff(BOOL bState)
{
	FDI_ForceDiscRomOff = bState;
}

/*-----------------------------------------------------------------------*/
/* floppy disc interface set drive */
void FDI_SetPhysicalDrive(unsigned long Value)
{
    fdi.PhysicalDrive = FDI_GetDriveToAccess(Value);

	fdi.drive = FDD_GetDrive(fdi.PhysicalDrive);
}

void FDI_SetCurrentFDDLEDState(BOOL fState)
{
	FDD_LED_SetState(fdi.PhysicalDrive, fState);
}

unsigned long FDI_GetCurrentDriveFlags()
{
	return FDD_GetFlags(fdi.PhysicalDrive);
}

void FDI_SwapSides()
{
	FDI_SideSwitch^=1;
}

/*-----------------------------------------------------------------------*/

int FDI_GetDriveToAccess(int nDrive)
{
  	/* map fdc drive output to physical drive selects */
	return (nDrive & FDI_DriveMask)^FDI_DriveSwitch;
}

/*-----------------------------------------------------------------------*/

int FDI_GetSideToAccess(int nDrive, int nSideWanted)
{
	if (FDI_ForceSide1)
	{
		/* force side 1 */
		return 1;
	}
	else
	{

		/* double sided? */
		if ((FDD_GetFlags(nDrive) & FDD_FLAGS_DOUBLE_SIDED)!=0)
		{
			/* side can be changed */
			/* the value is effected by side output from FDC,
			Side Switch AND if the disc has been turned */

/*            fdi.PhysicalSide = Value^FDI_SideSwitch^FDD_GetPhysicalSide(fdi.PhysicalDrive); */

			/* side can be changed, the value is effected if the disc has been turned */
			return nSideWanted^FDI_SideSwitch^FDD_GetPhysicalSide(nDrive);
		}
		else
		{
			/* side can't be changed */

			/* but allow the disc to be turned over like a 3" disc can be */
			return FDD_GetPhysicalSide(nDrive);
		}
	}
}

/* update physical side selection based on side output from fdc and
	side switch selection */
void FDI_SetPhysicalSide(unsigned long Value)
{
	unsigned long PreviousPhysicalSide = fdi.PhysicalSide;

    fdi.PhysicalSide = FDI_GetSideToAccess(fdi.PhysicalDrive, Value);

	/* only reset if choosing a different side. ideally we need to keep track of index for both sides */
	/* this makes it work when you try to read a non-existant sector and then read an id */
	if (fdi.PhysicalSide!=PreviousPhysicalSide)
	{
		fdi.drive->Flags &=~FDD_FLAGS_DRIVE_INDEX;
	}
}
/*-----------------------------------------------------------------------*/
unsigned long FDI_GetSelectedDriveFlags(void)
{
	return FDD_GetFlags(fdi.PhysicalDrive);
}

int FDI_GetPhysicalDrive()
{
	return fdi.PhysicalDrive;
}

int FDI_GetPhysicalSide()
{
	return fdi.PhysicalSide;
}

/*-----------------------------------------------------------------------*/
void FDI_PerformStep(unsigned long DriveIndex, int Direction)
{
	FDI_SetPhysicalDrive(DriveIndex);

	FDD_PerformStep(fdi.PhysicalDrive, Direction);

	fdi.drive->CurrentIDIndex = 0;
	fdi.drive->Flags &=~FDD_FLAGS_DRIVE_INDEX;
}

unsigned long	FDI_GetNextID(CHRN *pCHRN)
{
	unsigned long SPT;

	SPT = DiskImage_GetSectorsPerTrack(
			  fdi.PhysicalDrive,
			  fdi.drive->CurrentTrack,
			  fdi.PhysicalSide);

	if ((SPT==0) || (fdi.Density==0))
	{
		/* if there are no sectors on this track, we see a index pulse */
		return 1;
	}

	/* was index flag set last time around? */
	if ((fdi.drive->Flags & FDD_FLAGS_DRIVE_INDEX)==0)
	{
		/* no */

		/* increment */
		fdi.drive->CurrentIDIndex++;

		/* beyond last sector of this track? */
		if (fdi.drive->CurrentIDIndex==SPT)
		{
			/* yes, reset id */

			fdi.drive->CurrentIDIndex = 0;

			/* set index flag */
			fdi.drive->Flags |= FDD_FLAGS_DRIVE_INDEX;
			/* return index flag state */
			return 1;
		}
	}
	else
	{
		/* clear index flag, don't increment counter this time */
		fdi.drive->Flags &=~FDD_FLAGS_DRIVE_INDEX;
	}


	/* get the id */
	DiskImage_GetID(
		fdi.PhysicalDrive,
		fdi.drive->CurrentTrack,
		fdi.PhysicalSide,
		fdi.drive->CurrentIDIndex,pCHRN);

	return 0;
}


void	FDI_ReadSector(char *pBuffer)
{
	/* get sector data */
	DiskImage_GetSector(
		fdi.PhysicalDrive,
		fdi.drive->CurrentTrack,
		fdi.PhysicalSide,
		fdi.drive->CurrentIDIndex,pBuffer);
}

void	FDI_WriteSector(char *pBuffer, int Mark)
{
	DiskImage_PutSector(
		fdi.PhysicalDrive,
		fdi.drive->CurrentTrack,
		fdi.PhysicalSide,
		fdi.drive->CurrentIDIndex,pBuffer,Mark);
}

void	FDI_EmptyTrack(void)
{
	DiskImage_EmptyTrack(
		fdi.PhysicalDrive,
		fdi.drive->CurrentTrack,
		fdi.PhysicalSide
	);
}

void	FDI_AddSector(CHRN *pCHRN, int N, int Filler, int Gap3)
{
	DiskImage_AddSector(fdi.PhysicalDrive,
						fdi.drive->CurrentTrack,
						fdi.PhysicalSide, pCHRN,N, Filler, Gap3);
}

void	FDI_SetDensity(int State)
{
	if (State!=0)
	{
		fdi.Density = 1;
	}
	else
	{
		fdi.Density = 0;
	}
}
