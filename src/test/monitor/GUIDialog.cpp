///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  4 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif //WX_PRECOMP

#include "GUIDialog.h"

BEGIN_EVENT_TABLE(GUIDialog, wxDialog)
    EVT_PAINT(GUIDialog::OnPaint)
      // EVT_KEY_UP(GUIDialog::OnKeyUp)
       EVT_KEY_DOWN(GUIDialog::OnKeyUp)
END_EVENT_TABLE()

class Signal
{
public:
    int CountLow;
    int CountHigh;
    bool bState;
    int Count;
    int StoreCount;
    int nCharge;
    int StoredCharge;
    int CountHighAdd;
    int CountHighAddStore;
    bool StateStore;

public:
    void Set(int nCountLow, int nCountHigh)
    {
        Count = 0;
        bState = false;
         nCharge = 0;
        CountLow = nCountLow;
        CountHigh = nCountHigh;
        CountHighAdd = 0;

    }
    void Update()
    {
        Count++;
        if (bState)
        {
            if (Count==CountHigh)
            {
                bState = !bState;
                Count = 0;
                nCharge = 0;
                if (CountHighAdd<0)
                {
                    Count -= 1;
                }
                else if (CountHighAdd>0)
                {
                    Count += 1;
                }
                CountHighAdd = 0;
            }
        }
        else
        {
            nCharge++;
            if (Count==CountLow)
            {
                bState = !bState;
                Count = 0;
            }
        }

    }
};


#if 0

count++;
if (count=64)
    count = 0;

/* look for end of hsync */
if ((inprev^in) & (in==0))
{
    if (count<=32)
    {
        inc = 1;
    }
    else
    {
        inc = -1;
    }
}
#endif
Signal CPCSignal;
Signal MonitorSignal;


void DoUpdate()
{

    CPCSignal.Update();
    MonitorSignal.Update();

    /* hsync position changes monitor sync pos */
    /* hsync width changes monitor sync pos */
    /* monitor is working from end of cpc sync */

    if (CPCSignal.bState^MonitorSignal.bState)
    {
        if (CPCSignal.bState)
        {

            MonitorSignal.CountHighAdd--;
        }
        else
        {
            MonitorSignal.CountHighAdd++;
        }
    }
}

void GUIDialog::OnKeyUp(wxKeyEvent &event)
  {
    switch (event.GetUnicodeKey())
    {
    case ' ':
        {
                DoUpdate();
        }
        break;

    case 'Q':
    case 'q':
        {
            CPCSignal.CountLow--;
        }
        break;
    case 'A':
    case 'a':
        {
            CPCSignal.CountLow++;
        }
        break;
    case 'W':
    case 'w':
        {
            CPCSignal.CountHigh--;
        }
        break;
    case 'S':
    case 's':
        {
            CPCSignal.CountHigh++;
        }
        break;

    }

    event.Skip();
    Refresh();

}


void GUIDialog::OnPaint(wxPaintEvent &event)
{
    wxPaintDC  dc(this);

     int nWidth, nHeight;
    this->GetClientSize(&nWidth, &nHeight);

    int nUnits = 64;
    float nUnitWidth = nWidth/64.0f;

    int CPCY = 10;
    int MonitorY = 50;
    int XORY = 100;
    int MonitorYCharge = 250;

    CPCSignal.StateStore = CPCSignal.bState;
    CPCSignal.StoreCount = CPCSignal.Count;
    MonitorSignal.StateStore = MonitorSignal.bState;
    MonitorSignal.StoreCount = MonitorSignal.Count;
    MonitorSignal.StoredCharge = MonitorSignal.nCharge;
    MonitorSignal.CountHighAddStore = MonitorSignal.CountHighAdd;

   for (int i=0; i<nUnits; i++)
    {
        int YMod = 0;
        if (CPCSignal.bState)
        {
            YMod = -10;
        }
        dc.DrawLine((int)(i*nUnitWidth), CPCY+YMod, (int)((i+1)*nUnitWidth), CPCY+YMod);

        YMod = 0;
        if (MonitorSignal.bState)
        {
            YMod = -10;
        }
        dc.DrawLine((int)(i*nUnitWidth), MonitorY+YMod, (int)((i+1)*nUnitWidth), MonitorY+YMod);

        YMod = 0;
        if (CPCSignal.bState^MonitorSignal.bState)
        {
            YMod = -10;
        }
        dc.DrawLine((int)(i*nUnitWidth), XORY+YMod, (int)((i+1)*nUnitWidth), XORY+YMod);


        dc.DrawLine((int)(i*nUnitWidth), MonitorYCharge-(MonitorSignal.nCharge*1), (int)((i+1)*nUnitWidth), MonitorYCharge-(MonitorSignal.nCharge*1));
        DoUpdate();
    }
    MonitorSignal.CountHighAdd = MonitorSignal.CountHighAddStore;
    CPCSignal.Count = CPCSignal.StoreCount;
    MonitorSignal.Count = MonitorSignal.StoreCount;
    MonitorSignal.nCharge = MonitorSignal.StoredCharge;
   CPCSignal.bState = CPCSignal.StateStore;
    MonitorSignal.bState = MonitorSignal.StateStore;

    wxString sText;

    sText.Printf(_T("CPC Low: %d"), CPCSignal.CountLow);
    dc.DrawText(sText, 0,200);
    sText.Printf(_T("CPC High: %d"), CPCSignal.CountHigh);
    dc.DrawText(sText, 0,210);

    sText.Printf(_T("Monitor Low: %d"), MonitorSignal.CountLow);
    dc.DrawText(sText, 0,220);
    sText.Printf(_T("Monitor High: %d"), MonitorSignal.CountHigh);
    dc.DrawText(sText, 0,230);
    sText.Printf(_T("Monitor CountHighAdd: %d"), MonitorSignal.CountHighAdd);
    dc.DrawText(sText, 0,240);

}


///////////////////////////////////////////////////////////////////////////

GUIDialog::GUIDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
    MonitorSignal.Set(64-8, 8);
    CPCSignal.Set(64-8, 8);

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

//	wxBoxSizer* bSizer1;
	//bSizer1 = new wxBoxSizer( wxVERTICAL );


//	bSizer1->Add( m_customControl1, 1, wxALL|wxEXPAND, 5 );

//	this->SetSizer( bSizer1 );
	//this->Layout();

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GUIDialog::OnClose ) );
}

GUIDialog::~GUIDialog()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GUIDialog::OnClose ) );

}
