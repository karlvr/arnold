/***************************************************************
 * Name:      zzzzzMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Kevin Thacker (kev@arnoldemu.freeserve.co.uk)
 * Created:   2011-06-25
 * Copyright: Kevin Thacker (arnold.cpc-live.com)
 * License:
 **************************************************************/

#ifdef WX_PRECOMP
#include "wx_pch.h"
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "zzzzzMain.h"

//helper functions
enum wxbuildinfoformat {
    short_f, long_f };

wxString wxbuildinfo(wxbuildinfoformat format)
{
    wxString wxbuild(wxVERSION_STRING);

    if (format == long_f )
    {
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__WXMAC__)
        wxbuild << _T("-Mac");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
        wxbuild << _T("-Unicode build");
#else
        wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
    }

    return wxbuild;
}



zzzzzDialog::zzzzzDialog(wxDialog *dlg)
    : GUIDialog(dlg)
{
}

zzzzzDialog::~zzzzzDialog()
{
}

void zzzzzDialog::OnClose(wxCloseEvent &event)
{
    Destroy();
}

void zzzzzDialog::OnQuit(wxCommandEvent &event)
{
    Destroy();
}

void zzzzzDialog::OnAbout(wxCommandEvent &event)
{

}
