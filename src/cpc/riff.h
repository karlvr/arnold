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
#ifndef __RIFF_HEADER_INCLUDED__
#define __RIFF_HEADER_INCLUDED__

#include <stdint.h>

#ifdef CPC_LSB_FIRST
#define RIFF_FOURCC_CODE(a,b,c,d) \
        (uint32_t)(((a)&0x0ff)		| \
                       (((b)&0x0ff)<<8)		| \
                       (((c)&0x0ff)<<16)		| \
                       (((d)&0x0ff)<<24))
#else
#define RIFF_FOURCC_CODE(a,b,c,d) \
        (uint32_t)((((unsigned char)a)<<24)	| \
                       (((unsigned char)b)<<16)		| \
                       (((unsigned char)c)<<8)		| \
                       (((unsigned char)d)))
#endif


/* structure defining RIFF_CHUNK header. Chunk data follows header. */
typedef struct 
{
        uint32_t ChunkName;
        uint32_t ChunkLength;
} RIFF_CHUNK;

uint32_t Riff_GetChunkName(const RIFF_CHUNK *pChunk);
uint32_t Riff_GetChunkLength(const RIFF_CHUNK *pChunk);
void	Riff_SetChunkLength(RIFF_CHUNK *pChunk,uint32_t);
unsigned char   *Riff_GetChunkDataPtr(RIFF_CHUNK *pChunk);
const unsigned char   *Riff_GetChunkDataPtrConst(const RIFF_CHUNK *pChunk);
RIFF_CHUNK      *Riff_GetNextChunk(RIFF_CHUNK *pChunk);
RIFF_CHUNK      *Riff_FindNamedSubChunk(RIFF_CHUNK *pHeader, uint32_t ChunkName);
RIFF_CHUNK		*Riff_GetFirstChunk(unsigned char *pFileStart);
BOOL	Riff_CheckChunkSizesAreValid(const unsigned char *pRiffFile, size_t RiffFileSize);

#endif
