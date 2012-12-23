////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Window class for About dialog
////////////////////////////////////////////////////////////////////////////

#ifndef AboutDlg_H_
#define AboutDlg_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "gui/Dialog.h"
#include <string>

class AboutDlg : public Dialog
{
public:
	AboutDlg();
	virtual ~AboutDlg();
	virtual void Create();

protected:
	GtkWidget* create_dialog (GtkWidget* parent = NULL);

protected:
	std::string m_strValue;
};

#endif // ExportDialog_H_
