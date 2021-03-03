#include "cpcglob.h"

/* internal disc interface of CPC664 and CPC6128  */
/* or external DDI-1 disc interface */
void Amstrad_DiscInterface_Enable(BOOL bEnable);
void Amstrad_DiscInterface_SetRom(const unsigned char *pDOSRom);
void Amstrad_DiscInterface_Install(void);
void Amstrad_DiscInterface_Uninstall(void);
void Amstrad_DiscInterface_PortWrite(Z80_WORD Port, Z80_BYTE Data);
BOOL Amstrad_DiscInterface_PortRead(Z80_WORD Port, Z80_BYTE *pDeviceData);
void Amstrad_DiscInterface_SetRom( const unsigned char *pRom );
