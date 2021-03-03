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

/* keyjoy can be mapped to a key on the cpc keyboard OR a special action (e.g. reset emulator) */

#define HATNUMAXES 2

/* these apply to axis, which means diagonals can't be set (e.g up/left) */
/* the axes are treated seperatly and so are the min/max values. So you can have different settings for x min and x max */
typedef enum
{
  /* the following are used as index into an array */
	KEYJOY_AXIS_MIN = 0,
	KEYJOY_AXIS_MAX = 1,
	KEYJOY_AXIS_VALUE_NUM,

  /* this is a value we can ignore */
	KEYJOY_AXIS_MID = KEYJOY_AXIS_VALUE_NUM
} KEYJOY_AXIS_VALUE;

/* The hat can be described by an angle. It can also be described like a digital joystick with a bit for each direction.
Some hats can have more than up, down, left and right. But we don't support that. It could also be considered as having 4 buttons
where more than 1 can be pressed. We consider it as having 2 seperate axes */

void KeyJoy_Resolve(void);
void KeyJoy_Init(void);
BOOL KeyJoy_IsActive(void);
void KeyJoy_SetActive(BOOL bState);
int KeyJoy_GetSet(void);
int KeyJoy_GetPhysical(void);
void KeyJoy_SetPhysical(int);

/* input value will be min, max or mid */
/* if both axes are handled, then the result is a key press for both directions where diagonals are used */
void KeyJoy_UpdateKeyJoyAxisInput(int Axis, KEYJOY_AXIS_VALUE nValue);

/* input value will be a bit mask*/
/* if both left and up are passed in then the result is a press for both directions where diagonals are used */
void KeyJoy_UpdateKeyJoyHatInput(int hat, int axis, KEYJOY_AXIS_VALUE nValue);

/* input value is pressed or not */
void KeyJoy_UpdateKeyJoyButtonInput(int button,BOOL bPressed);

int KeyJoy_GetSpecialActionAxis(int Axis, KEYJOY_AXIS_VALUE nValue);
int KeyJoy_GetSpecialActionHat(int hat, int axis, KEYJOY_AXIS_VALUE value);
int KeyJoy_GetSpecialActionButton(int Button);

void KeyJoy_SetMappingJoyToKeyButton(int button,int mapping);
int KeyJoy_GetMappingJoyToKeyButton(int button);


void KeyJoy_SetMappingJoyToKeyAxis(int axis,KEYJOY_AXIS_VALUE value, int mapping);
int KeyJoy_GetMappingJoyToKeyAxis(int axis, KEYJOY_AXIS_VALUE value);

void KeyJoy_SetMappingJoyToKeyHat(int hat,int axis, KEYJOY_AXIS_VALUE value, int mapping);
int KeyJoy_GetMappingJoyToKeyHat(int hat, int axis, KEYJOY_AXIS_VALUE value);
