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
/**************************************/
/* ASIC CPC+ DMA					  */
/* dissassemble instruction to buffer */
#include "gendiss.h"
#include "memrange.h"

int		ASIC_DMA_GetOpcodeCount(int Address)
{
	return 2;
}

int     ASIC_DMA_GetInstructionTiming(int Address)
{
    return 1;
}

char *ASIC_DMA_DissassembleInstruction(MemoryRange *pRange, int Address, char *pDissString)
{
		int        Command;
        int     CommandOpcode;

		/* make it even */
		Address = Address & (0x0ffff^1);

        /* fetch command */
        Command = MemoryRange_ReadWord(pRange, Address);

        /* get opcode */
        CommandOpcode = (Command>>12) & 0x07;

		/* dissassemble */
		if (CommandOpcode == 0x0)
		{
			/* LOAD R,D */
            unsigned long Register,Data;

            Register = (Command>>8) & 0x0f;

            Data = Command & 0x0ff;

			pDissString = Diss_strcat(pDissString,"LOAD");
			pDissString = Diss_space(pDissString);
			pDissString = Diss_WriteHexByte(pDissString,Register, TRUE, FALSE);
			pDissString = Diss_comma(pDissString);
			pDissString = Diss_WriteHexByte(pDissString,Data, TRUE, FALSE);

		}
		else
		{
			/* PAUSE? */
			 if ((CommandOpcode & (1<<0))!=0)
		     {
                /* PAUSE n */

				unsigned long PauseCount = Command & 0x0fff;

				pDissString = Diss_strcat(pDissString,"PAUSE");
				pDissString = Diss_space(pDissString);
				pDissString = Diss_WriteHexWord(pDissString,PauseCount,TRUE, FALSE);

				/* clear bit */
				CommandOpcode &=~(1<<0);

				/* any other bits set? */
				if (CommandOpcode!=0)
				{
					pDissString = Diss_colon(pDissString);
				}
			 }



			/* REPEAT? */
             if ((CommandOpcode & (1<<1))!=0)
             {
                 /* REPEAT n */

				 unsigned long RepeatCount = Command & 0x0fff;

				 pDissString = Diss_strcat(pDissString,"REPEAT");
				 pDissString = Diss_space(pDissString);
				 pDissString = Diss_WriteHexWord(pDissString,RepeatCount,TRUE, FALSE);
				 CommandOpcode &=~(1<<1);

	 			 /* any other bits set? */
				 if (CommandOpcode!=0)
				 {
					 pDissString = Diss_colon(pDissString);
				 }
			 }


			/* NOP, LOOP, INT or STOP? */
            if ((CommandOpcode & (1<<2))!=0)
            {
				Command &=0x01 | 0x0010 | 0x0020;

				if (Command == 0)
				{
					pDissString = Diss_strcat(pDissString,"NOP");
				}
				else
				{

					/* NOP, LOOP, INT, STOP */
					if (Command & 0x0001)
					{
						/* LOOP */

						pDissString = Diss_strcat(pDissString,"LOOP");

						Command &=~0x0001;

       					if (Command!=0)
						{
							pDissString = Diss_colon(pDissString);
						}
					}


					if (Command & 0x0010)
					{
						/* INT */

						pDissString = Diss_strcat(pDissString,"INT");

						Command &=~0x0010;

						if (Command !=0)
						{
							pDissString = Diss_colon(pDissString);
						}
					}



					if (Command & 0x0020)
					{
						/* STOP */
						pDissString = Diss_strcat(pDissString,"STOP");
					}
				}
            }
   		}
    return pDissString;
}
