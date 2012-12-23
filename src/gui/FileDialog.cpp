////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements light wrapper around FileChooser GTK API
////////////////////////////////////////////////////////////////////////////

#include "FileDialog.h"
#include "../config.h"
#include <string>
#include <gdk/gdkkeysyms.h>
#include "../support.h"
#include "../lib/debug.h"

#ifdef _NOKIA_MAEMO
 #if _NOKIA_MAEMO == 1
  #include <hildon-widgets/hildon-file-chooser-dialog.h>
 #else
  #include <hildon/hildon-file-chooser-dialog.h>
 #endif
#endif

#if !GTK_CHECK_VERSION(2,4,0) //minimal version for new type of dialog
   #define _OLD_DIALOG
#else
   //#define _CUSTOM_FILE_DLG	//not yet implemented
#endif

#ifdef _CUSTOM_FILE_DLG
static void on_format_combo_selected (GtkComboBox *widget, gpointer user_data);
#endif

FileDialog::FileDialog(bool bOpen, bool bOnlyFolders, bool bExpand, const char *szTitle, void *pParent)
{
	m_pFileWidget = NULL;
	m_szFile = NULL;
	m_szDir  = NULL;
	m_bOpenDialog = bOpen;
	m_pFormatsCombo = NULL;
	m_pFileNameEntry = NULL;

	const char *szDlgTitle = szTitle;
	if(NULL == szDlgTitle)
		szDlgTitle = (bOpen) ? _("Open file") : _("Save file");

#ifdef _OLD_DIALOG
	m_pDialog = gtk_file_selection_new(szDlgTitle);
	gtk_dialog_set_default_response (GTK_DIALOG (m_pDialog), GTK_RESPONSE_OK);
#else
 #ifdef _CUSTOM_FILE_DLG
	m_pDialog = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (m_pDialog), szDlgTitle);
	gtk_window_set_modal (GTK_WINDOW (m_pDialog), TRUE);
	gtk_window_set_destroy_with_parent (GTK_WINDOW (m_pDialog), TRUE);
	gtk_window_set_default_size (GTK_WINDOW (m_pDialog), 500, 350);

	GtkWidget *dialog_vbox6 = GTK_DIALOG (m_pDialog)->vbox;
	gtk_widget_show (dialog_vbox6);

	GtkWidget *table1 = gtk_table_new (3, 2, FALSE);
	gtk_widget_show (table1);
	gtk_box_pack_start (GTK_BOX (dialog_vbox6), table1, TRUE, TRUE, 0);

	//always create in open mode (save mode starts in collapsed more which we want to avoid)
	m_pFileWidget = gtk_file_chooser_widget_new (GTK_FILE_CHOOSER_ACTION_OPEN);
	gtk_widget_show (GTK_WIDGET (m_pFileWidget));
	gtk_table_attach (GTK_TABLE (table1), m_pFileWidget, 0, 2, 0, 1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	GtkWidget *label5 = gtk_label_new (_("File Name:"));
	gtk_widget_show (label5);
	gtk_table_attach (GTK_TABLE (table1), label5, 0, 1, 1, 2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label5), 0, 0.5);

	GtkWidget *entry2 = gtk_entry_new ();
	gtk_widget_show (entry2);
	gtk_table_attach (GTK_TABLE (table1), entry2, 1, 2, 1, 2, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	m_pFileNameEntry = entry2;

	GtkWidget *label6 = gtk_label_new (_("Save as type:"));
	gtk_widget_show (label6);
	gtk_table_attach (GTK_TABLE (table1), label6, 0, 1, 2, 3, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label6), 0, 0.5);

	m_pFormatsCombo = gtk_combo_box_new_text ();
	gtk_widget_show (m_pFormatsCombo);
	gtk_table_attach (GTK_TABLE (table1), m_pFormatsCombo, 1, 2, 2, 3, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
	g_signal_connect(m_pFormatsCombo, "changed", G_CALLBACK (on_format_combo_selected), this);

	GtkWidget *dialog_action_area6 = GTK_DIALOG (m_pDialog)->action_area;
	gtk_widget_show (dialog_action_area6);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area6), GTK_BUTTONBOX_END);
	
	GtkWidget *cancelbutto5 = gtk_button_new_from_stock ("gtk-cancel");
	gtk_widget_show (cancelbutto5);
	gtk_dialog_add_action_widget (GTK_DIALOG (m_pDialog), cancelbutto5, GTK_RESPONSE_CANCEL);
	GTK_WIDGET_SET_FLAGS (cancelbutto5, GTK_CAN_DEFAULT);
	
	GtkWidget *okbutton6 = gtk_button_new_with_mnemonic ((bOpen)? _("_Open") : _("_Save"));
	gtk_widget_show (okbutton6);
	gtk_container_add (GTK_CONTAINER(dialog_action_area6), okbutton6);
	GTK_WIDGET_SET_FLAGS (okbutton6, GTK_CAN_DEFAULT);
 #else
  #ifdef _NOKIA_MAEMO
	GtkFileChooserAction action;
	if(bOpen)
		action = (bOnlyFolders)? GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER : GTK_FILE_CHOOSER_ACTION_OPEN;
	else
		action = (bOnlyFolders)? GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER : GTK_FILE_CHOOSER_ACTION_SAVE;
	
	m_pDialog = hildon_file_chooser_dialog_new(GTK_WINDOW (pParent), action);
	gtk_widget_show_all(GTK_WIDGET(m_pDialog));

	gtk_window_set_modal (GTK_WINDOW (m_pDialog), TRUE);
#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (m_pDialog), TRUE);
#endif
	gtk_window_set_skip_pager_hint (GTK_WINDOW (m_pDialog), TRUE);
	gtk_window_set_type_hint (GTK_WINDOW (m_pDialog), GDK_WINDOW_TYPE_HINT_DIALOG);
#if GTK_CHECK_VERSION(2,4,0) //new API TOFIX set proper version
	#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
	  //gtk_window_set_keep_above(GTK_WINDOW (m_pDialog), TRUE);
	#endif
#endif

  #else
	if(bOpen)
		m_pDialog = gtk_file_chooser_dialog_new(szDlgTitle, GTK_WINDOW (pParent), 
			(bOnlyFolders)? GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER : GTK_FILE_CHOOSER_ACTION_OPEN, 
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
			GTK_STOCK_OPEN, GTK_RESPONSE_OK, (char *)NULL);
	else
	{
		m_pDialog = gtk_file_chooser_dialog_new(szDlgTitle, GTK_WINDOW (pParent), 
			(bOnlyFolders)? GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER : GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
			GTK_STOCK_SAVE, GTK_RESPONSE_OK, (char *)NULL);

		if(bExpand)
		{
			//expand dialog to full size
			GList *children = gtk_container_get_children(GTK_CONTAINER(m_pDialog));
			GList *children1 = gtk_container_get_children(GTK_CONTAINER(g_list_nth_data (children, 0))); //children of vbox
			GList *children2 = gtk_container_get_children(GTK_CONTAINER(g_list_nth_data (children1, 0)));//children of file chooser widget
			GList *children3 = gtk_container_get_children(GTK_CONTAINER(g_list_nth_data (children2, 0)));//children of 
			GList *children4 = gtk_container_get_children(GTK_CONTAINER(g_list_nth_data (children3, 0)));//children of 
			GList *children5 = gtk_container_get_children(GTK_CONTAINER(g_list_nth_data (children4, 1)));//children of table widget

			GtkWidget *expander = (GtkWidget *)g_list_nth_data (children5, 0);
			ASSERT(GTK_IS_EXPANDER(expander));
			gtk_expander_set_expanded(GTK_EXPANDER(expander), true);
			
			g_list_free(children5);
			g_list_free(children4);	
			g_list_free(children3);
			g_list_free(children2);
			g_list_free(children1);
			g_list_free(children);
		}
	}

	gtk_window_set_modal (GTK_WINDOW (m_pDialog), TRUE);
#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (m_pDialog), TRUE);
#endif
	gtk_window_set_skip_pager_hint (GTK_WINDOW (m_pDialog), TRUE);
	gtk_window_set_type_hint (GTK_WINDOW (m_pDialog), GDK_WINDOW_TYPE_HINT_DIALOG);
#if GTK_CHECK_VERSION(2,4,0) //new API TOFIX set proper version
	#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
	  //gtk_window_set_keep_above(GTK_WINDOW (m_pDialog), TRUE);
	#endif
#endif
	
  #endif
 #endif
#endif
}

FileDialog::~FileDialog()
{
	Close();

#ifndef _WIN32
	if(m_szFile)
		g_free(m_szFile);
	if(m_szDir)
		g_free(m_szDir);
#endif
}

void FileDialog::AddFilter(const char *szText, const char *szPtrn)
{
	//add filters
#ifdef _OLD_DIALOG
#else
 #ifdef _CUSTOM_FILE_DLG
	//fill combo box with filter titles
	//gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(m_pFileWidget), filter);
	gtk_combo_box_append_text (GTK_COMBO_BOX (m_pFormatsCombo), szText);
	
 #else
	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, szText);

	//allow multiple wildcard patterns to be separated with | character
	std::string strMulti(szPtrn);
	std::string strPtrn;
	size_t nPos = strMulti.find('|');
	while(nPos != std::string::npos)
	{
		strPtrn  = strMulti.substr(0, nPos);
		strMulti = strMulti.substr(nPos+1, 10000);
		nPos = strMulti.find('|');

		gtk_file_filter_add_pattern(filter, strPtrn.c_str());
	}
	if(!strMulti.empty())
		gtk_file_filter_add_pattern(filter, strMulti.c_str());

	
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(m_pDialog), filter);
 #endif
#endif
}

bool FileDialog::DoModal()
{
	return (gtk_dialog_run(GTK_DIALOG(m_pDialog)) == GTK_RESPONSE_OK);
}

void FileDialog::Close()
{
	if(m_pDialog)
		gtk_widget_destroy(m_pDialog);
	m_pDialog = NULL;
}

const char *FileDialog::GetFilename()
{
	//free if already allocated
	if(m_szFile)
		g_free(m_szFile);

#ifdef _OLD_DIALOG
	m_szFile = (char *)gtk_file_selection_get_filename(GTK_FILE_SELECTION (m_pDialog));
#else
 #ifdef _CUSTOM_FILE_DLG
	m_szFile = (char *)gtk_entry_get_text (GTK_ENTRY(m_pFileNameEntry));
 #else
	m_szFile = (char *)gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_pDialog));
 #endif
#endif

	return m_szFile;
}

void FileDialog::SetFilename(const char *szName)
{
#ifdef _OLD_DIALOG
	gtk_file_selection_set_filename(GTK_FILE_SELECTION (m_pDialog), szName);
#else
 #ifdef _CUSTOM_FILE_DLG
	gtk_entry_set_text (GTK_ENTRY(m_pFileNameEntry), szName);
 #else
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(m_pDialog), szName);
 #endif
#endif
}

void FileDialog::SetDirectory(const char *szPath)
{
#ifdef _OLD_DIALOG
#else
 #ifdef _CUSTOM_FILE_DLG
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(m_pFileWidget), szPath);	
 #else
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(m_pDialog), szPath);	
 #endif
#endif
}

void FileDialog::SetTitle(const char *szText)
{
	gtk_window_set_title (GTK_WINDOW (m_pDialog), szText);
}

const char *FileDialog::GetCurrentFilterName()
{
#ifdef _OLD_DIALOG
#else
 #ifdef _CUSTOM_FILE_DLG
	//TOFIX
 #else
	GtkFileFilter *filter = gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(m_pDialog));
	if(filter)
		return gtk_file_filter_get_name(filter);
 #endif
#endif
	return NULL;
}

const char *FileDialog::GetDirectory()
{
#ifdef _OLD_DIALOG
#else	
	if(m_szDir)
		g_free(m_szDir);

 #ifdef _CUSTOM_FILE_DLG
	m_szDir = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(m_pFileWidget));
 #else
	m_szDir = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(m_pDialog));
 #endif
#endif

	return m_szDir;
}

void FileDialog::ForceFormatExtension()
{
	std::string strFile = GetFilename();
	if(!strFile.empty())
	{
		//get export format
		int nPos = gtk_combo_box_get_active (GTK_COMBO_BOX (m_pFormatsCombo));
		if(nPos >= 0)
		{
			const char *szFilter = GetCurrentFilterName();
			if(!szFilter)
				return;

			//extract extension enclosed in ()
			std::string strExt = szFilter;
			std::string::size_type nPos = strExt.rfind('(');
			if(nPos != std::string::npos){
				strExt.erase(strExt.begin(), strExt.begin()+nPos+1);
				nPos = strExt.rfind(')');
				if(nPos != std::string::npos)
					strExt.erase(strExt.begin()+nPos, strExt.end());
				strExt.erase(strExt.begin(), strExt.begin()+1);
			}
			
			unsigned int nSize = strExt.size();

			// make sure file is terminated with the matching extension
			if( strFile.size() > nSize &&
			    strFile.substr(strFile.size()-nSize) == strExt)
				return;

			//strip previous extension TOFIX?
			std::string::size_type nExt = strFile.rfind('.');
			if(nExt != std::string::npos)
				strFile = strFile.substr(0, nExt);

			//append extension
			strFile += strExt;

			//set new name
			SetFilename(strFile.c_str());
		}
	}
}

#ifdef _CUSTOM_FILE_DLG
void on_format_combo_selected (GtkComboBox *widget, gpointer user_data)
{
	FileDialog *pDlg = (FileDialog *)user_data;
	pDlg->ForceFormatExtension();
}
#endif
