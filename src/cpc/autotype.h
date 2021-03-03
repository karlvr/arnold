/*
 *  Arnold emulator (c) Copyright, Kevin Thacker 1995-2003
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
#ifndef __AUTOTYPE_HEADER_INCLUDED__
#define __AUTOTYPE_HEADER_INCLUDED__

#include "cpcglob.h"

void ASCII_to_CPC(int nASCII, BOOL bKeyDown);

/* auto-type is active and is "typing" */
#define AUTOTYPE_ACTIVE 0x01
/* auto-type is performing key release action */
/* if clear, auto-type is performing key press action */
#define AUTOTYPE_RELEASE 0x02
/* if set, auto-type is waiting for first keyboard scan to be done */
#define AUTOTYPE_WAITING 0x04
#define AUTOTYPE_ACTIVE_2 0x08
/* typing a fixed string, such as |TAPE, RUN, CAT and the autotype dialog */
/* if not set, then typing in translated mode */
#define AUTOTYPE_FIXED_STRING 0x010

extern const char *sAutoStringTape;
extern const char *sAutoStringRun;
extern const char *sAutoStringCat;

typedef struct
{
	/* 16-bit unicode character */
	int m_UTF16Ch;

	/* the string; as ascii characters to type */
	char *sUTF8String;

	/* number of frames to waste before continuing */
	int nFrames;

	unsigned long nFlags;

	int KeyBufferPosStart; /* this is the byte position within the buffer that we are reading from
									  for typing */
	int KeyBufferPosEnd; /* this is the byte position within the buffer that we are writing to for adding new chars */

	BOOL KeyBufferWraps; /* true if the buffer can wrap */
	int KeyBufferLength;
	int KeyBufferCapacity;  /* amount of bytes in the buffer */

	/* the currently pressed key */
	CPC_KEY_ID m_nCPCKey;
	BOOL m_bShift;
	BOOL m_bControl;
	
	BOOL bFreeBuffer;
	BOOL bResetCPC;
}  AUTOTYPE;

void AutoType_Init(void);
void AutoType_Finish(void);
BOOL AutoType_Active(void);

/* set the string to auto type */
void AutoType_SetString(const char *sUTF8String, BOOL bCopyBuffer, BOOL bWaitInput, BOOL bResetCPC);
void AutoType_AppendTypedString(const char *sUTF8String);
BOOL AutoType_IsFixedStringActive(void);

/* execute this every emulated frame; even if it will be skipped */
void AutoType_Update(void);

#endif
