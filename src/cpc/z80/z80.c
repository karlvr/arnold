/*
 *  Arnold emulator (c) Copyright, Kevin Thacker 1995-2011
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
#ifndef INKZ80
#include "z80.h"
#include "z80tables.h"
#include "z80macros.h"
#include "z80funcs.h"
#include "z80funcs2.h"
/***************************************************************************/
 static int Z80_Index_CB_ExecuteInstruction(Z80_REGISTERS *pZ80)
{
unsigned long Opcode;
unsigned long Cycles;
Opcode = Z80_RD_OPCODE_BYTE(pZ80);
Opcode = Opcode & 0x0ff;
        pZ80->R+=2;
 pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W);
switch (Opcode)
{
case 0x040:
case 0x041:
case 0x042:
case 0x043:
case 0x044:
case 0x045:
case 0x046:
case 0x047:
{
/* BIT 0,(INDEX+d) */

{
	Z80_BYTE	Flags;						
	const Z80_BYTE	Mask = (1<<0);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 
    pZ80->TempByte = pZ80->TempByte & Mask; 
	Flags |= pZ80->MemPtr.B.h & ((1<<5) | (1<<3)); 
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 6;
}
break;
case 0x048:
case 0x049:
case 0x04a:
case 0x04b:
case 0x04c:
case 0x04d:
case 0x04e:
case 0x04f:
{
/* BIT 1,(INDEX+d) */

{
	Z80_BYTE	Flags;						
	const Z80_BYTE	Mask = (1<<1);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 
    pZ80->TempByte = pZ80->TempByte & Mask; 
	Flags |= pZ80->MemPtr.B.h & ((1<<5) | (1<<3)); 
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 6;
}
break;
case 0x050:
case 0x051:
case 0x052:
case 0x053:
case 0x054:
case 0x055:
case 0x056:
case 0x057:
{
/* BIT 2,(INDEX+d) */

{
	Z80_BYTE	Flags;						
	const Z80_BYTE	Mask = (1<<2);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 
    pZ80->TempByte = pZ80->TempByte & Mask; 
	Flags |= pZ80->MemPtr.B.h & ((1<<5) | (1<<3)); 
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 6;
}
break;
case 0x058:
case 0x059:
case 0x05a:
case 0x05b:
case 0x05c:
case 0x05d:
case 0x05e:
case 0x05f:
{
/* BIT 3,(INDEX+d) */

{
	Z80_BYTE	Flags;						
	const Z80_BYTE	Mask = (1<<3);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 
    pZ80->TempByte = pZ80->TempByte & Mask; 
	Flags |= pZ80->MemPtr.B.h & ((1<<5) | (1<<3)); 
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 6;
}
break;
case 0x060:
case 0x061:
case 0x062:
case 0x063:
case 0x064:
case 0x065:
case 0x066:
case 0x067:
{
/* BIT 4,(INDEX+d) */

{
	Z80_BYTE	Flags;						
	const Z80_BYTE	Mask = (1<<4);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 
    pZ80->TempByte = pZ80->TempByte & Mask; 
	Flags |= pZ80->MemPtr.B.h & ((1<<5) | (1<<3)); 
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 6;
}
break;
case 0x068:
case 0x069:
case 0x06a:
case 0x06b:
case 0x06c:
case 0x06d:
case 0x06e:
case 0x06f:
{
/* BIT 5,(INDEX+d) */

{
	Z80_BYTE	Flags;						
	const Z80_BYTE	Mask = (1<<5);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 
    pZ80->TempByte = pZ80->TempByte & Mask; 
	Flags |= pZ80->MemPtr.B.h & ((1<<5) | (1<<3)); 
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 6;
}
break;
case 0x070:
case 0x071:
case 0x072:
case 0x073:
case 0x074:
case 0x075:
case 0x076:
case 0x077:
{
/* BIT 6,(INDEX+d) */

{
	Z80_BYTE	Flags;						
	const Z80_BYTE	Mask = (1<<6);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 
    pZ80->TempByte = pZ80->TempByte & Mask; 
	Flags |= pZ80->MemPtr.B.h & ((1<<5) | (1<<3)); 
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 6;
}
break;
case 0x078:
case 0x079:
case 0x07a:
case 0x07b:
case 0x07c:
case 0x07d:
case 0x07e:
case 0x07f:
{
/* BIT 7,(INDEX+d) */

{
	Z80_BYTE	Flags;						
	const Z80_BYTE	Mask = (1<<7);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 
    pZ80->TempByte = pZ80->TempByte & Mask; 
	Flags |= pZ80->MemPtr.B.h & ((1<<5) | (1<<3)); 
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 6;
}
break;
case 0x000:
{
/* LD B,RLC (INDEX+d) */
         
{                                                                       
        RLC_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->BC.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x001:
{
/* LD C,RLC (INDEX+d) */
         
{                                                                       
        RLC_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->BC.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x002:
{
/* LD D,RLC (INDEX+d) */
         
{                                                                       
        RLC_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->DE.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x003:
{
/* LD E,RLC (INDEX+d) */
         
{                                                                       
        RLC_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->DE.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x004:
{
/* LD H,RLC (INDEX+d) */
         
{                                                                       
        RLC_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->HL.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x005:
{
/* LD L,RLC (INDEX+d) */
         
{                                                                       
        RLC_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->HL.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x006:
{
/* RLC (INDEX+d) */

{                                                                       
        RLC_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);             
} Cycles = 7;
}
break;
case 0x007:
{
/* LD A,RLC (INDEX+d) */
         
{                                                                       
        RLC_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->AF.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x008:
{
/* LD B,RRC (INDEX+d) */
         
{                                                                       
        RRC_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->BC.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x009:
{
/* LD C,RRC (INDEX+d) */
         
{                                                                       
        RRC_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->BC.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x00a:
{
/* LD D,RRC (INDEX+d) */
         
{                                                                       
        RRC_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->DE.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x00b:
{
/* LD E,RRC (INDEX+d) */
         
{                                                                       
        RRC_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->DE.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x00c:
{
/* LD H,RRC (INDEX+d) */
         
{                                                                       
        RRC_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->HL.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x00d:
{
/* LD L,RRC (INDEX+d) */
         
{                                                                       
        RRC_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->HL.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x00e:
{
/* RRC (INDEX+d) */

{                                                                       
        RRC_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);             
}Cycles = 7;
}
break;
case 0x00f:
{
/* LD A,RRC (INDEX+d) */
         
{                                                                       
        RRC_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->AF.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x010:
{
/* LD B,RL (INDEX+d) */
          
{                                                                       
        RL_WITH_FLAGS(pZ80->TempByte);                      
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->BC.B.h = pZ80->TempByte;                                               
                                                                        
}Cycles = 7;
}
break;
case 0x011:
{
/* LD C,RL (INDEX+d) */
          
{                                                                       
        RL_WITH_FLAGS(pZ80->TempByte);                      
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->BC.B.l = pZ80->TempByte;                                               
                                                                        
}Cycles = 7;
}
break;
case 0x012:
{
/* LD D,RL (INDEX+d) */
          
{                                                                       
        RL_WITH_FLAGS(pZ80->TempByte);                      
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->DE.B.h = pZ80->TempByte;                                               
                                                                        
}Cycles = 7;
}
break;
case 0x013:
{
/* LD E,RL (INDEX+d) */
          
{                                                                       
        RL_WITH_FLAGS(pZ80->TempByte);                      
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->DE.B.l = pZ80->TempByte;                                               
                                                                        
}Cycles = 7;
}
break;
case 0x014:
{
/* LD H,RL (INDEX+d) */
          
{                                                                       
        RL_WITH_FLAGS(pZ80->TempByte);                      
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->HL.B.h = pZ80->TempByte;                                               
                                                                        
}Cycles = 7;
}
break;
case 0x015:
{
/* LD L,RL (INDEX+d) */
          
{                                                                       
        RL_WITH_FLAGS(pZ80->TempByte);                      
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->HL.B.l = pZ80->TempByte;                                               
                                                                        
}Cycles = 7;
}
break;
case 0x016:
{
/* RL (INDEX+d) */

{                                                                       
        RL_WITH_FLAGS(pZ80->TempByte);                                              
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);             
}Cycles = 7;
}
break;
case 0x017:
{
/* LD A,RL (INDEX+d) */
          
{                                                                       
        RL_WITH_FLAGS(pZ80->TempByte);                      
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->AF.B.h = pZ80->TempByte;                                               
                                                                        
}Cycles = 7;
}
break;
case 0x018:
{
/* LD B,RR (INDEX+d) */
          
{                                                                       
        RR_WITH_FLAGS(pZ80->TempByte);                      
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->BC.B.h = pZ80->TempByte;                                               
                                                                        
}Cycles = 7;
}
break;
case 0x019:
{
/* LD C,RR (INDEX+d) */
          
{                                                                       
        RR_WITH_FLAGS(pZ80->TempByte);                      
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->BC.B.l = pZ80->TempByte;                                               
                                                                        
}Cycles = 7;
}
break;
case 0x01a:
{
/* LD D,RR (INDEX+d) */
          
{                                                                       
        RR_WITH_FLAGS(pZ80->TempByte);                      
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->DE.B.h = pZ80->TempByte;                                               
                                                                        
}Cycles = 7;
}
break;
case 0x01b:
{
/* LD E,RR (INDEX+d) */
          
{                                                                       
        RR_WITH_FLAGS(pZ80->TempByte);                      
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->DE.B.l = pZ80->TempByte;                                               
                                                                        
}Cycles = 7;
}
break;
case 0x01c:
{
/* LD H,RR (INDEX+d) */
          
{                                                                       
        RR_WITH_FLAGS(pZ80->TempByte);                      
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->HL.B.h = pZ80->TempByte;                                               
                                                                        
}Cycles = 7;
}
break;
case 0x01d:
{
/* LD L,RR (INDEX+d) */
          
{                                                                       
        RR_WITH_FLAGS(pZ80->TempByte);                      
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->HL.B.l = pZ80->TempByte;                                               
                                                                        
}Cycles = 7;
}
break;
case 0x01e:
{
/* RR (INDEX+d) */


{                                                                       
        RR_WITH_FLAGS(pZ80->TempByte);                                              
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);             
}Cycles = 7;
}
break;
case 0x01f:
{
/* LD A,RR (INDEX+d) */
          
{                                                                       
        RR_WITH_FLAGS(pZ80->TempByte);                      
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->AF.B.h = pZ80->TempByte;                                               
                                                                        
}Cycles = 7;
}
break;
case 0x020:
{
/* LD B,SLA (INDEX+d) */

{                                                                       
        SLA_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->BC.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x021:
{
/* LD C,SLA (INDEX+d) */

{                                                                       
        SLA_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->BC.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x022:
{
/* LD D,SLA (INDEX+d) */

{                                                                       
        SLA_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->DE.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x023:
{
/* LD E,SLA (INDEX+d) */

{                                                                       
        SLA_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->DE.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x024:
{
/* LD H,SLA (INDEX+d) */

{                                                                       
        SLA_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->HL.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x025:
{
/* LD L,SLA (INDEX+d) */

{                                                                       
        SLA_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->HL.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x026:
{
/* SLA (INDEX+d) */

{                                                                       
        SLA_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
} Cycles = 7;
}
break;
case 0x027:
{
/* LD A,SLA (INDEX+d) */

{                                                                       
        SLA_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->AF.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x028:
{
/* LD B,SRA (INDEX+d) */
         
{                                                                       
        SRA_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->BC.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x029:
{
/* LD C,SRA (INDEX+d) */
         
{                                                                       
        SRA_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->BC.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x02a:
{
/* LD D,SRA (INDEX+d) */
         
{                                                                       
        SRA_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->DE.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x02b:
{
/* LD E,SRA (INDEX+d) */
         
{                                                                       
        SRA_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->DE.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x02c:
{
/* LD H,SRA (INDEX+d) */
         
{                                                                       
        SRA_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->HL.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x02d:
{
/* LD L,SRA (INDEX+d) */
         
{                                                                       
        SRA_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->HL.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x02e:
{
/* SRA (INDEX+d) */

{                                                                       
        SRA_WITH_FLAGS(pZ80->TempByte);                                             
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);                
}Cycles = 7;
}
break;
case 0x02f:
{
/* LD A,SRA (INDEX+d) */
         
{                                                                       
        SRA_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->AF.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x030:
{
/* LD B,SLL (INDEX+d) */
         
{                                                                       
        SLL_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->BC.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x031:
{
/* LD C,SLL (INDEX+d) */
         
{                                                                       
        SLL_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->BC.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x032:
{
/* LD D,SLL (INDEX+d) */
         
{                                                                       
        SLL_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->DE.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x033:
{
/* LD E,SLL (INDEX+d) */
         
{                                                                       
        SLL_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->DE.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x034:
{
/* LD H,SLL (INDEX+d) */
         
{                                                                       
        SLL_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->HL.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x035:
{
/* LD L,SLL (INDEX+d) */
         
{                                                                       
        SLL_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->HL.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x036:
{
/* SLL (INDEX+d) */

{                                                                       
        SLL_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
}Cycles = 7;
}
break;
case 0x037:
{
/* LD A,SLL (INDEX+d) */
         
{                                                                       
        SLL_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->AF.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x038:
{
/* LD B,SRL (INDEX+d) */
         
{                                                                       
        SRL_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->BC.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x039:
{
/* LD C,SRL (INDEX+d) */
         
{                                                                       
        SRL_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->BC.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x03a:
{
/* LD D,SRL (INDEX+d) */
         
{                                                                       
        SRL_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->DE.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x03b:
{
/* LD E,SRL (INDEX+d) */
         
{                                                                       
        SRL_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->DE.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x03c:
{
/* LD H,SRL (INDEX+d) */
         
{                                                                       
        SRL_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->HL.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x03d:
{
/* LD L,SRL (INDEX+d) */
         
{                                                                       
        SRL_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->HL.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x03e:
{
/* SRL (INDEX+d) */

{                                                                       
        SRL_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);               
}Cycles = 7;
}
break;
case 0x03f:
{
/* LD A,SRL (INDEX+d) */
         
{                                                                       
        SRL_WITH_FLAGS(pZ80->TempByte);                     
                                                                        
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                                        
        pZ80->AF.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x080:
{
/* LD B,RES 0, (INDEX+d) */ 
{                                                               
        RES((1<<0), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->BC.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x081:
{
/* LD C,RES 0, (INDEX+d) */ 
{                                                               
        RES((1<<0), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->BC.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x082:
{
/* LD D,RES 0, (INDEX+d) */ 
{                                                               
        RES((1<<0), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->DE.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x083:
{
/* LD E,RES 0, (INDEX+d) */ 
{                                                               
        RES((1<<0), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->DE.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x084:
{
/* LD H,RES 0, (INDEX+d) */ 
{                                                               
        RES((1<<0), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->HL.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x085:
{
/* LD L,RES 0, (INDEX+d) */ 
{                                                               
        RES((1<<0), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->HL.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x086:
{
/* RES 0, (INDEX+d) */
{                                                               
        RES((1<<0), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
}Cycles = 7;
}
break;
case 0x087:
{
/* LD A,RES 0, (INDEX+d) */ 
{                                                               
        RES((1<<0), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->AF.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x088:
{
/* LD B,RES 1, (INDEX+d) */ 
{                                                               
        RES((1<<1), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->BC.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x089:
{
/* LD C,RES 1, (INDEX+d) */ 
{                                                               
        RES((1<<1), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->BC.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x08a:
{
/* LD D,RES 1, (INDEX+d) */ 
{                                                               
        RES((1<<1), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->DE.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x08b:
{
/* LD E,RES 1, (INDEX+d) */ 
{                                                               
        RES((1<<1), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->DE.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x08c:
{
/* LD H,RES 1, (INDEX+d) */ 
{                                                               
        RES((1<<1), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->HL.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x08d:
{
/* LD L,RES 1, (INDEX+d) */ 
{                                                               
        RES((1<<1), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->HL.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x08e:
{
/* RES 1, (INDEX+d) */
{                                                               
        RES((1<<1), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
}Cycles = 7;
}
break;
case 0x08f:
{
/* LD A,RES 1, (INDEX+d) */ 
{                                                               
        RES((1<<1), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->AF.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x090:
{
/* LD B,RES 2, (INDEX+d) */ 
{                                                               
        RES((1<<2), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->BC.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x091:
{
/* LD C,RES 2, (INDEX+d) */ 
{                                                               
        RES((1<<2), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->BC.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x092:
{
/* LD D,RES 2, (INDEX+d) */ 
{                                                               
        RES((1<<2), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->DE.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x093:
{
/* LD E,RES 2, (INDEX+d) */ 
{                                                               
        RES((1<<2), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->DE.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x094:
{
/* LD H,RES 2, (INDEX+d) */ 
{                                                               
        RES((1<<2), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->HL.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x095:
{
/* LD L,RES 2, (INDEX+d) */ 
{                                                               
        RES((1<<2), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->HL.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x096:
{
/* RES 2, (INDEX+d) */
{                                                               
        RES((1<<2), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
}Cycles = 7;
}
break;
case 0x097:
{
/* LD A,RES 2, (INDEX+d) */ 
{                                                               
        RES((1<<2), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->AF.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x098:
{
/* LD B,RES 3, (INDEX+d) */ 
{                                                               
        RES((1<<3), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->BC.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x099:
{
/* LD C,RES 3, (INDEX+d) */ 
{                                                               
        RES((1<<3), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->BC.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x09a:
{
/* LD D,RES 3, (INDEX+d) */ 
{                                                               
        RES((1<<3), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->DE.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x09b:
{
/* LD E,RES 3, (INDEX+d) */ 
{                                                               
        RES((1<<3), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->DE.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x09c:
{
/* LD H,RES 3, (INDEX+d) */ 
{                                                               
        RES((1<<3), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->HL.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x09d:
{
/* LD L,RES 3, (INDEX+d) */ 
{                                                               
        RES((1<<3), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->HL.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x09e:
{
/* RES 3, (INDEX+d) */
{                                                               
        RES((1<<3), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
}Cycles = 7;
}
break;
case 0x09f:
{
/* LD A,RES 3, (INDEX+d) */ 
{                                                               
        RES((1<<3), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->AF.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0a0:
{
/* LD B,RES 4, (INDEX+d) */ 
{                                                               
        RES((1<<4), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->BC.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0a1:
{
/* LD C,RES 4, (INDEX+d) */ 
{                                                               
        RES((1<<4), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->BC.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0a2:
{
/* LD D,RES 4, (INDEX+d) */ 
{                                                               
        RES((1<<4), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->DE.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0a3:
{
/* LD E,RES 4, (INDEX+d) */ 
{                                                               
        RES((1<<4), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->DE.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0a4:
{
/* LD H,RES 4, (INDEX+d) */ 
{                                                               
        RES((1<<4), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->HL.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0a5:
{
/* LD L,RES 4, (INDEX+d) */ 
{                                                               
        RES((1<<4), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->HL.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0a6:
{
/* RES 4, (INDEX+d) */
{                                                               
        RES((1<<4), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
}Cycles = 7;
}
break;
case 0x0a7:
{
/* LD A,RES 4, (INDEX+d) */ 
{                                                               
        RES((1<<4), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->AF.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0a8:
{
/* LD B,RES 5, (INDEX+d) */ 
{                                                               
        RES((1<<5), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->BC.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0a9:
{
/* LD C,RES 5, (INDEX+d) */ 
{                                                               
        RES((1<<5), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->BC.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0aa:
{
/* LD D,RES 5, (INDEX+d) */ 
{                                                               
        RES((1<<5), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->DE.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0ab:
{
/* LD E,RES 5, (INDEX+d) */ 
{                                                               
        RES((1<<5), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->DE.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0ac:
{
/* LD H,RES 5, (INDEX+d) */ 
{                                                               
        RES((1<<5), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->HL.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0ad:
{
/* LD L,RES 5, (INDEX+d) */ 
{                                                               
        RES((1<<5), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->HL.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0ae:
{
/* RES 5, (INDEX+d) */
{                                                               
        RES((1<<5), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
}Cycles = 7;
}
break;
case 0x0af:
{
/* LD A,RES 5, (INDEX+d) */ 
{                                                               
        RES((1<<5), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->AF.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0b0:
{
/* LD B,RES 6, (INDEX+d) */ 
{                                                               
        RES((1<<6), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->BC.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0b1:
{
/* LD C,RES 6, (INDEX+d) */ 
{                                                               
        RES((1<<6), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->BC.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0b2:
{
/* LD D,RES 6, (INDEX+d) */ 
{                                                               
        RES((1<<6), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->DE.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0b3:
{
/* LD E,RES 6, (INDEX+d) */ 
{                                                               
        RES((1<<6), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->DE.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0b4:
{
/* LD H,RES 6, (INDEX+d) */ 
{                                                               
        RES((1<<6), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->HL.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0b5:
{
/* LD L,RES 6, (INDEX+d) */ 
{                                                               
        RES((1<<6), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->HL.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0b6:
{
/* RES 6, (INDEX+d) */
{                                                               
        RES((1<<6), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
}Cycles = 7;
}
break;
case 0x0b7:
{
/* LD A,RES 6, (INDEX+d) */ 
{                                                               
        RES((1<<6), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->AF.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0b8:
{
/* LD B,RES 7, (INDEX+d) */ 
{                                                               
        RES((1<<7), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->BC.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0b9:
{
/* LD C,RES 7, (INDEX+d) */ 
{                                                               
        RES((1<<7), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->BC.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0ba:
{
/* LD D,RES 7, (INDEX+d) */ 
{                                                               
        RES((1<<7), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->DE.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0bb:
{
/* LD E,RES 7, (INDEX+d) */ 
{                                                               
        RES((1<<7), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->DE.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0bc:
{
/* LD H,RES 7, (INDEX+d) */ 
{                                                               
        RES((1<<7), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->HL.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0bd:
{
/* LD L,RES 7, (INDEX+d) */ 
{                                                               
        RES((1<<7), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->HL.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0be:
{
/* RES 7, (INDEX+d) */
{                                                               
        RES((1<<7), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
}Cycles = 7;
}
break;
case 0x0bf:
{
/* LD A,RES 7, (INDEX+d) */ 
{                                                               
        RES((1<<7), pZ80->TempByte);                       
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
        pZ80->AF.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0c0:
{
/* LD B, SET 0, (INDEX+d) */ 
{                                                               
        SET((1<<0), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->BC.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0c1:
{
/* LD C, SET 0, (INDEX+d) */ 
{                                                               
        SET((1<<0), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->BC.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0c2:
{
/* LD D, SET 0, (INDEX+d) */ 
{                                                               
        SET((1<<0), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->DE.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0c3:
{
/* LD E, SET 0, (INDEX+d) */ 
{                                                               
        SET((1<<0), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->DE.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0c4:
{
/* LD H, SET 0, (INDEX+d) */ 
{                                                               
        SET((1<<0), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->HL.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0c5:
{
/* LD L, SET 0, (INDEX+d) */ 
{                                                               
        SET((1<<0), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->HL.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0c6:
{
/* RES 0, (INDEX+d) */
{                                                               
        SET((1<<0), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
}Cycles = 7;
}
break;
case 0x0c7:
{
/* LD A, SET 0, (INDEX+d) */ 
{                                                               
        SET((1<<0), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->AF.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0c8:
{
/* LD B, SET 1, (INDEX+d) */ 
{                                                               
        SET((1<<1), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->BC.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0c9:
{
/* LD C, SET 1, (INDEX+d) */ 
{                                                               
        SET((1<<1), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->BC.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0ca:
{
/* LD D, SET 1, (INDEX+d) */ 
{                                                               
        SET((1<<1), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->DE.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0cb:
{
/* LD E, SET 1, (INDEX+d) */ 
{                                                               
        SET((1<<1), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->DE.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0cc:
{
/* LD H, SET 1, (INDEX+d) */ 
{                                                               
        SET((1<<1), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->HL.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0cd:
{
/* LD L, SET 1, (INDEX+d) */ 
{                                                               
        SET((1<<1), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->HL.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0ce:
{
/* RES 1, (INDEX+d) */
{                                                               
        SET((1<<1), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
}Cycles = 7;
}
break;
case 0x0cf:
{
/* LD A, SET 1, (INDEX+d) */ 
{                                                               
        SET((1<<1), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->AF.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0d0:
{
/* LD B, SET 2, (INDEX+d) */ 
{                                                               
        SET((1<<2), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->BC.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0d1:
{
/* LD C, SET 2, (INDEX+d) */ 
{                                                               
        SET((1<<2), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->BC.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0d2:
{
/* LD D, SET 2, (INDEX+d) */ 
{                                                               
        SET((1<<2), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->DE.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0d3:
{
/* LD E, SET 2, (INDEX+d) */ 
{                                                               
        SET((1<<2), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->DE.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0d4:
{
/* LD H, SET 2, (INDEX+d) */ 
{                                                               
        SET((1<<2), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->HL.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0d5:
{
/* LD L, SET 2, (INDEX+d) */ 
{                                                               
        SET((1<<2), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->HL.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0d6:
{
/* RES 2, (INDEX+d) */
{                                                               
        SET((1<<2), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
}Cycles = 7;
}
break;
case 0x0d7:
{
/* LD A, SET 2, (INDEX+d) */ 
{                                                               
        SET((1<<2), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->AF.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0d8:
{
/* LD B, SET 3, (INDEX+d) */ 
{                                                               
        SET((1<<3), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->BC.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0d9:
{
/* LD C, SET 3, (INDEX+d) */ 
{                                                               
        SET((1<<3), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->BC.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0da:
{
/* LD D, SET 3, (INDEX+d) */ 
{                                                               
        SET((1<<3), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->DE.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0db:
{
/* LD E, SET 3, (INDEX+d) */ 
{                                                               
        SET((1<<3), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->DE.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0dc:
{
/* LD H, SET 3, (INDEX+d) */ 
{                                                               
        SET((1<<3), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->HL.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0dd:
{
/* LD L, SET 3, (INDEX+d) */ 
{                                                               
        SET((1<<3), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->HL.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0de:
{
/* RES 3, (INDEX+d) */
{                                                               
        SET((1<<3), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
}Cycles = 7;
}
break;
case 0x0df:
{
/* LD A, SET 3, (INDEX+d) */ 
{                                                               
        SET((1<<3), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->AF.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0e0:
{
/* LD B, SET 4, (INDEX+d) */ 
{                                                               
        SET((1<<4), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->BC.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0e1:
{
/* LD C, SET 4, (INDEX+d) */ 
{                                                               
        SET((1<<4), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->BC.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0e2:
{
/* LD D, SET 4, (INDEX+d) */ 
{                                                               
        SET((1<<4), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->DE.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0e3:
{
/* LD E, SET 4, (INDEX+d) */ 
{                                                               
        SET((1<<4), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->DE.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0e4:
{
/* LD H, SET 4, (INDEX+d) */ 
{                                                               
        SET((1<<4), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->HL.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0e5:
{
/* LD L, SET 4, (INDEX+d) */ 
{                                                               
        SET((1<<4), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->HL.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0e6:
{
/* RES 4, (INDEX+d) */
{                                                               
        SET((1<<4), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
}Cycles = 7;
}
break;
case 0x0e7:
{
/* LD A, SET 4, (INDEX+d) */ 
{                                                               
        SET((1<<4), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->AF.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0e8:
{
/* LD B, SET 5, (INDEX+d) */ 
{                                                               
        SET((1<<5), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->BC.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0e9:
{
/* LD C, SET 5, (INDEX+d) */ 
{                                                               
        SET((1<<5), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->BC.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0ea:
{
/* LD D, SET 5, (INDEX+d) */ 
{                                                               
        SET((1<<5), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->DE.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0eb:
{
/* LD E, SET 5, (INDEX+d) */ 
{                                                               
        SET((1<<5), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->DE.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0ec:
{
/* LD H, SET 5, (INDEX+d) */ 
{                                                               
        SET((1<<5), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->HL.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0ed:
{
/* LD L, SET 5, (INDEX+d) */ 
{                                                               
        SET((1<<5), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->HL.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0ee:
{
/* RES 5, (INDEX+d) */
{                                                               
        SET((1<<5), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
}Cycles = 7;
}
break;
case 0x0ef:
{
/* LD A, SET 5, (INDEX+d) */ 
{                                                               
        SET((1<<5), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->AF.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0f0:
{
/* LD B, SET 6, (INDEX+d) */ 
{                                                               
        SET((1<<6), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->BC.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0f1:
{
/* LD C, SET 6, (INDEX+d) */ 
{                                                               
        SET((1<<6), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->BC.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0f2:
{
/* LD D, SET 6, (INDEX+d) */ 
{                                                               
        SET((1<<6), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->DE.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0f3:
{
/* LD E, SET 6, (INDEX+d) */ 
{                                                               
        SET((1<<6), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->DE.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0f4:
{
/* LD H, SET 6, (INDEX+d) */ 
{                                                               
        SET((1<<6), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->HL.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0f5:
{
/* LD L, SET 6, (INDEX+d) */ 
{                                                               
        SET((1<<6), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->HL.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0f6:
{
/* RES 6, (INDEX+d) */
{                                                               
        SET((1<<6), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
}Cycles = 7;
}
break;
case 0x0f7:
{
/* LD A, SET 6, (INDEX+d) */ 
{                                                               
        SET((1<<6), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->AF.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0f8:
{
/* LD B, SET 7, (INDEX+d) */ 
{                                                               
        SET((1<<7), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->BC.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0f9:
{
/* LD C, SET 7, (INDEX+d) */ 
{                                                               
        SET((1<<7), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->BC.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0fa:
{
/* LD D, SET 7, (INDEX+d) */ 
{                                                               
        SET((1<<7), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->DE.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0fb:
{
/* LD E, SET 7, (INDEX+d) */ 
{                                                               
        SET((1<<7), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->DE.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0fc:
{
/* LD H, SET 7, (INDEX+d) */ 
{                                                               
        SET((1<<7), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->HL.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0fd:
{
/* LD L, SET 7, (INDEX+d) */ 
{                                                               
        SET((1<<7), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->HL.B.l = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
case 0x0fe:
{
/* RES 7, (INDEX+d) */
{                                                               
        SET((1<<7), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
}Cycles = 7;
}
break;
case 0x0ff:
{
/* LD A, SET 7, (INDEX+d) */ 
{                                                               
        SET((1<<7), pZ80->TempByte);                        
                                                                
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);     
                                                               
        pZ80->AF.B.h = pZ80->TempByte;                                               
}Cycles = 7;
}
break;
default:
/* the following tells MSDEV 6 to not generate */
/* code which checks if a input value to the  */
/* switch is not valid.*/
#ifdef _MSC_VER
#if (_MSC_VER>=1200)
	Cycles=0;
	__assume(0);
#endif
#else
	Cycles=0;
#endif
break;
}
return Cycles;
}
/***************************************************************************/
 static int Z80_FD_ExecuteInstruction(Z80_REGISTERS *pZ80)
{
unsigned long Opcode;
unsigned long Cycles;
Opcode = Z80_RD_OPCODE_BYTE(pZ80);
Opcode = Opcode & 0x0ff;
switch (Opcode)
{
case 0x000:
case 0x001:
case 0x002:
case 0x003:
case 0x004:
case 0x005:
case 0x006:
case 0x007:
case 0x008:
case 0x00a:
case 0x00b:
case 0x00c:
case 0x00d:
case 0x00e:
case 0x00f:
case 0x010:
case 0x011:
case 0x012:
case 0x013:
case 0x014:
case 0x015:
case 0x016:
case 0x017:
case 0x018:
case 0x01a:
case 0x01b:
case 0x01c:
case 0x01d:
case 0x01e:
case 0x01f:
case 0x020:
case 0x027:
case 0x028:
case 0x02f:
case 0x030:
case 0x031:
case 0x032:
case 0x033:
case 0x037:
case 0x038:
case 0x03a:
case 0x03b:
case 0x03c:
case 0x03d:
case 0x03e:
case 0x03f:
case 0x040:
case 0x041:
case 0x042:
case 0x043:
case 0x047:
case 0x048:
case 0x049:
case 0x04a:
case 0x04b:
case 0x04f:
case 0x050:
case 0x051:
case 0x052:
case 0x053:
case 0x057:
case 0x058:
case 0x059:
case 0x05a:
case 0x05b:
case 0x05f:
case 0x076:
case 0x078:
case 0x079:
case 0x07a:
case 0x07b:
case 0x07f:
case 0x080:
case 0x081:
case 0x082:
case 0x083:
case 0x087:
case 0x088:
case 0x089:
case 0x08a:
case 0x08b:
case 0x08f:
case 0x090:
case 0x091:
case 0x092:
case 0x093:
case 0x097:
case 0x098:
case 0x099:
case 0x09a:
case 0x09b:
case 0x09f:
case 0x0a0:
case 0x0a1:
case 0x0a2:
case 0x0a3:
case 0x0a7:
case 0x0a8:
case 0x0a9:
case 0x0aa:
case 0x0ab:
case 0x0af:
case 0x0b0:
case 0x0b1:
case 0x0b2:
case 0x0b3:
case 0x0b7:
case 0x0b8:
case 0x0b9:
case 0x0ba:
case 0x0bb:
case 0x0bf:
case 0x0c0:
case 0x0c1:
case 0x0c2:
case 0x0c3:
case 0x0c4:
case 0x0c5:
case 0x0c6:
case 0x0c7:
case 0x0c8:
case 0x0c9:
case 0x0ca:
case 0x0cc:
case 0x0cd:
case 0x0ce:
case 0x0cf:
case 0x0d0:
case 0x0d1:
case 0x0d2:
case 0x0d3:
case 0x0d4:
case 0x0d5:
case 0x0d6:
case 0x0d7:
case 0x0d8:
case 0x0d9:
case 0x0da:
case 0x0db:
case 0x0dc:
case 0x0dd:
case 0x0de:
case 0x0df:
case 0x0e0:
case 0x0e2:
case 0x0e4:
case 0x0e6:
case 0x0e7:
case 0x0e8:
case 0x0ea:
case 0x0eb:
case 0x0ec:
case 0x0ed:
case 0x0ee:
case 0x0ef:
case 0x0f0:
case 0x0f1:
case 0x0f2:
case 0x0f3:
case 0x0f4:
case 0x0f5:
case 0x0f6:
case 0x0f7:
case 0x0f8:
case 0x0fa:
case 0x0fb:
case 0x0fc:
case 0x0fd:
case 0x0fe:
case 0x0ff:
{
pZ80->PC.W.l--;
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x009:
{
ADD_RR_rr(pZ80->IY.W,pZ80->BC.W);
        pZ80->R+=2;
 Cycles = 4;
}
break;
case 0x019:
{
ADD_RR_rr(pZ80->IY.W,pZ80->DE.W);
        pZ80->R+=2;
 Cycles = 4;
}
break;
case 0x021:
{
/* LD IY,nnnn */
 
        pZ80->IY.W = Z80_RD_PC_WORD(pZ80); 
        pZ80->R+=2;
 Cycles = 4;
}
break;
case 0x022:
{
/* LD (nnnn),IY */
 
        pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);	      
		Z80_WR_WORD(pZ80,pZ80->MemPtr.W,pZ80->IY.W);    
		++pZ80->MemPtr.W;	
        pZ80->R+=2;
 Cycles = 6;
}
break;
case 0x023:
{
/* INC IY */
 
    ++pZ80->IY.W;                
        pZ80->R+=2;
 Cycles = 3;
}
break;
case 0x024:
{
INC_R(pZ80->IY.B.h);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x025:
{
DEC_R(pZ80->IY.B.h);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x026:
{
/* LD HIY, n */
 
        pZ80->IY.B.h = Z80_RD_PC_BYTE(pZ80); 
        pZ80->R+=2;
 Cycles = 3;
}
break;
case 0x029:
{
ADD_RR_rr(pZ80->IY.W,pZ80->IY.W);
        pZ80->R+=2;
 Cycles = 4;
}
break;
case 0x02a:
{
/* LD IY,(nnnn) */
 
        pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);	
        pZ80->IY.W = Z80_RD_WORD(pZ80,pZ80->MemPtr.W);              
		++pZ80->MemPtr.W;	
        pZ80->R+=2;
 Cycles = 6;
}
break;
case 0x02b:
{
/* DEC IY */
 
	--pZ80->IY.W;                
        pZ80->R+=2;
 Cycles = 3;
}
break;
case 0x02c:
{
INC_R(pZ80->IY.B.l);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x02d:
{
DEC_R(pZ80->IY.B.l);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x02e:
{
/* LD LIY, n */
 
        pZ80->IY.B.l = Z80_RD_PC_BYTE(pZ80); 
        pZ80->R+=2;
 Cycles = 3;
}
break;
case 0x034:
{
_INC_INDEX_(pZ80->IY.W);
        pZ80->R+=2;
 Cycles = 6;
}
break;
case 0x035:
{
_DEC_INDEX_(pZ80->IY.W);
        pZ80->R+=2;
 Cycles = 6;
}
break;
case 0x036:
{
/* LD (IY+d),n */
         SETUP_INDEXED_ADDRESS(pZ80,pZ80->IY.W); 
        pZ80->TempByte = Z80_RD_PC_BYTE(pZ80);	
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);                                
        pZ80->R+=2;
 Cycles = 6;
}
break;
case 0x039:
{
ADD_RR_rr(pZ80->IY.W,pZ80->SP.W);
        pZ80->R+=2;
 Cycles = 4;
}
break;
case 0x044:
{
/* LD B,hIY */
 
		pZ80->BC.B.h = pZ80->IY.B.h; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x045:
{
/* LD B,lIY */
 
		pZ80->BC.B.h = pZ80->IY.B.l; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x046:
{
/* LD B,(IY+D) */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IY.W);	
		pZ80->BC.B.h = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W);   
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x04c:
{
/* LD C,hIY */
 
		pZ80->BC.B.l = pZ80->IY.B.h; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x04d:
{
/* LD C,lIY */
 
		pZ80->BC.B.l = pZ80->IY.B.l; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x04e:
{
/* LD C,(IY+D) */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IY.W);	
		pZ80->BC.B.l = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W);   
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x054:
{
/* LD D,hIY */
 
		pZ80->DE.B.h = pZ80->IY.B.h; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x055:
{
/* LD D,lIY */
 
		pZ80->DE.B.h = pZ80->IY.B.l; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x056:
{
/* LD D,(IY+D) */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IY.W);	
		pZ80->DE.B.h = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W);   
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x05c:
{
/* LD E,hIY */
 
		pZ80->DE.B.l = pZ80->IY.B.h; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x05d:
{
/* LD E,lIY */
 
		pZ80->DE.B.l = pZ80->IY.B.l; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x05e:
{
/* LD E,(IY+D) */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IY.W);	
		pZ80->DE.B.l = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W);   
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x060:
{
/* LD hIY,B */
 
		pZ80->IY.B.h = pZ80->BC.B.h; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x061:
{
/* LD hIY,C */
 
		pZ80->IY.B.h = pZ80->BC.B.l; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x062:
{
/* LD hIY,D */
 
		pZ80->IY.B.h = pZ80->DE.B.h; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x063:
{
/* LD hIY,E */
 
		pZ80->IY.B.h = pZ80->DE.B.l; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x064:
{
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x065:
{
/* LD hIY,lIY */
 
		pZ80->IY.B.h = pZ80->IY.B.l; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x066:
{
/* LD H,(IY+D) */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IY.W);	
		pZ80->HL.B.h = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W);   
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x067:
{
/* LD hIY,A */
 
		pZ80->IY.B.h = pZ80->AF.B.h; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x068:
{
/* LD lIY,B */
 
		pZ80->IY.B.l = pZ80->BC.B.h; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x069:
{
/* LD lIY,C */
 
		pZ80->IY.B.l = pZ80->BC.B.l; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x06a:
{
/* LD lIY,D */
 
		pZ80->IY.B.l = pZ80->DE.B.h; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x06b:
{
/* LD lIY,E */
 
		pZ80->IY.B.l = pZ80->DE.B.l; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x06c:
{
/* LD lIY,hIY */
 
		pZ80->IY.B.l = pZ80->IY.B.h; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x06d:
{
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x06e:
{
/* LD L,(IY+D) */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IY.W);	
		pZ80->HL.B.l = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W);   
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x06f:
{
/* LD lIY,A */
 
		pZ80->IY.B.l = pZ80->AF.B.h; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x070:
{
/* LD (IY+D),B */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IY.W);	
		Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->BC.B.h); 
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x071:
{
/* LD (IY+D),C */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IY.W);	
		Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->BC.B.l); 
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x072:
{
/* LD (IY+D),D */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IY.W);	
		Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->DE.B.h); 
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x073:
{
/* LD (IY+D),E */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IY.W);	
		Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->DE.B.l); 
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x074:
{
/* LD (IY+D),H */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IY.W);	
		Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->HL.B.h); 
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x075:
{
/* LD (IY+D),L */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IY.W);	
		Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->HL.B.l); 
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x077:
{
/* LD (IY+D),A */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IY.W);	
		Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->AF.B.h); 
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x07c:
{
/* LD A,hIY */
 
		pZ80->AF.B.h = pZ80->IY.B.h; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x07d:
{
/* LD A,lIY */
 
		pZ80->AF.B.h = pZ80->IY.B.l; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x07e:
{
/* LD A,(IY+D) */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IY.W);	
		pZ80->AF.B.h = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W);   
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x084:
{
ADD_A_R(pZ80->IY.B.h);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x085:
{
ADD_A_R(pZ80->IY.B.l);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x086:
{
ADD_A_INDEX(pZ80->IY.W);
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x08c:
{
ADC_A_R(pZ80->IY.B.h);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x08d:
{
ADC_A_R(pZ80->IY.B.l);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x08e:
{
ADC_A_INDEX(pZ80->IY.W);
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x094:
{
SUB_A_R(pZ80->IY.B.h);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x095:
{
SUB_A_R(pZ80->IY.B.l);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x096:
{
SUB_A_INDEX(pZ80->IY.W);
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x09c:
{
SBC_A_R(pZ80->IY.B.h);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x09d:
{
SBC_A_R(pZ80->IY.B.l);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x09e:
{
SBC_A_INDEX(pZ80->IY.W);
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x0a4:
{
AND_A_R(pZ80->IY.B.h);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x0a5:
{
AND_A_R(pZ80->IY.B.l);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x0a6:
{
AND_A_INDEX(pZ80->IY.W);
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x0ac:
{
XOR_A_R(pZ80->IY.B.h);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x0ad:
{
XOR_A_R(pZ80->IY.B.l);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x0ae:
{
XOR_A_INDEX(pZ80->IY.W);
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x0b4:
{
OR_A_R(pZ80->IY.B.h);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x0b5:
{
OR_A_R(pZ80->IY.B.l);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x0b6:
{
OR_A_INDEX(pZ80->IY.W);
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x0bc:
{
CP_A_R(pZ80->IY.B.h);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x0bd:
{
CP_A_R(pZ80->IY.B.l);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x0be:
{
CP_A_INDEX(pZ80->IY.W);
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x0cb:
{
SETUP_INDEXED_ADDRESS(pZ80,pZ80->IY.W);
Cycles = Z80_Index_CB_ExecuteInstruction(pZ80);
}
break;
case 0x0e1:
{
/* POP IY */
pZ80->IY.W = Z80_POP_WORD(pZ80);
        pZ80->R+=2;
 Cycles = 4;
}
break;
case 0x0e3:
{
/* EX (SP),IY */
 
        pZ80->MemPtr.W = Z80_RD_WORD(pZ80,pZ80->SP.W); 
        Z80_WR_WORD(pZ80,pZ80->SP.W, pZ80->IY.W);    
        pZ80->IY.W = pZ80->MemPtr.W; 
        pZ80->R+=2;
 Cycles = 7;
}
break;
case 0x0e5:
{
/* PUSH IY */
Z80_PUSH_WORD(pZ80, pZ80->IY.W);
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x0e9:
{
/* JP (IY) */

    pZ80->PC.W.l=pZ80->IY.W; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x0f9:
{
/* LD SP,IY */

    pZ80->SP.W=pZ80->IY.W; 
        pZ80->R+=2;
 Cycles = 3;
}
break;
default:
/* the following tells MSDEV 6 to not generate */
/* code which checks if a input value to the  */
/* switch is not valid.*/
#ifdef _MSC_VER
#if (_MSC_VER>=1200)
	Cycles=0;
	__assume(0);
#endif
#else
	Cycles=0;
#endif
break;
}
return Cycles;
}
/***************************************************************************/
 static int Z80_DD_ExecuteInstruction(Z80_REGISTERS *pZ80)
{
unsigned long Opcode;
unsigned long Cycles;
Opcode = Z80_RD_OPCODE_BYTE(pZ80);
Opcode = Opcode & 0x0ff;
switch (Opcode)
{
case 0x000:
case 0x001:
case 0x002:
case 0x003:
case 0x004:
case 0x005:
case 0x006:
case 0x007:
case 0x008:
case 0x00a:
case 0x00b:
case 0x00c:
case 0x00d:
case 0x00e:
case 0x00f:
case 0x010:
case 0x011:
case 0x012:
case 0x013:
case 0x014:
case 0x015:
case 0x016:
case 0x017:
case 0x018:
case 0x01a:
case 0x01b:
case 0x01c:
case 0x01d:
case 0x01e:
case 0x01f:
case 0x020:
case 0x027:
case 0x028:
case 0x02f:
case 0x030:
case 0x031:
case 0x032:
case 0x033:
case 0x037:
case 0x038:
case 0x03a:
case 0x03b:
case 0x03c:
case 0x03d:
case 0x03e:
case 0x03f:
case 0x040:
case 0x041:
case 0x042:
case 0x043:
case 0x047:
case 0x048:
case 0x049:
case 0x04a:
case 0x04b:
case 0x04f:
case 0x050:
case 0x051:
case 0x052:
case 0x053:
case 0x057:
case 0x058:
case 0x059:
case 0x05a:
case 0x05b:
case 0x05f:
case 0x076:
case 0x078:
case 0x079:
case 0x07a:
case 0x07b:
case 0x07f:
case 0x080:
case 0x081:
case 0x082:
case 0x083:
case 0x087:
case 0x088:
case 0x089:
case 0x08a:
case 0x08b:
case 0x08f:
case 0x090:
case 0x091:
case 0x092:
case 0x093:
case 0x097:
case 0x098:
case 0x099:
case 0x09a:
case 0x09b:
case 0x09f:
case 0x0a0:
case 0x0a1:
case 0x0a2:
case 0x0a3:
case 0x0a7:
case 0x0a8:
case 0x0a9:
case 0x0aa:
case 0x0ab:
case 0x0af:
case 0x0b0:
case 0x0b1:
case 0x0b2:
case 0x0b3:
case 0x0b7:
case 0x0b8:
case 0x0b9:
case 0x0ba:
case 0x0bb:
case 0x0bf:
case 0x0c0:
case 0x0c1:
case 0x0c2:
case 0x0c3:
case 0x0c4:
case 0x0c5:
case 0x0c6:
case 0x0c7:
case 0x0c8:
case 0x0c9:
case 0x0ca:
case 0x0cc:
case 0x0cd:
case 0x0ce:
case 0x0cf:
case 0x0d0:
case 0x0d1:
case 0x0d2:
case 0x0d3:
case 0x0d4:
case 0x0d5:
case 0x0d6:
case 0x0d7:
case 0x0d8:
case 0x0d9:
case 0x0da:
case 0x0db:
case 0x0dc:
case 0x0dd:
case 0x0de:
case 0x0df:
case 0x0e0:
case 0x0e2:
case 0x0e4:
case 0x0e6:
case 0x0e7:
case 0x0e8:
case 0x0ea:
case 0x0eb:
case 0x0ec:
case 0x0ed:
case 0x0ee:
case 0x0ef:
case 0x0f0:
case 0x0f1:
case 0x0f2:
case 0x0f3:
case 0x0f4:
case 0x0f5:
case 0x0f6:
case 0x0f7:
case 0x0f8:
case 0x0fa:
case 0x0fb:
case 0x0fc:
case 0x0fd:
case 0x0fe:
case 0x0ff:
{
pZ80->PC.W.l--;
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x009:
{
ADD_RR_rr(pZ80->IX.W,pZ80->BC.W);
        pZ80->R+=2;
 Cycles = 4;
}
break;
case 0x019:
{
ADD_RR_rr(pZ80->IX.W,pZ80->DE.W);
        pZ80->R+=2;
 Cycles = 4;
}
break;
case 0x021:
{
/* LD IX,nnnn */
 
        pZ80->IX.W = Z80_RD_PC_WORD(pZ80); 
        pZ80->R+=2;
 Cycles = 4;
}
break;
case 0x022:
{
/* LD (nnnn),IX */
 
        pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);	      
		Z80_WR_WORD(pZ80,pZ80->MemPtr.W,pZ80->IX.W);    
		++pZ80->MemPtr.W;	
        pZ80->R+=2;
 Cycles = 6;
}
break;
case 0x023:
{
/* INC IX */
 
    ++pZ80->IX.W;                
        pZ80->R+=2;
 Cycles = 3;
}
break;
case 0x024:
{
INC_R(pZ80->IX.B.h);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x025:
{
DEC_R(pZ80->IX.B.h);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x026:
{
/* LD HIX, n */
 
        pZ80->IX.B.h = Z80_RD_PC_BYTE(pZ80); 
        pZ80->R+=2;
 Cycles = 3;
}
break;
case 0x029:
{
ADD_RR_rr(pZ80->IX.W,pZ80->IX.W);
        pZ80->R+=2;
 Cycles = 4;
}
break;
case 0x02a:
{
/* LD IX,(nnnn) */
 
        pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);	
        pZ80->IX.W = Z80_RD_WORD(pZ80,pZ80->MemPtr.W);              
		++pZ80->MemPtr.W;	
        pZ80->R+=2;
 Cycles = 6;
}
break;
case 0x02b:
{
/* DEC IX */
 
	--pZ80->IX.W;                
        pZ80->R+=2;
 Cycles = 3;
}
break;
case 0x02c:
{
INC_R(pZ80->IX.B.l);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x02d:
{
DEC_R(pZ80->IX.B.l);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x02e:
{
/* LD LIX, n */
 
        pZ80->IX.B.l = Z80_RD_PC_BYTE(pZ80); 
        pZ80->R+=2;
 Cycles = 3;
}
break;
case 0x034:
{
_INC_INDEX_(pZ80->IX.W);
        pZ80->R+=2;
 Cycles = 6;
}
break;
case 0x035:
{
_DEC_INDEX_(pZ80->IX.W);
        pZ80->R+=2;
 Cycles = 6;
}
break;
case 0x036:
{
/* LD (IX+d),n */
         SETUP_INDEXED_ADDRESS(pZ80,pZ80->IX.W); 
        pZ80->TempByte = Z80_RD_PC_BYTE(pZ80);	
        Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->TempByte);                                
        pZ80->R+=2;
 Cycles = 6;
}
break;
case 0x039:
{
ADD_RR_rr(pZ80->IX.W,pZ80->SP.W);
        pZ80->R+=2;
 Cycles = 4;
}
break;
case 0x044:
{
/* LD B,hIX */
 
		pZ80->BC.B.h = pZ80->IX.B.h; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x045:
{
/* LD B,lIX */
 
		pZ80->BC.B.h = pZ80->IX.B.l; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x046:
{
/* LD B,(IX+D) */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IX.W);	
		pZ80->BC.B.h = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W);   
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x04c:
{
/* LD C,hIX */
 
		pZ80->BC.B.l = pZ80->IX.B.h; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x04d:
{
/* LD C,lIX */
 
		pZ80->BC.B.l = pZ80->IX.B.l; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x04e:
{
/* LD C,(IX+D) */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IX.W);	
		pZ80->BC.B.l = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W);   
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x054:
{
/* LD D,hIX */
 
		pZ80->DE.B.h = pZ80->IX.B.h; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x055:
{
/* LD D,lIX */
 
		pZ80->DE.B.h = pZ80->IX.B.l; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x056:
{
/* LD D,(IX+D) */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IX.W);	
		pZ80->DE.B.h = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W);   
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x05c:
{
/* LD E,hIX */
 
		pZ80->DE.B.l = pZ80->IX.B.h; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x05d:
{
/* LD E,lIX */
 
		pZ80->DE.B.l = pZ80->IX.B.l; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x05e:
{
/* LD E,(IX+D) */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IX.W);	
		pZ80->DE.B.l = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W);   
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x060:
{
/* LD hIX,B */
 
		pZ80->IX.B.h = pZ80->BC.B.h; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x061:
{
/* LD hIX,C */
 
		pZ80->IX.B.h = pZ80->BC.B.l; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x062:
{
/* LD hIX,D */
 
		pZ80->IX.B.h = pZ80->DE.B.h; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x063:
{
/* LD hIX,E */
 
		pZ80->IX.B.h = pZ80->DE.B.l; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x064:
{
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x065:
{
/* LD hIX,lIX */
 
		pZ80->IX.B.h = pZ80->IX.B.l; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x066:
{
/* LD H,(IX+D) */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IX.W);	
		pZ80->HL.B.h = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W);   
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x067:
{
/* LD hIX,A */
 
		pZ80->IX.B.h = pZ80->AF.B.h; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x068:
{
/* LD lIX,B */
 
		pZ80->IX.B.l = pZ80->BC.B.h; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x069:
{
/* LD lIX,C */
 
		pZ80->IX.B.l = pZ80->BC.B.l; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x06a:
{
/* LD lIX,D */
 
		pZ80->IX.B.l = pZ80->DE.B.h; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x06b:
{
/* LD lIX,E */
 
		pZ80->IX.B.l = pZ80->DE.B.l; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x06c:
{
/* LD lIX,hIX */
 
		pZ80->IX.B.l = pZ80->IX.B.h; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x06d:
{
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x06e:
{
/* LD L,(IX+D) */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IX.W);	
		pZ80->HL.B.l = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W);   
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x06f:
{
/* LD lIX,A */
 
		pZ80->IX.B.l = pZ80->AF.B.h; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x070:
{
/* LD (IX+D),B */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IX.W);	
		Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->BC.B.h); 
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x071:
{
/* LD (IX+D),C */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IX.W);	
		Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->BC.B.l); 
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x072:
{
/* LD (IX+D),D */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IX.W);	
		Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->DE.B.h); 
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x073:
{
/* LD (IX+D),E */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IX.W);	
		Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->DE.B.l); 
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x074:
{
/* LD (IX+D),H */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IX.W);	
		Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->HL.B.h); 
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x075:
{
/* LD (IX+D),L */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IX.W);	
		Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->HL.B.l); 
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x077:
{
/* LD (IX+D),A */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IX.W);	
		Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->AF.B.h); 
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x07c:
{
/* LD A,hIX */
 
		pZ80->AF.B.h = pZ80->IX.B.h; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x07d:
{
/* LD A,lIX */
 
		pZ80->AF.B.h = pZ80->IX.B.l; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x07e:
{
/* LD A,(IX+D) */
 		SETUP_INDEXED_ADDRESS(pZ80,pZ80->IX.W);	
		pZ80->AF.B.h = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W);   
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x084:
{
ADD_A_R(pZ80->IX.B.h);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x085:
{
ADD_A_R(pZ80->IX.B.l);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x086:
{
ADD_A_INDEX(pZ80->IX.W);
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x08c:
{
ADC_A_R(pZ80->IX.B.h);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x08d:
{
ADC_A_R(pZ80->IX.B.l);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x08e:
{
ADC_A_INDEX(pZ80->IX.W);
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x094:
{
SUB_A_R(pZ80->IX.B.h);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x095:
{
SUB_A_R(pZ80->IX.B.l);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x096:
{
SUB_A_INDEX(pZ80->IX.W);
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x09c:
{
SBC_A_R(pZ80->IX.B.h);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x09d:
{
SBC_A_R(pZ80->IX.B.l);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x09e:
{
SBC_A_INDEX(pZ80->IX.W);
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x0a4:
{
AND_A_R(pZ80->IX.B.h);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x0a5:
{
AND_A_R(pZ80->IX.B.l);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x0a6:
{
AND_A_INDEX(pZ80->IX.W);
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x0ac:
{
XOR_A_R(pZ80->IX.B.h);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x0ad:
{
XOR_A_R(pZ80->IX.B.l);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x0ae:
{
XOR_A_INDEX(pZ80->IX.W);
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x0b4:
{
OR_A_R(pZ80->IX.B.h);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x0b5:
{
OR_A_R(pZ80->IX.B.l);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x0b6:
{
OR_A_INDEX(pZ80->IX.W);
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x0bc:
{
CP_A_R(pZ80->IX.B.h);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x0bd:
{
CP_A_R(pZ80->IX.B.l);
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x0be:
{
CP_A_INDEX(pZ80->IX.W);
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x0cb:
{
SETUP_INDEXED_ADDRESS(pZ80,pZ80->IX.W);
Cycles = Z80_Index_CB_ExecuteInstruction(pZ80);
}
break;
case 0x0e1:
{
/* POP IX */
pZ80->IX.W = Z80_POP_WORD(pZ80);
        pZ80->R+=2;
 Cycles = 4;
}
break;
case 0x0e3:
{
/* EX (SP),IX */
 
        pZ80->MemPtr.W = Z80_RD_WORD(pZ80,pZ80->SP.W); 
        Z80_WR_WORD(pZ80,pZ80->SP.W, pZ80->IX.W);    
        pZ80->IX.W = pZ80->MemPtr.W; 
        pZ80->R+=2;
 Cycles = 7;
}
break;
case 0x0e5:
{
/* PUSH IX */
Z80_PUSH_WORD(pZ80, pZ80->IX.W);
        pZ80->R+=2;
 Cycles = 5;
}
break;
case 0x0e9:
{
/* JP (IX) */

    pZ80->PC.W.l=pZ80->IX.W; 
        pZ80->R+=2;
 Cycles = 2;
}
break;
case 0x0f9:
{
/* LD SP,IX */

    pZ80->SP.W=pZ80->IX.W; 
        pZ80->R+=2;
 Cycles = 3;
}
break;
default:
/* the following tells MSDEV 6 to not generate */
/* code which checks if a input value to the  */
/* switch is not valid.*/
#ifdef _MSC_VER
#if (_MSC_VER>=1200)
	Cycles=0;
	__assume(0);
#endif
#else
	Cycles=0;
#endif
break;
}
return Cycles;
}
/***************************************************************************/
 static int Z80_ED_ExecuteInstruction(Z80_REGISTERS *pZ80)
{
unsigned long Opcode;
unsigned long Cycles;
        pZ80->R+=2;
 Opcode = Z80_RD_OPCODE_BYTE(pZ80);
Opcode = Opcode & 0x0ff;
switch (Opcode)
{
case 0x000:
case 0x001:
case 0x002:
case 0x003:
case 0x004:
case 0x005:
case 0x006:
case 0x007:
case 0x008:
case 0x009:
case 0x00a:
case 0x00b:
case 0x00c:
case 0x00d:
case 0x00e:
case 0x00f:
case 0x010:
case 0x011:
case 0x012:
case 0x013:
case 0x014:
case 0x015:
case 0x016:
case 0x017:
case 0x018:
case 0x019:
case 0x01a:
case 0x01b:
case 0x01c:
case 0x01d:
case 0x01e:
case 0x01f:
case 0x020:
case 0x021:
case 0x022:
case 0x023:
case 0x024:
case 0x025:
case 0x026:
case 0x027:
case 0x028:
case 0x029:
case 0x02a:
case 0x02b:
case 0x02c:
case 0x02d:
case 0x02e:
case 0x02f:
case 0x030:
case 0x031:
case 0x032:
case 0x033:
case 0x034:
case 0x035:
case 0x036:
case 0x037:
case 0x038:
case 0x039:
case 0x03a:
case 0x03b:
case 0x03c:
case 0x03d:
case 0x03e:
case 0x03f:
case 0x080:
case 0x081:
case 0x082:
case 0x083:
case 0x084:
case 0x085:
case 0x086:
case 0x087:
case 0x088:
case 0x089:
case 0x08a:
case 0x08b:
case 0x08c:
case 0x08d:
case 0x08e:
case 0x08f:
case 0x090:
case 0x091:
case 0x092:
case 0x093:
case 0x094:
case 0x095:
case 0x096:
case 0x097:
case 0x098:
case 0x099:
case 0x09a:
case 0x09b:
case 0x09c:
case 0x09d:
case 0x09e:
case 0x09f:
case 0x0a4:
case 0x0a5:
case 0x0a6:
case 0x0a7:
case 0x0ac:
case 0x0ad:
case 0x0ae:
case 0x0af:
case 0x0b4:
case 0x0b5:
case 0x0b6:
case 0x0b7:
case 0x0bc:
case 0x0bd:
case 0x0be:
case 0x0bf:
case 0x0c0:
case 0x0c1:
case 0x0c2:
case 0x0c3:
case 0x0c4:
case 0x0c5:
case 0x0c6:
case 0x0c7:
case 0x0c8:
case 0x0c9:
case 0x0ca:
case 0x0cb:
case 0x0cc:
case 0x0cd:
case 0x0ce:
case 0x0cf:
case 0x0d0:
case 0x0d1:
case 0x0d2:
case 0x0d3:
case 0x0d4:
case 0x0d5:
case 0x0d6:
case 0x0d7:
case 0x0d8:
case 0x0d9:
case 0x0da:
case 0x0db:
case 0x0dc:
case 0x0dd:
case 0x0de:
case 0x0df:
case 0x0e0:
case 0x0e1:
case 0x0e2:
case 0x0e3:
case 0x0e4:
case 0x0e5:
case 0x0e6:
case 0x0e7:
case 0x0e8:
case 0x0e9:
case 0x0ea:
case 0x0eb:
case 0x0ec:
case 0x0ed:
case 0x0ee:
case 0x0ef:
case 0x0f0:
case 0x0f1:
case 0x0f2:
case 0x0f3:
case 0x0f4:
case 0x0f5:
case 0x0f6:
case 0x0f7:
case 0x0f8:
case 0x0f9:
case 0x0fa:
case 0x0fb:
case 0x0fc:
case 0x0fd:
case 0x0fe:
{
Cycles = 2;
}
break;
case 0x040:
{
/* IN B,(C) */
 		pZ80->MemPtr.W = pZ80->BC.W; 
		pZ80->IOPort = pZ80->MemPtr.W;
		pZ80->Outputs = Z80_OUTPUT_IORQ | Z80_OUTPUT_RD;
		pZ80->BC.B.h = pZ80->IOData = Z80_DoIn(pZ80->IOPort);            
		++pZ80->MemPtr.W; 
		pZ80->Outputs = 0;
		{											
			Z80_BYTE	Flags;						
			Flags = Z80_FLAGS_REG;						
			Flags = Flags & Z80_CARRY_FLAG;			
			Flags |= ZeroSignParityTable[pZ80->BC.B.h];	
            Flags |= pZ80->BC.B.h & ((1<<5) | (1<<3));   
			Z80_FLAGS_REG = Flags;						
		}											
	Cycles = 4; 
}
break;
case 0x041:
{
 /* OUT (C),B */
 
	pZ80->IOData = pZ80->BC.B.h;
	pZ80->IOPort = pZ80->MemPtr.W = pZ80->BC.W; 
	pZ80->Outputs = Z80_OUTPUT_IORQ | Z80_OUTPUT_WR;
	Z80_DoOut(pZ80->IOPort,pZ80->IOData);                     
	pZ80->Outputs = 0; 
	++pZ80->MemPtr.W;                                       
	Cycles = 4;
}
break;
case 0x042:
{
SBC_HL_rr(pZ80->BC.W);
Cycles = 4;
}
break;
case 0x043:
{
/* LD (nnnn),BC */
 
        /* read destination address into memptr */ 
        pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);	
        /* write register to address */ 
        Z80_WR_WORD(pZ80,pZ80->MemPtr.W, pZ80->BC.W);    
        ++pZ80->MemPtr.W; 
Cycles = 6;
}
break;
case 0x044:
{
/* NEG */

	Z80_BYTE	Flags;	
	Z80_BYTE	AReg; 
						
	AReg = pZ80->AF.B.h;		
    Flags = Z80_SUBTRACT_FLAG;	
													
    if (AReg == 0x080)									
    {												
          Flags |= Z80_PARITY_FLAG;					
    }												
													
    if (AReg != 0x000)									
    {												
        Flags |= Z80_CARRY_FLAG;					
    }												
													
	if ((AReg & 0x0f)!=0)								
	{												
		Flags |= Z80_HALFCARRY_FLAG;				
	}												
													
    pZ80->AF.B.h = -AReg;							
													
	Flags |= ZeroSignTable[pZ80->AF.B.h&0x0ff];				
	Flags |= pZ80->AF.B.h & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Z80_FLAGS_REG = Flags;								
Cycles = 2;
}
break;
case 0x045:
{
/* RETN */
   pZ80->IFF1 = pZ80->IFF2; 
   /* update memptr */ 
   pZ80->MemPtr.W = Z80_RD_WORD(pZ80,pZ80->SP.W); 
   pZ80->SP.W+=2; 
   pZ80->PC.W.l = pZ80->MemPtr.W; 
   /* flags not changed */ 
	/* refresh check */
    Z80_RefreshInterruptRequest(pZ80);
Cycles = 4;
}
break;
case 0x046:
{
/* IM 0 */
         pZ80->IM = 0; 
Cycles = 2;
}
break;
case 0x047:
{
/* LD I,A */
 
	pZ80->I = pZ80->AF.B.h;        
    /* spare cycles at end of instruction that interrupt can consume if triggered */
    pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
Cycles = 2;
}
break;
case 0x048:
{
/* IN C,(C) */
 		pZ80->MemPtr.W = pZ80->BC.W; 
		pZ80->IOPort = pZ80->MemPtr.W;
		pZ80->Outputs = Z80_OUTPUT_IORQ | Z80_OUTPUT_RD;
		pZ80->BC.B.l = pZ80->IOData = Z80_DoIn(pZ80->IOPort);            
		++pZ80->MemPtr.W; 
		pZ80->Outputs = 0;
		{											
			Z80_BYTE	Flags;						
			Flags = Z80_FLAGS_REG;						
			Flags = Flags & Z80_CARRY_FLAG;			
			Flags |= ZeroSignParityTable[pZ80->BC.B.l];	
            Flags |= pZ80->BC.B.l & ((1<<5) | (1<<3));   
			Z80_FLAGS_REG = Flags;						
		}											
	Cycles = 4; 
}
break;
case 0x049:
{
 /* OUT (C),C */
 
	pZ80->IOData = pZ80->BC.B.l;
	pZ80->IOPort = pZ80->MemPtr.W = pZ80->BC.W; 
	pZ80->Outputs = Z80_OUTPUT_IORQ | Z80_OUTPUT_WR;
	Z80_DoOut(pZ80->IOPort,pZ80->IOData);                     
	pZ80->Outputs = 0; 
	++pZ80->MemPtr.W;                                       
	Cycles = 4;
}
break;
case 0x04a:
{
ADC_HL_rr(pZ80->BC.W);
Cycles = 4;
}
break;
case 0x04b:
{
/* LD BC,(nnnn) */
 
        /* read destination address into memptr */ 
        pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);	
        /* read register from address */ 
        pZ80->BC.W = Z80_RD_WORD(pZ80,pZ80->MemPtr.W);   
		++pZ80->MemPtr.W; 
Cycles = 6;
}
break;
case 0x04c:
{
/* NEG */

	Z80_BYTE	Flags;	
	Z80_BYTE	AReg; 
						
	AReg = pZ80->AF.B.h;		
    Flags = Z80_SUBTRACT_FLAG;	
													
    if (AReg == 0x080)									
    {												
          Flags |= Z80_PARITY_FLAG;					
    }												
													
    if (AReg != 0x000)									
    {												
        Flags |= Z80_CARRY_FLAG;					
    }												
													
	if ((AReg & 0x0f)!=0)								
	{												
		Flags |= Z80_HALFCARRY_FLAG;				
	}												
													
    pZ80->AF.B.h = -AReg;							
													
	Flags |= ZeroSignTable[pZ80->AF.B.h&0x0ff];				
	Flags |= pZ80->AF.B.h & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Z80_FLAGS_REG = Flags;								
Cycles = 2;
}
break;
case 0x04d:
{
/* RETI */
    pZ80->IFF1 = pZ80->IFF2; 
    Z80_Reti(pZ80); 
	 /* update memptr */ 
    pZ80->MemPtr.W = Z80_RD_WORD(pZ80,pZ80->SP.W); 
    pZ80->SP.W+=2; 
	pZ80->PC.W.l = pZ80->MemPtr.W; 
	/* flags not changed */ 
	/* refresh check */
    Z80_RefreshInterruptRequest(pZ80);
Cycles = 4;
}
break;
case 0x04e:
{
/* IM 0 */
         pZ80->IM = 0; 
Cycles = 2;
}
break;
case 0x04f:
{
/* LD R,A */
 
    /* store bit 7 */ 
    pZ80->RBit7 = pZ80->AF.B.h & 0x080; 
 
    /* store refresh register */ 
    pZ80->R = pZ80->AF.B.h & 0x07f; 
	/* no flags changed */ 
	/* spare cycles at end of instruction that interrupt can consume if triggered */
	pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
Cycles = 2;
}
break;
case 0x050:
{
/* IN D,(C) */
 		pZ80->MemPtr.W = pZ80->BC.W; 
		pZ80->IOPort = pZ80->MemPtr.W;
		pZ80->Outputs = Z80_OUTPUT_IORQ | Z80_OUTPUT_RD;
		pZ80->DE.B.h = pZ80->IOData = Z80_DoIn(pZ80->IOPort);            
		++pZ80->MemPtr.W; 
		pZ80->Outputs = 0;
		{											
			Z80_BYTE	Flags;						
			Flags = Z80_FLAGS_REG;						
			Flags = Flags & Z80_CARRY_FLAG;			
			Flags |= ZeroSignParityTable[pZ80->DE.B.h];	
            Flags |= pZ80->DE.B.h & ((1<<5) | (1<<3));   
			Z80_FLAGS_REG = Flags;						
		}											
	Cycles = 4; 
}
break;
case 0x051:
{
 /* OUT (C),D */
 
	pZ80->IOData = pZ80->DE.B.h;
	pZ80->IOPort = pZ80->MemPtr.W = pZ80->BC.W; 
	pZ80->Outputs = Z80_OUTPUT_IORQ | Z80_OUTPUT_WR;
	Z80_DoOut(pZ80->IOPort,pZ80->IOData);                     
	pZ80->Outputs = 0; 
	++pZ80->MemPtr.W;                                       
	Cycles = 4;
}
break;
case 0x052:
{
SBC_HL_rr(pZ80->DE.W);
Cycles = 4;
}
break;
case 0x053:
{
/* LD (nnnn),DE */
 
        /* read destination address into memptr */ 
        pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);	
        /* write register to address */ 
        Z80_WR_WORD(pZ80,pZ80->MemPtr.W, pZ80->DE.W);    
        ++pZ80->MemPtr.W; 
Cycles = 6;
}
break;
case 0x054:
{
/* NEG */

	Z80_BYTE	Flags;	
	Z80_BYTE	AReg; 
						
	AReg = pZ80->AF.B.h;		
    Flags = Z80_SUBTRACT_FLAG;	
													
    if (AReg == 0x080)									
    {												
          Flags |= Z80_PARITY_FLAG;					
    }												
													
    if (AReg != 0x000)									
    {												
        Flags |= Z80_CARRY_FLAG;					
    }												
													
	if ((AReg & 0x0f)!=0)								
	{												
		Flags |= Z80_HALFCARRY_FLAG;				
	}												
													
    pZ80->AF.B.h = -AReg;							
													
	Flags |= ZeroSignTable[pZ80->AF.B.h&0x0ff];				
	Flags |= pZ80->AF.B.h & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Z80_FLAGS_REG = Flags;								
Cycles = 2;
}
break;
case 0x055:
{
/* RETN */
   pZ80->IFF1 = pZ80->IFF2; 
   /* update memptr */ 
   pZ80->MemPtr.W = Z80_RD_WORD(pZ80,pZ80->SP.W); 
   pZ80->SP.W+=2; 
   pZ80->PC.W.l = pZ80->MemPtr.W; 
   /* flags not changed */ 
	/* refresh check */
    Z80_RefreshInterruptRequest(pZ80);
Cycles = 4;
}
break;
case 0x056:
{
/* IM 1 */
         pZ80->IM = 1; 
Cycles = 2;
}
break;
case 0x057:
{
/* LD A,I */

        pZ80->AF.B.h = pZ80->I;	
		{				
			Z80_BYTE	Flags;	
 
			/* HF, NF = 0, CF not changed */ 
			Flags = Z80_FLAGS_REG;	
			Flags &= Z80_CARRY_FLAG;	/* keep CF, zeroise everything else */ 
			/* NMOS Z80 */ 
			/* if interrupt request is pending we will execute an interrupt following this instruction */ 
			if (!Z80_GetInterruptRequest(pZ80)) 
			{ 
				Flags |= ((pZ80->IFF2 & 0x01) << Z80_PARITY_FLAG_BIT);	/* IFF2 into PV */ 
			} 
			Flags |= pZ80->AF.B.h & (Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2); /* Bits 5,3 from result */ 
			Z80_FLAGS_REG = Flags;	
			/* spare cycles at end of instruction that interrupt can consume if triggered */
            pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
        }	
Cycles = 2;
}
break;
case 0x058:
{
/* IN E,(C) */
 		pZ80->MemPtr.W = pZ80->BC.W; 
		pZ80->IOPort = pZ80->MemPtr.W;
		pZ80->Outputs = Z80_OUTPUT_IORQ | Z80_OUTPUT_RD;
		pZ80->DE.B.l = pZ80->IOData = Z80_DoIn(pZ80->IOPort);            
		++pZ80->MemPtr.W; 
		pZ80->Outputs = 0;
		{											
			Z80_BYTE	Flags;						
			Flags = Z80_FLAGS_REG;						
			Flags = Flags & Z80_CARRY_FLAG;			
			Flags |= ZeroSignParityTable[pZ80->DE.B.l];	
            Flags |= pZ80->DE.B.l & ((1<<5) | (1<<3));   
			Z80_FLAGS_REG = Flags;						
		}											
	Cycles = 4; 
}
break;
case 0x059:
{
 /* OUT (C),E */
 
	pZ80->IOData = pZ80->DE.B.l;
	pZ80->IOPort = pZ80->MemPtr.W = pZ80->BC.W; 
	pZ80->Outputs = Z80_OUTPUT_IORQ | Z80_OUTPUT_WR;
	Z80_DoOut(pZ80->IOPort,pZ80->IOData);                     
	pZ80->Outputs = 0; 
	++pZ80->MemPtr.W;                                       
	Cycles = 4;
}
break;
case 0x05a:
{
ADC_HL_rr(pZ80->DE.W);
Cycles = 4;
}
break;
case 0x05b:
{
/* LD DE,(nnnn) */
 
        /* read destination address into memptr */ 
        pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);	
        /* read register from address */ 
        pZ80->DE.W = Z80_RD_WORD(pZ80,pZ80->MemPtr.W);   
		++pZ80->MemPtr.W; 
Cycles = 6;
}
break;
case 0x05c:
{
/* NEG */

	Z80_BYTE	Flags;	
	Z80_BYTE	AReg; 
						
	AReg = pZ80->AF.B.h;		
    Flags = Z80_SUBTRACT_FLAG;	
													
    if (AReg == 0x080)									
    {												
          Flags |= Z80_PARITY_FLAG;					
    }												
													
    if (AReg != 0x000)									
    {												
        Flags |= Z80_CARRY_FLAG;					
    }												
													
	if ((AReg & 0x0f)!=0)								
	{												
		Flags |= Z80_HALFCARRY_FLAG;				
	}												
													
    pZ80->AF.B.h = -AReg;							
													
	Flags |= ZeroSignTable[pZ80->AF.B.h&0x0ff];				
	Flags |= pZ80->AF.B.h & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Z80_FLAGS_REG = Flags;								
Cycles = 2;
}
break;
case 0x05d:
{
/* RETI */
    pZ80->IFF1 = pZ80->IFF2; 
    Z80_Reti(pZ80); 
	 /* update memptr */ 
    pZ80->MemPtr.W = Z80_RD_WORD(pZ80,pZ80->SP.W); 
    pZ80->SP.W+=2; 
	pZ80->PC.W.l = pZ80->MemPtr.W; 
	/* flags not changed */ 
	/* refresh check */
    Z80_RefreshInterruptRequest(pZ80);
Cycles = 4;
}
break;
case 0x05e:
{
/* IM 2 */
         pZ80->IM = 2; 
Cycles = 2;
}
break;
case 0x05f:
{
/* LD A,R */

        pZ80->AF.B.h = Z80_GET_R;	
							
		{					
			Z80_BYTE	Flags;	
								
			/* HF, NF = 0, CF not changed */ 
			Flags = Z80_FLAGS_REG;	
			Flags &= Z80_CARRY_FLAG;	/* keep CF, zeroise everything else */ 
			/* NMOS Z80 */ 
			/* if interrupt request is pending we will execute an interrupt following this instruction */ 
			if (!Z80_GetInterruptRequest(pZ80)) 
			{ 
				Flags |= ((pZ80->IFF2 & 0x01) << Z80_PARITY_FLAG_BIT);	/* IFF2 into PV */ 
			} 
			Flags |= ZeroSignTable[pZ80->AF.B.h & 0x0ff];	/* SF, ZF */ 
			Flags |= pZ80->AF.B.h & (Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2); /* Bits 5,3 from result */ 
			Z80_FLAGS_REG = Flags;	
			/* spare cycles at end of instruction that interrupt can consume if triggered */
	pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
    }					
Cycles = 2;
}
break;
case 0x060:
{
/* IN H,(C) */
 		pZ80->MemPtr.W = pZ80->BC.W; 
		pZ80->IOPort = pZ80->MemPtr.W;
		pZ80->Outputs = Z80_OUTPUT_IORQ | Z80_OUTPUT_RD;
		pZ80->HL.B.h = pZ80->IOData = Z80_DoIn(pZ80->IOPort);            
		++pZ80->MemPtr.W; 
		pZ80->Outputs = 0;
		{											
			Z80_BYTE	Flags;						
			Flags = Z80_FLAGS_REG;						
			Flags = Flags & Z80_CARRY_FLAG;			
			Flags |= ZeroSignParityTable[pZ80->HL.B.h];	
            Flags |= pZ80->HL.B.h & ((1<<5) | (1<<3));   
			Z80_FLAGS_REG = Flags;						
		}											
	Cycles = 4; 
}
break;
case 0x061:
{
 /* OUT (C),H */
 
	pZ80->IOData = pZ80->HL.B.h;
	pZ80->IOPort = pZ80->MemPtr.W = pZ80->BC.W; 
	pZ80->Outputs = Z80_OUTPUT_IORQ | Z80_OUTPUT_WR;
	Z80_DoOut(pZ80->IOPort,pZ80->IOData);                     
	pZ80->Outputs = 0; 
	++pZ80->MemPtr.W;                                       
	Cycles = 4;
}
break;
case 0x062:
{
SBC_HL_rr(pZ80->HL.W);
Cycles = 4;
}
break;
case 0x063:
{
/* LD (nnnn),HL */
 
        /* read destination address into memptr */ 
        pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);	
        /* write register to address */ 
        Z80_WR_WORD(pZ80,pZ80->MemPtr.W, pZ80->HL.W);    
        ++pZ80->MemPtr.W; 
Cycles = 6;
}
break;
case 0x064:
{
/* NEG */

	Z80_BYTE	Flags;	
	Z80_BYTE	AReg; 
						
	AReg = pZ80->AF.B.h;		
    Flags = Z80_SUBTRACT_FLAG;	
													
    if (AReg == 0x080)									
    {												
          Flags |= Z80_PARITY_FLAG;					
    }												
													
    if (AReg != 0x000)									
    {												
        Flags |= Z80_CARRY_FLAG;					
    }												
													
	if ((AReg & 0x0f)!=0)								
	{												
		Flags |= Z80_HALFCARRY_FLAG;				
	}												
													
    pZ80->AF.B.h = -AReg;							
													
	Flags |= ZeroSignTable[pZ80->AF.B.h&0x0ff];				
	Flags |= pZ80->AF.B.h & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Z80_FLAGS_REG = Flags;								
Cycles = 2;
}
break;
case 0x065:
{
/* RETN */
   pZ80->IFF1 = pZ80->IFF2; 
   /* update memptr */ 
   pZ80->MemPtr.W = Z80_RD_WORD(pZ80,pZ80->SP.W); 
   pZ80->SP.W+=2; 
   pZ80->PC.W.l = pZ80->MemPtr.W; 
   /* flags not changed */ 
	/* refresh check */
    Z80_RefreshInterruptRequest(pZ80);
Cycles = 4;
}
break;
case 0x066:
{
/* IM 0 */
         pZ80->IM = 0; 
Cycles = 2;
}
break;
case 0x067:
{
/* RRD */
 
        pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->HL.W); 
        Z80_WR_BYTE(pZ80,pZ80->HL.W, (Z80_BYTE)((((pZ80->TempByte>>4)&0x0f) | ((pZ80->AF.B.h<<4)&0x0f0)))); 
        pZ80->AF.B.h = (pZ80->AF.B.h & 0x0f0) | (pZ80->TempByte & 0x0f); 
		pZ80->MemPtr.W = pZ80->HL.W+1; 
		{ 
			Z80_BYTE	Flags; 

			Flags = Z80_FLAGS_REG; 
			/* carry not affected */
			Flags &= Z80_CARRY_FLAG; 
			/* zero, sign and parity set */
			/* half carry and subtract reset */ 
			Flags |= ZeroSignParityTable[pZ80->AF.B.h&0x0ff]; 
            /* undocumented flags from result */
			Flags |= pZ80->AF.B.h&(Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2);
			Z80_FLAGS_REG = Flags; 
		} 
Cycles = 5;
}
break;
case 0x068:
{
/* IN L,(C) */
 		pZ80->MemPtr.W = pZ80->BC.W; 
		pZ80->IOPort = pZ80->MemPtr.W;
		pZ80->Outputs = Z80_OUTPUT_IORQ | Z80_OUTPUT_RD;
		pZ80->HL.B.l = pZ80->IOData = Z80_DoIn(pZ80->IOPort);            
		++pZ80->MemPtr.W; 
		pZ80->Outputs = 0;
		{											
			Z80_BYTE	Flags;						
			Flags = Z80_FLAGS_REG;						
			Flags = Flags & Z80_CARRY_FLAG;			
			Flags |= ZeroSignParityTable[pZ80->HL.B.l];	
            Flags |= pZ80->HL.B.l & ((1<<5) | (1<<3));   
			Z80_FLAGS_REG = Flags;						
		}											
	Cycles = 4; 
}
break;
case 0x069:
{
 /* OUT (C),L */
 
	pZ80->IOData = pZ80->HL.B.l;
	pZ80->IOPort = pZ80->MemPtr.W = pZ80->BC.W; 
	pZ80->Outputs = Z80_OUTPUT_IORQ | Z80_OUTPUT_WR;
	Z80_DoOut(pZ80->IOPort,pZ80->IOData);                     
	pZ80->Outputs = 0; 
	++pZ80->MemPtr.W;                                       
	Cycles = 4;
}
break;
case 0x06a:
{
ADC_HL_rr(pZ80->HL.W);
Cycles = 4;
}
break;
case 0x06b:
{
/* LD HL,(nnnn) */
 
        /* read destination address into memptr */ 
        pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);	
        /* read register from address */ 
        pZ80->HL.W = Z80_RD_WORD(pZ80,pZ80->MemPtr.W);   
		++pZ80->MemPtr.W; 
Cycles = 6;
}
break;
case 0x06c:
{
/* NEG */

	Z80_BYTE	Flags;	
	Z80_BYTE	AReg; 
						
	AReg = pZ80->AF.B.h;		
    Flags = Z80_SUBTRACT_FLAG;	
													
    if (AReg == 0x080)									
    {												
          Flags |= Z80_PARITY_FLAG;					
    }												
													
    if (AReg != 0x000)									
    {												
        Flags |= Z80_CARRY_FLAG;					
    }												
													
	if ((AReg & 0x0f)!=0)								
	{												
		Flags |= Z80_HALFCARRY_FLAG;				
	}												
													
    pZ80->AF.B.h = -AReg;							
													
	Flags |= ZeroSignTable[pZ80->AF.B.h&0x0ff];				
	Flags |= pZ80->AF.B.h & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Z80_FLAGS_REG = Flags;								
Cycles = 2;
}
break;
case 0x06d:
{
/* RETI */
    pZ80->IFF1 = pZ80->IFF2; 
    Z80_Reti(pZ80); 
	 /* update memptr */ 
    pZ80->MemPtr.W = Z80_RD_WORD(pZ80,pZ80->SP.W); 
    pZ80->SP.W+=2; 
	pZ80->PC.W.l = pZ80->MemPtr.W; 
	/* flags not changed */ 
	/* refresh check */
    Z80_RefreshInterruptRequest(pZ80);
Cycles = 4;
}
break;
case 0x06e:
{
/* IM 0 */
         pZ80->IM = 0; 
Cycles = 2;
}
break;
case 0x06f:
{
/* RLD */
 
	pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->HL.W); 
    Z80_WR_BYTE(pZ80,pZ80->HL.W,(Z80_BYTE)(((pZ80->TempByte<<4)&0x0f0)|(pZ80->AF.B.h & 0x0f))); 
    pZ80->AF.B.h = (pZ80->AF.B.h & 0x0f0) | ((pZ80->TempByte>>4)&0x0f); 
	pZ80->MemPtr.W = pZ80->HL.W+1; 
		{ 
			Z80_BYTE	Flags; 

			Flags = Z80_FLAGS_REG; 
			/* carry not affected */
			Flags &= Z80_CARRY_FLAG; 
			/* zero, sign and parity set */
			/* half carry and subtract reset */ 
			Flags |= ZeroSignParityTable[pZ80->AF.B.h&0x0ff]; 
            /* undocumented flags from result */
			Flags |= pZ80->AF.B.h&(Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2);
			Z80_FLAGS_REG = Flags; 
		} 
Cycles = 5;
}
break;
case 0x070:
{
/* IN X,(C) */
 		pZ80->MemPtr.W = pZ80->BC.W; 
		pZ80->IOPort = pZ80->MemPtr.W;
		pZ80->Outputs = Z80_OUTPUT_IORQ | Z80_OUTPUT_RD;
		pZ80->TempByte = pZ80->IOData = Z80_DoIn(pZ80->IOPort);            
		++pZ80->MemPtr.W; 
		pZ80->Outputs = 0;
		{											
			Z80_BYTE	Flags;						
			Flags = Z80_FLAGS_REG;						
			Flags = Flags & Z80_CARRY_FLAG;			
			Flags |= ZeroSignParityTable[pZ80->TempByte];	
            Flags |= pZ80->TempByte & ((1<<5) | (1<<3));   
			Z80_FLAGS_REG = Flags;						
		}											
	Cycles = 4; 
}
break;
case 0x071:
{
 /* OUT (C),0 - NMOS Z80 */
 
	pZ80->IOData = 0;
	pZ80->IOPort = pZ80->MemPtr.W = pZ80->BC.W; 
	pZ80->Outputs = Z80_OUTPUT_IORQ | Z80_OUTPUT_WR;
	Z80_DoOut(pZ80->IOPort,pZ80->IOData);                     
	pZ80->Outputs = 0; 
	++pZ80->MemPtr.W;                                       
	Cycles = 4;
}
break;
case 0x072:
{
SBC_HL_rr(pZ80->SP.W);
Cycles = 4;
}
break;
case 0x073:
{
/* LD (nnnn),SP */
 
        /* read destination address into memptr */ 
        pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);	
        /* write register to address */ 
        Z80_WR_WORD(pZ80,pZ80->MemPtr.W, pZ80->SP.W);    
        ++pZ80->MemPtr.W; 
Cycles = 6;
}
break;
case 0x074:
{
/* NEG */

	Z80_BYTE	Flags;	
	Z80_BYTE	AReg; 
						
	AReg = pZ80->AF.B.h;		
    Flags = Z80_SUBTRACT_FLAG;	
													
    if (AReg == 0x080)									
    {												
          Flags |= Z80_PARITY_FLAG;					
    }												
													
    if (AReg != 0x000)									
    {												
        Flags |= Z80_CARRY_FLAG;					
    }												
													
	if ((AReg & 0x0f)!=0)								
	{												
		Flags |= Z80_HALFCARRY_FLAG;				
	}												
													
    pZ80->AF.B.h = -AReg;							
													
	Flags |= ZeroSignTable[pZ80->AF.B.h&0x0ff];				
	Flags |= pZ80->AF.B.h & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Z80_FLAGS_REG = Flags;								
Cycles = 2;
}
break;
case 0x075:
{
/* RETN */
   pZ80->IFF1 = pZ80->IFF2; 
   /* update memptr */ 
   pZ80->MemPtr.W = Z80_RD_WORD(pZ80,pZ80->SP.W); 
   pZ80->SP.W+=2; 
   pZ80->PC.W.l = pZ80->MemPtr.W; 
   /* flags not changed */ 
	/* refresh check */
    Z80_RefreshInterruptRequest(pZ80);
Cycles = 4;
}
break;
case 0x076:
{
/* IM 1 */
         pZ80->IM = 1; 
Cycles = 2;
}
break;
case 0x077:
{
Cycles = 2;
}
break;
case 0x078:
{
/* IN A,(C) */
 		pZ80->MemPtr.W = pZ80->BC.W; 
		pZ80->IOPort = pZ80->MemPtr.W;
		pZ80->Outputs = Z80_OUTPUT_IORQ | Z80_OUTPUT_RD;
		pZ80->AF.B.h = pZ80->IOData = Z80_DoIn(pZ80->IOPort);            
		++pZ80->MemPtr.W; 
		pZ80->Outputs = 0;
		{											
			Z80_BYTE	Flags;						
			Flags = Z80_FLAGS_REG;						
			Flags = Flags & Z80_CARRY_FLAG;			
			Flags |= ZeroSignParityTable[pZ80->AF.B.h];	
            Flags |= pZ80->AF.B.h & ((1<<5) | (1<<3));   
			Z80_FLAGS_REG = Flags;						
		}											
	Cycles = 4; 
}
break;
case 0x079:
{
 /* OUT (C),A */
 
	pZ80->IOData = pZ80->AF.B.h;
	pZ80->IOPort = pZ80->MemPtr.W = pZ80->BC.W; 
	pZ80->Outputs = Z80_OUTPUT_IORQ | Z80_OUTPUT_WR;
	Z80_DoOut(pZ80->IOPort,pZ80->IOData);                     
	pZ80->Outputs = 0; 
	++pZ80->MemPtr.W;                                       
	Cycles = 4;
}
break;
case 0x07a:
{
ADC_HL_rr(pZ80->SP.W);
Cycles = 4;
}
break;
case 0x07b:
{
/* LD SP,(nnnn) */
 
        /* read destination address into memptr */ 
        pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);	
        /* read register from address */ 
        pZ80->SP.W = Z80_RD_WORD(pZ80,pZ80->MemPtr.W);   
		++pZ80->MemPtr.W; 
Cycles = 6;
}
break;
case 0x07c:
{
/* NEG */

	Z80_BYTE	Flags;	
	Z80_BYTE	AReg; 
						
	AReg = pZ80->AF.B.h;		
    Flags = Z80_SUBTRACT_FLAG;	
													
    if (AReg == 0x080)									
    {												
          Flags |= Z80_PARITY_FLAG;					
    }												
													
    if (AReg != 0x000)									
    {												
        Flags |= Z80_CARRY_FLAG;					
    }												
													
	if ((AReg & 0x0f)!=0)								
	{												
		Flags |= Z80_HALFCARRY_FLAG;				
	}												
													
    pZ80->AF.B.h = -AReg;							
													
	Flags |= ZeroSignTable[pZ80->AF.B.h&0x0ff];				
	Flags |= pZ80->AF.B.h & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Z80_FLAGS_REG = Flags;								
Cycles = 2;
}
break;
case 0x07d:
{
/* RETI */
    pZ80->IFF1 = pZ80->IFF2; 
    Z80_Reti(pZ80); 
	 /* update memptr */ 
    pZ80->MemPtr.W = Z80_RD_WORD(pZ80,pZ80->SP.W); 
    pZ80->SP.W+=2; 
	pZ80->PC.W.l = pZ80->MemPtr.W; 
	/* flags not changed */ 
	/* refresh check */
    Z80_RefreshInterruptRequest(pZ80);
Cycles = 4;
}
break;
case 0x07e:
{
/* IM 2 */
         pZ80->IM = 2; 
Cycles = 2;
}
break;
case 0x07f:
{
Cycles = 2;
}
break;
case 0x0a0:
{
/* LDI */
LDI();
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
Cycles = 4;
}
break;
case 0x0a1:
{
/* CPI */
CPI(pZ80);
Cycles = 4;
}
break;
case 0x0a2:
{
/* INI */
INI(pZ80);
Cycles = 5;
}
break;
case 0x0a3:
{
OUTI(pZ80);
Cycles = 5;
}
break;
case 0x0a8:
{
/* LDD */
LDD();
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
Cycles = 4;
}
break;
case 0x0a9:
{
/* CPD */
CPD(pZ80);
Cycles = 4;
}
break;
case 0x0aa:
{
/* IND */
IND(pZ80);
Cycles = 5;
}
break;
case 0x0ab:
{
OUTD(pZ80);
Cycles = 5;
}
break;
case 0x0b0:
{
/* LDIR */
LDI();
if (Z80_TEST_PARITY_EVEN)
{
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
pZ80->MemPtr.W = pZ80->PC.W.l-1;
pZ80->PC.W.l-=2;
Cycles=6-1;
}
else
{
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
Cycles=5-1;
}
}
break;
case 0x0b1:
{
/* CPIR */
CPI(pZ80);
/* to continue zero flag must not be set, and parity must be set */
if ((Z80_FLAGS_REG & (Z80_PARITY_FLAG | Z80_ZERO_FLAG))==Z80_PARITY_FLAG)
{
pZ80->MemPtr.W = pZ80->PC.W.l-1;
pZ80->PC.W.l-=2;
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
Cycles=6-1;
}
else
{
Cycles=4;
}
}
break;
case 0x0b2:
{
/* INIR */
INI(pZ80);
if (Z80_TEST_ZERO_SET)
{
Cycles=5;
}
else
{
pZ80->PC.W.l-=2;
Cycles=6;
}
}
break;
case 0x0b3:
{
OUTI(pZ80);
if (Z80_TEST_ZERO_SET)
{
Cycles=5;
}
else
{
pZ80->PC.W.l-=2;
Cycles=6;
}
}
break;
case 0x0b8:
{
/* LDDR */
LDD();
if (Z80_TEST_PARITY_EVEN)
{
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
pZ80->MemPtr.W = pZ80->PC.W.l-1;
pZ80->PC.W.l-=2;
Cycles=6-1;
}
else
{
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
Cycles=5-1;
}
}
break;
case 0x0b9:
{
/* CPDR */
CPD(pZ80);
/* to continue zero flag must not be set, and parity must be set */
if ((Z80_FLAGS_REG & (Z80_PARITY_FLAG | Z80_ZERO_FLAG))==Z80_PARITY_FLAG)
{
pZ80->MemPtr.W = pZ80->PC.W.l-1;
pZ80->PC.W.l-=2;
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
Cycles=6-1;
}
else
{
Cycles=4;
}
}
break;
case 0x0ba:
{
/* INDR */
IND(pZ80);
if (Z80_TEST_ZERO_SET)
{
Cycles=5;
}
else
{
pZ80->PC.W.l-=2;
Cycles=6;
}
}
break;
case 0x0bb:
{
OUTD(pZ80);
if (Z80_TEST_ZERO_SET)
{
Cycles=5;
}
else
{
pZ80->PC.W.l-=2;
Cycles=6;
}
}
break;
case 0x0ff:
{
Z80_DebugOpcodeTriggered();
Cycles = 2;
}
break;
default:
/* the following tells MSDEV 6 to not generate */
/* code which checks if a input value to the  */
/* switch is not valid.*/
#ifdef _MSC_VER
#if (_MSC_VER>=1200)
	Cycles=0;
	__assume(0);
#endif
#else
	Cycles=0;
#endif
break;
}
return Cycles;
}
/***************************************************************************/
 static int Z80_CB_ExecuteInstruction(Z80_REGISTERS *pZ80)
{
unsigned long Opcode;
unsigned long Cycles;
Opcode = Z80_RD_OPCODE_BYTE(pZ80);
Opcode = Opcode & 0x0ff;
switch (Opcode)
{
case 0x000:
{
RLC_REG(pZ80->BC.B.h);
Cycles = 2;
}
break;
case 0x001:
{
RLC_REG(pZ80->BC.B.l);
Cycles = 2;
}
break;
case 0x002:
{
RLC_REG(pZ80->DE.B.h);
Cycles = 2;
}
break;
case 0x003:
{
RLC_REG(pZ80->DE.B.l);
Cycles = 2;
}
break;
case 0x004:
{
RLC_REG(pZ80->HL.B.h);
Cycles = 2;
}
break;
case 0x005:
{
RLC_REG(pZ80->HL.B.l);
Cycles = 2;
}
break;
case 0x006:
{
RLC_HL();
Cycles = 4;
}
break;
case 0x007:
{
RLC_REG(pZ80->AF.B.h);
Cycles = 2;
}
break;
case 0x008:
{
RRC_REG(pZ80->BC.B.h);
Cycles = 2;
}
break;
case 0x009:
{
RRC_REG(pZ80->BC.B.l);
Cycles = 2;
}
break;
case 0x00a:
{
RRC_REG(pZ80->DE.B.h);
Cycles = 2;
}
break;
case 0x00b:
{
RRC_REG(pZ80->DE.B.l);
Cycles = 2;
}
break;
case 0x00c:
{
RRC_REG(pZ80->HL.B.h);
Cycles = 2;
}
break;
case 0x00d:
{
RRC_REG(pZ80->HL.B.l);
Cycles = 2;
}
break;
case 0x00e:
{
RRC_HL();
Cycles = 4;
}
break;
case 0x00f:
{
RRC_REG(pZ80->AF.B.h);
Cycles = 2;
}
break;
case 0x010:
{
RL_REG(pZ80->BC.B.h);
Cycles = 2;
}
break;
case 0x011:
{
RL_REG(pZ80->BC.B.l);
Cycles = 2;
}
break;
case 0x012:
{
RL_REG(pZ80->DE.B.h);
Cycles = 2;
}
break;
case 0x013:
{
RL_REG(pZ80->DE.B.l);
Cycles = 2;
}
break;
case 0x014:
{
RL_REG(pZ80->HL.B.h);
Cycles = 2;
}
break;
case 0x015:
{
RL_REG(pZ80->HL.B.l);
Cycles = 2;
}
break;
case 0x016:
{
RL_HL();
Cycles = 4;
}
break;
case 0x017:
{
RL_REG(pZ80->AF.B.h);
Cycles = 2;
}
break;
case 0x018:
{
RR_REG(pZ80->BC.B.h);
Cycles = 2;
}
break;
case 0x019:
{
RR_REG(pZ80->BC.B.l);
Cycles = 2;
}
break;
case 0x01a:
{
RR_REG(pZ80->DE.B.h);
Cycles = 2;
}
break;
case 0x01b:
{
RR_REG(pZ80->DE.B.l);
Cycles = 2;
}
break;
case 0x01c:
{
RR_REG(pZ80->HL.B.h);
Cycles = 2;
}
break;
case 0x01d:
{
RR_REG(pZ80->HL.B.l);
Cycles = 2;
}
break;
case 0x01e:
{
RR_HL();
Cycles = 4;
}
break;
case 0x01f:
{
RR_REG(pZ80->AF.B.h);
Cycles = 2;
}
break;
case 0x020:
{
SLA_REG(pZ80->BC.B.h);
Cycles = 2;
}
break;
case 0x021:
{
SLA_REG(pZ80->BC.B.l);
Cycles = 2;
}
break;
case 0x022:
{
SLA_REG(pZ80->DE.B.h);
Cycles = 2;
}
break;
case 0x023:
{
SLA_REG(pZ80->DE.B.l);
Cycles = 2;
}
break;
case 0x024:
{
SLA_REG(pZ80->HL.B.h);
Cycles = 2;
}
break;
case 0x025:
{
SLA_REG(pZ80->HL.B.l);
Cycles = 2;
}
break;
case 0x026:
{
SLA_HL();
Cycles = 4;
}
break;
case 0x027:
{
SLA_REG(pZ80->AF.B.h);
Cycles = 2;
}
break;
case 0x028:
{
SRA_REG(pZ80->BC.B.h);
Cycles = 2;
}
break;
case 0x029:
{
SRA_REG(pZ80->BC.B.l);
Cycles = 2;
}
break;
case 0x02a:
{
SRA_REG(pZ80->DE.B.h);
Cycles = 2;
}
break;
case 0x02b:
{
SRA_REG(pZ80->DE.B.l);
Cycles = 2;
}
break;
case 0x02c:
{
SRA_REG(pZ80->HL.B.h);
Cycles = 2;
}
break;
case 0x02d:
{
SRA_REG(pZ80->HL.B.l);
Cycles = 2;
}
break;
case 0x02e:
{
SRA_HL();
Cycles = 4;
}
break;
case 0x02f:
{
SRA_REG(pZ80->AF.B.h);
Cycles = 2;
}
break;
case 0x030:
{
SLL_REG(pZ80->BC.B.h);
Cycles = 2;
}
break;
case 0x031:
{
SLL_REG(pZ80->BC.B.l);
Cycles = 2;
}
break;
case 0x032:
{
SLL_REG(pZ80->DE.B.h);
Cycles = 2;
}
break;
case 0x033:
{
SLL_REG(pZ80->DE.B.l);
Cycles = 2;
}
break;
case 0x034:
{
SLL_REG(pZ80->HL.B.h);
Cycles = 2;
}
break;
case 0x035:
{
SLL_REG(pZ80->HL.B.l);
Cycles = 2;
}
break;
case 0x036:
{
SLL_HL();
Cycles = 4;
}
break;
case 0x037:
{
SLL_REG(pZ80->AF.B.h);
Cycles = 2;
}
break;
case 0x038:
{
SRL_REG(pZ80->BC.B.h);
Cycles = 2;
}
break;
case 0x039:
{
SRL_REG(pZ80->BC.B.l);
Cycles = 2;
}
break;
case 0x03a:
{
SRL_REG(pZ80->DE.B.h);
Cycles = 2;
}
break;
case 0x03b:
{
SRL_REG(pZ80->DE.B.l);
Cycles = 2;
}
break;
case 0x03c:
{
SRL_REG(pZ80->HL.B.h);
Cycles = 2;
}
break;
case 0x03d:
{
SRL_REG(pZ80->HL.B.l);
Cycles = 2;
}
break;
case 0x03e:
{
SRL_HL();
Cycles = 4;
}
break;
case 0x03f:
{
SRL_REG(pZ80->AF.B.h);
Cycles = 2;
}
break;
case 0x040:
{
/* BIT 0,B */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<0);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->BC.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x041:
{
/* BIT 0,C */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<0);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->BC.B.l & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x042:
{
/* BIT 0,D */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<0);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->DE.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x043:
{
/* BIT 0,E */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<0);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->DE.B.l & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x044:
{
/* BIT 0,H */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<0);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->HL.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x045:
{
/* BIT 0,L */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<0);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->HL.B.l & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x046:
{
/* BIT 0,(HL) */
pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->HL.W);

{
	Z80_BYTE	Flags;						
	const Z80_BYTE	Mask = (1<<0);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 
    pZ80->TempByte = pZ80->TempByte & Mask; 
	Flags |= pZ80->MemPtr.B.h & ((1<<5) | (1<<3)); 
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 3;
}
break;
case 0x047:
{
/* BIT 0,A */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<0);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->AF.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x048:
{
/* BIT 1,B */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<1);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->BC.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x049:
{
/* BIT 1,C */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<1);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->BC.B.l & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x04a:
{
/* BIT 1,D */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<1);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->DE.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x04b:
{
/* BIT 1,E */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<1);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->DE.B.l & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x04c:
{
/* BIT 1,H */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<1);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->HL.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x04d:
{
/* BIT 1,L */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<1);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->HL.B.l & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x04e:
{
/* BIT 1,(HL) */
pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->HL.W);

{
	Z80_BYTE	Flags;						
	const Z80_BYTE	Mask = (1<<1);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 
    pZ80->TempByte = pZ80->TempByte & Mask; 
	Flags |= pZ80->MemPtr.B.h & ((1<<5) | (1<<3)); 
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 3;
}
break;
case 0x04f:
{
/* BIT 1,A */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<1);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->AF.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x050:
{
/* BIT 2,B */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<2);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->BC.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x051:
{
/* BIT 2,C */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<2);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->BC.B.l & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x052:
{
/* BIT 2,D */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<2);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->DE.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x053:
{
/* BIT 2,E */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<2);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->DE.B.l & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x054:
{
/* BIT 2,H */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<2);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->HL.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x055:
{
/* BIT 2,L */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<2);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->HL.B.l & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x056:
{
/* BIT 2,(HL) */
pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->HL.W);

{
	Z80_BYTE	Flags;						
	const Z80_BYTE	Mask = (1<<2);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 
    pZ80->TempByte = pZ80->TempByte & Mask; 
	Flags |= pZ80->MemPtr.B.h & ((1<<5) | (1<<3)); 
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 3;
}
break;
case 0x057:
{
/* BIT 2,A */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<2);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->AF.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x058:
{
/* BIT 3,B */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<3);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->BC.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x059:
{
/* BIT 3,C */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<3);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->BC.B.l & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x05a:
{
/* BIT 3,D */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<3);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->DE.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x05b:
{
/* BIT 3,E */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<3);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->DE.B.l & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x05c:
{
/* BIT 3,H */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<3);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->HL.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x05d:
{
/* BIT 3,L */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<3);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->HL.B.l & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x05e:
{
/* BIT 3,(HL) */
pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->HL.W);

{
	Z80_BYTE	Flags;						
	const Z80_BYTE	Mask = (1<<3);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 
    pZ80->TempByte = pZ80->TempByte & Mask; 
	Flags |= pZ80->MemPtr.B.h & ((1<<5) | (1<<3)); 
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 3;
}
break;
case 0x05f:
{
/* BIT 3,A */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<3);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->AF.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x060:
{
/* BIT 4,B */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<4);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->BC.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x061:
{
/* BIT 4,C */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<4);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->BC.B.l & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x062:
{
/* BIT 4,D */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<4);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->DE.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x063:
{
/* BIT 4,E */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<4);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->DE.B.l & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x064:
{
/* BIT 4,H */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<4);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->HL.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x065:
{
/* BIT 4,L */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<4);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->HL.B.l & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x066:
{
/* BIT 4,(HL) */
pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->HL.W);

{
	Z80_BYTE	Flags;						
	const Z80_BYTE	Mask = (1<<4);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 
    pZ80->TempByte = pZ80->TempByte & Mask; 
	Flags |= pZ80->MemPtr.B.h & ((1<<5) | (1<<3)); 
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 3;
}
break;
case 0x067:
{
/* BIT 4,A */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<4);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->AF.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x068:
{
/* BIT 5,B */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<5);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->BC.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x069:
{
/* BIT 5,C */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<5);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->BC.B.l & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x06a:
{
/* BIT 5,D */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<5);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->DE.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x06b:
{
/* BIT 5,E */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<5);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->DE.B.l & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x06c:
{
/* BIT 5,H */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<5);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->HL.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x06d:
{
/* BIT 5,L */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<5);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->HL.B.l & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x06e:
{
/* BIT 5,(HL) */
pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->HL.W);

{
	Z80_BYTE	Flags;						
	const Z80_BYTE	Mask = (1<<5);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 
    pZ80->TempByte = pZ80->TempByte & Mask; 
	Flags |= pZ80->MemPtr.B.h & ((1<<5) | (1<<3)); 
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 3;
}
break;
case 0x06f:
{
/* BIT 5,A */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<5);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->AF.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x070:
{
/* BIT 6,B */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<6);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->BC.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x071:
{
/* BIT 6,C */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<6);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->BC.B.l & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x072:
{
/* BIT 6,D */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<6);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->DE.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x073:
{
/* BIT 6,E */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<6);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->DE.B.l & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x074:
{
/* BIT 6,H */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<6);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->HL.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x075:
{
/* BIT 6,L */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<6);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->HL.B.l & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x076:
{
/* BIT 6,(HL) */
pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->HL.W);

{
	Z80_BYTE	Flags;						
	const Z80_BYTE	Mask = (1<<6);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 
    pZ80->TempByte = pZ80->TempByte & Mask; 
	Flags |= pZ80->MemPtr.B.h & ((1<<5) | (1<<3)); 
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 3;
}
break;
case 0x077:
{
/* BIT 6,A */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<6);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->AF.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x078:
{
/* BIT 7,B */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<7);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->BC.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x079:
{
/* BIT 7,C */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<7);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->BC.B.l & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x07a:
{
/* BIT 7,D */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<7);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->DE.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x07b:
{
/* BIT 7,E */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<7);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->DE.B.l & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x07c:
{
/* BIT 7,H */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<7);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->HL.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x07d:
{
/* BIT 7,L */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<7);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->HL.B.l & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x07e:
{
/* BIT 7,(HL) */
pZ80->TempByte = Z80_RD_BYTE(pZ80,pZ80->HL.W);

{
	Z80_BYTE	Flags;						
	const Z80_BYTE	Mask = (1<<7);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 
    pZ80->TempByte = pZ80->TempByte & Mask; 
	Flags |= pZ80->MemPtr.B.h & ((1<<5) | (1<<3)); 
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 3;
}
break;
case 0x07f:
{
/* BIT 7,A */
 
{   
	Z80_BYTE	Flags;					
	const Z80_BYTE	Mask = (1<<7);				
	Flags = Z80_FLAGS_REG & Z80_CARRY_FLAG;	/* CF not changed, NF set to zero */ 
	Flags |= Z80_HALFCARRY_FLAG;			/* HF set */ 

	pZ80->TempByte = pZ80->AF.B.h & Mask;		/* perform AND operation */ 
	/* handle SF,YF,XF */ 
	/* there will be 1 in the place of the bit if it is set */ 
	/* if bit 7 was tested there will be a 1 there, but not in 5 or 3 */ 
	/* if bit 5 was tested there will be a 1 there, but not in 7 or 3 */ 
	/* if bit 3 was tested there will be a 1 there, but not in 7 or 5 */ 
	Flags |= pZ80->TempByte & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
	Flags |= ZeroSignParityTable[pZ80->TempByte & 0x0ff]; 
	Z80_FLAGS_REG = Flags; 
}
Cycles = 2;
}
break;
case 0x080:
{
RES_REG(0x01,pZ80->BC.B.h);
Cycles = 2;
}
break;
case 0x081:
{
RES_REG(0x01,pZ80->BC.B.l);
Cycles = 2;
}
break;
case 0x082:
{
RES_REG(0x01,pZ80->DE.B.h);
Cycles = 2;
}
break;
case 0x083:
{
RES_REG(0x01,pZ80->DE.B.l);
Cycles = 2;
}
break;
case 0x084:
{
RES_REG(0x01,pZ80->HL.B.h);
Cycles = 2;
}
break;
case 0x085:
{
RES_REG(0x01,pZ80->HL.B.l);
Cycles = 2;
}
break;
case 0x086:
{
RES_HL(0x01);
Cycles = 4;
}
break;
case 0x087:
{
RES_REG(0x01,pZ80->AF.B.h);
Cycles = 2;
}
break;
case 0x088:
{
RES_REG(0x02,pZ80->BC.B.h);
Cycles = 2;
}
break;
case 0x089:
{
RES_REG(0x02,pZ80->BC.B.l);
Cycles = 2;
}
break;
case 0x08a:
{
RES_REG(0x02,pZ80->DE.B.h);
Cycles = 2;
}
break;
case 0x08b:
{
RES_REG(0x02,pZ80->DE.B.l);
Cycles = 2;
}
break;
case 0x08c:
{
RES_REG(0x02,pZ80->HL.B.h);
Cycles = 2;
}
break;
case 0x08d:
{
RES_REG(0x02,pZ80->HL.B.l);
Cycles = 2;
}
break;
case 0x08e:
{
RES_HL(0x02);
Cycles = 4;
}
break;
case 0x08f:
{
RES_REG(0x02,pZ80->AF.B.h);
Cycles = 2;
}
break;
case 0x090:
{
RES_REG(0x04,pZ80->BC.B.h);
Cycles = 2;
}
break;
case 0x091:
{
RES_REG(0x04,pZ80->BC.B.l);
Cycles = 2;
}
break;
case 0x092:
{
RES_REG(0x04,pZ80->DE.B.h);
Cycles = 2;
}
break;
case 0x093:
{
RES_REG(0x04,pZ80->DE.B.l);
Cycles = 2;
}
break;
case 0x094:
{
RES_REG(0x04,pZ80->HL.B.h);
Cycles = 2;
}
break;
case 0x095:
{
RES_REG(0x04,pZ80->HL.B.l);
Cycles = 2;
}
break;
case 0x096:
{
RES_HL(0x04);
Cycles = 4;
}
break;
case 0x097:
{
RES_REG(0x04,pZ80->AF.B.h);
Cycles = 2;
}
break;
case 0x098:
{
RES_REG(0x08,pZ80->BC.B.h);
Cycles = 2;
}
break;
case 0x099:
{
RES_REG(0x08,pZ80->BC.B.l);
Cycles = 2;
}
break;
case 0x09a:
{
RES_REG(0x08,pZ80->DE.B.h);
Cycles = 2;
}
break;
case 0x09b:
{
RES_REG(0x08,pZ80->DE.B.l);
Cycles = 2;
}
break;
case 0x09c:
{
RES_REG(0x08,pZ80->HL.B.h);
Cycles = 2;
}
break;
case 0x09d:
{
RES_REG(0x08,pZ80->HL.B.l);
Cycles = 2;
}
break;
case 0x09e:
{
RES_HL(0x08);
Cycles = 4;
}
break;
case 0x09f:
{
RES_REG(0x08,pZ80->AF.B.h);
Cycles = 2;
}
break;
case 0x0a0:
{
RES_REG(0x10,pZ80->BC.B.h);
Cycles = 2;
}
break;
case 0x0a1:
{
RES_REG(0x10,pZ80->BC.B.l);
Cycles = 2;
}
break;
case 0x0a2:
{
RES_REG(0x10,pZ80->DE.B.h);
Cycles = 2;
}
break;
case 0x0a3:
{
RES_REG(0x10,pZ80->DE.B.l);
Cycles = 2;
}
break;
case 0x0a4:
{
RES_REG(0x10,pZ80->HL.B.h);
Cycles = 2;
}
break;
case 0x0a5:
{
RES_REG(0x10,pZ80->HL.B.l);
Cycles = 2;
}
break;
case 0x0a6:
{
RES_HL(0x10);
Cycles = 4;
}
break;
case 0x0a7:
{
RES_REG(0x10,pZ80->AF.B.h);
Cycles = 2;
}
break;
case 0x0a8:
{
RES_REG(0x20,pZ80->BC.B.h);
Cycles = 2;
}
break;
case 0x0a9:
{
RES_REG(0x20,pZ80->BC.B.l);
Cycles = 2;
}
break;
case 0x0aa:
{
RES_REG(0x20,pZ80->DE.B.h);
Cycles = 2;
}
break;
case 0x0ab:
{
RES_REG(0x20,pZ80->DE.B.l);
Cycles = 2;
}
break;
case 0x0ac:
{
RES_REG(0x20,pZ80->HL.B.h);
Cycles = 2;
}
break;
case 0x0ad:
{
RES_REG(0x20,pZ80->HL.B.l);
Cycles = 2;
}
break;
case 0x0ae:
{
RES_HL(0x20);
Cycles = 4;
}
break;
case 0x0af:
{
RES_REG(0x20,pZ80->AF.B.h);
Cycles = 2;
}
break;
case 0x0b0:
{
RES_REG(0x40,pZ80->BC.B.h);
Cycles = 2;
}
break;
case 0x0b1:
{
RES_REG(0x40,pZ80->BC.B.l);
Cycles = 2;
}
break;
case 0x0b2:
{
RES_REG(0x40,pZ80->DE.B.h);
Cycles = 2;
}
break;
case 0x0b3:
{
RES_REG(0x40,pZ80->DE.B.l);
Cycles = 2;
}
break;
case 0x0b4:
{
RES_REG(0x40,pZ80->HL.B.h);
Cycles = 2;
}
break;
case 0x0b5:
{
RES_REG(0x40,pZ80->HL.B.l);
Cycles = 2;
}
break;
case 0x0b6:
{
RES_HL(0x40);
Cycles = 4;
}
break;
case 0x0b7:
{
RES_REG(0x40,pZ80->AF.B.h);
Cycles = 2;
}
break;
case 0x0b8:
{
RES_REG(0x80,pZ80->BC.B.h);
Cycles = 2;
}
break;
case 0x0b9:
{
RES_REG(0x80,pZ80->BC.B.l);
Cycles = 2;
}
break;
case 0x0ba:
{
RES_REG(0x80,pZ80->DE.B.h);
Cycles = 2;
}
break;
case 0x0bb:
{
RES_REG(0x80,pZ80->DE.B.l);
Cycles = 2;
}
break;
case 0x0bc:
{
RES_REG(0x80,pZ80->HL.B.h);
Cycles = 2;
}
break;
case 0x0bd:
{
RES_REG(0x80,pZ80->HL.B.l);
Cycles = 2;
}
break;
case 0x0be:
{
RES_HL(0x80);
Cycles = 4;
}
break;
case 0x0bf:
{
RES_REG(0x80,pZ80->AF.B.h);
Cycles = 2;
}
break;
case 0x0c0:
{
SET_REG(0x01,pZ80->BC.B.h);
Cycles = 2;
}
break;
case 0x0c1:
{
SET_REG(0x01,pZ80->BC.B.l);
Cycles = 2;
}
break;
case 0x0c2:
{
SET_REG(0x01,pZ80->DE.B.h);
Cycles = 2;
}
break;
case 0x0c3:
{
SET_REG(0x01,pZ80->DE.B.l);
Cycles = 2;
}
break;
case 0x0c4:
{
SET_REG(0x01,pZ80->HL.B.h);
Cycles = 2;
}
break;
case 0x0c5:
{
SET_REG(0x01,pZ80->HL.B.l);
Cycles = 2;
}
break;
case 0x0c6:
{
SET_HL(0x01);
Cycles = 4;
}
break;
case 0x0c7:
{
SET_REG(0x01,pZ80->AF.B.h);
Cycles = 2;
}
break;
case 0x0c8:
{
SET_REG(0x02,pZ80->BC.B.h);
Cycles = 2;
}
break;
case 0x0c9:
{
SET_REG(0x02,pZ80->BC.B.l);
Cycles = 2;
}
break;
case 0x0ca:
{
SET_REG(0x02,pZ80->DE.B.h);
Cycles = 2;
}
break;
case 0x0cb:
{
SET_REG(0x02,pZ80->DE.B.l);
Cycles = 2;
}
break;
case 0x0cc:
{
SET_REG(0x02,pZ80->HL.B.h);
Cycles = 2;
}
break;
case 0x0cd:
{
SET_REG(0x02,pZ80->HL.B.l);
Cycles = 2;
}
break;
case 0x0ce:
{
SET_HL(0x02);
Cycles = 4;
}
break;
case 0x0cf:
{
SET_REG(0x02,pZ80->AF.B.h);
Cycles = 2;
}
break;
case 0x0d0:
{
SET_REG(0x04,pZ80->BC.B.h);
Cycles = 2;
}
break;
case 0x0d1:
{
SET_REG(0x04,pZ80->BC.B.l);
Cycles = 2;
}
break;
case 0x0d2:
{
SET_REG(0x04,pZ80->DE.B.h);
Cycles = 2;
}
break;
case 0x0d3:
{
SET_REG(0x04,pZ80->DE.B.l);
Cycles = 2;
}
break;
case 0x0d4:
{
SET_REG(0x04,pZ80->HL.B.h);
Cycles = 2;
}
break;
case 0x0d5:
{
SET_REG(0x04,pZ80->HL.B.l);
Cycles = 2;
}
break;
case 0x0d6:
{
SET_HL(0x04);
Cycles = 4;
}
break;
case 0x0d7:
{
SET_REG(0x04,pZ80->AF.B.h);
Cycles = 2;
}
break;
case 0x0d8:
{
SET_REG(0x08,pZ80->BC.B.h);
Cycles = 2;
}
break;
case 0x0d9:
{
SET_REG(0x08,pZ80->BC.B.l);
Cycles = 2;
}
break;
case 0x0da:
{
SET_REG(0x08,pZ80->DE.B.h);
Cycles = 2;
}
break;
case 0x0db:
{
SET_REG(0x08,pZ80->DE.B.l);
Cycles = 2;
}
break;
case 0x0dc:
{
SET_REG(0x08,pZ80->HL.B.h);
Cycles = 2;
}
break;
case 0x0dd:
{
SET_REG(0x08,pZ80->HL.B.l);
Cycles = 2;
}
break;
case 0x0de:
{
SET_HL(0x08);
Cycles = 4;
}
break;
case 0x0df:
{
SET_REG(0x08,pZ80->AF.B.h);
Cycles = 2;
}
break;
case 0x0e0:
{
SET_REG(0x10,pZ80->BC.B.h);
Cycles = 2;
}
break;
case 0x0e1:
{
SET_REG(0x10,pZ80->BC.B.l);
Cycles = 2;
}
break;
case 0x0e2:
{
SET_REG(0x10,pZ80->DE.B.h);
Cycles = 2;
}
break;
case 0x0e3:
{
SET_REG(0x10,pZ80->DE.B.l);
Cycles = 2;
}
break;
case 0x0e4:
{
SET_REG(0x10,pZ80->HL.B.h);
Cycles = 2;
}
break;
case 0x0e5:
{
SET_REG(0x10,pZ80->HL.B.l);
Cycles = 2;
}
break;
case 0x0e6:
{
SET_HL(0x10);
Cycles = 4;
}
break;
case 0x0e7:
{
SET_REG(0x10,pZ80->AF.B.h);
Cycles = 2;
}
break;
case 0x0e8:
{
SET_REG(0x20,pZ80->BC.B.h);
Cycles = 2;
}
break;
case 0x0e9:
{
SET_REG(0x20,pZ80->BC.B.l);
Cycles = 2;
}
break;
case 0x0ea:
{
SET_REG(0x20,pZ80->DE.B.h);
Cycles = 2;
}
break;
case 0x0eb:
{
SET_REG(0x20,pZ80->DE.B.l);
Cycles = 2;
}
break;
case 0x0ec:
{
SET_REG(0x20,pZ80->HL.B.h);
Cycles = 2;
}
break;
case 0x0ed:
{
SET_REG(0x20,pZ80->HL.B.l);
Cycles = 2;
}
break;
case 0x0ee:
{
SET_HL(0x20);
Cycles = 4;
}
break;
case 0x0ef:
{
SET_REG(0x20,pZ80->AF.B.h);
Cycles = 2;
}
break;
case 0x0f0:
{
SET_REG(0x40,pZ80->BC.B.h);
Cycles = 2;
}
break;
case 0x0f1:
{
SET_REG(0x40,pZ80->BC.B.l);
Cycles = 2;
}
break;
case 0x0f2:
{
SET_REG(0x40,pZ80->DE.B.h);
Cycles = 2;
}
break;
case 0x0f3:
{
SET_REG(0x40,pZ80->DE.B.l);
Cycles = 2;
}
break;
case 0x0f4:
{
SET_REG(0x40,pZ80->HL.B.h);
Cycles = 2;
}
break;
case 0x0f5:
{
SET_REG(0x40,pZ80->HL.B.l);
Cycles = 2;
}
break;
case 0x0f6:
{
SET_HL(0x40);
Cycles = 4;
}
break;
case 0x0f7:
{
SET_REG(0x40,pZ80->AF.B.h);
Cycles = 2;
}
break;
case 0x0f8:
{
SET_REG(0x80,pZ80->BC.B.h);
Cycles = 2;
}
break;
case 0x0f9:
{
SET_REG(0x80,pZ80->BC.B.l);
Cycles = 2;
}
break;
case 0x0fa:
{
SET_REG(0x80,pZ80->DE.B.h);
Cycles = 2;
}
break;
case 0x0fb:
{
SET_REG(0x80,pZ80->DE.B.l);
Cycles = 2;
}
break;
case 0x0fc:
{
SET_REG(0x80,pZ80->HL.B.h);
Cycles = 2;
}
break;
case 0x0fd:
{
SET_REG(0x80,pZ80->HL.B.l);
Cycles = 2;
}
break;
case 0x0fe:
{
SET_HL(0x80);
Cycles = 4;
}
break;
case 0x0ff:
{
SET_REG(0x80,pZ80->AF.B.h);
Cycles = 2;
}
break;
default:
/* the following tells MSDEV 6 to not generate */
/* code which checks if a input value to the  */
/* switch is not valid.*/
#ifdef _MSC_VER
#if (_MSC_VER>=1200)
	Cycles=0;
	__assume(0);
#endif
#else
	Cycles=0;
#endif
break;
}
        pZ80->R+=2;
 return Cycles;
}
/***************************************************************************/
int Z80_ExecuteInstruction(Z80_REGISTERS *pZ80)
{
unsigned long Opcode;
unsigned long Cycles;
Opcode = Z80_RD_OPCODE_BYTE(pZ80);
Opcode = Opcode & 0x0ff;
switch (Opcode)
{
case 0x000:
{
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x001:
{
/* LD BC,nnnn */
 
        pZ80->BC.W = Z80_RD_PC_WORD(pZ80); 
        pZ80->R+=1;
 Cycles = 3;
}
break;
case 0x002:
{
/* LD (BC),A */
 
    Z80_WR_BYTE(pZ80,pZ80->BC.W,pZ80->AF.B.h); 
	pZ80->MemPtr.B.l = (pZ80->BC.W+1) & 0x0ff; 
	pZ80->MemPtr.B.h = pZ80->AF.B.h; 
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x003:
{
/* INC BC */
 
    ++pZ80->BC.W;                
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x004:
{
INC_R(pZ80->BC.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x005:
{
DEC_R(pZ80->BC.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x006:
{
 /* LD B,n */
pZ80->BC.B.h = Z80_RD_PC_BYTE(pZ80);
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x007:
{

{ 
	Z80_BYTE        OrByte; 
	Z80_BYTE		Flags; 
	OrByte = (pZ80->AF.B.h>>7) & 0x01;	
	Flags = Z80_FLAGS_REG; 
	Flags = Flags & (Z80_SIGN_FLAG | Z80_ZERO_FLAG | Z80_PARITY_FLAG);	
    Flags |= OrByte;			
	{							
		Z80_BYTE Reg;			
								
		Reg = pZ80->AF.B.h;			
		Reg = Reg<<1;			
		Reg = (Reg&0x0fe)|OrByte;		
	    pZ80->AF.B.h=Reg;            
		Reg &= (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
		Flags = Flags | Reg;	
	}		
	Z80_FLAGS_REG = Flags;				
}        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x008:
{
SWAP(pZ80->AF.W,pZ80->altAF.W);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x009:
{
ADD_RR_rr(pZ80->HL.W,pZ80->BC.W);
        pZ80->R+=1;
 Cycles = 3;
}
break;
case 0x00a:
{
/* LD A,(BC) */
 
    pZ80->AF.B.h = Z80_RD_BYTE(pZ80,pZ80->BC.W); 
	pZ80->MemPtr.W = pZ80->BC.W+1; 
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x00b:
{
/* DEC BC */
 
	--pZ80->BC.W;                
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x00c:
{
INC_R(pZ80->BC.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x00d:
{
DEC_R(pZ80->BC.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x00e:
{
 /* LD C,n */
pZ80->BC.B.l = Z80_RD_PC_BYTE(pZ80);
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x00f:
{

{               
	Z80_BYTE	Flags;	
	Z80_BYTE	OrByte;	
	OrByte = pZ80->AF.B.h & 0x01;			
	Flags = Z80_FLAGS_REG;	
	Flags = Flags & (Z80_SIGN_FLAG | Z80_ZERO_FLAG | Z80_PARITY_FLAG); 
	Flags |= OrByte; 
	OrByte = OrByte<<7;	
	{					
		Z80_BYTE Reg;	
		Reg = pZ80->AF.B.h; 
		Reg = Reg>>1; 
		Reg = (Reg&0x07f)| OrByte; 
		pZ80->AF.B.h = Reg;		
		Reg = Reg & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
		Flags = Flags | Reg;								
	} 
	Z80_FLAGS_REG = Flags; 
}        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x010:
{
/* DJNZ dd */
Z80_BYTE_OFFSET Offset;
Offset = Z80_RD_PC_BYTE(pZ80);
pZ80->BC.B.h--;
if (pZ80->BC.B.h==0)
{
Cycles = 3;
}
else
{
pZ80->MemPtr.W = pZ80->PC.W.l + Offset;
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles = 4;
}
        pZ80->R+=1;
 }
break;
case 0x011:
{
/* LD DE,nnnn */
 
        pZ80->DE.W = Z80_RD_PC_WORD(pZ80); 
        pZ80->R+=1;
 Cycles = 3;
}
break;
case 0x012:
{
/* LD (DE),A */
 
    Z80_WR_BYTE(pZ80,pZ80->DE.W,pZ80->AF.B.h); 
	pZ80->MemPtr.B.l = (pZ80->DE.W+1) & 0x0ff; 
	pZ80->MemPtr.B.h = pZ80->AF.B.h; 
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x013:
{
/* INC DE */
 
    ++pZ80->DE.W;                
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x014:
{
INC_R(pZ80->DE.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x015:
{
DEC_R(pZ80->DE.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x016:
{
 /* LD D,n */
pZ80->DE.B.h = Z80_RD_PC_BYTE(pZ80);
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x017:
{
 
{ 
	Z80_BYTE	OrByte;	
	Z80_BYTE	Flags;	
	OrByte = Z80_FLAGS_REG & 0x01;	
	Flags = Z80_FLAGS_REG;			
	Flags = Flags & (Z80_SIGN_FLAG | Z80_ZERO_FLAG | Z80_PARITY_FLAG);	
	Flags |= ((pZ80->AF.B.h>>7) & 0x01); 
	{							
		Z80_BYTE	Reg;		
		Reg = pZ80->AF.B.h;			
		Reg = Reg<<1;			
		Reg = (Reg&0x0fe)|OrByte;		
		pZ80->AF.B.h = Reg;			
		Reg &= (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
		Flags = Flags | Reg;	
	} 
	Z80_FLAGS_REG = Flags;			
}        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x018:
{
/* JR dd */
Z80_BYTE_OFFSET Offset;
Offset = Z80_RD_PC_BYTE(pZ80);
pZ80->MemPtr.W = pZ80->PC.W.l + Offset;
pZ80->PC.W.l = pZ80->MemPtr.W;
        pZ80->R+=1;
 Cycles = 3;
}
break;
case 0x019:
{
ADD_RR_rr(pZ80->HL.W,pZ80->DE.W);
        pZ80->R+=1;
 Cycles = 3;
}
break;
case 0x01a:
{
/* LD A,(DE) */
 
    pZ80->AF.B.h = Z80_RD_BYTE(pZ80,pZ80->DE.W); 
	pZ80->MemPtr.W = pZ80->DE.W+1; 
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x01b:
{
/* DEC DE */
 
	--pZ80->DE.W;                
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x01c:
{
INC_R(pZ80->DE.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x01d:
{
DEC_R(pZ80->DE.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x01e:
{
 /* LD E,n */
pZ80->DE.B.l = Z80_RD_PC_BYTE(pZ80);
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x01f:
{

{ 
	Z80_BYTE	OrByte;	
	Z80_BYTE	Flags;	
	OrByte = (Z80_FLAGS_REG & 0x01)<<7;	
	Flags = Z80_FLAGS_REG;			
	Flags = Flags & (Z80_SIGN_FLAG | Z80_ZERO_FLAG | Z80_PARITY_FLAG);	
	Flags |= (pZ80->AF.B.h & 0x01); 
	{							
		Z80_BYTE	Reg;		
		Reg = pZ80->AF.B.h;			
		Reg = Reg>>1;			
		Reg = (Reg&0x07f)|OrByte;		
		pZ80->AF.B.h = Reg;			
		Reg &= (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
		Flags = Flags | Reg;	
	} 
	Z80_FLAGS_REG = Flags;			
}        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x020:
{
/* JR nz,dd */
Z80_BYTE_OFFSET Offset;
Offset = Z80_RD_PC_BYTE(pZ80);
if (Z80_TEST_ZERO_NOT_SET)
{
pZ80->MemPtr.W = pZ80->PC.W.l + Offset;
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles=3;
}
else
{
Cycles=2;
}
        pZ80->R+=1;
 }
break;
case 0x021:
{
/* LD HL,nnnn */
 
        pZ80->HL.W = Z80_RD_PC_WORD(pZ80); 
        pZ80->R+=1;
 Cycles = 3;
}
break;
case 0x022:
{
/* LD (nnnn),HL */
 	pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);
 	Z80_WR_WORD(pZ80,pZ80->MemPtr.W, pZ80->HL.W);
 	++pZ80->MemPtr.W;
        pZ80->R+=1;
 Cycles = 5;
}
break;
case 0x023:
{
/* INC HL */
 
    ++pZ80->HL.W;                
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x024:
{
INC_R(pZ80->HL.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x025:
{
DEC_R(pZ80->HL.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x026:
{
 /* LD H,n */
pZ80->HL.B.h = Z80_RD_PC_BYTE(pZ80);
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x027:
{
DAA(pZ80);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x028:
{
/* JR z,dd */
Z80_BYTE_OFFSET Offset;
Offset = Z80_RD_PC_BYTE(pZ80);
if (Z80_TEST_ZERO_SET)
{
pZ80->MemPtr.W = pZ80->PC.W.l + Offset;
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles=3;
}
else
{
Cycles=2;
}
        pZ80->R+=1;
 }
break;
case 0x029:
{
ADD_RR_rr(pZ80->HL.W,pZ80->HL.W);
        pZ80->R+=1;
 Cycles = 3;
}
break;
case 0x02a:
{
/* LD HL,(nnnn) */
 	pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);
 	pZ80->HL.W = Z80_RD_WORD(pZ80,pZ80->MemPtr.W);
 	++pZ80->MemPtr.W;
         pZ80->R+=1;
 Cycles = 5;
}
break;
case 0x02b:
{
/* DEC HL */
 
	--pZ80->HL.W;                
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x02c:
{
INC_R(pZ80->HL.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x02d:
{
DEC_R(pZ80->HL.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x02e:
{
 /* LD L,n */
pZ80->HL.B.l = Z80_RD_PC_BYTE(pZ80);
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x02f:
{
/* CPL */
				
	Z80_BYTE Flags;			
        /* complement */	
        pZ80->AF.B.h = (Z80_BYTE)(~pZ80->AF.B.h);	
											
		Flags = Z80_FLAGS_REG;					
		Flags = Flags & (Z80_SIGN_FLAG | Z80_ZERO_FLAG | Z80_PARITY_FLAG | Z80_CARRY_FLAG);	
		Flags |= pZ80->AF.B.h & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);	
		Flags |= Z80_SUBTRACT_FLAG | Z80_HALFCARRY_FLAG;			
        Z80_FLAGS_REG = Flags;											
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x030:
{
/* JR nc,dd */
Z80_BYTE_OFFSET Offset;
Offset = Z80_RD_PC_BYTE(pZ80);
if (Z80_TEST_CARRY_NOT_SET)
{
pZ80->MemPtr.W = pZ80->PC.W.l + Offset;
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles=3;
}
else
{
Cycles=2;
}
        pZ80->R+=1;
 }
break;
case 0x031:
{
/* LD SP,nnnn */
 
        pZ80->SP.W = Z80_RD_PC_WORD(pZ80); 
        pZ80->R+=1;
 Cycles = 3;
}
break;
case 0x032:
{
/* LD (nnnn),A */

	/* get memory address to read from and store in memptr */ 
	pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80); 
	/* write byte */ 
	Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->AF.B.h); 
	/* increment memory pointer */ 
	pZ80->MemPtr.B.l++; 
	/* and store a in upper byte */ 
	pZ80->MemPtr.B.h = pZ80->AF.B.h; 
        pZ80->R+=1;
 Cycles = 4;
}
break;
case 0x033:
{
/* INC SP */
 
    ++pZ80->SP.W;                
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x034:
{
INC_HL_();
        pZ80->R+=1;
 Cycles = 3;
}
break;
case 0x035:
{
DEC_HL_();
        pZ80->R+=1;
 Cycles = 3;
}
break;
case 0x036:
{
 /* LD (HL),n */
pZ80->TempByte = Z80_RD_PC_BYTE(pZ80);
Z80_WR_BYTE(pZ80,pZ80->HL.W,pZ80->TempByte);
        pZ80->R+=1;
 Cycles = 3;
}
break;
case 0x037:
{
/* SCF */
	
	Z80_BYTE	Flags;			
								
	Flags = Z80_FLAGS_REG;			
								
	Flags = Flags & (Z80_ZERO_FLAG | Z80_PARITY_FLAG | Z80_SIGN_FLAG);	
    Flags = Flags | Z80_CARRY_FLAG;										
	Flags |= pZ80->AF.B.h & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);			
																		
	Z80_FLAGS_REG = Flags;													
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x038:
{
/* JR c,dd */
Z80_BYTE_OFFSET Offset;
Offset = Z80_RD_PC_BYTE(pZ80);
if (Z80_TEST_CARRY_SET)
{
pZ80->MemPtr.W = pZ80->PC.W.l + Offset;
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles=3;
}
else
{
Cycles=2;
}
        pZ80->R+=1;
 }
break;
case 0x039:
{
ADD_RR_rr(pZ80->HL.W,pZ80->SP.W);
        pZ80->R+=1;
 Cycles = 3;
}
break;
case 0x03a:
{
/* LD A,(nnnn) */

	/* get memory address to read from */ 
	pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80); 
 
	/* read byte */ 
	pZ80->AF.B.h = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W); 
 
	/* increment memptr */ 
	++pZ80->MemPtr.W; 
        pZ80->R+=1;
 Cycles = 4;
}
break;
case 0x03b:
{
/* DEC SP */
 
	--pZ80->SP.W;                
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x03c:
{
INC_R(pZ80->AF.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x03d:
{
DEC_R(pZ80->AF.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x03e:
{
 /* LD A,n */
pZ80->AF.B.h = Z80_RD_PC_BYTE(pZ80);
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x03f:
{
/* CCF */
	
	Z80_BYTE Flags;				
								
	Flags = Z80_FLAGS_REG;			
    Flags &= (Z80_CARRY_FLAG | Z80_ZERO_FLAG | Z80_PARITY_FLAG | Z80_SIGN_FLAG);	
	Flags |= ((Flags & Z80_CARRY_FLAG)<<Z80_HALFCARRY_FLAG_BIT);					
	Flags |= pZ80->AF.B.h & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);						
    Z80_FLAGS_REG = Flags ^ Z80_CARRY_FLAG;												
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x040:
{
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x041:
{
/* LD B,C */
 
		pZ80->BC.B.h = pZ80->BC.B.l; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x042:
{
/* LD B,D */
 
		pZ80->BC.B.h = pZ80->DE.B.h; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x043:
{
/* LD B,E */
 
		pZ80->BC.B.h = pZ80->DE.B.l; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x044:
{
/* LD B,H */
 
		pZ80->BC.B.h = pZ80->HL.B.h; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x045:
{
/* LD B,L */
 
		pZ80->BC.B.h = pZ80->HL.B.l; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x046:
{
/* LD B,(HL) */
 
        pZ80->BC.B.h = Z80_RD_BYTE(pZ80,pZ80->HL.W); 
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x047:
{
/* LD B,A */
 
		pZ80->BC.B.h = pZ80->AF.B.h; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x048:
{
/* LD C,B */
 
		pZ80->BC.B.l = pZ80->BC.B.h; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x049:
{
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x04a:
{
/* LD C,D */
 
		pZ80->BC.B.l = pZ80->DE.B.h; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x04b:
{
/* LD C,E */
 
		pZ80->BC.B.l = pZ80->DE.B.l; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x04c:
{
/* LD C,H */
 
		pZ80->BC.B.l = pZ80->HL.B.h; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x04d:
{
/* LD C,L */
 
		pZ80->BC.B.l = pZ80->HL.B.l; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x04e:
{
/* LD C,(HL) */
 
        pZ80->BC.B.l = Z80_RD_BYTE(pZ80,pZ80->HL.W); 
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x04f:
{
/* LD C,A */
 
		pZ80->BC.B.l = pZ80->AF.B.h; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x050:
{
/* LD D,B */
 
		pZ80->DE.B.h = pZ80->BC.B.h; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x051:
{
/* LD D,C */
 
		pZ80->DE.B.h = pZ80->BC.B.l; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x052:
{
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x053:
{
/* LD D,E */
 
		pZ80->DE.B.h = pZ80->DE.B.l; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x054:
{
/* LD D,H */
 
		pZ80->DE.B.h = pZ80->HL.B.h; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x055:
{
/* LD D,L */
 
		pZ80->DE.B.h = pZ80->HL.B.l; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x056:
{
/* LD D,(HL) */
 
        pZ80->DE.B.h = Z80_RD_BYTE(pZ80,pZ80->HL.W); 
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x057:
{
/* LD D,A */
 
		pZ80->DE.B.h = pZ80->AF.B.h; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x058:
{
/* LD E,B */
 
		pZ80->DE.B.l = pZ80->BC.B.h; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x059:
{
/* LD E,C */
 
		pZ80->DE.B.l = pZ80->BC.B.l; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x05a:
{
/* LD E,D */
 
		pZ80->DE.B.l = pZ80->DE.B.h; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x05b:
{
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x05c:
{
/* LD E,H */
 
		pZ80->DE.B.l = pZ80->HL.B.h; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x05d:
{
/* LD E,L */
 
		pZ80->DE.B.l = pZ80->HL.B.l; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x05e:
{
/* LD E,(HL) */
 
        pZ80->DE.B.l = Z80_RD_BYTE(pZ80,pZ80->HL.W); 
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x05f:
{
/* LD E,A */
 
		pZ80->DE.B.l = pZ80->AF.B.h; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x060:
{
/* LD H,B */
 
		pZ80->HL.B.h = pZ80->BC.B.h; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x061:
{
/* LD H,C */
 
		pZ80->HL.B.h = pZ80->BC.B.l; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x062:
{
/* LD H,D */
 
		pZ80->HL.B.h = pZ80->DE.B.h; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x063:
{
/* LD H,E */
 
		pZ80->HL.B.h = pZ80->DE.B.l; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x064:
{
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x065:
{
/* LD H,L */
 
		pZ80->HL.B.h = pZ80->HL.B.l; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x066:
{
/* LD H,(HL) */
 
        pZ80->HL.B.h = Z80_RD_BYTE(pZ80,pZ80->HL.W); 
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x067:
{
/* LD H,A */
 
		pZ80->HL.B.h = pZ80->AF.B.h; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x068:
{
/* LD L,B */
 
		pZ80->HL.B.l = pZ80->BC.B.h; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x069:
{
/* LD L,C */
 
		pZ80->HL.B.l = pZ80->BC.B.l; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x06a:
{
/* LD L,D */
 
		pZ80->HL.B.l = pZ80->DE.B.h; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x06b:
{
/* LD L,E */
 
		pZ80->HL.B.l = pZ80->DE.B.l; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x06c:
{
/* LD L,H */
 
		pZ80->HL.B.l = pZ80->HL.B.h; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x06d:
{
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x06e:
{
/* LD L,(HL) */
 
        pZ80->HL.B.l = Z80_RD_BYTE(pZ80,pZ80->HL.W); 
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x06f:
{
/* LD L,A */
 
		pZ80->HL.B.l = pZ80->AF.B.h; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x070:
{
/* LD (HL),B */
 
        Z80_WR_BYTE(pZ80,pZ80->HL.W,pZ80->BC.B.h); 
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x071:
{
/* LD (HL),C */
 
        Z80_WR_BYTE(pZ80,pZ80->HL.W,pZ80->BC.B.l); 
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x072:
{
/* LD (HL),D */
 
        Z80_WR_BYTE(pZ80,pZ80->HL.W,pZ80->DE.B.h); 
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x073:
{
/* LD (HL),E */
 
        Z80_WR_BYTE(pZ80,pZ80->HL.W,pZ80->DE.B.l); 
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x074:
{
/* LD (HL),H */
 
        Z80_WR_BYTE(pZ80,pZ80->HL.W,pZ80->HL.B.h); 
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x075:
{
/* LD (HL),L */
 
        Z80_WR_BYTE(pZ80,pZ80->HL.W,pZ80->HL.B.l); 
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x076:
{
pZ80->Outputs |= Z80_OUTPUT_HALT;
pZ80->Flags |=Z80_EXECUTING_HALT_FLAG;
pZ80->PC.W.l--;
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x077:
{
/* LD (HL),A */
	Z80_WR_BYTE(pZ80, pZ80->HL.W, pZ80->AF.B.h);
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x078:
{
/* LD A,B */
 
		pZ80->AF.B.h = pZ80->BC.B.h; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x079:
{
/* LD A,C */
 
		pZ80->AF.B.h = pZ80->BC.B.l; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x07a:
{
/* LD A,D */
 
		pZ80->AF.B.h = pZ80->DE.B.h; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x07b:
{
/* LD A,E */
 
		pZ80->AF.B.h = pZ80->DE.B.l; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x07c:
{
/* LD A,H */
 
		pZ80->AF.B.h = pZ80->HL.B.h; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x07d:
{
/* LD A,L */
 
		pZ80->AF.B.h = pZ80->HL.B.l; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x07e:
{
/* LD A,(HL) */
 
        pZ80->AF.B.h = Z80_RD_BYTE(pZ80,pZ80->HL.W); 
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x07f:
{
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x080:
{
ADD_A_R(pZ80->BC.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x081:
{
ADD_A_R(pZ80->BC.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x082:
{
ADD_A_R(pZ80->DE.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x083:
{
ADD_A_R(pZ80->DE.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x084:
{
ADD_A_R(pZ80->HL.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x085:
{
ADD_A_R(pZ80->HL.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x086:
{
ADD_A_HL(pZ80);
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x087:
{
ADD_A_R(pZ80->AF.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x088:
{
ADC_A_R(pZ80->BC.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x089:
{
ADC_A_R(pZ80->BC.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x08a:
{
ADC_A_R(pZ80->DE.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x08b:
{
ADC_A_R(pZ80->DE.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x08c:
{
ADC_A_R(pZ80->HL.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x08d:
{
ADC_A_R(pZ80->HL.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x08e:
{
ADC_A_HL(pZ80);
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x08f:
{
ADC_A_R(pZ80->AF.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x090:
{
SUB_A_R(pZ80->BC.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x091:
{
SUB_A_R(pZ80->BC.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x092:
{
SUB_A_R(pZ80->DE.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x093:
{
SUB_A_R(pZ80->DE.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x094:
{
SUB_A_R(pZ80->HL.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x095:
{
SUB_A_R(pZ80->HL.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x096:
{
SUB_A_HL(pZ80);
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x097:
{
Z80_BYTE Flags;
pZ80->AF.B.h = 0;
Flags = Z80_ZERO_FLAG | Z80_SUBTRACT_FLAG;
Z80_FLAGS_REG = Flags;
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x098:
{
SBC_A_R(pZ80->BC.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x099:
{
SBC_A_R(pZ80->BC.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x09a:
{
SBC_A_R(pZ80->DE.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x09b:
{
SBC_A_R(pZ80->DE.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x09c:
{
SBC_A_R(pZ80->HL.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x09d:
{
SBC_A_R(pZ80->HL.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x09e:
{
SBC_A_HL(pZ80);
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x09f:
{
SBC_A_R(pZ80->AF.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0a0:
{
AND_A_R(pZ80->BC.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0a1:
{
AND_A_R(pZ80->BC.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0a2:
{
AND_A_R(pZ80->DE.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0a3:
{
AND_A_R(pZ80->DE.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0a4:
{
AND_A_R(pZ80->HL.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0a5:
{
AND_A_R(pZ80->HL.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0a6:
{
AND_A_HL(pZ80);
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x0a7:
{
Z80_BYTE Flags;
Flags = pZ80->AF.B.h & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);
Flags |= Z80_HALFCARRY_FLAG;
Flags |= ZeroSignParityTable[pZ80->AF.B.h];
Z80_FLAGS_REG = Flags;
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0a8:
{
XOR_A_R(pZ80->BC.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0a9:
{
XOR_A_R(pZ80->BC.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0aa:
{
XOR_A_R(pZ80->DE.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0ab:
{
XOR_A_R(pZ80->DE.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0ac:
{
XOR_A_R(pZ80->HL.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0ad:
{
XOR_A_R(pZ80->HL.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0ae:
{
XOR_A_HL(pZ80);
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x0af:
{
Z80_BYTE Flags;
pZ80->AF.B.h=0;
Flags = Z80_ZERO_FLAG | Z80_PARITY_FLAG;
Z80_FLAGS_REG = Flags;
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0b0:
{
OR_A_R(pZ80->BC.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0b1:
{
OR_A_R(pZ80->BC.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0b2:
{
OR_A_R(pZ80->DE.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0b3:
{
OR_A_R(pZ80->DE.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0b4:
{
OR_A_R(pZ80->HL.B.h);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0b5:
{
OR_A_R(pZ80->HL.B.l);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0b6:
{
OR_A_HL(pZ80);
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x0b7:
{
Z80_BYTE Flags;
Flags = pZ80->AF.B.h & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);
Flags |= ZeroSignParityTable[pZ80->AF.B.h];
Z80_FLAGS_REG = Flags;
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0b8:
{
/* CP A,B */

{
    Z80_WORD Result;
    Z80_FLAGS_REG &= ~(Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);  
    Z80_FLAGS_REG |= pZ80->BC.B.h & (Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2); 
    Z80_FLAGS_REG |= Z80_SUBTRACT_FLAG;              
    Result = (Z80_WORD)pZ80->AF.B.h - (Z80_WORD)pZ80->BC.B.h;
    SET_OVERFLOW_FLAG_A_SUB(pZ80->BC.B.h,Result); 
    SET_ZERO_SIGN(Result);                  
    SET_CARRY_FLAG_SUB(Result);                     
    SET_HALFCARRY(pZ80->BC.B.h, Result);
}
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0b9:
{
/* CP A,C */

{
    Z80_WORD Result;
    Z80_FLAGS_REG &= ~(Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);  
    Z80_FLAGS_REG |= pZ80->BC.B.l & (Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2); 
    Z80_FLAGS_REG |= Z80_SUBTRACT_FLAG;              
    Result = (Z80_WORD)pZ80->AF.B.h - (Z80_WORD)pZ80->BC.B.l;
    SET_OVERFLOW_FLAG_A_SUB(pZ80->BC.B.l,Result); 
    SET_ZERO_SIGN(Result);                  
    SET_CARRY_FLAG_SUB(Result);                     
    SET_HALFCARRY(pZ80->BC.B.l, Result);
}
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0ba:
{
/* CP A,D */

{
    Z80_WORD Result;
    Z80_FLAGS_REG &= ~(Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);  
    Z80_FLAGS_REG |= pZ80->DE.B.h & (Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2); 
    Z80_FLAGS_REG |= Z80_SUBTRACT_FLAG;              
    Result = (Z80_WORD)pZ80->AF.B.h - (Z80_WORD)pZ80->DE.B.h;
    SET_OVERFLOW_FLAG_A_SUB(pZ80->DE.B.h,Result); 
    SET_ZERO_SIGN(Result);                  
    SET_CARRY_FLAG_SUB(Result);                     
    SET_HALFCARRY(pZ80->DE.B.h, Result);
}
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0bb:
{
/* CP A,E */

{
    Z80_WORD Result;
    Z80_FLAGS_REG &= ~(Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);  
    Z80_FLAGS_REG |= pZ80->DE.B.l & (Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2); 
    Z80_FLAGS_REG |= Z80_SUBTRACT_FLAG;              
    Result = (Z80_WORD)pZ80->AF.B.h - (Z80_WORD)pZ80->DE.B.l;
    SET_OVERFLOW_FLAG_A_SUB(pZ80->DE.B.l,Result); 
    SET_ZERO_SIGN(Result);                  
    SET_CARRY_FLAG_SUB(Result);                     
    SET_HALFCARRY(pZ80->DE.B.l, Result);
}
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0bc:
{
/* CP A,H */

{
    Z80_WORD Result;
    Z80_FLAGS_REG &= ~(Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);  
    Z80_FLAGS_REG |= pZ80->HL.B.h & (Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2); 
    Z80_FLAGS_REG |= Z80_SUBTRACT_FLAG;              
    Result = (Z80_WORD)pZ80->AF.B.h - (Z80_WORD)pZ80->HL.B.h;
    SET_OVERFLOW_FLAG_A_SUB(pZ80->HL.B.h,Result); 
    SET_ZERO_SIGN(Result);                  
    SET_CARRY_FLAG_SUB(Result);                     
    SET_HALFCARRY(pZ80->HL.B.h, Result);
}
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0bd:
{
/* CP A,L */

{
    Z80_WORD Result;
    Z80_FLAGS_REG &= ~(Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);  
    Z80_FLAGS_REG |= pZ80->HL.B.l & (Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2); 
    Z80_FLAGS_REG |= Z80_SUBTRACT_FLAG;              
    Result = (Z80_WORD)pZ80->AF.B.h - (Z80_WORD)pZ80->HL.B.l;
    SET_OVERFLOW_FLAG_A_SUB(pZ80->HL.B.l,Result); 
    SET_ZERO_SIGN(Result);                  
    SET_CARRY_FLAG_SUB(Result);                     
    SET_HALFCARRY(pZ80->HL.B.l, Result);
}
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0be:
{
CP_A_HL(pZ80);
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x0bf:
{
Z80_BYTE Flags;
Flags = pZ80->AF.B.h & (Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);
Flags |= Z80_ZERO_FLAG | Z80_SUBTRACT_FLAG;
Z80_FLAGS_REG = Flags;
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0c0:
{
/* RET nz */
if (Z80_TEST_ZERO_NOT_SET)
{
RETURN();
Cycles=4;
}
else
{
Cycles=2-1;
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
}
        pZ80->R+=1;
 }
break;
case 0x0c1:
{
/* POP BC */
pZ80->BC.W = Z80_POP_WORD(pZ80);
        pZ80->R+=1;
 Cycles = 3;
}
break;
case 0x0c2:
{
/* JP nz,nnnn */
if (Z80_TEST_ZERO_NOT_SET)
{
pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
}
else
{
	Z80_RD_PC_WORD(pZ80);
}
Cycles=3;
        pZ80->R+=1;
 }
break;
case 0x0c3:
{
/* JP nnnn */
pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
        pZ80->R+=1;
 Cycles = 3;
}
break;
case 0x0c4:
{
/* CALL nz,nnnn */
if (Z80_TEST_ZERO_NOT_SET)
{
pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);
Z80_PUSH_PC(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles=5;
}
else
{
	Z80_RD_PC_WORD(pZ80);
Cycles=3;
}
        pZ80->R+=1;
 }
break;
case 0x0c5:
{
/* PUSH BC */
Z80_PUSH_WORD(pZ80, pZ80->BC.W);
        pZ80->R+=1;
 Cycles = 4;
}
break;
case 0x0c6:
{
pZ80->TempByte = Z80_RD_PC_BYTE(pZ80);
ADD_A_X(pZ80->TempByte);
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x0c7:
{
/* RST 0x0000 */
	
/* push return address on stack */	
    Z80_PUSH_PC(pZ80); 
/* set memptr to address */	
pZ80->MemPtr.W = 0x0000;	
/* set program counter to memptr */ 
pZ80->PC.W.l = pZ80->MemPtr.W; 
        pZ80->R+=1;
 Cycles = 4;
}
break;
case 0x0c8:
{
/* RET z */
if (Z80_TEST_ZERO_SET)
{
RETURN();
Cycles=4;
}
else
{
Cycles=2-1;
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
}
        pZ80->R+=1;
 }
break;
case 0x0c9:
{
RETURN();
        pZ80->R+=1;
 Cycles = 3;
}
break;
case 0x0ca:
{
/* JP z,nnnn */
if (Z80_TEST_ZERO_SET)
{
pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
}
else
{
	Z80_RD_PC_WORD(pZ80);
}
Cycles=3;
        pZ80->R+=1;
 }
break;
case 0x0cb:
{
Cycles = Z80_CB_ExecuteInstruction(pZ80);
}
break;
case 0x0cc:
{
/* CALL z,nnnn */
if (Z80_TEST_ZERO_SET)
{
pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);
Z80_PUSH_PC(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles=5;
}
else
{
	Z80_RD_PC_WORD(pZ80);
Cycles=3;
}
        pZ80->R+=1;
 }
break;
case 0x0cd:
{
/* CALL nnnn */
pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);
Z80_PUSH_PC(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
        pZ80->R+=1;
 Cycles = 5;
}
break;
case 0x0ce:
{
pZ80->TempByte = Z80_RD_PC_BYTE(pZ80);
ADC_A_X(pZ80->TempByte);
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x0cf:
{
/* RST 0x0008 */
	
/* push return address on stack */	
    Z80_PUSH_PC(pZ80); 
/* set memptr to address */	
pZ80->MemPtr.W = 0x0008;	
/* set program counter to memptr */ 
pZ80->PC.W.l = pZ80->MemPtr.W; 
        pZ80->R+=1;
 Cycles = 4;
}
break;
case 0x0d0:
{
/* RET nc */
if (Z80_TEST_CARRY_NOT_SET)
{
RETURN();
Cycles=4;
}
else
{
Cycles=2-1;
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
}
        pZ80->R+=1;
 }
break;
case 0x0d1:
{
/* POP DE */
pZ80->DE.W = Z80_POP_WORD(pZ80);
        pZ80->R+=1;
 Cycles = 3;
}
break;
case 0x0d2:
{
/* JP nc,nnnn */
if (Z80_TEST_CARRY_NOT_SET)
{
pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
}
else
{
	Z80_RD_PC_WORD(pZ80);
}
Cycles=3;
        pZ80->R+=1;
 }
break;
case 0x0d3:
{
/* OUT (n),A */
    /* A in upper byte of port, Data in lower byte of port */ 
    pZ80->MemPtr.B.l = Z80_RD_PC_BYTE(pZ80); 
	pZ80->MemPtr.B.h = pZ80->AF.B.h; 
	pZ80->IOPort = pZ80->MemPtr.W;
	pZ80->IOData = pZ80->AF.B.h;
	/* perform out */ 
    Z80_DoOut(pZ80->IOPort, pZ80->IOData); 
	/* update mem ptr */ 
	pZ80->MemPtr.B.l++; 
	pZ80->MemPtr.B.h = pZ80->AF.B.h; 
	/* no flags changed */ 
	Cycles = 3;
        pZ80->R+=1;
 }
break;
case 0x0d4:
{
/* CALL nc,nnnn */
if (Z80_TEST_CARRY_NOT_SET)
{
pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);
Z80_PUSH_PC(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles=5;
}
else
{
	Z80_RD_PC_WORD(pZ80);
Cycles=3;
}
        pZ80->R+=1;
 }
break;
case 0x0d5:
{
/* PUSH DE */
Z80_PUSH_WORD(pZ80, pZ80->DE.W);
        pZ80->R+=1;
 Cycles = 4;
}
break;
case 0x0d6:
{
pZ80->TempByte = Z80_RD_PC_BYTE(pZ80);
SUB_A_X(pZ80->TempByte);
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x0d7:
{
/* RST 0x0010 */
	
/* push return address on stack */	
    Z80_PUSH_PC(pZ80); 
/* set memptr to address */	
pZ80->MemPtr.W = 0x0010;	
/* set program counter to memptr */ 
pZ80->PC.W.l = pZ80->MemPtr.W; 
        pZ80->R+=1;
 Cycles = 4;
}
break;
case 0x0d8:
{
/* RET c */
if (Z80_TEST_CARRY_SET)
{
RETURN();
Cycles=4;
}
else
{
Cycles=2-1;
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
}
        pZ80->R+=1;
 }
break;
case 0x0d9:
{
SWAP(pZ80->DE.W, pZ80->altDE.W);
SWAP(pZ80->HL.W, pZ80->altHL.W);
SWAP(pZ80->BC.W, pZ80->altBC.W);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0da:
{
/* JP c,nnnn */
if (Z80_TEST_CARRY_SET)
{
pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
}
else
{
	Z80_RD_PC_WORD(pZ80);
}
Cycles=3;
        pZ80->R+=1;
 }
break;
case 0x0db:
{
/* IN A,(n) */
 
    /* A in upper byte of port, Data in lower byte of port */
	pZ80->MemPtr.B.l = Z80_RD_PC_BYTE(pZ80);
	pZ80->MemPtr.B.h = pZ80->AF.B.h;
	pZ80->IOPort = pZ80->MemPtr.W;
    /* a in upper byte of port, data in lower byte of port */
    pZ80->AF.B.h = pZ80->IOData = Z80_DoIn(pZ80->IOPort);
	/* update mem ptr */
	pZ80->MemPtr.W++;
	/* no flags changed */
	Cycles = 3;
        pZ80->R+=1;
 }
break;
case 0x0dc:
{
/* CALL c,nnnn */
if (Z80_TEST_CARRY_SET)
{
pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);
Z80_PUSH_PC(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles=5;
}
else
{
	Z80_RD_PC_WORD(pZ80);
Cycles=3;
}
        pZ80->R+=1;
 }
break;
case 0x0dd:
{
Cycles = Z80_DD_ExecuteInstruction(pZ80);
}
break;
case 0x0de:
{
pZ80->TempByte = Z80_RD_PC_BYTE(pZ80);
SBC_A_X(pZ80->TempByte);
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x0df:
{
/* RST 0x0018 */
	
/* push return address on stack */	
    Z80_PUSH_PC(pZ80); 
/* set memptr to address */	
pZ80->MemPtr.W = 0x0018;	
/* set program counter to memptr */ 
pZ80->PC.W.l = pZ80->MemPtr.W; 
        pZ80->R+=1;
 Cycles = 4;
}
break;
case 0x0e0:
{
/* RET po */
if (Z80_TEST_PARITY_ODD)
{
RETURN();
Cycles=4;
}
else
{
Cycles=2-1;
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
}
        pZ80->R+=1;
 }
break;
case 0x0e1:
{
/* POP HL */
pZ80->HL.W = Z80_POP_WORD(pZ80);
        pZ80->R+=1;
 Cycles = 3;
}
break;
case 0x0e2:
{
/* JP po,nnnn */
if (Z80_TEST_PARITY_ODD)
{
pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
}
else
{
	Z80_RD_PC_WORD(pZ80);
}
Cycles=3;
        pZ80->R+=1;
 }
break;
case 0x0e3:
{
 
        pZ80->MemPtr.W = Z80_RD_WORD(pZ80,pZ80->SP.W); 
        Z80_WR_WORD(pZ80,pZ80->SP.W, pZ80->HL.W);    
        pZ80->HL.W = pZ80->MemPtr.W; 
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG; 
        pZ80->R+=1;
 Cycles = 5;
}
break;
case 0x0e4:
{
/* CALL po,nnnn */
if (Z80_TEST_PARITY_ODD)
{
pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);
Z80_PUSH_PC(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles=5;
}
else
{
	Z80_RD_PC_WORD(pZ80);
Cycles=3;
}
        pZ80->R+=1;
 }
break;
case 0x0e5:
{
/* PUSH HL */
Z80_PUSH_WORD(pZ80, pZ80->HL.W);
        pZ80->R+=1;
 Cycles = 4;
}
break;
case 0x0e6:
{
pZ80->TempByte = Z80_RD_PC_BYTE(pZ80);
AND_A_X(pZ80->TempByte);
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x0e7:
{
/* RST 0x0020 */
	
/* push return address on stack */	
    Z80_PUSH_PC(pZ80); 
/* set memptr to address */	
pZ80->MemPtr.W = 0x0020;	
/* set program counter to memptr */ 
pZ80->PC.W.l = pZ80->MemPtr.W; 
        pZ80->R+=1;
 Cycles = 4;
}
break;
case 0x0e8:
{
/* RET pe */
if (Z80_TEST_PARITY_EVEN)
{
RETURN();
Cycles=4;
}
else
{
Cycles=2-1;
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
}
        pZ80->R+=1;
 }
break;
case 0x0e9:
{
/* JP (HL) */

    pZ80->PC.W.l=pZ80->HL.W; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0ea:
{
/* JP pe,nnnn */
if (Z80_TEST_PARITY_EVEN)
{
pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
}
else
{
	Z80_RD_PC_WORD(pZ80);
}
Cycles=3;
        pZ80->R+=1;
 }
break;
case 0x0eb:
{
SWAP(pZ80->HL.W,pZ80->DE.W);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0ec:
{
/* CALL pe,nnnn */
if (Z80_TEST_PARITY_EVEN)
{
pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);
Z80_PUSH_PC(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles=5;
}
else
{
	Z80_RD_PC_WORD(pZ80);
Cycles=3;
}
        pZ80->R+=1;
 }
break;
case 0x0ed:
{
Cycles = Z80_ED_ExecuteInstruction(pZ80);
}
break;
case 0x0ee:
{
pZ80->TempByte = Z80_RD_PC_BYTE(pZ80);
XOR_A_X(pZ80->TempByte);
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x0ef:
{
/* RST 0x0028 */
	
/* push return address on stack */	
    Z80_PUSH_PC(pZ80); 
/* set memptr to address */	
pZ80->MemPtr.W = 0x0028;	
/* set program counter to memptr */ 
pZ80->PC.W.l = pZ80->MemPtr.W; 
        pZ80->R+=1;
 Cycles = 4;
}
break;
case 0x0f0:
{
/* RET p */
if (Z80_TEST_POSITIVE)
{
RETURN();
Cycles=4;
}
else
{
Cycles=2-1;
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
}
        pZ80->R+=1;
 }
break;
case 0x0f1:
{
/* POP AF */
pZ80->AF.W = Z80_POP_WORD(pZ80);
        pZ80->R+=1;
 Cycles = 3;
}
break;
case 0x0f2:
{
/* JP p,nnnn */
if (Z80_TEST_POSITIVE)
{
pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
}
else
{
	Z80_RD_PC_WORD(pZ80);
}
Cycles=3;
        pZ80->R+=1;
 }
break;
case 0x0f3:
{
/* DI */

        pZ80->IFF1 = pZ80->IFF2 = 0; 
        pZ80->Flags |=Z80_NO_CHECK_INTERRUPT_FLAG;	
        Z80_RefreshInterruptRequest(pZ80);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0f4:
{
/* CALL p,nnnn */
if (Z80_TEST_POSITIVE)
{
pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);
Z80_PUSH_PC(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles=5;
}
else
{
	Z80_RD_PC_WORD(pZ80);
Cycles=3;
}
        pZ80->R+=1;
 }
break;
case 0x0f5:
{
/* PUSH AF */
Z80_PUSH_WORD(pZ80, pZ80->AF.W);
        pZ80->R+=1;
 Cycles = 4;
}
break;
case 0x0f6:
{
pZ80->TempByte = Z80_RD_PC_BYTE(pZ80);
OR_A_X(pZ80->TempByte);
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x0f7:
{
/* RST 0x0030 */
	
/* push return address on stack */	
    Z80_PUSH_PC(pZ80); 
/* set memptr to address */	
pZ80->MemPtr.W = 0x0030;	
/* set program counter to memptr */ 
pZ80->PC.W.l = pZ80->MemPtr.W; 
        pZ80->R+=1;
 Cycles = 4;
}
break;
case 0x0f8:
{
/* RET m */
if (Z80_TEST_MINUS)
{
RETURN();
Cycles=4;
}
else
{
Cycles=2-1;
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
}
        pZ80->R+=1;
 }
break;
case 0x0f9:
{
/* LD SP,HL */

    pZ80->SP.W=pZ80->HL.W; 
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG; 
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0fa:
{
/* JP m,nnnn */
if (Z80_TEST_MINUS)
{
pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
}
else
{
	Z80_RD_PC_WORD(pZ80);
}
Cycles=3;
        pZ80->R+=1;
 }
break;
case 0x0fb:
{
/* EI */

        pZ80->IFF1 = pZ80->IFF2 = 1; 
        pZ80->Flags |=Z80_NO_CHECK_INTERRUPT_FLAG; 
        Z80_RefreshInterruptRequest(pZ80);
        pZ80->R+=1;
 Cycles = 1;
}
break;
case 0x0fc:
{
/* CALL m,nnnn */
if (Z80_TEST_MINUS)
{
pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80);
Z80_PUSH_PC(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles=5;
}
else
{
	Z80_RD_PC_WORD(pZ80);
Cycles=3;
}
        pZ80->R+=1;
 }
break;
case 0x0fd:
{
Cycles = Z80_FD_ExecuteInstruction(pZ80);
}
break;
case 0x0fe:
{
pZ80->TempByte = Z80_RD_PC_BYTE(pZ80);
CP_A_X(pZ80->TempByte);
        pZ80->R+=1;
 Cycles = 2;
}
break;
case 0x0ff:
{
/* RST 0x0038 */
	
/* push return address on stack */	
    Z80_PUSH_PC(pZ80); 
/* set memptr to address */	
pZ80->MemPtr.W = 0x0038;	
/* set program counter to memptr */ 
pZ80->PC.W.l = pZ80->MemPtr.W; 
        pZ80->R+=1;
 Cycles = 4;
}
break;
default:
/* the following tells MSDEV 6 to not generate */
/* code which checks if a input value to the  */
/* switch is not valid.*/
#ifdef _MSC_VER
#if (_MSC_VER>=1200)
	Cycles=0;
	__assume(0);
#endif
#else
	Cycles=0;
#endif
break;
}
return Cycles;
}
/***************************************************************************/
int Z80_Execute(Z80_REGISTERS *pZ80)
{
unsigned long Cycles;
if ((pZ80->Flags & Z80_INTERRUPT_CHECK)!=0)
{
   /* if there is an nmi, execute it*/
    if ((pZ80->Flags & Z80_INTERRUPT_NMI_FLAG)!=0)
    {
      Cycles = Z80_ExecuteNMI(pZ80);
    }
    else
    {
       /* after an EI we should ignore interrupt */
       if ((pZ80->Flags & Z80_NO_CHECK_INTERRUPT_FLAG)!=0)
       {
			int nPreFlags = pZ80->Flags & Z80_INTERRUPT_NO_DELAY_FLAG;
			pZ80->Flags &= ~Z80_INTERRUPT_NO_DELAY_FLAG;
          pZ80->Flags &=~Z80_NO_CHECK_INTERRUPT_FLAG;
	  Z80_RefreshInterruptRequest(pZ80);
          Cycles = Z80_ExecuteInstruction(pZ80);
			if (nPreFlags != 0)
			{
				Cycles++;
			}
       }
       else
       {
         /* execute maskable interrupt */
         Cycles = Z80_ExecuteInterrupt(pZ80);
       }
    }
}
else
{
	int nPreFlags = pZ80->Flags & Z80_INTERRUPT_NO_DELAY_FLAG;
	pZ80->Flags &= ~Z80_INTERRUPT_NO_DELAY_FLAG;
  Cycles = Z80_ExecuteInstruction(pZ80);
	if (nPreFlags != 0)
	{
		Cycles++;
	}
}
return Cycles;
}
/***************************************************************************/
int Z80_ExecuteIM0(Z80_REGISTERS *pZ80)
{
unsigned long Opcode;
unsigned long Cycles;
Opcode = pZ80->InterruptVectorBase;
Opcode = Opcode & 0x0ff;
switch (Opcode)
{
case 0x001:
case 0x003:
case 0x005:
case 0x007:
case 0x009:
case 0x00b:
case 0x00d:
case 0x00f:
case 0x011:
case 0x013:
case 0x015:
case 0x017:
case 0x019:
case 0x01b:
case 0x01d:
case 0x01f:
case 0x021:
case 0x023:
case 0x025:
case 0x027:
case 0x029:
case 0x02b:
case 0x02d:
case 0x02f:
case 0x031:
case 0x033:
case 0x035:
case 0x037:
case 0x039:
case 0x03b:
case 0x03d:
case 0x03f:
case 0x041:
case 0x043:
case 0x045:
case 0x047:
case 0x049:
case 0x04b:
case 0x04d:
case 0x04f:
case 0x051:
case 0x053:
case 0x055:
case 0x057:
case 0x059:
case 0x05b:
case 0x05d:
case 0x05f:
case 0x061:
case 0x063:
case 0x065:
case 0x067:
case 0x069:
case 0x06b:
case 0x06d:
case 0x06f:
case 0x071:
case 0x073:
case 0x075:
case 0x077:
case 0x079:
case 0x07b:
case 0x07d:
case 0x07f:
case 0x081:
case 0x083:
case 0x085:
case 0x087:
case 0x089:
case 0x08b:
case 0x08d:
case 0x08f:
case 0x091:
case 0x093:
case 0x095:
case 0x097:
case 0x099:
case 0x09b:
case 0x09d:
case 0x09f:
case 0x0a1:
case 0x0a3:
case 0x0a5:
case 0x0a7:
case 0x0a9:
case 0x0ab:
case 0x0ad:
case 0x0af:
case 0x0b1:
case 0x0b3:
case 0x0b5:
case 0x0b7:
case 0x0b9:
case 0x0bb:
case 0x0bd:
case 0x0bf:
case 0x0c1:
case 0x0c3:
case 0x0c5:
case 0x0c7:
case 0x0c9:
case 0x0cb:
case 0x0cd:
case 0x0cf:
case 0x0d1:
case 0x0d3:
case 0x0d5:
case 0x0d7:
case 0x0d9:
case 0x0db:
case 0x0dd:
case 0x0df:
case 0x0e1:
case 0x0e3:
case 0x0e5:
case 0x0e7:
case 0x0e9:
case 0x0eb:
case 0x0ed:
case 0x0ef:
case 0x0f1:
case 0x0f3:
case 0x0f5:
case 0x0f7:
case 0x0f9:
case 0x0fb:
case 0x0fd:
{
Cycles = 0;
}
break;
case 0x000:
{
Cycles = 1;
}
break;
case 0x002:
{
/* LD (BC),A */
 
    Z80_WR_BYTE(pZ80,pZ80->BC.W,pZ80->AF.B.h); 
	pZ80->MemPtr.B.l = (pZ80->BC.W+1) & 0x0ff; 
	pZ80->MemPtr.B.h = pZ80->AF.B.h; 
Cycles = 2;
}
break;
case 0x004:
{
INC_R(pZ80->BC.B.h);
Cycles = 1;
}
break;
case 0x006:
{
 /* LD B,n */
pZ80->BC.B.h = Z80_RD_PC_BYTE(pZ80);
Cycles = 2;
}
break;
case 0x008:
{
SWAP(pZ80->AF.W,pZ80->altAF.W);
Cycles = 1;
}
break;
case 0x00a:
{
/* LD A,(BC) */
 
    pZ80->AF.B.h = Z80_RD_BYTE(pZ80,pZ80->BC.W); 
	pZ80->MemPtr.W = pZ80->BC.W+1; 
Cycles = 2;
}
break;
case 0x00c:
{
INC_R(pZ80->BC.B.l);
Cycles = 1;
}
break;
case 0x00e:
{
 /* LD C,n */
pZ80->BC.B.l = Z80_RD_PC_BYTE(pZ80);
Cycles = 2;
}
break;
case 0x010:
{
/* DJNZ dd */
Z80_BYTE_OFFSET Offset;
Offset = Z80_RD_BYTE_IM0();
pZ80->BC.B.h--;
if (pZ80->BC.B.h==0)
{
Cycles = 3;
}
else
{
pZ80->MemPtr.W = pZ80->PC.W.l + Offset;
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles = 4;
}
}
break;
case 0x012:
{
/* LD (DE),A */
 
    Z80_WR_BYTE(pZ80,pZ80->DE.W,pZ80->AF.B.h); 
	pZ80->MemPtr.B.l = (pZ80->DE.W+1) & 0x0ff; 
	pZ80->MemPtr.B.h = pZ80->AF.B.h; 
Cycles = 2;
}
break;
case 0x014:
{
INC_R(pZ80->DE.B.h);
Cycles = 1;
}
break;
case 0x016:
{
 /* LD D,n */
pZ80->DE.B.h = Z80_RD_PC_BYTE(pZ80);
Cycles = 2;
}
break;
case 0x018:
{
/* JR dd */
Z80_BYTE_OFFSET Offset;
Offset = Z80_RD_BYTE_IM0();
pZ80->MemPtr.W = pZ80->PC.W.l + Offset;
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles = 3;
}
break;
case 0x01a:
{
/* LD A,(DE) */
 
    pZ80->AF.B.h = Z80_RD_BYTE(pZ80,pZ80->DE.W); 
	pZ80->MemPtr.W = pZ80->DE.W+1; 
Cycles = 2;
}
break;
case 0x01c:
{
INC_R(pZ80->DE.B.l);
Cycles = 1;
}
break;
case 0x01e:
{
 /* LD E,n */
pZ80->DE.B.l = Z80_RD_PC_BYTE(pZ80);
Cycles = 2;
}
break;
case 0x020:
{
/* JR nz,dd */
Z80_BYTE_OFFSET Offset;
Offset = Z80_RD_BYTE_IM0();
if (Z80_TEST_ZERO_NOT_SET)
{
pZ80->MemPtr.W = pZ80->PC.W.l + Offset;
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles=3;
}
else
{
Cycles=2;
}
}
break;
case 0x022:
{
/* LD (nnnn),HL */
    pZ80->MemPtr.W = Z80_RD_WORD_IM0();
 	Z80_WR_WORD(pZ80,pZ80->MemPtr.W, pZ80->HL.W);
 	++pZ80->MemPtr.W;
Cycles = 5;
}
break;
case 0x024:
{
INC_R(pZ80->HL.B.h);
Cycles = 1;
}
break;
case 0x026:
{
 /* LD H,n */
pZ80->HL.B.h = Z80_RD_PC_BYTE(pZ80);
Cycles = 2;
}
break;
case 0x028:
{
/* JR z,dd */
Z80_BYTE_OFFSET Offset;
Offset = Z80_RD_BYTE_IM0();
if (Z80_TEST_ZERO_SET)
{
pZ80->MemPtr.W = pZ80->PC.W.l + Offset;
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles=3;
}
else
{
Cycles=2;
}
}
break;
case 0x02a:
{
/* LD HL,(nnnn) */
    pZ80->MemPtr.W = Z80_RD_WORD_IM0();
 	pZ80->HL.W = Z80_RD_WORD(pZ80,pZ80->MemPtr.W);
 	++pZ80->MemPtr.W;
Cycles = 5;
}
break;
case 0x02c:
{
INC_R(pZ80->HL.B.l);
Cycles = 1;
}
break;
case 0x02e:
{
 /* LD L,n */
pZ80->HL.B.l = Z80_RD_PC_BYTE(pZ80);
Cycles = 2;
}
break;
case 0x030:
{
/* JR nc,dd */
Z80_BYTE_OFFSET Offset;
Offset = Z80_RD_BYTE_IM0();
if (Z80_TEST_CARRY_NOT_SET)
{
pZ80->MemPtr.W = pZ80->PC.W.l + Offset;
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles=3;
}
else
{
Cycles=2;
}
}
break;
case 0x032:
{
/* LD (nnnn),A */

	/* get memory address to read from and store in memptr */ 
	pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80); 
	/* write byte */ 
	Z80_WR_BYTE(pZ80,pZ80->MemPtr.W, pZ80->AF.B.h); 
	/* increment memory pointer */ 
	pZ80->MemPtr.B.l++; 
	/* and store a in upper byte */ 
	pZ80->MemPtr.B.h = pZ80->AF.B.h; 
Cycles = 4;
}
break;
case 0x034:
{
INC_HL_();
Cycles = 3;
}
break;
case 0x036:
{
 /* LD (HL),n */
pZ80->TempByte = Z80_RD_PC_BYTE(pZ80);
Z80_WR_BYTE(pZ80,pZ80->HL.W,pZ80->TempByte);
Cycles = 3;
}
break;
case 0x038:
{
/* JR c,dd */
Z80_BYTE_OFFSET Offset;
Offset = Z80_RD_BYTE_IM0();
if (Z80_TEST_CARRY_SET)
{
pZ80->MemPtr.W = pZ80->PC.W.l + Offset;
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles=3;
}
else
{
Cycles=2;
}
}
break;
case 0x03a:
{
/* LD A,(nnnn) */

	/* get memory address to read from */ 
	pZ80->MemPtr.W = Z80_RD_PC_WORD(pZ80); 
 
	/* read byte */ 
	pZ80->AF.B.h = Z80_RD_BYTE(pZ80,pZ80->MemPtr.W); 
 
	/* increment memptr */ 
	++pZ80->MemPtr.W; 
Cycles = 4;
}
break;
case 0x03c:
{
INC_R(pZ80->AF.B.h);
Cycles = 1;
}
break;
case 0x03e:
{
 /* LD A,n */
pZ80->AF.B.h = Z80_RD_PC_BYTE(pZ80);
Cycles = 2;
}
break;
case 0x040:
{
Cycles = 1;
}
break;
case 0x042:
{
/* LD B,D */
 
		pZ80->BC.B.h = pZ80->DE.B.h; 
Cycles = 1;
}
break;
case 0x044:
{
/* LD B,H */
 
		pZ80->BC.B.h = pZ80->HL.B.h; 
Cycles = 1;
}
break;
case 0x046:
{
/* LD B,(HL) */
 
        pZ80->BC.B.h = Z80_RD_BYTE(pZ80,pZ80->HL.W); 
Cycles = 2;
}
break;
case 0x048:
{
/* LD C,B */
 
		pZ80->BC.B.l = pZ80->BC.B.h; 
Cycles = 1;
}
break;
case 0x04a:
{
/* LD C,D */
 
		pZ80->BC.B.l = pZ80->DE.B.h; 
Cycles = 1;
}
break;
case 0x04c:
{
/* LD C,H */
 
		pZ80->BC.B.l = pZ80->HL.B.h; 
Cycles = 1;
}
break;
case 0x04e:
{
/* LD C,(HL) */
 
        pZ80->BC.B.l = Z80_RD_BYTE(pZ80,pZ80->HL.W); 
Cycles = 2;
}
break;
case 0x050:
{
/* LD D,B */
 
		pZ80->DE.B.h = pZ80->BC.B.h; 
Cycles = 1;
}
break;
case 0x052:
{
Cycles = 1;
}
break;
case 0x054:
{
/* LD D,H */
 
		pZ80->DE.B.h = pZ80->HL.B.h; 
Cycles = 1;
}
break;
case 0x056:
{
/* LD D,(HL) */
 
        pZ80->DE.B.h = Z80_RD_BYTE(pZ80,pZ80->HL.W); 
Cycles = 2;
}
break;
case 0x058:
{
/* LD E,B */
 
		pZ80->DE.B.l = pZ80->BC.B.h; 
Cycles = 1;
}
break;
case 0x05a:
{
/* LD E,D */
 
		pZ80->DE.B.l = pZ80->DE.B.h; 
Cycles = 1;
}
break;
case 0x05c:
{
/* LD E,H */
 
		pZ80->DE.B.l = pZ80->HL.B.h; 
Cycles = 1;
}
break;
case 0x05e:
{
/* LD E,(HL) */
 
        pZ80->DE.B.l = Z80_RD_BYTE(pZ80,pZ80->HL.W); 
Cycles = 2;
}
break;
case 0x060:
{
/* LD H,B */
 
		pZ80->HL.B.h = pZ80->BC.B.h; 
Cycles = 1;
}
break;
case 0x062:
{
/* LD H,D */
 
		pZ80->HL.B.h = pZ80->DE.B.h; 
Cycles = 1;
}
break;
case 0x064:
{
Cycles = 1;
}
break;
case 0x066:
{
/* LD H,(HL) */
 
        pZ80->HL.B.h = Z80_RD_BYTE(pZ80,pZ80->HL.W); 
Cycles = 2;
}
break;
case 0x068:
{
/* LD L,B */
 
		pZ80->HL.B.l = pZ80->BC.B.h; 
Cycles = 1;
}
break;
case 0x06a:
{
/* LD L,D */
 
		pZ80->HL.B.l = pZ80->DE.B.h; 
Cycles = 1;
}
break;
case 0x06c:
{
/* LD L,H */
 
		pZ80->HL.B.l = pZ80->HL.B.h; 
Cycles = 1;
}
break;
case 0x06e:
{
/* LD L,(HL) */
 
        pZ80->HL.B.l = Z80_RD_BYTE(pZ80,pZ80->HL.W); 
Cycles = 2;
}
break;
case 0x070:
{
/* LD (HL),B */
 
        Z80_WR_BYTE(pZ80,pZ80->HL.W,pZ80->BC.B.h); 
Cycles = 2;
}
break;
case 0x072:
{
/* LD (HL),D */
 
        Z80_WR_BYTE(pZ80,pZ80->HL.W,pZ80->DE.B.h); 
Cycles = 2;
}
break;
case 0x074:
{
/* LD (HL),H */
 
        Z80_WR_BYTE(pZ80,pZ80->HL.W,pZ80->HL.B.h); 
Cycles = 2;
}
break;
case 0x076:
{
pZ80->Outputs |= Z80_OUTPUT_HALT;
pZ80->Flags |=Z80_EXECUTING_HALT_FLAG;
pZ80->PC.W.l--;
Cycles = 1;
}
break;
case 0x078:
{

/* LD A,B */
 
		pZ80->AF.B.h = pZ80->BC.B.h; 
Cycles = 1;
}
break;
case 0x07a:
{
/* LD A,D */
 
		pZ80->AF.B.h = pZ80->DE.B.h; 
Cycles = 1;
}
break;
case 0x07c:
{
/* LD A,H */
 
		pZ80->AF.B.h = pZ80->HL.B.h; 
Cycles = 1;
}
break;
case 0x07e:
{
/* LD A,(HL) */
 
        pZ80->AF.B.h = Z80_RD_BYTE(pZ80,pZ80->HL.W); 
Cycles = 2;
}
break;
case 0x080:
{
ADD_A_R(pZ80->BC.B.h);
Cycles = 1;
}
break;
case 0x082:
{
ADD_A_R(pZ80->DE.B.h);
Cycles = 1;
}
break;
case 0x084:
{
ADD_A_R(pZ80->HL.B.h);
Cycles = 1;
}
break;
case 0x086:
{
ADD_A_HL(pZ80);
Cycles = 2;
}
break;
case 0x088:
{
ADC_A_R(pZ80->BC.B.h);
Cycles = 1;
}
break;
case 0x08a:
{
ADC_A_R(pZ80->DE.B.h);
Cycles = 1;
}
break;
case 0x08c:
{
ADC_A_R(pZ80->HL.B.h);
Cycles = 1;
}
break;
case 0x08e:
{
ADC_A_HL(pZ80);
Cycles = 2;
}
break;
case 0x090:
{
SUB_A_R(pZ80->BC.B.h);
Cycles = 1;
}
break;
case 0x092:
{
SUB_A_R(pZ80->DE.B.h);
Cycles = 1;
}
break;
case 0x094:
{
SUB_A_R(pZ80->HL.B.h);
Cycles = 1;
}
break;
case 0x096:
{
SUB_A_HL(pZ80);
Cycles = 2;
}
break;
case 0x098:
{
SBC_A_R(pZ80->BC.B.h);
Cycles = 1;
}
break;
case 0x09a:
{
SBC_A_R(pZ80->DE.B.h);
Cycles = 1;
}
break;
case 0x09c:
{
SBC_A_R(pZ80->HL.B.h);
Cycles = 1;
}
break;
case 0x09e:
{
SBC_A_HL(pZ80);
Cycles = 2;
}
break;
case 0x0a0:
{
AND_A_R(pZ80->BC.B.h);
Cycles = 1;
}
break;
case 0x0a2:
{
AND_A_R(pZ80->DE.B.h);
Cycles = 1;
}
break;
case 0x0a4:
{
AND_A_R(pZ80->HL.B.h);
Cycles = 1;
}
break;
case 0x0a6:
{
AND_A_HL(pZ80);
Cycles = 2;
}
break;
case 0x0a8:
{
XOR_A_R(pZ80->BC.B.h);
Cycles = 1;
}
break;
case 0x0aa:
{
XOR_A_R(pZ80->DE.B.h);
Cycles = 1;
}
break;
case 0x0ac:
{
XOR_A_R(pZ80->HL.B.h);
Cycles = 1;
}
break;
case 0x0ae:
{
XOR_A_HL(pZ80);
Cycles = 2;
}
break;
case 0x0b0:
{
OR_A_R(pZ80->BC.B.h);
Cycles = 1;
}
break;
case 0x0b2:
{
OR_A_R(pZ80->DE.B.h);
Cycles = 1;
}
break;
case 0x0b4:
{
OR_A_R(pZ80->HL.B.h);
Cycles = 1;
}
break;
case 0x0b6:
{
OR_A_HL(pZ80);
Cycles = 2;
}
break;
case 0x0b8:
{
/* CP A,B */

{
    Z80_WORD Result;
    Z80_FLAGS_REG &= ~(Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);  
    Z80_FLAGS_REG |= pZ80->BC.B.h & (Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2); 
    Z80_FLAGS_REG |= Z80_SUBTRACT_FLAG;              
    Result = (Z80_WORD)pZ80->AF.B.h - (Z80_WORD)pZ80->BC.B.h;
    SET_OVERFLOW_FLAG_A_SUB(pZ80->BC.B.h,Result); 
    SET_ZERO_SIGN(Result);                  
    SET_CARRY_FLAG_SUB(Result);                     
    SET_HALFCARRY(pZ80->BC.B.h, Result);
}
Cycles = 1;
}
break;
case 0x0ba:
{
/* CP A,D */

{
    Z80_WORD Result;
    Z80_FLAGS_REG &= ~(Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);  
    Z80_FLAGS_REG |= pZ80->DE.B.h & (Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2); 
    Z80_FLAGS_REG |= Z80_SUBTRACT_FLAG;              
    Result = (Z80_WORD)pZ80->AF.B.h - (Z80_WORD)pZ80->DE.B.h;
    SET_OVERFLOW_FLAG_A_SUB(pZ80->DE.B.h,Result); 
    SET_ZERO_SIGN(Result);                  
    SET_CARRY_FLAG_SUB(Result);                     
    SET_HALFCARRY(pZ80->DE.B.h, Result);
}
Cycles = 1;
}
break;
case 0x0bc:
{
/* CP A,H */

{
    Z80_WORD Result;
    Z80_FLAGS_REG &= ~(Z80_UNUSED_FLAG1 | Z80_UNUSED_FLAG2);  
    Z80_FLAGS_REG |= pZ80->HL.B.h & (Z80_UNUSED_FLAG1|Z80_UNUSED_FLAG2); 
    Z80_FLAGS_REG |= Z80_SUBTRACT_FLAG;              
    Result = (Z80_WORD)pZ80->AF.B.h - (Z80_WORD)pZ80->HL.B.h;
    SET_OVERFLOW_FLAG_A_SUB(pZ80->HL.B.h,Result); 
    SET_ZERO_SIGN(Result);                  
    SET_CARRY_FLAG_SUB(Result);                     
    SET_HALFCARRY(pZ80->HL.B.h, Result);
}
Cycles = 1;
}
break;
case 0x0be:
{
CP_A_HL(pZ80);
Cycles = 2;
}
break;
case 0x0c0:
{
/* RET nz */
if (Z80_TEST_ZERO_NOT_SET)
{
RETURN();
Cycles=4;
}
else
{
Cycles=2-1;
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
}
}
break;
case 0x0c2:
{
/* JP nz,nnnn */
if (Z80_TEST_ZERO_NOT_SET)
{
pZ80->MemPtr.W = Z80_RD_WORD_IM0();
pZ80->PC.W.l = pZ80->MemPtr.W;
}
else
{
	Z80_RD_PC_WORD(pZ80);
}
Cycles=3;
}
break;
case 0x0c4:
{
/* CALL nz,nnnn */
if (Z80_TEST_ZERO_NOT_SET)
{
pZ80->MemPtr.W = Z80_RD_WORD_IM0();
Z80_PUSH_PC(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles=5;
}
else
{
	Z80_RD_PC_WORD(pZ80);
Cycles=3;
}
}
break;
case 0x0c6:
{
pZ80->TempByte = Z80_RD_PC_BYTE(pZ80);
ADD_A_X(pZ80->TempByte);
Cycles = 2;
}
break;
case 0x0c8:
{
/* RET z */
if (Z80_TEST_ZERO_SET)
{
RETURN();
Cycles=4;
}
else
{
Cycles=2-1;
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
}
}
break;
case 0x0ca:
{
/* JP z,nnnn */
if (Z80_TEST_ZERO_SET)
{
pZ80->MemPtr.W = Z80_RD_WORD_IM0();
pZ80->PC.W.l = pZ80->MemPtr.W;
}
else
{
	Z80_RD_PC_WORD(pZ80);
}
Cycles=3;
}
break;
case 0x0cc:
{
/* CALL z,nnnn */
if (Z80_TEST_ZERO_SET)
{
pZ80->MemPtr.W = Z80_RD_WORD_IM0();
Z80_PUSH_PC(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles=5;
}
else
{
	Z80_RD_PC_WORD(pZ80);
Cycles=3;
}
}
break;
case 0x0ce:
{
pZ80->TempByte = Z80_RD_PC_BYTE(pZ80);
ADC_A_X(pZ80->TempByte);
Cycles = 2;
}
break;
case 0x0d0:
{
/* RET nc */
if (Z80_TEST_CARRY_NOT_SET)
{
RETURN();
Cycles=4;
}
else
{
Cycles=2-1;
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
}
}
break;
case 0x0d2:
{
/* JP nc,nnnn */
if (Z80_TEST_CARRY_NOT_SET)
{
pZ80->MemPtr.W = Z80_RD_WORD_IM0();
pZ80->PC.W.l = pZ80->MemPtr.W;
}
else
{
	Z80_RD_PC_WORD(pZ80);
}
Cycles=3;
}
break;
case 0x0d4:
{
/* CALL nc,nnnn */
if (Z80_TEST_CARRY_NOT_SET)
{
pZ80->MemPtr.W = Z80_RD_WORD_IM0();
Z80_PUSH_PC(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles=5;
}
else
{
	Z80_RD_PC_WORD(pZ80);
Cycles=3;
}
}
break;
case 0x0d6:
{
pZ80->TempByte = Z80_RD_PC_BYTE(pZ80);
SUB_A_X(pZ80->TempByte);
Cycles = 2;
}
break;
case 0x0d8:
{
/* RET c */
if (Z80_TEST_CARRY_SET)
{
RETURN();
Cycles=4;
}
else
{
Cycles=2-1;
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
}
}
break;
case 0x0da:
{
/* JP c,nnnn */
if (Z80_TEST_CARRY_SET)
{
pZ80->MemPtr.W = Z80_RD_WORD_IM0();
pZ80->PC.W.l = pZ80->MemPtr.W;
}
else
{
	Z80_RD_PC_WORD(pZ80);
}
Cycles=3;
}
break;
case 0x0dc:
{
/* CALL c,nnnn */
if (Z80_TEST_CARRY_SET)
{
pZ80->MemPtr.W = Z80_RD_WORD_IM0();
Z80_PUSH_PC(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles=5;
}
else
{
	Z80_RD_PC_WORD(pZ80);
Cycles=3;
}
}
break;
case 0x0de:
{
pZ80->TempByte = Z80_RD_PC_BYTE(pZ80);
SBC_A_X(pZ80->TempByte);
Cycles = 2;
}
break;
case 0x0e0:
{
/* RET po */
if (Z80_TEST_PARITY_ODD)
{
RETURN();
Cycles=4;
}
else
{
Cycles=2-1;
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
}
}
break;
case 0x0e2:
{
/* JP po,nnnn */
if (Z80_TEST_PARITY_ODD)
{
pZ80->MemPtr.W = Z80_RD_WORD_IM0();
pZ80->PC.W.l = pZ80->MemPtr.W;
}
else
{
	Z80_RD_PC_WORD(pZ80);
}
Cycles=3;
}
break;
case 0x0e4:
{
/* CALL po,nnnn */
if (Z80_TEST_PARITY_ODD)
{
pZ80->MemPtr.W = Z80_RD_WORD_IM0();
Z80_PUSH_PC(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles=5;
}
else
{
	Z80_RD_PC_WORD(pZ80);
Cycles=3;
}
}
break;
case 0x0e6:
{
pZ80->TempByte = Z80_RD_PC_BYTE(pZ80);
AND_A_X(pZ80->TempByte);
Cycles = 2;
}
break;
case 0x0e8:
{
/* RET pe */
if (Z80_TEST_PARITY_EVEN)
{
RETURN();
Cycles=4;
}
else
{
Cycles=2-1;
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
}
}
break;
case 0x0ea:
{
/* JP pe,nnnn */
if (Z80_TEST_PARITY_EVEN)
{
pZ80->MemPtr.W = Z80_RD_WORD_IM0();
pZ80->PC.W.l = pZ80->MemPtr.W;
}
else
{
	Z80_RD_PC_WORD(pZ80);
}
Cycles=3;
}
break;
case 0x0ec:
{
/* CALL pe,nnnn */
if (Z80_TEST_PARITY_EVEN)
{
pZ80->MemPtr.W = Z80_RD_WORD_IM0();
Z80_PUSH_PC(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles=5;
}
else
{
	Z80_RD_PC_WORD(pZ80);
Cycles=3;
}
}
break;
case 0x0ee:
{
pZ80->TempByte = Z80_RD_PC_BYTE(pZ80);
XOR_A_X(pZ80->TempByte);
Cycles = 2;
}
break;
case 0x0f0:
{
/* RET p */
if (Z80_TEST_POSITIVE)
{
RETURN();
Cycles=4;
}
else
{
Cycles=2-1;
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
}
}
break;
case 0x0f2:
{
/* JP p,nnnn */
if (Z80_TEST_POSITIVE)
{
pZ80->MemPtr.W = Z80_RD_WORD_IM0();
pZ80->PC.W.l = pZ80->MemPtr.W;
}
else
{
	Z80_RD_PC_WORD(pZ80);
}
Cycles=3;
}
break;
case 0x0f4:
{
/* CALL p,nnnn */
if (Z80_TEST_POSITIVE)
{
pZ80->MemPtr.W = Z80_RD_WORD_IM0();
Z80_PUSH_PC(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles=5;
}
else
{
	Z80_RD_PC_WORD(pZ80);
Cycles=3;
}
}
break;
case 0x0f6:
{
pZ80->TempByte = Z80_RD_PC_BYTE(pZ80);
OR_A_X(pZ80->TempByte);
Cycles = 2;
}
break;
case 0x0f8:
{
/* RET m */
if (Z80_TEST_MINUS)
{
RETURN();
Cycles=4;
}
else
{
Cycles=2-1;
/* spare cycles at end of instruction that interrupt can consume if triggered */
pZ80->Flags |= Z80_INTERRUPT_NO_DELAY_FLAG;
}
}
break;
case 0x0fa:
{
/* JP m,nnnn */
if (Z80_TEST_MINUS)
{
pZ80->MemPtr.W = Z80_RD_WORD_IM0();
pZ80->PC.W.l = pZ80->MemPtr.W;
}
else
{
	Z80_RD_PC_WORD(pZ80);
}
Cycles=3;
}
break;
case 0x0fc:
{
/* CALL m,nnnn */
if (Z80_TEST_MINUS)
{
pZ80->MemPtr.W = Z80_RD_WORD_IM0();
Z80_PUSH_PC(pZ80);
pZ80->PC.W.l = pZ80->MemPtr.W;
Cycles=5;
}
else
{
	Z80_RD_PC_WORD(pZ80);
Cycles=3;
}
}
break;
case 0x0fe:
{
pZ80->TempByte = Z80_RD_PC_BYTE(pZ80);
CP_A_X(pZ80->TempByte);
Cycles = 2;
}
break;
case 0x0ff:
{
/* RST 0x0038 */
	
/* push return address on stack */	
    Z80_PUSH_PC(pZ80); 
/* set memptr to address */	
pZ80->MemPtr.W = 0x0038;	
/* set program counter to memptr */ 
pZ80->PC.W.l = pZ80->MemPtr.W; 
Cycles = 4;
}
break;
default:
/* the following tells MSDEV 6 to not generate */
/* code which checks if a input value to the  */
/* switch is not valid.*/
#ifdef _MSC_VER
#if (_MSC_VER>=1200)
	Cycles=0;
	__assume(0);
#endif
#else
	Cycles=0;
#endif
break;
}
return Cycles;
}
#endif /* INKZ80 */

