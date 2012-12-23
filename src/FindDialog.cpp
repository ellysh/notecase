////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Window class for Find dialog
////////////////////////////////////////////////////////////////////////////

#include "FindDialog.h"
#include "support.h"

extern GtkWidget *window1;
static GtkWidget* create_find_dialog (GtkWidget* parent=NULL);

FindDialog::FindDialog()
{
	Create();
}

FindDialog::~FindDialog()
{
	gtk_window_present(GTK_WINDOW(window1)); //activate main window
}

void FindDialog::Create()
{
	m_pDialog = create_find_dialog (window1);

	//fill target combo
	GtkWidget *cbo3	= lookup_widget(m_pDialog, "comboboxentry3");
	gtk_combo_box_append_text(GTK_COMBO_BOX(cbo3), _("Node title/contents"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(cbo3), _("Node tags"));
	gtk_combo_box_set_active(GTK_COMBO_BOX(cbo3), 0);
}

GtkWidget* create_find_dialog (GtkWidget* parent)
{
	GtkWidget *Find;
	GtkWidget *dialog_vbox4;
	GtkWidget *table2;
	GtkWidget *label9;
	GtkWidget *entry1;
	GtkWidget *label14;
	GtkWidget *label10;
	GtkWidget *comboboxentry3;
	GtkWidget *radiobutton8;
	GSList *radiobutton8_group = NULL;
	GtkWidget *radiobutton9;
	GtkWidget *checkbutton14;
	GtkWidget *dialog_action_area4;
	GtkWidget *cancelbutton4;
	GtkWidget *okbutton4;

	Find = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (Find), _("Find"));
	gtk_window_set_modal (GTK_WINDOW (Find), TRUE);
	gtk_window_set_destroy_with_parent (GTK_WINDOW (Find), TRUE);
	gtk_window_set_type_hint (GTK_WINDOW (Find), GDK_WINDOW_TYPE_HINT_DIALOG);
	if(parent)
		gtk_window_set_transient_for(GTK_WINDOW (Find), GTK_WINDOW(parent));   //set parent

	dialog_vbox4 = GTK_DIALOG (Find)->vbox;
	gtk_widget_show (dialog_vbox4);

	table2 = gtk_table_new (4, 2, FALSE);
	gtk_widget_show (table2);
	gtk_box_pack_start (GTK_BOX (dialog_vbox4), table2, TRUE, TRUE, 0);

	label9 = gtk_label_new (_("Text:"));
	gtk_widget_show (label9);
	gtk_table_attach (GTK_TABLE (table2), label9, 0, 1, 0, 1,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label9), 0, 0.5);

	entry1 = gtk_entry_new ();
	gtk_widget_show (entry1);
	gtk_table_attach (GTK_TABLE (table2), entry1, 1, 2, 0, 1,
		(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_activates_default (GTK_ENTRY (entry1), TRUE);

	label10 = gtk_label_new (_("Search target:"));
	gtk_widget_show (label10);
	gtk_table_attach (GTK_TABLE (table2), label10, 0, 1, 1, 2,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label10), 0, 0.5);

	comboboxentry3 = gtk_combo_box_new_text ();
	gtk_widget_show (comboboxentry3);
	gtk_table_attach (GTK_TABLE (table2), comboboxentry3, 1, 2, 1, 2,
		(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);

	label14 = gtk_label_new (_("Direction:"));
	//gtk_widget_show (label14);
	gtk_table_attach (GTK_TABLE (table2), label14, 0, 1, 2, 3,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label14), 0, 0.5);

	radiobutton8 = gtk_radio_button_new_with_mnemonic (NULL, _("Up"));
	//gtk_widget_show (radiobutton8);
	gtk_table_attach (GTK_TABLE (table2), radiobutton8, 1, 2, 2, 3,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (radiobutton8), radiobutton8_group);
	radiobutton8_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radiobutton8));

	radiobutton9 = gtk_radio_button_new_with_mnemonic (NULL, _("Down"));
	//gtk_widget_show (radiobutton9);
	gtk_table_attach (GTK_TABLE (table2), radiobutton9, 1, 2, 3, 4,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (radiobutton9), radiobutton8_group);
	radiobutton8_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radiobutton9));

	checkbutton14 = gtk_check_button_new_with_mnemonic (_("Case sensitive"));
	gtk_widget_show (checkbutton14);
	gtk_table_attach (GTK_TABLE (table2), checkbutton14, 1, 2, 4, 5,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);

	dialog_action_area4 = GTK_DIALOG (Find)->action_area;
	gtk_widget_show (dialog_action_area4);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area4), GTK_BUTTONBOX_END);

	cancelbutton4 = gtk_button_new_from_stock ("gtk-cancel");
	gtk_widget_show (cancelbutton4);
	gtk_dialog_add_action_widget (GTK_DIALOG (Find), cancelbutton4, GTK_RESPONSE_CANCEL);
	GTK_WIDGET_SET_FLAGS (cancelbutton4, GTK_CAN_DEFAULT);

	okbutton4 = gtk_button_new_from_stock ("gtk-ok");
	gtk_widget_show (okbutton4);
	gtk_dialog_add_action_widget (GTK_DIALOG (Find), okbutton4, GTK_RESPONSE_OK);
	GTK_WIDGET_SET_FLAGS (okbutton4, GTK_CAN_DEFAULT);

	/* Store pointers to all widgets, for use by lookup_widget(). */
	GLADE_HOOKUP_OBJECT_NO_REF (Find, Find, "Find");
	GLADE_HOOKUP_OBJECT_NO_REF (Find, dialog_vbox4, "dialog_vbox4");
	GLADE_HOOKUP_OBJECT (Find, table2, "table2");
	GLADE_HOOKUP_OBJECT (Find, label9, "label9");
	GLADE_HOOKUP_OBJECT (Find, entry1, "entry1");
	GLADE_HOOKUP_OBJECT (Find, label14, "label14");
	GLADE_HOOKUP_OBJECT (Find, label10, "label10");
	GLADE_HOOKUP_OBJECT (Find, comboboxentry3, "comboboxentry3");
	GLADE_HOOKUP_OBJECT (Find, radiobutton8, "radiobutton8");
	GLADE_HOOKUP_OBJECT (Find, radiobutton9, "radiobutton9");
	GLADE_HOOKUP_OBJECT (Find, checkbutton14, "casesensitive_chk");
	GLADE_HOOKUP_OBJECT_NO_REF (Find, dialog_action_area4, "dialog_action_area4");
	GLADE_HOOKUP_OBJECT (Find, cancelbutton4, "cancelbutton4");
	GLADE_HOOKUP_OBJECT (Find, okbutton4, "okbutton4");

	gtk_widget_grab_focus (entry1);
	gtk_widget_grab_default (okbutton4);

	return Find;
}
