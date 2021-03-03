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
/* THIS EMULATION IS INCOMPLETE. MORE TESTING IS NEEDED ON A REAL DEVICE */

#include "cpc.h"
#include "emudevice.h"

unsigned char *HardDisc = NULL;

/* appears to be based on wd1002 winchester controller */
/* Multi sector operation? */
enum
{
	STATE_RESET,
	STATE_IDLE,
	STATE_SELECTION,
	STATE_COMMAND_FIRST_BYTE,
	STATE_COMMAND_BYTES,
	STATE_DATA,
	STATE_STATUS
};


#define CMD_TEST_READY 0x00
#define CMD_REQUEST_SENSE 0x03
#define CMD_READ 0x08
#define CMD_WRITE 0x0a
#define CMD_SEEK 0x0b
#define CMD_SET_PARAMETERS 0x0c

/* Bits for command status byte */
#define CSB_ERROR       0x02
#define CSB_LUN 		0x20

/* status bits */
#define STA_REQUEST	0x01
#define STA_INPUT		0x02
#define STA_COMMAND 	0x04
#define STA_BUSY	0x08
#define STA_DMA_REQUEST 0x10
#define STA_INTERRUPT	0x20

/* XT hard disk controller control bits */
#define CTL_PIO 		0x00
#define CTL_DMA 		0x01
/*;; fbe0		read/write
;; fbe1		status/reset
;; fbe2		config/select
;; fbe3		dma/int
;; fbe4
*/
 
typedef struct  
{
	int State;


	int nBytesRemaining;
	int CommandBuffer[ 16 ];
	int NumCylinders;
	int NumHeads;
	int CurrentSector;
	int CurrentCylinder;
	int CurrentHead;
	int BlockCount;
	/* 256, 512 or 1024 bytes */
	int BlockSize;
	/* 32,18,17 or 9 */
	int SectorsPerTrack;

	unsigned char Buffer[ 1024 ];
	int nBufferPos;

	int Result;
	int Status;
	int Configuration;
} DobbertinHardDiskController ;

static DobbertinHardDiskController controller;

void enter_command_state(void)
{
	controller.State = STATE_COMMAND_FIRST_BYTE;
	controller.Status |= STA_COMMAND | STA_REQUEST;
}


void enter_idle_state(void)
{
	/* enter idle state */
	controller.State = STATE_IDLE;
}


void enter_data_state(int nBytes, int nInput)
{
	/* position in buffer */
	controller.nBufferPos = 0;
	/* number of bytes */
	controller.nBytesRemaining = nBytes;
	/* data state */
	controller.State = STATE_DATA;
	if (nInput)
	{
		/* read */
		controller.Status |= STA_INPUT;
	}
	controller.Status &= ~STA_COMMAND;
	/* request data */
	controller.Status |= STA_REQUEST;
}

unsigned long get_sector_offset(void)
{
	unsigned long SectorOffset = ( controller.CurrentCylinder*controller.SectorsPerTrack*controller.NumHeads ) + ( controller.SectorsPerTrack * controller.CurrentHead ) + controller.CurrentSector;
	return SectorOffset;
}


void enter_result_state(void)
{
	controller.Status = STA_REQUEST | STA_BUSY | STA_INPUT | STA_COMMAND;
	controller.State = STATE_STATUS;
}

void transfer_sector_start(int read)
{
	if (read)
	{
		unsigned long SectorOffset = get_sector_offset();
		unsigned long Offset = SectorOffset*controller.BlockSize;
		memcpy( controller.Buffer, &HardDisc[ Offset ], controller.BlockSize );
	}

	enter_data_state( controller.BlockSize, read );
	controller.nBytesRemaining = controller.BlockSize;
}

void transfer_sector(int read)
{

	if (!read)
	{
		unsigned long SectorOffset = get_sector_offset();
		unsigned long Offset = SectorOffset*controller.BlockSize;
		memcpy( &HardDisc[ Offset ], controller.Buffer, controller.BlockSize );
	}

	controller.BlockCount--;
	if ( controller.BlockCount != 0 )
	{
		controller.CurrentSector++;
		if ( controller.CurrentSector == controller.SectorsPerTrack )
		{
			controller.CurrentSector = 0;
			controller.CurrentHead++;
			if ( controller.CurrentHead == controller.NumHeads )
			{
				controller.CurrentHead = 0;
				controller.CurrentCylinder++;
			}
		}

		transfer_sector_start(read);
	}
	else
	{
		enter_result_state();

	}
}

void enter_reset_state(void)
{
	controller.State = STATE_RESET;
	controller.Status = 0;

	enter_idle_state();
}


BOOL dobbertin_hd20_read(Z80_WORD Addr, Z80_BYTE *pDeviceData)
{
	switch (Addr & 0x07)
    {
		/* data read */
		case 0:
		{
			Z80_BYTE Data=0x0ff;
			if ( controller.State == STATE_STATUS )
			{
				Data = controller.Result;

				controller.Status &= ~( STA_COMMAND | STA_INPUT | STA_BUSY | STA_REQUEST );
				enter_idle_state();
			}
			else if ( controller.State == STATE_DATA )
			{
				Data = controller.Buffer[ controller.nBufferPos ];
				controller.nBufferPos++;
				controller.nBytesRemaining--;
				if ( controller.nBytesRemaining == 0 )
				{
					if ( controller.CommandBuffer[ 0 ] == CMD_REQUEST_SENSE )
					{
						enter_result_state();
					}
					else
						if ( controller.CommandBuffer[ 0 ] == CMD_READ )
					{
						transfer_sector(1);
					}
					else
					{
						enter_result_state();
					}
				}
			}
			*pDeviceData = Data;
		}
		return TRUE;

		/* status */
        case 1:
			*pDeviceData = controller.Status;
			return TRUE;

		/* configuration */
        case 2:
			*pDeviceData = controller.Configuration;
			return TRUE;
		case 4:
		{			
			enter_reset_state();
		}
		break;
    }

    return FALSE;
}


void dobbertin_hd20_write(Z80_WORD Addr, Z80_BYTE Data)
{
	switch (Addr & 0x07)
    {
		/* data write */
	case 0:
	{
		if ( controller.State == STATE_DATA )
		{
			controller.Status &= ~STA_REQUEST;
			controller.Buffer[ controller.nBufferPos ] = Data;
			controller.nBufferPos++;
			controller.nBytesRemaining--;
			if ( controller.nBytesRemaining != 0 )
			{
				controller.Status |= STA_REQUEST;

			}
			else
			{
				if ( controller.CommandBuffer[ 0 ] == CMD_SET_PARAMETERS )
				{
					controller.NumCylinders = ( ( controller.Buffer[ 0 ] & 0x0ff ) << 8 ) | ( controller.Buffer[ 1 ] & 0x0ff );
					controller.NumHeads = controller.Buffer[ 2 ] & 0x0ff;
				//	printf("Num cylinders: %d\n", NumCylinders);
				//	printf("Num heads: %d\n", NumHeads);
				
				}

				if ( controller.CommandBuffer[ 0 ] == CMD_WRITE )
				{
					transfer_sector(0);
				}
				else
				{
					enter_result_state();
				}
			}
		}
		else
			if ( controller.State == STATE_COMMAND_BYTES )
		{
			controller.Status &= ~STA_REQUEST;

			controller.CommandBuffer[ controller.nBufferPos ] = Data;
			controller.nBufferPos++;
			controller.nBytesRemaining--;

			if ( controller.nBytesRemaining != 0 )
			{
				controller.Status |= STA_REQUEST;
			}
			else
			{
				controller.Status &= ~STA_COMMAND;
				if ( controller.CommandBuffer[ 0 ] == CMD_SET_PARAMETERS )
				{
					/* host write 8 bytes */
					enter_data_state(8, 0);
				}
				else if ( controller.CommandBuffer[ 0 ] == CMD_TEST_READY )
				{
					enter_result_state();
				}
				else if ( controller.CommandBuffer[ 0 ] == CMD_REQUEST_SENSE )
				{
					enter_data_state(4,1);
				}
				else if ( controller.CommandBuffer[ 0 ] == CMD_SEEK )
				{
					enter_result_state();
				}
				else if ( controller.CommandBuffer[ 0 ] == CMD_READ )
				{
					controller.CurrentSector = controller.CommandBuffer[ 2 ] & 0x03f;
					controller.CurrentCylinder = ( controller.CommandBuffer[ 3 ] & 0x0ff ) | ( ( controller.CommandBuffer[ 2 ] << 2 ) & 0x0300 );
					controller.CurrentHead = controller.CommandBuffer[ 1 ] & 0x01f;
					controller.BlockCount = controller.CommandBuffer[ 4 ] & 0x0ff;
					if ( controller.BlockCount == 0 )
					{
						controller.BlockCount = 256;
					}
				//	printf("Sector: %d\n", CurrentSector);
				//	printf("Current Cylinder: %d\n", CurrentCylinder);
				//	printf("Current Head: %d\n", CurrentHead);
				//	printf("Num blocks: %d\n", BlockCount);
					transfer_sector_start(1);
				}
				else if ( controller.CommandBuffer[ 0 ] == CMD_WRITE )
				{
					controller.CurrentSector = controller.CommandBuffer[ 2 ] & 0x03f;
					controller.CurrentCylinder = ( controller.CommandBuffer[ 3 ] & 0x0ff ) | ( ( controller.CommandBuffer[ 2 ] << 2 ) & 0x0300 );
					controller.CurrentHead = controller.CommandBuffer[ 1 ] & 0x01f;
					controller.BlockCount = controller.CommandBuffer[ 4 ] & 0x0ff;
					if ( controller.BlockCount == 0 )
					{
						controller.BlockCount = 256;
					}
				//	printf("Sector: %d\n", CurrentSector);
				//	printf("Current Cylinder: %d\n", CurrentCylinder);
				//	printf("Current Head: %d\n", CurrentHead);
				//	printf("Num blocks: %d\n", BlockCount);
					transfer_sector_start(0);
				}
			}
		}
		else
			if ( controller.State == STATE_COMMAND_FIRST_BYTE )
		{
			controller.Status &= ~STA_REQUEST;

			/* first command byte */
			controller.CommandBuffer[ 0 ] = Data;
			controller.nBufferPos = 1;
			switch (Data)
			{
			case CMD_SET_PARAMETERS:
			{
				controller.nBytesRemaining = 6 - 1;
				controller.State = STATE_COMMAND_BYTES;
				controller.Status |= STA_REQUEST;
			}
			break;

			case CMD_TEST_READY:
			{
				controller.nBytesRemaining = 6 - 1;
				controller.State = STATE_COMMAND_BYTES;
				controller.Status |= STA_REQUEST;
			}
			break;

			case CMD_READ:
			{
				controller.nBytesRemaining = 6 - 1;
				controller.State = STATE_COMMAND_BYTES;
				controller.Status |= STA_REQUEST;
			}
			break;
			case CMD_WRITE:
			{
				controller.nBytesRemaining = 6 - 1;
				controller.State = STATE_COMMAND_BYTES;
				controller.Status |= STA_REQUEST;
			}
			break;
			case CMD_SEEK:
			{
				controller.nBytesRemaining = 6 - 1;
				controller.State = STATE_COMMAND_BYTES;
				controller.Status |= STA_REQUEST;
			}
			break;


			default:
				break;
			}
		}
	}
	break;
		/* reset */
		case 4:
        case 1:
        {
			/* reset */
			enter_reset_state();
        }
        break;

		/* select */
        case 2:
        {
			/* only accepted in idle state */
			if ( controller.State != STATE_IDLE )
				break;

			/* set busy and enter command state */
			controller.Status |= STA_BUSY;

			/* enter command state */
			enter_command_state();
		}
        break;

		default:
			break;
    }




}

static CPCPortRead DobbertinRead[1]=
{
	/* TODO Actual decoding */
	{
		0x0ffe0,
		0x0fbe0,
		dobbertin_hd20_read
	}
};


static CPCPortWrite DobbertinWrite[1]=
{
	/* TODO Actual decoding */
	{
		0x0ffe0,
		0x0fbe0,
		dobbertin_hd20_write
	}
};

void DobbertinHD20Device_Init(void)
{
	HardDisc = (unsigned char *)malloc(20 * 1024 * 1024);
	memset(HardDisc, 0xe5, 20 * 1024 * 1024);
}
void DobbertinHD20Device_Exit(void)
{
	free(HardDisc);
}
static EmuDevice DobbertinHD20Device =
{
	NULL,
	DobbertinHD20Device_Init,
	DobbertinHD20Device_Exit,
	"DOBHD20",
	"DobbertinHD20",
	"Dobbertin HD20 Hard disk",
	CONNECTION_EXPANSION,   /* connected to expansion */
	0,
	sizeof(DobbertinRead)/sizeof(DobbertinRead[0]),                /* 1 read port */
	DobbertinRead,
	sizeof(DobbertinWrite) / sizeof(DobbertinWrite[0]),                /* 1 read port */
	DobbertinWrite,
	0,                /* no memory read*/
	NULL,
	0,                /* no memory write */
	NULL,
	NULL, /* reset function */
	NULL, /* memory rethink */
	NULL, /* power function*/
	0,      /* no switches*/
	NULL,
	0,                      /* no buttons */
	NULL,
	0,                      /* no onboard roms */
	NULL,
	NULL,                   /* no cursor function */
	NULL,                 /* no generic roms */
	NULL,
	NULL,
	0,
	NULL,
	NULL, /* sound */
	NULL, /* lpen */
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	NULL, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
	NULL,
};

void DobbertinHD20_Init(void)
{
	memset( &controller, 0, sizeof( DobbertinHardDiskController ) );
	controller.State = STATE_RESET;
	/* 256, 512 or 1024 bytes */
	controller.BlockSize = 512;
	/* 32,18,17 or 9 */
	controller.SectorsPerTrack = 17;
	controller.Configuration = 1;

	RegisterDevice(&DobbertinHD20Device);
}

