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
 /* Multiface 2 emulation */

 // TODO: 0065 is read to hide mf2? xx000000 011001x (64-67)
 // TOOLS, WINDOW -> crash on arnold
// on cpc6128 with type 2, out &fee8,&e8 from basic appears to cause hang, but not on arnold

#include "multface.h"
#include "cpcglob.h"
#include "headers.h"
#include "garray.h"
#include "cpc.h"
#include "emudevice.h"
#include "riff.h"

/* flags */
static unsigned long Multiface_Flags = MULTIFACE_FLAGS_VISIBLE;

/* multiface occupies memory &0000-&4000. &0000-&1fff is ROM, 0x02000-0x03fff is RAM */
/* lower 8k is multiface rom */
static unsigned char MultifaceRom[8192];
static unsigned char MultifaceRam[8192];
static unsigned char MultifaceGAPen = 0;
static unsigned char MultifaceCRTCRegister = 0;

/* when a write to the I/O is performed, this is called. The I/O writes are therefore
trapped by the Multiface 2 */
void	Multiface_WriteIO(unsigned short Port, unsigned char Data)
{
	/* PAL decoding --111110 111010-- */
	if (
		((Port & 0x0fffc) == 0x0fee8)
		)
	{
		/* enable and visible */
		if ((((~Port) & 0x02) >> 1) && ((Multiface_Flags & MULTIFACE_FLAGS_VISIBLE) != 0))
		{
			Multiface_Flags |= MULTIFACE_FLAGS_ACTIVE;
		}
		else
		{
			/* disable or disabled and visible */
			Multiface_Flags &= ~MULTIFACE_FLAGS_ACTIVE;
		}
		Computer_RethinkMemory();
	}

	{
		unsigned char PortHighByte = (unsigned char)(Port>>8);
		
		/* Gate Array Write */
		if (PortHighByte == (unsigned char)0x07f)
		{
			if ((Data & 0x0c0) == 0x0)
			{
				MultifaceGAPen = Data;
				MultifaceRam[0x01fcf] = Data;
			}
			else
			{
				/* write gate array data - pen select, colour data, rom/mode, ram config*/
				if ((Data & 0x0c0) == 0x040)
				{
					/* get selected pen */
					int PenIndex = MultifaceGAPen;
					if (PenIndex & 0x010) /* IC6 */
					{
						/* border */
						MultifaceRam[0x01fdf] = Data;
					}
					else /* IC7 */
					{
						/* pen */
						MultifaceRam[0x01f90 | (PenIndex & 0x0f)] = Data;
					}
				}
				else
				{

					MultifaceRam[0x01fcf | ((Data & 0x0c0) >> 2)] = Data;
				}
			}
		}
		else
		/* CRTC write */
		if (PortHighByte==(unsigned char)0x0bc)
		{
			MultifaceCRTCRegister = Data;

			/* store reg index */
			MultifaceRam[0x01cff] = Data;
		}
		else
		if (PortHighByte == (unsigned char)0x0bd)
		{
			int CRTCRegIndex;

			/* get reg index */
			CRTCRegIndex = MultifaceCRTCRegister;
			if (CRTCRegIndex < 0x010)
			{
				/* write reg data */
				MultifaceRam[(0x01db0 + (CRTCRegIndex & 0x0f))] = Data;
			}
		}
		else
		if (PortHighByte==(unsigned char)0x0f7)
		{
			MultifaceRam[0x017ff] = Data;
		}
	}
}


/* call here when Multiface stop button is pressed */
void    Multiface_Stop(void)
{
	/* stop button pressed already? */
	if (Multiface_Flags & MULTIFACE_STOP_BUTTON_PRESSED)
	{
		return;
	}

	/* no, but it is pressed now, and enable ram too */
	Multiface_Flags |= MULTIFACE_STOP_BUTTON_PRESSED | MULTIFACE_FLAGS_ACTIVE|MULTIFACE_FLAGS_VISIBLE;

/* not going to work.... */
    CPU_SetNMIState(FALSE);
    CPU_SetNMIState(TRUE);
    CPU_SetNMIState(FALSE);

	Computer_RethinkMemory();
}

void Multiface_SetMemPointers(MemoryData *pData)
{
	/* is multiface ram/rom enabled? */
	if ((Multiface_Flags & MULTIFACE_FLAGS_ACTIVE)!=0)
	{
		/* doesn't actually use ROMEN */
		/* writing to multiface ram doesn't write through to normal ram */
		if (pData->bRomEnable[1])
		{
			/* put in multiface rom/ram pointer in memory space */
			pData->pReadPtr[1] = pData->pWritePtr[1] = MultifaceRam - 8192;
			pData->bRamDisable[1] = TRUE;
			pData->bRomDisable[1] = TRUE;
		}
		/* writing to multiface rom writes through to normal ram */
		if (pData->bRomEnable[0])
		{
			pData->pReadPtr[0] = MultifaceRom;
			pData->pWritePtr[0] = GetDummyWriteRam();
			pData->bRamDisable[0] = TRUE;
			pData->bRomDisable[0] = TRUE;
		}
	}
}

void	Multiface_Initialise(void)
{
	memset(MultifaceRam, 0x0ff, 8192);
	memset(MultifaceRom, 0x0ff, 8192);
	Multiface_Flags = 0;
}


/* TODO: Do not return data, we want to return what is there, we are only interested in 
watching the cpu outputs */
BOOL Multiface2_MemoryReadHandler(Z80_WORD Addr, Z80_BYTE *pDeviceData)
{
	/* Test shows this doesn't work! */
	/* multiface is no longer visible */
	if (
		((Addr&(1<<1))==0) && 
		((Multiface_Flags & MULTIFACE_FLAGS_ACTIVE)!=0)
	)
	{
		Multiface_Flags &= ~MULTIFACE_FLAGS_VISIBLE;
	}
	return FALSE;
}

static CPCPortRead Multiface2MemoryRead[1] =
{
	{
		0xfffe, /* and */
		0x0064, /* cmp */
		Multiface2_MemoryReadHandler
	}
};

CPCPortWrite multifacePortWrite[1] =
{
	{
		0x0000,
		0x0000,
		Multiface_WriteIO
	}
};

void Multiface_SetRom(const unsigned char *pRom, unsigned long RomLength)
{
	EmuDevice_CopyRomData(MultifaceRom, 8192, pRom, RomLength);
}

void Multiface_ClearRom(void)
{
	EmuDevice_ClearRomData(MultifaceRom, 8192);
}

/* As with a real Multiface, when the CPC is reset, the multiface
is enabled. If the user has set the Multiface emulation to be active,
then the multiface will be enabled when it is reset, otherwise it will
not */

void    Multiface_Reset(void)
{
	/* stop button not pressed */
	Multiface_Flags = MULTIFACE_FLAGS_VISIBLE;
	Computer_RethinkMemory();
}

static EmuDeviceRom MultifaceRoms[1]=
{
 {
    "Multiface System ROM",
	"SystemRom",
    Multiface_SetRom,
	Multiface_ClearRom,
   8192,
     0   /* ROM CRC - todo */

  },
};

static EmuDeviceButton MultifaceButtons[2]=
{
  {
      "Reset Button",
      NULL
  },
  {
    "Stop Button",
    Multiface_Stop
  }
};



MemoryRange Multiface2_MemoryRanges[2]=
{
	{"Multiface RAM", RIFF_FOURCC_CODE('M','F','C','E'), FALSE, FALSE, NULL,8192},
	{"Multiface ROM", RIFF_FOURCC_CODE('M','F','C','R'), FALSE, FALSE, NULL,8192},
};


static EmuDevice Multiface2Device =
{
	NULL,
	Multiface_Initialise,
	NULL,
	"MULTIFACE2",
	"Multiface2",
	"Multiface 2",
	CONNECTION_EXPANSION,   /* connected to expansion */
	DEVICE_FLAGS_HAS_PASSTHROUGH| DEVICE_FLAGS_TESTED,
   0,                /* no read ports */
  NULL,
  sizeof(multifacePortWrite)/sizeof(multifacePortWrite[0]),                    /* 1 write ports*/
  multifacePortWrite, 
  1,                /* memory read*/ 
  Multiface2MemoryRead,
  0,                /* no memory write */
  NULL,
  Multiface_Reset, /* reset function */
  Multiface_SetMemPointers, /* memory rethink */
  Multiface_Reset, /* power function */
	0,                      /* no switches */
    NULL,
    sizeof(MultifaceButtons)/sizeof(MultifaceButtons[0]),                      /* 2 buttons, one reset, other is stop*/
    MultifaceButtons,
    sizeof(MultifaceRoms)/sizeof(MultifaceRoms[0]),                      /* onboard roms */
    MultifaceRoms,
    NULL,                      /* no cursor update */
    NULL ,                      /* no generic roms */
	NULL,						/* printer */
	NULL,						/* joystick */
	sizeof(Multiface2_MemoryRanges)/sizeof(Multiface2_MemoryRanges[0]),
	Multiface2_MemoryRanges, 
	NULL, /* sound */
	NULL, /* reti */
	NULL, /* ack maskable interrupt */
	NULL, /* dkram data */
	NULL, /* device ram */
	NULL, /* device backup */
	NULL,
};

void Multiface2_Init(void)
{
	RegisterDevice(&Multiface2Device);
}

