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
#include "connection.h"
#include "arnguiApp.h"
#include <wx/ipc.h>
#include <wx/dialog.h>
#include <wx/msgdlg.h>
#include <wx/dataobj.h>

#ifndef wxOVERRIDE
#define wxOVERRIDE
#endif

class ArnoldServerConnection : public wxConnection
{
public:
	virtual bool OnExecute(const wxString& WXUNUSED(topic), const void *data, size_t size, wxIPCFormat format) wxOVERRIDE
	{
//		wxLogMessage(wxT("OnExecute %s, %s,%p,%d,%d"), topic, "", data, size, format);
		switch (format)
		{
		case wxIPC_TEXT:
		case wxIPC_UNICODETEXT:
		{
			wxTextDataObject textDataObject;
			textDataObject.SetData(size, data);

			wxString sText = textDataObject.GetText();
			wxGetApp().ProcessCommandLineFromString(sText);

				return true;
		}
		break;
		default:
			break;
		}

		return false;
	}

	virtual const void *OnRequest(const wxString& WXUNUSED(topic), const wxString& WXUNUSED(item), size_t *size, wxIPCFormat WXUNUSED(format)) wxOVERRIDE
	{
		*size = 0;

		if (!*size)
		{
//	wxLogMessage("Unknown request for \"%s\"", item);
			return NULL;
		}

		// store the data pointer to which we return in a member variable to ensure
		// that the pointer remains valid even after we return
		//m_requestData = s.mb_str();
		//const void * const data = m_requestData;
//		const void * const data = nullptr;
//		wxLogMessage(wxT("OnRequest %s %s %p %d %d"), topic, item, data, *size, format);
	//	return data;

		return NULL;


	}

	virtual bool OnPoke(const wxString& topic, const wxString& item, const void *data, size_t size, wxIPCFormat format) wxOVERRIDE
	{
//		wxLogMessage(wxT("OnPoke %s %s %p %d %d"), topic, item, data, size, format);
		return wxConnection::OnPoke(topic, item, data, size, format);
	}

	virtual bool OnStartAdvise(const wxString& WXUNUSED(topic), const wxString& WXUNUSED(item)) wxOVERRIDE
	{
//		wxLogMessage("OnStartAdvise(\"%s\", \"%s\")", topic, item);
//		wxLogMessage("Returning true");
		//m_advise = item;
		return true;
	}

	virtual bool OnStopAdvise(const wxString& WXUNUSED(topic), const wxString& WXUNUSED(item)) wxOVERRIDE
	{
//		wxLogMessage("OnStopAdvise(\"%s\",\"%s\")", topic, item);
//		wxLogMessage("Returning true");
//		m_advise.clear();
//		wxGetApp().GetFrame()->UpdateUI();
		return true;
	}

	virtual bool DoAdvise(const wxString& item, const void *data, size_t size, wxIPCFormat format) wxOVERRIDE
	{
//		wxLogMessage(wxT("Advise %s %s %p %d %d"), "", item, data, size, format);
		return wxConnection::DoAdvise(item, data, size, format);
	}
	virtual bool OnDisconnect() wxOVERRIDE
	{
		return wxConnection::OnDisconnect();
//		wxLogMessage("OnDisconnect()");
//		wxGetApp().GetFrame()->Disconnect();
//		return true;
	}
};

class ArnoldServer : public wxServer
{
public:
	ArnoldServer() : wxServer() {}

	virtual wxConnectionBase *OnAcceptConnection(const wxString& topic) wxOVERRIDE
	{
//		wxLogMessage("OnAcceptConnection(\"%s\")", topic);

		if (topic == wxGetApp().GetAppName())
		{
			ArnoldServerConnection *connection = new ArnoldServerConnection();
		//	wxGetApp().GetFrame()->UpdateUI();
	//		wxLogMessage("Connection accepted");
			return connection;
		}
		//else: unknown topic

	//	wxLogMessage("Unknown topic, connection refused");
		return NULL;
	}

};

class ArnoldClientConnection : public wxConnection
{
public:
	virtual bool DoExecute(const void *data, size_t size, wxIPCFormat format) wxOVERRIDE
	{
	//	wxLogMessage(wxT("Execute(%s,%s,%p,%d,%d)"), wxEmptyString, wxEmptyString, data, size, format);
		bool retval = wxConnection::DoExecute(data, size, format);
		if (!retval)
		{
		//	wxLogMessage(wxT("Execute failed!"));
		}
		return retval;
	}

	virtual const void *Request(const wxString& item, size_t *size = NULL, wxIPCFormat format = wxIPC_TEXT) wxOVERRIDE
	{
		const void *data = wxConnection::Request(item, size, format);
//		wxLogMessage(wxT("Request %s, %s, %p, %d, %d"), wxEmptyString, item, data, size ? *size : wxNO_LEN, format);
		return data;
	}

	virtual bool DoPoke(const wxString& item, const void* data, size_t size, wxIPCFormat format) wxOVERRIDE
	{
	//	wxLogMessage(wxT("Poke %s, %s, %p, %d, %d"), wxEmptyString, item, data, size, format);
		return wxConnection::DoPoke(item, data, size, format);
	}
	
	virtual bool OnAdvise(const wxString& WXUNUSED(topic), const wxString& WXUNUSED(item), const void * WXUNUSED(data), size_t WXUNUSED(size), wxIPCFormat WXUNUSED(format)) wxOVERRIDE
	{
	//	wxLogMessage(wxT("OnAdvise %s,%s,%p,%d,%d"), topic, item, data, size, format);
		return true;

	}
};

class ArnoldClient : public wxClient
{
public:
	ArnoldClient() : wxClient() {}

	virtual wxConnectionBase *OnMakeConnection() wxOVERRIDE
	{ 
		//wxLogMessage(wxT("OnMakeConnection"));
		return new ArnoldClientConnection; 
	}
};


static ArnoldServer *m_server = NULL;

void SendDataToOtherInstance()
{
	// OK, there IS another one running, so try to connect to it
	// and send it any filename before exiting.
	ArnoldClient* client = new ArnoldClient;
	if (client != NULL)
	{
		// ignored under DDE, host name in TCP/IP based classes
		wxString hostName = wxT("localhost");
		//            wxGetHostName(hostName);
		// Create the connection
		wxConnectionBase* connection =
			client->MakeConnection(hostName, wxGetApp().GetAppName(), wxGetApp().GetAppName());
		if (connection)
		{
			wxString sCommandLine;

			for (int i = 0; i < wxGetApp().argc; i++)
			{
				sCommandLine += wxT(" \"");
				sCommandLine += wxString(wxGetApp().argv[i]);
				sCommandLine += wxT("\"");
			}
			// Ask the other instance to open a file or raise itself
			connection->Execute(sCommandLine);
			connection->Disconnect();
			delete connection;
		}
		else
		{
#if !defined(__WXMAC__)
			wxMessageBox(wxT("Sorry, the existing instance may be too busy too respond.\nPlease close any open dialogs and try again"),
				wxGetApp().GetAppName(), wxICON_INFORMATION | wxOK);
#endif
		}
		delete client;
	}
}

void InitServer()
{
	// Create a new server
	m_server = new ArnoldServer;
	if (m_server)
	{
		// valgrind will throw an error because the port is not initialised.
		// wxWidgets allows named service or port number here.
		// I chose to use the app name.
		m_server->Create(wxGetApp().GetAppName());
	}
}

void ShutdownServer()
{
	if (m_server)
	{
		delete m_server;
		m_server = NULL;
	}
}
