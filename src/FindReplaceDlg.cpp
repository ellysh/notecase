////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Window class for Find/Replace dialog
////////////////////////////////////////////////////////////////////////////

#include "FindReplaceDlg.h"
#include "FindReplaceInfo.h"
#include "lib/NoteDocument.h"
#include "lib/DocumentIterator.h"
#include "DocAction.h"
#include "lib/DocActionManager.h"
#include "TextView.h"
#include "TreeView.h"
#include "support.h"
#include "interface.h"

extern FindReplaceInfo g_objSearchInfo;
extern GtkWidget *window1;
extern NoteDocument g_doc;
extern TextView g_text;
extern TreeView g_tree;
extern DocActionManager g_undoManager;

bool IteratorFromNodeIdx(int nIdx, GtkTreeIter &iter);
void UpdateTextFromScreen();
bool text_find(bool bAutoProceedToStart = false, bool bNotifyDone = true);
int GetSelectedNodeIdx();
void RefreshMainTitle();
static void on_find_clicked(GtkMenuItem *menuitem, gpointer user_data);
static void on_replace_clicked(GtkMenuItem *menuitem, gpointer user_data);
static void on_replace_all_clicked(GtkMenuItem *menuitem, gpointer user_data);
static void on_cancel_clicked(GtkMenuItem *menuitem, gpointer user_data);
void RefreshAllLinkTags();
void on_delete_text (GtkTextBuffer *textbuffer, GtkTextIter *arg1, GtkTextIter *arg2, gpointer user_data);
void on_insert_text (GtkTextBuffer *textbuffer, GtkTextIter *arg1, gchar *arg2, gint arg3, gpointer user_data);
void UpdateUndoRedoMenus();

#ifdef _WIN32
 #ifndef __MINGW32__
  #ifndef snprintf
   #define snprintf _snprintf
  #endif
 #endif
#endif

FindReplaceDialog::FindReplaceDialog()
{
	m_bOpStarted = false;
	Create();
}

FindReplaceDialog::~FindReplaceDialog()
{
	Destroy();
	gtk_window_present(GTK_WINDOW(window1)); //activate main window
}

void FindReplaceDialog::Create()
{
	m_pDialog = create_find_dialog (window1);

	UpdateTextFromScreen();

	//default search string: selected text OR previous search string
	std::string selText =  g_text.GetSelectedText();
	if(!selText.empty())
		g_objSearchInfo.m_strFindText = selText;
	GtkWidget *entry1 = lookup_widget(m_pDialog, "entry1");
	gtk_entry_set_text(GTK_ENTRY(entry1), g_objSearchInfo.m_strFindText.c_str());
	gtk_editable_select_region (GTK_EDITABLE (entry1), 0, g_objSearchInfo.m_strFindText.size());
}

GtkWidget* FindReplaceDialog::create_find_dialog (GtkWidget* parent)
{
	GtkWidget *Find;
	GtkWidget *dialog_vbox4;
	GtkWidget *table2;
	GtkWidget *label9;
	GtkWidget *entry1;
	GtkWidget *label10;
	GtkWidget *entry2;
	//GtkWidget *label14;
	//GtkWidget *radiobutton8;
	//GSList *radiobutton8_group = NULL;
	//GtkWidget *radiobutton9;
	GtkWidget *checkbutton14;
	GtkWidget *dialog_action_area4;
	GtkWidget *findnext;
	GtkWidget *replace;
	GtkWidget *replaceall;
	GtkWidget *cancelbutton4;

	Find = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (Find), _("Replace"));
	gtk_window_set_modal (GTK_WINDOW (Find), TRUE);
	gtk_window_set_destroy_with_parent (GTK_WINDOW (Find), TRUE);
	gtk_window_set_type_hint (GTK_WINDOW (Find), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG (Find), FALSE);
	gtk_window_set_resizable (GTK_WINDOW (Find), FALSE);
	gtk_widget_realize(Find);
	gdk_window_set_decorations(Find->window, (GdkWMDecoration)(GDK_DECOR_BORDER|GDK_DECOR_TITLE));
	if(parent)
		gtk_window_set_transient_for(GTK_WINDOW (Find), GTK_WINDOW(parent));   //set parent

	dialog_vbox4 = GTK_DIALOG (Find)->vbox;
	gtk_widget_show (dialog_vbox4);

	table2 = gtk_table_new (4, 3, FALSE);
	gtk_widget_show (table2);
	gtk_box_pack_start (GTK_BOX (dialog_vbox4), table2, TRUE, TRUE, 0);

	label9 = gtk_label_new (_("Find what:"));
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

	label10 = gtk_label_new (_("Replace with:"));
	gtk_widget_show (label10);
	gtk_table_attach (GTK_TABLE (table2), label10, 0, 1, 1, 2,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label10), 0, 0.5);

	entry2 = gtk_entry_new ();
	gtk_widget_show (entry2);
	gtk_table_attach (GTK_TABLE (table2), entry2, 1, 2, 1, 2,
		(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_activates_default (GTK_ENTRY (entry2), TRUE);

/*
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
*/
	checkbutton14 = gtk_check_button_new_with_mnemonic (_("Case sensitive"));
	gtk_widget_show (checkbutton14);
	gtk_table_attach (GTK_TABLE (table2), checkbutton14, 1, 2, 2, 3,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);

	dialog_action_area4 = GTK_DIALOG (Find)->action_area;
	//gtk_widget_show (dialog_action_area4);
	//gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area4), GTK_BUTTONBOX_END);

	findnext = gtk_button_new_with_mnemonic (_("_Find next"));
	gtk_widget_show (findnext);
	gtk_table_attach (GTK_TABLE (table2), findnext, 2, 3, 0, 1,
		(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		(GtkAttachOptions) (0), 3, 1);
	GTK_WIDGET_SET_FLAGS (findnext, GTK_CAN_DEFAULT);

	replace = gtk_button_new_with_mnemonic (_("_Replace"));
	gtk_widget_show (replace);
	gtk_table_attach (GTK_TABLE (table2), replace, 2, 3, 1, 2,
		(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		(GtkAttachOptions) (0), 3, 1);
	GTK_WIDGET_SET_FLAGS (replace, GTK_CAN_DEFAULT);

	replaceall = gtk_button_new_with_mnemonic (_("Replace _All"));
	gtk_widget_show (replaceall);
	gtk_table_attach (GTK_TABLE (table2), replaceall, 2, 3, 2, 3,
		(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		(GtkAttachOptions) (0), 3, 1);
	GTK_WIDGET_SET_FLAGS (replaceall, GTK_CAN_DEFAULT);

	cancelbutton4 = gtk_button_new_with_mnemonic (_("Close"));
	gtk_widget_show (cancelbutton4);
	gtk_table_attach (GTK_TABLE (table2), cancelbutton4, 2, 3, 3, 4,
		(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		(GtkAttachOptions) (0), 3, 1);
	GTK_WIDGET_SET_FLAGS (cancelbutton4, GTK_CAN_DEFAULT);

	g_signal_connect (findnext,      "clicked", G_CALLBACK (on_find_clicked), this);
	g_signal_connect (replace,       "clicked", G_CALLBACK (on_replace_clicked), this);
	g_signal_connect (replaceall,    "clicked", G_CALLBACK (on_replace_all_clicked), this);
	g_signal_connect (cancelbutton4, "clicked", G_CALLBACK (on_cancel_clicked), this);

	/* Store pointers to all widgets, for use by lookup_widget(). */
	GLADE_HOOKUP_OBJECT_NO_REF (Find, Find, "Find");
	GLADE_HOOKUP_OBJECT_NO_REF (Find, dialog_vbox4, "dialog_vbox4");
	GLADE_HOOKUP_OBJECT (Find, table2, "table2");
	GLADE_HOOKUP_OBJECT (Find, label9, "label9");
	GLADE_HOOKUP_OBJECT (Find, entry1, "entry1");
	GLADE_HOOKUP_OBJECT (Find, entry2, "entry2");
	//GLADE_HOOKUP_OBJECT (Find, label14, "label14");
	//GLADE_HOOKUP_OBJECT (Find, radiobutton8, "radiobutton8");
	//GLADE_HOOKUP_OBJECT (Find, radiobutton9, "radiobutton9");
	GLADE_HOOKUP_OBJECT (Find, checkbutton14, "casesensitive_chk");
	//GLADE_HOOKUP_OBJECT_NO_REF (Find, dialog_action_area4, "dialog_action_area4");
	GLADE_HOOKUP_OBJECT (Find, cancelbutton4, "cancelbutton4");
	GLADE_HOOKUP_OBJECT (Find, findnext, "findnext");
	GLADE_HOOKUP_OBJECT (Find, replace, "replace");
	GLADE_HOOKUP_OBJECT (Find, replaceall, "replaceall");

	gtk_widget_grab_focus (entry1);
	gtk_widget_grab_default (findnext);

	return Find;
}

bool FindReplaceDialog::DoFindNext(bool bAutoProceedToStart, bool bNotifyDone)
{
	GtkWidget *entry1 = lookup_widget(m_pDialog, "entry1");
	GtkWidget *entry2 = lookup_widget(m_pDialog, "entry2");
	GtkWidget *checkbutton14 = lookup_widget(m_pDialog, "casesensitive_chk");

	if(!m_bOpStarted)
	{
		//fill required fields
		g_objSearchInfo.m_bFindSensitive = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton14)) > 0;
		g_objSearchInfo.m_strFindText = gtk_entry_get_text(GTK_ENTRY(entry1));
		g_objSearchInfo.m_strReplaceText = gtk_entry_get_text(GTK_ENTRY(entry2));

		if(g_objSearchInfo.m_strFindText.empty()){
			gtkMessageBox(_("Error: Search text is empty!"));
			return false;
		}
		m_bOpStarted = true;

		//start searching from the current node
		int nIdx = GetSelectedNodeIdx();
		if(nIdx < 0 && g_doc.GetNodeCount() > 0)
			nIdx = 0;

		DocumentIterator it(g_doc);
		g_objSearchInfo.m_nCurNodeRecursiveIdx   = it.NodeIdx2RecursiveIdx(nIdx);
		g_objSearchInfo.m_nStartNodeRecursiveIdx = g_objSearchInfo.m_nCurNodeRecursiveIdx;
		g_objSearchInfo.m_nFindBufferPos = 0; //start from begining of the buffer
		g_objSearchInfo.m_bFindInTitle = true;	//first search the title field
		g_objSearchInfo.m_nLastResultIdx = -1;
		g_objSearchInfo.m_nLastResultPos = -1;
		g_objSearchInfo.m_bFirstNodeSearched = false;
		g_objSearchInfo.m_bSearchKeywords = false;
	}

	return text_find(bAutoProceedToStart, bNotifyDone);
}

bool FindReplaceDialog::DoReplace()
{
	//TOFIX check some internal data on found pos
	int nIdx = GetSelectedNodeIdx();
	if(nIdx >= 0)
	{
		gint nCursor = -1;
		gint nSelection = -1;

		if(g_objSearchInfo.m_bFindInTitle)
		{
			nCursor    = g_objSearchInfo.m_nLastResultPos;
			nSelection = nCursor + g_utf8_strlen(g_objSearchInfo.m_strFindText.c_str(), -1);
		}
		else{
			//TOFIX use last find result member data instead of selection!!!!
			g_text.GetSelectionBounds(nCursor, nSelection);
		}

		if(nCursor >= 0 && nSelection != nCursor)
		{
			NoteNode &node = g_doc.GetNodeByIdx(nIdx);

			// some text selected, replace it with the given string
			std::string strTxt;
			if(g_objSearchInfo.m_bFindInTitle)
				strTxt = node.GetTitle();
			else
				strTxt = node.GetText();

			//convert from character offset to buffer offset
			int nCursorBuf, nSelBuf;
			const char *szBuf = strTxt.c_str();
			gchar* pszPos = g_utf8_offset_to_pointer(szBuf, nCursor);
			nCursorBuf = pszPos - szBuf;
			pszPos = g_utf8_offset_to_pointer(szBuf, nSelection);
			nSelBuf = pszPos - szBuf;

			//now replace the text part with new content
			//TOFIX what if cursor < nSelBuf - convert both values using min
			strTxt = strTxt.erase(nCursorBuf, nSelBuf-nCursorBuf);
			strTxt.insert(nCursorBuf, g_objSearchInfo.m_strReplaceText.c_str());

			if(g_objSearchInfo.m_bFindInTitle)
			{
				node.SetTitle(strTxt.c_str());

				//update GUI text
				GtkTreeIter iter;
				IteratorFromNodeIdx(nIdx, iter);
				GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)g_tree.m_pWidget);
				gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 0, strTxt.c_str(), -1);

				set_title_bar(strTxt.c_str());
			}
			else
			{
				node.SetText(strTxt.c_str());

				//update GUI text
				GtkTextView *textview =  (GtkTextView *)g_text.m_pWidget;
				GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);

				g_signal_handlers_block_by_func(buffer1, (void *)on_delete_text, 0);
				g_signal_handlers_block_by_func(buffer1, (void *)on_insert_text, 0);

				g_text.ReplaceText(nCursor, nSelection, g_objSearchInfo.m_strReplaceText.c_str());
				g_text.SelectRange(nCursor, nCursor + g_utf8_strlen(g_objSearchInfo.m_strReplaceText.c_str(), -1));

				g_signal_handlers_unblock_by_func(buffer1, (void *)on_delete_text, 0);
				g_signal_handlers_unblock_by_func(buffer1, (void *)on_insert_text, 0);

				node.OnTxtReplaced(nCursor, nSelection, g_objSearchInfo.m_strReplaceText.c_str());
				RefreshAllLinkTags();
			}

			node.m_nDateModified = time(NULL);

			//push UNDO action
			DocAction *pAction = new DocAction;
			pAction->SetType(ACT_TEXT_REPLACE);
			pAction->SetDoc(g_doc);
			pAction->m_nNodeIndex = nIdx; //TOFIX recursive index ????
			pAction->m_nOffset = nCursor;
			pAction->m_strFind = g_objSearchInfo.m_strFindText;
			pAction->m_strReplace = g_objSearchInfo.m_strReplaceText;
			pAction->m_bNodeTitle = (g_objSearchInfo.m_bFindInTitle > 0);

			g_undoManager.AddAction(pAction);
			UpdateUndoRedoMenus();

			g_doc.SetModified(true);
			RefreshMainTitle();

			return true;
		}
	}

	return false;
}

bool FindReplaceDialog::DoReplaceAll()
{
	ShowBusyCursor();

	m_nReplaceAllCount = 0;

	//true -> do not ask to resume from document start - do it automatically
	while(DoFindNext(true, false)){
		DoReplace();
		m_nReplaceAllCount++;
	}

	HideBusyCursor();

	//close the dialog
	Destroy();

	//write statistics
	char szBuffer[1024];
	snprintf(szBuffer, sizeof(szBuffer)-1, _("Done!\n\n%d string instances were replaced!"), m_nReplaceAllCount);
	gtkMessageBox(szBuffer);

	return true;
}

void on_find_clicked(GtkMenuItem *menuitem, gpointer user_data)
{
	FindReplaceDialog *pDlg = (FindReplaceDialog *)user_data;
	pDlg->DoFindNext();
}

void on_replace_clicked(GtkMenuItem *menuitem, gpointer user_data)
{
	FindReplaceDialog *pDlg = (FindReplaceDialog *)user_data;
	pDlg->DoReplace();
}

void on_replace_all_clicked(GtkMenuItem *menuitem, gpointer user_data)
{
	FindReplaceDialog *pDlg = (FindReplaceDialog *)user_data;
	pDlg->DoReplaceAll();
}

void on_cancel_clicked(GtkMenuItem *menuitem, gpointer user_data)
{
	FindReplaceDialog *pDlg = (FindReplaceDialog *)user_data;
	pDlg->Destroy();
}

void RefreshAllLinkTags()
{
	int nIdx = GetSelectedNodeIdx();
	if(nIdx >= 0)
	{
		//remove all tags
		long nChars = g_utf8_strlen(g_doc.GetNodeByIdx(nIdx).GetText().c_str(), -1);
		g_text.RemoveTextStyles(0, nChars);

		//regenerate all tags
		int nLinks = g_doc.GetNodeByIdx(nIdx).m_lstLinks.size();
		for(int i=0; i<nLinks; i++){
			LinkInfo &info = g_doc.GetNodeByIdx(nIdx).m_lstLinks[i];
			g_text.SetTextUnderlined(info.m_nStartOffset, info.m_nStartOffset + info.m_nTextLength);
		}
	}
}
