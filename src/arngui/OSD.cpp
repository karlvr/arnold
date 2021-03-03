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
#ifdef USE_SDL2

#ifdef USE_OSD

#include <stdio.h>
//#include <tchar.h>
#ifdef _MSC_VER
// both sdl and sdl2
#include <SDL_image.h>
#else
// choose sdl or sdl2 directory
#ifdef USE_SDL
#include <SDL/SDL_image.h>
#endif
#ifdef USE_SDL2
#include <SDL2/SDL_image.h>
#endif
#endif

#include "OSD.h"

#include <wx/filename.h>
#include <wx/regex.h>

#include "arnguiApp.h"

void DisplayMessage(wxString t);

extern "C"
{
#include "../cpc/joystick.h"
}

//menu
static char *MenuText[]=
{
	"Resume",
	"Change disc",
	"Reset",
	"Take screenshot",
	"Send Key :  RTN  SPC  ESC  ",
	"Save snapshoot :  0  1  2  3",
	"Load snapshoot :  0  1  2  3",
	"Restart in 6128 Normal",
	"Exit emulator",
	NULL
};

//Regex for disk detection
//wxRegEx reDisk1 = "(\(disk ([0-9]) of ([0-9])\)+)";
wxRegEx reDisk1 = "(\(disk ([0-9]) of ([0-9])\))";
// can use http://www.regexr.com/ for test

#define NBREMENU 9
#define MENUHEIGHT 28
#define MAXSNAP 3
#define CROCH(s,b) (b?"<"#s">":#s)

const char *SimKey[] = {"ESC","SPC","A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z","0","1","2","3","4","5","6","7","8","9","F1","F2","F3","F4","F5","F6","F7","F8","F9","RTN","ESC","SPC","A","B"};
const int IdAmstradKey[] = {66,47,69,54,62,61,58,53,52,44,35,45,37,36,38,46,34,27,67,50,60,51,42,55,59,63,43,71,32,64,65,57,56,49,48,41,40,33,13,14,5,20,12,4,10,11,3,18,66,47,69,54 };

//SDL_Surface* surfacePolice(TTF_Font* police, wxString text, SDL_Color couleur);

//***************************************************************************************

//Constructor
OSDDisplay::OSDDisplay() {

	m_currentIndex = 0;
	m_selKey = 3;
	m_selSnap = 0;
	m_selDisk = 1;

	b_DisplayMenu = false;

}

//Desctructor
OSDDisplay::~OSDDisplay() {
	if (snap) SDL_DestroyTexture(snap);
}


void OSDDisplay::Init(SDL_Renderer* Render,SDL_Window *Window)
{
	//load font for message
	wxString path = wxGetApp().GetAppPath() + wxT("font\\MyFont16.bmp");
	if (!font_message.LoadFont( (std::string)path.mb_str() ,Render))
	{
		printf("Failed to load Bitmap Font\n");
		return;
	}

	//load font for menu
	path = wxGetApp().GetAppPath() + wxT("font\\MyFont16.bmp");
	if (!font_menu.LoadFont( (std::string)path.mb_str() ,Render))
	{
		printf("Failed to load Bitmap Font\n");
		return;
	}

	//set Base size
	int w,h;
	SDL_GetWindowSize(Window,&w,&h);
	UpdateDimension(w,h);

	//Menu initialisation
	font_menu.SetColor(0,255,0);
	//setMenuPosition(25, 14);
	InitMenuPosition(w,h);


	//Message initialisation
	font_message.SetColor(255,0,0);
	setMessagePosition(5, 14);
	//font_message.Resize(80);
	//font_message.SetXPadding(2);
	font_message.SetYPadding(1);

}

void OSDDisplay::UpdateDimension(int w,int h)
{
	font_message.MemBaseScreen(w,h);
	font_menu.MemBaseScreen(w,h);
	width = w;
	height = h;
}

//New message information to display
void OSDDisplay::SetInfoMessage(wxString text)
{
	message=text;
	delay = 500;
}


void OSDDisplay::InitMenuPosition(int w,int h) {

	int maxw = 0;

	//check longer link
	for(int i = 0; i < NBREMENU; i++) {
		if (font_menu.CalcSize(MenuText[i]) > maxw) maxw = font_menu.CalcSize(MenuText[i]);
	}

	setMenuPosition((w - maxw) / 2 , (h - ( NBREMENU *MENUHEIGHT) ) /2 );

	//memorise menu size
	Rect_Menu.w = maxw * 1.1f;
	Rect_Menu.h = (NBREMENU * MENUHEIGHT) * 1.1f;
	Rect_Menu.x = (w - Rect_Menu.w) / 2 ;
	Rect_Menu.y = (h - Rect_Menu.h ) /2 ;

}

void OSDDisplay::InitMessagePosition(int w,int h) {

	int maxw = 0;

	//check longer link
	for(int i = 0; i < NBREMENU; i++) {
		if (font_menu.CalcSize(MenuText[i]) > maxw) maxw = font_menu.CalcSize(MenuText[i]);
	}

	setMenuPosition((w - maxw) / 2 , (h - ( NBREMENU *MENUHEIGHT) ) /2 );
}

void OSDDisplay::OnDraw(SDL_Renderer* renderer,SDL_Surface *SurfaceText,int x,int y) {

	if (SurfaceText) {
		SDL_Rect position;
		position.x=x;
		position.y=y;
		position.w=SurfaceText->w;
		position.h=SurfaceText->h;
		SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, SurfaceText);
		SDL_RenderCopy(renderer, texture, NULL, &position);
	}

}

void OSDDisplay::GetMediaName(void)
{
	wxString path;
	//Get name file of loaded media
	for (unsigned int i=0; i!=wxGetApp().m_Media.GetCount(); i++)
	{
		Media *pMedia = wxGetApp().m_Media[i];
		if (pMedia->GetMediaInserted())
		{
			path = pMedia->GetCurrentPath();
			int pos = path.rfind("#");
			if (pos > 0)
			{
				path = path.Left(pos);
			}
			wxFileName File(path);
			m_FileName = File.GetName();
			break;

		}
	}

	m_FileName = "test";
}

void OSDDisplay::OnRender(SDL_Renderer* render)
{
	//update OSD Menu and avoid button keep pressed
	int j = JoystickResumePostion();
	if (j != lastjoyaction){ 
		JoyAction(j);
		lastjoyaction = j;
	}


	//display menu
	if (b_DisplayMenu)
	{
		wxString tmp;

		//load thumb
		Loadthumb(render);

		//first, diaplay a background in alpha blending black
		SDL_Rect r;
		r = Rect_Menu;
		font_menu.RectConvert(&r);
		SDL_SetRenderDrawBlendMode(render,SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(render, 0, 0, 0, 170);
		SDL_RenderFillRect(render,&r);
		SDL_SetRenderDrawColor(render, 150, 150, 150, 255);
		SDL_RenderDrawRect(render,&r);

		//now display menu
		SDL_SetRenderDrawColor(render, 150, 25, 100, 255);
		for (int i=0; i < NBREMENU; i++)
		{
			if (i == m_currentIndex) { font_menu.SetColor(68, 117, 229); }
			else { font_menu.SetColor(150, 150, 150); }
			tmp = MenuText[i];

			//Special menu link, to make rectangle under char
			SDL_Rect Rect;
			Rect.x = m_MenuPosX ;
			Rect.y = m_MenuPosY + ( i * MENUHEIGHT );
			Rect.h = MENUHEIGHT;
			Rect.w = MENUHEIGHT;
			font_menu.RectConvert(&Rect);

			//4 - Key simulation
			if (i == 4)
			{
				tmp = "Send Key : ";
				for(int t = m_selKey - 1; t <= m_selKey + 1; t++) {

					//tmp.append(" " + wxT(SimKey[t]) + " ");
					tmp = tmp + " " + SimKey[t] + " ";

					if (t == m_selKey - 1)
					{
						Rect.x+=font_menu.CalcSize(tmp.ToStdString());
					}
					if (t == m_selKey)
					{
						wxString tmp2;
						tmp2 = tmp2 + " " + SimKey[m_selKey] + " ";
						Rect.w = font_menu.CalcSize(tmp2.ToStdString());
						SDL_RenderFillRect(render,&Rect);
					}
				}

			}
			//5 - Snaps Save
			if (i == 5)
			{
				Rect.x += font_menu.CalcSize("Save snapshoot : ") + m_selSnap * (font_menu.CalcSize("1 1"));
				SDL_RenderFillRect(render,&Rect);
			}

			//1 - Chnage disk
			if (i == 1)
			{
				tmp = "Change disc (" + wxString::Format(wxT("%i"),m_CurDisk) + "/" + wxString::Format(wxT("%i"),m_MaxDisk) + ")";

				//display a little menu
				if (m_currentIndex == 1)
				{
					tmp = "Change disc : ( ";
					wxString list = " 1  2  3  4  5  6";
					Rect.x += font_menu.CalcSize(tmp.ToStdString()) + (m_selDisk - 1) * (font_menu.CalcSize("......"));
					SDL_RenderFillRect(render,&Rect);
					tmp += list.Left(m_MaxDisk * 3) + "/" + wxString::Format(wxT("%i"),m_MaxDisk) + " )";
				}
			}

			//6 - Snaps load
			if (i == 6)
			{
				Rect.x += font_menu.CalcSize("Save snapshoot : ") + m_selSnap * (font_menu.CalcSize("1 1"));
				SDL_RenderFillRect(render,&Rect);

				//display thumb in a corner
				if (m_currentIndex == 6)
				{
					SDL_Rect RectDest;
					RectDest.x=0;
					RectDest.y=0;
					RectDest.w=160;
					RectDest.h=100;
					SDL_RenderCopy(render,snap,NULL,&RectDest);
				}

			}
			//7 - System changement
			if ( i == 7)
			{
				if (!m_cpctype.Cmp("6128plus")) tmp = "Restart in 6128 Normal";
				else tmp = "Restart in 6128 Plus";
			}

			font_menu.show_text(m_MenuPosX  , m_MenuPosY + ( i * MENUHEIGHT ),tmp.ToStdString(),render);

		}
	}

	//display info message
	if (delay > 0)
	{
		font_message.show_text( m_MessPosX, m_MessPosY, message.ToStdString(),render );
		//delay--;
	}

}

//TODO : need to prevent the OSD menu can be use in windowed mode, may cause error in file management
void OSDDisplay::ToogleOSDMenu(bool force)
{
	/*
	if (!force)
	{
	TempoMenu +=1;
	if (TempoMenu < 7) return;
	TempoMenu = 0;
	}
	*/

	b_DisplayMenu = !(b_DisplayMenu);
	m_currentIndex = 0;
	//TempoMenu = 0;

	//initialise info for menu
	if (b_DisplayMenu)
	{
		//Get info about system
		m_cpctype = wxGetApp().CurrentConfig();

		//Get info about media
		GetMediaName();
		m_MaxDisk = 1;
		m_CurDisk = 1;

		if ( reDisk1.Matches(m_FileName) )
		{
			wxString tmp = reDisk1.GetMatch(m_FileName,2);
			m_CurDisk = wxAtoi(tmp);
			tmp = reDisk1.GetMatch(m_FileName,3);
			m_MaxDisk = wxAtoi(tmp);
		}

		// to avoid error
		if (m_MaxDisk > 4) m_MaxDisk = 4;
		if (m_CurDisk < 1) m_CurDisk = 1;
		if (m_CurDisk > m_MaxDisk) m_CurDisk = 1;

	}
}

void OSDDisplay::Loadthumb(SDL_Renderer* render)
{
	wxString file;
	file = wxGetApp().GetAppPath() + wxT("Snapshot\\") + m_FileName + wxString::Format(wxT("%i"),m_selSnap) + ".png";

	//if already loaded return
	if (!file.Cmp(SnapPath)) return;

	if (snap) SDL_DestroyTexture(snap);
	snap = NULL;

	SDL_Surface* loadedImage = NULL;
	loadedImage = IMG_Load( file.mb_str() );
	SnapPath = file;
	snap = SDL_CreateTextureFromSurface(render,loadedImage);
	SDL_FreeSurface(loadedImage);
}

void OSDDisplay::setMenuPosition(int X, int Y) {
	m_MenuPosX = X;
	m_MenuPosY = Y;
}

void OSDDisplay::setMessagePosition(int X, int Y) {
	m_MessPosX = X;
	m_MessPosY = Y;
}

void OSDDisplay::ExternCommand(int action,int data)
{
	wxString path;
	bool b = true;

	//to be sure the media have good name
	GetMediaName();

	switch (action)
	{
	case 0://resume
		ToogleOSDMenu();
		break;
	case 1://chnage disk
		if (m_CurDisk != m_selDisk)//disk have chnaged
		{
			if (reDisk1.Matches(m_FileName) )//ok for regex
			{
				//make file name chnagement
				wxString NewDisk;
				NewDisk = m_FileName;
				reDisk1.Replace(&NewDisk, wxString::Format(wxT("%i"),m_selDisk));
			}
		}
		break;
	case 2://reset
		ToogleOSDMenu();
		wxGetApp().RestartPower(false,false);
		break;
	case 3://save screenshot
		if (m_FileName.IsEmpty())
		{
			DisplayMessage("ERROR : No media found");
			break;
		}
		ToogleOSDMenu();
		path = wxGetApp().GetAppPath() + wxT("Screenshot\\") + m_FileName + ".bmp";
		//		b = wxGetApp().SaveScreenshot(path);
		if (b) DisplayMessage("Screenshot saved to :" + path);
		break;
	case 4://send key
		break;
	case 5://save snaps
		if (m_FileName.IsEmpty())
		{
			DisplayMessage("ERROR : No media found");
			break;
		}
		ToogleOSDMenu();
		path = wxGetApp().GetAppPath() + wxT("Snapshot\\") + m_FileName + wxString::Format(wxT("%i"),m_selSnap) + ".sna";
		b = wxGetApp().SaveSnapshot(path);
		if (b) DisplayMessage("Snapshot saved to :" + path);
		break;
	case 6://load snaps
		if (m_FileName.IsEmpty())
		{
			DisplayMessage("ERROR : No media found");
			break;
		}
		path = wxGetApp().GetAppPath() + wxT("Snapshot\\")+ m_FileName + wxString::Format(wxT("%i"),m_selSnap) + ".sna";
		b = wxGetApp().LoadSnapshot(path);
		if (!b)
		{
			DisplayMessage("ERROR : Snapshot not found");
			break;
		}
		ToogleOSDMenu();
		DisplayMessage("Snapshot successful loaded");
		break;
	case 7://restart in other mode
		ToogleOSDMenu();
		if (!m_cpctype.Cmp("6128plus")) wxGetApp().ChooseConfig("cpc6128en");
		else wxGetApp().ChooseConfig("6128plus");
		break;
	case 8://exit all
		wxGetApp().Exit();//<< crash
		break;
	}
}

void OSDDisplay::JoyAction(int action) {
	switch (action)
	{
	case OSD_UP:
		m_currentIndex--;
		if (m_currentIndex < 0) m_currentIndex = NBREMENU - 1;
		break;
	case OSD_DOWN:
		m_currentIndex++;
		if (m_currentIndex >= NBREMENU) m_currentIndex = 0;
		break;
	case OSD_LEFT:
		if ((m_currentIndex == 5) || (m_currentIndex == 6))
		{
			m_selSnap++;
			if (m_selSnap > MAXSNAP) m_selSnap = 0;
		}
		if (m_currentIndex == 4)
		{
			m_selKey++;
			if (m_selKey > 49) m_selKey = 2;
		}
		if (m_currentIndex == 1)
		{
			m_selDisk++;
			if (m_selDisk > m_MaxDisk) m_selDisk = m_MaxDisk;
		}
		break;
	case OSD_RIGHT:
		if ((m_currentIndex == 5) || (m_currentIndex == 6))
		{
			m_selSnap--;
			if (m_selSnap < 0) m_selSnap = MAXSNAP;
		}
		if (m_currentIndex == 4)
		{
			m_selKey--;
			if (m_selKey < 2) m_selKey = 49;
		}
		if (m_currentIndex == 1)
		{
			m_selDisk--;
			if (m_selDisk < 1) m_selDisk = 1;
		}
		break;
	case OSD_SELECT:
		switch (m_currentIndex)
		{
		case 4:
			ExternCommand(m_currentIndex,IdAmstradKey[m_selKey]);
			break;
		case 5:
			ExternCommand(m_currentIndex,m_selSnap);
			break;
		case 6:
			ExternCommand(m_currentIndex,m_selSnap);
			break;
		default:
			ExternCommand(m_currentIndex,0);
		}
		break;
	}
}

//*****************************************************************************************************
/*
// matchColorKeys: This copies the transparency data from one
//      surface to another.
void matchColorKeys( SDL_Surface* src, SDL_Surface* dest )
{
// If the original had an alpha color key, give it to the new one.
if( src->flags & SDL_SRCCOLORKEY )
{
// Acquire the original Key
Uint32 colorkey = src->format->colorkey;

// Set to the new image
SDL_SetColorKey( dest, SDL_SRCCOLORKEY, colorkey );
}
}

// resizeImage: Resizes an image at its current place in memory. This means
//      that if you skew and stretch, you'll lose quality, but it also
//      means no worrying about new pointers. This uses the zoomSurface functions
//      in SDL_rotozoom.h ( SDL_gfx package )
void resizeImage( SDL_Surface*& img, const double newwidth, const double newheight )
{
// Zoom function uses doubles for rates of scaling, rather than
// exact size values. This is how we get around that:
double zoomx = newwidth  / (float)img->w;
double zoomy = newheight / (float)img->h;

// This function assumes no smoothing, so that any colorkeys wont bleed.
SDL_Surface* sized = zoomSurface( img, zoomx, zoomy, SMOOTHING_OFF );

// Copy transparency data.
matchColorKeys( img, sized );

// The original picture is no longer needed.
SDL_FreeSurface( img );

// Set it instead to the new image.
img =  sized;
}
*/

#endif
#endif