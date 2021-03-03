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
#include "gendiss.h"

static char HexPrefix = '#';

char Diss_GetHexPrefix()
{
    return HexPrefix;
}

void Diss_SetHexPrefix(char ch)
{
    HexPrefix = ch;
}

char *Diss_WriteAscii(char *pString, unsigned char byte, BOOL bSevenBitASCII)
{
  /* useful if bit 7 is set to indicate end of string */
  if (bSevenBitASCII)
  {
    byte = byte & 0x07f;
  }
  
     /* convert un-printable chars into '.' */
    if ((byte<32) || (byte>=127))
        byte = '.';

    pString[0] = byte;
    ++pString;

    return pString;
}

char Diss_GetHexDigitAsChar(unsigned char HexDigit)
{
    HexDigit &=0x0f;
    if (HexDigit<=9)
        return '0'+HexDigit;
    return 'A'+(HexDigit-10);
}


char *Diss_WriteHexByte(char *pString, unsigned char Value, BOOL bPrefix, BOOL bSigned)
{
	unsigned char HexDigit;


    if (bSigned)
    {
        char ch;

        /* write signed offset */
        /* negative value? */
        if ((Value&0x080)!=0)
        {
            ch = '-';
            Value=Value^0x0ff;
            Value=Value+1;
        }
        else
        {
            ch = '+';
        }
        pString[0] = ch;
        ++pString;

    }

	if (bPrefix)
	{
		/* write identifier to indicate hex value */
		*pString = HexPrefix;
		++pString;
	}

	/* write high digit */
	HexDigit = Value>>4;

	*pString = Diss_GetHexDigitAsChar(HexDigit);
	++pString;
	HexDigit = Value & 0x0f;

	*pString = Diss_GetHexDigitAsChar(HexDigit);
	++pString;
	return pString;
}

char *Diss_WriteDecByte(char *pString, unsigned char Value, BOOL bSigned)
{
    int nDigit;
    if (bSigned)
    {
        char ch;

        if ((Value & 0x080)!=0)
        {
            ch = '-';
            Value = Value^0x0ff;
            Value = Value+1;
        }
        else
        {
            ch = '+';
        }
        *pString = ch;
        ++pString;
    }
    nDigit = Value/100;
    *pString = nDigit+'0';
    ++pString;

    nDigit = Value-(nDigit*100);
    nDigit = Value/10;
    *pString = nDigit+'0';
    ++pString;

    nDigit = Value-(nDigit*10);
    *pString = nDigit+'0';
    ++pString;
    return pString;
}


char *Diss_WriteDecWord(char *pString, unsigned short Value, BOOL bSigned)
{
    int nDigit;
    if (bSigned)
    {
        char ch;

        if ((Value & 0x08000)!=0)
        {
            ch = '-';
            Value = Value^0x0ffff;
            Value = Value+1;
        }
        else
        {
            ch = '+';
        }
        *pString = ch;
        ++pString;
    }
    nDigit = Value/10000;
    *pString = nDigit+'0';
    ++pString;

    nDigit = Value-(nDigit*10000);
    nDigit = Value/1000;
    *pString = nDigit+'0';
    ++pString;

    nDigit = Value-(nDigit*1000);
    nDigit = Value/100;
    *pString = nDigit+'0';
    ++pString;

    nDigit = Value-(nDigit*100);
    nDigit = Value/10;
    *pString = nDigit+'0';
    ++pString;

    nDigit = Value-(nDigit*10);
    *pString = nDigit+'0';
    ++pString;
    return pString;
}



char *Diss_WriteHexWord(char *pString, unsigned short Value, BOOL bPrefix, BOOL bSigned)
{
	unsigned char HexDigit;
    if (bSigned)
    {
        char ch;

        if ((Value & 0x08000)!=0)
        {
            ch = '-';
            Value = Value^0x0ffff;
            Value = Value+1;
        }
        else
        {
            ch = '+';
        }
        *pString = ch;
        ++pString;
    }
	if (bPrefix)
	{
		/* write identifier to indicate hex value */
		*pString = HexPrefix;
		++pString;
	}

	HexDigit = Value>>12;

	*pString = Diss_GetHexDigitAsChar(HexDigit);
	++pString;

	HexDigit = Value>>8;

	*pString = Diss_GetHexDigitAsChar(HexDigit);
	++pString;

	HexDigit = Value>>4;

	*pString = Diss_GetHexDigitAsChar(HexDigit);
	++pString;

	HexDigit = Value;

	*pString = Diss_GetHexDigitAsChar(HexDigit);
	++pString;

    return pString;
}


static char *Diss_char(char *pString, const char ch)
{
        pString[0] = ch;
        ++pString;
        return pString;
}

char *Diss_space(char *pString)
{
	return Diss_char(pString,' ');
}

char *Diss_comma(char *pString)
{
	return Diss_char(pString,',');
}

char *Diss_colon(char *pString)
{
	return Diss_char(pString,':');
}

char *Diss_endstring(char *pString)
{
    pString[0] = '\0';
    ++pString;
    return pString;
}

char *Diss_strcat(char *pString, const char *pToken)
{
    int nTokenLength = strlen(pToken);
    strncpy(pString, pToken, nTokenLength);
    pString += nTokenLength;
    return pString;
}
