/***************************************************************
 * Name:      zzzzzMain.h
 * Purpose:   Defines Application Frame
 * Author:    Kevin Thacker (kev@arnoldemu.freeserve.co.uk)
 * Created:   2011-06-25
 * Copyright: Kevin Thacker (arnold.cpc-live.com)
 * License:
 **************************************************************/

#ifndef ZZZZZMAIN_H
#define ZZZZZMAIN_H



#include "zzzzzApp.h"



#include "GUIDialog.h"

class zzzzzDialog: public GUIDialog
{
    public:
        zzzzzDialog(wxDialog *dlg);
        ~zzzzzDialog();
    private:
        virtual void OnClose(wxCloseEvent& event);
        virtual void OnQuit(wxCommandEvent& event);
        virtual void OnAbout(wxCommandEvent& event);
};
#endif // ZZZZZMAIN_H
