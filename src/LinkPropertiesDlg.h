////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Window to define hyperlink properties
////////////////////////////////////////////////////////////////////////////

#ifndef LINKPROPERTIESDLG_H__
#define LINKPROPERTIESDLG_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "gui/Dialog.h"
#include "lib/LinkInfo.h"

class LinkPropertiesDlg : public Dialog
{
public:
	LinkPropertiesDlg();
	virtual ~LinkPropertiesDlg();

	virtual void Create();
	void UpdateData(bool bToScreen);

	LinkInfo m_info;

protected:
	GtkWidget* create_link_wizard ();
};

#endif // LINKPROPERTIESDLG_H__
