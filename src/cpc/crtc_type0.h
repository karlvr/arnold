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
#ifndef __CRTC0_HEADER_INCLUDED__
#define __CRTC0_HEADER_INCLUDED__

int CRTC0_ReadStatusRegister(void);
void CRTC0_Reset(void);
void CRTC0_WriteData(int Data);
void CRTC0_DoLine(void);
int CRTC0_ReadData(void);
void CRTC0_DoHDisp(void);
void CRTC0_DoReg3(void);
void CRTC0_DoReg1(void);
void CRTC0_DoReg9(void);
void CRTC0_DoReg8(void);
int CRTC0_GetVerticalSyncWidth(void);
int CRTC0_GetHorizontalSyncWidth(void);
int CRTC0_GetRAOutput(void);

#endif
