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
#ifndef __Z80_HEADER_INCLUDED__
#define __Z80_HEADER_INCLUDED__

/* When int handler is called, ints are effectively disabled!!! */

/* Flag definitions and how to calculate flags:

  SIGN:

  8-bit values: Bit 7 state.
  16-bit values: Bit 16 state.

  PARITY:

  Odd: Sum of all binary digits is odd.
  Even: Sum of all binary digits is even.

  ZERO:

  8-bit values: If all 8-bits are 0.
  16-bit values: If all 16-bits are 0.

  OVERFLOW:

  This refers to numbers in signed arithmetic.

  0x000..0x07f are positive numbers,
  0x080..0x0ff are negative numbers.

  Addition:


  +ve   +  +ve  yielding +ve  = no overflow.
  +ve   +  +ve  yielding -ve  = overflow.
  +ve   +  -ve  yielding +ve  = no overflow.
  +ve   +  -ve  yielding -ve  = no overflow.
  -ve   +  +ve  yielding +ve  = no overflow.
  -ve   +  +ve  yielding -ve  = no overflow
  -ve   +  -ve  yielding +ve  = overflow.
  -ve   +  -ve  yielding -ve  = no overflow.


  Subtraction:

  check
  +ve   -  +ve  yielding +ve  = no overflow.
  +ve   -  +ve  yielding -ve  = no overflow.
  +ve   -  -ve  yielding +ve  = no overflow.
  +ve   -  -ve  yielding -ve  = overflow.
  -ve   -  +ve  yielding +ve  = overflow.
  -ve   -  +ve  yielding -ve  = no overflow.
  -ve   -  -ve  yielding +ve  = no overflow.
  -ve   -  -ve  yielding -ve  = no overflow.


  CARRY:

  Addition:

  7f + 7f       = no carry
  7f + 80       = no carry
  80 + 7f       = no carry
  80 + 80       = carry

  Subtraction:

  7f - 7f       = no carry
  7f - 80   = carry
  80 - 7f   = no carry
  80 - 80   = no carry


#define SET_CARRY_FLAG_SUB(Result)      \
{                                                                               \
Z80_FLAGS_REG = Z80_FLAGS_REG & (0x0ff^Z80_CARRY_FLAG); \
Z80_FLAGS_REG = Z80_FLAGS_REG | ((Result>>8) & 0x01); \
}

*/


/*
Flags Register (F)

Bit 7: Sign Flag
Bit 6: Zero Flag
Bit 5: Unused
Bit 4: Half Carry Flag
Bit 3: Unused
Bit 2: Parity/Overflow Flag
Bit 1: Subtract
Bit 0: Carry
*/


#include "../cpcglob.h"
#include <stdint.h>

/**/

/**/
#define Z80_ZERO_FLAG_BIT                       6
#define Z80_HALFCARRY_FLAG_BIT          4
#define Z80_PARITY_FLAG_BIT                     2

#define Z80_SIGN_FLAG                           0x080
#define Z80_ZERO_FLAG                           0x040
#define Z80_UNUSED_FLAG1                        0x020
#define Z80_HALFCARRY_FLAG                      0x010
#define Z80_UNUSED_FLAG2                        0x008

#define Z80_PARITY_FLAG                         0x004
#define Z80_OVERFLOW_FLAG                       0x004

#define Z80_SUBTRACT_FLAG                       0x002
#define Z80_CARRY_FLAG                          0x001

#define Z80_OUTPUT_HALT (1<<0)
#define Z80_OUTPUT_IORQ (1<<1)
#define Z80_OUTPUT_M1 (1<<2)
#define Z80_OUTPUT_MREQ (1<<3)
#define Z80_OUTPUT_RD (1<<4)
#define Z80_OUTPUT_WR (1<<5)
#define Z80_OUTPUT_RFSH (1<<6)



/* size defines */
//typedef unsigned char  Z80_BYTE;
//typedef unsigned short	Z80_WORD;
//typedef signed char        Z80_BYTE_OFFSET;
//typedef signed short   Z80_WORD_OFFSET;
//typedef unsigned long   Z80_LONG;
typedef uint8_t Z80_BYTE;
typedef uint16_t Z80_WORD;
typedef int8_t Z80_BYTE_OFFSET;
typedef int16_t Z80_WORD_OFFSET;
typedef uint32_t Z80_LONG;

#ifndef NULL
#define NULL 0
#endif

#ifndef FALSE
#define FALSE (1==0)
#endif

#ifndef TRUE
#define TRUE (1==1)
#endif

#ifdef CPC_LSB_FIRST
/* register pair definition */
typedef union
{
                /* read as a word */
				Z80_WORD W;
                /* read as seperate bytes, l for low, h for high bytes */
                struct
                {
                        Z80_BYTE l;
                        Z80_BYTE h;
				} B;
} Z80_REGISTER_PAIR;
#else
/* register pair definition */
typedef union
{
                /* read as a word */
                Z80_WORD W;

                /* read as seperate bytes, l for low, h for high bytes */
                struct
                {
                        Z80_BYTE h;
                        Z80_BYTE l;
                } B;
} Z80_REGISTER_PAIR;
#endif

#ifdef CPC_LSB_FIRST
typedef union
{
	Z80_LONG	L;

	struct
	{
		Z80_WORD	l;
		Z80_WORD	h;
	} W;

	struct
	{
		Z80_BYTE	l;
		Z80_BYTE	h;
		Z80_BYTE	pad[2];
	} B;
} Z80_REGISTER_LONG;
#else
typedef union
{
	Z80_LONG	L;

	struct
	{
		Z80_WORD	l;
		Z80_WORD	h;
	} W;

	struct
	{
		Z80_BYTE	pad[2];
		Z80_BYTE	h;
		Z80_BYTE	l;
	} B;
} Z80_REGISTER_LONG;
#endif

/* structure holds all register data */
typedef struct  _Z80_REGISTERS
{
	unsigned long			Flags;
	unsigned long			Outputs;

	Z80_REGISTER_LONG       PC;
        Z80_REGISTER_PAIR       AF;
        Z80_REGISTER_PAIR       HL;
        Z80_REGISTER_PAIR       SP;
		Z80_REGISTER_PAIR       DE;
        Z80_REGISTER_PAIR       BC;
		Z80_REGISTER_PAIR		MemPtr;

		Z80_REGISTER_PAIR       IX;
        Z80_REGISTER_PAIR       IY;

        Z80_REGISTER_PAIR       altHL;
        Z80_REGISTER_PAIR       altDE;
        Z80_REGISTER_PAIR       altBC;
        Z80_REGISTER_PAIR       altAF;
		Z80_WORD                IOPort;

        /* interrupt vector register. High byte of address */
        Z80_BYTE                I;

        /* refresh register */
        Z80_BYTE                R;

        /* interrupt status */
        Z80_BYTE                IFF1;
        Z80_BYTE                IFF2;

        /* bit 7 of R register */
        Z80_BYTE                RBit7;

        /* interrupt mode 0,1,2 */
        Z80_BYTE                IM;
        Z80_BYTE                TempByte;
   Z80_BYTE				InterruptVectorBase;
   Z80_BYTE                IOData;
   

} Z80_REGISTERS;

#define Z80_GET_R   (pZ80->RBit7 | (pZ80->R & 0x07f))

/* external defined functions */

/* read a byte of data from memory */
Z80_BYTE        Z80_RD_DATA(const Z80_WORD Addr);

/* write a byte to memory */
void            Z80_WR_DATA(const Z80_WORD Addr, const Z80_BYTE Data);

Z80_WORD        Z80_RD_MEM_WORD(const Z80_WORD Addr);

void            Z80_WR_MEM_WORD(const Z80_WORD Addr, const Z80_WORD Data);

Z80_BYTE		Z80_RD_BYTE_IM0(void);
Z80_WORD		Z80_RD_WORD_IM0(void);

void Z80_DebugOpcodeTriggered(void);

void			Z80_ExecuteCycles(int);

/* write a byte to a I/O port */
void            Z80_DoOut(const Z80_WORD Addr, const Z80_BYTE Data);

/* read a byte from a I/O port */
Z80_BYTE        Z80_DoIn(const Z80_WORD Addr);

Z80_WORD Z80_GetIOPort(Z80_REGISTERS *pZ80);
Z80_BYTE Z80_GetIOData(Z80_REGISTERS *pZ80);

/*--------*/





#define Z80_FLAGS_REG                       pZ80->AF.B.l

#define Z80_TEST_CARRY_SET                       ((Z80_FLAGS_REG & Z80_CARRY_FLAG)!=0)
#define Z80_TEST_CARRY_NOT_SET           (Z80_FLAGS_REG & Z80_CARRY_FLAG)==0
#define Z80_TEST_ZERO_SET                        (Z80_FLAGS_REG & Z80_ZERO_FLAG)!=0
#define Z80_TEST_ZERO_NOT_SET            (Z80_FLAGS_REG & Z80_ZERO_FLAG)==0
#define Z80_TEST_MINUS                           (Z80_FLAGS_REG & Z80_SIGN_FLAG)!=0
#define Z80_TEST_POSITIVE                        (Z80_FLAGS_REG & Z80_SIGN_FLAG)==0

/* parity even. bit = 1, parity odd, bit = 0 */
#define Z80_TEST_PARITY_EVEN                 ((Z80_FLAGS_REG & Z80_PARITY_FLAG)!=0)
#define Z80_TEST_PARITY_ODD                  ((Z80_FLAGS_REG & Z80_PARITY_FLAG)==0)

#define Z80_KEEP_UNUSED_FLAGS       (Z80_UNUSED1_FLAG | Z80_UNUSED2_FLAG)


void    Z80_Init(Z80_REGISTERS *pZ80);
void    Z80_RestartPower(Z80_REGISTERS *pZ80);
void    Z80_RestartReset(Z80_REGISTERS *pZ80);
int    Z80_ExecuteInterrupt(Z80_REGISTERS *pZ80);


int Z80_ExecuteNMI(Z80_REGISTERS *pZ80);

void    Z80_Reti(Z80_REGISTERS *pZ80);


/* set the interrupt vector - used by IM 0 and IM 2 */
void    Z80_SetInterruptVector(Z80_REGISTERS *pZ80,int);
int     Z80_GetInterruptVector(Z80_REGISTERS *pZ80);


/* Z80 executes this to acknowledge a requested interrupt */
void	Z80_AcknowledgeInterrupt(Z80_REGISTERS *pZ80);
void	Z80_AcknowledgeNMI(Z80_REGISTERS *pZ80);

void    Z80_SetInterruptInput(Z80_REGISTERS *pZ80,BOOL bState);
BOOL	Z80_GetInterruptRequest(Z80_REGISTERS *pZ80);

void    Z80_SetNMIInterruptInput(Z80_REGISTERS *pZ80,BOOL bState);
BOOL    Z80_GetNMIInterruptInput(Z80_REGISTERS *pZ80);
BOOL    Z80_GetNMIInterruptRequest(Z80_REGISTERS *pZ80);


void Z80_RefreshInterruptRequest(Z80_REGISTERS *pZ80);

int Z80_Execute(Z80_REGISTERS *pZ80);
int Z80_GetOutputs(Z80_REGISTERS *pZ80);

#define Z80_NO_CHECK_INTERRUPT_FLAG			0x0001  /* z80 can check interrupt at the end of this instruction */
#define Z80_EXECUTE_INTERRUPT_FLAG	        0x0002 /* z80 should execute interrupt handler */
#define Z80_EXECUTING_HALT_FLAG				0x0004  /* z80 is executing a HALT instruction */
#define Z80_MASKABLE_INTERRUPT_INPUT_FLAG					0x0008 /* maskable interrupt state/maskable interrupt state */
#define Z80_INTERRUPT_NO_DELAY_FLAG           0x0010 /* z80 should apply 1 microsecond wait state */
#define Z80_INTERRUPT_NMI_FLAG              0x0020 /* Non-maskable interrupt triggered */
#define Z80_INTERRUPT_NMI_INPUT_STATE_FLAG              0x0040 /* Non-maskable interrupt signal state*/
#define Z80_INTERRUPT_CHECK               0x0080 /* either maskable or nmi */
/* for debugging */
enum
{
	Z80_PC,
	Z80_SP,
	Z80_IX,
	Z80_IY,
	Z80_I,
	Z80_R,
	Z80_AF,
	Z80_BC,
	Z80_DE,
	Z80_HL,
	Z80_IM,
	Z80_AF2,
	Z80_BC2,
	Z80_DE2,
	Z80_HL2,
	Z80_F,
	Z80_H,
	Z80_L,
	Z80_D,
	Z80_E,
	Z80_B,
	Z80_C,
	Z80_A,
	Z80_IFF1,
	Z80_IFF2,
	Z80_MEMPTR
} /*Z80_REG_ID */;

int Z80_GetReg(Z80_REGISTERS *pZ80,int RegID);
void Z80_SetReg(Z80_REGISTERS *pZ80,int RegID, int Value);

int	Z80_ExecuteInstruction(Z80_REGISTERS *pZ80);
int Z80_ExecuteIM0(Z80_REGISTERS *pZ80);

#endif
