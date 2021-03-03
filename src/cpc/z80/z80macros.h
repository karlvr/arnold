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
#include "z80.h"

#if 0
#define ADD_PC(count) \
		pZ80->PC.W.l += count
#endif

/* for addition, operands with differen signs never cause overflow. When adding operands
with like signs and the result has a differnet sign the overflow flag is set */
#define SET_OVERFLOW_FLAG_A_ADD(Reg, Result) \
        Z80_FLAGS_REG &= (~Z80_PARITY_FLAG);           \
        Z80_FLAGS_REG |= (((pZ80->AF.B.h^Reg^0x080)&(pZ80->AF.B.h^Result))>>(7-Z80_PARITY_FLAG_BIT))&Z80_PARITY_FLAG;


/* for subtraction, overflow can occur for operands of unlike signs, operands
of like signs never cause overflow */
#define SET_OVERFLOW_FLAG_A_SUB(Reg, Result) \
        Z80_FLAGS_REG &= (~Z80_PARITY_FLAG);                   \
        /* if signs are different then sign will be 1 */ \
        /* if signs are same then sign will be 0 */ \
        Z80_FLAGS_REG |= (((Reg^pZ80->AF.B.h)&(pZ80->AF.B.h^Result))>>(7-Z80_PARITY_FLAG_BIT))&Z80_PARITY_FLAG;



#define SET_HALFCARRY(Reg, Result)              \
        Z80_FLAGS_REG &= (~Z80_HALFCARRY_FLAG);        \
        Z80_FLAGS_REG |= ((Reg^pZ80->AF.B.h^Result) & Z80_HALFCARRY_FLAG)

/*
#define SET_OVERFLOW_INC(Register, Reg,Result) \
        Z80_FLAGS_REG &= (~Z80_PARITY_FLAG);           \
        Z80_FLAGS_REG |= (((Register^Reg^0x080)&(Reg^Result)&0x080)>>5)
*/

/*#define SET_OVERFLOW_DEC(Register,Reg, Result) \
        Z80_FLAGS_REG &= (~Z80_PARITY_FLAG);                   \
        Z80_FLAGS_REG |= (((Register^Reg)&(Reg^Result)&0x080)>>5)
*/

/* halfcarry carry out of bit 11 */
#define SET_HALFCARRY_16(Reg1, Reg2, Result)       \
{ \
        Z80_FLAGS_REG &= (~Z80_HALFCARRY_FLAG);        \
        Z80_FLAGS_REG |= ((Reg1^Reg2^Result)>>8) & Z80_HALFCARRY_FLAG; \
}

#define SET_OVERFLOW_FLAG_HL_ADD(Reg, Result) \
{ \
        Z80_FLAGS_REG &= (~Z80_PARITY_FLAG);           \
        Z80_FLAGS_REG |= (((pZ80->HL.W^Reg^0x08000)&(pZ80->HL.W^Result))>>(15-Z80_PARITY_FLAG_BIT))&Z80_PARITY_FLAG; \
}

#define SET_OVERFLOW_FLAG_HL_SUB(Reg, Result) \
{ \
        Z80_FLAGS_REG &= (~Z80_PARITY_FLAG);           \
        Z80_FLAGS_REG |= (((Reg^pZ80->HL.W)&(pZ80->HL.W^Result))>>(15-Z80_PARITY_FLAG_BIT))&Z80_PARITY_FLAG; \
}

/* used macros */
#define SET_ZERO_FLAG16(Register)               \
{                                                                 \
        unsigned char ZeroResult = ((Register&0x0ff)|((Register>>8)&0x0ff)); \
        Z80_FLAGS_REG &= (~Z80_ZERO_FLAG);     \
        Z80_FLAGS_REG = Z80_FLAGS_REG | (ZeroSignParityTable[ZeroResult & 0x0ff]&Z80_ZERO_FLAG); \
}

#define SET_CARRY_FLAG_ADD(Result)              \
{                                                                       \
Z80_FLAGS_REG = Z80_FLAGS_REG & (~Z80_CARRY_FLAG); \
Z80_FLAGS_REG = Z80_FLAGS_REG | ((Result>>8) & 0x01); \
}

#define SET_CARRY_FLAG_ADD16(Result)    \
{                                                                       \
Z80_FLAGS_REG = Z80_FLAGS_REG & (~Z80_CARRY_FLAG); \
Z80_FLAGS_REG = Z80_FLAGS_REG | ((Result>>16) & 0x01); \
}

#define SET_CARRY_FLAG_SUB(Result)      \
{                                                                               \
Z80_FLAGS_REG = Z80_FLAGS_REG & (~Z80_CARRY_FLAG); \
Z80_FLAGS_REG = Z80_FLAGS_REG | ((Result>>8) & 0x01); \
}

#define SET_CARRY_FLAG_SUB16(Result)    \
{                                                                               \
Z80_FLAGS_REG = Z80_FLAGS_REG & (~Z80_CARRY_FLAG); \
Z80_FLAGS_REG = Z80_FLAGS_REG | ((Result>>16) & 0x01); \
}

#define SET_SIGN_FLAG16(Register)       \
{                                                                       \
Z80_FLAGS_REG = Z80_FLAGS_REG & (~Z80_SIGN_FLAG);          \
Z80_FLAGS_REG = Z80_FLAGS_REG | ((Register & 0x08000)>>8); \
}

#define SET_ZERO_SIGN(Register)                 \
{                                                                               \
        Z80_FLAGS_REG = Z80_FLAGS_REG & ~(Z80_ZERO_FLAG | Z80_SIGN_FLAG); \
        Z80_FLAGS_REG = Z80_FLAGS_REG | (ZeroSignTable[Register & 0x0ff]); \
}

#define SET_ZERO_SIGN_PARITY(Register)  \
{                                                                               \
        Z80_FLAGS_REG = Z80_FLAGS_REG & ~(Z80_ZERO_FLAG | Z80_SIGN_FLAG | Z80_PARITY_FLAG); \
        Z80_FLAGS_REG = Z80_FLAGS_REG | ZeroSignParityTable[Register & 0x0ff]; \
}

#define SET_ZERO_FLAG(Register) \
{                                                                               \
        Z80_FLAGS_REG = Z80_FLAGS_REG & ~(Z80_ZERO_FLAG); \
        Z80_FLAGS_REG = Z80_FLAGS_REG | (ZeroSignParityTable[Register & 0x0ff] & Z80_ZERO_FLAG); \
}

/*------------------*/
/* RES */

#define RES(AndMask,Register) \
        Register = Register & (~AndMask); \

#define RES_REG(AndMask, Register) \
{                                                                       \
        RES(AndMask, Register);                 \
                                                                        \
}

#define RES_HL(AndMask)                 \
{                                                               \
        /*Z80_BYTE Data;        */                      \
                                                                \
        pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->HL.W);       \
                                                                \
        RES(AndMask, pZ80->TempByte);                       \
                                                                \
        Z80_WR_BYTE(pZ80,pZ80->HL.W, pZ80->TempByte); \
}

#if 0
#define RES_INDEX(AndMask,IndexReg)     \
{                                                               \
        /*Z80_BYTE Data;*/                              \
		SETUP_INDEXED_ADDRESS(pZ80,IndexReg); \
                                                                \
        pZ80->TempByte = Z80_RD_BYTE(pZ80->MemPtr.W);   \
                                                                \
        RES(AndMask, pZ80->TempByte);                       \
                                                                \
        Z80_WR_BYTE(pZ80->MemPtr.W, pZ80->TempByte);     \
}
#endif


/* SET */

#define SET(OrMask, Register) \
        Register = Register | OrMask;   \

#define SET_REG(OrMask, Register) \
{                                                                \
        SET(OrMask, Register)           \
                                                                \
}

#define SET_HL(OrMask)                  \
{                                                               \
        /*Z80_BYTE Data;*/                              \
                                                                \
        pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->HL.W); \
                                                                \
        SET(OrMask, pZ80->TempByte);                        \
                                                                \
        Z80_WR_BYTE(pZ80,pZ80->HL.W, pZ80->TempByte);  \
}

#if 0
#define SET_INDEX(OrMask, IndexReg)     \
{                                                               \
        /*Z80_BYTE      Data;*/                         \
                                                                \
		SETUP_INDEXED_ADDRESS(pZ80,IndexReg); \
        pZ80->TempByte = Z80_RD_BYTE(pZ80->MemPtr.W);   \
                                                                \
        SET(OrMask, pZ80->TempByte);                        \
                                                                \
        Z80_WR_BYTE(pZ80->MemPtr.W, pZ80->TempByte);     \
}
#endif

#if 0
#define BIT_REG(BitIndex, Register)     \
{    \
        BIT(BitIndex, Register);                \
                                                                        \
}
#endif

#if 0
#define BIT_HL(BitIndex)                                \
{ \
    pZ80->TempByte =Z80_RD_BYTE(pZ80->HL.W); \
    BIT_(BitIndex,Z80_RD_BYTE(pZ80->HL.W));                               \
                                                                        \
}

#define BIT_INDEX(BitIndex, IndexReg)                           \
{                                                                       \
        BIT_MP(BitIndex,RD_BYTE_INDEX(IndexReg));                               \
}
#endif

/*------------------*/
#define SHIFTING_FLAGS(Register)        \
        SET_ZERO_SIGN_PARITY(Register);  \
        Z80_FLAGS_REG &= Z80_SIGN_FLAG | Z80_ZERO_FLAG | Z80_PARITY_FLAG | Z80_CARRY_FLAG; \
        Z80_FLAGS_REG |= (Register & (Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2)); \





#define SET_CARRY_LEFT_SHIFT(Register)          \
Z80_FLAGS_REG = Z80_FLAGS_REG & (~Z80_CARRY_FLAG); \
Z80_FLAGS_REG = Z80_FLAGS_REG | ((Register>>7) & 0x01)



#define SET_CARRY_RIGHT_SHIFT(Register)         \
Z80_FLAGS_REG = Z80_FLAGS_REG & (~Z80_CARRY_FLAG); \
Z80_FLAGS_REG = Z80_FLAGS_REG | (Register & 0x001)




#define RL(Register)                            \
{                                                                       \
        Z80_BYTE Carry;                                 \
                                                                        \
        Carry = (Z80_FLAGS_REG & (Z80_BYTE)Z80_CARRY_FLAG); \
                                                                        \
        SET_CARRY_LEFT_SHIFT(Register); \
                                                                        \
        Register=Register<<1;                   \
                                                                        \
        Register=Register | Carry;                              \
}

#define RL_WITH_FLAGS(Register)         \
{                                                                       \
        RL(Register);                                   \
                                                                        \
        SHIFTING_FLAGS(Register);               \
}

#define RL_REG(Register)                        \
{                                                                       \
        RL_WITH_FLAGS(Register);                \
}

#define RL_HL()								\
{											\
	/*Z80_BYTE      Data;*/				\
											\
        pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->HL.W);	\
											\
        RL_WITH_FLAGS(pZ80->TempByte);			\
											\
        Z80_WR_BYTE(pZ80,pZ80->HL.W,pZ80->TempByte);		\
}


#define RL_INDEX(IndexReg)                      \
{                                                                       \
        /*Z80_BYTE Data;        */                              \
                                                                        \
 		SETUP_INDEXED_ADDRESS(pZ80,IndexReg); \
       pZ80->TempByte = Z80_RD_BYTE(pZ80->MemPtr.W);   \
                                                                        \
        RL_WITH_FLAGS(pZ80->TempByte);                                              \
                                                                        \
        Z80_WR_BYTE(pZ80->MemPtr.W, pZ80->TempByte);             \
}                                                                       \


#define RR(Register)                            \
{                                                                       \
        Z80_BYTE Carry;                                         \
                                                                        \
        Carry = Z80_FLAGS_REG & Z80_CARRY_FLAG;     \
                                                                        \
        SET_CARRY_RIGHT_SHIFT(Register);                        \
                                                                        \
        Register=Register>>1;                   \
                                                                        \
        Register=Register | (Carry<<7); \
                                                                        \
}

#define RR_WITH_FLAGS(Register)         \
{                                                                       \
        RR(Register);                                   \
                                                                        \
        SHIFTING_FLAGS(Register);               \
}

#define RR_REG(Register)                        \
{                                                                       \
        RR_WITH_FLAGS(Register);                \
}

#define RR_HL()									\
{												\
        /*Z80_BYTE      Data;	*/				\
												\
        pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->HL.W);		\
												\
        RR_WITH_FLAGS(pZ80->TempByte);				\
												\
        Z80_WR_BYTE(pZ80,pZ80->HL.W,pZ80->TempByte);			\
}


#define RR_INDEX(IndexReg)                      \
{                                                                       \
        /*Z80_BYTE Data;*/                                      \
                                                                        \
 		SETUP_INDEXED_ADDRESS(pZ80,IndexReg); \
       pZ80->TempByte = Z80_RD_BYTE(pZ80->MemPtr.W);   \
                                                                        \
        RR_WITH_FLAGS(pZ80->TempByte);                                              \
                                                                        \
        Z80_WR_BYTE(pZ80->MemPtr.W, pZ80->TempByte);             \
}                                                                       \

/* rol doesn't set sign or zero! */



#define RLC(Register)                           \
{                                                                       \
        Z80_BYTE        OrByte;                         \
                                                                        \
        OrByte = (Register>>7) & 0x01;  \
                                                                        \
        SET_CARRY_LEFT_SHIFT(Register);                 \
                                                                        \
        Register=Register<<1;                   \
        Register=Register | OrByte;                             \
}

#define RLC_WITH_FLAGS(Register)        \
{                                                                       \
        RLC(Register);                                  \
                                                                        \
        SHIFTING_FLAGS(Register);               \
}

#define RLC_REG(Register)                       \
{                                                                       \
        RLC_WITH_FLAGS(Register);       \
}

#define RLC_HL()						\
{											\
        pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->HL.W);	\
											\
        RLC_WITH_FLAGS(pZ80->TempByte);			\
											\
        Z80_WR_BYTE(pZ80,pZ80->HL.W,pZ80->TempByte);		\
}


#define RLC_INDEX(IndexReg)                     \
{                                                                       \
        /*Z80_BYTE      Data;   */                              \
                                                                        \
		SETUP_INDEXED_ADDRESS(pZ80,IndexReg); \
        pZ80->TempByte = Z80_RD_BYTE(pZ80->MemPtr.W);   \
                                                                        \
        RLC_WITH_FLAGS(pZ80->TempByte);                     \
                                                                        \
        Z80_WR_BYTE(pZ80->MemPtr.W, pZ80->TempByte);             \
}                                                                       \

#define RRC(Register)                           \
{                                                                       \
        Z80_BYTE        OrByte;                                 \
                                                                        \
        OrByte = Register & 0x001;              \
                                                                        \
        SET_CARRY_RIGHT_SHIFT(Register);                        \
                                                                        \
        Register=Register>>1;                   \
                                                                        \
        Register=Register | (OrByte<<7);                                \
}

#define RRC_WITH_FLAGS(Register)        \
{                                                                       \
        RRC(Register);                                  \
                                                                        \
        SHIFTING_FLAGS(Register);               \
}

#define RRC_REG(Register)                       \
{                                                                       \
        RRC_WITH_FLAGS(Register);       \
}

#define  RRC_HL()			\
{							\
        pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->HL.W);	\
											\
        RRC_WITH_FLAGS(pZ80->TempByte);			\
											\
        Z80_WR_BYTE(pZ80,pZ80->HL.W,pZ80->TempByte);		\
}


#define RRC_INDEX(IndexReg)                     \
{                                                                       \
        /*Z80_BYTE      Data;   */                      \
                                                                        \
		SETUP_INDEXED_ADDRESS(pZ80,IndexReg); \
        pZ80->TempByte = Z80_RD_BYTE(pZ80->MemPtr.W);   \
                                                                        \
        RRC_WITH_FLAGS(pZ80->TempByte);                     \
                                                                        \
        Z80_WR_BYTE(pZ80->MemPtr.W, pZ80->TempByte);             \
}                                                                       \

#define SLA(Register)							\
{												\
	Z80_BYTE	Reg;							\
	Z80_BYTE	Flags;							\
	Reg = Register;								\
	Flags = ((Reg>>7) & 0x01);	/* carry */				\
	Reg = (Reg<<1);								\
	Flags = Flags | ZeroSignParityTable[Reg]|(Reg&((1<<5)|(1<<3)));	/* sign, zero, parity, f5, f3 */ \
	Register = Reg;								\
	Z80_FLAGS_REG = Flags;							\
}




#define SLA_WITH_FLAGS(Register)        \
{                                                                       \
        SLA(Register);                                  \
}

#define SLA_REG(Register)                       \
{                                                                       \
        SLA_WITH_FLAGS(Register);               \
}

#define SLA_HL()								\
{												\
        pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->HL.W);		\
        SLA_WITH_FLAGS(pZ80->TempByte);				\
        Z80_WR_BYTE(pZ80,pZ80->HL.W,pZ80->TempByte);			\
}


#define SLA_INDEX(IndexReg)                     \
{                                                                       \
        /*Z80_BYTE      Data;   */                      \
                                                                        \
		SETUP_INDEXED_ADDRESS(pZ80,IndexReg); \
        pZ80->TempByte = Z80_RD_BYTE(pZ80->MemPtr.W);   \
                                                                        \
        SLA_WITH_FLAGS(pZ80->TempByte);                     \
                                                                        \
        Z80_WR_BYTE(pZ80->MemPtr.W, pZ80->TempByte);     \
}                                                                       \


#define SRA(Register)							\
{												\
	Z80_BYTE	Reg;							\
	Z80_BYTE	Flags;							\
	Z80_BYTE	OrByte;							\
	Reg = Register;								\
	Flags = Reg & 0x01;	/* carry */				\
	OrByte = Reg & 0x080;						\
	Reg = (Reg>>1) | OrByte;					\
	Flags = Flags | ZeroSignParityTable[Reg]|(Reg&((1<<5)|(1<<3)));	/* sign, zero, parity, f5, f3 */ \
	Register = Reg;								\
	Z80_FLAGS_REG = Flags;							\
}



#define SRA_WITH_FLAGS(Register)        \
{                                                                       \
        SRA(Register);                                  \
}

#define SRA_REG(Register)                       \
{                                                                       \
        SRA_WITH_FLAGS(Register);               \
}

#define SRA_HL()							\
{											\
        pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->HL.W);	\
        SRA_WITH_FLAGS(pZ80->TempByte);			\
        Z80_WR_BYTE(pZ80,pZ80->HL.W,pZ80->TempByte);		\
}

#define SRA_INDEX(Index)                \
{                                                                       \
        /*Z80_BYTE      Data;*/                                 \
                                                                        \
		SETUP_INDEXED_ADDRESS(pZ80,Index); \
        pZ80->TempByte = Z80_RD_BYTE(pZ80->MemPtr.W);      \
                                                                        \
        SRA_WITH_FLAGS(pZ80->TempByte);                                             \
                                                                        \
        Z80_WR_BYTE(pZ80->MemPtr.W, pZ80->TempByte);                \
}                                                                       \




#define SRL(Register)                           \
{												\
	Z80_BYTE	Reg;							\
	Z80_BYTE	Flags;							\
	Reg = Register;								\
	Flags = Reg & 0x001; /* carry */			\
	Reg = Reg>>1;								\
	Flags = Flags | ZeroSignParityTable[Reg]|(Reg&((1<<5)|(1<<3)));	/* sign, zero, parity, f5, f3 */	\
	Register = Reg;								\
	Z80_FLAGS_REG = Flags;							\
}

#define SRL_WITH_FLAGS(Register)        \
{                                                                       \
        SRL(Register);                                  \
                                                                        \
}

#define SRL_REG(Register)                       \
{                                                                       \
        SRL_WITH_FLAGS(Register);               \
}

#define SRL_HL()								\
{												\
        pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->HL.W);		\
        SRL_WITH_FLAGS(pZ80->TempByte);				\
        Z80_WR_BYTE(pZ80,pZ80->HL.W,pZ80->TempByte);			\
}

#define SRL_INDEX(Index)                        \
{                                                                       \
        /*Z80_BYTE      Data;*/                         \
                                                                        \
		SETUP_INDEXED_ADDRESS(pZ80,Index); \
        pZ80->TempByte = Z80_RD_BYTE(pZ80->MemPtr.W);      \
                                                                        \
        SRL_WITH_FLAGS(pZ80->TempByte);                     \
                                                                        \
        Z80_WR_BYTE(pZ80->MemPtr.W, pZ80->TempByte);                \
}                                                                       \


#define SLL(Register)							\
{												\
	Z80_BYTE	Reg;							\
	Z80_BYTE	Flags;							\
	Reg = Register;								\
	Flags = (Reg>>7) & 0x01;	/* carry */				\
	Reg = ((Reg<<1) | 1);								\
	Flags = Flags | ZeroSignParityTable[Reg]|(Reg&((1<<5)|(1<<3)));	/* sign, zero, parity, f5, f3 */ \
	Register = Reg;								\
	Z80_FLAGS_REG = Flags;							\
}




#define SLL_WITH_FLAGS(Register)        \
{                                                                       \
        SLL(Register);                                  \
}

#define SLL_REG(Register)                       \
{                                                                       \
        SLL_WITH_FLAGS(Register);               \
}

#define SLL_HL()							\
{											\
        pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->HL.W);	\
        SLL_WITH_FLAGS(pZ80->TempByte);			\
        Z80_WR_BYTE(pZ80,pZ80->HL.W,pZ80->TempByte);		\
}

#define SLL_INDEX(IndexReg)                     \
{                                                                       \
        /*Z80_BYTE      Data;   */                      \
                                                                        \
		SETUP_INDEXED_ADDRESS(pZ80,IndexReg); \
        pZ80->TempByte = Z80_RD_BYTE(pZ80->MemPtr.W);   \
                                                                        \
        SLL_WITH_FLAGS(pZ80->TempByte);                     \
                                                                        \
        Z80_WR_BYTE(pZ80->MemPtr.W, pZ80->TempByte);     \
}                                                                       \

/*-----------------*/
/*
#define A_SHIFTING_FLAGS \
        Z80_FLAGS_REG = Z80_FLAGS_REG & (Z80_SIGN_FLAG | Z80_ZERO_FLAG | Z80_PARITY_FLAG | Z80_CARRY_FLAG)
*/
/*---------------*/

#define SET_LOGIC_FLAGS                         \
        SET_ZERO_SIGN_PARITY(pZ80->AF.B.h); \
        Z80_FLAGS_REG = Z80_FLAGS_REG & (Z80_ZERO_FLAG | Z80_PARITY_FLAG | Z80_SIGN_FLAG)


#define AND_A_X(Register)					\
{											\
	Z80_BYTE	Flags;						\
											\
	pZ80->AF.B.h = pZ80->AF.B.h & Register;			\
	Flags = ZeroSignParityTable[pZ80->AF.B.h];	\
	Flags = Flags & ~(Z80_CARRY_FLAG|Z80_SUBTRACT_FLAG|Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2); \
	Flags = Flags | (pZ80->AF.B.h & (Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2)); \
	Flags = Flags | Z80_HALFCARRY_FLAG;		\
	Z80_FLAGS_REG = Flags;						\
}


#define AND_A_R(Register)                       \
{                                                                       \
        AND_A_X(Register);                              \
}


#define AND_A_INDEX(Index)              \
{                                                               \
        /*Z80_BYTE      Data;*/                         \
                                                                \
                                                                \
 		SETUP_INDEXED_ADDRESS(pZ80,Index); \
       pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W);      \
                                                                                \
        AND_A_X(pZ80->TempByte);                                    \
}





#define XOR_A_X(Register)					\
{											\
	Z80_BYTE	Flags;						\
											\
	pZ80->AF.B.h = pZ80->AF.B.h ^ Register;			\
	Flags = ZeroSignParityTable[pZ80->AF.B.h];	\
	Flags = Flags & ~(Z80_CARRY_FLAG|Z80_SUBTRACT_FLAG|Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2|Z80_HALFCARRY_FLAG); \
	Flags = Flags | (pZ80->AF.B.h & (Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2)); \
	Z80_FLAGS_REG = Flags;						\
}


#define XOR_A_R(Register)                       \
{                                                                       \
        XOR_A_X(Register);                              \
}


#define XOR_A_INDEX(Index)              \
{                                                               \
        /*Z80_BYTE      Data;*/                         \
                                                                \
		SETUP_INDEXED_ADDRESS(pZ80,Index); \
        pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W);      \
                                                                        \
        XOR_A_X(pZ80->TempByte);                                    \
}



#define OR_A_X(Register)					\
{											\
	Z80_BYTE	Flags;						\
											\
	pZ80->AF.B.h = pZ80->AF.B.h | Register;			\
	Flags = ZeroSignParityTable[pZ80->AF.B.h];	\
	Flags = Flags & ~(Z80_CARRY_FLAG|Z80_SUBTRACT_FLAG|Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2|Z80_HALFCARRY_FLAG); \
	Flags = Flags | (pZ80->AF.B.h & (Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2)); \
	Z80_FLAGS_REG = Flags;						\
}


#define OR_A_R(Register)                        \
{                                                                       \
        OR_A_X(Register);                               \
}



#define OR_A_INDEX(Index)               \
{                                                               \
        /*Z80_BYTE      Data;*/                         \
                                                                \
		SETUP_INDEXED_ADDRESS(pZ80,Index); \
        pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W);      \
                                                                        \
        OR_A_X(pZ80->TempByte);                                     \
}

/*--------------*/

/* INC */
#define INC_X(Register)                                                         \
{                                                                                                       \
        Z80_BYTE Flags;						\
		Flags = Z80_FLAGS_REG;					\
		Flags  = Flags & Z80_CARRY_FLAG;	 \
        if ((Z80_BYTE)(Register & 0x0f)==(Z80_BYTE)0x0f)                                        \
        {                                                                                               \
                Flags |= Z80_HALFCARRY_FLAG;                        \
        }                                                                                               \
                                                                                                        \
        if ((Z80_BYTE)Register==(Z80_BYTE)0x07f)                                        \
        {                                                                                               \
                Flags |= Z80_OVERFLOW_FLAG;                         \
        }                                                                                               \
        Register++;                                                                                     \
		Flags |= ZeroSignTable[Register];								\
		Flags |= Register & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);		\
		Z80_FLAGS_REG = Flags;												\
        \
}



#define INC_R(Register)                         \
{                                                                       \
        INC_X(Register);                                \
                                                                        \
}
#if 0
#define INC_INDEX8(Register)            \
{                                                                       \
        INC_X(Register);                                \
                                                                        \
        pZ80->PC.W+=2;                                              \
}
#endif

#define INC_HL_()		\
{											\
        pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->HL.W);	\
        INC_X(pZ80->TempByte);					\
        Z80_WR_BYTE(pZ80,pZ80->HL.W,pZ80->TempByte);		\
}

#define _INC_INDEX_(Index)                      \
{                                                                       \
        /*Z80_BYTE      Data;*/                         \
                                                                        \
 		SETUP_INDEXED_ADDRESS(pZ80,Index); \
       pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W);      \
                                                                        \
        INC_X(pZ80->TempByte);                                      \
                                                                        \
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);                \
}


/* INC */
#define DEC_X(Register)                                                         \
{                                                                                                       \
        Z80_BYTE Flags;						\
		Flags = Z80_FLAGS_REG;					\
		Flags  = Flags & Z80_CARRY_FLAG;	 \
		Flags = Flags | Z80_SUBTRACT_FLAG;	\
        if ((Z80_BYTE)(Register & 0x0f)==0)                                        \
        {                                                                                               \
                Flags |= Z80_HALFCARRY_FLAG;                        \
        }                                                                                               \
                                                                                                        \
        if ((Z80_BYTE)Register==(Z80_BYTE)0x080)                                        \
        {                                                                                               \
                Flags |= Z80_OVERFLOW_FLAG;                         \
        }                                                                                               \
        Register--;                                                                                     \
		Flags |= ZeroSignTable[Register];								\
		Flags |= Register & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);		\
		Z80_FLAGS_REG = Flags;												\
        \
}

#if 0
/* DEC */
#define DEC_X(Register)                                                                 \
{       Z80_BYTE		Flags;                                                                                                        \
        register Z80_BYTE		Reg;                                                                                                        \
		Reg=Register;	\
		Flags = Z80_FLAGS_REG;	\
        Flags = Flags & ~(Z80_HALFCARRY_FLAG | Z80_OVERFLOW_FLAG);             \
        if ((Z80_BYTE)Reg==(Z80_BYTE)0x080)                                                                \
        {                                                                                                       \
                Flags |= Z80_OVERFLOW_FLAG;                                 \
        }                                                                                                       \
        if ((Z80_BYTE)(Reg & 0x0f)==(Z80_BYTE)0x00)                                                \
        {                                                                                                       \
                Flags|= Z80_HALFCARRY_FLAG;                                \
        }                                                                                                       \
                                                                                                                \
		Flags |= Z80_SUBTRACT_FLAG;	\
		Z80_FLAGS_REG = Flags;			\
		Register = Reg-1;	\
        SET_ZERO_SIGN(Register);                                                        \
}
#endif

#define DEC_R(Register)                         \
{                                                                       \
        DEC_X(Register);                                \
                                                                        \
}

#if 0
#define DEC_INDEX8(Register)            \
{                                                                       \
        DEC_X(Register);                                \
                                                                       \
        pZ80->PC.W+=2;                                              \
}
#endif

#define DEC_HL_()				\
{												\
        pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->HL.W);		\
        DEC_X(pZ80->TempByte);						\
        Z80_WR_BYTE(pZ80,pZ80->HL.W,pZ80->TempByte);			\
}


#define _DEC_INDEX_(Index)                      \
{                                                                       \
        /*Z80_BYTE      Data;*/                                 \
                                                                        \
		SETUP_INDEXED_ADDRESS(pZ80,Index); \
        pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W);      \
                                                                        \
        DEC_X(pZ80->TempByte);                                      \
                                                                        \
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);                \
}

/*------------*/
/* Macros */
#define ADD_A_X(Register)      \
{                                                                       \
        Z80_WORD        Result=0;                                       \
                                                                        \
        Result = (Z80_WORD)pZ80->AF.B.h + (Z80_WORD)Register;\
                                                                        \
        SET_OVERFLOW_FLAG_A_ADD(Register,Result); \
        SET_CARRY_FLAG_ADD(Result);                     \
        SET_HALFCARRY(Register, Result);       \
        SET_ZERO_SIGN(Result);                \
        pZ80->AF.B.h = (Z80_BYTE)(Result&0x0ff);            \
        Z80_FLAGS_REG &= ~(Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2 | Z80_SUBTRACT_FLAG);  \
        Z80_FLAGS_REG |= Result & (Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2); \
}

#define ADD_A_R(Register)                       \
{                                                                       \
        ADD_A_X(Register);                              \
}

#if 0
#define ADD_A_INDEX8(Register)          \
{                                                                       \
        ADD_A_X(Register);                              \
                                                                        \
        pZ80->PC.W+=2;                                              \
}
#endif



#define ADC_A_X(Register)                       \
{                                                                       \
        Z80_WORD        Result=0;                                       \
                                                                        \
        Result = (Z80_WORD)pZ80->AF.B.h + (Z80_WORD)Register + (Z80_WORD)(Z80_FLAGS_REG & Z80_CARRY_FLAG);      \
                                                                        \
        SET_OVERFLOW_FLAG_A_ADD(Register,Result); \
        SET_CARRY_FLAG_ADD(Result);                     \
        SET_HALFCARRY(Register, Result);        \
                                                                        \
        SET_ZERO_SIGN(Result);                \
        pZ80->AF.B.h = (Z80_BYTE)(Result&0x0ff);            \
        Z80_FLAGS_REG &= ~(Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2|Z80_SUBTRACT_FLAG);  \
        Z80_FLAGS_REG |= Result & (Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2); \
}

#define ADC_A_R(Register)                       \
{                                                                       \
        ADC_A_X(Register);                              \
}

#if 0
#define ADC_A_INDEX8(Register)          \
{                                                                       \
        ADC_A_X(Register);                      \
                                                                        \
        pZ80->PC.W+=2;                                              \
}
#endif


#define SUB_A_X(Register)                       \
{                                                                       \
        Z80_WORD        Result=0;                                       \
                                                                        \
        Result = (Z80_WORD)pZ80->AF.B.h - (Z80_WORD)Register;       \
                                                                        \
        SET_OVERFLOW_FLAG_A_SUB(Register,Result); \
        SET_CARRY_FLAG_SUB(Result);                     \
        SET_HALFCARRY(Register, Result);        \
                                                                        \
        SET_ZERO_SIGN(Result);                \
        pZ80->AF.B.h = (Z80_BYTE)(Result&0x0ff);            \
        Z80_FLAGS_REG &= ~(Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);  \
        Z80_FLAGS_REG |= Result & (Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2); \
        Z80_FLAGS_REG = Z80_FLAGS_REG | Z80_SUBTRACT_FLAG;              \
}

#define SUB_A_R(Register)                       \
{                                                                       \
        SUB_A_X(Register);                              \
}

#if 0
#define SUB_A_INDEX8(Register)          \
{                                                                       \
        SUB_A_X(Register);                              \
                                                                        \
        pZ80->PC.W+=2;                                              \
}
#endif

#define CP_A_X(Register)                        \
{                                                                       \
        Z80_WORD        Result=0;                                       \
                                                                        \
        Result = (Z80_WORD)pZ80->AF.B.h - (Z80_WORD)Register;       \
                                                                        \
        SET_OVERFLOW_FLAG_A_SUB(Register,Result); \
        SET_CARRY_FLAG_SUB(Result);                     \
        SET_HALFCARRY(Register, Result);        \
                                                                        \
        SET_ZERO_SIGN(Result);                  \
                                                                        \
        Z80_FLAGS_REG = Z80_FLAGS_REG | Z80_SUBTRACT_FLAG;              \
        Z80_FLAGS_REG &= ~(Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);  \
        Z80_FLAGS_REG |= Register & (Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2); \
}

#define CP_A_R(Register)                        \
{                                                                       \
        CP_A_X(Register);                               \
}

#if 0
#define CP_A_INDEX8(Register)           \
{                                                                       \
        CP_A_X(Register);                               \
                                                                        \
        pZ80->PC.W+=2;                                              \
}
#endif

#define SBC_A_X(Register)                       \
{                                                                       \
        Z80_WORD                Result=0;                                       \
                                                                        \
         Z80_FLAGS_REG &= ~(Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);  \
       Result = (Z80_WORD)pZ80->AF.B.h - (Z80_WORD)Register - (Z80_WORD)(Z80_FLAGS_REG & Z80_CARRY_FLAG);      \
                                                                        \
        SET_OVERFLOW_FLAG_A_SUB(Register,Result);  \
        SET_CARRY_FLAG_SUB(Result);                     \
        SET_HALFCARRY(Register, Result);        \
                                                                        \
         SET_ZERO_SIGN(Result);                \
        pZ80->AF.B.h = (Z80_BYTE)(Result&0x0ff);            \
        Z80_FLAGS_REG |= Result & (Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2); \
        Z80_FLAGS_REG = Z80_FLAGS_REG | Z80_SUBTRACT_FLAG;              \
}

#define SBC_A_R(Register)                       \
{                                                                       \
        SBC_A_X(Register);                              \
}

#if 0
#define SBC_A_INDEX8(Register)          \
{                                                                       \
        SBC_A_X(Register);                              \
                                                                        \
        pZ80->PC.W+=2;                                              \
}
#endif



#define ADD_RR_rr(Register1, Register2)                 \
{                                                                       \
        Z80_LONG                Result=0;                               \
                                                                        \
        pZ80->MemPtr.W = Register1+1; \
        Result = (Z80_LONG)Register1 + (Z80_LONG)Register2;             \
                                                                        \
        SET_CARRY_FLAG_ADD16(Result);           \
        SET_HALFCARRY_16(Register1, Register2, Result);     \
                                                                        \
        Register1 = Result&0x0ffff;           \
                                                                        \
        /* sign, zero and parity are not changed */ \
        /* subtract flag is reset */ \
        Z80_FLAGS_REG = Z80_FLAGS_REG & (Z80_CARRY_FLAG | Z80_ZERO_FLAG | Z80_PARITY_FLAG | Z80_SIGN_FLAG|Z80_HALFCARRY_FLAG); \
        /* undocumented flags affected by upper byte */ \
        Z80_FLAGS_REG |= (Result>>8) & (Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2); \
}


#define ADC_HL_rr(Register)                     \
{                                                                       \
        Z80_LONG                Result=0;                                       \
                                                                        \
        pZ80->MemPtr.W = pZ80->HL.W+1; \
        Result = (Z80_LONG)pZ80->HL.W + (Z80_LONG)Register + (Z80_LONG)(Z80_FLAGS_REG & Z80_CARRY_FLAG);                \
                                                                        \
        SET_OVERFLOW_FLAG_HL_ADD(Register,Result); \
        SET_CARRY_FLAG_ADD16(Result);           \
        SET_HALFCARRY_16(pZ80->HL.W, Register, Result);     \
                                                                        \
        pZ80->HL.W = (Z80_WORD)(Result&0x0ffff);              \
                                                                        \
        SET_SIGN_FLAG16(pZ80->HL.W);                \
        SET_ZERO_FLAG16(pZ80->HL.W);                \
                                                                        \
        Z80_FLAGS_REG = Z80_FLAGS_REG & (Z80_CARRY_FLAG | Z80_ZERO_FLAG | Z80_PARITY_FLAG | Z80_SIGN_FLAG|Z80_HALFCARRY_FLAG); \
        /* undocumented flags affected by upper byte */ \
        Z80_FLAGS_REG |= pZ80->HL.B.h & (Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2); \
}


#define SBC_HL_rr(Register)                     \
{                                                                       \
        Z80_LONG                Result=0;                                       \
                                                                        \
        Result = (Z80_LONG)pZ80->HL.W - (Z80_LONG)Register - (Z80_LONG)(Z80_FLAGS_REG & Z80_CARRY_FLAG);                \
        pZ80->MemPtr.W = pZ80->HL.W+1; \
        SET_OVERFLOW_FLAG_HL_SUB(Register,Result); \
        SET_CARRY_FLAG_SUB16(Result);                   \
        SET_HALFCARRY_16(pZ80->HL.W,Register, Result);     \
                                                                        \
        pZ80->HL.W = (Z80_WORD)Result;              \
                                                                        \
        SET_SIGN_FLAG16(pZ80->HL.W);                \
        SET_ZERO_FLAG16(pZ80->HL.W);                \
                                                                        \
        Z80_FLAGS_REG = Z80_FLAGS_REG & (Z80_CARRY_FLAG | Z80_ZERO_FLAG | Z80_PARITY_FLAG | Z80_SIGN_FLAG|Z80_HALFCARRY_FLAG); \
        /* set subtract flag */ \
        Z80_FLAGS_REG = Z80_FLAGS_REG | Z80_SUBTRACT_FLAG;              \
       /* undocumented flags affected by upper byte */ \
        Z80_FLAGS_REG |= pZ80->HL.B.h & (Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2); \
}

#if 0
/* do a sla of index and copy into reg specified */
#define INDEX_CB_SLA_REG(IndexReg, Reg)         \
{                                                                       \
        /*Z80_BYTE      Data;*/                         \
                                                                        \
		SETUP_INDEXED_ADDRESS(pZ80,IndexReg); \
        pZ80->TempByte = Z80_RD_BYTE(pZ80->MemPtr.W);   \
                                                                        \
        SLA_WITH_FLAGS(pZ80->TempByte);                     \
                                                                        \
        Z80_WR_BYTE(pZ80->MemPtr.W, pZ80->TempByte);     \
                                                                        \
        Reg = pZ80->TempByte;                                               \
}

/* do a sra of index and copy into reg specified */
#define INDEX_CB_SRA_REG(IndexReg, Reg)         \
{                                                                       \
        /*Z80_BYTE      Data;*/                         \
                                                                        \
 		SETUP_INDEXED_ADDRESS(pZ80,IndexReg); \
       pZ80->TempByte = Z80_RD_BYTE(pZ80->MemPtr.W);   \
                                                                        \
        SRA_WITH_FLAGS(pZ80->TempByte);                     \
                                                                        \
        Z80_WR_BYTE(pZ80->MemPtr.W, pZ80->TempByte);     \
                                                                        \
        Reg = pZ80->TempByte;                                               \
}

/* do a sll of index and copy into reg specified */
#define INDEX_CB_SLL_REG(IndexReg, Reg)         \
{                                                                       \
        /*Z80_BYTE      Data;*/                         \
                                                                        \
 		SETUP_INDEXED_ADDRESS(pZ80,IndexReg); \
       pZ80->TempByte =  Z80_RD_BYTE(pZ80->MemPtr.W);  \
                                                                        \
        SLL_WITH_FLAGS(pZ80->TempByte);                     \
                                                                        \
        Z80_WR_BYTE(pZ80->MemPtr.W, pZ80->TempByte);     \
                                                                        \
        Reg = pZ80->TempByte;                                               \
}

/* do a srl of index and copy into reg specified */
#define INDEX_CB_SRL_REG(IndexReg, Reg)         \
{                                                                       \
        /*Z80_BYTE      Data;   */                      \
                                                                        \
 		SETUP_INDEXED_ADDRESS(pZ80,IndexReg); \
       pZ80->TempByte = Z80_RD_BYTE(pZ80->MemPtr.W);   \
                                                                        \
        SRL_WITH_FLAGS(pZ80->TempByte);                     \
                                                                        \
        Z80_WR_BYTE(pZ80->MemPtr.W, pZ80->TempByte);     \
                                                                        \
        Reg = pZ80->TempByte;                                               \
}

#define INDEX_CB_RLC_REG(IndexReg, Reg)         \
{                                                                       \
        /*Z80_BYTE      Data;   */                      \
                                                                        \
 		SETUP_INDEXED_ADDRESS(pZ80,IndexReg); \
       pZ80->TempByte = Z80_RD_BYTE(pZ80->MemPtr.W);   \
                                                                        \
        RLC_WITH_FLAGS(pZ80->TempByte);                     \
                                                                        \
        Z80_WR_BYTE(pZ80->MemPtr.W, pZ80->TempByte);     \
                                                                        \
        Reg = pZ80->TempByte;                                               \
}

#define INDEX_CB_RRC_REG(IndexReg, Reg)         \
{                                                                       \
        /*Z80_BYTE      Data;*/                         \
                                                                        \
		SETUP_INDEXED_ADDRESS(pZ80,IndexReg); \
        pZ80->TempByte = Z80_RD_BYTE(pZ80->MemPtr.W);   \
                                                                        \
        RRC_WITH_FLAGS(pZ80->TempByte);                     \
                                                                        \
        Z80_WR_BYTE(pZ80->MemPtr.W, pZ80->TempByte);     \
                                                                        \
        Reg = pZ80->TempByte;                                               \
}


#define INDEX_CB_RR_REG(IndexReg, Reg)          \
{                                                                       \
        /*Z80_BYTE      Data;*/                         \
                                                                        \
		SETUP_INDEXED_ADDRESS(pZ80,IndexReg); \
        pZ80->TempByte = Z80_RD_BYTE(pZ80->MemPtr.W);   \
                                                                        \
        RR_WITH_FLAGS(pZ80->TempByte);                      \
                                                                        \
        Z80_WR_BYTE(pZ80->MemPtr.W, pZ80->TempByte);     \
                                                                        \
        Reg = pZ80->TempByte;                                               \
                                                                        \
}


#define INDEX_CB_RL_REG(IndexReg, Reg)          \
{                                                                       \
        /*Z80_BYTE      Data;*/                         \
                                                                        \
		SETUP_INDEXED_ADDRESS(pZ80,IndexReg); \
        pZ80->TempByte = Z80_RD_BYTE(pZ80->MemPtr.W);   \
                                                                        \
        RL_WITH_FLAGS(pZ80->TempByte);                      \
                                                                        \
        Z80_WR_BYTE(pZ80->MemPtr.W, pZ80->TempByte);     \
                                                                        \
        Reg = pZ80->TempByte;                                               \
                                                                        \
}

#define INDEX_CB_SET_REG(OrMask, IndexReg, Reg) \
{                                                               \
        /*Z80_BYTE      Data;*/                         \
                                                                \
		SETUP_INDEXED_ADDRESS(pZ80,IndexReg); \
        pZ80->TempByte = Z80_RD_BYTE(pZ80->MemPtr.W);   \
                                                                \
        SET(OrMask, pZ80->TempByte);                        \
                                                                \
        Z80_WR_BYTE(pZ80->MemPtr.W, pZ80->TempByte);     \
                                                               \
        Reg = pZ80->TempByte;                                               \
}

#define INDEX_CB_RES_REG(AndMask,IndexReg, Reg) \
{                                                               \
        /*Z80_BYTE Data;*/                              \
                                                                \
		SETUP_INDEXED_ADDRESS(pZ80,IndexReg); \
        pZ80->TempByte = Z80_RD_BYTE(pZ80->MemPtr.W);   \
                                                                \
        RES(AndMask, pZ80->TempByte);                       \
                                                                \
        Z80_WR_BYTE(pZ80->MemPtr.W, pZ80->TempByte);     \
        Reg = pZ80->TempByte;                                               \
}
#endif

#if 0
#define PrefixIgnore()		\
	pZ80->Flags &= ~Z80_CHECK_INTERRUPT_FLAG;		\
	pZ80->PC.W.l++;	/*pZ80->PC.W++;*/									\
	INC_REFRESH(1);								\
	Z80_UpdateCycles(1)
#endif


/*---------------------------------------------------------*/
/* pop a word from the stack */



/*---------------------------------------------------------*/
/* put a word on the stack */




/* swap two words */
#define SWAP(Reg1,Reg2) \
{                                               \
        Z80_WORD        tempR;  \
                                                \
        tempR = Reg1;           \
        Reg1 = Reg2;            \
        Reg2 = tempR;           \
}
