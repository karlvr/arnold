#include "cpc.h"
#include "snapshot.h"

/* interrupt */
Z80_BYTE    KCC_AcknowledgeInterrupt(void);
BOOL    KCC_GetInterruptRequest(void);

void KCC_InitialiseMemoryOutputs(MemoryData *pData);
void KCC_InitialiseDefaultMemory(MemoryData *pData);

void KCC_Cycle(void);
void    KCC_SetColorROM(const unsigned char *pData);

void KCC_FillSnapshotMemoryBlocks(SNAPSHOT_MEMORY_BLOCKS *pMemoryBlocks, const SNAPSHOT_OPTIONS *pOptions, BOOL bReading);

/* vsync/hsync inputs */
void	KCC_UpdateHsync(BOOL bState);
void    KCC_UpdateVsync(BOOL bState);
void    KCC_DoDispEnable(BOOL);

int KCC_GetVRAMAddr(void);
void KCC_CalcVRAMAddr(void);
void KCC_CachePixelData(void);
unsigned short KCC_GetPixelData(void);

void KCC_RethinkMemory(void);
void    KCC_GA_Write(int Function);
void KCC_RestartReset(void);
void KCC_RestartPower(void);
void KCC_Init(void);
void KCC_Finish(void);
void KCC_UpdateGraphicsFunction(void);

const unsigned char *KCC_GetOSRom(void);
void KCC_SetOSRom(const unsigned char *);
const unsigned char *KCC_GetBASICRom(void);
void KCC_SetBASICRom(const unsigned char *);

void KCC_SetMonitorColourMode(CPC_MONITOR_TYPE_ID);

int KCC_GetFN(void);

int KCC_GetFW(void);

int KCC_GetMF(void);

int KCC_GetColour(int);

void KCC_SetFN(int nIndex);

void KCC_SetFW(int nIndex);

void KCC_SetMF(int nData);

void KCC_SetColour(int nIndex, int nData);

void	KCCompact_Out(const Z80_WORD Port, const Z80_BYTE Data);
Z80_BYTE        KCCompact_In(Z80_WORD Port);
int KCC_PPI_Port_Read(int nIndex);
void KCC_PPI_Port_Write(int nIndex, int Data);

int KCC_PSG_GetPortInputs(int Port);

void KCC_LoadFromSnapshot(SNAPSHOT_HEADER *pSnapshotHeader);


