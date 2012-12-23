////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Window class for Find/Replace dialog
////////////////////////////////////////////////////////////////////////////

#ifndef FINDREPLACEDIALOG_H_
#define FINDREPLACEDIALOG_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "gui/Dialog.h"

class FindReplaceDialog : public Dialog
{
public:
	FindReplaceDialog();
	virtual ~FindReplaceDialog();

	virtual void Create();

	bool m_bOpStarted;

	bool DoFindNext(bool bAutoProceedToStart = false, bool bNotifyDone = true);
	bool DoReplace();
	bool DoReplaceAll();

protected:
	GtkWidget* create_find_dialog (GtkWidget* parent = NULL);

protected:
	int m_nReplaceAllCount;
};

#endif // FINDREPLACEDIALOG_H_
