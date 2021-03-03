#ifndef __RAM_PAL_HEADER_INCLUDED__
#define __RAM_PAL_HEADER_INCLUDED__

int PAL16L8_Init(void);
void	Yarek4MB_Init(void);

typedef struct 
{
	unsigned long RamConfig;
	unsigned long Bank;
	unsigned char *pRamPtr;
} PAL16L8Data;

#endif
