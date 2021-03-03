#ifndef __WINAPE_POKE_DATABASE__
#define __WINAPE_POKE_DATABASE__

typedef enum
{
    WINAPE_POKE_DATABASE_TYPE_DECIMAL = 0,
    WINAPE_POKE_DATABASE_TYPE_HEXADECIMAL,
    WINAPE_POKE_DATABASE_TYPE_BCD,
    WINAPE_POKE_DATABASE_TYPE_NUMERIC,          /* Zero based */
    WINAPE_POKE_DATABASE_TYPE_NUMERIC_ASCII,    /* ASCII based */
    WINAPE_POKE_DATABASE_TYPE_LONG,
    WINAPE_POKE_DATABASE_TYPE_STRING
} WINAPE_POKE_DATABASE_TYPE;

/* Zero based means each digit is stored as a byte (00..09), ASCII means they are stored as the ASCII
characters (30 ('0')..39 ('9')) */

void WinapePokeDatabase_Init(const char *pWinapePokeDatabase);

void WinapePokeDatabase_ApplyPoke(int nGame, int nPoke, const char *pPoke);

void WinapePokeDatabase_Free(void);

#endif
