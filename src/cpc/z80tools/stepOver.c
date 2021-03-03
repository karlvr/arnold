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
#include "../cpcglob.h"
#include "../cpc.h"
#include "../debugger/gdebug.h"

#if 0
Z80_WORD StepOutInstruction()
{
	/* Run to instruction after DJNZ? */


	return 0;
}
#endif

Z80_WORD GetJRDestination(MemoryRange *pRange)
{
    /* calc addr and go to it */
    Z80_BYTE_OFFSET Offset;

    Offset = (Z80_BYTE_OFFSET)MemoryRange_ReadByte(pRange, CPU_GetReg(CPU_PC)+1);

    return (Z80_WORD)(CPU_GetReg(CPU_PC) + (Z80_LONG)2 + Offset);
}

Z80_WORD GetRetDestination(MemoryRange *pRange)
{
    return MemoryRange_ReadWord(pRange, CPU_GetReg(CPU_SP));
}

Z80_WORD GetCallDestination(MemoryRange *pRange)
{
    return MemoryRange_ReadWord(pRange, CPU_GetReg(CPU_PC)+1);
}

int IsSeperatorOpcode(MemoryRange *pRange, int Address)
{
  int Instruction = MemoryRange_ReadByte(pRange, Address);

	switch (Instruction)
	{
		/* JR  */
		case 0x018:
		/* RET */
		case 0x0c9:
		/* JP (HL) */
		case 0x0e9:
      return 1;

		case 0x0fd:
		case 0x0dd:
		{
			int Instruction2 = MemoryRange_ReadByte(pRange, Address+1);
			switch (Instruction2)
			{
				case 0x0e9:
				{
          return 1;
				}
				break;

				default:
					break;
			}
		}
		break;

		case 0x0ed:
		{
			int Instruction2 = MemoryRange_ReadByte(pRange,Address+1);
			switch (Instruction2)
			{
                /* RETN, RETI*/
				case 0x045:
				case 0x04d:
				case 0x055:
				case 0x05d:
				case 0x065:
				case 0x06d:
				case 0x075:
				case 0x07d:
				{
          return 1;
				}
				break;

				default:
					break;
			}


		}
		break;

		/* JP nnnn */
		case 0x0c3:
    {
      return 1;
    }

     default:
            break;

	}

  return 0;
  
  
}

void StepIntoInstruction(MemoryRange *pRange)
{
	Debugger_StepInstruction();
}

void StepOverInstruction(MemoryRange *pRange)
{
	BOOL bHandled = FALSE;

	/* get current instruction */
	int Instruction = MemoryRange_ReadByte(pRange, CPU_GetReg(CPU_PC));

	switch (Instruction)
	{

	case 0x0ed:
	{
		int Instruction2 = MemoryRange_ReadByte(pRange, CPU_GetReg(CPU_PC) + 1);
		switch (Instruction2)
		{


		/* ldir */
		/* cpir */
		/* inir */
		/* otir */
		/* lddr */
		/* cpdr */
		/* indr */
		/* otdr */
		case 0x0b0:
		case 0x0b1:
		case 0x0b2:
		case 0x0b3:
		case 0x0b8:
		case 0x0b9:
		case 0x0ba:
		case 0x0bb:
		{
			int nBytes = Debug_GetOpcodeCount(pRange, CPU_GetReg(CPU_PC));
			Debug_SetRunTo(CPU_GetReg(CPU_PC) + nBytes);
			bHandled = TRUE;
		}
		break;

		default:
		{
		}
		break;
		}

	}
	break;

	case 0x010:	/* djnz */
	case 0x0cc: /* call z */
	case 0x0cd: /* call */
	case 0x0c4: /* call nz */
	case 0x0d4: /* call nc,*/
	case 0x0dc: /* call c,*/
	case 0x0e4: /* call po,*/
	case 0x0ec: /* call pe,*/
	case 0x0f4: /* call p,*/
	case 0x0fc: /* call m,*/
	/* RST 0 */
	case 0x0c7:
		/* RST 8 */
	case 0x0cf:
		/* RST 10 */
	case 0x0d7:
		/* RST 18 */
	case 0x0df:
		/* RST 20 */
	case 0x0e7:
		/* RST 28 */
	case 0x0ef:
		/* RST 30 */
	case 0x0f7:
		/* RST 38 */
	case 0x0ff:

	/* JR NZ, */
	case 0x020:
	/* JR Z, */
	case 0x028:
	/* JR NC */
	case 0x030:
	/* JR C */
	case 0x038:
	/* HALT */
	case 0x076:	
	/* JP nz,nnnn */
	case 0x0c2:
	/* JP z,nnnn */
	case 0x0ca:
	/* JP nc,nnnn */
	case 0x0d2:
	/* JP c,nnnn */
	case 0x0da:
	/* JP PO,nnnn */
	case 0x0e2:
	/* JP PE,nnnn */
	case 0x0EA:
	/* JP P,nnnn */
	case 0x0f2:
	/* JP M,nnnn */
	case 0x0fA:
	{
		int nBytes = Debug_GetOpcodeCount(pRange, CPU_GetReg(CPU_PC));
		Debug_SetRunTo(CPU_GetReg(CPU_PC) + nBytes);
		bHandled = TRUE;
	}
	break;

	default:
	{
		break;
	}
	}
	
	if (!bHandled)
	{
		Debugger_StepInstruction();
	}
}
/* get address after current opcode and run to that address? */
