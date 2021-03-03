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
#ifndef __MULTIFACE_HEADER_INCLUDED__
#define __MULTIFACE_HEADER_INCLUDED__

#include "cpcglob.h"
#include "cpc.h"

/* indicates the multiface ram and rom are paged into the address space */
#define MULTIFACE_FLAGS_ACTIVE			0x0001
/* stop button has been pressed - debounce button effectively */
#define MULTIFACE_STOP_BUTTON_PRESSED		0x0002
/* multiface is "visible" (fee8/feea can be used) */
#define MULTIFACE_FLAGS_VISIBLE				0x0008

/* called when Multiface stop button is pressed */
void	Multiface_Stop(void);

void Multiface2_Init(void);
void Multiface_Initialise(void);

/* called to set memory read/write pointers when Multiface RAM/ROM is paged
in and out of Z80 address space */
void	Multiface_SetMemPointers(MemoryData *pData);

/* called when a machine reset is done */
void	Multiface_Reset(void);

void	Multiface_WriteIO(unsigned short, unsigned char);

#endif
