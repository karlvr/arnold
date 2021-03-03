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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 */
int Z80_GetOutputs(Z80_REGISTERS *pZ80)
{
	return pZ80->Outputs;
}



int    Z80_ExecuteInterrupt(Z80_REGISTERS *pZ80)
{

	int Cycles;

	Cycles = 0;
	/* clear both iff1 and iff2 */
	pZ80->IFF1 = 0;
	pZ80->IFF2 = 0;

	/* "If a non-maskable interrupt has been received or a maskable
	interrupt has been received and the interrupt enable flip-flop is set, then the
	HALT state is exited on the next rising clock edge." */
	if (pZ80->Flags & Z80_EXECUTING_HALT_FLAG)
	{
		pZ80->Flags &= ~Z80_EXECUTING_HALT_FLAG;
		pZ80->PC.W.l++;
	}
	Cycles++;

	/* set it back again */
	pZ80->Flags &= ~Z80_INTERRUPT_NO_DELAY_FLAG;


	/* z80 undocumented says accepting a maskable or non-maskable interrupt causes R
	to be incremented by 1 */
	pZ80->R++;

	pZ80->Outputs = Z80_OUTPUT_M1 | Z80_OUTPUT_IORQ;
	Z80_AcknowledgeInterrupt(pZ80);
	pZ80->Outputs = 0;

	switch (pZ80->IM)
	{
	case 0x00:
	{
		/* TIMINGS NEED TO BE VERIFIED */
		/* R register needs to be verified */
		Cycles += Z80_ExecuteIM0(pZ80);
	}
	break;

	case 0x01:
	{
		/* Z80 doc claims 13 T states
		comprised as: 2,5,3,3
		- 2 T-states interrupt acknowledge; already accounted for above
		- 5 T-states for effectively opcode, CPC hardware sees them as 4T, 1T, the 3 T states will be unused
		because a memory access happens next
		- 2 memory accesses at 3 T states each; stretched to 4 T states by CPC hardware total 2 cycles

		So if we exclude 2T states for interrupt, we're effectively left with 4,4,4,4
		*/

		Cycles += 4;

		pZ80->MemPtr.W = 0x0038;

		Z80_PUSH_PC(pZ80);

		/* set program counter address */
		pZ80->PC.W.l = pZ80->MemPtr.W;
	}
	break;

	case 0x02:
	{
		/* Z80 doc claims 19 T states
		comprised as: 2,5,3,3,3,3
		- 2 T-states interrupt acknowledge; already accounted for above
		- 5 T-states for effectively opcode, CPC hardware sees them as 4T, 1T, the 3 T states will be unused
		because a memory access happens next
		- 4 memory accesses at 3 T states each; stretched to 4 T states by CPC hardware total 4 cycles

		So if we exclude 2T states for interrupt, we're effectively left with 4,4,4,4,4,4
		*/
		Cycles += 6;
		/* is this done internally?? */
		pZ80->MemPtr.W = (pZ80->I << 8) | (pZ80->InterruptVectorBase & 0x0ff);

		/* interrupt call to address sets memptr */
		pZ80->MemPtr.W = Z80_RD_WORD(pZ80,pZ80->MemPtr.W);

		Z80_PUSH_PC(pZ80);

		pZ80->PC.W.l = pZ80->MemPtr.W;
	}
	break;
	}
	return Cycles;
}

/*
 A value has even parity when all the binary digits added together give an even
 number. (result = 0). A value has an odd parity when all the digits added
 together give an odd number. (result = 1)
*/

static void    Z80_BuildParityTable(void)
{
        int     i,j;
        int     sum;

        for (i=0; i<256; i++)
        {
                Z80_BYTE        data;

                sum = 0;                                /* will hold sum of all bits */

                data = (Z80_BYTE)i;                               /* data byte to find sum of */

                for (j=0; j<8; j++)
                {
                        sum+=data & 0x01;       /* isolate bit and add */
                        data=data>>1;           /* shift for next bit */
                }

                /* in flags register, if result has even parity, then
                 bit is set to 1, if result has odd parity, then bit
                 is set to 0.
				*/

                /* check bit 0 of sum. If 1, then odd parity, else even parity. */
                if ((sum & 0x01)!=0)
                {
                        /* odd parity */
                        ParityTable[i] = 0;
                }
                else
                {
                        /* even parity */
                        ParityTable[i] = Z80_PARITY_FLAG;
                }

        }

        for (i=0; i<256; i++)
        {
                ZeroSignTable[i] = 0;

                if ((i & 0x0ff)==0)
                {
                        ZeroSignTable[i] |= Z80_ZERO_FLAG;
                }

                if (i & 0x080)
                {
                        ZeroSignTable[i] |= Z80_SIGN_FLAG;
                }
        }
/*        for (i=0; i<256; i++)
        {
			unsigned char Data;

            Data = 0;

                if ((i & 0x0ff)==0)
                {
                        Data |= Z80_ZERO_FLAG;
                }

                if (i & 0x080)
                {
                        Data |= Z80_SIGN_FLAG;
                }

				Data |= (i & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2));

				ZeroSignTable2[i] = Data;

        }
*/
        for (i=0; i<256; i++)
        {
                ZeroSignParityTable[i] = 0;

                if ((i & 0x0ff)==0)
                {
                        ZeroSignParityTable[i] |= Z80_ZERO_FLAG;
                }

                if ((i & 0x080)==0x080)
                {
                        ZeroSignParityTable[i] |= Z80_SIGN_FLAG;
                }

                ZeroSignParityTable[i] |= ParityTable[i];
        }
}

void    Z80_Init(Z80_REGISTERS *pZ80)
{
   Z80_BuildParityTable();
   pZ80->Flags = Z80_INTERRUPT_NMI_INPUT_STATE_FLAG;
   Z80_RestartPower(pZ80);
}


void 	Z80_RestartPower(Z80_REGISTERS *pZ80)
{
	/* this is a power on */

	/* all registers are set to 0x0ffff, excluding PC and IR */
	pZ80->AF.W=0x0ffff;
	pZ80->BC.W=0x0ffff;
	pZ80->DE.W=0x0ffff;
	pZ80->HL.W=0x0ffff;
	pZ80->IX.W=0x0ffff;
	pZ80->IY.W=0x0ffff;
	pZ80->SP.W=0x0ffff;
    pZ80->altAF.W=0x0ffff;
    pZ80->altBC.W=0x0ffff;
    pZ80->altDE.W=0x0ffff;
    pZ80->altHL.W=0x0ffff;

    /* clear PC register */
    pZ80->PC.L = 0;
    /* interrupt mode to 0 */
    pZ80->IM=0;
    /* clear flip flops */
    pZ80->IFF1=0;
    pZ80->IFF2=0;
	pZ80->Outputs = 0;
    /* clear I/R registers */
    pZ80->I = 0;
    pZ80->R = 0;
    pZ80->RBit7 = 0;
	/* nmi is not seen unless it transitions after power on */
	pZ80->Flags &= ~Z80_INTERRUPT_NMI_FLAG;
	Z80_RefreshInterruptRequest(pZ80);
}

void    Z80_RestartReset(Z80_REGISTERS *pZ80)
{
	/* this is a reset from the reset pulse on the z80 */
	/* based on mcleod_ideafix result */

	/* all other registers remain the same */
	pZ80->I = 0;
	pZ80->R = 0;
	pZ80->RBit7 = 0;

   /* clear PC register */
	pZ80->PC.L = 0;

  /* interrupt mode to 0 */
    pZ80->IM=0;
    /* clear flip flops */
    pZ80->IFF1=0;
    pZ80->IFF2=0;
	/* nmi is not seen unless it transitions after reset */
	pZ80->Flags &= ~Z80_INTERRUPT_NMI_FLAG;

	Z80_RefreshInterruptRequest(pZ80);
}


int    Z80_ExecuteNMI(Z80_REGISTERS *pZ80)
{
    /* no delay is added */
    /* delay is added to maskable interrupts to allow daisy chain of interrupts */

    pZ80->Flags &=~Z80_INTERRUPT_NO_DELAY_FLAG;
  pZ80->Flags &= ~Z80_INTERRUPT_NMI_FLAG;
  
    /* "If a non-maskable interrupt has been received or a maskable
    interrupt has been received and the interrupt enable flip-flop is set, then the
    HALT state is exited on the next rising clock edge." */
    if (pZ80->Flags & Z80_EXECUTING_HALT_FLAG)
    {
      pZ80->Flags &=~Z80_EXECUTING_HALT_FLAG;
      pZ80->PC.W.l++;
    }

    Z80_RefreshInterruptRequest(pZ80);

    /* disable maskable ints */
    pZ80->IFF1 = 0;

    Z80_PUSH_PC(pZ80);

    /* set program counter address */
    pZ80->PC.W.l = 0x0066;

    /* accepting a NMI increments R by 1 */
    pZ80->R++;

	Z80_AcknowledgeNMI(pZ80);

    /* should be about 12 T States */
    return 4;
}

Z80_WORD Z80_GetIOPort(Z80_REGISTERS *pZ80)
{
    return pZ80->IOPort;
}

Z80_BYTE Z80_GetIOData(Z80_REGISTERS *pZ80)
{
    return pZ80->IOData;
}

int		Z80_GetReg(Z80_REGISTERS *pZ80,int RegID)
{
	switch (RegID)
	{
	    case Z80_A:
            return pZ80->AF.B.h;
	    case Z80_F:
            return pZ80->AF.B.l;
		case Z80_PC:
			return pZ80->PC.W.l&0x0ffff;
		case Z80_HL:
			return pZ80->HL.W;
		case Z80_H:
			return pZ80->HL.B.h;
		case Z80_L:
			return pZ80->HL.B.l;
		case Z80_DE:
			return pZ80->DE.W;
		case Z80_D:
			return pZ80->DE.B.h;
		case Z80_E:
			return pZ80->DE.B.l;
		case Z80_BC:
			return pZ80->BC.W;
		case Z80_B:
			return pZ80->BC.B.h;
		case Z80_C:
			return pZ80->BC.B.l;
		case Z80_AF:
			return pZ80->AF.W;
		case Z80_SP:
			return pZ80->SP.W;
		case Z80_IX:
			return pZ80->IX.W;
		case Z80_IY:
			return pZ80->IY.W;
		case Z80_IM:
			return pZ80->IM;
		case Z80_IFF1:
			return pZ80->IFF1;
		case Z80_IFF2:
			return pZ80->IFF2;
		case Z80_I:
			return pZ80->I;
		case Z80_R:
			return Z80_GET_R;
		case Z80_AF2:
			return pZ80->altAF.W;
		case Z80_BC2:
			return pZ80->altBC.W;
		case Z80_DE2:
			return pZ80->altDE.W;
		case Z80_HL2:
			return pZ80->altHL.W;
        case Z80_MEMPTR:
            return pZ80->MemPtr.W;
		default:
			break;
	}

	return 0x0;
}


void		Z80_SetReg(Z80_REGISTERS *pZ80,int RegID, int Value)
{
	switch (RegID)
	{
		case Z80_PC:
			pZ80->PC.L = Value&0x0ffff;
			return;

		case Z80_HL:
			pZ80->HL.W = (Z80_WORD)Value;
			return;

		case Z80_H:
			pZ80->HL.B.h = (Z80_BYTE)Value;
			return;

		case Z80_L:
			pZ80->HL.B.l = (Z80_BYTE)Value;
			return;


		case Z80_DE:
			pZ80->DE.W = (Z80_WORD)Value;
			return;


		case Z80_D:
			pZ80->DE.B.h = (Z80_BYTE)Value;
			return;

		case Z80_E:
			pZ80->DE.B.l = (Z80_BYTE)Value;
			return;



		case Z80_BC:
			pZ80->BC.W = (Z80_WORD)Value;
			return;

		case Z80_B:
			pZ80->BC.B.h = (Z80_BYTE)Value;
			return;

		case Z80_C:
			pZ80->BC.B.l = (Z80_BYTE)Value;
			return;


		case Z80_AF:
			pZ80->AF.W = (Z80_WORD)Value;
			return;


		case Z80_A:
			pZ80->AF.B.h = (Z80_BYTE)Value;
			return;

		case Z80_F:
			pZ80->AF.B.l = (Z80_BYTE)Value;
			return;

		case Z80_SP:
			pZ80->SP.W = (Z80_WORD)Value;
			return;

		case Z80_IX:
			pZ80->IX.W = (Z80_WORD)Value;
			return;

		case Z80_IY:
			pZ80->IY.W = (Z80_WORD)Value;
			return;

		case Z80_IM:
			pZ80->IM = Value;
			return;

		case Z80_IFF1:
			pZ80->IFF1 = Value;
			return;

		case Z80_IFF2:
			pZ80->IFF2 = Value;
			return;

		case Z80_I:
			pZ80->I = (Z80_BYTE)Value;
			return;

		case Z80_R:
			pZ80->R = (Z80_BYTE)Value;
			pZ80->RBit7 = (Z80_BYTE)Value & 0x080;
			return;

		case Z80_AF2:
			pZ80->altAF.W = (Z80_WORD)Value;
			return;

		case Z80_BC2:
			pZ80->altBC.W = (Z80_WORD)Value;
			return;

		case Z80_DE2:
			pZ80->altDE.W = (Z80_WORD)Value;
			return;

		case Z80_HL2:
			pZ80->altHL.W = (Z80_WORD)Value;
			return;

		default:
			break;
	}
}


void    Z80_SetInterruptVector(Z80_REGISTERS *pZ80,int Base)
{
        pZ80->InterruptVectorBase = Base & 0x0ff;
}

int Z80_GetInterruptVector(Z80_REGISTERS *pZ80)
{
    return pZ80->InterruptVectorBase & 0x0ff;
}


void Z80_RefreshInterruptRequest(Z80_REGISTERS *pZ80)
{
    if (
        /* nmi is active */
        ((pZ80->Flags & Z80_INTERRUPT_NMI_FLAG)!=0) ||
     /* ignore interrupt after instruction following EI */
     //   (pZ80->Flags & Z80_NO_CHECK_INTERRUPT_FLAG) ||
    
       /* maskable interrupt is being requested and we are allowed to check interrupt */
        (
         ((pZ80->Flags & Z80_MASKABLE_INTERRUPT_INPUT_FLAG)!=0) &&
            /* maskable interrupts are enabled */
            (pZ80->IFF1!=0)
         )
        )
    {
        pZ80->Flags |= Z80_INTERRUPT_CHECK;
    }
    else
    {
        pZ80->Flags &=~Z80_INTERRUPT_CHECK;
    }
}

BOOL Z80_GetNMIInterruptInput(Z80_REGISTERS *pZ80)
{
    return ((pZ80->Flags & Z80_INTERRUPT_NMI_INPUT_STATE_FLAG)!=0);
}

BOOL Z80_GetNMIInterruptRequest(Z80_REGISTERS *pZ80)
{
    return ((pZ80->Flags & Z80_INTERRUPT_NMI_FLAG)!=0);
}



void    Z80_SetNMIInterruptInput(Z80_REGISTERS *pZ80,BOOL bState)
{
  // true means low, false means high
  
  // nmi is negative edge triggered
    if (bState)
    {
        /* signal was high, but now is low */
        if (pZ80->Flags & Z80_INTERRUPT_NMI_INPUT_STATE_FLAG)
        {
            pZ80->Flags |= Z80_INTERRUPT_NMI_FLAG;
        }
        pZ80->Flags &= ~Z80_INTERRUPT_NMI_INPUT_STATE_FLAG;
    }
    else
    {

        /* signal high */
        pZ80->Flags |= Z80_INTERRUPT_NMI_INPUT_STATE_FLAG;
    }

    Z80_RefreshInterruptRequest(pZ80);
}


void    Z80_SetInterruptInput(Z80_REGISTERS *pZ80,BOOL bState)
{
    if (bState)
    {
        pZ80->Flags |= Z80_MASKABLE_INTERRUPT_INPUT_FLAG;
    }
    else
    {
        pZ80->Flags &= ~Z80_MASKABLE_INTERRUPT_INPUT_FLAG;
    }

    Z80_RefreshInterruptRequest(pZ80);
}

BOOL	Z80_GetInterruptRequest(Z80_REGISTERS *pZ80)
{
	return ((pZ80->Flags & Z80_MASKABLE_INTERRUPT_INPUT_FLAG)!=0);
}

#if 0
int Z80_Execute(void)
{
/*unsigned long Opcode;*/
unsigned long Cycles;
if ((pZ80->Flags & Z80_INTERRUPT_REQUEST)!=0)
{
    if ((pZ80->Flags & Z80_INTERRUPT_NMI_FLAG)!=0)
    {
        Cycles = Z80_ExecuteNMI();
    }
    else
    {
        Cyces = Z80_ExecuteInterrupt();
    }
}
else
{
    Cycles = Z80_ExecuteInstruction();
}
return Cycles;
}
#endif


