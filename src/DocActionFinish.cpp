////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements Action to mark node "finished"
////////////////////////////////////////////////////////////////////////////

#include "DocActionFinish.h"
#include "callbacks.h"
#include "support.h"
#include "interface.h"
#include "lib/NoteDocument.h"
#include "lib/DocumentIterator.h"
#include "TextView.h"
#include "TreeView.h"
#include "gui/FileDialog.h"
#include "lib/debug.h"
#include <algorithm>

#ifdef _WIN32
 #include <io.h>
 #ifndef __MINGW32__
  #define strcasecmp stricmp
 #endif
#endif

extern NoteDocument g_doc;
extern GtkWidget *window1;
extern TextView g_text;
extern TreeView g_tree;

void set_finished_recursive(GtkTreeModel *model, GtkTreeIter *parent, bool bFinished);
void restore_finished_recursive(GtkTreeModel *model, GtkTreeIter *parent, DocActionFinish *action);

DocActionFinish::DocActionFinish()
{
	m_nNodeIndex		 = -1;
	m_bRecursiveFinished = false;
}

DocActionFinish::~DocActionFinish()
{
}

void DocActionFinish::Exec(bool bInteractive)
{
	if(bInteractive)
	{
		std::vector<NoteNode> lstSubtree;
		g_doc.GetSubtree(m_nNodeIndex, lstSubtree);
		m_objSubTree.AssignSubtree(lstSubtree);	//copy content

		g_doc.GetNodeByIdx(m_nNodeIndex).m_bFinished = ! g_doc.GetNodeByIdx(m_nNodeIndex).m_bFinished;

		GtkTreeView *treeview = (GtkTreeView *)g_tree.m_pWidget;
		GtkTreeModel *model = gtk_tree_view_get_model(treeview);
		GtkTreeIter iter;
		IteratorFromNodeIdx(m_nNodeIndex, iter);
		gtk_tree_store_set (GTK_TREE_STORE(model), &iter, STORE_IDX_STRIKE, g_doc.GetNodeByIdx(m_nNodeIndex).m_bFinished, -1);

		//recursively process the descendants
		if(!gtk_tree_model_iter_has_child(model, &iter))
			return;
		if(!g_doc.GetNodeByIdx(m_nNodeIndex).m_bFinished){
			if(GTK_RESPONSE_YES != gtkMessageBox(_("Do you want to remove 'Completed' status from all the descendants?"), GTK_BUTTONS_YES_NO))
				return;
		}
		set_finished_recursive(model, &iter, g_doc.GetNodeByIdx(m_nNodeIndex).m_bFinished);
		m_bRecursiveFinished = true;
	}
	else
		DoTreeFinished();
}

void DocActionFinish::Undo()
{
	DoTreeFinished();	//same operation
}

void DocActionFinish::DoTreeFinished()
{
	TRACE("UNDO/REDO: set nodes finished for branch Idx=%d\n", m_nNodeIndex);
	GtkTreeView *treeview = (GtkTreeView *)g_tree.m_pWidget;
	GtkTreeModel *model = gtk_tree_view_get_model(treeview);

	bool bWasFinished = g_doc.GetNodeByIdx(m_nNodeIndex).m_bFinished;

	//un-finish the node and optionally its children (based on m_bRecursiveFinished)
	GtkTreeIter iter;
	IteratorFromNodeIdx(m_nNodeIndex, iter);

	g_doc.GetNodeByIdx(m_nNodeIndex).m_bFinished = ! g_doc.GetNodeByIdx(m_nNodeIndex).m_bFinished;
	gtk_tree_store_set (GTK_TREE_STORE(model), &iter, STORE_IDX_STRIKE, g_doc.GetNodeByIdx(m_nNodeIndex).m_bFinished, -1);

	if(bWasFinished /*&& m_bRecursiveFinished*/){
		restore_finished_recursive(model, &iter, this); //refresh original states from the sub tree
	}
	else //if(!bWasFinished)
		set_finished_recursive(model, &iter, g_doc.GetNodeByIdx(m_nNodeIndex).m_bFinished);
}

void restore_finished_recursive(GtkTreeModel *model, GtkTreeIter *parent, DocActionFinish *action)
{
	//for each child
	int nCount = gtk_tree_model_iter_n_children(model, parent);

	for(int i=0; i<nCount; i++)
	{
		GtkTreeIter iter;
		if(!gtk_tree_model_iter_nth_child(model, &iter, parent, i))
			return;

		//TOFIX method to map from GUI tree iter to document node ID and back
		GtkTreePath *path1 = gtk_tree_model_get_path(model, &iter);
		int nIdx = NodeIdxFromPath(path1);
		gtk_tree_path_free(path1);

		if(nIdx >= 0)
		{
			int nIdx2 = action->m_objSubTree.GetIdxFromID(g_doc.GetNodeByIdx(nIdx).m_nID);
			ASSERT(nIdx2 >= 0);
			bool bFinished = action->m_objSubTree.GetNodeByIdx(nIdx2).m_bFinished;
			g_doc.GetNodeByIdx(nIdx).m_bFinished = bFinished;
			gtk_tree_store_set (GTK_TREE_STORE(model), &iter, STORE_IDX_STRIKE, bFinished, -1);
			restore_finished_recursive(model, &iter, action);	//recurse into children
		}
	}
}

void set_finished_recursive(GtkTreeModel *model, GtkTreeIter *parent, bool bFinished)
{
	//for each child
	int nCount = gtk_tree_model_iter_n_children(model, parent);

	for(int i=0; i<nCount; i++)
	{
		GtkTreeIter iter;
		if(!gtk_tree_model_iter_nth_child(model, &iter, parent, i))
			return;

		//TOFIX method to map from GUI tree iter to document node ID and back
		GtkTreePath *path1 = gtk_tree_model_get_path(model, &iter);
		int nIdx = NodeIdxFromPath(path1);
		gtk_tree_path_free(path1);

		if(nIdx >= 0)
		{
			g_doc.GetNodeByIdx(nIdx).m_bFinished = bFinished;
			gtk_tree_store_set (GTK_TREE_STORE(model), &iter, STORE_IDX_STRIKE, bFinished, -1);
			set_finished_recursive(model, &iter, bFinished);	//recurse into children
		}
	}
}
