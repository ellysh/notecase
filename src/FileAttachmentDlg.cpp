////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements window to handle file attachments (each document node has its own attachments)
////////////////////////////////////////////////////////////////////////////

#include "FileAttachmentDlg.h"
#include "gui/FileDialog.h"
#include "lib/NoteDocument.h"
#include "gui/ProgressDlg.h"
#include "lib/DocActionManager.h"
#include "lib/CircularBuffer.h"
#include "DocActionAtt.h"
#include "lib/IniFile.h"
#include "lib/FilePath.h"
#include "lib/File64.h"
#include "support.h"
#include "interface.h"
#include <string>
#include <errno.h>
#include "lib/HtmlParser.h"
#include "ExecuteFile.h"

#ifdef _WIN32
 #include <io.h> //access
 #define access _access
#endif

extern GtkWidget *window1;
extern NoteDocument g_doc;
extern DocActionManager g_undoManager;
extern CircularBuffer g_objCrashLog;
extern IniFile g_objIni;
extern GdkAtom atomTextUriList;

void RefreshMainTitle();
void UpdateUndoRedoMenus();
void replaceall(std::string &strData, const char *szFind, const char *szReplace);
bool Utf8ToLocale(const char *szTxt, std::string &strResult);
static void on_add_file_clicked(GtkButton *button, gpointer user_data);
static void on_remove_file_clicked(GtkButton *button, gpointer user_data);
static void on_save_file_clicked(GtkButton *button, gpointer user_data);
static void on_close_clicked (GtkButton *button, gpointer user_data);
static gboolean att_list_on_drop(GtkWidget *widget, GdkDragContext *drag_context, gint x, gint y, guint time, gpointer user_data);
static void att_list_drag_data_received(GtkWidget *widget, GdkDragContext *context, gint x, gint y,
        GtkSelectionData *selection_data, guint target_type, guint time,
        gpointer data);
gboolean is_format_provided(GdkDragContext *context, GdkAtom target);
void on_item_executed (GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer  userdata);

#ifndef min
 #define min(a,b) (((a)<(b))?(a):(b))
#endif

FileAttachmentDlg::FileAttachmentDlg()
{
}

FileAttachmentDlg::~FileAttachmentDlg()
{
	Destroy();
	gtk_window_present(GTK_WINDOW(window1)); //activate main window
}

void FileAttachmentDlg::Create()
{
	m_pDialog = create_dialog (window1);

	//fill attachment list
	GtkWidget *treeview = lookup_widget(m_pDialog, "treeview");
	GtkListStore *store = (GtkListStore *)gtk_tree_view_get_model((GtkTreeView *)treeview);

	int nAttCnt = g_doc.GetNodeByIdx(m_nNodeIdx).m_lstAttachments.size();
	for(int i=0; i<nAttCnt; i++)
	{
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);

		//std::string strName = g_doc.GetNodeByIdx(m_nNodeIdx).m_lstAttachments[i].m_strName;
		gtk_list_store_set (store, &iter,
							0, g_doc.GetNodeByIdx(m_nNodeIdx).m_lstAttachments[i].m_strName.c_str(),
							1, g_doc.GetNodeByIdx(m_nNodeIdx).m_lstAttachments[i].nDataSize,
							-1);
	}

	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("The document is read-only! You won't be able to make any changes!"));
	}
}

int FileAttachmentDlg::GetSelRow()
{
	GtkWidget *treeview = lookup_widget(m_pDialog, "treeview");
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

GtkWidget* FileAttachmentDlg::create_dialog (GtkWidget* parent)
{
	GtkWidget *attachments_dialog;
	GtkWidget *dialog_vbox6;
	GtkWidget *dialog_action_area6;
	GtkWidget *add_btn;
	GtkWidget *remove_btn;
	GtkWidget *save_btn;
	GtkWidget *close_btn;
	GtkWidget *scrolledwindow1;

	attachments_dialog = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (attachments_dialog), _("Node attachments"));
	gtk_window_set_modal (GTK_WINDOW (attachments_dialog), TRUE);
	gtk_window_set_destroy_with_parent (GTK_WINDOW (attachments_dialog), TRUE);
#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (attachments_dialog), TRUE);
#endif
	gtk_window_set_skip_pager_hint (GTK_WINDOW (attachments_dialog), TRUE);
	gtk_window_set_type_hint (GTK_WINDOW (attachments_dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
	if(parent)
		gtk_window_set_transient_for(GTK_WINDOW (attachments_dialog), GTK_WINDOW(parent));   //set parent
#if GTK_CHECK_VERSION(2,4,0) //new API TOFIX set proper version
	#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
	  //gtk_window_set_keep_above(GTK_WINDOW (attachments_dialog), TRUE);
	#endif
#endif
	gtk_widget_realize(attachments_dialog);

	dialog_vbox6 = GTK_DIALOG (attachments_dialog)->vbox;
	gtk_widget_show (dialog_vbox6);

	scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_widget_show (scrolledwindow1);
	gtk_box_pack_start (GTK_BOX (dialog_vbox6), scrolledwindow1, TRUE, TRUE, 0);

	// create file list
	GtkListStore *store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING);
  	GtkWidget *treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref (G_OBJECT (store));  //tree now holds reference

	gtk_widget_set_size_request(treeview, -1, 100);

	//make the "textview1" a DnD destination
	static const GtkTargetEntry targets[] = {
		{ (gchar *)"text/uri-list", 0, 0 },
	};
	gtk_drag_dest_set(GTK_WIDGET(treeview), GTK_DEST_DEFAULT_ALL, targets, G_N_ELEMENTS (targets), GdkDragAction(GDK_ACTION_COPY));

	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeview), TRUE);
	gtk_tree_view_set_headers_clickable (GTK_TREE_VIEW(treeview), TRUE);
	gtk_tree_view_set_enable_search(GTK_TREE_VIEW(treeview),FALSE);

	gtk_widget_show (treeview);
	gtk_container_add( GTK_CONTAINER(scrolledwindow1), treeview );

	GtkTreeViewColumn *col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, _("Name"));
	gtk_tree_view_column_set_clickable(col, TRUE);	//allow column header clicks
	gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_resizable(col, TRUE);
	gtk_tree_view_column_set_fixed_width(col, 150);
//	g_signal_connect (col, "clicked", G_CALLBACK (on_header_clicked), this);

	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();

	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", 0);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col);

	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, _("Size"));
	gtk_tree_view_column_set_clickable(col, TRUE);	//allow column header clicks
	gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_resizable(col, TRUE);
	gtk_tree_view_column_set_fixed_width(col, 50);
	gtk_tree_view_column_set_alignment(col, 1.0);	// right align size column
//	g_signal_connect (col, "clicked", G_CALLBACK (on_header_clicked), this);

	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", 1);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col);


	dialog_action_area6 = GTK_DIALOG (attachments_dialog)->action_area;
	gtk_widget_show (dialog_action_area6);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area6), GTK_BUTTONBOX_END);

	add_btn = gtk_button_new_with_mnemonic (_("Add"));
	gtk_widget_show (add_btn);
	gtk_container_add (GTK_CONTAINER(dialog_action_area6), add_btn);
	GTK_WIDGET_SET_FLAGS (add_btn, GTK_CAN_DEFAULT);

	remove_btn = gtk_button_new_with_mnemonic (_("Remove"));
	gtk_widget_show (remove_btn);
	gtk_container_add (GTK_CONTAINER(dialog_action_area6), remove_btn);
	GTK_WIDGET_SET_FLAGS (remove_btn, GTK_CAN_DEFAULT);

	save_btn = gtk_button_new_with_mnemonic (_("Save to file"));
	gtk_widget_show (save_btn);
	gtk_container_add (GTK_CONTAINER(dialog_action_area6), save_btn);
	GTK_WIDGET_SET_FLAGS (save_btn, GTK_CAN_DEFAULT);

	close_btn = gtk_button_new_with_mnemonic (_("Close"));
	gtk_widget_show (close_btn);
	gtk_container_add (GTK_CONTAINER(dialog_action_area6), close_btn);
	GTK_WIDGET_SET_FLAGS (close_btn, GTK_CAN_DEFAULT);

	g_signal_connect (treeview, "drag-drop",			G_CALLBACK (att_list_on_drop), this);
	g_signal_connect (treeview, "drag-data-received",	G_CALLBACK (att_list_drag_data_received), this);
	g_signal_connect (treeview, "row-activated",		G_CALLBACK (on_item_executed), this);

	g_signal_connect(add_btn,   "clicked", G_CALLBACK (on_add_file_clicked), this);
	g_signal_connect(remove_btn,"clicked", G_CALLBACK (on_remove_file_clicked), this);
	g_signal_connect(save_btn,  "clicked", G_CALLBACK (on_save_file_clicked), this);
	g_signal_connect(close_btn, "clicked", G_CALLBACK (on_close_clicked), this);

	// Store pointers to all widgets, for use by lookup_widget()
	GLADE_HOOKUP_OBJECT_NO_REF (attachments_dialog, attachments_dialog, "attachments_dialog");
	GLADE_HOOKUP_OBJECT_NO_REF (attachments_dialog, dialog_vbox6, "dialog_vbox6");
	GLADE_HOOKUP_OBJECT_NO_REF (attachments_dialog, dialog_action_area6, "dialog_action_area6");
	GLADE_HOOKUP_OBJECT (attachments_dialog, treeview, "treeview");
	GLADE_HOOKUP_OBJECT (attachments_dialog, add_btn, "add_btn");
	GLADE_HOOKUP_OBJECT (attachments_dialog, remove_btn, "remove_btn");
	GLADE_HOOKUP_OBJECT (attachments_dialog, save_btn, "save_btn");
	GLADE_HOOKUP_OBJECT (attachments_dialog, close_btn, "close_btn");

	gtk_widget_grab_default (close_btn);

	return attachments_dialog;
}

void on_add_file_clicked (GtkButton *button, gpointer user_data)
{
	FileAttachmentDlg *pDlg = (FileAttachmentDlg *)user_data;

	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}

	//load file to binary string
	FileDialog dlg(true);
	dlg.SetTitle(_("Add attachment"));

	//set initial directory from INI (store last used)
	std::string strDefaultDir;
	std::string strDir;
	g_objIni.GetValue("Cache", "LastAttachmentDir", strDir, "");
	if(!strDir.empty() && 0 == access(strDir.c_str(), 00))
		strDefaultDir = strDir;
	else
		strDefaultDir = GetHomeDir();

	dlg.SetDirectory(strDefaultDir.c_str());


	if(dlg.DoModal())
	{
		const gchar *filename = dlg.GetFilename();
		strDefaultDir = dlg.GetDirectory();
		dlg.Close();

		//store last open directory
		g_objIni.SetValue("Cache", "LastAttachmentDir", strDefaultDir.c_str());
		g_objIni.Save();

		std::string strLocaleFile;
		Utf8ToLocale(filename, strLocaleFile);
	#ifdef _WIN32
		replaceall(strLocaleFile, "/", "\\");
	#endif

		File64 file;
		if(!file.Open(filename)){
			gtkMessageBox(_("Failed to open input file!"));
			return;
		}

		g_objCrashLog.AddMsg("ACTION: add attachment\n");

		AttInfo info;

		//show wait dialog
		ProgressDlg dlg(200, _("Loading file. Please wait..."), window1);

		int nTotal = 0;
		int nLoopCnt = 0;
		char szBuffer[2000];
		size_t nRead = 0;
		while((nRead = file.Read(szBuffer, sizeof(szBuffer))) > 0)
		{
			info.m_strData.append(szBuffer, nRead);
			nTotal += nRead;
			nLoopCnt++;

			if(nLoopCnt%10 == 0)
				FormatIO_Base::RunLoop();
		}
		file.Close();

		info.nDataSize = info.m_strData.size();
		info.m_strName = filename;
		std::string::size_type nPos = info.m_strName.find_last_of("\\/");
		if(nPos != std::string::npos){
			info.m_strName = info.m_strName.substr(nPos+1);
		}

		g_doc.GetNodeByIdx(pDlg->m_nNodeIdx).m_lstAttachments.push_back(info);

		//refresh GUI list
		GtkWidget *treeview = lookup_widget(pDlg->m_pDialog, "treeview");
		GtkListStore *store = (GtkListStore *)gtk_tree_view_get_model((GtkTreeView *)treeview);

		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);

		gtk_list_store_set (store, &iter,
							0, info.m_strName.c_str(),
							1, info.nDataSize,
							-1);

		g_doc.SetModified(true);
		RefreshMainTitle();

		//push document change into undo/redo manager
		DocActionAtt *pAction = new DocActionAtt;
		pAction->m_bAddOp = true;
		pAction->m_info = info;
		pAction->m_nNodeIdx = pDlg->m_nNodeIdx;
		pAction->m_nAttIdx    = g_doc.GetNodeByIdx(pDlg->m_nNodeIdx).m_lstAttachments.size()-1;

		g_undoManager.AddAction(pAction);
		UpdateUndoRedoMenus();
	}
}

void on_remove_file_clicked(GtkButton *button, gpointer user_data)
{
	FileAttachmentDlg *pDlg = (FileAttachmentDlg *)user_data;

	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}

	int nSelIdx = pDlg->GetSelRow();
	if(nSelIdx >= 0)
	{
		g_objCrashLog.AddMsg("ACTION: remove attachment\n");

		//push document change into undo/redo manager
		DocActionAtt *pAction = new DocActionAtt;
		pAction->m_bAddOp	= false;
		pAction->m_info		= g_doc.GetNodeByIdx(pDlg->m_nNodeIdx).m_lstAttachments[nSelIdx];
		pAction->m_nNodeIdx = pDlg->m_nNodeIdx;
		pAction->m_nAttIdx  = nSelIdx;

		g_undoManager.AddAction(pAction);
		UpdateUndoRedoMenus();

		//do the action
		g_doc.GetNodeByIdx(pDlg->m_nNodeIdx).m_lstAttachments.erase(g_doc.GetNodeByIdx(pDlg->m_nNodeIdx).m_lstAttachments.begin()+nSelIdx);

		//refresh GUI list
		GtkWidget *treeview = lookup_widget(pDlg->m_pDialog, "treeview");
		GtkTreeModel *store = (GtkTreeModel *)gtk_tree_view_get_model((GtkTreeView *)treeview);

		//from index to path to iter
		char szLevel[50];
		sprintf(szLevel, "%d", nSelIdx);
		GtkTreePath* path1 = gtk_tree_path_new_from_string(szLevel);
		GtkTreeIter iter;
		gint bResult = gtk_tree_model_get_iter(store, &iter, path1);
		gtk_tree_path_free(path1);

		if(bResult)
			gtk_list_store_remove((GtkListStore *)store, &iter);

		g_doc.SetModified(true);
		RefreshMainTitle();
	}
}

void on_save_file_clicked (GtkButton *button, gpointer user_data)
{
	FileAttachmentDlg *pDlg = (FileAttachmentDlg *)user_data;

	int nSelRow = pDlg->GetSelRow();
	if(nSelRow < 0){
		gtkMessageBox(_("No attachment selected!"));
		return;
	}

	//TOFIX circular buf msg
	TRACE("Save attachment to file\n");

	g_objCrashLog.AddMsg("ACTION: save attachment\n");

	//save binary string to file
	FileDialog dlg(false);
	dlg.SetTitle(_("Save data"));

	//set initial directory from INI (store last used)
	std::string strDefaultDir;
	std::string strDir;
	g_objIni.GetValue("Cache", "LastAttachmentDir", strDir, "");
	if(!strDir.empty() && 0 == access(strDir.c_str(), 00))
		strDefaultDir = strDir;
	else
		strDefaultDir = GetHomeDir();

	dlg.SetDirectory(strDefaultDir.c_str());
	dlg.SetFilename(g_doc.GetNodeByIdx(pDlg->m_nNodeIdx).m_lstAttachments[nSelRow].m_strName.c_str());

	if(dlg.DoModal())
	{
		const gchar *filename = dlg.GetFilename();
		strDefaultDir = dlg.GetDirectory();
		dlg.Close();

		//store last open directory
		g_objIni.SetValue("Cache", "LastAttachmentDir", strDefaultDir.c_str());
		g_objIni.Save();

		std::string strLocaleFile;
		Utf8ToLocale(filename, strLocaleFile);
	#ifdef _WIN32
		replaceall(strLocaleFile, "/", "\\");
	#endif

		File64 file;
		if(!file.Open(filename, F64_WRITE|F64_SHARE_READ|F64_OPEN_NEW)){
			gtkMessageBox(_("Failed to open output file!"));
			return;
		}

		//show wait dialog (only if not from cmd line)
		ProgressDlg dlg(200, _("Saving file. Please wait..."), window1);

		int nSize = g_doc.GetNodeByIdx(pDlg->m_nNodeIdx).m_lstAttachments[nSelRow].nDataSize;
		const char *szData = g_doc.GetNodeByIdx(pDlg->m_nNodeIdx).m_lstAttachments[nSelRow].m_strData.c_str();

		int nPos = 0;
		int nChunkSize = 2048;
		int nSaveStep  = 0;
		int nLoopCnt = 0;
		size_t nWritten = 0;

		while(nPos < nSize-1)
		{
			nSaveStep = min(nChunkSize, nSize-nPos);
			nWritten = file.Write(szData+nPos, nSaveStep);
			nPos += nSaveStep;

			nLoopCnt++;
			if(nLoopCnt%10 == 0)
				FormatIO_Base::RunLoop();
		}

		file.Close();
	}
}

void on_close_clicked (GtkButton *button, gpointer user_data)
{
	FileAttachmentDlg *pDlg = (FileAttachmentDlg *)user_data;

	//terminate dialog
	gtk_dialog_response(GTK_DIALOG(pDlg->m_pDialog), GTK_RESPONSE_OK);
}

gboolean att_list_on_drop(GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint time, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return FALSE;
	}

	TRACE("DnD: att_list_on_drop\n");

	gboolean  is_valid_drop_site = TRUE;

	// If the source offers a target
    if (is_valid_drop_site && context->targets)
    {
        GdkAtom target_type;
		if(is_format_provided(context, atomTextUriList))
			target_type = atomTextUriList;
		else
			return FALSE;

        // Request the data from the source, actual data will be received by 'drag-data-received' signal
        gtk_drag_get_data( widget, context, target_type, time);
    }
    else
	{
		is_valid_drop_site = FALSE;	// No target offered by source => error
	}

	return  is_valid_drop_site;
}

void att_list_drag_data_received(GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *selection_data, guint target_type, guint time, gpointer data)
{
	TRACE("DnD: att_list_drag_data_received\n");

	FileAttachmentDlg *pDlg = (FileAttachmentDlg *)data;
	GtkWidget *treeview = lookup_widget(pDlg->m_pDialog, "treeview");
	GtkListStore *store = (GtkListStore *)gtk_tree_view_get_model((GtkTreeView *)treeview);

	gchar   *_sdata;

	gboolean dnd_success = FALSE;
	gboolean delete_selection_data = FALSE;

	const gchar *name = gtk_widget_get_name (widget);
	TRACE("%s: att_list_drag_data_received\n", name);

	// Deal with what we are given from source
	if( (selection_data != NULL) &&
		(selection_data->length >= 0))
	{
		if (context->action == GDK_ACTION_ASK)
		{
			// Ask the user to move or copy, then set the context action
			context->action = GDK_ACTION_COPY;
		}
		if (context->action == GDK_ACTION_MOVE)
			delete_selection_data = TRUE;

		_sdata = (gchar*)selection_data-> data;
		TRACE("string: %s", _sdata);
		dnd_success = TRUE;

		//
		std::vector<std::string> lstFiles;
		std::string strFiles(_sdata);
		std::string::size_type nPos = strFiles.find('\n');
		while(nPos != std::string::npos){
			std::string strLine = strFiles.substr(0, nPos);
			replaceall(strLine, "\r", "");

			strFiles = strFiles.substr(nPos+1);

			//strip "file://" prefix
			if(strLine.substr(0, 7) == "file://"){
				#ifdef _WIN32
					strLine = strLine.substr(8);
				#else
					strLine = strLine.substr(7);
				#endif
			}
			HTMLParser::UnescapeURI(strLine);
			lstFiles.push_back(strLine);

			nPos = strFiles.find('\n');
		}

		int nMax = lstFiles.size();
		if(nMax > 0)
		{
			std::string strMsg(_("Do you want to attach files to the node?\n\n"));
			for(int i=0; i<nMax; i++){
				strMsg += lstFiles[i]; //_sdata;
				strMsg += "\n";
			}

			gint result = gtkMessageBox(strMsg.c_str(), GTK_BUTTONS_YES_NO);
			if(GTK_RESPONSE_YES == result)
			{
				int nIdx = ((FileAttachmentDlg *)data)->m_nNodeIdx;

				for(int i=0; i<nMax; i++)
				{
					std::string strLocaleFile;
					Utf8ToLocale(lstFiles[i].c_str(), strLocaleFile);
	#ifdef _WIN32
					replaceall(strLocaleFile, "/", "\\");
	#endif

					//TOFIX duplicate of on_add_file_clicked in FileAttachmentDialog
					File64 file;
					if(!file.Open(lstFiles[i].c_str())){
						TRACE("%s\n", strerror(errno));
						std::string strMsg = _("Failed to open input file!\n");
						strMsg += "(";
						strMsg += strLocaleFile;
						strMsg += ")";

						gtkMessageBox(strMsg.c_str());
						continue;
					}

					g_objCrashLog.AddMsg("ACTION: add attachment\n");

					AttInfo info;

					//show wait dialog
					ProgressDlg dlg(200, _("Loading file. Please wait..."), window1);

					int nTotal = 0;
					int nLoopCnt = 0;
					char szBuffer[2000];
					size_t nRead = 0;
					while((nRead = file.Read(szBuffer, sizeof(szBuffer))) > 0)
					{
						info.m_strData.append(szBuffer, nRead);
						nTotal += nRead;
						nLoopCnt++;

						if(nLoopCnt%10 == 0)
							FormatIO_Base::RunLoop();
					}
					file.Close();

					info.nDataSize = info.m_strData.size();
					info.m_strName = lstFiles[i];
					std::string::size_type nPos = info.m_strName.find_last_of("\\/");
					if(nPos != std::string::npos){
						info.m_strName = info.m_strName.substr(nPos+1);
					}

					g_doc.GetNodeByIdx(nIdx).m_lstAttachments.push_back(info);

					//refresh GUI list
					GtkTreeIter iter;
					gtk_list_store_append(store, &iter);

					gtk_list_store_set (store, &iter,
										0, info.m_strName.c_str(),
										1, info.nDataSize,
										-1);

					g_doc.SetModified(true);
					RefreshMainTitle();

					//push document change into undo/redo manager
					DocActionAtt *pAction = new DocActionAtt;
					pAction->m_bAddOp = true;
					pAction->m_info = info;
					pAction->m_nNodeIdx = pDlg->m_nNodeIdx;
					pAction->m_nAttIdx    = g_doc.GetNodeByIdx(pDlg->m_nNodeIdx).m_lstAttachments.size()-1;

					g_undoManager.AddAction(pAction);
					UpdateUndoRedoMenus();
				}

				//force redraw to refresh scroll bar
				gdk_window_clear_area (treeview->window,
                        treeview->allocation.x,
                        treeview->allocation.y,
                        treeview->allocation.width,
                        treeview->allocation.height);
				gtk_widget_queue_draw (treeview);
			}
		}
	}

	if (dnd_success == FALSE)
	{
		TRACE("DnD data transfer failed!\n");
	}

	gtk_drag_finish (context, dnd_success, delete_selection_data, time);
}


void on_item_executed (GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer  userdata)
{
	FileAttachmentDlg *pDlg = (FileAttachmentDlg *)userdata;

	int nSelRow = pDlg->GetSelRow();
	if(nSelRow < 0)
		return;

	std::string strTmp = GetTempDirectory();
#ifdef _WIN32
	EnsureTerminated(strTmp, '\\');
#else
	EnsureTerminated(strTmp, '/');
#endif
	strTmp += g_doc.GetNodeByIdx(pDlg->m_nNodeIdx).m_lstAttachments[nSelRow].m_strName;

	File64 file;
	if(!file.Open(strTmp.c_str(), F64_WRITE|F64_SHARE_READ|F64_OPEN_NEW)){
		gtkMessageBox(_("Failed to open output file!"));
		return;
	}

	//show wait dialog (only if not from cmd line)
	ProgressDlg dlg(200, _("Saving file. Please wait..."), window1);

	int nSize = g_doc.GetNodeByIdx(pDlg->m_nNodeIdx).m_lstAttachments[nSelRow].nDataSize;
	const char *szData = g_doc.GetNodeByIdx(pDlg->m_nNodeIdx).m_lstAttachments[nSelRow].m_strData.c_str();

	int nPos = 0;
	int nChunkSize = 2048;
	int nSaveStep  = 0;
	int nLoopCnt = 0;
	size_t nWritten = 0;

	while(nPos < nSize-1)
	{
		nSaveStep = min(nChunkSize, nSize-nPos);
		ASSERT(nSaveStep > 0);

		nWritten = file.Write(szData+nPos, nSaveStep);
		ASSERT(nWritten > 0);

		nPos += nSaveStep;

		nLoopCnt++;
		if(nLoopCnt%10 == 0)
			FormatIO_Base::RunLoop();
	}

	file.Close();

	//TOFIX wait to delete the temp file, on_edit_picture_activate
#ifdef _WIN32
	ShellExecute(NULL, "open", strTmp.c_str(), "", "", SW_SHOW);
#else
	ExecuteFile(strTmp.c_str(), "", "", NULL);
#endif
}
