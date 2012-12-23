////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements window to list/change keyboard shortcuts for program actions
////////////////////////////////////////////////////////////////////////////

#ifndef SHORTCUTSLISTDLG_H__
#define SHORTCUTSLISTDLG_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "gui/Dialog.h"
#include <string>
#include "ShortcutsList.h"

class ShortcutsListDlg : public Dialog
{
public:
	ShortcutsListDlg();
	virtual ~ShortcutsListDlg();

	virtual void Create();
	void OnOK();
	void OnContextChanged();
	void OnClearAll();
	void OnResetAll();

	int  GetSelRow();
	void SetCurShortcut(ShortcutDef &def);
	void FillList(int nCtx);

public:
	ShortcutsList m_list;

protected:
	GtkWidget* create_dialog (GtkWidget* parent=NULL);
};

#endif // SHORTCUTSLISTDLG_H__
