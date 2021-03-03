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
#ifndef __CPC_HEADER_INCLUDED__
#define __CPC_HEADER_INCLUDED__

#define MAX_DRIVES 4

#include "amsdos.h"
#include "snapshot.h"

void Graphics_Update(void);

unsigned char *GetDummyReadRam(void);
unsigned char *GetDummyWriteRam(void);

void CaptureSnapshotData(Z80_WORD Addr, Z80_BYTE Data);
int Snapshot_GetRomSelected(void);
int Snapshot_GetRamSelected(void);

enum
{
	SYS_LANG_EN,
	SYS_LANG_FR,
	SYS_LANG_ES,
	SYS_LANG_FR2
};


enum
{
	KEYBOARD_LANGUAGE_ID_UNKNOWN = 0,
	/* cpc keyboard language id
	 also relates to os translation */
	 KEYBOARD_LANGUAGE_ID_ENGLISH = 1,
	 KEYBOARD_LANGUAGE_ID_FRENCH = 2,
	 KEYBOARD_LANGUAGE_ID_SPANISH = 4,
	 KEYBOARD_LANGUAGE_ID_DANISH = 8
};

/* status codes returned from functions */
enum
{
	/* file was opened ok */
	ARNOLD_STATUS_OK,
	/* file was not recognised */
	ARNOLD_STATUS_UNRECOGNISED,
	/* version of this file not supported */
	ARNOLD_VERSION_UNSUPPORTED,
	/* a general error */
	ARNOLD_STATUS_ERROR,
	/* the file or data was invalid */
	ARNOLD_STATUS_INVALID,
	/* the file or data had a bad length */
	ARNOLD_STATUS_INVALID_LENGTH,
	/* memory allocation request failed */
	ARNOLD_STATUS_OUT_OF_MEMORY
};

#define BUFFER_SIZE 256

typedef struct
{
	BOOL bRecordEnable;
	BOOL bCyclesLastValid;
	int CyclesLast;
	int Cycles[BUFFER_SIZE];
	int CycleIndex;
	int CyclesRecorded;
} CYCLE_RECORDING;

#define HBLANK_ACTIVE (1<<0)
#define VBLANK_ACTIVE (1<<1)
#define DISPTMG_ACTIVE (1<<2)

#define HSYNC_INPUT (1<<0)
#define VSYNC_INPUT (1<<1)

void CPC_PrintToFile(char ch);

void CycleRecording_Init(CYCLE_RECORDING *);
int CycleRecording_GetAverage(CYCLE_RECORDING *);
void CycleRecording_Update(CYCLE_RECORDING *);
void CycleRecording_Enable(CYCLE_RECORDING *, BOOL);

#include "cpcglob.h"

#include "memrange.h"

void CPC_SetBusValue(Z80_BYTE Byte);

void CPC_SetSysLang(int nLang);
int CPC_GetSysLang(void);

enum
{
	CPU_PC,
	CPU_IX,
	CPU_IY,
	CPU_I,
	CPU_R,
	CPU_AF,
	CPU_BC,
	CPU_DE,
	CPU_HL,
	CPU_AF2,
	CPU_BC2,
	CPU_HL2,
	CPU_DE2,
	CPU_SP,
	CPU_MEMPTR,
	CPU_IFF1,
	CPU_IFF2,
	CPU_IM,
	CPU_A,
	CPU_F,
	CPU_B,
	CPU_C,
	CPU_D,
	CPU_E,
	CPU_H,
	CPU_L
};

enum
{
	CPU_FLAG_SIGN,
	CPU_FLAG_ZERO,
	CPU_FLAG_BIT5,
	CPU_FLAG_HALF_CARRY,
	CPU_FLAG_BIT3,
	CPU_FLAG_PARITYOVERFLOW,
	CPU_FLAG_ADDSUBTRACT,
	CPU_FLAG_CARRY
};

#define CPU_OUTPUT_HALT  (1<<0) /* executed halt */
#define CPU_OUTPUT_IORQ (1<<1) /* IORQ and M1 = maskable interrupt acknowledge, IORQ and RD = I/O read, IORQ and WR = I/O write */
#define CPU_OUTPUT_M1 (1<<2) /* M1 and MREQ = opcode fetch */
#define CPU_OUTPUT_MREQ  (1<<3) /* memory read or memory write  */
#define CPU_OUTPUT_RD (1<<4) /* I/O or memory read */
#define CPU_OUTPUT_WR (1<<5) /* I/O or memory write */
#define CPU_OUTPUT_RFSH (1<<6) /* RFSH and MREQ = IR registers placed on bus  */

int CPU_GetOutputs(void);

int Computer_GetCycleCounter(void);
void Computer_ResetCycleCounter(void);

int CPU_GetCycles(void);
BOOL CPU_GetFlag(int nFlag);
int CPU_GetReg(int nReg);
void CPU_SetReg(int nReg, int nValue);
int CPU_ExecuteCycles(void);
void CPU_Reset(void);
void CPU_Power(void);
void CPU_SetNMIState(BOOL bState);
void CPU_SetINTState(BOOL bState);
void CPU_Init(void);
BOOL CPU_GetINTState(void);
int CPU_GetPC(void);
int CPU_GetSP(void);
Z80_BYTE CPU_RD_MEM(const Z80_WORD Addr);
void CPU_WR_MEM(const Z80_WORD Addr, const Z80_BYTE Data);
Z80_BYTE CPU_DoIn(const Z80_WORD Addr);
void CPU_DoOut(const Z80_WORD Addr, const Z80_BYTE Data);
Z80_WORD CPU_RD_BASE_WORD(Z80_WORD Addr);
Z80_BYTE CPU_RD_BASE_BYTE(Z80_WORD Addr);
void CPU_SetDataBus(Z80_BYTE Data);
Z80_BYTE CPU_GetDataBus(void);

void CPC_FillSnapshotMemoryBlocks(SNAPSHOT_MEMORY_BLOCKS *pBlocks, const SNAPSHOT_OPTIONS *pOptions,BOOL bReading);


void    CPU_SetDebugOpcodeEnabled(BOOL bState);
BOOL    CPU_GetDebugOpcodeEnabled(void);
BOOL    CPU_GetDebugOpcodeHit(void);
void    CPU_ResetDebugOpcodeHit(void);
void  CPU_SetDebugOpcodeHit(void);

void ClearOSOverrideROM(void);
void ClearBASICOverrideROM(void);
void ClearAmsdosOverrideROM(void);

typedef enum
{
	CPC_AUDIO_OUTPUT_MONO_SPEAKER = 0,
	CPC_AUDIO_OUTPUT_MONO_EXPANSION,
	CPC_AUDIO_OUTPUT_STEREO
} CPC_AUDIO_OUTPUT_TYPE;

CPC_AUDIO_OUTPUT_TYPE Audio_GetOutput(void);
void Audio_SetOutput(CPC_AUDIO_OUTPUT_TYPE);

BOOL Computer_GetDrawBlanking(void);
void Computer_SetDrawBlanking(BOOL bState);

BOOL Computer_GetDrawBorder(void);
void Computer_SetDrawBorder(BOOL bState);


BOOL CPU_GetDebugStop(void);
void CPU_ResetDebugStop(void);
void CPU_SetDebugStop(void);

unsigned short ExpansionRom_GetWord(const unsigned char *pAddr, int Offset);
BOOL    ExpansionRom_Validate(const unsigned char *pData, unsigned long DataSize);

#define NUM_CRTC_TYPES 6

#define PSG_CLOCK_FREQUENCY 1000000
#define Z80_CLOCK_FREQUENCY	4000000

/* number of nops in whole display */
/* use this to signal when we render the screen */

#define		NOPS_PER_LINE	64			/* time for a single scan line */
#define		BYTES_PER_NOP	2			/* number of bytes in a time unit */
#define		LINES_PER_SCREEN	39*8	/* number of scan-lines on a monitor screen */
#define		NOPS_PER_FRAME	(NOPS_PER_LINE*LINES_PER_SCREEN)

/* the following are emulation tweaks.

we can have a max speed. so that we can slow it down.
do a frame then delay and then do another frame and delay, but then that makes it sluggish?

we could also add features for making loading faster.

we can do frameskip to reduce the drawing.

we do need to measure fps

turbo mode
display every
timing so you can speed it up and slow it down..
has an on-screen fps rating
vice has refresh rate
max speed (which is the rate at which it's running)
warp mode

realtime speed
frame skip: x1,x2,x3,x4,x5,x6,x7 etc

1. Lock speed yes/no (fast as possible compared to a fixed time step)
2. Speed rate (50hz, 100hz, 200hz)
3.

*/

/* set a specific emulation rate. 100 is normal, 0 is unlimited, other values in between e.g. 50 to slow it down */
void CPC_SetEmuSpeedPercent(int nPercent);
int CPC_GetEmuSpeedPercent(void);

void CPC_SetWindowScale(int nPercent);
int CPC_GetWindowScale(void);


#define NOPS_PER_MONITOR_SCREEN (NOPS_PER_LINE*LINES_PER_SCREEN)

/* a device can register one of these */
typedef struct
{
	/* The port I/O address is logically ANDed with this value
	to remove bits we are not interested in */
	Z80_WORD PortAnd;
	/* Then the port I/O address is compared against this value
	which has the state of the bits we are interested in. The
	bits we are not interested in should be set to 0 or 1*/
	Z80_WORD PortCmp;
	/* if the comparison matches, then this function is called
	to write the data to the port */
	void(*pWriteFunction)(Z80_WORD, Z80_BYTE);
} CPCPortWrite;

/* a device can register one of these */
typedef struct
{
	/* The port I/O address is logically ANDed with this value
	to remove bits we are not interested in */
	Z80_WORD PortAnd;
	/* Then the port I/O address is compared against this value
	which has the state of the bits we are interested in. The
	bits we are not interested in should be set to 0 or 1*/
	Z80_WORD PortCmp;
	/* if the comparison matches, then this function is called
	to read from the port. BOOL means the device put data onto the bus, FALSE means otherwise */
	BOOL (*pReadFunction)(Z80_WORD, Z80_BYTE *pData);
} CPCPortRead;

typedef struct
{
	/* /RAMEN output from CPC */
	BOOL bRamRead[8];
	/* /ROMEN output from CPC. On standard CPC this is low when:
	lower:
	A15=0, A14=0,
	A15=0, A14=1,
	upper:
	A15=1, A14=0,
	A15=1, A14=1
	*/
	BOOL bRomEnable[8];

	/* blocks are:
	0: &0000-&1fff
	1: &2000-&3fff
	2: &4000-&5fff
	3: &6000-&7fff
	4: &8000-&9fff
	5: &a000-&bfff
	6: &c000-&dfff
	7: &e000-&ffff
	*/

	/* Access is done pointer + offset:
	0: +&0000
	1: +&2000
	2: +&4000
	3: +&6000
	4: +&8000
	5: +&a000
	6: +&c000
	7: +&e000

	So store address - this offset to make it correct.
	For a 16k block:
	0: -&0000
	1: -&0000
	2: -&4000
	3: -&4000
	4: -&8000
	5: -&8000
	6: -&c000
	7: -&c000

	*/

	const unsigned char *pReadPtr[8];
	unsigned char *pWritePtr[8];

	/* /RAMDIS input to CPC */
	BOOL bRamDisable[8];
	/* /ROMDIS input to CPC */
	BOOL bRomDisable[8];
} MemoryData;

typedef void(*CPC_RESET_FUNCTION)(void);
typedef void(*CPC_MEMORY_RETHINK_FUNCTION)(MemoryData *pMemData);
typedef void(*CPC_POWER_FUNCTION)(void);
typedef void(*CPC_UPDATE_FUNCTION)(void);
typedef void(*CPC_CURSOR_UPDATE_FUNCTION)(int nState);
typedef void(*CPC_PRINTER_UPDATE_FUNCTION)(void);
typedef void(*CPC_RETI_FUNCTION)(void);
typedef void(*CPC_ACK_MASKABLE_INTERRUPT_FUNCTION)(void);
typedef unsigned char(*CPC_JOYSTICK_READ_FUNCTION)(int);
typedef void(*CPC_SOUND_UPDATE_FUNCTION)(void);
typedef void(*CPC_LIGHT_SENSOR_FUNCTION)(BOOL bState);

void CPC_SetLightSensorPos(int x, int y);
void CPC_UpdateLightSensor(void);

/* register/unregister a function that gets called when the printer output changes */
void CPC_InstallPrinterUpdateFunction(CPC_PRINTER_UPDATE_FUNCTION printerUpdateFunction);
void CPC_UnInstallPrinterUpdateFunction(CPC_PRINTER_UPDATE_FUNCTION printerUpdateFunction);
CPC_PRINTER_UPDATE_FUNCTION CPC_GetPrinterUpdateFunction(void);

void CPC_InstallSoundUpdateFunction(CPC_SOUND_UPDATE_FUNCTION);
void CPC_UnInstallSoundUpdateFunction(CPC_SOUND_UPDATE_FUNCTION);

void CPC_InstallLightSensorFunction(CPC_LIGHT_SENSOR_FUNCTION);
void CPC_UnInstallLightSensorFunction(CPC_LIGHT_SENSOR_FUNCTION);

/* register/unregister a function that gets called when the printer output changes */
void CPC_InstallJoystickReadFunction(CPC_JOYSTICK_READ_FUNCTION printerUpdateFunction);
void CPC_UnInstallJoystickReadFunction(CPC_JOYSTICK_READ_FUNCTION printerUpdateFunction);

/* register/unregister a function that gets called when a reset occurs */
/* device reset functions */
void CPC_InstallResetFunction(CPC_RESET_FUNCTION resetFunction);
void CPC_UnInstallResetFunction(CPC_RESET_FUNCTION resetFunction);

/* register/unregister a function that gets called when the crtc cursor output changes */
/* device cursor changed functions */
void CPC_InstallCursorUpdateFunction(CPC_CURSOR_UPDATE_FUNCTION cursorUpdateFunction);
void CPC_UnInstallCursorUpdateFunction(CPC_CURSOR_UPDATE_FUNCTION cursorUpdateFunction);

/* register/unregister a function that gets called when a power on occurs */
void CPC_InstallPowerFunction(CPC_POWER_FUNCTION powerFunction);
void CPC_UnInstallPowerFunction(CPC_POWER_FUNCTION powerFunction);

void CPC_InstallUpdateFunction(CPC_UPDATE_FUNCTION updateFunction);
void CPC_UnInstallUpdateFunction(CPC_UPDATE_FUNCTION updateFunction);

/* register/unregister a memory range that the debugger can see */
void CPC_RegisterMemoryRange(const MemoryRange *pRange);
void CPC_UnRegisterMemoryRange(const MemoryRange *pRange);

int CPC_GetRegisteredMemoryRangeCount(void);

const MemoryRange *CPC_GetRegisteredMemoryRange(int nIndex);
const MemoryRange *CPC_GetDefaultMemoryRange(void);


/* device port read functions */
void CPC_InstallReadPort(CPCPortRead *readPort);
void CPC_UninstallReadPort(CPCPortRead *readPort);

/* device memory read functions */
void CPC_InstallReadMemory(CPCPortRead *readPort);
void CPC_UninstallReadMemory(CPCPortRead *readPort);

/* device port write functions */
void CPC_InstallWritePort(CPCPortWrite *writePort);
void CPC_UninstallWritePort(CPCPortWrite *writePort);


/* device port write functions */
void CPC_InstallWriteMemory(CPCPortWrite *writeMemory);
void CPC_UninstallWriteMemory(CPCPortWrite *writeMemory);

void CPC_ExecuteReadPortFunctions(Z80_WORD Port, Z80_BYTE *pData);
void CPC_ExecuteWritePortFunctions(Z80_WORD Port, Z80_BYTE Data);
void CPC_ExecuteReadMemoryFunctions(Z80_WORD Port, Z80_BYTE *pData);
void CPC_ExecuteWriteMemoryFunctions(Z80_WORD Port, Z80_BYTE Data);
void CPC_ExecuteLightSensorFunctions(BOOL);
void CPC_ExecuteSoundUpdateFunctions(void);

void CPC_ExecutePowerFunctions(void);
void CPC_ExecuteUpdateFunctions(void);
void CPC_ExecuteResetFunctions(void);

void CPC_InstallMemoryRethinkHandler(CPC_MEMORY_RETHINK_FUNCTION);
void CPC_UnInstallMemoryRethinkHandler(CPC_MEMORY_RETHINK_FUNCTION);

void CPC_InstallRetiHandler(CPC_RETI_FUNCTION);
void CPC_UnInstallRetiHandler(CPC_RETI_FUNCTION);
void CPC_ExecuteRetiFunctions(void);


void CPC_InstallAckMaskableInterruptHandler(CPC_ACK_MASKABLE_INTERRUPT_FUNCTION);
void CPC_UnInstallAckMaskableInterruptHandler(CPC_ACK_MASKABLE_INTERRUPT_FUNCTION);
void CPC_ExecuteAckMaskableInterruptHandler(void);

void Computer_DoVideoCycles(int);
void Computer_RefreshInterrupt(void);
void Computer_DoCycles(int nCycles);
void Computer_UpdateGraphicsFunction(void);

/* CRTC outputs */
void Computer_UpdateVsync(void);
void Computer_UpdateHsync(void);
void CRTC_SetDispEnable(BOOL DispEnable);
void CRTC_DoCursorOutput(BOOL nState);
void CRTC_SetVsyncOutput(BOOL bState);
void CRTC_SetHsyncOutput(BOOL bState);

void Amstrad_DiscInterface_Install(void);
void Amstrad_DiscInterface_UnInstall(void);

typedef enum
{
	/* line 0, bit 0..bit 7 */
	CPC_KEY_CURSOR_UP = 0,
	CPC_KEY_CURSOR_RIGHT,
	CPC_KEY_CURSOR_DOWN,
	CPC_KEY_F9,
	CPC_KEY_F6,
	CPC_KEY_F3,
	CPC_KEY_SMALL_ENTER,
	CPC_KEY_FDOT,
	/* line 1, bit 0..bit 7 */
	CPC_KEY_CURSOR_LEFT,
	CPC_KEY_COPY,
	CPC_KEY_F7,
	CPC_KEY_F8,
	CPC_KEY_F5,
	CPC_KEY_F1,
	CPC_KEY_F2,
	CPC_KEY_F0,
	/* line 2, bit 0..bit 7 */
	CPC_KEY_CLR,
	CPC_KEY_OPEN_SQUARE_BRACKET,
	CPC_KEY_RETURN,
	CPC_KEY_CLOSE_SQUARE_BRACKET,
	CPC_KEY_F4,
	CPC_KEY_SHIFT,
	CPC_KEY_BACKSLASH,
	CPC_KEY_CONTROL,
	/* line 3, bit 0.. bit 7 */
	CPC_KEY_HAT,
	CPC_KEY_MINUS,
	CPC_KEY_AT,
	CPC_KEY_P,
	CPC_KEY_SEMICOLON,
	CPC_KEY_COLON,
	CPC_KEY_FORWARD_SLASH,
	CPC_KEY_DOT,
	/* line 4, bit 0..bit 7 */
	CPC_KEY_ZERO,
	CPC_KEY_9,
	CPC_KEY_O,
	CPC_KEY_I,
	CPC_KEY_L,
	CPC_KEY_K,
	CPC_KEY_M,
	CPC_KEY_COMMA,
	/* line 5, bit 0..bit 7 */
	CPC_KEY_8,
	CPC_KEY_7,
	CPC_KEY_U,
	CPC_KEY_Y,
	CPC_KEY_H,
	CPC_KEY_J,
	CPC_KEY_N,
	CPC_KEY_SPACE,
	/* line 6, bit 0..bit 7 */
	CPC_KEY_6,
	CPC_KEY_5,
	CPC_KEY_R,
	CPC_KEY_T,
	CPC_KEY_G,
	CPC_KEY_F,
	CPC_KEY_B,
	CPC_KEY_V,
	/* line 7, bit 0.. bit 7 */
	CPC_KEY_4,
	CPC_KEY_3,
	CPC_KEY_E,
	CPC_KEY_W,
	CPC_KEY_S,
	CPC_KEY_D,
	CPC_KEY_C,
	CPC_KEY_X,
	/* line 8, bit 0.. bit 7 */
	CPC_KEY_1,
	CPC_KEY_2,
	CPC_KEY_ESC,
	CPC_KEY_Q,
	CPC_KEY_TAB,
	CPC_KEY_A,
	CPC_KEY_CAPS_LOCK,
	CPC_KEY_Z,
	/* line 9, bit 7..bit 0 */
	CPC_KEY_JOY_UP,
	CPC_KEY_JOY_DOWN,
	CPC_KEY_JOY_LEFT,
	CPC_KEY_JOY_RIGHT,
	CPC_KEY_JOY_FIRE2,
	CPC_KEY_JOY_FIRE1,
	CPC_KEY_SPARE,
	CPC_KEY_DEL,


	/* no key press */
	CPC_KEY_NUM_KEYS,
	CPC_KEY_NULL = CPC_KEY_NUM_KEYS
} CPC_KEY_ID;


const char *CPC_GetKeyName(CPC_KEY_ID nKey);

/* update keyboard with external keyboard data */
void CPC_ResolveKeys(unsigned char *pKeyboardData);
void CPC_PreResolveKeys(void);

BOOL Keyboard_GetAutoDetectLanguage(void);
void Keyboard_SetAutoDetectLanguage(BOOL);
BOOL Keyboard_DetectLanguage(void);


BOOL Firmware_GetAutoDetect16Roms(void);
void Firmware_SetAutoDetect16Roms(BOOL bState);
BOOL Firmware_Detect16Roms(void);
BOOL Firmware_GetSupports16Roms(void);
void Firmware_SetSupports16Roms(BOOL bState);

BOOL Firmware_Detect16Roms(void);

void Keyboard_SetMode(int nMode);
int Keyboard_GetMode(void);

void Keyboard_SetPositionalSet(int nSet);
int Keyboard_GetPositionalSet(void);

void Keyboard_SetLanguage(int nLanguage);
int Keyboard_GetLanguage(void);

/* digital joystick 0 */
#define CPC_JOY0_UP	CPC_KEY_JOY_UP
#define CPC_JOY0_DOWN CPC_KEY_JOY_DOWN
#define CPC_JOY0_LEFT	CPC_KEY_JOY_LEFT
#define CPC_JOY0_RIGHT CPC_KEY_JOY_RIGHT
#define CPC_JOY0_FIRE1	CPC_KEY_JOY_FIRE1
#define CPC_JOY0_FIRE2 CPC_KEY_JOY_FIRE2
#define CPC_JOY0_SPARE CPC_KEY_SPARE

/* digital joystick 1 */
#define CPC_JOY1_UP	CPC_KEY_6
#define CPC_JOY1_DOWN CPC_KEY_5
#define CPC_JOY1_LEFT	CPC_KEY_R
#define CPC_JOY1_RIGHT CPC_KEY_T
#define CPC_JOY1_FIRE1	CPC_KEY_F
#define CPC_JOY1_FIRE2 CPC_KEY_G
#define CPC_JOY1_SPARE CPC_KEY_B


#define MAXREDEFBUTTON 16	/* Maximum button on joystick we can redefine. */
#define MAXREDEFAXIS 4		/* maximum axis on joystick we can redefine. */
#define MAXREDEFHAT 1		/*Maximum hat on joystick we can redefine. */

/* this value is the start of the special action ids */
#define SPECIALACTIONID CPC_KEY_NULL+1

typedef enum
{
	/* CPC/CPC+ digital joystick 0 */
	CPC_DIGITAL_JOYSTICK0 = 0,
	/* CPC/CPC+ digital joystick 1 (joystick splitter required for CPC) */
	CPC_DIGITAL_JOYSTICK1,
	/* CPC+ analogue joystick */
	CPC_ANALOGUE_JOYSTICK0,

	CPC_ANALOGUE_JOYSTICK1,
	/* multiplay hardware */
	MULTIPLAY_JOYSTICK0,
	MULTIPLAY_JOYSTICK1,


	/* keep last */
	CPC_NUM_JOYSTICKS
} CPC_JOYSTICK_ID;

#define SPEAKER_VOLUME_MAX 100

int CPC_GetSpeakerVolume(void);
void CPC_SetSpeakerVolume(int nVolume);

#define CPC_NUM_DIGITAL_JOYSTICKS 2
#define CPC_NUM_ANALOGUE_JOYSTICKS 2
#define MULTIPLAY_NUM_JOYSTICKS 2

void Computer_RethinkMemory(void);

int CPC_GetVRAMAddr(void);
void CPC_CalcVRAMAddr(void);
void CPC_CachePixelData(void);
BOOL	CPC_Initialise(void);
void	CPC_Finish(void);

void	Computer_RestartPower(void);
void	Computer_RestartReset(void);

void CPC_PrinterWrite(unsigned char Data);

void Computer_SetIgnoreMotor(BOOL bState);

BOOL Computer_GetIgnoreMotor(void);

BOOL Computer_GetTapeMotorOutput(void);

void Computer_SetTapeMotor(BOOL bState);
void Computer_SetTapeWrite(unsigned char DataBit);
int Computer_GetTapeRead(void);
unsigned char Computer_GetTapeVolume(void);

enum
{
	/* CPC+ hardware design */
	/* ASIC combining 8255, CRTC and Gate Array */
	CPC_HW_CPCPLUS,
	/* CPC hardware design */
	/* seperate 8255, CRTC and Gate Array */
	CPC_HW_CPC,
	/* KC Compact hardware design */
	/* Z8536 CIO */
	CPC_HW_KCCOMPACT,
	/* Aleste 520 EX */
	CPC_HW_ALESTE
};



/* computer names */
enum
{
	PPI_COMPUTER_NAME_ISP = 0,
	PPI_COMPUTER_NAME_TRIUMPH,
	PPI_COMPUTER_NAME_SAISHO,
	PPI_COMPUTER_NAME_SOLAVOX,
	PPI_COMPUTER_NAME_AWA,
	PPI_COMPUTER_NAME_SCHNEIDER,
	PPI_COMPUTER_NAME_ORION,
	PPI_COMPUTER_NAME_AMSTRAD
};



#define PPI_TAPE_READ_DATA                              (1<<7)
#define PPI_CENTRONICS_BUSY                             (1<<6)
#define PPI_EXPANSION_PORT                              (1<<5)

#define PPI_SCREEN_REFRESH_50HZ                 (0x001<<4)
#define PPI_SCREEN_REFRESH_60HZ                 (0x000<<4)
#define VSYNC_ACTIVE    0x001

//void    Audio_Init(int newFrequency, int newBitsPerSample, int newNoOfChannels);
void    Audio_Finish(void);
void    Audio_Update(int);
void    Audio_Commit(void);

void	CPC_SetExpLow(BOOL bExp);
BOOL	CPC_GetExpLow(void);

void	CPC_SetHardware(int);
int		CPC_GetHardware(void);

BOOL	AllocateEmulatorMemory(void);
void	FreeEmulatorMemory(void);

/* select crtc emulation */
void	CPC_SetCRTCType(unsigned int);
int CPC_GetCRTCType(void);

/* define CPC types */
typedef enum
{
	CPC_TYPE_CPC464_EN = 0,
	CPC_TYPE_CPC464_FR,
	CPC_TYPE_CPC464_DK,
	CPC_TYPE_CPC664,
	CPC_TYPE_CPC6128_EN,
	CPC_TYPE_CPC6128_FR,
	CPC_TYPE_CPC6128_ES,
	CPC_TYPE_464PLUS,
	CPC_TYPE_6128PLUS,
	CPC_TYPE_KCCOMPACT,
	CPC_TYPE_ALESTE,
	CPC_TYPE_GX4000
} CPC_TYPE_ID;

typedef enum
{
	CPC_MONITOR_COLOUR = 0,
	CPC_MONITOR_GT64,
	CPC_MONITOR_MM12
} CPC_MONITOR_TYPE_ID;

/* colour, green screen or grey scale monitor type */
void	CPC_SetMonitorType(CPC_MONITOR_TYPE_ID);
CPC_MONITOR_TYPE_ID CPC_GetMonitorType(void);

/* CPC set on-board OS */
void CPC_SetOSRom(const unsigned char *pOSRom);
const unsigned char *CPC_GetOSROM(void);

/* CPC set on-board BASIC */
void CPC_SetBASICRom(const unsigned char *pBASICROM);
const unsigned char *CPC_GetBASICROM(void);

/* override */
void CPC_SetOSOverrideROM(const unsigned char *pOSRom, unsigned long Length);
const unsigned char *CPC_GetOSOverrideROM(void);
void CPC_SetBASICOverrideROM(const unsigned char *pOSRom, unsigned long Length);
const unsigned char *CPC_GetBASICOverrideROM(void);
void CPC_SetAmsdosOverrideROM(const unsigned char *pOSRom, unsigned long Length);
const unsigned char *CPC_GetAmsdosOverrideROM(void);
void CPC_SetOSOverrideROMEnable(BOOL bState);
BOOL CPC_GetOSOverrideROMEnable(void);
void CPC_SetBASICOverrideROMEnable(BOOL bState);
BOOL CPC_GetBASICOverrideROMEnable(void);
void CPC_SetAmsdosOverrideROMEnable(BOOL bState);
BOOL CPC_GetAmsdosOverrideROMEnable(void);



void CPC_RestartReset(void);
void CPC_RestartPower(void);

void	CPC_ResetTiming(void);
void	CPC_ResetNopCount(void);
unsigned long	CPC_GetNopCount(void);
void	CPC_UpdateNopCount(unsigned long);
void CPC_ExecuteCycles(int NopCount);
int CPU_GetInterruptVector(void);
int CPU_GetIOData(void);
int CPU_GetIOPort(void);
void CPU_SetIOData(int nData);
void CPU_SetIOPort(int nPort);
void CPU_Reti(void);

BOOL CPU_GetInterruptRequest(void);
BOOL CPU_GetNMIInput(void);
int CPU_GetInterruptVector(void);
void CPU_PowerOn(void);
int CPU_GetIntVectorAddress(void);

int CPU_GetIntVectorAddress(void);
void CPU_SetIntVectorAddress(int nValue);

void CPC_AcknowledgeNMI(void);

Z80_BYTE CPC_AcknowledgeInterrupt(void);

/*void	CPC_AcknowledgeInterrupt(void); */

void	AudioDAC_SetVolume(unsigned char, unsigned char);


void	CPC_SetComputerNameIndex(int);
int		CPC_GetComputerNameIndex(void);

void	CPC_Set50Hz(BOOL);
BOOL	CPC_Get50Hz(void);

/* keys and joystick and clash - current line */
unsigned char Keyboard_Read(void);
/* keys and joystick and clash */
unsigned char Keyboard_GetLine(int Line);
/* keys only, no clash */
unsigned char Keyboard_GetRealLine(int Line);
int Keyboard_GetSelectedLine(void);

/* instead of a clear keyboard, should we really have a disable input?? */
void	CPC_ClearKeyboard(void);

/* from host keyboard interface */
void CPC_SetKey(int KeyID);
/* from host keyboard interface */
void CPC_ClearKey(int KeyID);

/* do not use, internal, bypasses what can be pressed */
void CPC_SetKeyInternal(int KeyID);
void CPC_ClearKeyInternal(int KeyID);

void       CPC_GenerateKeyboardClash(void);


/*
typedef struct
{
union
{
signed long	L;

struct
{
unsigned short Fraction;
signed short Int;
} W;
};
} FIXED_POINT16;
*/

BOOL Keyboard_HasBeenScanned(void);
void Keyboard_ResetHasBeenScanned(void);

BOOL Keyboard_IsClashEnabled(void);
void Keyboard_EnableClash(BOOL bState);


unsigned long SaveRamToBufferGetLength(FILE_HEADER *pFileHeader);
void SaveRamToBuffer(const MemoryRange *pRange, unsigned char *pBuffer, FILE_HEADER *pFileHeader);
void GetHeaderDataFromBuffer(const unsigned char *pFileData, unsigned long FileDataSize, FILE_HEADER *pFileHeader);
void LoadBufferToRam(const MemoryRange *pRange, const char *pBuffer, FILE_HEADER *pFileHeader);

void CPC_LoadFromSnapshot(SNAPSHOT_HEADER *pSnapshotHeader);

#endif
