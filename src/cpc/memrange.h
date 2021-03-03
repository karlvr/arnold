#ifndef __MEMORY_RANGE_HEADER_INCLUDED__
#define __MEMORY_RANGE_HEADER_INCLUDED__

#include "cpcglob.h"

typedef struct
{
    const char *sName;
    int m_nID;
    BOOL m_bCPU;	/* if TRUE, is what CPU sees, if FALSE is what video hardware/dma sees */
    BOOL m_bReadOnly;
    unsigned char *pBase;
    int m_nLength;
} MemoryRange;

BOOL MemoryRange_IsReadOnly(const MemoryRange *pMemoryRange);
int MemoryRange_GetSize(const MemoryRange *pMemoryRange);
int MemoryRange_ReadWord(const MemoryRange *pRange, int nOffset);
void MemoryRange_WriteWord(const MemoryRange *pMemoryRange, int nOffset, int nWord);
int MemoryRange_ReadByte(const MemoryRange *pRange, int nOffset);
void MemoryRange_WriteByte(const MemoryRange *pMemoryRange, int nOffset, int nWord);

#endif
