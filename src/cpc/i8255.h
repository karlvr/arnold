#ifndef __PPI8255_HEADER_INCLUDED__
#define __PPI8255_HEADER_INCLUDED__

/* PPI write by CPU */
void	PPI_WritePort(int, int);
void	PPI_WriteControl(int);
/* PPI read by CPU */
unsigned int		PPI_ReadPort(int nIndex);
int PPI_ReadControl(void);

void PPI_Reset(void);

/* PPI functions for snapshot */
int PPI_GetControlForSnapshot(void);
void	PPI_SetPortDataFromSnapshot(int nPort, int Data);
int PPI_GetPortDataForSnapshot(int nPort);

extern int PPI_GetPortInput(int nIndex);

/* get final outputs from port */
int PPI_GetOutputPort(int nIndex);
int PPI_GetOutputMaskPort(int nIndex);

/* when port data is written this function is called */
extern void PPI_SetPortOutput(int, int);


#endif
