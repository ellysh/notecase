////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Defines atomic action performed on NoteCase document
//		 (base for Undo/Redo framework)
////////////////////////////////////////////////////////////////////////////

#include "DocAction.h"
#include "callbacks.h"
#include "support.h"
#include "interface.h"
#include "lib/NoteDocument.h"
#include "lib/DocumentIterator.h"
#include "TextView.h"
#include "TreeView.h"
#include "lib/debug.h"

extern NoteDocument g_doc;
extern GtkWidget *window1;
extern TextView g_text;
extern TreeView g_tree;

void rebuild_gui_tree (int nPID = -1);
bool IteratorFromNodeIdx(int nIdx, GtkTreeIter &iter);
void RefreshAllLinkTags();
int GetSelectedNodeIdx();

DocAction::DocAction()
{
	m_nType = ACT_UNDEFINED;
	m_bSelected = false;
	m_nNodeSibling = 0;
	m_nOffset = -1;
	m_bNodeTitle = false;
	m_nLinkNodeID = -1;
	m_bUndo = false;
}

DocAction::~DocAction()
{
}

void DocAction::Exec(bool bInteractive)
{
	m_bUndo = false;
	m_bInteractive = bInteractive;

	switch(m_nType){
		case ACT_TEXT_INSERT:
			DoTextInsert();
			break;
		case ACT_TEXT_DELETE:
			DoTextDelete();
			break;
		case ACT_NODE_INSERT:
			DoTreeInsert();
			break;
		case ACT_NODE_RENAME:
			DoNodeRename();
			break;
		case ACT_TREE_DELETE:
			DoTreeDelete();
			break;
		case ACT_TREE_MOVE:
			DoTreeMove();
			break;
		case ACT_TREE_IMPORT:
			DoTreeImport();
			break;
		case ACT_TREE_DND:
			DoTreeDND(true);
			break;
		case ACT_TEXT_REPLACE:
			DoTextReplace();
			break;
		case ACT_TEXT_LINK_ADD:
			DoLinkAdd();
			break;
		case ACT_TEXT_LINK_REMOVE:
			DoLinkRemove();
			break;
	}
}

void DocAction::Undo()
{
	m_bUndo = true;
	m_bInteractive = false;

	switch(m_nType){
		case ACT_TEXT_INSERT:
			DoTextDelete();
			break;
		case ACT_TEXT_DELETE:
			DoTextInsert();
			break;
		case ACT_NODE_INSERT:
			DoTreeDelete();
			break;
		case ACT_NODE_RENAME:
			DoNodeUnrename();
			break;
		case ACT_TREE_DELETE:
			DoTreeInsert();
			break;
		case ACT_TREE_MOVE:
			DoTreeUnmove();
			break;
		case ACT_TREE_IMPORT:
			DoTreeUnimport();
			break;
		case ACT_TREE_DND:
			DoTreeDND(false);
			break;
		case ACT_TEXT_REPLACE:
			DoTextUnreplace();
			break;
		case ACT_TEXT_LINK_ADD:
			DoLinkRemove();
			break;
		case ACT_TEXT_LINK_REMOVE:
			DoLinkAdd();
			break;
	}
}

void DocAction::DoTextInsert()
{
	if(m_bUndo){
		m_pDoc->GetNodeByIdx(m_nNodeIndex).m_lstTxtFmt = m_lstFmt;	//restore formatting
		m_pDoc->GetNodeByIdx(m_nNodeIndex).m_lstLinks = m_lstLinks;
		m_pDoc->GetNodeByIdx(m_nNodeIndex).m_lstPictures = m_lstPictures;
	}
	else
	{
		TRACE("DocActionTxt::DoTextInsert: Picture count =%d\n", m_lstPictures.size());
	
		if(m_bInteractive){
			m_lstFmt = m_pDoc->GetNodeByIdx(m_nNodeIndex).m_lstTxtFmt;	//store formatting
			m_lstLinks = m_pDoc->GetNodeByIdx(m_nNodeIndex).m_lstLinks;
			m_lstPictures = m_pDoc->GetNodeByIdx(m_nNodeIndex).m_lstPictures;
		}
		else
		{
			m_pDoc->GetNodeByIdx(m_nNodeIndex).m_lstTxtFmt = m_lstFmt;	//restore formatting
			m_pDoc->GetNodeByIdx(m_nNodeIndex).m_lstLinks = m_lstLinks;
			m_pDoc->GetNodeByIdx(m_nNodeIndex).m_lstPictures = m_lstPictures;
		}
	}

	InsertNodeText(m_nNodeIndex, m_nTextStartPos, m_strNodeText.c_str(), NULL);

	if(m_bSelected){
		g_text.SelectRange(m_nTextStartPos, m_nTextStartPos+m_strNodeText.size());

		GtkWidget *textview = lookup_widget(window1, "textview1");
		gtk_window_set_focus(GTK_WINDOW(window1), textview);
	}
}

void DocAction::DoTextDelete()
{
	int nPicCount = 0;
	if(m_bUndo){
		nPicCount = m_pDoc->GetNodeByIdx(m_nNodeIndex).m_lstPictures.size();

		m_lstFmt = m_pDoc->GetNodeByIdx(m_nNodeIndex).m_lstTxtFmt;	//store formatting
		m_lstLinks = m_pDoc->GetNodeByIdx(m_nNodeIndex).m_lstLinks;
		m_lstPictures = m_pDoc->GetNodeByIdx(m_nNodeIndex).m_lstPictures;
	}
	else{
		nPicCount = m_lstPictures.size();

		m_lstFmt = m_pDoc->GetNodeByIdx(m_nNodeIndex).m_lstTxtFmt;	//store formatting
		m_lstLinks = m_pDoc->GetNodeByIdx(m_nNodeIndex).m_lstLinks;
		m_lstPictures = m_pDoc->GetNodeByIdx(m_nNodeIndex).m_lstPictures;
	}

	//this skips the usual text view handlers, no new undo action generated
	int nCharsDeleted = g_utf8_strlen(m_strNodeText.c_str(), -1);
	int nTotalDeleted = nCharsDeleted + nPicCount;
	DeleteNodeText(m_nNodeIndex, m_nTextStartPos, nTotalDeleted);

	//refresh node internal state
	m_pDoc->GetNodeByIdx(m_nNodeIndex).OnTxtDelete(m_nTextStartPos, nCharsDeleted, nTotalDeleted, NULL);
}

void DocAction::DoTextReplace()
{
	TRACE("UNDO/REDO: replace text\n");

	SelectNodeByIdx(m_nNodeIndex);//select proper node

	//TOFIX this code is similar to the one in replace dialog - use this only?
	int nLen = g_utf8_strlen(m_strFind.c_str(), -1);

	NoteNode &node = g_doc.GetNodeByIdx(m_nNodeIndex);

	// some text selected, replace it with the given string
	std::string strTxt;
	if(m_bNodeTitle)
		strTxt = node.GetTitle();
	else
		strTxt = node.GetText();

	//convert from character offset to buffer offset
	int nCursorBuf, nSelBuf;
	const char *szBuf = strTxt.c_str();
	gchar* pszPos = g_utf8_offset_to_pointer(szBuf, m_nOffset);
	nCursorBuf = pszPos - szBuf;
	nSelBuf = m_nOffset + nLen;

	//now replace the text part with new content
	//TOFIX what if cursor < nSelBuf - convert both values using min
	strTxt = strTxt.erase(nCursorBuf, nSelBuf-nCursorBuf);
	strTxt.insert(nCursorBuf, m_strReplace.c_str());

	if(m_bNodeTitle)
	{
		node.SetTitle(strTxt.c_str());

		//update GUI text
		GtkTreeIter iter;
		IteratorFromNodeIdx(m_nNodeIndex, iter);
		GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)g_tree.m_pWidget);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 0, strTxt.c_str(), -1);

		set_title_bar(strTxt.c_str());
	}
	else
	{
		node.SetText(strTxt.c_str());

		//update GUI text
		GtkTextView *textview =  (GtkTextView *)g_text.m_pWidget;
		GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);

		g_signal_handlers_block_by_func(buffer1, (void *)on_delete_text, 0);
		g_signal_handlers_block_by_func(buffer1, (void *)on_insert_text, 0);

		g_text.ReplaceText(m_nOffset, m_nOffset+nLen, m_strReplace.c_str());
		g_text.SelectRange(m_nOffset, m_nOffset+g_utf8_strlen(m_strReplace.c_str(), -1));

		g_signal_handlers_unblock_by_func(buffer1, (void *)on_delete_text, 0);
		g_signal_handlers_unblock_by_func(buffer1, (void *)on_insert_text, 0);

		node.OnTxtReplaced(m_nOffset, m_nOffset+nLen, m_strReplace.c_str());
		RefreshAllLinkTags();
	}
}

void DocAction::DoTextUnreplace()
{
	TRACE("UNDO/REDO: undo replace text\n");

	SelectNodeByIdx(m_nNodeIndex);//select proper node

	//TOFIX this code is similar to the one in replace dialog - use this only?
	int nLen = g_utf8_strlen(m_strReplace.c_str(), -1);

	NoteNode &node = g_doc.GetNodeByIdx(m_nNodeIndex);

	// some text selected, replace it with the given string
	std::string strTxt;
	if(m_bNodeTitle)
		strTxt = node.GetTitle();
	else
		strTxt = node.GetText();

	//convert from character offset to buffer offset
	int nCursorBuf, nSelBuf;
	const char *szBuf = strTxt.c_str();
	gchar* pszPos = g_utf8_offset_to_pointer(szBuf, m_nOffset);
	nCursorBuf = pszPos - szBuf;
	nSelBuf = m_nOffset + nLen;

	//now replace the text part with new content
	//TOFIX what if cursor < nSelBuf - convert both values using min
	strTxt = strTxt.erase(nCursorBuf, nSelBuf-nCursorBuf);
	strTxt.insert(nCursorBuf, m_strFind.c_str());

	if(m_bNodeTitle)
	{
		node.SetTitle(strTxt.c_str());

		//update GUI text
		GtkTreeIter iter;
		IteratorFromNodeIdx(m_nNodeIndex, iter);
		GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)g_tree.m_pWidget);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 0, strTxt.c_str(), -1);

		set_title_bar(strTxt.c_str());
	}
	else
	{
		node.SetText(strTxt.c_str());

		//update GUI text
		GtkTextView *textview =  (GtkTextView *)g_text.m_pWidget;
		GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);

		g_signal_handlers_block_by_func(buffer1, (void *)on_delete_text, 0);
		g_signal_handlers_block_by_func(buffer1, (void *)on_insert_text, 0);

		g_text.ReplaceText(m_nOffset, m_nOffset+nLen, m_strFind.c_str());
		g_text.SelectRange(m_nOffset, m_nOffset + g_utf8_strlen(m_strFind.c_str(), -1));

		g_signal_handlers_unblock_by_func(buffer1, (void *)on_delete_text, 0);
		g_signal_handlers_unblock_by_func(buffer1, (void *)on_insert_text, 0);

		node.OnTxtReplaced(m_nOffset, m_nOffset+nLen, m_strFind.c_str());
		RefreshAllLinkTags();
	}
}

void DocAction::DoTreeInsert()
{
	TRACE("UNDO/REDO: insert %d nodes under PID=%d\n", m_objSubTree.GetNodeCount(), m_nNodePID);

	int nCount = g_doc.GetNodeCount();

	TRACE("subtree dump:\n");
	m_objSubTree.Dump();
	TRACE("UNDO/REDO insert: %d nodes before\n", g_doc.GetNodeCount());
	TRACE("UNDO/REDO insert: PID=%d, Sibling=%d\n", m_nNodePID, m_nNodeSibling);
	g_doc.Merge(m_objSubTree, m_nNodePID, m_nNodeSibling, true);
	TRACE("UNDO/REDO insert: %d nodes after\n", g_doc.GetNodeCount());

	//rebuild affected branch in the gui tree
	rebuild_gui_tree (m_nNodePID);

	g_doc.m_nActiveNodeIdx = -1;	//no selection

	//select the node in the GUI tree
	SelectNodeByIdx(nCount);  //select subtree 'root' node in new tree
}

void DocAction::DoTreeDelete()
{
	TRACE("UNDO/REDO: delete node ID=%d\n", m_nNodeID);

	TRACE("subtree dump:\n");
	m_objSubTree.Dump();

	GtkTreeIter  iter;
	TreeIterFromID(iter, m_nNodeID);

	g_doc.NodeDelete(m_nNodeID);

	g_doc.m_nActiveNodeIdx = -1;	//no selection

	//refresh GUI
	GtkTreeView *treeview = (GtkTreeView *)lookup_widget(window1, "treeview1");
	GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);

	//remove GUI tree node
	gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);

	//clear edit belonging to selected node
	g_text.Clear();

	set_title_bar("");	//update node title label
	RefreshMainTitle();
}

void DocAction::DoNodeRename()
{
	SetNodeTitle(m_nNodeIndex, m_strNodeNameNew.c_str());	//TOFIX recursive index ?
}

void DocAction::DoNodeUnrename()
{
	SetNodeTitle(m_nNodeIndex, m_strNodeNameOld.c_str());	//TOFIX recursive index ?
}

void DocAction::DoTreeMove()
{
	SelectNodeByIdx(m_nNodeIndex);//select proper node

	switch(m_nMoveDirection){
		case MOVE_LEFT:
			do_node_move_left(false);
			break;
		case MOVE_RIGHT:
			do_node_move_right(false);
			break;
		case MOVE_UP:
			do_node_move_up(false);
			break;
		case MOVE_DOWN:
			do_node_move_down(false);
			break;
	}
}

//reverse the moving operation
void DocAction::DoTreeUnmove()
{
	int i;

	SelectNodeByIdx(m_nNodeIndex);//select proper node

	switch(m_nMoveDirection){
		case MOVE_LEFT:
			do_node_move_right(false);
			//restore sibling index position when moving back from left to right
			for(i=0; i<m_nNodeSibling; i++)
				do_node_move_down(false);
			break;
		case MOVE_RIGHT:
			do_node_move_left(false);
			break;
		case MOVE_UP:
			do_node_move_down(false);
			break;
		case MOVE_DOWN:
			do_node_move_up(false);
			break;
	}
}

void DocAction::DoTreeImport()
{
	DocumentIterator it(g_doc);
	int nCnt = it.GetChildCount(-1);

	//TOFIX ask user for import details and merge with current document
	g_doc.Merge(m_objSubTree);

	//refresh tree starting from new content
	add_child_nodes(NULL, -1, nCnt);
}

void DocAction::DoTreeUnimport()
{
	DocumentIterator it(g_doc);
	int nCount = g_doc.GetNodeCount();
	int i;

	g_doc.m_nActiveNodeIdx = -1;	//no selection

	GtkTreeView *treeview = (GtkTreeView *)lookup_widget(window1, "treeview1");
	GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);

	//delete all nodes after idx = m_nNodeIndex
	//delete in backward direction
	for(i=nCount-1; i>=m_nNodeIndex; i--)
	{
		int nID = g_doc.GetNodeByIdx(i).m_nID;

		GtkTreeIter  iter;
		TreeIterFromID(iter, nID);

		//delete in memory
		g_doc.NodeDelete(nID);

		//remove GUI tree node
		gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);

		//clear edit belonging to selected node
		g_text.Clear();  // TOFIX only if current node is one of being deleted

		//update node title label
		set_title_bar("");
		RefreshMainTitle();
	}
}

void DocAction::DoTreeDND(bool bForward)
{
	GtkTreeView *treeview = (GtkTreeView *)lookup_widget(window1, "treeview1");
	GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);

	if(bForward)
	{
		//redo - move node branch from original position to new one
		int nID = m_nNodeID;

		GtkTreeIter iter;
		TreeIterFromID(iter, nID);

		TRACE("REDO dnd: tree dump on start\n");
		#ifdef _DEBUG
			g_doc.Dump();
		#endif

		TRACE("REDO dnd: delete node ID=%d\n", nID);

		//delete in memory
		g_doc.NodeDelete(nID);

		TRACE("REDO dnd: tree dump after delete\n");
		#ifdef _DEBUG
			g_doc.Dump();
		#endif

		//remove GUI tree node
		gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);

		//clear edit belonging to selected node
		g_text.Clear();

		TRACE("REDO dnd: merge subtree under PID=%d, SIB=%d\n", m_nNodeNewPID, m_nNodeNewSibling);

		//insert node branch into new position
		g_doc.Merge(m_objSubTree, m_nNodeNewPID, m_nNodeNewSibling, true);  //keep IDs

		TRACE("REDO dnd: tree dump after merge\n");
		#ifdef _DEBUG
			g_doc.Dump();
		#endif

		//rebuild GUI
		rebuild_gui_tree (m_nNodeNewPID);
	}
	else
	{
		//undo - move new node branch back to original position
		int nID = m_nNewNodeID;

		GtkTreeIter iter;
		TreeIterFromID(iter, nID);

		TRACE("UNDO dnd: tree dump on start\n");
		#ifdef _DEBUG
			g_doc.Dump();
		#endif

		TRACE("UNDO dnd: delete node ID=%d\n", nID);

		//delete in memory
		g_doc.NodeDelete(nID);

		TRACE("UNDO dnd: tree dump after delete\n");
		#ifdef _DEBUG
			g_doc.Dump();
		#endif

		//remove GUI tree node
		gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);

		//clear edit belonging to selected node
		g_text.Clear();

		TRACE("UNDO dnd: merge subtree under PID=%d, SIdx=%d\n", m_nNodePID, m_nNodeSibling);

		#ifdef _DEBUG
			//m_objSubTree.Dump();
		#endif
		//insert node branch into original position
		g_doc.Merge(m_objSubTree, m_nNodePID, m_nNodeSibling, true);	//keep IDs

		TRACE("UNDO dnd: tree dump after merge\n");
		#ifdef _DEBUG
			g_doc.Dump();
		#endif

		//rebuild GUI
		rebuild_gui_tree (m_nNodePID);
	}
}

void rebuild_gui_tree (int nPID)
{
	GtkTreeView *treeview = (GtkTreeView *)lookup_widget(window1, "treeview1");
	GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);

	if(nPID == -1)
	{
		//clear all nodes
		gtk_tree_store_clear(GTK_TREE_STORE(model));

		//rebuild parent node children
		add_child_nodes(NULL, nPID, 0);
	}
	else
	{
		GtkTreeIter iter;
		TreeIterFromID(iter, nPID);

		//remove all parent node children
		GtkTreeIter iterChild;
		while(gtk_tree_model_iter_children(model, &iterChild, &iter))
			gtk_tree_store_remove(GTK_TREE_STORE(model), &iterChild); //delete all children

		//rebuild parent node children
		add_child_nodes(&iter, nPID, 0);
	}
}

void DocAction::DoLinkAdd()
{
	LinkInfo info;
	info.m_nStartOffset = m_nLinkOffset;
	info.m_strText = m_strLinkTitle;
	info.m_strTargetURL = m_strLinkUrl;
	info.m_nTargetNodeID = m_nLinkNodeID;
	info.RefreshLength();

	// add link into the node
	g_doc.GetNodeByIdx(m_nNodeIndex).m_lstLinks.push_back(info);

	//fix gui display
	int nIdx = GetSelectedNodeIdx();
	if(nIdx == m_nNodeIndex)
	{
		//underline the text as a link
		g_text.RemoveTextStyles(info.m_nStartOffset, info.m_nStartOffset+info.m_nTextLength);
		g_text.SetTextUnderlined(info.m_nStartOffset, info.m_nStartOffset+info.m_nTextLength);
	}
}

void DocAction::DoLinkRemove()
{
	NoteNode &node = g_doc.GetNodeByIdx(m_nNodeIndex);

	//find link info
	int nPos = node.m_lstLinks.Find(m_nLinkOffset);
	if(nPos >= 0)
	{
		LinkInfo info;
		info = node.m_lstLinks[nPos];

		//fix gui display
		int nIdx = GetSelectedNodeIdx();
		if(nIdx == m_nNodeIndex)
		{
			g_text.RemoveTextStyles(info.m_nStartOffset, info.m_nStartOffset+info.m_nTextLength);
		}

		//remove link from the storage
		node.m_lstLinks.erase(g_doc.GetNodeByIdx(nIdx).m_lstLinks.begin()+nPos);
	}
}
