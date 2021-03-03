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
#include "keyjoy.h"
#include "cpc.h"

/* this holds a keyboard state for the keys pressed by keyjoy */
static unsigned char KeyJoyKeyboardData[16]=
{
	0x0ff,0x0ff,0x0ff,0x0ff,
	0x0ff,0x0ff,0x0ff,0x0ff,
	0x0ff,0x0ff,0x0ff,0x0ff,
	0x0ff,0x0ff,0x0ff,0x0ff,
};

/* set a key in the keyjoy keyboard data */
void KeyJoy_SetKey(CPC_KEY_ID KeyID)
{
 
      	if (KeyID!=CPC_KEY_NULL)
	{
		int Line = KeyID>>3;
		int Bit = KeyID & 0x07;

		KeyJoyKeyboardData[Line] &= ~(1<<Bit);
  }
}

/* release a key in the keyjoy keyboard data */
void KeyJoy_ClearKey(CPC_KEY_ID KeyID)
{
  
if (KeyID!=CPC_KEY_NULL)
	{
		int Line = KeyID>>3;
		int Bit = KeyID & 0x07;

		KeyJoyKeyboardData[Line] |= (1<<Bit);
	}
}

/* true if active, false otherwise */
static BOOL m_bKeyJoyActive = FALSE;
/* an id/index of the physical/host joypad to use */
static int m_nKeyJoyPhysical = -1;


/* mapping for when button is pressed */
static int ButtonMapping[MAXREDEFBUTTON];
/* mapping for axis */
static int AxisMapping[MAXREDEFAXIS*KEYJOY_AXIS_VALUE_NUM];
/* mapping for hat */
static int HatMapping[MAXREDEFHAT*HATNUMAXES*KEYJOY_AXIS_VALUE_NUM];
/* last action for this axis. Axis has min/none and max */
static CPC_KEY_ID AxisLast[MAXREDEFAXIS];
/* last action for this hat. Axis has min/none and max */
static CPC_KEY_ID HatLast[MAXREDEFHAT*HATNUMAXES];
/* last action for this button */
static CPC_KEY_ID ButtonLast[MAXREDEFBUTTON];


/* associate a cpc key or action to this button */
void KeyJoy_SetMappingJoyToKeyButton(int button,int key)
{
	if ((button<0) || (button >= MAXREDEFBUTTON)) return;

	ButtonMapping[button] = key;
}

/* get the cpc key or action associated with this button */
int KeyJoy_GetMappingJoyToKeyButton(int button)
{
  	if ((button<0) || (button >= MAXREDEFBUTTON)) return -1;
	return ButtonMapping[button];
}

/* associate a cpc key or action with this specific value on an axis. Value is min or max */
void KeyJoy_SetMappingJoyToKeyAxis(int axis, KEYJOY_AXIS_VALUE value,int key)
{
  int index = (axis*KEYJOY_AXIS_VALUE_NUM)+value;
	if ((axis<0) || (axis >= MAXREDEFAXIS)) return;
  if (value>=KEYJOY_AXIS_VALUE_NUM) return;
	AxisMapping[index] = key;
}

/* get the cpc key or action associated with this specific value on an axis */
int KeyJoy_GetMappingJoyToKeyAxis(int axis, KEYJOY_AXIS_VALUE value)
{
  int index = (axis*KEYJOY_AXIS_VALUE_NUM)+value;
	if ((axis<0) || (axis >= MAXREDEFAXIS)) return -1;
   if (value>=KEYJOY_AXIS_VALUE_NUM) return -1;
	return AxisMapping[index];
}


/* associate a cpc key or action with this specific value on an axis. Value is min or max */
void KeyJoy_SetMappingJoyToKeyHat(int hat, int axis, KEYJOY_AXIS_VALUE value,int key)
{
  int index = (hat*HATNUMAXES*KEYJOY_AXIS_VALUE_NUM)+value;
	if ((hat<0) || (hat>= MAXREDEFHAT)) return;
	if ((axis<0) || (axis >= HATNUMAXES)) return;
   if (value>=KEYJOY_AXIS_VALUE_NUM) return;
HatMapping[index] = key;
}

/* get the cpc key or action associated with this specific value on an axis */
int KeyJoy_GetMappingJoyToKeyHat(int hat, int axis, KEYJOY_AXIS_VALUE value)
{
  int index = (hat*HATNUMAXES*KEYJOY_AXIS_VALUE_NUM)+value;
	if ((hat<0) || (hat>= MAXREDEFHAT)) return -1;
	if ((axis<0) || (axis >= HATNUMAXES)) return -1;
   if (value>=KEYJOY_AXIS_VALUE_NUM) return -1;
	return HatMapping[index];
}

/* is there a special action associated with this value on this axis */
/* returns -1 if no special action set, otherwise the index of the special action */
int KeyJoy_GetSpecialActionAxis(int Axis, KEYJOY_AXIS_VALUE nValue)
{
  int r = KeyJoy_GetMappingJoyToKeyAxis(Axis, nValue);
  if (r>=SPECIALACTIONID)
  {
      return r-SPECIALACTIONID;
  }
  return -1;
}


/* is there a special action associated with this value on this axis */
/* returns -1 if no special action set, otherwise the index of the special action */
int KeyJoy_GetSpecialActionHat(int hat, int axis, KEYJOY_AXIS_VALUE nValue)
{
  int r = KeyJoy_GetMappingJoyToKeyHat(hat, axis, nValue);
  if (r>=SPECIALACTIONID)
  {
      return r-SPECIALACTIONID;
  }
  return -1;
}


/* is there a special action associated with this button?*/
int KeyJoy_GetSpecialActionButton(int button)
{
	int r;

	if (button >= MAXREDEFBUTTON) return -1;

  r = ButtonMapping[button];
  
  if (r>=SPECIALACTIONID)
  {
      return r-SPECIALACTIONID;
  }
  return -1;
}

/* update an input coming from a joypad axis */
void KeyJoy_UpdateKeyJoyAxisInput(int Axis, KEYJOY_AXIS_VALUE nValue)
{
  CPC_KEY_ID Key = CPC_KEY_NULL;
  /* get mapping */
  int r = KeyJoy_GetMappingJoyToKeyAxis(Axis, nValue);

  //printf("keyjoy axis %d value: %d key: %d\n",Axis,nValue, r);
  
  /* special mapping? */
  if (r>=SPECIALACTIONID)
  {
    /* mapped to a special action */
    return;
  }
  if (r!=-1)
  {
    Key = (CPC_KEY_ID)r;
  }

  /* store key */
  AxisLast[Axis] = Key;
	}

/* input is physical joystick button index, and pressed state */
void KeyJoy_UpdateKeyJoyButtonInput(int button,BOOL bPressed)
{
  CPC_KEY_ID Key = CPC_KEY_NULL;
 
    int r = KeyJoy_GetMappingJoyToKeyButton(button);
     printf("keyjoy button %d pressed: %s key: %d\n",button,bPressed ? "Yes" : "No", r);

  /* special mapping? */
  if (r>=SPECIALACTIONID)
  {
    /* mapped to a special action */
    return;
  }

  if (r!=-1)
  {
    Key = (CPC_KEY_ID)r;
  }

  if (bPressed)
  {
    ButtonLast[button] = Key;
  }
  else
  {
    ButtonLast[button] = CPC_KEY_NULL;
  }
}


/* update an input coming from a joypad axis */
void KeyJoy_UpdateKeyJoyHatInput(int Hat, int axis, KEYJOY_AXIS_VALUE nValue)
{
  CPC_KEY_ID Key = CPC_KEY_NULL;
  /* get mapping */
  int r = KeyJoy_GetMappingJoyToKeyHat(Hat, axis, nValue);
  int LastIndex = (Hat*HATNUMAXES)+axis;
  
  /* special mapping? */
  if (r>=SPECIALACTIONID)
  {
    /* mapped to a special action */
    return;
  }
  if (r!=-1)
  {
    Key = (CPC_KEY_ID)r;
  }
  
  /* different */
  
  /* store and press new */
  HatLast[LastIndex] = Key;
	}


/* set the index/id of the physical joypad controlling the joykey */
void KeyJoy_SetPhysical(int nPhysical)
{
  m_nKeyJoyPhysical = nPhysical;
}

/* get the index of the joypad controlling the joykey */
int KeyJoy_GetPhysical(void)
{
  return m_nKeyJoyPhysical;
}

void KeyJoy_Clear(void)
{
	int i;

	/* clear the keyboard data */
	memset(KeyJoyKeyboardData, 0x0ff, sizeof(KeyJoyKeyboardData));

	/* clear the values for the axis */
	for (i=0; i< MAXREDEFAXIS; i++)
	{
		AxisLast[i] = CPC_KEY_NULL;
	}
	/* clear the values for the axis */
	for (i=0; i< MAXREDEFHAT*HATNUMAXES; i++)
	{
		HatLast[i] = CPC_KEY_NULL;
	}
  /* clear the values for the axis */
	for (i=0; i< MAXREDEFBUTTON; i++)
	{
		ButtonLast[i] = CPC_KEY_NULL;
	}
}

void KeyJoy_Init(void)
{
  int i;
  
  for (i=0; i<MAXREDEFBUTTON; i++)
  {
      ButtonMapping[i] = -1;
  }
  for (i=0; i<MAXREDEFAXIS*KEYJOY_AXIS_VALUE_NUM; i++)
  {
      AxisMapping[i] = -1;
  }
   for (i=0; i<MAXREDEFHAT*HATNUMAXES*KEYJOY_AXIS_VALUE_NUM; i++)
  {
      HatMapping[i] = -1;
  }
  KeyJoy_Clear();
}

/* is the keyjoy active? */
BOOL KeyJoy_IsActive(void)
{
	return m_bKeyJoyActive;
}

void KeyJoy_Resolve(void)
{
  	int i;

	if (KeyJoy_IsActive())
	{
    /* this handles multiple inputs mapped to the same cpc key */
    /* it also only sets the key if it's still held by the axis, button or hat */
    /* clear the keyboard data */
    memset(KeyJoyKeyboardData, 0x0ff, sizeof(KeyJoyKeyboardData));

    /* clear the values for the axis */
    for (i=0; i< MAXREDEFAXIS; i++)
    {
      if (AxisLast[i]!=CPC_KEY_NULL)
      {
        KeyJoy_SetKey(AxisLast[i]);
      }
    }
    /* clear the values for the axis */
    for (i=0; i< MAXREDEFHAT*HATNUMAXES; i++)
    {
        if (HatLast[i]!=CPC_KEY_NULL)
      {
        KeyJoy_SetKey(HatLast[i]);
      }
    }
    /* clear the values for the axis */
    for (i=0; i< MAXREDEFBUTTON; i++)
    {
          if (ButtonLast[i]!=CPC_KEY_NULL)
      {
        KeyJoy_SetKey(ButtonLast[i]);
      }
    }

		CPC_ResolveKeys(KeyJoyKeyboardData);

	}
}

/* set if keyjoy is active or not */
void KeyJoy_SetActive(BOOL bState)
{
	m_bKeyJoyActive = bState;
  if (!bState)
  {
    KeyJoy_Clear();
  }
}
