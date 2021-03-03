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
#include "emudevice.h"
#include "cpc.h"

static int m_nDevices = 0;
static BOOL m_bDevicesEnabled[MAX_DEVICES];
static const EmuDevice *m_pDevices[MAX_DEVICES];


// number of enabled devices
static int m_nEnabledDevices = 0;
// id of device
static int m_nEnabledDeviceId[MAX_DEVICES];

int EmuDevice_GetNumEnabled(void)
{
	return m_nEnabledDevices;
}

// get the id of the device at the slot
int EmuDevice_GetEnabledDeviceId(int nEnabledIndex)
{
	return m_nEnabledDeviceId[nEnabledIndex];
}

/* remove expansion rom */
void    ExpansionRom_Remove(ExpansionRomData *pData, int RomIndex)
{
	if (RomIndex >= MAX_EXPANSION_ROMS)
	{
		return;
	}

	if (pData->ExpansionRomData[RomIndex] != NULL)
	{
		free(pData->ExpansionRomData[RomIndex]);
		pData->ExpansionRomData[RomIndex] = NULL;
	}
	pData->ExpansionRomActive[RomIndex] = FALSE;
}


void    ExpansionRom_Init(ExpansionRomData *pData, ClearExpansionRom pClearExpansionRom, SetExpansionRom pSetExpansionRom)
{
	int i;

	for (i = 0; i < MAX_EXPANSION_ROMS; i++)
	{
		pData->ExpansionRomData[i] = NULL;
		pData->ExpansionRomActive[i] = FALSE;
		pData->ExpansionRomAvailable[i] = FALSE;
	}
	pData->m_ClearExpansionRom = pClearExpansionRom;
	pData->m_SetExpansionRom = pSetExpansionRom;
}

/* finish expansion roms. Delete all roms */
void    ExpansionRom_Finish(ExpansionRomData *pData)
{
	int i;

	for (i = 0; i < MAX_EXPANSION_ROMS; i++)
	{
		ExpansionRom_Remove(pData, i);
	}
}


/* return TRUE if slot is in-use, RomName will hold a pointer to the name */
/* return FALSE if slot is not-used */
BOOL ExpansionRom_GetRomName(ExpansionRomData *pData, const int RomIndex, char **RomName)
{
	unsigned char *pRomData;

	*RomName = NULL;

	if (RomIndex >= MAX_EXPANSION_ROMS)
	{
		return FALSE;
	}

	pRomData = pData->ExpansionRomData[RomIndex];

	if (pRomData != NULL)
	{
		unsigned short NameTableAddr;
		unsigned char *pName;
		unsigned int RomType = pRomData[0] & 0x07f;
		unsigned int RomMark = pRomData[1] & 0x0ff;
		unsigned int RomVersion = pRomData[2] & 0x0ff;
		unsigned int RomModification = pRomData[3] & 0x0ff;
		int NameLength;
		char *pNameBuffer;
		unsigned char *pRomEnd = pRomData + 16384;

		if (RomType > 0x02)
		{
			return FALSE;
		}

		/* get address of name table */
		NameTableAddr = ExpansionRom_GetWord(pRomData, 4);
		if ((NameTableAddr < 0x0c000) || (NameTableAddr>0x0ffff))
		{
			return FALSE;
		}

		/* get pointer to name */
		pName = pRomData + (NameTableAddr & 0x03fff);

		/* the last character of the name is marked with bit 7 set */
		NameLength = 0;
		do
		{
			/* count this character */
			NameLength++;

			/* if this character has bit 7 set, then it is the last
			character in the name. If this character doesn't have bit 7 set
			then this is not the last character */
		} while (((pName[NameLength] & 0x080) == 0) && (pName != pRomEnd));
		if (pName != pRomEnd)
		{
			NameLength++;
		}
		/* allocate buffer to hold name */
		pNameBuffer = (char *)malloc(NameLength + 8 + 1);

		if (pNameBuffer != NULL)
		{
			int Count;

			/* fill in buffer */
			for (Count = 0; Count < NameLength; Count++)
			{
				pNameBuffer[Count] = pName[Count] & 0x07f;
			}
			pNameBuffer[Count] = '\0';

			if ((RomMark < 10) && (RomVersion < 10) && (RomModification < 10))
			{
				pNameBuffer[Count] = ' ';
				Count++;
				sprintf(&pNameBuffer[Count], "[v%1x.%01x%01x]", RomMark, RomVersion, RomModification);
			}

			*RomName = pNameBuffer;
			return TRUE;
		}
	}
	return FALSE;
}

ExpansionRomData *EmuDevice_GetExpansionRomData(int nDevice)
{
	return m_pDevices[nDevice]->m_ExpansionRoms;
}

/* insert a expansion rom; returns status code */
/* cpc-rulez is added data on the end of the file! */
int    ExpansionRom_SetRomData(ExpansionRomData *pData, const unsigned char *pSourceRomData, const unsigned long SourceRomDataSize, const int RomIndex)
{
	const unsigned char *pRomData;
	unsigned long RomLength;
	int Status = ARNOLD_STATUS_ERROR;

	if (RomIndex >= MAX_EXPANSION_ROMS)
	{
		return ARNOLD_STATUS_ERROR;
	}

	if (pData->m_ClearExpansionRom)
	{
		pData->m_ClearExpansionRom(RomIndex);
	}
	else
	{
		/* remove an existing rom */
		if (pData->ExpansionRomData[RomIndex] != NULL)
		{
			ExpansionRom_Remove(pData, RomIndex);
		}
	}

	pRomData = pSourceRomData;
	RomLength = SourceRomDataSize;

	AMSDOS_GetUseableSize(&pRomData, &RomLength);

	/* is rom length valid? */
	if (RomLength > 0)
	{
		/* validate */
		if (ExpansionRom_Validate(pRomData, RomLength))
		{
			/* it is valid */

			/* we want to check if a foreground rom data file is
			loaded into any slot above 0, or a non-foreground not
			loaded into slot 0 */

			/* store rom pointer in expansion rom table */


			/* The following is done, so that roms with less
			than 16k can be supported. We allocate 16k, so that
			if a memory access is done which would be outside the rom
			data area, it doesn't cause an invalid memory access */

			if (pData->m_SetExpansionRom)
			{
				if (pData->m_SetExpansionRom(RomIndex, pRomData, RomLength))
				{
					/* mark it as active */
					ExpansionRom_SetActiveState(pData, RomIndex, TRUE);
					Status = ARNOLD_STATUS_OK;
				}
			}
			else
			{
				/* allocate 16384 bytes */
				pData->ExpansionRomData[RomIndex] = (unsigned char *)malloc(16384);

				if (pData->ExpansionRomData[RomIndex] != NULL)
				{
					int LengthToCopy;

					memset(pData->ExpansionRomData[RomIndex], 0x0ff, 16384);

					/* if larger, policy is to crop */
					LengthToCopy = RomLength;
					if (LengthToCopy > 16384)
					{
						LengthToCopy = 16384;
					}
					/* copy rom data into allocated block */
					memcpy(pData->ExpansionRomData[RomIndex], pRomData, LengthToCopy);
					/* if shorter, policy is to fill with 0x0ff */
					if (LengthToCopy < 16384)
					{
						memset(pData->ExpansionRomData[RomIndex] + LengthToCopy, 0x0ff, 16384 - LengthToCopy);
					}
					/* mark it as active */
					ExpansionRom_SetActiveState(pData, RomIndex, TRUE);

					/* ok! */
					Status = ARNOLD_STATUS_OK;
				}
				else
				{
					Status = ARNOLD_STATUS_OUT_OF_MEMORY;
				}
			}
		}
		else
		{
			/* invalid */
			Status = ARNOLD_STATUS_INVALID;
		}
	}
	else
	{
		/* the length is invalid */
		Status = ARNOLD_STATUS_INVALID_LENGTH;
	}

	/* return status */
	return Status;
}

const unsigned char *ExpansionRom_Get(ExpansionRomData *pData, int RomIndex)
{
	if (RomIndex >= MAX_EXPANSION_ROMS)
	{
		return NULL;
	}

	return pData->ExpansionRomData[RomIndex];
}

const unsigned char *ExpansionRom_GetSafe(ExpansionRomData *pData, int RomIndex)
{
	const unsigned char *pRomData = ExpansionRom_Get(pData, RomIndex);
	if (pRomData != NULL)
		return pRomData;

	return GetDummyReadRam();
}

BOOL    ExpansionRom_IsActive(ExpansionRomData *pData, int RomIndex)
{
	if (RomIndex >= MAX_EXPANSION_ROMS)
	{
		return FALSE;
	}

	return pData->ExpansionRomActive[RomIndex];
}


BOOL    ExpansionRom_IsAvailable(ExpansionRomData *pData, int RomIndex)
{
	if (RomIndex >= MAX_EXPANSION_ROMS)
	{
		return FALSE;
	}

	return pData->ExpansionRomAvailable[RomIndex];
}

void    ExpansionRom_SetActiveState(ExpansionRomData *pData, int RomIndex, BOOL State)
{
	if (RomIndex >= MAX_EXPANSION_ROMS)
	{
		return;
	}

	pData->ExpansionRomActive[RomIndex] = State;
}


void    ExpansionRom_SetAvailableState(ExpansionRomData *pData, int RomIndex, BOOL State)
{
	if (RomIndex >= MAX_EXPANSION_ROMS)
	{
		return;
	}

	pData->ExpansionRomAvailable[RomIndex] = State;
}

#if 0
/* this function is called to override the ExpansionROMTable entries with */
/* any expansion ROMs that are selected */
void	ExpansionROM_Override(void)
{
	int i;

	for (i=0; i<16; i++)
	{
		unsigned char *pExpansionRom;

		BOOL bCanOverride = TRUE;

		/* cpc hardware only */
		if (CPC_GetHardware()==CPC_HW_CPC)
		{
			/* cpc */

			if (
				/* is disc rom active? */
				(pDOS!=NULL) &&
				/* it's forced off */
				(!FDI_GetForceDiscRomOff())
				)
			{
				int nIndex = 0;
				if (CPC_GetExpLow())
				{
					nIndex = 7;
				}

				/* we can't override it */
				if (i==nIndex)
				{
					bCanOverride = FALSE;
				}
			}
		}

		if (bCanOverride)
		{
			/* get rom at rom index specified. An expansion rom
			in the same position as a on-board rom will be selected in
			preference to the on-board rom */
			pExpansionRom = ExpansionRom_Get(i);

			/* is it active ? */
			if ((pExpansionRom!=NULL) && (ExpansionRom_IsActive(i)))
			{
				/* perform override. */
				pExpansionRom = (pExpansionRom - 0x00c000);

				ExpansionROMTable[i] = pExpansionRom;
			}
		}
	}
}
#endif

void EmuDevice_RestoreFromSnapshot(int nDevice, int Config)
{
	if (m_pDevices[nDevice])
	{
		if (m_pDevices[nDevice]->m_RestorefromSnapshot)
		{
			m_pDevices[nDevice]->m_RestorefromSnapshot(Config);
		}
	}

}

BOOL EmuDevice_RespondsToDkRamSelection(int nDevice, int nSelection)
{
	if (m_pDevices[nDevice])
	{
		if (m_pDevices[nDevice]->m_DkRamData)
		{
			return m_pDevices[nDevice]->m_DkRamData->PageAvailable[nSelection];
		}
	}
	return FALSE;
}

unsigned char *EmuDevice_GetDkRamSelection(int nDevice, int nSelection)
{
	if (m_pDevices[nDevice])
	{
		if (m_pDevices[nDevice]->m_DkRamData)
		{
			return m_pDevices[nDevice]->m_DkRamData->Pages[nSelection];
		}
	}
	return NULL;
}

const char *EmuDevice_GetName(int nDevice)
{
	return m_pDevices[nDevice]->sDisplayName;
}

const char *EmuDevice_GetSaveName(int nDevice)
{
	return m_pDevices[nDevice]->sSaveName;
}

BOOL EmuDevice_IsEnabled(int nDevice)
{
	return m_bDevicesEnabled[nDevice];
}

BOOL EmuDevice_HasPassthrough(int nDevice)
{
	return m_pDevices[nDevice]->m_nFlags & DEVICE_FLAGS_HAS_PASSTHROUGH;
}

int EmuDevice_ConnectedTo(int nDevice)
{
	return m_pDevices[nDevice]->m_Connection;
}

int EmuDevice_GetFlags(int nDevice)
{
	return m_pDevices[nDevice]->m_nFlags;
}


int EmuDevice_GetNumButtons(int nDevice)
{
	if (m_pDevices[nDevice])
	{
		return m_pDevices[nDevice]->m_nButtons;
	}
	return 0;
}

const char *EmuDevice_GetButtonName(int nDevice, int nButton)
{
	if (m_pDevices[nDevice])
	{
		if (nButton < m_pDevices[nDevice]->m_nButtons)
		{
			return m_pDevices[nDevice]->m_pButtons[nButton].sButtonName;
		}
	}
	return "";
}

void EmuDevice_PressButton(int nDevice, int nButton)
{
	if (m_pDevices[nDevice])
	{
		if (nButton < m_pDevices[nDevice]->m_nButtons)
		{
			m_pDevices[nDevice]->m_pButtons[nButton].m_Press();
		}
	}
}


int EmuDevice_GetNumRoms(int nDevice)
{
	if (m_pDevices[nDevice])
	{
		return m_pDevices[nDevice]->m_nRoms;
	}
	return 0;
}


const char *EmuDevice_GetRomName(int nDevice, int nRom)
{
	if (m_pDevices[nDevice])
	{
		if (nRom < m_pDevices[nDevice]->m_nRoms)
		{
			return m_pDevices[nDevice]->m_pRoms[nRom].sRomName;
		}
	}
	return "";
}

const char *EmuDevice_GetRomSaveName(int nDevice, int nRom)
{
	if (m_pDevices[nDevice])
	{
		if (nRom < m_pDevices[nDevice]->m_nRoms)
		{
			return m_pDevices[nDevice]->m_pRoms[nRom].sRomSaveName;
		}
	}
	return "";
}


size_t EmuDevice_GetRomSize(int nDevice, int nRom)
{
	if (m_pDevices[nDevice])
	{
		if (nRom < m_pDevices[nDevice]->m_nRoms)
		{
			return m_pDevices[nDevice]->m_pRoms[nRom].m_nRomSize;
		}
	}
	return 0;
}

uint32_t EmuDevice_GetRomCRC(int nDevice, int nRom)
{
	if (m_pDevices[nDevice])
	{
		if (nRom < m_pDevices[nDevice]->m_nRoms)
		{
			return m_pDevices[nDevice]->m_pRoms[nRom].m_nCRC;
		}
	}
	return 0x0;
}

void EmuDevice_ClearRom(int nDevice, int nRom)
{
	if (m_pDevices[nDevice])
	{
		if (nRom < m_pDevices[nDevice]->m_nRoms)
		{
			if (m_pDevices[nDevice]->m_pRoms[nRom].m_ClearSystemRom)
			{
				m_pDevices[nDevice]->m_pRoms[nRom].m_ClearSystemRom();
			}
		}
	}
}

void EmuDevice_SetRom(int nDevice, int nRom, const unsigned char *pSystemRomData, unsigned long SystemRomLength)
{
	if (m_pDevices[nDevice])
	{
		if (nRom < m_pDevices[nDevice]->m_nRoms)
		{
			if (m_pDevices[nDevice]->m_pRoms[nRom].m_SetSystemRom)
			{
				const unsigned char *pRomData = pSystemRomData;
				unsigned long RomLength = SystemRomLength;

				AMSDOS_GetUseableSize(&pRomData, &RomLength);

				/* is rom length valid? */
				if (RomLength > 0)
				{
					m_pDevices[nDevice]->m_pRoms[nRom].m_SetSystemRom(pRomData, RomLength);
				}
				else
				{
					EmuDevice_ClearRom(nDevice, nRom);
				}
			}
		}
	}
}



int EmuDevice_GetNumSwitches(int nDevice)
{
	return m_pDevices[nDevice]->m_nSwitches;
}

const char *EmuDevice_GetSwitchName(int nDevice, int nSwitch)
{
	if (m_pDevices[nDevice])
	{
		if (nSwitch< m_pDevices[nDevice]->m_nSwitches)
		{
			return m_pDevices[nDevice]->m_pSwitches[nSwitch].sSwitchName;
		}
	}
	return "";
}

const char *EmuDevice_GetSwitchSaveName(int nDevice, int nSwitch)
{
	if (m_pDevices[nDevice])
	{
		if (nSwitch< m_pDevices[nDevice]->m_nSwitches)
		{
			return m_pDevices[nDevice]->m_pSwitches[nSwitch].sSwitchSaveName;
		}
	}
	return "";
}


BOOL EmuDevice_GetSwitchState(int nDevice, int nSwitch)
{
	if (m_pDevices[nDevice])
	{
		if (nSwitch < m_pDevices[nDevice]->m_nSwitches)
		{
			return m_pDevices[nDevice]->m_pSwitches[nSwitch].m_GetState();
		}
	}
	return FALSE;
}


void EmuDevice_SetSwitchState(int nDevice, int nSwitch, BOOL bState)
{
	if (m_pDevices[nDevice])
	{
		if (nSwitch < m_pDevices[nDevice]->m_nSwitches)
		{
			m_pDevices[nDevice]->m_pSwitches[nSwitch].m_SetState(bState);
		}
	}
}

int EmuDevice_GetDeviceByName(const char *sName)
{
	int i;

	if (sName == NULL)
		return -1;

	for (i = 0; i < EmuDevice_GetNumDevices(); i++)
	{
		if (m_pDevices[i]->sDisplayName != NULL)
		{
#ifdef WIN32
			if (stricmp(m_pDevices[i]->sDisplayName, sName) == 0)
#else
			if (strcasecmp(m_pDevices[i]->sDisplayName, sName)==0)
#endif
			{
				return i;
			}
		}

	}
	return -1;
}

/* need to define order too really. */
void EmuDevice_Enable(int nDevice, BOOL bState)
{
	if (bState != m_bDevicesEnabled[nDevice])
	{
		// add to end
		if (bState)
		{
			m_nEnabledDeviceId[m_nEnabledDevices] = nDevice;
			m_nEnabledDevices++;
		}
		else
		{
			int i;
			for (i = 0; i < m_nEnabledDevices; i++)
			{
				if (m_nEnabledDeviceId[i] == nDevice)
				{
					int j;
					for (j = 0; j < (m_nEnabledDevices - i); j++)
					{
						m_nEnabledDeviceId[j] = m_nEnabledDeviceId[j + 1];
					}
					m_nEnabledDevices--;
					break;
				}
			}
		}

		if (bState)
		{
			if (m_pDevices[nDevice]->m_Enable)
			{
				m_pDevices[nDevice]->m_Enable(TRUE);
			}
			int i;
			if (m_pDevices[nDevice]->m_pPortRead != NULL)
			{
				for (i = 0; i < m_pDevices[nDevice]->m_nPortRead; i++)
				{
					CPC_InstallReadPort(&m_pDevices[nDevice]->m_pPortRead[i]);
				}
			}
			if (m_pDevices[nDevice]->m_pMemoryRead != NULL)
			{
				for (i = 0; i < m_pDevices[nDevice]->m_nMemoryRead; i++)
				{
					CPC_InstallReadMemory(&m_pDevices[nDevice]->m_pMemoryRead[i]);
				}
			}
			if (m_pDevices[nDevice]->m_pMemoryWrite != NULL)
			{
				for (i = 0; i < m_pDevices[nDevice]->m_nMemoryWrite; i++)
				{
					CPC_InstallWriteMemory(&m_pDevices[nDevice]->m_pMemoryWrite[i]);
				}
			}
			if (m_pDevices[nDevice]->m_pPortWrite != NULL)
			{
				for (i = 0; i < m_pDevices[nDevice]->m_nPortWrite; i++)
				{
					CPC_InstallWritePort(&m_pDevices[nDevice]->m_pPortWrite[i]);
				}
			}
			if (m_pDevices[nDevice]->m_pResetFunction != NULL)
			{
				CPC_InstallResetFunction(m_pDevices[nDevice]->m_pResetFunction);
			}
			if (m_pDevices[nDevice]->m_pPowerFunction != NULL)
			{
				CPC_InstallPowerFunction(m_pDevices[nDevice]->m_pPowerFunction);
			}

			if (m_pDevices[nDevice]->m_pMemoryRethink != NULL)
			{
				CPC_InstallMemoryRethinkHandler(m_pDevices[nDevice]->m_pMemoryRethink);
			}
			if (m_pDevices[nDevice]->m_pCursorUpdateFunction != NULL)
			{
				CPC_InstallCursorUpdateFunction(m_pDevices[nDevice]->m_pCursorUpdateFunction);
			}
			if (m_pDevices[nDevice]->m_PrinterUpdate != NULL)
			{
				CPC_InstallPrinterUpdateFunction(m_pDevices[nDevice]->m_PrinterUpdate);
			}
			if (m_pDevices[nDevice]->m_JoystickReadFunction != NULL)
			{
				CPC_InstallJoystickReadFunction(m_pDevices[nDevice]->m_JoystickReadFunction);
			}
			for (i = 0; i < m_pDevices[nDevice]->m_nMemoryRanges; i++)
			{
				CPC_RegisterMemoryRange(&m_pDevices[nDevice]->m_pMemoryRange[i]);
			}
			if (m_pDevices[nDevice]->m_AudioUpdate != NULL)
			{
				CPC_InstallSoundUpdateFunction(m_pDevices[nDevice]->m_AudioUpdate);
			}
			if (m_pDevices[nDevice]->m_LightSensorFunction != NULL)
			{
				CPC_InstallLightSensorFunction(m_pDevices[nDevice]->m_LightSensorFunction);
			}
			if (m_pDevices[nDevice]->m_RetiFunction != NULL)
			{
				CPC_InstallRetiHandler(m_pDevices[nDevice]->m_RetiFunction);
			}
			if (m_pDevices[nDevice]->m_AckMaskableInterrupt != NULL)
			{
				CPC_InstallAckMaskableInterruptHandler(m_pDevices[nDevice]->m_AckMaskableInterrupt);
			}
		}
		else
		{
			int i;
			if (m_pDevices[nDevice]->m_Enable)
			{
				m_pDevices[nDevice]->m_Enable(FALSE);
			}
			if (m_pDevices[nDevice]->m_pPortRead != NULL)
			{
				for (i = 0; i < m_pDevices[nDevice]->m_nPortRead; i++)
				{
					CPC_UninstallReadPort(&m_pDevices[nDevice]->m_pPortRead[i]);
				}
			}
			if (m_pDevices[nDevice]->m_pPortWrite != NULL)
			{
				for (i = 0; i < m_pDevices[nDevice]->m_nPortWrite; i++)
				{
					CPC_UninstallWritePort(&m_pDevices[nDevice]->m_pPortWrite[i]);
				}
			}
			if (m_pDevices[nDevice]->m_pMemoryRead != NULL)
			{
				for (i = 0; i < m_pDevices[nDevice]->m_nMemoryRead; i++)
				{
					CPC_UninstallReadMemory(&m_pDevices[nDevice]->m_pMemoryRead[i]);
				}
			}
			if (m_pDevices[nDevice]->m_pMemoryWrite != NULL)
			{
				for (i = 0; i < m_pDevices[nDevice]->m_nMemoryWrite; i++)
				{
					CPC_UninstallWriteMemory(&m_pDevices[nDevice]->m_pMemoryWrite[i]);
				}
			}
			if (m_pDevices[nDevice]->m_pResetFunction != NULL)
			{
				CPC_UnInstallResetFunction(m_pDevices[nDevice]->m_pResetFunction);
			}
			if (m_pDevices[nDevice]->m_pPowerFunction != NULL)
			{
				CPC_UnInstallPowerFunction(m_pDevices[nDevice]->m_pPowerFunction);
			}

			if (m_pDevices[nDevice]->m_pMemoryRethink != NULL)
			{
				CPC_UnInstallMemoryRethinkHandler(m_pDevices[nDevice]->m_pMemoryRethink);
			}
			if (m_pDevices[nDevice]->m_pCursorUpdateFunction != NULL)
			{
				CPC_UnInstallCursorUpdateFunction(m_pDevices[nDevice]->m_pCursorUpdateFunction);
			}
			if (m_pDevices[nDevice]->m_PrinterUpdate != NULL)
			{
				CPC_UnInstallPrinterUpdateFunction(m_pDevices[nDevice]->m_PrinterUpdate);
			}
			if (m_pDevices[nDevice]->m_JoystickReadFunction != NULL)
			{
				CPC_UnInstallJoystickReadFunction(m_pDevices[nDevice]->m_JoystickReadFunction);
			}
			for (i = 0; i < m_pDevices[nDevice]->m_nMemoryRanges; i++)
			{
				CPC_UnRegisterMemoryRange(&m_pDevices[nDevice]->m_pMemoryRange[i]);
			}
			if (m_pDevices[nDevice]->m_AudioUpdate != NULL)
			{
				CPC_UnInstallSoundUpdateFunction(m_pDevices[nDevice]->m_AudioUpdate);
			}
			if (m_pDevices[nDevice]->m_LightSensorFunction != NULL)
			{
				CPC_UnInstallLightSensorFunction(m_pDevices[nDevice]->m_LightSensorFunction);
			}
			if (m_pDevices[nDevice]->m_RetiFunction != NULL)
			{
				CPC_UnInstallRetiHandler(m_pDevices[nDevice]->m_RetiFunction);
			}
			if (m_pDevices[nDevice]->m_AckMaskableInterrupt != NULL)
			{
				CPC_UnInstallAckMaskableInterruptHandler(m_pDevices[nDevice]->m_AckMaskableInterrupt);
			}

		}

		m_bDevicesEnabled[nDevice] = bState;
	}
}


int RegisterDevice(EmuDevice *pDevice)
{
	int nDevice = -1;
	if (m_nDevices < MAX_DEVICES)
	{
		nDevice = m_nDevices;
		m_pDevices[m_nDevices] = pDevice;
		if (pDevice->m_Init)
		{
			int i;

			/* clear all system roms to known uninitialised state */
			for (i = 0; i < EmuDevice_GetNumRoms(m_nDevices); i++)
			{
				EmuDevice_ClearRom(m_nDevices, i);
			}

			pDevice->m_Init();

		}
		m_nDevices++;
	}
	return nDevice;
}

void DoUnRegisterDevice(EmuDevice *pDevice)
{
	/* perform device specific shutdown if it has it */
	if (pDevice->m_Shutdown)
	{
		pDevice->m_Shutdown();
	}
	/* if device exposes expansion roms then free them now */
	if (pDevice->m_ExpansionRoms)
	{
		ExpansionRom_Finish(pDevice->m_ExpansionRoms);
	}
}

void UnRegisterDevice(EmuDevice *pDevice)
{
	int i;
	for (i = 0; i < m_nDevices; i++)
	{
		/* found it. */
		if (m_pDevices[i] == pDevice)
		{
			int nCount = (m_nDevices - 1) - i;

			if (nCount != 0)
			{
				/* copy down */
				int j;
				for (j = 0; j < nCount; j++)
				{
					m_pDevices[j] = m_pDevices[j + 1];
				}
			}
			m_nDevices--;

			DoUnRegisterDevice(pDevice);
			break;
		}

	}
}

void UnRegisterAllDevices(void)
{
	int i;
	for (i = m_nDevices - 1; i >= 0; i--)
	{
		DoUnRegisterDevice((EmuDevice *)m_pDevices[i]);
	}
	m_nDevices = 0;
}

int EmuDevice_GetNumDevices(void)
{
	return m_nDevices;
}

void EmuDevice_ClearRomData(unsigned char *pDest, unsigned long DestSize)
{
	memset(pDest, 0x0ff, DestSize);
}

void EmuDevice_CopyRomData(unsigned char *pDest, unsigned long DestSize, const unsigned char *pSrc, unsigned long SrcSize)
{
	unsigned long CopySize;

	if (pSrc == NULL)
	{
		memset(pDest, 0x0ff, DestSize);
		return;
	}

	/* if dest is smaller, then choose dest size */
	/* if src is smaller choose src size */
	CopySize = DestSize;
	if (SrcSize < DestSize)
	{
		CopySize = SrcSize;
	}
	memcpy(pDest, pSrc, CopySize);
	if (CopySize < DestSize)
	{
		/* fill remainder if smaller */
		memset(pDest + CopySize, 0x0ff, DestSize - CopySize);
	}
}
