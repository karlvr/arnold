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
#ifndef __ALESTE_HEADER_INCLUDED__
#define __ALESTE_HEADER_INCLUDED__

#include "cpc.h"

/* interrupt */
Z80_BYTE    Aleste_AcknowledgeInterrupt(void);
BOOL    Aleste_GetInterruptRequest(void);

void    Aleste_SetColorROM(const unsigned char *pData);

/* vsync/hsync inputs */
void	Aleste_UpdateHsync(BOOL bState);
void    Aleste_UpdateVsync(BOOL bState);

void	Aleste_Out(const Z80_WORD Port, const Z80_BYTE Data);
Z80_BYTE        Aleste_In(Z80_WORD Port);

int Aleste_GetVRAMAddr(void);
void Aleste_CalcVRAMAddr(void);
void Aleste_CachePixelData(void);
unsigned short Aleste_GetPixelData(void);

void Aleste_RethinkMemory(void);

void    Aleste_SetMonitorColourMode(CPC_MONITOR_TYPE_ID MonitorMode);

int Aleste_GetLED0(void);
int Aleste_GetLED1(void);


void Aleste_FillSnapshotMemoryBlocks(SNAPSHOT_MEMORY_BLOCKS *pMemoryBlocks, const SNAPSHOT_OPTIONS *pOptions,BOOL bReading);

const unsigned char *Aleste_GetBASICRom(void);
const unsigned char *Aleste_GetOSRom(void);

void Aleste_InitialiseMemoryOutputs(MemoryData *pData);
void Aleste_InitialiseDefaultMemory(MemoryData *pData);

int Aleste_GetCurrentPen(void);
int Aleste_GetPenColour(int nIndex);
int Aleste_GetMF(void);
int Aleste_GetMFForSnapshot(void);
int Aleste_GetMapper(int nIndex);
int Aleste_GetExtport(void);

void Aleste_DoDispEnable(BOOL);
void Aleste_Init(void);
void Aleste_Finish(void);
void Aleste_RestartReset(void);
void Aleste_RestartPower(void);
void Aleste_SetMapperROM(const unsigned char *pData);
void Aleste_SetRfColDatROM(const unsigned char *pData);
void Aleste_SetAl512(const unsigned char *pData);
int Aleste_GetExtport(void);
void Aleste_SetRomRamROM(const unsigned char *);
void Aleste_UpdateGraphicsFunction(void);

void Aleste_PPI_Port_Write(int nPort, int Data);

int Aleste_PPI_Port_Read(int nPort);

void Aleste_PSG_SetPortOutputs(int Port, int Data);
int Aleste_PSG_GetPortInputs(int Port);

void Aleste_LoadFromSnapshot(SNAPSHOT_HEADER *pHeader);

#endif
