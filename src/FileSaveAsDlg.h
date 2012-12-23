////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements window for file "save as" operation
////////////////////////////////////////////////////////////////////////////

#ifndef FILESAVEASDLG_H__
#define FILESAVEASDLG_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "gui/Dialog.h"
#include <string>

struct _fmt1 {
	const char *szName;
	const char *szFilter;
};

class FileSaveAsDlg : public Dialog
{
public:
	FileSaveAsDlg();
	virtual ~FileSaveAsDlg();

	virtual void Create();

	const char *GetFileName();
	const char *GetFilePath();
	const char *GetSelectedFormat();

	int GetSelectedFormatIdx();

	void SetFilename(const char *szName){	m_strFileName = szName; };
	void SetDirectory(const char *szName){	m_strFileDir  = szName; };

public:
	struct _fmt1 m_formats[2];	//export formats info
	void ForceFormatExtension();

protected:
	std::string m_strFileName;
	std::string m_strFileDir;

protected:
	GtkWidget* create_file_export_dialog (GtkWidget* parent=NULL);
};

#endif // $
