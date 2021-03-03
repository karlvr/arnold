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
#include "../memrange.h"

static int	Z80_Index_GetNopCountForInstruction(MemoryRange *pRange, int DisAddr)
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
			return 1;
		}

		case 0x0cb:
		{
			/* CB prefix */

			/* signed offset from IX */
			MemoryRange_ReadByte(pRange,DisAddr);
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

				return 7;
			}
			else
			{
				/* 01bbbrrr - BIT */
				/* 10bbbrrr - RES */
				/* 11bbbrrr - SET */

				if ((Opcode & 0x0c0)==0x040)
				{
					return 6;
				}

				return 7;
			}
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
						case 1:
						{
							if ((Opcode & 0x08)!=0)
							{
								/* 00ss1001 - ADD HL,ss */
								return 4;
							}
							else
							{
								/* 00dd0001 - LD dd,nn */
								return 4;
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
									return 6;
								}
								break;

								default:
									break;
							}


						}
						break;

						case 3:
							/* 00ss0011 - INC ss */
							/* 00ss1011 - DEC ss */
							return 3;

						case 4:
						case 5:
						{
							/* 00rrr100 - INC r */
							/* 00rrr101 - DEC r */
							if (((Opcode>>3) & 0x07)==6)
								return 6;

							return 2;

						}

						case 6:
						{
							/* LD r,n - 00rrr110 */
							if (((Opcode>>3) & 0x07)==6)
								return 6;

							return 3;
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
					unsigned char Reg1,Reg2;
					/* will not get here if defb &dd:HALT is encountered! */
					/* 01rrrRRR - LD r,R */

					Reg1 = (Opcode>>3) & 0x07;
					Reg2 = Opcode & 0x07;

					if ((Reg1==6) || (Reg2==6))
					{
						return 5;
					}

					return 2;
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

					if ((Opcode & 0x07)==6)
						return 5;

					return 2;
				}

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
								return 4;

							}
							else
							{
								/* 11001001 - RET */
								/* 11011001 - EXX */
								/* 11101001 - JP (HL) */
								/* 11111001 - LD SP,HL */

								if (Opcode==0x0e9)
									return 2;

								if (Opcode==0x0f9)
									return 3;
							}
						}
						break;

						case 3:
						{
							/* 11001011 - CB prefix */
							if ((Opcode & (3<<4))==(1<<4))
							{
								/* 11011011 - IN A,(n) */
								/* 11010011 - OUT (n),A */

							}
							else if (Opcode == 0x0c3)
							{
								/* 11000011 - JP nn */
							}
							else
							{
								/* 11100011 - EX (SP).HL */
								/* 11101011 - EX DE,HL */

								/* 11110011 - DI */
								/* 11111011 - EI */

								if (Opcode==0x0e3)
									return 7;
							}

						}
						break;

						case 5:
						{
							/* 11qq0101 - PUSH qq */
							if ((Opcode & (1<<3))==0)
							{
								return 5;
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

	return -1;
}

/* returns NOP count for whole instruction - values may be different
depending on flags! */
/* TODO: Handle flags? */
int		Z80_GetNopCountForInstruction(MemoryRange *pRange, int Addr)
{
	unsigned char Opcode;
	int		DisAddr = Addr;

	Opcode = MemoryRange_ReadByte(pRange,DisAddr);
	DisAddr++;

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

				/* (HL)? */
				if ((Opcode & 0x07)==6)
					return 4;

				return 2;
			}
			else
			{
				/* 01bbbrrr - BIT */
				/* 10bbbrrr - RES */
				/* 11bbbrrr - SET */


				/* (HL)? */
				if ((Opcode & 0x07)==6)
				{
					/* BIT? */
					if ((Opcode & 0x0c0)==0x040)
						return 3;

					/* RES/SET */
					return 4;
				}

				return 2;
			}
		}
		break;

		case 0x0fd:
		case 0x0dd:
			return Z80_Index_GetNopCountForInstruction(pRange,DisAddr);

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
						/* IN r,(C) - 01rrr000 */
						/* OUT (C),r - 01rrr001 */
						return 4;

					case 2:
						/* ADC HL,ss - 01ss1010 */
						/* SBC HL,ss - 01ss0010 */
						return 4;

					case 3:
						/* LD dd,(nn) - 01dd1011 */
						/* LD (nn),dd - 01dd0011 */
						return 6;

					case 4:
						/* NEG - 01xxx100 */
						return 2;

					case 5:
						/* RETI - 01xx1010 */
						/* RETN - 01xx0010 */
						return 4;

					case 6:
						/* IM 0 - 01x00110 */
						/* IM ? - 01x01110 */
						/* IM 1 - 01x10110 */
						/* IM 2 - 01x11110 */
						return 2;

					case 7:
					{
						if ((Opcode & 0x020)==0)
						{
							/* 01000111 - LD I,A */
							/* 01001111 - LD R,A */
							/* 01010111 - LD A,I */
							/* 01011111 - LD A,R */
							return 3;
						}

						if ((Opcode & 0x010)==0)
						{
							/* 01101111 - RLD */
							/* 01100111 - RRD */
							return 5;
						}

						/* 01110111 - ED NOP */
						/* 01111111 - ED NOP */

						return 2;
					}


				}

			}
			else if ((Opcode & 0x0e4)==0x0a0)
			{

				switch (Opcode & 0x03)
				{
					case 0:
					{
						/* 10100000 - LDI */
						/* 10101000 - LDD */
						/* 10110000 - LDIR */
						/* 10111000 - LDDR */

						/* correct? */
						if ((Opcode==0x0a0) || (Opcode==0x0a8))
							return 5;

					}
					break;

					case 1:
					{
						/* 10100001 - CPI */
						/* 10111001 - CPDR */
						/* 10110001 - CPIR */
						/* 10101001 - CPD */

						if ((Opcode==0x0a1) || (Opcode==0x0a9))
							return 4;


					}
					break;

					case 2:
					{
						/* 10100010 - INI */
						/* 10110010 - INIR */
						/* 10101010 - IND */
						/* 10111010 - INDR */

						/* correct? */
						if ((Opcode==0x0a2) || (Opcode==0x0aa))
							return 5;
					}
					break;

					case 3:
					{
						/* 10100011 - OUTI */
						/* 10110011 - OTIR */
						/* 10101011 - outd */
						/* 10111011 - otdr */

						/* correct? */
						if ((Opcode==0x0a3) || (Opcode==0x0ab))
							return 5;
					}
					break;

				}
			}
			else
				/* ED-NOP */
				return 2;
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


							}
							else
							{
								if ((Opcode & 0x010)!=0)
								{
									/* 00010000 - DJNZ */
									/* 00011000 - JR */

									if (Opcode==0x010)
									{
										/* conditional */
										/* will return -1 */
									}
									else
									{
										return 3;
									}

								}
								else
								{
									/* 00000000 - NOP */
									/* 00001000 - EX AF,AF */
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
								return 3;
							}

							/* 00dd0001 - LD dd,nn */
							return 3;
						}
						break;

						case 2:
						{
							switch ((Opcode>>4) & 0x03)
							{
								case 0:
									/* 00000010 - LD (BC),A */
									/* 00001010 - LD A,(BC) */
									return 2;

								case 1:
									/* 00010010 - LD (DE),A */
									/* 00011010 - LD A,(DE) */
									return 2;

								case 2:
									/* 00100010 - LD (nnnn),HL */
									/* 00101010 - LD HL,(nn) */
									return 5;

								case 3:
									return 4;
							}

						}
						break;

						case 3:
							/* 00ss0011 - INC ss */
							/* 00ss1011 - DEC ss */
							return 2;


						case 4:
						case 5:
							/* 00rrr100 - INC r */
							/* 00rrr101 - DEC r */
							if (((Opcode>>3) & 0x07)==6)
								return 3;

							return 1;

						case 6:
							/* LD r,n - 00rrr110 */
							if (Opcode==0x036)
                                return 3;

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
				{
					/* 01xxxxxx */
					/* HALT, LD r,R */

					/* 01rrrRRR - LD r,R */


					if (Opcode == 0x076)
					{
						return 1;
					}
					else
					{
						unsigned char Reg1,Reg2;

						Reg1 = ((Opcode>>3) & 0x07);
						Reg2 = (Opcode & 0x07);

						if ((Reg1==6) || (Reg2==6))
						{
							return 2;
						}

						return 1;
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

					if ((Opcode & 0x07)==6)
						return 2;

					return 1;
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
							/* will return -1 */
							

						}
						break;

						case 1:
						{
							if ((Opcode & (1<<3))==0)
							{
								/* 11qq0001 - POP qq */
								return 3;
							}
							else
							{
								/* 11001001 - RET */
								/* 11011001 - EXX */
								/* 11101001 - JP (HL) */
								/* 11111001 - LD SP,HL */

								switch (Opcode)
								{
									case 0x0c9:
										return 3;

									case 0x0d9:
										return 1;

									case 0x0e9:
										return 1;

									case 0x0f9:
										return 2;

									default:
										break;
								}
							}
						}
						break;

						case 2:
							/* 11 ccc 010 - JP cc,nnnn */
							return 3;

						case 3:
						{
							/* 11001011 - CB prefix */
							if ((Opcode & (3<<4))==(1<<4))
							{
								/* 11011011 - IN A,(n) */
								/* 11010011 - OUT (n),A */
								return 3;
							}
							else if (Opcode == 0x0c3)
							{
								/* 11000011 - JP nn */
								return 3;
							}
							else
							{
								/* 11100011 - EX (SP).HL */
								/* 11101011 - EX DE,HL */

								/* 11110011 - DI */
								/* 11111011 - EI */

								switch (Opcode)
								{
									case 0x0e3:
										return 6;
									case 0x0eb:
										return 1;
									case 0x0f3:
										return 1;
									case 0x0fb:
										return 1;

									default:
										break;
								}

							}

						}
						break;

						case 4:
						{
							/* 11 ccc 100 - CALL cc,nnnn */
							/* will return -1 */
						}
						break;


						case 5:
						{
							/* 11qq0101 - PUSH qq */
							if ((Opcode & (1<<3))==0)
							{
								/* 11qq0101 - PUSH qq */
								return 4;

							}
							else
							{
								/* 11001101 - CALL nn */
								/* 11011101 - DD */
								/* 11101101 - ED */
								/* 11111101 - FD */

								switch (Opcode)
								{
									case 0x0cd:
										return 5;

									default:
										break;
								}
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
							return 4;
					}

				}
				break;
			}
		}
		break;
	}

	return -1;
}

