
BOOL MegaROM_GetJ3(void);

void MegaROM_SetJ3(BOOL bState);


BOOL MegaROM_GetJ1(void);

void MegaROM_SetJ1(BOOL bState);

BOOL MegaROM_GetJ2(void);

void MegaROM_SetJ2(BOOL bState);

void	MegaROM_ROMSelect(Z80_WORD Port, Z80_BYTE Data);

void MegaROM_MemoryRethink(MemoryData *pData);

void	MegaROM_Install(void);

void	MegaROM_UnInstall(void);

void MegaROM_Init(void);







