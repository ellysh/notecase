////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: GTK+ TreeView wrapper class
////////////////////////////////////////////////////////////////////////////

#ifndef TREEVIEW_H__
#define TREEVIEW_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if _MSC_VER > 1000
  #pragma warning(disable: 4786)
#endif

#include <gtk/gtk.h>
#include <vector>

#define STORE_IDX_TEXT		0
#define STORE_IDX_ICON		1
#define STORE_IDX_STRIKE	2
#define STORE_IDX_UNDERLINE 3
#define STORE_IDX_COLOR		4

#define SCREEN_IDX_TEXT	1
#define SCREEN_IDX_ICON	0

class TreeView
{
public:
	TreeView();
	virtual ~TreeView();

	void Create();
	void Clear();		// remove tree contents
	void EditLabel();	// enters edit mode for focused node

	GtkTreePath *GetFirstVisibleNode();
	bool GetNextVisibleNode(GtkTreePath **path1);
	bool GetPreviousVisibleNode(GtkTreePath **path1);
	bool IsTreePathValid(GtkTreePath *path);
	void DumpPath(GtkTreePath *path, const char *szFile);
	void EnableDragAndDrop(bool bEnable = true);

	//selection handlers
	void SelectionPageDown();
	void SelectionPageUp();
	void SelectionEnd();
	void SelectionHome();
	void SelectionLevelDown();
	void SelectionLevelUp();
	void SelectionUp();
	void SelectionDown();

	void SetFocus();

	void WrapTreeToSize(int nSize);
	void SetTreeLines(bool bTree);
	bool IsPathInVisbleRange(GtkTreePath *path);

public:
	GtkWidget *m_pWidget;

	// Drag and drop
	bool m_bDND;               //is DND in progress
	int  m_nFirstDroppedIdx;
	int  m_nFirstDraggedIdx;   //used for UNDO
	bool m_bCellEdited;		   //is in progress
};

#endif // TREEVIEW_H__
