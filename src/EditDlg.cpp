////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Window class for multiline text edit dialog
////////////////////////////////////////////////////////////////////////////

#include "EditDlg.h"
#include "support.h"
#include <gdk/gdkkeysyms.h>
#include "ShortcutsList.h"
#include "lib/IniFile.h"

gboolean date_time_paste_to_text_widget(GtkWidget *widget);

extern GtkWidget *window1;
static void on_ok_clicked (GtkMenuItem *menuitem, gpointer user_data);
static gint entry_keyboard_handler(GtkWidget *widget, GdkEventKey *event);
static GtkWidget *g_editdialog = NULL;
static GtkWidget *g_cancelbtn = NULL;
static GtkWidget *g_editor = NULL;
extern bool g_bCloseDlgOnTextEnter;
extern IniFile g_objIni;
extern ShortcutsList g_shortcuts;
ShortcutDef g_InsDateTime;

EditDlg::EditDlg()
{
	Create();

	int nPos = g_shortcuts.FindByAction(NC_ACTION_EDIT_INSERT_DATETIME);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		g_InsDateTime = g_shortcuts[nPos];
}

EditDlg::~EditDlg()
{
	UpdateData();

	//store dialog size to INI
	gint nPosLeft, nPosTop, nPosWidth, nPosHeight;
	gtk_window_get_position(GTK_WINDOW(g_editdialog), &nPosLeft, &nPosTop);
	gtk_window_get_size(GTK_WINDOW(g_editdialog), &nPosWidth, &nPosHeight);

	g_objIni.SetValue("EditDlg", "LastPos_Left",	nPosLeft);
	g_objIni.SetValue("EditDlg", "LastPos_Top",		nPosTop);
	g_objIni.SetValue("EditDlg", "LastPos_Width",	nPosWidth);
	g_objIni.SetValue("EditDlg", "LastPos_Height",	nPosHeight);

	//destroy
	Destroy();
	gtk_window_present(GTK_WINDOW(window1)); //activate main window
}

void EditDlg::UpdateData()
{
	//update text
	if(m_pDialog){
		GtkWidget *entry1 = lookup_widget(m_pDialog, "entry1");
		GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(entry1));
		GtkTextIter start,end;
		gtk_text_buffer_get_start_iter(buffer, &start);
		gtk_text_buffer_get_end_iter(buffer, &end);
		gchar *pszData = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
		m_strValue = pszData;
		g_free(pszData);
	}
}

void EditDlg::Create()
{
	m_pDialog = create_dialog (window1);
	g_editdialog = m_pDialog;

	//restore dialog size from INI
	gint nPosLeft, nPosTop, nPosWidth, nPosHeight;
	nPosLeft = nPosTop = nPosWidth = nPosHeight = 0;

	g_objIni.GetValue("EditDlg", "LastPos_Left", nPosLeft, -1);
	g_objIni.GetValue("EditDlg", "LastPos_Top", nPosTop, -1);
	g_objIni.GetValue("EditDlg", "LastPos_Width", nPosWidth, -1);
	g_objIni.GetValue("EditDlg", "LastPos_Height", nPosHeight, -1);

	if(nPosLeft >= 0 && nPosTop >= 0)
		gtk_window_move(GTK_WINDOW(g_editdialog), nPosLeft, nPosTop);
	if(nPosWidth >= 0 && nPosHeight >= 0)
	{
		gtk_window_set_default_size(GTK_WINDOW(g_editdialog), nPosWidth, nPosHeight);
		gtk_window_resize(GTK_WINDOW(g_editdialog), nPosWidth, nPosHeight);
	}
}

void EditDlg::SetValue(const char *szText)
{
	GtkWidget *entry1 = lookup_widget(m_pDialog, "entry1");
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(entry1));
	gtk_text_buffer_set_text(buffer, szText, -1);

	//select all
	GtkTextIter start, end;
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_get_end_iter(buffer, &end);
	gtk_text_buffer_select_range(buffer, &start, &end);
}

GtkWidget* EditDlg::create_dialog (GtkWidget* parent)
{
	GtkWidget *dialog;
	GtkWidget *dialog_vbox9;
	GtkWidget *vbox11;
	GtkWidget *entry1;
	GtkWidget *dialog_action_area9;
	GtkWidget *cancelbutton9;
	GtkWidget *okbutton9;
	GtkWidget *scrolledwindow1;

	dialog = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (dialog), _("Edit text"));
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), TRUE);
#endif
	gtk_window_set_skip_pager_hint (GTK_WINDOW (dialog), TRUE);
	gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
	if(parent)
		gtk_window_set_transient_for(GTK_WINDOW (dialog), GTK_WINDOW(parent));   //set parent
#if GTK_CHECK_VERSION(2,4,0) //new API TOFIX set proper version
	#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
	  //gtk_window_set_keep_above(GTK_WINDOW (dialog), TRUE);
	#endif
#endif
	gtk_widget_realize(dialog);
	gdk_window_set_decorations(dialog->window, (GdkWMDecoration)(GDK_DECOR_BORDER|GDK_DECOR_TITLE));
	gtk_dialog_set_has_separator (GTK_DIALOG(dialog), FALSE);

	dialog_vbox9 = GTK_DIALOG (dialog)->vbox;
	gtk_widget_show (dialog_vbox9);

	vbox11 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox11);
	gtk_box_pack_start (GTK_BOX (dialog_vbox9), vbox11, TRUE, TRUE, 0);

	scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindow1);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (vbox11), scrolledwindow1, FALSE, TRUE, 0);

	entry1 =  gtk_text_view_new();
	gtk_widget_show (entry1);
	gtk_container_add (GTK_CONTAINER (scrolledwindow1), entry1);

	dialog_action_area9 = GTK_DIALOG (dialog)->action_area;
	gtk_widget_show (dialog_action_area9);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area9), GTK_BUTTONBOX_END);

	cancelbutton9 = gtk_button_new_from_stock ("gtk-cancel");
	gtk_widget_show (cancelbutton9);
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), cancelbutton9, GTK_RESPONSE_CANCEL);
	GTK_WIDGET_SET_FLAGS (cancelbutton9, GTK_CAN_DEFAULT);

	okbutton9 = gtk_button_new_from_stock ("gtk-ok");
	gtk_widget_show (okbutton9);
	gtk_container_add (GTK_CONTAINER (dialog_action_area9), okbutton9);
	GTK_WIDGET_SET_FLAGS (okbutton9, GTK_CAN_DEFAULT);

	g_signal_connect(okbutton9, "clicked",	G_CALLBACK (on_ok_clicked), this);
	g_signal_connect(entry1,	"key_press_event",  G_CALLBACK (entry_keyboard_handler), this);

	/* Store pointers to all widgets, for use by lookup_widget(). */
	GLADE_HOOKUP_OBJECT_NO_REF (dialog, dialog, "dialog");
	GLADE_HOOKUP_OBJECT_NO_REF (dialog, dialog_vbox9, "dialog_vbox9");
	GLADE_HOOKUP_OBJECT (dialog, vbox11, "vbox11");
	GLADE_HOOKUP_OBJECT (dialog, entry1, "entry1");
	GLADE_HOOKUP_OBJECT_NO_REF (dialog, dialog_action_area9, "dialog_action_area9");
	GLADE_HOOKUP_OBJECT (dialog, cancelbutton9, "cancelbutton9");
	GLADE_HOOKUP_OBJECT (dialog, okbutton9, "okbutton9");

	gtk_widget_grab_focus (entry1);
	gtk_widget_grab_default (okbutton9);

	g_cancelbtn = cancelbutton9;
	g_editor = entry1;

	return dialog;
}

void on_ok_clicked (GtkMenuItem *menuitem, gpointer user_data)
{
	EditDlg *pDlg = (EditDlg *)user_data;
	gtk_dialog_response(GTK_DIALOG(pDlg->m_pDialog), GTK_RESPONSE_OK);
}

gint entry_keyboard_handler(GtkWidget *widget, GdkEventKey *event)
{
	if(g_InsDateTime.Match(event))
	{
		return date_time_paste_to_text_widget(widget);
	}
	else if(event->keyval == GDK_Return)
	{
		if(event->state & GDK_CONTROL_MASK)
		{
			return FALSE;
		}
		else
		{
			if(g_bCloseDlgOnTextEnter){
				gtk_dialog_response(GTK_DIALOG(g_editdialog), GTK_RESPONSE_OK);
				return TRUE;
			}
		}
	}
    else if(event->keyval == GDK_Tab)
	{
		if(event->state & GDK_CONTROL_MASK)
		{
			//insert tab character
			GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(g_editor));
			gtk_text_buffer_insert_at_cursor(buf, "\t", -1);
			return TRUE;
		}
		else
		{
            gtk_widget_grab_focus (g_cancelbtn); // pass focus forward
			return TRUE;
		}
	}

	return FALSE;
}

void EditDlg::SetSelection(int nSelFrom, int nSelLen)
{
	if(nSelFrom < 0)
		return;

	GtkWidget *entry1 = lookup_widget(m_pDialog, "entry1");
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(entry1));

	//select
	GtkTextIter start, end;
	gtk_text_buffer_get_iter_at_offset(buffer, &start, nSelFrom);
	gtk_text_buffer_get_iter_at_offset(buffer, &end, nSelFrom+nSelLen);
	gtk_text_buffer_select_range(buffer, &start, &end);
}

void EditDlg::SetTitle(const char *szText)
{
	gtk_window_set_title (GTK_WINDOW (m_pDialog), szText);
}
