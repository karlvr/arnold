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
#include "fdd.h"
#include "fdi.h"

/* the two drives */
static FDD drives[MAX_DRIVES];

FDD		*FDD_GetDrive(int DriveIndex)
{
	return &drives[DriveIndex];
}

int FDD_GetTracks(int DriveIndex)
{
	return drives[DriveIndex].NumberOfTracks;
}

BOOL FDD_GetDoubleSided(int DriveIndex)
{
	return ((drives[DriveIndex].Flags & FDD_FLAGS_DOUBLE_SIDED)!=0);
}

unsigned long FDD_GetFlags(int DriveIndex)
{
	if ((drives[DriveIndex].Flags & FDD_FLAGS_DRIVE_ENABLED) != 0)
	{
		return drives[DriveIndex].Flags;
	}
	/* not enabled */
	return FDD_FLAGS_WRITE_PROTECTED | FDD_FLAGS_HEAD_AT_TRACK_0|FDD_FLAGS_TWO_SIDE;
}

void FDD_SetAlwaysWriteProtected(int DriveIndex, BOOL bWriteProtected)
{
    if (bWriteProtected)
    {
        drives[DriveIndex].Flags |= FDD_FLAGS_FORCE_WRITE_PROTECT;
    }
    else
    {
        drives[DriveIndex].Flags &= ~FDD_FLAGS_FORCE_WRITE_PROTECT;
    }
    FDD_RefreshState(DriveIndex);
}


BOOL FDD_IsAlwaysWriteProtected(int DriveIndex)
{
	return ((drives[DriveIndex].Flags & FDD_FLAGS_FORCE_WRITE_PROTECT)!=0);
}

void FDD_SetAlwaysReady(int DriveIndex, BOOL bReady)
{
    if (bReady)
    {
        drives[DriveIndex].Flags |= FDD_FLAGS_FORCE_READY;
    }
    else
    {
        drives[DriveIndex].Flags &= ~FDD_FLAGS_FORCE_READY;
    }
    FDD_RefreshState(DriveIndex);
}


BOOL FDD_IsAlwaysReady(int DriveIndex)
{
	return ((drives[DriveIndex].Flags & FDD_FLAGS_FORCE_READY)!=0);
}


void FDD_RefreshWriteProtect(int DriveIndex)
{
    drives[DriveIndex].Flags&=~(FDD_FLAGS_WRITE_PROTECTED|FDD_FLAGS_TWO_SIDE);

    /* drive enabled? */
	if ((drives[DriveIndex].Flags & FDD_FLAGS_DRIVE_ENABLED)!=0)
	{
        /* always write protected? */
        if ((drives[DriveIndex].Flags & FDD_FLAGS_FORCE_WRITE_PROTECT)!=0)
        {
		/* write protect and two side reported the same, at least on 3.5" and EME-157 */
            drives[DriveIndex].Flags|= FDD_FLAGS_WRITE_PROTECTED|FDD_FLAGS_TWO_SIDE;
        }
	}
#if 0
	else
	{

	    if (drives[DriveIndex].Flags & FDD_FLAGS_DISK_PRESENT)
	    {
            /* determine based on dsk? */
            drives[DriveIndex].Flags&=~FDD_FLAGS_WRITE_PROTECTED;
        }
        else
        {
            /* write protected is set if disc is not in drive */
            drives[DriveIndex].Flags&=~FDD_FLAGS_WRITE_PROTECTED;
        /*    drives[DriveIndex].Flags|=FDD_FLAGS_WRITE_PROTECTED; */
        }
	}
#endif
}

void FDD_RefreshTrack0(int DriveIndex)
{
    drives[DriveIndex].Flags &= ~FDD_FLAGS_HEAD_AT_TRACK_0;

    if ((drives[DriveIndex].Flags & FDD_FLAGS_DRIVE_ENABLED)!=0)
    {
        if /* drive head at track 0? */
        (drives[DriveIndex].CurrentTrack==0)
        {
            drives[DriveIndex].Flags |= FDD_FLAGS_HEAD_AT_TRACK_0;
        }
    }
}


void FDD_SetDoubleSided(int DriveIndex, BOOL bDoubleSided)
{
	if (bDoubleSided)
	{
		drives[DriveIndex].Flags|= FDD_FLAGS_DOUBLE_SIDED;
	}
	else
	{
		drives[DriveIndex].Flags&=~FDD_FLAGS_DOUBLE_SIDED;
	}
}

void FDD_SetTracks(int DriveIndex, int nTracks)
{
	drives[DriveIndex].NumberOfTracks = nTracks;
}

#if 0
void FDD_SetSingleSided40Track(int DriveIndex)
{
	FDD_SetDoubleSided(DriveIndex,FALSE);
	FDD_SetTracks(DriveIndex,43);
}

void FDD_SetDoubleSided80Track(int DriveIndex)
{
	FDD_SetDoubleSided(DriveIndex, TRUE);
	FDD_SetTracks(DriveIndex,83);
}

BOOL FDD_IsSingleSided40Track(int DriveIndex)
{
	if (FDD_GetDoubleSided(DriveIndex))
		return FALSE;

	if ((FDD_GetTracks(DriveIndex)>=40) && (FDD_GetTracks(DriveIndex)<=45))
		return TRUE;

	return FALSE;
}

BOOL FDD_IsDoubleSided80Track(int DriveIndex)
{
	if (!FDD_GetDoubleSided(DriveIndex))
		return FALSE;

	if ((FDD_GetTracks(DriveIndex)>=80) && (FDD_GetTracks(DriveIndex)<=85))
		return TRUE;

	return FALSE;
}
#endif

void	FDD_InitialiseAll(void)
{
	int i;

	for (i=0; i<MAX_DRIVES; i++)
	{
		FDD_Initialise(i);

		if (i==0)
		{
		    FDD_SetDoubleSided(i, FALSE);
		    FDD_SetTracks(i, MAX_TRACKS_40_TRACK);
		}
		else
		{
		    FDD_SetDoubleSided(i, TRUE);
		    FDD_SetTracks(i, MAX_TRACKS_80_TRACK);
		}
	}
	/* by default enable drive 0 and drive 1 */
	FDD_Enable(0, TRUE);
	FDD_Enable(1, TRUE);
}

/* perform the actual step */
void	FDD_PerformStep(unsigned long DriveIndex, signed int StepDirection)
{
	FDD *theDrive;
	int CurrentTrack;

	theDrive = FDD_GetDrive(DriveIndex);

	/* perform step */
	CurrentTrack = theDrive->CurrentTrack;
	CurrentTrack += StepDirection;

	/* range check head position */
	if (CurrentTrack<0)
	{
		CurrentTrack = 0;
	}
	else
	if (CurrentTrack>=theDrive->NumberOfTracks)
	{
		CurrentTrack = theDrive->NumberOfTracks-1;
	}

	theDrive->CurrentTrack = CurrentTrack;
	FDD_RefreshState(DriveIndex);

	theDrive->CurrentIDIndex = 0;
}

void FDD_RefreshState(int Drive)
{
    FDD_RefreshReadyState(Drive);
    FDD_RefreshWriteProtect(Drive);
    FDD_RefreshTrack0(Drive);
}


/* insert or remove a disk from a drive */
void	FDD_InsertDisk(int Drive,BOOL Status)
{
	FDD *drive = FDD_GetDrive(Drive);

	/* say disk is or isn't present */
	drive->Flags &=~FDD_FLAGS_DISK_PRESENT;

	if (Status)
	{
		drive->Flags |= FDD_FLAGS_DISK_PRESENT;
	}

	/* setup initial parameters for when a disk is present */
	drive->CurrentIDIndex = 0;

	FDD_RefreshState(Drive);
}

BOOL	FDD_IsDiskPresent(int Drive)
{
	return FDD_GetDrive(Drive)->Flags & FDD_FLAGS_DISK_PRESENT;
}

int     FDD_GetPhysicalSide(int Drive)
{
    FDD *drive = FDD_GetDrive(Drive);

    return drive->PhysicalSide;
}

BOOL    FDD_IsEnabled(int Drive)
{
    FDD *drive = FDD_GetDrive(Drive);
    return ((drive->Flags & FDD_FLAGS_DRIVE_ENABLED)!=0);
}

void    FDD_Enable(int Drive, BOOL bState)
{
    FDD *drive = FDD_GetDrive(Drive);

    if (bState)
    {
        drive->Flags |= FDD_FLAGS_DRIVE_ENABLED;
    }
    else
    {
        drive->Flags &=~FDD_FLAGS_DRIVE_ENABLED;
     }

    FDD_RefreshState(Drive);
}


BOOL    FDD_IsTurnDiskToSideB(int Drive)
{
	FDD *drive = FDD_GetDrive(Drive);

    return (drive->PhysicalSide!=0);
}

/* turn disk in the drive */
void	FDD_TurnDiskToSideB(int Drive, BOOL bState)
{
	FDD *drive = FDD_GetDrive(Drive);

	if (bState)
	{
		drive->PhysicalSide |= 1;
	}
	else
	{
		drive->PhysicalSide &= ~1;
	}
}

void	FDD_Initialise(int Drive)
{
	FDD *drive = FDD_GetDrive(Drive);

	/* TODO: generally the drive is unlikely to be at track 0 when turned on/off */
    drive->CurrentTrack = 1;

    drive->CurrentIDIndex = 0;
    /* set default side */
	drive->PhysicalSide = 0;
    /* set flags */
	drive->Flags = 0;

    FDD_RefreshState(Drive);
}

void	FDD_LED_SetState(unsigned long Drive, int LedState)
{
	FDD *drive = FDD_GetDrive(Drive);

	if (LedState)
	{
		drive->Flags |= FDD_FLAGS_LED_ON;
	}
	else
	{
		drive->Flags &=~FDD_FLAGS_LED_ON;
	}
}

int		FDD_LED_GetState(unsigned long Drive)
{
	FDD *drive = FDD_GetDrive(Drive);

	return ((drive->Flags & FDD_FLAGS_LED_ON)!=0);
}


void	FDD_RefreshReadyState(int Drive)
{
    /* really need a delay before drive becomes ready! */

	BOOL bReady = FALSE;

	FDD *drive = FDD_GetDrive(Drive);

    /* drive enabled? */
    if (drive->Flags & FDD_FLAGS_DRIVE_ENABLED)
    {
        /* force ready? */
        if (drive->Flags & FDD_FLAGS_FORCE_READY)
        {
            bReady = TRUE;
        }
        else
        {
            /* ok, ready not forced, check if drive is ready.. */

            /* disk present? */
            if (drive->Flags & FDD_FLAGS_DISK_PRESENT)
            {
                /* motor on? */
                if (FDI_GetMotorState())
                    bReady = TRUE;
            }
        }
    }

    if (bReady)
        drive->Flags |=FDD_FLAGS_DRIVE_READY;
    else
        drive->Flags &=~FDD_FLAGS_DRIVE_READY;
}

