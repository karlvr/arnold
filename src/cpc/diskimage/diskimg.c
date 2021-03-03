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
/*
        DISK IMAGE HANDLING CODE

        If disk image is single sided, and access made to 2nd side, currently
        forces to side 0.

*/
#include "../fdd.h"
#include "diskimg.h"
#include "dsk.h"
#include "extdsk.h"
#include "iextdsk.h"
#include "../host.h"
#include "../device.h"
#include "../cpc.h"

static DISKIMAGE_UNIT   Drives[MAX_DRIVES];

static void DiskImage_ImageRecognised(int DriveID,int Type)
{
	DISKIMAGE_UNIT  *pDrive=&Drives[DriveID];

	pDrive->Flags |= DISKIMAGE_DISK_INSERTED;
	pDrive->nImageType = Type;
}

void    DiskImage_Initialise(void)
{
	memset(Drives,0,sizeof(DISKIMAGE_UNIT));
}

void    DiskImage_Finish(void)
{
	int i;
	for (i=0; i<MAX_DRIVES; i++)
	{
		DiskImage_RemoveDisk(i);
		FDD_InsertDisk(i,FALSE);
	}
}

/*********************************************************************************/

int		DiskImage_InsertUnformattedDisk(int DriveID)
{
	/* remove existing disk */
	DiskImage_RemoveDisk(DriveID);

	DiskImage_ImageRecognised(DriveID,DISK_IMAGE_TYPE_UNDEFINED);

	ExtDskInternal_Initialise(&Drives[DriveID]);

	Drives[DriveID].Flags |= DISKIMAGE_DISK_DIRTY;

	FDD_InsertDisk(DriveID, TRUE);
	return TRUE;
}

static unsigned char DATA_Sectors[9]= {0x0c1, 0x0c6, 0x0c2, 0x0c7, 0x0c3, 0x0c8, 0x0c4, 0x0c9, 0x0c5};
static unsigned char VENDOR_Sectors[9]= {0x041, 0x046, 0x042, 0x047, 0x043, 0x048, 0x044, 0x049, 0x045};
static unsigned char VORTEX_Sectors[9] = { 0x001, 0x06, 0x02, 0x07, 0x03, 0x08, 0x04, 0x09, 0x05 };
static unsigned char ROMDOSD1_Sectors[9] = { 0x01, 0x06, 0x02, 0x07, 0x03, 0x08, 0x04, 0x09, 0x05 };
static unsigned char ROMDOSD2_Sectors[9] = { 0x21, 0x26, 0x22, 0x27, 0x23, 0x28, 0x24, 0x29, 0x25 };
static unsigned char ROMDOSD10_Sectors[10] = { 0x11, 0x012, 0x013, 0x014, 0x015, 0x016, 0x017, 0x018, 0x019, 0x01a };
static unsigned char ROMDOSD20_Sectors[10] = { 0x31, 0x032, 0x033, 0x034, 0x035, 0x036, 0x037, 0x038, 0x039, 0x03a };


static FORMAT_DESCRIPTION FormatDescriptions[10]=
{
	{
		"DATA 40T SS", 1, 40, 2, 9, 0x0e5,
		{
			DATA_Sectors,
			DATA_Sectors
		},
	},
	{
		"DATA 80T SS", 1, 80, 2, 9, 0x0e5,
		{
			DATA_Sectors,
			DATA_Sectors
		},
	},
	{
		"DATA 40T DS", 2, 40, 2, 9, 0x0e5,
		{
			DATA_Sectors,
			DATA_Sectors
		},
	},
	{
		"DATA 80T DS", 2, 80, 2, 9, 0x0e5,
		{
			DATA_Sectors,
			DATA_Sectors
		},
	},
	{
		"VENDOR/SYSTEM 40T SS", 1, 40, 2, 9, 0x0e5,
		{
			VENDOR_Sectors,
			VENDOR_Sectors
		},
	},
	{
		"VORTEX 80T DS", 2, 80, 2, 9, 0x0e5,
		{
			VORTEX_Sectors,
			VORTEX_Sectors
		},
	},

	{
		"ROMDOS/RAMDOS D1 80T DS", 2, 80, 2, 9, 0x0e5,
		{
			ROMDOSD1_Sectors,
			ROMDOSD1_Sectors,
		}
	},
	{
		"ROMDOS/RAMDOS D2 80T DS", 2, 80, 2, 9, 0x0e5,
		{
			ROMDOSD2_Sectors,
			ROMDOSD2_Sectors,
		}
	},
	{
		"ROMDOS/RAMDOS D10 80T DS", 2, 80, 2, 10, 0x0e5,
		{
			ROMDOSD10_Sectors,
			ROMDOSD10_Sectors,
		}
	},
	{
		"ROMDOS/RAMDOS D20 80T DS", 2, 80, 2, 10, 0x0e5,
		{
			ROMDOSD20_Sectors,
			ROMDOSD20_Sectors,
		}
	}
};

int GetNumFormatDescriptions()
{
	return sizeof(FormatDescriptions)/sizeof(FormatDescriptions[0]);
}

const FORMAT_DESCRIPTION *GetFormatDescription(int nDescription)
{
	return &FormatDescriptions[nDescription];
}


int DiskImage_InsertFormattedDisk(int nDrive, int nDescription)
{
	const  FORMAT_DESCRIPTION *pDescription = GetFormatDescription(nDescription);
	/* remove existing disk */
	DiskImage_RemoveDisk(nDrive);

	DiskImage_ImageRecognised(nDrive,DISK_IMAGE_TYPE_UNDEFINED);

	ExtDskInternal_InsertFormattedDisk(&Drives[nDrive], pDescription);

	Drives[nDrive].Flags |= DISKIMAGE_DISK_DIRTY;

	FDD_InsertDisk(nDrive, TRUE);
	return TRUE;
}
/*********************************************************************************/
#if 0
int		DiskImage_InsertDataFormattedDisk(int DriveID)
{
	/* remove existing disk */
	DiskImage_RemoveDisk(DriveID);

	DiskImage_ImageRecognised(DriveID,DISK_IMAGE_TYPE_UNDEFINED);

	ExtDskInternal_InsertDataFormatDisk(&Drives[DriveID]);

	Drives[DriveID].Flags |= DISKIMAGE_DISK_DIRTY;

	FDD_InsertDisk(DriveID, TRUE);
	return TRUE;
}
#endif

int    DiskImage_Recognised(const unsigned char *pDiskImage, const unsigned long DiskImageLength)
{
	if (Dsk_Validate(pDiskImage, DiskImageLength))
	{
		return ARNOLD_STATUS_OK;
	}
	else
		if (ExtDsk_Validate(pDiskImage,DiskImageLength))
		{
			return ARNOLD_STATUS_OK;
		}
		else
			if (Dif_Validate(pDiskImage,DiskImageLength))
			{
				return ARNOLD_STATUS_OK;
			}

	return ARNOLD_STATUS_UNRECOGNISED;
}

BOOL DiskImage_HasSide(int DriveID, int nSide)
{
	return ExtDskInternal_HasSide(&Drives[DriveID], nSide);
}


/* install a disk image into the drive */

/* pDiskImage is pointer to disk image data loaded from host */
/* DiskImageLength is length of data loaded from host */
/* assumes therefore that disc image data is contained in a single file */
int     DiskImage_InsertDisk(int DriveID, const unsigned char *pDiskImage, const unsigned long DiskImageLength)
{
	/* validate disk image */
	if (Dsk_Validate(pDiskImage, DiskImageLength))
	{
		/* standard dsk */
		DiskImage_RemoveDisk(DriveID);

		/* valid */
		DiskImage_ImageRecognised(DriveID,DISK_IMAGE_TYPE_STANDARD);

		ExtDskInternal_Dsk2ExtDskInternal(&Drives[DriveID],pDiskImage, DiskImageLength);

		FDD_InsertDisk(DriveID, TRUE);
		return ARNOLD_STATUS_OK;
	}
	else
		if (ExtDsk_Validate(pDiskImage,DiskImageLength))
		{
			/* extdsk */
			DiskImage_RemoveDisk(DriveID);

			DiskImage_ImageRecognised(DriveID,DISK_IMAGE_TYPE_EXTENDED);

			ExtDskInternal_ExtDsk2ExtDskInternal(&Drives[DriveID],pDiskImage, DiskImageLength);

			FDD_InsertDisk(DriveID, TRUE);
			return ARNOLD_STATUS_OK;
		}
		else
			if (Dif_Validate(pDiskImage,DiskImageLength))
			{
				/* dif */
				DiskImage_RemoveDisk(DriveID);

				DiskImage_ImageRecognised(DriveID,DISK_IMAGE_TYPE_DIF);

				ExtDskInternal_Dif2ExtDskInternal(&Drives[DriveID],pDiskImage,DiskImageLength);

				FDD_InsertDisk(DriveID, TRUE);
				return ARNOLD_STATUS_OK;
			}
	return ARNOLD_STATUS_UNRECOGNISED;
}

/* remove a disk image from the drive */
void    DiskImage_RemoveDisk(int DriveID)
{
	DISKIMAGE_UNIT  *pDrive=&Drives[DriveID];

	ExtDskInternal_Free(pDrive);

	/* initialise drive */
	memset(pDrive,0,sizeof(DISKIMAGE_UNIT));

	FDD_InsertDisk(DriveID,FALSE);
}

/**********************************************************************/
/* DISK IMAGE ACCESS FUNCTIONS - INTERFACE BETWEEN DISK IMAGE AND FDC */

int             DiskImage_GetSectorsPerTrack(int DriveID, int PhysicalTrack,int PhysicalSide)
{
	DISKIMAGE_UNIT  *pDrive=&Drives[DriveID];

	return ExtDskInternal_GetSectorsPerTrack(pDrive, PhysicalTrack, PhysicalSide);
}

/* get a ID from physical track, physical side and fill in CHRN structure with details */
void    DiskImage_GetID(int DriveID, int PhysicalTrack,int PhysicalSide, int Index, CHRN *pCHRN)
{
	DISKIMAGE_UNIT  *pDrive=&Drives[DriveID];

	ExtDskInternal_GetID(pDrive, PhysicalTrack, PhysicalSide, Index, pCHRN);
}

/* get a sector of data from the disk image */
void    DiskImage_GetSector(int DriveID, int PhysicalTrack, int PhysicalSide, int Index, char *pData)
{
	DISKIMAGE_UNIT  *pDrive = &Drives[DriveID];

	ExtDskInternal_GetSector(pDrive, PhysicalTrack, PhysicalSide, Index, pData);
}

/* write a sector of data to the disk image */
void    DiskImage_PutSector(int DriveID, int PhysicalTrack, int PhysicalSide, int Index, char *pData, int Mark)
{
	DISKIMAGE_UNIT *pDrive = &Drives[DriveID];

	pDrive->Flags |= DISKIMAGE_DISK_DIRTY;

	ExtDskInternal_PutSector(pDrive, PhysicalTrack, PhysicalSide, Index, pData,Mark);
}


/* write a sector of data to the disk image */
void    DiskImage_AddSector(int DriveID, int PhysicalTrack, int PhysicalSide, CHRN *pCHRN, int FormatN,int FillerByte, int Gap3)
{
	DISKIMAGE_UNIT *pDrive = &Drives[DriveID];

	pDrive->Flags |= DISKIMAGE_DISK_DIRTY;

	ExtDskInternal_AddSector(pDrive, PhysicalTrack, PhysicalSide, pCHRN, FormatN,FillerByte);
	ExtDskInternal_WriteGAP3(pDrive, PhysicalTrack, PhysicalSide, Gap3);
}

void	DiskImage_EmptyTrack(int DriveID, int PhysicalTrack, int PhysicalSide)
{
	DISKIMAGE_UNIT *pDrive = &Drives[DriveID];

	pDrive->Flags |= DISKIMAGE_DISK_DIRTY;

	ExtDskInternal_EmptyTrack(pDrive, PhysicalTrack, PhysicalSide);
}

void    DiskImage_ResetDirty(int DriveID)
{
	DISKIMAGE_UNIT  *pDrive = &Drives[DriveID];

	pDrive->Flags &= ~DISKIMAGE_DISK_DIRTY;
}

BOOL    DiskImage_IsImageDirty(int DriveID)
{
	DISKIMAGE_UNIT  *pDrive = &Drives[DriveID];

	if (pDrive->Flags & DISKIMAGE_DISK_DIRTY)
	{
		return TRUE;
	}

	return FALSE;
}

unsigned long DiskImage_CalculateOutputSize(int DriveID, BOOL bCompatibility)
{
	DISKIMAGE_UNIT  *pDrive = &Drives[DriveID];

	return ExtDskInternal_CalculateOutputDataSize(pDrive,bCompatibility);
}


void    DiskImage_GenerateOutputData(unsigned char *pBuffer, int DriveID, BOOL bCompatibility)
{
	DISKIMAGE_UNIT  *pDrive = &Drives[DriveID];

	ExtDskInternal_GenerateOutputData(pBuffer, pDrive,bCompatibility);
}

