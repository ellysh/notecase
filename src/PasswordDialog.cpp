////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Window class for Password input
////////////////////////////////////////////////////////////////////////////

#include "PasswordDialog.h"
#include "support.h"
#include "lib/IniFile.h"
#include "../res/key.xpm"
#include "config.h"

extern GtkWidget *window1;
extern IniFile g_objIni;

int gtkMessageBox(const char *szText, int nButtons = GTK_BUTTONS_OK, int nIcon = GTK_MESSAGE_INFO);
static void on_ok_clicked(GtkButton *button, gpointer user_data);
static void on_show_password_checked (GtkMenuItem *menuitem, gpointer user_data);

PasswordDialog::PasswordDialog(int nMode, int nTries)
{
	m_nDialogMode	= nMode;
	m_nNumTries		= nTries;
	m_nEntryCount	= 0;

	Create();

	//do we need to show password state ?
	bool bShowPass;
	if(g_objIni.GetValue("Other", "ShowPassword", bShowPass) && bShowPass){
		GtkWidget *checkbutton1 = lookup_widget(m_pDialog, "checkbutton1");
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(checkbutton1), TRUE);
	}
}

PasswordDialog::~PasswordDialog()
{
}

void PasswordDialog::Destroy()
{
	//store "show password" state
	GtkWidget *checkbutton1 = lookup_widget(m_pDialog, "checkbutton1");
	bool bShowPass = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(checkbutton1)) > 0;
	g_objIni.SetValue("Other", "ShowPassword", bShowPass);

	Dialog::Destroy();

	gtk_window_present(GTK_WINDOW(window1)); //activate main window
}

void PasswordDialog::Create()
{
	m_pDialog = create_password_dialog (window1);

	GtkWidget *okbutton1 = lookup_widget(m_pDialog, "okbutton1");
	g_signal_connect(okbutton1, "clicked", G_CALLBACK (on_ok_clicked), this);
}

const char *PasswordDialog::GetPassword()
{
	const char *szPass = NULL;

	//get new password
	if(MODE_PASS_CHANGE == m_nDialogMode)
	{
		GtkWidget *entry2 = lookup_widget(m_pDialog, "entry2");
		szPass = gtk_entry_get_text (GTK_ENTRY(entry2));
	}
	else
	{
		GtkWidget *entry1 = lookup_widget(m_pDialog, "entry1");
		szPass = gtk_entry_get_text (GTK_ENTRY(entry1));
	}

	return (NULL != szPass)? szPass : "";
}

GtkWidget* PasswordDialog::create_password_dialog (GtkWidget* parent1)
{
	GtkWidget *dialog1;
	GtkWidget *dialog_vbox1;
	GtkWidget *vbox1;
	GtkWidget *hbox1;
	GtkWidget *label1;
	GtkWidget *label2;
	GtkWidget *label3;
	GtkWidget *entry1;
	GtkWidget *entry2;
	GtkWidget *entry3;
	GtkWidget *dialog_action_area1;
	GtkWidget *cancelbutton1;
	GtkWidget *okbutton1;
	GtkWidget *checkbutton1;

	dialog1 = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (dialog1), _("Password"));
	gtk_window_set_modal (GTK_WINDOW (dialog1), TRUE);
	gtk_window_set_skip_pager_hint (GTK_WINDOW (dialog1), TRUE);
#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog1), TRUE);
#endif
	gtk_window_set_type_hint (GTK_WINDOW (dialog1), GDK_WINDOW_TYPE_HINT_DIALOG);
	if(parent1)
		gtk_window_set_transient_for(GTK_WINDOW (dialog1), GTK_WINDOW(parent1));   //set parent

#if GTK_CHECK_VERSION(2,4,0) //new API TOFIX set proper version
  #ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
	//gtk_window_set_keep_above(GTK_WINDOW (dialog1), TRUE);
  #endif
#endif
	gtk_window_set_position(GTK_WINDOW (dialog1), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_default_size(GTK_WINDOW(dialog1), 300, -1);

	dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
	gtk_widget_show (dialog_vbox1);

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox1);
	gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox1, TRUE, TRUE, 0);

	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, FALSE, 0);

	GdkPixbuf* pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&key);
	GtkWidget *button3 = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show (button3);
	gtk_box_pack_start (GTK_BOX (hbox1), button3, FALSE, FALSE, 0);

	label1 = gtk_label_new (_("Please enter the password:"));
	gtk_widget_show (label1);
	gtk_box_pack_start (GTK_BOX (hbox1), label1, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);
	gtk_misc_set_padding (GTK_MISC (label1), 7, 5);

	entry1 = gtk_entry_new ();
	gtk_widget_show (entry1);
	gtk_box_pack_start (GTK_BOX (vbox1), entry1, FALSE, FALSE, 0);
	gtk_entry_set_visibility (GTK_ENTRY (entry1), FALSE);
	gtk_entry_set_activates_default (GTK_ENTRY (entry1), TRUE);
#ifdef _NOKIA_MAEMO
	//maemo capitalizes entered text by default - disable that
	g_object_set(G_OBJECT(entry1), "autocap", FALSE, NULL);
#endif

	if(MODE_PASS_CHANGE == m_nDialogMode)
	{
		label3 = gtk_label_new (_("Please enter the new password:"));
		gtk_widget_show (label3);
		gtk_box_pack_start (GTK_BOX (vbox1), label3, FALSE, FALSE, 0);
		gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_LEFT);
		gtk_misc_set_padding (GTK_MISC (label3), 7, 5);

		entry3 = gtk_entry_new ();
		gtk_widget_show (entry3);
		gtk_box_pack_start (GTK_BOX (vbox1), entry3, FALSE, FALSE, 0);
		gtk_entry_set_visibility (GTK_ENTRY (entry3), FALSE);
		gtk_entry_set_activates_default (GTK_ENTRY (entry3), TRUE);
	#ifdef _NOKIA_MAEMO
		//maemo capitalizes entered text by default - disable that
		g_object_set(G_OBJECT(entry3), "autocap", FALSE, NULL);
	#endif
	}
	else
		entry3 = NULL;


	if(MODE_PASS_NEW == m_nDialogMode || MODE_PASS_CHANGE == m_nDialogMode)
	{
		label2 = gtk_label_new (_("Please enter the new password (once more):"));
		gtk_widget_show (label2);
		gtk_box_pack_start (GTK_BOX (vbox1), label2, FALSE, FALSE, 0);
		gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_LEFT);
		gtk_misc_set_padding (GTK_MISC (label2), 7, 5);

		entry2 = gtk_entry_new ();
		gtk_widget_show (entry2);
		gtk_box_pack_start (GTK_BOX (vbox1), entry2, FALSE, FALSE, 0);
		gtk_entry_set_visibility (GTK_ENTRY (entry2), FALSE);
		gtk_entry_set_activates_default (GTK_ENTRY (entry2), TRUE);
	#ifdef _NOKIA_MAEMO
		//maemo capitalizes entered text by default - disable that
		g_object_set(G_OBJECT(entry2), "autocap", FALSE, NULL);
	#endif
	}
	else
		entry2 = NULL;

	checkbutton1 = gtk_check_button_new_with_mnemonic (_("_Show passwords"));
	gtk_widget_show (checkbutton1);
	gtk_box_pack_start (GTK_BOX (vbox1), checkbutton1, FALSE, FALSE, 0);
	g_signal_connect (checkbutton1, "clicked",  G_CALLBACK (on_show_password_checked), this);

	dialog_action_area1 = GTK_DIALOG (dialog1)->action_area;
	gtk_widget_show (dialog_action_area1);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

	cancelbutton1 = gtk_button_new_from_stock ("gtk-cancel");
	gtk_widget_show (cancelbutton1);
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), cancelbutton1, GTK_RESPONSE_CANCEL);
	GTK_WIDGET_SET_FLAGS (cancelbutton1, GTK_CAN_DEFAULT);

	okbutton1 = gtk_button_new_from_stock ("gtk-ok");
	gtk_widget_show (okbutton1);
	gtk_container_add (GTK_CONTAINER(dialog_action_area1), okbutton1);
	GTK_WIDGET_SET_FLAGS (okbutton1, GTK_CAN_DEFAULT);

	/* Store pointers to all widgets, for use by lookup_widget(). */
	GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog1, "dialog1");
	GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_vbox1, "dialog_vbox1");
	GLADE_HOOKUP_OBJECT (dialog1, vbox1, "vbox1");
	GLADE_HOOKUP_OBJECT (dialog1, label1, "label1");
	GLADE_HOOKUP_OBJECT (dialog1, entry1, "entry1");

	if(MODE_PASS_NEW == m_nDialogMode || MODE_PASS_CHANGE == m_nDialogMode)
		GLADE_HOOKUP_OBJECT (dialog1, entry2, "entry2");
	if(MODE_PASS_CHANGE == m_nDialogMode)
		GLADE_HOOKUP_OBJECT (dialog1, entry3, "entry3");

	GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");
	GLADE_HOOKUP_OBJECT (dialog1, cancelbutton1, "cancelbutton1");
	GLADE_HOOKUP_OBJECT (dialog1, okbutton1, "okbutton1");
	GLADE_HOOKUP_OBJECT (dialog1, checkbutton1, "checkbutton1");

	gtk_widget_grab_default (okbutton1);
	return dialog1;
}

void PasswordDialog::SetLabel(const char *szText)
{
	GtkWidget *label1 = lookup_widget(m_pDialog, "label1");
	gtk_label_set_text(GTK_LABEL(label1), szText);
}

void on_ok_clicked (GtkButton *button, gpointer user_data)
{
	PasswordDialog *pDlg = (PasswordDialog *)user_data;
	GtkWidget *entry1 = lookup_widget(pDlg->m_pDialog, "entry1");

	//TOFIX optimize and clean
	if(MODE_PASS_NEW == pDlg->m_nDialogMode)
	{
		std::string strEntry = pDlg->GetPassword();
		if(strEntry.size() < 1){
			gtkMessageBox(_("Password must not be empty!"));
			gtk_widget_grab_focus (entry1);
			return;
		}

		GtkWidget *entry2 = lookup_widget(pDlg->m_pDialog, "entry2");

		//compare passwords
		std::string strEntry1 = gtk_entry_get_text (GTK_ENTRY(entry1)) ? gtk_entry_get_text (GTK_ENTRY(entry1)) : "";
		std::string strEntry2 = gtk_entry_get_text (GTK_ENTRY(entry2)) ? gtk_entry_get_text (GTK_ENTRY(entry2)) : "";
		if(strEntry1 != strEntry2)
		{
			gtkMessageBox(_("Passwords do not match! Check for typing errors."));
			return;
		}
		else
		{
			//terminate with OK
			gtk_dialog_response(GTK_DIALOG(pDlg->m_pDialog), GTK_RESPONSE_OK);
		}
	}
	else if(MODE_PASS_CHANGE == pDlg->m_nDialogMode)
	{
		//check if old password is correct
		std::string strOldPass = gtk_entry_get_text (GTK_ENTRY(entry1)) ? gtk_entry_get_text (GTK_ENTRY(entry1)) : "";;

		if(strOldPass != pDlg->m_strOldPass){
			gtkMessageBox(_("Old password was not correct!"));
			gtk_dialog_response(GTK_DIALOG(pDlg->m_pDialog), GTK_RESPONSE_CANCEL); //terminate with error ?
			return;
		}

		GtkWidget *entry2 = lookup_widget(pDlg->m_pDialog, "entry2");
		GtkWidget *entry3 = lookup_widget(pDlg->m_pDialog, "entry3");

		//compare passwords
		std::string strEntry2 = gtk_entry_get_text (GTK_ENTRY(entry2)) ? gtk_entry_get_text (GTK_ENTRY(entry2)) : "";
		std::string strEntry3 = gtk_entry_get_text (GTK_ENTRY(entry3)) ? gtk_entry_get_text (GTK_ENTRY(entry3)) : "";
		if(strEntry2 != strEntry3)
		{
			gtkMessageBox(_("New passwords do not match! Check for typing errors."));
			return;
		}

		//terminate with OK
		gtk_dialog_response(GTK_DIALOG(pDlg->m_pDialog), GTK_RESPONSE_OK);
	}
	else{
		std::string strEntry = pDlg->GetPassword();
		if(strEntry.size() < 1){
			gtkMessageBox(_("Password must not be empty!"));
			gtk_widget_grab_focus (entry1);
			return;
		}

		//terminate with OK
		gtk_dialog_response(GTK_DIALOG(pDlg->m_pDialog), GTK_RESPONSE_OK);
	}
}

void on_show_password_checked (GtkMenuItem *menuitem, gpointer user_data)
{
	PasswordDialog *pDlg = (PasswordDialog *)user_data;

	GtkWidget *checkbutton1 = lookup_widget(pDlg->m_pDialog, "checkbutton1");
	GtkWidget *entry1 = lookup_widget(pDlg->m_pDialog, "entry1");
	GtkWidget *entry2 = NULL;
	GtkWidget *entry3 = NULL;
	if(MODE_PASS_NEW == pDlg->m_nDialogMode || MODE_PASS_CHANGE == pDlg->m_nDialogMode)
		entry2 = lookup_widget(pDlg->m_pDialog, "entry2");
	if(MODE_PASS_CHANGE == pDlg->m_nDialogMode)
		entry3 = lookup_widget(pDlg->m_pDialog, "entry3");

	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(checkbutton1))){
		gtk_entry_set_visibility (GTK_ENTRY (entry1), TRUE);
		if(MODE_PASS_NEW == pDlg->m_nDialogMode || MODE_PASS_CHANGE == pDlg->m_nDialogMode)
			gtk_entry_set_visibility (GTK_ENTRY (entry2), TRUE);
		if(MODE_PASS_CHANGE == pDlg->m_nDialogMode)
			gtk_entry_set_visibility (GTK_ENTRY (entry3), TRUE);
	}
	else{
		gtk_entry_set_visibility (GTK_ENTRY (entry1), FALSE);
		if(MODE_PASS_NEW == pDlg->m_nDialogMode || MODE_PASS_CHANGE == pDlg->m_nDialogMode)
			gtk_entry_set_visibility (GTK_ENTRY (entry2), FALSE);
		if(MODE_PASS_CHANGE == pDlg->m_nDialogMode)
			gtk_entry_set_visibility (GTK_ENTRY (entry3), FALSE);
	}
}
