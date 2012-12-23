////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements window to list/change keyboard shortcuts for program actions
////////////////////////////////////////////////////////////////////////////

#include "ShortcutsListDlg.h"
#include "gui/FileDialog.h"
#include "lib/NoteDocument.h"
#include "lib/DocActionManager.h"
#include "lib/CircularBuffer.h"
#include "support.h"
#include "interface.h"
#include <string>
#include <gdk/gdkkeysyms.h>

#include "../res/bulb.xpm"

#ifdef _WIN32
 #include <windows.h>
 #ifndef __MINGW32__
   #ifndef snprintf
    #define snprintf _snprintf
   #endif
 #endif
#endif

extern GtkWidget *window1;
extern CircularBuffer g_objCrashLog;

extern ShortcutsList g_lstDefAction;
gint treeview_keyboard_handler(GtkWidget *widget, GdkEventKey *event, gpointer data);
static void on_ok_clicked (GtkButton *button, gpointer user_data);
static void on_cancel_clicked (GtkButton *button, gpointer user_data);
static void on_context_combo_selected (GtkComboBox *widget, gpointer user_data);
static void on_clear_all_clicked (GtkButton *button, gpointer user_data);
static void on_reset_all_clicked (GtkButton *button, gpointer user_data);


ShortcutsListDlg::ShortcutsListDlg()
{
	m_list.Load();
}

ShortcutsListDlg::~ShortcutsListDlg()
{
	Destroy();
	gtk_window_present(GTK_WINDOW(window1)); //activate main window
}

void ShortcutsListDlg::OnOK()
{
	m_list.Save();
	gtkMessageBox(_("You need to restart the application to use new shortcut definitions!"));
}

void ShortcutsListDlg::Create()
{
	m_pDialog = create_dialog (window1);
	FillList(SH_CTX_GLOBAL);
}

int ShortcutsListDlg::GetSelRow()
{
	GtkWidget *treeview = lookup_widget(m_pDialog, "treeview3");
	GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);
	GtkTreeSelection* treesel = gtk_tree_view_get_selection ((GtkTreeView *)treeview);

	//multiple selection list?
	GList* list = gtk_tree_selection_get_selected_rows(treesel, &model);

	int nSelIdx = -1;

	int ncount = g_list_length(list);
	if(ncount > 0){
		GtkTreePath *path = (GtkTreePath *)g_list_nth(list, 0)->data;
		nSelIdx = gtk_tree_path_get_indices(path)[0];	//get index from path
	}

	//cleanup list
	g_list_foreach (list, (GFunc)gtk_tree_path_free, NULL);
	g_list_free (list);

	return nSelIdx;
}

GtkWidget* ShortcutsListDlg::create_dialog (GtkWidget* parent)
{
	GtkWidget *shortcuts_dialog;
	GtkWidget *dialog_vbox8;
	GtkWidget *hbox3;
	GtkWidget *label33;
	GtkWidget *label34;
	GtkWidget *combobox4;
	GtkWidget *scrolledwindow5;
	GtkWidget *treeview3;
	GtkWidget *label35;
	GtkWidget *dialog_action_area8;
	GtkWidget *cancelbutton8;
	GtkWidget *okbutton8;
	GtkWidget *clearall_btn;
	GtkWidget *resetall_btn;

	shortcuts_dialog = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (shortcuts_dialog), _("Shortcut editor"));
	gtk_window_set_type_hint (GTK_WINDOW (shortcuts_dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_window_set_default_size(GTK_WINDOW(shortcuts_dialog), 420, 450);
	gtk_window_set_modal (GTK_WINDOW (shortcuts_dialog), TRUE);
	gtk_window_set_destroy_with_parent (GTK_WINDOW (shortcuts_dialog), TRUE);
#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (shortcuts_dialog), TRUE);
#endif
	gtk_window_set_skip_pager_hint (GTK_WINDOW (shortcuts_dialog), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW (shortcuts_dialog), GTK_WINDOW(window1));   //set parent


	dialog_vbox8 = GTK_DIALOG (shortcuts_dialog)->vbox;
	gtk_widget_show (dialog_vbox8);

	hbox3 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox3);
	gtk_box_pack_start (GTK_BOX (dialog_vbox8), hbox3, FALSE, TRUE, 0);

	label33 = gtk_label_new (_("Context:"));
	gtk_widget_show (label33);
	gtk_box_pack_start (GTK_BOX (hbox3), label33, FALSE, TRUE, 2);
	gtk_misc_set_alignment (GTK_MISC (label33), 0, 0.5);

	combobox4 = gtk_combo_box_new_text ();
	gtk_widget_show (combobox4);
	gtk_box_pack_start (GTK_BOX (hbox3), combobox4, TRUE, TRUE, 2);
	gtk_combo_box_append_text(GTK_COMBO_BOX(combobox4), _("Global"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(combobox4), _("Tree widget"));
	gtk_combo_box_set_active (GTK_COMBO_BOX(combobox4), 0);
	g_signal_connect(combobox4, "changed", G_CALLBACK (on_context_combo_selected), this);

	label34 = gtk_label_new ("");
	gtk_widget_show (label34);
	gtk_box_pack_start (GTK_BOX (hbox3), label34, FALSE, TRUE, 2);
	gtk_misc_set_alignment (GTK_MISC (label34), 1.0, 0.5);

	scrolledwindow5 = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindow5);
	gtk_box_pack_start (GTK_BOX (dialog_vbox8), scrolledwindow5, TRUE, TRUE, 0);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow5), GTK_SHADOW_IN);

	// create shortcuts list
	GtkListStore *store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
  	treeview3 = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref (G_OBJECT (store));  //tree now holds reference
	gtk_widget_show (treeview3);
	gtk_container_add (GTK_CONTAINER (scrolledwindow5), treeview3);
	g_signal_connect (treeview3, "key_press_event", G_CALLBACK (treeview_keyboard_handler), this);

	GtkTreeViewColumn *col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, _("Action"));
	gtk_tree_view_column_set_clickable(col, TRUE);	//allow column header clicks
	gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_resizable(col, TRUE);
	gtk_tree_view_column_set_fixed_width(col, 300);
//	g_signal_connect (col, "clicked", G_CALLBACK (on_header_clicked), this);

	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();

	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", 0);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview3), col);

	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, _("Shortcut"));
	gtk_tree_view_column_set_clickable(col, TRUE);	//allow column header clicks
	gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_resizable(col, TRUE);
	gtk_tree_view_column_set_fixed_width(col, 50);
//	g_signal_connect (col, "clicked", G_CALLBACK (on_header_clicked), this);

	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", 1);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview3), col);

	GtkWidget *hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (dialog_vbox8), hbox1, FALSE, TRUE, 5);

	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&bulb);
	GtkWidget *icon = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show (icon);
	gtk_box_pack_start (GTK_BOX (hbox1), icon, FALSE, TRUE, 2);

	label35 = gtk_label_new (_("Use mouse click to select an action, and then press key combination to assign the shortcut to it. Some actions are listed in more than one context so they can have more than one shortcut attached."));
	gtk_widget_show (label35);
	gtk_box_pack_start (GTK_BOX (hbox1), label35, TRUE, TRUE, 2);
	gtk_misc_set_alignment (GTK_MISC (label35), 0.5, 0.5);
	gtk_label_set_line_wrap(GTK_LABEL(label35), true);
#if GTK_CHECK_VERSION(2,10,0) //minimal version for this api
	gtk_label_set_line_wrap_mode(GTK_LABEL(label35), PANGO_WRAP_WORD_CHAR);
#endif

	//action area

	dialog_action_area8 = GTK_DIALOG (shortcuts_dialog)->action_area;
	gtk_widget_show (dialog_action_area8);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area8), GTK_BUTTONBOX_END);

	clearall_btn = gtk_button_new_with_mnemonic(_("Clear All"));
	gtk_widget_show (clearall_btn);
	gtk_container_add (GTK_CONTAINER(dialog_action_area8), clearall_btn);
	GTK_WIDGET_SET_FLAGS (clearall_btn, GTK_CAN_DEFAULT);
	g_signal_connect(clearall_btn, "clicked", G_CALLBACK (on_clear_all_clicked), this);

	resetall_btn = gtk_button_new_with_mnemonic(_("Reset All"));
	gtk_widget_show (resetall_btn);
	gtk_container_add (GTK_CONTAINER(dialog_action_area8), resetall_btn);
	GTK_WIDGET_SET_FLAGS (resetall_btn, GTK_CAN_DEFAULT);
	g_signal_connect(resetall_btn, "clicked", G_CALLBACK (on_reset_all_clicked), this);

	cancelbutton8 = gtk_button_new_with_mnemonic (_("Cancel"));
	gtk_widget_show (cancelbutton8);
	gtk_dialog_add_action_widget (GTK_DIALOG (shortcuts_dialog), cancelbutton8, GTK_RESPONSE_CANCEL);
	GTK_WIDGET_SET_FLAGS (cancelbutton8, GTK_CAN_DEFAULT);
	g_signal_connect(cancelbutton8, "clicked", G_CALLBACK (on_cancel_clicked), this);

	okbutton8 = gtk_button_new_with_mnemonic (_("OK"));
	gtk_widget_show (okbutton8);
	gtk_container_add (GTK_CONTAINER(dialog_action_area8), okbutton8);
	GTK_WIDGET_SET_FLAGS (okbutton8, GTK_CAN_DEFAULT);
	g_signal_connect(okbutton8, "clicked", G_CALLBACK (on_ok_clicked), this);

	/* Store pointers to all widgets, for use by lookup_widget(). */
	GLADE_HOOKUP_OBJECT_NO_REF (shortcuts_dialog, shortcuts_dialog, "shortcuts_dialog");
	GLADE_HOOKUP_OBJECT_NO_REF (shortcuts_dialog, dialog_vbox8, "dialog_vbox8");
	GLADE_HOOKUP_OBJECT (shortcuts_dialog, label33, "label33");
	GLADE_HOOKUP_OBJECT (shortcuts_dialog, label34, "label34");
	GLADE_HOOKUP_OBJECT (shortcuts_dialog, combobox4, "combobox4");
	GLADE_HOOKUP_OBJECT (shortcuts_dialog, scrolledwindow5, "scrolledwindow5");
	GLADE_HOOKUP_OBJECT (shortcuts_dialog, treeview3, "treeview3");
	GLADE_HOOKUP_OBJECT_NO_REF (shortcuts_dialog, dialog_action_area8, "dialog_action_area8");
	GLADE_HOOKUP_OBJECT (shortcuts_dialog, cancelbutton8, "cancelbutton8");
	GLADE_HOOKUP_OBJECT (shortcuts_dialog, okbutton8, "okbutton8");
	GLADE_HOOKUP_OBJECT (shortcuts_dialog, resetall_btn, "resetall_btn");
	GLADE_HOOKUP_OBJECT (shortcuts_dialog, clearall_btn, "clearall_btn");

	return shortcuts_dialog;
}

void ShortcutsListDlg::OnContextChanged()
{
	//filter list to show only selected context
	GtkWidget *combobox4 = lookup_widget(m_pDialog, "combobox4");
	int nCtx = gtk_combo_box_get_active (GTK_COMBO_BOX(combobox4));
	FillList(nCtx);
}

void ShortcutsListDlg::SetCurShortcut(ShortcutDef &def)
{
	//process events only if something selected
	int nIdx = GetSelRow();
	if(nIdx < 0)
		return;

	GtkWidget *combobox4 = lookup_widget(m_pDialog, "combobox4");
	int nCtx = gtk_combo_box_get_active (GTK_COMBO_BOX(combobox4));

	//find action id by index
	int nPos = g_lstDefAction.FindByContext(nCtx, nIdx);
	def.m_nActionID  = g_lstDefAction[nPos].m_nActionID;
	def.m_nCtx		 = nCtx;

	//fetch GUI iterator
	GtkWidget *treeview = lookup_widget(m_pDialog, "treeview3");
	GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);
	//calc iterator from index
	char szLevel[30]="";
	snprintf(szLevel, sizeof(szLevel), "%d", nIdx);
	GtkTreePath *path1 = gtk_tree_path_new_from_string(szLevel);
	if(!path1)
		return;
	GtkTreeIter iter;
	bool bResult;
	bResult = (FALSE != gtk_tree_model_get_iter(model, &iter, path1));
	gtk_tree_path_free(path1);
	ASSERT(bResult);

	//check if valid shortcut for this context
	if(!def.IsValid(nCtx)){
		m_list.ClearAction(def.m_nActionID, nCtx);	//press "Alt" or similar invalid combination to clear shortcut line
		gtk_list_store_set ((GtkListStore *)model, &iter, 1, "", -1);
		return;
	}

	//check if shortcut is already taken by someone else
	int nOwnerIdx = m_list.FindByKey(def, nIdx);
	if(nOwnerIdx >= 0 &&
		(m_list[nOwnerIdx].m_nActionID != def.m_nActionID ||
		 m_list[nOwnerIdx].m_nCtx != def.m_nCtx)){
		gtkMessageBox(_("This shortcut combination is already taken!"));
		return;
	}

	//store the changes into the list
	m_list.UpdateAction(def);

	//refresh list display
	gtk_list_store_set ((GtkListStore *)model, &iter, 1, def.GetDisplayString().c_str(), -1);
}

void ShortcutsListDlg::FillList(int nCtx)
{
	//fill shortcut list
	GtkWidget *treeview = lookup_widget(m_pDialog, "treeview3");
	GtkWidget *label34 = lookup_widget(m_pDialog, "label34");
	GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);

	gtk_list_store_clear((GtkListStore *)model);

	//fill the list of actions
	int nCnt = 0;
	int nMax = g_lstDefAction.size();
	for(int i=0; i<nMax; i++)
	{
		if(g_lstDefAction[i].m_nCtx == nCtx)
		{
			nCnt ++;
			GtkTreeIter iter;
			gtk_list_store_append((GtkListStore *)model, &iter);
			gtk_list_store_set ((GtkListStore *)model, &iter, 0, g_lstDefAction[i].m_strActionName.c_str(), -1);
		}
	}

	//update label with row count
	char szMsg[1024];
	sprintf(szMsg, _("%d items in this context"), nCnt);
	gtk_label_set_text(GTK_LABEL(label34), szMsg);

	//update list shortcuts from Ini data
	int nStart = g_lstDefAction.FindByContext(nCtx, 0); // the beginning of this context
	ASSERT(nStart >= 0);
	int nMax1 = m_list.size();
	for(int j=0; j<nMax1; j++)
	{
		if(m_list[j].m_nCtx != nCtx)
			continue;	//skip different context

		int nPos = g_lstDefAction.FindByAction(m_list[j].m_nActionID, nCtx);
		if(nPos >= 0)
		{
			int nIdx = nPos - nStart;

			//calc iterator from index
			char szLevel[30]="";
			snprintf(szLevel, sizeof(szLevel), "%d", nIdx);
			GtkTreePath *path1 = gtk_tree_path_new_from_string(szLevel);
			if(!path1)
				return;
			GtkTreeIter iter;
			bool bResult;
			bResult = (FALSE != gtk_tree_model_get_iter(model, &iter, path1));
			gtk_tree_path_free(path1);
			if(!bResult) return;
			ASSERT(bResult);

			gtk_list_store_set ((GtkListStore *)model, &iter, 1, m_list[j].GetDisplayString().c_str(), -1);
		}
	}
}

void ShortcutsListDlg::OnClearAll()
{
	m_list.clear();

	OnContextChanged();
}

void ShortcutsListDlg::OnResetAll()
{
	//set default shortcuts and refresh the GUI list
	m_list.clear();
	m_list = g_lstDefAction;

	OnContextChanged();
}

gint treeview_keyboard_handler(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	//process only key press events here
	if( event->type != GDK_KEY_PRESS )
		return FALSE;

	ShortcutsListDlg *pDlg = (ShortcutsListDlg *)data;

	bool bAlt   = ( event->state & GDK_MOD1_MASK || GDK_Alt_L == event->keyval  || GDK_Alt_R == event->keyval) ? true : false;
	bool bCtrl  = ( event->state & GDK_CONTROL_MASK ) ? true : false;
	bool bShift = ( event->state & GDK_SHIFT_MASK ) ? true : false;

	// allow navigation with UP/DOWN/HOME/END keys
	if(!bAlt && !bCtrl && !bShift && (event->keyval == GDK_Up || event->keyval == GDK_Down || event->keyval == GDK_Home || event->keyval == GDK_End))
		return FALSE;

	ShortcutDef def;
	def.m_bModShift = bShift;
	def.m_bModCtrl	= bCtrl;
	def.m_bModAlt	= bAlt;
	def.m_nKey		= event->keyval;

	pDlg->SetCurShortcut(def);
	return TRUE;
}

void on_ok_clicked (GtkButton *button, gpointer user_data)
{
	ShortcutsListDlg *pDlg = (ShortcutsListDlg *)user_data;
	pDlg->OnOK();
	gtk_dialog_response(GTK_DIALOG(pDlg->m_pDialog), GTK_RESPONSE_OK);
}

void on_cancel_clicked (GtkButton *button, gpointer user_data)
{
	ShortcutsListDlg *pDlg = (ShortcutsListDlg *)user_data;
	gtk_dialog_response(GTK_DIALOG(pDlg->m_pDialog), GTK_RESPONSE_CANCEL);
}

void on_clear_all_clicked (GtkButton *button, gpointer user_data)
{
	ShortcutsListDlg *pDlg = (ShortcutsListDlg *)user_data;
	pDlg->OnClearAll();
}

void on_reset_all_clicked (GtkButton *button, gpointer user_data)
{
	ShortcutsListDlg *pDlg = (ShortcutsListDlg *)user_data;
	pDlg->OnResetAll();
}

void on_context_combo_selected (GtkComboBox *widget, gpointer user_data)
{
	ShortcutsListDlg *pDlg = (ShortcutsListDlg *)user_data;
	pDlg->OnContextChanged();
}
