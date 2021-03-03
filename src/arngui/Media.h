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
#ifndef __MEDIA_HEADER_INCLUDED__
#define __MEDIA_HEADER_INCLUDED__

#include <wx/dynarray.h>
#if (wxVERSION_NUMBER >= 2900)
#include <wx/filehistory.h>
#else
#include <wx/docview.h>
#endif

const int FILE_HISTORY_SIZE = 10;

class Media
{
public:


	typedef enum
	{
		MEDIA_TYPE_UNKNOWN = 0,
		MEDIA_TYPE_DISK,
		MEDIA_TYPE_CARTRIDGE,
		MEDIA_TYPE_CASSETTE,
		MEDIA_TYPE_SNAPSHOT
	} MEDIA_TYPE;

protected:
	int m_nType;
	wxString m_sName;
	wxString m_sCurrentPath;
	bool m_bMediaInserted;
	wxFileHistory m_History;
	//wxArrayString m_History;    // history of picked paths suitable for ui
	int m_nUnit;
	wxString GetSettingsKey();


	void LoadI(const wxString &sFilename);
	void RemoveI();
	void SetName(const wxString &sName);
	void SetUnit(int nUnit);
	void SetType(int nType);

	Media();



	virtual bool Load(const wxString &sFilename, bool bAllowFallback);
	virtual bool Save(const wxString &sFilename);
	virtual bool SaveModified(bool bForce);
	bool IgnoreModified();
	// // save history to settings
	void SaveHistory();
	// reload history from settings
	//  void LoadHistory();

public:
	void ClearHistory();
	bool IsOurHistoryItem(size_t nID);
	bool RemoveHistoryItem(size_t nID);
	bool LoadHistoryItem(size_t nID);
	size_t m_nBaseID;
	void SetHistoryMenu(wxMenu *pMenu, size_t nID);
	virtual ~Media();
	virtual void InsertUninitialised() {}
	virtual void InsertInitialised(int) {}
	void LoadConfig();
	void SaveConfig();

	// get the most recent file (first in the history list)
	wxString GetRecentPath() const;

	// get the path of the current image in use
	wxString GetCurrentPath() const { return m_sCurrentPath; }

	bool GetMediaInserted() const { return m_bMediaInserted; }
	int GetType() const { return m_nType; }
	int GetUnit() const { return m_nUnit; }
	const wxString &GetName() const { return m_sName; }


	//  virtual bool LoadWithCheck(const wxString &sFilename);
	//virtual bool SaveWithCheck(const wxString &sFilename);

	// save existing if modified then do load
	bool LoadWithSaveModified(const wxString &sFilename, bool bAllowFallback);
	// remove and save modified
	void Unload(bool bForce = false);
	void SaveAs();
	// reload existing
	bool Reload(bool bForce = false);

	// perform a save without unloading
	void ForceSave();

	virtual void Remove();

	virtual bool CanBeModified() const;
	virtual bool IsModified() const;
};

class CassetteMedia : public Media
{
public:
	CassetteMedia();
	~CassetteMedia();

	virtual bool Load(const wxString &sFilename, bool bAllowFallback);
	virtual void Remove();

	virtual bool IsModified()  const;
	virtual bool CanBeModified()  const;
};


class CartridgeMedia : public Media
{
public:
	CartridgeMedia();
	~CartridgeMedia();

	virtual bool Load(const wxString &sFilename, bool bAllowFallback);
	virtual void Remove();
};

class DiskMedia : public Media
{
private:
public:
	DiskMedia(int nDrive);
	~DiskMedia();
	virtual bool Load(const wxString &sFilename, bool bAllowFallback);
	virtual bool Save(const wxString &sFilename);
	virtual void Remove();
	virtual void InsertUninitialised();
	virtual void InsertInitialised(int nFormatCode);

	virtual bool IsModified() const;
	virtual bool CanBeModified() const;
};


class SnapshotMedia : public Media
{
private:

public:
	SnapshotMedia();
	~SnapshotMedia();

	virtual bool Load(const wxString &sFilename, bool bAllowFallback);
	virtual void Remove();
};

#if 0
class RomMedia : public Media
{
private:

public:
	RomMedia();
	~RomMedia();
	virtual bool Load(const wxString &sFilename);
	virtual void Remove();
};
#endif

#endif
