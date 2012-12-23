////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Window class for multiline text edit dialog
////////////////////////////////////////////////////////////////////////////

#ifndef EDITDLG_H_
#define EDITDLG_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "gui/Dialog.h"
#include <string>

class EditDlg : public Dialog
{
public:
	EditDlg();
	virtual ~EditDlg();
	virtual void Create();

	std::string GetValue(){ UpdateData(); return m_strValue; }
	void SetValue(const char *szText);
	void SetTitle(const char *szText);
	void SetSelection(int nSelFrom, int nSelLen);

protected:
	GtkWidget* create_dialog (GtkWidget* parent = NULL);
	void UpdateData();

protected:
	std::string m_strValue;
};

#endif // EDITDLG_H_
