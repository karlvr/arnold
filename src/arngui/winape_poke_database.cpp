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
#include "../cpc/cpcglob.h"
#include "winape_poke_database.h"

/* read winape cheat database */

const char *WinapePokeDatabase_ReadWord(const char *pPtr, unsigned short *pNumber)
{
	unsigned short nNumber;
	nNumber = (*pPtr) & 0x0ff;
	++pPtr;

	/*
	nNumber = nNumber<<8;
	nNumber = ((*pPtr) & 0x0ff)<<8;//missing part
	*/
	nNumber = (((*pPtr) & 0x0ff) << 8) + nNumber;

	++pPtr;

	*pNumber = nNumber;

	return pPtr;
}

const char *WinapePokeDatabase_ReadCompressedNumber(const char *pPtr, unsigned long *pNumber)
{
	BOOL bFirst = TRUE;
	BOOL bNegative = FALSE;
	int nNumber = 0;
	char nData;
	int nShift = 0;

	do
	{
		nData = *pPtr;
		++pPtr;

		if (bFirst)
		{
			/* if first byte, then bit 6 identifies if number should
			be negated after it is generated */
			if ((nData & (1 << 6)) != 0)
			{
				bNegative = TRUE;
			}
			/* set bits in number */
			//nNumber = nNumber | (nData & ((1<<6)-1));
			nNumber = nNumber | (nData & 63);

			/* indicate we've done first byte */
			bFirst = FALSE;
			/* number of bits to shift number if there are additional bytes */
			//nShift = 5; //Error ?
			nShift = 6;
		}
		else
		{
			/* there were additional bytes, shift number */
			//nNumber = nNumber<<nShift;
			/* combine bits from this byte, 6 bits of byte define number */
			//nNumber = nNumber | (nData & ((1<<7)-1));
			nNumber = nNumber | ((nData & 127) << nShift);
			/* next shift */
			//nShift = 6;//error ?
			nShift = 7;

		}
	}
	/* if bit 7 is set there are additional bytes to read */
	while ((nData & (1 << 7)) != 0);

	/* negate number now? */
	if (bNegative)
	{
		nNumber = -nNumber;
	}
	*pNumber = nNumber;

	return pPtr;
}

POKEGAMEINFO::POKEGAMEINFO(void)
{
	PokeInfoByGame = NULL;
}

POKEGAMEINFO::~POKEGAMEINFO(void)
{
	for (int i = 0; i < NbrePoke; i++)
	{
		if (PokeInfoByGame[i].PokeInfo)
		{
			free(PokeInfoByGame[i].PokeInfo);
		}
	}
	if (PokeInfoByGame) free(PokeInfoByGame);
}

WinapePokeDatabase::~WinapePokeDatabase(void)
{
	if (m_pWinapePokeDatabase != NULL)
	{
		free((void *)m_pWinapePokeDatabase);
		m_pWinapePokeDatabase = NULL;
	}
	if (pGamePtrs != NULL)
	{
		free((void *)pGamePtrs);
		pGamePtrs = NULL;
	}
}

WinapePokeDatabase::WinapePokeDatabase(void)
{
	NbreGame = 0;
	m_pWinapePokeDatabase = NULL;
	pGamePtrs = NULL;
}


int WinapePokeDatabase::GetNumberGame()
{
	return NbreGame;
}

bool WinapePokeDatabase::GetNameofGame(int i, wxString *Name)
{
	if (i > NbreGame) return false;

	const char *tmp;
	tmp = pGamePtrs[i];

	unsigned long nStringLength;
	tmp = WinapePokeDatabase_ReadCompressedNumber(tmp, &nStringLength);

	wxString tmp2;

	tmp2 = wxString::FromUTF8(tmp);
	*Name = tmp2.Mid(0, nStringLength);
	return true;
}

bool WinapePokeDatabase::SetGame(int id, POKEGAMEINFO * pgi)
{
	if (id > NbreGame) return false;
	SelGame = id;

	wxString tmp2;

	const char *pPtr;
	pPtr = pGamePtrs[id];


	unsigned long nStringLength;
	unsigned short nIdentifierSize;
	unsigned long nPokes;
	unsigned long p;


	//Name of game - string
	pPtr = WinapePokeDatabase_ReadCompressedNumber(pPtr, &nStringLength);
	//memorise
	tmp2 = wxString::FromUTF8(pPtr);
	pgi->Name = tmp2.Mid(0, nStringLength);
	// add on name length
	pPtr += nStringLength;

	//identifier size
	pPtr = WinapePokeDatabase_ReadWord(pPtr, &nIdentifierSize);
	//add on identifier size (if zero, IDAddr and IDData not present)
	if (nIdentifierSize != 0)
	{
		//Address of Identifier (Word)
		pPtr += sizeof(unsigned short);
		//Identifier Data 
		pPtr += nIdentifierSize;
	}

	//Number of poke
	pPtr = WinapePokeDatabase_ReadCompressedNumber(pPtr, &nPokes);
	//memorise them
	pgi->NbrePoke = nPokes;

	//chngae array size
	pgi->PokeInfoByGame = (POKEINFOBYGAME*)malloc(nPokes * sizeof(POKEINFOBYGAME));


	for (p = 0; p < nPokes; p++)
	{
		unsigned long nEntries;
		unsigned long e;

		// poke description - string
		pPtr = WinapePokeDatabase_ReadCompressedNumber(pPtr, &nStringLength);
		//memorise it
		strncpy(pgi->PokeInfoByGame[p].Desc, pPtr, nStringLength);
		pgi->PokeInfoByGame[p].Desc[nStringLength] = '\0';
		// add on description length
		pPtr += nStringLength;

		// poke comment - string
		pPtr = WinapePokeDatabase_ReadCompressedNumber(pPtr, &nStringLength);
		//memorise it
		strncpy(pgi->PokeInfoByGame[p].Commment, pPtr, nStringLength);
		pgi->PokeInfoByGame[p].Commment[nStringLength] = '\0';
		// add on coment length
		pPtr += nStringLength;

		// data type
		pgi->PokeInfoByGame[p].type = *pPtr;
		++pPtr;

		// reversed
		pgi->PokeInfoByGame[p].reversed = (pPtr[0] == '0') ? true : false;
		++pPtr;

		// ram bank
		pgi->PokeInfoByGame[p].ram_bank = *pPtr;
		++pPtr;

		//Number of pokes entries
		pPtr = WinapePokeDatabase_ReadCompressedNumber(pPtr, &nEntries);
		//memorise it
		pgi->PokeInfoByGame[p].npokes = nEntries;


		//chnage array size
		pgi->PokeInfoByGame[p].PokeInfo = (POKEINFO*)malloc(nEntries * sizeof(POKEINFO));

		for (e = 0; e < nEntries; e++)
		{
			unsigned short nBytes;
			unsigned short nAddress;

			// number of bytes
			pPtr = WinapePokeDatabase_ReadWord(pPtr, &nBytes);
			//memorize
			pgi->PokeInfoByGame[p].PokeInfo[e].NbrBytes = nBytes;

			// address
			pPtr = WinapePokeDatabase_ReadWord(pPtr, &nAddress);
			//memorise
			pgi->PokeInfoByGame[p].PokeInfo[e].address = nAddress;

			// bytes to poke
			//memorize pointer
			pgi->PokeInfoByGame[p].PokeInfo[e].byte = (unsigned short*)pPtr;

			//add on byte lenght
			pPtr += (sizeof(unsigned short)*nBytes);



		}


	}








	return true;
}

bool WinapePokeDatabase::Init(const char *pWinapePokeDatabase, long size)
{
	const char *pPtr = pWinapePokeDatabase;
	m_pWinapePokeDatabase = (char *)pWinapePokeDatabase;

	if (memcmp(pPtr, "WPOK", 4) == 0)
	{
		unsigned long nGames;
		unsigned long i;

		pPtr += 4;

		//number of game
		pPtr = WinapePokeDatabase_ReadCompressedNumber(pPtr, &nGames);

		pGamePtrs = (const char **)malloc(sizeof(unsigned char *)*nGames);

		NbreGame = nGames;

		for (i = 0; i < nGames; i++)
		{

			unsigned long nStringLength;
			unsigned short nIdentifierSize;
			unsigned long nPokes;
			unsigned long p;

			pGamePtrs[i] = pPtr;

			//Name of game - string
			pPtr = WinapePokeDatabase_ReadCompressedNumber(pPtr, &nStringLength);
			// add on name length
			pPtr += nStringLength;

			//identifier size
			pPtr = WinapePokeDatabase_ReadWord(pPtr, &nIdentifierSize);
			//add on identifier size (if zero, IDAddr and IDData not present)
			if (nIdentifierSize != 0)
			{
				//Address of Identifier (Word)
				pPtr += sizeof(unsigned short);
				//Identifier Data 
				pPtr += nIdentifierSize;
			}

			//Number of poke
			pPtr = WinapePokeDatabase_ReadCompressedNumber(pPtr, &nPokes);

			for (p = 0; p < nPokes; p++)
			{
				unsigned long nEntries;
				unsigned long e;

				// poke description - string
				pPtr = WinapePokeDatabase_ReadCompressedNumber(pPtr, &nStringLength);
				/* add on description length */
				pPtr += nStringLength;

				// poke comment - string
				pPtr = WinapePokeDatabase_ReadCompressedNumber(pPtr, &nStringLength);
				/* add on coment length */
				pPtr += nStringLength;

				/* data type */
				++pPtr;
				/* reversed */
				++pPtr;
				/* ram bank */
				++pPtr;

				//Number of pokes entries
				pPtr = WinapePokeDatabase_ReadCompressedNumber(pPtr, &nEntries);

				for (e = 0; e < nEntries; e++)
				{
					unsigned short nBytes;
					unsigned short nAddress;


					/* number of bytes */
					pPtr = WinapePokeDatabase_ReadWord(pPtr, &nBytes);

					/* address */
					pPtr = WinapePokeDatabase_ReadWord(pPtr, &nAddress);

					/* bytes to poke */
					pPtr += (sizeof(unsigned short)*nBytes);
				}


			}
		}
	}
	return true;
}

#if 0
void WinapePokeDatabase_ApplyPoke(int nGame, int nPoke, const char *pPoke)
{
	const char *pPtr = pPoke;
	unsigned char DataType;
	unsigned char Reversed;
	unsigned char RamBank;
	unsigned long nEntries;
	unsigned long e;
	unsigned long nStringLength;

	/* skip poke comment */

	/* read string length */
	pPtr = WinapePokeDatabase_ReadCompressedNumber(pPtr, &nStringLength);
	/* add on string length */
	pPtr += nStringLength;

	/* data type */
	DataType= *pPtr;
	++pPtr;
	/* reversed */
	Reversed = *pPtr;
	++pPtr;
	/* ram bank */
	RamBank = *pPtr;
	++pPtr;
	RamBank = RamBank;
	Reversed = Reversed;
	DataType = DataType;

	/* read entry count */
	pPtr = WinapePokeDatabase_ReadCompressedNumber(pPtr, &nEntries);

	for (e=0; e<nEntries; e++)
	{
		unsigned long b;
		unsigned short nBytes;
		unsigned short nAddress;

		/* number of bytes */
		pPtr = WinapePokeDatabase_ReadWord(pPtr, &nBytes);

		/* address */
		pPtr = WinapePokeDatabase_ReadWord(pPtr, &nAddress);

		for (b = 0; b < nBytes; b++)
		{

			/* bytes to poke */
			pPtr += (2 * nBytes);
		}
				}
			}

#endif