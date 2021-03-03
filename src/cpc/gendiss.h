#ifndef __GEN_DISS_HEADER_INCLUDED__
#define __GEN_DISS_HEADER_INCLUDED__

#include "cpcglob.h"
#include "memrange.h"

/* number base */
enum
{
    NUMBER_BASE_BINARY = 0,     /* not implemented yet */
    NUMBER_BASE_DECIMAL,
    NUMBER_BASE_OCTAL,          /* not implemented yet */
    NUMBER_BASE_HEXADECIMAL
};

/* signed/unsigned display */
enum
{
    NUMBER_UNSIGNED = 0,
    NUMBER_SIGNED
};

/* byte or word */
enum
{
    NUMBER_SIZE_BYTE = 0,
    NUMBER_SIZE_WORD
};

char *Diss_WriteHexByte(char *pString, unsigned char Value, BOOL bPrefix, BOOL bSigned);
char *Diss_WriteDecByte(char *pString, unsigned char Value, BOOL bSigned);
char *Diss_space(char *pString);
char *Diss_comma(char *pString);
char *Diss_colon(char *pString);
char *Diss_endstring(char *pString);
char *Diss_strcat(char *pString, const char *pToken);
char *Diss_WriteHexWord(char *pString, unsigned short Value, BOOL bPrefix, BOOL bSigned);
char *Diss_WriteDecWord(char *pString, unsigned short Value, BOOL bSigned);
const char *GetLabelForAddress(MemoryRange *,int Addr);
char *Diss_WriteAscii(char *pString, unsigned char byte, BOOL bSevenBit);

#endif
