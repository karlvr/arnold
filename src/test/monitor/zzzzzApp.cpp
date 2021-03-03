/***************************************************************
 * Name:      zzzzzApp.cpp
 * Purpose:   Code for Application Class
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

#include "zzzzzApp.h"
#include "zzzzzMain.h"

IMPLEMENT_APP(zzzzzApp);

bool zzzzzApp::OnInit()
{
    
    zzzzzDialog* dlg = new zzzzzDialog(0L);
    dlg->SetIcon(wxICON(aaaa)); // To Set App Icon
    dlg->Show();
    return true;
}
