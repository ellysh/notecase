////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Dialog to prepare image for insertion into the document (resize + select format)
////////////////////////////////////////////////////////////////////////////

#include "PixPropertiesDlg.h"
#include "support.h"
#include "callbacks.h"
#include "lib/debug.h"
#include "config.h"

#ifdef _WIN32
 #ifndef __MINGW32__
   #ifndef snprintf
    #define snprintf _snprintf
   #endif
 #endif
#endif

extern GtkWidget *window1;
int gtkMessageBox(const char *szText, int nButtons = GTK_BUTTONS_OK, int nIcon = GTK_MESSAGE_INFO);

static void on_resize_combo_selected (GtkComboBox *widget, gpointer user_data);
static void on_use_png_checked (GtkMenuItem *menuitem, gpointer user_data);

PixPropertiesDlg::PixPropertiesDlg()
{
}

PixPropertiesDlg::~PixPropertiesDlg()
{
	Destroy();
	gtk_window_present(GTK_WINDOW(window1)); //activate main window
}

void PixPropertiesDlg::Create()
{
	m_pDialog = create_node_properties_dialog ();

	GtkWidget *label25 = lookup_widget(m_pDialog, "label25");
	char szBuffer[100];
	sprintf(szBuffer, "%d x %d", m_nOrigWidth, m_nOrigHeight);
	gtk_label_set_text(GTK_LABEL(label25), szBuffer);

	//fill internal icons combo
	GtkWidget *cbo2	= lookup_widget(m_pDialog, "comboboxentry2");
	gtk_combo_box_append_text(GTK_COMBO_BOX(cbo2), "100%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(cbo2), "90%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(cbo2), "80%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(cbo2), "75%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(cbo2), "70%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(cbo2), "60%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(cbo2), "50%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(cbo2), "40%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(cbo2), "30%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(cbo2), "25%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(cbo2), "20%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(cbo2), "10%");

	//select first entry in combo (must not be empty selection)
	gtk_combo_box_set_active(GTK_COMBO_BOX(cbo2), 0);

	RecalcNewSize();
}

GtkWidget* PixPropertiesDlg::create_node_properties_dialog ()
{
	GtkWidget *node_properties_dialog;
	GtkWidget *dialog_vbox5;
	GtkWidget *table4;
	GtkWidget *label21;
	GtkWidget *label25;
	GtkWidget *label26;
	GtkWidget *label27;
	GtkWidget *label28;
	GtkWidget *chk1;
	GtkWidget *comboboxentry2;
	GtkWidget *dialog_action_area5;
	GtkWidget *cancelbutton5;
	GtkWidget *okbutton5;

	node_properties_dialog = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (node_properties_dialog), _("Image Properties"));
	gtk_window_set_type_hint (GTK_WINDOW (node_properties_dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_window_set_modal (GTK_WINDOW (node_properties_dialog), TRUE);
	gtk_window_set_skip_pager_hint (GTK_WINDOW (node_properties_dialog), TRUE);
#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (node_properties_dialog), TRUE);
#endif
	gtk_window_set_position(GTK_WINDOW (node_properties_dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_transient_for(GTK_WINDOW (node_properties_dialog), GTK_WINDOW(window1));   //set parent

	dialog_vbox5 = GTK_DIALOG (node_properties_dialog)->vbox;
	gtk_widget_show (dialog_vbox5);

	table4 = gtk_table_new (2, 2, FALSE);
	gtk_widget_show (table4);
	gtk_box_pack_start (GTK_BOX (dialog_vbox5), table4, TRUE, TRUE, 0);

	label21 = gtk_label_new (_("Original size:"));
	gtk_widget_show (label21);
	gtk_table_attach (GTK_TABLE (table4), label21, 0, 1, 0, 1,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 2, 2);
	gtk_misc_set_alignment (GTK_MISC (label21), 0, 0.5);

	label25 = gtk_label_new (_("unknown"));
	gtk_widget_show (label25);
	gtk_table_attach (GTK_TABLE (table4), label25, 1, 2, 0, 1,
		(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		(GtkAttachOptions) (0), 2, 2);

	label27 = gtk_label_new (_("Selected size:"));
	gtk_widget_show (label27);
	gtk_table_attach (GTK_TABLE (table4), label27, 0, 1, 1, 2,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 2, 2);
	gtk_misc_set_alignment (GTK_MISC (label27), 0, 0.5);

	label28 = gtk_label_new (_("unknown"));
	gtk_widget_show (label28);
	gtk_table_attach (GTK_TABLE (table4), label28, 1, 2, 1, 2,
		(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		(GtkAttachOptions) (0), 2, 2);

	label26 = gtk_label_new (_("Resize to:"));
	gtk_widget_show (label26);
	gtk_table_attach (GTK_TABLE (table4), label26, 0, 1, 2, 3,
		(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		(GtkAttachOptions) (0), 2, 2);
	gtk_misc_set_alignment (GTK_MISC (label26), 0, 0.5);

	comboboxentry2 = gtk_combo_box_new_text ();
	gtk_widget_show (comboboxentry2);
	gtk_table_attach (GTK_TABLE (table4), comboboxentry2, 1, 2, 2, 3,
		(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		(GtkAttachOptions) (GTK_FILL), 2, 2);

	chk1 = gtk_check_button_new_with_mnemonic (_("Store as PNG"));
	gtk_widget_show (chk1);
	gtk_table_attach (GTK_TABLE (table4), chk1, 0, 2, 3, 4,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 2, 2);

	dialog_action_area5 = GTK_DIALOG (node_properties_dialog)->action_area;
	gtk_widget_show (dialog_action_area5);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area5), GTK_BUTTONBOX_END);

	cancelbutton5 = gtk_button_new_from_stock ("gtk-cancel");
	gtk_widget_show (cancelbutton5);
	gtk_dialog_add_action_widget (GTK_DIALOG (node_properties_dialog), cancelbutton5, GTK_RESPONSE_CANCEL);
	GTK_WIDGET_SET_FLAGS (cancelbutton5, GTK_CAN_DEFAULT);

	okbutton5 = gtk_button_new_from_stock ("gtk-ok");
	gtk_widget_show (okbutton5);
	gtk_dialog_add_action_widget (GTK_DIALOG (node_properties_dialog), okbutton5, GTK_RESPONSE_OK);
	GTK_WIDGET_SET_FLAGS (okbutton5, GTK_CAN_DEFAULT);

	g_signal_connect (comboboxentry2,	"changed", G_CALLBACK (on_resize_combo_selected), this);
	g_signal_connect (chk1, "clicked",  G_CALLBACK (on_use_png_checked), this);

	/* Store pointers to all widgets, for use by lookup_widget(). */
	GLADE_HOOKUP_OBJECT_NO_REF (node_properties_dialog, node_properties_dialog, "node_properties_dialog");
	GLADE_HOOKUP_OBJECT_NO_REF (node_properties_dialog, dialog_vbox5, "dialog_vbox5");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, table4, "table4");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, label21, "label21");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, label25, "label25");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, label26, "label26");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, label27, "label27");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, label28, "label28");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, comboboxentry2, "comboboxentry2");
	GLADE_HOOKUP_OBJECT_NO_REF (node_properties_dialog, dialog_action_area5, "dialog_action_area5");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, cancelbutton5, "cancelbutton5");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, okbutton5, "okbutton5");

	return node_properties_dialog;
}

void PixPropertiesDlg::RecalcNewSize()
{
	GtkWidget *cbo2	= lookup_widget(m_pDialog, "comboboxentry2");
	int nIdx = gtk_combo_box_get_active(GTK_COMBO_BOX(cbo2));
	ASSERT(nIdx >= 0);

	const int arPercent[] = {100, 90, 80, 75, 70, 60, 50, 40, 30, 25, 20, 10 };
	int nPercent = arPercent[nIdx];

	m_nNewWidth  = m_nOrigWidth  * nPercent / 100;
	m_nNewHeight = m_nOrigHeight * nPercent / 100;

	GtkWidget *label28 = lookup_widget(m_pDialog, "label28");
	char szBuffer[100];
	sprintf(szBuffer, "%d x %d", m_nNewWidth, m_nNewHeight);
	gtk_label_set_text(GTK_LABEL(label28), szBuffer);
}

void on_resize_combo_selected (GtkComboBox *widget, gpointer user_data)
{
	PixPropertiesDlg *pDlg = (PixPropertiesDlg *)user_data;
	pDlg->RecalcNewSize();
}

void on_use_png_checked (GtkMenuItem *menuitem, gpointer user_data)
{
	PixPropertiesDlg *pDlg = (PixPropertiesDlg *)user_data;
	pDlg->m_bUsePNG = (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(menuitem)) != FALSE);
}
