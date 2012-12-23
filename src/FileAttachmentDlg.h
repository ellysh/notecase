////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements window to handle file attachments (each document node has its own attachments)
////////////////////////////////////////////////////////////////////////////

#ifndef FILATTACHMENTDLG_H__
#define FILATTACHMENTDLG_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "gui/Dialog.h"
#include <string>

class FileAttachmentDlg : public Dialog
{
public:
	FileAttachmentDlg();
	virtual ~FileAttachmentDlg();

	virtual void Create();

	int GetSelRow();

public:
	int m_nNodeIdx;

protected:
	GtkWidget* create_dialog (GtkWidget* parent=NULL);
};

#endif // FILATTACHMENTDLG_H__
