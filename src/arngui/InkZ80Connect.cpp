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
#ifdef INKZ80

extern "C"
{
#include "../cpc/cpcglob.h"
#include "../cpc/cpc.h"
#include "../cpc/debugger/breakpt.h"

}
#include <wx/msgout.h>


#include "../inkz80/inkz80.h"

class ArnoldZ80 : public Z80
{
public:
	ArnoldZ80() : Z80()
	{
		m_nFraction = 0;
	}
	int m_nFraction;

private:
	virtual void	OnExitReti()
	{
		CPU_Reti();
	}


	/* wait is applied 3/4 in this pattern relative to Z80's T1.

	 /WAIT, -, /WAIT, /WAIT

	 Opcode fetch samples /WAIT in T2, so is not lengthened.
	 Read samples /WAIT in T3

	 // ED,49 (out (c),c -> 4):
	 // op: T1,T2,T3,-
	 // op: T1,T2,T3,-
	 // i/o: T1,T2,TW,TW*
	 // i/o cont: TW*,T3,-,-?
	 */
	// Pure virtual functions for the client to supply
	virtual BYTE	memoryReadRaw(WORD addr)
	{
		/* align to divisible by 4 */
		int burn = (4-((getTStateCount())&0x03))&0x03;
		burnTS(burn);
		BYTE data = (BYTE)CPU_RD_MEM((Z80_WORD)addr);

		/* breakpoint? */
		if (Breakpoints_IsAHitableBreakpointData(BREAKPOINT_TYPE_MEMORY_READ,(Z80_WORD)addr, data))
		{
			/* we hit a data write breakpoint */
			CPU_SetDebugStop();
		}

		CPC_ExecuteReadMemoryFunctions(addr, &data);
		return data;
	}

	virtual void	memoryWriteRaw(WORD addr, BYTE val)
	{
		/* align to divisible by 4 */
		int burn = (4-((getTStateCount())&0x03))&0x03;
		burnTS(burn);

		CPU_WR_MEM((Z80_WORD)addr, (Z80_BYTE)val);

		// check memory write breakpoints
		// need to see if enabled and need to speed these up

		/* breakpoint? */
		if (Breakpoints_IsAHitableBreakpointData(BREAKPOINT_TYPE_MEMORY_WRITE,(Z80_WORD)addr, val))
		{
			/* we hit a data write breakpoint */
			CPU_SetDebugStop();
		}
		CPC_ExecuteWriteMemoryFunctions(addr, val);

	}

	virtual BYTE	portRead(WORD port)
	{
		// burn 2 states
		// 1 force wait
		burnTS(3);
		// align up
		int burn = (4-(getTStateCount()&0x03))&0x03;
		burnTS(burn);

		burnTS(1);


		if (CPC_GetHardware()==CPC_HW_CPCPLUS)
		{
			/* on plus, byte on the bus will be last byte of the instruction used to do the read */
			/* tested and confirmed on 464 Plus, and this result is stable */
			/* doesn't happen on CPC */
			CPU_SetDataBus(CPU_RD_MEM(CPU_GetPC()-1));
		}

		CPU_SetIOPort(port);
		BYTE data = CPU_DoIn((const Z80_WORD)port);
		CPU_SetIOData(data);

		/* breakpoint? */
		if (Breakpoints_IsAHitableBreakpointData(BREAKPOINT_TYPE_IO_READ,(Z80_WORD)port, data))
		{
			/* we hit a data write breakpoint */
			CPU_SetDebugStop();
		}

		return data;
	}

	virtual void	portWrite(WORD port, BYTE val)
	{
		// burn 2 states
		// 1 force wait
		burnTS(3);
		// align up
		int burn = (4-(getTStateCount()&0x03))&0x03;
		burnTS(burn);
		/* 3 M1 cycles. Second is an inserted wait */
		/* burn up to end */
		burnTS(8);

		burnTS(1);
		CPU_SetIOData(val);
		CPU_SetIOPort(port);

		CPU_DoOut((const Z80_WORD)port, (const Z80_BYTE)val);

		/* breakpoint? */
		if (Breakpoints_IsAHitableBreakpointData(BREAKPOINT_TYPE_IO_WRITE,(Z80_WORD)port, val))
		{
			/* we hit a data write breakpoint */
			CPU_SetDebugStop();
		}

	}

	virtual void acknowledgeINT(BYTE &busValue)
	{
		busValue = CPC_AcknowledgeInterrupt();

		CPU_SetIntVectorAddress(((CPU_GetReg(CPU_I)&0x0ff)<<8)+(busValue&0x0ff));
	}

	virtual void	acknowledgeNMI()
	{
		CPC_AcknowledgeNMI();
	}

	virtual void	acknowledgeEDNOP(WORD pc, BYTE opcode)
	{
		// ed, ff debug opcode handling code
		if (opcode==0x0ff)
		{
			if (CPU_GetDebugOpcodeEnabled())
			{
				CPU_SetDebugOpcodeHit();
			}
		}



	}
public:

	void    GetRegisterShiftAndMask(int nReg, int &nShift, int &nMask);

};

static class ArnoldZ80 theZ80;


extern "C" BOOL CPU_GetINTState()
{
	return theZ80.getINTLine()==Z80::LineLow;
}

extern "C" int CPU_GetOutputs()
{
	int Output = 0;

	if (theZ80.getM1Line()==Z80::LineLow)
	{
		Output |= CPU_OUTPUT_M1;
	}
	if (theZ80.getMREQLine() == Z80::LineLow)
	{
		Output |= CPU_OUTPUT_MREQ;
	}
	if (theZ80.getHALTLine() == Z80::LineLow)
	{
		Output |= CPU_OUTPUT_HALT;
	}
	if (theZ80.getIORQLine() == Z80::LineLow)
	{
		Output |= CPU_OUTPUT_IORQ;
	}
	if (theZ80.getRDLine() == Z80::LineLow)
	{
		Output |= CPU_OUTPUT_RD;
	}
	if (theZ80.getWRLine() == Z80::LineLow)
	{
		Output |= CPU_OUTPUT_WR;
	}
	return Output;
}
#if 0
#define CPU_OUTPUT_RFSH (1<<6) /* RFSH and MREQ = IR registers placed on bus  */
#endif


extern "C" void CPU_Init()
{
	theZ80.powerOn();
}

extern "C" void CPU_Reset()
{
	theZ80.m_nFraction = 0;
	theZ80.reset();
}

extern "C" void CPU_Power()
{
	theZ80.powerOn();
}

extern "C" BOOL CPU_GetInterruptRequest()
{
	return (theZ80.getINTLine()==Z80::LineLow);
}

extern "C" BOOL CPU_GetNMIInput()
{
	return (theZ80.getNMILine()==Z80::LineLow);
}

extern "C" void CPU_SetNMIState(BOOL bState)
{
	if (bState)
	{
		theZ80.setNMILine(Z80::LineLow);
	}
	else
	{
		theZ80.setNMILine(Z80::LineHigh);
	}
}

extern "C" void CPU_SetINTState(BOOL bState)
{
	if (bState)
	{

		// trigger int
		theZ80.setINTLine(Z80::LineLow);
	}
	else
	{

		// no int
		theZ80.setINTLine(Z80::LineHigh);
	}
}

void ArnoldZ80::GetRegisterShiftAndMask(int nReg, int &nShift, int &nMask)
{
	nShift = 0;
	nMask = 0x0ffff;
	switch (nReg)
	{
	case CPU_A:
	{
		nShift = 8;
		nMask = 0x0ff;
	}
	return;
	case CPU_B:
	{
		nShift = 8;
		nMask = 0x0ff;
	}
	return;
	case CPU_C:
	{
		nShift = 0;
		nMask = 0x0ff;
	}
	return;
	case CPU_D:
	{
		nShift = 8;
		nMask = 0x0ff;
	}
	return;
	case CPU_E:
	{
		nShift = 0;
		nMask = 0x0ff;
	}
	return;
	case CPU_H:
	{
		nShift = 8;
		nMask = 0x0ff;
	}
	return;
	case CPU_L:
	{
		nShift = 0;
		nMask = 0x0ff;
	}
	return;
	default:
		break;
	}
}

extern "C" int CPU_GetPC()
{
	return theZ80.getRegisterPC();
}

extern "C" int CPU_GetSP()
{
	return theZ80.getRegisterSP();
}

extern "C" BOOL CPU_GetFlag(int nCPUFlag)
{
	Z80::Flag nFlag = (Z80::Flag)-1;

	switch (nCPUFlag)
	{
	case CPU_FLAG_SIGN:
		nFlag = Z80::Flag_Sign;
		break;

	case CPU_FLAG_ZERO:
		nFlag = Z80::Flag_Zero;
		break;

	case CPU_FLAG_BIT5:
		nFlag = Z80::Flag_Bit5;
		break;
	case CPU_FLAG_HALF_CARRY:
		nFlag = Z80::Flag_HalfCarry;
		break;
	case CPU_FLAG_BIT3:
		nFlag = Z80::Flag_Bit3;
		break;
	case CPU_FLAG_PARITYOVERFLOW:
		nFlag = Z80::Flag_ParityOverflow;
		break;
	case CPU_FLAG_ADDSUBTRACT:
		nFlag = Z80::Flag_AddSubtract;
		break;
	case CPU_FLAG_CARRY:
		nFlag = Z80::Flag_Carry;
		break;

	}
	return theZ80.isFlagSet(nFlag);

}

extern "C" int CPU_GetReg(int nReg)
{
	int nData = 0x00;
	int nShift, nMask;
	theZ80.GetRegisterShiftAndMask(nReg, nShift, nMask);
	switch (nReg)
	{
	case CPU_A:
		nData = theZ80.getRegisterAF();
		break;
	case CPU_B:
		nData = theZ80.getRegisterBC();
		break;
	case CPU_C:
		nData = theZ80.getRegisterBC();
		break;
	case CPU_D:
		nData = theZ80.getRegisterDE();
		break;
	case CPU_E:
		nData = theZ80.getRegisterDE();
		break;
	case CPU_H:
		nData = theZ80.getRegisterHL();
		break;
	case CPU_L:
		nData = theZ80.getRegisterHL();
		break;
	case CPU_HL:
		nData = theZ80.getRegisterHL();
		break;
	case CPU_DE:
		nData = theZ80.getRegisterDE();
		break;
	case CPU_BC:
		nData = theZ80.getRegisterBC();
		break;
	case CPU_AF:
		nData = theZ80.getRegisterAF();
		break;
	case CPU_IX:
		nData = theZ80.getRegisterIX();
		break;
	case CPU_IY:
		nData = theZ80.getRegisterIY();
		break;
	case CPU_I:
		nData = theZ80.getRegisterI();
		break;
	case CPU_R:
		nData = theZ80.getRegisterR();
		break;
	case CPU_IM:
	{
		Z80::InterruptMode mode = theZ80.getInterruptMode();
		switch(mode)
		{
			case Z80::InterruptMode::IM0:
			{
				nData = 0;
			}
			break;
			default:
			case Z80::InterruptMode::IM1:
			{
				nData = 1;
			}
			break;
			case Z80::InterruptMode::IM2:
			{
				nData = 2;
			}
			break;
		}
	}
		break;
	case CPU_MEMPTR:
		nData = theZ80.getRegisterWZ();
		break;
	case CPU_HL2:
		nData = theZ80.getRegisterHLAlt();
		break;
	case CPU_DE2:
		nData = theZ80.getRegisterDEAlt();
		break;
	case CPU_BC2:
		nData = theZ80.getRegisterBCAlt();
		break;
	case CPU_AF2:
		nData = theZ80.getRegisterAFAlt();
		break;
	case CPU_PC:
		nData = theZ80.getRegisterPC();
		break;
	case CPU_SP:
		nData = theZ80.getRegisterSP();
		break;
	case CPU_IFF1:
		nData = (theZ80.getIFF1()==theZ80.FlipFlopSet) ? 0x01 : 0x00;
		break;
	case CPU_IFF2:
		nData = (theZ80.getIFF2()==theZ80.FlipFlopSet) ? 0x01 : 0x00;;
		break;
	}
	return (nData>>nShift) & nMask;
}

extern "C" void CPU_SetReg(int nReg, int nValue)
{

	int nData = 0x00;
	int nShift, nMask;
	theZ80.GetRegisterShiftAndMask(nReg, nShift, nMask);
	switch (nReg)
	{
	case CPU_A:
		nData = theZ80.getRegisterAF();
		break;
	case CPU_B:
		nData = theZ80.getRegisterBC();
		break;
	case CPU_C:
		nData = theZ80.getRegisterBC();
		break;
	case CPU_D:
		nData = theZ80.getRegisterDE();
		break;
	case CPU_E:
		nData = theZ80.getRegisterDE();
		break;
	case CPU_H:
		nData = theZ80.getRegisterHL();
		break;
	case CPU_L:
		nData = theZ80.getRegisterHL();
		break;
	case CPU_HL:
		nData = theZ80.getRegisterHL();
		break;
	case CPU_DE:
		nData = theZ80.getRegisterDE();
		break;
	case CPU_BC:
		nData = theZ80.getRegisterBC();
		break;
	case CPU_AF:
		nData = theZ80.getRegisterAF();
		break;
	case CPU_IX:
		nData = theZ80.getRegisterIX();
		break;
	case CPU_IY:
		nData = theZ80.getRegisterIY();
		break;
	case CPU_I:
		nData = theZ80.getRegisterI();
		break;
	case CPU_R:
		nData = theZ80.getRegisterR();
		break;
	case CPU_IM:
		nData = theZ80.getInterruptMode();
		break;
	case CPU_MEMPTR:
		nData = theZ80.getRegisterWZ();
		break;
	case CPU_HL2:
		nData = theZ80.getRegisterHLAlt();
		break;
	case CPU_DE2:
		nData = theZ80.getRegisterDEAlt();
		break;
	case CPU_BC2:
		nData = theZ80.getRegisterBCAlt();
		break;
	case CPU_AF2:
		nData = theZ80.getRegisterAFAlt();
		break;
	case CPU_PC:
		nData = theZ80.getRegisterPC();
		break;
	case CPU_SP:
		nData = theZ80.getRegisterSP();
		break;
	case CPU_IFF1:
		nData = (theZ80.getIFF1()==theZ80.FlipFlopSet) ? 0x01 : 0x00;
		break;
	case CPU_IFF2:
		nData = (theZ80.getIFF2()==theZ80.FlipFlopSet) ? 0x01 : 0x00;
		break;
	}
	nData = nData & !nMask;
	nData = nData | ((nValue&nMask)<<nShift);

	switch (nReg)
	{
	case CPU_A:
		theZ80.setRegisterAF(nData);
		break;
	case CPU_B:
		theZ80.setRegisterBC(nData);
		break;
	case CPU_C:
		theZ80.setRegisterBC(nData);
		break;
	case CPU_D:
		theZ80.setRegisterDE(nData);
		break;
	case CPU_E:
		theZ80.setRegisterDE(nData);
		break;
	case CPU_H:
		theZ80.setRegisterHL(nData);
		break;
	case CPU_L:
		theZ80.setRegisterHL(nData);
		break;
	case CPU_HL:
		theZ80.setRegisterHL(nData);
		break;
	case CPU_DE:
		theZ80.setRegisterDE(nData);
		break;
	case CPU_BC:
		theZ80.setRegisterBC(nData);
		break;
	case CPU_AF:
		theZ80.setRegisterAF(nData);
		break;
	case CPU_IX:
		theZ80.setRegisterIX(nData);
		break;
	case CPU_IY:
		theZ80.setRegisterIY(nData);
		break;
	case CPU_I:
		theZ80.setRegisterI(nData);
		break;
	case CPU_IM:
	{
		Z80::InterruptMode mode;
		switch (nData)
		{
			case 0:
			{
				mode = Z80::InterruptMode::IM0;
			}
			break;
			default:
			case 1:
			{
				mode = Z80::InterruptMode::IM1;
			}
			break;
			case 2:
			{
				mode = Z80::InterruptMode::IM2;
			}
			break;
		}
					
		theZ80.setInterruptMode(mode);
	}
		break;
	case CPU_R:
		theZ80.setRegisterR(nData);
		break;
	case CPU_MEMPTR:
		theZ80.setRegisterWZ(nData);
		break;
	case CPU_HL2:
		theZ80.setRegisterHLAlt(nData);
		break;
	case CPU_DE2:
		theZ80.setRegisterDEAlt(nData);
		break;
	case CPU_BC2:
		theZ80.setRegisterBCAlt(nData);
		break;
	case CPU_AF2:
		theZ80.setRegisterAFAlt(nData);
		break;
	case CPU_PC:
		theZ80.setRegisterPC(nData);
		break;
	case CPU_SP:
		theZ80.setRegisterSP(nData);
		break;
	case CPU_IFF1:
		theZ80.setIFF1(nData ? theZ80.FlipFlopSet : theZ80.FlipFlopReset);
		break;
	case CPU_IFF2:
		theZ80.setIFF2(nData ? theZ80.FlipFlopSet : theZ80.FlipFlopReset);
		break;
	}
}

// under arnold; we would execute a single instruction and this would return the timings in Nop cycles.
// all instructions took an exact number of nop cycles. I/O operations within the instruction were bodged.
//
// with inkz80 we attempt to simulate the nop cycles by forcing read/write and i/o to happen on the correct boundaries
// by burning T states *before* we attempt the memory read/write/io. That should making timings within the instruction accurate.
// However we don't take into account the extra cycles left over at the end. We need those to not be rounded up
// because we need the interrupts to use them if possible.
//
// ink z80 processes an entire frame in one go.
//
// the crtc is running at a fixed update rate, same with gate-array.
// essentially every x t-states we need to perform a crtc update
extern "C" int CPU_ExecuteCycles()
{
	theZ80.setTStateCount(theZ80.m_nFraction);
	int nTStates = theZ80.executeSingleInstruction();
	/* turn into NOP cycles */
	int nNOPS = nTStates >> 2;
	theZ80.m_nFraction = nTStates & 0x03;
	return nNOPS;
}

#endif // INKZ80
