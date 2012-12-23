////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements Action to delete all "finished" nodes
////////////////////////////////////////////////////////////////////////////

#include "DocActionFinishDel.h"
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
extern GdkColor g_linkColor;

int GetSelectedNodeIdx();
void UpdateNodeIcon(int nIdx);
std::string calc_node_info(int nIdx);

DocActionFinishDel::DocActionFinishDel()
{
	m_nSelIdx = -1;
}

DocActionFinishDel::~DocActionFinishDel()
{
}

void DocActionFinishDel::Exec(bool bInteractive)
{
	if(bInteractive)
	{
		//TOFIX implement some less memory-hungry solution
		//store copy of the entire document
		m_objOrigTree = *m_pDoc;
		m_nSelIdx = g_doc.m_nActiveNodeIdx;
	}

	m_nDeletedCnt = 0;

	GtkTreeView *treeview = (GtkTreeView *)g_tree.m_pWidget;
	GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);

	DocumentIterator it(g_doc);
	int nCnt = g_doc.GetNodeCount();
	//TRACE("Delete finished nodes: %d total nodes to check\n", nCnt);

	for(int i=0; i<nCnt; i++)
	{
		if(g_doc.GetNodeByIdx(i).m_bFinished)
		{
			TRACE("Deleting (finished) node: %s\n", g_doc.GetNodeByIdx(i).GetTitle().c_str());

			//remove GUI tree node
			GtkTreeIter iter;
			IteratorFromNodeIdx(i, iter);
			gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);

			//TOFIX DeleteNode function
			bool bRes = false;
			bRes = g_doc.NodeDelete(g_doc.GetNodeByIdx(i).m_nID);
			ASSERT(bRes);

			m_nDeletedCnt ++;

			//restart search from start
			nCnt = g_doc.GetNodeCount();
			i = -1;

			//TRACE("Delete finished nodes: %d total nodes to check\n", nCnt);
		}
	}

	//refresh selection
	g_doc.m_nActiveNodeIdx = GetSelectedNodeIdx();

	//clear text if current node deleted
	if(g_doc.m_nActiveNodeIdx < 0){
		g_text.Clear(); //clear edit view
		g_text.SetEditable(false);	// no selected node
		set_title_bar("", false);
	}

	//update node info (status bar)
	std::string strInfo;
	if(g_doc.m_nActiveNodeIdx >= 0)
		strInfo = calc_node_info(g_doc.m_nActiveNodeIdx);
	set_status_bar(strInfo.c_str(), false);
}

void DocActionFinishDel::Undo()
{
	//use a copy of an original tree data to add back the "finished" nodes
	*m_pDoc = m_objOrigTree;

	GtkTreeView *treeview = (GtkTreeView *)g_tree.m_pWidget;
	GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);

	//TOFIX the most simple rebuild, but more time consuming
	gtk_tree_store_clear(GTK_TREE_STORE(model));
	add_child_nodes(NULL);

	//ensure that the node is visible and selected
	if(m_nSelIdx >= 0)
		SelectNodeByIdx(m_nSelIdx);
}
