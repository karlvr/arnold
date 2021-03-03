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
#ifndef INKZ80

extern "C"
{
#include "../cpc/cpcglob.h"
#include "../cpc/cpc.h"
#include "../cpc/debugger/breakpt.h"
}

extern "C"
{

#include "../cpc/z80/z80.h"
}

Z80_REGISTERS Z80;

extern "C" void 	Z80_DebugOpcodeTriggered()
{
	if (CPU_GetDebugOpcodeEnabled())
	{
		CPU_SetDebugOpcodeHit();
	}
}

extern "C" void    Z80_Reti(Z80_REGISTERS *pZ80)
{
	CPU_Reti();
}
extern "C" void Z80_AcknowledgeInterrupt(Z80_REGISTERS *pZ80)
{
	Z80_BYTE vec = CPC_AcknowledgeInterrupt();
	Z80_SetInterruptVector(pZ80, vec);
	CPU_SetIntVectorAddress(((CPU_GetReg(CPU_I) & 0x0ff) << 8) + (vec & 0x0ff));
}

extern "C" void Z80_AcknowledgeNMI(Z80_REGISTERS *pZ80)
{
	CPC_AcknowledgeNMI();
}

extern "C" Z80_BYTE Z80_RD_DATA(const Z80_WORD addr)
{
	Z80_BYTE data = CPU_RD_MEM(addr);

	/* breakpoint? */
	if (Breakpoints_IsAHitableBreakpointData(BREAKPOINT_TYPE_MEMORY_READ, (Z80_WORD)addr, data))
	{
		/* we hit a data write breakpoint */
		CPU_SetDebugStop();
	}
	CPC_ExecuteReadMemoryFunctions(addr, &data);
	return data;
}

extern "C" void Z80_WR_DATA(const Z80_WORD addr, const Z80_BYTE val)
{
	CPU_WR_MEM(addr, val);

	/* breakpoint? */
	if (Breakpoints_IsAHitableBreakpointData(BREAKPOINT_TYPE_MEMORY_WRITE, (Z80_WORD)addr, val))
	{
		/* we hit a data write breakpoint */
		CPU_SetDebugStop();
	}
	CPC_ExecuteWriteMemoryFunctions(addr, val);
}

extern "C" Z80_BYTE Z80_DoIn(Z80_WORD port)
{

	if (CPC_GetHardware() == CPC_HW_CPCPLUS)
	{
		/* on plus, byte on the bus will be last byte of the instruction used to do the read */
		/* tested and confirmed on 464 Plus, and this result is stable */
		/* doesn't happen on CPC */
		CPU_SetDataBus(CPU_RD_MEM(CPU_GetPC() - 1));
	}

	CPU_SetIOPort(port);
	Z80_BYTE data = CPU_DoIn((const Z80_WORD)port);
	CPU_SetIOData(data);



	/* breakpoint? */
	if (Breakpoints_IsAHitableBreakpointData(BREAKPOINT_TYPE_IO_READ, (Z80_WORD)port, data))
	{
		/* we hit a data write breakpoint */
		CPU_SetDebugStop();
	}
	return data;
}


extern "C" void Z80_DoOut(Z80_WORD port, Z80_BYTE val)
{
	CPU_SetIOData(val);
	CPU_SetIOPort(port);

	CPU_DoOut((const Z80_WORD)port, (const Z80_BYTE)val);

	/* breakpoint? */
	if (Breakpoints_IsAHitableBreakpointData(BREAKPOINT_TYPE_IO_WRITE, (Z80_WORD)port, val))
	{
		/* we hit a data write breakpoint */
		CPU_SetDebugStop();
	}

}



Z80_BYTE		Z80_RD_BYTE_IM0()
{
	// change depending on cpc or cpc+
	return 0x0ff;
}

Z80_WORD		Z80_RD_WORD_IM0()
{
	// change depending on cpc or cpc+
	return 0x0ffff;
}

extern "C" BOOL CPU_GetINTState()
{
	return Z80_GetInterruptRequest(&Z80);
}

extern "C" void CPU_Init()
{
	Z80_Init(&Z80);
}

extern "C" void CPU_Reset()
{
	Z80_RestartReset(&Z80);
}

extern "C" void CPU_Power()
{
	Z80_RestartPower(&Z80);
}

extern "C" BOOL CPU_GetInterruptRequest()
{
	return Z80_GetInterruptRequest(&Z80);
}

extern "C" BOOL CPU_GetNMIInput()
{
	return (!Z80_GetNMIInterruptInput(&Z80));
}

extern "C" void CPU_SetNMIState(BOOL bState)
{
	//comes in false which means high
	//if bState ==true want low
	// bState sets NMI low
	Z80_SetNMIInterruptInput(&Z80, bState);
}

extern "C" void CPU_SetINTState(BOOL bState)
{
	Z80_SetInterruptInput(&Z80, bState);
}

void GetRegisterShiftAndMask(int nReg, int &nShift, int &nMask)
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
	return Z80_GetReg(&Z80, Z80_PC);
}

extern "C" int CPU_GetOutputs()
{
	return Z80_GetOutputs(&Z80);
}
extern "C" int CPU_GetSP()
{
	return Z80_GetReg(&Z80, Z80_SP);
}

extern "C" BOOL CPU_GetFlag(int nCPUFlag)
{
	int nFlag = 0;

	switch (nCPUFlag)
	{
	case CPU_FLAG_SIGN:
		nFlag = Z80_SIGN_FLAG;
		break;

	case CPU_FLAG_ZERO:
		nFlag = Z80_ZERO_FLAG;
		break;

	case CPU_FLAG_BIT5:
		nFlag = Z80_UNUSED_FLAG2;
		break;
	case CPU_FLAG_HALF_CARRY:
		nFlag = Z80_HALFCARRY_FLAG;
		break;
	case CPU_FLAG_BIT3:
		nFlag = Z80_UNUSED_FLAG1;
		break;
	case CPU_FLAG_PARITYOVERFLOW:
		nFlag = Z80_PARITY_FLAG;
		break;
	case CPU_FLAG_ADDSUBTRACT:
		nFlag = Z80_SUBTRACT_FLAG;
		break;
	case CPU_FLAG_CARRY:
		nFlag = Z80_CARRY_FLAG;
		break;

	}
	return ((Z80_GetReg(&Z80, Z80_F)&nFlag) != 0);

}

extern "C" int CPU_GetReg(int nReg)
{
	int nData = 0x00;
	int nShift, nMask;
	GetRegisterShiftAndMask(nReg, nShift, nMask);
	switch (nReg)
	{
	case CPU_A:
		nData = Z80_GetReg(&Z80, Z80_AF);
		break;
	case CPU_B:
		nData = Z80_GetReg(&Z80, Z80_BC);
		break;
	case CPU_C:
		nData = Z80_GetReg(&Z80, Z80_BC);
		break;
	case CPU_D:
		nData = Z80_GetReg(&Z80, Z80_DE);
		break;
	case CPU_E:
		nData = Z80_GetReg(&Z80, Z80_DE);
		break;
	case CPU_H:
		nData = Z80_GetReg(&Z80, Z80_HL);
		break;
	case CPU_L:
		nData = Z80_GetReg(&Z80, Z80_HL);
		break;
	case CPU_HL:
		nData = Z80_GetReg(&Z80, Z80_HL);
		break;
	case CPU_DE:
		nData = Z80_GetReg(&Z80, Z80_DE);
		break;
	case CPU_BC:
		nData = Z80_GetReg(&Z80, Z80_BC);
		break;
	case CPU_AF:
		nData = Z80_GetReg(&Z80, Z80_AF);
		break;
	case CPU_IX:
		nData = Z80_GetReg(&Z80, Z80_IX);
		break;
	case CPU_IY:
		nData = Z80_GetReg(&Z80, Z80_IY);
		break;
	case CPU_I:
		nData = Z80_GetReg(&Z80, Z80_I);
		break;
	case CPU_R:
		nData = Z80_GetReg(&Z80, Z80_R);
		break;
	case CPU_IM:
		nData = Z80_GetReg(&Z80, Z80_IM);
		break;
	case CPU_MEMPTR:
		nData = Z80_GetReg(&Z80, Z80_MEMPTR);
		break;
	case CPU_HL2:
		nData = Z80_GetReg(&Z80, Z80_HL2);
		break;
	case CPU_DE2:
		nData = Z80_GetReg(&Z80, Z80_DE2);
		break;
	case CPU_BC2:
		nData = Z80_GetReg(&Z80, Z80_BC2);
		break;
	case CPU_AF2:
		nData = Z80_GetReg(&Z80, Z80_AF2);
		break;
	case CPU_PC:
		nData = Z80_GetReg(&Z80, Z80_PC);
		break;
	case CPU_SP:
		nData = Z80_GetReg(&Z80, Z80_SP);
		break;
	case CPU_IFF1:
		nData = Z80_GetReg(&Z80, Z80_IFF1);
		break;
	case CPU_IFF2:
		nData = Z80_GetReg(&Z80, Z80_IFF2);
		break;
	}
	return (nData >> nShift) & nMask;
}

extern "C" void CPU_SetReg(int nReg, int nValue)
{

	int nData = 0x00;
	int nShift, nMask;
	GetRegisterShiftAndMask(nReg, nShift, nMask);
	switch (nReg)
	{
	case CPU_A:
		nData = Z80_GetReg(&Z80, Z80_AF);
		break;
	case CPU_B:
		nData = Z80_GetReg(&Z80, Z80_BC);
		break;
	case CPU_C:
		nData = Z80_GetReg(&Z80, Z80_BC);
		break;
	case CPU_D:
		nData = Z80_GetReg(&Z80, Z80_DE);
		break;
	case CPU_E:
		nData = Z80_GetReg(&Z80, Z80_DE);
		break;
	case CPU_H:
		nData = Z80_GetReg(&Z80, Z80_HL);
		break;
	case CPU_L:
		nData = Z80_GetReg(&Z80, Z80_HL);
		break;
	case CPU_HL:
		nData = Z80_GetReg(&Z80, Z80_HL);
		break;
	case CPU_DE:
		nData = Z80_GetReg(&Z80, Z80_DE);
		break;
	case CPU_BC:
		nData = Z80_GetReg(&Z80, Z80_BC);
		break;
	case CPU_AF:
		nData = Z80_GetReg(&Z80, Z80_AF);
		break;
	case CPU_IX:
		nData = Z80_GetReg(&Z80, Z80_IX);
		break;
	case CPU_IY:
		nData = Z80_GetReg(&Z80, Z80_IY);
		break;
	case CPU_I:
		nData = Z80_GetReg(&Z80, Z80_I);
		break;
	case CPU_R:
		nData = Z80_GetReg(&Z80, Z80_R);
		break;
	case CPU_IM:
		nData = Z80_GetReg(&Z80, Z80_IM);
		break;
	case CPU_MEMPTR:
		nData = Z80_GetReg(&Z80, Z80_MEMPTR);
		break;
	case CPU_HL2:
		nData = Z80_GetReg(&Z80, Z80_HL2);
		break;
	case CPU_DE2:
		nData = Z80_GetReg(&Z80, Z80_DE2);
		break;
	case CPU_BC2:
		nData = Z80_GetReg(&Z80, Z80_BC2);
		break;
	case CPU_AF2:
		nData = Z80_GetReg(&Z80, Z80_AF2);
		break;
	case CPU_PC:
		nData = Z80_GetReg(&Z80, Z80_PC);
		break;
	case CPU_SP:
		nData = Z80_GetReg(&Z80, Z80_SP);
		break;
	case CPU_IFF1:
		nData = Z80_GetReg(&Z80, Z80_IFF1);
		break;
	case CPU_IFF2:
		nData = Z80_GetReg(&Z80, Z80_IFF2);
		break;
	}
	nData = nData & !nMask;
	nData = nData | ((nValue&nMask) << nShift);

	switch (nReg)
	{
	case CPU_A:
		Z80_SetReg(&Z80, Z80_AF, nData);
		break;
	case CPU_B:
		Z80_SetReg(&Z80, Z80_BC, nData);
		break;
	case CPU_C:
		Z80_SetReg(&Z80, Z80_BC, nData);
		break;
	case CPU_D:
		Z80_SetReg(&Z80, Z80_DE, nData);
		break;
	case CPU_E:
		Z80_SetReg(&Z80, Z80_DE, nData);
		break;
	case CPU_H:
		Z80_SetReg(&Z80, Z80_HL, nData);
		break;
	case CPU_L:
		Z80_SetReg(&Z80, Z80_HL, nData);
		break;
	case CPU_HL:
		Z80_SetReg(&Z80, Z80_HL, nData);
		break;
	case CPU_DE:
		Z80_SetReg(&Z80, Z80_DE, nData);
		break;
	case CPU_BC:
		Z80_SetReg(&Z80, Z80_BC, nData);
		break;
	case CPU_AF:
		Z80_SetReg(&Z80, Z80_AF, nData);
		break;
	case CPU_IX:
		Z80_SetReg(&Z80, Z80_IX, nData);
		break;
	case CPU_IY:
		Z80_SetReg(&Z80, Z80_IY, nData);
		break;
	case CPU_I:
		Z80_SetReg(&Z80, Z80_I, nData);
		break;
	case CPU_IM:
		Z80_SetReg(&Z80, Z80_IM, nData);
		break;
	case CPU_R:
		Z80_SetReg(&Z80, Z80_R, nData);
		break;
	case CPU_MEMPTR:
		Z80_SetReg(&Z80, Z80_MEMPTR, nData);
		break;
	case CPU_HL2:
		Z80_SetReg(&Z80, Z80_HL2, nData);
		break;
	case CPU_DE2:
		Z80_SetReg(&Z80, Z80_DE2, nData);
		break;
	case CPU_BC2:
		Z80_SetReg(&Z80, Z80_BC2, nData);
		break;
	case CPU_AF2:
		Z80_SetReg(&Z80, Z80_AF2, nData);
		break;
	case CPU_PC:
		Z80_SetReg(&Z80, Z80_PC, nData);
		break;
	case CPU_SP:
		Z80_SetReg(&Z80, Z80_SP, nData);
		break;
	case CPU_IFF1:
		Z80_SetReg(&Z80, Z80_IFF1, nData);
		break;
	case CPU_IFF2:
		Z80_SetReg(&Z80, Z80_IFF2, nData);
		break;
	}
}
extern "C" int CPU_ExecuteCycles()
{
	return Z80_Execute(&Z80);
}


#endif // INKZ80
