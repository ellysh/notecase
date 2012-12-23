////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implementations of menu action handlers
////////////////////////////////////////////////////////////////////////////

#ifndef _CALLBACKS_H__
#define _CALLBACKS_H__

#include <gtk/gtk.h>
#include "lib/NoteNode.h"

void on_new_notecase1_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_new1_activate        (GtkMenuItem *menuitem, gpointer user_data);
void on_open1_activate       (GtkMenuItem *menuitem, gpointer user_data);
void on_close1_activate       (GtkMenuItem *menuitem, gpointer user_data);
void on_save1_activate       (GtkMenuItem *menuitem, gpointer user_data);
void on_save_as1_activate    (GtkMenuItem *menuitem, gpointer user_data);
void on_reload1_activate     (GtkMenuItem *menuitem, gpointer user_data);
void on_lock_document_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_change_pass1_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_read_only_activate	 (GtkMenuItem *menuitem, gpointer user_data);
void on_quit1_activate       (GtkMenuItem *menuitem, gpointer user_data);
void on_cut1_activate        (GtkMenuItem *menuitem, gpointer user_data);
void on_copy1_activate       (GtkMenuItem *menuitem, gpointer user_data);
void on_paste1_activate      (GtkMenuItem *menuitem, gpointer user_data);
void on_delete1_activate     (GtkMenuItem *menuitem, gpointer user_data);
void on_about1_activate      (GtkMenuItem *menuitem, gpointer user_data);
void on_help1_activate       (GtkMenuItem *menuitem, gpointer user_data);
void on_menu_visit_website	 (GtkMenuItem *menuitem, gpointer user_data);
void on_tree_row_click       (GtkTreeView *treeview, gpointer user_data);
void on_menu_insert_node     (GtkMenuItem *menuitem, gpointer user_data);
void on_menu_insert_child_node(GtkMenuItem *menuitem, gpointer user_data);
void on_menu_delete_node     (GtkMenuItem *menuitem, gpointer user_data);
void on_menu_rename_node     (GtkMenuItem *menuitem, gpointer user_data);
void on_menu_move_up	     (GtkMenuItem *menuitem, gpointer user_data);
void on_menu_move_down	     (GtkMenuItem *menuitem, gpointer user_data);
void on_menu_move_left	     (GtkMenuItem *menuitem, gpointer user_data);
void on_menu_move_right	     (GtkMenuItem *menuitem, gpointer user_data);
void on_menu_cut_branch		 (GtkMenuItem *menuitem, gpointer user_data);
void on_menu_copy_branch	 (GtkMenuItem *menuitem, gpointer user_data);
void on_menu_copy_branch_structure(GtkMenuItem *menuitem, gpointer user_data);
void on_menu_paste_branch	 (GtkMenuItem *menuitem, gpointer user_data);
void on_menu_paste_branch_root	 (GtkMenuItem *menuitem, gpointer user_data);
void on_menu_insert_date_time	 (GtkMenuItem *menuitem, gpointer user_data);
void on_menu_expand_all      (GtkMenuItem *menuitem, gpointer user_data);
void on_menu_collapse_all    (GtkMenuItem *menuitem, gpointer user_data);
void on_menu_node_properties (GtkMenuItem *menuitem, gpointer user_data);
void on_menu_import          (GtkMenuItem *menuitem, gpointer user_data);
void on_menu_export          (GtkMenuItem *menuitem, gpointer user_data);
void on_options1_activate    (GtkMenuItem *menuitem, gpointer user_data);
void on_find1_activate	     (GtkMenuItem *menuitem, gpointer user_data);
void on_find2_activate       (GtkMenuItem *menuitem, gpointer user_data);
void on_find_replace_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_show_node_titlebar_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_link_wizard_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_save_picture_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_insert_picture_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_node_attachments_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_view_shortcuts_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_text_bold_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_text_italic_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_text_underline_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_text_strikethrough_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_text_color_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_text_background_color_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_remove_format_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_view_text_only_mode (GtkMenuItem *menuitem, gpointer user_data);
void on_view_calc_word_count (GtkMenuItem *menuitem, gpointer user_data);
void on_menu_sort_child_ascending (GtkMenuItem *menuitem, gpointer user_data);
void on_menu_sort_child_descending (GtkMenuItem *menuitem, gpointer user_data);
void on_menu_sort_root_ascending (GtkMenuItem *menuitem, gpointer user_data);
void on_menu_sort_root_descending (GtkMenuItem *menuitem, gpointer user_data);
void on_menu_delete_finished_nodes (GtkMenuItem *menuitem, gpointer user_data);

void set_show_node_title(bool bShow);
bool get_node_title_set();

void set_wrap_activated(bool bActivate);
void refresh_wrap_menu(bool bActivate);
void on_wrap_activate (GtkMenuItem *menuitem, gpointer user_data);

void set_show_toolbar(bool bShow);
bool get_show_toolbar();
void on_show_toolbar_activate (GtkMenuItem *menuitem, gpointer user_data);

void set_show_status_bar(bool bShow);
bool get_show_status_bar();
void on_show_status_bar_activate (GtkMenuItem *menuitem, gpointer user_data);

void on_undo_activate(GtkMenuItem *menuitem, gpointer  user_data);
void on_redo_activate (GtkMenuItem *menuitem, gpointer  user_data);
void on_textview_populate_popup(GtkWidget *widget, GtkMenu *menu, void *user_data);

void UpdateUndoRedoMenus();
void SetNodeTitle(int nIdx, const char *szTitle);
void InsertNodeText(int nIdx, int nOffset, const char *szText, std::vector<PixInfo> *pLstPictures = NULL);
void DeleteNodeText(int nIdx, int nOffset, int nLength);

void on_delete_text (GtkTextBuffer *textbuffer,
                     GtkTextIter *arg1,
                     GtkTextIter *arg2,
                     gpointer user_data);
void on_insert_text (GtkTextBuffer *textbuffer,
                     GtkTextIter *arg1,
                     gchar *arg2,
                     gint arg3,
                     gpointer user_data);

void RefreshTextFormat(NoteNode &node, bool bAddPictures = true, int nFrom = -1, int nTo = -1);
void UpdateTextFromScreen();
void RefreshMainTitle();
void TreeIterFromID(GtkTreeIter  &iter, int nNodeID);
void add_child_nodes(GtkTreeIter *iter, int nParentID = -1, int nFirstSibling = 0);
void SelectNodeByIdx(int nIdx, gfloat fRowAlign = -1.0, bool bForceAlign = false);

//TOFIX return success?
void do_node_move_up (bool bStoreUndo = true);
void do_node_move_down(bool bStoreUndo = true);
void do_node_move_left(bool bStoreUndo = true);
void do_node_move_right(bool bStoreUndo = true);

gboolean on_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer data);

void restart_autosave();
bool autosave_check_crash();
void autosave_shutdown();
void remove_autosave_unencrypted();

void restart_enc_unload_timer();

void do_sort_branch(int nParentIdx, bool bAscending);

void set_read_only(bool bSet, bool bModify = false);
bool get_read_only();

void on_full_screen_activate (GtkMenuItem *menuitem, gpointer user_data);
void edit_node_title(int nSelFrom = -1, int nSelLen = -1, const char *szTitle = NULL);
gboolean date_time_paste_to_text_widget(GtkWidget *widget);

void on_node_toggle_finished(GtkMenuItem *menuitem, gpointer user_data);

gboolean textview_expose_event_cb (GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean textview1_expose_event_cb (GtkWidget *widget, GdkEventExpose *event, gpointer data);

void on_ontop1_activate (GtkMenuItem *menuitem, gpointer user_data);

#ifdef HAVE_GTKSOURCEVIEW
void on_show_line_numbers_activate (GtkMenuItem *menuitem, gpointer user_data);
#endif

#endif
