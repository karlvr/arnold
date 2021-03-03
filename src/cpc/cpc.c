/*
 *  Arnold emulator (c) Copyright, Kevin Thacker 1997-2001
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
/*********************************

  CPC Hardware code
  (c) Kevin Thacker 1997-2015

  **********************************/

#include <time.h>
  
#include "cpc.h"
#include "cassette.h"
#include "z8536.h"
#include "fdi.h"
#include "fdd.h"
#include "amsdos.h"
#include "pal.h"
#include "fdc.h"
#include "crtc.h"
#include "garray.h"
#include "debugger/gdebug.h"
#include "asic.h"
#include "cpcglob.h"
#include "diskimage/diskimg.h"
#include "printer.h"
#include "kempston.h"
#include "memory.h"
#include "jukebox.h"
#include "kcc.h"
#include "aleste.h"
#include "multface.h"
#include "monitor.h"
#include "i8255.h"
#include "dumpym.h"
#include "autotype.h"
#include "autorunfile.h"
#include "riff.h"
#include "joystick.h"
#include "asic.h"
#include "emudevice.h"
#include "render.h"
#include "keyjoy.h"
#include "debugger/breakpt.h"

static Z80_BYTE Snapshot_RomSelected = 0;
static Z80_BYTE Snapshot_RamSelected = 0;

void CaptureSnapshotData(Z80_WORD Addr, Z80_BYTE Data)
{
	/* capture ROM selected for snapshot */
	/* devices can handle this port as they wish */
	if ((Addr & 0x02000) == 0)
	{
		Snapshot_RomSelected = Data;
	}

	/* capture ram selection */
	if ((Addr & 0x08000) == 0)
	{
		if ((Data & 0x0c0) == 0x0c0)
		{
			Snapshot_RamSelected = Data;
		}
	}
}

int Snapshot_GetRamSelected(void)
{
	return Snapshot_RamSelected;
}

int Snapshot_GetRomSelected(void)
{
	return Snapshot_RomSelected;
}


/* dummy ram we can read from; simulates where no ram exists, always returns
same data */
unsigned char DummyReadRam[16384];
/* dummy ram we can write to; simulates area where no ram exists, or where
writes don't go to normal ram */
unsigned char DummyWriteRam[16384];

unsigned char *GetDummyReadRam(void)
{
	return DummyReadRam;
}

unsigned char *GetDummyWriteRam(void)
{
	return DummyWriteRam;
}
MemoryData MemData;
AY_3_8912 OnBoardAY;


static int CPC_SysLang = SYS_LANG_EN;

void CPC_SetSysLang(int nLang)
{
	CPC_SysLang = nLang;
}

int CPC_GetSysLang(void)
{
	return CPC_SysLang;
}

/* Hardware we are emulating */
/* CPC, PLUS or KC Compact */
static int CPC_Hardware = CPC_HW_CPC;


static CPC_AUDIO_OUTPUT_TYPE m_nAudioOutput = CPC_AUDIO_OUTPUT_MONO_SPEAKER;

CPC_AUDIO_OUTPUT_TYPE Audio_GetOutput(void)
{
	return m_nAudioOutput;
}

void Audio_SetOutput(CPC_AUDIO_OUTPUT_TYPE nOutput)
{
	m_nAudioOutput = nOutput;
}

/* normal speed */
int m_nSpeedPercent = 100;

/* 0 means unlimited */
void CPC_SetEmuSpeedPercent(int nSpeed)
{
	/* negative; say it's normal */
	if (nSpeed < 0)
	{
		nSpeed = 100;
	}
	/* 500% speed */
	if (nSpeed > 500)
	{
		nSpeed = 500;
	}
	m_nSpeedPercent = nSpeed;
}

int CPC_GetEmuSpeedPercent(void)
{
	return m_nSpeedPercent;
}


int m_nWindowScale = 100;

void CPC_SetWindowScale(int nScale)
{
	if (nScale < 0)
	{
		nScale = 100;
	}
	if (nScale > 500)
	{
		nScale = 500;
	}
	m_nWindowScale = nScale;
}

int CPC_GetWindowScale(void)
{
	return m_nWindowScale;
}

static int NumReadPorts = 0;
static CPCPortRead	readPorts[MAX_DEVICES];
static int NumWritePorts = 0;
static CPCPortWrite writePorts[MAX_DEVICES];
static int NumReadMemory = 0;
static CPCPortRead	readMemorys[MAX_DEVICES];
static int NumWriteMemory = 0;
static CPCPortWrite writeMemorys[MAX_DEVICES];
static int NumResetFunctions = 0;
static CPC_RESET_FUNCTION resetFunctions[MAX_DEVICES];
static int NumPowerFunctions = 0;
static CPC_POWER_FUNCTION powerFunctions[MAX_DEVICES];
static int NumMemoryRethinkFunctions = 0;
static CPC_MEMORY_RETHINK_FUNCTION memoryRethinkFunctions[MAX_DEVICES];
static int NumUpdateFunctions = 0;
static CPC_UPDATE_FUNCTION updateFunctions[MAX_DEVICES];
static int NumCursorOutputFunctions = 0;
static CPC_CURSOR_UPDATE_FUNCTION cursorOutputFunctions[MAX_DEVICES];
static CPC_JOYSTICK_READ_FUNCTION m_pJoystickReadFunction = NULL;
static CPC_PRINTER_UPDATE_FUNCTION m_pPrinterUpdateFunction = NULL;
static int NumLightSensorFunctions = 0;
static CPC_LIGHT_SENSOR_FUNCTION lightSensorFunctions[MAX_DEVICES];
static int NumAudioUpdateFunctions = 0;
static CPC_SOUND_UPDATE_FUNCTION audioUpdateFunctions[MAX_DEVICES];
static int NumRetiFunctions = 0;
static CPC_RETI_FUNCTION retiFunctions[MAX_DEVICES];
static int NumAckMaskableInterruptFunctions = 0;
static CPC_ACK_MASKABLE_INTERRUPT_FUNCTION ackMaskableInterruptFunctions[MAX_DEVICES];

/* register/unregister a function that gets called when the printer output changes */
void CPC_InstallPrinterUpdateFunction(CPC_PRINTER_UPDATE_FUNCTION printerUpdateFunction)
{
	m_pPrinterUpdateFunction = printerUpdateFunction;
}
void CPC_UnInstallPrinterUpdateFunction(CPC_PRINTER_UPDATE_FUNCTION printerUpdateFunction)
{
	m_pPrinterUpdateFunction = NULL;
}


/* install a function that gets called when the cursor output from the crtc changes */
void CPC_InstallLightSensorFunction(CPC_LIGHT_SENSOR_FUNCTION lightSensorFunction)
{
	int i;

	for (i = 0; i < NumLightSensorFunctions; i++)
	{
		if (lightSensorFunctions[i] == lightSensorFunction)
		{
			return;
		}
	}
	if (NumLightSensorFunctions < MAX_DEVICES)
	{
		lightSensorFunctions[NumLightSensorFunctions] = lightSensorFunction;
		NumLightSensorFunctions++;
	}
}

/* uninstall a function that gets called when the cursor output from the crtc changes */
void CPC_UnInstallLightSensorFunction(CPC_LIGHT_SENSOR_FUNCTION lightSensorFunction)
{
	int i;

	for (i = 0; i < NumLightSensorFunctions; i++)
	{
		if (lightSensorFunctions[i] == lightSensorFunction)
		{
			int Count = ((NumLightSensorFunctions - 1) - i);
			if (Count != 0)
			{
				int j;
				for (j = 0; j < Count; j++)
				{
					lightSensorFunctions[j] = lightSensorFunctions[j + 1];
				}
			}
			NumLightSensorFunctions--;

			return;
		}
	}
}


/* install a function that gets called when the cursor output from the crtc changes */
void CPC_InstallSoundUpdateFunction(CPC_SOUND_UPDATE_FUNCTION soundUpdateFunction)
{
	int i;

	for (i = 0; i < NumAudioUpdateFunctions; i++)
	{
		if (audioUpdateFunctions[i] == soundUpdateFunction)
		{
			return;
		}
	}
	if (NumAudioUpdateFunctions < MAX_DEVICES)
	{
		audioUpdateFunctions[NumAudioUpdateFunctions] = soundUpdateFunction;
		NumAudioUpdateFunctions++;
	}
}

/* uninstall a function that gets called when the cursor output from the crtc changes */
void CPC_UnInstallSoundUpdateFunction(CPC_SOUND_UPDATE_FUNCTION soundUpdateFunction)
{
	int i;

	for (i = 0; i < NumAudioUpdateFunctions; i++)
	{
		if (audioUpdateFunctions[i] == soundUpdateFunction)
		{
			int Count = ((NumLightSensorFunctions - 1) - i);
			if (Count != 0)
			{
				int j;
				for (j = 0; j < Count; j++)
				{
					audioUpdateFunctions[j] = audioUpdateFunctions[j + 1];
				}
			}
			NumAudioUpdateFunctions--;

			return;
		}
	}
}

/* register/unregister a function that gets called when the printer output changes */
void CPC_InstallJoystickReadFunction(CPC_JOYSTICK_READ_FUNCTION joystickReadFunction)
{
	m_pJoystickReadFunction = joystickReadFunction;
}

void CPC_UnInstallJoystickReadFunction(CPC_JOYSTICK_READ_FUNCTION joystickReadFunction)
{
	m_pJoystickReadFunction = NULL;
}


#define MAX_MEMORY_RANGES 255
static int NumMemoryRanges = 0;
static const MemoryRange *MemoryRanges[MAX_MEMORY_RANGES];

MemoryRange BaseMemoryRange;
MemoryRange DefaultMemoryRange;

const MemoryRange *CPC_GetDefaultMemoryRange(void)
{
	return &DefaultMemoryRange;
}

void CPC_ExecuteCursorOutputFunctions(int nState)
{
	int i;
	for (i = 0; i < NumCursorOutputFunctions; i++)
	{
		cursorOutputFunctions[i](nState);
	}
}


void CPC_ExecuteRetiFunctions(void)
{
	int i;
	for (i = 0; i < NumRetiFunctions; i++)
	{
		retiFunctions[i]();
	}
}


void CPC_ExecuteAckMaskableInterruptHandler(void)
{
	int i;
	for (i = 0; i < NumAckMaskableInterruptFunctions; i++)
	{
		ackMaskableInterruptFunctions[i]();
	}
}

void CPC_ExecuteSoundUpdateFunctions(void)
{
	int i;
	for (i = 0; i < NumAudioUpdateFunctions; i++)
	{
		audioUpdateFunctions[i]();
	}

}

void CPC_ExecuteLightSensorFunctions(int nState)
{
	int i;
	for (i = 0; i < NumLightSensorFunctions; i++)
	{
		lightSensorFunctions[i](nState);
	}
}


/* install a function that gets called when the cursor output from the crtc changes */
void CPC_InstallCursorUpdateFunction(CPC_CURSOR_UPDATE_FUNCTION cursorFunction)
{
	int i;

	for (i = 0; i < NumCursorOutputFunctions; i++)
	{
		if (cursorOutputFunctions[i] == cursorFunction)
		{
			return;
		}
	}
	if (NumCursorOutputFunctions < MAX_DEVICES)
	{
		cursorOutputFunctions[NumCursorOutputFunctions] = cursorFunction;
		NumCursorOutputFunctions++;
	}
}

/* uninstall a function that gets called when the cursor output from the crtc changes */
void CPC_UnInstallCursorUpdateFunction(CPC_CURSOR_UPDATE_FUNCTION cursorFunction)
{
	int i;

	for (i = 0; i < NumCursorOutputFunctions; i++)
	{
		if (cursorOutputFunctions[i] == cursorFunction)
		{
			int Count = ((NumCursorOutputFunctions - 1) - i);
			if (Count != 0)
			{
				int j;
				for (j = 0; j < Count; j++)
				{
					cursorOutputFunctions[j + i] = cursorOutputFunctions[j + i + 1];
				}

			}
			NumCursorOutputFunctions--;

			return;
		}
	}
}

int CPC_GetRegisteredMemoryRangeCount(void)
{
	return NumMemoryRanges;
}
const MemoryRange *CPC_GetRegisteredMemoryRange(int nIndex)
{
	return MemoryRanges[nIndex];
}


void CPC_RegisterMemoryRange(const MemoryRange *pRange)
{
	if (NumMemoryRanges != 0)
	{
		int i;

		for (i = 0; i < NumMemoryRanges; i++)
		{
			if (MemoryRanges[i]->m_nID == pRange->m_nID)
				return;
		}
	}
	if (NumMemoryRanges < MAX_MEMORY_RANGES)
	{
		MemoryRanges[NumMemoryRanges] = pRange;
		NumMemoryRanges++;
	}
}

void CPC_UnRegisterMemoryRange(const MemoryRange *pRange)
{
	if (NumMemoryRanges != 0)
	{
		int i;

		for (i = 0; i < NumMemoryRanges; i++)
		{
			if (MemoryRanges[i]->m_nID == pRange->m_nID)
			{
				int nCount = (NumMemoryRanges - 1) - i;

				if (nCount != 0)
				{
					int j;
					for (j = 0; j < nCount; j++)
					{
						MemoryRanges[j + i] = MemoryRanges[j + i + 1];
					}
				}
				NumMemoryRanges--;
				return;
			}
		}
	}
}


void CPC_ExecuteReadPortFunctions(Z80_WORD Port, Z80_BYTE *pData)
{
	if (NumReadPorts != 0)
	{
		int i;

		for (i = 0; i < NumReadPorts; i++)
		{
			if ((Port&readPorts[i].PortAnd) == readPorts[i].PortCmp)
			{
				/* data read from device */
				Z80_BYTE DeviceData;
				/* if the device asserted the data onto the bus then store it, else ignore it */
				if (readPorts[i].pReadFunction(Port, &DeviceData))
				{
					*pData = DeviceData;
				}
			}
		}
	}
}


void CPC_ExecuteWritePortFunctions(Z80_WORD Port, Z80_BYTE Data)
{
	if (NumWritePorts != 0)
	{
		int i;

		for (i = 0; i < NumWritePorts; i++)
		{
			if ((Port&writePorts[i].PortAnd) == writePorts[i].PortCmp)
			{
				writePorts[i].pWriteFunction(Port, Data);
			}
		}
	}
}



void CPC_ExecuteReadMemoryFunctions(Z80_WORD Port, Z80_BYTE *pData)
{
	if (NumReadMemory != 0)
	{
		int i;

		for (i = 0; i < NumReadMemory; i++)
		{
			if ((Port&readMemorys[i].PortAnd) == readMemorys[i].PortCmp)
			{
				/* data read from device */
				Z80_BYTE DeviceData;
				/* if the device asserted the data onto the bus then store it, else ignore it */
				if (readMemorys[i].pReadFunction(Port, &DeviceData))
				{
					*pData = DeviceData;
				}
			}
		}
	}
}


void CPC_ExecuteWriteMemoryFunctions(Z80_WORD Port, Z80_BYTE Data)
{
	if (NumWriteMemory != 0)
	{
		int i;

		for (i = 0; i < NumWriteMemory; i++)
		{
			if ((Port&writeMemorys[i].PortAnd) == writeMemorys[i].PortCmp)
			{
				writeMemorys[i].pWriteFunction(Port, Data);
			}
		}
	}
}

void CPC_ExecuteUpdateFunctions(void)
{
	int i;
	for (i = 0; i < NumUpdateFunctions; i++)
	{
		updateFunctions[i]();
	}
}

void CPC_ExecutePowerFunctions(void)
{
	int i;
	for (i = 0; i < NumPowerFunctions; i++)
	{
		powerFunctions[i]();
	}
}


void CPC_ExecuteResetFunctions(void)
{
	int i;
	for (i = 0; i < NumResetFunctions; i++)
	{
		resetFunctions[i]();
	}
}


void CPC_InstallResetFunction(CPC_RESET_FUNCTION resetFunction)
{
	int i;

	for (i = 0; i < NumResetFunctions; i++)
	{
		if (resetFunctions[i] == resetFunction)
		{
			return;
		}
	}
	resetFunctions[NumResetFunctions] = resetFunction;
	NumResetFunctions++;
}

void CPC_UnInstallResetFunction(CPC_RESET_FUNCTION resetFunction)
{
	int i;

	for (i = 0; i < NumResetFunctions; i++)
	{
		if (resetFunctions[i] == resetFunction)
		{
			int Count = ((NumResetFunctions - 1) - i);
			if (Count != 0)
			{
				int j;
				for (j = 0; j < Count; j++)
				{
					resetFunctions[j + i] = resetFunctions[j + i + 1];
				}
			}
			NumResetFunctions--;

			return;
		}
	}
}


void CPC_InstallUpdateFunction(CPC_UPDATE_FUNCTION updateFunction)
{
	int i;

	for (i = 0; i < NumUpdateFunctions; i++)
	{
		if (updateFunctions[i] == updateFunction)
		{
			return;
		}
	}
	if (NumUpdateFunctions < MAX_DEVICES)
	{

		updateFunctions[NumUpdateFunctions] = updateFunction;
		NumUpdateFunctions++;
	}
}

void CPC_UnInstallUpdateFunction(CPC_UPDATE_FUNCTION updateFunction)
{
	int i;

	for (i = 0; i < NumUpdateFunctions; i++)
	{
		if (updateFunctions[i] == updateFunction)
		{
			int Count = ((NumUpdateFunctions - 1) - i);
			if (Count != 0)
			{
				int j;
				for (j = 0; j < Count; j++)
				{
					updateFunctions[j + i] = updateFunctions[j + i + 1];
				}
			}
			NumUpdateFunctions--;

			return;
		}
	}
}



void CPC_InstallPowerFunction(CPC_POWER_FUNCTION powerFunction)
{
	int i;

	for (i = 0; i < NumPowerFunctions; i++)
	{
		if (powerFunctions[i] == powerFunction)
		{
			return;
		}
	}
	if (NumPowerFunctions < MAX_DEVICES)
	{
		powerFunctions[NumPowerFunctions] = powerFunction;
		NumPowerFunctions++;
	}
}

void CPC_UnInstallPowerFunction(CPC_POWER_FUNCTION powerFunction)
{
	int i;

	for (i = 0; i < NumPowerFunctions; i++)
	{
		if (powerFunctions[i] == powerFunction)
		{
			int Count = ((NumPowerFunctions - 1) - i);
			if (Count != 0)
			{
				int j;
				for (j = 0; j < Count; j++)
				{
					powerFunctions[j + i] = powerFunctions[j + i + 1];
				}
			}
			NumPowerFunctions--;

			return;
		}
	}
}



void CPC_InstallMemoryRethinkHandler(CPC_MEMORY_RETHINK_FUNCTION memoryFunction)
{
	int i;

	for (i = 0; i < NumMemoryRethinkFunctions; i++)
	{
		if (memoryRethinkFunctions[i] == memoryFunction)
		{
			return;
		}
	}
	if (NumMemoryRethinkFunctions < MAX_DEVICES)
	{
		memoryRethinkFunctions[NumMemoryRethinkFunctions] = memoryFunction;
		NumMemoryRethinkFunctions++;
	}
}

void CPC_UnInstallMemoryRethinkHandler(CPC_MEMORY_RETHINK_FUNCTION memoryFunction)
{
	int i;

	for (i = 0; i < NumMemoryRethinkFunctions; i++)
	{
		if (memoryRethinkFunctions[i] == memoryFunction)
		{
			int Count = ((NumMemoryRethinkFunctions - 1) - i);
			if (Count != 0)
			{
				int j;
				for (j = 0; j < Count; j++)
				{
					memoryRethinkFunctions[j + i] = memoryRethinkFunctions[j + i + 1];
				}
			}
			NumMemoryRethinkFunctions--;

			return;
		}
	}
}


void CPC_InstallRetiHandler(CPC_RETI_FUNCTION retiFunction)
{
	int i;

	for (i = 0; i < NumRetiFunctions; i++)
	{
		if (retiFunctions[i] == retiFunction)
		{
			return;
		}
	}
	if (NumRetiFunctions < MAX_DEVICES)
	{
		retiFunctions[NumRetiFunctions] = retiFunction;
		NumRetiFunctions++;
	}
}

void CPC_UnInstallRetiHandler(CPC_RETI_FUNCTION retiFunction)
{
	int i;

	for (i = 0; i < NumRetiFunctions; i++)
	{
		if (retiFunctions[i] == retiFunction)
		{
			int Count = ((NumRetiFunctions - 1) - i);
			if (Count != 0)
			{
				int j;
				for (j = 0; j < Count; j++)
				{
					retiFunctions[j + i] = retiFunctions[j + i + 1];
				}
			}
			NumRetiFunctions--;

			return;
		}
	}
}


void CPC_InstallAckMaskableInterruptHandler(CPC_ACK_MASKABLE_INTERRUPT_FUNCTION ackFunction)
{
	int i;

	for (i = 0; i < NumAckMaskableInterruptFunctions; i++)
	{
		if (ackMaskableInterruptFunctions[i] == ackFunction)
		{
			return;
		}
	}
	if (NumAckMaskableInterruptFunctions < MAX_DEVICES)
	{
		ackMaskableInterruptFunctions[NumAckMaskableInterruptFunctions] = ackFunction;
		NumAckMaskableInterruptFunctions++;
	}
}

void CPC_UnInstallAckMaskableInterruptHandler(CPC_ACK_MASKABLE_INTERRUPT_FUNCTION ackFunction)
{
	int i;

	for (i = 0; i < NumAckMaskableInterruptFunctions; i++)
	{
		if (ackMaskableInterruptFunctions[i] == ackFunction)
		{
			int Count = ((NumAckMaskableInterruptFunctions - 1) - i);
			if (Count != 0)
			{
				int j;
				for (j = 0; j < Count; j++)
				{
					ackMaskableInterruptFunctions[j + i] = ackMaskableInterruptFunctions[j + i + 1];
				}

			}
			NumAckMaskableInterruptFunctions--;

			return;
		}
	}
}



void CPC_InstallReadPort(CPCPortRead *readPort)
{
	int i;
	/* does it exist already? */
	for (i = 0; i < NumReadPorts; i++)
	{
		if (memcmp(&readPorts[i], readPort, sizeof(CPCPortRead)) == 0)
		{
			return;
		}
	}
	if (NumReadPorts < MAX_DEVICES)
	{
		/* no, add it */
		memcpy(&readPorts[NumReadPorts], readPort, sizeof(CPCPortRead));
		NumReadPorts++;
	}
}

void CPC_InstallWritePort(CPCPortWrite *writePort)
{
	int i;
	/* does it exist already? */
	for (i = 0; i < NumWritePorts; i++)
	{
		if (memcmp(&writePorts[i], writePort, sizeof(CPCPortWrite)) == 0)
		{
			return;
		}
	}
	if (NumWritePorts < MAX_DEVICES)
	{
		/* no, add it */
		memcpy(&writePorts[NumWritePorts], writePort, sizeof(CPCPortWrite));
		NumWritePorts++;
	}
}

void CPC_UninstallWritePort(CPCPortWrite *writePort)
{
	int i;
	for (i = 0; i < NumWritePorts; i++)
	{
		/* found it */
		if (memcmp(&writePorts[i], writePort, sizeof(CPCPortWrite)) == 0)
		{
			/* if ports=3, and i=2, we don't copy anything */

			int DataSize = sizeof(CPCPortWrite)*((NumWritePorts - 1) - i);
			if (DataSize != 0)
			{
				/* remove! */
				memcpy(&writePorts[i], &writePorts[i + 1], DataSize);
			}
			NumWritePorts--;

			return;
		}
	}
}


void CPC_UninstallReadPort(CPCPortRead *readPort)
{
	int i;
	for (i = 0; i < NumReadPorts; i++)
	{
		/* found it */
		if (memcmp(&readPorts[i], readPort, sizeof(CPCPortRead)) == 0)
		{
			/* if ports=3, and i=2, we don't copy anything */

			int DataSize = sizeof(CPCPortRead)*((NumReadPorts - 1) - i);
			if (DataSize != 0)
			{
				/* remove! */
				memcpy(&readPorts[i], &readPorts[i + 1], DataSize);
			}
			NumReadPorts--;

			return;
		}
	}
}




void CPC_InstallReadMemory(CPCPortRead *readMemory)
{
	int i;
	/* does it exist already? */
	for (i = 0; i < NumReadMemory; i++)
	{
		if (memcmp(&readMemorys[i], readMemory, sizeof(CPCPortRead)) == 0)
		{
			return;
		}
	}
	if (NumReadMemory < MAX_DEVICES)
	{
		/* no, add it */
		memcpy(&readMemorys[NumReadMemory], readMemory, sizeof(CPCPortRead));
		NumReadMemory++;
	}
}

void CPC_InstallWriteMemory(CPCPortWrite *writeMemory)
{
	int i;
	/* does it exist already? */
	for (i = 0; i < NumWriteMemory; i++)
	{
		if (memcmp(&writeMemorys[i], writeMemory, sizeof(CPCPortWrite)) == 0)
		{
			return;
		}
	}
	if (NumWriteMemory < MAX_DEVICES)
	{
		/* no, add it */
		memcpy(&writeMemorys[NumWriteMemory], writeMemory, sizeof(CPCPortWrite));
		NumWriteMemory++;
	}
}

void CPC_UninstallWriteMemory(CPCPortWrite *writeMemory)
{
	int i;
	for (i = 0; i < NumWriteMemory; i++)
	{
		/* found it */
		if (memcmp(&writeMemorys[i], writeMemory, sizeof(CPCPortWrite)) == 0)
		{
			/* if ports=3, and i=2, we don't copy anything */

			int DataSize = sizeof(CPCPortWrite)*((NumWriteMemory - 1) - i);
			if (DataSize != 0)
			{
				/* remove! */
				memcpy(&writeMemorys[i], &writeMemory[i + 1], DataSize);
			}
			NumWriteMemory--;

			return;
		}
	}
}


void CPC_UninstallReadMemory(CPCPortRead *readMemory)
{
	int i;
	for (i = 0; i < NumReadMemory; i++)
	{
		/* found it */
		if (memcmp(&readMemorys[i], readMemory, sizeof(CPCPortRead)) == 0)
		{
			int DataSize = sizeof(CPCPortRead)*((NumReadMemory - 1) - i);
			if (DataSize != 0)
			{
				/* remove! */
				memcpy(&readMemorys[i], &readMemory[i + 1], DataSize);
			}
			NumReadMemory--;

			return;
		}
	}
}



static unsigned long   NopCount = 0;

/*********************************************************************************/
/* override on-board roms */
static unsigned char OSOverrideROM[16384];
static BOOL OSOverrideROMEnabled = FALSE;
static unsigned char BasicOverrideROM[16384];
static BOOL BASICOverrideROMEnabled = FALSE;
static unsigned char AmsdosOverrideROM[16384];
static BOOL AmsdosOverrideROMEnabled = FALSE;

void CPC_SetOSOverrideROMEnable(BOOL bState)
{
	OSOverrideROMEnabled = bState;

	/* refresh language */
	Keyboard_DetectLanguage();
}


BOOL CPC_GetOSOverrideROMEnable(void)
{
	return OSOverrideROMEnabled;
}

void CPC_SetOSOverrideROM(const unsigned char *pOSRom, unsigned long Length)
{
	const unsigned char *pRomData = pOSRom;
	unsigned long RomLength = Length;

	AMSDOS_GetUseableSize(&pRomData, &RomLength);

	EmuDevice_CopyRomData(OSOverrideROM, sizeof(OSOverrideROM), pRomData, RomLength);

	/* refresh language */
	Keyboard_DetectLanguage();
}

const unsigned char *CPC_GetOSOverrideROM(void)
{
	return OSOverrideROM;
}


const unsigned char *CPC_GetBASICOverrideROM(void)
{
	return BasicOverrideROM;
}

void CPC_SetBASICOverrideROM(const unsigned char *pBASICROM, unsigned long Length)
{
	const unsigned char *pRomData = pBASICROM;
	unsigned long RomLength = Length;

	AMSDOS_GetUseableSize(&pRomData, &RomLength);

	EmuDevice_CopyRomData(BasicOverrideROM, sizeof(BasicOverrideROM), pRomData, RomLength);
}

const unsigned char *CPC_GetAmsdosOverrideROM(void)
{
	return AmsdosOverrideROM;
}

void CPC_SetAmsdosOverrideROM(const unsigned char *pAmsdosROM, unsigned long Length)
{
	const unsigned char *pRomData = pAmsdosROM;
	unsigned long RomLength = Length;

	AMSDOS_GetUseableSize(&pRomData, &RomLength);

	EmuDevice_CopyRomData(AmsdosOverrideROM, sizeof(AmsdosOverrideROM), pRomData, RomLength);
}


void CPC_SetBASICOverrideROMEnable(BOOL bState)
{
	BASICOverrideROMEnabled = bState;
}


BOOL CPC_GetBASICOverrideROMEnable(void)
{
	return BASICOverrideROMEnabled;
}


void CPC_SetAmsdosOverrideROMEnable(BOOL bState)
{
	AmsdosOverrideROMEnabled = bState;
}


BOOL CPC_GetAmsdosOverrideROMEnable(void)
{
	return AmsdosOverrideROMEnabled;
}

/*********************************************************************************/
/* On board roms - CPC */

/* basic rom */
static const unsigned char *pBasic;

/* os rom */
static const unsigned char *pOS;


void CPC_SetOSRom(const unsigned char *pOSRom)
{
	pOS = pOSRom;
}

const unsigned char *CPC_GetOSROM(void)
{
	return pOS;
}

const unsigned char *CPC_GetBASICROM(void)
{
	return pBasic;
}

void CPC_SetBASICRom(const unsigned char *pBASICROM)
{
	pBasic = pBASICROM;
}


/********************/

/* keyboard data */
int              SelectedKeyboardLine;
static BOOL				KeyboardScanned;

int Keyboard_GetSelectedLine(void)
{
	return SelectedKeyboardLine;
}

/* final result */
unsigned char KeyboardData[16] =
{
	0x0ff, 0x0ff, 0x0ff, 0x0ff,
	0x0ff, 0x0ff, 0x0ff, 0x0ff,
	0x0ff, 0x0ff, 0x0ff, 0x0ff,
	0x0ff, 0x0ff, 0x0ff, 0x0ff,
};

/* the result from keyboard pressing keys (excludes keyjoy or other inputs that can trigger keys) */
unsigned char RealKeyboardData[16] =
{
	0x0ff, 0x0ff, 0x0ff, 0x0ff,
	0x0ff, 0x0ff, 0x0ff, 0x0ff,
	0x0ff, 0x0ff, 0x0ff, 0x0ff,
	0x0ff, 0x0ff, 0x0ff, 0x0ff,
};

/* before resolving; clash is applied after to generate keyboard data */
unsigned char ResolvedKeys[16] =
{
	0x0ff, 0x0ff, 0x0ff, 0x0ff,
	0x0ff, 0x0ff, 0x0ff, 0x0ff,
	0x0ff, 0x0ff, 0x0ff, 0x0ff,
	0x0ff, 0x0ff, 0x0ff, 0x0ff,
};

void CPC_WriteMemory(Z80_WORD, Z80_BYTE);

extern  unsigned char *Z80MemoryBase;

static int SensorX = 0;
static int SensorY = 0;

void CPC_SetLightSensorPos(int x, int y)
{
	SensorX = x;
	SensorY = y;
}

void CPC_UpdateLightSensor(void)
{
#if 0
	// determine if the sensor is near the pos; and if it is trigger the function
	int monitorx;
	int monitory;

	BOOL bState = FALSE;

	if ((xpos==monitorx) && (ypos==monitory))
	{
		bState = TRUE;
	}
#endif
	BOOL bState = FALSE;
	CPC_ExecuteLightSensorFunctions(bState);

}

BOOL Keyboard_HasBeenScanned(void)
{
	return KeyboardScanned;
}

void Keyboard_ResetHasBeenScanned(void)
{
	KeyboardScanned = FALSE;
}

uint32_t EnglishOSROMCRC[7] =
{
	0x056b2960d,	// 6128 English
	0x0d5fc7fa6,	// 464 English,
	0x06bf140bd, // 664 English,
	0x02b319e8e, // KC Compact 
	0x0ea6f7bc8, // Plus English
	0x04c294fb3,	// Aleste
	0x0d8e99fe8,	// fw3.12 English
};
uint32_t FrenchOSROMCRC[4] =
{
	0x099134a79,	// 6128 French
	0x0d3e4fdb8,	// 464 French
	0x0f8923158,	// Plus French
	0x0684772be,	//fw3.12 French
};
uint32_t SpanishOSROMCRC[4] =
{
	0x0fd38520c,	// 6128 Spanish
	0x05d598652,	// 464 Spanish
	0x06fac2755, // Plus Spanish
	0x0728d8587,	// Fw3.12 Spanish
};
uint32_t DanishOSROMCRC[2] =
{
	0x0c9d26cf0,		// 464 Danish
	0x0ce5bb6e6		// 6128 Danish
};

BOOL Keyboard_LanguageFound(uint32_t *pList, int nItems, uint32_t CRC)
{
	size_t i;
	for (i = 0; i < nItems; i++)
	{
		if (pList[i] == CRC)
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL KeyboardAutoDetectLanguage = TRUE;

BOOL Keyboard_GetAutoDetectLanguage(void)
{
	return KeyboardAutoDetectLanguage;
}
void Keyboard_SetAutoDetectLanguage(BOOL bState)
{
	KeyboardAutoDetectLanguage = bState;
}

extern uint32_t crc32(uint32_t, const void *, size_t);

BOOL Keyboard_DetectLanguage(void)
{
	BOOL bDetected = FALSE;
	const unsigned char *pRomData = NULL;

	if (!KeyboardAutoDetectLanguage)
		return TRUE;

	if (CPC_GetHardware() == CPC_HW_CPCPLUS)
	{
		pRomData = ASIC_GetCartPage(0);
	}
	else if (CPC_GetHardware() == CPC_HW_CPC)
	{
		pRomData = pOS;
	}
	else if (CPC_GetHardware() == CPC_HW_KCCOMPACT)
	{
		pRomData = KCC_GetOSRom();
	}
	else if (CPC_GetHardware() == CPC_HW_ALESTE)
	{
		pRomData = Aleste_GetOSRom();
	}

	/* override not for plus?? */

	/* override enabled? */
	if (CPC_GetOSOverrideROMEnable())
	{
		/* override rom set? */
		pRomData = CPC_GetOSOverrideROM();
	}

	if (pRomData != NULL)
	{
		uint32_t ComputedCRC;
		int LanguageID = KEYBOARD_LANGUAGE_ID_UNKNOWN;

		ComputedCRC = crc32(-1, pRomData, 16384);
		//	printf("0x%08x\n", ComputedCRC);

		/* compute CRC */
		if (Keyboard_LanguageFound(EnglishOSROMCRC, sizeof(EnglishOSROMCRC) / sizeof(EnglishOSROMCRC[0]), ComputedCRC))
		{
			bDetected = TRUE;
			LanguageID = KEYBOARD_LANGUAGE_ID_ENGLISH;
		}
		else if (Keyboard_LanguageFound(FrenchOSROMCRC, sizeof(FrenchOSROMCRC) / sizeof(FrenchOSROMCRC[0]), ComputedCRC))
		{
			bDetected = TRUE;
			LanguageID = KEYBOARD_LANGUAGE_ID_FRENCH;
		}
		else if (Keyboard_LanguageFound(SpanishOSROMCRC, sizeof(SpanishOSROMCRC) / sizeof(SpanishOSROMCRC[0]), ComputedCRC))
		{
			bDetected = TRUE;
			LanguageID = KEYBOARD_LANGUAGE_ID_SPANISH;
		}
		else if (Keyboard_LanguageFound(DanishOSROMCRC, sizeof(DanishOSROMCRC) / sizeof(DanishOSROMCRC[0]), ComputedCRC))
		{
			bDetected = TRUE;
			LanguageID = KEYBOARD_LANGUAGE_ID_DANISH;
		}
		if (bDetected)
		{
			Keyboard_SetLanguage(LanguageID);
		}
	}
	return bDetected;
}

uint32_t Firmware16Roms[13] =
{
	0x056b2960d,	// 6128 English
	0x06bf140bd, // 664 English,
	0x02b319e8e, // KC Compact 
	0x0ea6f7bc8, // Plus English
	0x04c294fb3,	// Aleste
	0x0d8e99fe8,	// fw3.12 English
	0x099134a79,	// 6128 French
	0x0f8923158,	// Plus French
	0x0684772be,	//fw3.12 French
	0x0fd38520c,	// 6128 Spanish
	0x06fac2755, // Plus Spanish
	0x0728d8587,	// Fw3.12 Spanish
	0x0ce5bb6e6		// 6128 Danish
};


BOOL Firmware16AutoDetect = TRUE;
BOOL FirmwareSupports16Roms = TRUE;

BOOL Firmware_GetAutoDetect16Roms(void)
{
	return Firmware16AutoDetect;
}
void Firmware_SetAutoDetect16Roms(BOOL bState)
{
	Firmware16AutoDetect = bState;
}

BOOL Firmware_GetSupports16Roms(void)
{
	return FirmwareSupports16Roms;
}

void Firmware_SetSupports16Roms(BOOL bState)
{
	FirmwareSupports16Roms = bState;
}

BOOL Firmware_Detect16Roms(void)
{
	const unsigned char *pRomData = NULL;

	if (!Firmware16AutoDetect)
		return Firmware_GetSupports16Roms();

	if (CPC_GetHardware() == CPC_HW_CPCPLUS)
	{
		pRomData = ASIC_GetCartPage(0);
	}
	else if (CPC_GetHardware() == CPC_HW_CPC)
	{
		pRomData = pOS;
	}
	else if (CPC_GetHardware() == CPC_HW_KCCOMPACT)
	{
		pRomData = KCC_GetOSRom();
	}
	else if (CPC_GetHardware() == CPC_HW_ALESTE)
	{
		pRomData = Aleste_GetOSRom();
	}

	/* override not for plus?? */

	/* override enabled? */
	if (CPC_GetOSOverrideROMEnable())
	{
		/* override rom set? */
		pRomData = CPC_GetOSOverrideROM();
	}

	if (pRomData != NULL)
	{
		int i;
		uint32_t ComputedCRC;

		ComputedCRC = crc32(-1, pRomData, 16384);

		for (i = 0; i < sizeof(Firmware16Roms) / sizeof(Firmware16Roms[0]); i++)
		{
			if (Firmware16Roms[i] == ComputedCRC)
				return TRUE;
		}
	}

	return FALSE;
}


void Printer_RefreshOutputs(void)
{
	if (m_pPrinterUpdateFunction)
		m_pPrinterUpdateFunction();
}

unsigned char Keyboard_GetRealLine(int nLine)
{
	if (nLine > 9)
		return 0x0ff;

	return RealKeyboardData[nLine];
}

/* function is also used by debugger */
unsigned char Keyboard_GetLine(int nLine)
{
	if (nLine > 9)
		return 0x0ff;

	return KeyboardData[nLine];
}

/* function used by emulation */
unsigned char Keyboard_Read(void)
{
	KeyboardScanned = TRUE;
	return Keyboard_GetLine(SelectedKeyboardLine);
}

/*--------------------------------------------------------------*/
void    CPC_ClearKeyboard(void)
{
	/* set all keys to not be pressed */
	memset(KeyboardData, 0x0ff, 16);
	memset(RealKeyboardData, 0x0ff, 16);
	memset(ResolvedKeys, 0x0ff, 16);
	/* need to do?? reset key lines comming from keyboard and joystick */
	Joystick_KeyboardLine_Reset();
}

/*--------------------------------------------------------------*/
void CPC_SetKeyInternal(int KeyID)
{
	if (KeyID != CPC_KEY_NULL)
	{
		int Line = KeyID >> 3;
		int Bit = KeyID & 0x07;

		RealKeyboardData[Line] &= ~(1 << Bit);
	}
}
/*--------------------------------------------------------------*/
void CPC_PreResolveKeys(void)
{
	memcpy(ResolvedKeys, RealKeyboardData, sizeof(RealKeyboardData));
}

void CPC_ResolveKeys(unsigned char *pKeyboardData)
{
	int i;
	/* if RealKeyboardData has it pressed, keep it pressed.
	If pKeyboardData has it pressed, make it pressed.
	If it's released in either, then leave it released */
	/* Key is 0 when pressed, so if we do an AND this will do it */

	for (i = 0; i < (CPC_KEY_NUM_KEYS / 8); i++)
	{
		ResolvedKeys[i] &= pKeyboardData[i];
	}
}

void CPC_ClearKeyInternal(int KeyID)
{
	if (KeyID != CPC_KEY_NULL)
	{
		int Line = KeyID >> 3;
		int Bit = KeyID & 0x07;

		RealKeyboardData[Line] |= (1 << Bit);
	}
}

/* this should be a filter!. e.g. if we switch from allowed to not allowed, there will be errors, but if we had
it as a final resolve then it will always work */
BOOL CPC_KeyAllowed(int KeyID)
{
	if (KeyID != CPC_KEY_NULL)
	{
		int Line = KeyID >> 3;
		int Bit = KeyID & 0x07;

		if (CPC_GetHardware() == CPC_HW_CPCPLUS)
		{
			/* filter keys based on GX4000 */
			if (ASIC_GetGX4000())
			{
				switch (Line)
				{
					/* these lines are not connected */
				case 0:
				case 1:
				case 2:
				case 4:
				case 5:
				case 7:
				case 8:
					return FALSE;

					/* only allow P pressed */
				case 3:
				{
					/* allow P */
					if (Bit != 3)
					{
						return FALSE;
					}
				}
				break;

				/* check joysticks!!! */
				/* do not allow bit 6 or 7 */
				case 6:
				case 9:
				{
					if ((Bit == 7) || (Bit == 6))
					{
						return FALSE;
					}
				}
				break;
				}
			}
		}

		return TRUE;
	}
	return FALSE;
}

/*--------------------------------------------------------------*/
void CPC_SetKey(int KeyID)
{
	if (CPC_KeyAllowed(KeyID))
	{
		/* store state of key press on joystick lines */

		CPC_SetKeyInternal(KeyID);
	}
}
/*--------------------------------------------------------------*/
void CPC_ClearKey(int KeyID)
{
	if (CPC_KeyAllowed(KeyID))
	{

		CPC_ClearKeyInternal(KeyID);
	}
}
/*--------------------------------------------------------------*/


/* On Plus:

* Joysticks don't clash with each other
* Joysticks don't clash with keyboard
* Keyboard clashes with itself

On CPC:

* Joysticks clash with each other
* Joysticks clash with keyboard
* Keyboard clashes with itself

TODO:
* Check clash on KC Compact
*/

/* This simulates the keyboard clash that is seen on a real CPC */
void       CPC_GenerateKeyboardClash(void)
{
	int i, j;

	if (!Keyboard_IsClashEnabled())
	{
		/* My CPC664 doesn't appear to suffer from keyboard clash */
		memcpy(KeyboardData, ResolvedKeys, 16);

		if (m_pJoystickReadFunction)
		{
			unsigned char Data;
			Data = m_pJoystickReadFunction(0);
			KeyboardData[9] = Joystick_KeyboardLine_Refresh3(0, KeyboardData[9], Data);
			Data = m_pJoystickReadFunction(1);
			KeyboardData[6] = Joystick_KeyboardLine_Refresh3(1, KeyboardData[6], Data);
		}
		else
		{
			/* setup keyboard based on keys pressed and joystick, and allow them to clash */
			KeyboardData[9] = Joystick_KeyboardLine_Refresh2(0, KeyboardData[9]);
			KeyboardData[6] = Joystick_KeyboardLine_Refresh2(1, KeyboardData[6]);
		}

		return;
	}

	/* work out clash based on keys pressed (and joystick on cpc) */
	for (i = 0; i < 16; i++)
	{
		unsigned char Line1;

		Line1 = ResolvedKeys[i];

		/* if not plus, mix joystick with keyboard */
		if (CPC_GetHardware() != CPC_HW_CPCPLUS)
		{
			if (i == 9)
			{
				if (m_pJoystickReadFunction)
				{
					unsigned char Data = m_pJoystickReadFunction(0);
					Line1 = Joystick_KeyboardLine_Refresh3(0, Line1, Data);
				}
				else
				{
					Line1 = Joystick_KeyboardLine_Refresh2(0, Line1);
				}
			}
			else if (i == 6)
			{
				if (m_pJoystickReadFunction)
				{
					unsigned char Data = m_pJoystickReadFunction(1);
					Line1 = Joystick_KeyboardLine_Refresh3(1, Line1, Data);
				}
				else
				{
					Line1 = Joystick_KeyboardLine_Refresh2(1, Line1);
				}
			}
		}

		KeyboardData[i] = Line1;

		if (Line1 != 0x0ff)
		{
			/* key(s) pressed in this row */

			for (j = 0; j < 16; j++)
			{

				if (i != j)
				{

					unsigned char Line2;

					Line2 = ResolvedKeys[j];

					/* if not plus, mix joystick with keyboard */
					if (CPC_GetHardware() != CPC_HW_CPCPLUS)
					{
						if (i == 9)
						{
							if (m_pJoystickReadFunction)
							{
								unsigned char Data = m_pJoystickReadFunction(0);
								Line2 = Joystick_KeyboardLine_Refresh3(0, Line2, Data);
							}
							else
							{

								Line2 = Joystick_KeyboardLine_Refresh2(0, Line2);
							}
						}
						else if (i == 6)
						{
							if (m_pJoystickReadFunction)
							{
								unsigned char Data = m_pJoystickReadFunction(1);
								Line2 = Joystick_KeyboardLine_Refresh3(1, Line2, Data);
							}
							else
							{

								Line2 = Joystick_KeyboardLine_Refresh2(1, Line2);
							}
						}
					}


					if (Line2 != 0x0ff)
					{
						/* keys pressed in this row also */
						if ((Line1 | Line2) != 0x0ff)
						{
							/* common key(s) pressed in these two lines */

							/* update this line to ensure ghost keys are pressed */
							Line1 = Line1 & (~(Line1 ^ Line2));

						}
					}
				}
			}

			KeyboardData[i] = Line1;
		}
	}

	/* for plus, we now want to apply the joystick values to the final keyboard data to avoid the clash */
	if (CPC_GetHardware() == CPC_HW_CPCPLUS)
	{
		if (m_pJoystickReadFunction)
		{
			unsigned char Data;
			Data = m_pJoystickReadFunction(0);
			KeyboardData[9] = Joystick_KeyboardLine_Refresh3(0, KeyboardData[9], Data);
			Data = m_pJoystickReadFunction(1);
			KeyboardData[6] = Joystick_KeyboardLine_Refresh3(1, KeyboardData[6], Data);
		}
		else
		{
			/* apply after clash */
			KeyboardData[9] = Joystick_KeyboardLine_Refresh2(0, KeyboardData[9]);
			KeyboardData[6] = Joystick_KeyboardLine_Refresh2(1, KeyboardData[6]);
		}
	}

}

static BOOL Keyboard_ClashEnabled = TRUE; /* clash on cpc464, cpc6128*/

BOOL Keyboard_IsClashEnabled(void)
{
	return Keyboard_ClashEnabled;
}

void Keyboard_EnableClash(BOOL bState)
{
	Keyboard_ClashEnabled = bState;

}

/*--------------------------------------------------------------*/
/* COMMON ROM HELPER FUNCTIONS */

/* get a word of data from the expansion rom selected */
unsigned short ExpansionRom_GetWord(const unsigned char *pAddr, int Offset)
{
	unsigned short WordData;

	WordData = (unsigned short)((pAddr[(Offset & 0x03fff)]) | ((pAddr[(Offset + 1) & 0x03fff]) << 8));

	return WordData;
}

/* check address lies within expansion rom address space */
BOOL    ExpansionRom_CheckRomAddrValid(unsigned short Addr)
{
	/* if addr is not in range 0x0c000-0x0ffff then addr
	is invalid */
	if ((Addr & 0x0c000) != 0x0c000)
	{
		return FALSE;
	}

	return TRUE;
}

/* Byte 0: ROM Type
				0 = Foreground, 1 = Background, 2 = Extension
				Byte 1: ROM Mark
				Byte 2: ROM Version
				Byte 3: ROM Modification
				Byte 4,5: External Name Table */

BOOL    ExpansionRom_Validate(const unsigned char *pData, unsigned long DataSize)
{
	unsigned char RomType = (unsigned char)(pData[0] & 0x07f);
	unsigned short NameTableAddr;


	/* not a foreground, background or extension rom */
	if ((RomType != 0) && (RomType != 1) && (RomType != 2))
	{
		return FALSE;
	}

	/* rom size must be >0k */
	/* it can be greater than 16384 bytes but will be cropped */
	if (DataSize == 0)
	{
		return FALSE;
	}

	/* get name table addr */
	NameTableAddr = ExpansionRom_GetWord(pData, 4);

	/* check it is valid */
	if (ExpansionRom_CheckRomAddrValid(NameTableAddr))
	{
		/* it is valid */
		const unsigned char   *pNames = pData + (NameTableAddr & 0x03fff);
		int             NameCount = 0;

		while (pNames[0] != 0x00)
		{
			/* the last character in each name has bit 7 set */
			if (pNames[0] & 0x080)
			{
				NameCount++;
			}

			/* a 0 indicates the end of the name table */
			if (pNames[0] != 0x00)
			{
				pNames++;

				/* if we were reading through rom data and ran out of
				space the name table is damaged */
				if (pNames >= (pData + 0x04000))
				{
					return FALSE;
				}
			}
		}

		/* must have at least one name = name for startup of rom */
		if (NameCount == 0)
		{
			return FALSE;
		}
	}

	return TRUE;
}
/*--------------------------------------------------------------*/
static int Keyboard_Mode = 0;

void Keyboard_SetMode(int nMode)
{
	Keyboard_Mode = nMode;
}

int Keyboard_GetMode(void)
{
	return Keyboard_Mode;
}


static int Keyboard_PositionalSet = 0;

void Keyboard_SetPositionalSet(int nSet)
{
	Keyboard_PositionalSet = nSet;
}

int Keyboard_GetPositionalSet(void)
{
	return Keyboard_PositionalSet;
}

static int Keyboard_Language = KEYBOARD_LANGUAGE_ID_ENGLISH;

void Keyboard_SetLanguage(int nLanguage)
{
	Keyboard_Language = nLanguage;
}

int Keyboard_GetLanguage(void)
{
	return Keyboard_Language;
}

/*--------------------------------------------------------------*/

static int ComputerNameIndex = PPI_COMPUTER_NAME_AMSTRAD;

/*--------------------------------------------------------------*/

static BOOL Link50Hz = TRUE;

/*--------------------------------------------------------------*/

static BOOL ExpLow = TRUE; /* TODO: this is on the floppy disc interface */

/*--------------------------------------------------------------*/

void	CPC_Set50Hz(BOOL fState)
{
	Link50Hz = fState;
}

/*--------------------------------------------------------------*/

BOOL	CPC_Get50Hz(void)
{
	return Link50Hz;
}
/*--------------------------------------------------------------*/

void	CPC_SetExpLow(BOOL bExp)
{
	ExpLow = bExp;
}
/*--------------------------------------------------------------*/

BOOL	CPC_GetExpLow(void)
{
	return ExpLow;
}

/*--------------------------------------------------------------*/

void      CPC_SetComputerNameIndex(int Index)
{
	if ((Index < 0) || (Index>7))
	{
		Index = 7;
	}

	ComputerNameIndex = Index;
}

/*--------------------------------------------------------------*/

int CPC_GetComputerNameIndex(void)
{
	return ComputerNameIndex;
}


/*--------------------------------------------------------------*/

int CPC_PPI_GetPortInput(int nPort)
{
	switch (nPort)
	{
	case 0:
		return PSG_Read(&OnBoardAY);

	case 1:
	{
		unsigned char Data = 0;

		/* set computer name */
		Data |= ((CPC_GetComputerNameIndex() & 0x07) << 1);

		/* set screen refresh */
		if (CPC_Get50Hz())
		{
			Data |= PPI_SCREEN_REFRESH_50HZ;
		}

		if (!CPC_GetExpLow())
		{
			Data |= PPI_EXPANSION_PORT;
		}

		if (Printer_GetBusyState())
		{
			Data |= PPI_CENTRONICS_BUSY;
		}

		/* set state of vsync bit */
		if (CRTC_GetVsyncOutput() != 0)
		{
			Data |= (1 << 0);
		}

		if (Computer_GetTapeRead())
		{

			Data |= (1 << 7);
		}
		return Data;
	}
	break;

	case 2:
		/* tested on cpc6128 */
		return 0x02f;

	}
	return 0x0ff;
}

/*--------------------------------------------------------------*/

int PPI_GetPortInput(int nPort)
{
	/* for computers with real 8255 */
	switch (CPC_GetHardware())
	{
	case CPC_HW_CPC:
		return CPC_PPI_GetPortInput(nPort);

	case CPC_HW_KCCOMPACT:
		return KCC_PPI_Port_Read(nPort);

	case CPC_HW_ALESTE:
		return Aleste_PPI_Port_Read(nPort);
	}

	return 0x0ff;
}

/*--------------------------------------------------------------*/


static unsigned char TapeWriteOutput = 0x00;

unsigned char Computer_GetTapeVolume(void)
{
	if (TapeWriteOutput != 0)
	{
		return 0x020;
	}
	else
	{
		return 0x000;
	}
}


static BOOL bTapeMotor = FALSE;

/*--------------------------------------------------------------*/

BOOL Computer_GetTapeMotorOutput(void)
{
	return bTapeMotor;
}

/*--------------------------------------------------------------*/

void Computer_SetTapeMotor(BOOL bState)
{
	bTapeMotor = bState;
}

/*--------------------------------------------------------------*/

int Computer_GetTapeRead(void)
{
	return Cassette_Read();
}

void Computer_SetTapeWrite(unsigned char DataBit)
{
	TapeWriteOutput = (DataBit & (1 << 5));
}


/*--------------------------------------------------------------*/

void CPC_PPI_SetPortOutput(int nPort, int Data)
{
	switch (nPort)
	{

	case 0:
		PSG_Write(&OnBoardAY, Data);
		break;
	case 1:
		Computer_UpdateVsync();
		break;

	case 2:
	{

		/* bits 3..0 are keyboard */
		SelectedKeyboardLine = Data & 0x0f;

		/* bit 5 is tape write */
		Computer_SetTapeWrite((Data & (1 << 5)));

		/* Happy computer printer modification */
		/* Bit 7 of printer data is cassette write */
		/*Printer_SetDataBit7State(Data & (1<<5)); */


		/* bit 4 is tape motor */
		Computer_SetTapeMotor((Data & (1 << 4)) ? TRUE : FALSE);

		/* bit 6,7 are PSG control */
		PSG_SetBDIRState(&OnBoardAY, Data & (1 << 7));
		PSG_SetBC1State(&OnBoardAY, Data & (1 << 6));
		PSG_RefreshState(&OnBoardAY);

		PSG_Write(&OnBoardAY, PPI_GetOutputPort(0));

	}
	break;
	}
}

/*--------------------------------------------------------------*/

void PPI_SetPortOutput(int nPort, int Data)
{
	/* for computers with *real* 8255 */
	switch (CPC_GetHardware())
	{
	case CPC_HW_CPC:
	{
		CPC_PPI_SetPortOutput(nPort, Data);
	}
	break;

	case CPC_HW_KCCOMPACT:
	{
		KCC_PPI_Port_Write(nPort, Data);
	}
	break;

	case CPC_HW_ALESTE:
	{
		Aleste_PPI_Port_Write(nPort, Data);
	}
	break;

	default:
		break;
	}

}


/*------------------------------------------------------------------------*/

int CPC_PSG_GetPortInputs(int Port)
{
	if (Port == 0)
	{
		return Keyboard_Read();
	}

	return 0x0ff;
}

/*------------------------------------------------------------------------*/

int PSG_GetPortInputs(AY_3_8912 *ay, int Port)
{
	if (ay != &OnBoardAY)
		return 0x0ff;

	switch (CPC_GetHardware())
	{
	case CPC_HW_CPC:
		return CPC_PSG_GetPortInputs(Port);

	case CPC_HW_CPCPLUS:
		return ASIC_PSG_GetPortInputs(Port);

	case CPC_HW_KCCOMPACT:
		return KCC_PSG_GetPortInputs(Port);

	case CPC_HW_ALESTE:
		return Aleste_PSG_GetPortInputs(Port);
	}

	return 0x0ff;
}

/*------------------------------------------------------------------------*/

void PSG_SetPortOutputs(AY_3_8912 *ay, int Port, int Data)
{
	if (ay != &OnBoardAY)
		return;
	if (CPC_GetHardware() == CPC_HW_ALESTE)
	{
		Aleste_PSG_SetPortOutputs(Port, Data);
	}
}

/*--------------------------------------------------------------------------*/

static CPC_MONITOR_TYPE_ID CurrentMonitorType;

void    CPC_SetMonitorType(CPC_MONITOR_TYPE_ID MonitorType)
{
	/* force value to be valid */
	if ((MonitorType != CPC_MONITOR_COLOUR) && (MonitorType != CPC_MONITOR_GT64)
		&& (MonitorType != CPC_MONITOR_MM12))
	{
		MonitorType = CPC_MONITOR_COLOUR;
	}

	/* set monitor type */
	CurrentMonitorType = MonitorType;

	switch (CPC_GetHardware())
	{
	default:
	case CPC_HW_CPC:
	{
		/* update gate array/CPC colours */
		GateArray_SetMonitorColourMode(CurrentMonitorType);
	}
	break;

	case CPC_HW_CPCPLUS:
	{
		/* update ASIC/Plus colours */
		ASIC_SetMonitorColourMode(CurrentMonitorType);
	}
	break;

	case CPC_HW_ALESTE:
	{
		Aleste_SetMonitorColourMode(CurrentMonitorType);
	}
	break;

	case CPC_HW_KCCOMPACT:
	{
		KCC_SetMonitorColourMode(CurrentMonitorType);
	}
	break;
	}
}

CPC_MONITOR_TYPE_ID CPC_GetMonitorType(void)
{
	return CurrentMonitorType;
}


/*-----------------------------------------------------------------------*/

static int CurrentCRTCType;

/* set the CRTC emulation type */
void    CPC_SetCRTCType(unsigned int Type)
{
	if (Type >= NUM_CRTC_TYPES)
	{
		Type = 0;
	}

	switch (CPC_GetHardware())
	{
		/* force type 5 for KC Compact (HD6845R) *///
//	case CPC_HW_KCCOMPACT:
//	{
//		Type = 5;
//	}
//	break;

	/* force type 0 for KC Compact & Aleste 520 EX*/
	case CPC_HW_ALESTE:
	{
		Type = 0;
	}
	break;

	/* force type 3 for CPC+ */
	case CPC_HW_CPCPLUS:
	{
		Type = 3;
	}
	break;

	default
		:
	case CPC_HW_CPC:
	{
		if (Type == 3)
		{
			Type = 0;
		}
	}
	break;
	}

	CurrentCRTCType = Type;

	CRTC_SetType(Type);
}

int CPC_GetCRTCType(void)
{
	return CurrentCRTCType;
}

/*---------------------------------------------------------------------------*/
void CPC_RestartPower(void)
{
	int i;

	CPC_RestartReset();


	/* fill it with some data; TODO: Find values hardware seems to use */
	/* for the moment fill with random data */
	for (i = 0; i < 64 * 1024; i++)
	{
		Z80MemoryBase[i] = rand() % 256;
	}
	FDC_Power();
	FDC_Reset();

	CPU_Power();
}



void CPC_LoadFromSnapshot(SNAPSHOT_HEADER *pSnapshotHeader)
{
	int i;

	/* CPC initialise defaults */
	/**** GATE ARRAY ****/
	/* initialise colour palette */
	for (i = 0; i < 17; i++)
	{
		unsigned char HwColourIndex = ((char *)pSnapshotHeader)[0x02f + i];

		/* pen select */
		GateArray_Write(i);

		/* write colour for pen */
		GateArray_Write((HwColourIndex & 0x01f) | 0x040);
		printf("GA Pen %d Colour &%02x\n", i, (HwColourIndex & 0x01f) | 0x040);

	}

	/* pen select */
	GateArray_Write(((char *)pSnapshotHeader)[0x02e] & 0x01f);
	printf("GA Selected Pen %d\n", ((char *)pSnapshotHeader)[0x02e] & 0x01f);

	/* mode and rom configuration select */
	GateArray_Write((((char *)pSnapshotHeader)[0x040] & 0x03f) | 0x080);
	printf("GA MREM %02x\n", (((char *)pSnapshotHeader)[0x040] & 0x03f) | 0x080);
}

void CPC_SaveToSnapshot(SNAPSHOT_HEADER *pHeader)
{
	/* CPC initialise defaults */
}

void CPC_FillSnapshotMemoryBlocks(SNAPSHOT_MEMORY_BLOCKS *pSnapshotMemoryBlocks, const SNAPSHOT_OPTIONS *pOptions, BOOL bReading)
{
	int i;
	for (i = 0; i < 4; i++)
	{
		SNAPSHOT_MEMORY_BLOCK *pBlock = &pSnapshotMemoryBlocks->Blocks[i];
		pBlock->nSourceId = SNAPSHOT_SOURCE_INTERNAL_ID;
		pBlock->bAvailable = TRUE;
		pBlock->pPtr = Z80MemoryBase + (i << 14);
	}
}


void CPC_RestartReset(void)
{
	PSG_SetBC2State(&OnBoardAY, 1);

	/* CPC 464 */
	/* 8255 reset */
	PPI_Reset();

	/* PSG type and do reset */
	PSG_SetType(&OnBoardAY, PSG_TYPE_AY8912);
	PSG_Reset(&OnBoardAY);

	/* crtc reset */
	CRTC_Reset();

	/* Gate Array reset */
	GateArray_RestartReset();

	/* Printer reset */
	CPC_PrinterWrite(0);

	/* vector most seen on bus */
	CPU_Reset();


	// put into ddi
	
	/* CPC 664 */
	FDC_Reset();
	/* TC is connected to reset */
	FDC_SetTerminalCount(1);
	FDC_SetTerminalCount(0);
	/* output enable for disc rom is reset, but derived from something else */
}

/*---------------------------------------------------------------------------*/

void CostdownASIC_Reset(void)
{

}

/*---------------------------------------------------------------------------*/

void CPCCostdown_Reset(void)
{
	/* CPC 464 costdown */
	/* 8255 reset */
	PPI_Reset();

	/* PSG type and do reset */
	PSG_SetType(&OnBoardAY, PSG_TYPE_AY8912);
	PSG_Reset(&OnBoardAY);

	/* Printer reset */
	CPC_PrinterWrite(0);

	CostdownASIC_Reset();

	/* vector most seen on bus */
	CPU_Reset();

	/* CPC 6128 cost down */
	FDC_Reset();
	/* TC is connected to reset */
	FDC_SetTerminalCount(1);
	FDC_SetTerminalCount(0);

}

static Z80_BYTE BusValue;

/*---------------------------------------------------------------------------*/
void CPC_PrinterWrite(unsigned char Data)
{
	Printer_Write7BitData(Data);

	/* strobe is bit 7, invert  is handled by hardware */
	Printer_SetStrobeState((((Data & 0x080) ^ 0x080) != 0));
}

/*---------------------------------------------------------------------------*/


void CPC_InitialiseMemoryOutputs(MemoryData *pData)
{
	unsigned char RomConfiguration = GateArray_GetMultiConfiguration();

	/* rom activated in range &c000-&ffff? */
	pData->bRomEnable[7] = ((RomConfiguration & 0x08) == 0);
	pData->bRomEnable[6] = pData->bRomEnable[7];

	/* rom not activated in these ranges */
	pData->bRomEnable[5] = FALSE;
	pData->bRomEnable[4] = FALSE;
	pData->bRomEnable[3] = FALSE;
	pData->bRomEnable[2] = FALSE;

	/* rom activated in range &0000-&ffff? */
	pData->bRomEnable[1] = ((RomConfiguration & 0x04) == 0);
	pData->bRomEnable[0] = pData->bRomEnable[1];


	pData->bRamRead[7] = ((RomConfiguration & 0x08) != 0);
	pData->bRamRead[6] = pData->bRamRead[7];

	pData->bRamRead[5] = TRUE;
	pData->bRamRead[4] = TRUE;

	pData->bRamRead[3] = TRUE;
	pData->bRamRead[2] = TRUE;

	pData->bRamRead[1] = ((RomConfiguration & 0x04) != 0);
	pData->bRamRead[0] = pData->bRamRead[1];
}


void CPC_InitialiseDefaultMemory(MemoryData *pData)
{
	int i;
	for (i = 0; i < 8; i++)
	{
		if (!pData->bRamDisable[i])
		{
			pData->pWritePtr[i] = Z80MemoryBase;
			if (!pData->bRomEnable[i])
			{
				pData->pReadPtr[i] = pData->pWritePtr[i];
			}
		}
	}

//	if ((pData->bRomEnable[6]) && (!pData->bRomDisable[6]) && (pBasic != NULL))
//	{
//		pData->pReadPtr[7] = pBasic - 0x0c000;
//		pData->pReadPtr[6] = pBasic - 0x0c000;
//	}

	/* upper rom enabled, and not disabled */
	if ((pData->bRomEnable[6]) && (!pData->bRomDisable[6]))
	{
		const unsigned char *pRomData = pBasic;

		/* override enabled? */
		if (CPC_GetBASICOverrideROMEnable())
		{
			/* override rom set? */
			pRomData = CPC_GetBASICOverrideROM();
		}

		if (pRomData == NULL)
		{
			pData->pReadPtr[6] = (const unsigned char *)DummyReadRam - 0x0c000;
			pData->pReadPtr[7] = (const unsigned char *)DummyReadRam - 0x0c000;
		}
		else
		{
			pData->pReadPtr[6] = (const unsigned char *)pRomData - 0x0c000;
			pData->pReadPtr[7] = (const unsigned char *)pRomData - 0x0c000;
		}
	}

	/* lower rom enabled, and not disabled */
	if ((pData->bRomEnable[0]) && (!pData->bRomDisable[0]))
	{
		const unsigned char *pRomData = pOS;

		/* override enabled? */
		if (CPC_GetOSOverrideROMEnable())
		{
			/* override rom set? */
			pRomData = CPC_GetOSOverrideROM();
		}

		if (pRomData == NULL)
		{
			pData->pReadPtr[0] = (const unsigned char *)DummyReadRam - 0x00000;
			pData->pReadPtr[1] = (const unsigned char *)DummyReadRam - 0x00000;
		}
		else
		{
			pData->pReadPtr[0] = (const unsigned char *)pRomData - 0x00000;
			pData->pReadPtr[1] = (const unsigned char *)pRomData - 0x00000;
		}
	}
}


/*---------------------------------------------------------------------------*/
void Computer_RethinkMemory(void)
{
	int i;
	/* init defaults */
	for (i = 0; i < 8; i++)
	{
		int n16KPage = (i >> 1);
		int nOffset = n16KPage << 14;

		/* dummy read set for ram */
		MemData.pReadPtr[i] = (const unsigned char *)DummyReadRam - nOffset;

		/* dummy write set for ram */
		MemData.pWritePtr[i] = (unsigned char *)DummyWriteRam - nOffset;

		MemData.bRamRead[i] = FALSE; /* ramrd */
		MemData.bRomEnable[i] = FALSE; /* romen */
		MemData.bRamDisable[i] = FALSE; /* ramdis */
		MemData.bRomDisable[i] = FALSE; /* romdis */
	}
	
	/* depending on computer set some outputs */
	switch (CPC_GetHardware())
	{
	case CPC_HW_CPC:
	{
		CPC_InitialiseMemoryOutputs(&MemData);
	}
	break;

	case CPC_HW_ALESTE:
	{
		Aleste_InitialiseMemoryOutputs(&MemData);
	}
	break;

	case CPC_HW_KCCOMPACT:
	{

		KCC_InitialiseMemoryOutputs(&MemData);
	}
	break;

	case CPC_HW_CPCPLUS:
	{
		ASIC_InitialiseMemoryOutputs(&MemData);
	}
	break;
	}

	/* do memory handlers */
	if (NumMemoryRethinkFunctions != 0)
	{
		for (i = 0; i < NumMemoryRethinkFunctions; i++)
		{
			memoryRethinkFunctions[i](&MemData);
		}
	}

	/* now do default */
	switch (CPC_GetHardware())
	{
	case CPC_HW_CPC:
	{
		CPC_InitialiseDefaultMemory(&MemData);
	}
	break;

	case CPC_HW_ALESTE:
	{
		Aleste_InitialiseDefaultMemory(&MemData);
	}
	break;

	case CPC_HW_KCCOMPACT:
	{

		KCC_InitialiseDefaultMemory(&MemData);
	}
	break;

	case CPC_HW_CPCPLUS:
	{
		ASIC_InitialiseDefaultMemory(&MemData);
	}
	break;
	}
}

/*---------------------------------------------------------------------------*/
void    Computer_RestartReset(void)
{
	/* continue execution */
	Debug_Continue();

	Monitor_Reset();

	/* NMI is high */
	CPU_SetNMIState(FALSE);
	/* this should be done in disc interface code?*/
	FDI_SetMotorState(0);

	/* refresh vector base */
	switch (CPC_GetHardware())
	{
	case CPC_HW_CPC:
	{
		CPC_RestartReset();
	}
	break;

	case CPC_HW_ALESTE:
	{

		Aleste_RestartReset();

	}
	break;

	case CPC_HW_KCCOMPACT:
	{

		KCC_RestartReset();
	}
	break;

	case CPC_HW_CPCPLUS:
	{
		Plus_RestartReset();
	}
	break;
	}

	//	CPC_ResetTiming();
	CPC_ResetNopCount();

	CPC_ExecuteResetFunctions();

	Computer_RefreshInterrupt();
	Computer_RethinkMemory();

	Debugger_CheckHalt();
}

/*---------------------------------------------------------------------------*/
void    Computer_RestartPower(void)
{
	/* continue execution */
	Debug_Continue();


	Monitor_Reset();

	/* NMI is high */
	CPU_SetNMIState(FALSE);
	/* this should be done in disc interface code?*/
	FDI_SetMotorState(0);

	/* refresh vector base */
	switch (CPC_GetHardware())
	{
	case CPC_HW_CPC:
	{
		CPC_RestartPower();
	}
	break;

	case CPC_HW_ALESTE:
	{

		Aleste_RestartPower();

	}
	break;

	case CPC_HW_KCCOMPACT:
	{

		KCC_RestartPower();
	}
	break;

	case CPC_HW_CPCPLUS:
	{
		Plus_RestartPower();
	}
	break;
	}

	//	CPC_ResetTiming();
	CPC_ResetNopCount();

	CPC_ExecutePowerFunctions();


	Computer_RefreshInterrupt();
	Computer_RethinkMemory();

	Debugger_CheckHalt();

}
unsigned char   *Z80MemoryBase = NULL;                    /* Location of memory block */

/* free all ram allocated */
void    FreeEmulatorMemory(void)
{
	if (Z80MemoryBase != NULL)
	{
		free(Z80MemoryBase);
		Z80MemoryBase = NULL;
	}
}

/* allocate emulator base memory */
BOOL    AllocateEmulatorMemory(void)
{
	/* allocate base ram size */
	Z80MemoryBase = (unsigned char *)malloc(64 * 1024);

	/* ensure base range is setup */
	BaseMemoryRange.pBase = Z80MemoryBase;
	if (Z80MemoryBase == NULL)
	{
		/* 0x0ff is closer but may not be the accurate value */
		memset(Z80MemoryBase, 0x0ff, 64 * 1024);

		return FALSE;
	}

	return TRUE;
}



void    CPC_SetHardware(int Hardware)
{
	CPC_Hardware = Hardware;
#if 0
	switch (CPC_GetHardware())
	{
	case CPC_HW_CPC:
	{
		Multiface_SetMode(MULTIFACE_CPC_MODE);
	}
	break;

	case CPC_HW_CPCPLUS:
	{
		Multiface_SetMode(MULTIFACE_CPCPLUS_MODE);
	}
	break;

	default
		:
			break;

	}
#endif
	/* HACK: ensures plus gets correct crtc type ! */
	CPC_SetCRTCType(CPC_GetCRTCType());


	/*  CRTC_SetRenderFunction(RENDER_MODE_STANDARD); */
}

int     CPC_GetHardware(void)
{
	return CPC_Hardware;
}

void ClearOSOverrideROM(void)
{
	memset(OSOverrideROM, 0x0ff, 16384);
}

void ClearBASICOverrideROM(void)
{
	memset(BasicOverrideROM, 0x0ff, 16384);
}

void ClearAmsdosOverrideROM(void)
{
	memset(AmsdosOverrideROM, 0x0ff, 16384);
}

BOOL    CPC_Initialise(void)
{
	srand ((unsigned int)(time(0)));

	ClearOSOverrideROM();
	ClearBASICOverrideROM();
	ClearAmsdosOverrideROM();

	KeyJoy_Init();

	CPU_Init();

	BaseMemoryRange.m_nID = RIFF_FOURCC_CODE('M', 'A', 'I', 'N');
	BaseMemoryRange.m_bReadOnly = FALSE;
	BaseMemoryRange.m_bCPU = FALSE;
	BaseMemoryRange.sName = "Base 64K RAM";
	BaseMemoryRange.m_nLength = 65536;
	BaseMemoryRange.pBase = Z80MemoryBase;
	CPC_RegisterMemoryRange(&BaseMemoryRange);

	DefaultMemoryRange.m_nID = RIFF_FOURCC_CODE('C', 'P', 'U', ' ');
	DefaultMemoryRange.sName = "CPU";
	DefaultMemoryRange.m_bReadOnly = FALSE;
	DefaultMemoryRange.m_nLength = 65536;
	DefaultMemoryRange.pBase = NULL;
	DefaultMemoryRange.m_bCPU = TRUE;
	CPC_RegisterMemoryRange(&DefaultMemoryRange);


	memset(DummyReadRam, 0x0ff, sizeof(DummyReadRam));

	/*		Memory_Init(); */

	//		BrightnessControl_Initialise();

	/* setup rendering tables etc */
	Render_Initialise();

	/* allocate base memory */
	AllocateEmulatorMemory();

	FDD_InitialiseAll();

	PSG_Init(&OnBoardAY);


	GateArray_Initialise();

	/* set colour mode */
	//	GateArray_SetMonitorColourMode(MONITOR_MODE_COLOUR);

	/* initialise keyboard */
	CPC_ClearKeyboard();

	/* TAPEFILE */
	/* patch rom for tape file loading/saving */
	/*      Tape_PatchRom(Roms6128.pOs, Roms6128.pBasic); */
	/*      Z80_InstallPatches(TRUE); */


	/* CPC PLUS EMULATION */

	/* Initialise ASIC */
	ASIC_Initialise();


	Monitor_Init();

	Cassette_Init();

	DiskImage_Initialise();


	/*   WavOutput_Init("wavout.tmp");

		//AudioEvent_Initialise();
		//Audio_Init();
		*/
	Jukebox_Init();


	KCC_Init();
	Aleste_Init();

	return TRUE;
}

void    CPC_Finish(void)
{
	Aleste_Finish();
	KCC_Finish();

	Render_Finish();
	Cassette_Finish();

	/*       AudioEvent_Finish();
	//Audio_Finish(); */
	Cartridge_RemoveI();

	ASIC_Finish();

	/* free ram allocated for memory */
	FreeEmulatorMemory();

	/* removes all disc images; doesn't save! */
	DiskImage_Finish();

	/* remove any tape image inserted */
	Tape_Remove();

	Jukebox_Finish();

}


/*--------------------------------------------------------------*/
/* read a byte from base memory without memory paging */
Z80_BYTE CPU_RD_BASE_BYTE(Z80_WORD Addr)
{
	return Z80MemoryBase[Addr];
}

/* read a word from base memory without memory paging */
Z80_WORD CPU_RD_BASE_WORD(Z80_WORD Addr)
{
	return (unsigned short)((((Z80_WORD)CPU_RD_BASE_BYTE(Addr)) | (((Z80_WORD)CPU_RD_BASE_BYTE((Z80_WORD)(Addr + 1))) << 8)));
}

/* read a byte from emulator memory with paging */
Z80_BYTE        CPU_RD_MEM(Z80_WORD Addr)
{
	unsigned long    MemBlock;
	const unsigned char                   *pAddr;

	/* calculate 16k page */
	MemBlock = (Addr >> 13) & 0x07;

	/* calculate address to read from */
	pAddr = MemData.pReadPtr[MemBlock] + Addr;

	/* return byte at memory address */
	return pAddr[0];
}



/*--------------------------------------------------------------*/

/* write a byte to emulator memory with paging */
void CPU_WR_MEM(Z80_WORD Addr, Z80_BYTE Data)
{
	unsigned int MemBlock;
	unsigned char           *pAddr;

/* calculate 16k page */
MemBlock = (Addr >> 13) & 0x07;

/* calculate address to write to */
pAddr = MemData.pWritePtr[MemBlock] + Addr;

/* write byte to memory address */
pAddr[0] = Data;

/* CONFIRMED: Ast found it. Gerald found explanation. Write will go to ASIC ram if enabled in addition to ram */
/* For an expansion this means the data writes to asic RAM AND expansion ram. So always do the write */
ASIC_WriteRAMIfEnabled(Addr, Data);
}

/*--------------------------------------------------------------------------*/
/* Port Write

  bit 15 = 0; Gate Array
  bit 14 = 0; CRTC Write
  bit 13 = 0; Select Expansion Rom
  bit 12 = 0; Printer Port
  bit 11 = 0; PPI Write
  bit 10 = bit 7 = 0; FDC Write
  */

  /*--------------------------------------------------------------------------*/

void	CPC_Out(const Z80_WORD Port, const Z80_BYTE Data)
{
	if ((Port & 0x0c000) == 0x04000)
	{
		/* gate array cannot be selected if CRTC is also
		selected */
		GateArray_Write(Data);
	}


	if ((Port & 0x04000) == 0)
	{
		/* crtc selected */

		unsigned int            Index;

		Index = (Port >> 8) & 0x03;

		switch (Index)
		{
		case 0:
		{
			CRTC_RegisterSelect(Data);
		}
		break;

		case 1:
		{
			CRTC_WriteData(Data);
		}
		break;

		default
			:
				break;
		}
	}


	if ((Port & 0x01000) == 0)
	{
		CPC_PrinterWrite(Data);
	}

	if ((Port & 0x0800) == 0)
	{
		unsigned int            Index;

		Index = (Port & 0x0300) >> 8;

		if (Index == 3)
		{
			PPI_WriteControl(Data);
		}
		else
		{
			PPI_WritePort(Index, Data);
		}
	}
}


/*--------------------------------------------------------------------------*/

/* Write data to a I/O port */
void    CPU_DoOut(const Z80_WORD Port, const Z80_BYTE Data)
{
	/* capture for snapshot */
	CaptureSnapshotData(Port, Data);

	switch (CPC_GetHardware())
	{
	case CPC_HW_CPC:
	{
		CPC_Out(Port, Data);
	}
	break;

	case CPC_HW_CPCPLUS:
	{
		CPCPLUS_Out(Port, Data);
	}
	break;

	case CPC_HW_ALESTE:
	{
		Aleste_Out(Port, Data);
	}
	break;

	case CPC_HW_KCCOMPACT:
	{
		KCCompact_Out(Port, Data);
	}
	break;
	}

	CPC_ExecuteWritePortFunctions(Port, Data);
}

/*--------------------------------------------------------------------------*/
/* Port Read

  bit 14 = 0; CRTC Read
  bit 11 = 0; PPI Read
  bit 10 = bit 7 = 0; FDC Read
  */


/* In of port &efxx on CPC?! */

Z80_BYTE        CPC_In(Z80_WORD Port)
{
	Z80_BYTE Data = 0x0ff;

	if ((Port & 0x04000) == 0)
	{
		unsigned int            Index;

		Index = (Port & 0x0300) >> 8;

		switch (Index)
		{
			/* possibly write to crtc when do in */
		case 2:
		{
			Data = CRTC_ReadStatusRegister();
		}
		break;

		case 3:
		{
			Data = CRTC_ReadData();
		}
		break;

		default
			:
				break;
		}
	}

	if ((Port & 0x0800) == 0)
	{
		unsigned int            Index;

		Index = (Port & 0x0300) >> 8;
		if (Index == 3)
		{
			Data = PPI_ReadControl();
		}
		else
		{
			Data = PPI_ReadPort(Index);
		}
	}

	CPC_ExecuteReadPortFunctions(Port, &Data);

	/* confirmed: on all CPCs, you can write to the GA using a read */
	/* /RD and /WR is ignored. Do an IN on a port such as PPI but also set the 
	port to perform an IN with the GA */
	if ((Port & 0x0c000) == 0x04000)
	{
		GateArray_Write(Data);
	}

	/* confirmed: on all CPCs the CRTC can be written to with an IN instruction */
	/* EN is connected to IORD or IOWR, with R/W controlled by an address line */
	if ((Port & 0x04000) == 0)
	{
		unsigned int            Index;

		Index = (Port & 0x0300) >> 8;

		switch (Index)
		{
		case 0:
		{
			CRTC_RegisterSelect(Data);
		}
		break;

		case 1:
		{
			CRTC_WriteData(Data);
		}
		break;

		default
			:
				break;

		}
	}

	return (Z80_BYTE)Data;
}

/*----------------------------------------------------------------------------*/


Z80_BYTE        CPU_DoIn(Z80_WORD Port)
{
	switch (CPC_GetHardware())
	{
	case CPC_HW_CPC:
		return CPC_In(Port);

	case CPC_HW_CPCPLUS:
		return CPCPlus_In(Port);

	case CPC_HW_KCCOMPACT:
		return KCCompact_In(Port);

	case CPC_HW_ALESTE:
		return Aleste_In(Port);
	}

	return 0x0ff;
}

/*----------------------------------------------------------------------------*/

void	CPC_ResetNopCount(void)
{
	NopCount = 0;
}



extern MONITOR_INTERNAL_STATE Monitor_State;


void    Render_RenderBorder_Paletted(void)
{
	Render_Paletted_PutBorder(Monitor_State.MonitorHorizontalCount, Monitor_State.MonitorScanLineCount);
}


void    Render_RenderBorder_TrueColour(void)
{
	/* a bit of a hack here.. need to clean up */
	 ASIC_CalcVRAMAddr();
    ASIC_CachePixelData();
	
	Render_TrueColourRGB_PutBorder(Monitor_State.MonitorHorizontalCount, Monitor_State.MonitorScanLineCount);
}


void    Render_RenderBlack_TrueColour(void)
{
	Render_TrueColourRGB_PutBlack(Monitor_State.MonitorHorizontalCount, Monitor_State.MonitorScanLineCount);
}


void    Render_GetGraphicsDataCPC_TrueColour(void)
{
	unsigned short GraphicsWord = 0;
	switch (CPC_GetHardware())
	{
	case CPC_HW_CPC:
	{
		CPC_CalcVRAMAddr();
		CPC_CachePixelData();
		GraphicsWord = CPC_GetPixelData();
	}
	break;

	case CPC_HW_CPCPLUS:
	{
		ASIC_CalcVRAMAddr();
		ASIC_CachePixelData();
		GraphicsWord = (unsigned short)ASIC_GetPixelData();
	}
	break;

	case CPC_HW_KCCOMPACT:
	{
		KCC_CalcVRAMAddr();
		KCC_CachePixelData();
		GraphicsWord = KCC_GetPixelData();
	}
	break;
	}

	Render_TrueColourRGB_PutDataWord(Monitor_State.MonitorHorizontalCount, GraphicsWord, Monitor_State.MonitorScanLineCount);
}



void    Render_GetGraphicsDataCPC_Paletted(void)
{
	unsigned int Addr;
	unsigned int LocalMA;
	unsigned int GraphicsWord;

	/* CPC version */

	//        LocalMA = (unsigned int)(MALine + HCount);
	LocalMA = (CRTC_GetMAOutput() << 1);

	/* get screen scrolling */
	Addr = (unsigned int)(((LocalMA & 0x03000) << 2) | ((LocalMA & 0x03ff) << 1));

	/* take address, and put in vertical line count in place of these 3 bits. */
	Addr |= CRTC_GetRAOutput();


	GraphicsWord = ((Z80MemoryBase[(unsigned int)(Addr)]) << 8)
		| (Z80MemoryBase[(unsigned int)(Addr + 1)]);

	Render_Paletted_PutDataWord(Monitor_State.MonitorHorizontalCount, GraphicsWord, Monitor_State.MonitorScanLineCount);
}




void    CRTC_RenderSync_Paletted(void)
{
	Render_Paletted_PutSync(Monitor_State.MonitorHorizontalCount, Monitor_State.MonitorScanLineCount);
}

void    CRTC_RenderSync_TrueColour(void)
{
	Render_TrueColourRGB_PutSync(Monitor_State.MonitorHorizontalCount, Monitor_State.MonitorScanLineCount);
}

void    CRTC_RenderBlack_TrueColour(void)
{
	Render_TrueColourRGB_PutBlack(Monitor_State.MonitorHorizontalCount, Monitor_State.MonitorScanLineCount);
}

void    Computer_UpdateGraphicsFunction(void)
{
	switch (CPC_GetHardware())
	{
	case CPC_HW_KCCOMPACT:
	{
		KCC_UpdateGraphicsFunction();
	}
	break;

	case CPC_HW_ALESTE:
	{

		Aleste_UpdateGraphicsFunction();


	}
	break;

	case CPC_HW_CPC:
	{
		CPC_UpdateGraphicsFunction();
	}
	break;

	case CPC_HW_CPCPLUS:
	{
		Plus_UpdateGraphicsFunction();
	}
	break;
	}
}

void    CPC_UpdateNopCount(unsigned long NopsToAdd)
{
	NopCount += NopsToAdd;
}

unsigned long CPC_GetNopCount(void)
{
	return NopCount;
}


void Computer_DoVideoCycles(int nCycles)
{
	switch (CPC_GetHardware())
	{
	case CPC_HW_CPC:
	{
		int i;
		for (i = 0; i < nCycles; i++)
		{
			CRTC_DoCycles(1);
			GateArray_Cycle();
		}
	}
	break;

	case CPC_HW_CPCPLUS:
	{
		int i;
		for (i = 0; i < nCycles; i++)
		{
			CRTC_DoCycles(1);
			ASIC_Cycle();
		}

	}
	break;

	case CPC_HW_KCCOMPACT:
	{
		int i;
		for (i = 0; i < nCycles; i++)
		{
			CRTC_DoCycles(1);
			KCC_Cycle();
		}
	}
	break;

	case CPC_HW_ALESTE:
	{
		int i;
		if (Aleste_GetExtport()&(1 << 1))
		{
			nCycles *= 2;
		}
		for (i = 0; i < nCycles; i++)
		{
			CRTC_DoCycles(1);
		}
	}
	break;
	}
}

void CRTC_DoCursorOutput(int nState)
{
	/*printf("Cursor changed: %d\n", nState); */
	CPC_ExecuteCursorOutputFunctions(nState);
}

BOOL bCRTCVsyncOutput = FALSE;

void Computer_UpdateVsync(void)
{

	switch (CPC_GetHardware())
	{

	case CPC_HW_CPC:
	{
		/* combinations:
		CRTC Vsync|PPI Out|PPI Set|Result
		0|0|0|0
		1|0|0|1
		0|1|1|1
		0|1|0|0

		1|1|0|?  TEST
		1|1|1|?  TEST

		CRTC Vsync -> CRTC asserting VSYNC
		PPI Out -> PPI Port B set to output
		PPI Set -> PPI Output setting value
		*/

		/* confirmed: no effect type 1 */
		/* confirmed: can be triggered on type 0 */
		/* confirmed: no effect type 2 */
		/* CHECK: type 4 and type 5 */
		BOOL bVsync = FALSE;

		/* vsync can be driven by PPI or by crtc */
		if ((CPC_GetCRTCType() == 0) && (PPI_GetOutputMaskPort(1) == 0x0ff))
		{
			/* PPI is driving vsync */
			/* assume that it takes control always */
			bVsync = ((PPI_GetOutputPort(1) & 1) != 0);
		}
		else
		{
			/* CRTC is driving vsync */
			bVsync = bCRTCVsyncOutput;
		}
		GateArray_UpdateVsync(bVsync);
	}
	break;

	case CPC_HW_CPCPLUS:
	{
		/* confirmed: VSYNC can't be forced by ASIC because PPI port B can't be set to output */
		ASIC_UpdateVsync(bCRTCVsyncOutput);
	}
	break;

	case CPC_HW_KCCOMPACT:
	{
		/* kcc can force VSYNC? */
		KCC_UpdateVsync(bCRTCVsyncOutput);
	}
	break;

	case CPC_HW_ALESTE:
	{
		/* aleste can force VSYNC? */
		Aleste_UpdateVsync(bCRTCVsyncOutput);

	}
	break;
	}
}

CYCLE_RECORDING VsyncCycleRecording;
CYCLE_RECORDING HsyncCycleRecording;

void CycleRecording_Init(CYCLE_RECORDING *pRecording)
{
	pRecording->bRecordEnable = FALSE;
	pRecording->bCyclesLastValid = FALSE;
	pRecording->CycleIndex = 0;
	pRecording->CyclesRecorded = 0;
}

int CycleRecording_GetAverage(CYCLE_RECORDING *pRecording)
{
	int i;
	int Index = pRecording->CycleIndex - 1;
	int CumulativeCycles = 0;
	if (Index < 0)
	{
		Index = BUFFER_SIZE - 1;
	}
	for (i = 0; i < pRecording->CyclesRecorded; i++)
	{
		CumulativeCycles += pRecording->Cycles[Index];
		Index--;
		if (Index < 0)
		{
			Index = BUFFER_SIZE - 1;
		}
	}

	return CumulativeCycles / pRecording->CyclesRecorded;
}

void CycleRecording_Enable(CYCLE_RECORDING *pRecording, BOOL bState)
{
	pRecording->bRecordEnable = bState;
	if (bState == TRUE)
	{
		pRecording->bCyclesLastValid = FALSE;
		pRecording->CycleIndex = 0;
		pRecording->CyclesRecorded = 0;
	}
}



void CycleRecording_Update(CYCLE_RECORDING *pRecording)
{
	/* if state has changed, and state is now active */
	if (pRecording->bRecordEnable)
	{
		/* get cycles this time */
		int CyclesThis = CPU_GetCycles();

		/* if cycles are not valid  record now and mark valid, this will be the basis of our first recording */
		if (!pRecording->bCyclesLastValid)
		{
			pRecording->CyclesLast = CyclesThis;
			pRecording->bCyclesLastValid = TRUE;
		}
		else
		{
			/* we have at least one valid recording */
			/* calculate difference */
			int CycleDiff = CyclesThis - pRecording->CyclesLast;
			/* store difference in array */
			pRecording->Cycles[pRecording->CycleIndex] = CycleDiff;
			pRecording->CycleIndex++;
			/* loop around buffer if we go off the end */
			if (pRecording->CycleIndex >= BUFFER_SIZE)
			{
				pRecording->CycleIndex = 0;
			}
			/* keep track of number recorded, but we can only keep track of so many */
			pRecording->CyclesRecorded++;
			if (pRecording->CyclesRecorded > BUFFER_SIZE)
			{
				pRecording->CyclesRecorded = BUFFER_SIZE;
			}
			/* store new recorded value */
			pRecording->CyclesLast = CyclesThis;
		}
	}
}

/* set vsync output from CRTC */
void CRTC_SetVsyncOutput(BOOL bState)
{
	/* if state has changed, and state is now active */
	if ((bCRTCVsyncOutput != bState) && (bState))
	{
		CycleRecording_Update(&VsyncCycleRecording);
	}

	bCRTCVsyncOutput = bState;

	if (bState)
	{
		TriggerBreakOn(BREAK_ON_VSYNC_START);
	}
	else
	{
		TriggerBreakOn(BREAK_ON_VSYNC_END);
	}

	Computer_UpdateVsync();
}



void Computer_RefreshInterrupt(void)
{
	BOOL bIntRequest = FALSE;
	switch (CPC_GetHardware())
	{
	case CPC_HW_CPCPLUS:
	{
		/* ASIC acknowledge int */
		bIntRequest = ASIC_GetInterruptRequest();
	}
	break;

	case CPC_HW_CPC:
	{
		/* CRTC acknowledge int */
		bIntRequest = GateArray_GetInterruptRequest();
	}
	break;

	case CPC_HW_KCCOMPACT:
	{
		/* KCC acknowledge interrupt */
		bIntRequest = KCC_GetInterruptRequest();
	}
	break;

	case CPC_HW_ALESTE:
	{
		bIntRequest = Aleste_GetInterruptRequest();
	}
	break;

	}

	CPU_SetINTState(bIntRequest);
}

void CPC_AcknowledgeNMI(void)
{
	TriggerBreakOn(BREAK_ON_NMI);
}


Z80_BYTE    CPC_AcknowledgeInterrupt(void)
{

	Z80_BYTE Vec = 0x0ff;

	TriggerBreakOn(BREAK_ON_INT);

	switch (CPC_GetHardware())
	{
	case CPC_HW_CPCPLUS:
	{
		/* ASIC acknowledge int */
		Vec = ASIC_AcknowledgeInterrupt();
	}
	break;

	case CPC_HW_CPC:
	{
		/* CRTC acknowledge int */
		Vec = GateArray_AcknowledgeInterrupt();
	}
	break;

	case CPC_HW_KCCOMPACT:
	{
		/* KCC acknowledge interrupt */
		Vec = KCC_AcknowledgeInterrupt();
	}
	break;

	case CPC_HW_ALESTE:
	{
		Vec = Aleste_AcknowledgeInterrupt();
	}
	break;
	}

	// need to return vec
	CPC_ExecuteAckMaskableInterruptHandler();

	Computer_RefreshInterrupt();
	return Vec;
}

void CRTC_SetDispEnable(BOOL DispEnable)
{
	switch (CPC_GetHardware())
	{
	case CPC_HW_CPCPLUS:
	{
		ASIC_DoDispEnable(DispEnable);
	}
	break;

	case CPC_HW_CPC:
	{
		GateArray_DoDispEnable(DispEnable);
	}
	break;

	case CPC_HW_KCCOMPACT:
	{
		KCC_DoDispEnable(DispEnable);
	}
	break;

	case CPC_HW_ALESTE:
	{
		Aleste_DoDispEnable(DispEnable);
	}
	break;


	default
		:
			break;
	}
}

static int SpeakerVolume = SPEAKER_VOLUME_MAX;

int CPC_GetSpeakerVolume(void)
{
	return SpeakerVolume;
}

void CPC_SetSpeakerVolume(int Volume)
{
	if (Volume < 0)
	{
		Volume = 0;
	}
	if (Volume > SPEAKER_VOLUME_MAX)
	{
		Volume = SPEAKER_VOLUME_MAX;
	}
	SpeakerVolume = Volume;
}

static BOOL DrawBlanking = TRUE;
BOOL Computer_GetDrawBlanking(void)
{
	return DrawBlanking;
}

void Computer_SetDrawBlanking(BOOL bState)
{
	DrawBlanking = bState;
}

static BOOL DrawBorder = TRUE;
BOOL Computer_GetDrawBorder(void)
{
	return DrawBorder;
}

void Computer_SetDrawBorder(BOOL bState)
{
	DrawBorder = bState;
}

void    CRTC_SetHsyncOutput(BOOL bState)
{
	//  if ((bCRTCVsyncOutput!=bState) && (bState))
	// {
	//   CycleRecording_Update(&VsyncCycleRecording);
	//}

	if (bState)
	{
		TriggerBreakOn(BREAK_ON_HSYNC_START);
	}
	else
	{
		TriggerBreakOn(BREAK_ON_HSYNC_END);
	}

	switch (CPC_GetHardware())
	{
	case CPC_HW_CPCPLUS:
	{
		ASIC_UpdateHsync(bState);
	}
	break;

	case CPC_HW_CPC:
	{
		GateArray_UpdateHsync(bState);
	}
	break;
	case CPC_HW_KCCOMPACT:
	{
		KCC_UpdateHsync(bState);

	}
	break;

	case CPC_HW_ALESTE:
	{
		Aleste_UpdateHsync(bState);
	}
	break;


	default
		:
			break;
	}
}

/* end is the end address to save */
/* if end<start, then assume wrap around ram */
unsigned long SaveRamToBufferGetLength(FILE_HEADER *pFileHeader)
{
	unsigned long nLength = pFileHeader->MemoryEnd - pFileHeader->MemoryStart;
	if (pFileHeader->bHasHeader)
	{
		nLength += 0x080;
	}
	return nLength;
}


void SaveRamToBuffer(const MemoryRange *pRange, unsigned char *pBuffer, FILE_HEADER *pFileHeader)
{
	int i;

	if (pFileHeader->bHasHeader)
	{
		/* write AMSDOS header, mark the file as unprotected binary */
		AMSDOS_MakeHeader(pBuffer, pFileHeader->HeaderStartAddress, pFileHeader->HeaderLength, pFileHeader->HeaderExecutionAddress, pFileHeader->HeaderFileType);
		pBuffer += 0x080;
	}

	/* fill data */
	for (i = pFileHeader->MemoryStart; i < pFileHeader->MemoryEnd; i++)
	{
		/* note this reads memory as it is currently seen */
		int nData = MemoryRange_ReadByte(pRange, i);

		*pBuffer = nData;
		++pBuffer;
	}
}

void GetHeaderDataFromBuffer(const unsigned char *pFileData, unsigned long FileDataSize, FILE_HEADER *pFileHeader)
{
	pFileHeader->bHasHeader = FALSE;
	pFileHeader->MemoryStart = 0;
	pFileHeader->MemoryEnd = FileDataSize;

	if (AMSDOS_HasAmsdosHeader(pFileData))
	{
		AMSDOS_HEADER *pHeader = (AMSDOS_HEADER *)pFileData;

		/* fetch file type */
		pFileHeader->HeaderFileType = pHeader->FileType;

		/* get length reported by header */
		pFileHeader->HeaderLength = ((pHeader->DataLengthLow & 0x0ff) | ((pHeader->DataLengthMid & 0x0ff) << 8) | ((pHeader->DataLengthHigh & 0x0ff) << 16));

		/* fetch start address from header */
		pFileHeader->HeaderStartAddress = ((pHeader->LocationLow & 0x0ff) | ((pHeader->LocationHigh & 0x0ff) << 8));

		pFileHeader->HeaderExecutionAddress = ((pHeader->ExecutionAddressLow & 0x0ff) | ((pHeader->ExecutionAddressHigh & 0x0ff) << 8));

		pFileHeader->bHasHeader = TRUE;
	}
	else
	{
		/* fill in dummy values */
		pFileHeader->HeaderFileType = 2;

		pFileHeader->HeaderStartAddress = 0;

		pFileHeader->HeaderLength = FileDataSize;

		pFileHeader->HeaderExecutionAddress = pFileHeader->HeaderStartAddress;
	}
}

void LoadBufferToRam(const MemoryRange *pRange, const char *pFileData, FILE_HEADER *pFileHeader)
{
	if (pFileData != NULL)
	{
		int i;
		int Addr = pFileHeader->MemoryStart;
		unsigned long MemoryLength = pFileHeader->MemoryEnd - pFileHeader->MemoryStart;
		unsigned long CopyLength = MemoryLength;
		const char *pData = pFileData;
		if (pFileHeader->bHasHeader)
		{
			pData += 0x080;
			if (pFileHeader->HeaderLength < MemoryLength)
			{
				CopyLength = pFileHeader->HeaderLength;
			}
		}

		for (i = 0; i < CopyLength; i++)
		{
			MemoryRange_WriteByte(pRange, Addr, *pData);
			++pData;
			Addr = (Addr + 1) & 0x0ffff;
		}
	}
}

static BOOL bDebugStop = FALSE;
static BOOL bDebugOpcodeEnabled = FALSE;
static Z80_BYTE DataBus = 0x0ff;

int CPU_GetCycles(void)
{
	return CPC_GetNopCount();
}

Z80_BYTE CPU_GetDataBus(void)
{
	return DataBus;
}

void CPU_SetDataBus(Z80_BYTE Value)
{
	DataBus = Value;
}

BOOL CPU_GetDebugStop(void)
{
	return bDebugStop;
}

void CPU_SetDebugStop(void)
{
	bDebugStop = TRUE;
}

void CPU_ResetDebugStop(void)
{
	bDebugStop = FALSE;
}

void CPU_Reti(void)
{
	CPC_ExecuteRetiFunctions();
}


BOOL    CPU_GetDebugOpcodeEnabled(void)
{
	return bDebugOpcodeEnabled;
}

void CPU_SetDebugOpcodeHit(void)
{
	if (bDebugOpcodeEnabled)
	{
		CPU_SetDebugStop();
	}
}

void    CPU_SetDebugOpcodeEnabled(BOOL bState)
{
	bDebugOpcodeEnabled = bState;
}

static int IOData = 0x0ff;
static int IOPort = 0x0ffff;

void CPU_SetIOData(int nData)
{
	IOData = nData;
}

void CPU_SetIOPort(int nData)
{
	IOPort = nData;
}

int CPU_GetIOData(void)
{
	return IOData;
}

int CPU_GetIOPort(void)
{
	return IOPort;
}

static int IntVectorAddress = 0;

int CPU_GetIntVectorAddress(void)
{
	return IntVectorAddress;
}

void CPU_SetIntVectorAddress(int nValue)
{
	IntVectorAddress = nValue;
}


const char *sCPCKeyNames[CPC_KEY_NUM_KEYS] =
{
	"Cursor Up",
	"Cursor Right",
	"Cursor Down",
	"f9",
	"f6",
	"f3",
	"Enter",
	"f.",
	"Cursor Left",
	"Copy",
	"f7",
	"f8",
	"f5",
	"f1",
	"f2",
	"f0",
	"Clr",
	"{[",
	"Return",
	"}]",
	"f4",
	"Shift",
	"'\\",
	"Ctrl",
	"£^",
	"=-",
	"|@",
	"P",
	"+;",
	"*:",
	"?/",
	">,",
	"_0",
	")9",
	"O",
	"I",
	"L",
	"K",
	"M",
	"<.",
	"(8",
	"'7",
	"U",
	"Y",
	"H",
	"J",
	"N",
	"Space",
	"&6",
	"%5",
	"R",
	"T",
	"G",
	"F",
	"B",
	"V",
	"$4",
	"#3",
	"E",
	"W",
	"S",
	"D",
	"C",
	"X",
	"!1",
	"\"2",
	"Esc",
	"Q",
	"Tab",
	"A",
	"Caps Lock",
	"Z",
	"Joy 0 Up",
	"Joy 0 Down",
	"Joy 0 Left",
	"Joy 0 Right",
	"Joy 0 Fire2",
	"Joy 0 Fire1",
	"Unused",
	"Del"
};

const char *CPC_GetKeyName(CPC_KEY_ID nKey)
{
	return sCPCKeyNames[nKey];
}

