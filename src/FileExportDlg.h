////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements window for file export options
////////////////////////////////////////////////////////////////////////////

#ifndef FILEEXPORTDLG_H__
#define FILEEXPORTDLG_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "gui/Dialog.h"
#include <string>

//export modes
#define EXPORT_BRANCH		0
#define EXPORT_NODE			1
#define EXPORT_DOCUMENT		2

struct _fmt {
	const char *szName;
	const char *szFilter;
};

class FileExportDlg : public Dialog
{
public:
	FileExportDlg();
	virtual ~FileExportDlg();

	virtual void Create();

	int GetExportMode();
	int GetExportFormat();
	int GetFormatCode();
	const char *GetExportFile();
	const char *GetPostExportScript();
	bool GetHtmlExportCSS(std::string &strCSS);
	bool GetExportLinked();

public:
	struct _fmt m_formats[6];	//export formats info
	void ForceFormatExtension();

protected:
	GtkWidget* create_file_export_dialog (GtkWidget* parent=NULL);
};

#endif // FILEEXPORTDLG_H__
