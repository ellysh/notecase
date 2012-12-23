////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements window for file export options
////////////////////////////////////////////////////////////////////////////

#include "FileExportDlg.h"
#include "gui/FileDialog.h"
#include "lib/IniFile.h"
#include "lib/NoteDocument.h"
#include "support.h"
#include "interface.h"
#include <string.h>
#include <string>
#include <algorithm>
#include "mru.h"
#include "lib/FilePath.h"

extern GtkWidget *window1;
extern IniFile g_objIni;

void replaceall(std::string &strData, const char *szFind, const char *szReplace);

static void on_pick_file_clicked(GtkButton *button, gpointer user_data);
static void on_save_clicked(GtkButton *button, gpointer user_data);
static void on_format_combo_selected (GtkComboBox *widget, gpointer user_data);
static void on_use_custom_css (GtkMenuItem *menuitem, gpointer user_data);
static void on_use_post_export (GtkMenuItem *menuitem, gpointer user_data);

typedef struct {
	std::string strTitle;
	std::string strPath;
} tScriptInfo;

static std::vector<tScriptInfo> g_lstExportScripts;

FileExportDlg::FileExportDlg()
{
	// init formats table
	m_formats[0].szName   = _("NoteCase document (*.ncd)");
	m_formats[0].szFilter = "*.ncd";
	m_formats[1].szName   = _("NoteCase encrypted document (*.nce)");
	m_formats[1].szFilter = "*.nce";
	m_formats[2].szName   = _("NoteCenter document (*.hnc)");
	m_formats[2].szFilter = "*.hnc";
	m_formats[3].szName   = _("HTML file (*.html)");
	m_formats[3].szFilter = "*.html";
	m_formats[4].szName   = _("Text file (*.txt)");
	m_formats[4].szFilter = "*.txt";
#ifdef _WIN32
	m_formats[5].szName   = _("Standalone executable (*.exe)");
	m_formats[5].szFilter = "*.exe";
#else
	m_formats[5].szName   = _("Standalone executable");
	m_formats[5].szFilter = "*";
#endif

	Create();
}

FileExportDlg::~FileExportDlg()
{
	Destroy();
	gtk_window_present(GTK_WINDOW(window1)); //activate main window
}

void FileExportDlg::Create()
{
	m_pDialog = create_file_export_dialog (window1);

	// fill export sources
	GtkWidget *combobox1 = lookup_widget(m_pDialog, "combobox1");
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox1), _("Selected branch"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox1), _("Selected node only"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox1), _("Entire document"));

	// fill format combo box
	GtkWidget *combobox2 = lookup_widget(m_pDialog, "combobox2");
	for(unsigned int i=0; i<sizeof(m_formats)/sizeof(m_formats[0]); i++)
		gtk_combo_box_append_text (GTK_COMBO_BOX (combobox2), m_formats[i].szName);

	//restore last used export source in INI
	int nExportSource = 0;
	g_objIni.GetValue("Export", "DocumentSource", nExportSource, 0);
	if(0 > nExportSource || nExportSource > 2)
		nExportSource = 0;

	gtk_combo_box_set_active (GTK_COMBO_BOX (combobox1), nExportSource);

	//restore last used export format in INI
	int nExportFormat = 0;
	g_objIni.GetValue("Export", "TargetFormat", nExportFormat, 0);
	if(0 > nExportFormat || nExportFormat > (int)(sizeof(m_formats)/sizeof(m_formats[0])))
		nExportFormat = 0;

	gtk_combo_box_set_active (GTK_COMBO_BOX (combobox2), nExportFormat);

	//restore last used export file name
	std::string strFile;
	g_objIni.GetValue("Export", "TargetFile", strFile, "");
	if(strFile.empty())
	{
		//default export directory/name
		strFile = GetHomeDir();
		EnsureTerminated(strFile, '/');
		#ifdef _NOKIA_MAEMO
			//Maemo file selector is not easy to use, by default set correct document directory
			strFile += "MyDocs/.documents/";
		#endif
		strFile += "default.ncd";
	}
	GtkWidget *entry5 = lookup_widget(m_pDialog, "entry5");
	gtk_entry_set_text(GTK_ENTRY(entry5), strFile.c_str());
	ForceFormatExtension();

	//fill post export scripts
	g_lstExportScripts.clear();
	GtkWidget *combobox3 = lookup_widget(m_pDialog, "combobox3");
	int nNumScripts = 0;
	g_objIni.GetValue("Export", "PostExportCount", nNumScripts);

	for(int j=0; j<nNumScripts; j++)
	{
		tScriptInfo info;

		char szKey[100];
		sprintf(szKey, "PostExportTitle_%d", j+1);
		g_objIni.GetValue("Export", szKey, info.strTitle, "");
		sprintf(szKey, "PostExportScript_%d", j+1);
		g_objIni.GetValue("Export", szKey, info.strPath, "");

		g_lstExportScripts.push_back(info);
		gtk_combo_box_append_text (GTK_COMBO_BOX (combobox3), info.strTitle.c_str());
	}
	if(nNumScripts < 1)
	{
		GtkWidget *checkbox3 = lookup_widget(m_pDialog, "checkbox3");
		gtk_widget_hide(checkbox3);
		gtk_widget_hide(combobox3);
	}
	else{
		gtk_combo_box_set_active (GTK_COMBO_BOX (combobox3), 0);
		on_use_post_export (NULL, this);
	}
}

void FileExportDlg::ForceFormatExtension()
{
	std::string strFile = GetExportFile();
	if(!strFile.empty())
	{
		//get export format
		GtkWidget *combobox2 = lookup_widget(m_pDialog, "combobox2");

		int nPos = gtk_combo_box_get_active (GTK_COMBO_BOX (combobox2));
		if(nPos >= 0)
		{
			std::string strExt = m_formats[nPos].szFilter;
			strExt.erase(strExt.begin(), strExt.begin()+1);

			unsigned int nSize = strExt.size();

			// make sure file is terminated with the matchign extension
			if( strFile.size() > nSize &&
			    strFile.substr(strFile.size()-nSize) == strExt)
				return;

			//strip previous extension
			std::string::size_type nExt = strFile.rfind('.');
			if(nExt != std::string::npos)
				strFile = strFile.substr(0, nExt);

			//append extension
			strFile += strExt;

			//set new name
			GtkWidget *entry5 = lookup_widget(m_pDialog, "entry5");
			gtk_entry_set_text(GTK_ENTRY(entry5), strFile.c_str());
		}
	}
}

int FileExportDlg::GetExportFormat()
{
	GtkWidget *combobox2 = lookup_widget(m_pDialog, "combobox2");
	return 	gtk_combo_box_get_active (GTK_COMBO_BOX (combobox2));
}

int FileExportDlg::GetFormatCode()
{
	int nIdx = GetExportFormat();
	switch(nIdx){
		case 0: return FORMAT_NOTECASE_HTML;
		case 1: return FORMAT_NOTECASE_HTML_ENC;
		case 2: return FORMAT_NOTECENTER_HTML;
		case 3: return FORMAT_HTML;
		case 4: return FORMAT_TXT;
		case 5: return FORMAT_EXECUTABLE;
	}
	return FORMAT_NONE;
}

int FileExportDlg::GetExportMode()
{
	GtkWidget *combobox1 = lookup_widget(m_pDialog, "combobox1");
	return 	gtk_combo_box_get_active (GTK_COMBO_BOX (combobox1));
}

bool FileExportDlg::GetExportLinked()
{
	if(3 != GetExportFormat())
		return false;

	GtkWidget *checkbox2 = lookup_widget(m_pDialog, "checkbox2");
	if(!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(checkbox2)))
		return false;

	return true;
}

const char *FileExportDlg::GetExportFile()
{
	GtkWidget *entry5 = lookup_widget(m_pDialog, "entry5");
	return 	gtk_entry_get_text(GTK_ENTRY(entry5));
}

const char *FileExportDlg::GetPostExportScript()
{
	GtkWidget *checkbox3 = lookup_widget(m_pDialog, "checkbox3");
	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(checkbox3)))
	{
		GtkWidget *combobox3 = lookup_widget(m_pDialog, "combobox3");
		int nIdx = gtk_combo_box_get_active (GTK_COMBO_BOX (combobox3));
		if(nIdx >= 0)
			return g_lstExportScripts[nIdx].strPath.c_str();
	}
	return "";
}

bool FileExportDlg::GetHtmlExportCSS(std::string &strCSS)
{
	if(3 != GetExportFormat())
		return false;

	GtkWidget *checkbox1 = lookup_widget(m_pDialog, "checkbox1");
	if(!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(checkbox1)))
		return false;

	GtkWidget *entry6 = lookup_widget(m_pDialog, "entry6");
	strCSS = gtk_entry_get_text(GTK_ENTRY(entry6));

	//Convert all backslashes in (CSS) links to slashes
	replaceall(strCSS, "\\", "/");

	return true;
}

GtkWidget* FileExportDlg::create_file_export_dialog (GtkWidget* parent)
{
	GtkWidget *file_export_dialog;
	GtkWidget *dialog_vbox6;
	GtkWidget *table7;
	GtkWidget *label24;
	GtkWidget *label25;
	GtkWidget *combobox1;
	GtkWidget *combobox2;
	GtkWidget *label26;
	GtkWidget *hbox3;
	GtkWidget *entry5;
	GtkWidget *button5;
	GtkWidget *image2;
	GtkWidget *hbox4;
	GtkWidget *checkbox1;
	GtkWidget *entry6;
	GtkWidget *checkbox2;
	GtkWidget *checkbox3;
	GtkWidget *combobox3;
	GtkWidget *dialog_action_area6;
	GtkWidget *cancelbutton6;
	GtkWidget *okbutton6;

	file_export_dialog = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (file_export_dialog), _("Export"));
	gtk_window_set_modal (GTK_WINDOW (file_export_dialog), TRUE);
	gtk_window_set_destroy_with_parent (GTK_WINDOW (file_export_dialog), TRUE);
#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (file_export_dialog), TRUE);
#endif
	gtk_window_set_skip_pager_hint (GTK_WINDOW (file_export_dialog), TRUE);
	gtk_window_set_type_hint (GTK_WINDOW (file_export_dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
	if(parent)
		gtk_window_set_transient_for(GTK_WINDOW (file_export_dialog), GTK_WINDOW(parent));   //set parent
#if GTK_CHECK_VERSION(2,4,0) //new API TOFIX set proper version
	#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
	  //gtk_window_set_keep_above(GTK_WINDOW (file_export_dialog), TRUE);
	#endif
#endif
	gtk_widget_realize(file_export_dialog);
	gdk_window_set_decorations(file_export_dialog->window, (GdkWMDecoration)(GDK_DECOR_BORDER|GDK_DECOR_TITLE));

	dialog_vbox6 = GTK_DIALOG (file_export_dialog)->vbox;
	gtk_widget_show (dialog_vbox6);

	table7 = gtk_table_new (4, 2, FALSE);
	gtk_widget_show (table7);
	gtk_box_pack_start (GTK_BOX (dialog_vbox6), table7, TRUE, TRUE, 0);

	label24 = gtk_label_new (_("Document source:"));
	gtk_widget_show (label24);
	gtk_table_attach (GTK_TABLE (table7), label24, 0, 1, 0, 1,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label24), 0, 0.5);

	label25 = gtk_label_new (_("File format:"));
	gtk_widget_show (label25);
	gtk_table_attach (GTK_TABLE (table7), label25, 0, 1, 1, 2,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label25), 0, 0.5);

	combobox1 = gtk_combo_box_new_text ();
	gtk_widget_show (combobox1);
	gtk_table_attach (GTK_TABLE (table7), combobox1, 1, 2, 0, 1,
		(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		(GtkAttachOptions) (GTK_FILL), 0, 0);

	combobox2 = gtk_combo_box_new_text ();
	gtk_widget_show (combobox2);
	gtk_table_attach (GTK_TABLE (table7), combobox2, 1, 2, 1, 2,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (GTK_FILL), 0, 0);

	label26 = gtk_label_new (_("File name:"));
	gtk_widget_show (label26);
	gtk_table_attach (GTK_TABLE (table7), label26, 0, 1, 2, 3,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label26), 0, 0.5);

	hbox3 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox3);
	gtk_table_attach (GTK_TABLE (table7), hbox3, 1, 2, 2, 3,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (GTK_FILL), 0, 0);

	entry5 = gtk_entry_new ();
	gtk_widget_show (entry5);
	gtk_box_pack_start (GTK_BOX (hbox3), entry5, TRUE, TRUE, 0);
	gtk_entry_set_activates_default (GTK_ENTRY (entry5), TRUE);

	button5 = gtk_button_new ();
	gtk_widget_show (button5);
	gtk_box_pack_start (GTK_BOX (hbox3), button5, FALSE, FALSE, 0);

	image2 = gtk_image_new_from_stock ("gtk-open", GTK_ICON_SIZE_BUTTON);
	gtk_widget_show (image2);
	gtk_container_add (GTK_CONTAINER (button5), image2);

	hbox4 = gtk_hbox_new (FALSE, 0);
	//gtk_widget_show (hbox4);
	gtk_table_attach (GTK_TABLE (table7), hbox4, 0, 2, 3, 4,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (GTK_FILL), 0, 0);

	checkbox1 = gtk_check_button_new_with_mnemonic (_("Use custom CSS path:"));
	gtk_widget_show (checkbox1);
	gtk_box_pack_start (GTK_BOX (hbox4), checkbox1, TRUE, TRUE, 0);

	entry6 = gtk_entry_new ();
	gtk_widget_show (entry6);
	gtk_box_pack_start (GTK_BOX (hbox4), entry6, TRUE, TRUE, 0);
	gtk_widget_set_sensitive(entry6, FALSE);

	checkbox2 = gtk_check_button_new_with_mnemonic (_("Export linked documents"));
	gtk_widget_show (checkbox2);
	gtk_table_attach (GTK_TABLE (table7), checkbox2, 0, 2, 4, 5,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (GTK_FILL), 0, 0);

	checkbox3 = gtk_check_button_new_with_mnemonic (_("Post-export script:"));
	gtk_widget_show (checkbox3);
	gtk_table_attach (GTK_TABLE (table7), checkbox3, 0, 1, 5, 6,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (GTK_FILL), 0, 0);

	combobox3 = gtk_combo_box_new_text ();
	gtk_widget_show (combobox3);
	gtk_table_attach (GTK_TABLE (table7), combobox3, 1, 2, 5, 6,
		(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		(GtkAttachOptions) (GTK_FILL), 0, 0);

	dialog_action_area6 = GTK_DIALOG (file_export_dialog)->action_area;
	gtk_widget_show (dialog_action_area6);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area6), GTK_BUTTONBOX_END);

	cancelbutton6 = gtk_button_new_from_stock ("gtk-cancel");
	gtk_widget_show (cancelbutton6);
	gtk_dialog_add_action_widget (GTK_DIALOG (file_export_dialog), cancelbutton6, GTK_RESPONSE_CANCEL);
	GTK_WIDGET_SET_FLAGS (cancelbutton6, GTK_CAN_DEFAULT);

	okbutton6 = gtk_button_new_with_mnemonic (_("Save"));
	gtk_widget_show (okbutton6);
	gtk_container_add (GTK_CONTAINER(dialog_action_area6), okbutton6);
	GTK_WIDGET_SET_FLAGS (okbutton6, GTK_CAN_DEFAULT);

	g_signal_connect(button5,   "clicked", G_CALLBACK (on_pick_file_clicked), this);
	g_signal_connect(okbutton6, "clicked", G_CALLBACK (on_save_clicked), this);
	g_signal_connect(combobox2, "changed", G_CALLBACK (on_format_combo_selected), this);
	g_signal_connect(checkbox1, "clicked", G_CALLBACK (on_use_custom_css), this);
	g_signal_connect(checkbox3, "clicked", G_CALLBACK (on_use_post_export), this);

	/* Store pointers to all widgets, for use by lookup_widget(). */
	GLADE_HOOKUP_OBJECT_NO_REF (file_export_dialog, file_export_dialog, "file_export_dialog");
	GLADE_HOOKUP_OBJECT_NO_REF (file_export_dialog, dialog_vbox6, "dialog_vbox6");
	GLADE_HOOKUP_OBJECT (file_export_dialog, table7, "table7");
	GLADE_HOOKUP_OBJECT (file_export_dialog, label24, "label24");
	GLADE_HOOKUP_OBJECT (file_export_dialog, label25, "label25");
	GLADE_HOOKUP_OBJECT (file_export_dialog, combobox1, "combobox1");
	GLADE_HOOKUP_OBJECT (file_export_dialog, combobox2, "combobox2");
	GLADE_HOOKUP_OBJECT (file_export_dialog, label26, "label26");
	GLADE_HOOKUP_OBJECT (file_export_dialog, hbox3, "hbox3");
	GLADE_HOOKUP_OBJECT (file_export_dialog, entry5, "entry5");
	GLADE_HOOKUP_OBJECT (file_export_dialog, button5, "button5");
	GLADE_HOOKUP_OBJECT (file_export_dialog, image2, "image2");
	GLADE_HOOKUP_OBJECT (file_export_dialog, checkbox1, "checkbox1");
	GLADE_HOOKUP_OBJECT (file_export_dialog, checkbox2, "checkbox2");
	GLADE_HOOKUP_OBJECT (file_export_dialog, checkbox3, "checkbox3");
	GLADE_HOOKUP_OBJECT (file_export_dialog, combobox3, "combobox3");
	GLADE_HOOKUP_OBJECT (file_export_dialog, entry6, "entry6");
	GLADE_HOOKUP_OBJECT (file_export_dialog, hbox4, "hbox4");
	GLADE_HOOKUP_OBJECT_NO_REF (file_export_dialog, dialog_action_area6, "dialog_action_area6");
	GLADE_HOOKUP_OBJECT (file_export_dialog, cancelbutton6, "cancelbutton6");
	GLADE_HOOKUP_OBJECT (file_export_dialog, okbutton6, "okbutton6");

	gtk_widget_grab_focus (entry5);
	gtk_widget_grab_default (okbutton6);

	return file_export_dialog;
}

void on_pick_file_clicked (GtkButton *button, gpointer user_data)
{
	FileExportDlg *pDlg = (FileExportDlg *)user_data;
	GtkWidget *combobox2 = lookup_widget(pDlg->m_pDialog, "combobox2");
	GtkWidget *entry5    = lookup_widget(pDlg->m_pDialog, "entry5");

	int nFmt = gtk_combo_box_get_active (GTK_COMBO_BOX (combobox2));

	FileDialog dlg(FALSE, FALSE, TRUE, _("Select export file"));

	//set initial directory using the file name already set in the entry field
	std::string strFile;
	strFile = gtk_entry_get_text(GTK_ENTRY(entry5));
	std::string::size_type nPos = strFile.find_last_of("/\\");
	if(nPos != std::string::npos)
		strFile = strFile.substr(0, nPos+1);

	dlg.SetDirectory(strFile.c_str());

	//set single format filter based on current format selection
	if(nFmt >= 0)
		dlg.AddFilter(pDlg->m_formats[nFmt].szName, pDlg->m_formats[nFmt].szFilter);

	// pick file name and write it to text field
	if(dlg.DoModal())
	{
		gtk_entry_set_text(GTK_ENTRY(entry5), dlg.GetFilename());
		pDlg->ForceFormatExtension();
	}
}

void on_save_clicked (GtkButton *button, gpointer user_data)
{
	FileExportDlg *pDlg = (FileExportDlg *)user_data;

	const char *szFile = pDlg->GetExportFile();
	if(NULL == szFile || 0 == strlen(szFile))
	{
		gtkMessageBox(_("You must define the export file name!"));
		return;
	}

	pDlg->ForceFormatExtension();

	// remeber state in INI
	GtkWidget *combobox1 = lookup_widget(pDlg->m_pDialog, "combobox1");
	GtkWidget *combobox2 = lookup_widget(pDlg->m_pDialog, "combobox2");
	GtkWidget *entry5	 = lookup_widget(pDlg->m_pDialog, "entry5");

	int nExportSource = gtk_combo_box_get_active (GTK_COMBO_BOX (combobox1));
	g_objIni.SetValue("Export", "DocumentSource", nExportSource);
	int nExportFormat = gtk_combo_box_get_active (GTK_COMBO_BOX (combobox2));
	g_objIni.SetValue("Export", "TargetFormat", nExportFormat);
	std::string strFile = gtk_entry_get_text(GTK_ENTRY(entry5));
	g_objIni.SetValue("Export", "TargetFile", strFile.c_str());
	g_objIni.Save();

	//terminate with OK
	gtk_dialog_response(GTK_DIALOG(pDlg->m_pDialog), GTK_RESPONSE_OK);
}

void on_format_combo_selected (GtkComboBox *widget, gpointer user_data)
{
	FileExportDlg *pDlg = (FileExportDlg *)user_data;
	pDlg->ForceFormatExtension();

	//additional export options are	visible only for HTML format
	GtkWidget *hbox4 = lookup_widget(pDlg->m_pDialog, "hbox4");
	GtkWidget *checkbox2 = lookup_widget(pDlg->m_pDialog, "checkbox2");

	if(3 == pDlg->GetExportFormat()){
		gtk_widget_show(hbox4);
		gtk_widget_show(checkbox2);
	}
	else
	{
		gtk_widget_hide(hbox4);
		gtk_widget_hide(checkbox2);
	}
}

void on_use_custom_css (GtkMenuItem *menuitem, gpointer user_data)
{
	FileExportDlg *pDlg = (FileExportDlg *)user_data;

	GtkWidget *checkbox1 = lookup_widget(pDlg->m_pDialog, "checkbox1");
	GtkWidget *entry6 = lookup_widget(pDlg->m_pDialog, "entry6");

	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(checkbox1)))
		gtk_widget_set_sensitive(entry6, TRUE);
	else
		gtk_widget_set_sensitive(entry6, FALSE);
}

void on_use_post_export (GtkMenuItem *menuitem, gpointer user_data)
{
	FileExportDlg *pDlg = (FileExportDlg *)user_data;

	GtkWidget *checkbox3 = lookup_widget(pDlg->m_pDialog, "checkbox3");
	GtkWidget *combobox3 = lookup_widget(pDlg->m_pDialog, "combobox3");

	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(checkbox3)))
		gtk_widget_set_sensitive(combobox3, TRUE);
	else
		gtk_widget_set_sensitive(combobox3, FALSE);
}
