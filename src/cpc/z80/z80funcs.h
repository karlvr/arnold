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

/* read a byte from memory */
 Z80_BYTE Z80_RD_BYTE(Z80_REGISTERS *pZ80, Z80_WORD Addr)
{
	Z80_BYTE Data;
	pZ80->Outputs = Z80_OUTPUT_MREQ | Z80_OUTPUT_RD;
	Data = Z80_RD_DATA(Addr) & 0x0ff;
	pZ80->Outputs = 0;
	return Data;
}

/* read a byte from memory */
 void Z80_WR_BYTE(Z80_REGISTERS *pZ80, Z80_WORD Addr, Z80_BYTE Data)
{
	pZ80->Outputs = Z80_OUTPUT_MREQ | Z80_OUTPUT_WR;
	Z80_WR_DATA(Addr, Data);
	pZ80->Outputs = 0;
}


/* read a word from memory */
 Z80_WORD Z80_RD_WORD(Z80_REGISTERS *pZ80, Z80_WORD Addr)
{
    Z80_WORD Data;
	pZ80->Outputs = Z80_OUTPUT_MREQ | Z80_OUTPUT_RD;
    Data = Z80_RD_BYTE(pZ80,Addr)&0x0ff;
    Addr++;
	Data |= (Z80_RD_BYTE(pZ80, Addr) & 0x0ff) << 8;
	pZ80->Outputs = 0;
	return Data;
}

/*---------------------------------------------------------*/
/* write a word to memory */
 void Z80_WR_WORD(Z80_REGISTERS *pZ80, Z80_WORD Addr, Z80_WORD Data)
{
	pZ80->Outputs = Z80_OUTPUT_MREQ | Z80_OUTPUT_WR;
	/* write low byte */
	Z80_WR_BYTE(pZ80, Addr, (Z80_BYTE)(Data & 0x0ff));
    Addr++;
    /* write high byte */
	Z80_WR_BYTE(pZ80, Addr, (Z80_BYTE)((Data >> 8)) & 0x0ff);
	pZ80->Outputs = 0;
}

/*---------------------------------------------------------*/
 void Z80_PUSH_WORD(Z80_REGISTERS *pZ80,Z80_WORD Data)
{
    pZ80->SP.W--;
    Z80_WR_BYTE(pZ80, pZ80->SP.W,((Data>>8)&0x0ff));
    pZ80->SP.W--;
	Z80_WR_BYTE(pZ80, pZ80->SP.W, (Data & 0x0ff));
}

/*---------------------------------------------------------*/
 Z80_WORD Z80_POP_WORD(Z80_REGISTERS *pZ80)
{
    Z80_WORD Data;
	Data = Z80_RD_BYTE(pZ80, pZ80->SP.W) & 0x0ff;
    pZ80->SP.W++;
	Data |= (Z80_RD_BYTE(pZ80, pZ80->SP.W) & 0x0ff) << 8;
    pZ80->SP.W++;
    return Data;
}

/*---------------------------------------------------------*/
/* read a opcode byte */
 Z80_BYTE	Z80_RD_OPCODE_BYTE(Z80_REGISTERS *pZ80)
{
	Z80_WORD Addr = pZ80->PC.W.l;
	pZ80->PC.W.l++;
	pZ80->Outputs = Z80_OUTPUT_M1|Z80_OUTPUT_MREQ|Z80_OUTPUT_RD;
	Z80_BYTE Data = Z80_RD_DATA(Addr);
	pZ80->Outputs = 0;
	return Data;
}


/*---------------------------------------------------------*/
 Z80_WORD	Z80_RD_PC_WORD(Z80_REGISTERS *pZ80)
{
    Z80_WORD Data;
	pZ80->Outputs = Z80_OUTPUT_MREQ | Z80_OUTPUT_RD;
	Data = Z80_RD_DATA(pZ80->PC.W.l);
	pZ80->PC.W.l++;
	Data |= (Z80_RD_DATA(pZ80->PC.W.l) << 8);
	pZ80->PC.W.l++;
	pZ80->Outputs = 0;
	return Data;
}

/*---------------------------------------------------------*/
 Z80_BYTE	Z80_RD_PC_BYTE(Z80_REGISTERS *pZ80)
{
	Z80_BYTE Data;
	pZ80->Outputs = Z80_OUTPUT_MREQ | Z80_OUTPUT_RD;
	Data = Z80_RD_DATA(pZ80->PC.W.l);
	pZ80->PC.W.l++;
	pZ80->Outputs = 0;
	return Data;
}


/*---------------------------------------------------------*/

 void SETUP_INDEXED_ADDRESS(Z80_REGISTERS *pZ80,Z80_WORD Index)
{
	Z80_BYTE_OFFSET Offset;

	/* read signed offset */
    Offset = (Z80_BYTE_OFFSET)Z80_RD_PC_BYTE(pZ80);

	pZ80->MemPtr.W = Index+Offset;
}

/*---------------------------------------------------------*/

 void Z80_PUSH_PC(Z80_REGISTERS *pZ80)
{
/* what order does a real z80 do it? */
    Z80_PUSH_WORD(pZ80, pZ80->PC.W.l);
}

/*---------------------------------------------------------*/

 void Z80_POP_PC(Z80_REGISTERS *pZ80)
{
/* what order does a real z80 do it? */
     pZ80->PC.W.l = Z80_POP_WORD(pZ80);
}


/*---------------------------------------------------------*/
/* jump relative to a memory location */
#if 0
#define JR() \
{ \
        Z80_BYTE_OFFSET Offset; \
 \
        Offset = (Z80_BYTE_OFFSET)Z80_RD_OPCODE_BYTE(pZ80,1); \
 \
        pZ80->PC.W.l = pZ80->PC.W.l + (Z80_LONG)2 + Offset; \
}
#endif


#define RETURN() \
{ \
 Z80_POP_PC(pZ80); \
    pZ80->MemPtr.W = pZ80->PC.W.l; \
}

#if 0
 void CALL_I(Z80_REGISTERS *pZ80, Z80_WORD Addr)
{
	// for now we don't store return address on stack here...
	// we do it in the appropiate routine
	//
    /* set program counter to sub-routine address */
    pZ80->MemPtr.W = Addr;
	pZ80->PC.W.l = pZ80->MemPtr.W;
}


 void CALL(void)
{
    /* store return address on stack */
    pZ80->PC.W.l+=3;
    Z80_PUSH_PC(pZ80);

   pZ80->MemPtr.W = Z80_RD_OPCODE_WORD();
	pZ80->PC.W.l = pZ80->MemPtr.W;
	CALL_I(Z80_RD_OPCODE_WORD(1));
	/* no flags changed */
}
#endif

 void CALL_IM0(Z80_REGISTERS *pZ80)
{
    /* store return address on stack */
      Z80_PUSH_PC(pZ80);

	  pZ80->MemPtr.W = Z80_RD_WORD_IM0();
	pZ80->PC.W.l = pZ80->MemPtr.W;
		/* no flags changed */

}

#define DJNZ_dd() \
{ \
    pZ80->MemPtr.W = ((signed)Z80_RD_PC_BYTE(pZ80)); \
    \
        /* decrement B */ \
        pZ80->BC.B.h--; \
 \
        /* if zero */ \
        if (pZ80->BC.B.h==0) \
        { \
 \
				return 3; \
        } \
        else \
        { \
                pZ80->PC.W.l = pZ80->PC.W.l + pZ80->MemPtr.W; \
 \
				return 4; \
		} \
		/* no flags changed */ \
}


 int DJNZ_dd_IM0(Z80_REGISTERS *pZ80)
{
        /* decrement B */
        pZ80->BC.B.h--;

        /* if zero */
        if (pZ80->BC.B.h==0)
        {

				return 3;

        }
        else
        {
                /* branch */
		
		/* FIX!!! */
              /*  JR(); */

				return 4;
		}
		/* no flags changed */
}


// Z80_BYTE  RD_BYTE_INDEX()
//{
//     return Z80_RD_BYTE(pZ80->MemPtr.W);
//}

// void WR_BYTE_INDEX(Z80_BYTE Data)
//{
//	Z80_WR_BYTE(pZ80->MemPtr.W, Data);
//}





 void ADD_A_HL(Z80_REGISTERS *pZ80)
{

        pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->HL.W);

        ADD_A_X(pZ80->TempByte);
}
#if 0
 void ADD_A_n(void)
{

        pZ80->TempByte = Z80_RD_OPCODE_BYTE(1);

        ADD_A_X(pZ80->TempByte);
}
#endif

#define ADD_A_INDEX(Index)              \
{                                                                       \
        /*Z80_BYTE      Data;*/                                 \
                                                                        \
 		SETUP_INDEXED_ADDRESS(pZ80,Index); \
       pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W);      \
                                                                        \
        ADD_A_X(pZ80->TempByte);                                    \
}

 void ADC_A_HL(Z80_REGISTERS *pZ80)
{

	pZ80->TempByte = Z80_RD_BYTE(pZ80, pZ80->HL.W);

        ADC_A_X(pZ80->TempByte);
}

#if 0
 void ADC_A_n(void)
{

        pZ80->TempByte = Z80_RD_OPCODE_BYTE(1);

        ADC_A_X(pZ80->TempByte);

}
#endif

#define ADC_A_INDEX(Index)              \
{                                                               \
        /*Z80_BYTE      Data;*/                         \
                                                                \
		SETUP_INDEXED_ADDRESS(pZ80,Index); \
        pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W);      \
                                                                                \
        ADC_A_X(pZ80->TempByte);                                    \
}


 void SUB_A_HL(Z80_REGISTERS *pZ80)
{

	pZ80->TempByte = Z80_RD_BYTE(pZ80, pZ80->HL.W);

        SUB_A_X(pZ80->TempByte);
}

#if 0
 void SUB_A_n(void)
{
/*      Z80_BYTE        Data;*/

        pZ80->TempByte = Z80_RD_OPCODE_BYTE(1);

        SUB_A_X(pZ80->TempByte);
}
#endif

#define SUB_A_INDEX(Index)              \
{                                                               \
        /*Z80_BYTE      Data;           */              \
                                                                \
		SETUP_INDEXED_ADDRESS(pZ80,Index); \
        pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W);      \
                                                                        \
        SUB_A_X(pZ80->TempByte);                                    \
}

 void SBC_A_HL(Z80_REGISTERS *pZ80)
{
        /*Z80_BYTE      Data;*/

	pZ80->TempByte = Z80_RD_BYTE(pZ80, pZ80->HL.W);

        SBC_A_X(pZ80->TempByte);
}

#if 0
 void SBC_A_n(void)
{
        /*Z80_BYTE      Data;*/

        pZ80->TempByte = Z80_RD_OPCODE_BYTE(1);

        SBC_A_X(pZ80->TempByte);

}
#endif

#define SBC_A_INDEX(Index)              \
{                                                               \
        /*Z80_BYTE      Data;*/                         \
                                                                \
		SETUP_INDEXED_ADDRESS(pZ80,Index); \
        pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W);      \
                                                                        \
        SBC_A_X(pZ80->TempByte);                                    \
}

 void CP_A_HL(Z80_REGISTERS *pZ80)
{
        /*Z80_BYTE      Data;*/

	pZ80->TempByte = Z80_RD_BYTE(pZ80, pZ80->HL.W);

        CP_A_X(pZ80->TempByte);
}

#if 0
 void CP_A_n(void)
{
        /*Z80_BYTE      Data;*/

        pZ80->TempByte = Z80_RD_OPCODE_BYTE(1);

        CP_A_X(pZ80->TempByte);

}
#endif

#define CP_A_INDEX(Index)               \
{                                                                       \
        /*int   Data;   */                              \
                                                                        \
		SETUP_INDEXED_ADDRESS(pZ80,Index); \
        pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W);      \
                                                                        \
        CP_A_X(pZ80->TempByte);                                     \
}

#if 0
 void AND_A_n(void)
{

        pZ80->TempByte = Z80_RD_OPCODE_BYTE(1);

        AND_A_X(pZ80->TempByte);
}
#endif

 void AND_A_HL(Z80_REGISTERS *pZ80)
{

	pZ80->TempByte = Z80_RD_BYTE(pZ80, pZ80->HL.W);

        AND_A_X(pZ80->TempByte);
}

#if 0
 void XOR_A_n(void)
{

        pZ80->TempByte = Z80_RD_OPCODE_BYTE(1);

        XOR_A_X(pZ80->TempByte);
}
#endif

 void XOR_A_HL(Z80_REGISTERS *pZ80)
{

	pZ80->TempByte = Z80_RD_BYTE(pZ80, pZ80->HL.W);

        XOR_A_X(pZ80->TempByte);
}

 void OR_A_HL(Z80_REGISTERS *pZ80)
{

	pZ80->TempByte = Z80_RD_BYTE(pZ80, pZ80->HL.W);

        OR_A_X(pZ80->TempByte);
}

#if 0
 void OR_A_n(void)
{

        pZ80->TempByte = Z80_RD_OPCODE_BYTE(1);

        OR_A_X(pZ80->TempByte);
}
#endif


/* LDI */
#define LDI()		\
{					\
        Z80_BYTE      Data;	\
		Z80_BYTE		Flags;	\
							\
        Data  = Z80_RD_BYTE(pZ80,pZ80->HL.W);	\
        Z80_WR_BYTE(pZ80,pZ80->DE.W,Data);		\
										\
        pZ80->HL.W++;						\
        pZ80->DE.W++;						\
        pZ80->BC.W--;						\
										\
        Flags = Z80_FLAGS_REG & (Z80_CARRY_FLAG | Z80_ZERO_FLAG | Z80_SIGN_FLAG);	/* Do not change CF, ZF, SF */ \
		/* HF, NF = 0 */ \
		/* according to Sean's z80 documentation: (Data+A) bit 1 is YF, bit 3 is XF */ \
		/* YF is bit 5 yet it takes a copy of bit 1 */ \
		Flags |= ((Data + pZ80->AF.B.h)&(1<<1))<<4; \
        /* XF is bit 3 and takes a copy of bit 3 */ \
		Flags |= (Data + pZ80->AF.B.h)&(1<<3); \
        /* if BC==0, then PV =0, else PV = 1 */	\
        if (pZ80->BC.W!=0)						\
        {									\
                Flags |= Z80_PARITY_FLAG;	\
        }								\
		Z80_FLAGS_REG = Flags;				\
}

/* LDIR */
#define LDIR() \
{ \
    Z80_BYTE      Data;	\
	Z80_BYTE		Flags;	\
						\
    Data  = Z80_RD_BYTE(pZ80,pZ80->HL.W);	\
    Z80_WR_BYTE(pZ80,pZ80->DE.W,Data);		\
									\
    pZ80->HL.W++;						\
    pZ80->DE.W++;						\
    pZ80->BC.W--;						\
									\
    Flags = Z80_FLAGS_REG & (Z80_CARRY_FLAG | Z80_ZERO_FLAG | Z80_SIGN_FLAG|Z80_PARITY_FLAG);	/* Do not change CF, ZF, SF */ \
	/* HF, NF = 0 */ \
	/* according to Sean's z80 documentation: (Data+A) bit 1 is YF, bit 3 is XF */ \
	Flags |= ((Data + pZ80->AF.B.h)&((1<<1)|(1<<3)))<<2; \
    /* if BC==0, then PV =0 */	\
    if (pZ80->BC.W==0)						\
    {									\
        Flags &= ~Z80_PARITY_FLAG;	\
		pZ80->PC.W.l+=2; \
		Cycles=6;					\
    }								\
	else							\
	{								\
		pZ80->MemPtr.W = pZ80->PC.W.l+1;	\
		Cycles=5;					\
	}								\
	Z80_FLAGS_REG = Flags;				\
    /* spare cycles at end of instruction that interrupt can consume if triggered */ \
    pZ80->Flags &= ~Z80_INTERRUPT_DELAY_FLAG; \
} \


/* LDD */
#define LDD()			\
{						\
        Z80_BYTE      Data,Flags;		\
										\
        Data  = Z80_RD_BYTE(pZ80,pZ80->HL.W);	\
        Z80_WR_BYTE(pZ80,pZ80->DE.W,Data);		\
										\
        pZ80->HL.W--;						\
        pZ80->DE.W--;						\
        pZ80->BC.W--;						\
										\
        Flags = Z80_FLAGS_REG & (Z80_CARRY_FLAG | Z80_ZERO_FLAG | Z80_SIGN_FLAG);	/* Do not change CF, ZF, SF */ \
		/* HF, NF = 0 */ \
		/* according to Sean's z80 documentation: (Data+A) bit 1 is YF, bit 3 is XF */ \
		/* YF is bit 5 yet it takes a copy of bit 1 */ \
		Flags |= ((Data + pZ80->AF.B.h)&(1<<1))<<4; \
        /* XF is bit 3 and takes a copy of bit 3 */ \
		Flags |= (Data + pZ80->AF.B.h)&(1<<3); \
        /* if BC==0, then PV =0, else PV = 1 */	\
        if (pZ80->BC.W!=0)						\
        {									\
                Flags |= Z80_PARITY_FLAG;	\
        }								\
		Z80_FLAGS_REG = Flags;				\
}


/* LDDR */
#define LDDR()			\
{						\
        Z80_BYTE      Data,Flags;		\
										\
        Data  = Z80_RD_BYTE(pZ80,pZ80->HL.W);	\
        Z80_WR_BYTE(pZ80,pZ80->DE.W,Data);		\
										\
        pZ80->HL.W--;						\
        pZ80->DE.W--;						\
        pZ80->BC.W--;						\
										\
    Flags = Z80_FLAGS_REG & (Z80_CARRY_FLAG | Z80_ZERO_FLAG | Z80_SIGN_FLAG|Z80_PARITY_FLAG);	/* Do not change CF, ZF, SF */ \
	/* HF, NF = 0 */ \
	/* according to Sean's z80 documentation: (Data+A) bit 1 is YF, bit 3 is XF */ \
	Flags |= ((Data + pZ80->AF.B.h)&((1<<1)|(1<<3)))<<2; \
    /* ifZ80_ BC==0, then PV =0 */	\
    if (pZ80->BC.W==0)						\
    {									\
        Flags &= ~Z80_PARITY_FLAG;	\
		pZ80->PC.W.l+=2; \
		Cycles=6;					\
    }								\
	else							\
	{								\
		pZ80->MemPtr.W = pZ80->PC.W.l+1;	\
		Cycles=5;					\
	}								\
	Z80_FLAGS_REG = Flags;				\
    /* spare cycles at end of instruction that interrupt can consume if triggered */ \
    pZ80->Flags &= ~Z80_INTERRUPT_DELAY_FLAG; \
}

static void CPI(Z80_REGISTERS *pZ80)
{
        Z80_WORD Result;

		pZ80->TempByte = Z80_RD_BYTE(pZ80, pZ80->HL.W);
        Result = (Z80_WORD)pZ80->AF.B.h - (Z80_WORD)pZ80->TempByte;
        Z80_FLAGS_REG |= Z80_SUBTRACT_FLAG;
        SET_ZERO_SIGN(Result);
        SET_HALFCARRY(pZ80->TempByte, Result);

        pZ80->TempByte = Result - ((Z80_FLAGS_REG>>Z80_HALFCARRY_FLAG_BIT) & 0x01);
		Z80_FLAGS_REG &=~(Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2);
		Z80_FLAGS_REG |= ((pZ80->TempByte)&(1<<1))<<4;
        /* XF is bit 3 and takes a copy of bit 3 */
		Z80_FLAGS_REG |= (pZ80->TempByte)&(1<<3);

        pZ80->HL.W++;
        pZ80->BC.W--;
		pZ80->MemPtr.W++;

        Z80_FLAGS_REG = Z80_FLAGS_REG & (~Z80_PARITY_FLAG);
        if (pZ80->BC.W!=0)
        {
                Z80_FLAGS_REG = Z80_FLAGS_REG | Z80_PARITY_FLAG;
        }
}

static void CPD(Z80_REGISTERS *pZ80)
{
        Z80_BYTE        Result;

		pZ80->TempByte = Z80_RD_BYTE(pZ80, pZ80->HL.W);
        Result = (Z80_WORD)pZ80->AF.B.h - (Z80_WORD)pZ80->TempByte;
        Z80_FLAGS_REG |= Z80_SUBTRACT_FLAG;
        SET_ZERO_SIGN(Result);
        SET_HALFCARRY(pZ80->TempByte, Result);

        pZ80->TempByte = Result - ((Z80_FLAGS_REG>>Z80_HALFCARRY_FLAG_BIT) & 0x01);
		Z80_FLAGS_REG &=~(Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2);
		Z80_FLAGS_REG |= ((pZ80->TempByte)&(1<<1))<<4;
        /* XF is bit 3 and takes a copy of bit 3 */
		Z80_FLAGS_REG |= (pZ80->TempByte)&(1<<3);

        pZ80->HL.W--;
        pZ80->BC.W--;
		pZ80->MemPtr.W--;

        Z80_FLAGS_REG = Z80_FLAGS_REG & (~Z80_PARITY_FLAG);
        if (pZ80->BC.W!=0)
        {
                Z80_FLAGS_REG = Z80_FLAGS_REG | Z80_PARITY_FLAG;
        }
}

#if 0
static void CPIR(void)
{
        CPI();

        /* not zero and BC isn't zero */
        if (((Z80_FLAGS_REG & Z80_ZERO_FLAG)==0) && (pZ80->BC.W!=0))
        {
                pZ80->PC.W-=2;
        }
}

static void CPDR(void)
{
        CPD();

        /* if A=(HL) and BC!=0 */
        if (((Z80_FLAGS_REG & Z80_ZERO_FLAG)==0) && (pZ80->BC.W!=0))
        {
                pZ80->PC.W-=2;
        }
}
#endif


static void OUTI(Z80_REGISTERS *pZ80)
{
	pZ80->Outputs = Z80_OUTPUT_IORQ | Z80_OUTPUT_WR;
	pZ80->IOData = Z80_RD_BYTE(pZ80, pZ80->HL.W);

        pZ80->BC.B.h --;
		pZ80->MemPtr.W = pZ80->BC.W+1;
        pZ80->IOPort = pZ80->BC.W;
        Z80_DoOut(pZ80->IOPort,pZ80->IOData);

        Z80_FLAGS_REG |= Z80_SUBTRACT_FLAG;
        SET_ZERO_FLAG(pZ80->BC.B.h);

        pZ80->HL.W++;
		pZ80->Outputs = 0;
}

#if 0
/* two R refresh per instruction execution */
static void OTIR(void)
{
        OUTI();

        if ((Z80_FLAGS_REG & Z80_ZERO_FLAG)==0)
        {
                pZ80->PC.W-=2;
        }
}
#endif

/* B is pre-decremented before execution */
static void OUTD(Z80_REGISTERS *pZ80)
{

        /*Z80_UpdateCycles(2); */


	pZ80->IOData = Z80_RD_BYTE(pZ80, pZ80->HL.W);

        pZ80->BC.B.h--;
		pZ80->MemPtr.W = pZ80->BC.W-1;

        pZ80->IOPort = pZ80->BC.W;

        Z80_DoOut(pZ80->IOPort,pZ80->IOData);

        /* as per Zilog docs */
        Z80_FLAGS_REG |= Z80_SUBTRACT_FLAG;
        SET_ZERO_FLAG(pZ80->BC.B.h);

        pZ80->HL.W--;
}

static void INI(Z80_REGISTERS *pZ80)
{
        pZ80->IOPort = pZ80->BC.W;
        pZ80->IOData = Z80_DoIn(pZ80->IOPort);

		Z80_WR_BYTE(pZ80, pZ80->HL.W, pZ80->IOData);

        pZ80->HL.W++;
		pZ80->MemPtr.W = pZ80->BC.W+1;

        pZ80->BC.B.h--;

        SET_ZERO_FLAG(pZ80->BC.B.h);

        Z80_FLAGS_REG |= Z80_SUBTRACT_FLAG;
}


static void IND(Z80_REGISTERS *pZ80)
{
        pZ80->IOPort = pZ80->BC.W;
        pZ80->IOData = Z80_DoIn(pZ80->IOPort);

		Z80_WR_BYTE(pZ80, pZ80->HL.W, pZ80->IOData);

        pZ80->HL.W--;
		pZ80->MemPtr.W = pZ80->BC.W-1;
        pZ80->BC.B.h--;

        SET_ZERO_FLAG(pZ80->BC.B.h);

        Z80_FLAGS_REG |= Z80_SUBTRACT_FLAG;
}

#include "z80daa.h"



/* half carry not set */
static void DAA(Z80_REGISTERS *pZ80)
{
 int i;
 i=pZ80->AF.B.h;
 if (pZ80->AF.B.l&Z80_CARRY_FLAG) i|=256;
 if (pZ80->AF.B.l&Z80_HALFCARRY_FLAG) i|=512;
 if (pZ80->AF.B.l&Z80_SUBTRACT_FLAG) i|=1024;
 pZ80->AF.W=DAATable[i];
}

