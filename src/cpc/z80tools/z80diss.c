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
/*------------------------------------------*/
/* Z80 Dissassembler						*/
/* (smaller program because of less tables) */
#include "../gendiss.h"
#include "../memrange.h"
#include "../debugger/gdebug.h"
#include "../debugger/labelset.h"

static const char *RegA[]=
{
	"B",
	"C",
	"D",
	"E",
	"H",
	"L",
	"(HL)",
	"A",
	"X",
	"0"
};




static const char *RegB[]=
{
	"BC",
	"DE",
	"HL",
	"SP"
};

static const char *RegC[]=
{
	"BC",
	"DE",
	"HL",
	"AF"
};

static const char *CB_ShiftMneumonics[]=
{
	"RLC",
	"RRC",
	"RL",
	"RR",
	"SLA",
	"SRA",
	"SLL",
	"SRL"
};

static const char *CB_BitOperationMneumonics[]=
{
	"BIT",
	"RES",
	"SET"
};

static const char *ArithmeticMneumonics[]=
{
	"ADD",
	"ADC",
	"SUB",
	"SBC",
	"AND",
	"XOR",
	"OR",
	"CP"
};

static const char *ConditionCodes[]=
{
	"NZ",
	"Z",
	"NC",
	"C",
	"PO",
	"PE",
	"P",
	"M"
};

static const char *ShiftMneumonics[]=
{
	"RLCA",
	"RRCA",
	"RLA",
	"RRA"
};

static const char *MiscMneumonics1[]=
{
	"DAA",
	"CPL",
	"SCF",
	"CCF"
};

static const char *MiscMneumonics2[]=
{
	"NOP",
	"EX AF,AF'",
	"DJNZ",
	"JR"
};

static const char *MiscMneumonics3[]=
{
	"RET",
	"EXX",
	"JP (HL)",
	"LD SP,HL"
};

static const char *MiscMneumonics4[]=
{
	"LD I,A",
	"LD R,A",
	"LD A,I",
	"LD A,R",
	"RRD",
	"RLD",
	"",
	"",
};

static const char *MiscMneumonics5[]=
{
	"LDI",
	"LDD",
	"LDIR",
	"LDDR"
};

static const char *MiscMneumonics6[]=
{
	"CPI",
	"CPD",
	"CPIR",
	"CPDR"
};

static const char *MiscMneumonics7[]=
{
	"INI",
	"IND",
	"INIR",
	"INDR"
};

const char *MiscMneumonics8[]=
{
	"OUTI",
	"OUTD",
	"OTIR",
	"OTDR"
};

static const char *MiscMneumonics9[]=
{
	"EX (SP),HL",
	"EX DE,HL",
	"DI",
	"EI"
};
#if 0
/* get word at address specified by Addr */
static unsigned long Diss_GetWord(int Addr)
{
	return (MemoryRange_ReadByte(pRange,(Addr) & 0x0ff) |
		((MemoryRange_ReadByte(pRange,((Addr+1) & 0x0ffff) & 0x0ff)<<8);
}
#endif

const char *GetLabelForAddress(MemoryRange *pRange, int Addr)
{
	LABEL *pLabel = labelsets_find_label_by_exact_address(Addr);
	if (pLabel!=NULL)
	{
		if (pLabel->m_sName!=NULL)
		{
			return pLabel->m_sName;
		}
	}
	
	return NULL;
}

/* get relative addr for JR,DJNZ */
static  int	Diss_GetRelativeAddr(MemoryRange *pRange, int Addr)
{
	int Offset;

	/* get signed byte offset */
	Offset = MemoryRange_ReadByte(pRange,Addr);

	/* sign extend */
	if ((Offset & 0x080)!=0)
	{
		Offset|=0x0ff00;
	}
	else
	{
		Offset&=0x0ff;
	}

	/* return addr */
	return (Addr+1+Offset);
}

#if 0
 static char Diss_GetHexDigitAsChar(char HexDigit)
{
	HexDigit&=0x0f;
	HexDigit+='0';

	/* 10-15? */
	if (HexDigit>=('0'+10))
	{
		/* A-F */
		HexDigit+='A'-'0'-10;
	}

	return HexDigit;
}
#endif
#if 0
 static char *Diss_WriteHexWord(char *pDissString, unsigned long Value)
{
	char HexDigit;
	char *pString = pDissString;

	pString[0] = '#';

	HexDigit = Value>>12;

	pString[1] = Diss_GetHexDigitAsChar(HexDigit);

	HexDigit = Value>>8;

	pString[2] = Diss_GetHexDigitAsChar(HexDigit);

	HexDigit = Value>>4;

	pString[3] = Diss_GetHexDigitAsChar(HexDigit);

	HexDigit = Value & 0x0f;

	pString[4] = Diss_GetHexDigitAsChar(HexDigit);

	return &pString[5];
}
#endif

 static char *Diss_IndexReg(char *pDissString, char IndexCh)
{
	pDissString[0] = 'I';
	pDissString[1] = IndexCh;
    pDissString+=2;

    return pDissString;
}


 static char *Diss_IndexedOffset(char *pDissString, signed char Offset, char IndexCh)
{
	pDissString[0]='(';
	++pDissString;
	pDissString = Diss_IndexReg(pDissString, IndexCh);

	pDissString = Diss_WriteHexByte(pDissString,Offset,TRUE, TRUE);


	pDissString[0]=')';
	pDissString++;

    return pDissString;
}


 static char *Diss_bracket_open(char *pDissString)
{
	pDissString[0]='(';
    ++pDissString;
    return pDissString;
}

 static char *Diss_bracket_close(char *pDissString)
{
	pDissString[0]=')';
    ++pDissString;
    return pDissString;
}

 static char *Diss_AddressOrLabel(MemoryRange *pRange, char *pDissString, int Addr)
{
	int DisWord = Addr;	

	const char *sLabel = GetLabelForAddress(pRange,DisWord);
	if (sLabel!=NULL)
	{
		pDissString = Diss_strcat(pDissString, sLabel);
	}
	else
	{
		pDissString = Diss_WriteHexWord(pDissString, DisWord, TRUE,FALSE);
	}

	return pDissString;
}


 static char *Diss_ContentsOfAddress(MemoryRange *pRange, char *pDissString, int DisAddr)
{
    pDissString = Diss_bracket_open(pDissString);

	pDissString = Diss_AddressOrLabel(pRange, pDissString,MemoryRange_ReadWord(pRange,DisAddr));

    pDissString = Diss_bracket_close(pDissString);

    return pDissString;
}


 static char *Diss_Index_Reg8Bit(MemoryRange *pRange,char *pDissString,char RegIndex, int DisAddr, char IndexCh)
{
	if ((RegIndex<4) || (RegIndex>6))
		return pDissString;

	if (RegIndex==6)
	{
		/* (HL) -> (IX+offs) */
		char Offset;

		/* signed offset from index register */
		Offset = MemoryRange_ReadByte(pRange,DisAddr);

		return Diss_IndexedOffset(pDissString,Offset,IndexCh);
	}
	else if ((RegIndex==4) || (RegIndex==5))
	{
		/* H -> HIX */
		/* L -> HIX */

		if (RegIndex==4)
		{
			pDissString[0] = 'H';
		}
		else
		{
			pDissString[0] = 'L';
		}
        ++pDissString;
        pDissString = Diss_IndexReg(pDissString, IndexCh);

		return pDissString;

	}

	return pDissString;
}

 static BOOL	Diss_Index_IsReg8Bit(char RegIndex)
{
	if ((RegIndex<4) || (RegIndex>6))
		return FALSE;

	return TRUE;
}

 static BOOL Diss_Index_IsRegIndex(char RegIndex)
{
	if (RegIndex==2)
		return TRUE;

	return FALSE;
}


 static char *Diss_Port(char *pDissString)
{
	pDissString[0]='(';
	pDissString[1]='C';
	pDissString[2]=')';
    pDissString+=3;
    return pDissString;
}

 static char *Diss_EDNOP(char *pDissString, const int Opcode)
{
	pDissString = Diss_strcat(pDissString,"DEFB");
    pDissString = Diss_space(pDissString);
    pDissString = Diss_WriteHexByte(pDissString, 0x0ed,TRUE,FALSE);
    pDissString = Diss_comma(pDissString);
    pDissString = Diss_WriteHexByte(pDissString, Opcode,TRUE,FALSE);
    return pDissString;

}
static char *Diss_Index(MemoryRange *pRange, int DisAddr, char *pDissString,char IndexCh)
{
	unsigned char Opcode;

	/* DD prefix - IX */
	Opcode = MemoryRange_ReadByte(pRange,DisAddr);
	DisAddr++;

	switch (Opcode)
	{
		case 0x0dd:
		case 0x0fd:
		case 0x0ed:
		{
/*			pDissString = Diss_strcat(pDissString,"DEFB");
			Diss_WriteHexByte(Opcode);

*/		}
		break;

		case 0x0cb:
		{
			signed char Offset;

			/* CB prefix */

			/* signed offset from IX */
			Offset = MemoryRange_ReadByte(pRange,DisAddr);
			DisAddr++;

			/* opcode */
			Opcode = MemoryRange_ReadByte(pRange,DisAddr);

			if ((Opcode & 0x0c0)==0x00)
			{
				/* 00000rrr - RLC */
				/* 00001rrr - RRC */
				/* 00010rrr - RL */
				/* 00011rrr - RR */
				/* 00100rrr - SLA */
				/* 00101rrr - SRA */
				/* 00110rrr - SLL */
				/* 00111rrr - SRL */

				if ((Opcode & 7)!=6)
				{
					/* LD r, */
					pDissString = Diss_strcat(pDissString,"LD");
					pDissString = Diss_space(pDissString);
					pDissString = Diss_strcat(pDissString,RegA[(Opcode & 7)]);
					pDissString = Diss_comma(pDissString);
				}

				/* write shift mneumonic */
				pDissString = Diss_strcat(pDissString,CB_ShiftMneumonics[((Opcode>>3) & 0x07)]);
				pDissString = Diss_space(pDissString);

				pDissString = Diss_IndexedOffset(pDissString,Offset, IndexCh);



			}
			else
			{
				/* 01bbbrrr - BIT */
				/* 10bbbrrr - RES */
				/* 11bbbrrr - SET */
				int BitIndex = ((Opcode>>3) & 0x07);
				int RegIndex = (Opcode & 0x07);

				if (((Opcode & 0x0c0)!=0x040) && (RegIndex!=6))
				{
					pDissString = Diss_strcat(pDissString,"LD");
					pDissString = Diss_space(pDissString);
					pDissString = Diss_strcat(pDissString,RegA[RegIndex]);
					pDissString = Diss_comma(pDissString);
				}

				pDissString = Diss_strcat(pDissString,CB_BitOperationMneumonics[(((Opcode>>6)&3)-1)]);
				pDissString = Diss_space(pDissString);
				*pDissString = BitIndex+'0';
				pDissString++;
				pDissString = Diss_comma(pDissString);

				pDissString = Diss_IndexedOffset(pDissString,Offset, IndexCh);


			}

		}
		 return pDissString;

		default:
		{
			switch (Opcode & 0x0c0)
			{
				case 0x000:
				{
					switch (Opcode & 0x07)
					{
						case 1:
						{
							if ((Opcode & 0x08)!=0)
							{
								/* 00ss1001 - ADD HL,ss */
								int RegIndex = ((Opcode>>4) & 0x03);


								pDissString = Diss_strcat(pDissString,"ADD");
								pDissString = Diss_space(pDissString);
                                pDissString = Diss_IndexReg(pDissString,IndexCh);
								pDissString = Diss_comma(pDissString);

								if (Diss_Index_IsRegIndex(RegIndex))
								{
									pDissString = Diss_IndexReg(pDissString,IndexCh);
								}
								else
								{
									pDissString = Diss_strcat(pDissString,RegB[RegIndex]);
								}

								 return pDissString;
							}
							else
							{
								/* 00dd0001 - LD dd,nn */

								if (Diss_Index_IsRegIndex(((Opcode>>4) & 0x03)))
								{
									pDissString = Diss_strcat(pDissString,"LD");
									pDissString = Diss_space(pDissString);
									pDissString = Diss_IndexReg(pDissString,IndexCh);
									pDissString = Diss_comma(pDissString);
									pDissString = Diss_AddressOrLabel(pRange, pDissString,MemoryRange_ReadWord(pRange,DisAddr));

									 return pDissString;
								}
							}

						}
						break;

						case 2:
						{
							switch ((Opcode>>4) & 0x03)
							{
								case 2:
								{
									/* 00100010 - LD (nnnn),HL */
									/* 00101010 - LD HL,(nn) */

									pDissString = Diss_strcat(pDissString,"LD");
									pDissString = Diss_space(pDissString);


									if ((Opcode & (1<<3))==0)
									{
										pDissString = Diss_ContentsOfAddress(pRange,pDissString,DisAddr);
										pDissString = Diss_comma(pDissString);
										pDissString = Diss_IndexReg(pDissString,IndexCh);
									}
									else
									{
										pDissString = Diss_IndexReg(pDissString, IndexCh);
										pDissString = Diss_comma(pDissString);
										pDissString = Diss_ContentsOfAddress(pRange,pDissString,DisAddr);
									}


								}
								 return pDissString;

								default:
									break;
							}

						}
						break;

						case 3:
						{
							char *Instruction;

							if (Diss_Index_IsRegIndex(((Opcode>>4) & 0x03)))
							{
								if ((Opcode & 0x08)==0)
								{
									/* 00ss0011 - INC ss */
									Instruction = "INC";
								}
								else
								{
									/* 00ss1011 - DEC ss */
									Instruction= "DEC";
								}

								pDissString = Diss_strcat(pDissString,Instruction);
								pDissString = Diss_space(pDissString);
								pDissString = Diss_IndexReg(pDissString,IndexCh);

								 return pDissString;
							}

						}
						break;

						case 4:
						{
							/* 00rrr100 - INC r */
							pDissString = Diss_strcat(pDissString,"INC");
							pDissString = Diss_space(pDissString);
							pDissString = Diss_Index_Reg8Bit(pRange,pDissString,((Opcode>>3) & 0x07),DisAddr, IndexCh);


						}
						 return pDissString;

						case 5:
						{
							/* 00rrr101 - DEC r */
							pDissString = Diss_strcat(pDissString,"DEC");
							pDissString = Diss_space(pDissString);
							pDissString = Diss_Index_Reg8Bit(pRange,pDissString,((Opcode>>3) & 0x07),DisAddr, IndexCh);

						}
						 return pDissString;

						case 6:
						{
							/* LD r,n - 00rrr110 */
							char Data;
							int RegIndex = ((Opcode>>3) & 0x07);

							if (RegIndex!=6)
                            {
                                Data = MemoryRange_ReadByte(pRange,DisAddr);
                            }
                            else
                            {
                                Data = MemoryRange_ReadByte(pRange,DisAddr+1);
                            }
							pDissString = Diss_strcat(pDissString,"LD");
							pDissString = Diss_space(pDissString);
							pDissString= Diss_Index_Reg8Bit(pRange,pDissString,RegIndex,DisAddr, IndexCh);
							pDissString = Diss_comma(pDissString);
							pDissString = Diss_WriteHexByte(pDissString,Data,TRUE,FALSE);

						}
                        return pDissString;

						default:
							break;
					}
				}
				break;

				case 0x040:
				{
					/* 01xxxxxx */
					/* HALT, LD r,R */
					int Reg1,Reg2;
					/* will not get here if defb &dd:HALT is encountered! */

					/* 01rrrRRR - LD r,R */
					pDissString = Diss_strcat(pDissString,"LD");
					pDissString = Diss_space(pDissString);

					Reg1 = (Opcode>>3) & 0x07;
					Reg2 = Opcode & 0x07;

                    if ((Reg1==6) || (Reg2==6))
                    {
                        if (Reg1==6)
                        {
                           pDissString = Diss_Index_Reg8Bit(pRange,pDissString,Reg1, DisAddr, IndexCh);
                        }
                        else
                        {
                           pDissString = Diss_strcat(pDissString,RegA[Reg1]);
                        }

                         pDissString = Diss_comma(pDissString);

                        if (Reg2==6)
                        {
                           pDissString = Diss_Index_Reg8Bit(pRange,pDissString,Reg2, DisAddr, IndexCh);
                        }
                        else
                        {
                           pDissString = Diss_strcat(pDissString,RegA[Reg2]);
                        }

                    }
                    else
                    {
                        /* hix,lix,hiy,liy */


                        if (Diss_Index_IsReg8Bit(Reg1))
                        {
                            pDissString = Diss_Index_Reg8Bit(pRange,pDissString, Reg1, DisAddr, IndexCh);
                        }
                        else
                        {
                            pDissString = Diss_strcat(pDissString,RegA[Reg1]);
                        }

                        pDissString = Diss_comma(pDissString);

                        if (Diss_Index_IsReg8Bit(Reg2))
                        {
                            pDissString = Diss_Index_Reg8Bit(pRange,pDissString,Reg2, DisAddr, IndexCh);
                        }
                        else
                        {
                            pDissString = Diss_strcat(pDissString,RegA[Reg2]);
                        }
                    }

				}
				 return pDissString;

				case 0x080:
				{
					/* 10xxxxxx */
					/* 10000rrr - ADD */
					/* 10001rrr - ADC */
					/* 10010rrr - SUB */
					/* 10011rrr - SBC */
					/* 10100rrr - AND */
					/* 10101rrr - XOR */
					/* 10110rrr - OR */
					/* 10111rrr - CP */
					pDissString = Diss_strcat(pDissString,ArithmeticMneumonics[((Opcode>>3) & 0x07)]);
					pDissString = Diss_space(pDissString);
					pDissString = Diss_Index_Reg8Bit(pRange,pDissString,(Opcode & 0x07),DisAddr, IndexCh);

				}
				 return pDissString;

				case 0x0c0:
				{
					/* 11xxxxxx */


					switch (Opcode & 0x07)
					{
						case 1:
						{
							if ((Opcode & (1<<3))==0)
							{
								/* 11qq0001 - POP qq */
								pDissString = Diss_strcat(pDissString,"POP");
								pDissString = Diss_space(pDissString);
								pDissString = Diss_IndexReg(pDissString,IndexCh);

							}
							else
							{
								/* 11001001 - RET */
								/* 11011001 - EXX */
								/* 11101001 - JP (HL) */
								/* 11111001 - LD SP,HL */

								if (Opcode==0x0e9)
								{
									pDissString = Diss_strcat(pDissString,"JP");
									pDissString = Diss_space(pDissString);
									pDissString[0]='(';
									pDissString++;
									pDissString = Diss_IndexReg(pDissString,IndexCh);
									pDissString[0]=')';
									pDissString++;

								}
								else if (Opcode==0x0f9)
								{
									pDissString = Diss_strcat(pDissString,"LD");
									pDissString = Diss_space(pDissString);
									pDissString = Diss_strcat(pDissString,"SP");
									pDissString = Diss_comma(pDissString);
									pDissString = Diss_IndexReg(pDissString,IndexCh);

								}
							}
						}
						 return pDissString;

						case 3:
						{
							/* 11001011 - CB prefix */

							/* 11011011 - IN A,(n) */
							/* 11010011 - OUT (n),A */
							/* 11000011 - JP nn */
							/* 11100011 - EX (SP).HL */
							/* 11101011 - EX DE,HL */
							/* 11110011 - DI */
							/* 11111011 - EI */

							if (Opcode==0x0e3)
							{
								/* EX (SP), IX */
								pDissString = Diss_strcat(pDissString,"EX");
								pDissString = Diss_space(pDissString);
								pDissString = Diss_bracket_open(pDissString);
								pDissString = Diss_strcat(pDissString,"SP");
								pDissString = Diss_bracket_close(pDissString);
								pDissString = Diss_comma(pDissString);
								pDissString = Diss_IndexReg(pDissString,IndexCh);

							}

						}
						 return pDissString;

						case 5:
						{
							/* 11qq0101 - PUSH qq */
							if ((Opcode & (1<<3))==0)
							{
								pDissString = Diss_strcat(pDissString,"PUSH");
								pDissString = Diss_space(pDissString);
								pDissString = Diss_IndexReg(pDissString,IndexCh);

							}
						}
					    return pDissString;



						default:
							break;
					}

				}
				break;
			}
		}
		break;
	}

	pDissString = Diss_strcat(pDissString,"DEFB");
	pDissString = Diss_WriteHexByte(pDissString,Opcode,TRUE,FALSE);

    return pDissString;

}


char *Debug_DissassembleInstruction(MemoryRange *pRange, int Addr, char *pDissString)
{
	unsigned char Opcode;
	int		DisAddr = Addr;
	int			OpcodeCount;

	Opcode = MemoryRange_ReadByte(pRange, Addr);
	DisAddr++;

	OpcodeCount = Debug_GetOpcodeCount(pRange,Addr);

	if (OpcodeCount==1)
	{
		switch (Opcode)
		{
			case 0x0dd:
			case 0x0fd:
			{
				pDissString = Diss_strcat(pDissString,"DEFB");
				pDissString = Diss_space(pDissString);
				pDissString = Diss_WriteHexByte(pDissString,Opcode,TRUE,FALSE);
				return pDissString;
			}

			default:
				break;
		}
	}

	switch (Opcode)
	{
		case 0x0cb:
		{
			/* CB prefix */
			Opcode = MemoryRange_ReadByte(pRange,DisAddr);

			if ((Opcode & 0x0c0)==0x00)
			{
				/* 00000rrr - RLC */
				/* 00001rrr - RRC */
				/* 00010rrr - RL */
				/* 00011rrr - RR */
				/* 00100rrr - SLA */
				/* 00101rrr - SRA */
				/* 00110rrr - SLL */
				/* 00111rrr - SRL */

				pDissString = Diss_strcat(pDissString,CB_ShiftMneumonics[((Opcode>>3) & 0x07)]);
				pDissString = Diss_space(pDissString);
				pDissString = Diss_strcat(pDissString,RegA[(Opcode & 0x07)]);
			}
			else
			{
				/* 01bbbrrr - BIT */
				/* 10bbbrrr - RES */
				/* 11bbbrrr - SET */
				int BitIndex = ((Opcode>>3) & 0x07);

				pDissString = Diss_strcat(pDissString,CB_BitOperationMneumonics[(((Opcode>>6)&3)-1)]);
				pDissString = Diss_space(pDissString);
				*pDissString = BitIndex+'0';
				pDissString++;
				pDissString = Diss_comma(pDissString);
				pDissString = Diss_strcat(pDissString,RegA[Opcode & 0x07]);
			}
		}
		break;

		case 0x0dd:
		{
			pDissString = Diss_Index(pRange,DisAddr, pDissString,'X');
		}
		break;

		case 0x0ed:
		{
			/* ED prefix */
			Opcode = MemoryRange_ReadByte(pRange,DisAddr);
			DisAddr++;

			if ((Opcode & 0x0c0)==0x040)
			{
				switch (Opcode & 0x07)
				{
					case 0:
					{

						/* IN r,(C) - 01rrr000 */
						int RegIndex = ((Opcode>>3) & 0x07);

						if ((Opcode & (0x07<<3))==(6<<3))
						{
							/* IN X,(C) */
							RegIndex = 8;
						}

						pDissString = Diss_strcat(pDissString,"IN");
						pDissString = Diss_space(pDissString);
						pDissString = Diss_strcat(pDissString,RegA[RegIndex]);
						pDissString = Diss_comma(pDissString);
						pDissString = Diss_Port(pDissString);

					}
					break;

					case 1:
					{
						/* OUT (C),r - 01rrr001 */
						int RegIndex = ((Opcode>>3) & 0x07);

						if ((Opcode & (0x07<<3))==(6<<3))
						{
							/* OUT (C),0 */
							RegIndex = 9;
						}

						pDissString = Diss_strcat(pDissString,"OUT");
						pDissString = Diss_space(pDissString);
						pDissString = Diss_Port(pDissString);
						pDissString = Diss_comma(pDissString);
						pDissString = Diss_strcat(pDissString,RegA[RegIndex]);

					}
					break;

					case 2:
					{
						char *Instruction;

						if ((Opcode & 0x08)!=0)
						{
							/* ADC HL,ss - 01ss1010 */
							Instruction = "ADC";
						}
						else
						{
							/* SBC HL,ss - 01ss0010 */
							Instruction = "SBC";
						}

						pDissString = Diss_strcat(pDissString,Instruction);
						pDissString = Diss_space(pDissString);
						pDissString = Diss_strcat(pDissString,RegB[2]);
						pDissString = Diss_comma(pDissString);
						pDissString = Diss_strcat(pDissString,RegB[((Opcode>>4) & 0x03)]);

					}
					break;

					case 3:
					{
						/* LD dd,(nn) - 01dd1011 */
						/* LD (nn),dd - 01dd0011 */

						pDissString = Diss_strcat(pDissString,"LD");
						pDissString = Diss_space(pDissString);

						if ((Opcode & 0x08)!=0)
						{
							pDissString = Diss_strcat(pDissString,RegB[((Opcode>>4) & 0x03)]);
							pDissString = Diss_comma(pDissString);
							pDissString = Diss_ContentsOfAddress(pRange,pDissString,DisAddr);

						}
						else
						{
							pDissString = Diss_ContentsOfAddress(pRange,pDissString,DisAddr);
							pDissString = Diss_comma(pDissString);
							pDissString = Diss_strcat(pDissString,RegB[((Opcode>>4) & 0x03)]);
						}


					}
					break;

					case 4:
					{
						/* NEG - 01xxx100 */
						pDissString = Diss_strcat(pDissString,"NEG");

					}
					break;

					case 5:
					{
						/* RETI - 01xx1010 */
						/* RETN - 01xx0010 */
						if ((Opcode & 0x08)!=0)
						{
							pDissString = Diss_strcat(pDissString,"RETI");

						}
						else
						{
							pDissString = Diss_strcat(pDissString,"RETN");

						}
					}
					break;

					case 6:
					{
						char IM_Type = 0;

						/* IM 0 - 01x00110 */
						/* IM ? - 01x01110 */
						/* IM 1 - 01x10110 */
						/* IM 2 - 01x11110 */

						switch ((Opcode>>3) & 0x03)
						{
							default:
							case 0:
							{
								IM_Type = '0';
							}
							break;

							case 1:
							{
								IM_Type = '0';
							}
							break;

							case 2:
							{
								IM_Type = '1';
							}
							break;

							case 3:
							{
								IM_Type = '2';
							}
							break;
						}

						pDissString = Diss_strcat(pDissString,"IM");
						pDissString = Diss_space(pDissString);
						pDissString[0] = IM_Type;
						pDissString++;

					}
					break;

					case 7:
					{
						/* 01000111 - LD I,A */
						/* 01001111 - LD R,A */
						/* 01010111 - LD A,I */
						/* 01011111 - LD A,R */

						/* 01101111 - RLD */
						/* 01100111 - RRD */
						/* 01110111 - ED NOP */
						/* 01111111 - ED NOP */
						if ((Opcode==0x077) || (Opcode==0x07f))
                        {
                            pDissString = Diss_EDNOP(pDissString,Opcode);
                        }
                        else
                        {
       						pDissString = Diss_strcat(pDissString, MiscMneumonics4[((Opcode>>3) & 0x07)]);

                        }


					}
					break;


				}

			}
			else if ((Opcode & 0x0e4)==0x0a0)
			{
			    const char **sMneumonics;

				switch (Opcode & 0x03)
				{
				    default:
					case 0:
					{
						/* 10100000 - LDI */
						/* 10101000 - LDD */
						/* 10110000 - LDIR */
						/* 10111000 - LDDR */

 						sMneumonics = MiscMneumonics5;
					}
					break;

					case 1:
					{
						/* 10100001 - CPI */
						/* 10101001 - CPD */
						/* 10111001 - CPDR */
						/* 10110001 - CPIR */

						sMneumonics = MiscMneumonics6;
					}
					break;

					case 2:
					{
						/* 10100010 - INI */
						/* 10101010 - IND */
						/* 10110010 - INIR */
						/* 10111010 - INDR */
						sMneumonics = MiscMneumonics7;
					}
					break;

					case 3:
					{
						/* 10100011 - OUTI */
						/* 10101011 - outd */
						/* 10110011 - OTIR */
						/* 10111011 - otdr */

						sMneumonics = MiscMneumonics8;
					}
					break;

				}

                pDissString = Diss_strcat(pDissString, sMneumonics[((Opcode>>3) & 0x03)]);

			}
			else
			{
			    pDissString = Diss_EDNOP(pDissString,Opcode);
			}
		}
		break;

		case 0x0fd:
		{

			pDissString = Diss_Index(pRange,DisAddr,pDissString,'Y');
		}
		break;

		default:
		{
			switch (Opcode & 0x0c0)
			{
				case 0x000:
				{
					switch (Opcode & 0x07)
					{
						case 0:
						{
							if ((Opcode & 0x020)!=0)
							{
								/* 001cc000 - JR cc */

								pDissString = Diss_strcat(pDissString,"JR");
								pDissString = Diss_space(pDissString);
								pDissString = Diss_strcat(pDissString,ConditionCodes[((Opcode>>3) & 0x03)]);
								pDissString = Diss_comma(pDissString);
								pDissString = Diss_AddressOrLabel(pRange,pDissString,Diss_GetRelativeAddr(pRange,DisAddr));

							}
							else
							{

								if ((Opcode & 0x010)!=0)
								{
									/* 00010000 - DJNZ */
									/* 00011000 - JR */

									char *Instruction;

									if (Opcode==0x010)
									{
										Instruction = "DJNZ";
									}
									else
									{
										Instruction = "JR";
									}

									pDissString = Diss_strcat(pDissString,Instruction);
									pDissString = Diss_space(pDissString);
									pDissString = Diss_AddressOrLabel(pRange,pDissString,Diss_GetRelativeAddr(pRange,DisAddr));

								}
								else
								{
									/* 00000000 - NOP */
									/* 00001000 - EX AF,AF */
									pDissString = Diss_strcat(pDissString, MiscMneumonics2[((Opcode>>3) & 0x03)]);

								}

							}
						}
						break;

						case 1:
						{
							if ((Opcode & 0x08)!=0)
							{
								/* 00ss1001 - ADD HL,ss */
								pDissString = Diss_strcat(pDissString,"ADD HL,");
                                pDissString = Diss_strcat(pDissString,RegB[((Opcode>>4) & 0x03)]);

							}
							else
							{
								/* 00dd0001 - LD dd,nn */
								pDissString = Diss_strcat(pDissString,"LD");
								pDissString = Diss_space(pDissString);
								pDissString = Diss_strcat(pDissString,RegB[((Opcode>>4) & 0x03)]);
								pDissString = Diss_comma(pDissString);
								pDissString = Diss_AddressOrLabel(pRange,pDissString,MemoryRange_ReadWord(pRange,DisAddr));

							}
						}
						break;

						case 2:
						{
							switch ((Opcode>>4) & 0x03)
							{
								case 0:
								{
									/* 00000010 - LD (BC),A */
									/* 00001010 - LD A,(BC) */

									if ((Opcode & (1<<3))!=0)
									{
										pDissString = Diss_strcat(pDissString,"LD A,(BC)");
									}
									else
									{
										pDissString = Diss_strcat(pDissString,"LD (BC),A");
									}

								}
								break;

								case 1:
								{
									/* 00010010 - LD (DE),A */
									/* 00011010 - LD A,(DE) */

									if ((Opcode & (1<<3))!=0)
									{
										pDissString = Diss_strcat(pDissString,"LD A,(DE)");
									}
									else
									{
										pDissString = Diss_strcat(pDissString,"LD (DE),A");
									}
								}
								break;

								case 2:
								{
									/* 00100010 - LD (nnnn),HL */
									/* 00101010 - LD HL,(nn) */

									pDissString = Diss_strcat(pDissString,"LD");
									pDissString = Diss_space(pDissString);

									if ((Opcode & (1<<3))==0)
									{
										pDissString = Diss_ContentsOfAddress(pRange,pDissString,DisAddr);
										pDissString = Diss_comma(pDissString);
										pDissString = Diss_strcat(pDissString,"HL");
									}
									else
									{
										pDissString = Diss_strcat(pDissString,"HL");
										pDissString = Diss_comma(pDissString);
										pDissString = Diss_ContentsOfAddress(pRange,pDissString,DisAddr);
									}

								}
								break;

								case 3:
								{
									/* 00110010 - LD (nnnn),A */
									/* 00111010 - LD A,(nnnn) */

									pDissString = Diss_strcat(pDissString,"LD");
									pDissString = Diss_space(pDissString);

									if ((Opcode & (1<<3))==0)
									{
										pDissString = Diss_ContentsOfAddress(pRange,pDissString,DisAddr);
										pDissString = Diss_comma(pDissString);
										pDissString = Diss_strcat(pDissString,"A");
									}
									else
									{
										pDissString = Diss_strcat(pDissString,"A");
										pDissString = Diss_comma(pDissString);
										pDissString = Diss_ContentsOfAddress(pRange,pDissString,DisAddr);
									}

								}
								break;
							}

						}
						break;

						case 3:
						{
							char *Instruction;

							if ((Opcode & 0x08)==0)
							{
								/* 00ss0011 - INC ss */
								Instruction = "INC";
							}
							else
							{
								/* 00ss1011 - DEC ss */
								Instruction = "DEC";
							}

							pDissString = Diss_strcat(pDissString,Instruction);
							pDissString = Diss_space(pDissString);
							pDissString = Diss_strcat(pDissString,RegB[((Opcode>>4) & 0x03)]);

						}
						break;

						case 4:
						{
							/* 00rrr100 - INC r */
							pDissString = Diss_strcat(pDissString,"INC ");
                            pDissString = Diss_strcat(pDissString,RegA[((Opcode>>3) & 0x07)]);

						}
						break;

						case 5:
						{
							/* 00rrr101 - DEC r */
							pDissString = Diss_strcat(pDissString,"DEC ");
							pDissString = Diss_strcat(pDissString,RegA[((Opcode>>3) & 0x07)]);

						}
						break;

						case 6:
						{
							char Data = MemoryRange_ReadByte(pRange,DisAddr);
							/* LD r,n - 00rrr110 */
							pDissString = Diss_strcat(pDissString,"LD");
							pDissString = Diss_space(pDissString);
							pDissString = Diss_strcat(pDissString,RegA[(Opcode>>3) & 0x07]);
							pDissString = Diss_comma(pDissString);
							pDissString = Diss_WriteHexByte(pDissString,Data,TRUE,FALSE);


						}
						break;

						case 7:
						{

							const char **MneumonicsList;

							if ((Opcode & (1<<5))==0)
							{
								/* 00000111 - RLCA */
								/* 00001111 - RRCA */
								/* 00010111 - RLA */
								/* 00011111 - RRA */
								MneumonicsList = ShiftMneumonics;
							}
							else
							{
								/* 00100111 - DAA */
								/* 00101111 - CPL */
								/* 00110111 - SCF */
								/* 00111111 - CCF */
								MneumonicsList = MiscMneumonics1;
							}

							pDissString = Diss_strcat(pDissString,MneumonicsList[((Opcode>>3) & 0x03)]);

						}
						break;
					}
				}
				break;

				case 0x040:
				{
					/* 01xxxxxx */
					/* HALT, LD r,R */

					if (Opcode==0x076)
					{
						pDissString = Diss_strcat(pDissString,"HALT");
					}
					else
					{
						/* 01rrrRRR - LD r,R */
						pDissString = Diss_strcat(pDissString,"LD");
						pDissString = Diss_space(pDissString);
						pDissString = Diss_strcat(pDissString,RegA[((Opcode>>3) & 0x07)]);
						pDissString = Diss_comma(pDissString);
						pDissString = Diss_strcat(pDissString,RegA[(Opcode & 0x07)]);
					}

				}
				break;

				case 0x080:
				{
					/* 10xxxxxx */
					/* 10000rrr - ADD */
					/* 10001rrr - ADC */
					/* 10010rrr - SUB */
					/* 10011rrr - SBC */
					/* 10100rrr - AND */
					/* 10101rrr - XOR */
					/* 10110rrr - OR */
					/* 10111rrr - CP */
					pDissString = Diss_strcat(pDissString,ArithmeticMneumonics[((Opcode>>3) & 0x07)]);
					pDissString = Diss_space(pDissString);
					pDissString = Diss_strcat(pDissString,RegA[7]);
					pDissString = Diss_comma(pDissString);
					pDissString = Diss_strcat(pDissString,RegA[Opcode & 0x07]);

				}
				break;

				case 0x0c0:
				{
					/* 11xxxxxx */


					switch (Opcode & 0x07)
					{

						case 0:
						{
							/* 11 ccc 000 - RET cc */
							pDissString = Diss_strcat(pDissString,"RET ");
							pDissString = Diss_strcat(pDissString,ConditionCodes[((Opcode>>3) & 0x07)]);
						}
						break;

						case 1:
						{
							if ((Opcode & (1<<3))==0)
							{
								/* 11qq0001 - POP qq */
								pDissString = Diss_strcat(pDissString,"POP ");
								pDissString = Diss_strcat(pDissString,RegC[((Opcode>>4) & 0x03)]);
							}
							else
							{
								/* 11001001 - RET */
								/* 11011001 - EXX */
								/* 11101001 - JP (HL) */
								/* 11111001 - LD SP,HL */

								pDissString = Diss_strcat(pDissString,MiscMneumonics3[((Opcode>>4) & 0x03)]);
							}
						}
						break;

						case 2:
						{
							/* 11 ccc 010 - JP cc,nnnn */

							pDissString = Diss_strcat(pDissString,"JP");
							pDissString = Diss_space(pDissString);
							pDissString = Diss_strcat(pDissString,ConditionCodes[((Opcode>>3) & 0x07)]);
							pDissString = Diss_comma(pDissString);
							pDissString = Diss_AddressOrLabel(pRange,pDissString,MemoryRange_ReadWord(pRange,DisAddr));

						}
						break;

						case 3:
						{
							/* 11001011 - CB prefix */
							if ((Opcode & (3<<4))==(1<<4))
							{
								char PortByte;

								PortByte = MemoryRange_ReadByte(pRange,DisAddr);

								/* 11011011 - IN A,(n) */
								/* 11010011 - OUT (n),A */

								if ((Opcode & (1<<3))!=0)
								{
									/* 11011011 - IN A,(n) */

									pDissString = Diss_strcat(pDissString,"IN");
									pDissString = Diss_space(pDissString);
									pDissString = Diss_strcat(pDissString,"A");
									pDissString = Diss_comma(pDissString);
									pDissString = Diss_bracket_open(pDissString);
									pDissString = Diss_WriteHexByte(pDissString,PortByte,TRUE,FALSE);
									pDissString = Diss_bracket_close(pDissString);

								}
								else
								{
									/* 11010011 - OUT (n),A */
									pDissString = Diss_strcat(pDissString,"OUT");
									pDissString = Diss_space(pDissString);
									pDissString = Diss_bracket_open(pDissString);
									pDissString = Diss_WriteHexByte(pDissString,PortByte,TRUE,FALSE);
									pDissString = Diss_bracket_close(pDissString);
									pDissString = Diss_comma(pDissString);
									pDissString = Diss_strcat(pDissString,"A");

								}

							}
							else if (Opcode == 0x0c3)
							{
								/* 11000011 - JP nn */
								pDissString = Diss_strcat(pDissString,"JP");
								pDissString = Diss_space(pDissString);
								pDissString = Diss_AddressOrLabel(pRange,pDissString,MemoryRange_ReadWord(pRange,DisAddr));


							}
							else
							{
								/* 11100011 - EX (SP).HL */
								/* 11101011 - EX DE,HL */

								/* 11110011 - DI */
								/* 11111011 - EI */

								pDissString = Diss_strcat(pDissString,MiscMneumonics9[(((Opcode>>3) & 0x07)-4)]);
							}

						}
						break;

						case 4:
						{
							/* 11 ccc 100 - CALL cc,nnnn */
							pDissString = Diss_strcat(pDissString,"CALL");
							pDissString = Diss_space(pDissString);
							pDissString = Diss_strcat(pDissString,ConditionCodes[((Opcode>>3) & 0x07)]);
							pDissString = Diss_comma(pDissString);
							pDissString = Diss_AddressOrLabel(pRange,pDissString,MemoryRange_ReadWord(pRange,DisAddr));


						}
						break;


						case 5:
						{
							/* 11qq0101 - PUSH qq */
							if ((Opcode & (1<<3))==0)
							{
								/* 11qq0101 - PUSH qq */
								pDissString = Diss_strcat(pDissString,"PUSH ");
								pDissString = Diss_strcat(pDissString,RegC[((Opcode>>4) & 0x03)]);

							}
							else
							{
								/* 11001101 - CALL nn */
								/* 11011101 - DD */
								/* 11101101 - ED */
								/* 11111101 - FD */

								pDissString = Diss_strcat(pDissString,"CALL");
								pDissString = Diss_space(pDissString);
									pDissString = Diss_AddressOrLabel(pRange,pDissString,MemoryRange_ReadWord(pRange,DisAddr));

							}
						}
						break;

						case 6:
						{
							/* 11000110 - ADD n */
							/* 11001110 - ADC n */
							/* 11010110 - SUB n */
							/* 11011110 - SBC n */
							/* 10100110 - AND */
							/* 10101110 - XOR */
							/* 10110110 - OR */
							/* 10111110 - CP */
							char Data = MemoryRange_ReadByte(pRange,DisAddr);

							pDissString = Diss_strcat(pDissString,ArithmeticMneumonics[((Opcode>>3) & 0x07)]);
							pDissString = Diss_space(pDissString);
							pDissString = Diss_strcat(pDissString,RegA[7]);
							pDissString = Diss_comma(pDissString);
							pDissString = Diss_WriteHexByte(pDissString,Data,TRUE,FALSE);

						}
						break;

						case 7:
						{
							Addr = Opcode & 0x038;

							pDissString = Diss_strcat(pDissString,"RST");
							pDissString = Diss_space(pDissString);
							pDissString = Diss_WriteHexWord(pDissString,Addr,TRUE,FALSE);

						}
						break;
					}

				}
				break;
			}
		}
		break;
	}

	return pDissString;
}



/* return number of extra bytes required including prefix */
int	Diss_Index_Get8BitRegCount(char OpcodeIndex)
{
	/* High or Low byte of Index register e.g. HIX */
	if ((OpcodeIndex == 4) || (OpcodeIndex==5))
		return 1;

	/* Index register with offset e.g. (IX+dd) */
	if (OpcodeIndex == 6)
		return 2;

	/* not index register */
	return 0;
}


int	Diss_Index_GetOpcodeCountForInstruction(MemoryRange *pRange,int DisAddr)
{
	unsigned char Opcode;

	/* DD prefix - IX */
	Opcode = MemoryRange_ReadByte(pRange,DisAddr);
	DisAddr++;

	switch (Opcode)
	{
		case 0x0dd:
		case 0x0fd:
		case 0x0ed:
			return 1;

		case 0x0cb:
			return 4;

		default:
		{
			switch (Opcode & 0x0c0)
			{
				case 0x000:
				{
					switch (Opcode & 0x07)
					{
						case 1:
						{
							if ((Opcode & 0x08)!=0)
							{
								/* 00ss1001 - ADD HL,ss */
								return 2;
							}
							else
							{
								/* 00dd0001 - LD dd,nn */
								if (Diss_Index_IsRegIndex((Opcode>>4) & 0x03))
								{
									return 4;
								}
							}
						}
						break;

						case 2:
						{
							/* 00100010 - LD (nnnn),HL */
							/* 00101010 - LD HL,(nn) */

							switch ((Opcode>>4) & 0x03)
							{
								case 2:
									return 4;

								default:
									break;
							}

						}
						break;

						case 3:
						{
							if (Diss_Index_IsRegIndex((Opcode>>4) & 0x03))
								return 2;
						}
						break;

						case 4:
						case 5:
						{
							/* 00rrr100 - INC r */
							/* 00rrr101 - DEC r */
							char OpcodeIndex;

							OpcodeIndex = ((Opcode>>3) & 0x07);

							if (Diss_Index_IsReg8Bit(OpcodeIndex))
								return Diss_Index_Get8BitRegCount(OpcodeIndex) + 1;
						}
						break;

						case 6:
						{
							/* LD r,n - 00rrr110 */
							char OpcodeIndex;

							OpcodeIndex = (Opcode>>3) & 0x07;

							if (Diss_Index_IsReg8Bit(OpcodeIndex))
								return Diss_Index_Get8BitRegCount(OpcodeIndex) + 2;
						}
						break;

						default:
							break;
					}
				}
				break;

				case 0x040:
				{
					/* 01xxxxxx */
					/* HALT, LD r,R */

					if (Opcode!=0x076)
					{
						/* 01rrrRRR - LD r,R */

						char Reg1, Reg2;

						Reg1 = (Opcode>>3) & 0x07;
						Reg2 = Opcode & 0x07;

						if (Diss_Index_IsReg8Bit(Reg1) ||
							Diss_Index_IsReg8Bit(Reg2))
						{
							/* Reg1 or Reg2 is Indexed */
							char Length1, Length2;

							/* get longest length and return that */
							Length1 = Diss_Index_Get8BitRegCount(Reg1);
							Length2 = Diss_Index_Get8BitRegCount(Reg2);

							if (Length1<Length2)
							{
								return Length2 + 1;
							}

							return Length1 + 1;
						}
					}
				}
				break;

				case 0x080:
				{
					/* 10xxxxxx */
					/* 10000rrr - ADD */
					/* 10001rrr - ADC */
					/* 10010rrr - SUB */
					/* 10011rrr - SBC */
					/* 10100rrr - AND */
					/* 10101rrr - XOR */
					/* 10110rrr - OR */
					/* 10111rrr - CP */

					char OpcodeIndex;

					OpcodeIndex = (Opcode & 0x07);

					if (Diss_Index_IsReg8Bit(OpcodeIndex))
						return Diss_Index_Get8BitRegCount(OpcodeIndex) + 1;
				}
				break;

				case 0x0c0:
				{
					/* 11xxxxxx */


					switch (Opcode & 0x07)
					{
						case 1:
						{
							if ((Opcode & (1<<3))==0)
							{
								/* 11qq0001 - POP qq */
								if (Diss_Index_IsRegIndex((Opcode>>4) & 0x03))
									return 2;
							}
							else
							{
								/* 11001001 - RET */
								/* 11011001 - EXX */
								/* 11101001 - JP (HL) */
								/* 11111001 - LD SP,HL */

								if ((Opcode==0x0f9) || (Opcode==0x0e9))
									return 2;

							}
						}
						break;

						case 3:
						{
							/* 11001011 - CB prefix */
							if ((Opcode & (3<<4))==(1<<4))
							{

							}
							else if (Opcode == 0x0c3)
							{
							}
							else
							{
								/* 11100011 - EX (SP).HL */
								/* 11101011 - EX DE,HL */

								/* 11110011 - DI */
								/* 11111011 - EI */

								if (Opcode==0x0e3)
									return 2;

							}

						}
						break;


						case 5:
						{
							/* 11qq0101 - PUSH qq */
							if ((Opcode & (1<<3))==0)
							{
								/* 11qq0101 - PUSH qq */
								if (Diss_Index_IsRegIndex((Opcode>>4) & 0x03))
									return 2;
							}
						}
						break;

						default:
							break;
					}
				}
				break;
			}
		}
		break;
	}

	/* default is 1, which is the Index prefix */
	return 1;
}


int	Debug_GetOpcodeCount(MemoryRange *pRange,int Addr)
{
	unsigned char Opcode;
	int		DisAddr = Addr;

	Opcode = MemoryRange_ReadByte(pRange,DisAddr);
	DisAddr++;

	switch (Opcode)
	{
		case 0x0cb:
			/* 00000rrr - RLC */
			/* 00001rrr - RRC */
			/* 00010rrr - RL */
			/* 00011rrr - RR */
			/* 00100rrr - SLA */
			/* 00101rrr - SRA */
			/* 00110rrr - SLL */
			/* 00111rrr - SRL */
			/* 01bbbrrr - BIT */
			/* 10bbbrrr - RES */
			/* 11bbbrrr - SET */
			return 2;

		case 0x0ed:
		{
			/* ED prefix */
			Opcode = MemoryRange_ReadByte(pRange,DisAddr);
			DisAddr++;

			if ((Opcode & 0x0c0)==0x040)
			{
				switch (Opcode & 0x07)
				{
					case 0:
					case 1:
					case 2:
						/* IN r,(C) - 01rrr000 */
						/* OUT (C),r - 01rrr001 */
						/* ADC HL,ss - 01ss1010 */
						/* SBC HL,ss - 01ss0010 */
						return 2;

					case 3:
						/* LD dd,(nn) - 01dd1011 */
						/* LD (nn),dd - 01dd0011 */
						return 4;

					case 4:
					case 5:
					case 6:
					case 7:
						/* NEG - 01xxx100 */
						/* RETI - 01xx1010 */
						/* RETN - 01xx0010 */
						/* IM 0 - 01x00110 */
						/* IM ? - 01x01110 */
						/* IM 1 - 01x10110 */
						/* IM 2 - 01x11110 */
						return 2;
				}

			}
			else if ((Opcode & 0x0e4)==0x0a0)
			{

				switch (Opcode & 0x03)
				{
					case 0:
					case 1:
					case 2:
					case 3:
						/* 10100000 - LDI */
						/* 10101000 - LDD */
						/* 10110000 - LDIR */
						/* 10111000 - LDDR */
						/* 10100001 - CPI */
						/* 10111001 - CPDR */
						/* 10110001 - CPIR */
						/* 10101001 - CPD */
						/* 10100010 - INI */
						/* 10110010 - INIR */
						/* 10101010 - IND */
						/* 10111010 - INDR */
						/* 10100011 - OUTI */
						/* 10110011 - OTIR */
						/* 10101011 - outd */
						/* 10111011 - otdr */
						return 2;
				}
			}
			else
			{
				return 2;
			}
		}
		break;

		case 0x0dd:
		case 0x0fd:
			return Diss_Index_GetOpcodeCountForInstruction(pRange,DisAddr);

		default:
		{
			switch (Opcode & 0x0c0)
			{
				case 0x000:
				{
					switch (Opcode & 0x07)
					{
						case 0:
						{
							if ((Opcode & 0x020)!=0)
							{
								/* 001cc000 - JR cc */
								return 2;
							}
							else
							{

								if ((Opcode & 0x010)!=0)
								{
									/* 00010000 - DJNZ */
									/* 00011000 - JR */
									return 2;
								}
								else
								{
									return 1;
								}
							}

						}
						break;

						case 1:
						{
							if ((Opcode & 0x08)!=0)
							{
								/* 00ss1001 - ADD HL,ss */
								return 1;
							}

							/* 00dd0001 - LD dd,nn */
							return 3;
						}

						case 2:
						{
							switch ((Opcode>>4) & 0x03)
							{
								case 0:
								case 1:
									/* 00000010 - LD (BC),A */
									/* 00001010 - LD A,(BC) */
									/* 00010010 - LD (DE),A */
									/* 00011010 - LD A,(DE) */
									return 1;

								case 2:
								case 3:
									/* 00100010 - LD (nnnn),HL */
									/* 00101010 - LD HL,(nn) */
									/* 00110010 - LD (nnnn),A */
									/* 00111010 - LD A,(nnnn) */
									return 3;
							}

						}
						break;

						case 3:
						case 4:
						case 5:
							/* 00ss0011 - INC ss */
							/* 00ss1011 - DEC ss */
							/* 00rrr100 - INC r */
							/* 00rrr101 - DEC r */
							return 1;

						case 6:
							/* LD r,n - 00rrr110 */
							return 2;

						case 7:
							/* 00000111 - RLCA */
							/* 00001111 - RRCA */
							/* 00010111 - RLA */
							/* 00011111 - RRA */
							/* 00100111 - DAA */
							/* 00101111 - CPL */
							/* 00110111 - SCF */
							/* 00111111 - CCF */
							return 1;
					}
				}
				break;

				case 0x040:
					/* 01xxxxxx */
					/* 01110110 - HALT*/
					/* 01rrrRRR - LD r,R */
					return 1;

				case 0x080:
					/* 10xxxxxx */
					/* 10000rrr - ADD */
					/* 10001rrr - ADC */
					/* 10010rrr - SUB */
					/* 10011rrr - SBC */
					/* 10100rrr - AND */
					/* 10101rrr - XOR */
					/* 10110rrr - OR */
					/* 10111rrr - CP */
					return 1;

				case 0x0c0:
				{
					/* 11xxxxxx */

					/* 11110011 - DI */
					/* 11111011 - EI */
					/* 11101011 - EX DE,HL */
					/* 11011011 - IN A,(n) */
					/* 11010011 - OUT (n),A */
					/* 11000011 - JP */
					/* 11100011 - EX (SP).HL */


					switch (Opcode & 0x07)
					{

						case 0:
							/* 11 ccc 000 - RET cc */
							return 1;

						case 1:
						{
							if ((Opcode & (1<<3))==0)
							{
								/* 11qq0001 - POP qq */
								return 1;
							}
							else
							{
								/* 11001001 - RET */
								/* 11011001 - EXX */
								/* 11101001 - JP (HL) */
								/* 11111001 - LD SP,HL */

								return 1;
							}
						}
						break;

						case 2:
						case 4:
							/* 11 ccc 010 - JP cc,nnnn */
							/* 11 ccc 100 - CALL cc,nnnn */
							return 3;

						case 3:
						{
							if ((Opcode & (3<<4))==(1<<4))
							{
								/* 11011011 - IN A,(n) */
								/* 11010011 - OUT (n),A */
								return 2;
							}
							else if (Opcode == 0x0c3)
							{
								return 3;
							}
							else
							{
								return 1;
							}
						}
						break;

						case 5:
						{
							if ((Opcode & (1<<3))==0)
							{
								/* 11qq0101 - PUSH qq */
								return 1;
							}
							else
							{
								/* 11001101 - CALL nn */
								/* 11011101 - DD */
								/* 11101101 - ED */
								/* 11111101 - FD */
								return 3;
							}
						}
						break;

						case 6:
							/* 11000110 - ADD n */
							/* 11001110 - ADC n */
							/* 11010110 - SUB n */
							/* 11011110 - SBC n */
							/* 10100110 - AND */
							/* 10101110 - XOR */
							/* 10110110 - OR */
							/* 10111110 - CP */
							return 2;

						case 7:
							/* 11ttt111 - RST */
							return 1;
					}

				}
				break;
			}
		}
		break;
	}

	return 0;
}



int	Z80_GetRIncrementForInstruction(MemoryRange *pRange,int DisAddr)
{
	unsigned char Opcode;

	Opcode = MemoryRange_ReadByte(pRange,DisAddr);

	switch (Opcode)
	{
		case 0x0ed:
			return 2;

		case 0x0cb:
			return 2;

		case 0x0fd:
		case 0x0dd:
		{
			DisAddr++;
			Opcode = MemoryRange_ReadByte(pRange,DisAddr);

			switch (Opcode)
			{
				case 0x0dd:
				case 0x0fd:
				case 0x0ed:
					return 1;

				break;
			}
		}
		return 2;

		default:
			break;
	}

	return 1;
}

