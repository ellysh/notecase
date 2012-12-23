////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements window for file "save as" operation
////////////////////////////////////////////////////////////////////////////

#include "FileSaveAsDlg.h"
#include "gui/FileDialog.h"
#include "lib/IniFile.h"
#include "lib/FilePath.h"
#include "support.h"
#include "interface.h"
#include <string.h>
#include <string>
#include <algorithm>
#include "mru.h"
#include "lib/debug.h"

extern GtkWidget *window1;
extern IniFile g_objIni;

static void on_pick_dir_clicked(GtkButton *button, gpointer user_data);
static void on_home_dir_clicked(GtkButton *button, gpointer user_data);
#ifdef _NOKIA_MAEMO
 static void on_mmc1_clicked(GtkButton *button, gpointer user_data);
 static void on_mmc2_clicked(GtkButton *button, gpointer user_data);
#endif
static void on_save_clicked(GtkButton *button, gpointer user_data);
static void on_format_combo_selected (GtkComboBox *widget, gpointer user_data);

FileSaveAsDlg::FileSaveAsDlg()
{
	// init formats table
	m_formats[0].szName   = _("NoteCase document (*.ncd)");
	m_formats[0].szFilter = "*.ncd";
	m_formats[1].szName   = _("NoteCase encrypted document (*.nce)");
	m_formats[1].szFilter = "*.nce";
}

FileSaveAsDlg::~FileSaveAsDlg()
{
	Destroy();
	gtk_window_present(GTK_WINDOW(window1)); //activate main window
}

void FileSaveAsDlg::Create()
{
	m_pDialog = create_file_export_dialog (window1);

	// fill format combo box
	GtkWidget *combobox2 = lookup_widget(m_pDialog, "combobox2");
	for(unsigned int i=0; i<sizeof(m_formats)/sizeof(m_formats[0]); i++)
		gtk_combo_box_append_text (GTK_COMBO_BOX (combobox2), m_formats[i].szName);

	//set initial name/path
	GtkWidget *entry4 = lookup_widget(m_pDialog, "entry4");
	gtk_entry_set_text(GTK_ENTRY(entry4), m_strFileName.c_str());

	GtkWidget *entry5 = lookup_widget(m_pDialog, "entry5");
	gtk_entry_set_text(GTK_ENTRY(entry5), m_strFileDir.c_str());

	//restore last used export format in INI
	int nExportFormat = 0;
	g_objIni.GetValue("Save", "TargetFormat", nExportFormat, 0);
	if(0 > nExportFormat || nExportFormat > 1)
		nExportFormat = 0;

	gtk_combo_box_set_active (GTK_COMBO_BOX (combobox2), nExportFormat);
}

void FileSaveAsDlg::ForceFormatExtension()
{
	std::string strFile = GetFileName();
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

			//strip previous extension TOFIX?
			std::string::size_type nExt = strFile.rfind('.');
			if(nExt != std::string::npos)
				strFile = strFile.substr(0, nExt);

			//append extension
			strFile += strExt;

			//set new name
			GtkWidget *entry4 = lookup_widget(m_pDialog, "entry4");
			gtk_entry_set_text(GTK_ENTRY(entry4), strFile.c_str());
		}
	}
}

const char *FileSaveAsDlg::GetSelectedFormat()
{
	int nIdx = GetSelectedFormatIdx();
	if(nIdx >= 0)
		return m_formats[nIdx].szFilter;
	return NULL;
}

int FileSaveAsDlg::GetSelectedFormatIdx()
{
	GtkWidget *combobox2 = lookup_widget(m_pDialog, "combobox2");
	return 	gtk_combo_box_get_active (GTK_COMBO_BOX (combobox2));
}

const char *FileSaveAsDlg::GetFileName()
{
	GtkWidget *entry4 = lookup_widget(m_pDialog, "entry4");
	return 	gtk_entry_get_text(GTK_ENTRY(entry4));
}

const char *FileSaveAsDlg::GetFilePath()
{
	GtkWidget *entry5 = lookup_widget(m_pDialog, "entry5");
	return 	gtk_entry_get_text(GTK_ENTRY(entry5));
}

GtkWidget* FileSaveAsDlg::create_file_export_dialog (GtkWidget* parent)
{
	GtkWidget *file_export_dialog;
	GtkWidget *dialog_vbox6;
	GtkWidget *table7;
	GtkWidget *label25;
	GtkWidget *combobox2;
	GtkWidget *label24;
	GtkWidget *entry4;
	GtkWidget *label26;
	GtkWidget *hbox3;
	GtkWidget *entry5;
	GtkWidget *button5;
	GtkWidget *button6;
#ifdef _NOKIA_MAEMO
	GtkWidget *button7;
	GtkWidget *button8;
#endif
	GtkWidget *image2;
	GtkWidget *dialog_action_area6;
	GtkWidget *cancelbutton6;
	GtkWidget *okbutton6;

	file_export_dialog = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (file_export_dialog), _("Save document"));
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

	dialog_vbox6 = GTK_DIALOG (file_export_dialog)->vbox;
	gtk_widget_show (dialog_vbox6);

	table7 = gtk_table_new (3, 2, FALSE);
	gtk_widget_show (table7);
	gtk_box_pack_start (GTK_BOX (dialog_vbox6), table7, TRUE, TRUE, 0);

	label24 = gtk_label_new (_("File name:"));
	gtk_widget_show (label24);
	gtk_table_attach (GTK_TABLE (table7), label24, 0, 1, 0, 1,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label24), 0, 0.5);

	entry4 = gtk_entry_new ();
	gtk_widget_show (entry4);
	gtk_table_attach (GTK_TABLE (table7), entry4, 1, 2, 0, 1,
		(GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_activates_default (GTK_ENTRY (entry4), TRUE);

	label26 = gtk_label_new (_("File directory:"));
	gtk_widget_show (label26);
	gtk_table_attach (GTK_TABLE (table7), label26, 0, 1, 1, 2,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label26), 0, 0.5);

	hbox3 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox3);
	gtk_table_attach (GTK_TABLE (table7), hbox3, 1, 2, 1, 2,
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

	button6 = gtk_button_new ();
	gtk_widget_show (button6);
	gtk_box_pack_start (GTK_BOX (hbox3), button6, FALSE, FALSE, 0);

	image2 = gtk_image_new_from_stock (GTK_STOCK_HOME, GTK_ICON_SIZE_BUTTON);
	gtk_widget_show (image2);
	gtk_container_add (GTK_CONTAINER (button6), image2);

#ifdef _NOKIA_MAEMO
	button7 = gtk_button_new ();
	gtk_widget_show (button7);
	gtk_box_pack_start (GTK_BOX (hbox3), button7, FALSE, FALSE, 0);

	image2 = gtk_image_new_from_stock (GTK_STOCK_HARDDISK, GTK_ICON_SIZE_BUTTON);
	gtk_widget_show (image2);
	gtk_container_add (GTK_CONTAINER (button7), image2);

	button8 = gtk_button_new ();
	gtk_widget_show (button8);
	gtk_box_pack_start (GTK_BOX (hbox3), button8, FALSE, FALSE, 0);

	image2 = gtk_image_new_from_stock (GTK_STOCK_FLOPPY, GTK_ICON_SIZE_BUTTON);
	gtk_widget_show (image2);
	gtk_container_add (GTK_CONTAINER (button8), image2);
#endif

	label25 = gtk_label_new (_("File format:"));
	gtk_widget_show (label25);
	gtk_table_attach (GTK_TABLE (table7), label25, 0, 1, 2, 3,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label25), 0, 0.5);

	combobox2 = gtk_combo_box_new_text ();
	gtk_widget_show (combobox2);
	gtk_table_attach (GTK_TABLE (table7), combobox2, 1, 2, 2, 3,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (GTK_FILL), 0, 0);

	dialog_action_area6 = GTK_DIALOG (file_export_dialog)->action_area;
	gtk_widget_show (dialog_action_area6);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area6), GTK_BUTTONBOX_END);

	cancelbutton6 = gtk_button_new_from_stock ("gtk-cancel");
	gtk_widget_show (cancelbutton6);
	gtk_dialog_add_action_widget (GTK_DIALOG (file_export_dialog), cancelbutton6, GTK_RESPONSE_CANCEL);
	GTK_WIDGET_SET_FLAGS (cancelbutton6, GTK_CAN_DEFAULT);

	okbutton6 = gtk_button_new_with_mnemonic (_("_Save"));
	gtk_widget_show (okbutton6);
	gtk_container_add (GTK_CONTAINER(dialog_action_area6), okbutton6);
	GTK_WIDGET_SET_FLAGS (okbutton6, GTK_CAN_DEFAULT);

	g_signal_connect(button5,   "clicked", G_CALLBACK (on_pick_dir_clicked), this);
	g_signal_connect(button6,   "clicked", G_CALLBACK (on_home_dir_clicked), this);
#ifdef _NOKIA_MAEMO
	g_signal_connect(button7,   "clicked", G_CALLBACK (on_mmc2_clicked), this);
	g_signal_connect(button8,   "clicked", G_CALLBACK (on_mmc1_clicked), this);
#endif

	g_signal_connect(okbutton6, "clicked", G_CALLBACK (on_save_clicked), this);
	g_signal_connect(combobox2, "changed", G_CALLBACK (on_format_combo_selected), this);

	/* Store pointers to all widgets, for use by lookup_widget(). */
	GLADE_HOOKUP_OBJECT_NO_REF (file_export_dialog, file_export_dialog, "file_export_dialog");
	GLADE_HOOKUP_OBJECT_NO_REF (file_export_dialog, dialog_vbox6, "dialog_vbox6");
	GLADE_HOOKUP_OBJECT (file_export_dialog, table7, "table7");
	GLADE_HOOKUP_OBJECT (file_export_dialog, label25, "label25");
	GLADE_HOOKUP_OBJECT (file_export_dialog, combobox2, "combobox2");
	GLADE_HOOKUP_OBJECT (file_export_dialog, entry4, "entry4");
	GLADE_HOOKUP_OBJECT (file_export_dialog, label26, "label26");
	GLADE_HOOKUP_OBJECT (file_export_dialog, hbox3, "hbox3");
	GLADE_HOOKUP_OBJECT (file_export_dialog, entry5, "entry5");
	GLADE_HOOKUP_OBJECT (file_export_dialog, button5, "button5");
	GLADE_HOOKUP_OBJECT (file_export_dialog, image2, "image2");
	GLADE_HOOKUP_OBJECT_NO_REF (file_export_dialog, dialog_action_area6, "dialog_action_area6");
	GLADE_HOOKUP_OBJECT (file_export_dialog, cancelbutton6, "cancelbutton6");
	GLADE_HOOKUP_OBJECT (file_export_dialog, okbutton6, "okbutton6");

	gtk_widget_grab_focus (entry5);
	gtk_widget_grab_default (okbutton6);

	return file_export_dialog;
}

void on_pick_dir_clicked (GtkButton *button, gpointer user_data)
{
	FileSaveAsDlg *pDlg = (FileSaveAsDlg *)user_data;
	GtkWidget *entry5    = lookup_widget(pDlg->m_pDialog, "entry5");

	std::string strDir = pDlg->GetFilePath();

	//TOFIX set initial directory from INI (store last used)
	FileDialog dlg(FALSE, TRUE, TRUE, _("Select target directory"));

	if(strDir.size() > 0)
		dlg.SetDirectory(strDir.c_str());

	if(dlg.DoModal())
	{
		gtk_entry_set_text(GTK_ENTRY(entry5), dlg.GetFilename());
		pDlg->ForceFormatExtension();
	}
}

void on_home_dir_clicked (GtkButton *button, gpointer user_data)
{
	std::string strDefaultDir = GetHomeDir();
	EnsureTerminated(strDefaultDir, '/');
	#ifdef _NOKIA_MAEMO
		//Maemo file selector is not easy to use, by default set correct document directory
		strDefaultDir += "MyDocs/.documents/";
	#endif

	FileSaveAsDlg *pDlg = (FileSaveAsDlg *)user_data;
	GtkWidget *entry5    = lookup_widget(pDlg->m_pDialog, "entry5");
	gtk_entry_set_text(GTK_ENTRY(entry5), strDefaultDir.c_str());
	pDlg->ForceFormatExtension();
}

void on_mmc1_clicked(GtkButton *button, gpointer user_data)
{
	std::string strDefaultDir = "/media/mmc1/";

	FileSaveAsDlg *pDlg = (FileSaveAsDlg *)user_data;
	GtkWidget *entry5    = lookup_widget(pDlg->m_pDialog, "entry5");
	gtk_entry_set_text(GTK_ENTRY(entry5), strDefaultDir.c_str());
	pDlg->ForceFormatExtension();
}

void on_mmc2_clicked(GtkButton *button, gpointer user_data)
{
	std::string strDefaultDir = "/media/mmc2/";

	FileSaveAsDlg *pDlg = (FileSaveAsDlg *)user_data;
	GtkWidget *entry5    = lookup_widget(pDlg->m_pDialog, "entry5");
	gtk_entry_set_text(GTK_ENTRY(entry5), strDefaultDir.c_str());
	pDlg->ForceFormatExtension();
}

void on_save_clicked (GtkButton *button, gpointer user_data)
{
	FileSaveAsDlg *pDlg = (FileSaveAsDlg *)user_data;

	const char *szFile = pDlg->GetFilePath();
	if(NULL == szFile || 0 == strlen(szFile))
	{
		gtkMessageBox(_("File name must not be empty!"));
		return;
	}

	pDlg->ForceFormatExtension();

	// remeber state in INI
	GtkWidget *combobox2 = lookup_widget(pDlg->m_pDialog, "combobox2");

	int nExportFormat = gtk_combo_box_get_active (GTK_COMBO_BOX (combobox2));
	g_objIni.SetValue("Save", "TargetFormat", nExportFormat);
	g_objIni.Save();

	//terminate with OK
	gtk_dialog_response(GTK_DIALOG(pDlg->m_pDialog), GTK_RESPONSE_OK);
}

void on_format_combo_selected (GtkComboBox *widget, gpointer user_data)
{
	FileSaveAsDlg *pDlg = (FileSaveAsDlg *)user_data;
	pDlg->ForceFormatExtension();
}
