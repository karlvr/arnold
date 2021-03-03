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
#ifndef __SNAPSHOT_V3_HEADER_INCLUDED__
#define __SNAPSHOT_V3_HEADER_INCLUDED__

#include "cpcglob.h"
#include "riff.h"
#include "snapshot.h"

/* handle a V3 chunk on reading */
void SnapshotV3_HandleChunk(const RIFF_CHUNK *pCurrentChunk,unsigned long Size, const SNAPSHOT_MEMORY_BLOCKS *pMemoryBlocks);
/* write the V3 CPC+ chunk */
unsigned char *SnapshotV3_CPCPlus_WriteChunk(unsigned char *buffer);

/* begin a chunk */
unsigned char *SnapshotV3_BeginChunk(unsigned char *buffer, unsigned long ChunkName);

/* write data to chunk */
unsigned char *SnapshotV3_WriteDataToChunk(unsigned char *buffer, const unsigned char *pData, unsigned long Length);

/* end a chunk */
void SnapshotV3_EndChunk(void);

unsigned long SnapshotV3_CPCPlus_CalculateOutputSize(void);

unsigned char *SnapshotV3_Memory_WriteChunk(unsigned char *buffer, const SNAPSHOT_MEMORY_BLOCKS *pMemoryBlocks, BOOL bCompressed);
unsigned char *SnapshotV3_DiscFiles_WriteChunk(unsigned char *buffer);

BOOL Snapshot_DecompressBlock(const unsigned char *pChunkData, unsigned long nChunkLength, unsigned char *pOutputData, unsigned long nOutputLength);


#endif

