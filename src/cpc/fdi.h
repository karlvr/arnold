#ifndef __FLOPPY_DISC_INTERFACE_HEADER_INCLUDED__
#define __FLOPPY_DISC_INTERFACE_HEADER_INCLUDED__

#include "cpcglob.h"
#include "device.h"
#include "fdd.h"

typedef struct
{
	/* drive and side we are actually accessing */
	FDD *drive;

	/* index of physical drive selected */
	unsigned long PhysicalDrive;
	/* index of physical side selected */
	unsigned long PhysicalSide;

	int Density;
} FDI;

#define CPC_MAX_DRIVES 4

BOOL FDI_Get4Drives(void);
void FDI_Set4Drives(BOOL bTwoDrives);

int FDI_GetDriveToAccess(int nDrive);
int FDI_GetSideToAccess(int nDrive, int nSide);

BOOL FDI_GetSwapDrives(void);
BOOL FDI_GetSwapSides(void);
void FDI_SetSwapDrives(BOOL);
void FDI_SetSwapSides(BOOL);
void FDI_SwapDrives(void);
/* convert from FDC drive output to floppy disc interface output */
void FDI_SetPhysicalDrive(unsigned long);
/* convert from FDC side output to floppy disc interface output */
void FDI_SetPhysicalSide(unsigned long);

int FDI_GetPhysicalDrive(void);
int FDI_GetPhysicalSide(void);

void FDI_SetCurrentFDDLEDState(BOOL fState);
unsigned long FDI_GetCurrentDriveFlags(void);
void FDI_SwapSides(void);

void FDI_SetForceSide1(BOOL);
BOOL FDI_GetForceSide1(void);

BOOL FDI_GetForceDiscRomOff(void);
void FDI_SetForceDiscRomOff(BOOL bState);


BOOL FDI_GetMotorState(void);
void FDI_SetMotorState(int);

unsigned long FDI_GetSelectedDriveFlags(void);

/* translate from fdc drive select outputs to fdi drive select outputs and
perform a step on that drive */
void	FDI_PerformStep(unsigned long DriveIndex, int Direction);
unsigned long	FDI_GetNextID(CHRN *);
void			FDI_ReadSector(char *);
void			FDI_WriteSector(char *, int Mark);
void			FDI_EmptyTrack(void);
void			FDI_AddSector(CHRN *, int N, int Filler, int Gap3);
void			FDI_SetDensity(int);

#endif

