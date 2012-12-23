////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Main application window implementation
////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include <string>
#include <gdk/gdkkeysyms.h>

#include "support.h"
#include "interface.h"
#include "MainWnd.h"
#include "TreeView.h"
#include "TextView.h"
#include "mru.h"
#include "callbacks.h"
#include "lib/debug.h"
#include "lib/NoteDocument.h"
#include "lib/IniFile.h"
#include "lib/FilePath.h"
#include "PortableTrayIcon.h"
#include "PasswordDialog.h"
#include "gui/GuiLanguage.h"
#include "ShortcutsList.h"
#include "FindReplaceInfo.h"

#ifdef _NOKIA_MAEMO
 #if _NOKIA_MAEMO == 1
  #include <hildon-widgets/hildon-program.h>
 #else
  #include <hildon/hildon-program.h>
 #endif
#endif

#include "../res/notecase.xpm"
#include "../res/icon_attachment.xpm"
#include "../res/icon_picture.xpm"
#include "../res/page_link.xpm"
#include "../res/text_bold.xpm"
#include "../res/text_italic.xpm"
#include "../res/text_underline.xpm"
#include "../res/text_strikethrough.xpm"
#include "../res/text_color.xpm"
#include "../res/text_bg_color.xpm"
#include "../res/text_no_format.xpm"
#include "../res/icon_datetime.xpm"
#include "../res/internal_lock.xpm"
#include "../res/key.xpm"

extern bool g_bBusyCursor;
void ShowBusyCursor();
void HideBusyCursor();

extern NoteDocument g_doc;
extern MRU g_objMRU;
extern GtkWidget *window1;
extern GtkWidget *g_menuitem5;
extern TreeView g_tree;
extern TextView g_text;
extern bool g_bMinimizeToTray;
extern bool g_bCloseToTray;
extern bool g_bCloseDlgOnTextEnter;
extern bool g_bAutoSaveOnClose;
extern bool g_bSkipMemOnlyAutosave;
extern bool g_bExportDataAsSeparateFile;
extern MainWindow g_wnd;
extern IniFile g_objIni;
extern GuiLanguage g_lang;
extern bool g_bTreeToTheRight;
extern bool g_bTreeToTheRightInitial;
extern GdkColor g_linkColor;
extern int g_nDefaultNodeIconType;
extern std::string g_strDefaultIconFile;
extern FindReplaceInfo g_objSearchInfo;

extern bool g_bInitialLoad;
extern bool g_bUpdateMRU;
extern bool g_bLoadEmbedded;
extern std::string g_strLoadDocumentFile;
extern std::string g_strLoadDocumentPass;
extern int g_nLoadDocumentNodeID;

extern ShortcutDef g_defTreeNodeNew;
extern ShortcutDef g_defTreeNodeNewChild;
extern ShortcutDef g_defTreeNodeDelete;
extern ShortcutDef g_defTreeNodeRename;
extern ShortcutDef g_defTreeNodeMoveUp;
extern ShortcutDef g_defTreeNodeMoveDown;
extern ShortcutDef g_defTreeNodeMoveLeft;
extern ShortcutDef g_defTreeNodeMoveRight;
extern ShortcutDef g_defTreeNodeProperties;
extern ShortcutDef g_defTreeNodeDelFinished;
extern ShortcutDef g_defTreeNodeToggleFinished;
extern ShortcutDef g_defTreeNodeSortChildrenAsc;
extern ShortcutDef g_defTreeNodeSortChildrenDsc;
extern ShortcutDef g_defTreeNodeSortRootAsc;
extern ShortcutDef g_defTreeNodeSortRootDsc;
extern ShortcutDef g_defTreeNodeExpandAll;
extern ShortcutDef g_defTreeNodeColapseAll;
extern ShortcutDef g_defTreeCopyBranchStruct;
extern ShortcutDef g_defTreePasteBranchRoot;

ShortcutsList g_shortcuts;
PortableTrayIcon g_tray;
int g_nTitleBarTextSize = 10 * PANGO_SCALE; //default size
bool g_bWrapTree=false;
bool g_bDrawTextviewExpose = false;

GdkColor g_rgbTextViewBkColor = { 0, 0xFFFF, 0xFFFF, 0xFFFF };	//white

void register_extensions_check();
static void create_menu (GtkWidget *window1, GtkWidget *menubar1, GtkAccelGroup *accel_group);
static gboolean window_state_event (GtkWidget *widget, GdkEventWindowState *event);
const char *PasswordCallback(const char *szFile, bool bSave);
static gboolean on_focus_event (GtkWidget *widget, GtkDirectionType arg1, gpointer user_data);
bool LocaleToUtf8(const char *szTxt, std::string &strResult);
static gint dlg_keyboard_handler (GtkWidget *widget, GdkEventKey *event, gpointer data);
void SetTextViewCaretColor(bool bBlack);
void RefreshTextViewCaretColor(GdkColor &BkColor);
gboolean initial_load_timer(gpointer data);
int  load_file_embedded(int nCurNodeIdx = -1);
int load_file(const char *filename, const char *szPassword, int nCurNodeIdx);
void on_tree_resize(GtkWidget *widget, GtkAllocation *allocation, gpointer user_data);
void update_tree_wrap();

MainWindow::MainWindow()
{
	m_pWidget = NULL;
	m_pToolbarBold = NULL;
	m_pToolbarItalic = NULL;
	m_pToolbarUnderline = NULL;
	m_pToolbarColor = NULL;
	m_pToolbarAttachment = NULL;
	m_pToolbarBkgColor = NULL;
	m_pToolbarUndo = NULL;
	m_pToolbarRedo = NULL;
}

MainWindow::~MainWindow()
{
}

void MainWindow::Create()
{
	g_shortcuts.Load();

	g_objIni.GetValue("Display", "DrawGrayTextView", g_bDrawTextviewExpose, 1);

	g_objIni.GetValue("Display", "TreeToTheRight", g_bTreeToTheRight);
	g_bTreeToTheRightInitial = g_bTreeToTheRight;

	//set global RTL support
	if(g_lang.IsCurLangRTL()) //RTL support
		gtk_widget_set_default_direction (GTK_TEXT_DIR_RTL);

	m_pWidget = create_main_win();
	OnCreate();

	//cache loaded shortcuts for faster lookup
	int nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_NODE_NEW, SH_CTX_TREE_WIDGET);
	if(nPos >= 0)	g_defTreeNodeNew = g_shortcuts[nPos];

	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_NODE_NEW_CHILD, SH_CTX_TREE_WIDGET);
	if(nPos >= 0)	g_defTreeNodeNewChild = g_shortcuts[nPos];

	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_NODE_DELETE, SH_CTX_TREE_WIDGET);
	if(nPos >= 0)	g_defTreeNodeDelete = g_shortcuts[nPos];

	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_NODE_RENAME, SH_CTX_TREE_WIDGET);
	if(nPos >= 0)	g_defTreeNodeRename = g_shortcuts[nPos];

	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_NODE_MOVE_UP, SH_CTX_TREE_WIDGET);
	if(nPos >= 0)	g_defTreeNodeMoveUp = g_shortcuts[nPos];

	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_NODE_MOVE_DOWN, SH_CTX_TREE_WIDGET);
	if(nPos >= 0)	g_defTreeNodeMoveDown = g_shortcuts[nPos];

	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_NODE_MOVE_LEFT, SH_CTX_TREE_WIDGET);
	if(nPos >= 0)	g_defTreeNodeMoveLeft = g_shortcuts[nPos];

	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_NODE_MOVE_RIGHT, SH_CTX_TREE_WIDGET);
	if(nPos >= 0)	g_defTreeNodeMoveRight = g_shortcuts[nPos];

	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_NODE_MOVE_RIGHT, SH_CTX_TREE_WIDGET);
	if(nPos >= 0)	g_defTreeNodeMoveRight = g_shortcuts[nPos];

	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_NODE_PROPERTIES, SH_CTX_TREE_WIDGET);
	if(nPos >= 0)	g_defTreeNodeProperties = g_shortcuts[nPos];

	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_NODE_DELETE_FINISHED, SH_CTX_TREE_WIDGET);
	if(nPos >= 0)	g_defTreeNodeDelFinished = g_shortcuts[nPos];

	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_NODE_TOGGLE_FINISHED, SH_CTX_TREE_WIDGET);
	if(nPos >= 0)	g_defTreeNodeToggleFinished = g_shortcuts[nPos];

	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_SORT_CHILDREN_ASC, SH_CTX_TREE_WIDGET);
	if(nPos >= 0)	g_defTreeNodeSortChildrenAsc = g_shortcuts[nPos];

	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_SORT_CHILDREN_DESC, SH_CTX_TREE_WIDGET);
	if(nPos >= 0)	g_defTreeNodeSortChildrenDsc = g_shortcuts[nPos];

	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_SORT_ROOT_ASC, SH_CTX_TREE_WIDGET);
	if(nPos >= 0)	g_defTreeNodeSortRootAsc = g_shortcuts[nPos];

	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_SORT_ROOT_DESC, SH_CTX_TREE_WIDGET);
	if(nPos >= 0)	g_defTreeNodeSortRootDsc = g_shortcuts[nPos];

	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_EXPAND_ALL, SH_CTX_TREE_WIDGET);
	if(nPos >= 0)	g_defTreeNodeExpandAll = g_shortcuts[nPos];

	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_COLAPSE_ALL, SH_CTX_TREE_WIDGET);
	if(nPos >= 0)	g_defTreeNodeColapseAll = g_shortcuts[nPos];

	nPos = g_shortcuts.FindByAction(NC_ACTION_EDIT_COPY_BR_STRUCT, SH_CTX_TREE_WIDGET);
	if(nPos >= 0)	g_defTreeCopyBranchStruct = g_shortcuts[nPos];

	nPos = g_shortcuts.FindByAction(NC_ACTION_EDIT_PASTE_BR_ROOT, SH_CTX_TREE_WIDGET);
	if(nPos >= 0)	g_defTreePasteBranchRoot = g_shortcuts[nPos];
}

void MainWindow::OnCreate()
{
	//check startup options
	window1 = g_wnd.m_pWidget;
	g_signal_connect (window1, "destroy", G_CALLBACK (on_quit1_activate), NULL);
	g_signal_connect (window1, "delete_event", G_CALLBACK (on_window_delete_event), NULL);
	g_signal_connect (window1, "key_press_event", G_CALLBACK (dlg_keyboard_handler), this);

	GtkWidget *treeview = lookup_widget(window1, "treeview1");
	GtkWidget *textview = lookup_widget(window1, "textview1");
	GtkWidget *label1   = lookup_widget(window1, "label1");

	//set global margin
	gtk_text_view_set_left_margin (GTK_TEXT_VIEW(textview), 5);
	gtk_text_view_set_right_margin(GTK_TEXT_VIEW(textview), 5);

	//set initial fonts
	std::string strFont1;
	g_objIni.GetValue("Display", "TreeFont", strFont1, "");
	if(!strFont1.empty()){
		PangoFontDescription *desc1 = pango_font_description_from_string(strFont1.c_str());
		gtk_widget_modify_font(treeview, desc1);
		pango_font_description_free(desc1);
	}

	std::string strFont2;
	g_objIni.GetValue("Display", "TextFont", strFont2, "");
	if(!strFont2.empty()){
		PangoFontDescription *desc1 = pango_font_description_from_string(strFont2.c_str());
		gtk_widget_modify_font(textview, desc1);
		//calculate text size for node title bar as 1.5 the size of other text
		g_nTitleBarTextSize = (pango_font_description_get_size(desc1) * 15)/10;
		pango_font_description_free(desc1);
	}

	//set font size for node title bar (1.5 times of node text font)
	PangoContext *context = gtk_widget_get_pango_context (label1);
	PangoFontDescription *desc2 = pango_context_get_font_description(context);
	PangoFontDescription *desc3 = pango_font_description_copy(desc2);
#if GTK_CHECK_VERSION(2,6,0) //minimal version for pango_font_description_set_absolute_size
	pango_font_description_set_absolute_size(desc3, g_nTitleBarTextSize);
#else
	pango_font_description_set_size (desc3, (8*g_nTitleBarTextSize)/10);	//TOFIX convert from pixels to fractions?
#endif
	gtk_widget_modify_font(label1, desc3);
	pango_font_description_free(desc3);
	//TRACE("Node title bar: Font size=%d\n", g_nTitleBarTextSize);

	bool bMaximize;
	if(g_objIni.GetValue("Startup", "Maximize", bMaximize))
		if(bMaximize)
			gtk_window_maximize(GTK_WINDOW(window1));

	//on demand check if we need to register file formats
	bool bRegisterFormats;
	if(g_objIni.GetValue("Startup", "RegisterFormats", bRegisterFormats))
		if(bRegisterFormats)
			register_extensions_check();

	//restore text wrapping option
	bool bWrapText;
	if(g_objIni.GetValue("Display", "WrapText", bWrapText))
	if(bWrapText)
		set_wrap_activated(true);

	//restore tree wrapping option
	g_objIni.GetValue("Display", "WrapTree", g_bWrapTree);

	//restore "tree lines" option
	bool bTreeLines;
	g_objIni.GetValue("Display", "ShowTreeLines", bTreeLines);
	g_tree.SetTreeLines(bTreeLines);

	//restore "toolbar visible" option
	bool bShowToolbar;
	g_objIni.GetValue("Display", "ShowToolBar", bShowToolbar, 1);
		if(bShowToolbar)
			set_show_toolbar(true);

	//restore "node title visible" option
	bool bShowTitle;
	if(g_objIni.GetValue("Display", "NodeTitleBar", bShowTitle))
		if(bShowTitle)
			set_show_node_title(true);

	//restore "status bar visible" option
	bool bShowStatBar;
	g_objIni.GetValue("Display", "ShowStatusBar", bShowStatBar, 1);
	if(bShowStatBar)	// default is true, change only if false
		set_show_status_bar(true);

	//restore last position
	bool bRestoreLastPos;
	g_objIni.GetValue("Startup", "RestoreLastPosition", bRestoreLastPos, 1);
	if(bRestoreLastPos)
	{
		gint nPosLeft, nPosTop, nPosWidth, nPosHeight, nPosDivider;
		nPosLeft = nPosTop = nPosWidth = nPosHeight = nPosDivider = 0;

		g_objIni.GetValue("Startup", "LastPos_Left", nPosLeft);
		g_objIni.GetValue("Startup", "LastPos_Top", nPosTop);
		g_objIni.GetValue("Startup", "LastPos_Width", nPosWidth);
		g_objIni.GetValue("Startup", "LastPos_Height", nPosHeight);
		g_objIni.GetValue("Startup", "LastPos_Divider", nPosDivider);

		//apply some common sense checks/fixes
		if(nPosWidth  <= 0) nPosWidth   = 500;
		if(nPosHeight <= 0) nPosHeight  = 350;
		if(nPosLeft   <= 0) nPosLeft    = 0;
		if(nPosTop    <= 0) nPosTop     = 0;
		if(nPosDivider<= 0) nPosDivider = 150;

		gtk_window_move(GTK_WINDOW(window1), nPosLeft, nPosTop);
		gtk_window_set_default_size(GTK_WINDOW(window1), nPosWidth, nPosHeight);
		gtk_window_resize(GTK_WINDOW(window1), nPosWidth, nPosHeight);
		GtkWidget *divider = lookup_widget(window1, "hbox1");
		gtk_paned_set_position(GTK_PANED(divider), nPosDivider);
	}
	else
	{
		gtk_window_set_default_size (GTK_WINDOW (window1), 500, 350);
		GtkWidget *divider = lookup_widget(window1, "hbox1");
		if(g_bTreeToTheRight)
			gtk_paned_set_position (GTK_PANED(divider), 350);
		else
			gtk_paned_set_position (GTK_PANED(divider), 150);
	}

	//restore view mode
	int nMode = 0;
	g_objIni.GetValue("Display", "ViewMode", nMode, 0);
	if(nMode > 0){
		on_view_text_only_mode (NULL, NULL);
		if(nMode > 1)
			on_view_text_only_mode (NULL, NULL);
	}

	//set custom GUI colors
	std::string strColor, strColor1;
	g_objIni.GetValue("Display", "TextColor", strColor, "");
	g_objIni.GetValue("Display", "BackgroundColor", strColor1, "");
	if(strColor.size() > 0)
	{
		GdkColor color;
		gdk_color_parse (strColor.c_str(), &color);
		gtk_widget_modify_text(GTK_WIDGET(textview), GTK_STATE_NORMAL, &color);
		gtk_widget_modify_text(GTK_WIDGET(treeview), GTK_STATE_NORMAL, &color);
	}
	if(strColor1.size() > 0)
	{
		GdkColor color1;
		gdk_color_parse (strColor1.c_str(), &color1);
		gtk_widget_modify_base(GTK_WIDGET(textview), GTK_STATE_NORMAL, &color1);
		gtk_widget_modify_base(GTK_WIDGET(textview), GTK_STATE_INSENSITIVE, &color1);
		gtk_widget_modify_base(GTK_WIDGET(treeview), GTK_STATE_NORMAL, &color1);
		RefreshTextViewCaretColor(color1);

		g_rgbTextViewBkColor = color1;

		//fix for GTK bug - tree view loses background color
		static const char cursor_color_rc[] =
			"style \"aaa\"\n"
			"{\n"
			"GtkTreeView::odd-row-color=\"#%04x%04x%04x\"\n"
			"GtkTreeView::even-row-color =\"#%04x%04x%04x\"\n"
			"}\n"
			"widget \"*.aaa\" style \"aaa\"";

		gchar *rc_temp = g_strdup_printf (cursor_color_rc,
									color1.red,
									color1.green,
									color1.blue,
									color1.red,
									color1.green,
									color1.blue);
		gtk_rc_parse_string (rc_temp);
		gtk_widget_set_name(treeview, "aaa");
		gtk_widget_reset_rc_styles (treeview);
		g_free (rc_temp);
  	}

	if(g_bDrawTextviewExpose){
		//code to register drawing over text view
		g_signal_connect (G_OBJECT (textview), "expose_event", G_CALLBACK(textview_expose_event_cb), NULL);
		gtk_widget_set_app_paintable (textview, TRUE);
		gtk_widget_set_redraw_on_allocate (textview, TRUE);
	}

	//init link color
	std::string strColor2;
	g_objIni.GetValue("Display", "LinkColor", strColor2, "#0000FF");
	GdkColor color2;
	gdk_color_parse (strColor2.c_str(), &color2);
	g_linkColor = color2;

	//init default node icon
	g_objIni.GetValue("Default", "NodeIconType", g_nDefaultNodeIconType, -2);
	g_objIni.GetValue("Security", "NodeIconValue", g_strDefaultIconFile, "");

	//
	g_objIni.GetValue("Search", "CaseSensitive", g_objSearchInfo.m_bFindSensitive, false);

	//set custom tab size
	int	nCustomTabSize = 0;
	g_objIni.GetValue("Display", "CustomTabSize", nCustomTabSize, 0);
	if(nCustomTabSize > 0)
	{
		int nWidthPixels = g_text.CalcTabWidthInPixels(nCustomTabSize);
		g_text.SetTabWidth(nWidthPixels);
	}

	//disable "New notecase" item when single instance is on
	bool bSingleInst;
	if(g_objIni.GetValue("Startup", "AllowSingleInstance", bSingleInst) && bSingleInst){
		GtkWidget *new_notecase1 = lookup_widget(window1, "new_notecase1");
		gtk_widget_set_sensitive(new_notecase1, FALSE);
	}

	if(g_bInitialLoad)
		g_timeout_add (0, initial_load_timer, NULL);

	g_tree.SetFocus();	// set initial focus

	gtk_widget_show (window1);

	//load global option
	g_objIni.GetValue("Display", "MinimizeToTray", g_bMinimizeToTray);
	g_objIni.GetValue("Display", "CloseToTray", g_bCloseToTray);
	g_objIni.GetValue("Default", "CloseDlgOnTxtEnter", g_bCloseDlgOnTextEnter, true);
	g_objIni.GetValue("Save", "AutoSaveOnClose", g_bAutoSaveOnClose);
	g_objIni.GetValue("Save",  "SkipMemOnlyAutoSave", g_bSkipMemOnlyAutosave, true);
	g_objIni.GetValue("Export", "ExportEmbeddedAsSeparateFile", g_bExportDataAsSeparateFile, 1);

	if(g_bWrapTree)
		g_tree.WrapTreeToSize(g_tree.m_pWidget->allocation.width);

	restart_autosave();
}

//TOFIX make class member RefreshTitle()
void RefreshMainTitle()
{
	if(!g_wnd.m_pWidget)	//no need if doing export from cmd line
		return;

	std::string strTitle;

	//basename of the document
	if(g_doc.GetPath().size()>0)
	{
		strTitle += GetBaseName(g_doc.GetPath().c_str());
		if(g_doc.IsModified())
			strTitle += "*";

		strTitle += " - ";

		strTitle += APP_NAME_STR;
		strTitle += " ";
		strTitle += APP_VER_STR;
		strTitle += " ";

		strTitle += "[";
		strTitle += g_doc.GetPath().c_str();
		strTitle += "]";
	}
	else
	{
		strTitle += APP_NAME_STR;
		strTitle += " ";
		strTitle += APP_VER_STR;
	}

	gtk_window_set_title (GTK_WINDOW (window1), strTitle.c_str());
}

GtkWidget* MainWindow::create_main_win ()
{
#ifdef _NOKIA_MAEMO
    HildonProgram *program;
    HildonWindow *window1;
#else
	GtkWidget *window1;
#endif

	GtkWidget *vbox1;
	GtkWidget *menubar1;
	GtkWidget *hbox1;
	GtkWidget *scrolledwindow1;
	GtkWidget *treeview1;
	GtkWidget *scrolledwindow2;
	GtkWidget *textview1;
	GtkWidget *statusbar1;
	GtkWidget *vbox2;
	GtkWidget *label1;
	GtkAccelGroup *accel_group;
	GtkWidget *toolbar1;

#ifdef _NOKIA_MAEMO
	// Create the hildon program and setup the title
    program = HILDON_PROGRAM(hildon_program_get_instance());
    g_set_application_name("Notecase");
#endif

	accel_group = gtk_accel_group_new ();

	//prepare version string
	std::string strApp(APP_NAME_STR);
	strApp += " ";
	strApp += APP_VER_STR;

#ifdef _NOKIA_MAEMO
    // Create HildonWindow and set it to HildonProgram
    window1 = HILDON_WINDOW(hildon_window_new());
    hildon_program_add_window(program, window1);
#else
	window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window1), strApp.c_str());
#endif

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (window1), vbox1);

#ifdef _NOKIA_MAEMO
	menubar1 = NULL;
#else
	menubar1 = gtk_menu_bar_new ();
	gtk_widget_show (menubar1);
	gtk_box_pack_start (GTK_BOX (vbox1), menubar1, FALSE, FALSE, 0);
#endif

	create_menu(GTK_WIDGET(window1), menubar1, accel_group);

	// create toolbar
	toolbar1 = create_toolbar();

#ifdef _NOKIA_MAEMO
	hildon_window_add_toolbar(window1, GTK_TOOLBAR(toolbar1));
#else
	gtk_box_pack_start (GTK_BOX (vbox1), toolbar1, FALSE, FALSE, 0);
#endif

	//
	hbox1 = gtk_hpaned_new ();
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);
	gtk_paned_set_position (GTK_PANED (hbox1), 0);

	GtkWidget *frameLeft = gtk_frame_new (NULL);
	GtkWidget *frameRight = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frameLeft), GTK_SHADOW_IN);
	gtk_frame_set_shadow_type (GTK_FRAME (frameRight), GTK_SHADOW_IN);
	gtk_widget_show (frameLeft);
	gtk_widget_show (frameRight);

	scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindow1);

	if(g_lang.IsCurLangRTL() || g_bTreeToTheRight)	//RTL support
		gtk_container_add( GTK_CONTAINER(frameRight), scrolledwindow1 );
	else
		gtk_container_add( GTK_CONTAINER(frameLeft), scrolledwindow1 );

	gtk_paned_pack1(GTK_PANED(hbox1), frameLeft, FALSE, TRUE);

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	//create tree view
	g_tree.Create();
	treeview1 = g_tree.m_pWidget;
	gtk_container_add (GTK_CONTAINER (scrolledwindow1), treeview1);

	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox2);
	gtk_container_set_border_width (GTK_CONTAINER (vbox2), 2);

	if(g_lang.IsCurLangRTL() || g_bTreeToTheRight) //RTL support
		gtk_container_add( GTK_CONTAINER(frameLeft), vbox2 );
	else
		gtk_container_add( GTK_CONTAINER(frameRight), vbox2 );

	gtk_paned_pack2 (GTK_PANED (hbox1), frameRight, TRUE, TRUE);

	//create text node label
	label1 = gtk_label_new ("");
	gtk_label_set_selectable(GTK_LABEL(label1), TRUE);
	gtk_widget_set_size_request(label1, 300, -1);
	gtk_misc_set_alignment(GTK_MISC(label1), 0.0f, 0.7f);  // horizontal: left aligned, vertically centered
	gtk_misc_set_padding(GTK_MISC(label1), 0, 2);
	gtk_box_pack_start (GTK_BOX (vbox2), label1, FALSE, FALSE, 0);

	//hack trying to fake top margin for text view
	GtkWidget *margin1 = NULL;
	if(g_bDrawTextviewExpose){
		margin1 = gtk_label_new ("");
		gtk_widget_show (margin1);
		gtk_widget_set_size_request(margin1, -1, 5);
		gtk_box_pack_start (GTK_BOX (vbox2), margin1, FALSE, FALSE, 0);
		gtk_widget_set_app_paintable (margin1, TRUE);
		gtk_widget_set_redraw_on_allocate   (margin1, TRUE);
		g_signal_connect (G_OBJECT (margin1), "expose_event", G_CALLBACK(textview1_expose_event_cb), NULL);
	}

	scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindow2);
	gtk_box_pack_start (GTK_BOX (vbox2), scrolledwindow2, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	//hack trying to fake bottom margin for text view
	GtkWidget *margin2 = NULL;
	if(g_bDrawTextviewExpose){
		margin2 = gtk_label_new ("");
		gtk_widget_show (margin2);
		gtk_widget_set_size_request(margin2, -1, 5);
		gtk_widget_set_app_paintable (margin1, TRUE);
		gtk_widget_set_redraw_on_allocate (margin2, TRUE);
		gtk_box_pack_start (GTK_BOX (vbox2), margin2, FALSE, FALSE, 0);
		g_signal_connect (G_OBJECT (margin2), "expose_event", G_CALLBACK(textview1_expose_event_cb), NULL);
	}

	//create text view
	g_text.Create();
	g_text.SetEditable(false);	// no selected node
	textview1 = g_text.m_pWidget;
	gtk_container_add (GTK_CONTAINER (scrolledwindow2), textview1);

	statusbar1 = gtk_statusbar_new ();
	//gtk_widget_show (statusbar1);
	gtk_box_pack_start (GTK_BOX (vbox1), statusbar1, FALSE, FALSE, 0);

	g_signal_connect (GTK_OBJECT (window1), "window_state_event", G_CALLBACK(window_state_event),  NULL);
	g_signal_connect (GTK_OBJECT (scrolledwindow1), "size-allocate", G_CALLBACK(on_tree_resize),  NULL);

	// Store pointers to all widgets, for use by lookup_widget()
	GLADE_HOOKUP_OBJECT_NO_REF (window1, window1, "window1");
	GLADE_HOOKUP_OBJECT (window1, vbox1, "vbox1");
#ifndef _NOKIA_MAEMO
	GLADE_HOOKUP_OBJECT (window1, menubar1, "menubar1");
#endif
	GLADE_HOOKUP_OBJECT (window1, hbox1, "hbox1");
	GLADE_HOOKUP_OBJECT (window1, scrolledwindow1, "scrolledwindow1");
	GLADE_HOOKUP_OBJECT (window1, treeview1, "treeview1");
	GLADE_HOOKUP_OBJECT (window1, scrolledwindow2, "scrolledwindow2");
	GLADE_HOOKUP_OBJECT (window1, textview1, "textview1");
	GLADE_HOOKUP_OBJECT (window1, statusbar1, "statusbar1");
	GLADE_HOOKUP_OBJECT (window1, vbox2, "vbox2");
	GLADE_HOOKUP_OBJECT (window1, label1, "label1");
	GLADE_HOOKUP_OBJECT (window1, toolbar1, "toolbar1");
	if(g_bDrawTextviewExpose){
		GLADE_HOOKUP_OBJECT (window1, margin1, "margin1");
		GLADE_HOOKUP_OBJECT (window1, margin2, "margin2");
	}

	gtk_window_add_accel_group (GTK_WINDOW (window1), accel_group);

	g_doc.SetPassCallback(PasswordCallback);

#ifdef _WIN32
#else
	//set window/application icon
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&notecase_xpm);
	gtk_window_set_icon(GTK_WINDOW(window1), pixbuf);
	g_object_unref(pixbuf);
#endif

	//modify context menu for text view widget
	g_signal_connect(G_OBJECT(textview1), "populate-popup", G_CALLBACK(on_textview_populate_popup), NULL);

#ifdef _NOKIA_MAEMO
	//first one is required for maemo
	gtk_widget_show_all(GTK_WIDGET(window1));
	gtk_widget_hide_all(toolbar1);	// hidden on startup
	gtk_widget_hide(statusbar1);	// hidden on startup
	gtk_widget_hide(label1);		// hidden on startup (node title bar)
#endif

	return GTK_WIDGET(window1);
}

GtkWidget* MainWindow::create_toolbar()
{
	GtkWidget *toolitem1;
	GtkWidget *button3;
	GtkIconSize tmp_toolbar_icon_size;

	GtkWidget *toolbar1 = gtk_toolbar_new ();
	//gtk_widget_show (toolbar1);  //initialy invisible (set by .ini value)

	gtk_toolbar_set_style (GTK_TOOLBAR (toolbar1), GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_icon_size (GTK_TOOLBAR (toolbar1), GTK_ICON_SIZE_SMALL_TOOLBAR);
	tmp_toolbar_icon_size = gtk_toolbar_get_icon_size (GTK_TOOLBAR (toolbar1));

	g_signal_connect (GTK_OBJECT (toolbar1), "focus", G_CALLBACK(on_focus_event), NULL);

	gtk_toolbar_set_tooltips (GTK_TOOLBAR (toolbar1), TRUE );
	GtkTooltips *tips = gtk_tooltips_new ();

	button3 = gtk_image_new_from_stock (GTK_STOCK_NEW, tmp_toolbar_icon_size);
	gtk_widget_show (button3);

	toolitem1 = (GtkWidget*) gtk_tool_button_new (button3, _("New"));
	gtk_widget_show (toolitem1);
	gtk_container_add (GTK_CONTAINER (toolbar1), toolitem1);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolitem1), tips, _("New"), NULL);
	g_signal_connect (GTK_OBJECT (toolitem1), "clicked", G_CALLBACK(on_new1_activate), NULL);


	button3 = gtk_image_new_from_stock (GTK_STOCK_OPEN, tmp_toolbar_icon_size);
	gtk_widget_show (button3);

	toolitem1 = (GtkWidget*) gtk_tool_button_new (button3, _("Open"));
	gtk_widget_show (toolitem1);
	gtk_container_add (GTK_CONTAINER (toolbar1), toolitem1);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolitem1), tips, _("Open"), NULL);
	g_signal_connect (GTK_OBJECT (toolitem1), "clicked", G_CALLBACK(on_open1_activate), NULL);


	button3 = gtk_image_new_from_stock (GTK_STOCK_SAVE, tmp_toolbar_icon_size);
	gtk_widget_show (button3);

	toolitem1 = (GtkWidget*) gtk_tool_button_new (button3, _("Save"));
	gtk_widget_show (toolitem1);
	gtk_container_add (GTK_CONTAINER (toolbar1), toolitem1);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolitem1), tips, _("Save"), NULL);
	g_signal_connect (GTK_OBJECT (toolitem1), "clicked", G_CALLBACK(on_save1_activate), NULL);

#ifndef _NOKIA_MAEMO
	button3 = gtk_image_new_from_stock (GTK_STOCK_SAVE_AS, tmp_toolbar_icon_size);
	gtk_widget_show (button3);

	toolitem1 = (GtkWidget*) gtk_tool_button_new (button3, _("Save As"));
	gtk_widget_show (toolitem1);
	gtk_container_add (GTK_CONTAINER (toolbar1), toolitem1);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolitem1), tips, _("Save As"), NULL);
	g_signal_connect (GTK_OBJECT (toolitem1), "clicked", G_CALLBACK(on_save_as1_activate), NULL);

	//append space
	toolitem1 = (GtkWidget*) gtk_separator_tool_item_new();
	gtk_widget_show (toolitem1);
	gtk_container_add (GTK_CONTAINER (toolbar1), toolitem1);


	button3 = gtk_image_new_from_stock (GTK_STOCK_UNDO, tmp_toolbar_icon_size);
	gtk_widget_show (button3);

	toolitem1 = (GtkWidget*) gtk_tool_button_new (button3, _("Undo"));
	gtk_widget_show (toolitem1);
	gtk_container_add (GTK_CONTAINER (toolbar1), toolitem1);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolitem1), tips, _("Undo"), NULL);
	g_signal_connect (GTK_OBJECT (toolitem1), "clicked", G_CALLBACK(on_undo_activate), NULL);
	gtk_widget_set_sensitive(toolitem1, FALSE);
	m_pToolbarUndo = toolitem1;

	button3 = gtk_image_new_from_stock (GTK_STOCK_REDO, tmp_toolbar_icon_size);
	gtk_widget_show (button3);

	toolitem1 = (GtkWidget*) gtk_tool_button_new (button3, _("Redo"));
	gtk_widget_show (toolitem1);
	gtk_container_add (GTK_CONTAINER (toolbar1), toolitem1);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolitem1), tips, _("Redo"), NULL);
	g_signal_connect (GTK_OBJECT (toolitem1), "clicked", G_CALLBACK(on_redo_activate), NULL);
	gtk_widget_set_sensitive(toolitem1, FALSE);
	m_pToolbarRedo = toolitem1;

	button3 = gtk_image_new_from_stock (GTK_STOCK_CUT, tmp_toolbar_icon_size);
	gtk_widget_show (button3);

	toolitem1 = (GtkWidget*) gtk_tool_button_new (button3, _("Cut"));
	gtk_widget_show (toolitem1);
	gtk_container_add (GTK_CONTAINER (toolbar1), toolitem1);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolitem1), tips, _("Cut"), NULL);
	g_signal_connect (GTK_OBJECT (toolitem1), "clicked", G_CALLBACK(on_cut1_activate), NULL);


	button3 = gtk_image_new_from_stock (GTK_STOCK_COPY, tmp_toolbar_icon_size);
	gtk_widget_show (button3);

	toolitem1 = (GtkWidget*) gtk_tool_button_new (button3, _("Copy"));
	gtk_widget_show (toolitem1);
	gtk_container_add (GTK_CONTAINER (toolbar1), toolitem1);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolitem1), tips, _("Copy"), NULL);
	g_signal_connect (GTK_OBJECT (toolitem1), "clicked", G_CALLBACK(on_copy1_activate), NULL);


	button3 = gtk_image_new_from_stock (GTK_STOCK_PASTE, tmp_toolbar_icon_size);
	gtk_widget_show (button3);

	toolitem1 = (GtkWidget*) gtk_tool_button_new (button3, _("Paste"));
	gtk_widget_show (toolitem1);
	gtk_container_add (GTK_CONTAINER (toolbar1), toolitem1);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolitem1), tips, _("Paste"), NULL);
	g_signal_connect (GTK_OBJECT (toolitem1), "clicked", G_CALLBACK(on_paste1_activate), NULL);


	button3 = gtk_image_new_from_stock (GTK_STOCK_DELETE, tmp_toolbar_icon_size);
	gtk_widget_show (button3);

	toolitem1 = (GtkWidget*) gtk_tool_button_new (button3, _("Delete"));
	gtk_widget_show (toolitem1);
	gtk_container_add (GTK_CONTAINER (toolbar1), toolitem1);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolitem1), tips, _("Delete"), NULL);
	g_signal_connect (GTK_OBJECT (toolitem1), "clicked", G_CALLBACK(on_delete1_activate), NULL);
#endif

//#if 1
#ifdef _NOKIA_MAEMO
	//insert node
	button3 = gtk_image_new_from_stock (GTK_STOCK_ADD, tmp_toolbar_icon_size);
	gtk_widget_show (button3);

	toolitem1 = (GtkWidget*) gtk_tool_button_new (button3, _("Insert Node"));
	gtk_widget_show (toolitem1);
	gtk_container_add (GTK_CONTAINER (toolbar1), toolitem1);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolitem1), tips, _("Insert Node"), NULL);
	g_signal_connect (GTK_OBJECT (toolitem1), "clicked", G_CALLBACK(on_menu_insert_node), NULL);

	//move node left
	button3 = gtk_image_new_from_stock (GTK_STOCK_GO_BACK, tmp_toolbar_icon_size);
	gtk_widget_show (button3);

	toolitem1 = (GtkWidget*) gtk_tool_button_new (button3, _("Move Node Left"));
	gtk_widget_show (toolitem1);
	gtk_container_add (GTK_CONTAINER (toolbar1), toolitem1);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolitem1), tips, _("Move Node Left"), NULL);
	g_signal_connect (GTK_OBJECT (toolitem1), "clicked", G_CALLBACK(on_menu_move_left), NULL);

	//move node right
	button3 = gtk_image_new_from_stock (GTK_STOCK_GO_FORWARD, tmp_toolbar_icon_size);
	gtk_widget_show (button3);

	toolitem1 = (GtkWidget*) gtk_tool_button_new (button3, _("Move Node Right"));
	gtk_widget_show (toolitem1);
	gtk_container_add (GTK_CONTAINER (toolbar1), toolitem1);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolitem1), tips, _("Move Node Right"), NULL);
	g_signal_connect (GTK_OBJECT (toolitem1), "clicked", G_CALLBACK(on_menu_move_right), NULL);
#endif

	//append space
	toolitem1 = (GtkWidget*) gtk_separator_tool_item_new();
	gtk_widget_show (toolitem1);
	gtk_container_add (GTK_CONTAINER (toolbar1), toolitem1);


	// link wizard button
	GdkPixbuf* pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&page_link);
	button3 = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show (button3);

	toolitem1 = (GtkWidget*) gtk_tool_button_new (button3, _("Link wizard"));
	gtk_widget_show (toolitem1);
	gtk_container_add (GTK_CONTAINER (toolbar1), toolitem1);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolitem1), tips, _("Link wizard"), NULL);
	g_signal_connect (GTK_OBJECT (toolitem1), "clicked", G_CALLBACK(on_link_wizard_activate), NULL);

	// insert image button
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&icon_picture);
	button3 = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show (button3);

	toolitem1 = (GtkWidget*) gtk_tool_button_new (button3, _("Insert Picture"));
	gtk_widget_show (toolitem1);
	gtk_container_add (GTK_CONTAINER (toolbar1), toolitem1);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolitem1), tips, _("Insert Picture"), NULL);
	g_signal_connect (GTK_OBJECT (toolitem1), "clicked", G_CALLBACK(on_insert_picture_activate), NULL);

	// insert image button
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&icon_attachment);
	button3 = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show (button3);

	toolitem1 = (GtkWidget*) gtk_toggle_tool_button_new ();
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(toolitem1), _("Node Attachments"));
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(toolitem1), button3);

	gtk_widget_show (toolitem1);
	gtk_container_add (GTK_CONTAINER (toolbar1), toolitem1);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolitem1), tips, _("Node Attachments"), NULL);
	g_signal_connect (GTK_OBJECT (toolitem1), "clicked", G_CALLBACK(on_node_attachments_activate), NULL);

	GLADE_HOOKUP_OBJECT (toolbar1, toolitem1, "toolbar_attachments");
	m_pToolbarAttachment = toolitem1;

	// insert bold button
	//button3 = gtk_image_new_from_stock (GTK_STOCK_BOLD, tmp_toolbar_icon_size);
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&text_bold);
	button3 = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show (button3);

	toolitem1 = (GtkWidget*) gtk_toggle_tool_button_new ();
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(toolitem1), _("Bold"));
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(toolitem1), button3);

	gtk_widget_show (toolitem1);
	gtk_container_add (GTK_CONTAINER (toolbar1), toolitem1);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolitem1), tips, _("Bold"), NULL);
	g_signal_connect (GTK_OBJECT (toolitem1), "clicked", G_CALLBACK(on_text_bold_activate), NULL);

	GLADE_HOOKUP_OBJECT (toolbar1, toolitem1, "toolbar_bold");
	m_pToolbarBold = toolitem1;

	// insert italic button
	//button3 = gtk_image_new_from_stock (GTK_STOCK_ITALIC, tmp_toolbar_icon_size);
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&text_italic);
	button3 = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show (button3);

	toolitem1 = (GtkWidget*) gtk_toggle_tool_button_new  ();
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(toolitem1), _("Italic"));
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(toolitem1), button3);

	gtk_widget_show (toolitem1);
	gtk_container_add (GTK_CONTAINER (toolbar1), toolitem1);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolitem1), tips, _("Italic"), NULL);
	g_signal_connect (GTK_OBJECT (toolitem1), "clicked", G_CALLBACK(on_text_italic_activate), NULL);

	GLADE_HOOKUP_OBJECT (toolbar1, toolitem1, "toolbar_italic");
	m_pToolbarItalic = toolitem1;

	// insert underline button
	//button3 = gtk_image_new_from_stock (GTK_STOCK_UNDERLINE, tmp_toolbar_icon_size);
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&text_underline);
	button3 = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show (button3);

	toolitem1 = (GtkWidget*) gtk_toggle_tool_button_new  ();
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(toolitem1), _("Underline"));
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(toolitem1), button3);

	gtk_widget_show (toolitem1);
	gtk_container_add (GTK_CONTAINER (toolbar1), toolitem1);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolitem1), tips, _("Underline"), NULL);
	g_signal_connect (GTK_OBJECT (toolitem1), "clicked", G_CALLBACK(on_text_underline_activate), NULL);

	GLADE_HOOKUP_OBJECT (toolbar1, toolitem1, "toolbar_underline");
	m_pToolbarUnderline = toolitem1;

	// insert strikethrough button
	//button3 = gtk_image_new_from_stock (GTK_STOCK_UNDERLINE, tmp_toolbar_icon_size);
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&text_strikethrough);
	button3 = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show (button3);

	toolitem1 = (GtkWidget*) gtk_toggle_tool_button_new  ();
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(toolitem1), _("Strikethrough"));
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(toolitem1), button3);

	gtk_widget_show (toolitem1);
	gtk_container_add (GTK_CONTAINER (toolbar1), toolitem1);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolitem1), tips, _("Strikethrough"), NULL);
	g_signal_connect (GTK_OBJECT (toolitem1), "clicked", G_CALLBACK(on_text_strikethrough_activate), NULL);

	GLADE_HOOKUP_OBJECT (toolbar1, toolitem1, "toolbar_strikethrough");
	m_pToolbarStrike = toolitem1;

	// insert color button
	//button3 = gtk_image_new_from_stock (GTK_STOCK_SELECT_COLOR, tmp_toolbar_icon_size);
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&text_color_xpm);
	button3 = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show (button3);

	toolitem1 = (GtkWidget*) gtk_toggle_tool_button_new ();
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(toolitem1), _("Text Color"));
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(toolitem1), button3);

	gtk_widget_show (toolitem1);
	gtk_container_add (GTK_CONTAINER (toolbar1), toolitem1);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolitem1), tips, _("Text Color"), NULL);
	g_signal_connect (GTK_OBJECT (toolitem1), "clicked", G_CALLBACK(on_text_color_activate), NULL);

	GLADE_HOOKUP_OBJECT (toolbar1, toolitem1, "toolbar_color");
	m_pToolbarColor = toolitem1;

	// insert bkg color button
	//button3 = gtk_image_new_from_stock (GTK_STOCK_SELECT_COLOR, tmp_toolbar_icon_size);
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&text_bg_color_xpm);
	button3 = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show (button3);

	toolitem1 = (GtkWidget*) gtk_toggle_tool_button_new ();
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(toolitem1), _("Text Background Color"));
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(toolitem1), button3);

	gtk_widget_show (toolitem1);
	gtk_container_add (GTK_CONTAINER (toolbar1), toolitem1);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolitem1), tips, _("Text Background Color"), NULL);
	g_signal_connect (GTK_OBJECT (toolitem1), "clicked", G_CALLBACK(on_text_background_color_activate), NULL);

	GLADE_HOOKUP_OBJECT (toolbar1, toolitem1, "toolbar_bkg_color");
	m_pToolbarBkgColor = toolitem1;

	//insert help button
	button3 = gtk_image_new_from_stock (GTK_STOCK_HELP, tmp_toolbar_icon_size);
	gtk_widget_show (button3);

	toolitem1 = (GtkWidget*) gtk_tool_button_new (button3, _("Help"));
	gtk_widget_show (toolitem1);
	gtk_container_add (GTK_CONTAINER (toolbar1), toolitem1);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolitem1), tips, _("Help"), NULL);
	g_signal_connect (GTK_OBJECT (toolitem1), "clicked", G_CALLBACK(on_help1_activate), NULL);

	return toolbar1;
}

gboolean window_state_event (GtkWidget *widget, GdkEventWindowState *event)
{
	if(event->changed_mask & GDK_WINDOW_STATE_ICONIFIED)
	{
		if(event->new_window_state & GDK_WINDOW_STATE_ICONIFIED)
		{
			//on window minimized, show tray icon
			if(g_bMinimizeToTray)
			{
				g_tray.Show();
				g_tray.SetTooltip("NoteCase"); //important: call this after Show
				gtk_widget_hide(window1);
			}
		}
	}
	return FALSE;
}

const char *PasswordCallback(const char *szFile, bool bSave)
{
	static char szPass[1024] = "";
	szPass[0] = '\0';

	bool bCursor = g_bBusyCursor;
	if(bCursor)
		HideBusyCursor();	//restore the normal cursor for the moment

	//ask user for password
	PasswordDialog dlg(bSave ? MODE_PASS_NEW : MODE_PASS_VERIFY);

	//set label
	std::string strLabel = _("Please enter the password for file: ");
	strLabel += GetBaseName(szFile);
	dlg.SetLabel(strLabel.c_str());

	gtk_grab_add( dlg.GetDialog() );	//block all other widgets in the app

	if(GTK_RESPONSE_OK == dlg.ShowModal())
	{
		//get result
		const char *szText = dlg.GetPassword();
		if(szText)
			strcpy(szPass, szText);
	}

	gtk_grab_remove(dlg.GetDialog());	//unblock
	dlg.Destroy();

	if(bCursor)
		ShowBusyCursor(); //keep showing busy

	//return password
	if(strlen(szPass)<1)
		return NULL;
	return szPass;
}

void create_menu (GtkWidget *window1, GtkWidget *menubar1, GtkAccelGroup *accel_group)
{
	GtkWidget *menuitem1;
	GtkWidget *menu1;
	GtkWidget *new1;
	GtkWidget *new_notecase1;
	GtkWidget *open1;
	GtkWidget *close1;
	GtkWidget *save1;
	GtkWidget *save_as1;
	GtkWidget *reload1;
	GtkWidget *lock1;
	GtkWidget *change_pass1;
	GtkWidget *read_only1;
	GtkWidget *separatormenuitem1;
	GtkWidget *quit1;
	GtkWidget *menuitem2;
	GtkWidget *menu2;
	GtkWidget *cut1;
	GtkWidget *copy1;
	GtkWidget *paste1;
	GtkWidget *delete1;
	GtkWidget *find1;
	GtkWidget *find2;
	GtkWidget *replace1;
	GtkWidget *link1;
	GtkWidget *title1;
	GtkWidget *statbar1;
	GtkWidget *menutool1;
	GtkWidget *menufull1;
	GtkWidget *wrap1;
	GtkWidget *options1;
	GtkWidget *shortcuts1;
	GtkWidget *undo1;
	GtkWidget *redo1;
	GtkWidget *menuitem3;
	GtkWidget *menu3;
	GtkWidget *menuitem4;
	GtkWidget *menu4;
	GtkWidget *about1;
	GtkWidget *help1;
	GtkWidget *website1;
	GtkWidget *icon;
	GtkWidget *menu_item;
	GtkWidget *textonly1;
	GtkWidget *menu5;
	GtkWidget *menuitem5;
	GtkWidget *move_item;
	GtkWidget *submenu;
	GtkWidget *wordcount1;
	GtkWidget *sort_item;
	GtkWidget *bold1;
	GtkWidget *italic1;
	GtkWidget *underline1;
	GtkWidget *strikethrough1;
	GtkWidget *color1;
	GtkWidget *bkgcolor1;
	GtkWidget *picture1;
	GtkWidget *remove_format1;
	GtkWidget *attachments1;
	GtkTooltips *menutips;
	GtkWidget *ontop1;
#ifdef HAVE_GTKSOURCEVIEW
	GtkWidget *lines1;
#endif

	menutips = gtk_tooltips_new();
	gtk_tooltips_enable(menutips);

#ifdef _NOKIA_MAEMO
	GtkWidget *maemomenu = gtk_menu_new ();
#endif

	menuitem1 = gtk_menu_item_new_with_mnemonic (_("_File"));
	gtk_widget_show (menuitem1);
#ifdef _NOKIA_MAEMO
	gtk_menu_append(GTK_MENU(maemomenu), menuitem1);
#else
	gtk_container_add (GTK_CONTAINER (menubar1), menuitem1);
#endif

	menu1 = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem1), menu1);

	new_notecase1 = gtk_image_menu_item_new_with_mnemonic (_("New _Window"));
	gtk_widget_show (new_notecase1);
	gtk_container_add (GTK_CONTAINER (menu1), new_notecase1);
		GdkPixbuf *pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&notecase_xpm);
	GdkPixbuf *destpix = gdk_pixbuf_scale_simple(pixbuf, 16, 16, GDK_INTERP_BILINEAR);
	g_object_unref (G_OBJECT (pixbuf));
	icon = gtk_image_new_from_pixbuf (destpix);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (new_notecase1), icon);
	int nPos = g_shortcuts.FindByAction(NC_ACTION_APP_NEW);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (new_notecase1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	new1 = gtk_image_menu_item_new_with_mnemonic (_("_New"));
	gtk_widget_show (new1);
	gtk_container_add (GTK_CONTAINER (menu1), new1);
	icon = gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (new1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_DOC_NEW);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (new1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	open1 = gtk_image_menu_item_new_with_mnemonic (_("_Open"));
	gtk_widget_show (open1);
	gtk_container_add (GTK_CONTAINER (menu1), open1);
	icon = gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (open1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_DOC_OPEN);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (open1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	close1 = gtk_image_menu_item_new_with_mnemonic (_("_Close"));
	gtk_widget_show (close1);
	gtk_container_add (GTK_CONTAINER (menu1), close1);
	icon = gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (close1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_DOC_CLOSE);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (close1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	save1 = gtk_image_menu_item_new_with_mnemonic (_("_Save"));
	gtk_widget_show (save1);
	gtk_container_add (GTK_CONTAINER (menu1), save1);
	icon = gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (save1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_DOC_SAVE);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (save1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	save_as1 = gtk_image_menu_item_new_with_mnemonic (_("Save _As"));
	gtk_widget_show (save_as1);
	gtk_container_add (GTK_CONTAINER (menu1), save_as1);
	icon = gtk_image_new_from_stock (GTK_STOCK_SAVE_AS, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (save_as1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_DOC_SAVE_AS);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (save_as1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	reload1 = gtk_image_menu_item_new_with_mnemonic (_("Re_load"));
	gtk_widget_show (reload1);
	gtk_container_add (GTK_CONTAINER (menu1), reload1);
	icon = gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (reload1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_DOC_RELOAD);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (reload1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	read_only1 = gtk_check_menu_item_new_with_mnemonic (_("Read-Only"));
	gtk_check_menu_item_set_show_toggle(GTK_CHECK_MENU_ITEM(read_only1), TRUE);	//show check mark always
	gtk_widget_show (read_only1);
	gtk_container_add (GTK_CONTAINER (menu1), read_only1);
	nPos = g_shortcuts.FindByAction(NC_ACTION_READ_ONLY);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
 	gtk_widget_add_accelerator (read_only1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));
	gtk_tooltips_set_tip(menutips, read_only1, _("Document property that forbids document editing."), NULL);

	lock1 = gtk_image_menu_item_new_with_mnemonic (_("_Lock Document"));
	gtk_widget_show (lock1);
	gtk_container_add (GTK_CONTAINER (menu1), lock1);
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&lock_xpm);
	icon = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (lock1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_LOCK_DOCUMENT);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
	 	gtk_widget_add_accelerator (lock1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));
	gtk_widget_set_sensitive (lock1, FALSE);
	gtk_tooltips_set_tip(menutips, lock1, _("Unload encrypted document from memory."), NULL);

	change_pass1 = gtk_image_menu_item_new_with_mnemonic (_("_Change Password"));
	gtk_widget_show (change_pass1);
	gtk_container_add (GTK_CONTAINER (menu1), change_pass1);
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&key);
	icon = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (change_pass1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_PASSWORD_CHANGE);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
	 	gtk_widget_add_accelerator (change_pass1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));
	gtk_widget_set_sensitive (change_pass1, FALSE);

	separatormenuitem1 = gtk_separator_menu_item_new ();
	gtk_widget_show (separatormenuitem1);
	gtk_container_add (GTK_CONTAINER (menu1), separatormenuitem1);
	gtk_widget_set_sensitive (separatormenuitem1, FALSE);

	menu_item = gtk_menu_item_new_with_mnemonic(_("_Import"));
	gtk_widget_show (menu_item);  // Show the widget
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_import), NULL);
	gtk_menu_append(menu1, menu_item);
	nPos = g_shortcuts.FindByAction(NC_ACTION_DOC_IMPORT);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_menu_item_new_with_mnemonic(_("_Export"));
	gtk_widget_show (menu_item);  // Show the widget
	g_signal_connect(GTK_OBJECT(menu_item), "activate", G_CALLBACK (on_menu_export), NULL);
	gtk_menu_append(menu1, menu_item);
	nPos = g_shortcuts.FindByAction(NC_ACTION_DOC_EXPORT);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	separatormenuitem1 = gtk_separator_menu_item_new ();
	gtk_widget_show (separatormenuitem1);
	gtk_container_add (GTK_CONTAINER (menu1), separatormenuitem1);
	gtk_widget_set_sensitive (separatormenuitem1, FALSE);

	//
	//  MRU (most recently used) list
	//

	//create MRU item
	g_menuitem5 = gtk_menu_item_new_with_mnemonic (_("_Recent"));
	gtk_widget_show (g_menuitem5);
	gtk_container_add (GTK_CONTAINER (menu1), g_menuitem5);

	g_objMRU.Load();
	g_objMRU.RebuildMenu();

	//add separator
	separatormenuitem1 = gtk_separator_menu_item_new ();
	gtk_widget_show (separatormenuitem1);
	gtk_container_add (GTK_CONTAINER (menu1), separatormenuitem1);
	gtk_widget_set_sensitive (separatormenuitem1, FALSE);

	//continue creating menus
	quit1 = gtk_image_menu_item_new_with_mnemonic (_("_Quit"));
	gtk_widget_show (quit1);
	gtk_container_add (GTK_CONTAINER (menu1), quit1);
	icon = gtk_image_new_from_stock (GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (quit1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_APP_QUIT);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (quit1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	//
	// edit menu
	//

	menuitem2 = gtk_menu_item_new_with_mnemonic (_("_Edit"));
	gtk_widget_show (menuitem2);
#ifdef _NOKIA_MAEMO
	gtk_menu_append(GTK_MENU(maemomenu), menuitem2);
#else
	gtk_container_add (GTK_CONTAINER (menubar1), menuitem2);
#endif

	menu2 = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem2), menu2);

	undo1 = gtk_image_menu_item_new_with_mnemonic (_("_Undo"));
	gtk_widget_show (undo1);
	gtk_widget_set_sensitive(undo1, FALSE);
	gtk_container_add (GTK_CONTAINER (menu2), undo1);
	icon = gtk_image_new_from_stock (GTK_STOCK_UNDO, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (undo1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_EDIT_UNDO);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (undo1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	redo1 = gtk_image_menu_item_new_with_mnemonic (_("_Redo"));
	gtk_widget_show (redo1);
	gtk_widget_set_sensitive(redo1, FALSE);
	gtk_container_add (GTK_CONTAINER (menu2), redo1);
	icon = gtk_image_new_from_stock (GTK_STOCK_REDO, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (redo1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_EDIT_REDO);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (redo1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	separatormenuitem1 = gtk_separator_menu_item_new ();
	gtk_widget_show (separatormenuitem1);
	gtk_container_add (GTK_CONTAINER (menu2), separatormenuitem1);
	gtk_widget_set_sensitive (separatormenuitem1, FALSE);

	cut1 = gtk_image_menu_item_new_with_mnemonic (_("Cu_t"));
	gtk_widget_show (cut1);
	gtk_container_add (GTK_CONTAINER (menu2), cut1);
	icon = gtk_image_new_from_stock (GTK_STOCK_CUT, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (cut1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_EDIT_CUT);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (cut1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	copy1 = gtk_image_menu_item_new_with_mnemonic (_("_Copy"));
	gtk_widget_show (copy1);
	gtk_container_add (GTK_CONTAINER (menu2), copy1);
	icon = gtk_image_new_from_stock (GTK_STOCK_COPY, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (copy1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_EDIT_COPY);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (copy1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	paste1 = gtk_image_menu_item_new_with_mnemonic (_("_Paste"));
	gtk_widget_show (paste1);
	gtk_container_add (GTK_CONTAINER (menu2), paste1);
	icon = gtk_image_new_from_stock (GTK_STOCK_PASTE, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (paste1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_EDIT_PASTE);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (paste1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	delete1 = gtk_image_menu_item_new_with_mnemonic (_("_Delete"));
	gtk_widget_show (delete1);
	gtk_container_add (GTK_CONTAINER (menu2), delete1);
	icon = gtk_image_new_from_stock (GTK_STOCK_DELETE, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (delete1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_EDIT_DELETE);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (delete1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Copy Branch _Structure"));
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_copy_branch_structure), NULL);
	gtk_menu_append(menu2, menu_item);
	gtk_widget_show (menu_item);  // Show the widget
	icon = gtk_image_new_from_stock (GTK_STOCK_COPY, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_EDIT_COPY_BR_STRUCT);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Paste Branch To Root"));
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_paste_branch_root), NULL);
	gtk_menu_append(menu2, menu_item);
	gtk_widget_show (menu_item);  // Show the widget
	icon = gtk_image_new_from_stock (GTK_STOCK_PASTE, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_EDIT_PASTE_BR_ROOT);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Insert Date/Time"));
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_insert_date_time), NULL);
	gtk_menu_append(menu2, menu_item);
	gtk_widget_show (menu_item);  // Show the widget
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&icon_datetime);
	icon = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_EDIT_INSERT_DATETIME);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	separatormenuitem1 = gtk_separator_menu_item_new ();
	gtk_widget_show (separatormenuitem1);
	gtk_container_add (GTK_CONTAINER (menu2), separatormenuitem1);
	gtk_widget_set_sensitive (separatormenuitem1, FALSE);

	find1 = gtk_image_menu_item_new_with_mnemonic (_("_Find"));
	gtk_widget_show (find1);
	gtk_container_add (GTK_CONTAINER (menu2), find1);
	icon = gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (find1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_EDIT_FIND);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (find1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	find2 = gtk_image_menu_item_new_with_mnemonic (_("Find _Next"));
	gtk_widget_show (find2);
	gtk_container_add (GTK_CONTAINER (menu2), find2);
	icon = gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (find2), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_EDIT_FIND_NEXT);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (find2, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	replace1 = gtk_image_menu_item_new_with_mnemonic (_("R_eplace"));
	gtk_widget_show (replace1);
	gtk_container_add (GTK_CONTAINER (menu2), replace1);
	icon = gtk_image_new_from_stock (GTK_STOCK_FIND_AND_REPLACE, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (replace1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_EDIT_REPLACE);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (replace1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	link1 = gtk_image_menu_item_new_with_mnemonic (_("Link _Wizard"));
	gtk_widget_show (link1);
	gtk_container_add (GTK_CONTAINER (menu2), link1);
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&page_link);
	icon = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (link1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_EDIT_LINK_WIZARD);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (link1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	picture1 = gtk_image_menu_item_new_with_mnemonic (_("_Insert Picture"));
	gtk_widget_show (picture1);
	gtk_container_add (GTK_CONTAINER (menu2), picture1);
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&icon_picture);
	icon = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (picture1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_EDIT_INSERT_PICTURE);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (picture1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	attachments1 = gtk_image_menu_item_new_with_mnemonic (_("Node _Attachments"));
	gtk_widget_show (attachments1);
	gtk_container_add (GTK_CONTAINER (menu2), attachments1);
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&icon_attachment);
	icon = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (attachments1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_EDIT_ATTACHMENTS);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (attachments1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	separatormenuitem1 = gtk_separator_menu_item_new ();
	gtk_widget_show (separatormenuitem1);
	gtk_container_add (GTK_CONTAINER (menu2), separatormenuitem1);
	gtk_widget_set_sensitive (separatormenuitem1, FALSE);

	shortcuts1 = gtk_image_menu_item_new_with_mnemonic (_("Configure Shortcuts"));
	gtk_widget_show (shortcuts1);
	gtk_container_add (GTK_CONTAINER (menu2), shortcuts1);
	icon = gtk_image_new_from_stock (GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (shortcuts1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_EDIT_SHORTCUTS);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (shortcuts1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	options1 = gtk_image_menu_item_new_with_mnemonic (_("Configure NoteCase"));
	gtk_widget_show (options1);
	gtk_container_add (GTK_CONTAINER (menu2), options1);
	icon = gtk_image_new_from_stock (GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (options1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_EDIT_OPTIONS);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (options1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	//
	// View menu
	//
	menuitem3 = gtk_menu_item_new_with_mnemonic (_("_View"));
	gtk_widget_show (menuitem3);
#ifdef _NOKIA_MAEMO
	gtk_menu_append(GTK_MENU(maemomenu), menuitem3);
#else
	gtk_container_add (GTK_CONTAINER (menubar1), menuitem3);
#endif

	menu3 = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem3), menu3);

	menufull1 = gtk_check_menu_item_new_with_mnemonic (_("_Full Screen"));
	gtk_widget_show (menufull1);
	gtk_container_add (GTK_CONTAINER (menu3), menufull1);

#ifdef _NOKIA_MAEMO
	gtk_widget_add_accelerator (menufull1, "activate", accel_group, HILDON_HARDKEY_FULLSCREEN, (GdkModifierType)0, (GtkAccelFlags)(GTK_ACCEL_VISIBLE));
#endif
#ifndef _NOKIA_MAEMO
	nPos = g_shortcuts.FindByAction(NC_ACTION_VIEW_FULLSCREEN);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (menufull1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));
#endif

	ontop1 = gtk_check_menu_item_new_with_mnemonic (_("_Always on top"));
	gtk_widget_show (ontop1);
	gtk_container_add (GTK_CONTAINER (menu3), ontop1);
	//TOFIX nPos = g_shortcuts.FindByAction(NC_ACTION_KEEP_ON_TOP);
	//if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
	//	gtk_widget_add_accelerator (ontop1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menutool1 = gtk_check_menu_item_new_with_mnemonic (_("_Tool Bar"));
	gtk_check_menu_item_set_show_toggle(GTK_CHECK_MENU_ITEM(menutool1), TRUE);	//show check mark always
	gtk_widget_show (menutool1);
	gtk_container_add (GTK_CONTAINER (menu3), menutool1);
	nPos = g_shortcuts.FindByAction(NC_ACTION_VIEW_TOOLBAR);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (menutool1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	title1 = gtk_check_menu_item_new_with_mnemonic (_("_Node Title Bar"));
	gtk_check_menu_item_set_show_toggle(GTK_CHECK_MENU_ITEM(title1), TRUE);	//show check mark always
	gtk_widget_show (title1);
	gtk_container_add (GTK_CONTAINER (menu3), title1);
	nPos = g_shortcuts.FindByAction(NC_ACTION_VIEW_NODE_TITLEBAR);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (title1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	statbar1 = gtk_check_menu_item_new_with_mnemonic (_("_Status Bar"));
	gtk_widget_show (statbar1);
	gtk_check_menu_item_set_show_toggle(GTK_CHECK_MENU_ITEM(statbar1), TRUE);	//show check mark always
	gtk_container_add (GTK_CONTAINER (menu3), statbar1);
	nPos = g_shortcuts.FindByAction(NC_ACTION_VIEW_STATUS_BAR);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (statbar1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

#ifdef HAVE_GTKSOURCEVIEW
	lines1 = gtk_check_menu_item_new_with_mnemonic (_("Line numbers"));
	gtk_check_menu_item_set_show_toggle(GTK_CHECK_MENU_ITEM(lines1), TRUE);	//show check mark always
	gtk_widget_show (lines1);
	gtk_container_add (GTK_CONTAINER (menu3), lines1);
	//TOFIX nPos = g_shortcuts.FindByAction(NC_ACTION_VIEW_WRAP);
	//if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
	//	gtk_widget_add_accelerator (wrap1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));
#endif

	wrap1 = gtk_check_menu_item_new_with_mnemonic (_("_Wrap"));
	gtk_check_menu_item_set_show_toggle(GTK_CHECK_MENU_ITEM(wrap1), TRUE);	//show check mark always
	gtk_widget_show (wrap1);
	gtk_container_add (GTK_CONTAINER (menu3), wrap1);
	nPos = g_shortcuts.FindByAction(NC_ACTION_VIEW_WRAP);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (wrap1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	textonly1 = gtk_menu_item_new_with_mnemonic (_("Change View _Mode"));
	gtk_widget_show (textonly1);
	gtk_container_add (GTK_CONTAINER (menu3), textonly1);
	nPos = g_shortcuts.FindByAction(NC_ACTION_VIEW_CHANGE_MODE);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (textonly1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	wordcount1 = gtk_menu_item_new_with_mnemonic (_("_Calculate Word Count"));
	gtk_widget_show (wordcount1);
	gtk_container_add (GTK_CONTAINER (menu3), wordcount1);
	nPos = g_shortcuts.FindByAction(NC_ACTION_VIEW_WORD_COUNT);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (wordcount1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	//
	// tree menu (start)
	//

	menuitem5 = gtk_menu_item_new_with_mnemonic (_("_Tree"));
	gtk_widget_show (menuitem5);
#ifdef _NOKIA_MAEMO
	gtk_menu_append(GTK_MENU(maemomenu), menuitem5);
#else
	gtk_container_add (GTK_CONTAINER (menubar1), menuitem5);
#endif

	menu5 = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem5), menu5);

	// ... add menu items with accelerators ...
	menu_item = gtk_menu_item_new_with_mnemonic(_("_Insert Node"));
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_insert_node), NULL);
	gtk_menu_append(menu5, menu_item);
	gtk_widget_show (menu_item);  // Show the widget
	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_NODE_NEW);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_menu_item_new_with_mnemonic(_("Insert Child _Node"));
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_insert_child_node), NULL);
	gtk_menu_append(menu5, menu_item);
	gtk_widget_show (menu_item);  // Show the widget
	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_NODE_NEW_CHILD);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Delete Node"));
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_delete_node), NULL);
	gtk_menu_append(menu5, menu_item);
	gtk_widget_show (menu_item);  // Show the widget
	icon = gtk_image_new_from_stock (GTK_STOCK_DELETE, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_NODE_DELETE);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_menu_item_new_with_mnemonic(_("_Rename Node"));
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_rename_node), NULL);
	gtk_menu_append(menu5, menu_item);
	gtk_widget_show (menu_item);  // Show the widget
	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_NODE_RENAME);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	move_item = gtk_menu_item_new_with_mnemonic(_("_Move Node"));
	gtk_menu_append(menu5, move_item);
	gtk_widget_show (move_item);  // Show the widget

	sort_item = gtk_menu_item_new_with_mnemonic(_("_Sort"));
	gtk_menu_append(menu5, sort_item);
	gtk_widget_show (sort_item);  // Show the widget

	menu_item = gtk_menu_item_new_with_mnemonic(_("_Expand All"));
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_expand_all), NULL);
	gtk_menu_append(menu5, menu_item);
	gtk_widget_show (menu_item);  // Show the widget
	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_EXPAND_ALL);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_menu_item_new_with_mnemonic(_("_Collapse All"));
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_collapse_all), NULL);
	gtk_menu_append(menu5, menu_item);
	gtk_widget_show (menu_item);  // Show the widget
	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_COLAPSE_ALL);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Node _Properties"));
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_node_properties), NULL);
	gtk_menu_append(menu5, menu_item);
	gtk_widget_show (menu_item);  // Show the widget
	icon = gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_NODE_PROPERTIES);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	//"move node" submenu
	submenu = gtk_menu_new ();

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Up"));
	gtk_menu_append(submenu, menu_item);
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_move_up), NULL);
	gtk_widget_show (menu_item);  // Show the widget
	icon = gtk_image_new_from_stock (GTK_STOCK_GO_UP, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_NODE_MOVE_UP);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Down"));
	gtk_menu_append(submenu, menu_item);
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_move_down), NULL);
	gtk_widget_show (menu_item);  // Show the widget
	icon = gtk_image_new_from_stock (GTK_STOCK_GO_DOWN, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_NODE_MOVE_DOWN);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Left"));
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_move_left), NULL);
	gtk_menu_append(submenu, menu_item);
	gtk_widget_show (menu_item);  // Show the widget
	icon = gtk_image_new_from_stock (GTK_STOCK_GO_BACK, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_NODE_MOVE_LEFT);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Right"));
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_move_right), NULL);
	gtk_menu_append(submenu, menu_item);
	gtk_widget_show (menu_item);  // Show the widget
	icon = gtk_image_new_from_stock (GTK_STOCK_GO_FORWARD, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_NODE_MOVE_RIGHT);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (move_item), submenu);

	//"sort node" submenu
	submenu = gtk_menu_new ();

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Sort Children Ascending"));
	gtk_menu_append(submenu, menu_item);
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_sort_child_ascending), NULL);
	gtk_widget_show (menu_item);  // Show the widget
	icon = gtk_image_new_from_stock (GTK_STOCK_SORT_ASCENDING, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_SORT_CHILDREN_ASC);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Sort _Children Descending"));
	gtk_menu_append(submenu, menu_item);
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_sort_child_descending), NULL);
	gtk_widget_show (menu_item);  // Show the widget
	icon = gtk_image_new_from_stock (GTK_STOCK_SORT_DESCENDING, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_SORT_CHILDREN_DESC);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Sort Root Ascending"));
	gtk_menu_append(submenu, menu_item);
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_sort_root_ascending), NULL);
	gtk_widget_show (menu_item);  // Show the widget
	icon = gtk_image_new_from_stock (GTK_STOCK_SORT_ASCENDING, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_SORT_ROOT_ASC);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Sort Root Descending"));
	gtk_menu_append(submenu, menu_item);
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_sort_root_descending), NULL);
	gtk_widget_show (menu_item);  // Show the widget
	icon = gtk_image_new_from_stock (GTK_STOCK_SORT_DESCENDING, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_SORT_ROOT_DESC);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (sort_item), submenu);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Delete all _Completed nodes"));
	gtk_menu_append(menu5, menu_item);
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_delete_finished_nodes), NULL);
	gtk_widget_show (menu_item);  // Show the widget
	icon = gtk_image_new_from_stock (GTK_STOCK_DELETE, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_TREE_NODE_DELETE_FINISHED);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	//
	// Format menu
	//

	menuitem2 = gtk_menu_item_new_with_mnemonic (_("F_ormat"));
	gtk_widget_show (menuitem2);
#ifdef _NOKIA_MAEMO
	gtk_menu_append(GTK_MENU(maemomenu), menuitem2);
#else
	gtk_container_add (GTK_CONTAINER (menubar1), menuitem2);
#endif

	menu2 = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem2), menu2);

	bold1 = gtk_image_menu_item_new_with_mnemonic (_("_Bold"));
	gtk_widget_show (bold1);
	gtk_container_add (GTK_CONTAINER (menu2), bold1);
	//icon = gtk_image_new_from_stock (GTK_STOCK_BOLD, GTK_ICON_SIZE_MENU);
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&text_bold);
	icon = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (bold1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_FORMAT_BOLD);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (bold1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	italic1 = gtk_image_menu_item_new_with_mnemonic (_("_Italic"));
	gtk_widget_show (italic1);
	gtk_container_add (GTK_CONTAINER (menu2), italic1);
	//icon = gtk_image_new_from_stock (GTK_STOCK_ITALIC, GTK_ICON_SIZE_MENU);
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&text_italic);
	icon = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (italic1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_FORMAT_ITALIC);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (italic1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	underline1 = gtk_image_menu_item_new_with_mnemonic (_("_Underline"));
	gtk_widget_show (underline1);
	gtk_container_add (GTK_CONTAINER (menu2), underline1);
	//icon = gtk_image_new_from_stock (GTK_STOCK_UNDERLINE, GTK_ICON_SIZE_MENU);
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&text_underline);
	icon = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (underline1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_FORMAT_UNDERLINE);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (underline1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	strikethrough1 = gtk_image_menu_item_new_with_mnemonic (_("_Strikethrough"));
	gtk_widget_show (strikethrough1);
	gtk_container_add (GTK_CONTAINER (menu2), strikethrough1);
	//icon = gtk_image_new_from_stock (GTK_STOCK_UNDERLINE, GTK_ICON_SIZE_MENU);
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&text_strikethrough);
	icon = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (strikethrough1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_FORMAT_STRIKETHROUGH);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (strikethrough1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	color1 = gtk_image_menu_item_new_with_mnemonic (_("_Text color"));
	gtk_widget_show (color1);
	gtk_container_add (GTK_CONTAINER (menu2), color1);
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&text_color_xpm);
	icon = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (color1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_FORMAT_COLOR);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (color1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	bkgcolor1 = gtk_image_menu_item_new_with_mnemonic (_("Text Background _Color"));
	gtk_widget_show (bkgcolor1);
	gtk_container_add (GTK_CONTAINER (menu2), bkgcolor1);
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&text_bg_color_xpm);
	icon = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (bkgcolor1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_FORMAT_BKG_COLOR);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (bkgcolor1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	remove_format1 = gtk_image_menu_item_new_with_mnemonic (_("_Remove formatting"));
	gtk_widget_show (remove_format1);
	gtk_container_add (GTK_CONTAINER (menu2), remove_format1);
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&text_no_format_xpm);
	icon = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (remove_format1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_FORMAT_REMOVE);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (remove_format1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));


	//
	// Help menu
	//

	menuitem4 = gtk_menu_item_new_with_mnemonic (_("_Help"));
	gtk_widget_show (menuitem4);
#ifdef _NOKIA_MAEMO
	gtk_menu_append(GTK_MENU(maemomenu), menuitem4);
#else
	gtk_container_add (GTK_CONTAINER (menubar1), menuitem4);
#endif

	menu4 = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem4), menu4);

	website1 = gtk_image_menu_item_new_with_mnemonic (_("_Visit Website"));
	gtk_widget_show (website1);
	gtk_container_add (GTK_CONTAINER (menu4), website1);
	//icon = gtk_image_new_from_stock (GTK_STOCK_HELP, GTK_ICON_SIZE_MENU);
	//gtk_widget_show (icon);
	//gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (website1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_VISIT_WEBSITE);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (website1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	help1 = gtk_image_menu_item_new_with_mnemonic (_("_Show help"));
	gtk_widget_show (help1);
	gtk_container_add (GTK_CONTAINER (menu4), help1);
	icon = gtk_image_new_from_stock (GTK_STOCK_HELP, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (help1), icon);
	nPos = g_shortcuts.FindByAction(NC_ACTION_HELP_INDEX);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (help1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	separatormenuitem1 = gtk_separator_menu_item_new ();
	gtk_widget_show (separatormenuitem1);
	gtk_container_add (GTK_CONTAINER (menu4), separatormenuitem1);
	gtk_widget_set_sensitive (separatormenuitem1, FALSE);

	about1 = gtk_menu_item_new_with_mnemonic (_("_About"));
	gtk_widget_show (about1);
	gtk_container_add (GTK_CONTAINER (menu4), about1);
	nPos = g_shortcuts.FindByAction(NC_ACTION_HELP_ABOUT);
	if(nPos >= 0 && !g_shortcuts[nPos].IsEmpty())
		gtk_widget_add_accelerator (about1, "activate", accel_group, g_shortcuts[nPos].m_nKey, (GdkModifierType)g_shortcuts[nPos].GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	g_signal_connect (GTK_OBJECT (new_notecase1),    "activate", G_CALLBACK(on_new_notecase1_activate),  NULL);
	g_signal_connect (GTK_OBJECT (new1),    "activate", G_CALLBACK(on_new1_activate),  NULL);
	g_signal_connect (GTK_OBJECT (open1),   "activate", G_CALLBACK(on_open1_activate), NULL);
	g_signal_connect (GTK_OBJECT (close1),   "activate", G_CALLBACK(on_close1_activate), NULL);
	g_signal_connect (GTK_OBJECT (save1),   "activate", G_CALLBACK(on_save1_activate), NULL);
	g_signal_connect (GTK_OBJECT (save_as1),"activate", G_CALLBACK(on_save_as1_activate), NULL);
	g_signal_connect (GTK_OBJECT (reload1), "activate", G_CALLBACK(on_reload1_activate), NULL);
	g_signal_connect (GTK_OBJECT (ontop1), "activate", G_CALLBACK(on_ontop1_activate), NULL);
	g_signal_connect (GTK_OBJECT (read_only1), "activate", G_CALLBACK(on_read_only_activate), NULL);
	g_signal_connect (GTK_OBJECT (lock1), "activate", G_CALLBACK(on_lock_document_activate), NULL);
	g_signal_connect (GTK_OBJECT (change_pass1), "activate", G_CALLBACK(on_change_pass1_activate), NULL);
	g_signal_connect (GTK_OBJECT (quit1),   "activate", G_CALLBACK(on_quit1_activate), NULL);
	g_signal_connect (GTK_OBJECT (cut1),    "activate", G_CALLBACK(on_cut1_activate), NULL);
	g_signal_connect (GTK_OBJECT (copy1),   "activate", G_CALLBACK(on_copy1_activate), NULL);
	g_signal_connect (GTK_OBJECT (paste1),  "activate", G_CALLBACK(on_paste1_activate), NULL);
	g_signal_connect (GTK_OBJECT (delete1), "activate", G_CALLBACK(on_delete1_activate), NULL);
	g_signal_connect (GTK_OBJECT (about1),  "activate", G_CALLBACK(on_about1_activate), NULL);
	g_signal_connect (GTK_OBJECT (help1),   "activate", G_CALLBACK(on_help1_activate), NULL);
	g_signal_connect (GTK_OBJECT (website1),"activate", G_CALLBACK(on_menu_visit_website), NULL);
	g_signal_connect (GTK_OBJECT (options1),"activate", G_CALLBACK(on_options1_activate), NULL);
	g_signal_connect (GTK_OBJECT (shortcuts1),"activate", G_CALLBACK(on_view_shortcuts_activate), NULL);
	g_signal_connect (GTK_OBJECT (find1),	"activate", G_CALLBACK(on_find1_activate), NULL);
	g_signal_connect (GTK_OBJECT (find2),	"activate", G_CALLBACK(on_find2_activate), NULL);
	g_signal_connect (GTK_OBJECT (replace1),"activate", G_CALLBACK(on_find_replace_activate), NULL);
	g_signal_connect (GTK_OBJECT (wrap1),	"activate", G_CALLBACK(on_wrap_activate), NULL);
#ifdef HAVE_GTKSOURCEVIEW	
	g_signal_connect (GTK_OBJECT (lines1),	"activate", G_CALLBACK(on_show_line_numbers_activate), NULL);
#endif
	g_signal_connect (GTK_OBJECT (title1),	"activate", G_CALLBACK(on_show_node_titlebar_activate), NULL);
	g_signal_connect (GTK_OBJECT (statbar1),"activate", G_CALLBACK(on_show_status_bar_activate), NULL);
	g_signal_connect (GTK_OBJECT (menutool1),"activate", G_CALLBACK(on_show_toolbar_activate), NULL);
	g_signal_connect (GTK_OBJECT (menufull1),"activate", G_CALLBACK(on_full_screen_activate), NULL);
	g_signal_connect (GTK_OBJECT (undo1),	"activate", G_CALLBACK(on_undo_activate), NULL);
	g_signal_connect (GTK_OBJECT (redo1),	"activate", G_CALLBACK(on_redo_activate), NULL);
	g_signal_connect (GTK_OBJECT (link1),   "activate", G_CALLBACK(on_link_wizard_activate), NULL);
	g_signal_connect (GTK_OBJECT (textonly1),   "activate", G_CALLBACK(on_view_text_only_mode), NULL);
	g_signal_connect (GTK_OBJECT (wordcount1),   "activate", G_CALLBACK(on_view_calc_word_count), NULL);
	g_signal_connect (GTK_OBJECT (bold1),     "activate", G_CALLBACK(on_text_bold_activate), NULL);
	g_signal_connect (GTK_OBJECT (italic1),   "activate", G_CALLBACK(on_text_italic_activate), NULL);
	g_signal_connect (GTK_OBJECT (underline1),"activate", G_CALLBACK(on_text_underline_activate), NULL);
	g_signal_connect (GTK_OBJECT (strikethrough1),"activate", G_CALLBACK(on_text_strikethrough_activate), NULL);
	g_signal_connect (GTK_OBJECT (color1),    "activate", G_CALLBACK(on_text_color_activate), NULL);
	g_signal_connect (GTK_OBJECT (bkgcolor1),    "activate", G_CALLBACK(on_text_background_color_activate), NULL);
	g_signal_connect (GTK_OBJECT (picture1), "activate", G_CALLBACK(on_insert_picture_activate), NULL);
	g_signal_connect (GTK_OBJECT (remove_format1), "activate", G_CALLBACK(on_remove_format_activate), NULL);
	g_signal_connect (GTK_OBJECT (attachments1), "activate", G_CALLBACK(on_node_attachments_activate), NULL);

	GLADE_HOOKUP_OBJECT (window1, menuitem1, "menuitem1");
	GLADE_HOOKUP_OBJECT (window1, menu1, "menu1");
	GLADE_HOOKUP_OBJECT (window1, new1, "new1");
	GLADE_HOOKUP_OBJECT (window1, open1, "open1");
	GLADE_HOOKUP_OBJECT (window1, wrap1, "wrap1");
	GLADE_HOOKUP_OBJECT (window1, title1, "title1");
	GLADE_HOOKUP_OBJECT (window1, menutool1, "menutool1");
	GLADE_HOOKUP_OBJECT (window1, find1, "find1");
	GLADE_HOOKUP_OBJECT (window1, find2, "find2");
	GLADE_HOOKUP_OBJECT (window1, replace1, "replace1");
	GLADE_HOOKUP_OBJECT (window1, save1, "save1");
	GLADE_HOOKUP_OBJECT (window1, save_as1, "save_as1");
	GLADE_HOOKUP_OBJECT (window1, reload1, "reload1");
	GLADE_HOOKUP_OBJECT (window1, read_only1, "read_only1");
	GLADE_HOOKUP_OBJECT (window1, lock1, "lock1");
	GLADE_HOOKUP_OBJECT (window1, change_pass1, "change_pass1");
	GLADE_HOOKUP_OBJECT (window1, separatormenuitem1, "separatormenuitem1");
	GLADE_HOOKUP_OBJECT (window1, quit1, "quit1");
	GLADE_HOOKUP_OBJECT (window1, menuitem2, "menuitem2");
	GLADE_HOOKUP_OBJECT (window1, menu2, "menu2");
	GLADE_HOOKUP_OBJECT (window1, cut1, "cut1");
	GLADE_HOOKUP_OBJECT (window1, copy1, "copy1");
	GLADE_HOOKUP_OBJECT (window1, paste1, "paste1");
	GLADE_HOOKUP_OBJECT (window1, delete1, "delete1");
	GLADE_HOOKUP_OBJECT (window1, menuitem4, "menuitem4");
	GLADE_HOOKUP_OBJECT (window1, menu4, "menu4");
	GLADE_HOOKUP_OBJECT (window1, about1, "about1");
	GLADE_HOOKUP_OBJECT (window1, help1, "help1");
	GLADE_HOOKUP_OBJECT (window1, website1, "website1");
	GLADE_HOOKUP_OBJECT (window1, statbar1, "statbar1");
	GLADE_HOOKUP_OBJECT (window1, undo1, "undo1");
	GLADE_HOOKUP_OBJECT (window1, redo1, "redo1");
	GLADE_HOOKUP_OBJECT (window1, link1, "link1");
	GLADE_HOOKUP_OBJECT (window1, wordcount1, "wordcount1");
	GLADE_HOOKUP_OBJECT (window1, new_notecase1, "new_notecase1");
	GLADE_HOOKUP_OBJECT (window1, bold1, "bold1");
	GLADE_HOOKUP_OBJECT (window1, italic1, "italic1");
	GLADE_HOOKUP_OBJECT (window1, underline1, "underline1");
	GLADE_HOOKUP_OBJECT (window1, color1, "color1");
	GLADE_HOOKUP_OBJECT (window1, bkgcolor1, "bkgcolor1");
	GLADE_HOOKUP_OBJECT (window1, picture1, "picture1");
	GLADE_HOOKUP_OBJECT (window1, remove_format1, "remove_format1");
	GLADE_HOOKUP_OBJECT (window1, attachments1, "attachments1");
	GLADE_HOOKUP_OBJECT (window1, menufull1, "menufull1");

#ifdef _NOKIA_MAEMO
	hildon_window_set_menu(HILDON_WINDOW(window1), GTK_MENU(maemomenu));
#endif
}

gboolean on_focus_event (GtkWidget *widget, GtkDirectionType arg1, gpointer user_data)
{
	//FIX: prevent toolbar from getting focus by keyboard
	static bool bRecurse = false;
	if(!bRecurse)
	{
		bRecurse = true;
		gtk_widget_child_focus (widget, (GtkDirectionType)GTK_DIR_TAB_FORWARD);
		bRecurse = false;
	}

	return FALSE;
}

bool LocaleToUtf8(const char *szTxt, std::string &strResult)
{
	strResult = "";

	gsize nBytesWritten = 0;
#if defined(G_OS_WIN32)
	gchar *pFileNameInUTF8 = g_locale_to_utf8(szTxt, -1, 0, &nBytesWritten, 0);
#else
	gchar *pFileNameInUTF8 = g_filename_to_utf8(szTxt, -1, 0, &nBytesWritten, 0);
#endif

	if (pFileNameInUTF8)
	{
		strResult = pFileNameInUTF8;
		g_free(pFileNameInUTF8);
		return true;
	}

	// on failure, copy original string
	strResult = szTxt;
	return false;
}

bool Utf8ToLocale(const char *szTxt, std::string &strResult)
{
	gsize bytesWritten = 0;

	// On Windows GLib uses UTF-8 for filenames, but as well as we operate
	// on native Windows API, conversion to the encoding used in current
	// locale is required.
#if defined(G_OS_WIN32)
	gchar *pFileName = g_locale_from_utf8(szTxt, -1, 0, &bytesWritten, 0);
#else
	gchar *pFileName = g_filename_from_utf8(szTxt, -1, 0, &bytesWritten, 0);
#endif
	if (NULL != pFileName)
	{
		strResult = pFileName;
		g_free(pFileName);
		return true;
	}

	// on failure, copy original string
	strResult = szTxt;
	return false;
}

gint dlg_keyboard_handler(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	//Esc press in the main window minimizes the application
	if( event->keyval == GDK_Escape )
	{
		GtkWidget *widget1 = gtk_window_get_focus(GTK_WINDOW(window1));
		if( widget1 != g_text.m_pWidget &&
			widget1 != g_tree.m_pWidget)
			return FALSE;	// skip this, we are in the label editing mode

		gtk_window_iconify(GTK_WINDOW(widget));
		return TRUE;    //eat event (handled here)
	}
	return FALSE;
}

void SetTextViewCaretColor(bool bBlack)
{
	//hack to set 'caret' color dynamicaly (without .rc file on disk)

	GdkColor color = { 0, 0xFFFF, 0xFFFF, 0xFFFF };	//white
	if(bBlack){
		GdkColor black = { 0, 0, 0, 0 };	//black
		color = black;
	}

	//TOFIX move this to TextView fn.
	static const char cursor_color_rc[] =
            "style \"Notecase\"\n"
            "{\n"
            "GtkTextView::cursor-color=\"#%04x%04x%04x\"\n"
            "}\n"
            "widget \"*.GtkTextView*\" style : application \"Notecase\"\n";

    gchar *rc_temp = g_strdup_printf (cursor_color_rc,
							color.red,
                            color.green,
                            color.blue);
    gtk_rc_parse_string (rc_temp);
    gtk_widget_reset_rc_styles (g_text.m_pWidget);
    g_free (rc_temp);
}

//Based on text-view background color, calculate
//the color for text-view caret mark (a.k.a. cursor).
//Caret is set to either white or black color (dark bkg -> light caret and vice versa)
void RefreshTextViewCaretColor(GdkColor &BkColor)
{
	//TOFIX for better result try using HSL color model -> L = (MAX(r,g,b) + MIN(r,g,b))/2
	//is the background light enough for cursor to be black?
	int nR = BkColor.red/255;
	int nG = BkColor.green/255;
	int nB = BkColor.blue/255;
	int nPower = (nR*nR)+(nG*nG)+(nB*nB);

	bool bCaretColorBlack = (nPower > 48387);
	SetTextViewCaretColor(bCaretColorBlack);
}

gboolean initial_load_timer(gpointer data)
{
	//load initial document if any
	if(g_bLoadEmbedded)
		load_file_embedded(/*strCurNodeID*/);
	else{
		//allow password being entered up to 3 times
		int nResult = 0;
		int nTryCnt = 0;
		while(nTryCnt < 3){
			nResult = load_file(g_strLoadDocumentFile.c_str(), g_strLoadDocumentPass.c_str(), g_nLoadDocumentNodeID);
			nTryCnt ++;
			if(DOC_LOAD_WRONG_PASSWORD != nResult){
				break;
			}
		}
		if(g_bUpdateMRU)
			g_objMRU.Change(g_strLoadDocumentFile.c_str(), (DOC_LOAD_NOT_FOUND != nResult));//update MRU unless file not found
	}

	g_strLoadDocumentPass = "";	// reset password

	return FALSE; // kill timer
}

void treeview_show_popup_menu()
{
	GtkWidget *menu, *submenu;
	GtkWidget *menu_item, *move_item, *sort_item, *icon;
	int button, event_time;

	menu = gtk_menu_new ();
	//g_signal_connect (menu, "deactivate", G_CALLBACK(gtk_widget_destroy), NULL);

	GtkAccelGroup *accel_group = gtk_accel_group_new();
	gtk_menu_set_accel_group (GTK_MENU (menu), accel_group);

	// ... add menu items with accelerators ...
	menu_item = gtk_menu_item_new_with_label(_("Insert Node"));
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_insert_node), NULL);
	gtk_menu_append(menu, menu_item);
	gtk_widget_show (menu_item);  // Show the widget
	if(g_defTreeNodeNew.IsValid(SH_CTX_TREE_WIDGET))
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_defTreeNodeNew.m_nKey, (GdkModifierType)g_defTreeNodeNew.GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_menu_item_new_with_label(_("Insert Child Node"));
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_insert_child_node), NULL);
	gtk_menu_append(menu, menu_item);
	gtk_widget_show (menu_item);  // Show the widget
	if(g_defTreeNodeNewChild.IsValid(SH_CTX_TREE_WIDGET))
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_defTreeNodeNewChild.m_nKey, (GdkModifierType)g_defTreeNodeNewChild.GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_image_menu_item_new_with_label(_("Delete Node"));
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_delete_node), NULL);
	gtk_menu_append(menu, menu_item);
	gtk_widget_show (menu_item);  // Show the widget
	icon = gtk_image_new_from_stock (GTK_STOCK_DELETE, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
	if(g_defTreeNodeDelete.IsValid(SH_CTX_TREE_WIDGET))
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_defTreeNodeDelete.m_nKey, (GdkModifierType)g_defTreeNodeDelete.GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_menu_item_new_with_label(_("Rename Node"));
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_rename_node), NULL);
	gtk_menu_append(menu, menu_item);
	gtk_widget_show (menu_item);  // Show the widget
	if(g_defTreeNodeRename.IsValid(SH_CTX_TREE_WIDGET))
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_defTreeNodeRename.m_nKey, (GdkModifierType)g_defTreeNodeRename.GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	move_item = gtk_menu_item_new_with_label(_("Move Node"));
	gtk_menu_append(menu, move_item);
	gtk_widget_show (move_item);  // Show the widget

	sort_item = gtk_menu_item_new_with_label(_("Sort"));
	gtk_menu_append(menu, sort_item);
	gtk_widget_show (sort_item);  // Show the widget

	menu_item = gtk_menu_item_new_with_label(_("Expand All"));
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_expand_all), NULL);
	gtk_menu_append(menu, menu_item);
	gtk_widget_show (menu_item);  // Show the widget
	if(g_defTreeNodeExpandAll.IsValid(SH_CTX_TREE_WIDGET))
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_defTreeNodeExpandAll.m_nKey, (GdkModifierType)g_defTreeNodeExpandAll.GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_menu_item_new_with_label(_("Collapse All"));
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_collapse_all), NULL);
	gtk_menu_append(menu, menu_item);
	gtk_widget_show (menu_item);  // Show the widget
	if(g_defTreeNodeColapseAll.IsValid(SH_CTX_TREE_WIDGET))
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_defTreeNodeColapseAll.m_nKey, (GdkModifierType)g_defTreeNodeColapseAll.GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_image_menu_item_new_with_label(_("Mark node as Completed"));
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_node_toggle_finished), NULL);
	gtk_menu_append(menu, menu_item);
	gtk_widget_show (menu_item);  // Show the widget
	if(g_defTreeNodeToggleFinished.IsValid(SH_CTX_TREE_WIDGET))
			gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_defTreeNodeToggleFinished.m_nKey, (GdkModifierType)g_defTreeNodeToggleFinished.GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_image_menu_item_new_with_label(_("Node Properties"));
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_node_properties), NULL);
	gtk_menu_append(menu, menu_item);
	gtk_widget_show (menu_item);  // Show the widget
	icon = gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
	if(g_defTreeNodeProperties.IsValid(SH_CTX_TREE_WIDGET))
			gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_defTreeNodeProperties.m_nKey, (GdkModifierType)g_defTreeNodeProperties.GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_image_menu_item_new_with_label(_("Link wizard"));
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_link_wizard_activate), NULL);
	gtk_menu_append(menu, menu_item);
	gtk_widget_show (menu_item);  // Show the widget
	GdkPixbuf* pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)&page_link);
	icon = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);

	//"move node" submenu
	submenu = gtk_menu_new ();

	menu_item = gtk_image_menu_item_new_with_label(_("Up"));
	gtk_menu_append(submenu, menu_item);
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_move_up), NULL);
	gtk_widget_show (menu_item);  // Show the widget
	icon = gtk_image_new_from_stock (GTK_STOCK_GO_UP, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
	if(g_defTreeNodeMoveUp.IsValid(SH_CTX_TREE_WIDGET))
			gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_defTreeNodeMoveUp.m_nKey, (GdkModifierType)g_defTreeNodeMoveUp.GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_image_menu_item_new_with_label(_("Down"));
	gtk_menu_append(submenu, menu_item);
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_move_down), NULL);
	gtk_widget_show (menu_item);  // Show the widget
	icon = gtk_image_new_from_stock (GTK_STOCK_GO_DOWN, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
	if(g_defTreeNodeMoveDown.IsValid(SH_CTX_TREE_WIDGET))
			gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_defTreeNodeMoveDown.m_nKey, (GdkModifierType)g_defTreeNodeMoveDown.GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_image_menu_item_new_with_label(_("Left"));
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_move_left), NULL);
	gtk_menu_append(submenu, menu_item);
	gtk_widget_show (menu_item);  // Show the widget
	icon = gtk_image_new_from_stock (GTK_STOCK_GO_BACK, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
	if(g_defTreeNodeMoveLeft.IsValid(SH_CTX_TREE_WIDGET))
			gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_defTreeNodeMoveLeft.m_nKey, (GdkModifierType)g_defTreeNodeMoveLeft.GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_image_menu_item_new_with_label(_("Right"));
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_move_right), NULL);
	gtk_menu_append(submenu, menu_item);
	gtk_widget_show (menu_item);  // Show the widget
	icon = gtk_image_new_from_stock (GTK_STOCK_GO_FORWARD, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
	if(g_defTreeNodeMoveRight.IsValid(SH_CTX_TREE_WIDGET))
			gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_defTreeNodeMoveRight.m_nKey, (GdkModifierType)g_defTreeNodeMoveRight.GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (move_item), submenu);

	//"sort node" submenu
	submenu = gtk_menu_new ();

	menu_item = gtk_image_menu_item_new_with_label(_("_Sort Children Ascending"));
	gtk_menu_append(submenu, menu_item);
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_sort_child_ascending), NULL);
	gtk_widget_show (menu_item);  // Show the widget
	icon = gtk_image_new_from_stock (GTK_STOCK_SORT_ASCENDING, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
	if(g_defTreeNodeSortChildrenAsc.IsValid(SH_CTX_TREE_WIDGET))
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_defTreeNodeSortChildrenAsc.m_nKey, (GdkModifierType)g_defTreeNodeSortChildrenAsc.GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_image_menu_item_new_with_label(_("Sort _Children Descending"));
	gtk_menu_append(submenu, menu_item);
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_sort_child_descending), NULL);
	gtk_widget_show (menu_item);  // Show the widget
	icon = gtk_image_new_from_stock (GTK_STOCK_SORT_DESCENDING, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
	if(g_defTreeNodeSortChildrenAsc.IsValid(SH_CTX_TREE_WIDGET))
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_defTreeNodeSortChildrenAsc.m_nKey, (GdkModifierType)g_defTreeNodeSortChildrenAsc.GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_image_menu_item_new_with_label(_("Sort Root Ascending"));
	gtk_menu_append(submenu, menu_item);
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_sort_root_ascending), NULL);
	gtk_widget_show (menu_item);  // Show the widget
	icon = gtk_image_new_from_stock (GTK_STOCK_SORT_ASCENDING, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
	if(g_defTreeNodeSortRootAsc.IsValid(SH_CTX_TREE_WIDGET))
			gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_defTreeNodeSortRootAsc.m_nKey, (GdkModifierType)g_defTreeNodeSortRootAsc.GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));

	menu_item = gtk_image_menu_item_new_with_label(_("Sort Root Descending"));
	gtk_menu_append(submenu, menu_item);
	g_signal_connect(menu_item, "activate",	G_CALLBACK (on_menu_sort_root_descending), NULL);
	gtk_widget_show (menu_item);  // Show the widget
	icon = gtk_image_new_from_stock (GTK_STOCK_SORT_DESCENDING, GTK_ICON_SIZE_MENU);
	gtk_widget_show (icon);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
	if(g_defTreeNodeSortRootDsc.IsValid(SH_CTX_TREE_WIDGET))
		gtk_widget_add_accelerator (menu_item, "activate", accel_group, g_defTreeNodeSortRootDsc.m_nKey, (GdkModifierType)g_defTreeNodeSortRootDsc.GetModifierFlags(), (GtkAccelFlags)(GTK_ACCEL_VISIBLE));


	gtk_menu_item_set_submenu (GTK_MENU_ITEM (sort_item), submenu);

//	if (event)
//		event_time = event->time;
//	else
		event_time = gtk_get_current_event_time ();
	button = 0;	//FIX: allow mouse button to trigger the submenu

	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, button, event_time);
}

void on_tree_resize(GtkWidget *widget, GtkAllocation *allocation, gpointer user_data)
{
	static bool bSkip = false;

	if(g_bWrapTree/* && !bSkip*/){
		update_tree_wrap();
		bSkip = true;
	}
	else
		bSkip = false;
}

void update_tree_wrap()
{
	GtkWidget *scrolledwindow1 = lookup_widget(window1, "scrolledwindow1");
	GtkAdjustment *hadjustment = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW(scrolledwindow1));

	int nWidth = scrolledwindow1->allocation.width; //allocation->width

	TRACE("Wrap tree to %d\n", nWidth);
	g_tree.WrapTreeToSize(nWidth);

	//set horz scrollbar width

	gdouble  nValue = 0;
	g_object_get (G_OBJECT (hadjustment), "upper", &nValue, (char *)NULL);
	TRACE("Adj: %f\n", nValue);

	//g_signal_handlers_block_by_func(scrolledwindow1, (void *)on_tree_resize, 0);

	nValue = nWidth;
	//TOFIX crashes on Ubuntu g_object_set (G_OBJECT (hadjustment), "upper", &nValue, NULL);

	//gtk_adjustment_changed(hadjustment);
	//g_signal_handlers_unblock_by_func(scrolledwindow1, (void *)on_tree_resize, 0);
}
