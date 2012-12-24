////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements interface creation (main window, password dialog, options dialog, ...)
////////////////////////////////////////////////////////////////////////////

#include "config.h"

#if _MSC_VER > 1000
  #pragma warning(disable: 4786)
#endif

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>

#ifndef _WIN32
 #include <unistd.h>
#else
 #ifndef __MINGW32__
   #ifndef snprintf
    #define snprintf _snprintf
   #endif
 #endif
#endif

#include "lib/NoteDocument.h"
#include "lib/DocumentIterator.h"
#include "lib/IniFile.h"
#include "lib/FilePath.h"
#include "lib/DocActionManager.h"
#include "lib/File64.h"
#include "DocAction.h"
#include "mru.h"

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "TreeView.h"
#include "TextView.h"
#include "MainWnd.h"

#include "../res/internal_blank.xpm"
#include "../res/internal_folder.xpm"
#include "../res/internal_help.xpm"
#include "../res/internal_lock.xpm"
#include "../res/internal_new_dir.xpm"
#include "../res/internal_recycle.xpm"

#ifdef _WIN32
 #include "_win/RegisterExtension.h"
 #include <windows.h>
#else
 #include <sys/types.h>	//getpid
 #include <unistd.h>
 #include <signal.h>	//kill
 #include <errno.h>
#endif

extern NoteDocument g_doc;
extern GtkWidget *window1;
extern DocActionManager g_undoManager;
extern IniFile g_objIni;
TreeView g_tree;
TextView g_text;
MainWindow g_wnd;
bool g_bBusyCursor = false;
bool g_bMsgBox = false;

int load_file(const char *filename, const char *szPassword = NULL, int nCurNodeIdx = -1);
void UpdateTextFromScreen();
void register_extensions_check();

void cell_edited_callback(GtkCellRendererText *cell, gchar *path_string, gchar *new_text, gpointer user_data);
void on_MRU_activate (GtkMenuItem *menuitem, gpointer user_data);
void treeview_show_popup_menu();
bool copy_file(const char *szFrom, const char *szTo);

//
// MRU list
//
MRU g_objMRU;
GtkWidget *g_menuitem5;

gboolean treeview1_popup_menu_event_handler (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	// Ignore double-clicks and triple-clicks
	if (event->button == 3 && event->type == GDK_BUTTON_RELEASE)
	{
		treeview_show_popup_menu();
	}
	return FALSE;
}

void cell_edited_start_callback (GtkCellRenderer *renderer, GtkCellEditable *editable, gchar *path, gpointer user_data)
{
	TreeView *pTree = (TreeView *)user_data;
	if(pTree)
		pTree->m_bCellEdited = true;	//mark state

	//set the font that matches user defined for for the tree
	std::string strFont1;
	g_objIni.GetValue("Display", "TreeFont", strFont1, "");
	GtkEntry *entry = (GtkEntry *)editable;
	if(!strFont1.empty()){
		PangoFontDescription *desc1 = pango_font_description_from_string(strFont1.c_str());
		gtk_widget_modify_font(GTK_WIDGET(entry), desc1);
		pango_font_description_free(desc1);
	}

	//limit the size of edit field to the width of the tree control (GTK+ bug)
	int nWidth = 0;
	gdk_window_get_size(GTK_WIDGET(g_tree.m_pWidget)->window, &nWidth, NULL);
	/*
	gtk_widget_set_size_request(GTK_WIDGET(entry), nWidth, -1);
	gdk_window_move_resize (GTK_WIDGET(entry)->window,
  			    GTK_WIDGET(entry)->allocation.x, GTK_WIDGET(entry)->allocation.y,
  			    nWidth, GTK_WIDGET(entry)->allocation.height);
	gtk_widget_set_usize(GTK_WIDGET(entry), nWidth, -1);
	gtk_widget_queue_resize(GTK_WIDGET(entry));

	GtkAllocation allocate;
	allocate.x = GTK_WIDGET(entry)->allocation.x;
	allocate.y = GTK_WIDGET(entry)->allocation.y;
	allocate.height = GTK_WIDGET(entry)->allocation.height;
	allocate.width  = nWidth;
	gtk_widget_size_allocate(GTK_WIDGET(entry), &allocate);
	*/
}

void cell_edited_cancel_callback (GtkCellRenderer *renderer, gpointer user_data)
{
	TreeView *pTree = (TreeView *)user_data;
	if(pTree)
		pTree->m_bCellEdited = false;	//mark state
}

void cell_edited_callback(GtkCellRendererText *cell, gchar *path_string, gchar *new_text, gpointer user_data)
{
	//do not allow empty node titles
	if(0 == strlen(new_text))
		return;

	//get document node index from GUI tree iterator
	GtkTreePath* path1 = gtk_tree_path_new_from_string( path_string );
	int nIdx = NodeIdxFromPath(path1);

	//
	//store new text into the document node
	//
	std::string strOldTitle;

	//TOFIX separate method -> SetNodeTitle(idx, szText) -> w/o undo inside
	if(nIdx > -1)
	{
		strOldTitle = g_doc.GetNodeByIdx(nIdx).GetTitle();
		g_doc.GetNodeByIdx(nIdx).SetTitle(new_text);
	}

	//store new text into the tree store for given cell
	GtkWidget *treeview = lookup_widget(window1, "treeview1");
	GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);

	if(nIdx > -1){
		GtkTreeIter iter;
		gtk_tree_model_get_iter(model, &iter, path1);
		gtk_tree_store_set (GTK_TREE_STORE(model), &iter, STORE_IDX_TEXT, new_text, -1);

		//update node title label
		set_title_bar(new_text);

		//if the name was actually changed
		if(0 != strcmp(strOldTitle.c_str(), new_text))
		{
			//push document change into undo/redo manager
			DocAction *pAction = new DocAction;
			pAction->SetType(ACT_NODE_RENAME);
			pAction->SetDoc(g_doc);
			pAction->m_nNodeIndex = nIdx;  //TOFIX recursive index
			pAction->m_strNodeNameNew = new_text;
			pAction->m_strNodeNameOld = strOldTitle;

			g_undoManager.AddAction(pAction);
			UpdateUndoRedoMenus();
			g_doc.SetModified(true);
		}

		RefreshMainTitle(); // updates modified doc state in window title bar
	}

	gtk_tree_path_free(path1);

	TreeView *pTree = (TreeView *)user_data;
	if(pTree)
		pTree->m_bCellEdited = false;	//mark state
}

void on_textview_edited( GtkTextBuffer *widget, gpointer data)
{
	bool bModified = g_doc.IsModified();
	if(!bModified){
		g_doc.SetModified(true);
		RefreshMainTitle();
	}
}

//
// Helper methods
//
//TOFIX move to DocTreeNavigation or similar class ?
int NodeIdxFromPath(GtkTreePath* path1)
{
	gint* arrIndices = gtk_tree_path_get_indices(path1);
	int nMax = gtk_tree_path_get_depth(path1);

	int nIdx = -1;
	int nParentID = -1;
	DocumentIterator itDoc(g_doc);
	for(int i=0; i<nMax; i++)
	{
		//go to the next sibling node
		nIdx = itDoc.GetChildIdx(nParentID, arrIndices[i]);
		if(nIdx < 0)
			break;	//TOFIX assert
		nParentID = g_doc.GetNodeByIdx(nIdx).m_nID;
	}

	return nIdx;
}

//TOFIX move to DocTreeNavigation or similar class ?
bool PathFromNodeIdx(int nIdx, GtkTreePath *&path1)
{
	std::string strPath;

	//prepare path string
	while(nIdx >= 0)
	{
		//append level at start
		char szLevel[30]="";
		snprintf(szLevel, sizeof(szLevel), "%d:", g_doc.GetNodeByIdx(nIdx).m_nSiblingIdx);
		strPath.insert(0, szLevel);

		//calculate parent index
		nIdx = g_doc.GetIdxFromID(g_doc.GetNodeByIdx(nIdx).m_nParentID);
	}

	//strip off last ':' character
	strPath = strPath.substr(0, strPath.size()-1);

	//now convert that path string into the path object
	path1 = gtk_tree_path_new_from_string(strPath.c_str());
	return (path1 != NULL);
}

//TOFIX move to DocTreeNavigation or similar class ?
bool IteratorFromNodeIdx(int nIdx, GtkTreeIter &iter)
{
	GtkTreePath *path1 = NULL;
	if(!PathFromNodeIdx(nIdx, path1))
		return false;

	GtkWidget *treeview = lookup_widget(window1, "treeview1");
	GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);
	bool bResult = (FALSE != gtk_tree_model_get_iter(model, &iter, path1));
	gtk_tree_path_free(path1);
	return bResult;
}

void on_MRU_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	//pull file name from MRU list
	unsigned long nIdx = (unsigned long)user_data;

	if(nIdx < (unsigned long)g_objMRU.GetFileCount())
	{
		std::string strFile = g_objMRU.GetFile(nIdx);

		//allow password being entered up to 3 times
		int nResult = 0;
		int nTryCnt = 0;
		while(nTryCnt < 3){
			nResult = load_file(strFile.c_str());
			nTryCnt ++;
			if(DOC_LOAD_WRONG_PASSWORD != nResult)
				break;
		}


		//do not delete MRU for file that exists, but failed to open
		g_objMRU.Change(strFile.c_str(), (DOC_LOAD_NOT_FOUND != nResult));
	}
	else
	{
		int nRes = gtkMessageBox(_("Are you sure you want to delete history list?"),GTK_BUTTONS_YES_NO);
		if(GTK_RESPONSE_YES == nRes)
			g_objMRU.Clear();	//delete MRU
	}
}

void register_extensions_check()
{
#ifdef _WIN32
	std::string strApp = GetAppPath();
	std::string strCmd = strApp;
	strCmd += " \"%1\"";

	RegisterFileExtension reg;
	reg.SetFormatName("NoteCase.PlainFormat");
	reg.SetExtension(".ncd");
	reg.SetDescription("NoteCase unencrypted file format");
	reg.SetDefaultIcon(strApp.c_str(), 0);
	reg.AddShellAction("open", strCmd.c_str());
	reg.SetShellDefault("open");

	//register our encrypted format
	if(!reg.IsRegistered(".ncd"))
		reg.UpdateRegistry();

	reg.ClearData();
	reg.SetFormatName("NoteCase.EncryptedFormat");
	reg.SetExtension(".nce");
	reg.SetDescription("NoteCase encrypted file format");
	reg.SetDefaultIcon(strApp.c_str(), 0);
	reg.AddShellAction("open", strCmd.c_str());
	reg.SetShellDefault("open");

	//register our encrypted format
	if(!reg.IsRegistered(".nce"))
		reg.UpdateRegistry();

#else
	//install the formats into the .local database
	std::string strMime = GetHomeDir();
	EnsureTerminated(strMime, '/');
	strMime += ".local/share/mime/";

	std::string strDir = strMime + "packages/";
	EnsureDirExists(strDir);

	strDir = strMime + "packages/";
	EnsureDirExists(strDir);
	strDir += "notecase.xml";

	std::string strSrcFile;
	#ifdef _NOKIA_MAEMO
		strSrcFile = "/usr/share/mime/packages/notecase.xml";
	#else
		strSrcFile = "/usr/share/mime/packages/notecase.xml";
	#endif

	//copy file
	copy_file(strSrcFile.c_str(), strDir.c_str()); //TOFIX error report

	//update the system
	std::string strCmd = "update-mime-database " + strMime;
	/*bool bOK = */g_spawn_command_line_async(strCmd.c_str(), NULL);
	//TOFIX call desktop-file-utils?
#endif
}

void set_title_bar(const char *szText, bool bRed)
{
	GtkWidget *label1 = lookup_widget(window1, "label1");

	if(bRed){
		GdkColor red = { 0, 0xFFFF, 0, 0 };
		gtk_widget_modify_fg(GTK_WIDGET(label1), GTK_STATE_NORMAL, &red);
		gtk_label_set_text(GTK_LABEL(label1), szText);
	}
	else{
		GdkColor black = { 0, 0, 0, 0 };
		gtk_widget_modify_fg(GTK_WIDGET(label1), GTK_STATE_NORMAL, &black);
		gtk_label_set_text(GTK_LABEL(label1), szText);
	}
}

void set_status_bar(const char *szText, bool bRed)
{
	GtkWidget *statusbar1 = lookup_widget(window1, "statusbar1");

	const int nID = 1; //TOFIX? gtk_statusbar_get_context_id(GTK_STATUSBAR(statbar1), NULL);
	gtk_statusbar_pop(GTK_STATUSBAR(statusbar1), nID);

	//calculate status bar label
	GList *children = gtk_container_get_children(GTK_CONTAINER(statusbar1));
	GList *children1 = gtk_container_get_children(GTK_CONTAINER(g_list_nth_data (children, 0)));
	GtkWidget *label = (GtkWidget *)g_list_nth_data (children1, 0);

	if(bRed){
		GdkColor red = { 0, 0xFFFF, 0, 0 };
		gtk_widget_modify_fg(GTK_WIDGET(label), GTK_STATE_NORMAL, &red);
		gtk_statusbar_push (GTK_STATUSBAR(statusbar1), nID, szText);
	}
	else{
		GdkColor black = { 0, 0, 0, 0 };
		gtk_widget_modify_fg(GTK_WIDGET(label), GTK_STATE_NORMAL, &black);
		gtk_statusbar_push (GTK_STATUSBAR(statusbar1), nID, szText);
	}

	g_list_free(children1);
	g_list_free(children);
}

int gtkMessageBox(const char *szText, int nButtons, int nIcon)
{
	g_bMsgBox = true;

	GtkWidget* msgbox;
#if 0
	msgbox = gtk_message_dialog_new ( (GtkWindow*)window1,
		GTK_DIALOG_DESTROY_WITH_PARENT,
		(GtkMessageType)nIcon,
		(GtkButtonsType)nButtons,
		szText);
#else
	msgbox = gtk_dialog_new(); //TOFIX support for buttons?
#endif

	gtk_window_set_title (GTK_WINDOW (msgbox), _("Info"));
	gtk_window_set_modal (GTK_WINDOW (msgbox), TRUE);
	gtk_window_set_skip_pager_hint (GTK_WINDOW (msgbox), TRUE);
	gtk_window_set_type_hint (GTK_WINDOW (msgbox), GDK_WINDOW_TYPE_HINT_DIALOG);
#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (msgbox), TRUE);
#endif
	gtk_window_set_transient_for(GTK_WINDOW (msgbox), GTK_WINDOW(window1));   //set parent

#if GTK_CHECK_VERSION(2,4,0) //new API TOFIX set proper version
	#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
		//gtk_window_set_keep_above(GTK_WINDOW (msgbox), TRUE);
	#endif
#endif
	gtk_window_set_resizable (GTK_WINDOW (msgbox), FALSE);
	gtk_widget_realize(msgbox);
	gdk_window_set_decorations(msgbox->window, (GdkWMDecoration)(GDK_DECOR_BORDER|GDK_DECOR_TITLE));

#if 1
	GtkWidget *dialog_vbox3 = GTK_DIALOG (msgbox)->vbox;
	gtk_widget_show (dialog_vbox3);
	gtk_container_set_border_width  (GTK_CONTAINER (dialog_vbox3), 7);
	gtk_box_set_spacing(GTK_BOX (dialog_vbox3), 10);

	GtkWidget *label21 = gtk_label_new (szText);
	gtk_widget_show (label21);
	gtk_box_pack_start (GTK_BOX (dialog_vbox3), label21, TRUE, TRUE, 0);

	if(nButtons == GTK_BUTTONS_OK){
		gtk_dialog_add_button(GTK_DIALOG(msgbox), GTK_STOCK_OK, GTK_RESPONSE_OK);
	}
	else if(nButtons == GTK_BUTTONS_YES_NO){
		gtk_dialog_add_button(GTK_DIALOG(msgbox), GTK_STOCK_YES,    GTK_RESPONSE_YES);
		gtk_dialog_add_button(GTK_DIALOG(msgbox), GTK_STOCK_NO,     GTK_RESPONSE_NO);
	}

	gtk_dialog_set_has_separator (GTK_DIALOG(msgbox), FALSE);
#endif

	gint result = gtk_dialog_run (GTK_DIALOG (msgbox));

	gtk_widget_destroy (msgbox);

	g_bMsgBox = false;

	return result;
}

int gtkMessageBoxYNC(const char *szText)
{
	g_bMsgBox = true;

	GtkWidget* msgbox;
	msgbox = gtk_message_dialog_new ( (GtkWindow*)window1,
		GTK_DIALOG_DESTROY_WITH_PARENT,
		(GtkMessageType)GTK_MESSAGE_INFO,
		(GtkButtonsType)GTK_BUTTONS_NONE,
		"%s",
		szText);

	gtk_window_set_title (GTK_WINDOW (msgbox), _("Info"));
	gtk_window_set_modal (GTK_WINDOW (msgbox), TRUE);
	gtk_window_set_skip_pager_hint (GTK_WINDOW (msgbox), TRUE);
	gtk_window_set_type_hint (GTK_WINDOW (msgbox), GDK_WINDOW_TYPE_HINT_DIALOG);
#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (msgbox), TRUE);
#endif
#if GTK_CHECK_VERSION(2,4,0) //new API TOFIX set proper version
	#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
		//gtk_window_set_keep_above(GTK_WINDOW (msgbox), TRUE);
	#endif
#endif
	gtk_window_set_resizable (GTK_WINDOW (msgbox), FALSE);
	gtk_widget_realize(msgbox);
	gdk_window_set_decorations(msgbox->window, (GdkWMDecoration)(GDK_DECOR_BORDER|GDK_DECOR_TITLE));

	gtk_dialog_add_button(GTK_DIALOG(msgbox), GTK_STOCK_YES,    GTK_RESPONSE_YES);
	gtk_dialog_add_button(GTK_DIALOG(msgbox), GTK_STOCK_NO,     GTK_RESPONSE_NO);
	gtk_dialog_add_button(GTK_DIALOG(msgbox), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);

	gint result = gtk_dialog_run (GTK_DIALOG (msgbox));
	gtk_widget_destroy (msgbox);

	//map unwanted result code to Cancel event
	if(GTK_RESPONSE_DELETE_EVENT == result)
		result = GTK_RESPONSE_CANCEL;

	g_bMsgBox = false;

	return result;
}

const char **InternalIcon_GetFromIdx(int nIdx)
{
	switch (nIdx)
	{
		case 0:	return (const char **)&blank_xpm;
		case 1:	return (const char **)&folder_xpm;
		case 2:	return (const char **)&help_xpm;
		case 3:	return (const char **)&lock_xpm;
		case 4:	return (const char **)&new_dir_xpm;
		case 5:	return (const char **)&recycle_xpm;
		default: return NULL;
	}
}

int InternalIcon_Name2Index(const char *szName)
{
	if(!szName)
		return -1;

	if(0 == strcmp("blank", szName))
		return 0;
	else if(0 == strcmp("folder", szName))
		return 1;
	else if(0 == strcmp("help", szName))
		return 2;
	else if(0 == strcmp("lock", szName))
		return 3;
	else if(0 == strcmp("new_dir", szName))
		return 4;
	else if(0 == strcmp("recycle", szName))
		return 5;

	return -1;
}

const char *InternalIcon_Index2Name(int nIndex)
{
	switch (nIndex)
	{
		case 0:	return "blank";
		case 1:	return "folder";
		case 2:	return "help";
		case 3:	return "lock";
		case 4:	return "new_dir";
		case 5:	return "recycle";
		default: return NULL;
	}
}

const char *GetLoadErrorString(int nErrCode)
{
	switch(nErrCode)
	{
		case DOC_LOAD_OK:
			return _("OK!");
		case DOC_LOAD_ABORTED:
			return _("Loading aborted by user!");
		case DOC_LOAD_NOT_FOUND:
			return _("File not found!");
		case DOC_LOAD_WRONG_PASSWORD:
			return _("Invalid document password!");
		case DOC_LOAD_UNKNOWN_FORMAT:
			return _("Unsupported document format!");
		case DOC_LOAD_FORMAT_ERROR:
			return _("Error when parsing document (bad formatting)!");
		case DOC_LOAD_ERROR:
			return _("Failed to load the file!");
		case DOC_LOAD_PERMISSIONS:
			return _("Inadequate file permissions!");
		case DOC_LOAD_NOT_SUPPORTED:
			return _("Load operation is not supported for this format!");
	}

	return _("Unknown error!");	//should never get this
}

const char *GetSaveErrorString(int nErrCode)
{
	switch(nErrCode)
	{
		case DOC_SAVE_OK:
			return _("OK!");
		case DOC_SAVE_ABORTED:
			return _("Saving aborted by user!");
		case DOC_SAVE_UNKNOWN_FORMAT:
			return _("Unsupported format!");
		case DOC_SAVE_ERR_OVERWRITE:
			return _("Failed to overwrite target file!");
		case DOC_SAVE_ERR_FILE_OPEN:
			return _("Failed to open output file! Please check permissions or available disk size!");
		case DOC_SAVE_ERR_EXE_OPEN:
			return _("Failed to open application file!");
		case DOC_SAVE_ERROR:
			return _("Failed to save the file!");
		case DOC_SAVE_NOT_SUPPORTED:
			return _("Save operation is not supported for this format!");
	}

	return _("Unknown error!");	//should never get this
}

void ShowBusyCursor()
{
	if(!g_bBusyCursor)
	{
		// set busy cursor and disable the window
		GdkCursor *cursor=gdk_cursor_new(GDK_WATCH);
		gtk_widget_set_sensitive(GTK_WIDGET(window1),FALSE);

		gdk_pointer_grab(window1->window,
				TRUE,
				(GdkEventMask)(GDK_POINTER_MOTION_MASK|GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK),
				NULL,
				cursor,
				GDK_CURRENT_TIME);
		gdk_cursor_unref (cursor);

		g_bBusyCursor = true;
	}
}

void HideBusyCursor()
{
	if(g_bBusyCursor)
	{
		// restore the cursor and enable the window
		gdk_pointer_ungrab(GDK_CURRENT_TIME);
		gtk_widget_set_sensitive(GTK_WIDGET(window1),TRUE);

		g_bBusyCursor = false;
	}
}

unsigned int GetCurrentPID()
{
#ifdef _WIN32
	return GetCurrentProcessId();
#else
	return getpid();
#endif
}

bool IsPIDRunning(unsigned int nPID)
{
#ifdef _WIN32
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, nPID);
	if(NULL == hProcess)
		return false;
	CloseHandle(hProcess);
#else
	if(0 != kill(nPID, 0))
		if(errno == ESRCH)
			return false;
#endif

	return true;
}

bool copy_file(const char *szFrom, const char *szTo)
{
	//copy file
	File64 fileSrc, fileDst;
	if(!fileSrc.Open(szFrom, F64_READ|F64_SHARE_READ|F64_OPEN_EXISTING)){
		TRACE("copy_file: fail to open file [%s]\n", szFrom);
		return false;
	}
	if(!fileDst.Open(szTo, F64_WRITE|F64_SHARE_READ|F64_OPEN_NEW)){
		TRACE("copy_file: fail to create file [%s]\n", szTo);
		return false;
	}
	int nRead = 0;
	char szBuffer[1024];
	while((nRead = fileSrc.Read(szBuffer, sizeof(szBuffer))) > 0){
		fileDst.Write(szBuffer, nRead);
	}
	return true;
}
