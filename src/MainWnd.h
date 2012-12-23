////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Main application window implementation
////////////////////////////////////////////////////////////////////////////

#ifndef MAINWND_H_
#define MAINWND_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gtk/gtk.h>

class MainWindow
{
public:
	MainWindow();
	virtual ~MainWindow();

	void Create();
	void OnCreate();

public:
	GtkWidget *m_pWidget;
	GtkWidget *m_pToolbarBold;
	GtkWidget *m_pToolbarItalic;
	GtkWidget *m_pToolbarUnderline;
	GtkWidget *m_pToolbarColor;
	GtkWidget *m_pToolbarAttachment;
	GtkWidget *m_pToolbarStrike;
	GtkWidget *m_pToolbarBkgColor;
	GtkWidget *m_pToolbarUndo;
	GtkWidget *m_pToolbarRedo;

protected:
	GtkWidget* create_main_win ();
	GtkWidget* create_toolbar ();
};

#endif // MAINWND_H_
