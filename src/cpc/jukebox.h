#ifndef __JUKEBOX_HEADER_INCLUDED__
#define __JUKEBOX_HEADER_INCLUDED__

#include "cpcglob.h"

void Jukebox_Reset(void);
void Jukebox_Finish(void);
void Jukebox_Init(void);
void Jukebox_Write(Z80_WORD Addr, Z80_BYTE Data);
void Jukebox_CartridgeInsert(int nCartridge, const unsigned char *pData, const unsigned long CartridgeLength);
void Jukebox_SetCartridgeEnable(int nCartridge, BOOL fEnable);
BOOL Jukebox_IsCartridgeEnabled(int nCartridge);
void Jukebox_InsertSystemCartridge(const unsigned char *pCartridgeData, const unsigned long CartridgeLength);
void Jukebox_Enable(BOOL bEnable);
BOOL Jukebox_IsEnabled(void);
void Jukebox_SelectCartridge(int nCartridge);
BOOL Jukebox_Read(Z80_WORD Port, Z80_BYTE *pDeviceData);
void Jukebox_CartridgeRemove(int nCartridge);

void Jukebox_SetTimerEnabled(BOOL bEnabled);
BOOL Jukebox_IsTimerEnabled(void);
void Jukebox_SetTimerSelection(int nSelection);
int Jukebox_GetTimerSelection(void);
void Jukebox_SetManualTimerSelection(int nValue);
int Jukebox_GetManualTimerSelection(void);

int Jukebox_GetInputs(int nInput);
int Jukebox_GetOutputs(int nOutput);

#endif
