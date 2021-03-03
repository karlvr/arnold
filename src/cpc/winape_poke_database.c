#include "cpcglob.h"
#include "winape_poke_database.h"

/* read winape cheat database */
 char *m_pWinapePokeDatabase;
char **pGamePtrs;

const char *WinapePokeDatabase_ReadWord(const char *pPtr, unsigned short *pNumber)
{
    unsigned short nNumber;
    nNumber = (*pPtr) & 0x0ff;
    ++pPtr;
    nNumber = nNumber<<8;
    nNumber = ((*pPtr) & 0x0ff)<<8;
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
            if ((nData & (1<<6))!=0)
            {
                bNegative = TRUE;
            }
            /* set bits in number */
            nNumber = nNumber | (nData & ((1<<6)-1));

            /* indicate we've done first byte */
            bFirst = FALSE;
            /* number of bits to shift number if there are additional bytes */
            nShift = 5;
        }
        else
        {
            /* there were additional bytes, shift number */
            nNumber = nNumber<<nShift;
            /* combine bits from this byte, 6 bits of byte define number */
            nNumber = nNumber | (nData & ((1<<7)-1));
            /* next shift */
            nShift = 6;

        }
    }
    /* if bit 7 is set there are additional bytes to read */
    while ((nData & (1<<7))!=0);

    /* negate number now? */
    if (bNegative)
    {
        nNumber = -nNumber;
    }
    *pNumber = nNumber;

    return pPtr;
}

void WinapePokeDatabase_Init(const char *pWinapePokeDatabase)
{
    const char *pPtr = pWinapePokeDatabase;
    m_pWinapePokeDatabase = (char *)pWinapePokeDatabase;

    if (memcmp(pPtr, "WPOK", 4)==0)
    {
        unsigned long nGames;
        unsigned long i;

        pPtr+=4;

        pPtr = WinapePokeDatabase_ReadCompressedNumber(pPtr, &nGames);

        pGamePtrs = (char **)malloc(sizeof(unsigned char *)*nGames);

        for (i=0; i<nGames; i++)
        {
            unsigned long nStringLength;
            unsigned short nIdentifierSize;
            unsigned long nPokes;
            unsigned long p;

            pGamePtrs[i] = (char *)pPtr;

            /* read string length */
            pPtr = WinapePokeDatabase_ReadCompressedNumber(pPtr, &nStringLength);
            /* add on string length */
            pPtr += nStringLength;
            /* add on identifier size */
            pPtr = WinapePokeDatabase_ReadWord(pPtr, &nIdentifierSize);
            if (nIdentifierSize!=0)
            {
                pPtr += sizeof(unsigned short) + nIdentifierSize;
            }

            pPtr = WinapePokeDatabase_ReadCompressedNumber(pPtr, &nPokes);


            for (p=0; p<nPokes; p++)
            {
                unsigned long nEntries;
                unsigned long e;

                /* poke description */

                /* read string length */
                pPtr = WinapePokeDatabase_ReadCompressedNumber(pPtr, &nStringLength);
                /* add on string length */
                pPtr += nStringLength;

                /* poke comment */

                /* read string length */
                pPtr = WinapePokeDatabase_ReadCompressedNumber(pPtr, &nStringLength);
                /* add on string length */
                pPtr += nStringLength;

                /* data type */
                ++pPtr;
                /* reversed */
                ++pPtr;
                /* ram bank */
                ++pPtr;
                pPtr = WinapePokeDatabase_ReadCompressedNumber(pPtr, &nEntries);

                for (e=0; e<nEntries; e++)
                {
                    unsigned short nBytes;
                   /* unsigned short nAddress; */

                    /* number of bytes */
                    pPtr = WinapePokeDatabase_ReadWord(pPtr, &nBytes);

                    /* address */
                    pPtr = WinapePokeDatabase_ReadWord(pPtr, &nBytes);

                    /* bytes to poke */
                    pPtr += (sizeof(unsigned short)*nBytes);
                }


            }
        }
    }
}

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
//	RamBank = RamBank;
//	Reversed = Reversed;
//	DataType = DataType;
	
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

        for (b=0; b<nBytes; b++)
        {

            /* bytes to poke */
            pPtr += (2*nBytes);
        }
    }
}

void WinapePokeDatabase_Free(void)
{
    if (m_pWinapePokeDatabase!=NULL)
    {
        free((void *)m_pWinapePokeDatabase);
        m_pWinapePokeDatabase = NULL;
    }
}

