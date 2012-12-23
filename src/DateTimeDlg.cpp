////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Window class for Date/Time insert dialog
////////////////////////////////////////////////////////////////////////////

#include "DateTimeDlg.h"
#include "support.h"
#include "lib/IniFile.h"
#include <gdk/gdkkeysyms.h>

extern IniFile g_objIni;
extern GtkWidget *window1;
static void on_format_combo_selected (GtkComboBox *widget, gpointer user_data);
static void on_ok_clicked (GtkMenuItem *menuitem, gpointer user_data);
static gboolean on_combo_key_press (GtkWidget *widget, GdkEventKey *event, gpointer user_data);

DateTimeDlg::DateTimeDlg()
{
	Create();
}

DateTimeDlg::~DateTimeDlg()
{
	Destroy();
	gtk_window_present(GTK_WINDOW(window1)); //activate main window
}

void DateTimeDlg::Create()
{
	m_pDialog = create_date_insert_popup (window1);

	//fill target combo
	GtkWidget *cbo3	= lookup_widget(m_pDialog, "comboboxentry3");
	gtk_combo_box_append_text(GTK_COMBO_BOX(cbo3), "YYYY-MM-DD");
	gtk_combo_box_append_text(GTK_COMBO_BOX(cbo3), "YYYY-MM-DD hh:mm:ss");
	gtk_combo_box_append_text(GTK_COMBO_BOX(cbo3), "DD.MM.YYYY");
	gtk_combo_box_append_text(GTK_COMBO_BOX(cbo3), "DD.MM.YYYY hh:mm:ss");
	gtk_combo_box_append_text(GTK_COMBO_BOX(cbo3), "hh:mm:ss");

	//set selected date/time format
	int nDateTimeFmt = 0;
	g_objIni.GetValue("Display", "DateTimeFormat", nDateTimeFmt, 0);
	if(nDateTimeFmt < 0 || nDateTimeFmt > 3)
		nDateTimeFmt = 0;
	gtk_combo_box_set_active(GTK_COMBO_BOX(cbo3), nDateTimeFmt);
}

GtkWidget* DateTimeDlg::create_date_insert_popup (GtkWidget* parent)
{
	GtkWidget *date_insert_popup;
	GtkWidget *dialog_vbox9;
	GtkWidget *vbox11;
	GtkWidget *label36;
	GtkWidget *comboboxentry3;
	GtkWidget *label37;
	GtkWidget *dialog_action_area9;
	GtkWidget *cancelbutton9;
	GtkWidget *okbutton9;

	date_insert_popup = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (date_insert_popup), _("Insert Date/Time"));
	gtk_window_set_modal (GTK_WINDOW (date_insert_popup), TRUE);
	gtk_window_set_type_hint (GTK_WINDOW (date_insert_popup), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_window_set_destroy_with_parent (GTK_WINDOW (date_insert_popup), TRUE);
#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (date_insert_popup), TRUE);
#endif
	gtk_window_set_skip_pager_hint (GTK_WINDOW (date_insert_popup), TRUE);
	gtk_window_set_type_hint (GTK_WINDOW (date_insert_popup), GDK_WINDOW_TYPE_HINT_DIALOG);
	if(parent)
		gtk_window_set_transient_for(GTK_WINDOW (date_insert_popup), GTK_WINDOW(parent));   //set parent
#if GTK_CHECK_VERSION(2,4,0) //new API TOFIX set proper version
	#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
	  //gtk_window_set_keep_above(GTK_WINDOW (date_insert_popup), TRUE);
	#endif
#endif
	gtk_widget_realize(date_insert_popup);
	gdk_window_set_decorations(date_insert_popup->window, (GdkWMDecoration)(GDK_DECOR_BORDER|GDK_DECOR_TITLE));

	dialog_vbox9 = GTK_DIALOG (date_insert_popup)->vbox;
	gtk_widget_show (dialog_vbox9);

	vbox11 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox11);
	gtk_box_pack_start (GTK_BOX (dialog_vbox9), vbox11, TRUE, TRUE, 0);

	label36 = gtk_label_new (_("Select date/time format:"));
	gtk_widget_show (label36);
	gtk_box_pack_start (GTK_BOX (vbox11), label36, FALSE, FALSE, 0);
	gtk_misc_set_alignment (GTK_MISC (label36), 0, 0.5);

	comboboxentry3 = gtk_combo_box_new_text ();
	gtk_widget_show (comboboxentry3);
	gtk_box_pack_start (GTK_BOX (vbox11), comboboxentry3, FALSE, TRUE, 0);
	gtk_entry_set_activates_default(GTK_ENTRY (gtk_bin_get_child (GTK_BIN (comboboxentry3))), TRUE);
	g_signal_connect (G_OBJECT (comboboxentry3), "key-press-event", G_CALLBACK (on_combo_key_press), this);

	label37 = gtk_label_new (_("value"));
	gtk_widget_show (label37);
	gtk_box_pack_start (GTK_BOX (vbox11), label37, FALSE, TRUE, 0);

	dialog_action_area9 = GTK_DIALOG (date_insert_popup)->action_area;
	gtk_widget_show (dialog_action_area9);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area9), GTK_BUTTONBOX_END);

	cancelbutton9 = gtk_button_new_from_stock ("gtk-cancel");
	gtk_widget_show (cancelbutton9);
	gtk_dialog_add_action_widget (GTK_DIALOG (date_insert_popup), cancelbutton9, GTK_RESPONSE_CANCEL);
	GTK_WIDGET_SET_FLAGS (cancelbutton9, GTK_CAN_DEFAULT);

	okbutton9 = gtk_button_new_from_stock ("gtk-ok");
	gtk_widget_show (okbutton9);
	gtk_container_add (GTK_CONTAINER (dialog_action_area9), okbutton9);
	GTK_WIDGET_SET_FLAGS (okbutton9, GTK_CAN_DEFAULT);

	g_signal_connect(comboboxentry3, "changed", G_CALLBACK (on_format_combo_selected), this);
	g_signal_connect(okbutton9, "clicked",	G_CALLBACK (on_ok_clicked), this);

	/* Store pointers to all widgets, for use by lookup_widget(). */
	GLADE_HOOKUP_OBJECT_NO_REF (date_insert_popup, date_insert_popup, "date_insert_popup");
	GLADE_HOOKUP_OBJECT_NO_REF (date_insert_popup, dialog_vbox9, "dialog_vbox9");
	GLADE_HOOKUP_OBJECT (date_insert_popup, vbox11, "vbox11");
	GLADE_HOOKUP_OBJECT (date_insert_popup, label36, "label36");
	GLADE_HOOKUP_OBJECT (date_insert_popup, comboboxentry3, "comboboxentry3");
	GLADE_HOOKUP_OBJECT (date_insert_popup, label37, "label37");
	GLADE_HOOKUP_OBJECT_NO_REF (date_insert_popup, dialog_action_area9, "dialog_action_area9");
	GLADE_HOOKUP_OBJECT (date_insert_popup, cancelbutton9, "cancelbutton9");
	GLADE_HOOKUP_OBJECT (date_insert_popup, okbutton9, "okbutton9");

	gtk_widget_grab_focus (comboboxentry3);
	gtk_widget_grab_default (okbutton9);

	return date_insert_popup;
}

void on_format_combo_selected (GtkComboBox *widget, gpointer user_data)
{
	DateTimeDlg *pDlg = (DateTimeDlg *)user_data;
	pDlg->OnFormatChanged();
}

void DateTimeDlg::OnFormatChanged()
{
	GtkWidget *cbo3 = lookup_widget(m_pDialog, "comboboxentry3");
	int nDateTimeFmt = gtk_combo_box_get_active(GTK_COMBO_BOX(cbo3));
	m_strValue = GetValueForFormat(nDateTimeFmt);

	//update label:
	GtkWidget *label37	= lookup_widget(m_pDialog, "label37");
	gtk_label_set_text(GTK_LABEL(label37), m_strValue.c_str());
}

std::string DateTimeDlg::GetValueForFormat(int nDateTimeFmt)
{
	std::string strDateTimeValue;
	char szBuffer[200];
	time_t nTime = time(NULL);
	tm *pCurTm = localtime(&nTime);

	//format date/time using settings
	switch(nDateTimeFmt){
	case 1:		//"YYYY-MM-DD hh:mm:ss"
		sprintf(szBuffer, "%04d-%02d-%02d %02d:%02d:%02d",
			pCurTm->tm_year + 1900,
			pCurTm->tm_mon + 1,
			pCurTm->tm_mday,
			pCurTm->tm_hour,
			pCurTm->tm_min,
			pCurTm->tm_sec);
		break;
	case 2:		//"DD.MM.YYYY"
		sprintf(szBuffer, "%02d.%02d.%04d",
			pCurTm->tm_mday,
			pCurTm->tm_mon + 1,
			pCurTm->tm_year + 1900);
		break;
	case 3:		//"DD.MM.YYYY hh:mm:ss"
		sprintf(szBuffer, "%02d.%02d.%04d %02d:%02d:%02d",
			pCurTm->tm_mday,
			pCurTm->tm_mon + 1,
			pCurTm->tm_year + 1900,
			pCurTm->tm_hour,
			pCurTm->tm_min,
			pCurTm->tm_sec);
		break;
	case 4:	//"hh:mm:ss"
		sprintf(szBuffer, "%02d:%02d:%02d",
			pCurTm->tm_hour,
			pCurTm->tm_min,
			pCurTm->tm_sec);
		break;
	default:	//"YYYY-MM-DD"
		sprintf(szBuffer, "%04d-%02d-%02d",
			pCurTm->tm_year + 1900,
			pCurTm->tm_mon + 1,
			pCurTm->tm_mday);
	}

	strDateTimeValue = szBuffer;
	return strDateTimeValue;
}

void on_ok_clicked (GtkMenuItem *menuitem, gpointer user_data)
{
	DateTimeDlg *pDlg = (DateTimeDlg *)user_data;

	//store date/time selection to ini
	GtkWidget *cbo3	= lookup_widget(pDlg->m_pDialog, "comboboxentry3");
	int nDateTimeFmt = gtk_combo_box_get_active(GTK_COMBO_BOX(cbo3));
	g_objIni.SetValue("Display", "DateTimeFormat", nDateTimeFmt);

	gtk_dialog_response(GTK_DIALOG(pDlg->m_pDialog), GTK_RESPONSE_OK);
}

gboolean on_combo_key_press (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    if (event->keyval == GDK_Return){
		on_ok_clicked (NULL, user_data);
        return TRUE;    // don't let the box "drop down"
    }

	return FALSE; // propogate event
}
