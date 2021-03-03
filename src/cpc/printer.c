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
#include "printer.h"
/*#include "host.h"*/



/* these represent final inputs/output on printer port */
BOOL m_bStrobe = FALSE;
BOOL m_bBusy = TRUE;

/* full 8 bit data */
unsigned char m_CurrentDataByte;

/* /strobe output from printer port */
BOOL Printer_GetStrobeOutputState(void)
{
	return m_bStrobe;
}


void	Printer_SetBusyInput(BOOL bBusy)
{
	m_bBusy = bBusy;
}


BOOL	Printer_GetBusyState(void)
{
	return m_bBusy;
}

void Printer_SetStrobeState(BOOL bStrobe)
{
	m_bStrobe = bStrobe;
	Printer_RefreshOutputs();
}

unsigned char Printer_Get8BitData(void)
{
    return m_CurrentDataByte;
}

void Printer_Write8BitData(int nDataByte)
{
    m_CurrentDataByte = nDataByte;
	Printer_RefreshOutputs();
}

/* write a 7-bit of data to printer port */
void	Printer_Write7BitData(int DataByte)
{
	m_CurrentDataByte  = (DataByte&0x07f)|(m_CurrentDataByte & (1<<8));
	Printer_RefreshOutputs();
}

unsigned char Printer_Get7BitData(void)
{
	return m_CurrentDataByte&0x07f;
}

void	Printer_SetDataBit7State(BOOL bState)
{
	m_CurrentDataByte  = m_CurrentDataByte & 0x07f;
	if (bState)
	{
		m_CurrentDataByte |= 0x080;
	}

	Printer_RefreshOutputs();
}

