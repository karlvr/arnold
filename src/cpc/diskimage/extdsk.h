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
#ifndef __EXTDSK_HEADER_INCLUDE__
#define __EXTDSK_HEADER_INCLUDE__


#include "../headers.h"
#include "../cpcglob.h"
#include "diskimg.h"

#include "../device.h"

typedef struct
{
	unsigned char	C;
	unsigned char	H;
	unsigned char	R;
	unsigned char	N;
	unsigned char	ST1;
	unsigned char	ST2;
	unsigned char	SectorSizeLow;
	unsigned char	SectorSizeHigh;
} EXTDSKCHRN;

typedef struct
{
	char	TrackHeader[12];
	char	pad0[4];
	unsigned char	track;
	unsigned char	side;
	unsigned char   DataRate;           /* extension from John Elliot */
	unsigned char   RecordingMode;      /* extension from John Elliot */
	unsigned char	BPS;
	unsigned char	SPT;
	unsigned char	Gap3;
	unsigned char	FillerByte;
} EXTEXTDSKTRACKHEADER;

typedef struct
{
	char	TrackHeader[12];
	char	pad0[4];
	unsigned char	track;
	unsigned char	side;
	unsigned char	pad1[2];
	unsigned char	BPS;
	unsigned char	SPT;
	unsigned char	Gap3;
	unsigned char	FillerByte;
	EXTDSKCHRN	SectorIDs[29];
} EXTDSKTRACKHEADER;

typedef struct
{
	char		DskHeader[34];
	char		DskCreator[14];
	unsigned char	NumTracks;
	unsigned char	NumSides;
	unsigned char	TrackSizeLow;
	unsigned char	TrackSizeHigh;
	char		TrackSizeTable[255-4-14-33];
} EXTDSKHEADER;


int		ExtDsk_Validate(const unsigned char *pDiskImage, const unsigned long DiskImageSize);
#endif
