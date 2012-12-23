////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Dialog to view/edit document node properties
////////////////////////////////////////////////////////////////////////////

#include "NodePropertiesDlg.h"
#include "support.h"
#include "callbacks.h"
#include "gui/FileDialog.h"
#include "lib/IniFile.h"
#include "lib/FilePath.h"
#include "lib/debug.h"
#include "config.h"
#include <gdk/gdkkeysyms.h>

#ifdef HAVE_GTKSOURCEVIEW
	#include <gtksourceview/gtksourceview.h>
	#include <gtksourceview/gtksourcelanguage.h>
	#include <gtksourceview/gtksourcelanguagemanager.h>
#endif

#ifdef _WIN32
 #ifndef __MINGW32__
   #ifndef snprintf
    #define snprintf _snprintf
   #endif
 #endif
 #include <io.h> //access
 #define access _access
#endif

extern GtkWidget *g_pNodePropWidget;
extern GtkWidget *window1;
extern IniFile g_objIni;
static GtkWidget *g_cancelbtn = NULL;
static GtkWidget *g_editor = NULL;
extern bool g_bCloseDlgOnTextEnter;

const char *InternalIcon_Index2Name(int nIndex);
const char **InternalIcon_GetFromIdx(int);
int InternalIcon_Name2Index(const char *);
int gtkMessageBox(const char *szText, int nButtons = GTK_BUTTONS_OK, int nIcon = GTK_MESSAGE_INFO);
const char *combo_get_text(GtkWidget *cbo3);
static gint entry_keyboard_handler(GtkWidget *widget, GdkEventKey *event);
static void on_icon_none_clicked (GtkMenuItem *menuitem, gpointer user_data);
static void on_icon_internal_clicked (GtkMenuItem *menuitem, gpointer user_data);
static void on_icon_custom_clicked (GtkMenuItem *menuitem, gpointer user_data);
static void on_xpm_select_clicked (GtkMenuItem *menuitem, gpointer user_data);
static void on_internal_combo_selected (GtkComboBox *widget, gpointer user_data);
static void on_keyword_add_selected (GtkMenuItem *menuitem, gpointer user_data);
static void on_keyword_remove_selected (GtkMenuItem *menuitem, gpointer user_data);
static void on_finished_checked (GtkMenuItem *menuitem, gpointer user_data);
static void FormatTime(time_t nTime, std::string& strTxt);

NodePropertiesDlg::NodePropertiesDlg()
{
	m_bFinished = false;
}

NodePropertiesDlg::~NodePropertiesDlg()
{
	g_pNodePropWidget = NULL;
	Destroy();
	gtk_window_present(GTK_WINDOW(window1)); //activate main window
}

void NodePropertiesDlg::Create()
{
	m_pDialog = create_node_properties_dialog ();
	g_pNodePropWidget = m_pDialog;

	//set node title
	if(m_strTitle.c_str())
	{
		GtkWidget *entry = lookup_widget(m_pDialog, "entry_node_title");
		//gtk_entry_set_text(GTK_ENTRY(entry), m_strTitle.c_str());
		GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(entry));
		gtk_text_buffer_set_text(buffer, m_strTitle.c_str(), -1);
	}

	//fill internal icons combo
	GtkWidget *cbo2	= lookup_widget(m_pDialog, "comboboxentry2");
	int i;
	for (i=ICON_INTERNAL_FIRST; i<=ICON_INTERNAL_LAST; i++)
		gtk_combo_box_append_text(GTK_COMBO_BOX(cbo2), InternalIcon_Index2Name(i));

	//select first entry in combo (must not be empty selection)
	gtk_combo_box_set_active(GTK_COMBO_BOX(cbo2), 0);

	//fill keywords combo
	GtkWidget *cbo3	= lookup_widget(m_pDialog, "comboboxentry3");
	std::string::size_type nPos = m_strKeywords.find(';');
	while(nPos != std::string::npos){
		std::string strWord = m_strKeywords.substr(0, nPos);
		m_strKeywords = m_strKeywords.substr(nPos+1);
		if(strWord.size()>0)
			gtk_combo_box_append_text(GTK_COMBO_BOX(cbo3), strWord.c_str());
		nPos = m_strKeywords.find(';');
	}
	if(m_strKeywords.size()>0)
		gtk_combo_box_append_text(GTK_COMBO_BOX(cbo3), m_strKeywords.c_str());
	//select first entry in combo
	gtk_combo_box_set_active(GTK_COMBO_BOX(cbo3), 0);

#ifdef HAVE_GTKSOURCEVIEW
	//fill source language combo
	GtkWidget *cbolang = lookup_widget(m_pDialog, "source_lang_combo");
	// first entry is empty - empty means no source language selected
	gtk_combo_box_append_text(GTK_COMBO_BOX(cbolang), "");
	GtkSourceLanguageManager *manager = gtk_source_language_manager_get_default();
	const gchar * const *languages = gtk_source_language_manager_get_language_ids(manager);

	int active = -1;
	i = 0;
	while (*languages != NULL)
	{
		i++;
		gtk_combo_box_append_text(GTK_COMBO_BOX(cbolang), *languages);
		if (0 == strcmp(m_strSourceLanguage.c_str(), *languages))
		{
			active = i;
		}
		++languages;
	}
	//select active entry in combo
	if(active >= 0)
		gtk_combo_box_set_active(GTK_COMBO_BOX(cbolang), active);
#endif

	//set dates (read only)
	GtkWidget *label_created	= lookup_widget(m_pDialog, "label_created");
	GtkWidget *label_modified	= lookup_widget(m_pDialog, "label_modified");

	//display node dates
	std::string strTxt;
	FormatTime(m_nCreated, strTxt);
	gtk_label_set_text(GTK_LABEL(label_created), strTxt.c_str());
	FormatTime(m_nModified, strTxt);
	gtk_label_set_text(GTK_LABEL(label_modified), strTxt.c_str());

	//set finished
	GtkWidget *chk_finished	= lookup_widget(m_pDialog, "chk_finished");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_finished), m_bFinished);
}

std::string NodePropertiesDlg::GetNodeTitle()
{
	GtkWidget *entry = lookup_widget(m_pDialog, "entry_node_title");
	//return gtk_entry_get_text(GTK_ENTRY(entry));
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(entry));
	GtkTextIter start,end;
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_get_end_iter(buffer, &end);
	gchar *pszData = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
	std::string strData = pszData;
	g_free(pszData);
	return strData;
}

void NodePropertiesDlg::RefreshKeywords()
{
	GtkWidget *cbo3	= lookup_widget(m_pDialog, "comboboxentry3");
	GtkTreeModel* model = gtk_combo_box_get_model (GTK_COMBO_BOX(cbo3));

	m_strKeywords = "";
	gint nSize = gtk_tree_model_iter_n_children  (model, NULL);
	for(int i=0; i<nSize; i++)
	{
		GtkTreeIter iter;
		gtk_tree_model_iter_nth_child (model, &iter, NULL, i);

		gchar *value = NULL;
		gtk_tree_model_get (model, &iter, 0, &value, -1);

		m_strKeywords += ";";
		m_strKeywords += value;

		g_free(value);
	}
	if(nSize > 0)
		m_strKeywords += ";";	//terminate list with delimiter
}

int NodePropertiesDlg::GetIconType()
{
	GtkWidget *radio_ico_none 	= lookup_widget(m_pDialog, "radio_ico_none");
	GtkWidget *radio_ico_internal	= lookup_widget(m_pDialog, "radio_ico_internal");

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_ico_none)))
		return ICON_NONE;
	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_ico_internal)))
		return ICON_INTERNAL;
	return ICON_CUSTOM;
}

const char *NodePropertiesDlg::GetIconValue()
{
	GtkWidget *cbo2		= lookup_widget(m_pDialog, "comboboxentry2");
	GtkWidget *entry4	= lookup_widget(m_pDialog, "entry4");

	switch (GetIconType())
	{
		case ICON_INTERNAL:	return combo_get_text(cbo2);
		case ICON_CUSTOM:	return gtk_entry_get_text(GTK_ENTRY(entry4));
		default: 			return "";
	}
}

void NodePropertiesDlg::SetIconType(int nType)
{
	GtkWidget *radio_ico_none 	= lookup_widget(m_pDialog, "radio_ico_none");
	GtkWidget *radio_ico_internal	= lookup_widget(m_pDialog, "radio_ico_internal");
	GtkWidget *radio_ico_custom	= lookup_widget(m_pDialog, "radio_ico_custom");
	GtkWidget *cbo2			= lookup_widget(m_pDialog, "comboboxentry2");

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

void NodePropertiesDlg::SetIconValue(const char *szValue)
{
	GtkWidget *entry4 = lookup_widget(m_pDialog, "entry4");
	gtk_entry_set_text(GTK_ENTRY(entry4), szValue);

	UpdatePreview();
}

void NodePropertiesDlg::KeywordAdd()
{
	GtkWidget *cbo3	= lookup_widget(m_pDialog, "comboboxentry3");
	ASSERT(NULL != cbo3);

	std::string strWord = combo_get_text(cbo3);
	if(strWord.size() > 0)
	{
		TRACE("Add tag: %s\n", strWord.c_str());

		//check if valid keyword
		if(std::string::npos != strWord.find(';')){
			gtkMessageBox(_("Tag must not contain \";\" character!"));
			return;
		}

		//check if keyword already exists
		GtkTreeModel* model = gtk_combo_box_get_model (GTK_COMBO_BOX(cbo3));
		gint nSize = gtk_tree_model_iter_n_children  (model, NULL);
		for(int i=0; i<nSize; i++)
		{
			GtkTreeIter iter;
			gtk_tree_model_iter_nth_child (model, &iter, NULL, i);
			gchar *value = NULL;
			gtk_tree_model_get (model, &iter, 0, &value, -1);

			if(value && 0 == strcmp(strWord.c_str(), value))
			{
				gtkMessageBox(_("This tag was already added for this node!"));
				g_free(value);
				return;
			}
			g_free(value);
		}

		TRACE("Append tag\n");

		//add the keyword
		gtk_combo_box_append_text(GTK_COMBO_BOX(cbo3), strWord.c_str());
	}
}

void NodePropertiesDlg::KeywordRemove()
{
	GtkWidget *cbo3	= lookup_widget(m_pDialog, "comboboxentry3");

	std::string strWord = combo_get_text(cbo3);
	if(strWord.size() > 0)
	{
		//search for the keyword in the combo
		GtkTreeModel* model = gtk_combo_box_get_model (GTK_COMBO_BOX(cbo3));
		gint nSize = gtk_tree_model_iter_n_children  (model, NULL);
		for(int i=0; i<nSize; i++)
		{
			GtkTreeIter iter;
			gtk_tree_model_iter_nth_child (model, &iter, NULL, i);
			gchar *value = NULL;
			gtk_tree_model_get (model, &iter, 0, &value, -1);

			if(0 == strcmp(strWord.c_str(), value))
			{
				//remove the keyword
				gtk_combo_box_remove_text(GTK_COMBO_BOX(cbo3), i);
				g_free(value);
				return;
			}
			g_free(value);
		}
	}

	//TOFIX how to clear entry part of the combo
	//gtk_combo_box_set_active(GTK_COMBO_BOX(cbo3), -1);
}

GtkWidget* NodePropertiesDlg::create_node_properties_dialog ()
{
	GtkWidget *node_properties_dialog;
	GtkWidget *dialog_vbox5;
	GtkWidget *table4;
	GtkWidget *label21;
	GtkWidget *entry_node_title;
	GtkWidget *label22;
	GtkWidget *vbox10;
	GtkWidget *hbox2;
	GtkWidget *table6;
	GtkWidget *radio_ico_none;
	GSList    *radio_ico_none_group = NULL;
	GtkWidget *radio_ico_internal;
	GtkWidget *radio_ico_custom;
	GtkWidget *entry4;
	GtkWidget *comboboxentry3;
	GtkWidget *hbox3;
#ifdef HAVE_GTKSOURCEVIEW
	GtkWidget *source_lang_combo;
	GtkWidget *source_lang_label;
#endif
	GtkWidget *label24;
	GtkWidget *image1;
	GtkWidget *addbtn;
	GtkWidget *removebtn;
	GtkWidget *button_xpm_select;
	GtkWidget *comboboxentry2;
	GtkWidget *dialog_action_area5;
	GtkWidget *cancelbutton5;
	GtkWidget *okbutton5;
	GtkWidget *label25;
	GtkWidget *label_created;
	GtkWidget *label26;
	GtkWidget *label_modified;
	GtkWidget *chk_finished;
	GtkWidget *scrolledwindow1;

	node_properties_dialog = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (node_properties_dialog), _("Node Properties"));
	gtk_window_set_type_hint (GTK_WINDOW (node_properties_dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator (GTK_DIALOG(node_properties_dialog), FALSE);

	dialog_vbox5 = GTK_DIALOG (node_properties_dialog)->vbox;
	gtk_widget_show (dialog_vbox5);

	table4 = gtk_table_new (2, 4, FALSE);
	gtk_widget_show (table4);
	gtk_box_pack_start (GTK_BOX (dialog_vbox5), table4, TRUE, TRUE, 0);

	label21 = gtk_label_new (_("Title:"));
	gtk_widget_show (label21);
	gtk_table_attach (GTK_TABLE (table4), label21, 0, 1, 0, 1,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label21), 0, 0.5);

	scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindow1);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_table_attach (GTK_TABLE (table4), scrolledwindow1, 1, 4, 0, 1,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);

	entry_node_title = gtk_text_view_new(); //gtk_entry_new
	gtk_widget_show (entry_node_title);
#ifdef _NOKIA_MAEMO
	gtk_widget_set_size_request(scrolledwindow1, -1, 25); // preserve the screen estate
	gtk_widget_set_size_request(entry_node_title, -1, 25); // preserve the screen estate
#else
	gtk_widget_set_size_request(entry_node_title, -1, 30);
#endif
	gtk_container_add (GTK_CONTAINER (scrolledwindow1), entry_node_title);
	g_editor = entry_node_title;

	//attach accelerator
	GtkAccelGroup *accel_group = gtk_accel_group_new ();
	GClosure *closure = g_cclosure_new (G_CALLBACK (on_menu_insert_date_time), NULL, NULL);
	gtk_accel_group_connect (accel_group, 'T', (GdkModifierType)(GDK_CONTROL_MASK|GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE, closure);
	g_closure_unref (closure);
	gtk_window_add_accel_group (GTK_WINDOW (node_properties_dialog), accel_group);

	label25 = gtk_label_new (_("Created:"));
	gtk_widget_show (label25);
	gtk_table_attach (GTK_TABLE (table4), label25, 0, 1, 1, 2,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label25), 0, 0.5);

	label_created = gtk_label_new (_("unknown"));
	gtk_widget_show (label_created);
	gtk_table_attach (GTK_TABLE (table4), label_created, 1, 2, 1, 2,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label_created), 0, 0.5);

	label26 = gtk_label_new (_("Modified:"));
	gtk_widget_show (label26);
	gtk_table_attach (GTK_TABLE (table4), label26, 2, 3, 1, 2,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label26), 0, 0.5);

	label_modified = gtk_label_new (_("unknown"));
	gtk_widget_show (label_modified);
	gtk_table_attach (GTK_TABLE (table4), label_modified, 3, 4, 1, 2,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label_modified), 0, 0.5);

	vbox10 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox10);
	gtk_table_attach (GTK_TABLE (table4), vbox10, 0, 4, 2, 3,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (GTK_FILL), 0, 0);

	hbox2 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox2);
	gtk_box_pack_start (GTK_BOX (vbox10), hbox2, FALSE, FALSE, 0);

	label22 = gtk_label_new (_("Icon:"));
	gtk_widget_show (label22);
	gtk_box_pack_start (GTK_BOX (hbox2), label22, TRUE, TRUE, 0);
	gtk_misc_set_alignment (GTK_MISC (label22), 0, 0);

	radio_ico_none = gtk_radio_button_new_with_mnemonic (NULL, _("None"));
	gtk_widget_show (radio_ico_none);
	gtk_box_pack_start (GTK_BOX (hbox2), radio_ico_none, TRUE, TRUE, 0);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (radio_ico_none), radio_ico_none_group);
	radio_ico_none_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio_ico_none));

	radio_ico_internal = gtk_radio_button_new_with_mnemonic (NULL, _("Internal"));
	gtk_widget_show (radio_ico_internal);
	gtk_box_pack_start (GTK_BOX (hbox2), radio_ico_internal, TRUE, TRUE, 0);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (radio_ico_internal), radio_ico_none_group);
	radio_ico_none_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio_ico_internal));

	radio_ico_custom = gtk_radio_button_new_with_mnemonic (NULL, _("Custom"));
	gtk_widget_show (radio_ico_custom);
	gtk_box_pack_start (GTK_BOX (hbox2), radio_ico_custom, TRUE, TRUE, 0);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (radio_ico_custom), radio_ico_none_group);
	radio_ico_none_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio_ico_custom));

	hbox3 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox3);
	gtk_box_pack_start (GTK_BOX (vbox10), hbox3, FALSE, FALSE, 0);

	image1 = gtk_image_new ();
	gtk_widget_show (image1);
	gtk_widget_set_size_request(image1, -1, 20);
	gtk_box_pack_start (GTK_BOX (hbox3), image1, FALSE, FALSE, 20);

	table6 = gtk_table_new (4, 3, FALSE);
	gtk_widget_show (table6);
	gtk_box_pack_start (GTK_BOX (hbox3), table6, TRUE, TRUE, 5);

	comboboxentry2 = gtk_combo_box_new_text ();
	gtk_widget_show (comboboxentry2);
	gtk_table_attach (GTK_TABLE (table6), comboboxentry2, 1, 2, 1, 2,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (GTK_FILL), 0, 0);

	entry4 = gtk_entry_new ();
	gtk_entry_set_editable (GTK_ENTRY(entry4), FALSE);
	gtk_widget_show (entry4);
	gtk_table_attach (GTK_TABLE (table6), entry4, 1, 2, 2, 3,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);

	button_xpm_select = gtk_button_new_with_mnemonic ("...");
	gtk_widget_show (button_xpm_select);
	gtk_table_attach (GTK_TABLE (table6), button_xpm_select, 2, 3, 2, 3,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);

	label24 = gtk_label_new (_("Tags:"));
	gtk_widget_show (label24);
	gtk_table_attach (GTK_TABLE (table4), label24, 0, 1, 5, 6,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label24), 0, 0.5);

	hbox3 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox3);
	gtk_table_attach (GTK_TABLE (table4), hbox3, 1, 4, 5, 6,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);

	comboboxentry3 = gtk_combo_box_entry_new_text ();
	gtk_widget_show (comboboxentry3);
	gtk_box_pack_start (GTK_BOX (hbox3), comboboxentry3, TRUE, TRUE, 0);

	addbtn = gtk_button_new_with_mnemonic (_("Add"));
	gtk_widget_show (addbtn);
	gtk_box_pack_start (GTK_BOX (hbox3), addbtn, TRUE, TRUE, 0);

	removebtn = gtk_button_new_with_mnemonic (_("Remove"));
	gtk_widget_show (removebtn);
	gtk_box_pack_start (GTK_BOX (hbox3), removebtn, TRUE, TRUE, 0);

	chk_finished = gtk_check_button_new_with_mnemonic (_("Mark node as Completed"));
	gtk_widget_show (chk_finished);
	gtk_table_attach (GTK_TABLE (table4), chk_finished, 0, 2, 6, 7,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);

#ifdef HAVE_GTKSOURCEVIEW
	source_lang_label = gtk_label_new (_("Source Language:"));
	gtk_widget_show (source_lang_label);
	gtk_table_attach (GTK_TABLE (table4), source_lang_label, 0, 1, 7, 8,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (source_lang_label), 0, 0.5);

	source_lang_combo = gtk_combo_box_entry_new_text ();
	gtk_widget_show (source_lang_combo);
	gtk_table_attach (GTK_TABLE (table4), source_lang_combo, 1, 4, 7, 8,
		(GtkAttachOptions) (GTK_FILL),
		(GtkAttachOptions) (0), 0, 0);
#endif

	dialog_action_area5 = GTK_DIALOG (node_properties_dialog)->action_area;
	gtk_widget_show (dialog_action_area5);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area5), GTK_BUTTONBOX_END);

	cancelbutton5 = gtk_button_new_from_stock ("gtk-cancel");
	gtk_widget_show (cancelbutton5);
	gtk_dialog_add_action_widget (GTK_DIALOG (node_properties_dialog), cancelbutton5, GTK_RESPONSE_CANCEL);
	GTK_WIDGET_SET_FLAGS (cancelbutton5, GTK_CAN_DEFAULT);
	g_cancelbtn = cancelbutton5;

	okbutton5 = gtk_button_new_from_stock ("gtk-ok");
	gtk_widget_show (okbutton5);
	gtk_dialog_add_action_widget (GTK_DIALOG (node_properties_dialog), okbutton5, GTK_RESPONSE_OK);
	GTK_WIDGET_SET_FLAGS (okbutton5, GTK_CAN_DEFAULT);

	g_signal_connect (radio_ico_none,	"clicked", G_CALLBACK (on_icon_none_clicked), this);
	g_signal_connect (radio_ico_internal,	"clicked", G_CALLBACK (on_icon_internal_clicked), this);
	g_signal_connect (radio_ico_custom,	"clicked", G_CALLBACK (on_icon_custom_clicked), this);
	g_signal_connect (button_xpm_select,	"clicked", G_CALLBACK (on_xpm_select_clicked), this);
	g_signal_connect (comboboxentry2,	"changed", G_CALLBACK (on_internal_combo_selected), this);
	g_signal_connect (addbtn,		"clicked", G_CALLBACK (on_keyword_add_selected), this);
	g_signal_connect (removebtn,		"clicked", G_CALLBACK (on_keyword_remove_selected), this);
	g_signal_connect (chk_finished, "clicked",  G_CALLBACK (on_finished_checked), this);
	g_signal_connect(entry_node_title,	"key_press_event",  G_CALLBACK (entry_keyboard_handler), this);

	/* Store pointers to all widgets, for use by lookup_widget(). */
	GLADE_HOOKUP_OBJECT_NO_REF (node_properties_dialog, node_properties_dialog, "node_properties_dialog");
	GLADE_HOOKUP_OBJECT_NO_REF (node_properties_dialog, dialog_vbox5, "dialog_vbox5");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, table4, "table4");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, label21, "label21");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, entry_node_title, "entry_node_title");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, label22, "label22");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, vbox10, "vbox10");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, hbox2, "hbox2");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, table6, "table6");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, radio_ico_none, "radio_ico_none");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, radio_ico_internal, "radio_ico_internal");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, radio_ico_custom, "radio_ico_custom");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, entry4, "entry4");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, hbox3, "hbox3");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, label24, "label24");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, comboboxentry3, "comboboxentry3");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, addbtn, "addbtn");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, removebtn, "removebtn");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, image1, "image1");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, button_xpm_select, "button_xpm_select");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, comboboxentry2, "comboboxentry2");
	GLADE_HOOKUP_OBJECT_NO_REF (node_properties_dialog, dialog_action_area5, "dialog_action_area5");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, cancelbutton5, "cancelbutton5");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, okbutton5, "okbutton5");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, label_created, "label_created");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, label_modified, "label_modified");
	GLADE_HOOKUP_OBJECT (node_properties_dialog, chk_finished, "chk_finished");
#ifdef HAVE_GTKSOURCEVIEW
	GLADE_HOOKUP_OBJECT (node_properties_dialog, source_lang_combo, "source_lang_combo");
#endif

	return node_properties_dialog;
}

void on_icon_none_clicked(GtkMenuItem *menuitem, gpointer user_data)
{
	NodePropertiesDlg *pDlg = (NodePropertiesDlg *)user_data;

	GtkWidget *radio_ico_none = lookup_widget(pDlg->m_pDialog, "radio_ico_none");
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_ico_none)))
	{
		GtkWidget *cbo2				= lookup_widget(pDlg->m_pDialog, "comboboxentry2");
		GtkWidget *entry4			= lookup_widget(pDlg->m_pDialog, "entry4");
		GtkWidget *button			= lookup_widget(pDlg->m_pDialog, "button_xpm_select");

		gtk_widget_set_sensitive(cbo2, FALSE);
		gtk_widget_set_sensitive(entry4, FALSE);
		gtk_widget_set_sensitive(button, FALSE);
		gtk_widget_hide(cbo2);
		gtk_widget_hide(entry4);
		gtk_widget_hide(button);

		pDlg->UpdatePreview();
	}
}

void on_icon_internal_clicked(GtkMenuItem *menuitem, gpointer user_data)
{
	NodePropertiesDlg *pDlg = (NodePropertiesDlg *)user_data;

	GtkWidget *radio_ico_internal = lookup_widget(pDlg->m_pDialog, "radio_ico_internal");
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_ico_internal)))
	{
		GtkWidget *cbo2		= lookup_widget(pDlg->m_pDialog, "comboboxentry2");
		GtkWidget *entry4	= lookup_widget(pDlg->m_pDialog, "entry4");
		GtkWidget *button	= lookup_widget(pDlg->m_pDialog, "button_xpm_select");

		gtk_widget_set_sensitive(cbo2, TRUE);
		gtk_widget_set_sensitive(entry4, FALSE);
		gtk_widget_set_sensitive(button, FALSE);

		gtk_widget_show(cbo2);
		gtk_widget_hide(entry4);
		gtk_widget_hide(button);

		pDlg->UpdatePreview();
	}
}

void on_icon_custom_clicked(GtkMenuItem *menuitem, gpointer user_data)
{
	NodePropertiesDlg *pDlg = (NodePropertiesDlg *)user_data;

	GtkWidget *radio_ico_custom = lookup_widget(pDlg->m_pDialog, "radio_ico_custom");
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_ico_custom)))
	{
		GtkWidget *cbo2		= lookup_widget(pDlg->m_pDialog, "comboboxentry2");
		GtkWidget *entry4	= lookup_widget(pDlg->m_pDialog, "entry4");
		GtkWidget *button	= lookup_widget(pDlg->m_pDialog, "button_xpm_select");

		gtk_widget_set_sensitive(cbo2, FALSE);
		gtk_widget_set_sensitive(entry4, TRUE);
		gtk_widget_set_sensitive(button, TRUE);

		gtk_widget_hide(cbo2);
		gtk_widget_show(entry4);
		gtk_widget_show(button);

		pDlg->UpdatePreview();
	}
}

void on_xpm_select_clicked(GtkMenuItem *menuitem, gpointer user_data)
{
	FileDialog dlg;
	dlg.AddFilter(_("All supported icon formats (*.xpm,*.ico,*.png,*.jpg,*.gif)"), "*.xpm|*.ico|*.png|*.jpg|*.gif");
	dlg.AddFilter(_("XPM icon file (*.xpm)"), "*.xpm");
	dlg.AddFilter(_("Icon file (*.ico)"), "*.ico");
	dlg.AddFilter(_("PNG file format (*.png)"), "*.png");
	dlg.AddFilter(_("JPEG file format (*.jpg)"), "*.jpg");
	dlg.AddFilter(_("GIF file format (*.gif)"), "*.gif");


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

		NodePropertiesDlg *pDlg = (NodePropertiesDlg *)user_data;
		GtkWidget *entry4	= lookup_widget(pDlg->m_pDialog, "entry4");
		gtk_entry_set_text(GTK_ENTRY(entry4), dlg.GetFilename());

		pDlg->UpdatePreview();
	}
}

void NodePropertiesDlg::UpdatePreview()
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
	NodePropertiesDlg *pDlg = (NodePropertiesDlg *)user_data;
	pDlg->UpdatePreview();
}

void on_keyword_add_selected (GtkMenuItem *menuitem, gpointer user_data)
{
	NodePropertiesDlg *pDlg = (NodePropertiesDlg *)user_data;
	pDlg->KeywordAdd();
}

void on_keyword_remove_selected (GtkMenuItem *menuitem, gpointer user_data)
{
	NodePropertiesDlg *pDlg = (NodePropertiesDlg *)user_data;
	pDlg->KeywordRemove();
}

void on_finished_checked (GtkMenuItem *menuitem, gpointer user_data)
{
	NodePropertiesDlg *pDlg = (NodePropertiesDlg *)user_data;
	GtkWidget *chk_finished = lookup_widget(pDlg->m_pDialog, "chk_finished");
	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(chk_finished)))
		pDlg->m_bFinished = true;
	else
		pDlg->m_bFinished = false;
}

void FormatTime(time_t nTime, std::string& strTxt)
{
	struct tm *pTM = localtime(&nTime);

	char szTime[60];
	snprintf(szTime, sizeof(szTime), "%d.%02d.%02d %02d:%02d:%02d", pTM->tm_year+1900, pTM->tm_mon+1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec);

	strTxt = szTime;
}

const char *combo_get_text(GtkWidget *cbo3)
{
	//FIX: gtk_combo_box_get_active_text API crashes on Zaurus
#ifdef _ZAURUS_BUILD
	return gtk_entry_get_text(GTK_ENTRY(GTK_BIN(cbo3)->child));
#else
 #if GTK_CHECK_VERSION(2,6,0) //new API //TOFIX set proper version//
	return gtk_combo_box_get_active_text (GTK_COMBO_BOX(cbo3));
 #else
	int nPos = gtk_combo_box_get_active(GTK_COMBO_BOX(cbo3));
	if(nPos >= 0)
	{
		GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(cbo3));
		GtkTreeIter iter;
		char szPath[10];
		sprintf(szPath, "%d", nPos);
		GtkTreePath *path1 = gtk_tree_path_new_from_string (szPath);
		if(gtk_tree_model_get_iter(model, &iter, path1))
		{
			gchar *value = NULL;
			gtk_tree_model_get (model, &iter, 0, &value, -1);
			return value;
			//g_free(value);
		}
		else
			return "";
	}
	else //TOFIX get text from entry
		return "";
 #endif
#endif
}


gint entry_keyboard_handler(GtkWidget *widget, GdkEventKey *event)
{
	if(event->keyval == GDK_Return)
	{
		if(event->state & GDK_CONTROL_MASK)
		{
			return FALSE;
		}
		else
		{
			if(g_bCloseDlgOnTextEnter){
				gtk_dialog_response(GTK_DIALOG(g_pNodePropWidget), GTK_RESPONSE_OK);
				return TRUE;
			}
		}
	}
    else if(event->keyval == GDK_Tab)
	{
		if(event->state & GDK_CONTROL_MASK)
		{
			//insert tab character
			GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(g_editor));
			gtk_text_buffer_insert_at_cursor(buf, "\t", -1);
			return TRUE;
		}
		else
		{
            gtk_widget_grab_focus (g_cancelbtn); // pass focus forward
			return TRUE;
		}
	}

	return FALSE;
}

#ifdef HAVE_GTKSOURCEVIEW
std::string NodePropertiesDlg::GetSourceLanguage()
{
		GtkWidget *entry = lookup_widget(m_pDialog, "source_lang_combo");
    std::string strData = combo_get_text(entry);
    m_strSourceLanguage = strData;
		return strData;
}
#endif
