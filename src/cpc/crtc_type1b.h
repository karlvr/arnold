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
#ifndef __CRTC1B_HEADER_INCLUDED__
#define __CRTC1B_HEADER_INCLUDED__

int CRTC1B_ReadStatusRegister( void );
void CRTC1B_Reset(void);
void CRTC1B_WriteData(int Data);
void CRTC1B_DoLine(void);
int CRTC1B_ReadData(void);
int CRTC1B_ReadData(void);
void CRTC1B_DoReg3(void);
void CRTC1B_DoHDisp(void);
void CRTC1B_DoReg1(void);
void CRTC1B_DoReg9(void);
void CRTC1B_DoReg8(void);
int CRTC1B_GetVerticalSyncWidth(void);
int CRTC1B_GetRAOutput(void);
int CRTC1B_GetHorizontalSyncWidth(void);

#endif

