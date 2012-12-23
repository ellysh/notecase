////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements progress for operations of unknown length
////////////////////////////////////////////////////////////////////////////

#include "ProgressDlg.h"
#include <string>

#include <gdk/gdkkeysyms.h>
#include "../support.h"
#include "../lib/debug.h"
#include "../MainWnd.h"
#include <string.h>

extern MainWindow g_wnd;
#define PROGRESS_REFRESH_INTERVAL 300

extern bool g_bMsgBox;
GtkWidget *g_wndProgressDlg = NULL;

//used by timer handlers
static GtkWidget *g_progress1 = NULL;
bool   g_bStartProgress = true;

int ProgressDlg::m_nCurrentSize = 0;
int ProgressDlg::m_nTotalSize = 0;
bool ProgressDlg::m_bCanceled = false;

static int progress_timer(gpointer data);
static int progress_start_timer(gpointer data);
static void on_abort_clicked(GtkButton *button, gpointer user_data);

ProgressDlg::ProgressDlg(int nWaitMs, const char *szTitle, GtkWidget *pParent, bool bCancelBtn)
{
	if(szTitle && strlen(szTitle) > 0)
		m_strTitle = szTitle;

	m_nProgressTimer = 0;
	m_nTotalSize	= -1;
	m_nCurrentSize	= -1;

	m_pDialog = NULL;
	m_pParent = pParent;
	m_bCancelBtn = bCancelBtn;

	//start timer
	g_bStartProgress = true;
	m_nStartTimer = g_timeout_add (PROGRESS_REFRESH_INTERVAL, progress_start_timer, this); 
}

ProgressDlg::~ProgressDlg()
{
	Close();
}

void ProgressDlg::SetTotalSize(int nSize)
{
	m_nTotalSize = nSize;
	m_nCurrentSize = 0;
}
	
void ProgressDlg::SetCurrentSize(int nSize)
{
	m_nCurrentSize = nSize;
}

void ProgressDlg::DoCreateWindow()
{
	//show wait dialog
	GtkWidget* msgbox = NULL;
	//no need if doing export from cmd line or if the window does not have focus (like in tray)
	if( g_wnd.m_pWidget && 
		gtk_window_is_active(GTK_WINDOW(g_wnd.m_pWidget)))	//TOFIX what if window becomes active, create always, but hidden if main wnd not active?
	{
		msgbox = gtk_window_new (GTK_WINDOW_TOPLEVEL); //TOFIX support for buttons?

		gtk_window_set_title (GTK_WINDOW (msgbox), _("Info"));
		gtk_window_set_modal (GTK_WINDOW (msgbox), TRUE);
		gtk_window_set_skip_pager_hint (GTK_WINDOW (msgbox), TRUE);
		gtk_window_set_type_hint (GTK_WINDOW (msgbox), GDK_WINDOW_TYPE_HINT_DIALOG);
	#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
		gtk_window_set_skip_taskbar_hint (GTK_WINDOW (msgbox), TRUE);
	#endif
		gtk_window_set_transient_for(GTK_WINDOW (msgbox), GTK_WINDOW(m_pParent));   //set parent
		gtk_window_set_default_size (GTK_WINDOW (msgbox), 200, 100);

	#if GTK_CHECK_VERSION(2,4,0) //new API TOFIX set proper version
		//#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
			//gtk_window_set_keep_above(GTK_WINDOW (msgbox), TRUE);
		//#endif
	#endif
		gtk_window_set_position(GTK_WINDOW (msgbox), GTK_WIN_POS_CENTER_ON_PARENT);
		gtk_window_set_resizable (GTK_WINDOW (msgbox), FALSE);
		gtk_widget_realize(msgbox);
		gdk_window_set_decorations(msgbox->window, (GdkWMDecoration)(GDK_DECOR_BORDER|GDK_DECOR_TITLE)); 

		GtkWidget *vbox1 = gtk_vbox_new (FALSE, 0);
		gtk_widget_show (vbox1);
		gtk_container_add (GTK_CONTAINER (msgbox), vbox1);
		gtk_container_set_border_width  (GTK_CONTAINER (vbox1), 20);
		
		GtkWidget *label21 = gtk_label_new (m_strTitle.c_str());
		gtk_widget_show (label21);
		gtk_box_pack_start (GTK_BOX (vbox1), label21, FALSE, FALSE, 3);
		
		g_progress1 = gtk_progress_bar_new();
		gtk_widget_show (g_progress1);
		gtk_box_pack_start (GTK_BOX (vbox1), g_progress1, FALSE, FALSE, 3);
		
		if(m_bCancelBtn)
		{
			GtkWidget *btnabort = gtk_button_new_with_mnemonic (_("Abort"));
			gtk_widget_show (btnabort);
			gtk_box_pack_start (GTK_BOX (vbox1), btnabort, FALSE, FALSE, 3);
			GTK_WIDGET_SET_FLAGS (btnabort, GTK_CAN_DEFAULT);
			g_signal_connect(btnabort, "clicked", G_CALLBACK (on_abort_clicked), this);
		}

		gtk_widget_show (msgbox);
		gtk_widget_realize(msgbox);
		m_pDialog = msgbox;

		m_nProgressTimer = g_timeout_add (PROGRESS_REFRESH_INTERVAL, progress_timer, NULL);

		g_wndProgressDlg = msgbox;
	}
	else
		g_wndProgressDlg = NULL;
}

void ProgressDlg::Close()
{
	//destroy progress window
	if(g_wnd.m_pWidget)	//no need if doing export from cmd line
	{
		//kill start timer
		g_bStartProgress = false;
		if(m_nStartTimer > 0)
			g_source_remove (m_nStartTimer);
		m_nStartTimer = 0;

		if(m_pDialog){
			if(m_nProgressTimer > 0)
				g_source_remove (m_nProgressTimer);
			m_nProgressTimer = 0;
			g_progress1 = NULL;
			gtk_widget_destroy (m_pDialog);
			m_pDialog = NULL;
			g_wndProgressDlg = NULL;
		}
	}
}

void ProgressDlg::OnCancel()
{
	m_bCanceled = true;
	Close();
}

gboolean progress_start_timer(gpointer data)
{
	if(g_bStartProgress)
	{
		ProgressDlg	*pDlg = (ProgressDlg *)data;
		
		pDlg->DoCreateWindow();

		//this is a one time timer, destroy it now
		if(pDlg->m_nStartTimer > 0)
			g_source_remove (pDlg->m_nStartTimer);
		pDlg->m_nStartTimer = 0;

	}

	return FALSE;
}

gboolean progress_timer(gpointer data)
{
	//TOFIX optimize, show only if hidden, hide only if visible
	if(g_bMsgBox){
		//hide progress dialog while waiting soem message box to finish
		gtk_widget_hide(g_wndProgressDlg);
	}
	else
	{
		//ensure progress dialog is visible and pulse the progress
		if(g_wndProgressDlg) gtk_widget_show(g_wndProgressDlg);
		if(g_progress1){
			if(ProgressDlg::m_nTotalSize > 0)	//TOFIX what if two loads at te same time ? not static!??
			{
				gtk_progress_set_percentage(GTK_PROGRESS(g_progress1), ((double)ProgressDlg::m_nCurrentSize)/ProgressDlg::m_nTotalSize);
			}
			else
				gtk_progress_bar_pulse(GTK_PROGRESS_BAR(g_progress1));
		}
	}
	return TRUE;
}

void on_abort_clicked (GtkButton *button, gpointer user_data)
{
	ProgressDlg *pDlg = (ProgressDlg *)user_data;
	pDlg->OnCancel();
}
