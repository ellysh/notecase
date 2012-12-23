////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements light wrapper around FileChooser GTK API
////////////////////////////////////////////////////////////////////////////

#ifndef FILEDIALOG_H__
#define FILEDIALOG_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gtk/gtk.h>

//TOFIX add multiple file selection support
/*
	gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (file_sel), FALSE);
	gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (file_sel), TRUE);
	gtk_file_chooser_get_filenames
*/


class FileDialog  
{
public:
	FileDialog(bool bOpen = true, bool bOnlyFolders = false, bool bExpand = false, const char *szTitle = NULL, void *pParent = NULL);
	virtual ~FileDialog();

	void SetTitle(const char *szText);
	void SetFilename(const char *szName);
	void SetDirectory(const char *szPath);
	void AddFilter(const char *szText, const char *szPtrn);
	
	bool IsOpenDialog(){ return m_bOpenDialog; }
	bool DoModal();
	void Close();

	const char *GetFilename();
	const char *GetCurrentFilterName();
	const char *GetDirectory(); 

	void ForceFormatExtension();

protected:
	GtkWidget *m_pDialog;
	gchar *m_szFile;
	gchar *m_szDir;
	bool   m_bOpenDialog;	//open dialog or save dialog

	//used only for custom dlg
	GtkWidget *m_pFileWidget;	
	GtkWidget *m_pFormatsCombo;
	GtkWidget *m_pFileNameEntry;
};

#endif // FILEDIALOG_H__
