/***************************************************************
 * Name:      psgMain.h
 * Purpose:   Defines Application Frame
 * Author:    Kevin Thacker (kev@arnoldemu.freeserve.co.uk)
 * Created:   2011-06-25
 * Copyright: Kevin Thacker (arnold.cpc-live.com)
 * License:
 **************************************************************/

#ifndef PSGMAIN_H
#define PSGMAIN_H



#include "psgApp.h"



#include "GUIDialog.h"

class psgDialog: public GUIDialog
{
    public:
        psgDialog(wxDialog *dlg);
        ~psgDialog();
    private:
        virtual void OnClose(wxCloseEvent& event);
        virtual void OnQuit(wxCommandEvent& event);
        virtual void OnAbout(wxCommandEvent& event);
};
#endif // PSGMAIN_H
