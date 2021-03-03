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
#ifndef __ASIC_HEADER_INCLUDED__
#define __ASIC_HEADER_INCLUDED__

#include "cpcglob.h"
#include "garray.h"
#include "cpc.h"

typedef struct
{
	int             PauseCount;                     /* pause current count */
	int             PrescaleCount;                  /* channel prescalar current count */
	int             LoopStart;                      /* reload address for loop */
	int             RepeatCount;            /* number of times to repeat the loop */
	Z80_WORD		Instruction;
} ASIC_DMA_CHANNEL;

BOOL    ASIC_Initialise(void);
void    ASIC_Finish(void);

void    ASIC_InitCart(void);
void    ASIC_Reset(void);
void    ASIC_EnableDisable(int);
int		ASIC_DMA_GetChannelAddr(int);
void	ASIC_DMA_SetChannelAddr(int, int);
int		ASIC_DMA_GetChannelPrescale(int);
int ASIC_DMA_GetChannelPrescaleCount(int ChannelIndex);
void ASIC_DMA_FetchOpcode(int ChannelIndex);
BOOL ASIC_DMA_ChannelEnabledAndNotPaused(int ChannelIndex);
BOOL ASIC_DMA_ChannelEnabled(int ChannelIndex);
void    ASIC_DMA_HandleChannel(int ChannelIndex);
void ASIC_WriteRAMIfEnabled(Z80_WORD Addr, Z80_BYTE Data);
void ASIC_InitialiseMemoryOutputs(MemoryData *pData);
void ASIC_InitialiseDefaultMemory(MemoryData *pData);

void ASIC_SetSpriteOverride(int);
int ASIC_GetSpriteOverride(void);

void ASIC_SetScreenSplitOverride(BOOL);
BOOL ASIC_GetScreenSplitOverride(void);


void ASIC_SetHorizontalScrollOverride(BOOL bState);
BOOL ASIC_GetHorizontalScrollOverride(void);
void ASIC_SetVerticalScrollOverride(BOOL bState);
BOOL ASIC_GetVerticalScrollOverride(void);
void ASIC_SetScrollBorderOverride(BOOL bState);
BOOL ASIC_GetScrollBorderOverride(void);


void ASIC_SetPRIInterruptOverride(BOOL);
BOOL ASIC_GetPRIInterruptOverride(void);


unsigned char   *ASIC_GetCartPage(int);
void    ASIC_SetRasterInterrupt(void);
void    ASIC_ClearRasterInterrupt(void);

/*char    *ASIC_DebugDMACommand(int,int); */

unsigned char *ASIC_GetRamPtr(void);

void CPCPLUS_Out(const Z80_WORD Port, const Z80_BYTE Data);
Z80_BYTE CPCPlus_In(const Z80_WORD Port);
void Plus_UpdateGraphicsFunction(void);
unsigned long ASIC_GetPixelData(void);

const unsigned char *Plus_GetBASICRom(void);

unsigned long ASIC_BuildDisplayReturnMaskWithPixels(int Line, int HCount, /*int MonitorHorizontalCount, int ActualY,*/ int *pPixels);

void    ASIC_DoDMA(void);
int ASIC_PPI_ReadControl(void);
int ASIC_PSG_GetPortInputs(int);

/* asic functions to be executed when Htot reached */
void    ASIC_HTot(int);

int             ASIC_CalculateInterruptVector(void);

/* get lock state of ASIC (features locked/unlocked) for snapshot */
BOOL	ASIC_GetUnLockState(void);
/* set lock state of ASIC (features locked/unlocked) for snapshot */
void	ASIC_SetUnLockState(BOOL);

void	ASIC_SetSecondaryRomMapping(unsigned char Data);
int ASIC_GetSecondaryRomMapping(void);

/* reset gate array in ASIC */
void    ASIC_GateArray_Reset(void);

/* trap writes to asic ram */
void    ASIC_WriteRam(int Addr, int Data);

/* used when setting up ASIC in reset or from snapshots */
void	ASIC_WriteRamFull(int Addr, int Data);

BOOL Cartridge_ValidateCartridge(const unsigned char *pData, const unsigned long CartridgeDataLength);
void Cartridge_InsertBinary(const unsigned char *pData, const unsigned long CartridgeDataLength);
int Cartridge_AttemptInsert(unsigned char *pCartridgeData, unsigned long CartridgeLength);
int		Cartridge_Insert(const unsigned char *pCartridgeData, const unsigned long CartridgeDataLength);
void	Cartridge_Autostart(void);
void    Cartridge_Remove(void);
void    Cartridge_RemoveI(void);
void Cartridge_InsertDummy(void);
void Cartridge_SetDefault(const unsigned char *pCartridgeData, const unsigned long CartridgeDataLength);
void Cartridge_InsertDefault(void);

BOOL    ASIC_RasterIntEnabled(void);

void    ASIC_DoRomSelection(void);
Z80_BYTE    ASIC_AcknowledgeInterrupt(void);

void    ASIC_DMA_EnableChannels(unsigned char);
BOOL ASIC_GetInterruptRequest(void);

/* debugger */
unsigned char   ASIC_GetRed(int);
unsigned char ASIC_GetGreen(int);
unsigned char ASIC_GetBlue(int);
unsigned char ASIC_GetSpritePixel(int SpriteIndex, int X, int Y);

BOOL ASIC_GetGX4000(void);
void ASIC_SetGX4000(BOOL bState);

BOOL ASIC_GetHasCassetteInterface(void);
void ASIC_SetHasCassetteInterface(BOOL bState);


typedef struct
{
	/* width of sprite in 16-pixel wide columns */
	unsigned long WidthInColumns;
	/* HCount of column that min sprite x is in */
	unsigned long MinColumn;
	/* height of sprite in scan-lines */
	unsigned long HeightInLines;

	unsigned int    XMagShift, YMagShift;
	unsigned long    x, y;

} ASIC_SPRITE_RENDER_INFO;

typedef enum
{
	DMA_PHASE_DEAD_CYCLE,
	DMA_PHASE_OPCODE_FETCH,
	DMA_PHASE_OPCODE_EXECUTE,
	DMA_PHASE_SELECT_AY_REGISTER,
	DMA_PHASE_SELECT_AY_REGISTER_INACTIVE,
	DMA_PHASE_WRITE_AY_REGISTER,
	DMA_PHASE_WRITE_AY_REGISTER_INACTIVE,
	DMA_PHASE_RESTORE_AY_REGISTER,
	DMA_PHASE_RESTORE_AY_REGISTER_INACTIVE,
	DMA_PHASE_RESTORE_PPI,
	DMA_PHASE_DONE
} DMA_PHASE;

#define ASIC_RAM_ENABLED	0x0002
#define ASIC_ENABLED		0x0001

/* this structure represents what is stored in internal ASIC registers */
typedef struct
{
	union
	{
		unsigned short    SpriteX_W;

#ifdef CPC_LSB_FIRST

		struct
		{
			unsigned char l;
			unsigned char h;
		} SpriteX_B;
#else
		struct
		{
			unsigned char h;
			unsigned char l;
		} SpriteX_B;
#endif

	} SpriteX;

	union
	{
		unsigned short    SpriteY_W;
#ifdef CPC_LSB_FIRST
		struct
		{
			unsigned char l;
			unsigned char h;
		} SpriteY_B;
#else
		struct
		{
			unsigned char h;
			unsigned char l;
		} SpriteY_B;
#endif

	} SpriteY;

	unsigned char   SpriteMag;

	unsigned char	pad[3];
} ASIC_SPRITE_INFO;

typedef struct
{
	union
	{
		unsigned short Addr_W;
#ifdef CPC_LSB_FIRST
		struct
		{
			unsigned char l;
			unsigned char h;
		} Addr_B;
#else
		struct
		{
			unsigned char h;
			unsigned char l;
		} Addr_B;
#endif

	} Addr;

	unsigned char Prescale;
	unsigned char Flags;
} ASIC_DMA_INFO;

typedef enum
{
	SEQUENCE_SYNCHRONISE_FIRST_BYTE = 1,
	SEQUENCE_SYNCHRONISE_SECOND_BYTE,
	SEQUENCE_RECOGNISE,
	SEQUENCE_GET_LOCK_STATUS,
	SEQUENCE_GET_POST_LOCK,
	SEQUENCE_SYNCHRONISE_THIRD_BYTE
} SEQUENCE_STATE;

typedef struct
{
	/* status flags */
	unsigned long	Flags;
	/* pointer to asic ram */
	unsigned char    *ASIC_Ram;
	/* pointer to asic ram for "re-thinking memory" */
	unsigned char	 *ASIC_Ram_Adjusted;
	/* a mask used for memory paging */
	BOOL			ASICRamEnabled;


	/* SPRITES */
	unsigned long SpriteEnableMask;
	unsigned long SpriteEnableMaskOnLine;
	ASIC_SPRITE_INFO Sprites[16];
	ASIC_SPRITE_RENDER_INFO SpriteInfo[16];

	/* DMA */
	ASIC_DMA_INFO    DMA[3];
	ASIC_DMA_CHANNEL DMAChannel[3];

	/* interrupt vector */
	unsigned char ASIC_InterruptVector;
	/* raster interrupt line */
	unsigned char ASIC_RasterInterruptLine;
	/* soft scroll */
	unsigned char ASIC_SoftScroll;
	/* raster split line */
	unsigned char ASIC_RasterSplitLine;

	/* Secondary Screen Address */
	union
	{
		unsigned short Addr_W;
#ifdef CPC_LSB_FIRST
		struct
		{
			unsigned char l;
			unsigned char h;
		} Addr_B;
#else
		struct
		{
			unsigned char h;
			unsigned char l;
		} Addr_B;
#endif
	} ASIC_SecondaryScreenAddress;

	/* holds DMA channel enable states and if interrupt that was acknowledged was from raster interrupts */
	unsigned char DMAChannelEnables;
	unsigned char DMAChannelPaused;

	DMA_PHASE DmaPhase;
	unsigned char OpcodeFetchChannels;
	unsigned char OpcodeExecuteChannels;

	unsigned char RasterInterruptAcknowledged;

	/* bit 7 = 1 if raster interrupt requested */
	/* bit 6 = 1 if DMA channel 0 interrupt requested */
	/* bit 5 = 1 if DMA channel 1 interrupt requested */
	/* bit 4 = 1 if DMA channel 2 interrupt requested */
	unsigned char InterruptRequest;

	unsigned char InternalDCSR;

	unsigned char SecondaryRomMapping;

	unsigned char AnalogueInputs[8];

	int DmaAyRegister;
	int DmaAyData;
	int CurrentAyRegister;
	int Current8255Select;
	int LowerRomIndex;

	int ASIC_Cartridge_Page;
	int ASIC_Cartridge_Page_Lower;
	int ASIC_RAM_Config;

	SEQUENCE_STATE   RecogniseSequenceState;
	int      CurrentSequencePos;

	BOOL	ASIC_PRI_Request;
} ASIC_DATA;

BOOL    ASIC_SpritesActive(void);
void Plus_RestartPower(void);
void Plus_RestartReset(void);
void ASIC_GenerateSpriteActiveMaskForLine(void);
void ASIC_RefreshRasterInterrupt(int, int);
BOOL    ASIC_RasterSplitLineMatch(int, int);

void    ASIC_SetMonitorColourMode(CPC_MONITOR_TYPE_ID Mode);
void    ASIC_InitialiseMonitorColourModes(void);

void    ASIC_SetDebug(BOOL);
void	ASIC_UpdateColours(void);
void ASIC_RethinkMemory(void);

unsigned char ASIC_GetDCSR(void);
unsigned char ASIC_GetPRI(void);
unsigned char ASIC_GetSPLT(void);
unsigned short ASIC_GetSSA(void);
unsigned char ASIC_GetSSCR(void);
unsigned char ASIC_GetIVR(void);

int ASIC_GetPRILine(void);
int ASIC_GetPRIVCC(void);
int ASIC_GetPRIRCC(void);
unsigned char ASIC_GetSPLTLine(void);
int ASIC_GetSPLTVCC(void);
int ASIC_GetSPLTRCC(void);

void	ASIC_SetPRI(unsigned char);
void	ASIC_SetSPLT(unsigned char);
void	ASIC_SetSSA(unsigned long);
void	ASIC_SetSSCR(unsigned char);
void	ASIC_SetIVR(unsigned char);

unsigned short ASIC_GetSpriteX(int SpriteIndex);
unsigned short ASIC_GetSpriteY(int SpriteIndex);
unsigned char ASIC_GetSpriteMagnification(int SpriteIndex);

int ASIC_GetSelectedCartridgePage(void);

/* update palette ram */
void	ASIC_GateArray_UpdatePaletteRam(unsigned char PaletteIndex, unsigned char Colour);

/* set secondary rom mapping? */
BOOL	ASIC_GateArray_CheckForSecondaryRomMapping(unsigned char Function);

void	ASIC_GateArray_RethinkMemory(unsigned char **pReadRamPtr, unsigned char **pWriteRamPtr);
void	ASIC_SetAnalogueInput(int InputID, unsigned char Data);
unsigned char ASIC_GetAnalogueInput(int InputID);
int ASIC_PPI_GetControlForSnapshot(void);

/* write data to gate array */
void    ASIC_GateArray_Write(int);

/* re-setup memory pointers */
void    ASIC_RethinkMemory(void);

int ASIC_GetVRAMAddr(void);
void ASIC_CalcVRAMAddr(void);
void ASIC_CachePixelData(void);

void ASIC_Cycle(void);
void ASIC_DoDispEnable(BOOL bState);
void ASIC_UpdateVsync(BOOL bState);
void ASIC_UpdateHsync(BOOL bState);




BOOL ASIC_GateArray_GetHBlankActive(void);
int ASIC_GateArray_GetHBlankCount(void);

BOOL ASIC_GateArray_GetVBlankActive(void);
int ASIC_GateArray_GetVBlankCount(void);


/* used for Snapshot and Multiface */
int             ASIC_GateArray_GetPaletteColour(int PenIndex);
int             ASIC_GateArray_GetSelectedPen(void);
int             ASIC_GateArray_GetMultiConfiguration(void);

typedef struct
{
	unsigned long	InterruptLineCount;
	unsigned long	InterruptSyncCount;

	unsigned char   PenSelection;
	unsigned char   ColourSelection;
	unsigned char   RomConfiguration;

	unsigned char   PenColour[18];

	unsigned char	PenIndex;
	unsigned char	pad0;
	unsigned char	pad1;


	unsigned long	BlankingOutput;
	unsigned long	CRTCSyncInputs;

	int nHBlankCycle;
	int nVBlankCycle;

	BOOL RasterInterruptRequest;
} ASIC_GATE_ARRAY_STATE;

/* 128k */
void ASIC_SetR128(BOOL bState);
/* disc interface */
void ASIC_SetR129(BOOL bState);

int		ASIC_GateArray_GetInterruptLineCount(void);

void    ASIC_GateArray_AcknowledgeInterrupt(void);
void	ASIC_GateArray_Update(void);
BOOL ASIC_GateArray_GetInterruptRequest(void);

void ASIC_PPI_WriteControl(int);
int ASIC_PPI_GetOutputPort(int nIndex);
int ASIC_PPI_GetPortInput(int nIndex);
void ASIC_PPI_SetPortOutput(int nIndex, int nData);
void    ASIC_PPI_SetPortDataFromSnapshot(int nPort, int Data);
void	ASIC_GateArray_SetInterruptLineCount(int Count);

int ASIC_PPI_GetPortDataForSnapshot(int nPort);
void Plus_FillSnapshotMemoryBlocks(SNAPSHOT_MEMORY_BLOCKS *pSnapshotMemoryBlocks, const SNAPSHOT_OPTIONS *pOptions, BOOL bReading);

void Plus_LoadFromSnapshot(SNAPSHOT_HEADER *pSnapshotHeader);
void Plus_ReadSnapshotChunk(unsigned char *pChunkPtr);
void Plus_WriteSnapshotBlock(unsigned char *pBlock);

#endif

