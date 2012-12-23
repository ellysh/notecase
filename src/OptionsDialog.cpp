////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Window class for Options dialog
////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
 #if _MSC_VER > 1000
  #pragma warning(disable:4786)
 #endif
 #include "_win/bootstart.h" //register program loading at boot time
 #ifndef __MINGW32__
   #ifndef snprintf
    #define snprintf _snprintf
   #endif
 #endif
 #include <io.h> //access
 #define access _access
#endif

#include "OptionsDialog.h"
#include "lib/IniFile.h"
#include "support.h"
#include "mru.h"
#include "gui/GuiLanguage.h"
#include "lib/FilePath.h"
#include "TreeView.h"
#include "TextView.h"
#include "callbacks.h"
#include "interface.h"
#include "gui/FileDialog.h"
#include "lib/IniFile.h"
#include <stdlib.h>
#include <gdk/gdkkeysyms.h>

#define ICON_NONE    -2
#define ICON_CUSTOM  -1
#define ICON_INTERNAL 0
#define ICON_INTERNAL_FIRST 0
#define ICON_INTERNAL_LAST	5

extern MRU g_objMRU;
extern GtkWidget *window1;
extern GuiLanguage g_lang;
extern TreeView g_tree;
extern TextView g_text;
extern bool g_bMinimizeToTray;
extern bool g_bCloseToTray;
extern bool g_bCloseDlgOnTextEnter;
extern bool g_bAutoSaveOnClose;
extern bool g_bSkipMemOnlyAutosave;
extern bool g_bExportDataAsSeparateFile;
extern bool g_bDrawTextviewExpose;
extern IniFile g_objIni;
extern bool g_bTreeToTheRight;
extern GdkColor g_linkColor;
extern bool g_bWrapTree;

int g_nDefaultNodeIconType = ICON_NONE;
std::string g_strDefaultIconFile;

extern GdkColor g_rgbTextViewBkColor;

void restart_autosave();
void restart_enc_unload_timer();
static void on_autosave_checked (GtkMenuItem *menuitem, gpointer user_data);
static void on_backup_checked (GtkMenuItem *menuitem, gpointer user_data);
static void on_ok_clicked (GtkMenuItem *menuitem, gpointer user_data);
static void on_tabsize_checked (GtkMenuItem *menuitem, gpointer user_data);
static void on_toc_checked (GtkMenuItem *menuitem, gpointer user_data);
static void on_enc_unload_checked (GtkMenuItem *menuitem, gpointer user_data);
static void on_system_locale_checked(GtkMenuItem *menuitem, gpointer user_data);
static void on_icon_none_clicked (GtkMenuItem *menuitem, gpointer user_data);
static void on_icon_internal_clicked (GtkMenuItem *menuitem, gpointer user_data);
static void on_icon_custom_clicked (GtkMenuItem *menuitem, gpointer user_data);
static void on_xpm_select_clicked (GtkMenuItem *menuitem, gpointer user_data);
static void on_internal_combo_selected (GtkComboBox *widget, gpointer user_data);
static gboolean on_numeric_entry_keypress(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
static gboolean on_numeric_entry_changed(GtkWidget *widget, GdkEventFocus *event, gpointer user_data);

void update_tree_wrap();

void RefreshTextViewCaretColor(GdkColor &BkColor);
const char *combo_get_text(GtkWidget *cbo3);
const char *InternalIcon_Index2Name(int nIndex);
const char **InternalIcon_GetFromIdx(int);
int InternalIcon_Name2Index(const char *);
bool copy_file(const char *szFrom, const char *szTo);
bool is_portable_mode();
std::string CalcLauncherPath();

OptionsDialog::OptionsDialog()
{
	Create();
}

OptionsDialog::~OptionsDialog()
{
	Destroy();
	gtk_window_present(GTK_WINDOW(window1)); //activate main window
}

void OptionsDialog::Create()
{
	m_pDialog = create_options_dialog (window1);
	OnDialogInit();
}

void OptionsDialog::OnDialogInit()
{
	GtkWidget *treeview = lookup_widget(window1, "treeview1");
	GtkWidget *textview = lookup_widget(window1, "textview1");

	GtkWidget *bootstart_btn  = lookup_widget(m_pDialog, "bootstart_btn");
	GtkWidget *maximize_btn   = lookup_widget(m_pDialog, "maximize_btn");
	GtkWidget *singleinst_btn = lookup_widget(m_pDialog, "singleinst_btn");
	GtkWidget *reloadlast_btn = lookup_widget(m_pDialog, "reloadlast_btn");
	GtkWidget *regformats_btn = lookup_widget(m_pDialog, "regformats_btn");
	GtkWidget *wraptext_btn	  = lookup_widget(m_pDialog, "wraptext_btn");
	GtkWidget *wraptree_btn	  = lookup_widget(m_pDialog, "wraptree_btn");
	GtkWidget *treelines_btn  = lookup_widget(m_pDialog, "treelines_btn");
	GtkWidget *fontbutton2	  = lookup_widget(m_pDialog, "fontbutton2");
	GtkWidget *fontbutton3	  = lookup_widget(m_pDialog, "fontbutton3");
	GtkWidget *comboboxentry1 = lookup_widget(m_pDialog, "comboboxentry1");
	GtkWidget *comboboxentry2 = lookup_widget(m_pDialog, "comboboxentry2");
	GtkWidget *min2tray_btn   = lookup_widget(m_pDialog, "min2tray_btn");
	GtkWidget *close2tray_btn = lookup_widget(m_pDialog, "close2tray_btn");
	GtkWidget *drawgray_btn = lookup_widget(m_pDialog, "drawgray_btn");
	GtkWidget *enterhandle_btn = lookup_widget(m_pDialog, "enterhandle_btn");
	GtkWidget *last_pos_btn   = lookup_widget(m_pDialog, "last_position_btn");
	GtkWidget *autosave_btn   = lookup_widget(m_pDialog, "autosave_btn");
	GtkWidget *autosave_fld   = lookup_widget(m_pDialog, "autosave_fld");
	GtkWidget *backup_btn	  = lookup_widget(m_pDialog, "backup_btn");
	GtkWidget *backup_fld	  = lookup_widget(m_pDialog, "backup_fld");
	GtkWidget *colorbtn1      = lookup_widget(m_pDialog, "colorbtn1");
	GtkWidget *colorbtn2      = lookup_widget(m_pDialog, "colorbtn2");
	GtkWidget *colorbtn3      = lookup_widget(m_pDialog, "colorbtn3");
	GtkWidget *tab_size_btn	  = lookup_widget(m_pDialog, "custom_tab_size_btn");
	GtkWidget *tab_size_fld   = lookup_widget(m_pDialog, "tabsize_fld");
	GtkWidget *use_toc_btn    = lookup_widget(m_pDialog, "use_toc_btn");
	GtkWidget *toc_depth_entry= lookup_widget(m_pDialog, "toc_depth_entry");
	GtkWidget *convert_image_links_btn = lookup_widget(m_pDialog, "convert_image_links_btn");
	GtkWidget *embedd_css_btn = lookup_widget(m_pDialog, "embedd_css_btn");
	GtkWidget *autosave_on_close_btn = lookup_widget(m_pDialog, "autosave_on_close_btn");
	GtkWidget *export_as_separate_files_btn = lookup_widget(m_pDialog, "export_as_separate_files_btn");
	GtkWidget *tree_right_btn = lookup_widget(m_pDialog, "tree_right_btn");
	GtkWidget *lock_memory_btn = lookup_widget(m_pDialog, "lock_memory_btn");
	GtkWidget *enc_unload_btn = lookup_widget(m_pDialog, "enc_unload_btn");
	GtkWidget *enc_unload_fld = lookup_widget(m_pDialog, "enc_unload_fld");
	GtkWidget *system_locale_btn = lookup_widget(m_pDialog, "system_locale_btn");
	GtkWidget *no_mem_autosave_btn = lookup_widget(m_pDialog, "no_mem_autosave_btn");

	//re-read options from the INI (in case some other instance changed it)
	g_objIni.Load(GetIniFile()); //TOFIX will this override already changed settings?

	bool bBootstart;
	if(g_objIni.GetValue("Startup", "BootStart", bBootstart))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bootstart_btn), bBootstart);

	bool bMaximize;
	if(g_objIni.GetValue("Startup", "Maximize", bMaximize))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(maximize_btn), bMaximize);

	bool bSingleInst;
	if(g_objIni.GetValue("Startup", "AllowSingleInstance", bSingleInst))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(singleinst_btn), bSingleInst);

	bool bLoadLast;
	if(g_objIni.GetValue("Startup", "ReloadLastDocument", bLoadLast))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(reloadlast_btn), bLoadLast);

	bool bRestoreLastPos = true;
	g_objIni.GetValue("Startup", "RestoreLastPosition", bRestoreLastPos, 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(last_pos_btn), bRestoreLastPos);

	bool bRegisterFormats;
	if(g_objIni.GetValue("Startup", "RegisterFormats", bRegisterFormats))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(regformats_btn), bRegisterFormats);

	bool bWrapText;
	if(g_objIni.GetValue("Display", "WrapText", bWrapText))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wraptext_btn), bWrapText);

	bool bWrapTree;
	if(g_objIni.GetValue("Display", "WrapTree", bWrapTree))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wraptree_btn), bWrapTree);

	bool bShowTreeLines;
	if(g_objIni.GetValue("Display", "ShowTreeLines", bShowTreeLines))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(treelines_btn), bShowTreeLines);

	bool bMin2Tray;
	if(g_objIni.GetValue("Display", "MinimizeToTray", bMin2Tray))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(min2tray_btn), bMin2Tray);

	bool bClose2Tray;
	if(g_objIni.GetValue("Display", "CloseToTray", bClose2Tray))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(close2tray_btn), bClose2Tray);

	bool bDrawGray;
	if(g_objIni.GetValue("Display", "DrawGrayTextView", bDrawGray))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(drawgray_btn), bDrawGray);

	bool bCloseOnEnter;
	if(g_objIni.GetValue("Default", "CloseDlgOnTxtEnter", bCloseOnEnter))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(enterhandle_btn), bCloseOnEnter);

	bool bTreeToTheRight;
	if(g_objIni.GetValue("Display", "TreeToTheRight", bTreeToTheRight))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tree_right_btn), bTreeToTheRight);

	//"custom tab size" settings
	int	nCustomTabSize = 0;
	if(g_objIni.GetValue("Display", "CustomTabSize", nCustomTabSize)){
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(tab_size_btn), nCustomTabSize>0);

		char szBuffer1[30];
		snprintf(szBuffer1, sizeof(szBuffer1), "%d", nCustomTabSize);
		gtk_entry_set_text(GTK_ENTRY(tab_size_fld), szBuffer1);
	}

	bool bAutosave;
	if(g_objIni.GetValue("Other", "UseAutosave", bAutosave))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(autosave_btn), bAutosave);

	int nAutosaveTimer;
	g_objIni.GetValue("Other", "AutosaveTimer", nAutosaveTimer, 30);
	char szBuffer[30];
	snprintf(szBuffer, sizeof(szBuffer), "%d", nAutosaveTimer);
	gtk_entry_set_text(GTK_ENTRY(autosave_fld), szBuffer);

	//backup settings
	bool bBackup;
	if(g_objIni.GetValue("Other", "UseBackup", bBackup))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(backup_btn), bBackup);

	int nBackupFiles;
	g_objIni.GetValue("Other", "BackupFiles", nBackupFiles, 0);
	snprintf(szBuffer, sizeof(szBuffer), "%d", nBackupFiles);
	gtk_entry_set_text(GTK_ENTRY(backup_fld), szBuffer);

	//set initial fonts
	std::string strFont1;
	g_objIni.GetValue("Display", "TreeFont", strFont1, "");
	if(strFont1.empty()){
		//copy current font from treeview
		GtkStyle *style = gtk_widget_get_style(treeview);
		strFont1 = pango_font_description_to_string(style->font_desc);
	}
#if GTK_CHECK_VERSION(2,4,0) //define minimal version for this api
	gtk_font_button_set_font_name(GTK_FONT_BUTTON(fontbutton2), strFont1.c_str());
#endif

	std::string strFont2;
	g_objIni.GetValue("Display", "TextFont", strFont2, "");
	if(strFont2.empty()){
		//copy current font from treeview
		GtkStyle *style = gtk_widget_get_style(textview);
		strFont2 = pango_font_description_to_string(style->font_desc);
	}
#if GTK_CHECK_VERSION(2,4,0) //define minimal version for this api
	gtk_font_button_set_font_name(GTK_FONT_BUTTON(fontbutton3), strFont2.c_str());
#endif

	//init text/background colors
	std::string strColor, strColor1;
	g_objIni.GetValue("Display", "TextColor", strColor, "#000000");
	g_objIni.GetValue("Display", "BackgroundColor", strColor1, "#FFFFFF");

	GdkColor color;
	gdk_color_parse (strColor.c_str(), &color);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(colorbtn1), &color);

	GdkColor color1;
	gdk_color_parse (strColor1.c_str(), &color1);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(colorbtn2), &color1);

	std::string strColor2;
	g_objIni.GetValue("Display", "LinkColor", strColor2, "#0000FF");
	GdkColor color2;
	gdk_color_parse (strColor2.c_str(), &color2);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(colorbtn3), &color2);

	//fill combo box with available languages
	std::string strLang;
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboboxentry1), "English (default)");
	g_lang.ListAvailableCatalogs();
	int nLangCount = g_lang.GetCatalogCount();
	int i;
	for(i=0; i<nLangCount; i++)
	{
		//map back from locale to Language name
		strLang = g_lang.GetLangName(g_lang.GetCatalogAt(i).c_str());
		gtk_combo_box_append_text(GTK_COMBO_BOX(comboboxentry1), strLang.c_str());
	}

	//select current saved INI language in the combo (not necessarily current language)
	std::string strLocale;
	g_objIni.GetValue("Display", "Language", strLocale, "");
	int nPos = g_lang.GetLocaleIdx(strLocale.c_str());
	gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxentry1), nPos+1);
	m_strLocale = strLocale;

	bool bUseSystemLang;
	if(g_objIni.GetValue("Display", "UseSystemLanguage", bUseSystemLang))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(system_locale_btn), bUseSystemLang);

	//set selected date/time format
	int nDateTimeFmt = 0;
	g_objIni.GetValue("Display", "DateTimeFormat", nDateTimeFmt, 0);
	if(nDateTimeFmt < 0 || nDateTimeFmt > 3)
		nDateTimeFmt = 0;
	gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxentry2), nDateTimeFmt);

	bool bUseTOC;
	if(g_objIni.GetValue("Export", "UseTOC", bUseTOC))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_toc_btn), bUseTOC);

	int nTocDepth;
	g_objIni.GetValue("Export", "TOCDepth", nTocDepth, 1);
	snprintf(szBuffer, sizeof(szBuffer), "%d", nTocDepth);
	gtk_entry_set_text(GTK_ENTRY(toc_depth_entry), szBuffer);

	bool bConvertImageLinks;
	if(g_objIni.GetValue("Export", "ConvertImageLinks", bConvertImageLinks))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(convert_image_links_btn), bConvertImageLinks);

	bool bEmbedCSS;
	if(g_objIni.GetValue("Export", "EmbedCSS", bEmbedCSS))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(embedd_css_btn), bEmbedCSS);

	if(g_bAutoSaveOnClose)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(autosave_on_close_btn), g_bAutoSaveOnClose);

	if(g_bSkipMemOnlyAutosave)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(no_mem_autosave_btn), g_bSkipMemOnlyAutosave);

	if(g_bExportDataAsSeparateFile)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(export_as_separate_files_btn), g_bExportDataAsSeparateFile);

	bool bLockMemory = false;
	if(g_objIni.GetValue("Security", "LockProcessMemory", bLockMemory))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lock_memory_btn), bLockMemory);

	bool bEncUnload = false;
	if(g_objIni.GetValue("Security", "UseEncUnload", bEncUnload))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(enc_unload_btn), bEncUnload);

	int nEncUnloadTimer;
	g_objIni.GetValue("Security", "EncUnloadTimer", nEncUnloadTimer, 0);
	snprintf(szBuffer, sizeof(szBuffer), "%d", nEncUnloadTimer);
	gtk_entry_set_text(GTK_ENTRY(enc_unload_fld), szBuffer);

	//fill internal icons combo
	GtkWidget *cbo2	= lookup_widget(m_pDialog, "comboboxentry12");
	for (i=ICON_INTERNAL_FIRST; i<=ICON_INTERNAL_LAST; i++)
		gtk_combo_box_append_text(GTK_COMBO_BOX(cbo2), InternalIcon_Index2Name(i));

	//select first entry in combo (must not be empty selection)
	gtk_combo_box_set_active(GTK_COMBO_BOX(cbo2), 0);

	SetIconType(g_nDefaultNodeIconType);
	if(ICON_CUSTOM == g_nDefaultNodeIconType)
		SetIconValue(g_strDefaultIconFile.c_str());

}

void OptionsDialog::OnDialogOK()
{
	GtkWidget *treeview = lookup_widget(window1, "treeview1");
	GtkWidget *textview = lookup_widget(window1, "textview1");

	GtkWidget *bootstart_btn  = lookup_widget(m_pDialog, "bootstart_btn");
	GtkWidget *maximize_btn   = lookup_widget(m_pDialog, "maximize_btn");
	GtkWidget *singleinst_btn = lookup_widget(m_pDialog, "singleinst_btn");
	GtkWidget *reloadlast_btn = lookup_widget(m_pDialog, "reloadlast_btn");
	GtkWidget *regformats_btn = lookup_widget(m_pDialog, "regformats_btn");
	GtkWidget *wraptext_btn	  = lookup_widget(m_pDialog, "wraptext_btn");
	GtkWidget *wraptree_btn	  = lookup_widget(m_pDialog, "wraptree_btn");
	GtkWidget *treelines_btn  = lookup_widget(m_pDialog, "treelines_btn");
	GtkWidget *fontbutton2	  = lookup_widget(m_pDialog, "fontbutton2");
	GtkWidget *fontbutton3	  = lookup_widget(m_pDialog, "fontbutton3");
	GtkWidget *comboboxentry1 = lookup_widget(m_pDialog, "comboboxentry1");
	GtkWidget *comboboxentry2 = lookup_widget(m_pDialog, "comboboxentry2");
	GtkWidget *min2tray_btn   = lookup_widget(m_pDialog, "min2tray_btn");
	GtkWidget *close2tray_btn = lookup_widget(m_pDialog, "close2tray_btn");
	GtkWidget *drawgray_btn = lookup_widget(m_pDialog, "drawgray_btn");
	GtkWidget *enterhandle_btn = lookup_widget(m_pDialog, "enterhandle_btn");
	GtkWidget *last_pos_btn   = lookup_widget(m_pDialog, "last_position_btn");
	GtkWidget *autosave_btn   = lookup_widget(m_pDialog, "autosave_btn");
	GtkWidget *autosave_fld   = lookup_widget(m_pDialog, "autosave_fld");
	GtkWidget *backup_btn	  = lookup_widget(m_pDialog, "backup_btn");
	GtkWidget *backup_fld	  = lookup_widget(m_pDialog, "backup_fld");
	GtkWidget *colorbtn1      = lookup_widget(m_pDialog, "colorbtn1");
	GtkWidget *colorbtn2      = lookup_widget(m_pDialog, "colorbtn2");
	GtkWidget *colorbtn3      = lookup_widget(m_pDialog, "colorbtn3");
	GtkWidget *tab_size_btn	  = lookup_widget(m_pDialog, "custom_tab_size_btn");
	GtkWidget *tab_size_fld   = lookup_widget(m_pDialog, "tabsize_fld");
	GtkWidget *use_toc_btn    = lookup_widget(m_pDialog, "use_toc_btn");
	GtkWidget *toc_depth_entry= lookup_widget(m_pDialog, "toc_depth_entry");
	GtkWidget *convert_image_links_btn = lookup_widget(m_pDialog, "convert_image_links_btn");
	GtkWidget *embedd_css_btn = lookup_widget(m_pDialog, "embedd_css_btn");
	GtkWidget *autosave_on_close_btn = lookup_widget(m_pDialog, "autosave_on_close_btn");
	GtkWidget *export_as_separate_files_btn = lookup_widget(m_pDialog, "export_as_separate_files_btn");
	GtkWidget *tree_right_btn = lookup_widget(m_pDialog, "tree_right_btn");
	GtkWidget *lock_memory_btn = lookup_widget(m_pDialog, "lock_memory_btn");
	GtkWidget *enc_unload_btn = lookup_widget(m_pDialog, "enc_unload_btn");
	GtkWidget *enc_unload_fld = lookup_widget(m_pDialog, "enc_unload_fld");
	GtkWidget *system_locale_btn = lookup_widget(m_pDialog, "system_locale_btn");
	GtkWidget *no_mem_autosave_btn = lookup_widget(m_pDialog, "no_mem_autosave_btn");


	//
	// write options back to the INI (and activate them if necessary)
	//
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(bootstart_btn)))
	{
		g_objIni.SetValue("Startup", "BootStart", 1);
		#ifdef _WIN32
			//register program loading at boot time
			if(!IsBootKeySet("notecase"))
				if(is_portable_mode())
					RunProgramAtBoot("notecase", CalcLauncherPath().c_str(), TRUE);	//remove
				else
					RunProgramAtBoot("notecase", GetAppPath().c_str(), TRUE);	//set
		#else
			//register program loading at boot time
			std::string strDir = GetHomeDir();
			EnsureTerminated(strDir, '/');
			strDir += ".config/autostart/";
			EnsureDirExists(strDir);
			strDir += "notecase.desktop";

			std::string strSrcFile;
			#ifdef _NOKIA_MAEMO
				strSrcFile = "/usr/share/applications/hildon/notecase.desktop";
			#else
				strSrcFile = "/usr/share/applications/notecase.desktop";
			#endif
			//copy file
			copy_file(strSrcFile.c_str(), strDir.c_str()); //TOFIX error report
		#endif
	}
	else
	{
		g_objIni.SetValue("Startup", "BootStart", 0);
		#ifdef _WIN32
			//unregister program loading at boot time
			if(IsBootKeySet("notecase"))
				if(is_portable_mode())
					RunProgramAtBoot("notecase", CalcLauncherPath().c_str(), FALSE);	//remove
				else
					RunProgramAtBoot("notecase", GetAppPath().c_str(), FALSE);	//remove
		#else
			//unregister program loading at boot time
			std::string strDir = GetHomeDir();
			EnsureTerminated(strDir, '/');
			strDir += ".config/autostart/";
			EnsureDirExists(strDir);
			strDir += "notecase.desktop";
			remove(strDir.c_str());
		#endif
	}

	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(maximize_btn)))
		g_objIni.SetValue("Startup", "Maximize", 1);
	else
		g_objIni.SetValue("Startup", "Maximize", 0);

	GtkWidget *new_notecase1 = lookup_widget(window1, "new_notecase1");
	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(singleinst_btn))){
		g_objIni.SetValue("Startup", "AllowSingleInstance", 1);
		gtk_widget_set_sensitive(new_notecase1, FALSE);
	}
	else{
		g_objIni.SetValue("Startup", "AllowSingleInstance", 0);
		gtk_widget_set_sensitive(new_notecase1, TRUE);
	}

	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(reloadlast_btn)))
		g_objIni.SetValue("Startup", "ReloadLastDocument", 1);
	else
		g_objIni.SetValue("Startup", "ReloadLastDocument", 0);

	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(regformats_btn)))
		g_objIni.SetValue("Startup", "RegisterFormats", 1);
	else
		g_objIni.SetValue("Startup", "RegisterFormats", 0);

	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(wraptext_btn)))
	{
		g_objIni.SetValue("Display", "WrapText", 1);
		//if default changed, change the current state also
		if(!g_text.IsWrapped()){
			set_wrap_activated(true);
		}
	}
	else
	{
		g_objIni.SetValue("Display", "WrapText", 0);
		//if default changed, change the current state also
		if(g_text.IsWrapped()){
			set_wrap_activated(false);
		}
	}

	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(wraptree_btn)))
	{
		g_objIni.SetValue("Display", "WrapTree", 1);
		g_bWrapTree=true;
		update_tree_wrap();
	}
	else
	{
		g_objIni.SetValue("Display", "WrapTree", 0);
		g_tree.WrapTreeToSize(-1);
		g_bWrapTree=false;
	}

	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(treelines_btn)))
	{
		g_objIni.SetValue("Display", "ShowTreeLines", 1);
		g_tree.SetTreeLines(true);
	}
	else
	{
		g_objIni.SetValue("Display", "ShowTreeLines", 0);
		g_tree.SetTreeLines(false);
	}

#if GTK_CHECK_VERSION(2,4,0) //define minimal version for this api
	//set fonts
	const gchar *szFont1 = gtk_font_button_get_font_name(GTK_FONT_BUTTON(fontbutton2));
	PangoFontDescription *desc1 = pango_font_description_from_string(szFont1);
	gtk_widget_modify_font(treeview, desc1);
	g_objIni.SetValue("Display", "TreeFont", szFont1); //save font description

	const gchar *szFont2 = gtk_font_button_get_font_name(GTK_FONT_BUTTON(fontbutton3));
	PangoFontDescription *desc2 = pango_font_description_from_string(szFont2);
	gtk_widget_modify_font(textview, desc2);
	g_objIni.SetValue("Display", "TextFont", szFont2);//save font description
#endif

	//save text/background colors
	GdkColor color;
	gtk_color_button_get_color(GTK_COLOR_BUTTON(colorbtn1), &color);
	gchar *szColor = g_strdup_printf ("#%04x%04x%04x", color.red, color.green, color.blue);
	g_objIni.SetValue("Display", "TextColor", szColor);
	g_free(szColor);

	GdkColor color1;
	gtk_color_button_get_color(GTK_COLOR_BUTTON(colorbtn2), &color1);
	szColor = g_strdup_printf ("#%04x%04x%04x", color1.red, color1.green, color1.blue);
	g_objIni.SetValue("Display", "BackgroundColor", szColor);
	g_free(szColor);
	g_rgbTextViewBkColor = color1;

	//update widgets with new color
	gtk_widget_modify_text(GTK_WIDGET(textview), GTK_STATE_NORMAL, &color);
	gtk_widget_modify_text(GTK_WIDGET(treeview), GTK_STATE_NORMAL, &color);
	gtk_widget_modify_base(GTK_WIDGET(textview), GTK_STATE_NORMAL, &color1);
	gtk_widget_modify_base(GTK_WIDGET(textview), GTK_STATE_INSENSITIVE, &color1);
	gtk_widget_modify_base(GTK_WIDGET(treeview), GTK_STATE_NORMAL, &color1);
	RefreshTextViewCaretColor(color1);

	//fix for GTK bug - tree view loses background color
	static const char color_rc[] =
		"style \"aaa\"\n"
		"{\n"
		"GtkTreeView::odd-row-color=\"#%04x%04x%04x\"\n"
		"GtkTreeView::even-row-color =\"#%04x%04x%04x\"\n"
		"}\n"
		"widget \"*.aaa\" style \"aaa\"";

	gchar *rc_temp = g_strdup_printf (color_rc,
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

	//save link color
	GdkColor color2;
	gtk_color_button_get_color(GTK_COLOR_BUTTON(colorbtn3), &color2);
	szColor = g_strdup_printf ("#%04x%04x%04x", color2.red, color2.green, color2.blue);
	g_objIni.SetValue("Display", "LinkColor", szColor);
	g_free(szColor);
	g_linkColor = color2;

	//save selected language locale
	std::string strNewLocale;
	int nRes = gtk_combo_box_get_active(GTK_COMBO_BOX(comboboxentry1));
	if(nRes > 0) //first item in the list is "Default"
		strNewLocale = g_lang.GetCatalogAt(nRes-1).c_str();
	g_objIni.SetValue("Display", "Language", strNewLocale.c_str());

	bool bUseSystemLang;
	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(system_locale_btn)))
		bUseSystemLang = true;
	else
		bUseSystemLang = false;
	g_objIni.SetValue("Display", "UseSystemLanguage", bUseSystemLang);

	//save selected date/tiem format
	int nDateTimeFmt = gtk_combo_box_get_active(GTK_COMBO_BOX(comboboxentry2));
	g_objIni.SetValue("Display", "DateTimeFormat", nDateTimeFmt);

	//"minimize to tray" setting
	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(min2tray_btn)))
		g_bMinimizeToTray = true;
	else
		g_bMinimizeToTray = false;
	g_objIni.SetValue("Display", "MinimizeToTray", g_bMinimizeToTray);

	//"close to tray" setting
	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(close2tray_btn)))
		g_bCloseToTray = true;
	else
		g_bCloseToTray = false;
	g_objIni.SetValue("Display", "CloseToTray", g_bCloseToTray);

	//"draw gray" setting
	bool bDrawGray;
	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(drawgray_btn)))
		bDrawGray = true;
	else
		bDrawGray = false;
	g_objIni.SetValue("Display", "DrawGrayTextView", bDrawGray);

	if(bDrawGray != g_bDrawTextviewExpose)
		gtkMessageBox(_("Some of the changed options require program restart!"));

	//how to handle enter key in text widgets
	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(enterhandle_btn)))
		g_bCloseDlgOnTextEnter = true;
	else
		g_bCloseDlgOnTextEnter = false;
	g_objIni.SetValue("Default", "CloseDlgOnTxtEnter", g_bCloseDlgOnTextEnter);

	//put node tree to the right side
	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(tree_right_btn)))
		g_bTreeToTheRight = true;
	else
		g_bTreeToTheRight = false;
	g_objIni.SetValue("Display", "TreeToTheRight", g_bTreeToTheRight);

	//"restore position" settings
	bool bRestoreLastPos = false;
	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(last_pos_btn)))
		bRestoreLastPos = true;
	g_objIni.SetValue("Startup", "RestoreLastPosition", bRestoreLastPos);

	//"auto save on close" setting
	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(autosave_on_close_btn)))
		g_bAutoSaveOnClose = true;
	else
		g_bAutoSaveOnClose = false;
	g_objIni.SetValue("Save", "AutoSaveOnClose", g_bAutoSaveOnClose);

	//"skip auto save for mem-only docs" setting
	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(no_mem_autosave_btn)))
		g_bSkipMemOnlyAutosave = true;
	else
		g_bSkipMemOnlyAutosave = false;
	g_objIni.SetValue("Save",    "SkipMemOnlyAutoSave", g_bSkipMemOnlyAutosave);

	//"export image and attachment data as separate file" setting
	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(export_as_separate_files_btn)))
		g_bExportDataAsSeparateFile = true;
	else
		g_bExportDataAsSeparateFile = false;
	g_objIni.SetValue("Export", "ExportEmbeddedAsSeparateFile", g_bExportDataAsSeparateFile);

	//"custom tab size" settings
	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(tab_size_btn)))
	{
		g_objIni.SetValue("Display", "CustomTabSize", gtk_entry_get_text(GTK_ENTRY(tab_size_fld)));

		//set custom tab size
		int	nCustomTabSize = 0;
		g_objIni.GetValue("Display", "CustomTabSize", nCustomTabSize, 0);
		if(nCustomTabSize > 0)
		{
			int nWidthPixels = g_text.CalcTabWidthInPixels(nCustomTabSize);
			g_text.SetTabWidth(nWidthPixels);
		}
	}
	else{
		g_objIni.SetValue("Display", "CustomTabSize", 0);

		//set default tab size
		int nWidthPixels = g_text.CalcTabWidthInPixels(4);
		g_text.SetTabWidth(nWidthPixels);
	}

	//autosave settings
	bool bAutosave = false;
	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(autosave_btn)))
		bAutosave = true;
	g_objIni.SetValue("Other", "UseAutosave", bAutosave);
	g_objIni.SetValue("Other", "AutosaveTimer", gtk_entry_get_text(GTK_ENTRY(autosave_fld)));

	//backup settings
	bool bBackup = false;
	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(backup_btn)))
		bBackup = true;
	g_objIni.SetValue("Other", "UseBackup", bBackup);
	g_objIni.SetValue("Other", "BackupFiles", gtk_entry_get_text(GTK_ENTRY(backup_fld)));

	bool bUseTOC = false;
	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(use_toc_btn)))
		bUseTOC = true;
	g_objIni.SetValue("Export", "UseTOC", bUseTOC);
	g_objIni.SetValue("Export", "TOCDepth", gtk_entry_get_text(GTK_ENTRY(toc_depth_entry)));

	bool bConvertImageLinks = false;
	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(convert_image_links_btn)))
		bConvertImageLinks = true;
	g_objIni.SetValue("Export", "ConvertImageLinks", bConvertImageLinks);

	bool bEmbedCSS = false;
	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(embedd_css_btn)))
		bEmbedCSS = true;
	g_objIni.SetValue("Export", "EmbedCSS", bEmbedCSS);

	bool bLockMemory = false;
	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(lock_memory_btn)))
		bLockMemory = true;
	g_objIni.SetValue("Security", "LockProcessMemory", bLockMemory);

	bool bEncUnload = false;
	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(enc_unload_btn)))
		bEncUnload = true;
	g_objIni.SetValue("Security", "UseEncUnload", bEncUnload);
	g_objIni.SetValue("Security", "EncUnloadTimer", gtk_entry_get_text(GTK_ENTRY(enc_unload_fld)));


	//set icon
	g_nDefaultNodeIconType	= GetIconType();
	g_strDefaultIconFile	= GetIconValue();
	if(g_nDefaultNodeIconType == ICON_INTERNAL_FIRST)
	{
		g_nDefaultNodeIconType += InternalIcon_Name2Index(g_strDefaultIconFile.c_str());
	}
	g_objIni.SetValue("Default", "NodeIconType", g_nDefaultNodeIconType);
	g_objIni.SetValue("Security", "NodeIconValue", g_strDefaultIconFile.c_str());

	//save new settings to INI
	g_objIni.Save();

	restart_autosave();
	restart_enc_unload_timer();

	//if language selection changed, issue the warning
	if(0 != strcmp(m_strLocale.c_str(), strNewLocale.c_str()))
	{
		GtkWidget* msgbox;
		msgbox = gtk_message_dialog_new ( (GtkWindow*)m_pDialog,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			(GtkMessageType)GTK_MESSAGE_INFO,
			(GtkButtonsType)GTK_BUTTONS_OK,
			_("You must restart NoteCase for this change to take effect!"));
		gtk_dialog_run (GTK_DIALOG (msgbox));
		gtk_widget_destroy (msgbox);
	}
}

GtkWidget* OptionsDialog::create_options_dialog (GtkWidget* parent)
{
	GtkWidget *Options;
	GtkWidget *dialog_vbox3;
	GtkWidget *notebook1;
	GtkWidget *vbox4;
	GtkWidget *checkbutton0;
	GtkWidget *checkbutton1;
	GtkWidget *checkbutton2;
	GtkWidget *checkbutton3;
	GtkWidget *checkbutton4;
	GtkWidget *checkbutton7;
	GtkWidget *checkbutton9;
	GtkWidget *label5;
	GtkWidget *label3;
	GtkWidget *dialog_action_area3;
	GtkWidget *cancelbutton3;
	GtkWidget *okbutton3;
	GtkWidget *table1;
	GtkWidget *fontbutton2;
	GtkWidget *fontbutton3;
	GtkWidget *checkbutton5;
	GtkWidget *label6;
	GtkWidget *label7;
	GtkWidget *comboboxentry1;
	GtkWidget *label8;
	GtkWidget *checkbutton6;
	GtkWidget *label19;
	GtkWidget *label20;
	GtkWidget *table2;
	GtkWidget *checkbutton8;
	GtkWidget *entry1;
	GtkWidget *label21;
	GtkWidget *label22;
	GtkWidget *colorbtn1;
	GtkWidget *colorbtn2;
	GtkWidget *colorbtn3;
	GtkWidget *entry2;
	GtkWidget *label25;
	GtkWidget *comboboxentry2;
	GtkWidget *table3;
	GtkWidget *checkbutton10;
	GtkWidget *entry3;
	GtkWidget *checkbutton11;
	GtkWidget *checkbutton12;
	GtkWidget *checkbutton13;
	GtkWidget *checkbutton14;
	GtkWidget *checkbutton15;
	GtkWidget *checkbutton16;
	GtkWidget *checkbutton17;
	GtkWidget *checkbutton18;
	GtkWidget *entry4;
	GtkWidget *entry5;
	GtkWidget *checkbutton19;
	GtkWidget *checkbutton22;
	GtkWidget *checkbutton23;
	GtkWidget *checkbutton24;
	GtkWidget *checkbutton27;
	GtkWidget *checkbutton30;

//
	GtkWidget *button_xpm_select;
	GtkWidget *image1;
	GtkWidget *table6;
	GtkWidget *radio_ico_none;
	GSList    *radio_ico_none_group = NULL;
	GtkWidget *radio_ico_internal;
	GtkWidget *radio_ico_custom;
	GtkWidget *entry14;
	GtkWidget *label23;
	GtkWidget *comboboxentry12;
	GtkWidget *checkbutton32;

	Options = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (Options), _("Options"));
	gtk_window_set_modal (GTK_WINDOW (Options), TRUE);
	gtk_window_set_skip_pager_hint (GTK_WINDOW (Options), TRUE);
	gtk_window_set_type_hint (GTK_WINDOW (Options), GDK_WINDOW_TYPE_HINT_DIALOG);
#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (Options), TRUE);
#endif
	if(parent)
		gtk_window_set_transient_for(GTK_WINDOW (Options), GTK_WINDOW(parent));   //set parent

#if GTK_CHECK_VERSION(2,4,0) //new API TOFIX set proper version
	#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
		//gtk_window_set_keep_above(GTK_WINDOW (Options), TRUE);
	#endif
#endif
	gtk_window_set_destroy_with_parent (GTK_WINDOW (Options), TRUE);
	gtk_window_set_resizable (GTK_WINDOW (Options), FALSE);
	gtk_widget_realize(Options);
	gdk_window_set_decorations(Options->window, (GdkWMDecoration)(GDK_DECOR_BORDER|GDK_DECOR_TITLE));

	dialog_vbox3 = GTK_DIALOG (Options)->vbox;
	gtk_widget_show (dialog_vbox3);

	notebook1 = gtk_notebook_new ();
	gtk_widget_show (notebook1);
	gtk_box_pack_start (GTK_BOX (dialog_vbox3), notebook1, TRUE, TRUE, 0);

	//
	// 1st tab
	//

	vbox4 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox4);
	gtk_container_add (GTK_CONTAINER (notebook1), vbox4);

	checkbutton0 = gtk_check_button_new_with_mnemonic (_("Start at logon"));
	gtk_widget_show (checkbutton0);
	gtk_box_pack_start (GTK_BOX (vbox4), checkbutton0, FALSE, FALSE, 0);

	checkbutton1 = gtk_check_button_new_with_mnemonic (_("Maximize on startup"));
	gtk_widget_show (checkbutton1);
	gtk_box_pack_start (GTK_BOX (vbox4), checkbutton1, FALSE, FALSE, 0);

	checkbutton2 = gtk_check_button_new_with_mnemonic (_("Allow single instance only"));
	gtk_widget_show (checkbutton2);
	gtk_box_pack_start (GTK_BOX (vbox4), checkbutton2, FALSE, FALSE, 0);

	checkbutton3 = gtk_check_button_new_with_mnemonic (_("Reload last used document"));
	gtk_widget_show (checkbutton3);
	gtk_box_pack_start (GTK_BOX (vbox4), checkbutton3, FALSE, FALSE, 0);

	checkbutton4 = gtk_check_button_new_with_mnemonic (_("Register document formats"));
	gtk_widget_show (checkbutton4);
	gtk_box_pack_start (GTK_BOX (vbox4), checkbutton4, FALSE, FALSE, 0);

	checkbutton7 = gtk_check_button_new_with_mnemonic (_("Restore last position/size"));
	gtk_widget_show (checkbutton7);
	gtk_box_pack_start (GTK_BOX (vbox4), checkbutton7, FALSE, FALSE, 0);

	label5 = gtk_label_new (_("Startup"));
	gtk_widget_show (label5);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), label5);

	//
	// 2nd tab
	//

	table1 = gtk_table_new (2, 7, FALSE);
	gtk_widget_show (table1);
	gtk_container_add (GTK_CONTAINER (notebook1), table1);

	label6 = gtk_label_new (_("Tree font:"));
	gtk_widget_show (label6);
	gtk_table_attach (GTK_TABLE (table1), label6, 0, 1, 0, 1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 2, 0);
	gtk_misc_set_alignment (GTK_MISC (label6), 0, 0.5);

	label7 = gtk_label_new (_("Text font:"));
	gtk_widget_show (label7);
	gtk_table_attach (GTK_TABLE (table1), label7, 0, 1, 1, 2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 2, 0);
	gtk_misc_set_alignment (GTK_MISC (label7), 0, 0.5);

#if GTK_CHECK_VERSION(2,4,0) //define minimal version for this api
	fontbutton2 = gtk_font_button_new ();
	gtk_widget_show (fontbutton2);
	gtk_table_attach (GTK_TABLE (table1), fontbutton2, 1, 3, 0, 1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 1);
	gtk_font_button_set_show_style(GTK_FONT_BUTTON(fontbutton2), TRUE);
	gtk_font_button_set_show_size(GTK_FONT_BUTTON(fontbutton2), TRUE);
	gtk_font_button_set_use_font(GTK_FONT_BUTTON(fontbutton2), TRUE);
	gtk_font_button_set_use_size(GTK_FONT_BUTTON(fontbutton2), TRUE);

	fontbutton3 = gtk_font_button_new ();
	gtk_widget_show (fontbutton3);
	gtk_table_attach (GTK_TABLE (table1), fontbutton3, 1, 3, 1, 2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 1);
	gtk_font_button_set_show_style(GTK_FONT_BUTTON(fontbutton3), TRUE);
	gtk_font_button_set_show_size(GTK_FONT_BUTTON(fontbutton3), TRUE);
	gtk_font_button_set_use_font(GTK_FONT_BUTTON(fontbutton3), TRUE);
	gtk_font_button_set_use_size(GTK_FONT_BUTTON(fontbutton3), TRUE);
#endif

	label21	= gtk_label_new (_("Text/background color:"));
	gtk_widget_show (label21);
	gtk_table_attach (GTK_TABLE (table1), label21, 0, 1, 4, 5, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 2, 0);
	gtk_misc_set_alignment (GTK_MISC (label21), 0, 0.5);

	colorbtn1 = gtk_color_button_new();
	gtk_widget_show (colorbtn1);
	gtk_table_attach (GTK_TABLE (table1), colorbtn1, 1, 2, 4, 5, (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 1);

	colorbtn2 = gtk_color_button_new();
	gtk_widget_show (colorbtn2);
	gtk_table_attach (GTK_TABLE (table1), colorbtn2, 2, 3, 4, 5, (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 1);

	label22	= gtk_label_new (_("Link color:"));
	gtk_widget_show (label22);
	gtk_table_attach (GTK_TABLE (table1), label22, 0, 1, 5, 6, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 2, 0);
	gtk_misc_set_alignment (GTK_MISC (label22), 0, 0.5);

	colorbtn3 = gtk_color_button_new();
	gtk_widget_show (colorbtn3);
	gtk_table_attach (GTK_TABLE (table1), colorbtn3, 1, 2, 5, 6, (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 1);

	checkbutton9 = gtk_check_button_new_with_mnemonic (_("Custom Tab Size"));
	gtk_widget_show (checkbutton9);
	gtk_table_attach (GTK_TABLE (table1), checkbutton9, 0, 1, 6, 7, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	entry2 = gtk_entry_new ();
	gtk_widget_show (entry2);
	gtk_widget_set_sensitive (entry2, FALSE);
	gtk_table_attach (GTK_TABLE (table1), entry2, 1, 3, 6, 7, (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 2);

	checkbutton5 = gtk_check_button_new_with_mnemonic (_("Wrap text"));
	gtk_widget_show (checkbutton5);
	gtk_table_attach (GTK_TABLE (table1), checkbutton5, 0, 1, 7, 8, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	checkbutton27 = gtk_check_button_new_with_mnemonic (_("Wrap titles tree"));
	gtk_widget_show (checkbutton27);
	gtk_table_attach (GTK_TABLE (table1), checkbutton27, 1, 2, 7, 8, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	checkbutton30 = gtk_check_button_new_with_mnemonic (_("Show tree lines"));
	gtk_widget_show (checkbutton30);
	gtk_table_attach (GTK_TABLE (table1), checkbutton30, 2, 3, 7, 8, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label3 = gtk_label_new (_("Display"));
	gtk_widget_show (label3);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), label3);

	// new display1 tab

	table1 = gtk_table_new (2, 7, FALSE);
	gtk_widget_show (table1);
	gtk_container_add (GTK_CONTAINER (notebook1), table1);

	checkbutton22 = gtk_check_button_new_with_mnemonic (_("Use system language settings"));
	gtk_widget_show (checkbutton22);
	gtk_table_attach (GTK_TABLE (table1), checkbutton22, 0, 2, 0, 1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label8	= gtk_label_new (_("Language:"));
	gtk_widget_show (label8);
	gtk_table_attach (GTK_TABLE (table1), label8, 0, 1, 1, 2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 2, 0);
	gtk_misc_set_alignment (GTK_MISC (label8), 0, 0.5);

	comboboxentry1 = gtk_combo_box_new_text ();
	gtk_widget_show (comboboxentry1);
	gtk_table_attach (GTK_TABLE (table1), comboboxentry1, 1, 3, 1, 2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 1);

	label25	= gtk_label_new (_("Date/time format:"));
	gtk_widget_show (label25);
	gtk_table_attach (GTK_TABLE (table1), label25, 0, 1, 2, 3, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 2, 0);
	gtk_misc_set_alignment (GTK_MISC (label25), 0, 0.5);

	comboboxentry2  = gtk_combo_box_new_text ();
	gtk_widget_show (comboboxentry2);
	gtk_table_attach (GTK_TABLE (table1), comboboxentry2, 1, 3, 2, 3, (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 1);

	gtk_combo_box_append_text(GTK_COMBO_BOX(comboboxentry2), "YYYY-MM-DD");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboboxentry2), "YYYY-MM-DD hh:mm:ss");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboboxentry2), "DD.MM.YYYY");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboboxentry2), "DD.MM.YYYY hh:mm:ss");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboboxentry2), "hh:mm:ss");

	checkbutton15 = gtk_check_button_new_with_mnemonic (_("Show node tree on the right"));
	gtk_widget_show (checkbutton15);
	gtk_table_attach (GTK_TABLE (table1), checkbutton15, 0, 1, 3, 4, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	checkbutton32 = gtk_check_button_new_with_mnemonic (_("Draw message over inactive text view"));
	gtk_widget_show (checkbutton32);
	gtk_table_attach (GTK_TABLE (table1), checkbutton32, 1, 2, 3, 4, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	checkbutton6 = gtk_check_button_new_with_mnemonic (_("Minimize to tray"));
	gtk_widget_show (checkbutton6);
	gtk_table_attach (GTK_TABLE (table1), checkbutton6, 0, 2, 4, 5, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	checkbutton19 = gtk_check_button_new_with_mnemonic (_("Close to tray"));
	gtk_widget_show (checkbutton19);
	gtk_table_attach (GTK_TABLE (table1), checkbutton19, 0, 2, 5, 6, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	checkbutton24 = gtk_check_button_new_with_mnemonic (_("[Enter] key press in text area closes the window"));
	gtk_widget_show (checkbutton24);
	gtk_table_attach (GTK_TABLE (table1), checkbutton24, 0, 2, 6, 7, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label3 = gtk_label_new (_("Global"));
	gtk_widget_show (label3);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 2), label3);

	//
	// 3rd tab
	//

	table2 = gtk_table_new (2, 5, FALSE);
	gtk_widget_show (table2);
	gtk_container_add (GTK_CONTAINER (notebook1), table2);

	checkbutton8 = gtk_check_button_new_with_mnemonic (_("Autosave to temporary crash-protection copy every:"));
	gtk_widget_show (checkbutton8);
	gtk_table_attach (GTK_TABLE (table2), checkbutton8, 0, 2, 0, 1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	entry1 = gtk_entry_new ();
	gtk_widget_show (entry1);
	gtk_widget_set_sensitive (entry1, FALSE);
	gtk_table_attach (GTK_TABLE (table2), entry1, 0, 1, 1, 2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 2, 0);
	g_signal_connect (entry1, "key-press-event", G_CALLBACK (on_numeric_entry_keypress), this);
	g_signal_connect (entry1, "focus-out-event", G_CALLBACK (on_numeric_entry_changed), GINT_TO_POINTER(30));

	label20 = gtk_label_new (_("seconds"));
	gtk_widget_show (label20);
	gtk_table_attach (GTK_TABLE (table2), label20, 1, 2, 1, 2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 3, 0);
	gtk_misc_set_alignment (GTK_MISC (label20), 0, 0.5);

	checkbutton23 = gtk_check_button_new_with_mnemonic (_("Do not autosave memory-only (new/unsaved) documents (data security)"));
	gtk_widget_show (checkbutton23);
	gtk_widget_set_sensitive (checkbutton23, FALSE);
	gtk_table_attach (GTK_TABLE (table2), checkbutton23, 0, 2, 2, 3, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	checkbutton18 = gtk_check_button_new_with_mnemonic (_("For each document, create up to:"));
	gtk_widget_show (checkbutton18);
	gtk_table_attach (GTK_TABLE (table2), checkbutton18, 0, 2, 3, 4, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	entry5 = gtk_entry_new ();
	gtk_widget_show (entry5);
	gtk_widget_set_sensitive (entry5, FALSE);
	gtk_table_attach (GTK_TABLE (table2), entry5, 0, 1, 4, 5, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 2, 0);
	g_signal_connect (entry5, "key-press-event", G_CALLBACK (on_numeric_entry_keypress), this);
	g_signal_connect (entry5, "focus-out-event", G_CALLBACK (on_numeric_entry_changed), GINT_TO_POINTER(1));

	label20 = gtk_label_new (_("backup files"));
	gtk_widget_show (label20);
	gtk_table_attach (GTK_TABLE (table2), label20, 1, 2, 4, 5, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 3, 0);
	gtk_misc_set_alignment (GTK_MISC (label20), 0, 0.5);

	checkbutton13 = gtk_check_button_new_with_mnemonic (_("Automatically save when closing application"));
	gtk_widget_show (checkbutton13);
	gtk_table_attach (GTK_TABLE (table2), checkbutton13, 0, 2, 5, 6, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label19 = gtk_label_new (_("Save"));
	gtk_widget_show (label19);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 3), label19);

	//
	// 4th tab
	//

	table3 = gtk_table_new (2, 5, FALSE);
	gtk_widget_show (table3);
	gtk_container_add (GTK_CONTAINER (notebook1), table3);

	checkbutton10 = gtk_check_button_new_with_mnemonic (_("Generate table of contents (depth):"));
	gtk_widget_show (checkbutton10);
	gtk_table_attach (GTK_TABLE (table3), checkbutton10, 0, 2, 0, 1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	entry3 = gtk_entry_new ();
	gtk_widget_show (entry3);
	gtk_widget_set_sensitive (entry3, FALSE);
	gtk_table_attach (GTK_TABLE (table3), entry3, 0, 1, 1, 2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 2, 0);
	g_signal_connect (entry3, "key-press-event", G_CALLBACK (on_numeric_entry_keypress), this);
	g_signal_connect (entry3, "focus-out-event", G_CALLBACK (on_numeric_entry_changed), GINT_TO_POINTER(1));

	checkbutton11 = gtk_check_button_new_with_mnemonic (_("Export link to image file as the image tag"));
	gtk_widget_show (checkbutton11);
	gtk_table_attach (GTK_TABLE (table3), checkbutton11, 0, 2, 2, 3, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	checkbutton12 = gtk_check_button_new_with_mnemonic (_("Embed CSS into exported file"));
	gtk_widget_show (checkbutton12);
	gtk_table_attach (GTK_TABLE (table3), checkbutton12, 0, 2, 3, 4, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	checkbutton14 = gtk_check_button_new_with_mnemonic (_("Export image and attachment data as separate files"));
	gtk_widget_show (checkbutton14);
	gtk_table_attach (GTK_TABLE (table3), checkbutton14, 0, 2, 4, 5, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label19 = gtk_label_new (_("Html export"));
	gtk_widget_show (label19);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 4), label19);

	//
	// 5th tab
	//

	table1 = gtk_table_new (2, 7, FALSE);
	gtk_widget_show (table1);
	gtk_container_add (GTK_CONTAINER (notebook1), table1);

	checkbutton16 = gtk_check_button_new_with_mnemonic (_("Protect memory space from swapping (requires Notecase restart)"));
	gtk_widget_show (checkbutton16);
	gtk_table_attach (GTK_TABLE (table1), checkbutton16, 0, 2, 0, 1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
#ifdef _WIN32
	gtk_widget_set_sensitive(checkbutton16, FALSE);
#endif

	label20 = gtk_label_new (_("Security"));
	gtk_widget_show (label20);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 5), label20);

	checkbutton17 = gtk_check_button_new_with_mnemonic (_("Save and unload encrypted document if inactive for:"));
	gtk_widget_show (checkbutton17);
	gtk_table_attach (GTK_TABLE (table1), checkbutton17, 0, 2, 1, 2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	entry4 = gtk_entry_new ();
	gtk_widget_show (entry4);
	gtk_widget_set_sensitive (entry4, FALSE);
	gtk_table_attach (GTK_TABLE (table1), entry4, 0, 1, 2, 3, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 2, 0);
	g_signal_connect (entry4, "key-press-event", G_CALLBACK (on_numeric_entry_keypress), this);
	g_signal_connect (entry4, "focus-out-event", G_CALLBACK (on_numeric_entry_changed), GINT_TO_POINTER(1));

	label20 = gtk_label_new (_("minutes"));
	gtk_widget_show (label20);
	gtk_table_attach (GTK_TABLE (table1), label20, 1, 2, 2, 3, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 3, 0);
	gtk_misc_set_alignment (GTK_MISC (label20), 0, 0.5);

	//
	// 6th tab
	//

	table6 = gtk_table_new (4, 3, FALSE);
	gtk_widget_show (table6);
	gtk_container_add (GTK_CONTAINER (notebook1), table6);

	label23 = gtk_label_new (_("Default Note Icon"));
	gtk_widget_show (label23);
	gtk_table_attach (GTK_TABLE (table6), label23, 0, 3, 0, 1,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 2, 0);
	gtk_misc_set_alignment (GTK_MISC (label23), 0, 0.5);

	radio_ico_none = gtk_radio_button_new_with_mnemonic (NULL, _("None"));
	gtk_widget_show (radio_ico_none);
	gtk_table_attach (GTK_TABLE (table6), radio_ico_none, 0, 1, 1, 2,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (radio_ico_none), radio_ico_none_group);
	radio_ico_none_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio_ico_none));

	radio_ico_internal = gtk_radio_button_new_with_mnemonic (NULL, _("Internal"));
	gtk_widget_show (radio_ico_internal);
	gtk_table_attach (GTK_TABLE (table6), radio_ico_internal, 0, 1, 2, 3,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (radio_ico_internal), radio_ico_none_group);
	radio_ico_none_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio_ico_internal));

	radio_ico_custom = gtk_radio_button_new_with_mnemonic (NULL, _("Custom"));
	gtk_widget_show (radio_ico_custom);
	gtk_table_attach (GTK_TABLE (table6), radio_ico_custom, 0, 1, 3, 4,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (radio_ico_custom), radio_ico_none_group);
	radio_ico_none_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio_ico_custom));

	comboboxentry12 = gtk_combo_box_new_text ();
	gtk_widget_show (comboboxentry12);
	gtk_table_attach (GTK_TABLE (table6), comboboxentry12, 1, 3, 2, 3,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (GTK_FILL), 0, 0);

	entry14 = gtk_entry_new ();
	gtk_entry_set_editable (GTK_ENTRY(entry14), FALSE);
	gtk_widget_show (entry14);
	gtk_table_attach (GTK_TABLE (table6), entry14, 1, 2, 3, 4,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);

	button_xpm_select = gtk_button_new_with_mnemonic ("...");
	gtk_widget_show (button_xpm_select);
	gtk_table_attach (GTK_TABLE (table6), button_xpm_select, 2, 3, 3, 4,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);

	label23 = gtk_label_new (_("Preview:"));
	gtk_widget_show (label23);
	gtk_table_attach (GTK_TABLE (table6), label23, 0, 1, 5, 6,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 2, 0);
	gtk_misc_set_alignment (GTK_MISC (label23), 0, 0.5);

	image1 = gtk_image_new ();
	gtk_widget_show (image1);
	gtk_widget_set_size_request(image1, -1, 20);
	gtk_table_attach (GTK_TABLE (table6), image1, 1, 2, 5, 6,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (GTK_FILL), 0, 0);

	label20 = gtk_label_new (_("Default"));
	gtk_widget_show (label20);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 6), label20);

	//
	// action area
	//

	dialog_action_area3 = GTK_DIALOG (Options)->action_area;
	gtk_widget_show (dialog_action_area3);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area3), GTK_BUTTONBOX_END);

	cancelbutton3 = gtk_button_new_from_stock ("gtk-cancel");
	gtk_widget_show (cancelbutton3);
	gtk_dialog_add_action_widget (GTK_DIALOG (Options), cancelbutton3, GTK_RESPONSE_CANCEL);
	GTK_WIDGET_SET_FLAGS (cancelbutton3, GTK_CAN_DEFAULT);

	okbutton3 = gtk_button_new_from_stock ("gtk-ok");
	gtk_widget_show (okbutton3);
	gtk_container_add (GTK_CONTAINER(dialog_action_area3), okbutton3);
	GTK_WIDGET_SET_FLAGS (okbutton3, GTK_CAN_DEFAULT);

	g_signal_connect (okbutton3, "clicked",	G_CALLBACK (on_ok_clicked), this);
	g_signal_connect (checkbutton8, "clicked",  G_CALLBACK (on_autosave_checked), this);
	g_signal_connect (checkbutton18, "clicked",  G_CALLBACK (on_backup_checked), this);
	g_signal_connect (checkbutton9, "clicked",  G_CALLBACK (on_tabsize_checked), this);
	g_signal_connect (checkbutton10, "clicked",  G_CALLBACK (on_toc_checked), this);
	g_signal_connect (checkbutton17, "clicked",  G_CALLBACK (on_enc_unload_checked), this);
	g_signal_connect (checkbutton22, "clicked",  G_CALLBACK (on_system_locale_checked), this);
	g_signal_connect (radio_ico_none,	"clicked", G_CALLBACK (on_icon_none_clicked), this);
	g_signal_connect (radio_ico_internal,	"clicked", G_CALLBACK (on_icon_internal_clicked), this);
	g_signal_connect (radio_ico_custom,	"clicked", G_CALLBACK (on_icon_custom_clicked), this);
	g_signal_connect (button_xpm_select,	"clicked", G_CALLBACK (on_xpm_select_clicked), this);
	g_signal_connect (comboboxentry12,	"changed", G_CALLBACK (on_internal_combo_selected), this);

	// Store pointers to all widgets, for use by lookup_widget()
	GLADE_HOOKUP_OBJECT_NO_REF (Options, Options, "Options");
	GLADE_HOOKUP_OBJECT_NO_REF (Options, dialog_vbox3, "dialog_vbox3");
	GLADE_HOOKUP_OBJECT (Options, notebook1, "notebook1");
	GLADE_HOOKUP_OBJECT (Options, vbox4, "vbox4");
	GLADE_HOOKUP_OBJECT (Options, checkbutton0, "bootstart_btn");
	GLADE_HOOKUP_OBJECT (Options, checkbutton1, "maximize_btn");
	GLADE_HOOKUP_OBJECT (Options, checkbutton2, "singleinst_btn");
	GLADE_HOOKUP_OBJECT (Options, checkbutton3, "reloadlast_btn");
	GLADE_HOOKUP_OBJECT (Options, checkbutton4, "regformats_btn");
	GLADE_HOOKUP_OBJECT (Options, checkbutton5, "wraptext_btn");
	GLADE_HOOKUP_OBJECT (Options, checkbutton27, "wraptree_btn");
	GLADE_HOOKUP_OBJECT (Options, checkbutton30, "treelines_btn");
	GLADE_HOOKUP_OBJECT (Options, checkbutton6, "min2tray_btn");
	GLADE_HOOKUP_OBJECT (Options, checkbutton7, "last_position_btn");
	GLADE_HOOKUP_OBJECT (Options, checkbutton19, "close2tray_btn");
	GLADE_HOOKUP_OBJECT (Options, checkbutton24, "enterhandle_btn");
	GLADE_HOOKUP_OBJECT (Options, checkbutton15, "tree_right_btn");
	GLADE_HOOKUP_OBJECT (Options, label5, "label5");
	GLADE_HOOKUP_OBJECT (Options, label3, "label3");
	GLADE_HOOKUP_OBJECT (Options, table1, "table1");
	GLADE_HOOKUP_OBJECT (Options, label6, "label6");
	GLADE_HOOKUP_OBJECT (Options, label7, "label7");
	GLADE_HOOKUP_OBJECT (Options, label8, "label8");
	GLADE_HOOKUP_OBJECT (Options, comboboxentry1, "comboboxentry1");
	GLADE_HOOKUP_OBJECT (Options, comboboxentry2, "comboboxentry2");
	GLADE_HOOKUP_OBJECT (Options, fontbutton2, "fontbutton2");
	GLADE_HOOKUP_OBJECT (Options, fontbutton3, "fontbutton3");
	GLADE_HOOKUP_OBJECT_NO_REF (Options, dialog_action_area3, "dialog_action_area3");
	GLADE_HOOKUP_OBJECT (Options, cancelbutton3, "cancelbutton3");
	GLADE_HOOKUP_OBJECT (Options, okbutton3, "okbutton3");
	GLADE_HOOKUP_OBJECT (Options, checkbutton8, "autosave_btn");
	GLADE_HOOKUP_OBJECT (Options, checkbutton18, "backup_btn");
	GLADE_HOOKUP_OBJECT (Options, checkbutton23, "no_mem_autosave_btn");
	GLADE_HOOKUP_OBJECT (Options, entry1, "autosave_fld");
	GLADE_HOOKUP_OBJECT (Options, entry5, "backup_fld");
	GLADE_HOOKUP_OBJECT (Options, colorbtn1, "colorbtn1");
	GLADE_HOOKUP_OBJECT (Options, colorbtn2, "colorbtn2");
	GLADE_HOOKUP_OBJECT (Options, colorbtn3, "colorbtn3");
	GLADE_HOOKUP_OBJECT (Options, entry2, "tabsize_fld");
	GLADE_HOOKUP_OBJECT (Options, checkbutton9, "custom_tab_size_btn");
	GLADE_HOOKUP_OBJECT (Options, checkbutton10, "use_toc_btn");
	GLADE_HOOKUP_OBJECT (Options, entry3, "toc_depth_entry");
	GLADE_HOOKUP_OBJECT (Options, checkbutton11, "convert_image_links_btn");
	GLADE_HOOKUP_OBJECT (Options, checkbutton12, "embedd_css_btn");
	GLADE_HOOKUP_OBJECT (Options, checkbutton13, "autosave_on_close_btn");
	GLADE_HOOKUP_OBJECT (Options, checkbutton14, "export_as_separate_files_btn");
	GLADE_HOOKUP_OBJECT (Options, checkbutton16, "lock_memory_btn");
	GLADE_HOOKUP_OBJECT (Options, checkbutton17, "enc_unload_btn");
	GLADE_HOOKUP_OBJECT (Options, entry4, "enc_unload_fld");
	GLADE_HOOKUP_OBJECT (Options, checkbutton22, "system_locale_btn");
	GLADE_HOOKUP_OBJECT (Options, table6, "table6");
	GLADE_HOOKUP_OBJECT (Options, radio_ico_none, "radio_ico_none");
	GLADE_HOOKUP_OBJECT (Options, radio_ico_internal, "radio_ico_internal");
	GLADE_HOOKUP_OBJECT (Options, radio_ico_custom, "radio_ico_custom");
	GLADE_HOOKUP_OBJECT (Options, image1, "image1");
	GLADE_HOOKUP_OBJECT (Options, button_xpm_select, "button_xpm_select");
	GLADE_HOOKUP_OBJECT (Options, entry14, "entry14");
	GLADE_HOOKUP_OBJECT (Options, label23, "label23");
	GLADE_HOOKUP_OBJECT (Options, comboboxentry12, "comboboxentry12");
	GLADE_HOOKUP_OBJECT (Options, checkbutton32, "drawgray_btn");

	return Options;
}

void on_autosave_checked (GtkMenuItem *menuitem, gpointer user_data)
{
	OptionsDialog *pDlg = (OptionsDialog *)user_data;

	GtkWidget *autosave_btn = lookup_widget(pDlg->m_pDialog, "autosave_btn");
	GtkWidget *autosave_fld = lookup_widget(pDlg->m_pDialog, "autosave_fld");
	GtkWidget *no_mem_autosave_btn = lookup_widget(pDlg->m_pDialog, "no_mem_autosave_btn");

	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(autosave_btn))){
		gtk_widget_set_sensitive (autosave_fld, TRUE);
		gtk_widget_set_sensitive (no_mem_autosave_btn, TRUE);
	}
	else{
		gtk_widget_set_sensitive (autosave_fld, FALSE);
		gtk_widget_set_sensitive (no_mem_autosave_btn, FALSE);
	}
}

void on_backup_checked (GtkMenuItem *menuitem, gpointer user_data)
{
	OptionsDialog *pDlg = (OptionsDialog *)user_data;

	GtkWidget *backup_btn = lookup_widget(pDlg->m_pDialog, "backup_btn");
	GtkWidget *backup_fld = lookup_widget(pDlg->m_pDialog, "backup_fld");

	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(backup_btn)))
		gtk_widget_set_sensitive (backup_fld, TRUE);
	else
		gtk_widget_set_sensitive (backup_fld, FALSE);
}

void on_tabsize_checked (GtkMenuItem *menuitem, gpointer user_data)
{
	OptionsDialog *pDlg = (OptionsDialog *)user_data;

	GtkWidget *tab_size_btn = lookup_widget(pDlg->m_pDialog, "custom_tab_size_btn");
	GtkWidget *tab_size_fld = lookup_widget(pDlg->m_pDialog, "tabsize_fld");

	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(tab_size_btn)))
		gtk_widget_set_sensitive (tab_size_fld, TRUE);
	else
		gtk_widget_set_sensitive (tab_size_fld, FALSE);
}

void on_toc_checked (GtkMenuItem *menuitem, gpointer user_data)
{
	OptionsDialog *pDlg = (OptionsDialog *)user_data;

	GtkWidget *use_toc_btn = lookup_widget(pDlg->m_pDialog, "use_toc_btn");
	GtkWidget *toc_depth_entry = lookup_widget(pDlg->m_pDialog, "toc_depth_entry");

	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(use_toc_btn)))
		gtk_widget_set_sensitive (toc_depth_entry, TRUE);
	else
		gtk_widget_set_sensitive (toc_depth_entry, FALSE);
}

void on_ok_clicked (GtkMenuItem *menuitem, gpointer user_data)
{
	OptionsDialog *pDlg = (OptionsDialog *)user_data;
	pDlg->OnDialogOK();
	pDlg->Destroy();
}

void on_enc_unload_checked (GtkMenuItem *menuitem, gpointer user_data)
{
	OptionsDialog *pDlg = (OptionsDialog *)user_data;

	GtkWidget *enc_unload_btn = lookup_widget(pDlg->m_pDialog, "enc_unload_btn");
	GtkWidget *enc_unload_fld = lookup_widget(pDlg->m_pDialog, "enc_unload_fld");

	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(enc_unload_btn)))
		gtk_widget_set_sensitive (enc_unload_fld, TRUE);
	else
		gtk_widget_set_sensitive (enc_unload_fld, FALSE);
}

void on_system_locale_checked(GtkMenuItem *menuitem, gpointer user_data)
{
	OptionsDialog *pDlg = (OptionsDialog *)user_data;

	GtkWidget *system_locale_btn = lookup_widget(pDlg->m_pDialog, "system_locale_btn");
	GtkWidget *comboboxentry1 = lookup_widget(pDlg->m_pDialog, "comboboxentry1");

	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(system_locale_btn)))
		gtk_widget_set_sensitive (comboboxentry1, FALSE);
	else
		gtk_widget_set_sensitive (comboboxentry1, TRUE);
}


void on_icon_none_clicked(GtkMenuItem *menuitem, gpointer user_data)
{
	OptionsDialog *pDlg = (OptionsDialog *)user_data;

	GtkWidget *radio_ico_none = lookup_widget(pDlg->m_pDialog, "radio_ico_none");
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_ico_none)))
	{
		GtkWidget *cbo2				= lookup_widget(pDlg->m_pDialog, "comboboxentry12");
		GtkWidget *entry4			= lookup_widget(pDlg->m_pDialog, "entry14");
		GtkWidget *button			= lookup_widget(pDlg->m_pDialog, "button_xpm_select");

		gtk_widget_set_sensitive(cbo2, FALSE);
		gtk_widget_set_sensitive(entry4, FALSE);
		gtk_widget_set_sensitive(button, FALSE);

		pDlg->UpdatePreview();
	}
}

void on_icon_internal_clicked(GtkMenuItem *menuitem, gpointer user_data)
{
	OptionsDialog *pDlg = (OptionsDialog *)user_data;

	GtkWidget *radio_ico_internal = lookup_widget(pDlg->m_pDialog, "radio_ico_internal");
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_ico_internal)))
	{
		GtkWidget *cbo2		= lookup_widget(pDlg->m_pDialog, "comboboxentry12");
		GtkWidget *entry4	= lookup_widget(pDlg->m_pDialog, "entry14");
		GtkWidget *button	= lookup_widget(pDlg->m_pDialog, "button_xpm_select");

		gtk_widget_set_sensitive(cbo2, TRUE);
		gtk_widget_set_sensitive(entry4, FALSE);
		gtk_widget_set_sensitive(button, FALSE);

		pDlg->UpdatePreview();
	}
}

void on_icon_custom_clicked(GtkMenuItem *menuitem, gpointer user_data)
{
	OptionsDialog *pDlg = (OptionsDialog *)user_data;

	GtkWidget *radio_ico_custom = lookup_widget(pDlg->m_pDialog, "radio_ico_custom");
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_ico_custom)))
	{
		GtkWidget *cbo2		= lookup_widget(pDlg->m_pDialog, "comboboxentry12");
		GtkWidget *entry4	= lookup_widget(pDlg->m_pDialog, "entry14");
		GtkWidget *button	= lookup_widget(pDlg->m_pDialog, "button_xpm_select");

		gtk_widget_set_sensitive(cbo2, FALSE);
		gtk_widget_set_sensitive(entry4, TRUE);
		gtk_widget_set_sensitive(button, TRUE);

		pDlg->UpdatePreview();
	}
}

void on_xpm_select_clicked(GtkMenuItem *menuitem, gpointer user_data)
{
	FileDialog dlg;
	dlg.AddFilter(_("All supported icon formats (*.xpm,*.ico,*.png,*.jpg)"), "*.xpm|*.ico|*.png|*.jpg");
	dlg.AddFilter(_("XPM icon file (*.xpm)"), "*.xpm");
	dlg.AddFilter(_("Icon file (*.ico)"), "*.ico");
	dlg.AddFilter(_("PNG file format (*.png)"), "*.png");
	dlg.AddFilter(_("JPEG file format (*.jpg)"), "*.jpg");

	//set initial directory from INI (store last used)
	std::string strDefaultDir;
	std::string strDir;
	g_objIni.GetValue("Cache", "LastIconDir", strDir, "");
	if(!strDir.empty() && 0 == access(strDir.c_str(), 00))
		strDefaultDir = strDir;
	else
		strDefaultDir = GetHomeDir();

	dlg.SetDirectory(strDefaultDir.c_str());

	if(dlg.DoModal())
	{
		strDefaultDir = dlg.GetDirectory();
		g_objIni.SetValue("Cache", "LastIconDir", strDefaultDir.c_str());

		OptionsDialog *pDlg = (OptionsDialog *)user_data;
		GtkWidget *entry4	= lookup_widget(pDlg->m_pDialog, "entry14");
		gtk_entry_set_text(GTK_ENTRY(entry4), dlg.GetFilename());

		pDlg->UpdatePreview();
	}
}

int OptionsDialog::GetIconType()
{
	GtkWidget *radio_ico_none 	= lookup_widget(m_pDialog, "radio_ico_none");
	GtkWidget *radio_ico_internal	= lookup_widget(m_pDialog, "radio_ico_internal");

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_ico_none)))
		return ICON_NONE;
	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_ico_internal)))
		return ICON_INTERNAL;
	return ICON_CUSTOM;
}

const char *OptionsDialog::GetIconValue()
{
	GtkWidget *cbo2		= lookup_widget(m_pDialog, "comboboxentry12");
	GtkWidget *entry4	= lookup_widget(m_pDialog, "entry14");

	switch (GetIconType())
	{
		case ICON_INTERNAL:	return combo_get_text(cbo2);
		case ICON_CUSTOM:	return gtk_entry_get_text(GTK_ENTRY(entry4));
		default: 			return "";
	}
}

void OptionsDialog::SetIconType(int nType)
{
	GtkWidget *radio_ico_none 	= lookup_widget(m_pDialog, "radio_ico_none");
	GtkWidget *radio_ico_internal	= lookup_widget(m_pDialog, "radio_ico_internal");
	GtkWidget *radio_ico_custom	= lookup_widget(m_pDialog, "radio_ico_custom");
	GtkWidget *cbo2			= lookup_widget(m_pDialog, "comboboxentry12");

	if(ICON_NONE == nType)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_ico_none), TRUE);
		on_icon_none_clicked(NULL, this);
	}
	else if(ICON_CUSTOM == nType)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_ico_custom), TRUE);
	else
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_ico_internal), TRUE);
		gtk_combo_box_set_active(GTK_COMBO_BOX(cbo2), nType);
	}

	UpdatePreview();
}

void OptionsDialog::SetIconValue(const char *szValue)
{
	GtkWidget *entry4 = lookup_widget(m_pDialog, "entry14");
	gtk_entry_set_text(GTK_ENTRY(entry4), szValue);

	UpdatePreview();
}

void OptionsDialog::UpdatePreview()
{
	GtkWidget *image1 = lookup_widget(m_pDialog, "image1");

	int nType = GetIconType();
	if(ICON_INTERNAL == nType)
	{
		int nIcoIdx = InternalIcon_Name2Index(GetIconValue());
		if(nIcoIdx >= 0)
		{
			const char **szIconData = InternalIcon_GetFromIdx(nIcoIdx);
			GdkPixbuf *pixbuf = gdk_pixbuf_new_from_xpm_data (szIconData);
			GdkPixbuf *destpix = gdk_pixbuf_scale_simple(pixbuf, 16, 16, GDK_INTERP_BILINEAR);
			g_object_unref (G_OBJECT (pixbuf));
			gtk_image_set_from_pixbuf (GTK_IMAGE(image1), destpix);
			g_object_unref (G_OBJECT (destpix));
		}
		else
		{
			gtk_image_set_from_file(GTK_IMAGE(image1), NULL);	//clear image
		}
	}
	else if(ICON_CUSTOM == nType)
	{
		GdkPixbuf *pixbuf = NULL;
		const char *szFile = GetIconValue();
		if(szFile && strlen(szFile) > 0)
			pixbuf = gdk_pixbuf_new_from_file_at_size(szFile, 16, 16, NULL);

		if(pixbuf){
			gtk_image_set_from_pixbuf (GTK_IMAGE(image1), pixbuf);
			g_object_unref (G_OBJECT (pixbuf));
		}
		else
		{
			gtk_image_set_from_file(GTK_IMAGE(image1), NULL);	//clear image

			if(szFile && strlen(szFile) > 0)
				gtkMessageBox(_("Failed to load node icon file!"));
		}
	}
	else
		gtk_image_set_from_file(GTK_IMAGE(image1), NULL);	//clear image
}

void on_internal_combo_selected (GtkComboBox *widget, gpointer user_data)
{
	OptionsDialog *pDlg = (OptionsDialog *)user_data;
	pDlg->UpdatePreview();
}

gboolean on_numeric_entry_keypress(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	if( event->type == GDK_KEY_PRESS &&
		((gdk_keyval_to_unicode (event->keyval) >= '0' && gdk_keyval_to_unicode (event->keyval) <= '9') ||
		event->keyval == GDK_Left  ||
		event->keyval == GDK_Right ||
		event->keyval == GDK_Home  ||
		event->keyval == GDK_End   ||
		event->keyval == GDK_Delete	||
		event->keyval == GDK_BackSpace) )
		return FALSE; //keep processing

	return TRUE;
}

gboolean on_numeric_entry_changed(GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
	int nMinValue = GPOINTER_TO_INT(user_data);

	//OptionsDialog *pDlg = (OptionsDialog *)user_data;
	std::string strValue = gtk_entry_get_text(GTK_ENTRY(widget));
	if(!strValue.empty())
	{
		int nValue = atoi(strValue.c_str());
		if(nValue < nMinValue)
		{
			char szBuffer[30];
			sprintf(szBuffer, "%d", nMinValue);
			gtk_entry_set_text(GTK_ENTRY(widget), szBuffer);

			char szMessage[1030];
			sprintf(szMessage, _("Minimal value is %d sec!"), nMinValue);
			gtkMessageBox(szMessage);
			return TRUE;
		}
	}

	return FALSE;	//propagate
}
