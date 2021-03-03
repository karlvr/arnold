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
#ifndef __CRTC2_HEADER_INCLUDED__
#define __CRTC2_HEADER_INCLUDED__

int CRTC2_ReadStatusRegister(void);
void CRTC2_Reset(void);
void CRTC2_WriteData(int Data);
void CRTC2_DoLine(void);
int CRTC2_ReadData(void);
void CRTC2_DoHDisp(void);
void CRTC2_DoReg3(void);
void CRTC2_DoReg1(void);
void CRTC2_DoReg9(void);
void CRTC2_DoReg8(void);
int CRTC2_GetVerticalSyncWidth(void);
int CRTC2_GetHorizontalSyncWidth(void);
int CRTC2_GetRAOutput(void);
void CRTC2_MaxRasterMatch(void);

#endif
