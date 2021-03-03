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


#ifndef MENU_H_
#define MENU_H_

#ifdef USE_OSD

#include <stdlib.h>
#include <stdio.h>
#ifdef _MSC_VER
// both sdl and sdl2
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_touch.h>
#else
// choose sdl or sdl2 directory
#ifdef USE_SDL
#include <SDL/SDL_image.h>
//#include <SDL/SDL_ttf.h>
//#include <SDL/SDL_touch.h>
#endif
#ifdef USE_SDL2
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_touch.h>
#endif
#endif


#include "BitmapFont.h"

#include <wx/string.h>


struct menuElement {
	SDL_Surface* surfaceNormal;
	SDL_Surface* surfaceHover;
};

class OSDDisplay {

public:
	//Constructeur
	OSDDisplay();

	//Destructeur
	virtual ~OSDDisplay();

public:
	//Dispaly
	void OnRender(SDL_Renderer*);

	//Setting
	void setMenuPosition(int X, int Y);
	void setMessagePosition(int X, int Y);
	void InitMenuPosition(int, int);
	void InitMessagePosition(int, int);
	void ToogleOSDMenu(bool force = false);
	void Init(SDL_Renderer* Render, SDL_Window *Window);
	void UpdateDimension(int, int);

	//Mouvement/Action
	void JoyAction(int);

	//Set message info
	void SetInfoMessage(wxString text);

private:
	//********************
	//For info message
	//*******************
	wxString message;
	int delay;
	BitmapFont font_message;
	//SDL_Color m_colorMessage;
	//Message position
	int m_MessPosX;
	int m_MessPosY;

	//**************
	//for menu
	//**************
	int m_selKey;
	int m_selSnap;
	int m_selDisk;
	int TempoMenu;
	BitmapFont font_menu;
	//Wich one link in menu have focus ?
	int m_currentIndex;
	//Link is selected ?
	bool m_select;
	//Menu position
	int m_MenuPosX;
	int m_MenuPosY;
	//snaps thumb
	SDL_Texture *snap;
	wxString SnapPath;

	//*****************
	//info about media
	//*****************
	int m_MaxDisk;
	int m_CurDisk;
	wxString m_FileName;

	//*****************
	//info about configuration
	//****************
	wxString m_cpctype;

	//******************
	//general
	//******************
	//display
	void OnDraw(SDL_Renderer* render, SDL_Surface*, int x, int y);
	void ExternCommand(int action, int);
	void GetMediaName(void);
	void Loadthumb(SDL_Renderer* render);
	bool b_DisplayMenu;
	int width;
	int height;
	int lastjoyaction;
	SDL_Rect Rect_Menu;

};

#endif

#endif /* MENU_H_ */