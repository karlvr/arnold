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
#include "memrange.h"
#include "cpc.h"

int MemoryRange_GetSize(const MemoryRange *pMemoryRange)
{
    if (pMemoryRange==NULL)
        return 0;
    return pMemoryRange->m_nLength;
}

BOOL MemoryRange_IsReadOnly(const MemoryRange *pMemoryRange)
{
    return pMemoryRange->m_bReadOnly;
}

int MemoryRange_ReadWord(const MemoryRange *pMemoryRange, int nOffset)
{
    if (pMemoryRange==NULL)
        return 0;

    if (pMemoryRange->m_bCPU)
        return (CPU_RD_MEM(nOffset)&0x0ff)|((CPU_RD_MEM(nOffset+1)&0x0ff)<<8);

    if (pMemoryRange->pBase==NULL)
        return 0;

    return
        (pMemoryRange->pBase[(nOffset%pMemoryRange->m_nLength)]&0x0ff)|
        ((pMemoryRange->pBase[((nOffset+1)%pMemoryRange->m_nLength)]&0x0ff)<<8);
}


int MemoryRange_ReadByte(const MemoryRange *pMemoryRange, int nOffset)
{
    if (pMemoryRange==NULL)
        return 0;

    if (pMemoryRange->m_bCPU)
        return (CPU_RD_MEM(nOffset)&0x0ff);

    if (pMemoryRange->pBase==NULL)
        return 0;

    return
        (pMemoryRange->pBase[(nOffset%pMemoryRange->m_nLength)]&0x0ff);
}

void MemoryRange_WriteByte(const MemoryRange *pMemoryRange, int nOffset, int nByte)
{
    if (pMemoryRange==NULL)
        return;

    if (pMemoryRange->m_bReadOnly)
        return;

    if (pMemoryRange->m_bCPU)
    {
        CPU_WR_MEM(nOffset, nByte);
        return;
    }

    if (pMemoryRange->pBase==NULL)
        return;

    pMemoryRange->pBase[(nOffset%pMemoryRange->m_nLength)] = nByte;
}


void MemoryRange_WriteWord(const MemoryRange *pMemoryRange, int nOffset, int nWord)
{
    if (pMemoryRange==NULL)
        return;

    if (pMemoryRange->m_bReadOnly)
        return;

    if (pMemoryRange->m_bCPU)
    {
        CPU_WR_MEM(nOffset, (nWord &0x0ff));
        CPU_WR_MEM(nOffset+1, ((nWord>>8) &0x0ff));
        return;
    }

    if (pMemoryRange->pBase==NULL)
        return;

    pMemoryRange->pBase[(nOffset%pMemoryRange->m_nLength)] = nWord &0x0ff;
    pMemoryRange->pBase[((nOffset+1)%pMemoryRange->m_nLength)] = (nWord>>8)&0x0ff;
}
