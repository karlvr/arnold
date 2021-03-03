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
#ifndef __ITEM_DATA_HEADER_INCLUDED__
#define __ITEM_DATA_HEADER_INCLUDED__

#include <wx/string.h>

enum
{
	CODE_Z80_REG_AF,
	CODE_Z80_REG_A,
	CODE_Z80_REG_BC,
	CODE_Z80_REG_B,
	CODE_Z80_REG_C,
	CODE_Z80_REG_DE,
	CODE_Z80_REG_D,
	CODE_Z80_REG_E,
	CODE_Z80_REG_F,
	CODE_Z80_REG_HL,
	CODE_Z80_REG_H,
	CODE_Z80_REG_L,
	CODE_Z80_REG_PC,
	CODE_Z80_REG_SP,
	CODE_Z80_REG_AFALT,
	CODE_Z80_REG_BCALT,
	CODE_Z80_REG_DEALT,
	CODE_Z80_REG_HLALT,
	CODE_Z80_REG_IX,
	CODE_Z80_REG_IXH,
	CODE_Z80_REG_IXL,
	CODE_Z80_REG_IY,
	CODE_Z80_REG_IYH,
	CODE_Z80_REG_IYL,
	CODE_Z80_REG_I,
	CODE_Z80_REG_R,
	CODE_Z80_IFF1,
	CODE_Z80_IFF2,
	CODE_Z80_INT_TABLE_ADDR,
	CODE_Z80_REG_IM,
	CODE_Z80_REG_MEMPTR,
	CODE_Z80_IO_PORT,
	CODE_Z80_IO_DATA,
	CODE_Z80_INTERRUPT_VECTOR,
	CODE_Z80_INTERRUPT_REQUEST,
	CODE_Z80_NMI_INPUT,
	CODE_Z80_NMI_REQUEST,
	CODE_Z80_DATABUS,
	CODE_Z80_OUTPUTS,
	CODE_CPU_FLAGS,
	CODE_ASIC_UNLOCKED,
	CODE_ASIC_DCSR,
	CODE_ASIC_IVR,
	CODE_ASIC_DMA_ADDR,
	CODE_ASIC_DMA_PRESCALE,
	CODE_ASIC_ANALOGUE_INPUT,
	CODE_ASIC_PRI,
	CODE_ASIC_PRI_VCC,
	CODE_ASIC_PRI_RCC,
	CODE_ASIC_SSA,
	CODE_ASIC_SPLT,
	CODE_ASIC_SPLT_VCC,
	CODE_ASIC_SPLT_RCC,
	CODE_ASIC_SSCR,
	CODE_ASIC_RMR2,
	CODE_ASIC_SPRITE_X,
	CODE_ASIC_SPRITE_Y,
	CODE_ASIC_SPRITE_MAG,
	CODE_ASIC_PALETTE_RGB,
	CODE_ASIC_GA_MRER,
	CODE_ASIC_GA_SELECTED_PEN,
	CODE_ASIC_SELECTED_CARTRIDGE_PAGE,
	CODE_ASIC_GA_INTERRUPT_LINE_COUNT,
	CODE_ASIC_INTERRUPT_OUTPUT,
	CODE_ASIC_GA_HBLANK_COUNT,
	CODE_ASIC_GA_HBLANK_ACTIVE,
	CODE_ASIC_GA_VBLANK_COUNT,
	CODE_ASIC_GA_VBLANK_ACTIVE,
	CODE_ASIC_PRI_LINE,
	CODE_ASIC_SPLT_LINE,
	CODE_KCC_FN,
	CODE_KCC_MF,
	CODE_KCC_COLOUR,
	CODE_Z8536_REGISTERS,
	CODE_Z8536_INPUTS,
	CODE_Z8536_OUTPUTS,
	CODE_ALESTE_PEN,
	CODE_ALESTE_COLOUR,
	CODE_ALESTE_MF,
	CODE_ALESTE_MAPPER,
	CODE_ALESTE_EXTPORT,
	CODE_CPC_GA_SELECTED_PEN,
	CODE_CPC_GA_MRER,
	CODE_CPC_GA_PALETTE_COLOUR,
	CODE_CPC_GA_INTERRUPT_LINE_COUNT,
	CODE_CPC_GA_INTERRUPT_OUTPUT,
	CODE_CPC_GA_HBLANK_COUNT,
	CODE_CPC_GA_HBLANK_ACTIVE,
	CODE_CPC_GA_VBLANK_COUNT,
	CODE_CPC_GA_VBLANK_ACTIVE,
	CODE_FDI_DRIVE,
	CODE_FDI_SIDE,
	CODE_FDI_MOTOR,
	CODE_FDC_MSR,
	CODE_FDC_PCN,
	CODE_FDC_DRIVE_OUTPUT,
	CODE_FDC_SIDE_OUTPUT,
	CODE_FDC_INTERRUPT_OUTPUT,
	CODE_FDC_DMA_MODE,
	CODE_FDC_HEAD_LOAD_TIME,
	CODE_FDC_HEAD_UNLOAD_TIME,
	CODE_FDC_STEP_RATE_TIME,
	CODE_FDC_COMMAND_BYTE,
	CODE_VRAM_ADDR,
	CODE_CRTC_REGISTER_DATA,
	CODE_CRTC_SELECTED_REGISTER,
	CODE_CRTC_ACTUAL_HORIZONTAL_SYNC_WIDTH,
	CODE_CRTC_ACTUAL_VERTICAL_SYNC_WIDTH,
	CODE_CRTC_STATUS_REGISTER,
	CODE_CRTC_INTERLACE_FRAME,
	CODE_CRTC_HCC,
	CODE_CRTC_VCC,
	CODE_CRTC_RA,
	CODE_CRTC_MA_OUTPUT,
	CODE_CRTC_RA_OUTPUT,
	CODE_CRTC_VSYNC_OUTPUT,
	CODE_CRTC_HSYNC_OUTPUT,
	CODE_CRTC_DISPTMG_OUTPUT,
	CODE_CRTC_CUDISP_OUTPUT,
	CODE_CRTC_FRAME_WIDTH,
	CODE_CRTC_FRAME_HEIGHT,
	CODE_CRTC_CHAR_HEIGHT,
	CODE_CRTC_LINES_AFTER_FRAME_START,
	CODE_CRTC_LINES_AFTER_VSYNC_START,
	CODE_CRTC_CHARS_AFTER_HSYNC_START,
	CODE_CRTC3_STATUS_REGISTER1,
	CODE_CRTC3_STATUS_REGISTER2,
	CODE_CRTC4_STATUS_REGISTER1,
	CODE_CRTC4_STATUS_REGISTER2,
	CODE_PSG_REGISTER_DATA,
	CODE_PSG_SELECTED_REGISTER,
	CODE_PSG_BDIR,
	CODE_PSG_BC1,
	CODE_PSG_BC2,
	CODE_PSG_IO_INPUT,
	CODE_PSG_IO_OUTPUT,
	CODE_PSG_MODE,
	CODE_PRINTER_BUSY,
	CODE_PRINTER_STROBE,
	CODE_PRINTER_DATA,
	CODE_PPI_CONTROL,
	CODE_PPI_PORT_INPUT,
	CODE_PPI_PORT_RESOLVED_INPUT,
	CODE_PPI_PORT_OUTPUT,
	CODE_PPI_PORT_RESOLVED_OUTPUT,
	CODE_FDD_TRACK,
	CODE_FDD_FLAGS,
	CODE_MONITOR_VSYNC_STATE,
	CODE_MONITOR_HSYNC_STATE,
	CODE_MONITOR_HORIZONTAL_POSITION,
	CODE_MONITOR_VERTICAL_POSITION,
	CODE_MONITOR_SEEN_HSYNC,
	CODE_MONITOR_SEEN_VSYNC,
	CODE_MONITOR_COUNT_AFTER_VSYNC,
	CODE_MONITOR_COUNT_AFTER_HSYNC,
	CODE_MONITOR_HORZ_ADJUSTMENT,
	CODE_MONITOR_VERT_ADJUSTMENT,
	CODE_KEYBOARD_ROW,
	CODE_KEYBOARD_SELECTED_ROW,
	CODE_CYCLE_COUNTER

};



class ItemData
{
public:
	enum
	{
		ITEM_TYPE_UNKNOWN = 0,
		ITEM_TYPE_8BIT,
		ITEM_TYPE_16BIT,
		ITEM_TYPE_BOOL,
		ITEM_TYPE_STRING_INDEX
	};
	enum
	{
		ITEM_REPRESENTATION_UNKNOWN = 0,
		ITEM_REPRESENTATION_BOOL,
		ITEM_REPRESENTATION_BINARY,
		ITEM_REPRESENTATION_HEX,
		ITEM_REPRESENTATION_DECIMAL,
		ITEM_REPRESENTATION_OCTAL,
		ITEM_REPRESENTATION_STRING
	};
	enum
	{
		ITEM_SIGNED,
		ITEM_UNSIGNED
	};
protected:
	int m_nType;
	int m_nRepresentation;
	int m_nValueCode;
	int m_nMask;
	int m_nComparison;
	int m_nIndex;
	int m_nValue;   // prev value
	int m_nShift;
	bool m_bNot;
	bool m_bDirty;  // true if dirty, false otherwise
	bool m_bChanged; // true if changed, false otherwise, for ui highlight/display
	const wxChar **m_sStringTable;
	int m_nStringTableLength;
public:
	ItemData &operator=(const ItemData &src)
	{
		this->m_nType = src.m_nType;
		this->m_nRepresentation = src.m_nRepresentation;
		this->m_nValueCode = src.m_nValueCode;
		this->m_nMask = src.m_nMask;
		this->m_nComparison = src.m_nComparison;
		this->m_nIndex = src.m_nIndex;
		this->m_nValue = src.m_nValue;   // prev value
		this->m_bDirty = src.m_bDirty;  // true if dirty, false otherwise
		this->m_bChanged = src.m_bChanged; //
		this->m_bNot = src.m_bNot;
		this->m_nShift = src.m_nShift;
		this->m_sStringTable = src.m_sStringTable;
		this->m_nStringTableLength = src.m_nStringTableLength;
		return (*this);
	}
	ItemData() : m_nType(ITEM_TYPE_UNKNOWN), m_nRepresentation(ITEM_REPRESENTATION_UNKNOWN), m_nValueCode(0), m_nMask(-1), m_nComparison(0), m_nIndex(0), m_nValue(0), m_nShift(0), m_bNot(false), m_bDirty(true), m_bChanged(false), m_sStringTable(NULL), m_nStringTableLength(0)
	{

	}

	bool RefreshNeeded(wxString &str);
	bool GetItemChanged() const
	{
		return m_bChanged;
	}
	void SetValueChanged(bool bState = true)
	{
		m_bChanged = bState;
		SetDirty();
	}

	void SetStringTable(const wxChar **pStringTable, int nStringTableLength)
	{
		m_sStringTable = pStringTable;
		m_nStringTableLength = nStringTableLength;
	}
	void SetDirty()
	{
		m_bDirty = true;
	}

	void Set8BitUnsigned(int nCode)
	{
		m_nType = ITEM_TYPE_8BIT;
		m_nValueCode = nCode;
		m_nRepresentation = ITEM_REPRESENTATION_HEX;
	}



	void SetStringID(int nCode)
	{
		m_nType = ITEM_TYPE_STRING_INDEX;
		m_nValueCode = nCode;
		m_nRepresentation = ITEM_REPRESENTATION_STRING;
	}
	void Set16BitUnsigned(int nCode)
	{
		m_nType = ITEM_TYPE_16BIT;
		m_nValueCode = nCode;
		m_nRepresentation = ITEM_REPRESENTATION_HEX;
	}

	void SetBool(int nCode)
	{
		m_nType = ITEM_TYPE_BOOL;
		m_nValueCode = nCode;
		m_nRepresentation = ITEM_REPRESENTATION_BOOL;
	};

	void SetBool(int nCode, bool bSet)
	{
		m_nType = ITEM_TYPE_BOOL;
		m_nValueCode = nCode;
		m_nMask = -1;
		m_nComparison = 0;
		m_bNot = bSet;
		m_nRepresentation = ITEM_REPRESENTATION_BOOL;
	};

	int GetValueCode() const { return m_nValueCode; }
	int GetType() const { return m_nType; }
	int GetIndex() const { return m_nIndex; }

	void SetIndex(int nIndex) { m_nIndex = nIndex; SetDirty(); }
	void SetMask(int nMask) { m_nMask = nMask; SetDirty(); }
	void SetShift(int nShift) { m_nShift = nShift; SetDirty(); }

	int GetMask() const { return m_nMask; }
	int GetComparison() const { return m_nComparison; }
	void SetComparison(int nComparison) { m_nComparison = nComparison; SetDirty(); }
	void SetRepresentation(int nRepresentation) { m_nRepresentation = nRepresentation; SetDirty(); }
	int GetRepresentation() const { return m_nRepresentation; }

	static void GenerateNumber(int nBits, bool bSigned, int nNumber, int nRepresentation, wxString &sValue);
	static void Generate16BitNumber(int nNumber, int nRepresentation, wxString &sValue);
	static void Generate8BitNumber(int nNumber, int nRepresentation, wxString &sValue);
	static void GenerateBool(bool bNumber, wxString &sValue);
	int GetValue(int nCode, int nIndex);
	bool GetNot() const { return m_bNot; }
	int GetShift() const { return m_nShift; }
};

#endif
