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
#ifndef __DEVICES_DIALOG_HEADER_INCLUDED__
#define __DEVICES_DIALOG_HEADER_INCLUDED__

#include <wx/dialog.h>
#include <wx/listctrl.h>

#ifndef wxOVERRIDE
#define wxOVERRIDE
#endif

#include "SortableListCtrl.h"

extern "C"
{
#include "../cpc/emudevice.h"
}



class DeviceRomsListCtrl : public SortableListCtrl
{
public:
	DeviceRomsListCtrl();
	~DeviceRomsListCtrl();
	void SetDeviceIndex(int nDevice) { m_nDevice = nDevice; }
private:
	int m_nDevice;
	// provide this in your base class to perform the sorting
	virtual int PerformSort(wxIntPtr item1, wxIntPtr item2) wxOVERRIDE;
	DECLARE_DYNAMIC_CLASS(DeviceRomsListCtrl)
};

class DevicesRomsDialog : public wxDialog
{
public:
	DevicesRomsDialog(wxWindow *pParent, int nDevice);
	~DevicesRomsDialog();
	virtual bool TransferDataToWindow() wxOVERRIDE;
	virtual bool TransferDataFromWindow() wxOVERRIDE;

private:
	int m_nDevice;
	void AddRom(int nDevice);
	//  void OnButtonPressed(wxCommandEvent &event);
	//  void OnROMEnable(wxCommandEvent &event);
	void OnROMRemove(wxCommandEvent &event);
	void VerifyRomData(const unsigned char *pData, unsigned long, unsigned long);
	void OnROMLoad(wxCommandEvent &event);
	void OnROMPickBuiltin(wxCommandEvent &event);
	void OnContextMenu(wxContextMenuEvent &event);
	void UpdateRom(int nIndex);
	void PerformUpdate();
	//  void OnClose(wxCloseEvent &event);
	wxListCtrl *GetListCtrl();
	void OnCharHook(wxKeyEvent& event);

	DECLARE_EVENT_TABLE()
};


class DeviceButtonsListCtrl : public SortableListCtrl
{
public:
	DeviceButtonsListCtrl();
	~DeviceButtonsListCtrl();
	void SetDeviceIndex(int nDevice) { m_nDevice = nDevice; }
private:
	int m_nDevice;

	// provide this in your base class to perform the sorting
	virtual int PerformSort(wxIntPtr item1, wxIntPtr item2) wxOVERRIDE;

	DECLARE_DYNAMIC_CLASS(DeviceButtonsListCtrl)
};

class DevicesButtonsDialog : public wxDialog
{
public:
	DevicesButtonsDialog(wxWindow *pParent, int nDevice);
	~DevicesButtonsDialog();
	virtual bool TransferDataToWindow() wxOVERRIDE;
	virtual bool TransferDataFromWindow() wxOVERRIDE;

private:
	int m_nDevice;
	void AddButton(int nDevice);
	void OnButtonPressed(wxCommandEvent &event);
	void OnContextMenu(wxContextMenuEvent &event);
	void UpdateButton(int nIndex);
	void PerformUpdate();
	//  void OnClose(wxCloseEvent &event);
	wxListCtrl *GetListCtrl();
	void OnCharHook(wxKeyEvent& event);

	DECLARE_EVENT_TABLE()
};


class DeviceSwitchesListCtrl : public SortableListCtrl
{
public:
	DeviceSwitchesListCtrl();
	~DeviceSwitchesListCtrl();
	void SetDeviceIndex(int nDevice) { m_nDevice = nDevice; }
private:
	int m_nDevice;

	// provide this in your base class to perform the sorting
	virtual int PerformSort(wxIntPtr item1, wxIntPtr item2) wxOVERRIDE;

	DECLARE_DYNAMIC_CLASS(DeviceSwitchesListCtrl)
};


class DeviceSwitchesDialog : public wxDialog
{
public:
	DeviceSwitchesDialog(wxWindow *pParent, int nDevice);
	~DeviceSwitchesDialog();
	virtual bool TransferDataToWindow() wxOVERRIDE;
	virtual bool TransferDataFromWindow() wxOVERRIDE;

private:
	int m_nDevice;
	void AddSwitch(int nDevice);
	void OnSwitchEnable(wxCommandEvent &event);
	void OnContextMenu(wxContextMenuEvent &event);
	void UpdateSwitch(int nIndex);
	void PerformUpdate();
	//  void OnClose(wxCloseEvent &event);
	wxListCtrl *GetListCtrl();
	void OnCharHook(wxKeyEvent& event);

	DECLARE_EVENT_TABLE()
};

#include "SortableListCtrl.h"

class DevicesListCtrl : public SortableListCtrl
{
public:
	DevicesListCtrl();
	~DevicesListCtrl();

	// provide this in your base class to perform the sorting
	virtual int PerformSort(wxIntPtr item1, wxIntPtr item2) wxOVERRIDE;

	DECLARE_DYNAMIC_CLASS(DevicesListCtrl)
};


class DevicesDialog : public wxDialog
{
public:
	DevicesDialog(wxWindow *pParent, int ConnectionTypeFilter);
	~DevicesDialog();
	virtual bool TransferDataToWindow() wxOVERRIDE;
	virtual bool TransferDataFromWindow() wxOVERRIDE;

private:
	int m_ConnectionTypeFilter;
	bool m_bShowUntested;
	bool m_bShowNotWorking;
	
	void AddDevice(int nDevice);
	void OnDeviceEnable(wxCommandEvent &event);
	void OnDeviceSwitches(wxCommandEvent &event);
	void OnDeviceButtons(wxCommandEvent &event);
	void OnDeviceRoms(wxCommandEvent &event);
	void OnDeviceExpansionRoms(wxCommandEvent & event);
	void OnCharHook(wxKeyEvent& event);
	void OnDeviceShowNotWorking(wxCommandEvent & event);
	void OnShowUntested(wxCommandEvent &event);
	void OnShowNotWorking(wxCommandEvent &event);
	void UpdateDeviceList();
	
	void OnContextMenu(wxContextMenuEvent &event);
	void UpdateDevice(int nIndex);
	void PerformUpdate();
	//  void OnClose(wxCloseEvent &event);
	wxListCtrl *GetListCtrl();

	DECLARE_EVENT_TABLE()
};

#endif
