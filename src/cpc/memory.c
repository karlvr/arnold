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
#include "memory.h"
#include "garray.h"
#include <memory.h>

#if 0
#ifndef NULL
#define NULL 0
#endif

/* pointers to the head of each memory block list
for each of the regions */
static MemoryBlock RAMReadHead[8];
static MemoryBlock RAMWriteHead[8];
static MemoryBlock ROMReadHead[8];
static MemoryBlock ROMWriteHead[8];

#if 0
/* these point to the actual ram/rom that the CPU can see
for reading and writing operations */
const unsigned char   *pReadRamPtr[8];
unsigned char   *pWriteRamPtr[8];
#endif

/* dummy memory to read from */
static unsigned char *pDummyReadMemory;
/* dummy memory to write to */
static unsigned char *pDummyWriteMemory;
/* dummy memory blocks for unassigned areas */

static MemoryBlock DummyReadMemoryBlock[8+8];
static MemoryBlock DummyWriteMemoryBlock[8+8];


/* if a bit is 1, then use the ROM table instead of the RAM table */
static unsigned long MemoryReadFlags;
static unsigned long MemoryWriteFlags;

unsigned long Memory_GetReadFlags(void)
{
	return MemoryReadFlags;
}

void	Memory_SetReadFlags(unsigned long Flags)
{
	MemoryReadFlags = Flags;
}

unsigned long	Memory_GetWriteFlags(void)
{
	return MemoryWriteFlags;
}

void	Memory_SetWriteFlags(unsigned long Flags)
{
	MemoryWriteFlags = Flags;
}


void Memory_Init(void)
{
	int i;

	pDummyReadMemory = (unsigned char *)malloc(8192);
	pDummyWriteMemory = (unsigned char *)malloc(8192);

	if (pDummyReadMemory!=NULL)
	{
		memset(pDummyReadMemory,0x0ff,8192);
	}

	for (i=0; i<8+8; i++)
	{
		DummyReadMemoryBlock[i].pPtr = (pDummyReadMemory - (unsigned)(i<<14));
		DummyReadMemoryBlock[i].Priority = 0;
	}

	for (i=0; i<8+8; i++)
	{
		DummyWriteMemoryBlock[i].pPtr = (pDummyWriteMemory - (unsigned)(i<<14));
		DummyWriteMemoryBlock[i].Priority = 0;
	}

	for (i=0; i<8; i++)
	{
		RAMReadHead[i].pNext = RAMReadHead[i].pPrev = &RAMReadHead[i];
	}

	for (i=0; i<8; i++)
	{
		RAMWriteHead[i].pNext = RAMWriteHead[i].pPrev = &RAMWriteHead[i];
	}

	for (i=0; i<8; i++)
	{
		ROMReadHead[i].pNext = ROMReadHead[i].pPrev = &ROMReadHead[i];
	}

	for (i=0; i<8; i++)
	{
		ROMWriteHead[i].pNext = ROMWriteHead[i].pPrev = &ROMWriteHead[i];
	}

	for (i=0; i<8; i++)
	{
		Memory_AddRAMReadMemoryBlock(i, &DummyReadMemoryBlock[i]);
	}

	for (i=0; i<8; i++)
	{
		Memory_AddRAMWriteMemoryBlock(i, &DummyWriteMemoryBlock[i]);
	}

	for (i=0; i<8; i++)
	{
		Memory_AddROMReadMemoryBlock(i, &DummyReadMemoryBlock[i+8]);
	}

	for (i=0; i<8; i++)
	{
		Memory_AddROMWriteMemoryBlock(i, &DummyWriteMemoryBlock[i+8]);
	}

}

void Memory_Finish(void)
{
	if (pDummyReadMemory!=NULL)
	{
		free(pDummyReadMemory);
		pDummyReadMemory = NULL;
	}

	if (pDummyWriteMemory!=NULL)
	{
		free(pDummyWriteMemory);
		pDummyWriteMemory = NULL;
	}
}


static void Memory_InsertBefore(MemoryBlock *pNext, MemoryBlock *pBlock)
{
	pBlock->pPrev = pNext->pPrev;
	pBlock->pNext = pNext;
	pNext->pPrev = pBlock;
	pBlock->pPrev->pNext = pBlock;
}

static void Memory_InsertAfter(MemoryBlock *pPrevious, MemoryBlock *pBlock)
{
	pBlock->pNext = pPrevious->pNext;
	pBlock->pPrev = pPrevious;
	pPrevious->pNext = pBlock;
	pBlock->pNext->pPrev = pBlock;
}

static void	Memory_AddMemoryBlock(MemoryBlock *pHead, MemoryBlock *pBlock)
{
	MemoryBlock *pCurrent = pHead->pNext;

	while (pCurrent->pNext!=pHead)
	{
		/* is the priority of the block we are inserting higher than the priority
		of the block we are looking at? */
		if (pBlock->Priority>=pCurrent->Priority)
		{
			/* insert the block */
			Memory_InsertBefore(pCurrent, pBlock);
			/* exit */
			return;
		}

		/* go to next block */
		pCurrent = pCurrent->pNext;
	}

	Memory_InsertAfter(pCurrent, pBlock);
}

void	Memory_AddRAMReadMemoryBlock(int MemoryRegion, MemoryBlock *pBlock)
{
	MemoryBlock *pHead = RAMReadHead[MemoryRegion].pNext;

	Memory_AddMemoryBlock(pHead, pBlock);
}

void	Memory_AddRAMWriteMemoryBlock(int MemoryRegion, MemoryBlock *pBlock)
{
	MemoryBlock *pHead = RAMWriteHead[MemoryRegion].pNext;

	Memory_AddMemoryBlock(pHead, pBlock);
}

void	Memory_AddROMReadMemoryBlock(int MemoryRegion, MemoryBlock *pBlock)
{
	MemoryBlock *pHead = ROMReadHead[MemoryRegion].pNext;

	Memory_AddMemoryBlock(pHead, pBlock);
}

void	Memory_AddROMWriteMemoryBlock(int MemoryRegion, MemoryBlock *pBlock)
{
	MemoryBlock *pHead = ROMWriteHead[MemoryRegion].pNext;

	Memory_AddMemoryBlock(pHead, pBlock);
}


void	Memory_Rethink(void)
{
#if 0
	int i;

	for (i=0; i<8; i++)
	{
		if (MemoryReadFlags & (1<<i))
		{
			pReadRamPtr[i] = RAMReadHead[i].pNext->pPtr;
		}
		else
		{
			pReadRamPtr[i] = ROMReadHead[i].pNext->pPtr;
		}

		if (MemoryWriteFlags & (1<<i))
		{
			pWriteRamPtr[i] = RAMWriteHead[i].pNext->pPtr;
		}
		else
		{
			pWriteRamPtr[i] = ROMWriteHead[i].pNext->pPtr;
		}
	}
#endif
}
#endif
