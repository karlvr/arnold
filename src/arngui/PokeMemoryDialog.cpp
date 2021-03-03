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
#include "PokeMemoryDialog.h"
#include <wx/xrc/xmlres.h>
#include <wx/listctrl.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>

#include "arnguiApp.h"
#include "winape_poke_database.h"
#include "IntClientData.h"
#include "EmuFileType.h"

extern "C"
{
#include "../cpc/cpc.h"
#include "../cpc/memrange.h"
	extern void WinapePokeDatabase_Free();
}

WinapePokeDatabase *m_WinapePokeDatabase = new WinapePokeDatabase;



BEGIN_EVENT_TABLE(Winape_poke_DataBaseDialog, wxDialog)
EVT_BUTTON(XRCID("m_validate"), Winape_poke_DataBaseDialog::Apply)
EVT_BUTTON(XRCID("m_check"), Winape_poke_DataBaseDialog::Check)
EVT_BUTTON(XRCID("ID_OPEN_DATABASE"), Winape_poke_DataBaseDialog::OpenDB)
EVT_LIST_ITEM_SELECTED(XRCID("IDC_LIST_CHEATS_GAME"), Winape_poke_DataBaseDialog::OnGameSelected)
EVT_LIST_ITEM_SELECTED(XRCID("IDC_LIST_CHEATS_LISTPOKE"), Winape_poke_DataBaseDialog::OnCheatSelected)
EVT_LIST_ITEM_SELECTED(XRCID("IDC_LIST_CHEATS_POKE"), Winape_poke_DataBaseDialog::OnPokeSelected)
EVT_CHOICE(XRCID("IDC_CHOICE_TYPE"), Winape_poke_DataBaseDialog::OnChangetype)
EVT_TEXT(XRCID("IDC_EDIT_ADDRESS"), Winape_poke_DataBaseDialog::OnChangeEditText)
END_EVENT_TABLE()


Winape_poke_DataBaseDialog::Winape_poke_DataBaseDialog(wxWindow *pParent)
{
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_DIALOG_CHEAT_DATABASE"));
}


int HexToDec(wxString src)
{
	int r = 0;
	long val;

	for (unsigned int i = 0; i < src.Length(); i += 1)
	{
		//if (!src.Mid(i,1).Cmp(' '))//skip if it s space
		//{
		if (!src.Mid(i, 1).ToLong(&val, 16)) return -1;//check if it s realy hexa
		r *= 16;
		r += (char)val;
		//}
	}

	return r;
}

Winape_poke_DataBaseDialog::~Winape_poke_DataBaseDialog(void)
{
}

Winape_poke_DataBaseDialog::Winape_poke_DataBaseDialog(void)
{

	NGame = wxNOT_FOUND;
	NPoke = wxNOT_FOUND;

	NbreByte = 0;
	ByteList = NULL;

	m_WinapePokeDatabase = NULL;
}

void Winape_poke_DataBaseDialog::OnChangetype(wxCommandEvent& WXUNUSED(event))
{
	UpdateDisplayBytes();
}

void Winape_poke_DataBaseDialog::OnChangeEditText(wxCommandEvent& WXUNUSED(event))
{
	//set bit count to zero for the application don't compare with database
	NbreByte = 0;
}


wxString Winape_poke_DataBaseDialog::ConvertToByteList(unsigned short* pByteList, int NbreBt, int type)
{
	wxString v;
	v.Clear();


	if (pByteList == NULL) return v;

	for (int j = 0; j < NbreBt; j++)
	{
		if (pByteList[j] == 0xffff)
		{
			v.append(wxT("?"));
		}
		else
		{
			switch (type)
			{
			case WINAPE_POKE_DATABASE_TYPE_DECIMAL:
				v.append(wxString::Format(wxT("%03i"), pByteList[j]));
				if (j < (NbreBt - 1)) v.append(wxT(" "));
				break;
			case WINAPE_POKE_DATABASE_TYPE_HEXADECIMAL:
				v.append(wxString::Format(wxT("%02X"), pByteList[j]));
				if (j < (NbreBt - 1)) v.append(wxT(" "));
				break;
			case WINAPE_POKE_DATABASE_TYPE_NUMERIC_ASCII:
				v.append(wxString::Format(wxT("%c"), pByteList[j]));
				break;
			case WINAPE_POKE_DATABASE_TYPE_BCD:
				v.append(wxString::Format(wxT("%02X"), pByteList[j]));
				if (j < (NbreBt - 1)) v.append(wxT(" "));
			case WINAPE_POKE_DATABASE_TYPE_NUMERIC:
				v.append(wxString::Format(wxT("%i"), pByteList[j]));
				break;

			default:
				v = wxT("Not defined");
				break;
			}

		}

	}

	return v;
}

void Winape_poke_DataBaseDialog::UpdateDisplayBytes(void)
{
	wxString v;

	//get display type values
	wxChoice *pChoice;
	pChoice = XRCCTRL(*this, "IDC_CHOICE_TYPE", wxChoice);
	int type = pChoice->GetSelection();

	//display values
	wxTextCtrl *pTextCtrl;
	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_VALUE", wxTextCtrl);

	//	v = pTextCtrl->GetValue();
	//	if (v.IsEmpty()) pTextCtrl->ChangeValue(ConvertToByteList(ByteList, NbreByte,type));

	pTextCtrl->ChangeValue(ConvertToByteList(ByteList, NbreByte, type));

}

void Winape_poke_DataBaseDialog::Check(wxCommandEvent & WXUNUSED(event))
{
	wxTextCtrl *pTextCtrl;
	wxString s;

	//get display type values
	wxChoice *pChoice;
	pChoice = XRCCTRL(*this, "IDC_CHOICE_TYPE", wxChoice);
	int type = pChoice->GetSelection();

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_VALUE", wxTextCtrl);
	s = pTextCtrl->GetValue();

	//Reformat string
	pTextCtrl->ChangeValue(ReFormatByteDisplay(type, s));
	s = pTextCtrl->GetValue();


	if (s.length() < 1)
	{
		wxMessageDialog dialog(NULL, wxT("Bad format for this type."), wxT(""), wxOK);
		dialog.ShowModal();
		return;
	}
}


void Winape_poke_DataBaseDialog::Apply(wxCommandEvent & WXUNUSED(event))
{
	wxTextCtrl *pTextCtrl;
	wxString s;

	unsigned short Address;
	unsigned short Value;

	//get address
	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_ADDRESS", wxTextCtrl);
	s = pTextCtrl->GetValue();
	Address = HexToDec(s);

	//get display type values
	wxChoice *pChoice;
	pChoice = XRCCTRL(*this, "IDC_CHOICE_TYPE", wxChoice);
	int type = pChoice->GetSelection();

	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_VALUE", wxTextCtrl);
	s = pTextCtrl->GetValue();

	//get if reversed
	wxCheckBox *pCheckBox;
	pCheckBox = XRCCTRL(*this, "m_reverse", wxCheckBox);
	bool reversed = (bool)pCheckBox->GetValue();

	//Reformat string
	pTextCtrl->ChangeValue(ReFormatByteDisplay(type, s));
	s = pTextCtrl->GetValue();

	//make inversion if reversed
	int n = s.Freq(32);
	if ((n == 1) && (reversed))
	{
		wxString Low;
		wxString Hight;
		switch (type)
		{
		case WINAPE_POKE_DATABASE_TYPE_DECIMAL:
			Low = s.Mid(0, 3);
			Hight = s.Mid(4, 3);
			break;
		case WINAPE_POKE_DATABASE_TYPE_HEXADECIMAL:
			Low = s.Mid(0, 2);
			Hight = s.Mid(3, 2);
			break;
		}
		s.clear();
		s.append(Hight);
		s.append(wxT(" "));
		s.append(Low);
	}

	if (s.length() < 1)
	{
		wxMessageDialog dialog(NULL, wxT("Bad format for this type."), wxT(""), wxOK);
		dialog.ShowModal();
		return;
	}


	unsigned int i = 0;
	unsigned char Byte = 0;
	wxString tmp;
	long lv;
	while (i < s.length())
	{
		//get char
		switch (type)
		{
		case WINAPE_POKE_DATABASE_TYPE_DECIMAL:
			tmp = s.Mid(i, 3);
			tmp.ToLong(&lv);
			Byte = (unsigned char)lv;
			i += 4;
			break;
		case WINAPE_POKE_DATABASE_TYPE_HEXADECIMAL:
			tmp = s.Mid(i, 2);
			lv = HexToDec(tmp);
			Byte = (unsigned char)lv;
			i += 3;
			break;
		case WINAPE_POKE_DATABASE_TYPE_NUMERIC_ASCII:
			Byte = s.GetChar(i);
			i += 1;
			break;
		case WINAPE_POKE_DATABASE_TYPE_BCD:
			tmp = s.Mid(i, 2);
			lv = HexToDec(tmp);
			Byte = (unsigned char)lv;
			i += 3;
			break;
		case WINAPE_POKE_DATABASE_TYPE_NUMERIC:
			Byte = s.GetChar(i);
			Byte -= 48;//to convert ascii to number
			i += 1;
			break;

			//default:

		}

		Value = Byte;

		if ((Address > 0) && (Value >= 0))//TODO:Need to check validity
		{
			// using Z80_WR_MEM is more work, may trigger breakpoints that kind of thing
			// using memory range applies it directly to appropiate memory range.
			//
			// default memory range is what the cpu sees (so current ram/rom paging for example)
			const MemoryRange *pRange = CPC_GetDefaultMemoryRange();
			MemoryRange_WriteByte(pRange, Address, Value);
		}

		//inc adress for next poke
		Address += 1;

	}

	this->Close();

}

wxString Winape_poke_DataBaseDialog::ReFormatByteDisplay(int type, wxString previous)
{
	wxString next;
	wxString tmp;

	next.Clear();

	char Byte;
	bool error;

	//special convertion for decimal type
	if (type == WINAPE_POKE_DATABASE_TYPE_DECIMAL)
	{

		long lv;

		wxString tmp2;
		tmp2 = previous;
		int deduction;
		int rest;
		int lentoremove;

		int pos = tmp2.length();

		previous.clear();

		while (!tmp2.IsEmpty())
		{
			//pos = tmp2.find(" ",true); << Broken in wxwidget 3.0
			pos = tmp2.rfind(wxT(" "));

			lentoremove = tmp2.length() - pos;

			tmp = tmp2.Mid(pos + 1, lentoremove - 1);
			tmp.ToLong(&lv);

			//check validity and convert (work up to 65792, not sure more will be usefull after)
			if (lv > 255)
			{
				deduction = lv / 256;
				rest = lv - (deduction * 256);
				if (rest > 255) rest = 255;

				tmp = wxString::Format(wxT("%03i"), deduction);
				previous.insert(0, wxString::Format(wxT("%03i"), rest));

			}

			previous.insert(0, tmp);

			if (pos == -1)
			{
				tmp2.clear();
			}
			else
			{
				previous.insert(0, wxT(" "));
				tmp2 = tmp2.Left(tmp2.length() - lentoremove);
			}

		}
	}


	//remove all spaces esxept for ASCII type
	if (type != WINAPE_POKE_DATABASE_TYPE_NUMERIC_ASCII)
	{
		for (unsigned int i = 0; i < previous.Length(); i++)
		{
			if (previous.GetChar(i) == 32) previous.Remove(i, 1);
		}
	}
	else
	{
		//special check for ascii type
		if (!previous.IsAscii())
		{
			wxMessageDialog dialog(NULL, wxT("There is a non ASCII value inside the chain."), wxT(""), wxOK);
			dialog.ShowModal();
			return wxEmptyString;
		}
	}

	//insert some '0' to have correct number bytes
	int rest;
	switch (type)
	{
	case WINAPE_POKE_DATABASE_TYPE_DECIMAL:
		rest = previous.Length() % 3;
		if (rest != 0) rest = 3 - rest;
		for (int k = 0; k < rest; k++) { previous.insert(0, wxT("0")); }
		break;

	case WINAPE_POKE_DATABASE_TYPE_BCD:
	case WINAPE_POKE_DATABASE_TYPE_HEXADECIMAL:
		rest = previous.Length() % 2;
		if (rest != 0) rest = 2 - rest;
		for (int k = 0; k < rest; k++) { previous.insert(0, wxT("0")); }
		break;
	}


	short len = 0;
	int TotalByte = 0;
	for (unsigned int i = 0; i < previous.Length(); i++)
	{
		//get char
		Byte = (char)previous.GetChar(i);

		//check it
		error = false;
		switch (type)
		{
		case WINAPE_POKE_DATABASE_TYPE_BCD:
		case WINAPE_POKE_DATABASE_TYPE_NUMERIC:
		case WINAPE_POKE_DATABASE_TYPE_DECIMAL:
			if (!isdigit(Byte)) error = true;
			break;
		case WINAPE_POKE_DATABASE_TYPE_HEXADECIMAL:
			if (!isxdigit(Byte)) error = true;
			break;
		case WINAPE_POKE_DATABASE_TYPE_NUMERIC_ASCII:
			if ((Byte < 32) || (Byte > 126)) error = true;//need to chnage theses values
			break;
		default:
			error = true;
		}

		//add char if no error
		if (!error)
		{
			tmp = wxString::Format(wxT("%c"), Byte);
			next.Append(tmp);
			len += 1;
		}
		else
		{
			wxMessageDialog dialog(NULL, wxT("Convertion not possible, Bad data."), wxT(""), wxOK);
			dialog.ShowModal();
			return wxEmptyString;
		}

		//byte counter
		switch (type)
		{
		case WINAPE_POKE_DATABASE_TYPE_NUMERIC_ASCII:
		case WINAPE_POKE_DATABASE_TYPE_NUMERIC:
			TotalByte += 1;
			break;
		case WINAPE_POKE_DATABASE_TYPE_BCD:
		case WINAPE_POKE_DATABASE_TYPE_HEXADECIMAL:
			if (len == 2) TotalByte += 1;
			break;
		case WINAPE_POKE_DATABASE_TYPE_DECIMAL:
			if (len == 3) TotalByte += 1;
			break;
		default:
			error = true;
		}

		//check to add space
		if (i < (previous.Length() - 1))
		{
			switch (type)
			{
			case WINAPE_POKE_DATABASE_TYPE_DECIMAL:
				if (len == 3)
				{
					next.Append(wxT(" "));
					len = 0;
				}
				break;

			case WINAPE_POKE_DATABASE_TYPE_BCD:
			case WINAPE_POKE_DATABASE_TYPE_HEXADECIMAL:
				if (len == 2)
				{
					next.Append(wxT(" "));
					len = 0;
				}
				break;
			}
		}

	}

	//Fill with 0 if not enought byte in chain regarding to database, normaly not used if user try its owns pokes
	if (TotalByte < NbreByte)
	{
		for (int k = 0; k < (NbreByte - TotalByte); k++) {
			switch (type)
			{
			case WINAPE_POKE_DATABASE_TYPE_NUMERIC_ASCII:
			case WINAPE_POKE_DATABASE_TYPE_NUMERIC:
				next.insert(0, wxT("0"));
				break;
			case WINAPE_POKE_DATABASE_TYPE_BCD:
			case WINAPE_POKE_DATABASE_TYPE_HEXADECIMAL:
				next.insert(0, wxT("00 "));
				break;
			case WINAPE_POKE_DATABASE_TYPE_DECIMAL:
				next.insert(0, wxT("000 "));
				break;
			}
		}
	}

	return next;
}

bool Winape_poke_DataBaseDialog::TransferDataToWindow()
{

	//need to set size, because XRC not working, please say me why ????
	//SetSize(400,850);

	unsigned char *pDiskImageData = NULL;
	unsigned long DiskImageLength = 0;

	//game List
	wxListCtrl *pListCtrl;
	pListCtrl = XRCCTRL(*this, "IDC_LIST_CHEATS_GAME", wxListCtrl);
	wxListItem Column;
	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT(""));
	pListCtrl->InsertColumn(0, Column);

	//try to load the database if exist memorised path
	wxString sFilename;

	//Get the saved path
	wxString sValue;
	wxConfig::Get(false)->Read(wxT("path/PokDatabase"), &sFilename, wxEmptyString);

	// try to load it
	if (wxGetApp().LoadLocalFile(sFilename, &pDiskImageData, &DiskImageLength))
	{
		//clear database
		WinapePokeDatabase_Free();
		m_WinapePokeDatabase = new WinapePokeDatabase;

		m_WinapePokeDatabase->Init((const char*)pDiskImageData, DiskImageLength);
		RefreshGameList();
	}


	//cheat list
	pListCtrl = XRCCTRL(*this, "IDC_LIST_CHEATS_LISTPOKE", wxListCtrl);
	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT("Description"));
	Column.SetWidth(150);
	pListCtrl->InsertColumn(0, Column);
	Column.SetText(wxT("Comment"));
	pListCtrl->InsertColumn(1, Column);



	//poke list
	pListCtrl = XRCCTRL(*this, "IDC_LIST_CHEATS_POKE", wxListCtrl);
	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT("Address"));
	pListCtrl->InsertColumn(0, Column);
	Column.SetText(wxT("Value"));
	Column.SetWidth(200);
	pListCtrl->InsertColumn(1, Column);


	//Combo list
	wxChoice *pChoice;
	IntClientData *pIntData;
	pChoice = XRCCTRL(*this, "IDC_CHOICE_TYPE", wxChoice);
	pIntData = new IntClientData(WINAPE_POKE_DATABASE_TYPE_DECIMAL);
	pChoice->Append(wxT("Decimal"), pIntData);

	pIntData = new IntClientData(WINAPE_POKE_DATABASE_TYPE_HEXADECIMAL);
	pChoice->Append(wxT("Hexa"), pIntData);

	pIntData = new IntClientData(WINAPE_POKE_DATABASE_TYPE_BCD);
	pChoice->Append(wxT("BCD"), pIntData);

	pIntData = new IntClientData(WINAPE_POKE_DATABASE_TYPE_NUMERIC);
	pChoice->Append(wxT("Numeric"), pIntData);

	pIntData = new IntClientData(WINAPE_POKE_DATABASE_TYPE_NUMERIC_ASCII);
	pChoice->Append(wxT("ASCII Numeric"), pIntData);

	pIntData = new IntClientData(WINAPE_POKE_DATABASE_TYPE_LONG);
	pChoice->Append(wxT("Long"), pIntData);

	pIntData = new IntClientData(WINAPE_POKE_DATABASE_TYPE_LONG);
	pChoice->Append(wxT("String"), pIntData);

	pChoice->Select(1);

	return true;
}




void Winape_poke_DataBaseDialog::OpenDB(wxCommandEvent & WXUNUSED(event))
{

	wxString sFilename;

	wxString sTitleSuffix;
	sTitleSuffix = wxT("POK file");

	wxString sFilter;
	sFilter = wxT("POK");

	EmuFileType m_FileType(wxT("Enter Poke database file"), wxT(""));
	FileFilter filter;
	filter.m_sExtension = wxT("POK");
	filter.m_sDescription = wxT("Winape Database");
	m_FileType.AddReadFilter(filter);


	if (m_FileType.Open(this, sFilename, sTitleSuffix))
	{
		unsigned char *pDiskImageData = NULL;
		unsigned long DiskImageLength = 0;

		// try to load it
		if (wxGetApp().LoadLocalFile(sFilename, &pDiskImageData, &DiskImageLength))
		{
			//Free database
			WinapePokeDatabase_Free();
			m_WinapePokeDatabase = new WinapePokeDatabase;

			m_WinapePokeDatabase->Init((const char*)pDiskImageData, DiskImageLength);
			RefreshGameList();

			//save new path
			wxConfig::Get(false)->Write(wxT("path/PokDatabase"), sFilename);

		}
	}



}

void Winape_poke_DataBaseDialog::OnPokeSelected(wxListEvent & WXUNUSED(event))
{
	wxListCtrl *pListCtrl;
	wxTextCtrl *pTextCtrl;

	//Get poke
	pListCtrl = XRCCTRL(*this, "IDC_LIST_CHEATS_POKE", wxListCtrl);
	long itemIndex = pListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	long NPokeSelected = wxNOT_FOUND;
	NPokeSelected = pListCtrl->GetItemData(itemIndex);

	if ((NPokeSelected != wxNOT_FOUND) && (NGame != wxNOT_FOUND))
	{
		//clear editbox
		ClearEdit();

		POKEGAMEINFO *pgi = new POKEGAMEINFO;
		m_WinapePokeDatabase->SetGame(NGame, pgi);

		unsigned short address = pgi->PokeInfoByGame[NPoke].PokeInfo[NPokeSelected].address;
		ByteList = (unsigned short *)(pgi->PokeInfoByGame[NPoke].PokeInfo[NPokeSelected].byte);
		NbreByte = pgi->PokeInfoByGame[NPoke].PokeInfo[NPokeSelected].NbrBytes;

		wxString v;

		//display adress
		pTextCtrl = XRCCTRL(*this, "IDC_EDIT_ADDRESS", wxTextCtrl);
		v = wxString::Format(wxT("%X"), address);
		pTextCtrl->ChangeValue(v);

		//diaply bytes
		UpdateDisplayBytes();

		delete(pgi);

	}


}

void Winape_poke_DataBaseDialog::OnCheatSelected(wxListEvent& WXUNUSED(event))
{
	wxListCtrl *pListCtrl;

	//Get poke list
	pListCtrl = XRCCTRL(*this, "IDC_LIST_CHEATS_LISTPOKE", wxListCtrl);
	long itemIndex = pListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	NPoke = wxNOT_FOUND;
	NPoke = pListCtrl->GetItemData(itemIndex);

	if ((NPoke != wxNOT_FOUND) && (NGame != wxNOT_FOUND))
	{
		POKEGAMEINFO *pgi = new POKEGAMEINFO;
		m_WinapePokeDatabase->SetGame(NGame, pgi);

		int nType = (int)pgi->PokeInfoByGame[NPoke].type;

		//display in listview
		pListCtrl = XRCCTRL(*this, "IDC_LIST_CHEATS_POKE", wxListCtrl);

		pListCtrl->Freeze();
		pListCtrl->DeleteAllItems();

		for (int i = 0; i < pgi->PokeInfoByGame[NPoke].npokes; i++)
		{

			wxListItem Item;
			wxString sText;

			Item.SetMask(wxLIST_MASK_TEXT);
			Item.SetId(i);
			sText = wxString::Format(wxT("%X"), pgi->PokeInfoByGame[NPoke].PokeInfo[i].address);
			Item.SetText(sText);
			Item.SetColumn(0);
			Item.SetImage(-1);
			Item.SetData(i);
			pListCtrl->InsertItem(Item);

			sText = ConvertToByteList((unsigned short *)(pgi->PokeInfoByGame[NPoke].PokeInfo[i].byte), pgi->PokeInfoByGame[NPoke].PokeInfo[i].NbrBytes, nType);
			if (sText.Length() > 0)
			{
				Item.SetText(sText);
				Item.SetColumn(1);
				pListCtrl->SetItem(Item);
			}


		}
		pListCtrl->Thaw();



		//display type
		wxChoice *pChoice;
		pChoice = XRCCTRL(*this, "IDC_CHOICE_TYPE", wxChoice);
		int nSelection = wxNOT_FOUND;

		for (unsigned int i = 0; i < pChoice->GetCount(); i++)
		{
			const IntClientData *pIntData = (const IntClientData *)pChoice->GetClientObject(i);
			if (pIntData->GetData() == nType)
			{
				nSelection = i;
				break;
			}
		}
		pChoice->SetSelection(nSelection);

		//check if reversed
		bool reversed = (int)pgi->PokeInfoByGame[NPoke].reversed;
		wxCheckBox *pCheckBox;
		pCheckBox = XRCCTRL(*this, "m_reverse", wxCheckBox);
		pCheckBox->SetValue(reversed ? true : false);

		delete(pgi);

	}

	//clear editbox
	ClearEdit();
}

void Winape_poke_DataBaseDialog::OnGameSelected(wxListEvent& WXUNUSED(event))
{

	wxListCtrl *pListCtrl;
	pListCtrl = XRCCTRL(*this, "IDC_LIST_CHEATS_GAME", wxListCtrl);

	NGame = -1;
	//need to use wxLC_SINGLE_SEL for this method
	long itemIndex = pListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	NGame = pListCtrl->GetItemData(itemIndex);

	if (NGame != -1)
	{
		POKEGAMEINFO *pgi = new POKEGAMEINFO;
		m_WinapePokeDatabase->SetGame(NGame, pgi);

		pListCtrl = XRCCTRL(*this, "IDC_LIST_CHEATS_LISTPOKE", wxListCtrl);

		pListCtrl->Freeze();
		pListCtrl->DeleteAllItems();

		for (int i = 0; i < pgi->NbrePoke; i++)
		{

			wxListItem Item;
			wxString sText;

			Item.SetMask(wxLIST_MASK_TEXT);
			Item.SetId(i);
			sText = wxString::FromUTF8(pgi->PokeInfoByGame[i].Desc);
			Item.SetText(sText);
			Item.SetColumn(0);
			Item.SetImage(-1);
			Item.SetData(i);
			pListCtrl->InsertItem(Item);

			//Item.SetMask(wxLIST_MASK_TEXT);
			//Item.SetId(i);
			sText = wxString::FromUTF8(pgi->PokeInfoByGame[i].Commment);
			if (sText.Length() > 0)
			{
				Item.SetText(sText);
				Item.SetColumn(1);
				//Item.SetImage(-1);
				//Item.SetData(i);
				pListCtrl->SetItem(Item);
			}


		}

		pListCtrl->Thaw();

		delete(pgi);

	}

	//clear poke list
	pListCtrl = XRCCTRL(*this, "IDC_LIST_CHEATS_POKE", wxListCtrl);
	pListCtrl->DeleteAllItems();

	//clear editbox
	ClearEdit();


}

void Winape_poke_DataBaseDialog::ClearEdit(void)
{
	wxTextCtrl *pTextCtrl;
	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_ADDRESS", wxTextCtrl);
	pTextCtrl->Clear();
	pTextCtrl = XRCCTRL(*this, "IDC_EDIT_VALUE", wxTextCtrl);
	pTextCtrl->Clear();

	NbreByte = 0;
}


void Winape_poke_DataBaseDialog::RefreshGameList(void)
{
	int i;

	wxString Name;

	wxListCtrl *pListCtrl;
	pListCtrl = XRCCTRL(*this, "IDC_LIST_CHEATS_GAME", wxListCtrl);

	pListCtrl->Freeze();
	pListCtrl->DeleteAllItems();

	for (i = 0; i < m_WinapePokeDatabase->GetNumberGame(); i++)
	{
		m_WinapePokeDatabase->GetNameofGame(i, &Name);

		wxListItem Item;
		wxString sValue;

		Item.SetMask(wxLIST_MASK_TEXT);
		Item.SetId(i);
		Item.SetText(Name);
		Item.SetColumn(0);
		Item.SetImage(-1);
		Item.SetData(i);
		pListCtrl->InsertItem(Item);

	}

	pListCtrl->Thaw();

}

void WinapePokeDatabase_Free(void)
{
	if (m_WinapePokeDatabase)
	{
		delete(m_WinapePokeDatabase);
	}
}