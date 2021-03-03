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
 /********************/
 /* snapshot support */
 /********************/

 /* The data common to snapshots 1,2,3 are in CPCEMU compatible form: */
 /* - ink values are stored with bits 5,6,7 = 0 */
 /* - PPI Control is the I/O state and mode of the ports and not the last value written to this port */
 /* - PPI port C is  the last value written to this port */
 /* - PPI port B is the inputs to this port */
 /* - PPI port A is the inputs to this port */
 /* - ram configuration does not have bit 7 and bit 6 set to 1 */

 /* Version 3 has the additional chunks as written by No$CPC v1.8 */
 /*
  TODO: Fix for all computers
  TODO: Fix uncompressed V3 causes crash
  */
#include "headers.h"

#include "snapshot.h"
#include "cpc.h"
#include "snapv3.h"
#include "fdi.h"
#include "i8255.h"
#include "psg.h"
#include "crtc.h"
#include "asic.h"
#include "kcc.h"
#include "printer.h"
#include "garray.h"
#include "aleste.h"
#include "emudevice.h"

#ifndef max
#define max(a,b) (a>b) ? a : b
#endif

#ifndef min
#define min(a,b) (a<b) ? a : b
#endif
extern AY_3_8912 OnBoardAY;

/* get a register pair from memory */
#define SNAPSHOT_GET_REGISTER_PAIR(ptr,offset) \
	((ptr[offset+1] & 0x0ff)<<8) | (ptr[offset+0] & 0x0ff);

/* get a register from memory */
#define SNAPSHOT_GET_REGISTER(ptr,offset) \
	(ptr[offset] & 0x0ff)

/* put a register pair to memory */
#define SNAPSHOT_PUT_REGISTER_PAIR(ptr,offset,data) \
	ptr[offset+0] = data & 0x0ff; \
	ptr[offset+1] = (data>>8) & 0x0ff

/* put a register to memory */
#define SNAPSHOT_PUT_REGISTER(ptr,offset,data) \
	ptr[offset+0] = data & 0x0ff

#define SNAPSHOT_CRTC_FLAGS_HSYNC_STATE (1<<0)
#define SNAPSHOT_CRTC_FLAGS_VSYNC_STATE (1<<1)
#define SNAPSHOT_CRTC_FLAGS_VADJ_STATE (1<<7)

static SNAPSHOT_OPTIONS DefaultOptions;

void SnapshotSettings_Init(SNAPSHOT_OPTIONS *pOptions)
{
	int i;
	SnapshotSettings_SetVersion(pOptions, 3);
	SnapshotSettings_SetCompressed(pOptions, TRUE);
	for (i = 0; i < MAX_SNAPSHOT_EXPORT_BLOCKS; i++)
	{
		pOptions->bBlocksToExport[i] = TRUE;
	}
}

SNAPSHOT_OPTIONS *SnapshotSettings_GetDefault(void)
{
	return &DefaultOptions;
}

void SnapshotSettings_SetVersion(SNAPSHOT_OPTIONS *pOptions, int nVersion)
{
	pOptions->Version = nVersion;
}

int SnapshotSettings_GetVersion(const SNAPSHOT_OPTIONS *pOptions)
{
	return pOptions->Version;
}

BOOL SnapshotSettings_GetCompressed(const SNAPSHOT_OPTIONS *pOptions)
{
	return pOptions->bCompressed;
}

void SnapshotSettings_SetCompressed(SNAPSHOT_OPTIONS *pOptions, BOOL bState)
{
	pOptions->bCompressed = bState;
}

void SnapshotSettings_SetSizeToExport(SNAPSHOT_OPTIONS *pOptions, SNAPSHOT_SIZE nSize)
{
	int i;
	for (i = 0; i <MAX_SNAPSHOT_EXPORT_BLOCKS; i++)
	{
		pOptions->bBlocksToExport[i] = i < nSize ? TRUE : FALSE;
	}
}


void SnapshotSettings_SetBlockForExport(SNAPSHOT_OPTIONS *pOptions,int nBlock, BOOL bState)
{
	if ((nBlock < 0) || (nBlock >= (MAX_SNAPSHOT_EXPORT_BLOCKS)))
		return;

	pOptions->bBlocksToExport[nBlock] = bState;
}

BOOL SnapshotSettings_GetBlockForExport(const SNAPSHOT_OPTIONS *pOptions, int nBlock)
{
	if ((nBlock < 0) || (nBlock >= (MAX_SNAPSHOT_EXPORT_BLOCKS)))
		return FALSE;

	return pOptions->bBlocksToExport[nBlock];
}

void Snapshot_CollectMemory(SNAPSHOT_MEMORY_BLOCKS *pSnapshotMemoryBlocks, const SNAPSHOT_OPTIONS *pOptions, BOOL bReading)
{
	int s;
	int i;

	for (i = 0; i < MAX_SNAPSHOT_EXPORT_BLOCKS; i++)
	{
		SNAPSHOT_MEMORY_BLOCK *pBlock = &pSnapshotMemoryBlocks->Blocks[i];
		pBlock->nSourceId = SNAPSHOT_SOURCE_UNMAPPED_ID;
		pBlock->bAvailable = FALSE;
		pBlock->pPtr = NULL;

	}

	for (i = 0; i < MAX_SNAPSHOT_EXPORT_BLOCKS / 4; i++)
	{
		pSnapshotMemoryBlocks->QualifyingBlocks[i] = FALSE;
	}

	/* fill with default ram from computer */
	if (CPC_GetHardware() == CPC_HW_ALESTE)
	{
		Aleste_FillSnapshotMemoryBlocks(pSnapshotMemoryBlocks, pOptions,bReading);
	}
	else if (CPC_GetHardware() == CPC_HW_KCCOMPACT)
	{
		KCC_FillSnapshotMemoryBlocks(pSnapshotMemoryBlocks, pOptions, bReading);
	}
	else if (CPC_GetHardware() == CPC_HW_CPC)
	{
		CPC_FillSnapshotMemoryBlocks(pSnapshotMemoryBlocks, pOptions, bReading);
	}
	else if (CPC_GetHardware() == CPC_HW_CPCPLUS)
	{
		Plus_FillSnapshotMemoryBlocks(pSnapshotMemoryBlocks, pOptions, bReading);
	}

	/* override with devices */

	/* loop over all dk'tronics ram selections */
	for (s = 0; s < 32; s++)
	{
		SNAPSHOT_MEMORY_BLOCK *pBlock = &pSnapshotMemoryBlocks->Blocks[s + 4];


		if (bReading || pOptions->bBlocksToExport[s+4])
		{
			/* TODO: Which is going to respond first? Can we do this by order from CPC? */
			/* loop over all active devices */
			for (i = 0; i < EmuDevice_GetNumEnabled(); i++)
			{
				/* get id of enabled device */
				int nDeviceId = EmuDevice_GetEnabledDeviceId(i);

				/* does this device respond to this selection? */
				/* device may have mirrors of this data, so this is a
				report of the base value not mirror */
				if (EmuDevice_RespondsToDkRamSelection(nDeviceId, s))
				{
					/* this block is available */
					pBlock->nSourceId = nDeviceId;
					pBlock->bAvailable = TRUE;
					pBlock->pPtr = EmuDevice_GetDkRamSelection(nDeviceId, s);

					printf("Device %d responds to selection %d\n", nDeviceId, s);
					break;
				}
			}
		}
		else
		{
			printf("Block %d not selected for export\n", s);
		}
	}

	{
		int Configurations[3] = { 32+4,16+4,8 };

		/* type 2 must be continuous AND it seems limited to
		64KB, 128KB, 256KB and 512KB */

		pSnapshotMemoryBlocks->nMaxBlockV2 = 4; /* base 64kb */

		for (i=0; i<sizeof(Configurations)/sizeof(Configurations[0]); i++)
		{
			int nMax = Configurations[i];

			BOOL bAllExpected = TRUE;

			for (s = 0; s < nMax; s++)
			{
				if ((!bReading && !pOptions->bBlocksToExport[s]) || (!pSnapshotMemoryBlocks->Blocks[s].bAvailable))
				{
					bAllExpected = FALSE;
					break;
				}
			}

			if (bAllExpected)
			{
				pSnapshotMemoryBlocks->nMaxBlockV2 = nMax;
				break;
			}
		}

		if (pOptions->Version == 3)
		{
			if (pOptions->bCompressed)
			{
				pSnapshotMemoryBlocks->nMaxBlockMainV3 = 0;
				pSnapshotMemoryBlocks->nStart64KBBlockV3 = 0;
			}
			else
			{
				pSnapshotMemoryBlocks->nMaxBlockMainV3 = 4;
				pSnapshotMemoryBlocks->nStart64KBBlockV3 = 1;
			}
		}
	}

	{
		if (pOptions->Version == 2)
		{
			for (i = 0; i < pSnapshotMemoryBlocks->nMaxBlockV2 / 4; i++)
			{
				pSnapshotMemoryBlocks->QualifyingBlocks[i] = TRUE;
			}
		}
		else
		{
			for (i = 0; i < MAX_SNAPSHOT_EXPORT_BLOCKS; i += 4)
			{
				/* need 64KB continuous */
				if (
					(pSnapshotMemoryBlocks->Blocks[i + 0].bAvailable) &&
					(pSnapshotMemoryBlocks->Blocks[i + 1].bAvailable) &&
					(pSnapshotMemoryBlocks->Blocks[i + 2].bAvailable) &&
					(pSnapshotMemoryBlocks->Blocks[i + 3].bAvailable) 
					)
				{
					pSnapshotMemoryBlocks->QualifyingBlocks[i/4] = TRUE;
				}
			}
		}
	}
}

BOOL Snapshot_AreRequirementsMet(const SNAPSHOT_REQUIREMENTS *pRequirements, const SNAPSHOT_MEMORY_BLOCKS *pMemoryBlocks)
{
	int i;

	/* check hardware matches */
	if (CPC_GetHardware() != pRequirements->HardwareRequired)
		return FALSE;

	/* this allows more memory to be allowed */
	for (i = 0; i < MAX_SNAPSHOT_EXPORT_BLOCKS; i++)
	{
		if (pRequirements->BlocksRequested[i] && !pMemoryBlocks->Blocks[i].bAvailable)
		{
			return FALSE;
		}
	}

	if (pRequirements->UnsupportedRamSize)
	{
		return FALSE;
	}

	/* warn */
	if (pRequirements->UnsupportedChunkCount != 0)
		return FALSE;

	return TRUE;
}

BOOL Snapshot_CanSaveAccurately(void)
{
	/* TODO: More testing. Add more here */
	int i;
	if ((CPC_GetHardware() == CPC_HW_ALESTE) || (CPC_GetHardware() == CPC_HW_KCCOMPACT))
	{
		return FALSE;
	}
	/* iterate through all devices */
	for (i = 0; i < EmuDevice_GetNumDevices(); i++)
	{
		/* device is enabled */
		if (EmuDevice_IsEnabled(i))
		{
			/* device only responds to dk'tronics ram expansion? */
			if (!((EmuDevice_GetFlags(i) & DEVICE_FLAGS_HAS_DKTRONICS_RAM)==DEVICE_FLAGS_HAS_DKTRONICS_RAM))
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

BOOL Snapshot_ConfigureHardware(const SNAPSHOT_REQUIREMENTS *pRequirements, SNAPSHOT_MEMORY_BLOCKS *pMemoryBlocks)
{
	int i;
	int s;

	switch (pRequirements->HardwareRequired)
	{
	case CPC_HW_CPC:
	{
		CPC_SetHardware(CPC_HW_CPC);
	}
	break;

	case CPC_HW_CPCPLUS:
	{
		CPC_SetHardware(CPC_HW_CPCPLUS);

		/* select a computer rather than a console */
		ASIC_SetGX4000(FALSE);

		/* if second bank is required, enable on-board extra 64KB ram */
		if (pRequirements->bSecond64KB)
		{
			ASIC_SetR128(TRUE);
		}

	}
	break;

	default:
		break;
	}
	/* TODO: choose a device that will respond to the most blocks? */
	for (s = 0; s < 32; s++)
	{
		/* requesting a block that is not available */
		if (pRequirements->BlocksRequested[s + 4] && !pMemoryBlocks->Blocks[s + 4].bAvailable)
		{
			/* iterate through all devices */
			for (i = 0; i < EmuDevice_GetNumDevices(); i++)
			{
				/* device is not already enabled */
				if (!EmuDevice_IsEnabled(i))
				{
					/* responds to this selection? */
					if (EmuDevice_RespondsToDkRamSelection(i, s))
					{
						int p;

						/* activate the device */
						EmuDevice_Enable(i, TRUE);

						/* re-mark all the available blocks now */
						for (p = 0; p < 32; p++)
						{
							if (EmuDevice_RespondsToDkRamSelection(i, p))
							{
								if (!pMemoryBlocks->Blocks[p + 4].bAvailable)
								{
									pMemoryBlocks->Blocks[p + 4].bAvailable = TRUE;
									pMemoryBlocks->Blocks[p + 4].nSourceId = i;
									pMemoryBlocks->Blocks[p + 4].pPtr = EmuDevice_GetDkRamSelection(i,p);
								}
							}
						}
					}
				}
			}
		}
	}
	return TRUE;
}

int Snapshot_IsValid(const unsigned char *pSnapshot, const unsigned long SnapshotLength, SNAPSHOT_REQUIREMENTS *pRequirements)
{
	int Status = ARNOLD_STATUS_UNRECOGNISED;

	SNAPSHOT_HEADER *pSnapshotHeader;
	
	pRequirements->Version = -1;
	pRequirements->HardwareRequired = CPC_HW_CPC;
	pRequirements->UnsupportedChunkCount = 0;
	pRequirements->UnsupportedRamSize = FALSE;
	pRequirements->bSecond64KB = FALSE;
	memset(pRequirements->BlocksRequested, 0, sizeof(pRequirements->BlocksRequested));

	/* failed so quit */
	if (pSnapshot != NULL)
	{
		pSnapshotHeader = (SNAPSHOT_HEADER *)pSnapshot;

		/* snapshot length must be at least the size of snapshot header */
		if (SnapshotLength > sizeof(SNAPSHOT_HEADER))
		{
			/* check version */
			if ((pSnapshotHeader->Version >= 1) && (pSnapshotHeader->Version <= 3))
			{
				/* 32 * 16KB expansion blocks + 4 main ram 16KB blocks */

				/* blocks requested by snapshot */

				pRequirements->Version = pSnapshotHeader->Version;

				/* header present */
				printf("Snapshot version: %d\n", pSnapshotHeader->Version);

				/* check version */
				if ((pSnapshotHeader->Version >= 1) && (pSnapshotHeader->Version <= 3))
				{
					int i;

					/* size of actual data in snapshot */
					int DataSizeInSnapshot = SnapshotLength - sizeof(SNAPSHOT_HEADER);

					/* mem size claimed to be in snapshot */
					int MemSizeInSnapshot = ((pSnapshotHeader->MemSizeLow & 0x0ff) | ((pSnapshotHeader->MemSizeHigh & 0x0ff) << 8)) << 10;

					if (pSnapshotHeader->Version >= 3)
					{
						int nBlocks = (MemSizeInSnapshot + 65535) / 65536;

						pRequirements->RamSizeRequested = (nBlocks * 65536);
						if (nBlocks > MAX_SNAPSHOT_BANKS)
						{
							pRequirements->UnsupportedRamSize = TRUE;
							nBlocks = MAX_SNAPSHOT_BANKS;
						}
						for (i = 0; i < nBlocks; i++)
						{
							pRequirements->BlocksRequested[(i * 4) + 0] = TRUE;
							pRequirements->BlocksRequested[(i * 4) + 1] = TRUE;
							pRequirements->BlocksRequested[(i * 4) + 2] = TRUE;
							pRequirements->BlocksRequested[(i * 4) + 3] = TRUE;
						}

						if (MemSizeInSnapshot < DataSizeInSnapshot)
						{
							/* must be at least enough data for a chunk header after the memory dump */
							if (SnapshotLength > (MemSizeInSnapshot + sizeof(SNAPSHOT_HEADER) + sizeof(RIFF_CHUNK)))
							{
								/* some chunks after the data */
								const RIFF_CHUNK *pChunk = (const RIFF_CHUNK *)((const char *)pSnapshotHeader + sizeof(SNAPSHOT_HEADER) + MemSizeInSnapshot);
								unsigned long SizeRemaining;

								SizeRemaining = SnapshotLength - MemSizeInSnapshot - sizeof(SNAPSHOT_HEADER);

								while (SizeRemaining != 0)
								{
									int ChunkSize;
									unsigned long ChunkName = Riff_GetChunkName(pChunk);

									/* get length of this chunk's data in bytes */
									ChunkSize = Riff_GetChunkLength(pChunk);

									if (ChunkSize == 0)
										break;

									/* chunk size bad? */
									if (ChunkSize > (SizeRemaining - sizeof(RIFF_CHUNK)))
									{
										/* size of chunk is greater than size of data remaining */
										Status = ARNOLD_STATUS_INVALID_LENGTH;
										break;
									}


									switch (ChunkName)
									{
									case RIFF_FOURCC_CODE('C', 'P', 'C', '+'):
									{
										pRequirements->HardwareRequired = CPC_HW_CPCPLUS;
									}
									break;

									case RIFF_FOURCC_CODE('M', 'E', 'M', '0'):
									case RIFF_FOURCC_CODE('M', 'E', 'M', '1'):
									case RIFF_FOURCC_CODE('M', 'E', 'M', '2'):
									case RIFF_FOURCC_CODE('M', 'E', 'M', '3'):
									case RIFF_FOURCC_CODE('M', 'E', 'M', '4'):
									case RIFF_FOURCC_CODE('M', 'E', 'M', '5'):
									case RIFF_FOURCC_CODE('M', 'E', 'M', '6'):
									case RIFF_FOURCC_CODE('M', 'E', 'M', '7'):
									case RIFF_FOURCC_CODE('M', 'E', 'M', '8'):
									{
										unsigned long MemBank = (ChunkName >> 24) & 0x0ff;
										int Bank = MemBank - '0';

										if (pRequirements->BlocksRequested[(Bank << 2) + 0] == FALSE)
										{
											pRequirements->RamSizeRequested += 65536;
										}



										pRequirements->BlocksRequested[(Bank << 2) + 0] = TRUE;
										pRequirements->BlocksRequested[(Bank << 2) + 1] = TRUE;
										pRequirements->BlocksRequested[(Bank << 2) + 2] = TRUE;
										pRequirements->BlocksRequested[(Bank << 2) + 3] = TRUE;
									}
									break;

									default:
									{
										BOOL bFound = FALSE;
										for (i = 0; i < pRequirements->UnsupportedChunkCount; i++)
										{
											if (pRequirements->UnsupportedChunks[i] == ChunkName)
											{
												bFound = TRUE;
												break;
											}
										}

										if (!bFound)
										{
											int nIndex = pRequirements->UnsupportedChunkCount;
											if (nIndex <= sizeof(pRequirements->UnsupportedChunks) / sizeof(pRequirements->UnsupportedChunks[0]))
											{
												pRequirements->UnsupportedChunks[pRequirements->UnsupportedChunkCount] = ChunkName;
												pRequirements->UnsupportedChunkCount++;
											}
										}
									}
									break;
									}
									/* update chunk pointer */
									pChunk = (RIFF_CHUNK *)((unsigned char *)pChunk + ChunkSize + sizeof(RIFF_CHUNK));

									/* update size of data remaining */
									SizeRemaining -= (ChunkSize + sizeof(RIFF_CHUNK));
								}
							}
						}
					}
					else
					{
						/* we will use the larger value to determine the memory expansion we will enable */
						int MemSizeMax = max(DataSizeInSnapshot, MemSizeInSnapshot);

						int nBlocks = (MemSizeMax + 65535) / 65536;

						pRequirements->RamSizeRequested = (nBlocks * 65536);
						if (nBlocks > MAX_SNAPSHOT_BANKS)
						{
							pRequirements->UnsupportedRamSize = TRUE;
							nBlocks = MAX_SNAPSHOT_BANKS;
						}
						for (i = 0; i < nBlocks; i++)
						{
							pRequirements->BlocksRequested[(i * 4) + 0] = TRUE;
							pRequirements->BlocksRequested[(i * 4) + 1] = TRUE;
							pRequirements->BlocksRequested[(i * 4) + 2] = TRUE;
							pRequirements->BlocksRequested[(i * 4) + 3] = TRUE;
						}
					}

					printf("Snapshot is requesting blocks: ");
					for (i = 0; i < MAX_SNAPSHOT_EXPORT_BLOCKS; i++)
					{
						printf("%d ", i);
					}
					printf("\n");

					pRequirements->bSecond64KB = FALSE;
					if (pRequirements->BlocksRequested[4] &&
						pRequirements->BlocksRequested[5] &&
						pRequirements->BlocksRequested[6] &&
						pRequirements->BlocksRequested[7])
					{
						pRequirements->bSecond64KB = TRUE;
					}

					Status = ARNOLD_STATUS_OK;

				}
			}
		}
	}

	return Status;
}


/* insert snapshot */
int Snapshot_Insert(const unsigned char *pSnapshot, const unsigned long SnapshotLength, const SNAPSHOT_MEMORY_BLOCKS *pSnapshotMemoryBlocks)
{
	int Status = ARNOLD_STATUS_UNRECOGNISED;
	SNAPSHOT_HEADER *pSnapshotHeader;

	/* failed so quit */
	if (pSnapshot != NULL)
	{
		pSnapshotHeader = (SNAPSHOT_HEADER *)pSnapshot;

		/* snapshot length must be at least the size of snapshot header */
		if (SnapshotLength > sizeof(SNAPSHOT_HEADER))
		{
			/* is snapshot header text present? */
			if (memcmp(pSnapshotHeader, SNAPSHOT_HEADER_TEXT, 8) == 0)
			{
				/* 32 * 16KB expansion blocks + 4 main ram 16KB blocks */

				/* blocks requested by snapshot */
				BOOL BlocksRequested[MAX_SNAPSHOT_EXPORT_BLOCKS];

				memset(BlocksRequested, 0, sizeof(BlocksRequested));

				/* header present */
				printf("Snapshot version: %d\n", pSnapshotHeader->Version);

				/* check version */
				if ((pSnapshotHeader->Version >= 1) && (pSnapshotHeader->Version <= 3))
				{
					char    *pSnapshotData;
					char	*pSnapshotDataPtr;
					int i;

					/* size of actual data in snapshot */
					int DataSizeInSnapshot = SnapshotLength - sizeof(SNAPSHOT_HEADER);

					/* mem size claimed to be in snapshot */
					int MemSizeInSnapshot = ((pSnapshotHeader->MemSizeLow & 0x0ff) | ((pSnapshotHeader->MemSizeHigh & 0x0ff) << 8)) << 10;

					int UseableMemSizeInSnapshot = MemSizeInSnapshot;

					if (pSnapshotHeader->Version >= 3)
					{
						int nBlocks = (MemSizeInSnapshot + 65535) / 65536;
						if (nBlocks > MAX_SNAPSHOT_BANKS)
						{
							nBlocks = MAX_SNAPSHOT_BANKS;
						}
						UseableMemSizeInSnapshot = nBlocks * 65536;
						for (i = 0; i < nBlocks; i++)
						{
							BlocksRequested[(i * 4) + 0] = TRUE;
							BlocksRequested[(i * 4) + 1] = TRUE;
							BlocksRequested[(i * 4) + 2] = TRUE;
							BlocksRequested[(i * 4) + 3] = TRUE;
						}

						if (MemSizeInSnapshot < DataSizeInSnapshot)
						{
							/* must be at least enough data for a chunk header after the memory dump */
							if (SnapshotLength > (MemSizeInSnapshot + sizeof(SNAPSHOT_HEADER) + sizeof(RIFF_CHUNK)))
							{
								/* some chunks after the data */
								RIFF_CHUNK *pChunk = (RIFF_CHUNK *)((char *)pSnapshotHeader + sizeof(SNAPSHOT_HEADER) + MemSizeInSnapshot);
								unsigned long SizeRemaining;

								SizeRemaining = SnapshotLength - MemSizeInSnapshot - sizeof(SNAPSHOT_HEADER);

								while (SizeRemaining != 0)
								{
									int ChunkSize;
									unsigned long ChunkName = Riff_GetChunkName(pChunk);

									/* get length of this chunk's data in bytes */
									ChunkSize = Riff_GetChunkLength(pChunk);

									if (ChunkSize == 0)
										break;

									/* chunk size bad? */
									if (ChunkSize > (SizeRemaining - sizeof(RIFF_CHUNK)))
									{
										/* size of chunk is greater than size of data remaining */
										Status = FALSE;
										break;
									}


									switch (ChunkName)
									{
										case RIFF_FOURCC_CODE('M','E','M','0'):
										case RIFF_FOURCC_CODE('M','E','M','1'):										
										case RIFF_FOURCC_CODE('M','E','M','2'):
										case RIFF_FOURCC_CODE('M','E','M','3'):
										case RIFF_FOURCC_CODE('M','E','M','4'):
										case RIFF_FOURCC_CODE('M','E','M','5'):
										case RIFF_FOURCC_CODE('M','E','M','6'):
										case RIFF_FOURCC_CODE('M','E','M','7'):
										case RIFF_FOURCC_CODE('M', 'E', 'M', '8'):
										{
											unsigned long MemBank = (ChunkName >> 24) & 0x0ff;
											int Bank = MemBank - '0';

											BlocksRequested[(Bank<<2) + 0] = TRUE;
											BlocksRequested[(Bank << 2) + 1] = TRUE;
											BlocksRequested[(Bank << 2) + 2] = TRUE;
											BlocksRequested[(Bank << 2) + 3] = TRUE;
										}
										break;

										default:
											break;
									}

									/* update chunk pointer */
									pChunk = (RIFF_CHUNK *)((unsigned char *)pChunk + ChunkSize + sizeof(RIFF_CHUNK));

									/* update size of data remaining */
									SizeRemaining -= (ChunkSize + sizeof(RIFF_CHUNK));
								}
							}
						}
					}
					else
					{
						/* we will use the larger value to determine the memory expansion we will enable */
						int MemSizeMax = max(DataSizeInSnapshot, MemSizeInSnapshot);

						int nBlocks = (MemSizeMax + 65535) / 65536;
						if (nBlocks > MAX_SNAPSHOT_BANKS)
						{
							nBlocks = MAX_SNAPSHOT_BANKS;
						}
						UseableMemSizeInSnapshot = nBlocks * 65536;
						for (i = 0; i < nBlocks; i++)
						{
							BlocksRequested[(i * 4) + 0] = TRUE;
							BlocksRequested[(i * 4) + 1] = TRUE;
							BlocksRequested[(i * 4) + 2] = TRUE;
							BlocksRequested[(i * 4) + 3] = TRUE;
						}
					}

					printf("Snapshot is requesting blocks: ");
					for (i = 0; i < MAX_SNAPSHOT_EXPORT_BLOCKS; i++)
					{
						printf("%d ",i);
					}
					printf("\n");

					printf("Blocks available: ");
					for (i = 0; i < MAX_SNAPSHOT_EXPORT_BLOCKS; i++)
					{
						if (pSnapshotMemoryBlocks->Blocks[i].bAvailable)
						{
							printf("%d ", i);
						}
					}
					printf("\n");


					/* file must be at least the size of the header */
					if (SnapshotLength >= sizeof(SNAPSHOT_HEADER))
					{
						/* Appears snapshot is valid. Setup Z80 and copy memory */

						unsigned long CopySize;
						int Register;

						/**** INIT Z80 ****/
						/* setup Z80 */
						char *pRegs = (char *)pSnapshotHeader + 0x011;

						printf("Performing restart\n");
						/* reset CPC */
						Computer_RestartReset();

						/* CPC Type is ignored because it's not reliable */

						Register = SNAPSHOT_GET_REGISTER_PAIR(pRegs, 0);
						CPU_SetReg(CPU_AF, Register);
						printf("Snapshot Z80 AF: &%04x\n", Register);

						Register = SNAPSHOT_GET_REGISTER_PAIR(pRegs, 2);
						CPU_SetReg(CPU_BC, Register);
						printf("Snapshot Z80 BC: &%04x\n", Register);


						Register = SNAPSHOT_GET_REGISTER_PAIR(pRegs, 4);
						CPU_SetReg(CPU_DE, Register);
						printf("Snapshot Z80 DE: &%04x\n", Register);

						Register = SNAPSHOT_GET_REGISTER_PAIR(pRegs, 6);
						CPU_SetReg(CPU_HL, Register);
						printf("Snapshot Z80 HL: &%04x\n", Register);


						CPU_SetReg(CPU_R, SNAPSHOT_GET_REGISTER(pRegs, 8));
						printf("Snapshot Z80 R: &%02x\n", SNAPSHOT_GET_REGISTER(pRegs, 8));


						CPU_SetReg(CPU_I, SNAPSHOT_GET_REGISTER(pRegs, 9));
						printf("Snapshot Z80 I: &%02x\n", SNAPSHOT_GET_REGISTER(pRegs, 9));


						CPU_SetReg(CPU_IFF1, pRegs[10] & 1);
						printf("Snapshot Z80 IFF1: &%04x\n", pRegs[10] & 1);


						CPU_SetReg(CPU_IFF2, pRegs[11] & 1);
						printf("Snapshot Z80 IFF2: &%04x\n", pRegs[11] & 1);

						Register = SNAPSHOT_GET_REGISTER_PAIR(pRegs, 12);
						CPU_SetReg(CPU_IX, Register);
						printf("Snapshot Z80 IX: &%04x\n", Register);


						Register = SNAPSHOT_GET_REGISTER_PAIR(pRegs, 14);
						CPU_SetReg(CPU_IY, Register);
						printf("Snapshot Z80 IY: &%04x\n", Register);


						Register = SNAPSHOT_GET_REGISTER_PAIR(pRegs, 16);
						CPU_SetReg(CPU_SP, Register);
						printf("Snapshot Z80 SP: &%04x\n", Register);


						Register = SNAPSHOT_GET_REGISTER_PAIR(pRegs, 18);
						CPU_SetReg(CPU_PC, Register);

						printf("Snapshot Z80 PC: &%04x\n", Register);

						Register = pRegs[20];

						if ((Register < 0) || (Register > 2))
						{
							/* IM 1 is better for CPC */
							Register = 1;
						}
						printf("Snapshot Z80 IM: &%04x\n", Register);

						CPU_SetReg(CPU_IM, Register);

						Register = SNAPSHOT_GET_REGISTER_PAIR(pRegs, 21);
						CPU_SetReg(CPU_AF2, Register);
						printf("Snapshot Z80 AF': &%04x\n", Register);

						Register = SNAPSHOT_GET_REGISTER_PAIR(pRegs, 23);
						CPU_SetReg(CPU_BC2, Register);
						printf("Snapshot Z80 BC': &%04x\n", Register);

						Register = SNAPSHOT_GET_REGISTER_PAIR(pRegs, 25);
						CPU_SetReg(CPU_DE2, Register);
						printf("Snapshot Z80 DE': &%04x\n", Register);

						Register = SNAPSHOT_GET_REGISTER_PAIR(pRegs, 27);
						CPU_SetReg(CPU_HL2, Register);
						printf("Snapshot Z80 HL': &%04x\n", Register);


						switch (CPC_GetHardware())
						{
						case CPC_HW_CPC:
						{
							CPC_LoadFromSnapshot(pSnapshotHeader);
						}
						break;


						case CPC_HW_CPCPLUS:
						{
							Plus_LoadFromSnapshot(pSnapshotHeader);
						}
						break;

						case CPC_HW_KCCOMPACT:
						{
							KCC_LoadFromSnapshot(pSnapshotHeader);

						}
						break;

						case CPC_HW_ALESTE:
						{
							Aleste_LoadFromSnapshot(pSnapshotHeader);
						}
						break;

						default:
						{
						}
						break;

						}

						/* ram configuration select */

						/* iterate through all devices */
						for (i = 0; i < EmuDevice_GetNumDevices(); i++)
						{
							/* device is enabled */
							if (EmuDevice_IsEnabled(i))
							{
								/* device responds to dk'tronics ram expansion? */
								if (EmuDevice_GetFlags(i) & DEVICE_FLAGS_HAS_DKTRONICS_RAM)
								{
									/* restore config */
									EmuDevice_RestoreFromSnapshot(i, (((char *)pSnapshotHeader)[0x041] & 0x03f)|0x0c0);
								}
							}
						}


						/**** CRTC ****/
						/* initialise CRTC */
						for (i = 0; i < 18; i++)
						{
							unsigned char CRTCRegData = ((unsigned char *)pSnapshotHeader)[0x043 + i];

							CRTC_RegisterSelect(i);

							CRTC_WriteData(CRTCRegData & 0x0ff);
							printf("CRTC Register %d Data &%02x\n", i, CRTCRegData);
						}

						/* select CRTC register */
						CRTC_RegisterSelect(((char *)pSnapshotHeader)[0x042]);
						printf("CRTC Selected Register %d\n", ((char *)pSnapshotHeader)[0x042]);

						/**** ROM INDEX ****/
						/* TODO: select expansion rom*/
						//ROM_SetSelectedROMIndex(((char *)pSnapshotHeader)[0x055]);
						printf("Upper ROM %02x - FIXME!!!\n", ((char *)pSnapshotHeader)[0x055]);


						/**** PSG ****/
						/* setup PSG registers */
						for (i = 0; i < 16; i++)
						{
							unsigned char PSGRegData = ((unsigned char *)pSnapshotHeader)[0x05b + i];

							/* !!!! */
							PSG_RegisterSelect(&OnBoardAY, i);

							/* !!!!! */
							PSG_WriteData(&OnBoardAY, PSGRegData);
							printf("On-board AY Register %d Data &%02x\n", i, PSGRegData);
						}

						/* !!!! */
						/* select PSG register */
						PSG_RegisterSelect(&OnBoardAY, ((char *)pSnapshotHeader)[0x05a]);
						printf("On-board AY Register selected %d\n", ((char *)pSnapshotHeader)[0x05a]);

						switch (CPC_GetHardware())
						{
						case CPC_HW_CPCPLUS:
						{
							/**** ASIC PPI ****/

							/* set control */
							ASIC_PPI_WriteControl(((char *)pSnapshotHeader)[0x059]);
							printf("(Plus) PPI Control &%02x\n", ((char *)pSnapshotHeader)[0x059]);

							/* these depend on input/output! */
							/* set port A data */
							ASIC_PPI_SetPortDataFromSnapshot(0, ((char *)pSnapshotHeader)[0x056]);
							printf("(Plus) PPI Port A Data &%02x\n", ((char *)pSnapshotHeader)[0x056]);
							/* set port B data */
							ASIC_PPI_SetPortDataFromSnapshot(1, ((char *)pSnapshotHeader)[0x057]);
							printf("(Plus) PPI Port B Data &%02x\n", ((char *)pSnapshotHeader)[0x057]);
							/* set port C data */
							ASIC_PPI_SetPortDataFromSnapshot(2, ((char *)pSnapshotHeader)[0x058]);
							printf("(Plus) PPI Port C Data &%02x\n", ((char *)pSnapshotHeader)[0x058]);

							PSG_SetBDIRState(&OnBoardAY, ((char *)pSnapshotHeader)[0x058] & (1 << 7));
							PSG_SetBC1State(&OnBoardAY, ((char *)pSnapshotHeader)[0x058] & (1 << 6));
							PSG_RefreshState(&OnBoardAY);

						}
						break;

						default:
						{


							/**** PPI ****/

							/* set control */
							PPI_WriteControl(((char *)pSnapshotHeader)[0x059]);
							printf("PPI Control &%02x\n", ((char *)pSnapshotHeader)[0x059]);

							/* set port A data */
							PPI_SetPortDataFromSnapshot(0, ((char *)pSnapshotHeader)[0x056]);
							printf("PPI Port A data &%02x\n", ((char *)pSnapshotHeader)[0x056]);
							/* set port B data */
							PPI_SetPortDataFromSnapshot(1, ((char *)pSnapshotHeader)[0x057]);
							printf("PPI Port B data &%02x\n", ((char *)pSnapshotHeader)[0x057]);
							/* set port C data */
							PPI_SetPortDataFromSnapshot(2, ((char *)pSnapshotHeader)[0x058]);
							printf("PPI Port C data &%02x\n", ((char *)pSnapshotHeader)[0x058]);

							PSG_SetBDIRState(&OnBoardAY, ((char *)pSnapshotHeader)[0x058] & (1 << 7));
							PSG_SetBC1State(&OnBoardAY, ((char *)pSnapshotHeader)[0x058] & (1 << 6));
							PSG_RefreshState(&OnBoardAY);

						}
						break;
						}

						if (pSnapshotHeader->Version == 3)
						{
							unsigned long CRTC_Flags;
							int CRTCType;
							CRTC_INTERNAL_STATE *pCRTC_State;

							pCRTC_State = CRTC_GetInternalState();

							FDI_SetMotorState(((char *)pSnapshotHeader)[0x09c] & 0x01);
							printf("Disc motor %d\n", ((char *)pSnapshotHeader)[0x09c] & 0x01);

							Printer_Write7BitData(((char *)pSnapshotHeader)[0x0a1]);
							printf("Printer data%d\n", ((char *)pSnapshotHeader)[0x0a1]);
							Printer_SetStrobeState(((((char *)pSnapshotHeader)[0x0a1] & 0x080) != 0));

							CRTCType = ((char *)pSnapshotHeader)[0x0a4];

							if ((CRTCType < 0) || (CRTCType >= 5))
							{
								CRTCType = 0;
							}
							printf("CRTC type %d\n", ((char *)pSnapshotHeader)[0x0a4]);

							CRTC_SetType(CRTCType);

							pCRTC_State->HCount = ((char *)pSnapshotHeader)[0x0a9] & 0x0ff;
							printf("CRTC HC %d\n", ((char *)pSnapshotHeader)[0x0a9] & 0x0ff);
							pCRTC_State->LineCounter = ((char *)pSnapshotHeader)[0x0ab] & 0x07f;
							printf("CRTC LC %d\n", ((char *)pSnapshotHeader)[0x0ab] & 0x07f);
							pCRTC_State->RasterCounter = ((char *)pSnapshotHeader)[0x0ac] & 0x01f;
							printf("CRTC RA %d\n", ((char *)pSnapshotHeader)[0x0ac] & 0x01f);

							CRTC_Flags = (((char *)pSnapshotHeader)[0x0b0] & 0x0ff) | ((((char *)pSnapshotHeader)[0x0b1] & 0x0ff) << 8);

							pCRTC_State->CRTC_Flags &= ~CRTC_VS_FLAG;
							if (CRTC_Flags & SNAPSHOT_CRTC_FLAGS_VSYNC_STATE)
							{
								pCRTC_State->VerticalSyncCount = ((char *)pSnapshotHeader)[0x0af];
								pCRTC_State->CRTC_Flags |= CRTC_VS_FLAG;
							}

							pCRTC_State->CRTC_Flags &= ~CRTC_HS_FLAG;
							if (CRTC_Flags & SNAPSHOT_CRTC_FLAGS_HSYNC_STATE)
							{
								pCRTC_State->HorizontalSyncCount = ((char *)pSnapshotHeader)[0x0ae];
								pCRTC_State->CRTC_Flags |= CRTC_HS_FLAG;
							}

							pCRTC_State->CRTC_Flags &= ~CRTC_VADJ_FLAG;
							if (CRTC_Flags & SNAPSHOT_CRTC_FLAGS_VADJ_STATE)
							{
								pCRTC_State->VertAdjustCount = ((char *)pSnapshotHeader)[0x0ad];
								pCRTC_State->CRTC_Flags |= CRTC_VADJ_FLAG;
							}

							switch (CPC_GetHardware())
							{

							case CPC_HW_CPC:
							{
								GateArray_SetInterruptLineCount(((char *)pSnapshotHeader)[0x0b3]);
								printf("GA ILC %d\n", ((char *)pSnapshotHeader)[0x0b3]);


								// TODO:

								//           GateArray_SetVsyncSynchronisationCount(((char *)pSnapshotHeader)[0x0b2]);
								//printf("GA VSYNC SYNC COUNT %d\n", ((char *)pSnapshotHeader)[0x0b2]);
							}
							break;

							case CPC_HW_CPCPLUS:
							{
								ASIC_GateArray_SetInterruptLineCount(((char *)pSnapshotHeader)[0x0b3]);
								printf("ASIC ILC %d\n", ((char *)pSnapshotHeader)[0x0b3]);
								// TODO:

								//ASIC_GateArray_SetVsyncSynchronisationCount( ( (char *) pSnapshotHeader )[ 0x0b2 ] );
								//printf("ASIC VSYNC SYNC COUNT %d\n", ((char *)pSnapshotHeader)[0x0b2]);
							}
							break;

							case CPC_HW_KCCOMPACT:
							{
								printf("(KCC) Interrupt line count not set! FIXME!\n");

							}
							break;

							case CPC_HW_ALESTE:
							{
								printf("(Aleste) Interrupt line count not set! FIXME!\n");



							}
							break;

							default:
							{
								printf("Interrupt line count not set! FIXME!\n");

							}
							break;

							}

							printf("Z80 IRQ input %d\n", (((char *)pSnapshotHeader)[0x0b4] & 0x01));

							CPU_SetINTState((((char *)pSnapshotHeader)[0x0b4] & 0x01));
						}


						/**** INIT RAM ****/
						pSnapshotData = ((char *)pSnapshotHeader + sizeof(SNAPSHOT_HEADER));

						/* size of main ram to copy. */
						CopySize = min((SnapshotLength - sizeof(SNAPSHOT_HEADER)), UseableMemSizeInSnapshot);

						{
							int nBlocks = (CopySize + 16383) / 16384;
							int SizeRemaining = CopySize;

							pSnapshotDataPtr = pSnapshotData;
							for (i = 0; i < nBlocks; i++)
							{
								if (SizeRemaining != 0)
								{
									CopySize = min(SizeRemaining, 16384);
									if (pSnapshotMemoryBlocks->Blocks[i].pPtr)
									{
										memcpy(pSnapshotMemoryBlocks->Blocks[i].pPtr, pSnapshotDataPtr, CopySize);
									}
									pSnapshotDataPtr += CopySize;
									SizeRemaining -= CopySize;
								}
							}
						}

						Status = ARNOLD_STATUS_OK;

						if (pSnapshotHeader->Version == 3)
						{
							/* must be at least enough data for a chunk header after the memory dump */
							if (SnapshotLength > (MemSizeInSnapshot + sizeof(SNAPSHOT_HEADER) + sizeof(RIFF_CHUNK)))
							{
								/* some chunks after the data */
								RIFF_CHUNK *pChunk = (RIFF_CHUNK *)((char *)pSnapshotHeader + sizeof(SNAPSHOT_HEADER) + MemSizeInSnapshot);
								unsigned long SizeRemaining;

								SizeRemaining = SnapshotLength - MemSizeInSnapshot - sizeof(SNAPSHOT_HEADER);

								while (SizeRemaining != 0)
								{
									int ChunkSize;

									/* get length of this chunk's data in bytes */
									ChunkSize = Riff_GetChunkLength(pChunk);

									if (ChunkSize == 0)
										break;

									/* chunk size bad? */
									if (ChunkSize > (SizeRemaining - sizeof(RIFF_CHUNK)))
									{
										/* size of chunk is greater than size of data remaining */
										Status = FALSE;
										break;
									}

									/* attempt to handle this chunk */
									SnapshotV3_HandleChunk(pChunk, ChunkSize, pSnapshotMemoryBlocks);

									/* update chunk pointer */
									pChunk = (RIFF_CHUNK *)((unsigned char *)pChunk + ChunkSize + sizeof(RIFF_CHUNK));

									/* update size of data remaining */
									SizeRemaining -= (ChunkSize + sizeof(RIFF_CHUNK));
								}
							}
						}

					}

				}

			}
		}
	}
	Computer_RethinkMemory();

	return Status;
}

/* calculate the size of the output snapshot based on the passed parameters */
unsigned long Snapshot_CalculateEstimatedOutputSize(const SNAPSHOT_OPTIONS *pOptions, const SNAPSHOT_MEMORY_BLOCKS *pMemoryBlocks)
{
	unsigned long SnapshotSize;
	int i;


	/* fixed size snapshot header */
	SnapshotSize = sizeof(SNAPSHOT_HEADER);

	if (
		/* version 2 */
		(pOptions->Version == 2)
		)
	{
		SnapshotSize += (pMemoryBlocks->nMaxBlockV2 << 14);
	}
	else
	{
	
		/* for version 3 the blocks do not need to be continuous */
		/* but they are 64KB each */
		int Start = pMemoryBlocks->nStart64KBBlockV3;

		SnapshotSize += (pMemoryBlocks->nMaxBlockMainV3<<14);

		/* find all 64KB blocks */
		for (i = Start; i < 9; i++)
		{
			if (pMemoryBlocks->QualifyingBlocks[i])
			{
				/* uncompressed size + header for chunk */
				SnapshotSize += (64 * 1024) + sizeof(RIFF_CHUNK);
			}
		}
	}

	if (pOptions->Version == 3)
	{
		/* V3 stuff */

		/* CPC+ hardware? */
		if (CPC_GetHardware() == CPC_HW_CPCPLUS)
		{
			SnapshotSize += SnapshotV3_CPCPlus_CalculateOutputSize();
		}

		/* KCC hardware? */

		/* Aleste hardware? */
	}

	return SnapshotSize;
}



/* fills the supplied pre-allocated buffer with snapshot data. It is the responsibility
of the calling function to allocate and free the data */
size_t Snapshot_GenerateOutputData(unsigned char *pBufferBase, const SNAPSHOT_OPTIONS *pOptions, const SNAPSHOT_MEMORY_BLOCKS *pMemoryBlocks)
{
	unsigned char *pBuffer = pBufferBase;
	int             SnapshotMemorySize = 0;

	if (pOptions->Version == 2)
	{
		SnapshotMemorySize = (pMemoryBlocks->nMaxBlockV2 << 14);
	}
	else
	{
		SnapshotMemorySize = (pMemoryBlocks->nMaxBlockMainV3<<14);
	}

	{
		int i;
		unsigned char SnapshotHeader[sizeof(SNAPSHOT_HEADER)];
		int CPCType;

		/* clear header */
		memset(&SnapshotHeader, 0, sizeof(SNAPSHOT_HEADER));

		/* setup header */
		memcpy(&SnapshotHeader, SNAPSHOT_HEADER_TEXT, 8);

		memcpy(&SnapshotHeader[0x0e0], SNAPSHOT_EMU_TEXT, 6);

		/* set version */
		SnapshotHeader[0x010] = (unsigned char)pOptions->Version;

		switch (pOptions->Version)
		{
		case 1:
		case 2:
		{
			/* type 2 doesn't support Plus! */
			CPCType = 2;
		}
		break;

		default:
		case 3:
		{
			/* version 3 doesn't recognise KC Compact or Aleste */
			/* it's doubtful it can even support GX4000 */
			/* convert my CPC type index to Snapshot CPC type index. */
			if (CPC_GetHardware() == CPC_HW_CPCPLUS)
			{
				CPCType = 4;
			}
			else
			{
				CPCType = 2;
			}
		}
		break;
		}

		/* get cpc type */
		SnapshotHeader[0x06d] = (unsigned char)CPCType;

		/* ***** Z80 CPU ***** */
		{
			int Register;

			char *pRegs = (char *)&SnapshotHeader[0x011];

			Register = CPU_GetReg(CPU_AF);

			SNAPSHOT_PUT_REGISTER_PAIR(pRegs, 0, Register);

			Register = CPU_GetReg(CPU_BC);

			SNAPSHOT_PUT_REGISTER_PAIR(pRegs, 2, Register);

			Register = CPU_GetReg(CPU_DE);

			SNAPSHOT_PUT_REGISTER_PAIR(pRegs, 4, Register);

			Register = CPU_GetReg(CPU_HL);

			SNAPSHOT_PUT_REGISTER_PAIR(pRegs, 6, Register);

			pRegs[8] = (char)CPU_GetReg(CPU_R);
			pRegs[9] = (char)CPU_GetReg(CPU_I);

			/* iff0 */
			pRegs[10] = CPU_GetReg(CPU_IFF1) ? 1 : 0;

			/* iff1 */
			pRegs[11] = CPU_GetReg(CPU_IFF2) ? 1 : 0;

			Register = CPU_GetReg(CPU_IX);

			SNAPSHOT_PUT_REGISTER_PAIR(pRegs, 12, Register);

			Register = CPU_GetReg(CPU_IY);

			SNAPSHOT_PUT_REGISTER_PAIR(pRegs, 14, Register);

			Register = CPU_GetReg(CPU_SP);

			SNAPSHOT_PUT_REGISTER_PAIR(pRegs, 16, Register);

			Register = CPU_GetReg(CPU_PC);

			SNAPSHOT_PUT_REGISTER_PAIR(pRegs, 18, Register);

			Register = CPU_GetReg(CPU_IM);
			pRegs[20] = (char)Register;

			Register = CPU_GetReg(CPU_AF2);

			SNAPSHOT_PUT_REGISTER_PAIR(pRegs, 21, Register);

			Register = CPU_GetReg(CPU_BC2);

			SNAPSHOT_PUT_REGISTER_PAIR(pRegs, 23, Register);

			Register = CPU_GetReg(CPU_DE2);

			SNAPSHOT_PUT_REGISTER_PAIR(pRegs, 25, Register);

			Register = CPU_GetReg(CPU_HL2);

			SNAPSHOT_PUT_REGISTER_PAIR(pRegs, 27, Register);

		}

		switch (CPC_GetHardware())
		{
		case CPC_HW_CPC:
		{
			/**** GATE ARRAY ****/
			/* store colour palette */
			for (i = 0; i < 17; i++)
			{
				SnapshotHeader[0x02f + i] = (unsigned char)((GateArray_GetPaletteColour(i) & 0x01f));
			}

			/* store selected pen */
			SnapshotHeader[0x02e] = (unsigned char)(GateArray_GetSelectedPen());

			/* store multi-configuration */
			SnapshotHeader[0x040] = (unsigned char)((GateArray_GetMultiConfiguration() & 0x01f) | 0x080);
		}
		break;

		case CPC_HW_CPCPLUS:
		{
			/**** GATE ARRAY inside ASIC ****/
			/* store colour palette */
			for (i = 0; i < 17; i++)
			{
				SnapshotHeader[0x02f + i] = (unsigned char)((ASIC_GateArray_GetPaletteColour(i) & 0x01f));
			}

			/* store selected pen */
			SnapshotHeader[0x02e] = (unsigned char)(ASIC_GateArray_GetSelectedPen());

			/* store multi-configuration */
			SnapshotHeader[0x040] = (unsigned char)((ASIC_GateArray_GetMultiConfiguration() & 0x01f) | 0x080);
		}
		break;

		case CPC_HW_KCCOMPACT:
		{
			/* store colour palette */
			for (i = 0; i < 17; i++)
			{
				SnapshotHeader[0x02f + i] = (unsigned char)((KCC_GetColour(i) & 0x01f));
			}

			/* store selected pen */
			SnapshotHeader[0x02e] = (unsigned char)(KCC_GetFN());

			/* store multi-configuration */
			SnapshotHeader[0x040] = (unsigned char)((KCC_GetMF() & 0x01f) | 0x080);

		}
		break;

		case CPC_HW_ALESTE:
		{
			/* store colour palette */
			/* When Aleste is in "MSX mode". This will be RGB */
			for (i = 0; i < 17; i++)
			{
				SnapshotHeader[0x02f + i] = (unsigned char)((Aleste_GetPenColour(i) & 0x01f));
			}

			/* store selected pen */
			SnapshotHeader[0x02e] = (unsigned char)(Aleste_GetCurrentPen());

			/* store multi-configuration; not all bits for aleste! */
			SnapshotHeader[0x040] = (unsigned char)((Aleste_GetMFForSnapshot() & 0x01f) | 0x080);

		}
		break;
		}

		/* store ram configuration */
		SnapshotHeader[0x041] = (unsigned char)(Snapshot_GetRamSelected());

		/**** CRTC ****/
		/* get CRTC status */
		for (i = 0; i < 18; i++)
		{
			SnapshotHeader[0x043 + i] = CRTC_GetRegisterData(i);
		}

		/* select CRTC register */
		SnapshotHeader[0x042] = (unsigned char)(CRTC_GetSelectedRegister() & 0x01f);

		/**** ROM INDEX ****/
		SnapshotHeader[0x055] = (unsigned char)Snapshot_GetRomSelected();


		/**** PSG ****/
		/* get PSG status */
		for (i = 0; i < 16; i++)
		{
			SnapshotHeader[0x05b + i] = (unsigned char)PSG_GetRegisterData(&OnBoardAY, i);
		}

		/* select PSG register */
		SnapshotHeader[0x05a] = (unsigned char)(PSG_GetSelectedRegister(&OnBoardAY) & 0x0f);

		switch (CPC_GetHardware())
		{
		case CPC_HW_CPCPLUS:
		{
			/**** ASIC PPI ****/
			/* get PPI control */
			SnapshotHeader[0x059] = (unsigned char)ASIC_PPI_GetControlForSnapshot();

			/* get port A data */
			SnapshotHeader[0x056] = (unsigned char)ASIC_PPI_GetPortDataForSnapshot(0);
			/* get port B data */
			SnapshotHeader[0x057] = (unsigned char)ASIC_PPI_GetPortDataForSnapshot(1);
			/* get port C data */
			SnapshotHeader[0x058] = (unsigned char)ASIC_PPI_GetPortDataForSnapshot(2);

		}
		break;

		default:
		{
			/* KC Compact has a real 8255 */
			/* CPC has a real 8255 */
			/* Costdown CPC has a real 8255 */
			/* Aleste has a real 8255 */

			/**** PPI ****/
			/* get PPI control */
			SnapshotHeader[0x059] = (unsigned char)PPI_GetControlForSnapshot();

			/* get port A data */
			SnapshotHeader[0x056] = (unsigned char)PPI_GetPortDataForSnapshot(0);
			/* get port B data */
			SnapshotHeader[0x057] = (unsigned char)PPI_GetPortDataForSnapshot(1);
			/* get port C data */
			SnapshotHeader[0x058] = (unsigned char)PPI_GetPortDataForSnapshot(2);
		}
		break;
		}


		/* set memory size */
		SnapshotHeader[0x06b] = (unsigned char)(SnapshotMemorySize >> 10);
		SnapshotHeader[0x06c] = (unsigned char)((SnapshotMemorySize >> 10) >> 8);

		if (
			/* version 2 snapshot */
			(pOptions->Version == 2)
			)
		{
			memcpy(pBuffer, SnapshotHeader, sizeof(SNAPSHOT_HEADER));
			pBuffer += sizeof(SNAPSHOT_HEADER);

			/* finished header for v2 */

			/* copy all blocks */
			for (i = 0; i < pMemoryBlocks->nMaxBlockV2; i++)
			{
				memcpy(pBuffer, pMemoryBlocks->Blocks[i].pPtr, 16384);
				pBuffer += 16384;
			}
		}
		else
		{
			/* TODO: Move version 3 into it's own function */
			if (pOptions->Version == 3)
			{
				/* poke V3 stuff into header */
				unsigned long CRTC_Flags;

				/* put V3 stuff into header */
				CRTC_INTERNAL_STATE *pCRTC_State;

				pCRTC_State = CRTC_GetInternalState();

				if (FDI_GetMotorState())
				{
					SnapshotHeader[0x09c] = 1;
				}
				else
				{
					SnapshotHeader[0x09c] = 0;
				}

				/* last byte written to printer port */
				/* TODO: Check! */
				SnapshotHeader[0x0a1] = Printer_Get7BitData();
				if (Printer_GetStrobeOutputState())
				{
					SnapshotHeader[0x0a1] |= 0x080;
				}

				/* crtc type */
				SnapshotHeader[0x0a4] = CPC_GetCRTCType();

				/* crtc internal state */
				SnapshotHeader[0x0a9] = pCRTC_State->HCount;
				SnapshotHeader[0x0ab] = pCRTC_State->LineCounter;
				SnapshotHeader[0x0ac] = pCRTC_State->RasterCounter;
				SnapshotHeader[0x0ad] = pCRTC_State->VertAdjustCount;

				CRTC_Flags = 0;

				if (pCRTC_State->CRTC_Flags & CRTC_VS_FLAG)
				{
					SnapshotHeader[0x0af] = pCRTC_State->VerticalSyncCount & 0x0f;
					CRTC_Flags |= SNAPSHOT_CRTC_FLAGS_VSYNC_STATE;
				}

				if (pCRTC_State->CRTC_Flags & CRTC_HS_FLAG)
				{
					SnapshotHeader[0x0ae] = pCRTC_State->HorizontalSyncCount & 0x0f;
					CRTC_Flags |= SNAPSHOT_CRTC_FLAGS_HSYNC_STATE;
				}

				if (pCRTC_State->CRTC_Flags & CRTC_VADJ_FLAG)
				{
					SnapshotHeader[0x0ad] = pCRTC_State->VertAdjustCount & 0x01f;
					CRTC_Flags |= SNAPSHOT_CRTC_FLAGS_VADJ_STATE;
				}


				SnapshotHeader[0x0b0] = CRTC_Flags & 0x0ff;
				SnapshotHeader[0x0b1] = (CRTC_Flags >> 8) & 0x0ff;


				if (CPU_GetINTState())
				{
					SnapshotHeader[0x0b4] = 1;
				}
				else
				{
					SnapshotHeader[0x0b4] = 0;
				}

				switch (CPC_GetHardware())
				{
				case CPC_HW_CPC:
				{
					int nVBlankCycle = 0;
					if (GateArray_GetVBlankActive())
					{
						nVBlankCycle = GateArray_GetVBlankCount();
					}
					if (nVBlankCycle > 2)
					{
						nVBlankCycle = 0;
					}

					SnapshotHeader[0x0b2] = nVBlankCycle;
					SnapshotHeader[0x0b3] = GateArray_GetInterruptLineCount();
				}
				break;

				case CPC_HW_CPCPLUS:
				{
					int nVBlankCycle = 0;
					if (ASIC_GateArray_GetVBlankActive())
					{
						nVBlankCycle = ASIC_GateArray_GetVBlankCount();
					}
					if (nVBlankCycle > 2)
					{
						nVBlankCycle = 0;
					}

					SnapshotHeader[0x0b2] = nVBlankCycle;
					SnapshotHeader[0x0b3] = ASIC_GateArray_GetInterruptLineCount();
				}
				break;

				case CPC_HW_KCCOMPACT:
				{
					/* interrupt line count is set inside z8536 */
					SnapshotHeader[0x0b2] = 0;
					SnapshotHeader[0x0b3] = 0;
				}
				break;

				case CPC_HW_ALESTE:
				{
					SnapshotHeader[0x0b2] = 0;
					SnapshotHeader[0x0b3] = 0;
				}
				break;
				}

				/* finished header for v3 */
				/* copy it and advance the pointer */
				memcpy(pBuffer, SnapshotHeader, sizeof(SNAPSHOT_HEADER));
				pBuffer += sizeof(SNAPSHOT_HEADER);

				/* copy all blocks */
				for (i = 0; i < pMemoryBlocks->nMaxBlockMainV3; i++)
				{
					memcpy(pBuffer, pMemoryBlocks->Blocks[i].pPtr, 16384);
					pBuffer += 16384;
				}

				/* write out dk'tronics compatible ram */
				pBuffer = SnapshotV3_Memory_WriteChunk(pBuffer, pMemoryBlocks, pOptions->bCompressed);

				/* CPC+ hardware? */
				if (CPC_GetHardware() == CPC_HW_CPCPLUS)
				{
					pBuffer = SnapshotV3_CPCPlus_WriteChunk(pBuffer);
				}

				/* KCC hardware? */


				/* Aleste hardware? */

				/*	pBuffer = SnapshotV3_DiscFiles_WriteChunk(pBuffer); */
			}
		}
	}

	return pBuffer - pBufferBase;
}
