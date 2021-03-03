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
 /* Snapshot V3 format as used by No$CPC v1.8 and Winape */

#include "cpcglob.h"
#include "snapshot.h"
#include "snapv3.h"
#include "riff.h"
#include "cpc.h"
#include "asic.h"
#include "diskimage/diskimg.h"
#include <string.h>
#include "emudevice.h"

/* start of chunk */
static	unsigned long SnapshotV3_ChunkLength;
static	RIFF_CHUNK	*pRiffChunk;

extern BOOL LoadFile(const unsigned char *, unsigned char **, unsigned long *);


/* begin a chunk */
unsigned char *SnapshotV3_BeginChunk(unsigned char *buffer, unsigned long ChunkName)
{
	RIFF_CHUNK ChunkHeader;

	/* setup initial header */
	ChunkHeader.ChunkName = ChunkName;
	ChunkHeader.ChunkLength = 0;

	/* write into buffer */
	pRiffChunk = (RIFF_CHUNK *)buffer;
	memcpy(buffer, (unsigned char *)&ChunkHeader, sizeof(RIFF_CHUNK));
	buffer += sizeof(RIFF_CHUNK);

	/* reset chunk length */
	SnapshotV3_ChunkLength = 0;

	return buffer;
}

/* write data to chunk */
unsigned char *SnapshotV3_WriteDataToChunk(unsigned char *buffer, const unsigned char *pData, unsigned long Length)
{
	/* write data to buffer and update length */
	memcpy(buffer, pData, Length);
	SnapshotV3_ChunkLength += Length;

	buffer += Length;

	return buffer;
}

/* end a chunk */
void SnapshotV3_EndChunk(void)
{
	/* calc pointer to header */
	RIFF_CHUNK *pHeader = (RIFF_CHUNK *)(pRiffChunk);

	/* write length into header */
	Riff_SetChunkLength(pHeader, SnapshotV3_ChunkLength);
}

unsigned char *SnapshotV3_WriteByte(unsigned char *buffer, unsigned char ByteData)
{
	return SnapshotV3_WriteDataToChunk(buffer, &ByteData, 1);
}

unsigned char *SnapshotV3_WriteWord(unsigned char *buffer, unsigned short WordData)
{
	unsigned char ByteData;

	ByteData = WordData & 0x0ff;
	buffer = SnapshotV3_WriteByte(buffer, ByteData);
	ByteData = (WordData >> 8) & 0x0ff;
	buffer = SnapshotV3_WriteByte(buffer, ByteData);

	return buffer;
}

/* need to handle BRKS
 2 byte address
 1 byte code (0=central, 1=banks)
 2 bytes (condition)
*/

/* handle chunk on reading */
void	SnapshotV3_HandleChunk(const RIFF_CHUNK *pCurrentChunk, unsigned long Size, const SNAPSHOT_MEMORY_BLOCKS *pMemoryBlocks)
{
	unsigned long ChunkName = Riff_GetChunkName(pCurrentChunk);

	switch (ChunkName)
	{
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
			const unsigned char *pChunkData = Riff_GetChunkDataPtrConst (pCurrentChunk);

			/* this is the length of the compressed data and also serves to indicate number of bytes remaining to transfer */
			unsigned long nChunkLength = Riff_GetChunkLength(pCurrentChunk);

			unsigned long nOutputLength = 64 * 1024;

			/* length of block to decompress */
			unsigned long MemBank = (ChunkName >> 24) & 0x0ff;
			int Bank = MemBank - '0';

			if (pMemoryBlocks->QualifyingBlocks[Bank])
			{
				if (nChunkLength == 65536)
				{
					int i;

					printf("Block is exactly 65536 bytes. Assume stored uncompressed\n");

					/* if exactly 65536 then it is stored uncompressed */
					for (i = 0; i < 4; i++)
					{
						memcpy(pMemoryBlocks->Blocks[(Bank << 2) + i].pPtr, pChunkData + (i << 14), 16384);
					}
				}
				else
				{
					unsigned char *pCombinedBlock = malloc(65536);
					if (pCombinedBlock)
					{
						int i;

						if (Snapshot_DecompressBlock(pChunkData, nChunkLength, pCombinedBlock, nOutputLength))
						{
							for (i = 0; i < 4; i++)
							{
								memcpy(pMemoryBlocks->Blocks[(Bank << 2) + i].pPtr, pCombinedBlock + (i << 14), 16384);
							}
						}
						else
						{
							printf("Failed to decompress block\n");
						}
						free(pCombinedBlock);
					}
				}
			}
		}
		break;

		case RIFF_FOURCC_CODE('D', 'S', 'C', 'A'):
		case RIFF_FOURCC_CODE('D', 'S', 'C', 'B'):
		{
			/* disc image names, not null terminated */
			const unsigned char *pChunkData = Riff_GetChunkDataPtrConst(pCurrentChunk);
			unsigned long ChunkLength = Riff_GetChunkLength(pCurrentChunk);

			/* allocate a buffer for the filename including null terminator */
			char *pDiscImageFilename = (char *)malloc(ChunkLength + 1);
			if (pDiscImageFilename != NULL)
			{
				int nDriveID = ((ChunkName >> 24) & 0x0ff) - 'A';
				unsigned char *pDiskImage = NULL;
				unsigned long DiskImageLength = 0;

				memcpy(pDiscImageFilename, pChunkData, ChunkLength);
				pDiscImageFilename[ChunkLength] = '\0';

				/* load disk image file to memory */
	/*				LoadFile(pDiscImageFilename, &pDiskImage, &DiskImageLength); */

				if (pDiskImage != NULL)
				{
					/* try to insert it */
					DiskImage_InsertDisk(nDriveID, pDiskImage, DiskImageLength);

					free(pDiskImage);
				}
				free(pDiscImageFilename);
			}
		}
		break;


		case RIFF_FOURCC_CODE('C', 'P', 'C', '+'):
		{
			if (CPC_GetHardware() == CPC_HW_CPCPLUS)
			{
				if (Riff_GetChunkLength(pCurrentChunk) == 0x08f8)
				{
					const unsigned char *pChunkData = Riff_GetChunkDataPtr(pCurrentChunk);

					Plus_ReadSnapshotChunk(pChunkData);
				}
			}
		}
		break;

	default:
		printf("Unhandled chunk\n");
		break;

	}
}

/* calculate size of CPC Plus chunk */
unsigned long SnapshotV3_CPCPlus_CalculateOutputSize(void)
{
	unsigned long nChunkLength;

	/* chunk header */
	nChunkLength = sizeof(RIFF_CHUNK);
	nChunkLength += 0x08f8;

	return nChunkLength;
}

BOOL Snapshot_DecompressBlock(const unsigned char *pChunkData, unsigned long nChunkLength, unsigned char *pOutputData, unsigned long nOutputLength)
{
	unsigned char ch;

	while ((nChunkLength != 0) && (nOutputLength != 0))
	{
		/* get byte */
		ch = *pChunkData;
		++pChunkData;
		nChunkLength--;

		/* possible repetition of a byte */
		if (ch == 0xe5)
		{
			/* more data... */
			if (nChunkLength != 0)
			{
				/* get count */
				ch = *pChunkData;
				++pChunkData;
				nChunkLength--;

				if (ch == 0)
				{
					/* single 0x0e5 */
					*pOutputData = 0x0e5;
					++pOutputData;
					--nOutputLength;
				}
				else
				{
					/* more data... */

					/* if chunk length remaining is zero then we do not have enough info */
					if (nChunkLength != 0)
					{
						unsigned long nCount = ch & 0x0ff;

						/* now get byte */
						ch = *pChunkData;
						++pChunkData;
						nChunkLength--;

						/* write out repetition of byte */
						while ((nCount != 0) && (nOutputLength != 0))
						{
							*pOutputData = ch;
							++pOutputData;
							--nCount;
							--nOutputLength;
						}
					}
				}
			}
		}
		else
		{
			/* byte as-is */
			*pOutputData = ch;
			++pOutputData;
			--nOutputLength;
		}
	}
	/* if we reached the end of the chunk data AND we decompressed the correct amount */
	return ((nChunkLength == 0) && (nOutputLength==0));
}


unsigned char *Snapshot_WriteCompressedData(unsigned char *pDest, unsigned long *pDestLength, unsigned char chPrev, int nCount)
{
	unsigned long nDestLength = *pDestLength;

	if (nCount == 1)
	{
		/* control byte? */
		if (chPrev == 0x0e5)
		{
			/* single 0x0e5 */
			*pDest = 0x0e5;
			++pDest;
			--nDestLength;
			*pDest = 0x00;
			++pDest;
			--nDestLength;
		}
		else
		{
			/* not control byte */

			/* store byte as is */
			*pDest = chPrev;
			++pDest;
			--nDestLength;
		}
	}
	else
	{
		/* seen two repetitions and not control byte? */
		if ((nCount == 2) && (chPrev != 0x0e5))
		{
			/* store them as is; because it is one byte shorter than writing repeat sequence */
			*pDest = chPrev;
			++pDest;
			--nDestLength;

			*pDest = chPrev;
			++pDest;
			--nDestLength;
		}
		else
		{
			/* store count */
			*pDest = 0x0e5;
			++pDest;
			--nDestLength;
			*pDest = nCount;
			++pDest;
			--nDestLength;
			*pDest = chPrev;
			++pDest;
			--nDestLength;
		}
	}
	*pDestLength = nDestLength;

	return pDest;
}

BOOL Snapshot_CompressBlock(const unsigned char *pSrc, unsigned long nSrcLength, unsigned long *pDestLength, unsigned char **ppCompressedBlock)
{
	unsigned long OriginalLength = nSrcLength + (nSrcLength >> 1);
	unsigned long nDestLength = OriginalLength;

	/* worst case: e5,00,<not e5> */
	/* allocate enough for worse case */
	unsigned char *pCompressedBlock = (unsigned char *)malloc(nDestLength);
	*ppCompressedBlock = pCompressedBlock;

	if (pCompressedBlock != NULL)
	{
		unsigned char chPrev = 0;
		unsigned char *pDest = pCompressedBlock;
		int nCount = 0;

		while (nSrcLength != 0)
		{
			/* get char */
			chPrev = *pSrc;
			++pSrc;
			--nSrcLength;

			/* init count */
			nCount = 1;

			/* if we're not at the end, collect more */
			while (nSrcLength != 0)
			{
				unsigned char chCurrent;

				/* get char; note do not advance, because if char is different we will re-read it
				in the outer loop */
				chCurrent = *pSrc;

				/* same as previous? */
				if (chCurrent == chPrev)
				{
					/* same */

					/* advance */
					++pSrc;
					--nSrcLength;

					++nCount;

					/* have we reached limit of count? */
					if (nCount == 255)
					{
						break;
					}
				}
				else
				{
					/* not same, */
					break;
				}
			}

			pDest = Snapshot_WriteCompressedData(pDest, &nDestLength, chPrev, nCount);
		}
	}

	*pDestLength = OriginalLength - nDestLength;

	/* consumed all bytes in source data AND length is less than max */
	return ((nSrcLength == 0) && (*pDestLength < 65536));
}

unsigned char *SnapshotV3_MemoryBlock_WriteChunk(unsigned char *buffer, int nBlock, const unsigned char *pMemoryBlock, BOOL bCompressed)
{
	buffer = SnapshotV3_BeginChunk(buffer, RIFF_FOURCC_CODE('M', 'E', 'M', '0' + nBlock));

	if (bCompressed)
	{
		unsigned char *pCompressedBlock;
		unsigned long CompressedLength;

		if (!Snapshot_CompressBlock(pMemoryBlock, 65536, &CompressedLength, &pCompressedBlock))
		{
			/* if compression resulted in a block too large, store it */
			buffer = SnapshotV3_WriteDataToChunk(buffer, pMemoryBlock, 65536);
		}
		else
		{
			/* defensive */
			if (CompressedLength >= 65536)
			{
				/* if compression resulted in a block too large, store it */
				buffer = SnapshotV3_WriteDataToChunk(buffer, pMemoryBlock, 65536);
			}
			else
			{
				/* store compressed data */
				buffer = SnapshotV3_WriteDataToChunk(buffer, pCompressedBlock, CompressedLength);
			}
		}
		if (pCompressedBlock != NULL)
		{
			free(pCompressedBlock);
		}
	}
	else
	{
		buffer = SnapshotV3_WriteDataToChunk(buffer, pMemoryBlock, 65536);
	}

	SnapshotV3_EndChunk();

	return buffer;
}

unsigned char *SnapshotV3_Memory_WriteChunk(unsigned char *buffer, const SNAPSHOT_MEMORY_BLOCKS *pMemoryBlocks, BOOL bCompressed)
{
	int i;

	int Start = (pMemoryBlocks->nMaxBlockMainV3>>2);

	/* find all 64KB blocks */
	for (i = Start; i < 9; i++)
	{
		if (pMemoryBlocks->QualifyingBlocks[i])
		{
			unsigned char *pCombinedBlock = malloc(65536);
			if (pCombinedBlock)
			{
				memcpy(pCombinedBlock, pMemoryBlocks->Blocks[(i << 2) + 0].pPtr, 16384);
				memcpy(pCombinedBlock + 16384, pMemoryBlocks->Blocks[(i << 2) + 1].pPtr, 16384);
				memcpy(pCombinedBlock + 32768, pMemoryBlocks->Blocks[(i << 2) + 2].pPtr, 16384);
				memcpy(pCombinedBlock + 49152, pMemoryBlocks->Blocks[(i << 2) + 3].pPtr, 16384);
				buffer = SnapshotV3_MemoryBlock_WriteChunk(buffer, i, pCombinedBlock, bCompressed);
				free(pCombinedBlock);
			}
		}
	}
	return buffer;
}

unsigned char *SnapshotV3_DiscFile_WriteChunk(unsigned char *buffer, int nDrive)
{
	buffer = SnapshotV3_BeginChunk(buffer, RIFF_FOURCC_CODE('D', 'S', 'C', 'A' + nDrive));

	/*	buffer = SnapshotV3_WriteDataToChunk(buffer, pCompressedBlock,CompressedLength); */

	SnapshotV3_EndChunk();

	return buffer;
}

unsigned char *SnapshotV3_DiscFiles_WriteChunk(unsigned char *buffer)
{
	int i;
	for (i = 0; i < 3; i++)
	{
		buffer = SnapshotV3_DiscFile_WriteChunk(buffer, i);
	}

	return buffer;
}

unsigned char *SnapshotV3_CPCPlus_WriteChunk(unsigned char *buffer)
{
	int i;
	char TempBlock[0x08f8];
	buffer = SnapshotV3_BeginChunk(buffer, RIFF_FOURCC_CODE('C', 'P', 'C', '+'));

	Plus_WriteSnapshotBlock(TempBlock);

	for (i = 0; i < 0x08f8; i++)
	{
		buffer = SnapshotV3_WriteByte(buffer, TempBlock[i]);
	}

	SnapshotV3_EndChunk();

	return buffer;
}

