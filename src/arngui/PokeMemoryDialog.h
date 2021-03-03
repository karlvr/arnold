#ifndef __POKE_MEMORY_DIALOG_HEADER_INCLUDED__
#define __POKE_MEMORY_DIALOG_HEADER_INCLUDED__

#include <wx/dialog.h>
#include <wx/listctrl.h>

#ifndef wxOVERRIDE
#define wxOVERRIDE
#endif


class Winape_poke_DataBaseDialog : public wxDialog
{
public:
	Winape_poke_DataBaseDialog(wxWindow *);

	~Winape_poke_DataBaseDialog(void);
	Winape_poke_DataBaseDialog(void);

protected:
	void Apply(wxCommandEvent & WXUNUSED(event));
	void Check(wxCommandEvent & WXUNUSED(event));
	void OpenDB(wxCommandEvent & WXUNUSED(event));
	void OnGameSelected(wxListEvent& event);
	void OnPokeSelected(wxListEvent& event);
	void OnCheatSelected(wxListEvent& event);
	void OnChangetype(wxCommandEvent& event);
	void OnChangeEditText(wxCommandEvent& event);

	void RefreshGameList(void);
	virtual bool TransferDataToWindow(void) wxOVERRIDE;
	void UpdateDisplayBytes(void);
	void ClearEdit(void);

	wxString ConvertToByteList(unsigned short* pByteList, int NbreBt, int type);
	wxString ReFormatByteDisplay(int type, wxString s);

	int NbreByte;
	unsigned short *ByteList;

	//Memorisation item list
	long NGame;
	long NPoke;

	//database
	//WinapePokeDatabase *m_WinapePokeDatabase;


	DECLARE_EVENT_TABLE()
};

#endif
