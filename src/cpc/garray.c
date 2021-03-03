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
#include "garray.h"
#include "render.h"
#include "multface.h"
#include "crtc.h"
#include "cpc.h"
#include "headers.h"
#include "memory.h"
#include "pal.h"
#include <math.h>
#include "monitor.h"

unsigned short PixelBytes;

GATE_ARRAY_STATE	GateArray_State;
/* actual colours to display */
RGBCOLOUR       DisplayColours[32];

static int        Luminances[32];

extern GATE_ARRAY_STATE GateArray_State;


#if 0
// these measured by Markus I think.
// I need to get close to these using emulation
/* base hardware colours */
static const RGBCOLOUR HardwareColours[32] =
{
	{{{0x06e,0x07d,0x06b,0}}},                     /* r1 g1 b1     White */
	{{{0x06e,0x07d,0x06b,0}}},                     /* -------- ** White ** */
	{{{0x00,0x0f3,0x06b,0}}},                    /* r0 g2 b1 Sea Green */
	{{{0x0f3,0x0f3,0x06d,0}}},                   /* r2 g2 b1 Pastel yellow */
	{{{0x00,0x02,0x06b,0}}},                     /* r0 g0 b1 Blue */
	{{{0x0f0,0x02,0x068,0}}},                    /* r2 g0 b1 Purple */
	{{{0x00,0x078,0x068,0}}},                     /* r0 g1 b1 Cyan */
	{{{0x0f3,0x07d,0x06b,0}}},                    /* r2 g1 b1 Pink */
	{{{0x0f0,0x02,0x068,0}}},                    /* -------- ** Purple ** */
	{{{0x0f3,0x0f3,0x06d,0}}},                   /* -------- ** Pastel yellow ** */
	{{{0x0f3,0x0f3,0x0d,0}}},                   /* r2 g2 b0 Bright Yellow */
	{{{0x0ff,0x0f3,0x0f9,0}}},                  /* r2 g2 b2 Bright White */
	{{{0x0f3,0x05,0x06,0}}},                    /* r2 g0 b0 Bright Red */
	{{{0x0f3,0x02,0x0f4,0}}},                   /* r2 g0 b2 Bright Magenta */
	{{{0x0f3,0x07d,0x0d,0}}},                    /* r2 g1 b0 Orange */
	{{{0x0fa,0x080,0x0f9,0}}},                   /* r2 g1 b2 Pastel Magenta */
	{{{0x00,0x02,0x06b,0}}},                     /* -------- ** Blue ** */
	{{{0x00,0x0f3,0x06b,0}}},                    /* -------- ** Sea Green ** */
	{{{0x02,0x0f0,0x01,0}}},                    /* r0 g2 b0 Bright green */
	{{{0x0f,0x0f3,0x0f2,0}}},                   /* r0 g2 b2 Bright Cyan */
	{{{0x00,0x02,0x01,0}}},                     /* r0 g0 b0 Black */
	{{{0x0c,0x02,0x0f4,0}}},                    /* r0 g0 b2 Bright Blue */
	{{{0x02,0x078,0x01,0}}},                     /* r0 g1 b0 Green */
	{{{0x0c,0x07b,0x0f4,0}}},                    /* r0 g1 b2 Sky Blue */
	{{{0x069,0x02,0x068,0}}},                     /* r1 g0 b1 Magenta */
	{{{0x071,0x0f3,0x06b,0}}},                    /* r1 g2 b1 Pastel green */
	{{{0x071,0x0f5,0x04,0}}},                    /* r1 g2 b0 Lime */
	{{{0x071,0x0f3,0x0f4,0}}},                   /* r1 g2 b2 Pastel cyan */
	{{{0x06c,0x02,0x01,0}}},                     /* r1 g0 b0 Red */
	{{{0x06c,0x02,0x0f2,0}}},                    /* r1 g0 b2 Mauve */
	{{{0x06e,0x07b,0x01,0}}},                     /* r1 g1 b0 Yellow */
	{{{0x06e,0x07b,0x0f6,0}}}                             /* r1 g1 b2 Pastel blue */
};
#endif

/* base hardware colours */
static const RGBCOLOUR HardwareColours[32] =
{
	{{{0x060,0x060,0x060,0}}},                     /* r1 g1 b1     White */
	{{{0x060,0x060,0x060,0}}},                     /* -------- ** White ** */
	{{{0x00,0x0ff,0x060,0}}},                    /* r0 g2 b1 Sea Green */
	{{{0x0ff,0x0ff,0x060,0}}},                   /* r2 g2 b1 Pastel yellow */
	{{{0x00,0x00,0x060,0}}},                     /* r0 g0 b1 Blue */
	{{{0x0ff,0x00,0x060,0}}},                    /* r2 g0 b1 Purple */
	{{{0x00,0x060,0x060,0}}},                     /* r0 g1 b1 Cyan */
	{{{0x0ff,0x060,0x060,0}}},                    /* r2 g1 b1 Pink */
	{{{0x0ff,0x00,0x060,0}}},                    /* -------- ** Purple ** */
	{{{0x0ff,0x0ff,0x060,0}}},                   /* -------- ** Pastel yellow ** */
	{{{0x0ff,0x0ff,0x00,0}}},                   /* r2 g2 b0 Bright Yellow */
	{{{0x0ff,0x0ff,0x0ff,0}}},                  /* r2 g2 b2 Bright White */
	{{{0x0ff,0x00,0x00,0}}},                    /* r2 g0 b0 Bright Red */
	{{{0x0ff,0x00,0x0ff,0}}},                   /* r2 g0 b2 Bright Magenta */
	{{{0x0ff,0x060,0x00,0}}},                    /* r2 g1 b0 Orange */
	{{{0x0ff,0x060,0x0ff,0}}},                   /* r2 g1 b2 Pastel Magenta */
	{{{0x00,0x00,0x060,0}}},                     /* -------- ** Blue ** */
	{{{0x00,0x0ff,0x060,0}}},                    /* -------- ** Sea Green ** */
	{{{0x00,0x0ff,0x00,0}}},                    /* r0 g2 b0 Bright green */
	{{{0x00,0x0ff,0x0ff,0}}},                   /* r0 g2 b2 Bright Cyan */
	{{{0x00,0x00,0x00,0}}},                     /* r0 g0 b0 Black */
	{{{0x00,0x00,0x0ff,0}}},                    /* r0 g0 b2 Bright Blue */
	{{{0x00,0x060,0x00,0}}},                     /* r0 g1 b0 Green */
	{{{0x00,0x060,0x0ff,0}}},                    /* r0 g1 b2 Sky Blue */
	{{{0x060,0x00,0x060,0}}},                     /* r1 g0 b1 Magenta */
	{{{0x060,0x0ff,0x060,0}}},                    /* r1 g2 b1 Pastel green */
	{{{0x060,0x0ff,0x00,0}}},                    /* r1 g2 b0 Lime */
	{{{0x060,0x0ff,0x0ff,0}}},                   /* r1 g2 b2 Pastel cyan */
	{{{0x060,0x00,0x00,0}}},                     /* r1 g0 b0 Red */
	{{{0x060,0x00,0x0ff,0}}},                    /* r1 g0 b2 Mauve */
	{{{0x060,0x060,0x00,0}}},                     /* r1 g1 b0 Yellow */
	{{{0x060,0x060,0x0ff,0}}}                             /* r1 g1 b2 Pastel blue */
};



extern void    Render_RenderBorder_Paletted(void);
extern void    Render_RenderBorder_TrueColour(void);
extern void    Render_GetGraphicsDataCPC_TrueColour(void);
extern void    Render_GetGraphicsDataCPC_Paletted(void);
extern void    CRTC_RenderSync_Paletted(void);
extern void    CRTC_RenderSync_TrueColour(void);
extern void    CRTC_RenderBlack_TrueColour(void);


static unsigned char *pVRAMAddr;
static unsigned int VRAMAddr;
static unsigned short PixelData;
extern unsigned char *Z80MemoryBase;

int CPC_GetVRAMAddr(void)
{
	return VRAMAddr;
}

void CPC_CalcVRAMAddr(void)
{
	int LocalMA = CRTC_GetMAOutput()<<1;
	int Addr = ((LocalMA & 0x06000)<<1) |(LocalMA & 0x07ff);
	/* MA0-MA9, MA12, MA13 used */
	/* RA0-RA7 used */

	/* precache this */
	Addr |= (CRTC_GetRAOutput()&0x07)<<11;

	VRAMAddr = Addr;

	pVRAMAddr = &Z80MemoryBase[VRAMAddr];
}

void CPC_CachePixelData(void)
{
	PixelData = (pVRAMAddr[0]<<8)|(pVRAMAddr[1]);
}

unsigned short CPC_GetPixelData(void)
{
	return PixelData;
}


void CPC_UpdateGraphicsFunction(void)
{
	if (Monitor_DrawSync())
	{
		CRTC_SetRenderFunction2(CRTC_RenderSync_TrueColour);
	}
	else
	if (Computer_GetDrawBlanking() && ((GateArray_State.BlankingOutput & (HBLANK_ACTIVE|VBLANK_ACTIVE))!=0))
	{
		
		CRTC_SetRenderFunction2(CRTC_RenderBlack_TrueColour);
	}
	else
	if (Computer_GetDrawBorder() && ((GateArray_State.BlankingOutput & (DISPTMG_ACTIVE))!=0))
	{
		CRTC_SetRenderFunction2(Render_RenderBorder_TrueColour);
	}
	else
	{
		CRTC_SetRenderFunction2(Render_GetGraphicsDataCPC_TrueColour);
	}
}

BOOL GateArray_GetHBlankActive(void)
{
	return ((GateArray_State.BlankingOutput & HBLANK_ACTIVE)!=0);
}

int GateArray_GetHBlankCount(void)
{
	return GateArray_State.nHBlankCycle;
}


BOOL GateArray_GetVBlankActive(void)
{
	return ((GateArray_State.BlankingOutput & VBLANK_ACTIVE)!=0);
}

int GateArray_GetVBlankCount(void)
{
	return GateArray_State.nVBlankCycle;
}

void GateArray_UpdateHsync(BOOL bState)
{
	if (bState)
	{
		GateArray_State.CRTCSyncInputs|=HSYNC_INPUT;
		GateArray_State.BlankingOutput |= HBLANK_ACTIVE;
	
		
		GateArray_State.nHBlankCycle = 0;
		CPC_UpdateGraphicsFunction();
	}
	else
	{
		GateArray_State.CRTCSyncInputs&=~HSYNC_INPUT;
		GateArray_State.BlankingOutput &= ~HBLANK_ACTIVE;
		
		Monitor_DoHsyncEnd();
		
		/* increment interrupt line count */
		GateArray_State.InterruptLineCount++;

		/* if line == 52 then interrupt should be triggered */
		if (GateArray_State.InterruptLineCount==52)
		{
			/* clear counter. */
			GateArray_State.InterruptLineCount = 0;

			GateArray_State.RasterInterruptRequest = TRUE;

			Computer_RefreshInterrupt();

		}
			
		/* CHECK: Vblank triggered off hsync end or start? */
		if (GateArray_State.BlankingOutput & VBLANK_ACTIVE)
		{
			/* CHECK: VSYNC to monitor switched off at next HSYNC? */
			
			GateArray_State.nVBlankCycle++;
			if (GateArray_State.nVBlankCycle==2) 
			{
				/* has interrupt line counter overflowed? */
				if (GateArray_State.InterruptLineCount>=32)
				{
					/* following might not be required, because it is probably */
					/* set by the code above */

					GateArray_State.RasterInterruptRequest = TRUE;

					Computer_RefreshInterrupt();

				}

				/* reset interrupt line count */
				GateArray_State.InterruptLineCount = 0;

			
				/* TODO: Check */
				if (GateArray_State.BlankingOutput & VSYNC_INPUT)
				{
					Monitor_DoVsyncStart();
				}
			}
			else if (GateArray_State.nVBlankCycle==6)
			{
				Monitor_DoVsyncEnd();
			}
			else if ( GateArray_State.nVBlankCycle == 26 )
			{
				GateArray_State.BlankingOutput &=~VBLANK_ACTIVE;
			}
		}
	
		
		CPC_UpdateGraphicsFunction();
	}
}

void GateArray_UpdateVsync(BOOL bState)
{
	BOOL bCurrentState = ((GateArray_State.CRTCSyncInputs & VSYNC_INPUT) != 0);
	if (bState == bCurrentState)
		return;

	if (bState)
	{
		GateArray_State.CRTCSyncInputs|=VSYNC_INPUT;
		GateArray_State.BlankingOutput |= VBLANK_ACTIVE;
		GateArray_State.nVBlankCycle = 0;
		CPC_UpdateGraphicsFunction();
	}
	else
	{
		GateArray_State.CRTCSyncInputs&=~VSYNC_INPUT;
		/* CHECK, immediate or at hsync */
		Monitor_DoVsyncEnd();
		CPC_UpdateGraphicsFunction();

	}
}

void GateArray_DoDispEnable(BOOL bState)
{
	if (bState)
	{

		GateArray_State.BlankingOutput &= ~DISPTMG_ACTIVE;
		Computer_UpdateGraphicsFunction();
	}
	else
	{
		GateArray_State.BlankingOutput |= DISPTMG_ACTIVE;
		Computer_UpdateGraphicsFunction();
	}
}

/* chances are the crtc hsync could happen in the middle */
void GateArray_Cycle(void)
{
	/* Gate-array outputs black for the duration of the hsync -> Horizontal blanking.
	This black overrides border and graphics. The HSYNC to the monitor is 2us later and lasts for 4us (or shorter if HSYNC is shorter). */

	/* Gate-array outputs black for 26 hsyncs for vblank -> Vertical blanking.
	It is triggered by the start of vsync.
	2 HSYNC later sync to monitor is triggered and lasts a max of 4 hsync */

	if (GateArray_State.BlankingOutput & HBLANK_ACTIVE)
	{
		/* CHECK: Mode changed or not if hsync is too short? */
		switch (GateArray_State.nHBlankCycle)
		{
			case 0:
				break;

			case 1:
			{
				/* set pixel translation table which is dependant on mode */
				Render_SetPixelTranslation(GateArray_State.RomConfiguration & 0x03);

				if (GateArray_State.CRTCSyncInputs & HSYNC_INPUT)
				{
					Monitor_DoHsyncStart();
				}
				
				CPC_UpdateGraphicsFunction();
			}
			break;

			case 5:
			{
				/* end monitor hsync but continue blanking */
				Monitor_DoHsyncEnd();
				
				CPC_UpdateGraphicsFunction();
			}
			break;
		}
		GateArray_State.nHBlankCycle++;
	}
}

/* set monitor colour mode */
void    GateArray_SetMonitorColourMode(CPC_MONITOR_TYPE_ID      MonitorMode)
{
	int i;

	switch (MonitorMode)
	{
		/* the brightness on 464 seems to be more than on 6128!? */
		case CPC_MONITOR_COLOUR:
		{
			memcpy(&DisplayColours, &HardwareColours, sizeof(RGBCOLOUR)*32);

			for (i=0; i<32; i++)
			{
				Monitor_AdjustRGBForDisplay(&DisplayColours[i]);
			}
		}
		break;

		case CPC_MONITOR_GT64:
		{
			for (i=0; i<32; i++)
			{
				int AdjustedLuminance  = Monitor_AdjustLuminanceForDisplay(Luminances[i]);
				/* does green show any red/blue at all? */
				DisplayColours[i].u.element.Red = 0;
				DisplayColours[i].u.element.Green = AdjustedLuminance;
				DisplayColours[i].u.element.Blue = 0;
			}
		}
		break;

		case CPC_MONITOR_MM12:
		{
			for (i=0; i<32; i++)
			{
				int AdjustedLuminance = Monitor_AdjustLuminanceForDisplay(Luminances[i]);
				DisplayColours[i].u.element.Red = AdjustedLuminance;
				DisplayColours[i].u.element.Green = AdjustedLuminance;
				DisplayColours[i].u.element.Blue = AdjustedLuminance;
			}
		}
		break;

	}

	
	/* update all colours in correct monitor mode. These will
	take effect at next rendering. */
	for (i=0; i<17; i++)
	{
/*		int Red, Green,Blue; */
		int HwColourIndex;

		/* get hardware colour index for this pen */
		HwColourIndex = GateArray_State.PenColour[i];

		/* get R,G,B from either colour, green screen etc colour table */
		/*   Red = DisplayColours[HwColourIndex].u.element.Red;
		   Green = DisplayColours[HwColourIndex].u.element.Green;
		   Blue = DisplayColours[HwColourIndex].u.element.Blue; */

		/* set colour */
		Render_SetColour(&DisplayColours[HwColourIndex],/*Red,Green,Blue,*/i);
	}
	
	Render_SetBlack(&DisplayColours[20]);
}


/*-------------------------------------------------------------------------*/

void    GateArray_Initialise(void)
{
	int i;

	/* setup green screen colours from hardware colours */
	for (i=0; i<32; i++)
	{
		/* R = 3.3K resistor
		 G = 1K resistor
		 B = 10K resistor */
		float R;
		float  G;
		float  B;
		float Luminance;
/* V=IR voltage drop */
		/* 
		R = 3.3K
		G = 1K
		B = 10K
		*/
	
		R = ((HardwareColours[i].u.element.Red>>4)&0x0f)/15.0f;
		G = ((HardwareColours[i].u.element.Green >> 4) & 0x0f) / 15.0f;
		B = ((HardwareColours[i].u.element.Blue >> 4) & 0x0f) / 15.0f;

		/* relatively r is reduced by 3 times */
		/* b is reduced by 10 times */
		Luminance = ((R*0.303f)+(B*0.1f)+G);
		/* scale it into range */
		Luminance = (Luminance/(0.303f+0.1f+1.0f))*255.0f;
		/* and make it an integer */
		Luminances[i] = (int)floorf(Luminance);
	}
}


void    GateArray_RestartPower(void)
{
	int i;

	GateArray_RestartReset();

	/* gerald's analysis of the ga seems to indicate that colours
	are 0 on reset although not explictly set by the hw. 0 equates to grey
	and matches more closely what people see when ga fails */
	for (i=0; i<15; i++)
	{
		GateArray_State.PenColour[i] = 0;
	}

	/* randomize pen selection */
	GateArray_State.PenSelection = rand()&0x01f;
}

void    GateArray_RestartReset(void)
{
	GateArray_State.RasterInterruptRequest = FALSE;

	/* SOFT 158 and Arnold 5A docs says that only border is black */
	GateArray_State.PenColour[16] = 20;

	/* set mode and rom enable register to zero, enabling both
	halves of the rom */
	GateArray_Write(0x080 | 0x00);

	GateArray_State.CRTCSyncInputs = 0;
	GateArray_State.BlankingOutput = 0;
	GateArray_State.nHBlankCycle = 0;
	GateArray_State.nVBlankCycle = 0;

	Render_SetPixelTranslation(0x0);
}


int             GateArray_GetPaletteColour(int PenIndex)
{
	return GateArray_State.PenColour[PenIndex];
}

int             GateArray_GetSelectedPen(void)
{
	return GateArray_State.PenSelection;
}


int             GateArray_GetMultiConfiguration(void)
{
	return GateArray_State.RomConfiguration;
}



void    GateArray_ClearInterruptCount(void)
{
	/* reset interrupt line count */
	GateArray_State.InterruptLineCount = 0;
}

void	GateArray_SetInterruptLineCount(int Count)
{
	GateArray_State.InterruptLineCount = Count;
}

int		GateArray_GetInterruptLineCount(void)
{
	return GateArray_State.InterruptLineCount;
}


BOOL GateArray_GetInterruptRequest(void)
{
	return GateArray_State.RasterInterruptRequest;
}

/* This is called when a OUT to Gate Array rom configuration/mode
select register, bit 4 is set */
void    GateArray_ClearInterrupt(void)
{
	GateArray_State.RasterInterruptRequest = FALSE;
	GateArray_ClearInterruptCount();
	Computer_RefreshInterrupt();
}

/*---------------------------------------------------------------------------*/

/* called when Z80 acknowledges interrupt */
Z80_BYTE    GateArray_AcknowledgeInterrupt(void)
{
	/* reset top bit of interrupt line counter */
	/* this ensures that the next interrupt is no closer than 32 lines */
	GateArray_State.InterruptLineCount &= 0x01f;

	/* Gate Array interrupt acknowledged; so clear interrupt request */
	GateArray_State.RasterInterruptRequest = FALSE;
	return 0x0ff;
}

void	GateArray_Update(void)
{
	/* increment interrupt line count */
	GateArray_State.InterruptLineCount++;

	/* if line == 52 then interrupt should be triggered */
	if (GateArray_State.InterruptLineCount==52)
	{
		/* clear counter. */
		GateArray_State.InterruptLineCount = 0;

		GateArray_State.RasterInterruptRequest = TRUE;

		Computer_RefreshInterrupt();

	}
}



void    GateArray_Write(int Function)
{
	switch (Function & 0x0c0)
	{
		case 0x000:
		{
			/* function 00xxxxxx */
			GateArray_State.PenSelection = (unsigned char)Function;

			if (Function & 0x010)
			{
				GateArray_State.PenIndex = (unsigned char)16;
			}
			else
			{
				GateArray_State.PenIndex = (unsigned char)(Function & 0x0f);
			}
 	}
		break;

		case 0x040:
		{

			/* function 01xxxxxx */

			int     PenIndex;
			int		ColourIndex;

			GateArray_State.ColourSelection = (unsigned char)Function;

			ColourIndex = Function & 0x01f;

			PenIndex = GateArray_State.PenIndex;

			GateArray_State.PenColour[PenIndex] = (unsigned char)ColourIndex;

			Render_SetColour(&DisplayColours[ColourIndex],PenIndex);
		}
		break;

		case 0x080:
		{
			/* function 10xxxxxx */
			GateArray_State.RomConfiguration = (unsigned char)Function;

			/* clear interrupt counter? */
			if (Function & 0x010)
			{
				GateArray_ClearInterrupt();
			}


			Computer_RethinkMemory();
		}
		break;

		/* not part of Gate Array in CPC464, CPC664 or CPC6128 */
		/* handled by PAL16L8 hardware */
		case 0x0c0:
			break;
	}
}


/* return red colour component for palette index specified */
int             GateArray_GetRed(int PaletteIndex)
{
	int Index = PaletteIndex;
	int HwColourIndex;
	if (PaletteIndex & 0x010)
	{
		Index = 16;
	}
	HwColourIndex = GateArray_State.PenColour[Index];

	return HardwareColours[HwColourIndex].u.element.Red;
}


/* return green colour component for palette index specified */
int             GateArray_GetGreen(int PaletteIndex)
{
	int HwColourIndex;

	int Index = PaletteIndex;
	if (PaletteIndex & 0x010)
	{
		Index = 16;
	}

	HwColourIndex = GateArray_State.PenColour[Index];

	return HardwareColours[HwColourIndex].u.element.Green;
}

/* return blue colour component for palette index specified */
int             GateArray_GetBlue(int PaletteIndex)
{
	int HwColourIndex;
	int Index = PaletteIndex;
	if (PaletteIndex & 0x010)
	{
		Index = 16;
	}
	HwColourIndex = GateArray_State.PenColour[Index];

	return HardwareColours[HwColourIndex].u.element.Blue;
}

void	GateArray_UpdateColours(void)
{
	int i;

	for (i=0; i<17; i++)
	{
		/*int Red,Green,Blue; */
		int HwColourIndex;

		HwColourIndex = GateArray_State.PenColour[i];

		Render_SetColour(&DisplayColours[HwColourIndex],/*Red,Green,Blue,*/i);
	}
}


