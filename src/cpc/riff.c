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
#include "cpcglob.h"
#include "riff.h"
#include "cpcendian.h"

/* return length of chunk */
uint32_t Riff_GetChunkLength(const RIFF_CHUNK *pChunk)
{
#ifdef CPC_LSB_FIRST
	return pChunk->ChunkLength;
#else
	return SwapEndianLong(pChunk->ChunkLength);
#endif
}

/* set length of a chunk */
void	Riff_SetChunkLength(RIFF_CHUNK *pChunk, uint32_t Length)
{
#ifdef CPC_LSB_FIRST
	pChunk->ChunkLength = Length;
#else
	pChunk->ChunkLength = SwapEndianLong(Length);
#endif
}

/* return length of chunk */
uint32_t Riff_GetChunkName(const RIFF_CHUNK *pChunk)
{
	return pChunk->ChunkName;
}

/* return pointer to chunk data */
unsigned char   *Riff_GetChunkDataPtr(RIFF_CHUNK *pChunk)
{
        return (unsigned char *)((unsigned char *)pChunk + sizeof(RIFF_CHUNK));
}

const unsigned char   *Riff_GetChunkDataPtrConst(const RIFF_CHUNK *pChunk)
{
        return (const unsigned char *)((const unsigned char *)pChunk + sizeof(RIFF_CHUNK));
}

/* get next chunk after chunk specified */
RIFF_CHUNK      *Riff_GetNextChunk(RIFF_CHUNK *pChunk)
{
        return (RIFF_CHUNK *)((unsigned char *)pChunk + Riff_GetChunkLength(pChunk) + sizeof(RIFF_CHUNK));
}

const RIFF_CHUNK      *Riff_GetNextChunkConst(const RIFF_CHUNK *pChunk)
{
        return (const RIFF_CHUNK *)((const unsigned char *)pChunk + Riff_GetChunkLength(pChunk) + sizeof(RIFF_CHUNK));
}

/* given the pointer to the header, the sub-chunks are searched
 for a chunk of the name given. If one is found a pointer to
 it is returned, otherwise NULL is returned.*/
RIFF_CHUNK      *Riff_FindNamedSubChunk(RIFF_CHUNK *pHeader, uint32_t ChunkName)
{
        /* pointer to data to start searching from
         is pointer to data, +4 for the AMS! type. */
        RIFF_CHUNK *pChunk = (RIFF_CHUNK *)(Riff_GetChunkDataPtr(pHeader) + 4);
        
        /* get length of RIFF chunk */
        int ParentChunkLength = Riff_GetChunkLength(pHeader);

        do
        {
                /* does current chunk have the name of the chunk
                 we are looking for? */
                if (pChunk->ChunkName == ChunkName)
                {
                        /* found named chunk */
                        return pChunk;
                }

                /* next chunk */
                pChunk = Riff_GetNextChunk(pChunk);
        }
        while (((unsigned char *)pChunk-(unsigned char *)pHeader)<ParentChunkLength);

        /* couldn't find named chunk */
        return NULL;
}

/* current chunk is first chunk in file. */
RIFF_CHUNK *Riff_GetFirstChunk(unsigned char *pFileStart)
{
	return (RIFF_CHUNK *)(pFileStart + sizeof(RIFF_CHUNK) + sizeof(uint32_t));
}

/* check all chunks are of a valid size in the RIFF file */
/* a chunck has a valid size if it doesn't go over the end of the file! */
BOOL	Riff_CheckChunkSizesAreValid(const unsigned char *pRiffFile, size_t RiffFileSize)
{
	size_t OffsetInFile = 0;
	uint32_t ChunkLength;

	/* get first chunk in file */
	const RIFF_CHUNK *pCurrentChunk = Riff_GetFirstChunk(pRiffFile);

	/* calc current offset in file */
	OffsetInFile = (unsigned char *)pCurrentChunk - (unsigned char *)pRiffFile;

	do
	{
		/* get length of this current chunk */
		ChunkLength = Riff_GetChunkLength(pCurrentChunk);

		if (ChunkLength>(RiffFileSize-OffsetInFile))
		{
			/* chunk size is greater than size of remaining data in file */
			return FALSE;
		}

		/* get the next chunk */
		pCurrentChunk = Riff_GetNextChunk(pCurrentChunk);
		
		/* calc current offset in file */
		OffsetInFile = (unsigned char *)pCurrentChunk - (unsigned char *)pRiffFile;
	}
	while (OffsetInFile<RiffFileSize);

	return TRUE;
}
