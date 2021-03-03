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
#include "cpc.h"
#include "kcc.h"
#include "aleste.h"
#include "asic.h"
#include "autorunfile.h"
#include "autotype.h"

typedef struct
{
	unsigned long nFlags;
    unsigned char *pData;
    unsigned long nDataLength;
    FILE_HEADER FileHeader;

	BOOL bResetCPC;
}  AUTORUNFILE;

static AUTORUNFILE AutoRunFile;
extern unsigned char *Z80MemoryBase;

/* init the auto type functions */
void AutoRunFile_Init(void)
{
	AutoRunFile.nFlags = 0;
	AutoRunFile.pData = NULL;
	AutoRunFile.nDataLength = 0;
	AutoRunFile.bResetCPC = FALSE;
}

BOOL AutoRunFile_Active(void)
{
	/* if actively typing, or waiting for first keyboard scan
	before typing then auto-type is active */
	return ((AutoRunFile.nFlags & (AUTOTYPE_ACTIVE|AUTOTYPE_WAITING))!=0);
}

void AutoRunFile_Finish(void)
{
    if (AutoRunFile.pData!=NULL)
    {
        free((void *)AutoRunFile.pData);
    }
    AutoRunFile.pData = NULL;
    AutoRunFile.nDataLength = 0;
    AutoRunFile.nFlags = 0;
}

/* set the string to auto type */
void AutoRunFile_SetData(const char *pData, unsigned long nDataLength, BOOL bWaitInput, BOOL bResetCPC, FILE_HEADER *pFileHeader)
{
	if (pData == NULL)
		return;
	if (nDataLength == 0)
		return;

	AutoRunFile.pData = (unsigned char *)malloc(nDataLength);
	if (AutoRunFile.pData == NULL)
		return;
	
	/* copy passed in data */
	memcpy(AutoRunFile.pData, pData, nDataLength);

    AutoRunFile.bResetCPC = bResetCPC;
	AutoRunFile.nDataLength = nDataLength;
	AutoRunFile.nFlags&=~AUTOTYPE_ACTIVE_2;
	memcpy(&AutoRunFile.FileHeader, pFileHeader, sizeof(FILE_HEADER));
	if (bWaitInput)
	{
	    if (bResetCPC)
	    {
            /* reset */
            Computer_RestartPower();
	    }

        Keyboard_ResetHasBeenScanned();

		/* wait for first keyboard */
		AutoRunFile.nFlags|=AUTOTYPE_WAITING;
		AutoRunFile.nFlags&=~AUTOTYPE_ACTIVE;
	}
	else
	{
		AutoRunFile.nFlags |= AUTOTYPE_ACTIVE;
	}
}

/* execute this every emulated frame; even if it will be skipped */
void AutoRunFile_Update(void)
{
	if ((AutoRunFile.nFlags & AUTOTYPE_ACTIVE)==0)
	{
		if ((AutoRunFile.nFlags & AUTOTYPE_WAITING)!=0)
		{
			if (Keyboard_HasBeenScanned())
			{
			    if (AutoRunFile.bResetCPC)
			    {
			        /* to handle CPC+, we need to do a reset,
			        wait for keyboard to be scanned which is when the menu
			        appears, then we need to do a second reset using
			        MC START PROGRAM. Now we have got to BASIC and can
			        autotype.


			        This solution also works with standard CPC too */

		            Keyboard_ResetHasBeenScanned();

			        if ((AutoRunFile.nFlags & AUTOTYPE_ACTIVE_2)!=0)
			        {
                        /* auto-type is now active */
                        AutoRunFile.nFlags |= AUTOTYPE_ACTIVE;
                        /* no longer waiting */
                        AutoRunFile.nFlags &=~AUTOTYPE_WAITING;
			        }
			        else
			        {
						/* can't execute any code at &c000-&ffff because this will clear
						the screen */

                        /* LD C,&ff */
                        Z80MemoryBase[0x04000] = 0x00e;
                        Z80MemoryBase[0x04001] = 0x0ff;
                        /* LD HL,&0 */
                        Z80MemoryBase[0x04002] = 0x021;
                        Z80MemoryBase[0x04003] = 0x000;
                        Z80MemoryBase[0x04004] = 0x000;
                        /* JP &BD16 - MC START PROGRAM */
                        Z80MemoryBase[0x04005] = 0x0c3;
                        Z80MemoryBase[0x04006] = 0x016;
                        Z80MemoryBase[0x04007] = 0x0bd;
                /* start executing code */
                        CPU_SetReg(CPU_PC,0x04000);

						/* we want to wait until the keyboard has been scanned again so we can do the auto-type */
                        AutoRunFile.nFlags |= AUTOTYPE_ACTIVE_2;


                    }
			    }
			    else
			    {
                    /* auto-type is now active */
                    AutoRunFile.nFlags |= AUTOTYPE_ACTIVE;
                    /* no longer waiting */
                    AutoRunFile.nFlags &=~AUTOTYPE_WAITING;
			    }
			}
		}
	}
	else
	{
        LoadBufferToRam(CPC_GetDefaultMemoryRange(), (const char *)AutoRunFile.pData, &AutoRunFile.FileHeader);


        switch ((AutoRunFile.FileHeader.HeaderFileType&(7<<1)))
        {
            case 2:
            {
		    /* binary */

                    /* LD C,&ff */
                    Z80MemoryBase[0x0bf00] = 0x00e;
                    Z80MemoryBase[0x0bf01] = 0x0ff;
                    /* LD HL,&0 */
                    Z80MemoryBase[0x0bf02] = 0x021;
                    Z80MemoryBase[0x0bf03] = AutoRunFile.FileHeader.HeaderExecutionAddress & 0x0ff;
                    Z80MemoryBase[0x0bf04] = (AutoRunFile.FileHeader.HeaderExecutionAddress>>8)&0x0ff;
                    /* JP &BD16 - MC START PROGRAM */
                    Z80MemoryBase[0x0bf05] = 0x0c3;
                    Z80MemoryBase[0x0bf06] = 0x016;
                    Z80MemoryBase[0x0bf07] = 0x0bd;
                    /* start executing code */
                    CPU_SetReg(CPU_PC,0x0bf00);
              }
            break;

            case 0:
            {
                unsigned short BasicEnd = AutoRunFile.FileHeader.HeaderStartAddress + AutoRunFile.FileHeader.HeaderLength;
                int nBasicVarBase = 0x00ae66;

                /* -1 is unknown, 0 is 464, 1 = 664, 6128, 6128+ */
                int nRomType = -1;

              const unsigned char *pBasicROM = NULL;
                switch (CPC_GetHardware())
              {
                  case CPC_HW_KCCOMPACT:
                  {
                      pBasicROM = KCC_GetBASICRom();
                  }
                  break;
                  case CPC_HW_ALESTE:
                  {
                      pBasicROM = Aleste_GetBASICRom();
                  }
                  break;
                  
                  case CPC_HW_CPCPLUS:
                  {
                      pBasicROM = Plus_GetBASICRom();
                  }
                  break;
                  
                  default:
                  {
                      /* CPC_HW_CPC */
                      pBasicROM = CPC_GetBASICROM();
                  }
                  break;
                }
                  
                  
                  
                if (pBasicROM!=NULL)
                {
                  if (pBasicROM[1]==1)
                  {
                      if (pBasicROM[2]==0)
                      {
                          nRomType = 0;
                          nBasicVarBase = 0x00ae83;
                      }
                      else if (
                          /* 664 version of basic */
                              (pBasicROM[2]==1) ||
                          /* 6128 version of basic */
                              (pBasicROM[2]==2) ||
                          /* cpc+ version of basic */
                              (pBasicROM[2]==4))
                      {
                          nRomType = 1;
                      }
                  }
                }
                
                if (nRomType!=-1)
                {

                    Z80MemoryBase[nBasicVarBase] = BasicEnd & 0x0ff;
                    ++nBasicVarBase;
                    Z80MemoryBase[nBasicVarBase] = (BasicEnd>>8) & 0x0ff;
                    ++nBasicVarBase;
                    Z80MemoryBase[nBasicVarBase] = BasicEnd & 0x0ff;
                    ++nBasicVarBase;
                    Z80MemoryBase[nBasicVarBase] = (BasicEnd>>8) & 0x0ff;
                    ++nBasicVarBase;
                    Z80MemoryBase[nBasicVarBase] = BasicEnd & 0x0ff;
                    ++nBasicVarBase;
                    Z80MemoryBase[nBasicVarBase] = (BasicEnd>>8) & 0x0ff;
                    ++nBasicVarBase;
                    Z80MemoryBase[nBasicVarBase] = BasicEnd & 0x0ff;
                    ++nBasicVarBase;
                    Z80MemoryBase[nBasicVarBase] = (BasicEnd>>8) & 0x0ff;
                    ++nBasicVarBase;
                  /* pass in string and don't copy */
                    AutoType_SetString(sAutoStringRun,FALSE,FALSE, FALSE);
                }
            }
            break;

		default:
			//printf("Autorunfile Can't handle header %d\n",AutoRunFile.FileHeader.HeaderFileType);
			break;


        }

        AutoRunFile_Finish();
	}
}
