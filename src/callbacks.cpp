////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implementations of menu action handlers
////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
  #pragma warning(disable:4786)
  #pragma warning(disable: 4996)	//ignore deprecated functions warning
#endif

#ifdef _WIN32
 #include <windows.h>
 #ifndef __MINGW32__
  #ifndef snprintf
   #define snprintf _snprintf
  #endif
  #define strcasecmp stricmp
 #endif
#endif

#include "config.h"
#include "lib/debug.h"
#include "ExecuteFile.h"

#ifndef _WIN32
 #include <libgen.h>		//basename
#endif

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <pango/pango.h>
#include <glib.h>
#include <time.h>
#include <errno.h>
#include "lib/NoteDocument.h"
#include "lib/DocumentIterator.h"
#include "lib/FilePath.h"
#include "lib/TextSearch.h"
#include "lib/IniFile.h"
#include "lib/DocActionManager.h"
#include "DocAction.h"
#include "DocActionSort.h"
#include "DocActionPix.h"
#include "DocActionFinish.h"
#include "DocActionFinishDel.h"
#include "DocActionFmt.h"
#include "mru.h"
#include "LinkPropertiesDlg.h"
#include "FileSaveAsDlg.h"
#include "gui/FileDialog.h"
#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "OptionsDialog.h"
#include "FindDialog.h"
#include "FindReplaceDlg.h"
#include "FindReplaceInfo.h"
#include "gui/GuiLanguage.h"
#include "TreeView.h"
#include "TextView.h"
#include "NodePropertiesDlg.h"
#include "FileExportDlg.h"
#include "lib/LinkInfo.h"
#include "lib/File64.h"
#include "MainWnd.h"
#include "lib/CircularBuffer.h"
#include "FileAttachmentDlg.h"
#include "ShortcutsListDlg.h"
#include "DateTimeDlg.h"
#include "PortableTrayIcon.h"
#include "lib/EnumDirectory.h"
#include "PasswordDialog.h"
#include "AboutDlg.h"
#include "gui/ProgressDlg.h"
#include "DocActionAtt.h"
#include "EditDlg.h"
#include "lib/HtmlParser.h"
#include <stdlib.h>
#include <math.h>

#ifdef _NOKIA_MAEMO
 #include "../res/text_bold.xpm"
 #include "../res/text_italic.xpm"
 #include "../res/text_underline.xpm"
 #include "../res/text_strikethrough.xpm"
#endif

#ifdef _WIN32
 #include <io.h> //access
 #define access _access
#endif

#ifndef min
 #define min(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef max
 #define max(a,b) (((a)>(b))?(a):(b))
#endif

GtkWidget *g_pNodePropWidget = NULL;
GdkColor g_linkColor = { 0, 0, 0, 0xFFFF };
GdkColor g_blackColor = { 0, 0, 0, 0 };

//
// cache individual shortcuts for faster keypress handler
//

//tree only actions
ShortcutDef g_defTreeNodeNew;
ShortcutDef g_defTreeNodeNewChild;
ShortcutDef g_defTreeNodeDelete;
ShortcutDef g_defTreeNodeRename;
ShortcutDef g_defTreeNodeMoveUp;
ShortcutDef g_defTreeNodeMoveDown;
ShortcutDef g_defTreeNodeMoveLeft;
ShortcutDef g_defTreeNodeMoveRight;
ShortcutDef g_defTreeNodeProperties;
ShortcutDef g_defTreeNodeDelFinished;
ShortcutDef g_defTreeNodeToggleFinished;
ShortcutDef g_defTreeNodeSortChildrenAsc;
ShortcutDef g_defTreeNodeSortChildrenDsc;
ShortcutDef g_defTreeNodeSortRootAsc;
ShortcutDef g_defTreeNodeSortRootDsc;
ShortcutDef g_defTreeNodeExpandAll;
ShortcutDef g_defTreeNodeColapseAll;
ShortcutDef g_defTreeCopyBranchStruct;
ShortcutDef g_defTreePasteBranchRoot;

extern GdkColor g_rgbTextViewBkColor;

//formatting override settings
//(pressing ctrl+B with no selection generates temporary flag that forces the un-bold/bold on some offset, as soon as you enter text somewhere else or scroll the caret, this flag is reset!!!!)
int g_nTextOffset = -1;
int g_nBoldFmtTag = -1;	//bold,un-bold,none
int g_nItalicFmtTag = -1;
int g_nUnderlineFmtTag = -1;
int g_nStrikeFmtTag = -1;
void reset_format_override();

int  load_file_embedded(int nCurNodeIdx = -1);
int  load_file(const char *filename, const char *szPassword = NULL, int nCurNodeIdx = -1);
bool save_file(const char *filename, bool bRememberPath = true, bool bAutosave = false, const char *szPassword = NULL);
void on_textview_edited(GtkTextBuffer *, gpointer data);
bool text_find(bool bAutoProceedToStart = false, bool bNotifyDone = true);
gboolean indent_new_line_timer(gpointer data);
gboolean autosave_timer(gpointer data);
gboolean enc_unload_timer(gpointer data);
gboolean clear_formatting_timer(gpointer data);
void calc_word_count(const gchar *szText, int &nWords, int &nLines, int &nCharacters);
void export_document(const NoteDocument &document, int nExportMode, const std::string &strExportFile, const std::string &strPostProcessScript, const std::string &strCSS, int nEmbedCSS = -1, bool bCheckOverwrite = true, bool bExportLinked = false, int nFormat = FORMAT_NONE, int nExportBranchIdx=-1);
std::string calc_node_info(int nIdx);
bool PathFromNodeIdx(int nIdx, GtkTreePath *&path1);
int  GetSelectedNodeIdx();
void UpdateNodeIcon(int nIdx);
void RememberSelectionBounds(int nodeIdx);
bool Utf8ToLocale(const char *szTxt, std::string &strResult);
void ExecuteLink(LinkInfo &link);
bool Execute(const std::string& path, const std::string& args, const std::string& dir); //TOFIX
const char *PasswordCallback(const char *szFile, bool bSave);
int FormatCallback();
void textview_selection_changed(GtkTextBuffer *textbuffer, GtkTextIter *arg1, GtkTextMark *arg2, gpointer user_data);
bool OnListBackupFilesEnum(const char *szFile, void *data);
void replaceall(std::string &strData, const char *szFind, const char *szReplace);
int get_view_mode();

bool g_bAutoSaveOnClose = true;
bool g_bSkipMemOnlyAutosave = true;
bool g_bExportDataAsSeparateFile = true;
bool g_bTreeToTheRight = false;
bool g_bTreeToTheRightInitial = false;
bool g_bQuitByMenu = false;
bool g_bCloseToTray = false;
bool g_bCloseDlgOnTextEnter = true;
int	 g_nPasteOffset = 0;
int  g_nPasteLen	= 0;
bool g_bIgnorePaste = false;
bool g_bIoOperationInProgress = false;

extern bool g_bMinimizeToTray;
extern GuiLanguage g_lang;
extern GtkWidget *window1;
extern MRU g_objMRU;
extern TreeView g_tree;
extern TextView g_text;
extern MainWindow g_wnd;
extern IniFile g_objIni;
extern CircularBuffer g_objCrashLog;
extern PortableTrayIcon g_tray;
extern unsigned int g_nProcessID;
extern bool g_bMsgBox;
extern int g_nDefaultNodeIconType;
extern std::string g_strDefaultIconFile;
extern GdkAtom atomTextUriList;
extern GdkAtom atomString;

struct export_entry {
	export_entry(){ bParsed = false; bExported = false; }
	void operator == (const export_entry &that){
		strDoc = that.strDoc;
		bParsed = that.bParsed;
		bExported = that.bExported;
	}

	std::string strDoc;
	bool bParsed;
	bool bExported;
};

//stack with file list (with no duplicate entries) for recursive export
std::vector<export_entry> g_lstRecursiveExportStack;

int g_nAutosaveTimer = 0;
int g_nEncUnloadTimer = 0;
NoteDocument g_doc;
DocActionManager g_undoManager(UNDO_LIST_SIZE);
FindReplaceInfo g_objSearchInfo;
std::vector<NoteNode> g_lstNodeClipboard;

#define UNDO_MERGE_TIMEOUT	10	// 10 seconds

void on_new1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	//check to save current document
	UpdateTextFromScreen();

	if(g_doc.IsModified())
	{
		//ask to save the document
		gint result = gtkMessageBoxYNC(_("Document has been modified! Do you want to save it?"));
		if(GTK_RESPONSE_CANCEL == result)
			return;

		if(GTK_RESPONSE_YES == result)
		{
			bool bSaved = false;
			on_save1_activate(NULL, &bSaved);

			if(!bSaved)	return;	//quit action if user cancels old document saving
		}
	}

	g_objCrashLog.AddMsg("ACTION: new document\n");

	//
	// clear the content (both screen and memory)
	//
	g_tree.Clear(); //clear tree view
	g_text.Clear(); //clear edit view
	g_doc.Close();  //close the document
	g_text.SetEditable(false);	// no selected node
	set_title_bar("", false);
	set_read_only(false);
	g_doc.m_nActiveNodeIdx = -1;

	RefreshMainTitle();

	autosave_shutdown();
	restart_enc_unload_timer();

	//"change password" - set proper menu item state
	GtkWidget *change_pass1 = lookup_widget(window1, "change_pass1");
	gtk_widget_set_sensitive (change_pass1, FALSE);

	//"lock document" - set proper menu item state
	GtkWidget *lock1 = lookup_widget(window1, "lock1");
	gtk_widget_set_sensitive (lock1, FALSE);

	//focus the tree widget
	gtk_window_set_focus(GTK_WINDOW(window1), g_tree.m_pWidget);
}

void on_open1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	FileDialog dlg(true);
	dlg.SetTitle(_("Open document"));

	//define file filters
	dlg.AddFilter(_("All supported formats (*.ncd,*.nce)"), "*.ncd|*.nce");
	dlg.AddFilter(_("NoteCase document (*.ncd)"), "*.ncd");
	dlg.AddFilter(_("NoteCase encrypted document (*.nce)"), "*.nce");
	dlg.AddFilter(_("All files (*)"), "*");

	//set initial directory from INI (store last used)
	std::string strDefaultDir;
	std::string strDir;
	g_objIni.GetValue("Cache", "LastOpenDir", strDir, "");
	if(strDir.size() > 0 && 0 == access(strDir.c_str(), 00))
		strDefaultDir = strDir;
	else
		strDefaultDir = GetHomeDir();

	dlg.SetDirectory(strDefaultDir.c_str());

	//show dialog
	if(dlg.DoModal())
	{
		const gchar *filename = dlg.GetFilename();
		strDefaultDir = dlg.GetDirectory();
		dlg.Close();

		//check if the format is supported
		std::string strCurExt = GetFileExt(filename);
		if( strCurExt != ".ncd" &&
			strCurExt != ".nce" &&
			strCurExt != ".ncdb" &&
			strCurExt != ".ncde")
		{
			gtkMessageBox(_("Unsupported format!"));
			return;
		}

		//allow password being entered up to 3 times
		int nResult = 0;
		int nTryCnt = 0;
		while(nTryCnt < 3){
			nResult = load_file(filename);
			nTryCnt ++;
			if(DOC_LOAD_WRONG_PASSWORD != nResult)
				break;
		}

		set_read_only(g_doc.IsReadOnly());

		//do not delete MRU for file that exists, but failed to open
		g_objMRU.Change(filename, (DOC_LOAD_NOT_FOUND != nResult));

		//store last open directory
		g_objIni.SetValue("Cache", "LastOpenDir", strDefaultDir.c_str());
		g_objIni.Save();

		g_objCrashLog.AddMsg("ACTION: open document\n");
	}

	restart_enc_unload_timer();
}

void on_close1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	on_new1_activate(menuitem, user_data);
}

void on_save1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	if( g_doc.GetPath().size() > 0 ){
		bool bDone = save_file(g_doc.GetPath().c_str());
		if(NULL != user_data)
			*((bool *)user_data) = bDone;	//store result

		RefreshMainTitle();
	}
	else
		on_save_as1_activate(menuitem, user_data);

	g_objCrashLog.AddMsg("ACTION: save document\n");

	restart_enc_unload_timer();
}

void on_save_as1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	FileSaveAsDlg dlg;

	//calculate default document name and path
	std::string strDefaultName;
	std::string strDefaultDir;

	if( g_doc.GetPath().size() > 0 ){
		strDefaultName = g_doc.GetPath();
		std::string::size_type nPos = strDefaultName.find_last_of('/');
		if(nPos != std::string::npos)
			strDefaultName = strDefaultName.substr(nPos+1);
	#ifdef _WIN32
		nPos = strDefaultName.find_last_of('\\');
		if(nPos != std::string::npos)
			strDefaultName = strDefaultName.substr(nPos+1);
	#endif

		strDefaultDir = g_doc.GetPath();
		strDefaultDir = strDefaultDir.substr(0, strDefaultDir.length()-strDefaultName.length());
	}
	else
		strDefaultName = "untitled.ncd";

	dlg.SetFilename(strDefaultName.c_str());

	if(strDefaultDir.empty())
	{
		//get last used document folder
		std::string strDir;
		g_objIni.GetValue("Cache", "LastSaveDir", strDir, "");
		if( strDir.size() > 0 && 0 == access(strDir.c_str(), 00))
			strDefaultDir = strDir;
		else{
			//default directory
			strDefaultDir = GetHomeDir();
			EnsureTerminated(strDefaultDir, '/');
			#ifdef _NOKIA_MAEMO
				//Maemo file selector is not easy to use, by default set correct document directory
				strDefaultDir += "MyDocs/.documents/";
			#endif
		}
	}

	dlg.SetDirectory(strDefaultDir.c_str());

	//assume failure until success
	if(NULL != user_data)
		*((bool *)user_data) = false;	//store result

	//start dialog
	dlg.Create();
	if(GTK_RESPONSE_OK == dlg.ShowModal())
	{
		const gchar *filename = dlg.GetFileName();

		//cache current directory before closing
		strDefaultDir = dlg.GetFilePath();

		//calculate full path from directory + name
		std::string strFilePath = strDefaultDir;
		if(strFilePath.size() > 0){
			int nLastChar = strFilePath.at(strFilePath.size()-1);
		#ifdef _WIN32
			if(nLastChar != '/' && nLastChar != '\\' ) strFilePath += "\\";
		#else
			if(nLastChar != '/') strFilePath += "/";
		#endif
		}
		strFilePath += filename;

		//convert from UTF8 to locale encoding
		std::string strLocaleFile;
		Utf8ToLocale(strFilePath.c_str(), strLocaleFile);

		//force valid file extension using selected filter settings
		const gchar *filtername = dlg.GetSelectedFormat();
		if(filtername && strlen(filename) > 0)
		{
			std::string strExt;
			if(NULL != strstr(filtername, "*.ncd"))
				strExt = ".ncd";
			else if(NULL != strstr(filtername, "*.nce"))
				strExt = ".nce";

			std::string strCurExt = strLocaleFile.substr(strLocaleFile.size() - 4, 1000); //all my extensions have fixed size = 4

			//if no extension set, select the one set by current filter
			if( strCurExt != ".ncd" &&
				strCurExt != ".nce" &&
				strExt.size()>0)
			{
				strLocaleFile += strExt;
			}
		}

		dlg.Destroy();

		//if selected file name already exists on the disk
		if(0 == access(strLocaleFile.c_str(), 0))
		{
			char szBuffer[2000];
		#ifdef HAVE_SNPRINTF
			snprintf(szBuffer, sizeof(szBuffer), _("File %s already exists! Do you want to overwrite it?"), strLocaleFile.c_str());
		#else
			sprintf(szBuffer, _("File %s already exists! Do you want to overwrite it?"), strLocaleFile.c_str());
		#endif

			//ask permission to overwrite
			gint result = gtkMessageBox(szBuffer, GTK_BUTTONS_YES_NO);
			if(GTK_RESPONSE_NO == result)
				return;
		}

		//if saving to .hnc format, warn that the links will not be saved
		if(0 < g_doc.GetTotalLinkCount())
		{
			if( NULL != filtername &&
				NULL != strstr(filtername, "*.hnc"))
			{
				gtkMessageBox(_("Warning: .hnc format does not support hyperlinks!"));
			}
		}

		//if switching from unencrpypted to encrypted format
		//offer to delete original file (unencrpyted)
		bool bEncSwitchedOn = false;
		std::string strOrigFile;
		if( g_doc.GetPath().size() > 0 &&
			!g_doc.IsEncrypted())
		{
			strOrigFile = g_doc.GetPath();
			bEncSwitchedOn = true;
		}

		//save the file to disk
		if(save_file(strFilePath.c_str()))
		{
			//report success
			if(NULL != user_data)
				*((bool *)user_data) = true;	//store result

			if( bEncSwitchedOn &&
				g_doc.IsEncrypted())
			{
				//ask to delete original file
				gint result = gtkMessageBox(_("You have saved file in encrypted format! Do you want to delete the original (unencrypted) file?\nThis will also delete any backup of this file."), GTK_BUTTONS_YES_NO);
				if(GTK_RESPONSE_YES == result)
				{
					remove(strOrigFile.c_str());

					//delete all backup files "original.ncd.bak1", ...
					std::string strDir = GetParentDir(strOrigFile.c_str());

					std::string strBakFilesPfx;
					strBakFilesPfx  = strDir;
					strBakFilesPfx += GetBaseName(strOrigFile.c_str());
					strBakFilesPfx += ".bak";

					std::vector<std::string> lstFiles;
					EnumDirectory(strDir.c_str(), OnListBackupFilesEnum, (void *)&lstFiles, ENUM_LST_FILES);

					int nBakCnt = lstFiles.size();
					for(int i=0; i<nBakCnt; i++){
						if(0 == strncmp(lstFiles[i].c_str(), strBakFilesPfx.c_str(), strBakFilesPfx.size())){
							remove(lstFiles[i].c_str());	// remove backup file
						}
					}

					g_objMRU.Change(strOrigFile.c_str(), false);
				}

				//DATA SECURITY: automatically delete all unencrypted autosave files here
				remove_autosave_unencrypted();
			}

			RefreshMainTitle();
			g_objMRU.Change(strLocaleFile.c_str(), true);
		}

		//store last save directory
		g_objIni.SetValue("Cache", "LastSaveDir", strDefaultDir.c_str());
		g_objIni.Save();

		g_objCrashLog.AddMsg("ACTION: save document\n");
	}

	restart_enc_unload_timer();
}

bool OnListBackupFilesEnum(const char *szFile, void *data)
{
	if(data)
	{
		//calculate extension
		std::string strExt(szFile);
		int nPos = strExt.find_last_of('.');
		if(nPos > 0)
			strExt = strExt.substr(nPos);

		if(0 == strncmp(strExt.c_str(), ".bak", strlen(".bak"))){
			std::vector<std::string> *lstFiles = (std::vector<std::string> *)data;
			lstFiles->push_back(szFile);
		}
	}

	return true;	//keep enumerating
}

//on main window destruction
gboolean on_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	//on window minimized, show tray icon
	if(g_bCloseToTray && !g_bQuitByMenu)
	{
		g_tray.Show();
		g_tray.SetTooltip("NoteCase"); //important: call this after Show
		gtk_widget_hide(window1);
		g_bQuitByMenu = false;
		return TRUE;	//forbid destruction
	}
	g_bQuitByMenu = false;

	UpdateTextFromScreen();

	// prevents message box to show multiple times on Alt+F4
	static bool bAlreadyAsked = false;

	if(!bAlreadyAsked)
	{
		if(g_doc.IsModified())
		{
			if(g_bAutoSaveOnClose && g_doc.GetPath().size() > 0)
				on_save1_activate(NULL, NULL);
			else
			{
				//ask to save the document
				gint result = gtkMessageBoxYNC(_("Document has been modified! Do you want to save it?"));
				if(GTK_RESPONSE_CANCEL == result)
					return TRUE;	//forbid destruction
				if(GTK_RESPONSE_YES == result)
				{
					bool bSaved = false;
					on_save1_activate(NULL, &bSaved);

					if(!bSaved){
						result = gtkMessageBox(_("Document saving failed! Abort closing?"), GTK_BUTTONS_YES_NO);
						if(GTK_RESPONSE_YES == result)
						{
							bAlreadyAsked = false;
							return TRUE;	//forbid GTK to destroy window
						}
					}

				}
			}
		}
		bAlreadyAsked = true;

		//calculate position variables
		gint nPosLeft, nPosTop, nPosWidth, nPosHeight, nPosDivider;
		gtk_window_get_position(GTK_WINDOW(window1), &nPosLeft, &nPosTop);
		gtk_window_get_size(GTK_WINDOW(window1), &nPosWidth, &nPosHeight);

		nPosDivider = -1;
		if( GTK_WIDGET_VISIBLE(g_tree.m_pWidget) &&
			GTK_WIDGET_VISIBLE(g_text.m_pWidget))
		{
			GtkWidget *divider = lookup_widget(window1, "hbox1");
			nPosDivider = gtk_paned_get_position(GTK_PANED(divider));
		}

		//save some options
		if(nPosDivider < 0)
		{
			//recalculate new divider by using old percentage
			int nOldWidth, nOldDivider;
			g_objIni.GetValue("Startup", "LastPos_Width", nOldWidth, 0);
			g_objIni.GetValue("Startup", "LastPos_Divider", nOldDivider, 0);

			if(nOldWidth > 0 && nOldDivider > 0){
				nPosDivider = (int)((double)nOldDivider / nOldWidth) * nPosWidth;
			}
			else
				nPosDivider = nPosWidth / 4;
		}

		//tree side changed - invert nPosDivider value
		if(g_bTreeToTheRight != g_bTreeToTheRightInitial)
			nPosDivider = nPosWidth - nPosDivider;

		g_objIni.SetValue("Display", "ViewMode", get_view_mode());
		g_objIni.SetValue("Display", "NodeTitleBar", get_node_title_set());
		g_objIni.SetValue("Display", "ShowToolBar", get_show_toolbar());
		g_objIni.SetValue("Display", "ShowStatusBar", get_show_status_bar());
		g_objIni.SetValue("Display", "MinimizeToTray", g_bMinimizeToTray);
		g_objIni.SetValue("Display", "CloseToTray", g_bCloseToTray);
		g_objIni.SetValue("Default", "CloseDlgOnTxtEnter", g_bCloseDlgOnTextEnter);
		g_objIni.SetValue("Startup", "LastPos_Left", nPosLeft);
		g_objIni.SetValue("Startup", "LastPos_Top", nPosTop);
		g_objIni.SetValue("Startup", "LastPos_Width", nPosWidth);
		g_objIni.SetValue("Startup", "LastPos_Height", nPosHeight);
		g_objIni.SetValue("Startup", "LastPos_Divider", nPosDivider);
		g_objIni.SetValue("Save",    "AutoSaveOnClose", g_bAutoSaveOnClose);
		g_objIni.SetValue("Save",    "SkipMemOnlyAutoSave", g_bSkipMemOnlyAutosave);
		g_objIni.SetValue("Export", "ExportEmbeddedAsSeparateFile", g_bExportDataAsSeparateFile);
		g_objIni.SetValue("Search", "CaseSensitive", g_objSearchInfo.m_bFindSensitive);

		g_objMRU.Save();	//saves into global ini object
		g_objIni.Save();	//saves into the file

		autosave_shutdown();
	}

	return FALSE;	//allows GTK to destroy window
}

void on_quit1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	g_bQuitByMenu = true;
	if(on_window_delete_event(NULL, NULL, NULL))
		return;	// do not quit yet

	gtk_main_quit(); //quit application
	g_objCrashLog.AddMsg("ACTION: quit application\n");

	restart_enc_unload_timer();
}

void on_cut1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	g_objCrashLog.AddMsg("ACTION: cut\n");

	//if focus correct, apply command
	if(g_text.m_pWidget == gtk_window_get_focus(GTK_WINDOW(window1)))
		g_text.ClipboardCut();
	else if(g_tree.m_pWidget == gtk_window_get_focus(GTK_WINDOW(window1)))
		on_menu_cut_branch(NULL, NULL);
	else
	{
		//handle case when in the middle of tree label editing
		GtkWidget *pWidget = gtk_window_get_focus(GTK_WINDOW(window1));
		g_signal_emit_by_name(pWidget, "cut-clipboard", pWidget, 0);
	}

	restart_enc_unload_timer();
}

void on_copy1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	g_objCrashLog.AddMsg("ACTION: copy\n");

	//if focus correct, apply command
	if(g_text.m_pWidget == gtk_window_get_focus(GTK_WINDOW(window1)))
		g_text.ClipboardCopy();
	else if(g_tree.m_pWidget == gtk_window_get_focus(GTK_WINDOW(window1)))
		on_menu_copy_branch(NULL, NULL);
	else
	{
		//handle case when in the middle of tree label editing
		GtkWidget *pWidget = gtk_window_get_focus(GTK_WINDOW(window1));
		g_signal_emit_by_name(pWidget, "copy-clipboard", pWidget, 0);
	}

	restart_enc_unload_timer();
}

void on_paste1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	g_objCrashLog.AddMsg("ACTION: paste\n");

	//if focus correct, apply command
	if(g_text.m_pWidget == gtk_window_get_focus(GTK_WINDOW(window1))){
		g_bIgnorePaste = false;
		g_text.ClipboardPaste();
	}
	else if(g_tree.m_pWidget == gtk_window_get_focus(GTK_WINDOW(window1)))
		on_menu_paste_branch(NULL, NULL);
	else
	{
		//handle case when in the middle of tree label editing
		GtkWidget *pWidget = gtk_window_get_focus(GTK_WINDOW(window1));
		g_signal_emit_by_name(pWidget, "paste-clipboard", pWidget, 0);
	}

	restart_enc_unload_timer();
}

void on_delete1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	g_objCrashLog.AddMsg("ACTION: delete\n");

	//if focus correct, apply command
	if(g_text.m_pWidget == gtk_window_get_focus(GTK_WINDOW(window1)))
		g_text.DeleteSelection();
	else if(g_tree.m_pWidget == gtk_window_get_focus(GTK_WINDOW(window1)))
		on_menu_delete_node (NULL, NULL);

	restart_enc_unload_timer();
}

void on_about1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	g_objCrashLog.AddMsg("ACTION: show \"About\" box\n");

	AboutDlg dlg;
	dlg.ShowModal();

	restart_enc_unload_timer();
}

void on_help1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	g_objCrashLog.AddMsg("ACTION: show help\n");

	std::string strHelpFile = GetHelpFile();

	//skip if help already loaded and not modified
	if( g_doc.GetPath().size() > 0 &&
		0 == strcmp(strHelpFile.c_str(), g_doc.GetPath().c_str()) &&
		!g_doc.IsModified() )
	{
		return;
	}

	// if not restricted by "AllowSingleInstance" option, open help in the new Notecase instance
	bool bSingleInst;
	g_objIni.GetValue("Startup", "AllowSingleInstance", bSingleInst);
	if(!bSingleInst){
		//start new Notecase instance
		std::string strURL = GetAppPath();
		std::string strDir = GetParentDir(strURL.c_str());

		std::string strURLLocale;
		Utf8ToLocale(strURL.c_str(), strURLLocale);

	#ifdef _WIN32
		ShellExecute(NULL, "open", strURLLocale.c_str(), strHelpFile.c_str(), "", SW_SHOW);
	#else
		Execute(strURL, strHelpFile, strDir);
	#endif
		return;
	}

	//if some document loaded (and not help document) or document was changed
	if( (g_doc.GetPath().size() > 0 && 0 != strcmp(strHelpFile.c_str(), g_doc.GetPath().c_str()) ) ||
		 g_doc.IsModified())
	{
		//user must confirm current document closing (to load help)
		gint result = gtkMessageBox(_("Current document has to be closed in order to open help document!\nDo you wish to proceed?\n\n(Note that if you choose to proceed you'll be prompted to save current document changes)"), GTK_BUTTONS_YES_NO);
		if(GTK_RESPONSE_NO == result)
			return;
	}

	int nResult = load_file(strHelpFile.c_str());

	//do not delete MRU for file that exists, but failed to open
	g_objMRU.Change(strHelpFile.c_str(), (DOC_LOAD_NOT_FOUND != nResult));

	restart_enc_unload_timer();
}

int load_file_embedded(int nCurNodeIdx)
{
	g_objCrashLog.AddMsg("Load file embedded\n");

	ShowBusyCursor();

	g_doc.m_nActiveNodeIdx = -1;

#ifdef _DEBUG
	time_t nStartTime = time(NULL);
#endif

	//trying to load new content
	int nResult = g_doc.Load(GetAppPath().c_str(), NULL, FORMAT_EXECUTABLE);

	if(!g_wnd.m_pWidget)	//no need if doing export from cmd line
		return nResult;

	if(DOC_LOAD_OK == nResult)
	{
		//clear old content
		g_tree.Clear();
		g_text.Clear();

		//rebuild treeview
		//recursively add nodes into the tree
		add_child_nodes(NULL);

		//select initial node (as written in document)
		if(nCurNodeIdx >= 0)
			g_doc.m_nCurrentNode = nCurNodeIdx;
		DocumentIterator it(g_doc);
		int nIdx = it.RecursiveIdx2NodeIdx(g_doc.m_nCurrentNode);
		SelectNodeByIdx(nIdx);

		HideBusyCursor();
	}
	else
	{
		//clear old content (g_doc is in empty state)
		g_tree.Clear();
		g_text.Clear();

		HideBusyCursor();

		//error message
		gtkMessageBox(GetLoadErrorString(nResult), GTK_BUTTONS_OK, GTK_MESSAGE_ERROR);
	}

	//"change password" - set proper menu item state
	GtkWidget *change_pass1 = lookup_widget(window1, "change_pass1");
	gtk_widget_set_sensitive (change_pass1, FALSE);

	//"lock document" - set proper menu item state
	GtkWidget *lock1 = lookup_widget(window1, "lock1");
	gtk_widget_set_sensitive (lock1, FALSE);

#ifdef _DEBUG
	time_t nEndTime = time(NULL);
	char szBuffer[1000];
	sprintf(szBuffer, "File loaded and displayed in %d seconds!", (int)(nEndTime-nStartTime));
	gtkMessageBox(szBuffer);
#endif

	//focus the tree widget
	gtk_window_set_focus(GTK_WINDOW(window1), g_tree.m_pWidget);

	g_undoManager.Clear();
	UpdateUndoRedoMenus();

	RefreshMainTitle();
	return nResult;
}

int load_file(const char *filename, const char *szPassword, int nCurNodeIdx)
{
	g_objCrashLog.AddMsg("Load file\n");

	//TOFIX preserve old document content if loading fails
	//(requires additional doc object, and = operator)

	//check if the current document has been modified
	if(g_doc.IsModified())
	{
		//ask to save the document
		gint result = gtkMessageBoxYNC(_("Document has been modified! Do you want to save it?"));
		if(GTK_RESPONSE_CANCEL == result)
			return DOC_LOAD_ABORTED;

		if(GTK_RESPONSE_YES == result)
		{
			//save old document
			bool bSaved = false;
			on_save1_activate(NULL, &bSaved);

			if(!bSaved)	return DOC_LOAD_ABORTED;	//quit action if user cancels old document saving
		}
	}

	if(g_wnd.m_pWidget)	//no need if doing export from cmd line
		ShowBusyCursor();

	g_doc.m_nActiveNodeIdx = -1;

#ifdef _DEBUG
	time_t nStartTime = time(NULL);
#endif

	//trying to load new content
	int nResult = g_doc.Load(filename, szPassword);

	if(!g_wnd.m_pWidget)	//no need if doing export from cmd line
		return nResult;

	if(DOC_LOAD_OK == nResult)
	{
		//clear old content
		g_tree.Clear();
		g_text.Clear();

		//rebuild treeview
		//recursively add nodes into the tree
		add_child_nodes(NULL);

		//select initial node (as written in document)
		if(nCurNodeIdx >= 0)
			g_doc.m_nCurrentNode = nCurNodeIdx;
		DocumentIterator it(g_doc);
		int nIdx = it.RecursiveIdx2NodeIdx(g_doc.m_nCurrentNode);
		if(nIdx >= 0){
			bool bExpanded = g_doc.GetNodeByIdx(g_doc.m_nCurrentNode).m_bExpanded;
			SelectNodeByIdx(nIdx);

			//FIX: node selection always expands the node (prevent this)
			if(!bExpanded){
				GtkTreePath *path2 = NULL;
				if(PathFromNodeIdx(g_doc.m_nCurrentNode, path2)){
					gtk_tree_view_collapse_row(GTK_TREE_VIEW(g_tree.m_pWidget), path2);
					gtk_tree_path_free(path2);
				}
			}
		}

		//"change password" - set proper menu item state
		GtkWidget *change_pass1 = lookup_widget(window1, "change_pass1");
		if(g_doc.IsOpen() && g_doc.IsEncrypted() && !g_doc.IsReadOnly())
			gtk_widget_set_sensitive (change_pass1, TRUE);
		else
			gtk_widget_set_sensitive (change_pass1, FALSE);

		//"lock document" - set proper menu item state
		GtkWidget *lock1 = lookup_widget(window1, "lock1");
		if(g_doc.IsOpen() && g_doc.IsEncrypted())
			gtk_widget_set_sensitive (lock1, TRUE);
		else
			gtk_widget_set_sensitive (lock1, FALSE);

		HideBusyCursor();
	}
	else
	{
		//clear old content (g_doc is in empty state)
		g_tree.Clear();
		g_text.Clear();

		HideBusyCursor();

		//error message
		std::string strErr = GetLoadErrorString(nResult);
		if( DOC_LOAD_NOT_FOUND == nResult ||
			DOC_LOAD_PERMISSIONS == nResult)
		{
			strErr += "\n[";
			strErr += filename;
			strErr += "]";
		}
		gtkMessageBox(strErr.c_str(), GTK_BUTTONS_OK, GTK_MESSAGE_ERROR);
	}

	restart_enc_unload_timer();

#ifdef _DEBUG
	time_t nEndTime = time(NULL);
	char szBuffer[1000];
	sprintf(szBuffer, "File loaded and displayed in %d seconds!",(int)(nEndTime-nStartTime));
	gtkMessageBox(szBuffer);
#endif

	//focus the tree widget
	gtk_window_set_focus(GTK_WINDOW(window1), g_tree.m_pWidget);

	g_undoManager.Clear();
	UpdateUndoRedoMenus();

	g_doc.SetModified(false);
	RefreshMainTitle();

	return nResult;
}

//recursively builds document tree
void add_child_nodes(GtkTreeIter *iter, int nParentID, int nFirstSibling)
{
	GtkWidget *treeview = lookup_widget(window1, "treeview1");
	GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);

	GtkTreeIter  child;
	DocumentIterator itDoc(g_doc);

	int nSiblingIdx = nFirstSibling;
	int nIdx = itDoc.GetChildIdx(nParentID, nSiblingIdx); //first child
	while (nIdx >= 0)
	{
		if(nParentID >= 0 && iter != 0)
			gtk_tree_store_append(GTK_TREE_STORE(model), &child, iter);  /* Acquire a child iterator */
		else
			gtk_tree_store_append(GTK_TREE_STORE(model), &child, NULL);   /* Acquire a child iterator */

		//set data
		NoteNode &node = g_doc.GetNodeByIdx(nIdx);
		std::string strData = node.GetTitle();
		gtk_tree_store_set (GTK_TREE_STORE(model), &child, STORE_IDX_TEXT, strData.c_str(), -1);
		gtk_tree_store_set (GTK_TREE_STORE(model), &child, STORE_IDX_STRIKE, node.m_bFinished, -1);
		gtk_tree_store_set (GTK_TREE_STORE(model), &child, STORE_IDX_UNDERLINE, node.m_objLink.IsValid()? PANGO_UNDERLINE_SINGLE : PANGO_UNDERLINE_NONE , -1);

		if(node.m_objLink.IsValid())
			gtk_tree_store_set (GTK_TREE_STORE(model), &child, STORE_IDX_COLOR, &g_linkColor, -1);

		UpdateNodeIcon(nIdx);

		//run main loop to repaint correctly for big documents
		int nLoops=0;
		while(gtk_main_iteration_do(false) && nLoops<20) nLoops++;

		//recursively add child's children
		add_child_nodes(&child, node.m_nID);

		//expand if necessary
		if(node.m_bExpanded){
			GtkTreePath *path2 = NULL;
			if(PathFromNodeIdx(nIdx, path2)){
				gtk_tree_view_expand_row(GTK_TREE_VIEW(g_tree.m_pWidget), path2, false);
				gtk_tree_path_free(path2);
			}
		}

		//go to the next sibling node
		nSiblingIdx ++;
		nIdx = itDoc.GetChildIdx(nParentID, nSiblingIdx);
	}
}

void on_tree_row_click (GtkTreeView *treeview, gpointer user_data)
{
	int nIdx = GetSelectedNodeIdx();
	if(nIdx < 0)
		return;

	//skip action if this node is already selected (preserves scroll position)
	if(g_doc.m_nActiveNodeIdx == nIdx)
		return;

	g_objCrashLog.Printf("Row selection change to Idx=%d\n", nIdx);

	// update the text of previously selected node
	UpdateTextFromScreen();

	//store selection (to be restored when we get back to this node)
	RememberSelectionBounds(g_doc.m_nActiveNodeIdx);

	//delete previous content
	g_text.Clear();

	//
	//load the text for the selected node
	//
	g_doc.m_nActiveNodeIdx = nIdx;

	//set new node text
	NoteNode &myNode = g_doc.GetNodeByIdx(nIdx);
	g_text.SetText(myNode.GetText().c_str());

	#ifdef HAVE_GTKSOURCEVIEW			
		// refresh text source language
		g_text.SetSourceLanguage(myNode.GetSourceLanguage().c_str());
	#endif

	RefreshTextFormat(myNode);

	//update node title label
	set_title_bar(myNode.GetTitle().c_str());

	//restore cursor position and selection bounds
	g_text.RestoreSelectionBounds(myNode.GetSelStart(), myNode.GetSelEnd());

	//restore scroll position
	g_text.RestoreScrollPos(myNode.GetScrollPos());

	//update node info (status bar)
	std::string strInfo = calc_node_info(nIdx);
	set_status_bar(strInfo.c_str(), false);

	//refresh "node attachments" toggle button state
	bool bAttExist = (g_doc.GetNodeByIdx(nIdx).m_lstAttachments.size() > 0);
	g_signal_handlers_block_by_func(g_wnd.m_pToolbarAttachment, (void *)on_node_attachments_activate, 0);
	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(g_wnd.m_pToolbarAttachment), bAttExist);
	g_signal_handlers_unblock_by_func(g_wnd.m_pToolbarAttachment, (void *)on_node_attachments_activate, 0);

	g_text.SetModified(false);
	if(!g_doc.IsReadOnly())
		g_text.SetEditable(true);
	RefreshMainTitle();

	restart_enc_unload_timer();
}

void RefreshTextFormat(NoteNode &node, bool bAddPictures, int nFrom, int nTo)
{
	GtkTextView *textview =  (GtkTextView *)g_text.m_pWidget;
	GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);

	//remove formatting
	if(nFrom < 0){
		//entire text view
		GtkTextIter iterStart, iterEnd;
		gtk_text_buffer_get_bounds(buffer1, &iterStart, &iterEnd);
		gtk_text_buffer_remove_all_tags (buffer1, &iterStart, &iterEnd);
	}
	else
	{
		//only given range
		GtkTextIter iterStart, iterEnd;
		gtk_text_buffer_get_iter_at_offset(buffer1, &iterStart, nFrom);
		gtk_text_buffer_get_iter_at_offset(buffer1, &iterEnd, nTo);
		gtk_text_buffer_remove_all_tags (buffer1, &iterStart, &iterEnd);
	}


	//underline all the links
	int i;
	int nLinks = node.m_lstLinks.size();
	for(i=0; i<nLinks; i++)
		if(nFrom<0 || LinkInfoList::RangesOverlap(node.m_lstLinks[i].m_nStartOffset, node.m_lstLinks[i].m_nStartOffset+ node.m_lstLinks[i].m_nTextLength, nFrom, nTo))
			g_text.SetTextUnderlined(node.m_lstLinks[i].m_nStartOffset, node.m_lstLinks[i].m_nStartOffset+ node.m_lstLinks[i].m_nTextLength);

	//re-insert all the images
	g_signal_handlers_block_by_func(buffer1, (void *)on_insert_text, 0);
	g_signal_handlers_block_by_func(buffer1, (void *)on_textview_edited, 0);

	if(bAddPictures)
	{
		int nPixCount = node.m_lstPictures.size();
		for(i=0; i<nPixCount; i++)
		{
			if(nFrom<0 || (node.m_lstPictures[i].nOffset >= nFrom && node.m_lstPictures[i].nOffset < nTo)){
				GtkTextIter cursIter;
				gtk_text_buffer_get_iter_at_offset(buffer1, &cursIter, node.m_lstPictures[i].nOffset);
				gtk_text_buffer_insert_pixbuf(buffer1, &cursIter, node.m_lstPictures[i].pixbuf);
			}
		}
	}

	g_signal_handlers_unblock_by_func(buffer1, (void *)on_insert_text, 0);
	g_signal_handlers_unblock_by_func(buffer1, (void *)on_textview_edited, 0);

	//re-apply formatting to the text
	int nFmtCount = node.m_lstTxtFmt.size();
	for(i=0; i<nFmtCount; i++)
	{
		int nEndTag = FmtInfoList::CalcFormatEndTag(node.m_lstTxtFmt[i].nFormatTag);
		if(nEndTag >= 0)
		{
			int nEndIdx = -1;
			for(int j=i+1; j<nFmtCount; j++){
				if(node.m_lstTxtFmt[j].nFormatTag == nEndTag){
					nEndIdx = j; break;
				}
			}

			if(nEndIdx > 0)
			{
				int nOffsetFrom = node.m_lstTxtFmt[i].nOffset;
				int nOffsetTo   = node.m_lstTxtFmt[nEndIdx].nOffset;

				if(nFrom<0 || LinkInfoList::RangesOverlap(nOffsetFrom, nOffsetTo, nFrom, nTo))
				{
					//get the iterators
					GtkTextIter cursIter, boundIter;
					gtk_text_buffer_get_iter_at_offset(buffer1, &cursIter, nOffsetFrom);
					gtk_text_buffer_get_iter_at_offset(buffer1, &boundIter, nOffsetTo);

					if(FMT_BOLD_BEGIN == node.m_lstTxtFmt[i].nFormatTag)
					{
						//"boldtag"
						GtkTextTag *tag = gtk_text_buffer_create_tag (buffer1, NULL,
								"weight", PANGO_WEIGHT_BOLD,
								NULL);
						gtk_text_buffer_apply_tag(buffer1, tag, &cursIter, &boundIter);
					}
					else if(FMT_ITALIC_BEGIN == node.m_lstTxtFmt[i].nFormatTag)
					{
						//"italictag"
						GtkTextTag *tag = gtk_text_buffer_create_tag (buffer1, NULL,
								"style", PANGO_STYLE_ITALIC,
								NULL);
						gtk_text_buffer_apply_tag(buffer1, tag, &cursIter, &boundIter);
					}
					else if(FMT_UNDERLINE_BEGIN == node.m_lstTxtFmt[i].nFormatTag)
					{
						//"underlinetag"
						GtkTextTag *tag = gtk_text_buffer_create_tag (buffer1, NULL,
								"underline", PANGO_UNDERLINE_SINGLE,
								NULL);
						gtk_text_buffer_apply_tag(buffer1, tag, &cursIter, &boundIter);
					}
					else if(FMT_TXT_COLOR_BEGIN == node.m_lstTxtFmt[i].nFormatTag)
					{
						char szBuffer[200];
						sprintf(szBuffer, "#%04x%04x%04x",
							node.m_lstTxtFmt[i].color.red,
							node.m_lstTxtFmt[i].color.green,
							node.m_lstTxtFmt[i].color.blue);

						//"colortag"
						GtkTextTag *tag = gtk_text_buffer_create_tag (buffer1, NULL,
								"foreground", szBuffer,
								NULL);
						gtk_text_buffer_apply_tag(buffer1, tag, &cursIter, &boundIter);
					}
					else if (FMT_STRIKE_BEGIN == node.m_lstTxtFmt[i].nFormatTag)
					{
						//"strikethroughtag"
						GtkTextTag *tag = gtk_text_buffer_create_tag (buffer1, NULL,
								"strikethrough", TRUE,
								NULL);
						gtk_text_buffer_apply_tag(buffer1, tag, &cursIter, &boundIter);
					}
					else if(FMT_TXT_BKG_COLOR_BEGIN == node.m_lstTxtFmt[i].nFormatTag)
					{
						char szBuffer[200];
						sprintf(szBuffer, "#%04x%04x%04x",
							node.m_lstTxtFmt[i].color.red,
							node.m_lstTxtFmt[i].color.green,
							node.m_lstTxtFmt[i].color.blue);

						//"colortag"
						GtkTextTag *tag = gtk_text_buffer_create_tag (buffer1, NULL,
								"background", szBuffer,
								NULL);
						gtk_text_buffer_apply_tag(buffer1, tag, &cursIter, &boundIter);
					}
				}
			}
		}
	}
}

bool save_file(const char *filename, bool bRememberPath, bool bAutosave, const char *szPassword)
{
	g_objCrashLog.AddMsg("Save file\n");

	//FIX: this should reduce corruption when autosave and normal save overlap
	if(g_bIoOperationInProgress){
		TRACE("IO operation already in progress\n");
		if(!bAutosave)
			gtkMessageBox(_("Save operation already in progress!"));
		return false;
	}
	g_bIoOperationInProgress = true;


	//remember focus
	bool bTextFocused = gtk_widget_is_focus(g_text.m_pWidget) > 0;

	if(!bAutosave)
		ShowBusyCursor();

	UpdateTextFromScreen();

	bool bSuccess = true;

	//refresh selected node depth-first index before save
	DocumentIterator it(g_doc);
	g_doc.m_nCurrentNode = it.NodeIdx2RecursiveIdx(g_doc.m_nActiveNodeIdx);

#ifdef _DEBUG
	time_t nStartTime = time(NULL);
#endif

	//read ini settings
	int nBackupFiles = 0;
	bool bBackup = false;
	g_objIni.GetValue("Other", "UseBackup", bBackup);
	if(bBackup && !bAutosave)
		g_objIni.GetValue("Other", "BackupFiles", nBackupFiles, 0);

	// save to file
	int nRes = g_doc.Save(filename, bRememberPath, nBackupFiles, szPassword);
	if(DOC_SAVE_OK != nRes)
	{
		//error message
		std::string strMsg = _("Failed to save the file!");
		strMsg += "\n";
		strMsg += GetSaveErrorString(nRes);

		gtkMessageBox(strMsg.c_str(), GTK_BUTTONS_OK, GTK_MESSAGE_ERROR);
		bSuccess = false;
	}

	//"change password" - set proper menu item state
	GtkWidget *change_pass1 = lookup_widget(window1, "change_pass1");
	if(g_doc.IsOpen() && g_doc.IsEncrypted() && !g_doc.IsReadOnly())
		gtk_widget_set_sensitive (change_pass1, TRUE);
	else
		gtk_widget_set_sensitive (change_pass1, FALSE);

	//"lock document" - set proper menu item state
	GtkWidget *lock1 = lookup_widget(window1, "lock1");
	if(g_doc.IsOpen() && g_doc.IsEncrypted())
		gtk_widget_set_sensitive (lock1, TRUE);
	else
		gtk_widget_set_sensitive (lock1, FALSE);

	HideBusyCursor();

	//restore focus
	if(!bAutosave){
		if(bTextFocused)
			gtk_widget_grab_focus(g_text.m_pWidget);
		else
			gtk_widget_grab_focus(g_tree.m_pWidget);
	}

#ifdef _DEBUG
	time_t nEndTime = time(NULL);
	char szBuffer[1000];
	sprintf(szBuffer, "File saved in %d seconds!", (int)(nEndTime-nStartTime));
	gtkMessageBox(szBuffer);
#endif

	g_bIoOperationInProgress = false;

	return bSuccess;
}

void UpdateTextFromScreen()
{
	//
	// update the text of previously selected node
	//
	if(g_text.GetModified())
	{
		if(g_doc.m_nActiveNodeIdx >= 0)
		{
			//refresh text
			std::string strText = g_text.GetText();
			g_doc.GetNodeByIdx(g_doc.m_nActiveNodeIdx).SetText(strText.c_str());

			g_doc.SetModified(true);
			g_text.SetModified(false);
		}
	}
}

//insert as child of the selected node
void on_menu_insert_child_node (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	g_objCrashLog.AddMsg("ACTION: Insert child node\n");

	GtkTreeView *treeview = (GtkTreeView *)lookup_widget(window1, "treeview1");
	GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);

	//get iterator to selected node
	GtkTreeSelection* treesel = gtk_tree_view_get_selection (treeview);
	GtkTreeIter  iter, iternew;

	int nIdx = -1;
	if(gtk_tree_selection_get_selected(treesel, &model, &iter))
	{
		//insert node as child
		gtk_tree_store_append(GTK_TREE_STORE(model), &iternew, &iter);  /* Acquire a child iterator */

		//TOFIX method to map from GUI tree iter to document node ID and back
		GtkTreePath *path1;
		path1 = gtk_tree_model_get_path(model, &iter);
		nIdx = NodeIdxFromPath(path1);

		g_objCrashLog.Printf("ACTION: Insert child node -> parent IDX=%d\n", nIdx);

		gtk_tree_view_expand_row(treeview, path1, FALSE);	//expand new parent
		gtk_tree_path_free(path1);
	}
	else
	{
		//insert node under root
		gtk_tree_store_append(GTK_TREE_STORE(model), &iternew, NULL);   /* Acquire a child iterator */
	}

	//if parent is "finished" the new child must have the same state too
	bool bFinished = false;
	if(nIdx >= 0)
		bFinished = g_doc.GetNodeByIdx(nIdx).m_bFinished;

	std::string strName = _("New node");
	gtk_tree_store_set (GTK_TREE_STORE(model), &iternew, STORE_IDX_TEXT, strName.c_str(), -1);
	gtk_tree_store_set (GTK_TREE_STORE(model), &iternew, STORE_IDX_STRIKE, (bFinished)? TRUE : FALSE, -1);
	gtk_tree_store_set (GTK_TREE_STORE(model), &iternew, STORE_IDX_UNDERLINE, PANGO_UNDERLINE_NONE, -1);

	//now add the node into the document
	if(nIdx >= 0)
		g_doc.NodeInsert(g_doc.GetNodeByIdx(nIdx).m_nID, -1);
	else
		g_doc.NodeInsert(-1, -1);

#ifdef _DEBUG
	g_doc.AssertValid();
#endif

	//set default node name to the latest note
	int nNewIdx = g_doc.GetNodeCount()-1;

	NoteNode &newNode = g_doc.GetNodeByIdx(nNewIdx);
	newNode.SetTitle(strName.c_str());
	newNode.m_bFinished = bFinished;

	newNode.m_nIconType = g_nDefaultNodeIconType;
	newNode.m_strIconFile = g_strDefaultIconFile;
	UpdateNodeIcon(nNewIdx);

	//select node in a tree
	SelectNodeByIdx(nNewIdx);

	RefreshMainTitle();

	//push document change into undo/redo manager
	DocAction *pAction = new DocAction;
	pAction->SetType(ACT_NODE_INSERT);
	pAction->SetDoc(g_doc);
	pAction->m_nNodeIndex   = nNewIdx; //TOFIX recursive index
	pAction->m_nNodeID      = newNode.m_nID;
	pAction->m_nNodePID     = newNode.m_nParentID;
	pAction->m_nNodeSibling = newNode.m_nSiblingIdx;
	pAction->m_objSubTree.AssignSubtree(g_doc, nNewIdx, true);

	g_undoManager.AddAction(pAction);
	UpdateUndoRedoMenus();

	edit_node_title();

	restart_enc_unload_timer();
}

void on_menu_insert_node (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	g_objCrashLog.AddMsg("ACTION: Insert root node\n");

	GtkTreeView *treeview = (GtkTreeView *)lookup_widget(window1, "treeview1");
	GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);
	GtkTreeSelection* treesel = gtk_tree_view_get_selection (treeview);
	GtkTreeIter  iternew, iter;

	int nIdx = -1;	//parent node index
	int nPID = -1;	//parent id
	int nPrevSiblingIdx = -1;	//index of the sibling node
	int nSID = -1;	//sibling index for the new node
	if(gtk_tree_selection_get_selected(treesel, &model, &iter))
	{
		//TOFIX method to map from GUI tree iter to document node ID and back
		GtkTreePath *path1 = gtk_tree_model_get_path(model, &iter);
		nPrevSiblingIdx = NodeIdxFromPath(path1);
		gtk_tree_path_free(path1);

		nPID = g_doc.GetNodeByIdx(nPrevSiblingIdx).m_nParentID;
		nSID = g_doc.GetNodeByIdx(nPrevSiblingIdx).m_nSiblingIdx;
		nIdx = g_doc.GetIdxFromID(nPID);

		g_objCrashLog.Printf("ACTION: Insert root node after node IDX=%d\n", nPrevSiblingIdx);
		TRACE("Insert node: selected node detected! PID=%d, ParentIdx=%d, prev SibNodeIdx=%d\n", nPID, nIdx, nPrevSiblingIdx);

		//insert node as a next sibling of the selected node
		gtk_tree_store_insert_after(GTK_TREE_STORE(model), &iternew, NULL, &iter);
	}
	else
	{
		//insert node under root
		gtk_tree_store_append(GTK_TREE_STORE(model), &iternew, NULL);   /* Acquire a child iterator */
	}

	std::string strName = _("New node");
	gtk_tree_store_set (GTK_TREE_STORE(model), &iternew, STORE_IDX_TEXT, strName.c_str(), -1);
	gtk_tree_store_set (GTK_TREE_STORE(model), &iternew, STORE_IDX_STRIKE, FALSE, -1);
	gtk_tree_store_set (GTK_TREE_STORE(model), &iternew, STORE_IDX_UNDERLINE, PANGO_UNDERLINE_NONE, -1);

	//now add the node into the document
	if(nPrevSiblingIdx < 0)
		g_doc.NodeInsert(nPID, -1);
	else
		g_doc.NodeInsert(nPID, nSID+1);

#ifdef _DEBUG
	g_doc.AssertValid();
#endif

	DocumentIterator it(g_doc);

	//set default node name to the latest note
	int nNewIdx = g_doc.GetNodeCount()-1;
	g_objCrashLog.Printf("ACTION: Insert root node - new node IDX=%d\n", nNewIdx);

	TRACE("Insert node: new node index=%d\n", nNewIdx);

	NoteNode &newNode = g_doc.GetNodeByIdx(nNewIdx);
	newNode.SetTitle(strName.c_str());

	newNode.m_nIconType = g_nDefaultNodeIconType;
	newNode.m_strIconFile = g_strDefaultIconFile;
	UpdateNodeIcon(nNewIdx);

	//select node in a tree
	SelectNodeByIdx(nNewIdx);

	RefreshMainTitle();

	//push document change into undo/redo manager
	DocAction *pAction = new DocAction;
	pAction->SetType(ACT_NODE_INSERT);
	pAction->SetDoc(g_doc);
	pAction->m_nNodeIndex   = nNewIdx; //TOFIX recursive index
	pAction->m_nNodeID      = newNode.m_nID;
	pAction->m_nNodePID     = newNode.m_nParentID;
	pAction->m_nNodeSibling = newNode.m_nSiblingIdx;
	pAction->m_objSubTree.AssignSubtree(g_doc, nNewIdx, true);

	g_undoManager.AddAction(pAction);
	UpdateUndoRedoMenus();

	edit_node_title();

	restart_enc_unload_timer();
}

void on_menu_delete_node (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	g_objCrashLog.AddMsg("ACTION: Delete node\n");

	GtkTreeView *treeview = (GtkTreeView *)lookup_widget(window1, "treeview1");
	GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);

	//get iterator to selected node
	GtkTreeSelection* treesel = gtk_tree_view_get_selection (treeview);
	GtkTreeIter  iter;

	if(gtk_tree_selection_get_selected(treesel, &model, &iter))
	{
		//recursively delete node and all its children

		//TOFIX method to map from GUI tree iter to document node ID and back
		GtkTreePath *path1 = gtk_tree_model_get_path(model, &iter);
		int nIdx = NodeIdxFromPath(path1);

		//prepare path to show to the next selection candidate
		int nNextSelectedID = -1;
		if(g_tree.GetPreviousVisibleNode(&path1)){
			int nNextIdx = NodeIdxFromPath(path1);
			if(nNextIdx)
				nNextSelectedID = g_doc.GetNodeByIdx(nNextIdx).m_nID;
		}

		gtk_tree_path_free(path1);

		NoteNode &node = g_doc.GetNodeByIdx(nIdx);

		//do not ask confirmation if empty node
		UpdateTextFromScreen();
		bool bSkipMsg = false;
		DocumentIterator it(g_doc);
		if( node.IsEmpty() &&
			0 == it.GetChildCount(node.m_nID))
		{
			bSkipMsg = true;
		}

		if(!user_data && !bSkipMsg)
		{
			//prepare question
			std::string strMsg = _("Are you sure to delete the node\n\"%s\"?");

			//if node name is too long, use elipsis on the name
			std::string strTitle = node.GetTitle();
			if(strTitle.size() > 30){
				strTitle = strTitle.substr(0, 30);
				strTitle += "...";
			}

			char szBuffer[2024];
		#ifdef HAVE_SNPRINTF
			snprintf(szBuffer, sizeof(szBuffer), strMsg.c_str(), strTitle.c_str());
		#else
			sprintf(szBuffer, strMsg.c_str(), strTitle.c_str());
		#endif
			strMsg = szBuffer;

			//ask for confirmation
			gint result = gtkMessageBox(strMsg.c_str(), GTK_BUTTONS_YES_NO);
			if(GTK_RESPONSE_YES != result)
				return;
		}

		UpdateTextFromScreen();	//proper node contents neded for undo

		//push document change into undo/redo manager
		DocAction *pAction = new DocAction;
		pAction->SetType(ACT_TREE_DELETE);
		pAction->SetDoc(g_doc);
		pAction->m_objSubTree.AssignSubtree(g_doc, nIdx, true);
		pAction->m_nNodeIndex   = nIdx;
		pAction->m_nNodeID      = node.m_nID;
		pAction->m_nNodePID     = node.m_nParentID;
		pAction->m_nNodeSibling = node.m_nSiblingIdx;

		//remove node info (recursive)
		if(nIdx >= 0)
			g_doc.NodeDelete(node.m_nID);

		g_objCrashLog.Printf("ACTION: Delete node - deleted IDX=%d\n", nIdx);

		g_doc.m_nActiveNodeIdx = -1;

		//remove GUI tree node
		gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);

		//clear edit belonging to selected node
		g_text.Clear();

		//push document change into undo/redo manager
		g_undoManager.AddAction(pAction);
		UpdateUndoRedoMenus();

		//update node title label
		set_title_bar("");

		RefreshMainTitle();

		//select another node after selected node deleted
		if(nNextSelectedID >= 0){
			int nNewIdx = g_doc.GetIdxFromID(nNextSelectedID);
			SelectNodeByIdx(nNewIdx);
		}
		else {
			//no previous node, select the first visible node
			GtkTreePath *path = g_tree.GetFirstVisibleNode();
			if(path){
				int nNewIdx = NodeIdxFromPath(path);
				SelectNodeByIdx(nNewIdx);
				gtk_tree_path_free(path);
			}
			else
				g_text.SetEditable(false);	// no selected node
		}
	}

	restart_enc_unload_timer();
}

void on_menu_rename_node (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	g_objCrashLog.AddMsg("ACTION: Rename node\n");

	edit_node_title();
	RefreshMainTitle();
}

void on_menu_move_up (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	do_node_move_up();
}

void on_menu_move_down (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	do_node_move_down();
}

void on_menu_move_left (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	do_node_move_left();
}

void on_menu_move_right (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	do_node_move_right();
}

gint treeview_keyboard_handler(GtkWidget *widget, GdkEventKey *event)
{
	//TOFIX restart_enc_unload_timer();
	GtkWidget *pWidget = gtk_window_get_focus(GTK_WINDOW(window1));

	if(g_defTreeNodeNew.Match(event)){
		on_menu_insert_node(NULL, 0);
	}
	else if(g_defTreeNodeNewChild.Match(event)){
		on_menu_insert_child_node(NULL, 0);
	}
	else if(g_defTreeNodeDelete.Match(event)){
		on_menu_delete_node(NULL, 0);
	}
	else if(g_defTreeNodeRename.Match(event)){
		on_menu_rename_node(NULL, 0);
	}
	else if(g_defTreeNodeMoveDown.Match(event)){
		on_menu_move_down(NULL, 0); //move item one place down
	}
	else if(g_defTreeNodeMoveUp.Match(event)){
		on_menu_move_up(NULL, 0); //move item one place down
	}
	else if(g_defTreeNodeMoveLeft.Match(event)){
		on_menu_move_left(NULL, 0); //move item one place down
	}
	else if(g_defTreeNodeMoveRight.Match(event)){
		on_menu_move_right(NULL, 0); //move item one place down
	}
	else if(g_defTreeNodeToggleFinished.Match(event))
	{
		on_node_toggle_finished(NULL, NULL);
		return FALSE;
	}
	else if(g_defTreeNodeProperties.Match(event)){
		if(g_tree.m_pWidget == pWidget)
			on_menu_node_properties(NULL, 0);
	}
	else if(g_defTreeNodeDelFinished.Match(event)){
		//TOFIX
	}
	else if(g_defTreeNodeSortChildrenAsc.Match(event)){
		//TOFIX
	}
	else if(g_defTreeNodeSortChildrenDsc.Match(event)){
		//TOFIX
	}
	else if(g_defTreeNodeSortRootAsc.Match(event)){
		//TOFIX
	}
	else if(g_defTreeNodeSortRootDsc.Match(event)){
		//TOFIX
	}
	else if(g_defTreeNodeExpandAll.Match(event)){
		//TOFIX
	}
	else if(g_defTreeNodeColapseAll.Match(event)){
		//TOFIX
	}
	else if(g_defTreeCopyBranchStruct.Match(event)){
		//TOFIX
	}
	else if(g_defTreePasteBranchRoot.Match(event)){
		//TOFIX
	}

	//fixed keys
	if( event->keyval == GDK_Down ){
		if(0 == (event->state & (GDK_SHIFT_MASK|GDK_CONTROL_MASK|GDK_MOD1_MASK)))
			g_tree.SelectionDown(); //select next item below current
	}
	else if( event->keyval == GDK_Up ){
		if(0 == (event->state & (GDK_SHIFT_MASK|GDK_CONTROL_MASK|GDK_MOD1_MASK)))
			g_tree.SelectionUp(); //select previous item above current
	}
	else if( event->keyval == GDK_Left ){
		if(0 == (event->state & (GDK_SHIFT_MASK|GDK_CONTROL_MASK|GDK_MOD1_MASK)))
			g_tree.SelectionLevelUp(); //collapse selected node OR select parent node
	}
	else if( event->keyval == GDK_Right ){
		if(0 == (event->state & (GDK_SHIFT_MASK|GDK_CONTROL_MASK|GDK_MOD1_MASK)))
			g_tree.SelectionLevelDown(); //expand selected node OR select the first child (if already expanded)
	}
	else if( event->keyval == GDK_Home ){
		g_tree.SelectionHome();	//select first tree item
	}
	else if( event->keyval == GDK_End ){
		g_tree.SelectionEnd(); //select last item (only expanded counts)
	}
	else if( event->keyval == GDK_Page_Up ){
		g_tree.SelectionPageUp(); //moves one page up (no nodes are being expanded/collapsed)
	}
	else if( event->keyval == GDK_Page_Down ){
		g_tree.SelectionPageDown();	//moves one page up (no nodes are being expanded/collapsed)
	}
	else if(  event->keyval == GDK_F5 ||
			 (event->keyval == 'p' && (event->state & GDK_CONTROL_MASK)) )
 	{
		g_text.SetFocus();
		return FALSE;
	}
	else if( event->keyval == GDK_Tab )
 	{
		g_text.SetFocus();
		return TRUE;
	}
	else if( event->keyval == GDK_Return )
 	{
		if(  (event->state & GDK_CONTROL_MASK) &&
			!(event->state & GDK_SHIFT_MASK))
		{
			if(g_tree.m_pWidget != pWidget)
			{
				if(g_doc.IsReadOnly()){
					gtkMessageBox(_("Can not change read-only document!"));
					return FALSE;
				}
				//support for multiline node titles
				//insert new line into the edited entry
				gint nCursor, nSelection;
				g_object_get(G_OBJECT(pWidget), "cursor-position", &nCursor, "selection-bound", &nSelection, (char *)NULL);

				std::string strData = "\n";
				std::string strTxt = gtk_entry_get_text(GTK_ENTRY(pWidget));
				if(nCursor == nSelection)
				{
					strTxt.insert(nCursor, strData);

					nCursor = nCursor + strData.size();
					nSelection = nCursor - strData.size();
				}
				else
				{
					int nLeft = min(nCursor, nSelection);
					int nSize = abs(nCursor - nSelection);

					//replace selection with new line character
					strTxt.erase(nLeft, nSize);
					strTxt.insert(nLeft, strData);

					nCursor = nLeft + strData.size();
					nSelection = nCursor - strData.size();
				}
				gtk_entry_set_text(GTK_ENTRY(pWidget), strTxt.c_str());
				gtk_entry_select_region(GTK_ENTRY(pWidget), nCursor, nCursor);
			}
			return FALSE;
		}
	}
	return TRUE;
}

//
// remember actual selection and cursor position in textView and
// set corresponding values in nodeIdx's noteNode
//
void RememberSelectionBounds( int nodeIdx )
{
	if( nodeIdx >= 0 ) {
		int cursor, selection;
		g_text.GetSelectionBounds( cursor, selection );
		NoteNode &myNode = g_doc.GetNodeByIdx(nodeIdx);
		myNode.SetSelStart( cursor );
		myNode.SetSelEnd( selection );

		int nPos = g_text.GetScrollPos();
		myNode.SetScrollPos	(nPos);
	}
}

//TOFIX move to DocTreeNavigation or similar class ?
void SelectNodeByIdx(int nIdx, gfloat fRowAlign, bool bForceAlign)	//bool bExpand
{
	if(nIdx < 0)
		return;	//invalid index

	GtkTreeView *treeview = (GtkTreeView *)lookup_widget(window1, "treeview1");
	GtkTreePath *path2 = NULL;
	if(PathFromNodeIdx(nIdx, path2))
	{
		GtkTreeViewColumn *column1 = gtk_tree_view_get_column(treeview, 0);
		gtk_tree_view_expand_to_path(treeview, path2);
		gtk_tree_view_set_cursor(treeview, path2, column1, FALSE);	//refresh node selection

		//generally do not force scrolling if the cell is already within the visible range
		bool bVisible = g_tree.IsPathInVisbleRange(path2);
		if(fRowAlign >= 0.0 && bForceAlign)
			gtk_tree_view_scroll_to_cell(treeview, path2, NULL, TRUE, fRowAlign, 0.0);
		else if(!bVisible){
			if(fRowAlign < 0.0)
				fRowAlign = 0.1;
			gtk_tree_view_scroll_to_cell(treeview, path2, NULL, TRUE, fRowAlign, 0.0);
		}

		gtk_tree_path_free(path2);

		if(!g_doc.IsReadOnly())
			g_text.SetEditable(true);
	}
}

void on_menu_import (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	g_objCrashLog.AddMsg("ACTION: Import\n");

	FileDialog dlg(true);
	dlg.SetTitle(_("Import from file"));

	//define filters
	dlg.AddFilter(_("All supported formats"), "*.ncd|*.nce|*.hnc|*.gjots2|*.xml|*.MM|*.txt");
	dlg.AddFilter(_("NoteCase document (*.ncd)"), "*.ncd");
	dlg.AddFilter(_("NoteCase encrypted document (*.nce)"), "*.nce");
	dlg.AddFilter(_("NoteCenter document (*.hnc)"), "*.hnc");
	dlg.AddFilter(_("Gjots2 document (*.gjots2)"), "*.gjots2");
	dlg.AddFilter(_("Sticky Notes document (*.xml)"), "*.xml");
	dlg.AddFilter(_("MM/LX Mindmap/Outline document (*.MM)"), "*.MM");
	dlg.AddFilter(_("All files (*)"), "*");

	//set initial directory from INI (store last used)
	std::string strDefaultDir;
	std::string strDir;
	g_objIni.GetValue("Cache", "LastOpenDir", strDir, "");
	if(strDir.size() > 0 && 0 == access(strDir.c_str(), 00))
		strDefaultDir = strDir;
	else
		strDefaultDir = GetHomeDir();

	dlg.SetDirectory(strDefaultDir.c_str());

	if(dlg.DoModal())
	{
		const gchar *filename = dlg.GetFilename();
		strDefaultDir = dlg.GetDirectory();
		dlg.Close();

		NoteDocument doc;
		doc.SetPassCallback(PasswordCallback);
		doc.SetFmtCallback(FormatCallback);

		//convert from UTF8 to locale encoding
		std::string strLocaleFile;
		Utf8ToLocale(filename, strLocaleFile);

		int nRes = doc.Load(strLocaleFile.c_str());	//TOFIX handle errors
		if(DOC_LOAD_OK == nRes)
		{
			//IMPORTANT: get the facts before merge
			DocumentIterator it(g_doc);
			int nCnt   = it.GetChildCount(-1);
			int nTotal = g_doc.GetNodeCount();

			//TOFIX ask user for import details and merge with current document
		#ifdef _DEBUG
			g_doc.Dump();
			doc.Dump();
		#endif
			g_doc.Merge(doc);

			//refresh tree starting from new content
			add_child_nodes(NULL, -1, nCnt);

			//push document change into undo/redo manager
			DocAction *pAction = new DocAction;
			pAction->SetType(ACT_TREE_IMPORT);
			pAction->SetDoc(g_doc);
			pAction->m_objSubTree = doc;	//copy content
			pAction->m_nNodeIndex = nTotal;	//first node to be removed

			g_undoManager.AddAction(pAction);
			UpdateUndoRedoMenus();
		}
		else{
			//display error
			gtkMessageBox(GetLoadErrorString(nRes), GTK_BUTTONS_OK, GTK_MESSAGE_ERROR);
		}

		//store last open directory
		g_objIni.SetValue("Cache", "LastOpenDir", strDefaultDir.c_str());
	}

	restart_enc_unload_timer();
}

void on_menu_export (GtkMenuItem *menuitem, gpointer user_data)
{
	g_objCrashLog.AddMsg("ACTION: Export\n");

	FileExportDlg dlgExp;
	if(GTK_RESPONSE_OK != dlgExp.ShowModal())
		return;

	//fetch export details
	int nExportMode  = dlgExp.GetExportMode();	//0-(cur) branch, 1-curent node only, 2-all document
	std::string strExportFile = dlgExp.GetExportFile();
	std::string strPostExportScript = dlgExp.GetPostExportScript();
	std::string strCSS;
	bool bUseCSS = dlgExp.GetHtmlExportCSS(strCSS);
	if(!bUseCSS)
		strCSS = "";
	int nFormat = dlgExp.GetFormatCode();

	bool bExportLinked = dlgExp.GetExportLinked();

	dlgExp.Destroy();

	export_document(g_doc, nExportMode, strExportFile, strPostExportScript, strCSS, -1 /* read from ini*/, true, bExportLinked, nFormat);
	g_lstRecursiveExportStack.clear();

	restart_enc_unload_timer();
}

void export_document(const NoteDocument &document, int nExportMode, const std::string &strExportFile, const std::string &strPostProcessScript, const std::string &strCSS, int nEmbedCSS, bool bCheckOverwrite, bool bExportLinked, int nFormat, int nExportBranchIdx)
{
	g_objCrashLog.AddMsg("Export document\n");

	//convert from UTF8 to locale encoding
	std::string strLocaleFile;
	Utf8ToLocale(strExportFile.c_str(), strLocaleFile);
	TRACE("Export to file:%s, mode:%d\n", strLocaleFile.c_str(), nExportMode);

	if(bCheckOverwrite)
	{
		//if selected file name already exists on the disk
		if(0 == access(strLocaleFile.c_str(), 0))
		{
			char szBuffer[2000];
		#ifdef HAVE_SNPRINTF
			snprintf(szBuffer, sizeof(szBuffer), _("File %s already exists! Do you want to overwrite it?"), strLocaleFile.c_str());
		#else
			sprintf(szBuffer, _("File %s already exists! Do you want to overwrite it?"), strLocaleFile.c_str());
		#endif

			//ask permission to overwrite
			gint result = gtkMessageBox(szBuffer, GTK_BUTTONS_YES_NO);
			if(GTK_RESPONSE_NO == result)
				return;
		}
	}

	if(g_wnd.m_pWidget)
		UpdateTextFromScreen();	//no need if doing export from cmd line

	//prepare data to be exported as another document object
	NoteDocument doc;
	doc.SetPassCallback(PasswordCallback);
	DocumentIterator it(doc);

	switch(nExportMode){
		case EXPORT_BRANCH:	// selected branch only
			{
				doc = g_doc;
				doc.m_strCustomCSS = strCSS;
				doc.m_nEmbedCSS = nEmbedCSS;

				int nIdx = -1;
				if(nExportBranchIdx >= 0)
					nIdx = it.RecursiveIdx2NodeIdx(nExportBranchIdx);
				else
					nIdx = GetSelectedNodeIdx();
				if(nIdx >= 0)
				{
					//TOFIX convert to use AssignSubtree
					//remove all notes from document not being children of this node
					//STEP 1: move this node to be the root node
					while(doc.MoveNodeLeft(nIdx))
					{
					}

					//STEP 2: delete all other root nodes
					DocumentIterator it(doc);
					int nID = doc.GetNodeByIdx(nIdx).m_nID;
					int nSibling  = 0;
					int nChildIdx = it.GetChildIdx(-1, nSibling);
					while(nChildIdx >= 0)
					{
						int nChildID = doc.GetNodeByIdx(nChildIdx).m_nID;
						if(nChildID == nID)
							nSibling ++;
						else{
							doc.NodeDelete(nChildID);
							nSibling = 0;
						}

						nChildIdx = it.GetChildIdx(-1, nSibling);	//keep looping
					}

				#ifdef _DEBUG
					doc.AssertValid();
				#endif
				}
				else{
					gtkMessageBox(_("No selected node!"));
					return;
				}
				break;
			}

		case EXPORT_NODE: // selected node only
			{
				int nIdx = GetSelectedNodeIdx();
				if(nIdx >= 0)
				{
					//write note text into the file
					DocumentIterator it(g_doc);

					doc.m_strCustomCSS = strCSS;
					doc.m_nEmbedCSS = nEmbedCSS;
					doc.NodeInsert(-1, 0);
					doc.GetNodeByIdx(0).CopyState(g_doc.GetNodeByIdx(nIdx));
				}
				else{
					gtkMessageBox(_("No selected node!"));
					return;
				}
				break;
			}


		case EXPORT_DOCUMENT: // entire document
			{
				doc = document;
				doc.m_strCustomCSS = strCSS;
				doc.m_nEmbedCSS = nEmbedCSS;
				break;
			}
	}

	//
	// handle recursive export
	//
	if(bExportLinked)
	{
		//push main doc
		if( 0 == g_lstRecursiveExportStack.size() &&
			!doc.GetPath().empty())
		{
			export_entry entry;
			entry.strDoc = doc.GetPath();
			g_lstRecursiveExportStack.push_back(entry);
		}

		//find current entry on the stack
		int nCurEntry = -1;
		int nEntries  = g_lstRecursiveExportStack.size();
		for(int k=0; k<nEntries; k++){
			if(g_lstRecursiveExportStack[k].strDoc == doc.GetPath()){
				nCurEntry = k;
				break;
			}
		}
		if(nCurEntry < 0)
			return; // error

		if(g_lstRecursiveExportStack[nCurEntry].bExported)
			return;	// recursion control

		//mark current entry as exported
		g_lstRecursiveExportStack[nCurEntry].bExported = true;

		//parse doc object for all available links (push links on a global stack)
		if(!g_lstRecursiveExportStack[nCurEntry].bParsed)
		{
			int nNodes = doc.GetNodeCount();
			for(int i=0; i<nNodes; i++)
			{
				LinkInfoList &links = doc.GetNodeByIdx(i).m_lstLinks;	// hyperlinks within this node

				int nLinks = links.size();
				for(int j=0; j<nLinks; j++)
				{
					std::string strLink = links[j].m_strTargetURL;

					//handle only .ncd links (external links)
					if(strLink.size()<4 || 0 != strcasecmp(strLink.substr(strLink.size()-4).c_str(), ".ncd"))
						continue;

					//does this link already exists in the stack
					bool bFound = false;
					int nEntries = g_lstRecursiveExportStack.size();
					for(int k=0; k<nEntries; k++)
					{
						if(g_lstRecursiveExportStack[k].strDoc == links[j].m_strTargetURL)
						{
							bFound = true;
							break;
						}
					}

					//new link found, push to stack
					if(!bFound)
					{
						export_entry entry;
						entry.strDoc = links[j].m_strTargetURL;
						g_lstRecursiveExportStack.push_back(entry);
					}
				}
			}

			//mark current entry as parsed
			g_lstRecursiveExportStack[nCurEntry].bParsed = true;
		}

		//
		// before this document is exported,
		// patch all .ncd links to point to the .html files within the same dir as main exported document
		//
		unsigned int nNodes = doc.GetNodeCount();
		unsigned int i;
		for(i=0; i<nNodes; i++)
		{
			LinkInfoList &links = doc.GetNodeByIdx(i).m_lstLinks;	// hyperlinks within this node

			int nLinks = links.size();
			for(int j=0; j<nLinks; j++)
			{
				std::string strLink = links[j].m_strTargetURL;

				//handle only .ncd links
				if( strLink.size()<4 ||
					0 != strcasecmp(strLink.substr(strLink.size()-4).c_str(), ".ncd"))
					continue;

				//change extension
				strLink = strLink.substr(0, strLink.size()-3);
				strLink += "html";

				//append prefix
				if(0 != strcasecmp(strLink.substr(0, strlen("file://")).c_str(), "file://")){
				#ifdef _WIN32
					strLink.insert(0, "/");
				#endif
					strLink.insert(0, "file://");
				}

				links[j].m_strTargetURL = strLink;
			}
		}

		//
		// recursively handle exporting of all linked docs
		//
		i=0;
		while(i < g_lstRecursiveExportStack.size())
		{
			if(!g_lstRecursiveExportStack[i].bExported)
			{
				std::string strLink = g_lstRecursiveExportStack[i].strDoc;

				// calculate export name (same dir as root doc, html format, same name as orig doc)
				std::string strExportFile2 = strExportFile;
				std::string::size_type nPos = strExportFile2.find_last_of("\\/");
				if(nPos != std::string::npos)
					strExportFile2 = strExportFile2.substr(0, nPos);

				std::string strName = strLink;
				nPos = strName.find_last_of("\\/");
				if(nPos != std::string::npos)
					strName = strName.substr(nPos);

				//change extension
				strName = strName.substr(0, strName.size()-3);
				strName += "html";

				strExportFile2 += strName;

				//load and export linked doc
				NoteDocument docLinked;
				if(DOC_LOAD_OK == docLinked.Load(strLink.c_str()))
					export_document(docLinked, EXPORT_DOCUMENT, strExportFile2, strPostProcessScript, strCSS, nEmbedCSS, bCheckOverwrite, true, nFormat);
				else
					TRACE("Failed to open document: %s\n", strLink.c_str());
			}
			i++;
		}

	}

	doc.Save(strLocaleFile.c_str(), true, 0, NULL, nFormat);	//TOFIX TRACE errors

	if(strPostProcessScript.size()>0){
		TRACE("Call post-processing script: %s\n", strPostProcessScript.c_str());
		std::string strCommand;
		strCommand  = strPostProcessScript;
		strCommand += " \"";
		strCommand += strLocaleFile;
		strCommand += "\"";
		gboolean bOK;
		bOK = g_spawn_command_line_async(strCommand.c_str(), NULL);
	}

	TRACE("Export done!\n");
}

//TOFIX use this wrapper more often
//TOFIX move to DocTreeNavigation or similar class ?
int GetSelectedNodeIdx()
{
	GtkTreeView *treeview = (GtkTreeView *)lookup_widget(window1, "treeview1");
	GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);

	//get iterator to selected node
	GtkTreeSelection* treesel = gtk_tree_view_get_selection (treeview);
	GtkTreeIter  iter;

	if(gtk_tree_selection_get_selected(treesel, &model, &iter))
	{
		//get document node index from GUI tree iterator
		GtkTreePath* path1 = gtk_tree_model_get_path(model, &iter);
		if(path1){
			int nIdx = NodeIdxFromPath(path1);
			gtk_tree_path_free(path1);
			return nIdx;
		}
	}

	return -1;
}

void on_options1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	g_objCrashLog.AddMsg("ACTION: Options\n");

	OptionsDialog dlg;
	dlg.ShowModal();

	restart_enc_unload_timer();
}

void on_find1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	g_objCrashLog.AddMsg("ACTION: Find\n");

	if(g_doc.GetNodeCount() < 1){
		gtkMessageBox(_("Document is empty!"));
		return;
	}

	//find dialog
	FindDialog dlg;

	GtkWidget *casesensitive_chk = lookup_widget(dlg.GetDialog(), "casesensitive_chk");
	GtkWidget *entry1 = lookup_widget(dlg.GetDialog(), "entry1");
	GtkWidget *cbo3	= lookup_widget(dlg.GetDialog(), "comboboxentry3");

	//initialize from current settings
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(casesensitive_chk), g_objSearchInfo.m_bFindSensitive);

	//default search string: selected text OR previous search string
	std::string selText =  g_text.GetSelectedText();
	if(!selText.empty())
		g_objSearchInfo.m_strFindText = selText;

	gtk_entry_set_text(GTK_ENTRY(entry1), g_objSearchInfo.m_strFindText.c_str());
	gtk_editable_select_region (GTK_EDITABLE (entry1), 0, g_objSearchInfo.m_strFindText.size());

	if(GTK_RESPONSE_OK == dlg.ShowModal())
	{
		UpdateTextFromScreen();

		if(gtk_combo_box_get_active(GTK_COMBO_BOX(cbo3)) > 0)
			g_objSearchInfo.m_bSearchKeywords = true;
		else
			g_objSearchInfo.m_bSearchKeywords = false;

		if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(casesensitive_chk)))
			g_objSearchInfo.m_bFindSensitive = true;
		else
			g_objSearchInfo.m_bFindSensitive = false;

		g_objSearchInfo.m_strFindText = gtk_entry_get_text (GTK_ENTRY(entry1));
		if(g_objSearchInfo.m_strFindText.empty())
		{
			gtk_widget_destroy (dlg.GetDialog());
			gtkMessageBox(_("Error: Search text is empty!"));
			return;
		}

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

		//start find
		text_find();
	}
	gtk_widget_destroy (dlg.GetDialog());

	restart_enc_unload_timer();
}

void on_find2_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	g_objCrashLog.AddMsg("ACTION: Find next\n");

	if(g_doc.GetNodeCount() < 1){
		gtkMessageBox(_("Document is empty!"));
		return;
	}

	if(g_objSearchInfo.m_strFindText.empty())
	{
		on_find1_activate (menuitem, user_data);	//show empty find dialog
		return;
	}

	if(g_objSearchInfo.m_strFindText.empty())
	{
		gtkMessageBox(_("Error: Search text is empty!"));
		return;
	}

	UpdateTextFromScreen();

	if(!g_objSearchInfo.m_bSearchKeywords)
	{
		//refresh current node in case user moved in the document
		//or done some editing after the find action was started
		int nIdx = GetSelectedNodeIdx();
		if(nIdx < 0 && g_doc.GetNodeCount() > 0)
			nIdx = 0;

		//start searching from current caret position?
		if(!g_objSearchInfo.m_bFindInTitle)
		{
			//search from last result selection (or start)
			int cursor = -1;
			int selection = -1;
			g_text.GetSelectionBounds(cursor, selection);
			if(selection >= 0)
				cursor = selection;

			g_objSearchInfo.m_nFindBufferPos = cursor;
			TRACE("Find start pos = %d\n", cursor);
		}

		DocumentIterator it(g_doc);
		g_objSearchInfo.m_nCurNodeRecursiveIdx = it.NodeIdx2RecursiveIdx(nIdx);
	}

	//find next
	if(!g_objSearchInfo.m_strFindText.empty())
		text_find();

	restart_enc_unload_timer();
}

bool text_find(bool bAutoProceedToStart, bool bNotifyDone)
{
	bool bFound = false;

	DocumentIterator it(g_doc);
	TextSearch search;

	//show wait dialog - abortable
	ProgressDlg dlg(200, _("Searching ..."), window1, true);

	//set search parameters
	search.SetSearchPattern(g_objSearchInfo.m_strFindText.c_str());
	if(!g_objSearchInfo.m_bFindSensitive)
		search.SetScanStyle(FS_CASE_INSENSITIVE);

	//search all nodes in a depth first order
	while(g_objSearchInfo.m_nCurNodeRecursiveIdx >= 0)
	{
		int nCurIdx = GetSelectedNodeIdx();

		//detect if the search has reached end
		if( g_objSearchInfo.m_bFirstNodeSearched &&
			g_objSearchInfo.m_nCurNodeRecursiveIdx == g_objSearchInfo.m_nStartNodeRecursiveIdx){
			if(bNotifyDone)
				gtkMessageBox(_("No more results found!"));
			//TOFIX clear some find state variables?
			return false;	//no more
		}

		if(ProgressDlg::IsCanceled())
		{
			TRACE("text_find: search canceled\n");
			gtkMessageBox(_("Search canceled!"));
			return false;	//no more
		}

		int nIdx = it.RecursiveIdx2NodeIdx(g_objSearchInfo.m_nCurNodeRecursiveIdx);
		if(nIdx < 0){
			if(bNotifyDone)
				gtkMessageBox(_("No more results found!"));
			return false;	//no more
		}

		int nPos = -1;
		const char *szBuf = NULL;

		if(g_objSearchInfo.m_bSearchKeywords)
		{
			//search entire keyword (enclose by delimiters)
			std::string strPattern = ";";
			strPattern += g_objSearchInfo.m_strFindText;
			strPattern += ";";

			std::string::size_type nRes = g_doc.GetNodeByIdx(nIdx).GetKeywords().find(strPattern.c_str());
			if(nRes != std::string::npos)
				nPos = nRes;
		}
		else
		{
			if(g_objSearchInfo.m_bFindInTitle)
			{
				//find in note title
				szBuf = g_doc.GetNodeByIdx(nIdx).GetTitle().c_str();
				search.SetScanBuffer(szBuf, g_utf8_strlen(szBuf, -1));
			}
			else
			{
				//find in node text
				szBuf = g_doc.GetNodeByIdx(nIdx).GetText().c_str();
				search.SetScanBuffer(szBuf, g_utf8_strlen(szBuf, -1));
			}

			nPos = search.Search(g_objSearchInfo.m_nFindBufferPos);
		}

		if(nPos >= 0){
			//result found!!!
			bFound = true;

			//if not active node, activate it!
			if(nIdx != nCurIdx){
				SelectNodeByIdx(nIdx);
				g_doc.GetNodeByIdx(nCurIdx).ClearSel();	// clear selection
				nCurIdx = nIdx;
			}

			//
			//result notification
			//
			if(g_objSearchInfo.m_bSearchKeywords)
			{
				//TOFIX how to mark the find!!!
				gtkMessageBox(_("This node contains the tag!"));
			}
			else
			{
				//NOTE: result is a byte offset, convert to utf-8 character offset
				long nOffset   = nPos; //g_utf8_pointer_to_offset(szBuf, szBuf+nPos);
				long nTxtChars = g_utf8_strlen(g_objSearchInfo.m_strFindText.c_str(), -1);

				// store last find position
				g_objSearchInfo.m_nLastResultIdx = nIdx;
				g_objSearchInfo.m_nLastResultPos = nOffset;

				if(g_objSearchInfo.m_bFindInTitle)
				{
					edit_node_title(nOffset, nTxtChars);
				}
				else
				{
					//select pattern in a text view
					g_text.EnsureVisible(nOffset);
					g_text.SelectRange(nOffset, nOffset + g_utf8_strlen(g_objSearchInfo.m_strFindText.c_str(), -1));

					//restore focus
					GtkWidget *textview = lookup_widget(window1, "textview1");
					gtk_window_set_focus(GTK_WINDOW(window1), textview);
				}

				g_objSearchInfo.m_nFindBufferPos = nPos + g_objSearchInfo.m_strFindText.size();
				break;	// result found and selected, exit
			}
		}

		//TOFIX support direction up
		if(g_objSearchInfo.m_bSearchKeywords || !g_objSearchInfo.m_bFindInTitle)
		{
			if(g_objSearchInfo.m_nCurNodeRecursiveIdx == g_objSearchInfo.m_nStartNodeRecursiveIdx)
				g_objSearchInfo.m_bFirstNodeSearched = true;
			g_objSearchInfo.m_nCurNodeRecursiveIdx ++;
		}

		//reached to the end of document, ask to search from top (unless was started from top)!
		if((unsigned int)g_objSearchInfo.m_nCurNodeRecursiveIdx >= g_doc.GetNodeCount())
		{
			if(g_objSearchInfo.m_nStartNodeRecursiveIdx > 0)
			{
				gint result = 0;
				if(!bAutoProceedToStart)
					result = gtkMessageBox(_("Reached end of the document! Continue searching from the start?"), GTK_BUTTONS_YES_NO);

				if(bAutoProceedToStart || GTK_RESPONSE_YES == result)
					g_objSearchInfo.m_nCurNodeRecursiveIdx = 0;
				else
					break;
			}
			else{
				if(bNotifyDone)
					gtkMessageBox(_("Search done!"));
				break;
			}
		}

		if(bFound && g_objSearchInfo.m_bSearchKeywords)
			break;	//result found, new node calculated

		g_objSearchInfo.m_nFindBufferPos = 0;
		g_objSearchInfo.m_bFindInTitle = !g_objSearchInfo.m_bFindInTitle; // move from title to text search, or reset back to title
	}

	return bFound;
}

void on_find_replace_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	g_objCrashLog.AddMsg("ACTION: Find/replace\n");

	//reset some variables
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

	//start dialog (it will handle the operation)
	FindReplaceDialog dlg;
	dlg.ShowModal();

	restart_enc_unload_timer();
}

void set_wrap_activated(bool bActivate)
{
	bool bIsActive = g_text.IsWrapped();

	if((bActivate && !bIsActive) || (!bActivate && bIsActive))
		refresh_wrap_menu(bActivate); //this will trigger the signal
}

void refresh_wrap_menu (bool bActivate)
{
	GtkWidget *wrap1 = lookup_widget(window1, "wrap1");
	if(bActivate)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(wrap1), TRUE);
	else
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(wrap1), FALSE);
}

void on_wrap_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	g_objCrashLog.AddMsg("ACTION: Set wrap\n");
	GtkWidget *textview = lookup_widget(window1, "textview1");

	//swap wrapping state of text view control
	if(!g_text.IsWrapped())
		gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD);
	else
		gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_NONE);

	restart_enc_unload_timer();
}

void on_show_node_titlebar_activate	(GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWidget *title1 = lookup_widget(window1, "title1");
	GtkWidget *label1 = lookup_widget(window1, "label1");

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(title1)))
		gtk_widget_show(label1);
	else
		gtk_widget_hide(label1);

	restart_enc_unload_timer();
}

bool get_node_title_set()
{
	GtkWidget *label1 = lookup_widget(window1, "label1");
	return GTK_WIDGET_VISIBLE(label1);
}

void refresh_nodetitle_menu(bool bActivate)
{
	GtkWidget *title1	= lookup_widget(window1, "title1");
	if(bActivate)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(title1), TRUE);
	else
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(title1), FALSE);
}

void set_show_node_title(bool bShow)
{
	bool bIsVisible = get_node_title_set();

	if((bShow && !bIsVisible) || (!bShow && bIsVisible))
		refresh_nodetitle_menu(bShow); //this will trigger the signal
}

void on_show_toolbar_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWidget *menutool1 = lookup_widget(window1, "menutool1");
	GtkWidget *toolbar1 = lookup_widget(window1, "toolbar1");

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menutool1))){
		#ifdef _NOKIA_MAEMO
		 gtk_widget_show_all(toolbar1);
		#else
		 gtk_widget_show(toolbar1);
		#endif
	}
	else{
	    #ifdef _NOKIA_MAEMO
		 gtk_widget_hide_all(toolbar1);
		#else
		 gtk_widget_hide(toolbar1);
		#endif
	}

	restart_enc_unload_timer();
}

void refresh_toolbar_menu(bool bActivate)
{
	GtkWidget *menutool1 = lookup_widget(window1, "menutool1");
	if(bActivate)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menutool1), TRUE);
	else
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menutool1), FALSE);
}

void set_show_toolbar(bool bShow)
{
	bool bIsVisible = get_show_toolbar();

	if((bShow && !bIsVisible) || (!bShow && bIsVisible))
		refresh_toolbar_menu(bShow); //this will trigger the signal
}

bool get_show_toolbar()
{
	GtkWidget *toolbar1 = lookup_widget(window1, "toolbar1");
	return GTK_WIDGET_VISIBLE(toolbar1);
}

void on_undo_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}

	int nIdx = GetSelectedNodeIdx();
	if(nIdx >= 0)
		UpdateTextFromScreen();

	g_objCrashLog.AddMsg("ACTION: Undo\n");
	g_undoManager.Undo();
	UpdateUndoRedoMenus();

	restart_enc_unload_timer();
}

void on_redo_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}

	int nIdx = GetSelectedNodeIdx();
	if(nIdx >= 0)
		UpdateTextFromScreen();

	g_objCrashLog.AddMsg("ACTION: Redo\n");
	g_undoManager.Redo();
	UpdateUndoRedoMenus();

	restart_enc_unload_timer();
}

void UpdateUndoRedoMenus()
{
	// update undo/redo menu/toolbar state
	GtkWidget *undo1 = lookup_widget(window1, "undo1");
	GtkWidget *redo1 = lookup_widget(window1, "redo1");

	GtkWidget *tbr_undo = g_wnd.m_pToolbarUndo;
	GtkWidget *tbr_redo = g_wnd.m_pToolbarRedo;

	if(g_undoManager.CanUndo())
	{
		gtk_widget_set_sensitive(undo1, TRUE);
		if(tbr_undo)
			gtk_widget_set_sensitive(tbr_undo, TRUE);
	}
	else
	{
		gtk_widget_set_sensitive(undo1, FALSE);
		if(tbr_undo)
			gtk_widget_set_sensitive(tbr_undo, FALSE);
	}

	if(g_undoManager.CanRedo())
	{
		gtk_widget_set_sensitive(redo1, TRUE);
		if(tbr_redo)
			gtk_widget_set_sensitive(tbr_redo, TRUE);
	}
	else
	{
		gtk_widget_set_sensitive(redo1, FALSE);
		if(tbr_redo)
			gtk_widget_set_sensitive(tbr_redo, FALSE);
	}
}

void SetNodeTitle(int nIdx, const char *szTitle)
{
	//TOFIX recursive index
	g_doc.GetNodeByIdx(nIdx).SetTitle(szTitle);

	//store new text into the tree store for given cell
	GtkTreePath *path1 = NULL;
	if(PathFromNodeIdx(nIdx, path1))
	{
		GtkWidget *treeview = lookup_widget(window1, "treeview1");
		GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);

		GtkTreeIter iter;
		gtk_tree_model_get_iter(model, &iter, path1);
		gtk_tree_store_set (GTK_TREE_STORE(model), &iter, STORE_IDX_TEXT, szTitle, -1);
		gtk_tree_path_free(path1);
	}

	//update node title label
	if(g_doc.m_nActiveNodeIdx == nIdx){
		set_title_bar(szTitle);
	}
}

void InsertNodeText(int nIdx, int nOffset, const char *szText, std::vector<PixInfo> *pLstPictures)
{
	//select node if needed
	if(nIdx != GetSelectedNodeIdx())
		SelectNodeByIdx(nIdx);

	g_text.InsertText(nOffset, szText);

	NoteNode &node = g_doc.GetNodeByIdx(nIdx);

	if(pLstPictures)
		node.m_lstPictures.insert(node.m_lstPictures.end(), (*pLstPictures).begin(), (*pLstPictures).end());

	//TOFIX call RefreshTextFormatting here to fix formatting too
	//re-insert all the images in this range
	GtkTextView *textview =  (GtkTextView *)g_text.m_pWidget;
	GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);
	g_signal_handlers_block_by_func(buffer1, (void *)on_insert_text, 0);
	g_signal_handlers_block_by_func(buffer1, (void *)on_textview_edited, 0);

	int nTxtLen = g_utf8_strlen(szText,-1);
	int nPixCount = node.m_lstPictures.size();
	for(int i=0; i<nPixCount; i++)
	{
		if(node.m_lstPictures[i].nOffset >= nOffset && 
			node.m_lstPictures[i].nOffset <= (nOffset + nTxtLen))
		{
			GtkTextIter cursIter;
			gtk_text_buffer_get_iter_at_offset(buffer1, &cursIter, node.m_lstPictures[i].nOffset);
			gtk_text_buffer_insert_pixbuf(buffer1, &cursIter, node.m_lstPictures[i].pixbuf);
		}
	}

	g_signal_handlers_unblock_by_func(buffer1, (void *)on_insert_text, 0);
	g_signal_handlers_unblock_by_func(buffer1, (void *)on_textview_edited, 0);

	//fix other formatting too
	//NOTE: false - do not add pictures again, because they'll be doubled
	RefreshTextFormat(node, false, nOffset, nOffset+nTxtLen+nPixCount);
}

void DeleteNodeText(int nIdx, int nOffset, int nLength)
{
	//select node if needed
	if(nIdx != GetSelectedNodeIdx())
		SelectNodeByIdx(nIdx);

	g_text.DeleteText(nOffset, nLength);
}

void on_insert_text (GtkTextBuffer *textbuffer, GtkTextIter *arg1, gchar *arg2, gint arg3, gpointer user_data)
{
	unsigned int nOffset = gtk_text_iter_get_offset(arg1);
	int nIdx = GetSelectedNodeIdx();
	ASSERT(nIdx >= 0);

	//multiple typing changes can possibly be merged into the single big change
	//unless the timeout between changes was elapsed
	time_t nCurActionTime = time(NULL);
	static time_t nLastActionTime = 0;
	bool bCanMerge  = ((nLastActionTime > 0) && (nCurActionTime-nLastActionTime)<UNDO_MERGE_TIMEOUT);
	TRACE("Undo: Can_merge=%d\n", bCanMerge);
	nLastActionTime = nCurActionTime;

	g_nPasteOffset = nOffset;
	g_nPasteLen	= g_utf8_strlen(arg2, -1);
	g_timeout_add (0, clear_formatting_timer, NULL);

	DocAction *pCurAction = dynamic_cast<DocAction *>(g_undoManager.GetCurrentAction());

	if(bCanMerge &&
	   pCurAction &&
	   pCurAction->GetType() == ACT_TEXT_INSERT &&
	   pCurAction->m_nNodeIndex == nIdx &&
	   nOffset == (pCurAction->m_nTextStartPos +  g_utf8_strlen(pCurAction->m_strNodeText.c_str(), -1)))
	{
		//join multiple typed characters into single DocAction change
		pCurAction->m_strNodeText += arg2;
		TRACE("Undo: multiple text-insert actions merged\n");
	}
	else{
		//push document change into undo/redo manager
		DocAction *pAction = new DocAction;
		pAction->SetType(ACT_TEXT_INSERT);
		pAction->SetDoc(g_doc);
		pAction->m_nNodeIndex = nIdx; //TOFIX recursive index
		pAction->m_nTextStartPos = nOffset;
		pAction->m_strNodeText = arg2;
		pAction->m_lstFmt = g_doc.GetNodeByIdx(nIdx).m_lstTxtFmt;
		pAction->m_lstLinks = g_doc.GetNodeByIdx(nIdx).m_lstLinks;

		g_undoManager.AddAction(pAction);
		UpdateUndoRedoMenus();
	}

	//update date_modified for the node
	g_doc.GetNodeByIdx(nIdx).m_nDateModified = nCurActionTime;

	//refresh offsets of the all links within this node
	int nCharsAdded = g_utf8_strlen(arg2, -1);
	g_doc.GetNodeByIdx(nIdx).OnTxtInsert(nOffset, nCharsAdded, arg2);
	//NOTE: formatting will be refresed by clear_formatting_timer

	//apply the format override
	if(nOffset == g_nTextOffset)
	{
		//TOFIX single group undo change
		if(g_nBoldFmtTag > -1){
			DocActionFmt *pAction = new DocActionFmt;
			pAction->SetDoc(g_doc);
			pAction->m_nNodeIdx		 = nIdx;
			pAction->m_nOperationTag = FMT_BOLD_BEGIN;
			pAction->m_nStartOffset  = g_nTextOffset;
			pAction->m_nEndOffset	 = g_nTextOffset + nCharsAdded;

			//execute action
			pAction->Exec(false);

			//push document change into undo/redo manager
			g_undoManager.AddAction(pAction);
		}
		if(g_nItalicFmtTag > -1){
			DocActionFmt *pAction = new DocActionFmt;
			pAction->SetDoc(g_doc);
			pAction->m_nNodeIdx		 = nIdx;
			pAction->m_nOperationTag = FMT_ITALIC_BEGIN;
			pAction->m_nStartOffset  = g_nTextOffset;
			pAction->m_nEndOffset	 = g_nTextOffset + nCharsAdded;

			//execute action
			pAction->Exec(false);

			//push document change into undo/redo manager
			g_undoManager.AddAction(pAction);
		}
		if(g_nUnderlineFmtTag > -1){
			DocActionFmt *pAction = new DocActionFmt;
			pAction->SetDoc(g_doc);
			pAction->m_nNodeIdx		 = nIdx;
			pAction->m_nOperationTag = FMT_UNDERLINE_BEGIN;
			pAction->m_nStartOffset  = g_nTextOffset;
			pAction->m_nEndOffset	 = g_nTextOffset + nCharsAdded;

			//execute action
			pAction->Exec(false);

			//push document change into undo/redo manager
			g_undoManager.AddAction(pAction);
		}
		if(g_nStrikeFmtTag > -1){
			DocActionFmt *pAction = new DocActionFmt;
			pAction->SetDoc(g_doc);
			pAction->m_nNodeIdx		 = nIdx;
			pAction->m_nOperationTag = FMT_STRIKE_BEGIN;
			pAction->m_nStartOffset  = g_nTextOffset;
			pAction->m_nEndOffset	 = g_nTextOffset + nCharsAdded;

			//execute action
			pAction->Exec(false);

			//push document change into undo/redo manager
			g_undoManager.AddAction(pAction);
		}

		reset_format_override();
		UpdateUndoRedoMenus();
	}

	restart_enc_unload_timer();

	//indenting support
	if(nCharsAdded > 0 && arg2[0] == '\n'){
		g_timeout_add (0, indent_new_line_timer, NULL);
	}
}

void on_delete_text (GtkTextBuffer *textbuffer, GtkTextIter *arg1, GtkTextIter *arg2, gpointer user_data)
{
	unsigned int nOffset   = gtk_text_iter_get_offset(arg1);
	unsigned int nOffsetTo = gtk_text_iter_get_offset(arg2);
	int nIdx = GetSelectedNodeIdx();

	//refresh text
	gchar *szText = gtk_text_buffer_get_text(textbuffer, arg1, arg2, FALSE);
	int nTxtLen   = g_utf8_strlen(szText, -1);

	//multiple typing changes can possibly be merged into the single big change
	//unless the timeout between changes was elapsed
	time_t nCurActionTime = time(NULL);
	static time_t nLastActionTime = 0;
	bool bCanMerge  = ((nLastActionTime > 0) && (nCurActionTime-nLastActionTime)<UNDO_MERGE_TIMEOUT);
	TRACE("Undo (delete): Can_merge=%d\n", bCanMerge);
	nLastActionTime = nCurActionTime;

	DocAction *pDelAction = NULL;
	DocAction *pCurAction = dynamic_cast<DocAction *>(g_undoManager.GetCurrentAction());

	if(bCanMerge &&
	   pCurAction &&
	   pCurAction->GetType() == ACT_TEXT_DELETE &&
	   pCurAction->m_nNodeIndex == nIdx &&
	   nOffset == (unsigned int)pCurAction->m_nTextStartPos)
	{
		pDelAction = pCurAction;

		//join multiple deleted characters into single DocAction change
		//deletion using Delete key
		pCurAction->m_strNodeText += szText;

		TRACE("Undo: multiple text-delete actions merged\n");
	}
	else if(pCurAction &&
	   pCurAction->GetType() == ACT_TEXT_DELETE &&
	   pCurAction->m_nNodeIndex == nIdx &&
	   nOffset == (unsigned int)(pCurAction->m_nTextStartPos - nTxtLen))
	{
		pDelAction = pCurAction;

		//join multiple deleted characters into single DocAction change
		//deletion using Backspace key
		pCurAction->m_strNodeText = szText + pCurAction->m_strNodeText;
		pCurAction->m_nTextStartPos -= nTxtLen;

		TRACE("Undo: multiple text-delete actions merged\n");
	}
	else{
		//push document change into undo/redo manager
		//new deletion action
		DocAction *pAction = new DocAction;
		pAction->SetType(ACT_TEXT_DELETE);
		pAction->SetDoc(g_doc);
		pAction->m_nNodeIndex = nIdx; //TOFIX recursive index
		pAction->m_nTextStartPos = nOffset;
		pAction->m_strNodeText = szText;
		pAction->m_lstFmt = g_doc.GetNodeByIdx(nIdx).m_lstTxtFmt;
		pAction->m_lstLinks = g_doc.GetNodeByIdx(nIdx).m_lstLinks;

		pDelAction = pAction;

		g_undoManager.AddAction(pAction);
		UpdateUndoRedoMenus();
	}

	g_doc.GetNodeByIdx(nIdx).m_nDateModified = nCurActionTime;

	//refresh offsets of the all links within this node
	int nCharsDeleted = g_utf8_strlen(szText, -1);
	int nTotalDeleted = nOffsetTo - nOffset;
	g_doc.GetNodeByIdx(nIdx).OnTxtDelete(nOffset, nCharsDeleted, nTotalDeleted, pDelAction);

	g_free (szText);

	restart_enc_unload_timer();
}

void TreeIterFromID(GtkTreeIter  &iter, int nNodeID)
{
	int nIdx = g_doc.GetIdxFromID(nNodeID);
	IteratorFromNodeIdx(nIdx, iter);
}

void do_node_move_up (bool bStoreUndo)
{
	GtkTreeView *treeview = (GtkTreeView *)lookup_widget(window1, "treeview1");
	GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);

	//get iterator to selected node
	GtkTreeSelection* treesel = gtk_tree_view_get_selection (treeview);
	GtkTreeIter iter;

	if(gtk_tree_selection_get_selected(treesel, &model, &iter))
	{
		//get document node index from GUI tree iterator
		GtkTreePath* path1 = gtk_tree_model_get_path(model, &iter);
		int nIdx = NodeIdxFromPath(path1);
		if(nIdx > -1)
		{
			g_objCrashLog.Printf("ACTION: Move node up (IDX=%d)\n", nIdx);

			NoteNode &node = g_doc.GetNodeByIdx(nIdx);
			int nParentID = node.m_nParentID;
			int nSIB = node.m_nSiblingIdx;

			//find previous sibling child if exists
			DocumentIterator itDoc(g_doc);
			int nIdx2 = itDoc.GetChildIdx(nParentID, nSIB-1);
			if(-1 == nIdx2)
			{
				//TOFIX is it allowed to change node level with this fn.
				//no previous sibling, move one level up
				if(nParentID >= 0)
				{
					do_node_move_left (false);
					SelectNodeByIdx(nIdx);
					do_node_move_up (false);
				}
			}
			else if(nIdx2 > -1)
			{
				//find previous sibling
				if(!gtk_tree_path_prev(path1)){
					gtk_tree_path_free(path1);
					return;
				}

				//iter from path
				GtkTreeIter itPrev;
				gtk_tree_model_get_iter(model, &itPrev, path1);

				//swap nodes (with all child nodes)
				//gtk_tree_store_swap(GTK_TREE_STORE(model), &iter, &itPrev); //BUG crashes!
				gtk_tree_store_move_before(GTK_TREE_STORE(model), &iter, &itPrev);

				int nSIB = node.m_nSiblingIdx;

				//swap sibling indexes
				node.m_nSiblingIdx  --;
				g_doc.GetNodeByIdx(nIdx2).m_nSiblingIdx ++;

			#ifdef _DEBUG
				g_doc.AssertValid();
			#endif

				g_doc.SetModified(true);
				RefreshMainTitle();
				//TOFIX refresh node selection and text, update active ID, ...

				if(bStoreUndo)
				{
					//push document change into undo/redo manager
					DocAction *pAction = new DocAction;
					pAction->SetType(ACT_TREE_MOVE);
					pAction->SetDoc(g_doc);
					pAction->m_nMoveDirection = MOVE_UP;
					pAction->m_nNodeIndex = nIdx; //TOFIX recursive index
					pAction->m_nNodeSibling = nSIB;

					g_undoManager.AddAction(pAction);
					UpdateUndoRedoMenus();
				}
			}
		}
		gtk_tree_path_free(path1);
	}

	restart_enc_unload_timer();
}

void do_node_move_down(bool bStoreUndo)
{
	GtkTreeView *treeview = (GtkTreeView *)lookup_widget(window1, "treeview1");
	GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);

	//get iterator to selected node
	GtkTreeSelection* treesel = gtk_tree_view_get_selection (treeview);
	GtkTreeIter iter;

	if(gtk_tree_selection_get_selected(treesel, &model, &iter))
	{
		//get document node index from GUI tree iterator
		GtkTreePath* path1 = gtk_tree_model_get_path(model, &iter);
		int nIdx = NodeIdxFromPath(path1);
		if(nIdx > -1)
		{
			g_objCrashLog.Printf("ACTION: Move node down (IDX=%d)\n", nIdx);

			NoteNode &node = g_doc.GetNodeByIdx(nIdx);
			int nParentID = node.m_nParentID;
			int nSIB = node.m_nSiblingIdx;

			//find next sibling child if exists
			DocumentIterator itDoc(g_doc);
			int nIdx2 = itDoc.GetChildIdx(nParentID, nSIB+1);
			if(nIdx2 > -1)
			{
				//find next sibling
				//iter from path
				GtkTreeIter *itNext = gtk_tree_iter_copy(&iter);
				if(!gtk_tree_model_iter_next(model, itNext))
				{
					gtk_tree_iter_free(itNext);
					gtk_tree_path_free(path1);
					return;
				}

				//swap nodes (with all child nodes)
				gtk_tree_store_move_after(GTK_TREE_STORE(model), &iter, itNext);
				gtk_tree_iter_free(itNext);

				int nSIB = node.m_nSiblingIdx;

				//swap sibling indexes
				node.m_nSiblingIdx  ++;
				g_doc.GetNodeByIdx(nIdx2).m_nSiblingIdx --;

			#ifdef _DEBUG
				g_doc.AssertValid();
			#endif

				g_doc.SetModified(true);
				RefreshMainTitle();

				if(bStoreUndo)
				{
					//push document change into undo/redo manager
					DocAction *pAction = new DocAction;
					pAction->SetType(ACT_TREE_MOVE);
					pAction->SetDoc(g_doc);
					pAction->m_nMoveDirection = MOVE_DOWN;
					pAction->m_nNodeIndex = nIdx; //TOFIX recursive index
					pAction->m_nNodeSibling = nSIB;

					g_undoManager.AddAction(pAction);
					UpdateUndoRedoMenus();
				}
			}
		}
		gtk_tree_path_free(path1);
	}

	restart_enc_unload_timer();
}

void do_node_move_left(bool bStoreUndo)
{
	GtkTreeView *treeview = (GtkTreeView *)lookup_widget(window1, "treeview1");
	GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);

	//get iterator to selected node
	GtkTreeSelection* treesel = gtk_tree_view_get_selection (treeview);
	GtkTreeIter iter;

	if(gtk_tree_selection_get_selected(treesel, &model, &iter))
	{
		//get document node index from GUI tree iterator
		GtkTreePath* path1 = gtk_tree_model_get_path(model, &iter);
		int nIdx = NodeIdxFromPath(path1);
		if(nIdx > -1)
		{
			g_objCrashLog.Printf("ACTION: Move node left (IDX=%d)\n", nIdx);

			int nParentID = g_doc.GetNodeByIdx(nIdx).m_nParentID;
			if(nParentID > -1)	//parent exists
			{
				int nIdx2 = g_doc.GetIdxFromID(nParentID);

				//get parent's parent path
				if(!gtk_tree_path_up(path1)){
					gtk_tree_path_free(path1);
					return;
				}
				if(!gtk_tree_path_up(path1)){
					gtk_tree_path_free(path1);
					return;
				}
				//iter from path
				GtkTreeIter *itParent = NULL;
				GtkTreeIter itUp;
				if( gtk_tree_path_get_depth(path1) > 0 &&
					gtk_tree_model_get_iter(model, &itUp, path1))
					itParent = &itUp;

				//delete current node
				gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);

				int nSIB = g_doc.GetNodeByIdx(nIdx).m_nSiblingIdx;

				int nNewParentID = g_doc.GetNodeByIdx(nIdx2).m_nParentID;
				g_doc.MoveNodeLeft(nIdx);

				//attach as new child of a new parent (including subnodes)
				while(gtk_tree_model_iter_children(model, &iter, itParent))
					gtk_tree_store_remove(GTK_TREE_STORE(model), &iter); //delete all children
				add_child_nodes(&itUp, nNewParentID);				     //reattach children with new chld

				g_doc.SetModified(true);
				RefreshMainTitle();

				gtk_tree_view_expand_row(treeview, path1, FALSE);	//expand new parent
				SelectNodeByIdx(nIdx);

				if(bStoreUndo)
				{
					//push document change into undo/redo manager
					DocAction *pAction = new DocAction;
					pAction->SetType(ACT_TREE_MOVE);
					pAction->SetDoc(g_doc);
					pAction->m_nMoveDirection = MOVE_LEFT;
					pAction->m_nNodeIndex = nIdx; //TOFIX recursive index
					pAction->m_nNodeSibling = nSIB;

					g_undoManager.AddAction(pAction);
					UpdateUndoRedoMenus();
				}
			}
		}
		gtk_tree_path_free(path1);
	}

	restart_enc_unload_timer();
}

void do_node_move_right(bool bStoreUndo)
{
	GtkTreeView *treeview = (GtkTreeView *)lookup_widget(window1, "treeview1");
	GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);

	//get iterator to selected node
	GtkTreeSelection* treesel = gtk_tree_view_get_selection (treeview);
	GtkTreeIter iter;

	if(gtk_tree_selection_get_selected(treesel, &model, &iter))
	{
		//get document node index from GUI tree iterator
		GtkTreePath* path1 = gtk_tree_model_get_path(model, &iter);
		int nIdx = NodeIdxFromPath(path1);
		if(nIdx > -1)
		{
			g_objCrashLog.Printf("ACTION: Move node right (IDX=%d)\n", nIdx);

			//find previous sibling
			if(!gtk_tree_path_prev(path1)){
				gtk_tree_path_free(path1);
				return;
			}

			//iter from path
			GtkTreeIter itPrev;
			gtk_tree_model_get_iter(model, &itPrev, path1);

			//delete current node
			gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);

			//change node position in the tree
			NoteNode &node = g_doc.GetNodeByIdx(nIdx);
			int nParentID = node.m_nParentID;
			int nSIB = node.m_nSiblingIdx;

			DocumentIterator itDoc(g_doc);
			int nNewParentIdx = itDoc.GetChildIdx(nParentID, nSIB-1);
			if(nNewParentIdx < 0){
				gtk_tree_path_free(path1);
				return;
			}
			int nNewParentID = g_doc.GetNodeByIdx(nNewParentIdx).m_nID;

			//recalc sibling
			node.m_nSiblingIdx = itDoc.GetChildCount(nNewParentID);

			//change parent
			node.m_nParentID = nNewParentID;

			//refresh indexes of previous siblings
			for(unsigned int i=0; i<g_doc.GetNodeCount(); i++)
			{
				if( g_doc.GetNodeByIdx(i).m_nParentID == nParentID &&
					g_doc.GetNodeByIdx(i).m_nSiblingIdx > nSIB)
				{
					g_doc.GetNodeByIdx(i).m_nSiblingIdx --;
				}
			}

		#ifdef _DEBUG
			g_doc.Dump();
			g_doc.AssertValid();
		#endif

			//attach as new child of a new parent (including subnodes)
			while(gtk_tree_model_iter_children(model, &iter, &itPrev))
				gtk_tree_store_remove(GTK_TREE_STORE(model), &iter); //delete all children
			add_child_nodes(&itPrev, nNewParentID);				     //reattach children with new chld

			g_doc.SetModified(true);
			RefreshMainTitle();

			gtk_tree_view_expand_row(treeview, path1, FALSE);	//expand new parent

			GtkTreePath *path2 = NULL;
			if(PathFromNodeIdx(nIdx, path2)){
				GtkTreeViewColumn *column1 = gtk_tree_view_get_column(treeview, 0);
				gtk_tree_view_set_cursor(treeview, path2, column1, FALSE);	//refresh node selection
				gtk_tree_path_free(path2);
			}

			if(bStoreUndo)
			{
				//push document change into undo/redo manager
				DocAction *pAction = new DocAction;
				pAction->SetType(ACT_TREE_MOVE);
				pAction->SetDoc(g_doc);
				pAction->m_nMoveDirection = MOVE_RIGHT;
				pAction->m_nNodeIndex = nIdx; //TOFIX recursive index ????
				pAction->m_nNodeSibling = nSIB;

				g_undoManager.AddAction(pAction);
				UpdateUndoRedoMenus();
			}
		}
		gtk_tree_path_free(path1);
	}

	restart_enc_unload_timer();
}

void on_reload1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	g_objCrashLog.AddMsg("ACTION: Reload\n");

	if( g_doc.GetPath().size() > 0 )
	{
		gint result = gtkMessageBox(_("Do you want to reload the document and lose your possible changes?"), GTK_BUTTONS_YES_NO);
		if(GTK_RESPONSE_YES == result)
		{
			std::string strFile = g_doc.GetPath();

			int nResult = load_file(strFile.c_str());

			//do not delete MRU for file that exists, but failed to open
			g_objMRU.Change(strFile.c_str(), (DOC_LOAD_NOT_FOUND != nResult));
		}
	}
	else
		gtkMessageBox(_("Document has not been saved yet!"));

	restart_enc_unload_timer();
}

void on_menu_expand_all (GtkMenuItem *menuitem, gpointer user_data)
{
	g_objCrashLog.AddMsg("ACTION: Expand all\n");

	GtkTreeView *treeview = (GtkTreeView *)lookup_widget(window1, "treeview1");
	gtk_tree_view_expand_all(treeview);

	restart_enc_unload_timer();
}

void on_menu_copy_branch (GtkMenuItem *menuitem, gpointer user_data)
{
	g_objCrashLog.AddMsg("ACTION: Copy branch\n");

	int nIdx = GetSelectedNodeIdx();
	if(nIdx >= 0)
	{
		UpdateTextFromScreen();
		g_doc.GetSubtree(nIdx, g_lstNodeClipboard);
	}

	restart_enc_unload_timer();
}

void on_menu_copy_branch_structure(GtkMenuItem *menuitem, gpointer user_data)
{
	g_objCrashLog.AddMsg("ACTION: Copy branch struct\n");

	on_menu_copy_branch(NULL, NULL);

	//clear node contents (only titles + structure will remain)
	int nCount = g_lstNodeClipboard.size();
	for(int i=0; i<nCount; i++){
		std::string strTitle = g_lstNodeClipboard[i].GetTitle();
		g_lstNodeClipboard[i].Clear();
		g_lstNodeClipboard[i].SetTitle(strTitle.c_str());
	}
}

void on_menu_cut_branch (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	g_objCrashLog.AddMsg("ACTION: Cut branch\n");

	int nIdx = GetSelectedNodeIdx();
	if(nIdx >= 0)
	{
		UpdateTextFromScreen();
		g_doc.GetSubtree(nIdx, g_lstNodeClipboard);
		on_menu_delete_node(NULL, (void *)1); // 1 - skip user confirmation
	}

	restart_enc_unload_timer();
}

void on_menu_paste_branch (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	g_objCrashLog.AddMsg("ACTION: Paste branch\n");

	if(g_lstNodeClipboard.size()<1)
		return;

	int nIdx = GetSelectedNodeIdx();
	if(nIdx >= 0)
	{
		NoteDocument doc;
		doc.AssignSubtree(g_lstNodeClipboard);

		int nParentID = g_doc.GetNodeByIdx(nIdx).m_nID;

		DocumentIterator it(g_doc);
		int nCnt = it.GetChildCount(nParentID);

		g_doc.Merge(doc, nParentID);

		//refresh tree starting from new content
		GtkTreeIter iter;
		IteratorFromNodeIdx(nIdx, iter);
		add_child_nodes(&iter, nParentID, nCnt);

		int nNewIdx = it.GetChildIdx(nParentID, nCnt);
		NoteNode &newNode = g_doc.GetNodeByIdx(nNewIdx);

		//get subtree from the new tree (having correct IDs)
		std::vector<NoteNode> lstSubtree;
		g_doc.GetSubtree(nNewIdx, lstSubtree);

		//push document change into undo/redo manager
		DocAction *pAction = new DocAction;
		pAction->SetType(ACT_NODE_INSERT);
		pAction->SetDoc(g_doc);
		pAction->m_nNodeIndex   = nNewIdx; //TOFIX recursive index
		pAction->m_nNodeID      = newNode.m_nID;
		pAction->m_nNodePID     = newNode.m_nParentID;
		pAction->m_nNodeSibling = newNode.m_nSiblingIdx;
		pAction->m_objSubTree.AssignSubtree(lstSubtree);

		g_undoManager.AddAction(pAction);
		UpdateUndoRedoMenus();

		g_doc.SetModified(true);
		RefreshMainTitle();

		restart_enc_unload_timer();
	}
	else
	{
		on_menu_paste_branch_root (NULL, NULL);
	}
}

void on_menu_paste_branch_root (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	g_objCrashLog.AddMsg("ACTION: Paste branch to root\n");

	if(g_lstNodeClipboard.size()<1)
		return;

	NoteDocument doc;
	doc.AssignSubtree(g_lstNodeClipboard);
	doc.Dump();

	DocumentIterator it(g_doc);
	int nCnt   = it.GetChildCount(-1);

	g_doc.Merge(doc);

	//refresh tree starting from new content
	add_child_nodes(NULL, -1, nCnt);

	int nNewIdx = it.GetChildIdx(-1, nCnt);
	NoteNode &newNode = g_doc.GetNodeByIdx(nNewIdx);

	SelectNodeByIdx(nNewIdx);

	//get subtree from the new tree (having correct IDs)
	std::vector<NoteNode> lstSubtree;
	g_doc.GetSubtree(nNewIdx, lstSubtree);

	//push document change into undo/redo manager
	DocAction *pAction = new DocAction;
	pAction->SetType(ACT_NODE_INSERT);
	pAction->SetDoc(g_doc);
	pAction->m_nNodeIndex   = nNewIdx; //TOFIX recursive index
	pAction->m_nNodeID      = newNode.m_nID;
	pAction->m_nNodePID     = newNode.m_nParentID;
	pAction->m_nNodeSibling = newNode.m_nSiblingIdx;
	pAction->m_objSubTree.AssignSubtree(lstSubtree);

	g_undoManager.AddAction(pAction);
	UpdateUndoRedoMenus();

	g_doc.SetModified(true);
	RefreshMainTitle();

	restart_enc_unload_timer();
}

void on_menu_insert_date_time (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	g_objCrashLog.AddMsg("ACTION: Insert date/time\n");

	//paste current date/time into the text view (or tree node title being edited)
	//if focus correct, apply command
	if(g_text.m_pWidget == gtk_window_get_focus(GTK_WINDOW(window1)))
	{
		int nIdx = GetSelectedNodeIdx();
		if(nIdx < 0)
			return;

		DateTimeDlg dlg;
		if(GTK_RESPONSE_OK != dlg.ShowModal())
			return;
		std::string strDateTime = dlg.GetValue();

		if(g_text.IsEditable())
		{
			gint nCursor, nSelection;
			g_text.GetSelectionBounds(nCursor, nSelection);

			g_text.InsertText(nCursor, strDateTime.c_str());	//TOFIX replace selection if exists ?, select inserted text?
			int nSizeNew  = g_utf8_strlen(strDateTime.c_str(), -1);
			g_doc.GetNodeByIdx(nIdx).OnTxtInsert(nCursor, nSizeNew, strDateTime.c_str());
		}
	}
	else if(g_tree.m_pWidget != gtk_window_get_focus(GTK_WINDOW(window1)))
	{
		//get formatted date/time value
		//(we do not show popup dialog here because it would kill label editing)
		int nDateTimeFmt = 0;
		g_objIni.GetValue("Display", "DateTimeFormat", nDateTimeFmt, 0);
		std::string strDateTime = DateTimeDlg::GetValueForFormat(nDateTimeFmt);

		//handle case when in the middle of tree label editing
		GtkWidget *pWidget = gtk_window_get_focus(GTK_WINDOW(window1));

		gint nCursor, nSelection;
		g_object_get(G_OBJECT(pWidget), "cursor-position", &nCursor,
										"selection-bound", &nSelection, (char *)NULL);

		std::string strTxt = gtk_entry_get_text(GTK_ENTRY(pWidget));
		if(nCursor == nSelection)
		{
			strTxt.insert(nCursor, strDateTime);

			nCursor = nCursor + strDateTime.size();
			nSelection = nCursor - strDateTime.size();
		}
		else
		{
			int nLeft = min(nCursor, nSelection);
			int nSize = abs(nCursor - nSelection);

			//replace selection with date/time string
			strTxt.erase(nLeft, nSize);
			strTxt.insert(nLeft, strDateTime);

			nCursor = nLeft + strDateTime.size();
			nSelection = nCursor - strDateTime.size();
		}
		gtk_entry_set_text(GTK_ENTRY(pWidget), strTxt.c_str());
		gtk_entry_select_region(GTK_ENTRY(pWidget), nCursor, nCursor);
	}
	else if (NULL != g_pNodePropWidget)
	{
		//handle case when inside nodeproperties dialog
		GtkWidget *pWidget = gtk_window_get_focus(GTK_WINDOW(g_pNodePropWidget));
		date_time_paste_to_text_widget(pWidget);
	}

	restart_enc_unload_timer();
}

void on_menu_collapse_all (GtkMenuItem *menuitem, gpointer user_data)
{
	g_objCrashLog.AddMsg("ACTION: Collapse all\n");

	//TOFIX fix tree selection losing when node is collapsed and becomes invisible -> set empty screen active?
	GtkTreeView *treeview = (GtkTreeView *)lookup_widget(window1, "treeview1");
	gtk_tree_view_collapse_all(treeview);

	restart_enc_unload_timer();
}

void on_menu_node_properties (GtkMenuItem *menuitem, gpointer user_data)
{
	g_objCrashLog.AddMsg("ACTION: Show node properties\n");

	int nIdx = GetSelectedNodeIdx();
	if(nIdx >= 0)
	{
		NoteNode &node = g_doc.GetNodeByIdx(nIdx);

		if(g_doc.IsReadOnly()){
			gtkMessageBox(_("The document is read-only! You won't be able to make any changes!"));
		}

		NodePropertiesDlg dlg;
		dlg.m_strKeywords = node.GetKeywords();
		dlg.m_strTitle = node.GetTitle();
		dlg.m_nCreated = node.m_nDateCreated;
		dlg.m_nModified = node.m_nDateModified;
		dlg.m_bFinished = node.m_bFinished;
		#ifdef HAVE_GTKSOURCEVIEW	
		dlg.m_strSourceLanguage = node.GetSourceLanguage();
		#endif
		dlg.Create();
		dlg.SetIconType(node.m_nIconType);
		if(ICON_CUSTOM == node.m_nIconType)
			dlg.SetIconValue(node.m_strIconFile.c_str());

		if(GTK_RESPONSE_OK == dlg.ShowModal())
		{
			if(g_doc.IsReadOnly()){
				gtkMessageBox(_("Can not change read-only document!"));
				return;
			}

			if(0 != strcmp(node.GetTitle().c_str(), dlg.GetNodeTitle().c_str()))
			{
				SetNodeTitle(nIdx, dlg.GetNodeTitle().c_str());
				g_doc.SetModified(true);
				RefreshMainTitle();
			}

		#ifdef HAVE_GTKSOURCEVIEW			
			node.SetSourceLanguage(dlg.GetSourceLanguage().c_str());
			// refresh text source language
			g_text.SetSourceLanguage(node.GetSourceLanguage().c_str());
			RefreshTextFormat(node, false);
		#endif

			//set icon
			node.m_nIconType = dlg.GetIconType();
			node.m_strIconFile = dlg.GetIconValue();
			if(dlg.GetIconType() == ICON_INTERNAL_FIRST)
			{
				node.m_nIconType += InternalIcon_Name2Index(dlg.GetIconValue());
			}
			UpdateNodeIcon(nIdx);
			g_doc.SetModified(true); //TOFIX detect if icon changed
			RefreshMainTitle();

			//set keywords
			//TOFIX see if modified
			dlg.RefreshKeywords();
			node.SetKeywords(dlg.m_strKeywords.c_str());

			//set/refresh gui for the "finished" state
			node.m_bFinished = dlg.m_bFinished;
			GtkTreeIter iter;
			if(IteratorFromNodeIdx(nIdx, iter)){
				GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)g_tree.m_pWidget);
				gtk_tree_store_set (GTK_TREE_STORE(model), &iter, STORE_IDX_STRIKE, dlg.m_bFinished, -1);
			}
		}
	}
	else
		gtkMessageBox(_("No Node selected"));

	restart_enc_unload_timer();
}

void on_show_status_bar_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWidget *menu = lookup_widget(window1, "statbar1");
	GtkWidget *statusbar1 = lookup_widget(window1, "statusbar1");

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menu)))
		gtk_widget_show(statusbar1);
	else
		gtk_widget_hide(statusbar1);

	restart_enc_unload_timer();
}

void refresh_statbar_menu(bool bActivate)
{
	GtkWidget *menutool1 = lookup_widget(window1, "statbar1");
	if(bActivate)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menutool1), TRUE);
	else
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menutool1), FALSE);
}

void set_show_status_bar(bool bShow)
{
	bool bIsVisible = get_show_status_bar();

	if((bShow && !bIsVisible) || (!bShow && bIsVisible))
		refresh_statbar_menu(bShow); //this will trigger the signal
}

bool get_show_status_bar()
{
	GtkWidget *toolbar1 = lookup_widget(window1, "statusbar1");
	return GTK_WIDGET_VISIBLE(toolbar1);
}

void stop_autosave_timer()
{
	if(g_nAutosaveTimer > 0)
		g_source_remove (g_nAutosaveTimer);
	g_nAutosaveTimer = 0;
}

void restart_autosave()
{
	stop_autosave_timer();	//ensure timer is stopped

	if(g_doc.IsReadOnly()){
		//"Can not change read-only document!"
		return;
	}

	//read ini settings
	bool bAutosave = false;
	g_objIni.GetValue("Other", "UseAutosave", bAutosave);
	if(bAutosave)
	{
		g_objCrashLog.AddMsg("ACTION: Restart autosave\n");

		int nAutosaveTimer;
		g_objIni.GetValue("Other", "AutosaveTimer", nAutosaveTimer, 30);
		g_nAutosaveTimer = g_timeout_add (nAutosaveTimer*1000, autosave_timer, NULL);
	}
}

gboolean autosave_timer(gpointer data)
{
	TRACE("Starting autosave timer\n");

	//DATA SECURITY: should we skip autosaving memory-only documents (never been saved to disk)
	if( g_bSkipMemOnlyAutosave &&
		g_doc.GetPath().empty())
		return TRUE;

	//save the current document to the special/reserved file/place
	if(g_doc.m_bModifiedSinceAutosave)
	{
		g_doc.m_bModifiedSinceAutosave = false;

		//ensure target directory exists
		//(fixes problems with autosave in Win portable version)
		std::string strDir = get_autosave_dir();
		EnsureNotTerminated(strDir);
		EnsureDirExists(strDir);    //TOFIX check success

		if(g_doc.IsEncrypted())
			save_file(calculate_autosave_filename1(), false, true, g_doc.GetPassword());
		else
			save_file(calculate_autosave_filename(), false, true);
	}

	return TRUE;
}

bool OnAutosaveFileEnum(const char *szFile, void *data)
{
	std::vector<std::string> *lstFiles = (std::vector<std::string> *)data;
	if(data)
	{
	#ifdef _WIN32
		//calculate base name
		std::string strFile(szFile);
		int nPos = strFile.find_last_of('\\');
		if(nPos > 0)
			strFile = strFile.substr(nPos+1);
	#else
		std::string strFile(basename((char *)szFile));
	#endif

		if(0 == strncmp(strFile.c_str(), "autosave", strlen("autosave"))){
			lstFiles->push_back(szFile);
		}
	}

	return true;	//keep enumerating
}

bool autosave_check_crash()
{
	bool bLoaded = false;

	//list all autosave files
	std::string strDir = get_autosave_dir();
	std::vector<std::string> lstFiles;
	EnumDirectory(strDir.c_str(), OnAutosaveFileEnum, (void *)&lstFiles, ENUM_LST_FILES);

	int nCount = lstFiles.size();
	if(nCount > 0)
	{
		//process only first file that matches
		for(int i=0; i<nCount; i++)
		{
			std::string strPath = lstFiles[i];

		#ifdef _WIN32
			//calculate base name
			std::string strFile(strPath);
			int nPos1 = strFile.find_last_of('\\');
			if(nPos1 > 0)
				strFile = strFile.substr(nPos1+1);
		#else
			std::string strFile(basename((char *)strPath.c_str()));
		#endif

			//try to extract PID from file name (if exists)
			unsigned int nPID = 0;
			std::string strPID = strFile;
			std::string::size_type nPos = strPID.rfind('_');
			if(nPos != std::string::npos){
				strPID = strPID.substr(nPos+1);
				if(strPID.size() > 0 && strPID.at(0) == 'n') //correct segment with PID
				{
					strPID = strPID.substr(1); // cut 'n' marker

					nPos = strPID.find('_');
					if(nPos != std::string::npos)
						strPID = strPID.substr(0, nPos);

					nPID = atoi(strPID.c_str());
				}
			}

			//check if PID belongs to a running application
			if(nPID > 0 && (nPID == g_nProcessID || IsPIDRunning(nPID)))
			{
				continue; // skip this file, app that created it is still running
			}


			//file belongs to dead app, or old format with no PID (assume dead app)
			if(0 == access(strPath.c_str(), 0))
			{
				//crash file exists, ask the user for decision
				int nRes = gtkMessageBox(_("Autosave file found! Do you want to load it?"), GTK_BUTTONS_YES_NO);
				if(GTK_RESPONSE_YES == nRes){
					if(!g_wnd.m_pWidget) g_wnd.Create();

					//allow password being entered up to 3 times
					int nTryCnt = 0;
					while(nTryCnt < 3){
						int nRes = load_file(strPath.c_str());
						nTryCnt ++;
						if(DOC_LOAD_WRONG_PASSWORD != nRes)
							break;
					}

					bLoaded = true;
				}

				if(bLoaded){
					//clear the internal path and make it modified to force the user to save it
					g_doc.ResetPath();
					g_doc.SetModified(true);
					RefreshMainTitle();
				}

				//now, important thing, delete the autosave file
				remove(strPath.c_str());
				return bLoaded;
			}
		}
	}

	return false;
}

void remove_autosave_unencrypted()
{
	//list all autosave files
	std::string strDir = get_autosave_dir();
	std::vector<std::string> lstFiles;
	EnumDirectory(strDir.c_str(), OnAutosaveFileEnum, (void *)&lstFiles, ENUM_LST_FILES);

	int nCount = lstFiles.size();
	if(nCount > 0)
	{
		//process only first file that matches  //TOFIX load all ????
		for(int i=0; i<nCount; i++)
		{
			std::string strPath = lstFiles[i];

		#ifdef _WIN32
			//calculate base name
			std::string strFile(strPath);
			int nPos1 = strFile.find_last_of('\\');
			if(nPos1 > 0)
				strFile = strFile.substr(nPos1+1);
		#else
			std::string strFile(basename((char *)strPath.c_str()));
		#endif

			//try to extract PID from file name (if exists)
			unsigned int nPID = 0;
			std::string strPID = strFile;
			std::string::size_type nPos = strPID.rfind('_');
			if(nPos != std::string::npos){
				strPID = strPID.substr(nPos+1);
				if(!strPID.empty() && strPID.at(0) == 'n') //correct segment with PID
				{
					strPID = strPID.substr(1); // cut 'n' marker

					nPos = strPID.find('_');
					if(nPos != std::string::npos)
						strPID = strPID.substr(0, nPos);

					nPID = atoi(strPID.c_str());
				}
			}

			//only if PID belongs to us
			if(nPID > 0 && nPID == g_nProcessID)
			{
				//delete the autosave file if unencrypted one
				std::string strExt = GetFileExt(strPath.c_str());
				if(".ncd" == strExt)	//TOFIX !IsEncryptedFormat(strExt)
					remove(strPath.c_str());
			}
		}
	}
}

void autosave_shutdown()
{
	stop_autosave_timer();	//ensure timer is stopped
	if(0 == access(calculate_autosave_filename(), 0))
		remove(calculate_autosave_filename());
	if(0 == access(calculate_autosave_filename1(), 0))
		remove(calculate_autosave_filename1());
}

gint textview_keyboard_handler(GtkWidget *widget, GdkEventKey *event)
{
	if(  event->keyval == GDK_F5  ||
		(event->keyval == 'p' && (event->state & GDK_CONTROL_MASK) ))
	{
		g_tree.SetFocus();
		return FALSE;
	}
	return FALSE;
}

void UpdateNodeIcon(int nIdx)
{
	//get iterator to given node
	GtkWidget *treeview = lookup_widget(window1, "treeview1");
	GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);
	DocumentIterator itDoc(g_doc);

	//store new text into the tree store for given cell
	GtkTreePath *path1 = NULL;
	if(!PathFromNodeIdx(nIdx, path1))
		return;

	GtkTreeIter iter;
	gtk_tree_model_get_iter(model, &iter, path1);

	NoteNode &node = g_doc.GetNodeByIdx(nIdx);
	if(ICON_NONE != node.m_nIconType)
	{
		if(ICON_CUSTOM == node.m_nIconType)
		{
			//external xpm icon file
			GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size(node.m_strIconFile.c_str(), 16, 16, NULL);
			if(pixbuf)
			{
				gtk_tree_store_set (GTK_TREE_STORE(model), &iter, STORE_IDX_ICON, pixbuf, -1);
				g_object_unref (G_OBJECT (pixbuf));
			}
		}
		else if(node.m_nIconType > ICON_CUSTOM)
		{
			//one of the internal icons
			const char **szIconData = InternalIcon_GetFromIdx(node.m_nIconType);
			GdkPixbuf *pixbuf = gdk_pixbuf_new_from_xpm_data (szIconData);
			GdkPixbuf *destpix = gdk_pixbuf_scale_simple(pixbuf, 16, 16, GDK_INTERP_BILINEAR);
			g_object_unref (G_OBJECT (pixbuf));
			gtk_tree_store_set (GTK_TREE_STORE(model), &iter, STORE_IDX_ICON, destpix, -1);
			g_object_unref (G_OBJECT (destpix));
		}
	}
	else
		gtk_tree_store_set (GTK_TREE_STORE(model), &iter, STORE_IDX_ICON, NULL, -1);	//clear icon

	gtk_tree_path_free(path1);
}

void on_dnd_row_deleted (GtkTreeModel *treemodel, GtkTreePath *arg1, gpointer user_data)
{
	if(g_tree.m_bDND)	// handles only internal DND
	{
		g_objCrashLog.AddMsg("Drag and drop: row deleted\n");

		int nIdx = NodeIdxFromPath(arg1);
		if(nIdx >= 0)
		{
			//debugging
			TRACE("Dnd: Deleting node idx=%d at path [%s]\n", nIdx, gtk_tree_path_to_string(arg1));

			//on dragged subtree root node being deleted, first restore all the properties
			//of its node and subnodes at the new position (before deleting the nodes in document)
			int nNewRootIdx = g_tree.m_nFirstDroppedIdx;

			//is this the first node processed in this DnD operation ?
			//(if so, than it is the one being dragged, the top of the branch)
			if(-1 == g_tree.m_nFirstDraggedIdx)
				g_tree.m_nFirstDraggedIdx = nIdx;

			//copy node contents into new nodes
			//(map old subtree starting at nIdx to new subtree starting at nNewRootIdx)
			if(nNewRootIdx >= 0)
			{
				DocumentIterator it(g_doc);

				NoteNode &newRoot = g_doc.GetNodeByIdx(nNewRootIdx);
				NoteNode &node = g_doc.GetNodeByIdx(nIdx);

				int nNewPID = newRoot.m_nParentID;
				int nNewSID = newRoot.m_nSiblingIdx;

				//copy undo data right now (on first/top node being deleted)
				//push document change into undo/redo manager (menu is updated at the end of DnD)
				DocAction *pAction = new DocAction;
				pAction->SetType(ACT_TREE_DND);
				pAction->SetDoc(g_doc);
				pAction->m_nNodeIndex = nIdx;
				pAction->m_objSubTree.AssignSubtree(g_doc, nIdx, true);
				pAction->m_nNodeID         = node.m_nID;
				pAction->m_nNodePID        = node.m_nParentID;
				pAction->m_nNodeSibling    = node.m_nSiblingIdx;
				pAction->m_nNewNodeID      = pAction->m_nNodeID;
				pAction->m_nNodeNewPID     = nNewPID;
				pAction->m_nNodeNewSibling = nNewSID;

				//fix sibling index if the node was dropped within the same branch
				if(pAction->m_nNodePID == pAction->m_nNodeNewPID){
					//if(pAction->m_nNodeNewSibling < pAction->m_nNodeSibling){
						pAction->m_nNodeSibling --;
					//}
				}

				g_undoManager.AddAction(pAction);

				//move old node subtree to new place
				g_doc.NodeDelete(newRoot.m_nID); //was a placeholder
				g_doc.NodeMove(g_doc.GetNodeByIdx(nIdx).m_nID, nNewPID, nNewSID);
			}
		}

		//mark document changed
		g_doc.SetModified(true);
		RefreshMainTitle();
	}
}

void on_dnd_row_inserted (GtkTreeModel *treemodel, GtkTreePath *arg1, GtkTreeIter *arg2, gpointer user_data)
{
	if(g_tree.m_bDND)	// handles only internal DND
	{
		g_objCrashLog.AddMsg("Drag and drop: row inserted\n");

		//
		// make a new node at drop point (copying from selected node)
		//

		//calc parent node
		int nPID = -1;
		GtkTreePath *parent = gtk_tree_path_copy(arg1);
		if(gtk_tree_path_up(parent)){
			int nParentIdx = NodeIdxFromPath(parent);
			if(nParentIdx >= 0){
				ASSERT(0 <= nParentIdx && nParentIdx < (int)g_doc.GetNodeCount());
				nPID = g_doc.GetNodeByIdx(nParentIdx).m_nID;
			}
		}

		//calc sibling position of a new node
		gint* arrIndices = gtk_tree_path_get_indices(arg1);
		int nMax = gtk_tree_path_get_depth(arg1);
		int nSID = arrIndices[nMax-1];

		DocumentIterator it(g_doc);
		int nPrevSiblingIdx = it.GetChildIdx(nPID, nSID);
		int nPrevSiblingID  = -1;	//sibling index of previous sibling node
		if(nPrevSiblingIdx >= 0)
			nPrevSiblingID  = g_doc.GetNodeByIdx(nPrevSiblingIdx).m_nSiblingIdx;

		// insert new node into document
		g_doc.NodeInsert(nPID, nPrevSiblingID);

		//remeber the new node index (data copy will be handled on deleting old node)
		int nNewIdx = g_doc.GetNodeCount()-1;

		//debugging
	#ifdef _DEBUG
		char *szPath1 = gtk_tree_path_to_string(arg1);
		char *szPath2 = gtk_tree_path_to_string(parent);
		TRACE("Dnd: Inserted new node idx=%d at path %s\n", nNewIdx, szPath1);
		TRACE("Dnd: Parent path [%s], Parent ID = %d, SID=%d, prevSibIdx=%d, prevSIB=%d\n", szPath2, nPID, nSID, nPrevSiblingIdx, nPrevSiblingID);
		g_free(szPath1);
		g_free(szPath2);
	#endif

		if(g_tree.m_nFirstDroppedIdx == -1)
			g_tree.m_nFirstDroppedIdx = nNewIdx;

		g_doc.m_nActiveNodeIdx = -1;

		//fix the edit control (no more active node)
		g_text.Clear();
		g_text.SetEditable(false);

	#ifdef _DEBUG
		g_doc.Dump();
	#endif
		gtk_tree_path_free(parent);
	}
}

void on_link_wizard_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	g_objCrashLog.AddMsg("ACTION: Link wizard\n");

	int nIdx = GetSelectedNodeIdx();
	if(nIdx < 0){
		gtkMessageBox(_("No selected node!"));
		return;	// no node selected
	}

	bool bTreeLink = false;
	if(g_tree.m_pWidget == gtk_window_get_focus(GTK_WINDOW(window1)))
		bTreeLink = true;

	int nCursor = 0;
	int nSelection = 0;

	NoteNode &node = g_doc.GetNodeByIdx(nIdx);
	LinkInfo info;

	if(bTreeLink)
	{
		info = node.m_objLink;
		info.m_strText = node.GetTitle();
		info.RefreshLength();
	}
	else
	{
		UpdateTextFromScreen();

		//get selected text
		g_text.GetSelectionBounds(nCursor, nSelection);

		int nSelLower = min(nCursor, nSelection);
		int nSelUpper = max(nCursor, nSelection);

		//remove all non-chars before this pos in the buffer (pictures)
		int nPicBeforeCount = 0;
		int nCnt = node.m_lstPictures.size();
		for(int i=0; i<nCnt; i++)
		{
			if(node.m_lstPictures[i].nOffset < nSelLower)
				nPicBeforeCount ++;
			else
				break;	//TOFIX are they sorted by offset
		}
		nSelLower -= nPicBeforeCount; // buffer offset to char offset
		nSelUpper -= nPicBeforeCount; // buffer offset to char offset

		// some text selected, replace it with the given string
		std::string strTxt = node.GetText();

		//convert from character offset to buffer offset
		int nCursorBuf, nSelBuf;
		const char *szBuf = strTxt.c_str();
		gchar* pszPos = g_utf8_offset_to_pointer(szBuf, nSelLower);
		nCursorBuf = pszPos - szBuf;
		pszPos = g_utf8_offset_to_pointer(szBuf, nSelUpper);
		nSelBuf = pszPos - szBuf;

		int nLenBuf = nSelBuf-nCursorBuf;
		if(nLenBuf < 0)
			nLenBuf = -nLenBuf;
		nCursorBuf = min(nSelBuf, nCursorBuf);

		info.m_nStartOffset = nSelLower;
		info.m_strText = strTxt.substr(nCursorBuf, nLenBuf);
		info.RefreshLength();

		//see if this link already exists (and fill its target)
		int nPos = node.m_lstLinks.Find(info.m_nStartOffset);
		if(nPos >= 0)
		{
			if(nCursor == nSelection) // no text selected, assume boundaries of the found link
				info = node.m_lstLinks[nPos];
			else{
				info.m_strTargetURL		= node.m_lstLinks[nPos].m_strTargetURL;	// only copy target
				info.m_nTargetNodeID	= node.m_lstLinks[nPos].m_nTargetNodeID;// only copy target
			}
		}
		else{
			//no text selection and no link on the cursor pos
			if(nCursor == nSelection){
				gtkMessageBox(_("No text selection!"));
				return;	// no text selected
			}

			//set initial version of URL to selected text (user can change it later)
			info.m_strTargetURL		= info.m_strText;
		}
	}

	int nRes = 0;

	LinkPropertiesDlg dlg;
	dlg.m_info = info;
	dlg.Create();
	nRes = dlg.ShowModal();
	dlg.UpdateData(false);
	dlg.Destroy();

	if(GTK_RESPONSE_OK == nRes)
	{
		if( dlg.m_info.m_strTargetURL.size() > 0 ||
			dlg.m_info.m_nTargetNodeID >= 0)
		{
			if(dlg.m_info.m_strTargetURL.size() > 0)
			{
				//make sure url has valid prefix
				if(std::string::npos == dlg.m_info.m_strTargetURL.find("://")){
				#ifdef _WIN32
					dlg.m_info.m_strTargetURL.insert(0, "/");	// convert Windows path to Unix style /C:\a.html
				#endif
					dlg.m_info.m_strTargetURL.insert(0, "file://");	// assume local file if no protocol defined
				}
			}
			else
			{
				//increment "linked" count for the target ID
				std::map<int, int>::iterator it2 = g_doc.m_mapIdIsTarget.find(dlg.m_info.m_nTargetNodeID);
				if(it2 == g_doc.m_mapIdIsTarget.end())
					g_doc.m_mapIdIsTarget[dlg.m_info.m_nTargetNodeID] = 1;
				else
					g_doc.m_mapIdIsTarget[dlg.m_info.m_nTargetNodeID] ++;
			}

			if(bTreeLink)
			{
				node.m_objLink = dlg.m_info;

				GtkWidget *treeview = lookup_widget(window1, "treeview1");
				GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);
				GtkTreeIter iter;
				if(IteratorFromNodeIdx(nIdx, iter)){
					gtk_tree_store_set (GTK_TREE_STORE(model), &iter, STORE_IDX_UNDERLINE, node.m_objLink.IsValid()? PANGO_UNDERLINE_SINGLE : PANGO_UNDERLINE_NONE , -1);
					gtk_tree_store_set (GTK_TREE_STORE(model), &iter, STORE_IDX_COLOR, node.m_objLink.IsValid()? &g_linkColor : &g_blackColor, -1);
				}

				//TOFIX push undo state
			}
			else
			{
				//ensure no other links occupy the same space - break other links!!!
				node.m_lstLinks.OnLinkCreated(info.m_nStartOffset, info.m_nTextLength);

				//ensure no formatting exists in the link area
				node.m_lstTxtFmt.ClearFormatRange(info.m_nStartOffset, info.m_nStartOffset+info.m_nTextLength, FMT_BOLD_BEGIN);
				node.m_lstTxtFmt.ClearFormatRange(info.m_nStartOffset, info.m_nStartOffset+info.m_nTextLength, FMT_ITALIC_BEGIN);
				node.m_lstTxtFmt.ClearFormatRange(info.m_nStartOffset, info.m_nStartOffset+info.m_nTextLength, FMT_UNDERLINE_BEGIN);
				node.m_lstTxtFmt.ClearFormatRange(info.m_nStartOffset, info.m_nStartOffset+info.m_nTextLength, FMT_STRIKE_BEGIN);
				node.m_lstTxtFmt.ClearFormatRange(info.m_nStartOffset, info.m_nStartOffset+info.m_nTextLength, FMT_TXT_COLOR_BEGIN);
				node.m_lstTxtFmt.ClearFormatRange(info.m_nStartOffset, info.m_nStartOffset+info.m_nTextLength, FMT_TXT_BKG_COLOR_BEGIN);

				// add link into the node
				node.m_lstLinks.push_back(dlg.m_info);
				g_text.RemoveTextStyles(nCursor, nSelection);
				g_text.SetTextUnderlined(nCursor, nSelection);

				//push document change into undo/redo manager
				DocAction *pAction = new DocAction;
				pAction->SetType(ACT_TEXT_LINK_ADD);
				pAction->SetDoc(g_doc);
				pAction->m_nNodeIndex   = nIdx; //TOFIX recursive index
				pAction->m_nLinkOffset  = info.m_nStartOffset;
				pAction->m_strLinkUrl   = dlg.m_info.m_strTargetURL;
				pAction->m_nLinkNodeID  = dlg.m_info.m_nTargetNodeID;
				pAction->m_strLinkTitle = dlg.m_info.m_strText;
				g_undoManager.AddAction(pAction);
				UpdateUndoRedoMenus();
			}
		}
		else
		{
			if(bTreeLink)
			{
				node.m_objLink = LinkInfo();

				GtkWidget *treeview = lookup_widget(window1, "treeview1");
				GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);
				GtkTreeIter iter;
				if(IteratorFromNodeIdx(nIdx, iter)){
					gtk_tree_store_set (GTK_TREE_STORE(model), &iter, STORE_IDX_UNDERLINE, PANGO_UNDERLINE_NONE , -1);
					gtk_tree_store_set (GTK_TREE_STORE(model), &iter, STORE_IDX_COLOR, &g_blackColor, -1);
				}

				//TOFIX push undo state
			}
			else
			{
				//TOFIX remove all links that overlap this range?

				//remove link if having the empty target ("Remove" button clicked)
				int nPos = node.m_lstLinks.Find(dlg.m_info.m_nStartOffset);
				if(nPos >= 0)
				{
					if(info.m_nTargetNodeID >= 0)
					{
						//decrement "linked" count for this ID
						std::map<int, int>::iterator it2 = g_doc.m_mapIdIsTarget.find(info.m_nTargetNodeID);
						if(it2 != g_doc.m_mapIdIsTarget.end()){
							g_doc.m_mapIdIsTarget[info.m_nTargetNodeID] --;
							if((*it2).second <= 0)
								g_doc.m_mapIdIsTarget.erase(it2);
						}
					}

					//push document change into undo/redo manager
					DocAction *pAction = new DocAction;
					pAction->SetType(ACT_TEXT_LINK_REMOVE);
					pAction->SetDoc(g_doc);
					pAction->m_nNodeIndex   = nIdx; //TOFIX recursive index
					pAction->m_nLinkOffset  = info.m_nStartOffset;
					pAction->m_strLinkUrl   = node.m_lstLinks[nPos].m_strTargetURL;
					pAction->m_strLinkTitle = node.m_lstLinks[nPos].m_strText;
					pAction->m_nLinkNodeID  = node.m_lstLinks[nPos].m_nTargetNodeID;
					g_undoManager.AddAction(pAction);
					UpdateUndoRedoMenus();

					node.m_lstLinks.erase(node.m_lstLinks.begin()+nPos);
					g_text.RemoveTextStyles(dlg.m_info.m_nStartOffset, dlg.m_info.m_nStartOffset+dlg.m_info.m_nTextLength);
				}
			}
		}

		node.m_nDateModified = time(NULL);

		g_doc.SetModified(true);
		RefreshMainTitle();
		restart_enc_unload_timer();
	}
}

gboolean textview_mouse_moved(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
	static bool bHand = false;

	int nIdx = GetSelectedNodeIdx();
	if(nIdx >= 0)
	{
		//get clicked area character offset based on screen coordinates
		GtkTextIter iter;
		gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(widget), &iter, (gint)event->x, (gint)event->y);
		int nChar = gtk_text_iter_get_offset (&iter);

		//check if any link is at that position
		int nLnkIdx = g_doc.GetNodeByIdx(nIdx).m_lstLinks.Find(nChar);
		if(nLnkIdx >= 0)
		{
			//set hand cursor if mouse is over the link
			if(bHand)
				return FALSE;	//proper cursor already set

			bHand = true;
			GdkCursor* cursor = gdk_cursor_new(GDK_HAND2);
			gdk_window_set_cursor(gtk_text_view_get_window(GTK_TEXT_VIEW(widget),GTK_TEXT_WINDOW_TEXT), cursor);
			gdk_cursor_unref(cursor);
			return TRUE;
		}
	}

	//else - set normal cursor
	if(!bHand)
		return FALSE;	//proper cursor already set

	bHand = false;
	GdkCursor* cursor = gdk_cursor_new(GDK_XTERM);
	gdk_window_set_cursor(gtk_text_view_get_window(GTK_TEXT_VIEW(widget),GTK_TEXT_WINDOW_TEXT), cursor);
	gdk_cursor_unref(cursor);
	return TRUE;
}

gboolean textview_mouse_click(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	if(event->type == GDK_2BUTTON_PRESS)	// double-click
	{
		int nIdx = GetSelectedNodeIdx();
		if(nIdx >= 0)
		{
			//get clicked area character offset based on screen coordinates
			int  x, y;
			GtkTextWindowType text_window_type = gtk_text_view_get_window_type(GTK_TEXT_VIEW(widget), event->window);
			gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(widget), text_window_type, (gint)event->x, (gint)event->y, &x, &y);

			GtkTextIter iter;
			gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(widget), &iter, (gint)x, (gint)y);
			int nChar = gtk_text_iter_get_offset (&iter);

			NoteNode &node = g_doc.GetNodeByIdx(nIdx);

			//check if any link is at that position
			int nLnkIdx = node.m_lstLinks.Find(nChar);
			if(nLnkIdx >= 0)
			{
				ExecuteLink(node.m_lstLinks[nLnkIdx]);
				return TRUE;
			}
		}
	}

	return FALSE;	//propagate
}

void on_view_text_only_mode (GtkMenuItem *menuitem, gpointer user_data)
{
	g_objCrashLog.AddMsg("ACTION: Cycle view mode\n");

	GtkWidget *divider = lookup_widget(window1, "hbox1");
	GtkWidget *vbox2   = lookup_widget(window1, "vbox2");

	//get client area size
	gint nPosWidth = 0;
	gdk_window_get_size(GTK_WIDGET(divider)->window, &nPosWidth, NULL);

	static float fSplitterPercent = 0.5;

	if(GTK_WIDGET_VISIBLE(g_tree.m_pWidget))
	{
		if(GTK_WIDGET_VISIBLE(g_text.m_pWidget))
		{
			//store splitter position percent before hiding
			gint nWidthLeft = gtk_paned_get_position (GTK_PANED (divider));
			fSplitterPercent = (float)nWidthLeft / nPosWidth;

			//show text view only
			gtk_widget_hide (g_tree.m_pWidget);
			gtk_widget_set_sensitive(g_tree.m_pWidget, FALSE);

			gtk_paned_set_position(GTK_PANED(divider), 0);
			gtk_window_set_focus(GTK_WINDOW(window1), g_text.m_pWidget);
		}
		else
		{
			//store splitter position percent before hiding
			gint nWidthLeft = (gint)(fSplitterPercent * nPosWidth);

			//show both views
			gtk_paned_set_position(GTK_PANED(divider), nWidthLeft);

			gtk_widget_show (vbox2);
			gtk_widget_show (g_text.m_pWidget);
			gtk_widget_set_sensitive(g_text.m_pWidget, TRUE);

			gtk_widget_show (g_tree.m_pWidget);
			gtk_widget_set_sensitive(g_tree.m_pWidget, TRUE);
		}
	}
	else if(GTK_WIDGET_VISIBLE(g_text.m_pWidget))
	{
		//show tree view only
		gtk_widget_hide (vbox2);
		gtk_widget_hide (g_text.m_pWidget);
		gtk_widget_set_sensitive(g_text.m_pWidget, FALSE);

		gtk_widget_show (g_tree.m_pWidget);
		gtk_widget_set_sensitive(g_tree.m_pWidget, TRUE);

		gtk_paned_set_position(GTK_PANED(divider), nPosWidth);
		gtk_window_set_focus(GTK_WINDOW(window1), g_tree.m_pWidget);
	}
}

void on_view_calc_word_count (GtkMenuItem *menuitem, gpointer user_data)
{
	g_objCrashLog.AddMsg("ACTION: Calc word count\n");

	UpdateTextFromScreen();

	//calc word count for node title and node text separately
	int nTotalWords_Texts		= 0;
	int nTotalLines_Texts		= 0;
	int nTotalCharacters_Texts	= 0;

	int nTotalWords_Titles		= 0;
	int nTotalLines_Titles		= 0;
	int nTotalCharacters_Titles	= 0;

	int nCurNodeWords_Texts		= 0;
	int nCurNodeLines_Texts		= 0;
	int nCurNodeCharacters_Texts= 0;

	int nCurNodeWords_Titles		= 0;
	int nCurNodeLines_Titles		= 0;
	int nCurNodeCharacters_Titles	= 0;

	int nNodes = g_doc.GetNodeCount();
	for(int i=0; i<nNodes; i++)
	{
		//calc word count for node title
		int nWords		= 0;
		int nLines		= 0;
		int nCharacters	= 0;

		std::string strData = g_doc.GetNodeByIdx(i).GetTitle();
		calc_word_count(strData.c_str(), nWords, nLines, nCharacters);

		nTotalWords_Titles		+= nWords;
		nTotalLines_Titles		+= nLines;
		nTotalCharacters_Titles	+= nCharacters;

		if(i == g_doc.m_nActiveNodeIdx){
			nCurNodeWords_Titles		+= nWords;
			nCurNodeLines_Titles		+= nLines;
			nCurNodeCharacters_Titles	+= nCharacters;
		}

		//calc word count for node text
		nWords		= 0;
		nLines		= 0;
		nCharacters	= 0;

		strData = g_doc.GetNodeByIdx(i).GetText();
		calc_word_count(strData.c_str(), nWords, nLines, nCharacters);

		nTotalWords_Texts		+= nWords;
		nTotalLines_Texts		+= nLines;
		nTotalCharacters_Texts	+= nCharacters;

		if(i == g_doc.m_nActiveNodeIdx){
			nCurNodeWords_Texts			+= nWords;
			nCurNodeLines_Texts			+= nLines;
			nCurNodeCharacters_Texts	+= nCharacters;
		}
	}

	//
	// format and display the result
	//

	std::string strMsg;
	char szBuffer[2048];

	//display results in a special table
	GtkWidget* msgbox = gtk_dialog_new();

	gtk_window_set_title (GTK_WINDOW (msgbox), _("Word Count"));
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

	GtkWidget *dialog_vbox3 = GTK_DIALOG (msgbox)->vbox;
	gtk_widget_show (dialog_vbox3);

#ifndef _NOKIA_MAEMO // must conserve space on Nokia
	gtk_container_set_border_width  (GTK_CONTAINER (dialog_vbox3), 10);
	gtk_box_set_spacing(GTK_BOX (dialog_vbox3), 10);
#endif

	GtkWidget *table1 = gtk_table_new (4, 7, FALSE);
	gtk_widget_show (table1);
	gtk_container_add (GTK_CONTAINER (dialog_vbox3), table1);

	strMsg  = _("Word Count ( title / text / title and text )\n");
#ifndef _NOKIA_MAEMO // must conserve space on Nokia
	strMsg += _("NOTE: Line counting does not take into account word wrapping.\n");
	strMsg += "\n";
#endif
	strMsg += _("Total document");

	sprintf(szBuffer, _(" - %d node(s)"), g_doc.GetNodeCount());
	strMsg += szBuffer;

	GtkWidget *label21 = gtk_label_new (strMsg.c_str());
	gtk_widget_show (label21);
	gtk_table_attach (GTK_TABLE (table1), label21, 0, 4, 0, 1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label21), 0, 0.5);

	//----- words

	sprintf(szBuffer, "%d", nTotalWords_Titles);
	GtkWidget *label22 = gtk_label_new (szBuffer);
	gtk_widget_show (label22);
	gtk_table_attach (GTK_TABLE (table1), label22, 0, 1, 1, 2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label22), 1, 0.5);

	sprintf(szBuffer, "%d", nTotalWords_Texts);
	GtkWidget *label23 = gtk_label_new (szBuffer);
	gtk_widget_show (label23);
	gtk_table_attach (GTK_TABLE (table1), label23, 1, 2, 1, 2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label23), 1, 0.5);

	sprintf(szBuffer, "%d", nTotalWords_Titles + nTotalWords_Texts);
	GtkWidget *label24 = gtk_label_new (szBuffer);
	gtk_widget_show (label24);
	gtk_table_attach (GTK_TABLE (table1), label24, 2, 3, 1, 2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label24), 1, 0.5);

	GtkWidget *label25 = gtk_label_new (_("words"));
	gtk_widget_show (label25);
	gtk_table_attach (GTK_TABLE (table1), label25, 3, 4, 1, 2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label25), 1, 0.5);

	//----- lines

	sprintf(szBuffer, "%d", nTotalLines_Titles);
	label22 = gtk_label_new (szBuffer);
	gtk_widget_show (label22);
	gtk_table_attach (GTK_TABLE (table1), label22, 0, 1, 2, 3, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label22), 1, 0.5);

	sprintf(szBuffer, "%d", nTotalLines_Texts);
	label23 = gtk_label_new (szBuffer);
	gtk_widget_show (label23);
	gtk_table_attach (GTK_TABLE (table1), label23, 1, 2, 2, 3, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label23), 1, 0.5);

	sprintf(szBuffer, "%d", nTotalLines_Titles + nTotalLines_Texts);
	label24 = gtk_label_new (szBuffer);
	gtk_widget_show (label24);
	gtk_table_attach (GTK_TABLE (table1), label24, 2, 3, 2, 3, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label24), 1, 0.5);

	label25 = gtk_label_new (_("lines"));
	gtk_widget_show (label25);
	gtk_table_attach (GTK_TABLE (table1), label25, 3, 4, 2, 3, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label25), 1, 0.5);

	//----- characters

	sprintf(szBuffer, "%d", nTotalCharacters_Titles);
	label22 = gtk_label_new (szBuffer);
	gtk_widget_show (label22);
	gtk_table_attach (GTK_TABLE (table1), label22, 0, 1, 3, 4, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label22), 1, 0.5);

	sprintf(szBuffer, "%d", nTotalCharacters_Texts);
	label23 = gtk_label_new (szBuffer);
	gtk_widget_show (label23);
	gtk_table_attach (GTK_TABLE (table1), label23, 1, 2, 3, 4, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label23), 1, 0.5);

	sprintf(szBuffer, "%d", nTotalCharacters_Titles + nTotalCharacters_Texts);
	label24 = gtk_label_new (szBuffer);
	gtk_widget_show (label24);
	gtk_table_attach (GTK_TABLE (table1), label24, 2, 3, 3, 4, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label24), 1, 0.5);

	label25 = gtk_label_new (_("characters"));
	gtk_widget_show (label25);
	gtk_table_attach (GTK_TABLE (table1), label25, 3, 4, 3, 4, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label25), 1, 0.5);

	if(g_doc.m_nActiveNodeIdx >= 0)
	{
		strMsg = "";
#ifndef _NOKIA_MAEMO // must conserve space on Nokia
		strMsg = "\n";
#endif
		strMsg += _("Current node");

		DocumentIterator it(g_doc);
		sprintf(szBuffer, _(" - ancestor to %d node(s)"), it.GetChildCount(g_doc.GetNodeByIdx(g_doc.m_nActiveNodeIdx).m_nID, true));
		strMsg += szBuffer;

		label21 = gtk_label_new (strMsg.c_str());
		gtk_widget_show (label21);
		gtk_table_attach (GTK_TABLE (table1), label21, 0, 4, 4, 5, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_misc_set_alignment (GTK_MISC (label21), 0, 0.5);

		//----- words

		sprintf(szBuffer, "%d", nCurNodeWords_Titles);
		label22 = gtk_label_new (szBuffer);
		gtk_widget_show (label22);
		gtk_table_attach (GTK_TABLE (table1), label22, 0, 1, 5, 6, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_misc_set_alignment (GTK_MISC (label22), 1, 0.5);

		sprintf(szBuffer, "%d", nCurNodeWords_Texts);
		label23 = gtk_label_new (szBuffer);
		gtk_widget_show (label23);
		gtk_table_attach (GTK_TABLE (table1), label23, 1, 2, 5, 6, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_misc_set_alignment (GTK_MISC (label23), 1, 0.5);

		sprintf(szBuffer, "%d", nCurNodeWords_Titles + nCurNodeWords_Texts);
		label24 = gtk_label_new (szBuffer);
		gtk_widget_show (label24);
		gtk_table_attach (GTK_TABLE (table1), label24, 2, 3, 5, 6, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_misc_set_alignment (GTK_MISC (label24), 1, 0.5);

		label25 = gtk_label_new (_("words"));
		gtk_widget_show (label25);
		gtk_table_attach (GTK_TABLE (table1), label25, 3, 4, 5, 6, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_misc_set_alignment (GTK_MISC (label25), 1, 0.5);

		//----- lines

		sprintf(szBuffer, "%d", nCurNodeLines_Titles);
		label22 = gtk_label_new (szBuffer);
		gtk_widget_show (label22);
		gtk_table_attach (GTK_TABLE (table1), label22, 0, 1, 6, 7, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_misc_set_alignment (GTK_MISC (label22), 1, 0.5);

		sprintf(szBuffer, "%d", nCurNodeLines_Texts);
		label23 = gtk_label_new (szBuffer);
		gtk_widget_show (label23);
		gtk_table_attach (GTK_TABLE (table1), label23, 1, 2, 6, 7, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_misc_set_alignment (GTK_MISC (label23), 1, 0.5);

		sprintf(szBuffer, "%d", nCurNodeLines_Titles + nCurNodeLines_Texts);
		label24 = gtk_label_new (szBuffer);
		gtk_widget_show (label24);
		gtk_table_attach (GTK_TABLE (table1), label24, 2, 3, 6, 7, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_misc_set_alignment (GTK_MISC (label24), 1, 0.5);

		label25 = gtk_label_new (_("lines"));
		gtk_widget_show (label25);
		gtk_table_attach (GTK_TABLE (table1), label25, 3, 4, 6, 7, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_misc_set_alignment (GTK_MISC (label25), 1, 0.5);

		//----- characters

		sprintf(szBuffer, "%d", nCurNodeCharacters_Titles);
		label22 = gtk_label_new (szBuffer);
		gtk_widget_show (label22);
		gtk_table_attach (GTK_TABLE (table1), label22, 0, 1, 7, 8, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_misc_set_alignment (GTK_MISC (label22), 1, 0.5);

		sprintf(szBuffer, "%d", nCurNodeCharacters_Texts);
		label23 = gtk_label_new (szBuffer);
		gtk_widget_show (label23);
		gtk_table_attach (GTK_TABLE (table1), label23, 1, 2, 7, 8, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_misc_set_alignment (GTK_MISC (label23), 1, 0.5);

		sprintf(szBuffer, "%d", nCurNodeCharacters_Titles + nCurNodeCharacters_Texts);
		label24 = gtk_label_new (szBuffer);
		gtk_widget_show (label24);
		gtk_table_attach (GTK_TABLE (table1), label24, 2, 3, 7, 8, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_misc_set_alignment (GTK_MISC (label24), 1, 0.5);

		label25 = gtk_label_new (_("characters"));
		gtk_widget_show (label25);
		gtk_table_attach (GTK_TABLE (table1), label25, 3, 4, 7, 8, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_misc_set_alignment (GTK_MISC (label25), 1, 0.5);
	}

	gtk_dialog_add_button(GTK_DIALOG(msgbox), GTK_STOCK_OK, GTK_RESPONSE_OK);
	gtk_dialog_set_has_separator (GTK_DIALOG(msgbox), FALSE);
	gtk_dialog_run (GTK_DIALOG (msgbox));
	gtk_widget_destroy (msgbox);
}

void calc_word_count(const gchar *szText, int &nWords, int &nLines, int &nCharacters)
{
	//calc characters
	nCharacters = g_utf8_strlen(szText, -1);

	//calc lines
	if(nCharacters < 1)
		nLines = 0;
	else
	{
		nLines = 1;

		const gchar *szPos = szText;
		while(NULL != szPos)
		{
			if(*szPos == '\r' || *szPos == '\n')
				nLines++;

			const gchar *szStart = szPos;
			szPos = g_utf8_find_next_char(szPos, NULL);
			if(szStart == szPos)
				break;
		}
	}

	//calc words
	nWords = 0;
	bool bInsideWord = false;
	const gchar *szPos = szText;
	while(NULL != szPos)
	{
		gunichar nChar = g_utf8_get_char(szPos);
		if( g_unichar_isspace(nChar) ||
			g_unichar_ispunct(nChar) ||
			*szPos == '\r' ||
			*szPos == '\n' ||
			(!g_unichar_isdigit(nChar) && !g_unichar_isalpha(nChar) && (*szPos != '_')&& (*szPos != '-')))
		{
			if(bInsideWord)
				nWords ++;
			bInsideWord	= false;
		}
		else
			bInsideWord	= true;

		const gchar *szStart = szPos;
		szPos = g_utf8_find_next_char(szPos, NULL);
		if(szStart == szPos)
			break;
	}
}

void on_new_notecase1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	g_objCrashLog.AddMsg("ACTION: Start new app instance\n");

	//start new Notecase instance
	std::string strURL = GetAppPath();

	std::string strURLLocale;
	Utf8ToLocale(strURL.c_str(), strURLLocale);

#ifdef _WIN32
	ShellExecute(NULL, "open", strURLLocale.c_str(), "", "", SW_SHOW);
#else
	ExecuteFile(strURL.c_str(), "", "", NULL);
#endif
}

//calculate status bar info
std::string calc_node_info(int nIdx)
{
	std::string strMsg;

	if(nIdx > -1)
	{
		//calc child info
		DocumentIterator it(g_doc);
		int nID = g_doc.GetNodeByIdx(nIdx).m_nID;
		int nChildren	 = it.GetChildCount(nID);
		int nDescendants = it.GetChildCount(nID, true);

		if(nChildren > 0)
		{
			//how many of those have "finished" status
			int nDescendantsFinished = 0;
			int nChildrenFinished = 0;
			int nTotalNodes = g_doc.GetNodeCount();

			int i;
			for(i=0; i<nTotalNodes; i++){
				if( i != nIdx &&
					it.IsAncestorByIdx(nIdx, i) &&
					g_doc.GetNodeByIdx(i).m_bFinished)
				{
					nDescendantsFinished ++;
				}
			}
			for(i=0; i<nChildren; i++){
				int nChildIdx = it.GetChildIdx(nID, i);
				if(nChildIdx>=0 && g_doc.GetNodeByIdx(nChildIdx).m_bFinished)
					nChildrenFinished ++;
			}

			double nPercentFinished = 0;
			if(nDescendants > 0)
				nPercentFinished = 100 * nDescendantsFinished / (double)nDescendants;

			char szBuffer[2024];
		#ifdef HAVE_SNPRINTF
			snprintf(szBuffer, sizeof(szBuffer), _("Completed descendants: %5.1f%% (%d / %d) (children completed: %d / %d)"),
							nPercentFinished, nDescendantsFinished, nDescendants, nChildrenFinished, nChildren);
		#else
			sprintf(szBuffer, _("Completed descendants: %5.1f%% (%d / %d) (children completed: %d / %d)"),
							nPercentFinished, nDescendantsFinished, nDescendants, nChildrenFinished, nChildren);
		#endif
			strMsg += szBuffer;
		}
	}

	return strMsg;
}

void on_textview_populate_popup(GtkWidget *widget, GtkMenu *menu, void *user_data)
{
	//
	// modify context menu for text view widget
	//

	int nIdx = GetSelectedNodeIdx();
	if(nIdx >= 0){
		UpdateTextFromScreen();

		//get clicked area character offset based on screen coordinates
		gint x, y;
		gtk_widget_get_pointer(widget, &x, &y);
		GtkTextIter iter;
		gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(widget), &iter, x, y);
		int nPosOffset = gtk_text_iter_get_offset (&iter);

		//check if there is a picture below the cursor
		long nPicIdx = -1;
		int nPicCnt = g_doc.GetNodeByIdx(nIdx).m_lstPictures.size();
		for(int i=0; i<nPicCnt; i++){
			if(g_doc.GetNodeByIdx(nIdx).m_lstPictures[i].nOffset == nPosOffset){
				nPicIdx = i;
				break;
			}
		}
		if(nPicIdx >= 0){
			GtkWidget *pix1 = gtk_menu_item_new_with_mnemonic (_("Save Picture"));
			gtk_widget_show (pix1);
			gtk_container_add (GTK_CONTAINER (menu), pix1);
			g_signal_connect (GTK_OBJECT (pix1), "activate", G_CALLBACK(on_save_picture_activate), (gpointer)nPicIdx);
		}

		if(!g_doc.IsReadOnly())
		{
			//get selected text
			int nCursor = 0;
			int nSelection = 0;
			g_text.GetSelectionBounds(nCursor, nSelection);
			if(nCursor != nSelection)
			{
				//selection exists
				GtkWidget *link1 = gtk_menu_item_new_with_mnemonic (_("Link Wizard"));
				gtk_widget_show (link1);
				gtk_container_add (GTK_CONTAINER (menu), link1);
				g_signal_connect (GTK_OBJECT (link1),   "activate", G_CALLBACK(on_link_wizard_activate), NULL);
			}
		}

		//MAEMO only, copy format menu to context menu
	#ifdef _NOKIA_MAEMO
		GtkWidget *menuitem2 = gtk_menu_item_new_with_mnemonic (_("F_ormat"));
		gtk_widget_show (menuitem2);
		gtk_container_add (GTK_CONTAINER (menu), menuitem2);

		GtkWidget *menu2 = gtk_menu_new ();
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem2), menu2);

		GtkWidget *bold1 = gtk_image_menu_item_new_with_mnemonic (_("_Bold"));
		gtk_widget_show (bold1);
		gtk_container_add (GTK_CONTAINER (menu2), bold1);
		GdkPixbuf *pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&text_bold);
		GtkWidget *icon = gtk_image_new_from_pixbuf (pixbuf);
		gtk_widget_show (icon);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (bold1), icon);

		GtkWidget *italic1 = gtk_image_menu_item_new_with_mnemonic (_("_Italic"));
		gtk_widget_show (italic1);
		gtk_container_add (GTK_CONTAINER (menu2), italic1);
		pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&text_italic);
		icon = gtk_image_new_from_pixbuf (pixbuf);
		gtk_widget_show (icon);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (italic1), icon);

		GtkWidget *underline1 = gtk_image_menu_item_new_with_mnemonic (_("_Underline"));
		gtk_widget_show (underline1);
		gtk_container_add (GTK_CONTAINER (menu2), underline1);
		pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&text_underline);
		icon = gtk_image_new_from_pixbuf (pixbuf);
		gtk_widget_show (icon);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (underline1), icon);

		GtkWidget *strikethrough1 = gtk_image_menu_item_new_with_mnemonic (_("_Strikethrough"));
		gtk_widget_show (strikethrough1);
		gtk_container_add (GTK_CONTAINER (menu2), strikethrough1);
		pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&text_strikethrough);
		icon = gtk_image_new_from_pixbuf (pixbuf);
		gtk_widget_show (icon);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (strikethrough1), icon);

		GtkWidget *color1 = gtk_image_menu_item_new_with_mnemonic (_("_Text color"));
		gtk_widget_show (color1);
		gtk_container_add (GTK_CONTAINER (menu2), color1);
		icon = gtk_image_new_from_stock (GTK_STOCK_SELECT_COLOR, GTK_ICON_SIZE_MENU);
		gtk_widget_show (icon);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (color1), icon);

		GtkWidget *bkgcolor1 = gtk_image_menu_item_new_with_mnemonic (_("Text Background Color"));
		gtk_widget_show (bkgcolor1);
		gtk_container_add (GTK_CONTAINER (menu2), bkgcolor1);
		icon = gtk_image_new_from_stock (GTK_STOCK_SELECT_COLOR, GTK_ICON_SIZE_MENU);
		gtk_widget_show (icon);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (bkgcolor1), icon);

		GtkWidget *remove_format1 = gtk_image_menu_item_new_with_mnemonic (_("_Remove formatting"));
		gtk_widget_show (remove_format1);
		gtk_container_add (GTK_CONTAINER (menu2), remove_format1);
		icon = gtk_image_new_from_stock (GTK_STOCK_SELECT_COLOR, GTK_ICON_SIZE_MENU);
		gtk_widget_show (icon);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (remove_format1), icon);

		g_signal_connect (GTK_OBJECT (bold1),     "activate", G_CALLBACK(on_text_bold_activate), NULL);
		g_signal_connect (GTK_OBJECT (italic1),   "activate", G_CALLBACK(on_text_italic_activate), NULL);
		g_signal_connect (GTK_OBJECT (underline1),"activate", G_CALLBACK(on_text_underline_activate), NULL);
		g_signal_connect (GTK_OBJECT (strikethrough1),"activate", G_CALLBACK(on_text_strikethrough_activate), NULL);
		g_signal_connect (GTK_OBJECT (color1),    "activate", G_CALLBACK(on_text_color_activate), NULL);
		g_signal_connect (GTK_OBJECT (bkgcolor1),    "activate", G_CALLBACK(on_text_background_color_activate), NULL);
		g_signal_connect (GTK_OBJECT (remove_format1), "activate", G_CALLBACK(on_remove_format_activate), NULL);
	#endif
	}
}

void ExecuteLink(LinkInfo &link)
{
	g_objCrashLog.AddMsg("ACTION: Execute link\n");

	//exec link (strip "file://" prefix first if exists)
	std::string strURL = link.m_strTargetURL;
	int nTargetID = link.m_nTargetNodeID;

	if(strURL.size() > 0)
	{
		if(0 == strcasecmp(strURL.substr(0, strlen("file://")).c_str(), "file://"))
			strURL = strURL.substr(strlen("file://"));
	#ifdef _WIN32
		if(strURL.size() > 0 && strURL.at(0) == '/')
			strURL = strURL.substr(1);	// strip Unix style path prefix "/" from widows path
	#endif

		TRACE("Execute link URL=%s\n", strURL.c_str());

		std::string strURLLocale;
		Utf8ToLocale(strURL.c_str(), strURLLocale);

		//to prevent problems with opening links to local Notecase documents
		//when MIME is not correctly registered on the system, handle this case separately
		if( strURL.size() > 3 &&
			std::string::npos == strURL.find("://") &&
			(strURL.substr(strURL.size()-4) == ".ncd" ||
			 strURL.substr(strURL.size()-4) == ".nce"))
		{
			//start new Notecase instance with link path as an argument
			std::string strApp = GetAppPath();
			std::string strDir = GetParentDir(strApp.c_str());
			std::string strAppLocale;
			Utf8ToLocale(strApp.c_str(), strAppLocale);

		#ifdef _WIN32
			ShellExecute(NULL, "open", strAppLocale.c_str(), strURLLocale.c_str(), "", SW_SHOW);
		#else
			Execute(strAppLocale, strURL, strDir);
		#endif
		}
		else
		{
		#ifdef _WIN32
			ShellExecute(NULL, "open", strURLLocale.c_str(), "", "", SW_SHOW);
		#else
			ExecuteFile(strURL.c_str(), "", "", NULL);
		#endif
		}
	}
	else if (nTargetID >= 0)
	{
		TRACE("Execute link node ID=%d\n", nTargetID);
		g_doc.AssertValid();

		int nIdx = g_doc.GetIdxFromID(nTargetID);
		if(nIdx >= 0)
			SelectNodeByIdx(nIdx);
		else
			gtkMessageBox(_("Link target no longer exists."));
	}
}

void on_menu_sort_child_ascending (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	int nIdx = GetSelectedNodeIdx();
	do_sort_branch(nIdx, true);
}

void on_menu_sort_child_descending (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	int nIdx = GetSelectedNodeIdx();
	do_sort_branch(nIdx, false);
}

void on_menu_sort_root_ascending (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	do_sort_branch(-1, true);
}

void on_menu_sort_root_descending (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	do_sort_branch(-1, false);
}

void do_sort_branch(int nParentIdx, bool bAscending)
{
	g_objCrashLog.AddMsg("ACTION: sort branch\n");

	DocActionSort *pAction = new DocActionSort;
	pAction->SetDoc(g_doc);
	pAction->m_bAscending = bAscending;
	pAction->m_nNodeIdx = nParentIdx;

	//execute action
	pAction->Exec();

	//push document change into undo/redo manager
	g_undoManager.AddAction(pAction);
	UpdateUndoRedoMenus();

	g_doc.SetModified(true);
	RefreshMainTitle();
	restart_enc_unload_timer();
}

void on_insert_picture_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	int nIdx = GetSelectedNodeIdx();
	if(nIdx < 0){
		gtkMessageBox(_("No selected node!"));
		return;
	}

	g_objCrashLog.AddMsg("ACTION: insert picture\n");

	DocActionPix *pAction = new DocActionPix;
	pAction->SetDoc(g_doc);
	pAction->m_nNodeIdx = nIdx;

	//execute action
	pAction->Exec(true);

	//push document change into undo/redo manager
	g_undoManager.AddAction(pAction);
	UpdateUndoRedoMenus();

	g_doc.SetModified(true);
	RefreshMainTitle();
	restart_enc_unload_timer();
}

void on_text_bold_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	int nIdx = GetSelectedNodeIdx();
	if(nIdx < 0){
		gtkMessageBox(_("No selected node!"));
		return;
	}

	//do not allow text formatting action when no selection
	int nCursor, nSelection;
	g_text.GetSelectionBounds(nCursor, nSelection);
	if(nCursor == nSelection){
		g_nTextOffset = nCursor;
		//alternate from current state
		FmtInfoList &fmtList = g_doc.GetNodeByIdx(nIdx).m_lstTxtFmt;
		bool bExists = fmtList.IsRangeFormatted(nCursor, nCursor, FMT_BOLD_BEGIN);
		g_nBoldFmtTag = (bExists)? FMT_BOLD_END : FMT_BOLD_BEGIN;

		textview_selection_changed(NULL, NULL, NULL, NULL);	//will reset the toolbar button
		return;
	}
	else
		reset_format_override();

	g_objCrashLog.AddMsg("ACTION: add bold formatting\n");

	DocActionFmt *pAction = new DocActionFmt;
	pAction->SetDoc(g_doc);
	pAction->m_nNodeIdx		 = nIdx;
	pAction->m_nOperationTag = FMT_BOLD_BEGIN;

	//execute action
	pAction->Exec(true);

	//push document change into undo/redo manager
	g_undoManager.AddAction(pAction);
	UpdateUndoRedoMenus();

	g_doc.SetModified(true);
	RefreshMainTitle();
	restart_enc_unload_timer();
}

void on_text_italic_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	int nIdx = GetSelectedNodeIdx();
	if(nIdx < 0){
		gtkMessageBox(_("No selected node!"));
		return;
	}

	//do not allow text formatting action when no selection
	int nCursor, nSelection;
	g_text.GetSelectionBounds(nCursor, nSelection);
	if(nCursor == nSelection){
		g_nTextOffset = nCursor;
		//alternate from current state
		FmtInfoList &fmtList = g_doc.GetNodeByIdx(nIdx).m_lstTxtFmt;
		bool bExists = fmtList.IsRangeFormatted(nCursor, nCursor, FMT_ITALIC_BEGIN);
		g_nItalicFmtTag = (bExists)? FMT_ITALIC_END : FMT_ITALIC_BEGIN;

		textview_selection_changed(NULL, NULL, NULL, NULL);	//will reset the toolbar button
		return;
	}
	else
		reset_format_override();

	g_objCrashLog.AddMsg("ACTION: add italic formatting\n");

	DocActionFmt *pAction = new DocActionFmt;
	pAction->SetDoc(g_doc);
	pAction->m_nNodeIdx		 = nIdx;
	pAction->m_nOperationTag = FMT_ITALIC_BEGIN;

	//execute action
	pAction->Exec(true);

	//push document change into undo/redo manager
	g_undoManager.AddAction(pAction);
	UpdateUndoRedoMenus();

	g_doc.SetModified(true);
	RefreshMainTitle();
	restart_enc_unload_timer();
}

void on_text_underline_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	int nIdx = GetSelectedNodeIdx();
	if(nIdx < 0){
		gtkMessageBox(_("No selected node!"));
		return;
	}

	//do not allow text formatting action when no selection
	int nCursor, nSelection;
	g_text.GetSelectionBounds(nCursor, nSelection);
	if(nCursor == nSelection){
		g_nTextOffset = nCursor;
		//alternate from current state
		FmtInfoList &fmtList = g_doc.GetNodeByIdx(nIdx).m_lstTxtFmt;
		bool bExists = fmtList.IsRangeFormatted(nCursor, nCursor, FMT_UNDERLINE_BEGIN);
		g_nUnderlineFmtTag = (bExists)? FMT_UNDERLINE_END : FMT_UNDERLINE_BEGIN;

		textview_selection_changed(NULL, NULL, NULL, NULL);	//will reset the toolbar button
		return;
	}
	else
		reset_format_override();

	g_objCrashLog.AddMsg("ACTION: add underline formatting\n");

	DocActionFmt *pAction = new DocActionFmt;
	pAction->SetDoc(g_doc);
	pAction->m_nNodeIdx		 = nIdx;
	pAction->m_nOperationTag = FMT_UNDERLINE_BEGIN;

	//execute action
	pAction->Exec(true);

	//push document change into undo/redo manager
	g_undoManager.AddAction(pAction);
	UpdateUndoRedoMenus();

	g_doc.SetModified(true);
	RefreshMainTitle();
	restart_enc_unload_timer();
}

void on_text_strikethrough_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	int nIdx = GetSelectedNodeIdx();
	if(nIdx < 0){
		gtkMessageBox(_("No selected node!"));
		return;
	}

	//do not allow text formatting action when no selection
	int nCursor, nSelection;
	g_text.GetSelectionBounds(nCursor, nSelection);
	if(nCursor == nSelection){
		g_nTextOffset = nCursor;
		//alternate from current state
		FmtInfoList &fmtList = g_doc.GetNodeByIdx(nIdx).m_lstTxtFmt;
		bool bExists = fmtList.IsRangeFormatted(nCursor, nCursor, FMT_STRIKE_BEGIN);
		g_nStrikeFmtTag = (bExists)? FMT_STRIKE_END : FMT_STRIKE_BEGIN;

		textview_selection_changed(NULL, NULL, NULL, NULL);	//will reset the toolbar button
		return;
	}
	else
		reset_format_override();

	g_objCrashLog.AddMsg("ACTION: add strikethrough formatting\n");

	DocActionFmt *pAction = new DocActionFmt;
	pAction->SetDoc(g_doc);
	pAction->m_nNodeIdx		 = nIdx;
	pAction->m_nOperationTag = FMT_STRIKE_BEGIN;

	//execute action
	pAction->Exec(true);

	//push document change into undo/redo manager
	g_undoManager.AddAction(pAction);
	UpdateUndoRedoMenus();

	g_doc.SetModified(true);
	RefreshMainTitle();
	restart_enc_unload_timer();
}

void on_text_color_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	int nIdx = GetSelectedNodeIdx();
	if(nIdx < 0){
		gtkMessageBox(_("No selected node!"));
		return;
	}

	//do not allow text formatting action when no selection
	int nCursor, nSelection;
	g_text.GetSelectionBounds(nCursor, nSelection);
	if(nCursor == nSelection){
		textview_selection_changed(NULL, NULL, NULL, NULL);	//will reset the toolbar button
		return;
	}

	GdkEvent *ev = gtk_get_current_event();
	bool bThroughShortcut = (ev->type == GDK_KEY_PRESS);
	GdkModifierType state;
	bool bCtrl = (bThroughShortcut) ? false : (gtk_get_current_event_state (&state) && (state & GDK_CONTROL_MASK) != 0);
	TRACE("Ctrl clicked = %d\n", bCtrl);

	g_objCrashLog.AddMsg("ACTION: add color formatting\n");

	DocActionFmt *pAction = new DocActionFmt;
	pAction->SetDoc(g_doc);
	pAction->m_nNodeIdx		 = nIdx;
	pAction->m_nOperationTag = FMT_TXT_COLOR_BEGIN;
	pAction->m_bCtrlPressed  = bCtrl;

	//execute action
	pAction->Exec(true);

	//push document change into undo/redo manager
	g_undoManager.AddAction(pAction);
	UpdateUndoRedoMenus();

	g_doc.SetModified(true);
	RefreshMainTitle();
	restart_enc_unload_timer();
	textview_selection_changed(NULL, NULL, NULL, NULL);	//will reset the toolbar button
}

void on_remove_format_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	int nIdx = GetSelectedNodeIdx();
	if(nIdx < 0){
		gtkMessageBox(_("No selected node!"));
		return;
	}

	//do not allow text formatting action when no selection
	int nCursor, nSelection;
	g_text.GetSelectionBounds(nCursor, nSelection);
	if(nCursor == nSelection){
		textview_selection_changed(NULL, NULL, NULL, NULL);	//will reset the toolbar button
		return;
	}

	g_objCrashLog.AddMsg("ACTION: remove formatting\n");

	DocActionFmt *pAction = new DocActionFmt;
	pAction->SetDoc(g_doc);
	pAction->m_nNodeIdx		 = nIdx;
	pAction->m_nOperationTag = FMT_NONE;

	//execute action
	pAction->Exec(true);

	//push document change into undo/redo manager
	g_undoManager.AddAction(pAction);
	UpdateUndoRedoMenus();

	g_doc.SetModified(true);
	RefreshMainTitle();
	restart_enc_unload_timer();
}

void on_tree_row_collapsed (GtkTreeView *tree_view, GtkTreeIter *iter, GtkTreePath *path, gpointer user_data)
{
	int nIdx = NodeIdxFromPath(path);
	if(nIdx >= 0)
	{
		DocumentIterator it(g_doc);
		if(it.IsAncestorByIdx(nIdx, g_doc.m_nActiveNodeIdx))
		{
			//selected node is hidden when its ancestor was collapsed
			SelectNodeByIdx(nIdx);	// move selection to parent
			gtk_tree_view_collapse_row(tree_view, path);	//FIX: continue collapsing (skipped by Select... ???)
		}

		g_doc.GetNodeByIdx(nIdx).m_bExpanded = false; // store state
	}
}

void on_tree_row_expanded (GtkTreeView *tree_view, GtkTreeIter *iter, GtkTreePath *path, gpointer user_data)
{
	int nIdx = NodeIdxFromPath(path);
	if(nIdx >= 0)
	{
		g_doc.GetNodeByIdx(nIdx).m_bExpanded = true; // store state
	}
}

void on_save_picture_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	long nPixIdx = (long)user_data;

	//save pixbuf to file
	FileDialog dlg(false);
	dlg.SetTitle(_("Save picture"));
	dlg.AddFilter(_("PNG format (*.png)"), "*.png");
	dlg.AddFilter(_("JPEG format (*.jpg)"), "*.jpg");
	//TOFIX set default filter based on internal format (pix.bUsePNG? "png" : "jpeg")

	int nNode = GetSelectedNodeIdx();
	PixInfo &pix = g_doc.GetNodeByIdx(nNode).m_lstPictures[nPixIdx];
	dlg.SetFilename(pix.strName.c_str());

	if(dlg.DoModal())
	{
		dlg.ForceFormatExtension();
		const gchar *filename = dlg.GetFilename();
		dlg.Close();

		g_objCrashLog.AddMsg("ACTION: save picture\n");

		GdkPixbuf *pixbuf = pix.pixbuf;

		//calculate required format from extension
		bool bPNG = false;
		std::string strFmt = filename;
		std::string::size_type nExt = strFmt.rfind('.');
		if(nExt != std::string::npos)
			strFmt = strFmt.substr(nExt+1);
		if(0 == strcasecmp(strFmt.c_str(), "png"))
			bPNG = true;
		else
			strFmt = "jpeg";

	#if GTK_CHECK_VERSION(2,8,0) //new "compression" param TOFIX set proper version
		if(!gdk_pixbuf_save(pixbuf, filename, strFmt.c_str(), NULL, bPNG? "compression" : NULL,  bPNG? "9" : NULL, (char *)NULL))
	#else
		if(!gdk_pixbuf_save(pixbuf, filename, strFmt.c_str(), NULL, (char *)NULL))
	#endif
			gtkMessageBox(_("Failed to save picture to file!"));
	}
}

void textview_selection_changed(GtkTextBuffer *textbuffer, GtkTextIter *arg1, GtkTextMark *arg2, gpointer user_data)
{
	if(g_doc.m_nActiveNodeIdx<0)
		return;

	int nCursor, nSelection;
	g_text.GetSelectionBounds(nCursor, nSelection);
	int nSelLower = min(nCursor, nSelection);
	int nSelUpper = max(nCursor, nSelection);
	bool bFmtOverride = (nSelLower == nSelUpper && g_nTextOffset == nSelLower);

	FmtInfoList &fmtList = g_doc.GetNodeByIdx(g_doc.m_nActiveNodeIdx).m_lstTxtFmt;

	//refresh toolbar buttons toggle states
	bool bActive = fmtList.IsRangeFormatted(nSelLower, nSelUpper, FMT_BOLD_BEGIN);
	//adjust for current format override state
	if(bFmtOverride && g_nBoldFmtTag > -1)
		bActive = (g_nBoldFmtTag == FMT_BOLD_BEGIN);
	if(bActive != (0 < gtk_toggle_tool_button_get_active((GTK_TOGGLE_TOOL_BUTTON(g_wnd.m_pToolbarBold))))){
		g_signal_handlers_block_by_func(g_wnd.m_pToolbarBold, (void *)on_text_bold_activate, 0);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(g_wnd.m_pToolbarBold), bActive);
		g_signal_handlers_unblock_by_func(g_wnd.m_pToolbarBold, (void *)on_text_bold_activate, 0);
	}

	bActive = fmtList.IsRangeFormatted(nSelLower, nSelUpper, FMT_ITALIC_BEGIN);
	//adjust for current format override state
	if(bFmtOverride && g_nItalicFmtTag > -1)
		bActive = (g_nItalicFmtTag == FMT_ITALIC_BEGIN);
	if(bActive != (0 < gtk_toggle_tool_button_get_active((GTK_TOGGLE_TOOL_BUTTON(g_wnd.m_pToolbarItalic))))){
		g_signal_handlers_block_by_func(g_wnd.m_pToolbarItalic, (void *)on_text_italic_activate, 0);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(g_wnd.m_pToolbarItalic), bActive);
		g_signal_handlers_unblock_by_func(g_wnd.m_pToolbarItalic, (void *)on_text_italic_activate, 0);
	}

	bActive = fmtList.IsRangeFormatted(nSelLower, nSelUpper, FMT_UNDERLINE_BEGIN);
	//adjust for current format override state
	if(bFmtOverride && g_nUnderlineFmtTag > -1)
		bActive = (g_nUnderlineFmtTag == FMT_UNDERLINE_BEGIN);
	if(bActive != (0 < gtk_toggle_tool_button_get_active((GTK_TOGGLE_TOOL_BUTTON(g_wnd.m_pToolbarUnderline))))){
		g_signal_handlers_block_by_func(g_wnd.m_pToolbarUnderline, (void *)on_text_underline_activate, 0);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(g_wnd.m_pToolbarUnderline), bActive);
		g_signal_handlers_unblock_by_func(g_wnd.m_pToolbarUnderline, (void *)on_text_underline_activate, 0);
	}

	bActive = fmtList.IsRangeFormatted(nSelLower, nSelUpper, FMT_STRIKE_BEGIN);
	//adjust for current format override state
	if(bFmtOverride && g_nStrikeFmtTag > -1)
		bActive = (g_nStrikeFmtTag == FMT_STRIKE_BEGIN);
	if(bActive != (0 < gtk_toggle_tool_button_get_active((GTK_TOGGLE_TOOL_BUTTON(g_wnd.m_pToolbarStrike))))){
		g_signal_handlers_block_by_func(g_wnd.m_pToolbarStrike, (void *)on_text_strikethrough_activate, 0);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(g_wnd.m_pToolbarStrike), bActive);
		g_signal_handlers_unblock_by_func(g_wnd.m_pToolbarStrike, (void *)on_text_strikethrough_activate, 0);
	}

	bActive = fmtList.IsRangeFormatted(nSelLower, nSelUpper, FMT_TXT_COLOR_BEGIN);
	if(bActive != (0 < gtk_toggle_tool_button_get_active((GTK_TOGGLE_TOOL_BUTTON(g_wnd.m_pToolbarColor))))){
		g_signal_handlers_block_by_func(g_wnd.m_pToolbarColor, (void *)on_text_color_activate, 0);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(g_wnd.m_pToolbarColor), bActive);
		g_signal_handlers_unblock_by_func(g_wnd.m_pToolbarColor, (void *)on_text_color_activate, 0);
	}

	bActive = fmtList.IsRangeFormatted(nSelLower, nSelUpper, FMT_TXT_BKG_COLOR_BEGIN);
	if(bActive != (0 < gtk_toggle_tool_button_get_active((GTK_TOGGLE_TOOL_BUTTON(g_wnd.m_pToolbarBkgColor))))){
		g_signal_handlers_block_by_func(g_wnd.m_pToolbarBkgColor, (void *)on_text_background_color_activate, 0);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(g_wnd.m_pToolbarBkgColor), bActive);
		g_signal_handlers_unblock_by_func(g_wnd.m_pToolbarBkgColor, (void *)on_text_background_color_activate, 0);
	}
}

void on_node_attachments_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	int nIdx = GetSelectedNodeIdx();
	if(nIdx < 0){
		gtkMessageBox(_("No selected node!"));
		return;
	}

	g_objCrashLog.AddMsg("ACTION: start node attachments dialog\n");

	FileAttachmentDlg dlg;
	dlg.m_nNodeIdx = nIdx;
	dlg.Create();
	dlg.ShowModal();

	//refresh toggle button state
	bool bAttExist = (g_doc.GetNodeByIdx(nIdx).m_lstAttachments.size() > 0);
	g_signal_handlers_block_by_func(g_wnd.m_pToolbarAttachment, (void *)on_node_attachments_activate, 0);
	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(g_wnd.m_pToolbarAttachment), bAttExist);
	g_signal_handlers_unblock_by_func(g_wnd.m_pToolbarAttachment, (void *)on_node_attachments_activate, 0);
}

void on_menu_delete_finished_nodes (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	g_objCrashLog.AddMsg("ACTION: delete finished nodes\n");

	DocActionFinishDel *pAction = new DocActionFinishDel;
	pAction->SetDoc(g_doc);

	//execute action
	pAction->Exec(true);

	//push document change into undo/redo manager
	g_undoManager.AddAction(pAction);
	UpdateUndoRedoMenus();

	if(pAction->m_nDeletedCnt > 0)
		g_doc.SetModified(true);
	RefreshMainTitle();
	restart_enc_unload_timer();
}

void stop_enc_unload_timer()
{
	if(g_nEncUnloadTimer > 0)
		g_source_remove (g_nEncUnloadTimer);
	g_nEncUnloadTimer = 0;
}

void restart_enc_unload_timer()
{
	stop_enc_unload_timer();	//ensure timer is stopped

	//read ini settings
	bool bEncUnload = false;
	g_objIni.GetValue("Security", "UseEncUnload", bEncUnload);
	if(bEncUnload && g_doc.IsEncrypted() && g_doc.GetPath().size() > 0)
	{
		g_objCrashLog.AddMsg("ACTION: Restart enc. unload timer\n");

		int nEncUnloadTimer;
		g_objIni.GetValue("Security", "EncUnloadTimer", nEncUnloadTimer, 0);
		if(nEncUnloadTimer > 0)
			g_nEncUnloadTimer = g_timeout_add (nEncUnloadTimer*1000*60, enc_unload_timer, NULL);
	}
}

gboolean enc_unload_timer(gpointer data)
{
	TRACE("Starting enc. unload timer\n");

	//if current document is encrypted and on disk
	if(g_doc.IsEncrypted() && g_doc.GetPath().size() > 0){
		std::string strPath = g_doc.GetPath();

		//save file, then reload it -> the program should start the password dialog
		save_file(strPath.c_str(), false, false, g_doc.GetPassword());

		// clear the content (both screen and memory)
		g_tree.Clear(); //clear tree view
		g_text.Clear(); //clear edit view
		g_doc.Close();  //close the document
		g_text.SetEditable(false);	// no selected node
		set_title_bar("", false);
		g_doc.m_nActiveNodeIdx = -1;
		RefreshMainTitle();

		//allow password being entered up to 3 times
		int nResult = 0;
		int nTryCnt = 0;
		while(nTryCnt < 3){
			nResult = load_file(strPath.c_str());;
			nTryCnt ++;
			if(DOC_LOAD_WRONG_PASSWORD != nResult)
				break;
		}


		//TOFIX do we need to explicitly kill timer
	}

	return FALSE;
}

void on_view_shortcuts_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	ShortcutsListDlg dlg;
	dlg.Create();
	dlg.ShowModal();
}

int FormatCallback()
{
	int nRes = gtkMessageBox(_("Unknown format! Do you want to load it as text?"), GTK_BUTTONS_YES_NO);
	if(GTK_RESPONSE_YES == nRes)
		return FORMAT_TXT;

	return FORMAT_NONE;
}

void on_change_pass1_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}

	g_bMsgBox = true;
	PasswordDialog dlg(MODE_PASS_CHANGE);
	dlg.m_strOldPass = g_doc.GetPassword();

	//TOFIX set label
	//std::string strLabel = _("Please enter the password for file\n");
	//strLabel += szFile;
	//dlg.SetLabel(strLabel.c_str());

	gtk_grab_add( dlg.GetDialog() );	//block all other widgets in the app

	if(GTK_RESPONSE_OK == dlg.ShowModal())
	{
		//get result
		g_doc.SetPassword(dlg.GetPassword());
		g_doc.SetModified(false);
		RefreshMainTitle();

		gtkMessageBox(_("You'll need to save the document to commit the password change!"));
	}

	gtk_grab_remove(dlg.GetDialog());	//unblock
	dlg.Destroy();
	g_bMsgBox = false;
}

void on_text_background_color_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return;
	}
	int nIdx = GetSelectedNodeIdx();
	if(nIdx < 0){
		gtkMessageBox(_("No selected node!"));
		return;
	}

	//do not allow text formatting action when no selection
	int nCursor, nSelection;
	g_text.GetSelectionBounds(nCursor, nSelection);
	if(nCursor == nSelection){
		textview_selection_changed(NULL, NULL, NULL, NULL);	//will reset the toolbar button
		return;
	}

	GdkEvent *ev = gtk_get_current_event();
	bool bThroughShortcut = (ev->type == GDK_KEY_PRESS); //shortcut can legaly use "Ctrl"
	GdkModifierType state;
	bool bCtrl = (bThroughShortcut) ? false : (gtk_get_current_event_state (&state) && (state & GDK_CONTROL_MASK) != 0);
	TRACE("Ctrl clicked = %d\n", bCtrl);

	g_objCrashLog.AddMsg("ACTION: add bkg color formatting\n");

	DocActionFmt *pAction = new DocActionFmt;
	pAction->SetDoc(g_doc);
	pAction->m_nNodeIdx		 = nIdx;
	pAction->m_nOperationTag = FMT_TXT_BKG_COLOR_BEGIN;
	pAction->m_bCtrlPressed  = bCtrl;

	//execute action
	pAction->Exec(true);

	//push document change into undo/redo manager
	g_undoManager.AddAction(pAction);
	UpdateUndoRedoMenus();

	g_doc.SetModified(true);
	RefreshMainTitle();
	restart_enc_unload_timer();
	textview_selection_changed(NULL, NULL, NULL, NULL);	//will reset the toolbar button
}

void on_read_only_activate	(GtkMenuItem *menuitem, gpointer user_data)
{
	set_read_only(!get_read_only());
	restart_enc_unload_timer();
}

void set_read_only(bool bSet, bool bModify)
{
	GtkWidget *read_only1 = lookup_widget(window1, "read_only1");
	g_signal_handlers_block_by_func(GTK_CHECK_MENU_ITEM(read_only1), (void *)on_read_only_activate, 0);
	if(bSet)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(read_only1), TRUE);
	else
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(read_only1), FALSE);
	g_signal_handlers_unblock_by_func(GTK_CHECK_MENU_ITEM(read_only1), (void *)on_read_only_activate, 0);

	g_doc.SetReadOnly(bSet);

	int nIdx = GetSelectedNodeIdx();
	if(nIdx >= 0 && !g_doc.IsReadOnly())
		g_text.SetEditable(true);
	else
		g_text.SetEditable(false);

	//"change password" - set proper menu item state
	GtkWidget *change_pass1 = lookup_widget(window1, "change_pass1");
	if(g_doc.IsOpen() && g_doc.IsEncrypted() && !g_doc.IsReadOnly())
		gtk_widget_set_sensitive (change_pass1, TRUE);
	else
		gtk_widget_set_sensitive (change_pass1, FALSE);

	g_tree.EnableDragAndDrop(!g_doc.IsReadOnly());

	if(bModify){
		g_doc.SetModified(true);
		RefreshMainTitle();
	}
}

bool get_read_only()
{
	return g_doc.IsReadOnly();

	//GtkWidget *read_only1 = lookup_widget(window1, "read_only1");
	//ASSERT(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(read_only1)) == g_doc.IsReadOnly());
	//return (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(read_only1)) > 0);
}

void on_full_screen_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWidget *menufull1 = lookup_widget(window1, "menufull1");

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menufull1)))
		gtk_window_fullscreen(GTK_WINDOW(window1));
	else
		gtk_window_unfullscreen(GTK_WINDOW(window1));

	restart_enc_unload_timer();
}

gboolean is_format_provided(GdkDragContext *context, GdkAtom target)
{
	GList *targets = context->targets;
	while (targets && ((GdkAtom) targets->data != target))
		targets = targets->next;
	return targets != NULL;
}

gboolean textview_on_drop(GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint time, gpointer user_data)
{
	if(g_doc.IsReadOnly()){
		gtkMessageBox(_("Can not change read-only document!"));
		return FALSE;
	}
	ASSERT(widget == g_text.m_pWidget);

	TRACE("DnD: textview_on_drop\n");

	int nIdx = GetSelectedNodeIdx();
	if(nIdx < 0)
		return FALSE;

	gboolean  is_valid_drop_site = TRUE;

	// If the source offers a target
    if (is_valid_drop_site && context->targets)
    {
        GdkAtom target_type;
		if(is_format_provided(context, atomTextUriList))
			target_type = atomTextUriList;
		else if(is_format_provided(context, atomString))
			target_type = atomString;
		else
			target_type = GDK_POINTER_TO_ATOM(g_list_nth_data (context->targets, 0));	//choose first one

        // Request the data from the source, actual data will be received by 'drag-data-received' signal
        gtk_drag_get_data( widget, context, target_type, time);
    }
    else
	{
		is_valid_drop_site = FALSE;	// No target offered by source => error
	}

	return  is_valid_drop_site;
}

void drag_data_received_handl(GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *selection_data, guint target_type, guint time, gpointer data)
{
	gchar   *_sdata;

	gboolean dnd_success = FALSE;
	gboolean delete_selection_data = FALSE;

	TRACE("DnD: drag_data_received_handl\n");

	const gchar *name = gtk_widget_get_name (widget);
	TRACE("%s: drag_data_received_handl\n", name);

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

		// Check that we got the format we can use
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
				int nIdx = GetSelectedNodeIdx();

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

					g_doc.SetModified(true);
					RefreshMainTitle();

					//push document change into undo/redo manager
					DocActionAtt *pAction = new DocActionAtt;
					pAction->m_bAddOp   = true;
					pAction->m_info     = info;
					pAction->m_nNodeIdx = nIdx;
					pAction->m_nAttIdx  = g_doc.GetNodeByIdx(nIdx).m_lstAttachments.size()-1;

					g_undoManager.AddAction(pAction);
					UpdateUndoRedoMenus();
				}

				//refresh "node attachments" toggle button state
				bool bAttExist = (!g_doc.GetNodeByIdx(nIdx).m_lstAttachments.empty());
				g_signal_handlers_block_by_func(g_wnd.m_pToolbarAttachment, (void *)on_node_attachments_activate, 0);
				gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(g_wnd.m_pToolbarAttachment), bAttExist);
				g_signal_handlers_unblock_by_func(g_wnd.m_pToolbarAttachment, (void *)on_node_attachments_activate, 0);

				//show attachment dialog
				on_node_attachments_activate(NULL, NULL);
			}
		}
	}

	if (dnd_success == FALSE)
	{
		TRACE("DnD data transfer failed!\n");
	}

	gtk_drag_finish (context, dnd_success, delete_selection_data, time);
}

void edit_node_title(int nSelFrom, int nSelLen, const char *szTitle)
{
	//g_tree.EditLabel();	//trigger label edit mode
	int nIdx = GetSelectedNodeIdx();
	if(nIdx >= 0)
	{
		EditDlg dlg;
		dlg.SetValue(g_doc.GetNodeByIdx(nIdx).GetTitle().c_str());
		dlg.SetSelection(nSelFrom, nSelLen);
		if(NULL != szTitle)
			dlg.SetTitle(szTitle);

		if(GTK_RESPONSE_OK == dlg.ShowModal())
		{
			std::string strNewTitle = dlg.GetValue();
			if(strNewTitle.size() < 1){
				gtkMessageBox(_("Node title must not be empty!"));
				return;
			}

			//store new text into the document node
			std::string strOldTitle = g_doc.GetNodeByIdx(nIdx).GetTitle();
			SetNodeTitle(nIdx, strNewTitle.c_str());

			//update node title label
			set_title_bar(strNewTitle.c_str());

			//if the name was actually changed
			if(0 != strcmp(strOldTitle.c_str(), strNewTitle.c_str()))
			{
				//push document change into undo/redo manager
				DocAction *pAction = new DocAction;
				pAction->SetType(ACT_NODE_RENAME);
				pAction->SetDoc(g_doc);
				pAction->m_nNodeIndex = nIdx;  //TOFIX recursive index
				pAction->m_strNodeNameNew = strNewTitle;
				pAction->m_strNodeNameOld = strOldTitle;

				g_undoManager.AddAction(pAction);
				UpdateUndoRedoMenus();
				g_doc.SetModified(true);
			}

			RefreshMainTitle(); // updates modified doc state in window title bar
		}
	}
}

gboolean clear_formatting_timer(gpointer data)
{
	//only for paste operation
	if(!g_bIgnorePaste){
		//fix: remove all formatting from the pasted content
		//(else people would have false impression that formatting can be copied)
		//TOFIX -> only for edited part (not entire node) -> would be faster
		//NOTE: false - do not add pictures again, because they'll be doubled
		RefreshTextFormat(g_doc.GetNodeByIdx(g_doc.m_nActiveNodeIdx), false, g_nPasteOffset, g_nPasteOffset+g_nPasteLen);
	}

	g_bIgnorePaste = false;	//reset
	return FALSE;
}

gboolean date_time_paste_to_text_widget(GtkWidget *widget)
{
	//
	//get formatted date/time value
	//
	DateTimeDlg dlg;
	if(GTK_RESPONSE_OK != dlg.ShowModal())
		return FALSE;
	std::string strDateTime = dlg.GetValue();

	if(GTK_IS_ENTRY(widget))
	{
		gint nCursor, nSelection;
		g_object_get(G_OBJECT(widget), "cursor-position", &nCursor,
										"selection-bound", &nSelection, (char *)NULL);

		std::string strTxt = gtk_entry_get_text(GTK_ENTRY(widget));
		if(nCursor == nSelection)
		{
			strTxt.insert(nCursor, strDateTime);

			nCursor = nCursor + strDateTime.size();
			nSelection = nCursor - strDateTime.size();
		}
		else
		{
			int nLeft = min(nCursor, nSelection);
			int nSize = abs(nCursor - nSelection);

			//replace selection with date/time string
			strTxt.erase(nLeft, nSize);
			strTxt.insert(nLeft, strDateTime);

			nCursor = nLeft + strDateTime.size();
			nSelection = nCursor - strDateTime.size();
		}
		gtk_entry_set_text(GTK_ENTRY(widget), strTxt.c_str());
		gtk_entry_select_region(GTK_ENTRY(widget), nCursor, nCursor);
		gtk_widget_grab_focus (widget);
	}
	else if(GTK_IS_TEXT_VIEW(widget))
	{
		//get current text
		GtkWidget *entry1 = widget;

		GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(entry1));
		GtkTextIter start,end;
		gtk_text_buffer_get_start_iter(buffer, &start);
		gtk_text_buffer_get_end_iter(buffer, &end);
		gchar *pszData = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
		std::string strTxt = pszData;
		g_free(pszData);

		//calculate insertion point for the next text
		gint nCursor, nSelection;
		nCursor	= nSelection = gtk_text_iter_get_offset(&end);
		if(gtk_text_buffer_get_selection_bounds(buffer, &start, &end))
		{
			nCursor		= gtk_text_iter_get_offset(&start);
			nSelection	= gtk_text_iter_get_offset(&end);
		}

		if(nCursor == nSelection)
		{
			strTxt.insert(nCursor, strDateTime);

			nCursor = nCursor + strDateTime.size();
			nSelection = nCursor - strDateTime.size();
		}
		else
		{
			int nLeft = min(nCursor, nSelection);
			int nSize = abs(nCursor - nSelection);

			//replace selection with date/time string
			strTxt.erase(nLeft, nSize);
			strTxt.insert(nLeft, strDateTime);

			nCursor = nLeft + strDateTime.size();
			nSelection = nCursor - strDateTime.size();
		}
		gtk_text_buffer_set_text(buffer, strTxt.c_str(), -1);
		gtk_text_buffer_select_range(buffer, &start, &start);
		gtk_widget_grab_focus (widget);
		return TRUE;
	}

	return FALSE;
}

void on_lock_document_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	if(!g_doc.IsEncrypted()){
		gtkMessageBox(_("Document not encrypted!"));
		return;
	}
	if(g_doc.GetPath().empty()){
		gtkMessageBox(_("Document not saved!"));
		return;
	}

	//if the document is encrypted and on disk
	std::string strPath = g_doc.GetPath();

	//save file, then reload it -> the program should start the password dialog
	save_file(strPath.c_str(), false, false, g_doc.GetPassword());

	//TOFIX adapt to MDI, what if not current doc
	//clear the content (both screen and memory)
	g_tree.Clear(); //clear tree view
	g_text.Clear(); //clear edit view
	g_doc.Close();  //close the document

	g_text.SetEditable(false);	// no selected node
	set_title_bar("", false);
	g_doc.m_nActiveNodeIdx = -1;
	RefreshMainTitle();

	//allow password being entered up to 3 times
	int nResult = 0;
	int nTryCnt = 0;
	while(nTryCnt < 3){
		nResult = load_file(strPath.c_str());;
		nTryCnt ++;
		if(DOC_LOAD_WRONG_PASSWORD != nResult)
			break;
	}
}

void on_node_toggle_finished(GtkMenuItem *menuitem, gpointer user_data)
{
	int nIdx = GetSelectedNodeIdx();
	if(nIdx >= 0)
	{
		if(g_doc.IsReadOnly()){
			gtkMessageBox(_("Can not change read-only document!"));
			return;
		}
		g_objCrashLog.AddMsg("ACTION: Toggle node finished\n");

		//push document change into undo/redo manager
		DocActionFinish *pAction = new DocActionFinish;
		pAction->SetDoc(g_doc);
		pAction->m_nNodeIndex = nIdx;	//first node to be "finished"

		pAction->Exec(true);

		g_undoManager.AddAction(pAction);
		UpdateUndoRedoMenus();
	}
}

void on_menu_visit_website(GtkMenuItem *menuitem, gpointer user_data)
{
#ifdef _WIN32
	ShellExecute(NULL, "open", "http://notecase.sourceforge.net", "", "", SW_SHOW);
#else
	ExecuteFile("http://notecase.sourceforge.net", "", "", NULL);
#endif
}

gboolean textview_expose_event_cb (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
#if GTK_CHECK_VERSION(2,10,0) //new API
	int nIdx = GetSelectedNodeIdx();
	if(nIdx < 0)
	{
		cairo_t *cr = gdk_cairo_create (event->window);

		//draw gray bkg
		cairo_set_source_rgb (cr, 0.7, 0.7, 0.7);
		cairo_rectangle (cr, 0, 0, widget->allocation.width, widget->allocation.height);
		cairo_fill (cr);
		cairo_paint (cr);

		//draw info text with wrapping
		cairo_set_font_size(cr, 13);
		cairo_set_source_rgb (cr, 0, 0, 0);

		PangoLayout* layout= pango_cairo_create_layout(cr);
		pango_layout_set_text(layout,_("Insert new node or select an existing one to start typing."),-1);

		double nPosX = widget->allocation.width/4;	//approximate initial pos
		double nPosY = 25.0;
		if(nPosY >= widget->allocation.height)
			nPosY = 0.0;

		//restrict size
		pango_layout_set_width (layout, PANGO_SCALE * (int)(widget->allocation.width - nPosX));

		gint text_width, text_height;
		pango_layout_get_pixel_size (layout, &text_width, &text_height);

		nPosX = (widget->allocation.width-text_width)/2;
		cairo_move_to(cr, nPosX, nPosY);
		pango_cairo_show_layout(cr,layout);

		cairo_destroy (cr);
	}

	GtkWidget *margin1   = lookup_widget(window1, "margin1");
	gtk_widget_queue_draw(margin1);
	GtkWidget *margin2   = lookup_widget(window1, "margin2");
	gtk_widget_queue_draw(margin2);
#endif

	return FALSE;	// propagate
}


gboolean textview1_expose_event_cb (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
#if GTK_CHECK_VERSION(2,10,0) //new API
	//dynamically show/hide bottom margin widget based on whether the horz. scollbar is visible
	//(if visible, bottom margin would look strange - positioned below the scrollbar)
	GtkWidget* margin2 = lookup_widget(window1, "margin2");
	if(margin2 == widget)
	{
		GtkWidget* scrolledwindow2 = lookup_widget(window1, "scrolledwindow2");
		GtkWidget* horzscrollbar = gtk_scrolled_window_get_hscrollbar(GTK_SCROLLED_WINDOW(scrolledwindow2));
		if(GTK_WIDGET_VISIBLE(horzscrollbar))
			gtk_widget_hide(margin2);
	}
	else{
		GtkWidget* scrolledwindow2 = lookup_widget(window1, "scrolledwindow2");
		GtkWidget* horzscrollbar = gtk_scrolled_window_get_hscrollbar(GTK_SCROLLED_WINDOW(scrolledwindow2));
		if(!GTK_WIDGET_VISIBLE(horzscrollbar))
			gtk_widget_show(margin2);
	}

	cairo_t *cr = gdk_cairo_create (event->window);
	gdk_cairo_region (cr, event->region);
	cairo_clip (cr);

	//draw white bkg
	int nIdx = GetSelectedNodeIdx();
	if(nIdx < 0)
		cairo_set_source_rgb (cr, 0.7, 0.7, 0.7);
	else
		cairo_set_source_rgb (cr, g_rgbTextViewBkColor.red/65535.0, g_rgbTextViewBkColor.green/65535.0, g_rgbTextViewBkColor.blue/65535.0);

	cairo_rectangle (cr, 0, 0, widget->allocation.width, widget->allocation.height);
	cairo_fill (cr);
	cairo_paint (cr);

	cairo_destroy (cr);
#endif

	return TRUE;
}

void on_ontop1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	static gboolean setting = false;
	setting = !setting;
	gtk_window_set_keep_above(GTK_WINDOW(window1), setting);
}

int get_view_mode()
{
	if(GTK_WIDGET_VISIBLE(g_tree.m_pWidget))
	{
		if(GTK_WIDGET_VISIBLE(g_text.m_pWidget))
		{
			return 0;	//all visible
		}
		else
		{
			return 2; // tree only visible
		}
	}
	else if(GTK_WIDGET_VISIBLE(g_text.m_pWidget))
	{
		return 1; // text only visible
	}

	return 0;	//all visible (default)
}

gboolean indent_new_line_timer(gpointer data)
{
	//insert as many tabs at the start of the current line
	//as the number of tabs at the start of the previous line
	int nIdx = GetSelectedNodeIdx();

	UpdateTextFromScreen();

	gint nCursor, nSelection;
	g_text.GetSelectionBounds(nCursor, nSelection);
	gint nStart = min(nCursor, nSelection);

	GtkTextIter itLineStart;
	GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(g_text.m_pWidget));
	gtk_text_buffer_get_iter_at_offset(buffer, &itLineStart, nStart);
	//int nLine = gtk_text_iter_get_line_index(&itLineStart); //TOFIX invalid
	//int nA = gtk_text_iter_get_offset(&itLineStart);

	//if(nLine > 0)
	{
		GtkTextIter iterLinePrev = itLineStart;
		//gtk_text_iter_set_line(&iterLinePrev, nLine); //move to the start of current line
		gtk_text_iter_set_line_offset(&iterLinePrev, 0);
		//int nB = gtk_text_iter_get_offset(&iterLinePrev);
		if(!gtk_text_iter_backward_line (&iterLinePrev))
			return FALSE;

		int nOffset = gtk_text_iter_get_offset(&iterLinePrev);

		//get current word
		NoteNode &node = g_doc.GetNodeByIdx(nIdx);
		std::string strTxt = node.GetText();

		int nTabCount = 0;
		std::string strTabs;

		//convert from character offset to buffer offset
		const char *szBuf = strTxt.c_str();
		gchar* pszPos = g_utf8_offset_to_pointer(szBuf, nOffset);
		while(*pszPos && '\t' == *pszPos){
			nTabCount ++;
			strTabs += "\t";
			pszPos ++;
		}

		if(!strTabs.empty()){
			g_text.InsertText(nStart, strTabs.c_str());
			g_doc.GetNodeByIdx(nIdx).OnTxtInsert(nStart, strTabs.size(), strTabs.c_str());
		}
	}

	return FALSE;	// stop timer
}

#ifdef HAVE_GTKSOURCEVIEW
void on_show_line_numbers_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	GTK_TEXTVIEW_CLASS *textview =  (GTK_TEXTVIEW_CLASS *)g_text.m_pWidget;	
	gtk_source_view_set_show_line_numbers (textview, !gtk_source_view_get_show_line_numbers(textview));
}
#endif

void reset_format_override()
{
	g_nTextOffset = -1;
	g_nBoldFmtTag = -1;	//bold,un-bold,none
	g_nItalicFmtTag = -1;
	g_nUnderlineFmtTag = -1;
	g_nStrikeFmtTag = -1;
}
