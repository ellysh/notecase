////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Window class for About dialog
////////////////////////////////////////////////////////////////////////////

#include "AboutDlg.h"
#include "support.h"
#include "gui/GuiLanguage.h"
#include "ExecuteFile.h"
#include "config.h"

#include "../res/notecase.xpm"

#ifdef _WIN32
 #include <windows.h>
#endif

extern GtkWidget *window1;
extern GuiLanguage g_lang;

static gboolean on_website_link_clicked (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
static gboolean on_mail_link_clicked (GtkWidget *widget, GdkEventButton *event, gpointer user_data);

AboutDlg::AboutDlg()
{
	Create();
}

AboutDlg::~AboutDlg()
{
	Destroy();
	gtk_window_present(GTK_WINDOW(window1)); //activate main window
}

void AboutDlg::Create()
{
	m_pDialog = create_dialog (window1);

}

GtkWidget* AboutDlg::create_dialog (GtkWidget* parent)
{
	GtkWidget *dialog;
	GtkWidget *label1;
	GtkWidget *label2;
	GtkWidget *label3;
	GtkWidget *evbox1;
	GtkWidget *label4;
	GtkWidget *evbox2;
	GtkWidget *table;
	GtkWidget *image1;
	GtkWidget *hbox3;

	dialog = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(dialog), _("About Notecase"));
	gtk_dialog_set_has_separator(GTK_DIALOG(dialog), false);
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), TRUE);
#endif
	gtk_window_set_skip_pager_hint (GTK_WINDOW (dialog), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW (dialog), GTK_WINDOW(window1));   //set parent
#if GTK_CHECK_VERSION(2,4,0) //new API TOFIX set proper version
	#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
	  //gtk_window_set_keep_above(GTK_WINDOW (dialog), TRUE);
	#endif
#endif
	//gtk_window_set_default_size (GTK_WINDOW(dialog), 340, 145); //set default window size
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

	hbox3 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox3);
	//gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), hbox3);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox), hbox3, FALSE, TRUE, 10);

	image1 = gtk_image_new ();
	gtk_widget_show (image1);
	gtk_misc_set_alignment(GTK_MISC(image1), (float)0.7, (float)0.5); //alignment icon
	gtk_box_pack_start (GTK_BOX (hbox3), image1, TRUE, TRUE, 10);

	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&notecase_xpm);
	gtk_image_set_from_pixbuf (GTK_IMAGE(image1), pixbuf);
	g_object_unref (G_OBJECT (pixbuf));

	//version string
	std::string strMsg("<b>");
	strMsg += APP_NAME_STR;
	strMsg += _(" - Version ");
	strMsg += APP_VER_STR;
	strMsg += "</b>";

	label1 = gtk_label_new (strMsg.c_str());
	gtk_label_set_use_markup(GTK_LABEL(label1), true);
	gtk_misc_set_alignment(GTK_MISC(label1), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox3), label1, TRUE, TRUE, 10);

	table = gtk_table_new(2, 2, true);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), table);

	//link to the web site
	std::string strMessage;
	strMessage += _("Website:");
	strMessage += " <span foreground=\"blue\"underline=\"single\">http://notecase.sourceforge.net</span>";

	label2 = gtk_label_new (strMessage.c_str());
	gtk_label_set_use_markup(GTK_LABEL(label2), true);
	gtk_misc_set_alignment(GTK_MISC(label2), 0.5, 0.5);

	//put in eventbox to handle click
	evbox1 = gtk_event_box_new();
	gtk_event_box_set_above_child (GTK_EVENT_BOX(evbox1), true);
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(evbox1), false);
	gtk_container_add(GTK_CONTAINER(evbox1), label2);
	gtk_table_attach(GTK_TABLE(table), evbox1, 0, 2, 0, 1, (GtkAttachOptions)(GTK_EXPAND|GTK_SHRINK|GTK_FILL), (GtkAttachOptions)0, 0, 1);

	//mail address link
	strMessage  = _("Report bugs to:");
	strMessage += " <span foreground=\"blue\"underline=\"single\">miroslav.rajcic@inet.hr</span>";

	label4 = gtk_label_new (strMessage.c_str());
	gtk_label_set_use_markup(GTK_LABEL(label4), true);
	gtk_misc_set_alignment(GTK_MISC(label4), 0.5, 0.5);

	//put in eventbox to handle click
	evbox2 = gtk_event_box_new();
	gtk_event_box_set_above_child (GTK_EVENT_BOX(evbox2), true);
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(evbox2), false);
	gtk_container_add(GTK_CONTAINER(evbox2), label4);
	gtk_table_attach(GTK_TABLE(table), evbox2, 0, 2, 1, 2, (GtkAttachOptions)(GTK_EXPAND|GTK_SHRINK|GTK_FILL), (GtkAttachOptions)0, 0, 1);

	//locale info
	strMsg = _("Locale: ");
	strMsg += "<span foreground=\"blue\">";
	strMsg += g_lang.GetLocale();
	strMsg += "</span>";

	label3 = gtk_label_new (strMsg.c_str());
	gtk_label_set_use_markup(GTK_LABEL(label3), true);
	gtk_misc_set_alignment(GTK_MISC(label3), 0.5, 0.5);
	gtk_table_attach(GTK_TABLE(table), label3, 0, 2, 2, 3, (GtkAttachOptions)(GTK_EXPAND|GTK_SHRINK|GTK_FILL), (GtkAttachOptions)0, 0, 1);

#ifdef _NOKIA_MAEMO
	GtkWidget *dialog_action_area6 = GTK_DIALOG (dialog)->action_area;
	gtk_widget_show (dialog_action_area6);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area6), GTK_BUTTONBOX_END);

	GtkWidget *cancelbutton9 = gtk_button_new_with_mnemonic (_("Close"));
	gtk_widget_show (cancelbutton9);
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), cancelbutton9, GTK_RESPONSE_CANCEL);
	GTK_WIDGET_SET_FLAGS (cancelbutton9, GTK_CAN_DEFAULT);
#endif

	g_signal_connect(evbox1, "button-press-event",	G_CALLBACK (on_website_link_clicked), this);
	g_signal_connect(evbox2, "button-press-event",	G_CALLBACK (on_mail_link_clicked), this);


	gtk_widget_show_all (dialog);
	return dialog;
}

gboolean  on_website_link_clicked (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
#ifdef _WIN32
	ShellExecute(NULL, "open", "http://notecase.sourceforge.net", "", "", SW_SHOW);
#else
	ExecuteFile("http://notecase.sourceforge.net", "", "", NULL);
#endif

	return true;
}

gboolean on_mail_link_clicked (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
#ifdef _WIN32
	ShellExecute(NULL, "open", "mailto:miroslav.rajcic@inet.hr", "", "", SW_SHOW);
#else
	ExecuteFile("mailto:miroslav.rajcic@inet.hr", "", "", NULL);
#endif

	return true;
}
