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
#ifndef __SNAPSHOT_HEADER_INCLUDED__
#define __SNAPSHOT_HEADER_INCLUDED__

#include "cpcglob.h"

#define SNAPSHOT_HEADER_TEXT "MV - SNA"
#define SNAPSHOT_EMU_TEXT "Arnold"

typedef struct
{
	char	SnapshotID[8];
	char	pad0[8];
	char	Version;
	char	S_Z80_F;
	char	S_Z80_A;
	char	S_Z80_C;
	char	S_Z80_B;
	char	S_Z80_E;
	char	S_Z80_D;
	char	S_Z80_L;
	char	S_Z80_H;
	char	S_Z80_R;
	char	S_Z80_I;
	char	S_Z80_IFF0;
	char	S_Z80_IFF1;
	char	S_Z80_LIX;
	char	S_Z80_HIX;
	char	S_Z80_LIY;
	char	S_Z80_HIY;
	char	S_Z80_LSP;
	char	S_Z80_HSP;
	char	S_Z80_LPC;
	char	S_Z80_HPC;
	char	S_Z80_IM;
	char	S_Z80_F_alt;
	char	S_Z80_A_alt;
	char	S_Z80_C_alt;
	char	S_Z80_B_alt;
	char	S_Z80_E_alt;
	char	S_Z80_D_alt;
	char	S_Z80_L_alt;
	char	S_Z80_H_alt;

	char	GateArray_Pen;
	char	GateArray_Inks[17];

	char	GateArray_RomConfig;
	char	DkTronicsRamConfig; /* ram config for dk'tronics compatible ram */

	char	CRTC_Register;
	char	CRTC_Registers[18];

	char	ROM_UpperRom;

	char	PIO_PortA,PIO_PortB,PIO_PortC, PIO_Control;

	char	PSG_Register;
	char	PSG_Registers[16];

	char	MemSizeLow;
	char	MemSizeHigh;

	char	CPCType;

	char	InterruptBlock;

	char	MultiMode[6];

	char	pad1[0x0f0-0x075];
	char	EmuIdent[0x010];
} SNAPSHOT_HEADER;

#define MAX_SNAPSHOT_EXPORT_BLOCKS (32+4) // 4 base ram, 32 in expansion ram
#define MAX_SNAPSHOT_BANKS (MAX_SNAPSHOT_EXPORT_BLOCKS/4)

typedef	struct
{
	/* Version of snapshot, either 2 or 3 */
	/* do not bother with version 1 */
	int Version;

	/* TRUE for compressed memory blocks - only version 3 */
	/* FALSE otherwise */
	BOOL bCompressed;

	/* winape allows you to choose 64k, 128k or custom */
	/* custom not allowed on type 2,1 */
	/* automatic, 256k expansion, silicon disc, all extra banks */
	/* banks c4-c7, d4-d7 etc all the way up to fc-ff */
	/* allows cartridge/roms, disc in drive, breakpoints and data areas */
	BOOL bBlocksToExport[MAX_SNAPSHOT_EXPORT_BLOCKS];
} SNAPSHOT_OPTIONS;

#define SNAPSHOT_SOURCE_UNMAPPED_ID -2
#define SNAPSHOT_SOURCE_INTERNAL_ID -1

typedef struct
{
	int nSourceId;	
	BOOL bAvailable; // true if it is providing the data
	unsigned char *pPtr; // pointer to the data
} SNAPSHOT_MEMORY_BLOCK;

typedef struct
{
	SNAPSHOT_MEMORY_BLOCK Blocks[MAX_SNAPSHOT_EXPORT_BLOCKS];
	int nMaxBlockV2; /* for v2, this is the size required */
	int nMaxBlockMainV3; /* v3 uncompressed this is the size of the main block */
	int nStart64KBBlockV3; /* v3 - this is the index of the first block written in extra blocks */
	BOOL QualifyingBlocks[MAX_SNAPSHOT_EXPORT_BLOCKS/4]; /* which blocks qualify for exporting; version based */

} SNAPSHOT_MEMORY_BLOCKS;

typedef enum
{
	SNAPSHOT_SIZE_64KB_ONLY = 4,
	SNAPSHOT_SIZE_128KB_ONLY = 8,
	SNAPSHOT_SIZE_256KB_ONLY = 16+4,
	SNAPSHOT_SIZE_512KB_ONLY = 32+4
} SNAPSHOT_SIZE;

/* What does the snapshot require? Will we be able to actually fulfill those requirements or
should the user either 1) activate the features or 2) automatically configure */
typedef struct
{
	int Version;

	int HardwareRequired; /* CPC, KC Compact, Aleste, Plus */

	BOOL bSecond64KB; /* for CPC6128, 6128+ which have 128KB prefer built in RAM if requested */

	BOOL BlocksRequested[MAX_SNAPSHOT_EXPORT_BLOCKS]; /* Dk'tronics memory blocks requested */

	unsigned int UnsupportedChunkCount;
	unsigned int UnsupportedChunks[128];

	BOOL UnsupportedRamSize;
	int RamSizeRequested;
} SNAPSHOT_REQUIREMENTS;

SNAPSHOT_OPTIONS *SnapshotSettings_GetDefault(void);

void SnapshotSettings_Init(SNAPSHOT_OPTIONS *pOptions);

BOOL Snapshot_CanSaveAccurately(void);

/* configure hardware based on the requirements of the loaded snapshot and the existing configuration */
BOOL Snapshot_ConfigureHardware(const SNAPSHOT_REQUIREMENTS *pRequirements, SNAPSHOT_MEMORY_BLOCKS *pMemoryBlocks);

/* TRUE if the requirements of the loaded snapshot match the current configuration */
BOOL Snapshot_AreRequirementsMet(const SNAPSHOT_REQUIREMENTS *pRequirements, const SNAPSHOT_MEMORY_BLOCKS *pMemoryBlocks);

void SnapshotSettings_SetVersion(SNAPSHOT_OPTIONS *pOptions, int nVersion);
int SnapshotSettings_GetVersion(const SNAPSHOT_OPTIONS *pOptions);

/* if snapshot is valid, get it's requirements */
BOOL Snapshot_IsValid(const unsigned char *pSnapshot, const unsigned long SnapshotLength, SNAPSHOT_REQUIREMENTS *pRequirements);

void SnapshotSettings_SetSizeToExport(SNAPSHOT_OPTIONS *pOptions, SNAPSHOT_SIZE nSize);

BOOL SnapshotSettings_GetCompressed(const SNAPSHOT_OPTIONS *pOptions);
void SnapshotSettings_SetCompressed(SNAPSHOT_OPTIONS *pOptions, BOOL bState);

BOOL SnapshotSettings_GetBlockForExport(const SNAPSHOT_OPTIONS *pOptions, int nBlock);
void SnapshotSettings_SetBlockForExport(SNAPSHOT_OPTIONS *pOptions, int nBlock, BOOL bState);

void Snapshot_CollectMemory(SNAPSHOT_MEMORY_BLOCKS *pSnapshotMemoryBlocks, const SNAPSHOT_OPTIONS *pOptions, BOOL bReading);

int Snapshot_Insert(const unsigned char *, const unsigned long, const SNAPSHOT_MEMORY_BLOCKS *pMemoryBlocks);

/* calculates an estimated output file size. It is estimated because the final version
may be compressed */
unsigned long Snapshot_CalculateEstimatedOutputSize(const SNAPSHOT_OPTIONS *pOptions, const SNAPSHOT_MEMORY_BLOCKS *pMemoryBlocks);

size_t Snapshot_GenerateOutputData(unsigned char *pBuffer, const SNAPSHOT_OPTIONS *pOtions, const SNAPSHOT_MEMORY_BLOCKS *pMemoryBlocks);

#endif
