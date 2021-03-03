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
#include "ItemData.h"

extern "C"
{
#include "../cpc/z8536.h"
#include "../cpc/asic.h"
#include "../cpc/kcc.h"
#include "../cpc/i8255.h"
#include "../cpc/aleste.h"
#include "../cpc/cpc.h"
#include "../cpc/debugger/parse.h"
#include "../cpc/psg.h"
#include "../cpc/fdd.h"
#include "../cpc/fdi.h"
#include "../cpc/crtc.h"
#include "../cpc/printer.h"
#include "../cpc/fdc.h"
#include "../cpc/monitor.h"
#include "../cpc/crtc_type3.h"
#include "../cpc/crtc_type4.h"


	extern AY_3_8912 OnBoardAY;
}

static const wxChar *sYes = wxT("Yes");
static const wxChar *sNo = wxT("No");


void ItemData::GenerateNumber(int nBits, bool bSigned, int nNumber, int nRepresentation, wxString &sValue)
{
	switch (nRepresentation)
	{
	case ITEM_REPRESENTATION_BOOL:
	{
		if (nNumber!=0)
		{
			sValue = sYes;
		}
		else
		{
			sValue = sNo;
		}
	}
	break;
	case ITEM_REPRESENTATION_BINARY:
	{
#if 0
		sValue = wxT("%");
		int nMask = 1 << (nBits - 1);
		for (int i = 0; i < nBits; i++)
		{
			wxChar ch;
			if ((nNumber & nMask) != 0)
			{
				ch = wxT('1');
			}
			else
			{
				ch = wxT('0');
			}
			sValue += ch;
			nMask = nMask >> 1;
		}
#endif
		wxStringBuffer buf(sValue, nBits + 2);
		wxChar *pChar = (wxChar *)buf;
		pChar[0] = _T('%');
		++pChar;
		int nMask = 1 << (nBits - 1);
		for (int i = 0; i < nBits; i++)
		{
			wxChar ch;
			if ((nNumber & nMask) != 0)
			{
				ch = wxT('1');
			}
			else
			{
				ch = wxT('0');
			}
			pChar[0] = ch;
			++pChar;
			nMask = nMask>>1;
		}
		pChar[0] = wxT('\0');
	}
	break;
	case ITEM_REPRESENTATION_DECIMAL:
	{
		// a quicker way?
		sValue.Printf(wxT("%d"), nNumber);
	}
	break;
#if 0
	case ITEM_REPRESENTATION_STRING:
	{
		// number in string table range?
		if (nNumber < m_nStringTableLength)
		{
			if (m_sStringTable != NULL)
			{
				// look it up and return string
				sValue = m_sStringTable[nNumber];
				break;
			}
		}

		// default if number not in range or string table not set
		sValue = wxT("??");
	}
	break;
#endif
	case ITEM_REPRESENTATION_HEX:
	{
		int nNibbles = 1;
		int nShift = 0;
		if (nBits == 16)
		{
			nShift = 12;
			nNibbles = 4;
		}
		else
			if (nBits == 8)
			{
				nShift = 4;
				nNibbles = 2;
			}
		wxStringBuffer buf(sValue, nNibbles + 2);
		wxChar *pChar = (wxChar *)buf;
		pChar[0] = _T('&');
		++pChar;
		for (int i=0; i<nNibbles; i++)
		{
			wxChar nibChar;
			int nNibble = (nNumber>>nShift) & 0x0f;
			if (nNibble>=10)
			{
				nibChar = wxT('A')+(nNibble-10);
			}
			else
			{
				nibChar = wxT('0')+nNibble;
			}
			pChar[0] = nibChar;
			++pChar;
			nShift = nShift-4;
	}
		pChar[0] = wxT('\0');

#if 0
		wxString sFormat;
		if (nBits == 16)
		{
			sFormat = wxT("&%04x");
		}
		else
			if (nBits == 8)
			{
				sFormat = wxT("&%02x");
			}
			else
			{
				sFormat = wxT("&%1x");
			}
		sValue.Printf(sFormat, nNumber);
#endif
			}
	break;
		}
	}


// there must be a better way to do this..?
// we also want hardware to expose it's own variables.
int ItemData::GetValue(int nCode, int nIndex)
{
	int nValue = 0;
	switch (nCode)
	{
	case CODE_ASIC_UNLOCKED:
	{
		nValue = ASIC_GetUnLockState();
	}
	break;
	case CODE_ASIC_DCSR:
	{
		nValue = ASIC_GetDCSR();
	}
	break;
	case CODE_ASIC_SSCR:
	{
		nValue = ASIC_GetSSCR();
	}
	break;
	case CODE_ASIC_GA_INTERRUPT_LINE_COUNT:
	{
		nValue = ASIC_GateArray_GetInterruptLineCount();
	}
	break;
	case CODE_ASIC_INTERRUPT_OUTPUT:
	{
		nValue = ASIC_GetInterruptRequest();
	}
	break;
	case CODE_ASIC_GA_HBLANK_COUNT:
	{
		nValue = ASIC_GateArray_GetHBlankCount();
	}
	break;
	case CODE_ASIC_GA_HBLANK_ACTIVE:
	{
		nValue = ASIC_GateArray_GetHBlankActive();
	}
	break;
	case CODE_ASIC_GA_VBLANK_COUNT:
	{
		nValue = ASIC_GateArray_GetVBlankCount();
	}
	break;
	case CODE_ASIC_GA_VBLANK_ACTIVE:
	{
		nValue = ASIC_GateArray_GetVBlankActive();
	}
	break;
	case CODE_ALESTE_PEN:
	{
		nValue = Aleste_GetCurrentPen();
	}
	break;
	case CODE_ALESTE_COLOUR:
	{
		nValue = Aleste_GetPenColour(nIndex);
	}
	break;
	case CODE_ALESTE_MAPPER:
	{
		nValue = Aleste_GetMapper(nIndex);
	}
	break;
	case CODE_ALESTE_EXTPORT:
	{
		nValue = Aleste_GetExtport();
	}
	break;
	case CODE_CYCLE_COUNTER:
	{
		nValue = Computer_GetCycleCounter();
	}
	break;
	case CODE_KCC_FN:
	{
		nValue = KCC_GetFN();
	}
	break;
	case CODE_KCC_MF:
	{
		nValue = KCC_GetMF();
	}
	break;
	case CODE_KCC_COLOUR:
	{
		nValue = KCC_GetColour(nIndex);
	}
	break;
	case CODE_Z8536_REGISTERS:
	{
		nValue = Z8536_GetRegisterData(nIndex);
	}
	break;
	case CODE_Z8536_INPUTS:
	{
		nValue = Z8536_GetInputs(nIndex);
	}
	break;
	case CODE_Z8536_OUTPUTS:
	{
		nValue = Z8536_GetOutputs(nIndex);
	}
	break;
	case CODE_CPC_GA_INTERRUPT_LINE_COUNT:
	{
		nValue = GateArray_GetInterruptLineCount();
	}
	break;
	case CODE_CPC_GA_INTERRUPT_OUTPUT:
	{
		nValue = GateArray_GetInterruptRequest();
	}
	break;
	case CODE_CPC_GA_HBLANK_COUNT:
	{
		nValue = GateArray_GetHBlankCount();
	}
	break;
	case CODE_CPC_GA_HBLANK_ACTIVE:
	{
		nValue = GateArray_GetHBlankActive();
	}
	break;
	case CODE_CPC_GA_VBLANK_COUNT:
	{
		nValue = GateArray_GetVBlankCount();
	}
	break;
	case CODE_CPC_GA_VBLANK_ACTIVE:
	{
		nValue = GateArray_GetVBlankActive();
	}
	break;
	case CODE_CPC_GA_SELECTED_PEN:
	{
		nValue = GateArray_GetSelectedPen();
	}
	break;
	case CODE_CPC_GA_PALETTE_COLOUR:
	{
		nValue = GateArray_GetPaletteColour(nIndex);
	}
	break;
	case CODE_ASIC_PALETTE_RGB:
	{
		nValue = (ASIC_GetRed(nIndex) << 8) | (ASIC_GetGreen(nIndex) << 4) | ASIC_GetBlue(nIndex);
	}
	break;
	case CODE_ASIC_GA_SELECTED_PEN:
	{
		nValue = ASIC_GateArray_GetSelectedPen();
	}
	break;
	case CODE_ASIC_GA_MRER:
	{
		nValue = ASIC_GateArray_GetMultiConfiguration();
	}
	break;
	case CODE_CPC_GA_MRER:
	{
		nValue = GateArray_GetMultiConfiguration();
	}
	break;
	case CODE_ASIC_SSA:
	{
		nValue = ASIC_GetSSA();
	}
	break;
	case CODE_ASIC_ANALOGUE_INPUT:
	{
		nValue = ASIC_GetAnalogueInput(nIndex);
	}
	break;
	case CODE_ASIC_SPRITE_X:
	{
		nValue = ASIC_GetSpriteX(nIndex);
	}
	break;
	case CODE_ASIC_SPRITE_Y:
	{
		nValue = ASIC_GetSpriteY(nIndex);
	}
	break;
	case CODE_ASIC_SPRITE_MAG:
	{
		nValue = ASIC_GetSpriteMagnification(nIndex);
	}
	break;
	case CODE_ASIC_DMA_ADDR:
	{
		nValue = ASIC_DMA_GetChannelAddr(nIndex);
	}
	break;
	case CODE_ASIC_DMA_PRESCALE:
	{
		nValue = ASIC_DMA_GetChannelPrescale(nIndex);
	}
	break;
	case CODE_ASIC_IVR:
	{
		nValue = ASIC_GetIVR();
	}
	break;
	case CODE_VRAM_ADDR:
	{
		switch (CPC_GetHardware())
		{
			default
				:
		case CPC_HW_CPC:
		{
			nValue = CPC_GetVRAMAddr();
		}
		break;
		case CPC_HW_CPCPLUS:
		{
			nValue = ASIC_GetVRAMAddr();
		}
		break;
		case CPC_HW_KCCOMPACT:
		{
			nValue = KCC_GetVRAMAddr();
		}
		break;
		case CPC_HW_ALESTE:
		{
			nValue = Aleste_GetVRAMAddr();
		}
		break;
		}
	}
	break;
	case CODE_CRTC_INTERLACE_FRAME:
	{
		nValue = CRTC_GetInterlaceFrame();
	}
	break;
	case CODE_CRTC_LINES_AFTER_FRAME_START:
	{
		nValue = CRTC_GetLinesAfterFrameStart();
	}
	break;
	case CODE_CRTC_LINES_AFTER_VSYNC_START:
	{
		nValue = CRTC_GetLinesAfterVsyncStart();
	}
	break;

	case CODE_CRTC_CHARS_AFTER_HSYNC_START:
	{
		nValue = CRTC_GetCharsAfterHsyncStart();
	}
	break;

	case CODE_CRTC_FRAME_WIDTH:
	{
		nValue = CRTC_GetRegisterData(0) + 1;
	}
	break;
	case CODE_CRTC_FRAME_HEIGHT:
	{
		nValue = CRTC_GetRegisterData(4) + 1;
	}
	break;

	case CODE_CRTC_CHAR_HEIGHT:
	{
		nValue = CRTC_GetRegisterData(9) + 1;
	}
	break;


	case CODE_CRTC_ACTUAL_HORIZONTAL_SYNC_WIDTH:
	{
		nValue = CRTC_GetActualHorizontalSyncWidth();
	}
	break;
	case CODE_CRTC_ACTUAL_VERTICAL_SYNC_WIDTH:
	{
		nValue = CRTC_GetActualVerticalSyncWidth();
	}
	break;
	case CODE_CRTC_STATUS_REGISTER:
	{
		nValue = CRTC_ReadStatusRegister();
	}
	break;
	case CODE_CRTC3_STATUS_REGISTER1:
	{
		nValue = CRTC3_GetStatusRegister1();
	}
	break;
	case CODE_CRTC3_STATUS_REGISTER2:
	{
		nValue = CRTC3_GetStatusRegister2();
	}
	break;
	case CODE_CRTC4_STATUS_REGISTER1:
	{
		nValue = CRTC4_GetStatusRegister1();
	}
	break;
	case CODE_CRTC4_STATUS_REGISTER2:
	{
		nValue = CRTC4_GetStatusRegister2();
	}
	break;
	case CODE_CRTC_REGISTER_DATA:
	{
		nValue = CRTC_GetRegisterData(nIndex);
	}
	break;
	case CODE_PSG_MODE:
	{
		nValue = PSG_GetMode(&OnBoardAY);
	}
	break;
	case CODE_PSG_REGISTER_DATA:
	{
		nValue = PSG_GetRegisterData(&OnBoardAY, nIndex);
	}
	break;
	case CODE_PSG_IO_INPUT:
	{
		nValue = PSG_GetPortInputs(&OnBoardAY, nIndex);
	}
	break;
	case CODE_PSG_IO_OUTPUT:
	{
		nValue = PSG_GetPortOutputs(&OnBoardAY, nIndex);
	}
	break;
	case CODE_PSG_BDIR:
	{
		nValue = PSG_GetBDIR(&OnBoardAY);
	}
	break;
	case CODE_PSG_BC1:
	{
		nValue = PSG_GetBC1(&OnBoardAY);
	}
	break;
	case CODE_PSG_BC2:
	{
		nValue = PSG_GetBC2(&OnBoardAY);
	}
	break;
	case CODE_PSG_SELECTED_REGISTER:
	{
		nValue = PSG_GetSelectedRegister(&OnBoardAY);
	}
	break;
	case CODE_PPI_CONTROL:
	{
		if (CPC_GetHardware() == CPC_HW_CPCPLUS)
		{
			nValue = ASIC_PPI_GetControlForSnapshot();
		}
		else
		{
			nValue = PPI_GetControlForSnapshot();
		}
	}
	break;
	case CODE_PPI_PORT_RESOLVED_INPUT:
	{
	}
	break;
	case CODE_PPI_PORT_INPUT:
	{
		if (CPC_GetHardware() == CPC_HW_CPCPLUS)
		{
			nValue = ASIC_PPI_GetPortInput(nIndex);
		}
		else
		{
			nValue = PPI_GetPortInput(nIndex);
		}
	}
	break;
	case CODE_PPI_PORT_RESOLVED_OUTPUT:
	{
	}
	break;
	case CODE_PPI_PORT_OUTPUT:
	{
		if (CPC_GetHardware() == CPC_HW_CPCPLUS)
		{
			nValue = ASIC_PPI_GetOutputPort(nIndex);
		}
		else
		{
			nValue = PPI_GetOutputPort(nIndex);
		}
	}
	break;
	case CODE_PRINTER_BUSY:
	{
		nValue = Printer_GetBusyState();
	}
	break;
	case CODE_PRINTER_STROBE:
	{
		nValue = Printer_GetStrobeOutputState();
	}
	break;
	case CODE_PRINTER_DATA:
	{
		nValue = Printer_Get8BitData();
	}
	break;
	case CODE_CRTC_SELECTED_REGISTER:
	{
		nValue = CRTC_GetSelectedRegister();
	}
	break;
	case CODE_CRTC_HCC:
	{
		nValue = CRTC_GetHCC();
	}
	break;
	case CODE_CRTC_VCC:
	{
		nValue = CRTC_GetVCC();
	}
	break;
	case CODE_CRTC_RA:
	{
		nValue = CRTC_GetRA();
	}
	break;
	case CODE_CRTC_MA_OUTPUT:
	{
		nValue = CRTC_GetMAOutput();
	}
	break;
	case CODE_CRTC_RA_OUTPUT:
	{
		nValue = CRTC_GetRAOutput();
	}
	break;
	case CODE_CRTC_VSYNC_OUTPUT:
	{
		nValue = CRTC_GetVsyncOutput();
	}
	break;
	case CODE_CRTC_HSYNC_OUTPUT:
	{
		nValue = CRTC_GetHsyncOutput();
	}
	break;
	case CODE_CRTC_DISPTMG_OUTPUT:
	{
		nValue = CRTC_GetDispTmgOutput();
	}
	break;
	case CODE_CRTC_CUDISP_OUTPUT:
	{
		nValue = CRTC_GetCursorOutput();
	}
	break;
	case CODE_ASIC_PRI_LINE:
	{
		nValue = ASIC_GetPRILine();
	}
	break;
	case CODE_ASIC_SPLT_LINE:
	{
		nValue = ASIC_GetSPLTLine();
	}
	break;
	case CODE_ASIC_PRI_VCC:
	{
		nValue = ASIC_GetPRIVCC();
	}
	break;
	case CODE_ASIC_PRI_RCC:
	{
		nValue = ASIC_GetPRIRCC();
	}
	break;
	case CODE_ASIC_PRI:
	{
		nValue = ASIC_GetPRI();
	}
	break;
	case CODE_ASIC_SPLT:
	{
		nValue = ASIC_GetSPLT();
	}
	break;
	case CODE_ASIC_SPLT_VCC:
	{
		nValue = ASIC_GetSPLTVCC();
	}
	break;
	case CODE_ASIC_SPLT_RCC:
	{
		nValue = ASIC_GetSPLTRCC();
	}
	break;
	case CODE_ASIC_RMR2:
	{
		nValue = ASIC_GetSecondaryRomMapping();
	}
	break;
	case CODE_FDI_DRIVE:
	{
		nValue = FDI_GetPhysicalDrive();
	}
	break;
	case CODE_FDI_SIDE:
	{
		nValue = FDI_GetPhysicalSide();
	}
	break;
	case CODE_FDI_MOTOR:
	{
		nValue = FDI_GetMotorState();
	}
	break;
	case CODE_FDC_COMMAND_BYTE:
	{
		nValue = FDC_GetCommandByte(nIndex);
	}
	break;
	case CODE_FDC_PCN:
	{
		nValue = FDC_GetPCN(nIndex);
	}
	break;
	case CODE_FDC_DMA_MODE:
	{
		nValue = FDC_GetDmaMode();
	}
	break;
	case CODE_FDC_HEAD_LOAD_TIME:
	{
		nValue = FDC_GetHeadLoadTime();
	}
	break;
	case CODE_FDC_HEAD_UNLOAD_TIME:
	{
		nValue = FDC_GetHeadUnLoadTime();
	}
	break;
	case CODE_FDC_STEP_RATE_TIME:
	{
		nValue = FDC_GetStepRateTime();
	}
	break;
	case CODE_FDC_DRIVE_OUTPUT:
	{
		nValue = FDC_GetDriveOutput();
	}
	break;
	case CODE_FDC_SIDE_OUTPUT:
	{
		nValue = FDC_GetSideOutput();
	}
	break;
	case CODE_FDC_INTERRUPT_OUTPUT:
	{
		nValue = FDC_GetInterruptOutput();
	}
	break;
	case CODE_FDC_MSR:
	{
		nValue = FDC_GetMainStatusRegister();
	}
	break;
	case CODE_Z80_OUTPUTS:
	{
		nValue = CPU_GetOutputs();
	}
	break;
	case CODE_Z80_REG_AF:
	{
		nValue = CPU_GetReg(CPU_AF);
	}
	break;
	case CODE_Z80_REG_BC:
	{
		nValue = CPU_GetReg(CPU_BC);
	}
	break;
	case CODE_Z80_REG_DE:
	{
		nValue = CPU_GetReg(CPU_DE);
	}
	break;
	case CODE_Z80_REG_HL:
	{
		nValue = CPU_GetReg(CPU_HL);
	}
	break;
	case CODE_Z80_REG_AFALT:
	{
		nValue = CPU_GetReg(CPU_AF2);
	}
	break;
	case CODE_Z80_REG_BCALT:
	{
		nValue = CPU_GetReg(CPU_BC2);
	}
	break;
	case CODE_Z80_REG_DEALT:
	{
		nValue = CPU_GetReg(CPU_DE2);
	}
	break;
	case CODE_Z80_REG_HLALT:
	{
		nValue = CPU_GetReg(CPU_HL2);
	}
	break;
	case CODE_Z80_IFF1:
	{
		nValue = CPU_GetReg(CPU_IFF1);
	}
	break;
	case CODE_Z80_IFF2:
	{
		nValue = CPU_GetReg(CPU_IFF2);
	}
	break;
	case CODE_Z80_REG_IX:
	{
		nValue = CPU_GetReg(CPU_IX);
	}
	break;
	case CODE_Z80_REG_IXH:
	{
		nValue = ((CPU_GetReg(CPU_IX) >> 8) & 0x0ff);
	}
	break;
	case CODE_Z80_REG_IXL:
	{
		nValue = CPU_GetReg(CPU_IX) & 0x0ff;
	}
	break;
	case CODE_Z80_REG_IY:
	{
		nValue = CPU_GetReg(CPU_IY);
	}
	break;
	case CODE_Z80_REG_IYH:
	{
		nValue = ((CPU_GetReg(CPU_IY) >> 8) & 0x0ff);
	}
	break;
	case CODE_Z80_REG_IYL:
	{
		nValue = CPU_GetReg(CPU_IY) & 0x0ff;
	}
	break;
	case CODE_Z80_REG_PC:
	{
		nValue = CPU_GetReg(CPU_PC);
	}
	break;
	case CODE_Z80_REG_SP:
	{
		nValue = CPU_GetReg(CPU_SP);
	}
	break;
	case CODE_Z80_REG_MEMPTR:
	{
		nValue = CPU_GetReg(CPU_MEMPTR);
	}
	break;
	case CODE_Z80_IO_PORT:
	{
		nValue = CPU_GetIOPort();
	}
	break;
	//	case CODE_Z80_INTERRUPT_VECTOR:
	//	{
	//		nValue = CPU_GetInterruptVector();
	//	}
	//	break;
	case CODE_Z80_DATABUS:
	{
		nValue = CPU_GetDataBus();
	}
	break;

	case CODE_Z80_INTERRUPT_REQUEST:
	{
		nValue = CPU_GetInterruptRequest();
	}
	break;
	case CODE_Z80_NMI_INPUT:
	{
		nValue = CPU_GetNMIInput();
	}
	break;
	//	case CODE_Z80_NMI_REQUEST:
	//	{
	//		nValue = CPU_GetNMIInterruptRequest();
	//	}
	break;
	case CODE_Z80_INT_TABLE_ADDR:
	{
		nValue = CPU_GetIntVectorAddress();
	}
	break;
	case CODE_Z80_REG_A:
	{
		nValue = CPU_GetReg(CPU_A);
	}
	break;
	case CODE_Z80_REG_B:
	{
		nValue = CPU_GetReg(CPU_B);
	}
	break;
	case CODE_Z80_REG_C:
	{
		nValue = CPU_GetReg(CPU_C);
	}
	break;
	case CODE_Z80_REG_D:
	{
		nValue = CPU_GetReg(CPU_D);
	}
	break;
	case CODE_Z80_REG_E:
	{
		nValue = CPU_GetReg(CPU_E);
	}
	break;
	case CODE_CPU_FLAGS:
	{
		nValue = CPU_GetFlag(nIndex);
	}
	break;

	case CODE_Z80_REG_F:
	{
		nValue = CPU_GetReg(CPU_F);
	}
	break;
	case CODE_Z80_REG_H:
	{
		nValue = CPU_GetReg(CPU_H);
	}
	break;
	case CODE_Z80_REG_L:
	{
		nValue = CPU_GetReg(CPU_L);
	}
	break;
	case CODE_Z80_REG_I:
	{
		nValue = CPU_GetReg(CPU_I);
	}
	break;
	case CODE_Z80_REG_IM:
	{
		nValue = CPU_GetReg(CPU_IM);
	}
	break;
	case CODE_Z80_REG_R:
	{
		nValue = CPU_GetReg(CPU_R);
	}
	break;
	case CODE_Z80_IO_DATA:
	{
		nValue = CPU_GetIOData();
	}
	break;
	case CODE_FDD_TRACK:
	{
		FDD *pDrive = FDD_GetDrive(nIndex);
		nValue = pDrive->CurrentTrack;
	}
	break;
	case CODE_FDD_FLAGS:
	{
		FDD *pDrive = FDD_GetDrive(nIndex);
		nValue = pDrive->Flags;
	}
	break;
	case CODE_MONITOR_VSYNC_STATE:
	{
		nValue = Monitor_GetVsyncState();
	}
	break;
	case CODE_MONITOR_HSYNC_STATE:
	{
		nValue = Monitor_GetHsyncState();
	}
	break;
#if 0
	case CODE_MONITOR_HORIZONTAL_POSITION:
	{
		nValue  = Monitor_GetHorizontalPosition();
	}
	break;
	case CODE_MONITOR_VERTICAL_POSITION:
	{
		nValue  = Monitor_GetVerticalPosition();
	}
	break;
	case CODE_MONITOR_SEEN_HSYNC:
	{
		nValue  = Monitor_SeeHsync();
	}
	break;
	case CODE_MONITOR_SEEN_VSYNC:
	{
		nValue  = Monitor_SeeVsync();
	}
	break;
	case CODE_MONITOR_COUNT_AFTER_VSYNC:
	{
		nValue = Monitor_CountAfterVsync();
	}
	break;
	case CODE_MONITOR_COUNT_AFTER_HSYNC:
	{
		nValue = Monitor_CountAfterHsync();
	}
	break;
	case CODE_MONITOR_HORZ_ADJUSTMENT:
	{
		nValue = Monitor_HorzStartAdjustment();
	}
	break;
	case CODE_MONITOR_VERT_ADJUSTMENT:
	{
		nValue = Monitor_VertStartAdjustment();
	}
	break;
#endif
	case CODE_KEYBOARD_ROW:
	{
		nValue = Keyboard_GetLine(nIndex);
	}
	break;
	case CODE_KEYBOARD_SELECTED_ROW:
	{
		nValue = Keyboard_GetSelectedLine();
	}
	break;

	default
		:
			break;
	}
	return nValue;
	}

void ItemData::Generate16BitNumber(int nNumber, int nRepresentation, wxString &sValue)
{
	GenerateNumber(16, false, nNumber, nRepresentation, sValue);
}


void ItemData::Generate8BitNumber(int nNumber, int nRepresentation, wxString &sValue)
{
	GenerateNumber(8, false, nNumber, nRepresentation, sValue);
}

void ItemData::GenerateBool(bool bValue, wxString &sValue)
{
	if (bValue)
	{
		sValue = sYes;
	}
	else
	{
		sValue = sNo;
	}
}

bool ItemData::RefreshNeeded(wxString &sValue)
{
	int nValue = GetValue(GetValueCode(), GetIndex());
	if ((m_bDirty) || (m_nValue != nValue))
	{
		m_bDirty = false;
		if (m_nValue != nValue)
		{
			SetValueChanged();
		}
		m_nValue = nValue;
		switch (GetType())
		{
		case ItemData::ITEM_TYPE_BOOL:
		{
			bool bResult = (((nValue >> GetShift())&GetMask()) == GetComparison());
			if (GetNot())
			{
				bResult = !bResult;
			}
			ItemData::GenerateBool(bResult, sValue);
		}
		break;
		case ItemData::ITEM_TYPE_8BIT:
		{
			ItemData::Generate8BitNumber((nValue >> GetShift())&GetMask(), GetRepresentation(), sValue);
		}
		break;
		case ItemData::ITEM_TYPE_16BIT:
		{
			ItemData::Generate16BitNumber((nValue >> GetShift())&GetMask(), GetRepresentation(), sValue);
		}
		break;

		case ItemData::ITEM_TYPE_STRING_INDEX:
		{
			if (m_sStringTable != NULL)
			{
				int nIndex = (nValue >> GetShift())&GetMask();
				if ((nIndex >= 0) && (nIndex < m_nStringTableLength))
				{
					sValue = m_sStringTable[nIndex];
					break;
				}
			}
			sValue = wxT("No String");
		}
		break;
		}
		return true;
	}
	return false;
}
