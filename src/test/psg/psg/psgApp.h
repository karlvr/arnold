/***************************************************************
 * Name:      psgApp.h
 * Purpose:   Defines Application Class
 * Author:    Kevin Thacker (kev@arnoldemu.freeserve.co.uk)
 * Created:   2011-06-25
 * Copyright: Kevin Thacker (arnold.cpc-live.com)
 * License:
 **************************************************************/

#ifndef PSGAPP_H
#define PSGAPP_H

#include <wx/app.h>

class psgApp : public wxApp
{
    public:
        virtual bool OnInit();
        virtual int OnExit();
};

#endif // PSGAPP_H
