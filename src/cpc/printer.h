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
#ifndef __CPC_PRINTER_HEADER_INCLUDED__
#define __CPC_PRINTER_HEADER_INCLUDED__


/* /strobe output from printer port */
BOOL Printer_GetStrobeOutputState(void);

void	Printer_SetBusyInput(BOOL bBusy);

BOOL	Printer_GetBusyState(void);

void Printer_SetStrobeState(BOOL bStrobe);

void	Printer_Write7BitData(int DataByte);

unsigned char Printer_Get7BitData(void);

void	Printer_Write8BitData(int DataByte);

unsigned char Printer_Get8BitData(void);

void	Printer_SetDataBit7State(BOOL bState);

/* implemented by host */
void Printer_RefreshOutputs(void);

#endif


