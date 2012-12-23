////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements list of keyboard shortcuts for program actions
////////////////////////////////////////////////////////////////////////////

#ifndef SHORTCUTSLIST_H__
#define SHORTCUTSLIST_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
#include <string>
#include <gtk/gtk.h>
#include "lib/debug.h"

//define for each action (that can have a shortcut)
enum NC_Actions {
	NC_ACTION_DOC_NEW = 1,
	NC_ACTION_DOC_OPEN,
	NC_ACTION_DOC_SAVE,
	NC_ACTION_DOC_SAVE_AS,
	NC_ACTION_DOC_RELOAD,
	NC_ACTION_DOC_IMPORT,
	NC_ACTION_DOC_EXPORT,
	NC_ACTION_APP_NEW,
	NC_ACTION_APP_QUIT,
	NC_ACTION_TREE_NODE_NEW,
	NC_ACTION_TREE_NODE_NEW_CHILD,
	NC_ACTION_TREE_NODE_DELETE,
	NC_ACTION_TREE_NODE_RENAME,
	NC_ACTION_TREE_NODE_MOVE_UP,
	NC_ACTION_TREE_NODE_MOVE_DOWN,
	NC_ACTION_TREE_NODE_MOVE_LEFT,
	NC_ACTION_TREE_NODE_MOVE_RIGHT,
	NC_ACTION_TREE_NODE_PROPERTIES,
	NC_ACTION_TREE_NODE_DELETE_FINISHED,
	NC_ACTION_TREE_NODE_TOGGLE_FINISHED,
	NC_ACTION_TREE_SORT_CHILDREN_ASC,
	NC_ACTION_TREE_SORT_CHILDREN_DESC,
	NC_ACTION_TREE_SORT_ROOT_ASC,
	NC_ACTION_TREE_SORT_ROOT_DESC,
	NC_ACTION_TREE_EXPAND_ALL,
	NC_ACTION_TREE_COLAPSE_ALL,
	NC_ACTION_EDIT_UNDO,
	NC_ACTION_EDIT_REDO,
	NC_ACTION_EDIT_CUT,
	NC_ACTION_EDIT_COPY,
	NC_ACTION_EDIT_PASTE,
	NC_ACTION_EDIT_DELETE,
	NC_ACTION_EDIT_COPY_BR_STRUCT,	//copy branch structure
	NC_ACTION_EDIT_PASTE_BR_ROOT,	//paste branch to root
	NC_ACTION_EDIT_INSERT_DATETIME,
	NC_ACTION_EDIT_FIND,
	NC_ACTION_EDIT_FIND_NEXT,
	NC_ACTION_EDIT_REPLACE,
	NC_ACTION_EDIT_LINK_WIZARD,
	NC_ACTION_EDIT_INSERT_PICTURE,
	NC_ACTION_EDIT_ATTACHMENTS,
	NC_ACTION_EDIT_SHORTCUTS,
	NC_ACTION_EDIT_OPTIONS,
	NC_ACTION_FORMAT_BOLD,
	NC_ACTION_FORMAT_ITALIC,
	NC_ACTION_FORMAT_UNDERLINE,
	NC_ACTION_FORMAT_STRIKETHROUGH,
	NC_ACTION_FORMAT_COLOR,
	NC_ACTION_FORMAT_REMOVE,
	NC_ACTION_VIEW_TOOLBAR,
	NC_ACTION_VIEW_STATUS_BAR,
	NC_ACTION_VIEW_NODE_TITLEBAR,
	NC_ACTION_VIEW_WRAP,
	NC_ACTION_VIEW_CHANGE_MODE,
	NC_ACTION_VIEW_WORD_COUNT,
	NC_ACTION_HELP_INDEX,
	NC_ACTION_HELP_ABOUT,
	NC_ACTION_DOC_CLOSE,
	NC_ACTION_PASSWORD_CHANGE,
	DUMMY_ITEM2,					// keeps IDs sychronized with Pro version
	NC_ACTION_FORMAT_BKG_COLOR,
	NC_ACTION_READ_ONLY,
	NC_ACTION_VIEW_FULLSCREEN,
	DUMMY_ITEM3,					// keeps IDs sychronized with Pro version
	DUMMY_ITEM4,					// keeps IDs sychronized with Pro version
	NC_ACTION_LOCK_DOCUMENT,
	DUMMY_ITEM5,					// keeps IDs sychronized with Pro version
	DUMMY_ITEM6,					// keeps IDs sychronized with Pro version
	DUMMY_ITEM7,					// keeps IDs sychronized with Pro version
	DUMMY_ITEM8,					// keeps IDs sychronized with Pro version
	DUMMY_ITEM9,					// keeps IDs sychronized with Pro version
	DUMMY_ITEM10,					// keeps IDs sychronized with Pro version
	DUMMY_ITEM11,					// keeps IDs sychronized with Pro version
	DUMMY_ITEM12,					// keeps IDs sychronized with Pro version
	DUMMY_ITEM13,					// keeps IDs sychronized with Pro version
	DUMMY_ITEM14,					// keeps IDs sychronized with Pro version
	DUMMY_ITEM15,					// keeps IDs sychronized with Pro version
	DUMMY_ITEM16,					// keeps IDs sychronized with Pro version
	DUMMY_ITEM17,					// keeps IDs sychronized with Pro version
	DUMMY_ITEM18,					// keeps IDs sychronized with Pro version
	DUMMY_ITEM19,					// keeps IDs sychronized with Pro version
	NC_ACTION_VISIT_WEBSITE
};

//shortcut context
#define SH_CTX_GLOBAL		0
#define SH_CTX_TREE_WIDGET	1

class ShortcutDef
{
public:
	ShortcutDef(){};
	ShortcutDef(const ShortcutDef &other){ operator = (other); };
	ShortcutDef(int nCtx, int nID, const char *szName, int nKey, bool bShift, bool bCtrl, bool bAlt){
		m_strActionName = szName;
		m_nCtx			= nCtx;			//global, tree, text
		m_nActionID		= nID;
		m_nKey			= nKey;
		m_bModShift		= bShift;
		m_bModCtrl		= bCtrl;
		m_bModAlt		= bAlt;

		//ASSERT(IsValid(m_nCtx));
	}

	void operator =(const ShortcutDef &other){
		m_strActionName = other.m_strActionName;
		m_nCtx			= other.m_nCtx;
		m_nActionID		= other.m_nActionID;
		m_nKey			= other.m_nKey;
		m_bModShift		= other.m_bModShift;
		m_bModCtrl		= other.m_bModCtrl;
		m_bModAlt		= other.m_bModAlt;

		//ASSERT(IsValid(m_nCtx));
	}

	bool IsValid(int nContext = SH_CTX_GLOBAL);
	bool IsEmpty(){ return (0 == m_nKey); };

	std::string GetDisplayString();
	int GetModifierFlags();

	bool Match(GdkEventKey *evnt);

	//comparison
	bool operator ==(const ShortcutDef &other);

public:
	std::string m_strActionName;
	int	 m_nCtx;			//global, tree, text
	int  m_nActionID;
	int  m_nKey;
	bool m_bModShift :1;
	bool m_bModCtrl  :1;
	bool m_bModAlt   :1;
};

class ShortcutsList : public std::vector<ShortcutDef>
{
public:
	ShortcutsList();
	virtual ~ShortcutsList();

	bool Load();
	bool Save();

	int	FindByKey(ShortcutDef &key, int nSkipLine=-1, int nActionID=-1);
	int	FindByAction(int nActionID, int nCtx = SH_CTX_GLOBAL);
	int	FindByContext(int nCtx, int nIdx);

	void UpdateAction(ShortcutDef &key);
	void ClearAction(int nActionID, int nCtx);
};

#endif // SHORTCUTSLIST_H__
