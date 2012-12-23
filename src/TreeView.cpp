////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: GTK+ TreeView wrapper class
////////////////////////////////////////////////////////////////////////////

#include "TreeView.h"
#include <string.h>	//strlen
#include <stdio.h>	//FILE
#include "config.h"
#include "lib/LinkInfo.h"
#include "lib/NoteDocument.h"

void edit_node_title(int nSelFrom = -1, int nSelLen = -1, const char *szTitle = NULL);
void UpdateTextFromScreen();
void UpdateUndoRedoMenus();
void ExecuteLink(LinkInfo &link);
int GetSelectedNodeIdx();

extern NoteDocument g_doc;

//declare signal handlers
gboolean treeview1_popup_menu_event_handler (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
void cell_edited_callback(GtkCellRendererText *cell, gchar *path_string, gchar *new_text, gpointer user_data);
void cell_edited_start_callback (GtkCellRenderer *renderer, GtkCellEditable *editable, gchar *path, gpointer user_data);
void cell_edited_cancel_callback (GtkCellRenderer *renderer, gpointer user_data);

void on_tree_row_click (GtkTreeView *treeview, gpointer user_data);
void on_tree_row_collapsed (GtkTreeView *tree_view, GtkTreeIter *iter, GtkTreePath *path, gpointer user_data);
void on_tree_row_expanded (GtkTreeView *tree_view, GtkTreeIter *iter, GtkTreePath *path, gpointer user_data);
gint treeview_keyboard_handler(GtkWidget *widget, GdkEventKey *event);
gboolean on_tree_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
gint on_popup_menu(GtkWidget *widget, gpointer data);
void treeview_show_popup_menu();

void on_dnd_row_deleted  (GtkTreeModel *treemodel, GtkTreePath *arg1, gpointer user_data);
void on_dnd_row_inserted (GtkTreeModel *treemodel, GtkTreePath *arg1, GtkTreeIter *arg2, gpointer user_data);
void on_dnd_begin (GtkWidget *view, GdkDragContext *context, gpointer data);
void on_dnd_end (GtkWidget *view, GdkDragContext *context, gpointer data);

TreeView::TreeView()
{
	m_pWidget = NULL;
	m_bDND = false;
	m_nFirstDraggedIdx = -1;
	m_nFirstDroppedIdx = -1;
	m_bCellEdited = false;
}

TreeView::~TreeView()
{
}

void TreeView::Create()
{
	GtkWidget *treeview1;
	GtkTreeStore* store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	//
	// tree initialization
	//

	// data store: title, icon, strikethrough, underline, node color
	store = gtk_tree_store_new(5, G_TYPE_STRING, GDK_TYPE_PIXBUF, G_TYPE_BOOLEAN, G_TYPE_UINT, GDK_TYPE_COLOR);
	treeview1 = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
	g_object_unref (G_OBJECT (store));  //tree now holds reference

	gtk_tree_view_set_headers_visible((GtkTreeView *)treeview1, FALSE);
	gtk_tree_view_set_enable_search((GtkTreeView *)treeview1, FALSE);
	//g_object_set (treeview1, "level_indentation", 10, (char *)NULL);
	gtk_widget_show (treeview1);

	g_signal_connect (treeview1, "button_release_event", G_CALLBACK (treeview1_popup_menu_event_handler), NULL);
	g_signal_connect (treeview1, "cursor_changed",	     G_CALLBACK (on_tree_row_click), NULL);
	g_signal_connect (treeview1, "key_press_event",      G_CALLBACK (treeview_keyboard_handler), NULL);
	g_signal_connect (treeview1, "button-release-event", G_CALLBACK (on_tree_button_release), this);
	g_signal_connect (treeview1, "button-press-event",   G_CALLBACK (on_tree_button_release), this);
	g_signal_connect (treeview1, "row-collapsed",	     G_CALLBACK (on_tree_row_collapsed), this);
	g_signal_connect (treeview1, "row-expanded",	     G_CALLBACK (on_tree_row_expanded), this);
	g_signal_connect (treeview1, "popup-menu",			 G_CALLBACK (on_popup_menu), this);

#ifdef _NOKIA_MAEMO
	gtk_widget_tap_and_hold_setup(GTK_WIDGET(treeview1), NULL, NULL, (GtkWidgetTapAndHoldFlags)(GTK_TAP_AND_HOLD_NONE | GTK_TAP_AND_HOLD_NO_INTERNALS));
	g_signal_connect(treeview1, "tap-and-hold", G_CALLBACK(on_popup_menu), NULL);
#endif

	//pack two renderers in the single column
	column = gtk_tree_view_column_new();

	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "pixbuf", STORE_IDX_ICON, (char *)NULL);

	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "underline-set",  TRUE, (char *)NULL);
	g_object_set(renderer, "foreground-set", TRUE, (char *)NULL);

	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_attributes(column, renderer,
			"text",           STORE_IDX_TEXT,
			"strikethrough",  STORE_IDX_STRIKE,
			"underline",      STORE_IDX_UNDERLINE,
			"foreground-gdk", STORE_IDX_COLOR,
			(char *)NULL);

	g_signal_connect(renderer, "edited", (GCallback) cell_edited_callback, this);
	g_signal_connect(renderer, "editing-started", (GCallback) cell_edited_start_callback, this);
	g_signal_connect(renderer, "editing-canceled", (GCallback) cell_edited_cancel_callback, this);

	// add the column to the view
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview1), column);

	m_pWidget = treeview1;

	EnableDragAndDrop();

	//signals needed for DND reordering
	g_signal_connect(store, "row_deleted",	  G_CALLBACK(on_dnd_row_deleted),   this);
	g_signal_connect(store, "row_inserted",   G_CALLBACK(on_dnd_row_inserted),  this);
	g_signal_connect(treeview1, "drag_begin", G_CALLBACK(on_dnd_begin), this);
	g_signal_connect(treeview1, "drag_end",   G_CALLBACK(on_dnd_end),   this);
}

void TreeView::Clear()
{
	GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)m_pWidget);
	gtk_tree_store_clear(GTK_TREE_STORE(model));
}

void TreeView::EditLabel()
{
	GtkTreeView *treeview = (GtkTreeView *)m_pWidget;
	GtkTreeModel *model = gtk_tree_view_get_model(treeview);

	//get iterator to selected node
	GtkTreeSelection* treesel = gtk_tree_view_get_selection (treeview);
	GtkTreeIter iter;

	if(gtk_tree_selection_get_selected(treesel, &model, &iter))
	{
		//TOFIX use GtkCellRenderer::gtk_cell_renderer_start_editing for simpler code?
		//trigger cell editing (edit close is handled elsewhere)
		GtkTreeViewColumn *column1 = gtk_tree_view_get_column(treeview, 0);
		GList *list1 = gtk_tree_view_column_get_cell_renderers(column1);
		GtkCellRendererText *renderer = (GtkCellRendererText *)g_list_nth_data(list1, SCREEN_IDX_TEXT);
		GtkTreePath* path1 = gtk_tree_model_get_path(model, &iter);
		g_object_set(renderer, "editable", TRUE, (char *)NULL);	//makes tree cells editable
		gtk_tree_view_set_cursor(treeview, path1, column1, TRUE);	//trigger
		g_object_set(renderer, "editable", FALSE, (char *)NULL);//makes tree cells non-editable
		g_list_free(list1);
		gtk_tree_path_free(path1);
	}
}

void TreeView::DumpPath(GtkTreePath *path, const char *szFile)
{
#ifdef _DEBUG
	FILE *pOutFile = fopen(szFile, "a");
	if(pOutFile)
	{
		gchar *szPath = gtk_tree_path_to_string(path);
		fprintf(pOutFile, "Path: %s\n", szPath);
		g_free(szPath);
		fclose(pOutFile);
	}
#endif
}

bool TreeView::IsTreePathValid(GtkTreePath *path)
{
	bool bSuccess = true;
	gchar *szPath = gtk_tree_path_to_string(path);
	if(NULL == szPath || strlen(szPath) < 1)
		bSuccess = false;
	g_free(szPath);

	if(bSuccess)	//another check?
	{
		GtkTreeView *treeview = (GtkTreeView *)m_pWidget;
		GtkTreeModel *model = gtk_tree_view_get_model(treeview);
		GtkTreeIter iter;
		if(!gtk_tree_model_get_iter(model, &iter, path))
			return false;
	}

	return bSuccess;
}

GtkTreePath *TreeView::GetFirstVisibleNode()
{
	GtkTreeView *treeview = (GtkTreeView *)m_pWidget;
	GtkTreeViewColumn *column1 = gtk_tree_view_get_column(treeview, 0);

	GtkTreePath *path1 = NULL;
	if(gtk_tree_view_get_path_at_pos(treeview, 3, 3, &path1, &column1, NULL, NULL))
	{
	#ifdef _DEBUG
		//DumpPath(path1, "D:\\dump.txt");
	#endif

		//if(gtk_tree_model_get_iter(model,&iter, *path1)) //just check if path valid
		//	return true;
		return path1;
	}

	return NULL;
}

bool TreeView::GetNextVisibleNode(GtkTreePath **path1)
{
	GtkTreeView *treeview = (GtkTreeView *)m_pWidget;
	GtkTreeModel *model = gtk_tree_view_get_model(treeview);

	GtkTreeIter iter;
	if(!gtk_tree_model_get_iter(model, &iter, *path1))
		return false;

	if( gtk_tree_model_iter_has_child(model, &iter) &&
		gtk_tree_view_row_expanded(treeview, *path1))
	{
		//select first child
		gtk_tree_path_down(*path1);
	#ifdef _DEBUG
		//DumpPath(*path1, "D:\\dump.txt");
	#endif
		return true;
	}
	else
	{
		//if next sibling exists select it
		if(gtk_tree_model_iter_next(model, &iter))
		{
			gtk_tree_path_free(*path1);
			*path1 = gtk_tree_model_get_path(model, &iter);
		#ifdef _DEBUG
			//DumpPath(*path1, "D:\\dump.txt");
		#endif
			return true;
		}
		else
		{
			//there is no sibling available, so move up
			GtkTreeIter iterParent;
			int nDepth = gtk_tree_path_get_depth (*path1);	//get current depth - for convinience
			gboolean foundNextNode = FALSE;
			//
			// 1. if depth is sufficient - find parent of current node
			// 2. parent is now current node
			// 3. find next sibling of current node
			// 4. if no such sibling exists go to 1.
			//
			do {
				if( --nDepth < 1 ) return false;
				//get parent node
				gtk_tree_model_iter_parent( model, &iterParent, &iter);
				iter = iterParent;
				if( gtk_tree_model_iter_next( model, &iterParent ) ) foundNextNode = TRUE;
			} while ( !foundNextNode );

			if( foundNextNode ) {
				gtk_tree_path_free(*path1);
				*path1 = gtk_tree_model_get_path(model, &iterParent);
				return true;
			}
		}
	}

	return false;
}

bool TreeView::GetPreviousVisibleNode(GtkTreePath **path1)
{
	GtkTreeView *treeview = (GtkTreeView *)m_pWidget;
	GtkTreeModel *model = gtk_tree_view_get_model(treeview);

	//find previous sibling
	if(gtk_tree_path_prev(*path1))
	{
		//if expanded recursively follow its last child descendants
		GtkTreeIter iter;
		gtk_tree_model_get_iter(model, &iter, *path1);
		while(1)
		{
			//if item has expanded children
			if( gtk_tree_model_iter_has_child(model, &iter) &&
				gtk_tree_view_row_expanded(treeview, *path1))
			{
				//find last child
				int nCount = gtk_tree_model_iter_n_children(model, &iter);
				if(nCount > 0)
				{
					GtkTreeIter *parent = gtk_tree_iter_copy(&iter);
					gtk_tree_model_iter_nth_child(model, &iter, parent, nCount-1);
					gtk_tree_iter_free(parent);

					//refresh path
					gtk_tree_path_free(*path1);
					*path1 = gtk_tree_model_get_path(model, &iter);
				}
				else
					break;
			}
			else
				break;
		}

		//found last child of a last child, ...
		return true;
	}
	else
	{
		if( gtk_tree_path_get_depth (*path1) > 1 &&	//protection for going into path_up
			gtk_tree_path_up(*path1))
			return true;	//parent node found
	}

	return false;	//no previous node found
}

void TreeView::SelectionPageDown()
{
	//moves selection one page up (no nodes are being expanded/collapsed)
	GtkTreeView *treeview = (GtkTreeView *)m_pWidget;

	//get first selected or first visible node
	GtkTreePath *path1  = NULL;

	GtkTreeModel *model = gtk_tree_view_get_model(treeview);
	GtkTreeSelection* treesel = gtk_tree_view_get_selection (treeview);
	GtkTreeIter iter;
	if(gtk_tree_selection_get_selected(treesel, &model, &iter))
		path1 = gtk_tree_model_get_path(model, &iter);
	else
		path1 = GetFirstVisibleNode();

	if(path1 != NULL)
	{
		//calculate page size in items
		GdkRectangle rectWin, rectCell;
		gtk_tree_view_get_visible_rect(treeview, &rectWin);

		GtkTreeViewColumn *column1 = gtk_tree_view_get_column(treeview, 0);
		gtk_tree_view_get_background_area(treeview, path1, column1,&rectCell);
		int nItemsPerPage = rectWin.height/rectCell.height;

		//move cursor x items up (or less if there are no so much nodes visible)
		for(int i=0; i<nItemsPerPage; i++)
		{
			if(!GetNextVisibleNode(&path1))
				break;
		}

		//select found node
		gtk_tree_view_set_cursor(treeview, path1, column1, FALSE);	//refresh node selection
		gtk_tree_path_free(path1);
	}
}

void TreeView::SelectionPageUp()
{
	//moves one page up (no nodes are being expanded/collapsed)
	GtkTreeView *treeview = (GtkTreeView *)m_pWidget;

	//get first selected or first visible node
	GtkTreePath *path1  = NULL;

	GtkTreeModel *model = gtk_tree_view_get_model(treeview);
	GtkTreeSelection* treesel = gtk_tree_view_get_selection (treeview);
	GtkTreeIter iter;
	if(gtk_tree_selection_get_selected(treesel, &model, &iter))
		path1 = gtk_tree_model_get_path(model, &iter);
	else
		path1 = GetFirstVisibleNode();

	if(path1 != NULL)
	{
		//calculate page size in items
		GdkRectangle rectWin, rectCell;
		gtk_tree_view_get_visible_rect(treeview, &rectWin);

		GtkTreeViewColumn *column1 = gtk_tree_view_get_column(treeview, 0);
		gtk_tree_view_get_background_area(treeview, path1, column1,&rectCell);
		int nItemsPerPage = rectWin.height/rectCell.height;

		//move cursor x items up (or less if there are no so much nodes visible)
		for(int i=0; i<nItemsPerPage; i++)
		{
			if(!GetPreviousVisibleNode(&path1))
				break;
		}

		//select found node
		gtk_tree_view_set_cursor(treeview, path1, column1, FALSE);	//refresh node selection
		gtk_tree_path_free(path1);
	}
}

void TreeView::SelectionEnd()
{
	//select last item (only expanded counts)
	GtkTreeView *treeview = (GtkTreeView *)m_pWidget;
	GtkTreeModel *model = gtk_tree_view_get_model(treeview);

	//get iterator to first root node
	GtkTreeIter iter;
	if(gtk_tree_model_iter_children(model, &iter, NULL))
	{
		GtkTreePath* path1 = gtk_tree_model_get_path(model, &iter);

		//position on the last sibling on this level (if not already)
		GtkTreeIter parent;
		gboolean bParent = gtk_tree_model_iter_parent(model, &parent, &iter);

		//position to last parent's child
		int nChildren = gtk_tree_model_iter_n_children (model, (bParent)? &parent : NULL);
		if(nChildren < 1)
			return;
		if(!gtk_tree_model_iter_nth_child(model, &iter, (bParent)? &parent : NULL, nChildren-1))
			return;

		while(1)
		{
			//refresh path
			gtk_tree_path_free(path1);
			path1 = gtk_tree_model_get_path(model, &iter);

			//if expanded children exist
			if(  gtk_tree_model_iter_has_child(model, &iter) &&
				 gtk_tree_view_row_expanded(treeview, path1))
			{
				//position to last child
				int nChildren = gtk_tree_model_iter_n_children (model, &iter);
				if(nChildren < 1)
					break;
				GtkTreeIter iterChild;
				if(!gtk_tree_model_iter_nth_child(model, &iterChild, &iter, nChildren-1))
					break;
				iter = iterChild;
			}
			else
				break;
		}

		//select item
		GtkTreeViewColumn *column1 = gtk_tree_view_get_column(treeview, 0);
		gtk_tree_view_set_cursor(treeview, path1, column1, FALSE);	//refresh node selection
		gtk_tree_path_free(path1);
	}
}

void TreeView::SelectionHome()
{
	//select first item
	GtkTreeView *treeview = (GtkTreeView *)m_pWidget;
	GtkTreeModel *model = gtk_tree_view_get_model(treeview);

	GtkTreeIter iter;
	if(gtk_tree_model_iter_children(model, &iter, NULL))
	{
		GtkTreePath* path1 = gtk_tree_model_get_path(model, &iter);
		GtkTreeViewColumn *column1 = gtk_tree_view_get_column(treeview, 0);
		gtk_tree_view_set_cursor(treeview, path1, column1, FALSE);	//refresh node selection
		gtk_tree_path_free(path1);
	}
}

void TreeView::SelectionLevelDown()
{
	//expand selected node OR select the first child (if already expanded)
	GtkTreeView *treeview = (GtkTreeView *)m_pWidget;
	GtkTreeModel *model = gtk_tree_view_get_model(treeview);

	//get iterator to selected node
	GtkTreeSelection* treesel = gtk_tree_view_get_selection (treeview);
	GtkTreeIter iter;

	if(gtk_tree_selection_get_selected(treesel, &model, &iter))
	{
		GtkTreePath* path1 = gtk_tree_model_get_path(model, &iter);
		if(!gtk_tree_view_row_expanded(treeview, path1))
		{
			gtk_tree_view_expand_row(treeview, path1, FALSE);
		}
		else
		{
			//select first child
			gtk_tree_path_down(path1);
			GtkTreeViewColumn *column1 = gtk_tree_view_get_column(treeview, 0);
			gtk_tree_view_set_cursor(treeview, path1, column1, FALSE);	//refresh node selection
		}

		gtk_tree_path_free(path1);
	}
}

void TreeView::SelectionLevelUp()
{
	//collapse selected node OR select parent node
	GtkTreeView *treeview = (GtkTreeView *)m_pWidget;
	GtkTreeModel *model = gtk_tree_view_get_model(treeview);

	//get iterator to selected node
	GtkTreeSelection* treesel = gtk_tree_view_get_selection (treeview);
	GtkTreeIter iter;

	if(gtk_tree_selection_get_selected(treesel, &model, &iter))
	{
		GtkTreePath* path1 = gtk_tree_model_get_path(model, &iter);
		if(!gtk_tree_view_collapse_row(treeview, path1) && (gtk_tree_path_get_depth(path1) > 1))
		{
			//nothing to collapse, select the parent node
			if(gtk_tree_path_up(path1)){
				GtkTreeViewColumn *column1 = gtk_tree_view_get_column(treeview, 0);
				gtk_tree_view_set_cursor(treeview, path1, column1, FALSE);	//refresh node selection
			}
		}

		gtk_tree_path_free(path1);
	}
}

void TreeView::SelectionUp()
{
	//select previous item above current
	GtkTreeView *treeview = (GtkTreeView *)m_pWidget;
	GtkTreeModel *model = gtk_tree_view_get_model(treeview);

	//get iterator to selected node
	GtkTreeSelection* treesel = gtk_tree_view_get_selection (treeview);
	GtkTreeIter iter;

	if(gtk_tree_selection_get_selected(treesel, &model, &iter))
	{
		GtkTreePath* path1 = gtk_tree_model_get_path(model, &iter);
		if(GetPreviousVisibleNode(&path1))
		{
			GtkTreeViewColumn *column1 = gtk_tree_view_get_column(treeview, 0);
			gtk_tree_view_set_cursor(treeview, path1, column1, FALSE);	//refresh node selection
		}
		gtk_tree_path_free(path1);
	}
}

void TreeView::SelectionDown()
{
	//select next item below current
	GtkTreeView *treeview = (GtkTreeView *)m_pWidget;
	GtkTreeModel *model = gtk_tree_view_get_model(treeview);

	//get iterator to selected node
	GtkTreeSelection* treesel = gtk_tree_view_get_selection (treeview);
	GtkTreeIter iter;

	if(gtk_tree_selection_get_selected(treesel, &model, &iter))
	{
		GtkTreePath* path1 = gtk_tree_model_get_path(model, &iter);
		if(GetNextVisibleNode(&path1))
		{
			GtkTreeViewColumn *column1 = gtk_tree_view_get_column(treeview, 0);
			gtk_tree_view_set_cursor(treeview, path1, column1, FALSE);	//refresh node selection
		}
		gtk_tree_path_free(path1);
	}
}

void TreeView::SetFocus()
{
 	gtk_widget_grab_focus (m_pWidget);
}

void TreeView::EnableDragAndDrop(bool bEnable)
{
	//enable "drag and drop"
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW (m_pWidget), bEnable);
}

void on_dnd_begin (GtkWidget *view, GdkDragContext *context, gpointer data)
{
	TreeView *pTree = (TreeView *)data;
	pTree->m_bDND = true;
	pTree->m_nFirstDroppedIdx = -1;
	pTree->m_nFirstDraggedIdx = -1;
	UpdateTextFromScreen();
}

void on_dnd_end (GtkWidget *view, GdkDragContext *context, gpointer data)
{
	TreeView *pTree = (TreeView *)data;
	pTree->m_bDND = false;
	pTree->m_nFirstDroppedIdx = -1;
	pTree->m_nFirstDraggedIdx = -1;
	UpdateUndoRedoMenus();
}

gboolean on_tree_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	if(event->button == 1 && event->type == GDK_2BUTTON_PRESS)
	{
		int nIdx = GetSelectedNodeIdx();
		if(nIdx >= 0){ //execute link if exists
			NoteNode &node = g_doc.GetNodeByIdx(nIdx);
			if(node.m_objLink.IsValid()) //execute link if exists
				ExecuteLink(node.m_objLink);
			else
			{
				//double click expands/collapses normal branch
				//get iterator to selected node
				GtkTreeSelection* treesel = gtk_tree_view_get_selection (GTK_TREE_VIEW(widget));
				GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
				GtkTreeIter iter;

				if(gtk_tree_selection_get_selected(treesel, &model, &iter))
				{
					GtkTreePath* path1 = gtk_tree_model_get_path(model, &iter);
					if(!gtk_tree_view_row_expanded(GTK_TREE_VIEW(widget), path1))
					{
						gtk_tree_view_expand_row(GTK_TREE_VIEW(widget), path1, FALSE);
					}
					else
					{
						gtk_tree_view_collapse_row(GTK_TREE_VIEW(widget), path1);
					}
					gtk_tree_path_free(path1);
				}
			}
		}
	}

	return FALSE;
}

//this handler enables keyboard-only access to the context menu
// -> use shortcut: Shift + F10
gint on_popup_menu(GtkWidget *widget, gpointer data)
{
	treeview_show_popup_menu();
	return TRUE;
}

void TreeView::WrapTreeToSize(int nSize)
{
	GtkTreeView *treeview = (GtkTreeView *)m_pWidget;
	GtkTreeViewColumn *column1 = gtk_tree_view_get_column(treeview, 0);
	GList *list1 = gtk_tree_view_column_get_cell_renderers(column1);
	GtkCellRendererText *renderer = (GtkCellRendererText *)g_list_nth_data(list1, SCREEN_IDX_TEXT);

	g_object_set (G_OBJECT (renderer),
		"wrap-mode", PANGO_WRAP_CHAR,
		"wrap-width", nSize,
		(char *)NULL);
}

void TreeView::SetTreeLines(bool bTree)
{
#if GTK_CHECK_VERSION(2,10,0)
	g_object_set (G_OBJECT (m_pWidget),
		"enable-tree-lines", bTree,
		(char *)NULL);
#endif
}

bool TreeView::IsPathInVisbleRange(GtkTreePath *path)
{
	bool bVisible = false;
	GtkTreePath *start_path = NULL;
	GtkTreePath *end_path = NULL;
#if 0//GTK_CHECK_VERSION(2,8,0)
	//TOFIX: this causes runtime GTK critical warnings in Linux console!
	if(gtk_tree_view_get_visible_range(GTK_TREE_VIEW(m_pWidget), &start_path, &end_path)){
#else
	GdkRectangle rect;
	gtk_tree_view_get_visible_rect(GTK_TREE_VIEW(m_pWidget), &rect);
	gint bx = rect.x + rect.width - 2;
	gint by = rect.y + rect.height - 2;
	//gtk_tree_view_convert_tree_to_bin_window_coords(GTK_TREE_VIEW(m_pWidget), rect.x + rect.width - 2, rect.y + rect.height - 2, &bx, &by);
	if(gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(m_pWidget), 0, 0, &start_path, NULL, NULL, NULL) &&
	   gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(m_pWidget), bx, by, &end_path, NULL, NULL, NULL))
	{
#endif
		//is the path within the visible range
		if( gtk_tree_path_compare(path, start_path) >= 0 &&
			gtk_tree_path_compare(path, end_path) <= 0)
		{
			bVisible = true;
		}
		gtk_tree_path_free(start_path);
		gtk_tree_path_free(end_path);
	}
	return bVisible;
}
