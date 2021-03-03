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
#include <wx/dialog.h>
#include <wx/checkbox.h>
#ifdef __WXMSW__
#include <wx/msw/private.h>
#endif
#include <wx/msw/registry.h>
#include <wx/listctrl.h>
#include <wx/xrc/xmlres.h>
#include <wx/stdpaths.h>

#ifndef wxOVERRIDE
#define wxOVERRIDE
#endif


#include <shlobj.h>
#include "../SortableListCtrl.h"

//#include "arnguiMain.h"
#include "../Media.h"
#include "../arnguiApp.h"
#include "../IntClientData.h"
#include "../arnguiMain.h"

// corresponding media icons
enum
{
	ICON_DISK = 0,
	ICON_CASSETTE,
	ICON_CARTRIDGE,
	ICON_SNAPSHOT
};

// store icons in a seperate dll

typedef struct
{
	Media::MEDIA_TYPE eMediaType;
	const wxChar *sExtension;                 // "dsk"
	const wxChar *DefaultExtensionKey;      // "DskFile
	const wxChar *OpenWithDescription;      // "Open with Arnold"
	const wxChar *ExtensionDescription;     // "CPC Disk Image File"
	int     nIconIndex;                 // index of icon within icons.dll
	const wxChar *sCommand;                  // command to run
} ExtensionInformation;


class AssociationsListCtrl : public SortableListCtrl
{
public:
	AssociationsListCtrl();
	~AssociationsListCtrl();

	// provide this in your base class to perform the sorting
	virtual int PerformSort(wxIntPtr item1, wxIntPtr item2) wxOVERRIDE;

	DECLARE_DYNAMIC_CLASS(AssociationsListCtrl)
};




AssociationsListCtrl::AssociationsListCtrl() : SortableListCtrl()
{
}

AssociationsListCtrl::~AssociationsListCtrl()
{
}

IMPLEMENT_DYNAMIC_CLASS(AssociationsListCtrl, wxListCtrl)



class RegisterFileTypesDialog : public wxDialog
{
public:
	RegisterFileTypesDialog(wxWindow *pParent);
	~RegisterFileTypesDialog();

	virtual bool TransferDataFromWindow() wxOVERRIDE;
	virtual bool TransferDataToWindow() wxOVERRIDE;

	static const ExtensionInformation FileExtensions[];
	static wxString GetMediaName(Media::MEDIA_TYPE eMediaType);

private:

	//    int GetIconIndexFromMediaType(MEDIA_TYPE eMediaType);
	void OnApplyTypes(wxCommandEvent & WXUNUSED(event));

	wxListCtrl *GetListCtrl();
	void RegisterArnoldAppPath();
	void SetExtensionInRegistry(const ExtensionInformation &Information, bool bMakeDefault);
	void OnSelectAll(wxCommandEvent &);
	void OnClearAll(wxCommandEvent &);
	void OnClose(wxCloseEvent &event);
	void OnCharHook(wxKeyEvent& event);
	DECLARE_EVENT_TABLE()
};

const ExtensionInformation RegisterFileTypesDialog::FileExtensions[] =
{
	// sExtension, sDefaultExtensionKey, sExtensionDescription,nIconIndex,sCommand
	{
		Media::MEDIA_TYPE_DISK, wxT("dsk"), wxT("dskfile"), wxT("Open with Arnold"), wxT("Amstrad CPC/Plus Disk Image File"), ICON_DISK, wxT("/d")
	},
	{
		Media::MEDIA_TYPE_CARTRIDGE, wxT("cpr"), wxT("cprfile"), wxT("Open with Arnold"), wxT("Amstrad Plus Cartridge Image File"), ICON_CARTRIDGE, wxT("/c")
	},
	{
		Media::MEDIA_TYPE_CASSETTE, wxT("cdt"), wxT("cdtfile"), wxT("Open with Arnold"), wxT("Amstrad CPC/Plus Tape Image File"), ICON_CASSETTE, wxT("/t")
	},
	{
		Media::MEDIA_TYPE_CASSETTE, wxT("tzx"), wxT("tzxfile"), wxT("Open with Arnold"), wxT("Amstrad CPC/Plus Tape Image File"), ICON_CASSETTE, wxT("/t")
	},
	{
		Media::MEDIA_TYPE_CASSETTE, wxT("csw"), wxT("cswfile"), wxT("Open with Arnold"), wxT("Amstrad CPC/Plus Tape Audio File"), ICON_CASSETTE, wxT("/t")
	},
	{
		Media::MEDIA_TYPE_CASSETTE, wxT("wav"), wxT("wavfile"), wxT("Open with Arnold"), wxT("Amstrad CPC/Plus Tape Audio File"), ICON_CASSETTE, wxT("/t")
	},
	{
		Media::MEDIA_TYPE_CASSETTE, wxT("voc"), wxT("vocfile"), wxT("Open with Arnold"), wxT("Amstrad CPC/Plus Tape Audio File"), ICON_CASSETTE, wxT("/t")
	},
	{
		Media::MEDIA_TYPE_SNAPSHOT, wxT("sna"), wxT("snafile"), wxT("Open with Arnold"), wxT("Amstrad CPC/Plus Memory Snapshot File"), ICON_SNAPSHOT, wxT("/s")
	}
};


int AssociationsListCtrl::PerformSort(wxIntPtr item1id, wxIntPtr item2id)
{
	IntClientData *pClientData1 = (IntClientData *)item1id;
	IntClientData *pClientData2 = (IntClientData *)item2id;

	wxString a, b;

	switch (m_nSortColumn)
	{
	default:
	case 0:
	{
		a = RegisterFileTypesDialog::FileExtensions[pClientData1->GetData()].sExtension;
		b = RegisterFileTypesDialog::FileExtensions[pClientData2->GetData()].sExtension;
	}
	break;

	case 1:
	{
		a = RegisterFileTypesDialog::GetMediaName(RegisterFileTypesDialog::FileExtensions[pClientData1->GetData()].eMediaType);
		b = RegisterFileTypesDialog::GetMediaName(RegisterFileTypesDialog::FileExtensions[pClientData2->GetData()].eMediaType);
	}
	break;
	}

	int result;

	// a>b
	// a==b
	// a<b
	result = a.CmpNoCase(b);

	if (!m_bSortAscending)
	{
		result = -result;
	}
	return result;
}


/*)
int RegisterFileTypesDialog::GetIconIndexFromMediaType(MEDIA_TYPE eMediaType)
{
switch (eMediaType)
{
case MEDIA_TYPE_DISK:
return ICON_DISK;
case MEDIA_TYPE_CASSETTE:
return ICON_CASSETTE;
case MEDIA_TYPE_CARTRIDGE:
return ICON_CARTRIDGE;
case MEDIA_TYPE_SNAPSHOT:
return ICON_SNAPSHOT;
}

return 0;
}
*/

wxString RegisterFileTypesDialog::GetMediaName(Media::MEDIA_TYPE eMediaType)
{
	switch (eMediaType)
	{
	case Media::MEDIA_TYPE_DISK:
		return wxT("Disk");
	case Media::MEDIA_TYPE_CASSETTE:
		return wxT("Cassette");
	case Media::MEDIA_TYPE_CARTRIDGE:
		return wxT("Cartridge");
	case Media::MEDIA_TYPE_SNAPSHOT:
		return wxT("Snapshot");
	case Media::MEDIA_TYPE_UNKNOWN:
		return wxT("Unknown");
	}

	return wxEmptyString;
}

// tell windows where to find the exe and tell it the start up directory
// we need this so it can find the dlls
void RegisterFileTypesDialog::RegisterArnoldAppPath()
{
	// get location of executable
	wxString sApplicationPath = wxStandardPaths::Get().GetExecutablePath();
	wxString sIconPath = wxGetApp().GetIconsFullPath();

	// location of the exe
	wxString sAppPathKey = wxT("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\arnold.exe");

	wxRegKey *pAppPathKey = new wxRegKey(sAppPathKey);
	if (pAppPathKey != NULL)
	{
		// does it exist?
		if (!pAppPathKey->Exists())
		{
			// no create it
			pAppPathKey->Create();
		}
		// set extension description
		pAppPathKey->SetValue(wxEmptyString, sApplicationPath);

		wxFileName FileName(sApplicationPath);
		wxString sPath = FileName.GetPathWithSep();

		// set value
		pAppPathKey->SetValue(wxT("Path"), sPath);

		delete pAppPathKey;
	}
}

void RegisterFileTypesDialog::SetExtensionInRegistry(
	const ExtensionInformation &Information,
	bool bMakeDefault)
{
	// get location of executable; windows will find it.
	wxString sApplicationPath = wxT("arnold.exe");
	wxString sIconPath = wxGetApp().GetIconsFullPath();

	wxString sExtensionKeyPath = wxT("HKEY_CURRENT_USER\\Software\\Classes\\");
	sExtensionKeyPath += wxT(".");
	sExtensionKeyPath += Information.sExtension;

	wxString sExtensionKeyValue;

	// ".dsk" key

	// try and get extension key
	wxRegKey *pExtensionBaseKey = new wxRegKey(sExtensionKeyPath);
	if (pExtensionBaseKey != NULL)
	{
		bool bCreate = false;
		bool bSet = false;
		if (pExtensionBaseKey->Exists())
		{
			pExtensionBaseKey->QueryValue(wxEmptyString, sExtensionKeyValue);

			if (sExtensionKeyValue.Length() == 0)
			{
				bSet = true;
			}
		}
		else
		{
			bCreate = true;
			bSet = true;
		}

		// does it exist?
		if (bCreate)
		{
			pExtensionBaseKey->Exists();
		}
		if (bSet)
		{
			sExtensionKeyValue = Information.DefaultExtensionKey;
			pExtensionBaseKey->SetValue(wxEmptyString, sExtensionKeyValue);
		}

		delete pExtensionBaseKey;
	}

	// "dskfile" key
	wxString sExtensionKeyHandlerPath = wxT("HKEY_CURRENT_USER\\Software\\Classes\\") + sExtensionKeyValue + wxT("\\");
	wxRegKey *pExtensionKey = new wxRegKey(sExtensionKeyHandlerPath);
	if (pExtensionKey != NULL)
	{
		if (!pExtensionKey->Exists())
		{
			pExtensionKey->Create();
		}

		if (bMakeDefault)
		{
			// set extension description
			pExtensionKey->SetValue(wxEmptyString, Information.ExtensionDescription);

			wxString sDefaultIconKeyPath = sExtensionKeyHandlerPath + wxT("DefaultIcon\\");

			// set icon
			wxRegKey *pDefaultIconKey = new wxRegKey(sDefaultIconKeyPath);
			if (pDefaultIconKey != NULL)
			{
				// does it exist?
				if (!pDefaultIconKey->Exists())
				{
					// create if it doesn't exist
					pDefaultIconKey->Create();
				}

				// now create it's value
				wxString sDefaultIconKeyValue;
				sDefaultIconKeyValue.Printf(wxT("\"%s\",%d"), sIconPath.c_str(), -Information.nIconIndex);

				// set value
				pDefaultIconKey->SetValue(wxEmptyString, sDefaultIconKeyValue);

				// clean up
				delete pDefaultIconKey;
			}
		}

		wxString sShellKeyPath = sExtensionKeyHandlerPath + wxT("shell\\");

		wxRegKey *pShellKey = new wxRegKey(sShellKeyPath);
		if (pShellKey != NULL)
		{
			if (!pShellKey->Exists())
			{
				pShellKey->Create();
			}

			if (bMakeDefault)
			{
				// "open" key

				// create the path to the open key
				wxString sOpenKeyPath = sShellKeyPath + wxT("open\\");

				wxRegKey *pOpenKey = new wxRegKey(sOpenKeyPath);
				if (pOpenKey != NULL)
				{

					// create if it doesn't exist
					if (!pOpenKey->Exists())
					{
						pOpenKey->Create();
					}

					wxString sOpenKeyCommandPath = sOpenKeyPath + wxT("command\\");
					wxRegKey *pOpenCommandKey = new wxRegKey(sOpenKeyCommandPath);
					if (pOpenCommandKey != NULL)
					{
						// create if it doesn't exist
						if (!pOpenCommandKey->Exists())
						{
							pOpenCommandKey->Create();
						}
						// set it's value
						wxString sValue;
						sValue.Printf(wxT("\"%s\" %s \"%%1\""), sApplicationPath.c_str(), Information.sCommand);
						pOpenCommandKey->SetValue(wxEmptyString, sValue);
						delete pOpenCommandKey;
					}

					delete pOpenKey;
				}
			}

			//open with
			wxString sOpenWithRegistryKeyPath = sShellKeyPath + Information.OpenWithDescription + wxT("\\");

			wxRegKey *pOpenWithKey = new wxRegKey(sOpenWithRegistryKeyPath);
			if (pOpenWithKey != NULL)
			{

				if (!pOpenWithKey->Exists())
				{
					pOpenWithKey->Create();
				}

				wxString sOpenWithCommandKeyPath = sOpenWithRegistryKeyPath + wxT("command\\");
				wxRegKey *pOpenWithCommandKey = new wxRegKey(sOpenWithCommandKeyPath);
				if (pOpenWithCommandKey != NULL)
				{
					// create if it doesn't exist already
					if (!pOpenWithCommandKey->Exists())
					{
						pOpenWithCommandKey->Create();
					}

					wxString sValue;
					sValue.Printf(wxT("\"%s\" %s \"%%1\""), sApplicationPath.c_str(), Information.sCommand);
					pOpenWithCommandKey->SetValue(wxEmptyString, sValue);


					delete pOpenWithCommandKey;
				}

				delete pOpenWithKey;
			}
			delete pShellKey;
		}
		delete pExtensionKey;
	}
}


BEGIN_EVENT_TABLE(RegisterFileTypesDialog, wxDialog)
EVT_BUTTON(XRCID("ID_SELECT_ALL"), RegisterFileTypesDialog::OnSelectAll)
EVT_BUTTON(XRCID("ID_CLEAR_ALL"), RegisterFileTypesDialog::OnClearAll)
EVT_BUTTON(XRCID("wxID_APPLY"), RegisterFileTypesDialog::OnApplyTypes)
EVT_CHAR_HOOK(RegisterFileTypesDialog::OnCharHook)
EVT_CLOSE(RegisterFileTypesDialog::OnClose)
END_EVENT_TABLE()


void RegisterFileTypesDialog::OnClose(wxCloseEvent & WXUNUSED(event))
{
	wxGetApp().WriteConfigWindowPosAndSize(wxT("windows/file_associations/"), this);

	Destroy();
}

RegisterFileTypesDialog::~RegisterFileTypesDialog()
{
	wxListCtrl *pListCtrl = GetListCtrl();

	for (int i = 0; i < pListCtrl->GetItemCount(); i++)
	{
		IntClientData *pClientData = (IntClientData *)(pListCtrl->GetItemData(i));
		delete pClientData;
		//		pListCtrl->SetItemDataPtr(i, 0);
	}

	//pListCtrl->DeleteAllItems();

	// free all items and columns
	pListCtrl->ClearAll();


}

wxListCtrl *RegisterFileTypesDialog::GetListCtrl()
{
	return XRCCTRL(*this, "IDC_LIST_ASSOCIATIONS", wxListCtrl);
}

void RegisterFileTypesDialog::OnSelectAll(wxCommandEvent & WXUNUSED(event))
{
	wxListCtrl *pListCtrl = GetListCtrl();
	for (int i = 0; i < pListCtrl->GetItemCount(); i++)
	{
		pListCtrl->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}
}

void RegisterFileTypesDialog::OnClearAll(wxCommandEvent & WXUNUSED(event))
{
	wxListCtrl *pListCtrl = GetListCtrl();
	for (int i = 0; i < pListCtrl->GetItemCount(); i++)
	{
		pListCtrl->SetItemState(i, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
	}
}


bool RegisterFileTypesDialog::TransferDataToWindow()
{
	wxListCtrl *pListCtrl = GetListCtrl();
	pListCtrl->Freeze();
	pListCtrl->ClearAll();

	wxListItem Column;
	Column.SetMask(wxLIST_MASK_TEXT);
	Column.SetText(wxT("Extension"));
	pListCtrl->InsertColumn(0, Column);
	Column.SetText(wxT("Media"));
	pListCtrl->InsertColumn(1, Column);

	// fill list
	for (unsigned int i = 0; i != sizeof(FileExtensions) / sizeof(FileExtensions[0]); i++)
	{
		IntClientData *pClientData = new IntClientData();
		pClientData->SetData(i);

		wxListItem Item;

		wxString sName;
		sName = wxT(".");
		sName += FileExtensions[i].sExtension;

		// set name field
		Item.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_DATA);
		Item.SetText(sName);
		Item.SetColumn(0);
		Item.SetId(pListCtrl->GetItemCount());
		Item.SetData(pClientData);
		Item.SetImage(-1);
		int ItemID = pListCtrl->InsertItem(Item);

		//pListCtrl->SetItemData(nIndex, i);

		sName = GetMediaName(FileExtensions[i].eMediaType);

		// set size field
		Item.SetMask(wxLIST_MASK_TEXT);
		Item.SetId(ItemID);
		Item.SetText(sName);
		Item.SetColumn(1);
		pListCtrl->SetItem(Item);
	}
	pListCtrl->Thaw();

	wxGetApp().SetColumnSize(pListCtrl, 0);
	wxGetApp().SetColumnSize(pListCtrl, 1);

	return true;
}

void RegisterFileTypesDialog::OnApplyTypes(wxCommandEvent & WXUNUSED(event))
{
	wxCheckBox *pCheckBox;
	wxListCtrl *pListCtrl = GetListCtrl();

	RegisterArnoldAppPath();

	pCheckBox = XRCCTRL(*this, "IDC_CHECK_MAKEDEFAULT", wxCheckBox);

	bool bMakeDefault;
	bMakeDefault = pCheckBox->GetValue();

	// must be something selected and it must be a file in order
	// for dialog to quit
	long item = pListCtrl->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	while (item != -1)
	{
		IntClientData *pClientData = reinterpret_cast<IntClientData *>(pListCtrl->GetItemData(item));

		SetExtensionInRegistry(FileExtensions[pClientData->GetData()], bMakeDefault);

		item = pListCtrl->GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);
	}

	::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
}

bool RegisterFileTypesDialog::TransferDataFromWindow()
{

	return true;

}

void RegisterFileTypesDialog::OnCharHook(wxKeyEvent& event)
{
	if (IsEscapeKey(event))
	{
		Close();
		return;
	}

	event.Skip();
}



RegisterFileTypesDialog::RegisterFileTypesDialog(wxWindow *pParent)
{
	// load the resource
	wxXmlResource::Get()->LoadDialog(this, pParent, wxT("DLG_DIALOG_ASSOCIATIONS"));

	// default
	//	this->SetSize(275,380);
	wxGetApp().ReadConfigWindowPosAndSize(wxT("windows/file_associations/"), this);
	wxGetApp().EnsureWindowVisible(this);

	SetEscapeId(wxID_CANCEL);
	SetAffirmativeId(wxID_APPLY);
}

void arnguiFrame::OnRegisterFileTypes(wxCommandEvent & WXUNUSED(event))
{
	RegisterFileTypesDialog *pDialog = new RegisterFileTypesDialog(this);
	pDialog->ShowModal();
	delete pDialog;
}

