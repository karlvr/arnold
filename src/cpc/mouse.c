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
#include "cpcglob.h"
#include "mouse.h"

static signed int MouseAbsX = 0;
static signed int MouseAbsY = 0;
static int MouseButtons = 0;

void Mouse_SetPosition(signed int PosX, signed int PosY)
{
	MouseAbsX = PosX;
	MouseAbsY = PosY;
}

signed int Mouse_GetX(void)
{
    return MouseAbsX;
}


signed int Mouse_GetY(void)
{
    return MouseAbsY;
}

int Mouse_GetButtons(void)
{
    return MouseButtons;
}

void Mouse_SetButtons(int nButton, BOOL bState)
{
    MouseButtons &=~(1<<nButton);
    if (bState)
    {
        MouseButtons |= (1<<nButton);
    }
}
