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
#ifndef __CRTC3_HEADER_INCLUDED__
#define __CRTC3_HEADER_INCLUDED__

int CRTC3_GetStatusRegister1(void);
int CRTC3_GetStatusRegister2(void);

int CRTC3_ReadStatusRegister(void);
void CRTC3_Reset(void);
void CRTC3_DoHDisp(void);
int CRTC3_GetRAOutput(void);
void ASICCRTC_DoLine(void);
void CRTC3_WriteData(int Data);
int CRTC3_ReadData(void);
void CRTC3_DoHDisp(void);
void CRTC3_DoReg3(void);
void CRTC3_DoReg1(void);
void CRTC3_DoReg9(void);
void CRTC3_DoReg8(void);
int CRTC3_GetVerticalSyncWidth(void);
int CRTC3_GetHorizontalSyncWidth(void);

#endif
