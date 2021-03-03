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
 *
 *  Original implementation by Troels K.
 *  This implementation by Kev Thacker.
 *
 
 */
#include "cpc.h"
#include "autotype.h"
#include <string.h>

/* 
TOFIX:
autotype on plus:

idea is that keyboard is read by the menu, then a reset makes it go
to basic which then reads keyboard and allows autotype. reality is
it doesn't work properly.

keyboard can be read too early and autotype happens when menu is opened

debugger seems to break autotype if launched during typing?

also typing keys while autotyping if in translated mode appears to break it!
*/

/* tape if a disc system is active */
const char *sAutoStringTape = "|TAPE\nRUN\"\n\n\n\0";
/* auto run tape with no disc */
const char *sAutoStringTapeNoDisc = "RUN\"\n\n\n\0";
const char *sAutoStringRun = "RUN\n\0";
const char *sAutoStringCat = "CAT\n\0";

#define KEY_BUFFER_SIZE 8192
/* this is when we are using translated keyboard */
static char KeyBuffer[KEY_BUFFER_SIZE];

static AUTOTYPE AutoType;

/*
 this is for the translated keyboard mode

 this will translate a character/key into a CPC equivalent key combination.

 this is setup for BASIC.

  TODO: Approximate some accented characters into non-accented equivalent?

  need to identify difference between autotyping a pre-defined string, or autotyping 
  keys from keyboard.
*/

typedef struct
{
	unsigned int m_UTF16Ch;                    /* the character */
	 int     m_nLanguageMaskID;   /* which CPC keyboard language(s) this pertains to English, Spanish, Danish, French */
	CPC_KEY_ID     m_nCPCKeyID;             /* CPC key code to use. */
	BOOL    m_bShift;                       /* TRUE if CPC shift should be pressed */
	BOOL   m_bControl;                     /* TRUE if CPC control should be pressed */
} KeyTranslation;

typedef struct
{
	unsigned int m_UTF16ChOriginal;
	unsigned int m_UTF16ChNew;
} CharRemap;

static CharRemap RemappedChars[]=
{
	{
		/* non breaking space */
		0x0a0,' '
	},
	{
		/* broken bar */
		0x0a6, '|'
	},
	{
		/* left pointing double angle quotation */
		0x0ab, '"'
	},
	
	{
		/* right pointing double angle quotation */
		0x0bb, '"'
	},
	{
		/* en dash*/
		0x02013, '-'
	},
	{
		/* em dash*/
		0x02014, '-'
	},
	{
		/* horizontal bar */
		0x02015, '-'
	},
	{
		/* left single quotation mark*/
		0x02018, '\''
	},
	{
		/*Right single quotation mark */
		0x02019, '\''
	},
	{
		/* Single low-9 quotation mark */
		0x0201a, '\''
	},
	{
		/* Single high-reversed-9 quotation mark */
		0x0201b, '\''
	},
	{
		/* Left double quotation mark */
		0x0201c, '"'
	},
	{
		/* Right double quotation mark */
		0x0201d, '"'
	},
	{
		/* Double low-9 quotation mark */
		0x0201e, '"'
	},
	{
		/* prime */
		0x02032, '\''
	},
	{
		/* double prime */
		0x02033, '"'
	},
	{
		/* Single left-pointing angle quotation mark */
		0x02039, '"'
	},
	{
		/* Single right-pointing angle quotation mark */
		0x0203a, '"'
	},
	{
		/* Fraction slash */
		0x02044, '/'
	},
};


static KeyTranslation TranslatedKeys[]=
{
	/* these are made up */
	{
		0, KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH|KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_CURSOR_UP, FALSE, FALSE,
	},
	{
		1, KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH|KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_CURSOR_DOWN, FALSE, FALSE,
		},
		{
			2, KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH|KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_CURSOR_LEFT, FALSE, FALSE,
		},
		{
			3, KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH|KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_CURSOR_RIGHT, FALSE, FALSE,
			},


			/* attempted to match them against their ascii counterparts */
	{
		8, KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH|KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_DEL, FALSE, FALSE,
	},
	{
		9, KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH|KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_TAB, FALSE, FALSE,
	},
	{
		' ',KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH|KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_SPACE, FALSE, FALSE,
	},
	{
		10, KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH|KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_RETURN, FALSE, FALSE,
	},
	{
		13, KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH|KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_RETURN, FALSE, FALSE,
	},
	{
		27, KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH|KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_ESC, FALSE, FALSE,
	},
	{
		'a', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_A, FALSE, FALSE,
	},
	{
		'A', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_A, TRUE, FALSE,
	},
	{
		'a', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_Q, FALSE, FALSE,
	},
	{
		'A', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_Q, TRUE, FALSE,
	},
	{
		'b', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_B, FALSE, FALSE,
	},
	{
		'B', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_B, TRUE, FALSE,
	},
	{
		'c', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_C, FALSE, FALSE,
	},
	{
		'C', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_C, TRUE, FALSE,
	},
	{
		'd', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_D, FALSE, FALSE,
	},
	{
		'D', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_D, TRUE, FALSE,
	},
	{
		'e', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_E, FALSE, FALSE,
	},
	{
		'E', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_E, TRUE, FALSE,
	},
	{
		'f', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_F, FALSE, FALSE,
	},
	{
		'F', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_F, TRUE, FALSE,
	},
	{
		'g', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_G, FALSE, FALSE,
	},
	{
		'G', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_G, TRUE, FALSE,
	},
	{
		'h', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_H, FALSE, FALSE,
	},
	{
		'H', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_H, TRUE, FALSE,
	},
	{
		'i', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_I, FALSE, FALSE,
	},
	{
		'I', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_I, TRUE, FALSE,
	},
	{
		'j', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_J, FALSE, FALSE,
	},
	{
		'J', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_J, TRUE, FALSE,
	},
	{
		'k', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_K, FALSE, FALSE,
	},
	{
		'K', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_K, TRUE, FALSE,
	},
	{
		'l', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_L, FALSE, FALSE,
	},
	{
		'L', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_L, TRUE, FALSE,
	},
	{
		'm', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_M, FALSE, FALSE,
	},
	{
		'M', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_M, TRUE, FALSE,
	},
	{
		'm', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_COLON, FALSE, FALSE,
	},
	{
		'M', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_COLON, TRUE, FALSE,
	},
	{
		'n', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_N, FALSE, FALSE,
	},
	{
		'N', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_N, TRUE, FALSE,
	},
	{
		'o', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_O, FALSE, FALSE,
	},
	{
		'O', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_O, TRUE, FALSE,
	},
	{
		'p', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_P, FALSE, FALSE,
	},
	{
		'P', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_P, TRUE, FALSE,
	},
	{
		'q', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_Q, FALSE, FALSE,
	},
	{
		'Q', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_Q, TRUE, FALSE,
	},
	{
		'q', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_A, FALSE, FALSE,
	},
	{
		'Q', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_A, TRUE, FALSE,
	},
	{
		'r', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_R, FALSE, FALSE,
	},
	{
		'R', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_R, TRUE, FALSE,
	},
	{
		's', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_S, FALSE, FALSE,
	},
	{
		'S', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_S, TRUE, FALSE,
	},
	{
		't', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_T, FALSE, FALSE,
	},
	{
		'T', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_T, TRUE, FALSE,
	},
	{
		'u', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_U, FALSE, FALSE,
	},
	{
		'U', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_U, TRUE, FALSE,
	},
	{
		'v', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_V, FALSE, FALSE,
	},
	{
		'V', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_V, TRUE, FALSE,
	},
	{
		'w', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_W, FALSE, FALSE,
	},
	{
		'W', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_W, TRUE, FALSE,
	},
	{
		'w', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_Z, FALSE, FALSE,
	},
	{
		'W', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_Z, TRUE, FALSE,
	},
	{
		'x', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_X, FALSE, FALSE,
	},
	{
		'X', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_X, TRUE, FALSE,
	},
	{
		'y', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_Y, FALSE, FALSE,
	},
	{
		'Y', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_Y, TRUE, FALSE,
	},
	{
		'z', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_Z, FALSE, FALSE,
	},
	{
		'Z', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_Z, TRUE, FALSE,
	},
	{
		'z', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_W, FALSE, FALSE,
	},
	{
		'Z', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_W, TRUE, FALSE,
	},
	{
		'0', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_ZERO, FALSE, FALSE,
	},
	{
		'0', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_ZERO, TRUE, FALSE,
	},
	{
		'1', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_1, FALSE, FALSE,
	},
	{
		'1', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_1, TRUE, FALSE,
	},
	{
		'2', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_2, FALSE, FALSE,
	},

	{
		'2', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_2, TRUE, FALSE,
	},
	{
		'3', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_3, FALSE, FALSE,
	},
	{
		'3', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_3, TRUE, FALSE,
	},
	{
		'4', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_4, FALSE, FALSE,
	},
	{
		'4', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_4, TRUE, FALSE,
	},
	{
		'5', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_5, FALSE, FALSE,
	},
	{
		'5', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_5, TRUE, FALSE,
	},
	{
		'6', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_6, FALSE, FALSE,
	},
	{
		'6', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_6, TRUE, FALSE,
	},
	{
		'7', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_7, FALSE, FALSE,
	},
	{
		'7', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_7, TRUE, FALSE,
	},
	{
		'8', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_8, FALSE, FALSE,
	},
	{
		'8', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_8, TRUE, FALSE,
	},
	{
		'9', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_9, FALSE, FALSE,
	},
	{
		'9', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_9, TRUE, FALSE,
	},
	{
		'!', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_1, TRUE, FALSE,
	},
	{
		'!', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_8, FALSE, FALSE,
	},
	{
		'"', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_2, TRUE, FALSE,
	},
	{
		'"', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_3, FALSE, FALSE,
	},
	{
		'#', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_3, TRUE, FALSE,
	},
	{
		'#', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_CLOSE_SQUARE_BRACKET, FALSE, FALSE,
	},
	{
		'$', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_4, TRUE, FALSE,
	},
	{
		'$', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_BACKSLASH, FALSE, FALSE,
	},
	{
		'%', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_5, TRUE, FALSE,
	},
	{
		'%', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_SEMICOLON, TRUE, FALSE,
	},
	{
 '&', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_6, TRUE, FALSE,
	},
	{
 '&', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_1, FALSE, FALSE,
	},
	{
 '\'', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_7, TRUE, FALSE,
	},
	{
 '\'', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_4, FALSE, FALSE,
	},
	{
 '(', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_8, TRUE, FALSE,
	},
	{
 '(', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_5, FALSE, FALSE,
	},
	{
 ')', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_9, TRUE, FALSE,
	},
	{
     ')', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_MINUS, FALSE, FALSE,
	},
	{
     '_', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_ZERO, TRUE, FALSE,
	},
	{
 '_', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_HAT, TRUE, FALSE,
	},
	{
 '-', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_MINUS, FALSE, FALSE,
	},
	{
 '-', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_HAT, FALSE, FALSE,
	},
	{
 '=', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_MINUS, TRUE, FALSE,
	},
	{
 '=', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_FORWARD_SLASH, FALSE, FALSE,
	},
	{
		/* Pound sign */
    0x0a3, KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_HAT, TRUE, FALSE,
	},
	
 {
 '^', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_HAT, FALSE, FALSE,
	},
	{
 '^', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_AT, FALSE, FALSE,
	},
	{
 '@', KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_OPEN_SQUARE_BRACKET, FALSE, FALSE,
	},

	{
 '@', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH, CPC_KEY_AT, FALSE, FALSE,
	},

	/* it's on the CPC keyboard, but actually it displays a with grave */
#if 0
	{
       '@', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_BACKSLASH, TRUE, FALSE,
   },
#endif  
  /* NOTE: Use the same for french and danish because of the way it's translated by basic. */
	{
 '|', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_AT, TRUE, FALSE,
	},
#if 0
	 displays u with grave on french keyboards
  {
       '|', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_AT, TRUE, FALSE,
   },
#endif
	{
 '[', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH, CPC_KEY_OPEN_SQUARE_BRACKET, FALSE, FALSE,
	},
	{
 '[', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_MINUS, TRUE, FALSE,
	},
	{
 ']', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH, CPC_KEY_CLOSE_SQUARE_BRACKET, FALSE, FALSE,
	},
	{
 ']', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_6, FALSE, FALSE,
	},
	{
 '*', KEYBOARD_LANGUAGE_ID_ENGLISH, CPC_KEY_COLON, TRUE, FALSE,
	},
	{
 '*', KEYBOARD_LANGUAGE_ID_SPANISH, CPC_KEY_OPEN_SQUARE_BRACKET, TRUE, FALSE,
	},
	{
 '*', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_OPEN_SQUARE_BRACKET, FALSE, FALSE,
	},
	{
 '*', KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_CLOSE_SQUARE_BRACKET, TRUE, FALSE,
	},
	{
 '{', KEYBOARD_LANGUAGE_ID_ENGLISH, CPC_KEY_OPEN_SQUARE_BRACKET, TRUE, FALSE,
	},
	{
 '}', KEYBOARD_LANGUAGE_ID_ENGLISH, CPC_KEY_CLOSE_SQUARE_BRACKET, TRUE, FALSE,
	},
	{
 ':', KEYBOARD_LANGUAGE_ID_ENGLISH, CPC_KEY_COLON, FALSE, FALSE,
	},
	{
 ':', KEYBOARD_LANGUAGE_ID_SPANISH, CPC_KEY_SEMICOLON, TRUE, FALSE,
	},
	{
 ':', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_DOT, FALSE, FALSE,
	},
	{
 ':', KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_CLOSE_SQUARE_BRACKET, FALSE, FALSE,
	},
	{
 ';', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH, CPC_KEY_SEMICOLON, FALSE, FALSE,
	},
	{
 ';', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_COMMA, FALSE, FALSE,
	},
	{
 ';', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_COMMA, FALSE, FALSE,
	},
	{
 ';', KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_BACKSLASH, FALSE, FALSE,
	},
	{
		/* latin capital letter N with tilde */
     0x0d1, KEYBOARD_LANGUAGE_ID_SPANISH, CPC_KEY_COLON, TRUE, FALSE,
	},
	{
		/* latin small letter N with tilde*/
    0x0f1, KEYBOARD_LANGUAGE_ID_SPANISH, CPC_KEY_COLON, FALSE, FALSE,
	},
	{
		/* latin capital letter N with tilde - approximated on English, French and Danish*/
    0x0d1, KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_N, TRUE, FALSE,
	},
	{
		/* latin small letter N with tilde - approximated on English, French and Danish*/
     0x0f1, KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_FRENCH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_N, FALSE, FALSE,
	},
	{
 '+', KEYBOARD_LANGUAGE_ID_ENGLISH, CPC_KEY_SEMICOLON, TRUE, FALSE,
	},
	{
 '+', KEYBOARD_LANGUAGE_ID_SPANISH, CPC_KEY_CLOSE_SQUARE_BRACKET, TRUE, FALSE,
	},
	{
 '+', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_FORWARD_SLASH, TRUE, FALSE,
	},
	{
 '+', KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_BACKSLASH, TRUE, FALSE,
	},
	{
 ',', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_COMMA, FALSE, FALSE,
	},
	{
 ',', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_M, FALSE, FALSE,
	},
	{
 '<', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_COMMA, TRUE, FALSE,
	},
	{
 '<', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_OPEN_SQUARE_BRACKET, TRUE, FALSE,
	},
	{
 '.', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_DOT, FALSE, FALSE,
	},
	{
 '.', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_COMMA, TRUE, FALSE,
	},
	{
 '>', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_DOT, TRUE, FALSE,
	},
	{
 '>', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_CLOSE_SQUARE_BRACKET, TRUE, FALSE,
	},
	{
 '/', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_FORWARD_SLASH, FALSE, FALSE,
	},
	{
 '/', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_DOT, TRUE, FALSE,
	},
	{
 '?', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_FORWARD_SLASH, TRUE, FALSE,
	},
	{
 '?', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_M, TRUE, FALSE,
	},
	{
 '\\', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH, CPC_KEY_BACKSLASH, FALSE, FALSE,
	},
	{
 '`', KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH, CPC_KEY_BACKSLASH, TRUE, FALSE,
	},
	{
 '`', KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_4, FALSE, TRUE,
	},
	{
	 // Peseta
	 0x020a7, KEYBOARD_LANGUAGE_ID_SPANISH, CPC_KEY_HAT, TRUE, FALSE,
	 },
	 // macbook backspace.
	{
		0x07f, KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH|KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_DEL, FALSE, FALSE,
	},
	 {
		 // latin capital letter u with grave
		 0x0d9, KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_SEMICOLON, FALSE, FALSE,
	 },
			 {
				 // latin small letter u with grave
				 0x0f9,KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_SEMICOLON, FALSE, FALSE,
				 },
				 {
					 // latin capital letter u with grave - approximated
					 0x0d9,KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_U, TRUE, FALSE,
				 },
				 {
					 // latin small letter u with grave - approximated
					 0x0f9,KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_U, FALSE, FALSE,
					 },
					 {
						 // latin small letter a with grave
						 0x0e0,KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_ZERO, FALSE, FALSE,
					 },
					 {
						 // latin capital letter a with grave
						 0x0c0,KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_ZERO, FALSE, FALSE,
						 },
						 {
							 // latin small letter a with grave - approximated
							 0x0e0,KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_A, FALSE, FALSE,
						 },
						 {
							 // latin capital letter a with grave - approximated
							 0x0c0,KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_A, TRUE, FALSE,
							 },
							 {
								 // latin small letter c with Cedilla
								0x0e7,KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_9, FALSE, FALSE,
							 },
							 {
								 // latin capital letter c with Cedilla
								 0x0c7,KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_9, FALSE, FALSE,
								 },
								 {
									 // latin small letter c with Cedilla - approximated
									0x0e7,KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_C, FALSE, FALSE,
								 },
								 {
									 // latin capital letter c with Cedilla - approximated
									 0x0c7,KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_C, TRUE, FALSE,
									 },

									 {
										 // latin small letter e with grave
										 0x0e8,KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_7, FALSE, FALSE,
									 },
									 {
										 // latin capital letter e with grave
										 0x0c8,KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_7, FALSE, FALSE,
										 },
										 {
											 // latin small letter e with grave - approximated
											 0x0e8,KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_E, FALSE, FALSE,
										 },
										 {
											 // latin capital letter e with grave - approximated
											0x0c8,KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_E, TRUE, FALSE,
											 },
											 {
												 // latin capital letter e with acute
												0x0c9,KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_2, FALSE, FALSE,
												 },
											 {
												 // latin small letter e with acute
												0x0e9,KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_2, FALSE, FALSE,
												 },
												 {
													 // latin capital letter e with acute
													0x0c9,KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_E, TRUE, FALSE,
												 },
												 {
													 // latin small letter e with acute
													0x0e9,KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_E, FALSE, FALSE,
													 },

													 {
														 // latin capital letter Ae
														0x0c6,KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_COLON, TRUE, FALSE,
													 },
													 {
														 // latin small letter Ae
														0x0e6,KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_COLON, FALSE, FALSE,
														 },
														 {
															 // latin capital letter o with stroke
															0x0d8,KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_SEMICOLON, TRUE, FALSE,
														 },
														 {
															 // latin small letter o with stroke
															 0x0f8,KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_SEMICOLON, FALSE, FALSE,
															 },
															 {
																 // latin capital letter o with stroke - approximated
																  0x0d8,KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_O, TRUE, FALSE,
															 },
															 {
																 // latin small letter o with stroke - approximated
																 0x0f8,KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH|KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_O, FALSE, FALSE,
																 },

																 {
																	 // latin small  letter a with ring
																	 0x0e5,KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_AT, FALSE, FALSE,
																 },
																 {
																	 // latin capital letter a with ring
																	  0x0c5,KEYBOARD_LANGUAGE_ID_DANISH, CPC_KEY_AT, TRUE, FALSE,
																	 },
																	 {
																		 // latin small  letter a with ring - approximated
																		 0x0e5,KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH, CPC_KEY_A, FALSE, FALSE,
																	 },
																	 {
																		 // latin capital letter a with ring - approximated
																		 0x0c5,KEYBOARD_LANGUAGE_ID_ENGLISH|KEYBOARD_LANGUAGE_ID_SPANISH, CPC_KEY_A, TRUE, FALSE,
																		 },
																		 {
																			 // latin small  letter a with ring - approximated
																			 0x0e5,KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_Q, FALSE, FALSE,
																		 },
																		 {
																			 // latin capital letter a with ring - approximated
																			 0x0c5,KEYBOARD_LANGUAGE_ID_FRENCH, CPC_KEY_Q, TRUE, FALSE,
																			 },	
};

static void AutoType_UpdateReadPos(void)
{
	AutoType.KeyBufferPosStart++;
	if (AutoType.KeyBufferWraps)
	{
		if (AutoType.KeyBufferPosStart >= AutoType.KeyBufferLength)
		{
			AutoType.KeyBufferPosStart -= AutoType.KeyBufferLength;
		}
	}
	else
	{
		if (AutoType.KeyBufferPosStart >= AutoType.KeyBufferLength)
		{
			AutoType.KeyBufferPosStart = AutoType.KeyBufferLength;
		}
	}
	AutoType.KeyBufferCapacity++;
}

static char AutoType_ReadByte(void)
{
	char data = AutoType.sUTF8String[AutoType.KeyBufferPosStart];
	AutoType_UpdateReadPos();
	return data;
}


static void AutoType_UpdateWritePos(void)
{
	AutoType.KeyBufferPosEnd++;
	if (AutoType.KeyBufferWraps)
	{
		if (AutoType.KeyBufferPosEnd >= AutoType.KeyBufferLength)
		{
			AutoType.KeyBufferPosEnd -= AutoType.KeyBufferLength;
		}
	}
	else
	{
		if (AutoType.KeyBufferPosEnd >= AutoType.KeyBufferLength)
		{
			AutoType.KeyBufferPosEnd = AutoType.KeyBufferLength;
		}
	}
	AutoType.KeyBufferCapacity--;
}

static void AutoType_WriteByte(char ch)
{
	AutoType.sUTF8String[AutoType.KeyBufferPosEnd] = ch;
	AutoType_UpdateWritePos();
}

static BOOL AutoType_IsFinished(void)
{
	if (AutoType_IsFixedStringActive())
	{
		if (AutoType.KeyBufferPosStart == AutoType.KeyBufferLength)
		{
			return TRUE;
		}
		return FALSE;
	}

	if (AutoType.KeyBufferPosStart == AutoType.KeyBufferPosEnd)
	{
		return TRUE;
	}

	return FALSE;
}

static unsigned int AutoType_GetNextChar(void)
{
	uint32_t code = 0;
	int nbytes = 1;
	int i;

	char data = AutoType_ReadByte();
	
	char bytes = data;
	if (data == 0)
	{
		/* eol */
		return code;
	}

	if ((bytes & 0x080) != 0)
	{
		while ((bytes & 0x080) != 0)
		{
			bytes = bytes << 1;
			nbytes++;
		}
		nbytes--;
	}

	//printf("utf-8 char %d bytes\n", nbytes);

	if (nbytes == 1)
	{
		/* ascii */
		code = data & 0x07f;
		//printf("%02x\n",code);
		return code;
	}

	code = data & ((1 << (8 - (nbytes + 1))) - 1);

	for (i = 0; i < nbytes - 1; i++)
	{
		code = code << 6;
		data = AutoType_ReadByte();
		code = code | (data & 0x03f);
	}
	
	return code;

}

int AutoType_RemapChar(unsigned int nUTF16Char)
{
	int i;

	int nMap = sizeof(RemappedChars)/sizeof(RemappedChars[0]);
	for (i = 0; i < nMap; i++)
	{
		if (RemappedChars[i].m_UTF16ChOriginal==nUTF16Char)
		{
			return RemappedChars[i].m_UTF16ChNew;
		}
	}
	return nUTF16Char;
}
	
void AutoType_ReleaseKey(void)
{
	if (AutoType.m_nCPCKey!=CPC_KEY_NULL)
	{	
		/* shift*/
		if (AutoType.m_bShift)
		{
			CPC_ClearKey(CPC_KEY_SHIFT);
		}
		/*control*/
		if (AutoType.m_bControl)
		{
		  CPC_ClearKey(CPC_KEY_CONTROL);
		}
		/* key itself*/
		CPC_ClearKey(AutoType.m_nCPCKey);

		/* mark as released */
		AutoType.m_nCPCKey = CPC_KEY_NULL;
	}
}

BOOL AutoType_CharToCPC(unsigned int nUTF16Char)
{
	int i;
	KeyTranslation *pMap = TranslatedKeys;
	int nMap = sizeof(TranslatedKeys)/sizeof(TranslatedKeys[0]);

	int RemappedChar = AutoType_RemapChar(nUTF16Char);

	for (i = 0; i < nMap; i++)
	{
		if (
			  /* char matches */
			  (pMap->m_UTF16Ch == RemappedChar) &&
			/* used on the current keyboard language? */
			(pMap->m_nLanguageMaskID & Keyboard_GetLanguage()) 
	    )
		{
			/* store for release */
			AutoType.m_nCPCKey = pMap->m_nCPCKeyID;
			AutoType.m_bShift = pMap->m_bShift;
			AutoType.m_bControl = pMap->m_bControl;
			
			/*shift?*/
			if (AutoType.m_bShift)
			{
				CPC_SetKey(CPC_KEY_SHIFT);
			}
			/*control?*/
			if (AutoType.m_bControl)
			{
				CPC_SetKey(CPC_KEY_CONTROL);
			}
			/* key itself*/
			CPC_SetKey(AutoType.m_nCPCKey);
			      
			/*found key */
			return TRUE;
		}

		pMap++;
	}
	return FALSE;
}


/* init the auto type functions */
void AutoType_Init(void)
{
	//printf("auto type init\n");
	AutoType.nFlags = 0;
	AutoType.sUTF8String = NULL;
	AutoType.bFreeBuffer = FALSE;
	AutoType.nFrames = 0;
	AutoType.bResetCPC = 0;
	AutoType.m_nCPCKey = CPC_KEY_NULL;
	AutoType.m_bShift = FALSE;
	AutoType.m_bControl = FALSE;
	AutoType.KeyBufferPosEnd = 0;
	AutoType.KeyBufferPosStart = 0;
	AutoType.KeyBufferWraps = FALSE;
	AutoType.KeyBufferCapacity = 0;
	AutoType.KeyBufferLength = 0;
}

BOOL AutoType_IsFixedStringActive(void)
{
	return ((AutoType.nFlags & AUTOTYPE_FIXED_STRING) != 0);
}

BOOL AutoType_Active(void)
{
	/* if actively typing, or waiting for first keyboard scan
	before typing then auto-type is active */
	return ((AutoType.nFlags & (AUTOTYPE_ACTIVE|AUTOTYPE_WAITING))!=0);
}



void AutoType_Finish(void)
{
	AutoType_ReleaseKey();

	/* reset flags */
	AutoType.nFlags = 0;

	/* free up buffer if we were asked to */
	if (AutoType.bFreeBuffer)
	{
		if (AutoType.sUTF8String!=NULL)
		{
			free((void *)AutoType.sUTF8String);
			AutoType.sUTF8String = NULL;
		}
	}
}

void AutoType_AppendTypedString(const char *sUTF8String)
{
	/* if autotype is not active we can press a translated key */
	if (AutoType_IsFixedStringActive())
		return;

	if (!AutoType_Active())
	{
		AutoType_Init();
		AutoType.KeyBufferCapacity = sizeof(KeyBuffer);
		AutoType.KeyBufferLength = sizeof(KeyBuffer);
		AutoType.KeyBufferWraps = TRUE;
		AutoType.sUTF8String = KeyBuffer;
		AutoType.nFlags |= AUTOTYPE_ACTIVE;
	}
	if (sUTF8String != NULL)
	{
		int i;
		int lengthToAppend = strlen(sUTF8String);
		if (AutoType.KeyBufferCapacity >= lengthToAppend)
		{
			for (i = 0; i < lengthToAppend; i++)
			{
				AutoType_WriteByte(sUTF8String[i]);
			}
		}
	}
}


/* set the string to auto type (utf-8) */
void AutoType_SetString(const char *sUTF8String, BOOL bCopyBuffer, BOOL bWaitInput, BOOL bResetCPC)
{
	size_t nLength ;
  AutoType_Finish();
  
  
	/* if null pointer then quit */
  if (sUTF8String == NULL)
  {
    return;
  }
  
	/* get length of string in bytes */
  nLength = strlen(sUTF8String);
  
	/* if zero length then quit */
	if (nLength==0)
	{
		return;
	}
	AutoType.KeyBufferCapacity = nLength;
	AutoType.KeyBufferWraps = FALSE;
	AutoType.KeyBufferPosStart = 0;
	AutoType.KeyBufferPosEnd = 0;
	AutoType.KeyBufferLength = nLength;

	/* should reset CPC? */
	AutoType.bResetCPC = bResetCPC;
	AutoType.m_UTF16Ch = 0;
	AutoType.nFrames = 0;
	AutoType.nFlags |= AUTOTYPE_FIXED_STRING;
	AutoType.nFlags&=~AUTOTYPE_ACTIVE_2;
  
  if (bCopyBuffer)
  {

		/* allocate space including null terminator */
	  AutoType.sUTF8String = (char *)malloc(nLength + 1);
	  if (AutoType.sUTF8String != NULL)
		{ 
				/* copy string into buffer */
			memcpy((char *)AutoType.sUTF8String, sUTF8String, nLength + 1);
				/* free our buffer when done */
				AutoType.bFreeBuffer = TRUE;
		}
		

	}
	else
	{
		/* we don't free the string */
		AutoType.bFreeBuffer = FALSE;
		/* string must exist for the duration of the autotype. So it must be static */
		AutoType.sUTF8String = (char *)sUTF8String;
	}
	
	if (bWaitInput)
	{

		if (bResetCPC)
		{
			/* reset */
			Computer_RestartPower();
		}

		Keyboard_ResetHasBeenScanned();

		/* wait for first keyboard */
		AutoType.nFlags|=AUTOTYPE_WAITING;
		AutoType.nFlags&=~AUTOTYPE_ACTIVE;
	}
	else
	{
		AutoType.nFlags |= AUTOTYPE_ACTIVE;
	}

	//printf("auto type string: %s\n", AutoType.sUTF8String);
}

/* execute this every emulated frame; even if it will be skipped */
void AutoType_Update(void)
{
	/* auto type is not active.. */
	if ((AutoType.nFlags & AUTOTYPE_ACTIVE)==0)
	{
		/* auto type is waiting? */
		if ((AutoType.nFlags & AUTOTYPE_WAITING)!=0)
		{
			if (Keyboard_HasBeenScanned())
			{
				if (AutoType.bResetCPC)
				{
					/* to handle CPC+, we need to do a reset,
					wait for keyboard to be scanned which is when the menu
					appears, then we need to do a second reset using
					MC START PROGRAM. Now we have got to BASIC and can
					autotype.

					This solution also works with standard CPC too */

					Keyboard_ResetHasBeenScanned();

					if ((AutoType.nFlags & AUTOTYPE_ACTIVE_2)!=0)
					{
						/* auto-type is now active */
						AutoType.nFlags |= AUTOTYPE_ACTIVE;
						/* no longer waiting */
						AutoType.nFlags &=~AUTOTYPE_WAITING;
					}
					else
					{
						extern char *Z80MemoryBase;

						/* LD C,&ff */
						Z80MemoryBase[0x04000] = (char)0x0e;
						Z80MemoryBase[0x04001] = (char)0xff;
						/* LD HL,&0 */
						Z80MemoryBase[0x04002] = (char)0x21;
						Z80MemoryBase[0x04003] = (char)0x00;
						Z80MemoryBase[0x04004] = (char)0x00;
						/* JP &BD16 - MC START PROGRAM */
						Z80MemoryBase[0x04005] = (char)0xc3;
						Z80MemoryBase[0x04006] = (char)0x16;
						Z80MemoryBase[0x04007] = (char)0xbd;
						/* start executing code */
						CPU_SetReg(CPU_PC,0x04000);

						AutoType.nFlags |= AUTOTYPE_ACTIVE_2;

					}
				}
				else
				{
					/* auto-type is now active */
					AutoType.nFlags |= AUTOTYPE_ACTIVE;
					/* no longer waiting */
					AutoType.nFlags &=~AUTOTYPE_WAITING;
				}
			}
		}
	}
	else
	{
		/* auto-type is active */

		/* delay frames? */
		if (AutoType.nFrames!=0)
		{
			AutoType.nFrames--;
			return;
		}

		/* NOTES:
			- if SHIFT or CONTROL is pressed, then they must be released
			for at least one whole frame for the CPC operating system to recognise them
			as released.

			- When the same key is pressed in sequence (e.g. press, release, press, release)
			then there must be at least two frames for the key to be recognised as released.
			The CPC operating system is trying to 'debounce' the key
		*/
		/* release? */
		if (AutoType.nFlags & AUTOTYPE_RELEASE)
		{
			AutoType_ReleaseKey();

			/* release it now */
			AutoType.nFlags &=~AUTOTYPE_RELEASE;

			/* number of frames for release to be acknowledged */
			AutoType.nFrames = 1;
		}
		else
		{
			if (AutoType_IsFinished())
			{
				/* auto type is no longer active */
				AutoType_Finish();
			}
			else
			{
				AutoType.m_UTF16Ch = AutoType_GetNextChar();

				/* number of frames for key to be acknowledged */
				AutoType.nFrames=1;

				if (!AutoType_CharToCPC(AutoType.m_UTF16Ch))
				{
					AutoType_Finish();
					/* error, char not found */
					//printf("Autotype error; char %d is not mapped!", AutoType.m_UTF16Ch);
				}
				else
				{
					AutoType.nFlags |= AUTOTYPE_RELEASE;
				}
			}
		}
	}
}

