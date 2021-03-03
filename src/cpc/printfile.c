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
#include "emudevice.h"
#include "crtc.h"
#include "printer.h"

static unsigned char m_PrinterData = 0;
static BOOL m_bPrinterStrobe = TRUE;

void PrintToFile_PrinterUpdateFunction(void)
{
        /* high to low causes data to be written */
        m_PrinterData = Printer_Get8BitData();

      if (m_bPrinterStrobe)
        {
            if (!Printer_GetStrobeOutputState())
            {
		    /* and output to file */
		CPC_PrintToFile(m_PrinterData);
                m_bPrinterStrobe = FALSE;
            }
        }
        else
        {
            if (Printer_GetStrobeOutputState())
            {
                m_bPrinterStrobe = TRUE;
            }
        }
	
}

/**** PrintToFile ****/

static EmuDevice PrintToFileDevice =
{
	NULL,
	NULL,
	NULL,
	"PRINTFILE",
	"PrintToFile",
	"To File",
	CONNECTION_PRINTER, 
	0,
  0,
  NULL,
  0,
  NULL,
  0,                /* no memory read*/
  NULL,
  0,                /* no memory write */
  NULL,
  NULL,				/* reset */
  NULL,
  NULL,
	0,                      /* no switches */
	NULL,
    0,                      /* no buttons */
    NULL,
    0,                      /* no onboard roms */
    NULL,
    NULL,                   /* no cursor update */
    NULL,                   /* no expansion roms */
	PrintToFile_PrinterUpdateFunction,					/* printer */
	NULL,					/* joystick */
	0,
	NULL,		/* memory ranges */
	NULL, /* sound */
	NULL, /* lpen */
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	NULL, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
};

void PrintToFileDevice_Init(void)
{
    RegisterDevice(&PrintToFileDevice);
}
