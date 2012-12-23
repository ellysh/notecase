////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Window to define hyperlink properties
////////////////////////////////////////////////////////////////////////////

#include "LinkPropertiesDlg.h"
#include "support.h"
#include "gui/FileDialog.h"
#include "callbacks.h"
#include "interface.h"
#include "lib/NoteDocument.h"
#include <string.h>

extern GtkWidget *window1;
extern NoteDocument g_doc;

static void on_pick_file_clicked (GtkMenuItem *menuitem, gpointer user_data);
static void on_link_remove_clicked (GtkMenuItem *menuitem, gpointer user_data);
static void on_link_type_combo_changed (GtkComboBox *widget, gpointer user_data);
static void on_ok_clicked (GtkMenuItem *menuitem, gpointer user_data);

LinkPropertiesDlg::LinkPropertiesDlg()
{
}

LinkPropertiesDlg::~LinkPropertiesDlg()
{
	UpdateData(false);
	Destroy();
}

void LinkPropertiesDlg::Create()
{
	m_pDialog = create_link_wizard();

	//fill link types combo box
	GtkWidget *combobox3 = lookup_widget(m_pDialog, "combobox3");
	gtk_combo_box_append_text(GTK_COMBO_BOX(combobox3), _("Link to a node"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(combobox3), _("Link to a file or web address"));
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox3), 0); //TOFIX
	on_link_type_combo_changed (NULL, this);

	UpdateData(true);

	GtkWidget *entry9 = lookup_widget(m_pDialog, "entry9");
	gtk_window_set_focus(GTK_WINDOW(m_pDialog), entry9);
}

GtkWidget* LinkPropertiesDlg::create_link_wizard ()
{
	GtkWidget *link_wizard;
	GtkWidget *dialog_vbox7;
	GtkWidget *table8;
	GtkWidget *combobox3;
	GtkWidget *label29;
	GtkWidget *entry6;
	GtkWidget *combobox4;
	GtkWidget *label30;
	GtkWidget *label31;
	GtkWidget *label32;
	GtkWidget *hbox4;
	GtkWidget *entry9;
	GtkWidget *button6;
	GtkWidget *dialog_action_area7;
	GtkWidget *btnremove;
	GtkWidget *cancelbutton7;
	GtkWidget *okbutton7;

	link_wizard = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (link_wizard), _("Link Properties"));
	gtk_window_set_type_hint (GTK_WINDOW (link_wizard), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_window_set_default_size(GTK_WINDOW(link_wizard), 450, -1);
	gtk_window_set_modal (GTK_WINDOW (link_wizard), TRUE);
	gtk_window_set_destroy_with_parent (GTK_WINDOW (link_wizard), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW (link_wizard), GTK_WINDOW(window1));   //set parent

	dialog_vbox7 = GTK_DIALOG (link_wizard)->vbox;
	gtk_widget_show (dialog_vbox7);

	table8 = gtk_table_new (4, 2, FALSE);
	gtk_widget_show (table8);
	gtk_box_pack_start (GTK_BOX (dialog_vbox7), table8, TRUE, TRUE, 0);

	entry6 = gtk_entry_new ();
	gtk_widget_show (entry6);
	gtk_entry_set_editable(GTK_ENTRY(entry6), FALSE);
	gtk_widget_set_sensitive(entry6, FALSE);
	gtk_table_attach (GTK_TABLE (table8), entry6, 1, 2, 0, 1,
		(GtkAttachOptions) (GTK_EXPAND|GTK_FILL|GTK_SHRINK),
		(GtkAttachOptions) (0), 0, 0);

	label30 = gtk_label_new (_("Link text:"));
	gtk_widget_show (label30);
	gtk_table_attach (GTK_TABLE (table8), label30, 0, 1, 0, 1,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label30), 0, 0.5);

	combobox3 = gtk_combo_box_new_text ();
	gtk_widget_show (combobox3);
	gtk_table_attach (GTK_TABLE (table8), combobox3, 1, 2, 1, 2,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);

	label29 = gtk_label_new (_("Link type:"));
	gtk_widget_show (label29);
	gtk_table_attach (GTK_TABLE (table8), label29, 0, 1, 1, 2,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label29), 0, 0.5);

	label31 = gtk_label_new (_("Target file or URL:"));
	gtk_widget_show (label31);
	gtk_table_attach (GTK_TABLE (table8), label31, 0, 1, 2, 3,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label31), 0, 0.5);

	label32 = gtk_label_new (_("Target node:"));
	gtk_widget_show (label32);
	gtk_table_attach (GTK_TABLE (table8), label32, 0, 1, 3, 4,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label32), 0, 0.5);

	hbox4 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox4);
	gtk_table_attach (GTK_TABLE (table8), hbox4, 1, 2, 2, 3,
		(GtkAttachOptions) (GTK_EXPAND|GTK_FILL|GTK_SHRINK),
		(GtkAttachOptions) (0), 0, 0);

	entry9 = gtk_entry_new ();
	gtk_widget_show (entry9);
	gtk_box_pack_start (GTK_BOX (hbox4), entry9, TRUE, TRUE, 0);
	gtk_entry_set_activates_default (GTK_ENTRY (entry9), TRUE);

	button6 = gtk_button_new_with_mnemonic ("...");
	gtk_widget_show (button6);
	gtk_box_pack_start (GTK_BOX (hbox4), button6, FALSE, FALSE, 0);

	GtkWidget *treeview = lookup_widget(window1, "treeview1");
	GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);

	combobox4 = gtk_combo_box_new_with_model(model);
	gtk_widget_set_size_request(combobox4, 300, -1);
	gtk_widget_show (combobox4);
	gtk_table_attach (GTK_TABLE (table8), combobox4, 1, 2, 3, 4,
		(GtkAttachOptions) (GTK_EXPAND|GTK_FILL|GTK_SHRINK),
		(GtkAttachOptions) (0), 0, 0);

	GtkCellRenderer *text_renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT(combobox4), text_renderer, TRUE);
	gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT(combobox4), text_renderer, "text", 0);

	dialog_action_area7 = GTK_DIALOG (link_wizard)->action_area;
	gtk_widget_show (dialog_action_area7);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area7), GTK_BUTTONBOX_END);

	btnremove = gtk_button_new_with_mnemonic (_("Remove"));
	gtk_widget_show (btnremove);
	gtk_container_add (GTK_CONTAINER (dialog_action_area7), btnremove);
	GTK_WIDGET_SET_FLAGS (btnremove, GTK_CAN_DEFAULT);

	cancelbutton7 = gtk_button_new_from_stock ("gtk-cancel");
	gtk_widget_show (cancelbutton7);
	gtk_dialog_add_action_widget (GTK_DIALOG (link_wizard), cancelbutton7, GTK_RESPONSE_CANCEL);
	GTK_WIDGET_SET_FLAGS (cancelbutton7, GTK_CAN_DEFAULT);

	okbutton7 = gtk_button_new_from_stock ("gtk-ok");
	gtk_widget_show (okbutton7);
	gtk_container_add (GTK_CONTAINER (dialog_action_area7), okbutton7);
	GTK_WIDGET_SET_FLAGS (okbutton7, GTK_CAN_DEFAULT);

	g_signal_connect (button6, "clicked",	G_CALLBACK (on_pick_file_clicked), this);
	g_signal_connect (btnremove, "clicked",	G_CALLBACK (on_link_remove_clicked), this);
	g_signal_connect (combobox3, "changed", G_CALLBACK (on_link_type_combo_changed), this);
	g_signal_connect(okbutton7, "clicked",	G_CALLBACK (on_ok_clicked), this);

	/* Store pointers to all widgets, for use by lookup_widget(). */
	GLADE_HOOKUP_OBJECT_NO_REF (link_wizard, link_wizard, "link_wizard");
	GLADE_HOOKUP_OBJECT_NO_REF (link_wizard, dialog_vbox7, "dialog_vbox7");
	GLADE_HOOKUP_OBJECT (link_wizard, table8, "table8");
	GLADE_HOOKUP_OBJECT (link_wizard, combobox3, "combobox3");
	GLADE_HOOKUP_OBJECT (link_wizard, label29, "label29");
	GLADE_HOOKUP_OBJECT (link_wizard, entry6, "entry6");
	GLADE_HOOKUP_OBJECT (link_wizard, label30, "label30");
	GLADE_HOOKUP_OBJECT (link_wizard, label31, "label31");
	GLADE_HOOKUP_OBJECT (link_wizard, label32, "label32");
	GLADE_HOOKUP_OBJECT (link_wizard, hbox4, "hbox4");
	GLADE_HOOKUP_OBJECT (link_wizard, entry9, "entry9");
	GLADE_HOOKUP_OBJECT (link_wizard, button6, "button6");
	GLADE_HOOKUP_OBJECT (link_wizard, combobox4, "combobox4");
	GLADE_HOOKUP_OBJECT_NO_REF (link_wizard, dialog_action_area7, "dialog_action_area7");
	GLADE_HOOKUP_OBJECT (link_wizard, cancelbutton7, "cancelbutton7");
	GLADE_HOOKUP_OBJECT (link_wizard, okbutton7, "okbutton7");

	gtk_widget_grab_default (okbutton7);
	return link_wizard;
}

void LinkPropertiesDlg::UpdateData(bool bToScreen)
{
	if(NULL == m_pDialog)
		return;

	GtkWidget *entry6 = lookup_widget(m_pDialog, "entry6");
	GtkWidget *entry9 = lookup_widget(m_pDialog, "entry9");
	GtkWidget *combobox4 = lookup_widget(m_pDialog, "combobox4");
	GtkWidget *combobox3 = lookup_widget(m_pDialog, "combobox3");

	if(bToScreen)
	{
		gtk_entry_set_text(GTK_ENTRY(entry6), m_info.m_strText.c_str());
		gtk_entry_set_text(GTK_ENTRY(entry9), m_info.m_strTargetURL.c_str());

		//select node in the combo
		if(m_info.m_nTargetNodeID >= 0){
			gtk_combo_box_set_active(GTK_COMBO_BOX(combobox3), 0);

			GtkTreeIter iter;
			TreeIterFromID(iter, m_info.m_nTargetNodeID);
			gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combobox4), &iter);
		}
		else{
			gtk_combo_box_set_active(GTK_COMBO_BOX(combobox3), 1);
			//gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combobox4), NULL); //FIX: gtk warning
		}
	}
	else
	{
		m_info.m_strText       = gtk_entry_get_text(GTK_ENTRY(entry6));

		m_info.m_nTargetNodeID = -1;
		int nPos = gtk_combo_box_get_active(GTK_COMBO_BOX(combobox3));
		if(nPos > 0)
		{
			m_info.m_strTargetURL  = gtk_entry_get_text(GTK_ENTRY(entry9));
		}
		else
		{
			m_info.m_strTargetURL  = "";

			//get selected node in the combo
			GtkTreeIter iter;
			if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(combobox4), &iter)){
				GtkTreeView *treeview = (GtkTreeView *)lookup_widget(window1, "treeview1");
				GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);
				GtkTreePath* path1 = gtk_tree_model_get_path(model, &iter);
				int nNodeIdx = NodeIdxFromPath(path1);
				if(nNodeIdx >= 0)
					m_info.m_nTargetNodeID = g_doc.GetNodeByIdx(nNodeIdx).m_nID;
				gtk_tree_path_free(path1);
			}
		}

		//cache string size
		m_info.RefreshLength();
	}
}

void on_pick_file_clicked (GtkMenuItem *menuitem, gpointer user_data)
{
	LinkPropertiesDlg *pDlg = (LinkPropertiesDlg *)user_data;

	FileDialog dlg;
	if(dlg.DoModal())
	{
		pDlg->UpdateData(false);
		pDlg->m_info.m_strTargetURL  = "file://";
	#ifdef _WIN32
		pDlg->m_info.m_strTargetURL  += "/";
	#endif

		pDlg->m_info.m_strTargetURL += dlg.GetFilename();
		pDlg->UpdateData(true);
	}
}

void on_link_remove_clicked (GtkMenuItem *menuitem, gpointer user_data)
{
	LinkPropertiesDlg *pDlg = (LinkPropertiesDlg *)user_data;
	pDlg->m_info.m_strTargetURL  = "";	//erase link
	pDlg->m_info.m_nTargetNodeID = -1;	//erase link
	pDlg->UpdateData(true);
	gtk_dialog_response(GTK_DIALOG(pDlg->m_pDialog), GTK_RESPONSE_OK);
}

void on_link_type_combo_changed (GtkComboBox *widget, gpointer user_data)
{
	LinkPropertiesDlg *pDlg = (LinkPropertiesDlg *)user_data;
	GtkWidget *combobox3 = lookup_widget(pDlg->m_pDialog, "combobox3");
	GtkWidget *hbox4 = lookup_widget(pDlg->m_pDialog, "hbox4");
	GtkWidget *combobox4 = lookup_widget(pDlg->m_pDialog, "combobox4");
	GtkWidget *entry9 = lookup_widget(pDlg->m_pDialog, "entry9");

	int nPos = gtk_combo_box_get_active(GTK_COMBO_BOX(combobox3));
	if(0 == nPos)
	{
		gtk_entry_set_text(GTK_ENTRY(entry9), "");
		gtk_widget_set_sensitive(hbox4, FALSE);
		gtk_widget_set_sensitive(combobox4, TRUE);
	}
	else
	{
		gtk_widget_set_sensitive(hbox4, TRUE);
		gtk_widget_set_sensitive(combobox4, FALSE);
	}
}

void on_ok_clicked (GtkMenuItem *menuitem, gpointer user_data)
{
	LinkPropertiesDlg *pDlg = (LinkPropertiesDlg *)user_data;
	pDlg->UpdateData(false);
	gtk_dialog_response(GTK_DIALOG(pDlg->m_pDialog), GTK_RESPONSE_OK);
}
