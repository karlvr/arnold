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
#ifndef __FDD_HEADER_INCLUDED__
#define __FDD_HEADER_INCLUDED__

#include "cpcglob.h"
#define MAX_DRIVES 4

/* FDD functions */
void	FDD_InitialiseAll(void);
void	FDD_Initialise(int);
void	FDD_TurnDiskToSideB(int, BOOL bState);
BOOL    FDD_IsTurnDiskToSideB(int);
void	FDD_InsertDisk(int,int);
BOOL	FDD_IsDiskPresent(int);
int     FDD_GetPhysicalSide(int Drive);
BOOL    FDD_IsEnabled(int);
void    FDD_Enable(int, BOOL);
int		FDD_LED_GetState(unsigned long Drive);
void	FDD_LED_SetState(unsigned long Drive, int LedState);

/* head is positioned at track 0 */
#define FDD_FLAGS_HEAD_AT_TRACK_0 0x001
/* disk inserted into drive */
#define FDD_FLAGS_DISK_PRESENT	0x002
/* drive is enabled */
#define FDD_FLAGS_DRIVE_ENABLED 0x004
/* drive is double sided */
#define FDD_FLAGS_DOUBLE_SIDED 0x008
/* drive is ready */
#define FDD_FLAGS_DRIVE_READY 0x010
/* write protected */
#define FDD_FLAGS_WRITE_PROTECTED 0x040
/* motor state */
#define FDD_FLAGS_MOTOR_STATE 0x020
/* index flag */
#define FDD_FLAGS_DRIVE_INDEX 0x080
/* led is on */
#define FDD_FLAGS_LED_ON 0x0100
/* force write protect */
#define FDD_FLAGS_FORCE_WRITE_PROTECT 0x0200
/* force ready */
#define FDD_FLAGS_FORCE_READY 0x0400
#define FDD_FLAGS_TWO_SIDE 0x0800
#define FDD_FLAGS_FAULT 0x01000

#define MAX_TRACKS_80_TRACK 84
#define MAX_TRACKS_40_TRACK 43


void FDD_SetDoubleSided(int DriveIndex, BOOL bDoubleSided);
void FDD_SetTracks(int DriveIndex, int nTracks);

int FDD_GetTracks(int DriveIndex);
BOOL FDD_GetDoubleSided(int DriveIndex);

/*void FDD_SetSingleSided40Track(int DriveIndex);
void FDD_SetDoubleSided80Track(int DriveIndex);

BOOL FDD_IsSingleSided40Track(int DriveIndex);
BOOL FDD_IsDoubleSided80Track(int DriveIndex);
*/


/* need state to determine drive overrides, and then also to return "current state" based on disc inserted */
typedef struct
{
	/* flags */
	unsigned long Flags;
	/* current track drive is on */
	unsigned long CurrentTrack;
	/* total number of tracks the head can move. */
	unsigned long NumberOfTracks;

	/* rotation speed in revolutions per minute */
	int Rpm;
	/* 3ms (3.5"), 6ms (5.25") */
	int StepRateMs;
	/* 15ms(3.5"), 25ms (5.25") (50ms including head settingling time) */
	int SettlingTimeMS;


	// 500ms(3.5"), 250ms (5.25")
//	int MotorStartTimeMS;

	// head movement on trailing edge of step pulse (mitsubishi 5.25" drive)

	// spindle motor rotates when disc is inserted.
	// disc rotates
	// index pulses in minimum time switches current ready and held ready
	// motor off
	// step is normally logical 1, but 0 used.

	// index is 4ms long for each revolution (200ms)

	// ready: index detected twice or more, disc is in drive and door closed


	int PhysicalSide;

	/* temp here until more accurate emulation is done */
	unsigned long CurrentIDIndex;			/* current id index */
} FDD;



FDD		*FDD_GetDrive(int DriveIndex);
/* get flags */
unsigned long FDD_GetFlags(int DriveIndex);

void	FDD_RefreshReadyState(int DriveIndex);
void    FDD_RefreshWriteProtect(int DriveIndex);
void    FDD_RefreshTrack0(int DriveIndex);

void	FDD_PerformStep(unsigned long DriveIndex, signed int StepDirection);

void FDD_SetAlwaysWriteProtected(int DriveIndex, BOOL bWriteProtected);
BOOL FDD_IsAlwaysWriteProtected(int DriveIndex);

void FDD_SetAlwaysReady(int DriveIndex, BOOL bReady);
BOOL FDD_IsAlwaysReady(int DriveIndex);

void FDD_RefreshState(int DriveIndex);
#endif
