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
/* Rendering Functions */

#include "cpcglob.h"
#include "render.h"
#include "cpc.h"
#include "host.h"
#include "packedimage.h"
#include "monitor.h"

#define SHOW_SPEED

#ifdef SHOW_SPEED
#include "headers.h"
#endif

/*#define LESS_MULTS */

/*
#define CRTC_PIXELS_PER_CLOCK	16

static unsigned char CRTC_FirstVisibleClock;
static unsigned char CRTC_LastVisibleClock;

static void	CRTC_SetVisibleClocks
*/


extern unsigned char ASICCRTC_HorizontalSoftScroll;

/* if true, then memory has been setup to be filled with CPC graphics data, otherwise
no, so don't attempt to render anything */
static BOOL Renderer_Active = FALSE;
static int     BytesPerPixel=4;
int ScanLines = 0;
int FillScanLines = 1;
int FullDisplay = 0;
static int PIXEL_STEP=1;
int PIXEL_STEP_SHIFT=0;
static int Render_CPCRenderHeight, Render_CPCRenderWidth;
static int Render_CPCXOffset, Render_CPCYOffset;

static int MonitorWidth = X_CRTC_CHAR_WIDTH_DEFAULT;
static int MonitorHeight = Y_CRTC_LINE_HEIGHT_DEFAULT;

void	Render_SetMonitorDimensions(int Width, int Height)
{
	MonitorWidth = Width;
	MonitorHeight = Height;
}


/* the question is this:
 when gate-array is reset, vars are reset to 0.
 does this mean that mode 0 is used immediately.. or is something left before the hsync comes and it switches it?
*/
static PIXEL_DATA       NullPixelData[256];
static unsigned long		NullPackedPixels[256];
static void Render_DumpScreenNULL(RENDER_BUFFER_INFO *);

static void	Render_SetColourNULL(const RGBCOLOUR *pColour,/*int Red,
int Green, int Blue,*/ int Index);
static void	Render_SetBlackNULL(const RGBCOLOUR *pColour);
static void Render_PutDataWordNULL(int, unsigned long, int);
static void Render_PutSyncNULL(int, int);
static void Render_PutBorderNULL(int, int);
static void Render_PutBlackNULL(int, int);
static void Render_PutDataWordPLUSNULL(int HorizontalCount,unsigned long GraphicsData, int Line, unsigned long Mask, int *pPixels);

static void (*pRender_DumpScreen)(RENDER_BUFFER_INFO *pBufferInfo) = Render_DumpScreenNULL;
static void (*pRender_SetColour)(const RGBCOLOUR *pColour,/*int, int,
int,*/ int)=Render_SetColourNULL;
static void (*pRender_SetBlack)(const RGBCOLOUR *pColour)=Render_SetBlackNULL;
static void (*pRender_PutDataWord)(int, unsigned long, int)=Render_PutDataWordNULL;
static void (*pRender_PutSync)(int, int)=Render_PutSyncNULL;
static void (*pRender_PutBorder)(int, int)=Render_PutBorderNULL;
static void (*pRender_PutBlack)(int, int)=Render_PutBlackNULL;
static void (*pRender_PutDataWordPLUS)(int,unsigned long, int, unsigned long, int *) = Render_PutDataWordPLUSNULL;

/* start of screen buffer */
static unsigned char *pScreenBase = NULL;
/* number of bytes in screen buffer width */
static unsigned long ScreenPitch = 0;
static unsigned long ScreenHeight = 0;
static unsigned char *pScreenLine = NULL;
static unsigned char *pScreenLineDEBUG = NULL;

/* current graphics format */
static GRAPHICS_FORMAT  CurrentGraphicsFormat;

static int Render_ScreenXOffset, Render_ScreenYOffset;



/* colours not converted into destination format - this
is used when conversion between different colour formats is required
or conversion from a TrueColour to a paletted mode */
static PALETTE_ENTRY_RGB888    UnConvertedColourTable[32];
static PALETTE_ENTRY_RGB888    UnConvertedBlack;

/* **** RGB stuff **** */

/* each element contains colours packed into destination image format */
static unsigned long    ConvertedColourTable[32];
static  unsigned long   ConvertedBlack;

/* TrueColour RGB version of set colour */
static void Render_TrueColourRGB_SetColour(const RGBCOLOUR *pColour,
		/*int, int, int,*/int);
static void Render_TrueColourRGB_SetBlack(const RGBCOLOUR *pColour);
static void Render_TrueColourRGB_Setup(void);

/* **** PALETTE stuff **** */

/* this array is indexed with the pen index to display on this
screen, this then gives the index within the palette below for
the actual colour */
static signed long PaletteRemap[32];


/* colours in palette */

/* two palettes, one is visible while the other is being built */
static PALETTE_ENTRY    CPCPalette[256];

/* pointer to the visible palette */
/* palette we build up */

/* Paletted version of set colour */
static void Render_Paletted_SetColour(const RGBCOLOUR *pColour,
									  /*int,int,int,*/int);
static void Render_Paletted_Setup(void);



/*--------------------------------------------------------------------------*/
/* TrueColour RGB Stuff */

#define Render_TrueColourRGB_WriteColourToScreenBuffer(pScreen,ColourIndex) \
        ((unsigned long *)pScreen)[0] = ConvertedColourTable[ColourIndex]


/* given a R,G,B in 8:8:8 format, these tables will give the corresponding
R,G,B in X:Y:Z format */
static unsigned long RedConversionTable[256];
static unsigned long GreenConversionTable[256];
static unsigned long BlueConversionTable[256];


static void    Render_TrueColourRGB_BuildConversionTable(unsigned long
		*pTable, GRAPHICS_ELEMENT_FORMAT *pElement)
{
	int i;

	/* max value colour element can take */
	unsigned long MaxValue = 0x0ffffffff>>(32-pElement->BPP);

	/* this fraction is added each time through the loop. The upper
	16-bits hold the integer part and the lower 16-bits hold the fractional
	part. The following calculation works as long as the element has less
	than 16 bits for it's representation. (16-bit R, 16-bit G, 16-bit B is massive!!!) */
	unsigned long AddFraction = ((MaxValue+1)<<16)>>8;

	unsigned long CurrentValue = 0;

	for (i=0; i<256; i++)
	{
		/* store colour in packed colour format */
		pTable[i] = ((CurrentValue>>16)<<pElement->Shift);

		CurrentValue += AddFraction;
	}
}

static void    Render_TrueColourRGB_BuildConversionTables(GRAPHICS_FORMAT
		*pFormat)
{
	/* build red conversion table */
	Render_TrueColourRGB_BuildConversionTable(RedConversionTable,
			&pFormat->Red);
	/* build green conversion table */
	Render_TrueColourRGB_BuildConversionTable(GreenConversionTable,
			&pFormat->Green);
	/* build blue conversion table */
	Render_TrueColourRGB_BuildConversionTable(BlueConversionTable,
			&pFormat->Blue);
}

static void Render_TrueColourRGB_ConvertColoursToNewFormat(GRAPHICS_FORMAT
		*pDestFormat)
{
	int i;

	for (i=0; i<32; i++)
	{
		unsigned long r, g, b;
		unsigned long PackedColourData;

		r = UnConvertedColourTable[i].RGB.SeperateElements.u.element.Red;
		g = UnConvertedColourTable[i].RGB.SeperateElements.u.element.Green;
		b = UnConvertedColourTable[i].RGB.SeperateElements.u.element.Blue;

		/* store R,G,B colour into destination format */
		PackedColourData = PackRGBIntoDestinationImageFormat(r, g, b, pDestFormat);

		/* write packed R,G,B in dest format */
		ConvertedColourTable[i] = PackedColourData;
	}
}

static void Render_TrueColourRGB_Setup(void)
{
	Render_TrueColourRGB_BuildConversionTables(&CurrentGraphicsFormat);

	Render_TrueColourRGB_ConvertColoursToNewFormat(&CurrentGraphicsFormat);

	pRender_SetColour = Render_TrueColourRGB_SetColour;
	pRender_SetBlack = Render_TrueColourRGB_SetBlack;
	pRender_PutSync = Render_TrueColourRGB_PutSync;
	pRender_PutBorder = Render_TrueColourRGB_PutBorder;
	pRender_PutBlack = Render_TrueColourRGB_PutBlack;
	pRender_PutDataWord = Render_TrueColourRGB_PutDataWord;
	pRender_PutDataWordPLUS = Render_TrueColourRGB_PutDataWordPLUS;

	/*Render_SetTrueColourRender(TRUE); */

}

void	Render_TrueColourRGB_SetColour(const RGBCOLOUR *pColour,/*int Red,
int Green, int Blue,*/ int
									   Index)
{
	/* convert R,G,B into format of screen pixel */
	ConvertedColourTable[Index] = (RedConversionTable[pColour->u.element.Red & 0x0ff] |
								   GreenConversionTable[pColour->u.element.Green & 0x0ff] |
								   BlueConversionTable[pColour->u.element.Blue & 0x0ff]);
}


void	Render_TrueColourRGB_SetBlack(const RGBCOLOUR *pColour)
{
	/* convert R,G,B into format of screen pixel */
	ConvertedBlack = (RedConversionTable[pColour->u.element.Red & 0x0ff] |
								   GreenConversionTable[pColour->u.element.Green & 0x0ff] |
								   BlueConversionTable[pColour->u.element.Blue & 0x0ff]);
}


/*------------------------------------------------------------------------*/
/* PALETTED STUFF */

#define Render_Paletted_WriteColourToScreenBuffer(pScreen, ColourIndex) \
        ((unsigned char *)pScreen)[0] = PaletteRemap[ColourIndex]

static int Render_PaletteIndexBlack = 0;


/* find a colour match */
int     Palette_MatchColour(int Red, int Green, int Blue)
{
	int i;

	for (i=0; i<256; i++)
	{
		if (CPCPalette[i].Flags & PALETTE_ENTRY_USED)
		{
			if (!(CPCPalette[i].Flags & PALETTE_ENTRY_DO_NOT_MATCH))
			{
				if ((CPCPalette[i].Red == Red) && (CPCPalette[i].Green == Green) && (CPCPalette[i].Blue == Blue))
				{
					return i;
				}
			}
		}
	}

	return -1;
}


/* find a free palette entry */
int     Palette_StoreColour(int Red, int Green, int Blue)
{
	int i;

	for (i=0; i<256; i++)
	{
		/* is this entry free? */
		if (!(CPCPalette[i].Flags & PALETTE_ENTRY_USED))
		{
			/* free palette entry */

			CPCPalette[i].Red = (unsigned char)Red;
			CPCPalette[i].Green = (unsigned char)Green;
			CPCPalette[i].Blue = (unsigned char)Blue;

			/* mark it as used */
			CPCPalette[i].Flags |= PALETTE_ENTRY_USED;


			return i;
		}
	}

	/* if we get to here, no palette entries were free. */

	return -1;
}



/* current method is a bit slow, tries to keep all pens that
use the same colour pointing to same palette index, and drops
any colours that don't make it into the palette */
int     GetIndexInPalette(int PenIndex, int Red, int Green, int Blue)
{
	int PaletteIndex;
	int CurrentPaletteIndex;

	CurrentPaletteIndex = PaletteRemap[PenIndex];

	if (CurrentPaletteIndex==-1)
	{
		/* find a match */
		PaletteIndex = Palette_MatchColour(Red, Green, Blue);

		if (PaletteIndex==-1)
		{
			/* no match. Store colour in a free entry */
			PaletteIndex = Palette_StoreColour(Red, Green, Blue);
		}

		return PaletteIndex;
	}
	else
	{
		/* colour changed? */
		if ((CPCPalette[CurrentPaletteIndex].Red == Red) &&
				(CPCPalette[CurrentPaletteIndex].Green == Green) &&
				(CPCPalette[CurrentPaletteIndex].Blue == Blue))
		{
			/* same colour as before */
			PaletteIndex = CurrentPaletteIndex;
		}
		else
		{
			/* yes it's changed */

			/* find a match against existing colours in palette */
			PaletteIndex = Palette_MatchColour(Red, Green, Blue);

			if (PaletteIndex==-1)
			{
				/* no match. Store colour in a free entry */
				PaletteIndex = Palette_StoreColour(Red, Green, Blue);

				if (PaletteIndex==-1)
				{
					/* oh dear, can't store it, colour has just
					been dropped from palette */

					PaletteIndex = CurrentPaletteIndex;
				}
			}
		}
	}

	return PaletteIndex;
}


static void	Render_Paletted_SetColour(const RGBCOLOUR *pColour,/*int
Red, int Green, int Blue,*/
									  int Index)
{
	int PaletteIndex;

	PaletteIndex = GetIndexInPalette(Index, pColour->u.element.Red, pColour->u.element.Green, pColour->u.element.Blue);

	if (PaletteIndex!=-1)
	{
		/* set mapping */
		PaletteRemap[Index] = PaletteIndex;
	}
}

static void	Render_Paletted_Setup(void)
{
	pRender_SetColour = Render_Paletted_SetColour;

	pRender_PutSync = Render_Paletted_PutSync;
	pRender_PutBorder = Render_Paletted_PutBorder;
	pRender_PutDataWord = Render_Paletted_PutDataWord;
	pRender_PutDataWordPLUS = Render_Paletted_PutDataWordPLUS;

/*	Render_SetTrueColourRender(FALSE); */

}


static void Render_Paletted_ClearRemapping(void)
{
	int i;

	/* clear all re-map stuff */
	for (i=0; i<32; i++)
	{
		PaletteRemap[i] = -1;
	}
}

void    Palette_Initialise(void)
{
	Render_Paletted_ClearRemapping();
}

/* mark a palette entry as useable by host system, the palette
stuff will not then attempt to touch this colour */
void	Render_MarkPaletteEntryForHostUse(int Index)
{
	if ((Index<0) || (Index>255))
	{
		return;
	}

	CPCPalette[Index].Red = (unsigned char)Index;
	CPCPalette[Index].Green = 0;
	CPCPalette[Index].Blue = 0;
	CPCPalette[Index].Flags = PALETTE_ENTRY_USED | PALETTE_ENTRY_DO_NOT_MATCH | PALETTE_ENTRY_DO_NOT_REMOVE;
}

/* reset the palette. This can be done every VBL, so we can have different colours each
 VBL. */
void    Palette_Reset(void)
{
	int i;

	for (i=0; i<256; i++)
	{
		if (!(CPCPalette[i].Flags & PALETTE_ENTRY_DO_NOT_REMOVE))
		{
			CPCPalette[i].Flags = 0;
		}
	}

	for (i=0; i<32; i++)
	{
		PaletteRemap[i] = -1;
	}

/*	CPC_UpdateColours(); */
}

void	Palette_Set(void)
{
	int i;

	for (i=0; i<256; i++)
	{
		if (CPCPalette[i].Flags & PALETTE_ENTRY_USED)
		{
			if (!(CPCPalette[i].Flags & PALETTE_ENTRY_DO_NOT_MATCH))
			{
/*				unsigned char Red, Green, Blue;

				Red = CPCPalette[i].Red;
				Green = CPCPalette[i].Green;
				Blue = CPCPalette[i].Blue;
*/
				/* set palette entry in host */
				/*	Host_SetPaletteEntry(i, (unsigned char)Red, (unsigned char)Green, (unsigned char)Blue); */
			}
		}
	}
}




/*-----------------------------------------------------------------------*/


#ifdef LESS_MULTS
#define Render_CalcRenderAddress(HorizontalCount, Line)	\
	unsigned char *pScreen = (char *)pScreenLine
	/*char *pScreen = (char *)pScreenLineDEBUG */
#else
#define Render_CalcRenderAddress(HorizontalCount,Line)		\
    size_t XPixel = (HorizontalCount<<(1+3))>>PIXEL_STEP_SHIFT;		\
    size_t     ScreenOffset = (Line*ScreenPitch) + XPixel*BytesPerPixel; \
	unsigned char	*pScreen = (pScreenBase + ScreenOffset)
#endif

#ifdef LESS_MULTS
#define Render_UpdateRenderAddress			\
         pScreenLine = (unsigned char *)pScreen
#else
#define Render_UpdateRenderAddress
#endif

void    Render_Finish(void)
{
	if (pScreenBase!=NULL)
	{
		free(pScreenBase);
		pScreenBase = NULL;
	}
}

void	Render_SetColourNULL(const RGBCOLOUR *pColour,/*int Red, int
Green, int Blue,*/ int Index)
{
}


void	Render_SetBlackNULL(const RGBCOLOUR *pColour)
{
}

static void Render_PutDataWordNULL(int HorizontalCount, unsigned long GraphicsWord, int Line)
{
}

static void Render_DumpScreenNULL(RENDER_BUFFER_INFO *pInfo)
{
}


static void Render_PutSyncNULL(int HorizontalCount, int Line)
{
}

static void Render_PutBorderNULL(int HorizontalCount, int Line)
{
}


static void Render_PutBlackNULL(int HorizontalCount, int Line)
{
}

static void Render_PutDataWordPLUSNULL(int HorizontalCount,unsigned long GraphicsData, int Line, unsigned long Mask, int *pPixels)
{
}

/* set pen index specified by Index to Red, Green and Blue specified */
void    Render_SetBlack(const RGBCOLOUR *pColour)
{
	/* store chosen colour - in conversion table */
	UnConvertedBlack.RGB.SeperateElements.u.element.Red = pColour->u.element.Red;
	UnConvertedBlack.RGB.SeperateElements.u.element.Green = pColour->u.element.Green;
	UnConvertedBlack.RGB.SeperateElements.u.element.Blue = pColour->u.element.Blue;

	/* update palette (or encode into screen pixel format)
	depending on display */
	pRender_SetBlack(pColour);
}


/* set pen index specified by Index to Red, Green and Blue specified */
void    Render_SetColour(const RGBCOLOUR *pColour,/*int Red,int Green,int
Blue,*/ int Index)
{
	/* store chosen colour - in conversion table */
	UnConvertedColourTable[Index].RGB.SeperateElements.u.element.Red = pColour->u.element.Red;
	UnConvertedColourTable[Index].RGB.SeperateElements.u.element.Green = pColour->u.element.Green;
	UnConvertedColourTable[Index].RGB.SeperateElements.u.element.Blue = pColour->u.element.Blue;

	/* update palette (or encode into screen pixel format)
	depending on display */
	pRender_SetColour(pColour,/*Red,Green,Blue,*/Index);
}

void    Render_PutDataWord(int HorizontalCount,unsigned long GraphicsData, int Line)
{
	pRender_PutDataWord(HorizontalCount, GraphicsData, Line);
}

void	Render_PutSync(int HorizontalCount, int Line)
{
	
	pRender_PutSync(HorizontalCount, Line);
}

void    Render_PutBorder(int HorizontalCount, int Line)
{
	pRender_PutBorder(HorizontalCount, Line);
}


void    Render_PutBlack(int HorizontalCount, int Line)
{
	pRender_PutBlack(HorizontalCount, Line);
}

/* faster, but may not be accurate enough */
void    Render_PutDataWordPLUSMaskWithPixels(int HorizontalCount,unsigned long GraphicsData, int Line, unsigned long Mask, int *pPixels)
{
	pRender_PutDataWordPLUS(HorizontalCount,GraphicsData, Line, Mask, pPixels);
}


unsigned long *pPackedPixels = NullPackedPixels;
PIXEL_DATA *pPixelData = NullPixelData;
int CurrentMode = 0;
int CurrentModeMask = 0;
int CurrentModeShift = 0;
int ModeHorizontalPixelScroll = 0;
int CurrentTopShift = 0;

void    Render_SetPixelTranslation(int ModeIndex)
{
	CurrentMode = ModeIndex;
#if 0
	switch (CurrentMode)
	{
		case 0:
			CurrentModeMask = 0x0aaaaaaaa;
			CurrentModeShift = 2;
			CurrentTopShift = 7;
			break;
		case 1:
			CurrentModeMask = 0x0eeeeeeee;
			CurrentModeShift = 1;
			CurrentTopShift = 5;
			break;
		case 2:
			CurrentModeMask = 0x07f7f7f7f;
			CurrentModeShift = 0;
			CurrentTopShift = 1;
			break;
		case 3:
		{
			CurrentModeMask = 0x0eeeeeeee;
			CurrentModeShift = 2;
			CurrentTopShift = 7;
		}
		break;


	}
#endif

	pPixelData = CPC_GetModePixelData(ModeIndex);
	pPackedPixels = CPC_GetModePackedPixelData(ModeIndex);

}

#define GENERIC


void    Render_TrueColourRGB_PutDataWordHigh(int HorizontalCount,unsigned long GraphicsData, int Line)
{
	int i;
	PIXEL_DATA *pThisPixelData;

	Render_CalcRenderAddress(HorizontalCount,Line);

	pThisPixelData = &pPixelData[((GraphicsData>>8) & 0x0ff)];

	for (i=0; i<4; i+=PIXEL_STEP)
	{
		int     Pixel;

		Pixel = pThisPixelData->Pixel[i];
		Render_TrueColourRGB_WriteColourToScreenBuffer(pScreen, Pixel);
		pScreen+=BytesPerPixel;
	}

	pThisPixelData = &pPixelData[(GraphicsData & 0x0ff)];

	for (i=0; i<4; i+=PIXEL_STEP)
	{
		int Pixel;

		Pixel = pThisPixelData->Pixel[i];
		Render_TrueColourRGB_WriteColourToScreenBuffer(pScreen, Pixel);
		pScreen+=BytesPerPixel;
	}

	Render_UpdateRenderAddress;
}

#ifdef GENERIC
void    Render_TrueColourRGB_PutDataWord(int HorizontalCount,unsigned long GraphicsData, int Line)
{
	int i;
	PIXEL_DATA *pThisPixelData;

	Render_CalcRenderAddress(HorizontalCount,Line);
	if (pScreen==NULL)
		return;
	
	pThisPixelData = &pPixelData[((GraphicsData>>8) & 0x0ff)];

	for (i=0; i<8; i+=PIXEL_STEP)
	{
		int     Pixel;

		Pixel = pThisPixelData->Pixel[i];
		Render_TrueColourRGB_WriteColourToScreenBuffer(pScreen, Pixel);
		pScreen+=BytesPerPixel;
	}

	pThisPixelData = &pPixelData[(GraphicsData & 0x0ff)];

	for (i=0; i<8; i+=PIXEL_STEP)
	{
		int Pixel;

		Pixel = pThisPixelData->Pixel[i];
		Render_TrueColourRGB_WriteColourToScreenBuffer(pScreen, Pixel);
		pScreen+=BytesPerPixel;
	}

	Render_UpdateRenderAddress;
}
#else
void    Render_TrueColourRGB_PutDataWord(int HorizontalCount,unsigned long GraphicsData, int Line)
{
	int ScreenPixel;
	unsigned long PackedPixels;
	unsigned long Pixel;
	Render_CalcRenderAddress(HorizontalCount,Line);


	if (PIXEL_STEP == 1) /* Fixed! RFB */
	{
		PackedPixels = pPackedPixels[((GraphicsData>>8) & 0x0ff)];

		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>4;
		ScreenPixel = ConvertedColourTable[Pixel];
		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>4;
		ScreenPixel = ScreenPixel | (ConvertedColourTable[Pixel]<<16);
		((unsigned long *)pScreen)[0] = ScreenPixel;

		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>4;
		ScreenPixel = ConvertedColourTable[Pixel];
		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>4;
		ScreenPixel = ScreenPixel | (ConvertedColourTable[Pixel]<<16);
		((unsigned long *)pScreen)[1] = ScreenPixel;

		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>4;
		ScreenPixel = ConvertedColourTable[Pixel];
		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>4;
		ScreenPixel = ScreenPixel | (ConvertedColourTable[Pixel]<<16);
		((unsigned long *)pScreen)[2] = ScreenPixel;

		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>4;
		ScreenPixel = ConvertedColourTable[Pixel];
		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>4;
		ScreenPixel = ScreenPixel | (ConvertedColourTable[Pixel]<<16);
		((unsigned long *)pScreen)[3] = ScreenPixel;

		pScreen += (4 * 4);

		PackedPixels = pPackedPixels[(GraphicsData & 0x0ff)];

		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>4;
		ScreenPixel = ConvertedColourTable[Pixel];
		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>4;
		ScreenPixel = ScreenPixel | (ConvertedColourTable[Pixel]<<16);
		((unsigned long *)pScreen)[0] = ScreenPixel;

		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>4;
		ScreenPixel = ConvertedColourTable[Pixel];
		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>4;
		ScreenPixel = ScreenPixel | (ConvertedColourTable[Pixel]<<16);
		((unsigned long *)pScreen)[1] = ScreenPixel;

		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>4;
		ScreenPixel = ConvertedColourTable[Pixel];
		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>4;
		ScreenPixel = ScreenPixel | (ConvertedColourTable[Pixel]<<16);
		((unsigned long *)pScreen)[2] = ScreenPixel;

		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>4;
		ScreenPixel = ConvertedColourTable[Pixel];
		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>4;
		ScreenPixel = ScreenPixel | (ConvertedColourTable[Pixel]<<16);
		((unsigned long *)pScreen)[3] = ScreenPixel;

		pScreen += (4 * 4);
	}
	else
	{


		PackedPixels = pPackedPixels[((GraphicsData>>8) & 0x0ff)];


#ifndef CPC_LSB_FIRST
		PackedPixels = (((PackedPixels & 0x0000FF00) >> 8) |
						((PackedPixels & 0x000000FF) << 8) |
						((PackedPixels & 0xFF000000) >> 8) |
						((PackedPixels & 0x00FF0000) << 8));
#endif

		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>8;
		ScreenPixel = ConvertedColourTable[Pixel];
		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>8;
		ScreenPixel = ScreenPixel | (ConvertedColourTable[Pixel]<<16);
		((unsigned long *)pScreen)[0] = ScreenPixel;

		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>8;
		ScreenPixel = ConvertedColourTable[Pixel];
		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>8;
		ScreenPixel = ScreenPixel | (ConvertedColourTable[Pixel]<<16);
		((unsigned long *)pScreen)[1] = ScreenPixel;

		PackedPixels = pPackedPixels[(GraphicsData & 0x0ff)];

#ifndef CPC_LSB_FIRST
		PackedPixels = (((PackedPixels & 0x0000FF00) >> 8) |
						((PackedPixels & 0x000000FF) << 8) |
						((PackedPixels & 0xFF000000) >> 8) |
						((PackedPixels & 0x00FF0000) << 8));
#endif

		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>8;
		ScreenPixel = ConvertedColourTable[Pixel];
		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>8;
		ScreenPixel = ScreenPixel | (ConvertedColourTable[Pixel]<<16);
		((unsigned long *)pScreen)[2] = ScreenPixel;

		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>8;
		ScreenPixel = ConvertedColourTable[Pixel];
		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>8;
		ScreenPixel = ScreenPixel | (ConvertedColourTable[Pixel]<<16);
		((unsigned long *)pScreen)[3] = ScreenPixel;

		pScreen+=(4*4);
	}

	Render_UpdateRenderAddress;
}
#endif

#define CPC_WRITE_PIXEL_INC(ci, dest,increment)	\
		((unsigned long *)dest)[0] = ci;	\
		dest = &dest[increment]

#ifdef GENERIC
void    Render_TrueColourRGB_PutSync(int HorizontalCount, int Line)
{
	int i;

	Render_CalcRenderAddress(HorizontalCount,Line);
	if (pScreen==NULL)
		return;
	
	/*crash because pScreenLine have moved outside the limit */

	for (i=0; i<16; i+=PIXEL_STEP)
	{
		((unsigned long *)pScreen)[0] = 0x0;
		pScreen+=BytesPerPixel;
	}

	Render_UpdateRenderAddress;
}
#else
void    Render_TrueColourRGB_PutSync(int HorizontalCount, int Line)
{

	Render_CalcRenderAddress(HorizontalCount,Line);


	((unsigned long *)pScreen)[0] = 0x0;
	((unsigned long *)pScreen)[1] = 0x0;
	((unsigned long *)pScreen)[2] = 0x0;
	((unsigned long *)pScreen)[3] = 0x0;
	pScreen+=(4*4);
	Render_UpdateRenderAddress;
}
#endif
void    Render_TrueColourRGB_PutSyncHigh(int HorizontalCount, int Line)
{
	int i;

	Render_CalcRenderAddress(HorizontalCount,Line);

	for (i=0; i<8; i+=PIXEL_STEP)
	{
		((unsigned long *)pScreen)[0] = 0;
		pScreen+=BytesPerPixel;
	}

	Render_UpdateRenderAddress;
}
void    Render_TrueColourRGB_PutBorderHigh(int HorizontalCount, int Line)
{
	int i;
	int BorderColour = ConvertedColourTable[16];

	Render_CalcRenderAddress(HorizontalCount,Line);
	
	for (i=0; i<8; i+=PIXEL_STEP)
	{
		((unsigned long *)pScreen)[0] = BorderColour;
		pScreen+=BytesPerPixel;
	}

	Render_UpdateRenderAddress;
}
#ifdef GENERIC
void    Render_TrueColourRGB_PutBlack(int HorizontalCount, int Line)
{
	int i;

	Render_CalcRenderAddress(HorizontalCount,Line);
	if (pScreen==NULL)
		return;
	
	/*crash because pScreenLine have moved outside the limit */

	for (i=0; i<16; i+=PIXEL_STEP)
	{
		((unsigned long *)pScreen)[0] = ConvertedBlack;
		pScreen+=BytesPerPixel;
	}

	Render_UpdateRenderAddress;
}
#else
#endif

#ifdef GENERIC
void    Render_TrueColourRGB_PutBorder(int HorizontalCount, int Line)
{
	int i;
	int BorderColour = ConvertedColourTable[16];

	Render_CalcRenderAddress(HorizontalCount,Line);
	if (pScreen==NULL)
		return;
	
	for (i=0; i<16; i+=PIXEL_STEP)
	{
		((unsigned long *)pScreen)[0] = BorderColour;
		pScreen+=BytesPerPixel;
	}

	Render_UpdateRenderAddress;
}
#else
void    Render_TrueColourRGB_PutBorder(int HorizontalCount, int Line)
{
	int BorderColour;

	Render_CalcRenderAddress(HorizontalCount, Line);

	BorderColour = ConvertedColourTable[16];
	BorderColour = BorderColour | (BorderColour<<16);

	((unsigned long *)pScreen)[0] = BorderColour;
	((unsigned long *)pScreen)[1] = BorderColour;
	((unsigned long *)pScreen)[2] = BorderColour;
	((unsigned long *)pScreen)[3] = BorderColour;
	pScreen+=(4*4);

	Render_UpdateRenderAddress;
}
#endif

void    Render_Paletted_PutDataWord(int HorizontalCount,unsigned long GraphicsData, int Line)
{
	int i;
	PIXEL_DATA *pThisPixelData;

	Render_CalcRenderAddress(HorizontalCount,Line);

	pThisPixelData = &pPixelData[((GraphicsData>>8) & 0x0ff)];

	for (i=0; i<8; i+=PIXEL_STEP)
	{
		int     Pixel;

		Pixel = pThisPixelData->Pixel[i];
		Render_Paletted_WriteColourToScreenBuffer(pScreen, Pixel);
		pScreen+=BytesPerPixel;
	}

	pThisPixelData = &pPixelData[(GraphicsData & 0x0ff)];

	for (i=0; i<8; i+=PIXEL_STEP)
	{
		int Pixel;

		Pixel = pThisPixelData->Pixel[i];
		Render_Paletted_WriteColourToScreenBuffer(pScreen, Pixel);
		pScreen+=BytesPerPixel;
	}

	Render_UpdateRenderAddress;
}

void    Render_Paletted_PutSync(int HorizontalCount, int Line)
{
	int i;

	Render_CalcRenderAddress(HorizontalCount,Line);

	for (i=0; i<16; i+=PIXEL_STEP)
	{
		((unsigned char *)pScreen)[0] = Render_PaletteIndexBlack;
		pScreen+=BytesPerPixel;
	}

	Render_UpdateRenderAddress;
}

void    Render_Paletted_PutBorder(int HorizontalCount, int Line)
{
	int i;

	Render_CalcRenderAddress(HorizontalCount, Line);

	for (i=0; i<16; i+=PIXEL_STEP)
	{
		Render_TrueColourRGB_WriteColourToScreenBuffer(pScreen, 16);
		pScreen+=BytesPerPixel;
	}

	Render_UpdateRenderAddress;
}


static int HorizontalPixelScroll = 0;

void    Render_SetHorizontalPixelScroll(int PixelScroll)
{
	HorizontalPixelScroll = PixelScroll;

	ModeHorizontalPixelScroll = HorizontalPixelScroll>>CurrentModeShift;
}

// this needs review to make it the same as a real plus but works well for now
int GetPixelScroll(PIXEL_DATA * Current, PIXEL_DATA * Previous1, PIXEL_DATA * Previous2, int pix, int nb)
{

	//1 to 15 scrolling pixels possible
	if ((nb < 16) && (nb > 0))
	{
		if (nb > 0)
		{
			if ((pix - nb) >= 0)
			{
				return Current->Pixel[pix - nb];
			}
			else
			{
				if ((pix - nb + 8) >= 0)
				{
					return Previous1->Pixel[pix - nb + 8];
				}
				else
				{
					//pas lui
					return Previous2->Pixel[pix - nb + 16];
				}
			}
		}
	}

	return Current->Pixel[pix];
}

#ifdef GENERIC
/* faster, but may not be accurate enough */
void    Render_TrueColourRGB_PutDataWordPLUS(int HorizontalCount, unsigned long GraphicsData, int Line, unsigned long Mask, int *pPixels)
{
	int i;

	//current pixels
	PIXEL_DATA *pThisPixelData;
	//previous pixels
	PIXEL_DATA *pThisPixelDataP1;
	PIXEL_DATA *pThisPixelDataP2;

	int nbtoroll = 0;

	Render_CalcRenderAddress(HorizontalCount, Line);

	//GraphicsData = XX XX XX XX
	//               ^  ^   ^  ^
	//               |   | |__|________ Low Word used normaly
	//               |___|_____________ Hight word needed for scroll

	nbtoroll = ASICCRTC_HorizontalSoftScroll; //number of byte to roll
	/* Tested with dick tracy this value is : 0  4  8  12 */

	{
		//here we need
		//GraphicsData = SS SS NN ..        SS  = previous pixels used for scroll  NN = current pixel used normally
		pThisPixelData = &pPixelData[(unsigned long)(GraphicsData & 0x0ff00) >> 8];
		pThisPixelDataP1 = &pPixelData[(unsigned long)(GraphicsData & 0x0ff0000) >> 16];
		pThisPixelDataP2 = &pPixelData[(unsigned long)(GraphicsData & 0x0ff000000) >> 24];

		for (i = 0; i<8; i += PIXEL_STEP)
		{
			int     Pixel;

			if ((Mask & (1 << i)) != 0)
			{
				Pixel = GetPixelScroll(pThisPixelData, pThisPixelDataP1, pThisPixelDataP2, i, nbtoroll); //pThisPixelData->Pixel[i];
			}
			else
			{
				Pixel = pPixels[i];
			}

			Render_TrueColourRGB_WriteColourToScreenBuffer(pScreen, Pixel);
			pScreen += BytesPerPixel;
		}
	}


	{
		//here we need
		//GraphicsData = .. SS SS NN        SS  = previous pixels used for scroll  NN = current pixel used normally
		pThisPixelData = &pPixelData[(unsigned long)(GraphicsData & 0x0ff)];
		pThisPixelDataP1 = &pPixelData[(unsigned long)(GraphicsData & 0x0ff00) >> 8];
		pThisPixelDataP2 = &pPixelData[(unsigned long)(GraphicsData & 0x00ff0000) >> 16];


		for (i=0; i<8; i+=PIXEL_STEP)
		{
			int     Pixel;

			if ((Mask & (1<<(i+8)))!=0)
			{
				Pixel = GetPixelScroll(pThisPixelData,pThisPixelDataP1,pThisPixelDataP2,i,nbtoroll); //pThisPixelData->Pixel[i];
			}
			else
			{
				Pixel = pPixels[i+8];
			}

			Render_TrueColourRGB_WriteColourToScreenBuffer(pScreen, Pixel);

			pScreen+=BytesPerPixel;
		}
	}

	Render_UpdateRenderAddress;
}
#else
void    Render_TrueColourRGB_PutDataWordPLUS(int HorizontalCount,unsigned long GraphicsData, int Line, unsigned long Mask, int *pPixels)
{
	int i;

	Render_CalcRenderAddress(HorizontalCount,Line);

	for (i=0; i<ModeHorizontalPixelScroll; i++)
	{
		GraphicsData = (
						   ((GraphicsData & CurrentModeMask)>>1) |
						   ((GraphicsData & (~CurrentModeMask))>>CurrentTopShift)
					   );
	}

	if ((Mask & 0x0ffff)==0x0ffff)
	{
		/* all screen pixels */
		unsigned long PackedPixels;
		int Pixel;
		unsigned long ScreenPixel;

		PackedPixels = pPackedPixels[(GraphicsData & 0x0ff00)>>8];

		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>8;
		ScreenPixel = ConvertedColourTable[Pixel];

		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>8;
		ScreenPixel = ScreenPixel | (ConvertedColourTable[Pixel]<<16);
		((unsigned long *)pScreen)[0] = ScreenPixel;

		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>8;
		ScreenPixel = ConvertedColourTable[Pixel];

		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>8;
		ScreenPixel = ScreenPixel | (ConvertedColourTable[Pixel]<<16);
		((unsigned long *)pScreen)[1] = ScreenPixel;

		PackedPixels = pPackedPixels[(GraphicsData & 0x0ff)];

		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>8;
		ScreenPixel = ConvertedColourTable[Pixel];

		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>8;
		ScreenPixel = ScreenPixel | (ConvertedColourTable[Pixel]<<16);
		((unsigned long *)pScreen)[2] = ScreenPixel;

		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>8;
		ScreenPixel = ConvertedColourTable[Pixel];

		Pixel = (PackedPixels & 0x0f);
		PackedPixels = PackedPixels>>8;
		ScreenPixel = ScreenPixel | (ConvertedColourTable[Pixel]<<16);
		((unsigned long *)pScreen)[3] = ScreenPixel;


		pScreen+=(4*4);



	}
	else
		if ((Mask & 0x0ffff)==0x0000)
		{
			unsigned long PackedPixels;
			int Pixel;
			unsigned long ScreenPixel;

			PackedPixels = pPackedPixels[(GraphicsData & 0x0ff00)>>8];

			{
				Pixel = pPixels[0];
			}

			PackedPixels = PackedPixels>>8;
			ScreenPixel = ConvertedColourTable[Pixel];

			{
				Pixel = pPixels[2];
			}


			PackedPixels = PackedPixels>>8;
			ScreenPixel = ScreenPixel | (ConvertedColourTable[Pixel]<<16);
			((unsigned long *)pScreen)[0] = ScreenPixel;

			{
				Pixel = pPixels[4];
			}

			PackedPixels = PackedPixels>>8;
			ScreenPixel = ConvertedColourTable[Pixel];

			{
				Pixel = pPixels[6];
			}


			PackedPixels = PackedPixels>>8;
			ScreenPixel = ScreenPixel | (ConvertedColourTable[Pixel]<<16);
			((unsigned long *)pScreen)[1] = ScreenPixel;

			PackedPixels = pPackedPixels[(GraphicsData & 0x0ff)];

			{
				Pixel = pPixels[8];
			}

			PackedPixels = PackedPixels>>8;
			ScreenPixel = ConvertedColourTable[Pixel];

			{
				Pixel = pPixels[10];
			}


			PackedPixels = PackedPixels>>8;
			ScreenPixel = ScreenPixel | (ConvertedColourTable[Pixel]<<16);
			((unsigned long *)pScreen)[2] = ScreenPixel;

			{
				Pixel = pPixels[12];
			}

			PackedPixels = PackedPixels>>8;
			ScreenPixel = ConvertedColourTable[Pixel];

			{
				Pixel = pPixels[14];
			}


			PackedPixels = PackedPixels>>8;
			ScreenPixel = ScreenPixel | (ConvertedColourTable[Pixel]<<16);
			((unsigned long *)pScreen)[3] = ScreenPixel;


			pScreen+=(4*4);




		}
		else
		{
			unsigned long PackedPixels;
			int Pixel;
			unsigned long ScreenPixel;

			PackedPixels = pPackedPixels[(GraphicsData & 0x0ff00)>>8];

			if ((Mask & (1<<0))!=0)
			{
				Pixel = (PackedPixels & 0x0f);
			}
			else
			{
				Pixel = pPixels[0];
			}

			PackedPixels = PackedPixels>>8;
			ScreenPixel = ConvertedColourTable[Pixel];

			if ((Mask & (1<<2))!=0)
			{
				Pixel = (PackedPixels & 0x0f);
			}
			else
			{
				Pixel = pPixels[2];
			}


			PackedPixels = PackedPixels>>8;
			ScreenPixel = ScreenPixel | (ConvertedColourTable[Pixel]<<16);
			((unsigned long *)pScreen)[0] = ScreenPixel;

			if ((Mask & (1<<4))!=0)
			{
				Pixel = (PackedPixels & 0x0f);
			}
			else
			{
				Pixel = pPixels[4];
			}

			PackedPixels = PackedPixels>>8;
			ScreenPixel = ConvertedColourTable[Pixel];

			if ((Mask & (1<<6))!=0)
			{
				Pixel = (PackedPixels & 0x0f);
			}
			else
			{
				Pixel = pPixels[6];
			}


			PackedPixels = PackedPixels>>8;
			ScreenPixel = ScreenPixel | (ConvertedColourTable[Pixel]<<16);
			((unsigned long *)pScreen)[1] = ScreenPixel;

			PackedPixels = pPackedPixels[(GraphicsData & 0x0ff)];

			if ((Mask & (1<<8))!=0)
			{
				Pixel = (PackedPixels & 0x0f);
			}
			else
			{
				Pixel = pPixels[8];
			}

			PackedPixels = PackedPixels>>8;
			ScreenPixel = ConvertedColourTable[Pixel];

			if ((Mask & (1<<10))!=0)
			{
				Pixel = (PackedPixels & 0x0f);
			}
			else
			{
				Pixel = pPixels[10];
			}


			PackedPixels = PackedPixels>>8;
			ScreenPixel = ScreenPixel | (ConvertedColourTable[Pixel]<<16);
			((unsigned long *)pScreen)[2] = ScreenPixel;

			if ((Mask & (1<<12))!=0)
			{
				Pixel = (PackedPixels & 0x0f);
			}
			else
			{
				Pixel = pPixels[12];
			}

			PackedPixels = PackedPixels>>8;
			ScreenPixel = ConvertedColourTable[Pixel];

			if ((Mask & (1<<14))!=0)
			{
				Pixel = (PackedPixels & 0x0f);
			}
			else
			{
				Pixel = pPixels[14];
			}


			PackedPixels = PackedPixels>>8;
			ScreenPixel = ScreenPixel | (ConvertedColourTable[Pixel]<<16);
			((unsigned long *)pScreen)[3] = ScreenPixel;


			pScreen+=(4*4);


		}


	Render_UpdateRenderAddress;
}
#endif



/* faster, but may not be accurate enough */
void    Render_Paletted_PutDataWordPLUS(int HorizontalCount,unsigned long GraphicsData, int Line, unsigned long Mask, int *pPixels)
{
	int i;

	PIXEL_DATA *pThisPixelData;

	Render_CalcRenderAddress(HorizontalCount,Line);

	GraphicsData = GraphicsData>>ModeHorizontalPixelScroll;

	{
		pThisPixelData = &pPixelData[(GraphicsData & 0x0ff00)>>8];

		for (i=0; i<8; i+=PIXEL_STEP)
		{
			int     Pixel;

			if ((Mask & (1<<i))!=0)
			{
				Pixel = pThisPixelData->Pixel[i];
			}
			else
			{
				Pixel = pPixels[i];
			}

			Render_Paletted_WriteColourToScreenBuffer(pScreen, Pixel);
			pScreen+=BytesPerPixel;
		}
	}

	{
		pThisPixelData = &pPixelData[(GraphicsData & 0x0ff)];

		for (i=0; i<8; i+=PIXEL_STEP)
		{
			int     Pixel;

			if ((Mask & (1<<(i+8)))!=0)
			{
				Pixel = pThisPixelData->Pixel[i];
			}
			else
			{
				Pixel = pPixels[i+8];
			}

			Render_Paletted_WriteColourToScreenBuffer(pScreen, Pixel);
			pScreen+=BytesPerPixel;
		}
	}

	Render_UpdateRenderAddress;
}
#if 0
void	my_memcpy(void *pDest, void *pSrc, unsigned long Length)
{
	/* align start */

	unsigned char *pDestPtr, *pSrcPtr;

	pDestPtr = pDest;
	pSrcPtr = pSrc;

	{
		int i;
		int LengthInLongs = Length>>2;

		for (i=0; i<LengthInLongs; i++)
		{
			((unsigned long *)pDestPtr)[0] = ((unsigned long *)pSrcPtr)[0];
			pDestPtr+=4;
			pSrcPtr+=4;
		}
	}

	{
		int i;
		int BytesRemaining = Length - (Length  & (~0x02));

		for (i=0; i<BytesRemaining; i++)
		{
			((unsigned char *)pDestPtr)[0] = ((unsigned char *)pSrcPtr)[0];
			pDestPtr++;
			pSrcPtr++;
		}
	}
}
#endif
void	my_memcpy(unsigned char *pDest, unsigned char *pSrc, unsigned long Length)
{
	/* align start */
	unsigned char *pDestPtr, *pSrcPtr;

	pDestPtr = pDest;
	pSrcPtr = pSrc;

	{
		int i;
		int LengthInLongs = Length>>2;
		unsigned long Offset = 0;
		unsigned long *pLongSrcPtr = (unsigned long *)pSrcPtr, *pLongDestPtr = (unsigned long *)pDestPtr;

		for (i=0; i<LengthInLongs; i++)
		{
			pLongDestPtr[Offset] = pLongSrcPtr[Offset];

			Offset++;
		}


	}
}


void    Render_DumpScreen4(RENDER_BUFFER_INFO *pGraphicsBufferInfo)
{
	unsigned char   *pScreen;
	unsigned char   *pScr;
	unsigned char   *pSurface;
	unsigned char   *pSurf;
	int i;
	int nBytesPerPixel = ((CurrentGraphicsFormat.BPP+7)>>3);
	int Pitch;

	/* lock surface */
	/*   if (Host_LockGraphicsBuffer()) */
	{
		/*    GRAPHICS_BUFFER_INFO *pGraphicsBufferInfo = Host_GetGraphicsBufferInfo(); */

		int XOffset;
		int XLength;
	//	int RenderXOffset;

		Pitch = pGraphicsBufferInfo->SurfacePitch;

		/* source address */
		pScreen = pScreenBase + 64 * 64 * 8; /* aeliss fix to remove bad line */
		if (pScreen==NULL)
			return;
		
		/* dest address */
		pSurface = (unsigned char *)pGraphicsBufferInfo->pSurface;
		if (pSurface==NULL)
			return;
		

		/* if scanlines is true, this must be done before the pitch is adjusted. */
		pSurface += Render_ScreenYOffset*Pitch;

		if (ScanLines!=0)
		{
			Pitch = Pitch<<1;
		}

		pScr = pScreen;
		pSurf = pSurface;

		pScr += (Render_CPCYOffset+pGraphicsBufferInfo->CPCOffsetY)*ScreenPitch;

		XOffset = (Render_CPCXOffset+pGraphicsBufferInfo->CPCOffsetX)* nBytesPerPixel;
		XLength = pGraphicsBufferInfo->SurfaceWidth * nBytesPerPixel;
	
		int Height = pGraphicsBufferInfo->SurfaceHeight;
		if (ScanLines != 0)
		{
			Height = Height >> 1;
		}

		for (i = 0; i<Height; i++)
		{
			pSurface = pSurf;
			pScreen = pScr + XOffset;

			/*
			{
				unsigned long l;
				unsigned char *pSrcAddr = pScreen;
				unsigned char *pDestAddr = pSurface;

				for (l=0; l<XLength>>2; l++)
				{
					((unsigned long *)pDestAddr)[0] = ((unsigned long *)pSrcAddr)[0];
					((unsigned long *)pDestAddr)++;
					((unsigned long *)pSrcAddr)++;
				}

				for (l=0; l<(XLength&3); l++)
				{
					pDestAddr[0] = pSrcAddr[0];
					pDestAddr++;
					pSrcAddr++;
				}
			}
			*/
			/* FIXME: SDL crashes here! */
			memcpy(pSurface, pScreen, XLength);

			/* scanline display, but fill them */
			if (FillScanLines && ScanLines)
			{
				pSurface+=Pitch>>1;
				/* my_memcpy has a bug in it which copies too much and
				causes an access violation under linux. In addition memcpy
				should be faster than a version we implement ;) */
				memcpy(pSurface, pScreen, XLength);
			}
			else if (ScanLines)
			{
				/* scanline display but don't fill them */
				pSurface+=Pitch>>1;

				/* make scanlines black between - fix by Aeliss */
				memset(pSurface, 0, XLength);
			}

			pScr+=ScreenPitch;

			pSurf+=Pitch;
		}

		/* unlock surface */
		/*   Host_UnlockGraphicsBuffer();

			Host_RenderLEDs();
*/
		/* flip screen (in windowed mode performs a blit) */
		/*  Host_SwapGraphicsBuffers(); */

	}
}

void    Render_ClearDisplay(void)
{
	if (pScreenBase!=NULL)
	{
		memset(pScreenBase,0, ScreenPitch*(LINES_PER_SCREEN+1));
	}
}

void    Render_DumpDisplay(RENDER_BUFFER_INFO *pBufferInfo)
{
	pRender_DumpScreen(pBufferInfo);

	/*		Palette_Set();

			Palette_Reset(); */
}





void    Render_Initialise(void)
{
    int i;
    int b;
//printf("Render_Initialise\n");
	CPC_BuildModeRenderTables();

    for (i=0; i<256; i++)
    {
        for (b=0; b<8; b++)
        {
            NullPixelData[i].Pixel[b] = 0;
        }
        NullPackedPixels[i] = 0;
    }

    pPixelData = NullPixelData;
    pPackedPixels = NullPackedPixels;

	pScreenBase = NULL;
	ScreenPitch = 0;
	Renderer_Active = FALSE;

	Palette_Initialise();

	Render_SetRenderingAccuracyForWindowedMode(RENDERING_ACCURACY_HIGH);
}




void    Render_SetRenderingAccuracy(int Accuracy)
{
	if (Accuracy == RENDERING_ACCURACY_LOW)
	{
		/* mis out every other pixel in x to maintain image aspect,
		screen is smaller */
		PIXEL_STEP = 2;
		PIXEL_STEP_SHIFT = 1;
		ScanLines = 0;
		FillScanLines = 0;
		FullDisplay = 0;
	}

	if (Accuracy == RENDERING_ACCURACY_HIGH)
	{
		/* show every pixel, but render black lines between each graphical
		line to maintain image aspect - closer to tv image with this mode */
		PIXEL_STEP=1;
		PIXEL_STEP_SHIFT=0;
		ScanLines = 1;
		FillScanLines = 0;
		FullDisplay = 0;
	}

	if (Accuracy == RENDERING_ACCURACY_HIGHER)
	{
		/* show every pixel, but render black lines between each graphical
		line to maintain image aspect - closer to tv image with this mode */
		PIXEL_STEP=1;
		PIXEL_STEP_SHIFT=0;
		ScanLines = 1;
		FullDisplay = 0;
		FillScanLines = 1;
	}

}

/* Troels K. begin */
/*static*/ int Render_RenderingAccuracyForWindowedMode = RENDERING_ACCURACY_HIGH;
/* Troels K. end */


void	Render_SetRenderingAccuracyForWindowedMode(int Accuracy)
{
	Render_RenderingAccuracyForWindowedMode = Accuracy;

	Render_SetRenderingAccuracy(Accuracy);
}

void Render_SetFillScanlines(BOOL bState)
{
	FillScanLines = bState ? 1 : 0;
}

BOOL Render_GetFillScanlines(void)
{
	return FillScanLines;
}

void Render_SetFullDisplay(BOOL bState)
{
	FullDisplay = bState ? 1 : 0;
}

BOOL Render_GetFullDisplay(void)
{
	return FullDisplay;
}


void Render_SetScanlines(BOOL bState)
{
	ScanLines = bState ? 1 : 0;
}

BOOL Render_GetScanlines(void)
{
	return ScanLines;
}


/*

   ^   ----------------------------------
   B   |                                |
   ^   |      --------------------      |   ^
       |  	  |                  |      |   |
	   |	  |                  |      |   D
	   |	  |                  |      |   |
	   |	  |                  |      |   |
	   |	  --------------------      |
	   |                                |
	   ----------------------------------
	   <--A-->
	           <-----  C  ------>
	   

	   A = Render_CPCXOffset
	   B = Render_CPCYOffset
	   C = Render_CPCRenderWidth
	   D = Render_CPCRenderHeight
*/

/* ScreenResX, ScreenResY define rendering window dimensions.
 In FullScreen mode these are defined by the selected screen resolution.
 In Windowed mode these can be changed to allow for full-display */
BOOL InitialiseRender(int ScreenResX, int ScreenResY, int BPP)
{
	int ScreenWidth;
	//printf("InitialiseRender\n");
	if (pScreenBase!=NULL)
	{
		free(pScreenBase);
		pScreenBase = NULL;
		ScreenPitch = 0;
	}


	ScreenWidth = BITS_PER_LINE;
	ScreenHeight = LINES_PER_SCREEN+1;

	/* for full display */
	if (ScanLines)
	{
		ScreenHeight = ScreenHeight << 1;
	}
	ScreenPitch = (unsigned long)ScreenWidth * ((BPP+7)>>3);
	ScreenPitch += ((4-(ScreenPitch & 0x03)) & 0x03);

	pScreenBase = (unsigned char *)malloc((ScreenPitch * ScreenHeight));

	BytesPerPixel = ( BPP + 7 ) >> 3;

	{
		int CPCXOffset, CPCYOffset;
		int CPCScreenWidth, CPCScreenHeight;
		int ActualCPCScreenHeight;


		CPCXOffset = X_CRTC_CHAR_OFFSET << ( 1 + 3 );
		CPCYOffset = Y_CRTC_LINE_OFFSET;
		CPCScreenWidth = X_CRTC_CHAR_WIDTH_DEFAULT << ( 1 + 3 );
		CPCScreenHeight = Y_CRTC_LINE_HEIGHT_DEFAULT;

		if ( Render_GetFullDisplay() )
		{
			CPCXOffset = 0;
			CPCYOffset = 0;
			CPCScreenWidth = ScreenWidth;
			CPCScreenHeight = ScreenHeight;
		}

		CPCScreenWidth = CPCScreenWidth >> PIXEL_STEP_SHIFT;
		CPCXOffset = CPCXOffset >> PIXEL_STEP_SHIFT;

		if ( !Render_GetFullDisplay() )
		{
			if ( ScreenResX >= CPCScreenWidth )
			{
				/* screen resolution is higher than viewable CPC screen */

				/* centralise screen in display area */
				Render_ScreenXOffset = ( ScreenResX >> 1 ) - ( CPCScreenWidth >> 1 );
				Render_CPCRenderWidth = CPCScreenWidth;
				Render_CPCXOffset = CPCXOffset;
			}
			else
			{
				/* screen resolution is lower than viewable CPC screen */
				/* clip screen display */

				/* render from left hand side */
				Render_ScreenXOffset = 0;
				/* adjust width we are going to render */
				Render_CPCRenderWidth = ScreenResX;
				/* set new offset, so that the viewable area is still central */
				Render_CPCXOffset = CPCXOffset + ( CPCScreenWidth >> 1 ) - ( ScreenResX >> 1 );
			}

			if ( ScanLines != 0 )
			{
				ActualCPCScreenHeight = CPCScreenHeight << 1;
			}
			else
			{
				ActualCPCScreenHeight = CPCScreenHeight;
			}

			if ( ScreenResY >= ActualCPCScreenHeight )
			{
				/* screen Y resolution is higher than viewable CPC screen */
				/* centralise screen in display area */

				Render_ScreenYOffset = ( ScreenResY >> 1 ) - ( ActualCPCScreenHeight >> 1 );
				Render_CPCRenderHeight = ActualCPCScreenHeight;
				Render_CPCYOffset = CPCYOffset;
			}
			else
			{
				/* screen Y resolution is less than viewable CPC screen */
				/* clip screen display */

				/* render from top */
				Render_ScreenYOffset = 0;
				/* adjust height we are going to render */
				Render_CPCRenderHeight = ScreenResY;

				if ( ScanLines )
				{
					Render_CPCRenderHeight = Render_CPCRenderHeight >> 1;
				}

				/* set new offset, so that viewablew area is still central */
				Render_CPCYOffset = CPCYOffset + ( ( ActualCPCScreenHeight - ScreenResY ) >> 1 );
			}
		}
	}

	if (ScanLines)
	{
		Render_CPCRenderHeight = Render_CPCRenderHeight >> 1;
	}

	if (pScreenBase!=NULL)
	{
		memset(pScreenBase, 0, ScreenPitch*ScreenHeight);
		Renderer_Active = TRUE;
		Render_FirstLine();
	}
	else
	{
		Renderer_Active = FALSE;
	}
	pScreenLine = pScreenBase;


	return Renderer_Active;
}

BOOL	Render_IsRenderActive(void)
{
	return Renderer_Active;
}

#if 0
BOOL Render_SetDisplayFullScreen(int Width, int Height, int Depth)
{

	/* not recommended for any res as low as 320x240 */

	int ScreenResX = Width;
	int ScreenResY = Height;
	int ScreenDepth = Depth;

	Render_SetRenderingAccuracy(RENDERING_ACCURACY_HIGH);

	if (Host_SetDisplay(DISPLAY_TYPE_FULLSCREEN,ScreenResX,ScreenResY,ScreenDepth))
	{
		struct GRAPHICS_BUFFER_COLOUR_FORMAT   *pGraphicsBufferColourFormat = Host_GetGraphicsBufferColourFormat();

		CurrentGraphicsFormat.BPP = pGraphicsBufferColourFormat->BPP;

		CurrentGraphicsFormat.Red.BPP = pGraphicsBufferColourFormat->Red.BPP;
		CurrentGraphicsFormat.Red.Mask = pGraphicsBufferColourFormat->Red.Mask;
		CurrentGraphicsFormat.Red.Shift = pGraphicsBufferColourFormat->Red.Shift;

		CurrentGraphicsFormat.Green.BPP = pGraphicsBufferColourFormat->Green.BPP;
		CurrentGraphicsFormat.Green.Mask = pGraphicsBufferColourFormat->Green.Mask;
		CurrentGraphicsFormat.Green.Shift = pGraphicsBufferColourFormat->Green.Shift;

		CurrentGraphicsFormat.Blue.BPP = pGraphicsBufferColourFormat->Blue.BPP;
		CurrentGraphicsFormat.Blue.Mask = pGraphicsBufferColourFormat->Blue.Mask;
		CurrentGraphicsFormat.Blue.Shift = pGraphicsBufferColourFormat->Blue.Shift;

		if (CurrentGraphicsFormat.BPP!=8)
		{
			Render_TrueColourRGB_Setup();
		}
		else
		{
			Render_Paletted_Setup();
		}

		pRender_DumpScreen = Render_DumpScreen4;

		if (InitialiseRender(ScreenResX,ScreenResY,ScreenDepth))
		{
			return TRUE;
		}

		return FALSE;
	}

	return FALSE;
}
#endif

void Render_GetWindowedDisplayDimensions(int *pScreenWidth, int *pScreenHeight)
{
	int ScreenWidthUsed, ScreenHeightUsed;

	PIXEL_STEP=1;
	PIXEL_STEP_SHIFT=0;

	/*Render_SetRenderingAccuracy(Render_RenderingAccuracyForWindowedMode); */

	if (Render_GetFullDisplay())
	{
		ScreenWidthUsed = (X_CRTC_CHAR_WIDTH_FULL<<(1+3))>>PIXEL_STEP_SHIFT;
		ScreenHeightUsed = (Y_CRTC_LINE_HEIGHT_FULL);

	}
	else
	{
		ScreenWidthUsed = (MonitorWidth<<(1+3))>>PIXEL_STEP_SHIFT;
		ScreenHeightUsed = (MonitorHeight);
	}

	/* adjust height for scanlines */
	if (ScanLines)
	{
		ScreenHeightUsed = ScreenHeightUsed <<1;
	}

	*pScreenWidth = ScreenWidthUsed;
	*pScreenHeight = ScreenHeightUsed;
}

void Render_SetGraphicsBufferColourFormat(struct GRAPHICS_BUFFER_COLOUR_FORMAT *pGraphicsBufferColourFormat, int ScrWidth, int ScrHeight)
{
	//printf("Set graphics buffer\n");
	CurrentGraphicsFormat.BPP = pGraphicsBufferColourFormat->BPP;

	CurrentGraphicsFormat.Red.BPP = pGraphicsBufferColourFormat->Red.BPP;
	CurrentGraphicsFormat.Red.Mask = pGraphicsBufferColourFormat->Red.Mask;
	CurrentGraphicsFormat.Red.Shift = pGraphicsBufferColourFormat->Red.Shift;

	CurrentGraphicsFormat.Green.BPP = pGraphicsBufferColourFormat->Green.BPP;
	CurrentGraphicsFormat.Green.Mask = pGraphicsBufferColourFormat->Green.Mask;
	CurrentGraphicsFormat.Green.Shift = pGraphicsBufferColourFormat->Green.Shift;

	CurrentGraphicsFormat.Blue.BPP = pGraphicsBufferColourFormat->Blue.BPP;
	CurrentGraphicsFormat.Blue.Mask = pGraphicsBufferColourFormat->Blue.Mask;
	CurrentGraphicsFormat.Blue.Shift = pGraphicsBufferColourFormat->Blue.Shift;

	if (InitialiseRender(ScrWidth, ScrHeight, CurrentGraphicsFormat.BPP))
	{
		//printf("Successfully setup CPC render\n");
		
	pRender_DumpScreen = Render_DumpScreen4;
		
	if (CurrentGraphicsFormat.BPP!=8)
	{
		//printf("Setup truecolour render\n");
		Render_TrueColourRGB_Setup();
	}
	else
	{
		Render_Paletted_Setup();
	}
		
		return;
	}

	return;
}


void    Render_GetPixelRGBAtXY(int X,int Y, unsigned char *r, unsigned char *g, unsigned char *b)
{
	int     PixelBytes = (CurrentGraphicsFormat.BPP+7)>>3;
	unsigned long PackedPixelData;

	unsigned char *pScreenData = (unsigned char *)pScreenBase + ((Y+Y_CRTC_LINE_OFFSET)*ScreenPitch) + (((X_CRTC_CHAR_OFFSET<<(1+3))>>PIXEL_STEP_SHIFT)*PixelBytes) + (X*PixelBytes);
	unsigned long localr, localg, localb;

	PackedPixelData = ReadPackedImageData(pScreenData, PixelBytes);

	GetRGBFromSourceImageFormat(PackedPixelData, &localr, &localg, &localb, &CurrentGraphicsFormat);

	*r = (unsigned char)localr;
	*g = (unsigned char)localg;
	*b = (unsigned char)localb;
}


static int RenderLine=0;

void    Render_FirstLine(void)
{
	RenderLine = 0;

	pScreenLine = pScreenBase;
	pScreenLineDEBUG = pScreenLine;
}
/*
void    Render_SetLineStart(void)
{
        pScreenLine = pScreenBase + (RenderLine*ScreenPitch);
}
*/
void    Render_NextLine(void)
{
	RenderLine++;

	if (RenderLine>=ScreenHeight)
	{
		RenderLine = 0;
	}

	pScreenLine = pScreenBase + (RenderLine*ScreenPitch);
	pScreenLineDEBUG = pScreenLine;

}

/* ---------------------------------------------------------- */
#if 0
/* Put a pixel to the screen - used by Plus Sprite rendering routines */
static void    Render_PutPixel(int X, int Y, int Pixel)
{
	int     ScreenOffset = (Y*ScreenPitch) + (X>>PIXEL_STEP_SHIFT)*BytesPerPixel;
	unsigned char *pScreen = pScreenBase + ScreenOffset;


	Render_TrueColourRGB_WriteColourToScreenBuffer(pScreen, Pixel);
}
#endif

#if 0
extern char *FontGraphics;

void    PlotPixel(int X, int Y)
{
	int             Offset = (ScreenPitch * (Y+Render_CPCYOffset)) + (Render_CPCXOffset * BytesPerPixel) + (X * BytesPerPixel);
	unsigned char *pScreen = pScreenBase + Offset;

	unsigned long Pixel = 0x0fffffff;

	((unsigned long *)pScreen)[0] = Pixel;


}


/* plot a char on the display */
void    PlotChar(char ch, int CharX, int CharY)
{
	int PixelX = CharX<<3, PixelY = CharY<<3;
	char *pCharGfx = &FontGraphics[(ch<<3)];
	int y;

	for (y=0; y<8; y++)
	{
		int bit;
		char bits;

		bits = pCharGfx[y];

		for (bit=0; bit<8; bit++)
		{
			int PenIndex;

			if (bits & 0x080)
			{
				PenIndex = 2;
			}
			else
			{
				PenIndex = 0;
			}

			bits = bits<<1;


			PlotPixel(PixelX + bit, PixelY + y);

		}
	}
}


/* Plot a text string to graphics display */
void    Render_PlotText(char *pString, int X, int Y)
{
	int ScreenResX = 800, ScreenResY = 600;
	char *pStringPtr = pString;
	int XCharCoord, YCharCoord;

	XCharCoord = X;
	YCharCoord = Y;

	do
	{
		char ch;

		/* get char */
		ch = pStringPtr[0];

		pStringPtr++;

		/* if char is 0, then end of string reached, and quit */
		if (ch==0)
		{
			return;
		}

		/* Plot this character */
		PlotChar(ch, XCharCoord, YCharCoord);

		XCharCoord++;

		/* update char position */
		if (XCharCoord>=(ScreenResX>>3))
		{
			YCharCoord++;
			XCharCoord = 0;

			if (YCharCoord>=(ScreenResY>>3))
			{
				YCharCoord = 0;
			}
		}

	}
	while (1==1);
}
#endif

BOOL	Render_IsRendererActive(void)
{
	return Renderer_Active;
}
