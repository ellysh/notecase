////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements Node Sort Action
////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
  #pragma warning(disable: 4996)	//ignore deprecated functions warning
#endif

#include "DocActionSort.h"
#include "callbacks.h"
#include "support.h"
#include "interface.h"
#include "lib/NoteDocument.h"
#include "lib/DocumentIterator.h"
#include "TextView.h"
#include "TreeView.h"
#include "lib/debug.h"
#include <algorithm>

#ifdef _WIN32
 #include <io.h>
 #ifndef __MINGW32__
  #define strcasecmp stricmp
 #endif
#else
 #include <strings.h>
#endif

extern NoteDocument g_doc;
extern GtkWidget *window1;
extern TextView g_text;
extern TreeView g_tree;

class SortComparator{
public:
	bool operator()(const int &a, const int &b)
	{
		//operator < (is a<b ?)
		bool bRes = (strcasecmp(m_pDoc->GetNodeByIdx(a).GetTitle().c_str(), m_pDoc->GetNodeByIdx(b).GetTitle().c_str()) < 0);
		if(!m_bAscending)
			bRes = !bRes;
		return bRes;
	};

	NoteDocument *m_pDoc;
	bool m_bAscending;
};

DocActionSort::DocActionSort()
{
	m_nNodeIdx		= -1;
	m_bAscending	= true;
	//m_bRecursive	= false;
}

DocActionSort::~DocActionSort()
{
}

void DocActionSort::Exec(bool bInteractive)
{
	DocumentIterator it(*m_pDoc);

	UpdateTextFromScreen();

	int nParentID = -1;
	if(m_nNodeIdx >= 0)
		nParentID = m_pDoc->GetNodeByIdx(m_nNodeIdx).m_nID;

	int nChildCnt = it.GetChildCount(nParentID);

	//sort a list of indices
	int i;
	if(m_lstOldIdxOrder.size() < 1)
	{
		for(i=0; i<nChildCnt; i++){
			//TRACE("Before %d: %d\n", i, it.GetChildIdx(nParentID, i));
			m_lstOldIdxOrder.push_back(it.GetChildIdx(nParentID, i));
		}

		m_lstNewIdxOrder = m_lstOldIdxOrder;

		SortComparator cmp;
		cmp.m_pDoc = m_pDoc;
		cmp.m_bAscending = m_bAscending;
		std::sort(m_lstNewIdxOrder.begin(), m_lstNewIdxOrder.end(), cmp);
		//for(i=0; i<nChildCnt; i++)
		//	TRACE("After %d: %d\n", i, lstIdxOrder[i]);
	}

	GtkWidget *treeview = lookup_widget(window1, "treeview1");
	GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);

	// now use that to sort the document model
	for(i=0; i<nChildCnt; i++)
	{
		int nOldIdx = it.GetChildIdx(nParentID, i);
		int nNewIdx = m_lstNewIdxOrder[i];
		if(nOldIdx != nNewIdx)
		{
			NoteNode &node1 = m_pDoc->GetNodeByIdx(nOldIdx);
			NoteNode &node2 = m_pDoc->GetNodeByIdx(nNewIdx);

			int nTmp = node1.m_nSiblingIdx;
			node1.m_nSiblingIdx = node2.m_nSiblingIdx;
			node2.m_nSiblingIdx = nTmp;

			//swap two nodes in store
			GtkTreeIter iter, iter2;
			if( IteratorFromNodeIdx(nOldIdx, iter) &&
				IteratorFromNodeIdx(nNewIdx, iter2))
			{
				gtk_tree_store_swap(GTK_TREE_STORE(model), &iter, &iter2); //BUG crashes!
			}
		}
	}
}

void DocActionSort::Undo()
{
	DocumentIterator it(*m_pDoc);

	int nParentID = -1;
	if(m_nNodeIdx >= 0)
		nParentID = m_pDoc->GetNodeByIdx(m_nNodeIdx).m_nID;

	int nChildCnt = it.GetChildCount(nParentID);

	GtkWidget *treeview = lookup_widget(window1, "treeview1");
	GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *)treeview);

	// now use that to sort the document model
	for(int i=0; i<nChildCnt; i++)
	{
		int nOldIdx = it.GetChildIdx(nParentID, i);
		int nNewIdx = m_lstOldIdxOrder[i];
		if(nOldIdx != nNewIdx)
		{
			NoteNode &node1 = m_pDoc->GetNodeByIdx(nOldIdx);
			NoteNode &node2 = m_pDoc->GetNodeByIdx(nNewIdx);

			int nTmp = node1.m_nSiblingIdx;
			node1.m_nSiblingIdx = node2.m_nSiblingIdx;
			node2.m_nSiblingIdx = nTmp;

			//swap two nodes in store
			GtkTreeIter iter, iter2;
			if( IteratorFromNodeIdx(nOldIdx, iter) &&
				IteratorFromNodeIdx(nNewIdx, iter2))
			{
				gtk_tree_store_swap(GTK_TREE_STORE(model), &iter, &iter2); //BUG crashes!
			}
		}
	}
}
