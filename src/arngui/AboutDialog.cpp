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
#include "AboutDialog.h"
#include <wx/xrc/xmlres.h>
#include <wx/textctrl.h>
//#include "arnguiApp.h"

#ifdef USE_SDL

#ifdef _MSC_VER
#include <SDL.h>
#include <SDL_version.h>
#else
#include <SDL/SDL.h>
#include <SDL/SDL_version.h>
#endif

#endif


#ifdef USE_SDL2

#ifdef _MSC_VER
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

#endif


BEGIN_EVENT_TABLE(CAboutDialog, wxDialog)
END_EVENT_TABLE()

CAboutDialog::CAboutDialog(wxWindow *parent) : wxDialog()
{
	wxString sVersion;

	sVersion += wxT("Build Date: ");
	sVersion += wxT(__DATE__);
	sVersion += wxT("\n");
	sVersion += wxT("Build Time: ");
	sVersion += wxT(__TIME__);
	sVersion += wxT("\n");

#ifdef __WXDEBUG__
	sVersion += wxT("Build Type: Debug\n");
#endif

#if defined(USE_SDL) || defined(USE_SDL2)
	int CompiledMajor = 0;
	int CompiledMinor = 0;
	int CompiledPatch = 0;
	int LinkedMajor = 0;
	int LinkedMinor = 0;
	int LinkedPatch = 0;
#ifdef USE_SDL
	sVersion += wxT("Using SDL\n");

	CompiledMajor = SDL_MAJOR_VERSION;
	CompiledMinor = SDL_MINOR_VERSION;
	CompiledPatch = SDL_PATCHLEVEL;

	const SDL_version *version = SDL_Linked_Version();
	if (version)
	{
		LinkedMajor = version->major;
		LinkedMinor = version->minor;
		LinkedPatch = version->patch;
	}

#endif
#ifdef USE_SDL2
	sVersion += wxT("Using SDL2\n");

	SDL_version compiled;
	SDL_version linked;

	SDL_VERSION(&compiled);
	SDL_GetVersion(&linked);

	CompiledMajor = compiled.major;
	CompiledMinor = compiled.minor;
	CompiledPatch = compiled.patch;
	LinkedMajor = linked.major;
	LinkedMinor = linked.minor;
	LinkedPatch = linked.patch;
#endif

	sVersion += wxString(wxT("Compiled with: "));
	sVersion += wxString::Format(wxT("%i"), CompiledMajor);
	sVersion += wxT(".");
	sVersion += wxString::Format(wxT("%i"), CompiledMinor);
	sVersion += wxT(".");
	sVersion += wxString::Format(wxT("%i"), CompiledPatch);
	sVersion += wxT("\n");

	sVersion += wxString(wxT("Using with: "));
	sVersion += wxString::Format(wxT("%i"), LinkedMajor);
	sVersion += wxT(".");
	sVersion += wxString::Format(wxT("%i"), LinkedMinor);
	sVersion += wxT(".");
	sVersion += wxString::Format(wxT("%i"), LinkedPatch);
	sVersion += wxT("\n");
#endif	

	sVersion += wxString(wxT("wxWidgets version: "));
	sVersion += wxVERSION_STRING;
	sVersion += wxT("\n");

#if wxUSE_UNICODE
	sVersion += wxString(wxT("wxWidgets Unicode build\n"));
#else
	sVersion += wxString(wxT("wxWidgets ANSI build\n"));
#endif

#if defined(__WXMSW__)
	sVersion += wxString(wxT("wxWidgets for windows\n"));
#endif
#if defined(__WXGTK__)
	sVersion += wxString(wxT("wxWidgets for GTK2\n"));
#endif
#if defined(__WXMAC__)
	sVersion += wxString(wxT("wxWidgets for Mac OSX\n"));
#endif
#ifndef INKZ80
	sVersion += wxString(wxT("Using Arnold Z80 emulation\n"));
#else
	sVersion += wxString(wxT("Using Inkland Z80 emulation (www.inkland.org)\n"));
#endif

	// TODO automate this!
	sVersion += wxString(wxT("Fossil SCM checkout: 1b82f0c876ba1ac40ba84d089064f5c00e8b719e 2016-06-10 20:38:01 UTC"));

	wxXmlResource::Get()->LoadDialog(this, parent, wxT("DLG_DIALOG_ABOUT"));

	wxTextCtrl *pTextCtrl = XRCCTRL(*this, "m_textCtrl271", wxTextCtrl);
	if (pTextCtrl)
	{
		pTextCtrl->SetValue(sVersion);
	}
}
