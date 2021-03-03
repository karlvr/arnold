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
//#include <windows.h>
#ifdef _WIN32
#define WIN32_EXTRA_LEAN
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#if defined(__WXMSW__)
#include <tchar.h>
//#include "win.h"
//#include "directx/dd.h"
//#include "directx/graphlib.h"
//#include "directx/ds.h"
//#include <direct.h>
#endif
//#include "cpcemu.h"
#include "../cpc/host.h"
#include "../cpc/arnold.h"
#include "../cpc/fdd.h"
#include "../cpc/cpc.h"
#include "../cpc/autorunfile.h"
#include "../cpc/autotype.h"

//extern APP_DATA AppData;

#if 0
static GRAPHICS_BUFFER_INFO BufferInfo;
static struct GRAPHICS_BUFFER_COLOUR_FORMAT BufferColourFormat;
static SOUND_PLAYBACK_FORMAT SoundFormat;
static 	DDSURFACEDESC SurfaceDesc;

BOOL	Host_SetDisplay(int Type, int Width, int Height, int Depth)
{
	if (Type == DISPLAY_TYPE_WINDOWED)
	{
//		MyApp_SetWindowed(Width, Height);
	}
	else
	{
		//	MyApp_SetFullScreen(Width, Height);
	}

	return DD_SetVideoMode(Width, Height, Depth, (Type!=DISPLAY_TYPE_WINDOWED));
}


BOOL	Host_LockGraphicsBuffer(void);
GRAPHICS_BUFFER_INFO	*Host_GetGraphicsBufferInfo(void);
void	Host_UnlockGraphicsBuffer(void);
void	Host_SetPaletteEntry(int, unsigned char, unsigned char, unsigned char);
BOOL	Host_SetDisplay(int Type, int Width, int Height, int Depth);


struct GRAPHICS_BUFFER_COLOUR_FORMAT *Host_GetGraphicsBufferColourFormat()
{
//		DDSURFACEDESC SurfaceDesc;

	MODE_DETAILS ModeDetails;

	DD_ExamineMode(&ModeDetails);

//		BufferInfo.Height = SurfaceDesc.dwHeight;
//		BufferInfo.Width = SurfaceDesc.dwWidth;
//#ifdef _MSC_VER
//		BufferInfo.Pitch = SurfaceDesc.lPitch;
//#else
//		BufferInfo.Pitch = SurfaceDesc.u1.lPitch;
//#endif
//		BufferInfo.pSurface = SurfaceDesc.lpSurface;

	BufferColourFormat.BPP = ModeDetails.BPP;

	BufferColourFormat.Red.BPP = ModeDetails.RedBPP;
	BufferColourFormat.Red.Mask = ModeDetails.RedMask;
	BufferColourFormat.Red.Shift = ModeDetails.RedShift;

	BufferColourFormat.Green.BPP = ModeDetails.GreenBPP;
	BufferColourFormat.Green.Mask = ModeDetails.GreenMask;
	BufferColourFormat.Green.Shift = ModeDetails.GreenShift;

	BufferColourFormat.Blue.BPP = ModeDetails.BlueBPP;
	BufferColourFormat.Blue.Mask = ModeDetails.BlueMask;
	BufferColourFormat.Blue.Shift = ModeDetails.BlueShift;

	return &BufferColourFormat;
}

GRAPHICS_BUFFER_INFO *Host_GetGraphicsBufferInfo()
{
	return &BufferInfo;
}

BOOL	Host_LockGraphicsBuffer(void)
{
	BOOL State;

	State = DD_GetSurfacePtr(&SurfaceDesc);

	if (State)
	{
		MODE_DETAILS ModeDetails;

		DD_ExamineMode(&ModeDetails);

		BufferInfo.Height = SurfaceDesc.dwHeight;
		BufferInfo.Width = SurfaceDesc.dwWidth;
//#ifdef _MSC_VER
		BufferInfo.Pitch = SurfaceDesc.lPitch;
//#else
//		BufferInfo.Pitch = SurfaceDesc.u1.lPitch;
//#endif
		BufferInfo.pSurface = SurfaceDesc.lpSurface;

		BufferColourFormat.BPP = ModeDetails.BPP;

		BufferColourFormat.Red.BPP = ModeDetails.RedBPP;
		BufferColourFormat.Red.Mask = ModeDetails.RedMask;
		BufferColourFormat.Red.Shift = ModeDetails.RedShift;

		BufferColourFormat.Green.BPP = ModeDetails.GreenBPP;
		BufferColourFormat.Green.Mask = ModeDetails.GreenMask;
		BufferColourFormat.Green.Shift = ModeDetails.GreenShift;

		BufferColourFormat.Blue.BPP = ModeDetails.BlueBPP;
		BufferColourFormat.Blue.Mask = ModeDetails.BlueMask;
		BufferColourFormat.Blue.Shift = ModeDetails.BlueShift;



	}

	return State;
}


void	Host_UnlockGraphicsBuffer(void)
{
	DD_ReturnSurfacePtr(&SurfaceDesc);
}

void	Host_SwapGraphicsBuffers(void)
{
	DD_Flip();
}

void	Host_SetPaletteEntry(int Index, unsigned char R, unsigned char G, unsigned char B)
{
	DD_SetPaletteEntry(Index, R, G, B);
}

//void	Host_WriteDataToSoundBuffer(unsigned char *pData, unsigned long Length)
//{
//		DS_WriteBufferForSoundPlayback(pData,Length);
//}

BOOL	Host_AudioPlaybackPossible(void)
{
	return DS_AudioActive();
}

unsigned long	Host_GetCurrentTimeInMilliseconds(void)
{
	return 0;
//	return timeGetTime();
}


SOUND_PLAYBACK_FORMAT *Host_GetSoundPlaybackFormat(void)
{
	SoundFormat.NumberOfChannels = DS_GetSampleChannels();
	SoundFormat.BitsPerSample = DS_GetSampleBits();
	SoundFormat.Frequency = DS_GetSampleRate();

	return &SoundFormat;
}

BOOL	Host_ProcessSystemEvents(void)
{
	/* process system events. If QUIT has been selected, then break out of loop */
//	return WinApp_ProcessSystemEvents();

	return TRUE;
}


/*void	Host_SetDirectory(char *Directory)
{
	_tchdir(Directory);
}
*/

BOOL Host_LockAudioBuffer(unsigned char **ppBlock1, unsigned long *pBlock1Size, unsigned char **ppBlock2, unsigned long *pBlock2Size, int BlockSize)
{
	return DS_LockAudioBuffer(ppBlock1, pBlock1Size, ppBlock2, pBlock2Size, BlockSize);
}

void	Host_UnLockAudioBuffer(void)
{
	DS_UnLockAudioBuffer();
}

extern void AutoFileLoad_Update();
extern BOOL AutoFileLoad_IsActive();
extern BOOL bWin;
static unsigned long PreviousTime=0;
int Host_LockSpeed = FALSE;
unsigned long TimeError = 0;
extern BOOL DoNotScanKeyboard;

//extern APP_DATA AppData;
#if 0
void RenderMousePos()
{
	HBRUSH hBrush, hOldBrush;
	RECT Rect;
	POINT pt;
	COLORREF Colour;

	HDC hDC = DD_GetDC();

	pt.x = AppData.MousePosX;
	pt.y = AppData.MousePosY;

	Rect.left = pt.x-1;
	Rect.right = pt.x+1;
	Rect.top = pt.y-1;
	Rect.bottom = pt.y+1;

	Colour = RGB(0,0,255);

	hBrush = CreateSolidBrush(Colour);

	hOldBrush = SelectObject(hDC,hBrush);


	FillRect(hDC, &Rect, hBrush);

	SelectObject(hDC, hOldBrush);

	DeleteObject(hBrush);

//	ReleaseDC(AppData.ApplicationHwnd,hDC);
	DD_ReleaseDC(hDC);
}
#endif

void RenderDriveLED(RECT *pRect, BOOL bState)
{
#if 0
	HBRUSH hBrush, hOldBrush;
//	HDC hDC = GetDC(AppData.ApplicationHwnd);

	HDC hDC = DD_GetDC();


	COLORREF Colour;
	if (bState)
	{
		Colour = RGB(255,0,0);
	}
	else
	{
		Colour = RGB(128,0,0);
	}

	hBrush = CreateSolidBrush(Colour);

	hOldBrush = SelectObject(hDC,hBrush);

	FillRect(hDC, pRect, hBrush);

	SelectObject(hDC, hOldBrush);

	DeleteObject(hBrush);

//	ReleaseDC(AppData.ApplicationHwnd,hDC);
	DD_ReleaseDC(hDC);
#endif
}

void Host_RenderLEDs()
{
	int i;
	RECT LEDRect;
	LEDRect.left = 8;
	for (i=0; i<MAX_DRIVES; i++)
	{

		if (FDD_IsEnabled(i))
		{
			LEDRect.right = LEDRect.left + 16;
			LEDRect.top = 8;
			LEDRect.bottom = LEDRect.top + 8;
			RenderDriveLED(&LEDRect,FDD_LED_GetState(i));
		}
		LEDRect.left += 24;
	}


}

//BOOL bFirstThrottle = TRUE;
//HANDLE hEvent;

BOOL Host_Throttle(void)
{
//	if (bFirstThrottle)
//	{
//		bFirstThrottle = FALSE;
//		hEvent = CreateEvent(NULL, TRUE, FALSE, "TESTER");
//	}

	if (Host_LockSpeed)
	{
#if 0
		unsigned long	TimeDifference;
		unsigned long	Time;
		Time = timeGetTime();

		if (PreviousTime!=0)
		{
			TimeDifference = (PreviousTime-Time);

			if (TimeDifference<=(1000/50))
			{
				unsigned long SleepTime = ((1000/50)-TimeDifference);
				unsigned long SleepPrevTime = Time;
				while (1==1)
				{
					HANDLE hHandles[1];
					hHandles[0] = hEvent;
					if (MsgWaitForMultipleObjects(1,hHandles, TRUE, SleepTime, QS_ALLEVENTS)==WAIT_TIMEOUT)
					{
						MSG Message;

						while (PeekMessage(&Message,NULL,0,0,PM_NOREMOVE))
						{
							// must be a message
							// yes, get the message
							if (GetMessage(&Message,NULL,0,0))
							{
								// if the message is not WM_QUIT
								// Translate it and dispatch it
								TranslateMessage(&Message);
								DispatchMessage(&Message);
							}
							else
							{
								// Message was WM_QUIT. So break out of message loop
								// and quit
								return;	// TRUE;
							}
						}

						break;
					}

					{
						MSG Message;

						while (PeekMessage(&Message,NULL,0,0,PM_NOREMOVE))
						{
							// must be a message
							// yes, get the message
							if (GetMessage(&Message,NULL,0,0))
							{
								// if the message is not WM_QUIT
								// Translate it and dispatch it
								TranslateMessage(&Message);
								DispatchMessage(&Message);
							}
							else
							{
								// Message was WM_QUIT. So break out of message loop
								// and quit
								return;	// TRUE;
							}
						}
						{
							unsigned long CurTime = timeGetTime();
							unsigned long SleptFor = CurTime-SleepPrevTime;
							SleepPrevTime = CurTime;
							if (SleptFor>SleepTime)
							{
								break;
							}
							SleepTime -= SleptFor;
						}
					}
				}
			}
			else
			{
				MSG Message;

				while (PeekMessage(&Message,NULL,0,0,PM_NOREMOVE))
				{
					// must be a message
					// yes, get the message
					if (GetMessage(&Message,NULL,0,0))
					{
						// if the message is not WM_QUIT
						// Translate it and dispatch it
						TranslateMessage(&Message);
						DispatchMessage(&Message);
					}
					else
					{
						// Message was WM_QUIT. So break out of message loop
						// and quit
						return;	// TRUE;
					}
				}


			}
		}
#endif
#if 0
		/* use this to throttle speed */
		unsigned long	TimeDifference;
		unsigned long	Time;

		do
		{

			MSG Message;

			/* get current time */
			Time = timeGetTime();

			while (PeekMessage(&Message,NULL,0,0,PM_NOREMOVE))
			{
				// must be a message
				// yes, get the message
				if (GetMessage(&Message,NULL,0,0))
				{
					// if the message is not WM_QUIT
					// Translate it and dispatch it
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}
				else
				{
					// Message was WM_QUIT. So break out of message loop
					// and quit
					return TRUE;
				}
			}

			/* calc time difference */
			TimeDifference = Time - (PreviousTime-TimeError);
		}
		while (TimeDifference<(1000/50));

		PreviousTime = Time;
	}
	else
	{
		MSG Message;

		while (PeekMessage(&Message,NULL,0,0,PM_NOREMOVE))
		{
			// must be a message
			// yes, get the message
			if (GetMessage(&Message,NULL,0,0))
			{
				// if the message is not WM_QUIT
				// Translate it and dispatch it
				TranslateMessage(&Message);
				DispatchMessage(&Message);
			}
			else
			{
				// Message was WM_QUIT. So break out of message loop
				// and quit
				return TRUE;
			}
		}
#endif
	}
	// No message, so idle, execute user function
//	if (!(AppData.DoNotScanKeyboard))
	{
		DI_ScanKeyboard();
	}

	if (bWin)
	{
		//	CPC_UpdateAudio();

		if (AutoRunFile_Active())
		{
			AutoRunFile_Update();
		}

		/* auto type active? */
		if (AutoType_Active())
		{
			/* update it */
			AutoType_Update();
		}
		else
		{

			CPC_ClearKeyboard();

			/* scan keyboard/joysticks */
			//	DoKeyboard();
		}
	}
	return FALSE;

}
#endif
#if 0
HOST_FILE_HANDLE	Host_OpenFile(char *Filename, int Access)
{
	HOST_FILE_HANDLE fh;

	if (Access == HOST_FILE_ACCESS_READ)
	{
		fh = (HOST_FILE_HANDLE)fopen(Filename,"rb");
	}
	else
	{
		fh = (HOST_FILE_HANDLE)fopen(Filename,"wb");
	}

	return fh;
}

void	Host_CloseFile(HOST_FILE_HANDLE Handle)
{
	if (Handle!=0)
	{
		fclose((FILE *)Handle);
	}
}

int	Host_GetFileSize(HOST_FILE_HANDLE Handle)
{
	if (Handle!=0)
	{
		int len;
		fseek((FILE *)Handle, 0, SEEK_END);
		len = ftell((FILE *)Handle);
		fseek((FILE *)Handle, 0, SEEK_CUR);
		return len;

		//int fno = fileno((FILE *)Handle);

		//return filelength(fno);
	}

	return 0;
}



void	Host_ReadData(HOST_FILE_HANDLE Handle, unsigned char *pData, unsigned long Size)
{
	if (Handle!=0)
	{
		size_t nSizeRead = fread(pData, Size, 1, (FILE *)Handle);
    if (nSizeRead!=Size)
    {
    }
    
    }
}

void	Host_WriteData(HOST_FILE_HANDLE Handle, unsigned char *pData, unsigned long Size)
{
	if (Handle!=0)
	{
		size_t nSizeWritten = fwrite(pData, Size, 1, (FILE *)Handle);
    if (nSizeWritten!=Size)
    {
    }
    }
}

#endif

