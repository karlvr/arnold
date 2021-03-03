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
#ifndef __LABEL_MANAGER_HEADER_INCLUDED__
#define __LABEL_MANAGER_HEADER_INCLUDED__

#include <wx/dialog.h>
#include <wx/listctrl.h>

#ifndef wxOVERRIDE
#define wxOVERRIDE
#endif

#include "SortableListCtrl.h"

extern "C"
{
#include "../cpc/debugger/labelset.h"
}


class LabelsListCtrl : public SortableListCtrl
{
public:
	LabelsListCtrl();
	~LabelsListCtrl();

	// provide this in your base class to perform the sorting
	virtual int PerformSort(wxIntPtr WXUNUSED(item1), wxIntPtr WXUNUSED(item2)) wxOVERRIDE;


	void OnAddLabel(wxCommandEvent &event);
	void OnRenameLabel(wxCommandEvent &event);
	void OnRemoveLabel(wxCommandEvent &event);
	void OnEnableLabel(wxCommandEvent &event);
	void OnDisableLabel(wxCommandEvent & WXUNUSED(event));
	void OnGoToLabel(wxCommandEvent & WXUNUSED(event));
	void OnLabelActivated(wxListEvent &event);

	void OnContextMenu(wxContextMenuEvent &event);
	void OnKeyDown(wxKeyEvent &event);
	void AddNewLabel();
	void RenameSelectedLabels();
	void SendDataToDissassembly(LABEL *pLabel);

	void AddLabel(LABEL *pLabel);
	void UpdateLabel(int nIndex);

	void ActivateSelectedLabels(BOOL bState);
	void RemoveSelectedLabels();
	void RemoveSelectedLabelSets();

	void OnCreate(wxWindowCreateEvent &event);

	class LabelManagerDialog *m_pDialog;

	DECLARE_EVENT_TABLE()
	DECLARE_DYNAMIC_CLASS(LabelsListCtrl)
};


class LabelSetListCtrl : public SortableListCtrl
{
public:
	LabelSetListCtrl();
	~LabelSetListCtrl();

	// provide this in your base class to perform the sorting
	virtual int PerformSort(wxIntPtr WXUNUSED(item1), wxIntPtr WXUNUSED(item2)) wxOVERRIDE;
	LABELSET *m_pSelectedLabelSet;


	void SelectLabelSet(LABELSET *pLabelSet);

	void OnAddLabel(wxCommandEvent &event);
	void OnRenameLabel(wxCommandEvent &event);
	void OnRemoveLabel(wxCommandEvent &event);
	void OnEnableLabel(wxCommandEvent &event);
	void OnDisableLabel(wxCommandEvent & WXUNUSED(event));
	void OnLoadLabelsNOI(wxCommandEvent &event);
	void OnLoadLabelsPasmo(wxCommandEvent &event);
	void OnLoadLabelsRasm(wxCommandEvent & WXUNUSED(event));
	void OnLoadLabelsWinape(wxCommandEvent & WXUNUSED(event));
	void OnSaveLabelsPasmo(wxCommandEvent &event);
	void OnLoadLabelsSjasm(wxCommandEvent & WXUNUSED(event));

	void AddNewLabelSet();
	void RenameSelectedLabelSets();

	void OnContextMenu(wxContextMenuEvent &event);
	void OnLabelSetChange(wxListEvent &event);
	void OnKeyDown(wxKeyEvent &event);

	void ActivateSelectedLabelSets(BOOL bState);
	void RemoveSelectedLabelSets();

	void AddLabelSet(LABELSET *pLabelSet);
	void UpdateLabelSet(int nIndex);

	void RefreshLabelSets();

	void OnCreate(wxWindowCreateEvent &event);

	DECLARE_EVENT_TABLE()
	DECLARE_DYNAMIC_CLASS(LabelsListCtrl)

	class LabelManagerDialog *m_pDialog;
};

class LabelManagerDialog : public wxDialog
{
public:
	LabelManagerDialog(wxWindow *pParent);
	~LabelManagerDialog();
	virtual bool TransferDataToWindow() wxOVERRIDE;
	virtual bool TransferDataFromWindow() wxOVERRIDE;

	void OnClose(wxCloseEvent &event);
	LabelsListCtrl *GetLabelsListCtrl();
	LabelSetListCtrl *GetLabelSetListCtrl();
private:
	static LabelManagerDialog *m_pInstance;
public:
	// creator
	static LabelManagerDialog *CreateInstance(wxWindow *pParent)
	{
		if (m_pInstance == NULL)
		{
			m_pInstance = new LabelManagerDialog(pParent);
			if (m_pInstance != NULL)
			{
				m_pInstance->SetTitle(wxT("Label Manager"));
				m_pInstance->Show();
			}

		}
		else
		{
			m_pInstance->Raise();
		}
		return m_pInstance;
	}




	DECLARE_EVENT_TABLE()
};


class NewLabelDialog : public wxDialog
{
public:
	NewLabelDialog(wxWindow *pParent);
	virtual bool TransferDataToWindow() wxOVERRIDE;
	virtual bool TransferDataFromWindow() wxOVERRIDE;
	wxString m_sName;
	int m_nAddress;
private:


	DECLARE_EVENT_TABLE()
};


#endif
