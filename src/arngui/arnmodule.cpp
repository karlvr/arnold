// module.cpp
// This code by Troels K
#include <wx/module.h>
#include "arnmodule.h"

// The module can be included by other wxApps, so it can be embedded
// within them.
//
// our app just calls our module.


IMPLEMENT_DYNAMIC_CLASS(ArnoldModule, wxModule)

static ArnoldModule* instance = NULL;

/*static*/
ArnoldModule* ArnoldModule::Get()
{

	return instance;
}

ArnoldModule::ArnoldModule(void) : wxModule()
{
	instance = this;
}

bool ArnoldModule::OnInit(void)
{
	return true;
}

void ArnoldModule::OnExit(void)
{
}
#if 0
/*static*/ void ArnoldModule::GetVersionInfo(wxAboutDialogInfo* info)
{
	info->SetName(wxT(LIBARNOLD_DISPLAYNAME));
	info->SetDescription(wxString(_("CPC emulator\n\n")) + wxVERSION_STRING);
	info->SetCopyright(wxT("Copyright (c) 2011 ") wxT(LIBARNOLD_AUTHOR));
	info->AddDeveloper(wxT(LIBARNOLD_AUTHOR));
	info->SetLicense(wxT("GNU General Public License, Version 2"));
	info->SetWebSite(wxT(LIBARNOLD_WEBSITE));
}
#endif
