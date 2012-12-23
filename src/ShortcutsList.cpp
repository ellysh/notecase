////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements list of keyboard shortcuts for program actions
////////////////////////////////////////////////////////////////////////////

#include "ShortcutsList.h"
#include "lib/IniFile.h"
#include "support.h"
#include <gdk/gdkkeysyms.h>
#include "lib/debug.h"

extern IniFile g_objIni;

bool g_bListInited = false;
ShortcutsList g_lstDefAction;

std::string ShortcutDef::GetDisplayString()
{
	std::string strMsg;

	//format modifier keys
	if(m_bModCtrl)
		strMsg += _("Ctrl+");
	if(m_bModShift)
		strMsg += _("Shift+");
	if(m_bModAlt)
		strMsg += _("Alt+");

	//format main key
	guint32 cLetter = gdk_keyval_to_unicode (m_nKey);
	if(cLetter > 0)
	{
		if(cLetter == ' ')
			strMsg += _("Space");
		else
			strMsg += g_ascii_toupper(cLetter);
	}
	else if(m_nKey != GDK_Control_L &&
			m_nKey != GDK_Control_R &&
			m_nKey != GDK_Shift_L	&&
			m_nKey != GDK_Shift_R	&&
			m_nKey != GDK_Alt_L		&&
			m_nKey != GDK_Alt_R)
	{
		if(gdk_keyval_name(m_nKey))
			strMsg += gdk_keyval_name(m_nKey);
	}

	return strMsg;
}

int ShortcutDef::GetModifierFlags()
{
	int nRes = 0;
	if(m_bModCtrl)
		nRes |= GDK_CONTROL_MASK;
	if(m_bModShift)
		nRes |= GDK_SHIFT_MASK;
	if(m_bModAlt)
		nRes |= GDK_MOD1_MASK;

	return nRes;
}

bool ShortcutDef::Match(GdkEventKey *evnt)
{
	const int nFlagsMask = GDK_CONTROL_MASK|GDK_SHIFT_MASK|GDK_MOD1_MASK;

	if(IsValid(m_nCtx)){
		if(evnt->keyval == (unsigned int)m_nKey){
			unsigned int nFlags = GetModifierFlags();
			if(nFlags == (evnt->state & nFlagsMask))
				return true;
		}
	}
	return false;
}

bool ShortcutDef::IsValid(int nContext)
{
	//empty shortcut - not valid
	if(!m_bModCtrl && !m_bModShift && !m_bModAlt && m_nKey == 0)
		return false;

	//key must be valid - non-modifier code
	if( m_nKey == 0 ||
		m_nKey == GDK_Control_L ||
		m_nKey == GDK_Control_R ||
		m_nKey == GDK_Shift_L	||
		m_nKey == GDK_Shift_R	||
		m_nKey == GDK_Alt_L		||
		m_nKey == GDK_Alt_R)
		return false;

	//at least one modifier required (global shortcuts only) unless shortcut is the function key
	if(SH_CTX_GLOBAL == nContext)
		if(!m_bModCtrl && !m_bModShift && !m_bModAlt && gdk_keyval_to_unicode(m_nKey)>0)
			return false;

	//Shift only modifier is forbidden (forbids typing uppercase letters) unless shortcut is the function key
	if(!m_bModCtrl && m_bModShift && !m_bModAlt && gdk_keyval_to_unicode(m_nKey)>0)
		return false;

	return true;
}

bool ShortcutDef::operator ==(const ShortcutDef &other)
{
	return (m_nKey		== other.m_nKey		 &&
			m_bModShift == other.m_bModShift &&
			m_bModCtrl	== other.m_bModCtrl	 &&
			m_bModAlt	== other.m_bModAlt);
}

ShortcutsList::ShortcutsList()
{
	if(!g_bListInited)
	{
		//g_bListInited = true;
		g_lstDefAction.clear();
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_DOC_NEW,  _("New Document"), 'N', false, true, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_DOC_OPEN, _("Open Document"), 'O', false, true, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_DOC_SAVE, _("Save Document"), 'S', false, true, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_DOC_SAVE_AS, _("Save As Document"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_DOC_RELOAD, _("Reload Document"), 'R', false, true, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_DOC_IMPORT, _("Import"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_DOC_EXPORT, _("Export"), 'E', false, true, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_APP_NEW, _("New Notecase"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_READ_ONLY, _("Read-Only (Document)"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_LOCK_DOCUMENT, _("Lock Document"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_PASSWORD_CHANGE, _("Change Password"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_APP_QUIT, _("Quit Notecase"), 'Q', false, true, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_TREE_NODE_NEW, _("New Node"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_TREE_NODE_NEW_CHILD, _("New Child Node"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_TREE_NODE_DELETE, _("Delete Node"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_TREE_NODE_RENAME, _("Rename Node"), 'M', false, true, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_TREE_NODE_MOVE_UP, _("Move Node Up"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_TREE_NODE_MOVE_DOWN, _("Move Node Down"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_TREE_NODE_MOVE_LEFT, _("Move Node Left"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_TREE_NODE_MOVE_RIGHT, _("Move Node Right"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_TREE_NODE_PROPERTIES, _("Show Node Properties"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_TREE_NODE_DELETE_FINISHED, _("Delete Completed Nodes"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_TREE_NODE_TOGGLE_FINISHED, _("Toggle Completed Status"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_TREE_SORT_CHILDREN_ASC, _("Sort Children Ascending"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_TREE_SORT_CHILDREN_DESC, _("Sort Children Descending"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_TREE_SORT_ROOT_ASC, _("Sort Tree Ascending"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_TREE_SORT_ROOT_DESC, _("Sort Tree Descending"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_TREE_EXPAND_ALL, _("Expand All"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_TREE_COLAPSE_ALL, _("Colapse All"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_EDIT_UNDO, _("Undo"), 'Z', false, true, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_EDIT_REDO, _("Redo"), 'Y', false, true, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_EDIT_CUT, _("Cut"), 'X', false, true, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_EDIT_COPY, _("Copy"), 'C', false, true, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_EDIT_PASTE, _("Paste"), 'V', false, true, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_EDIT_DELETE, _("Delete"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_EDIT_COPY_BR_STRUCT, _("Copy Branch Structure"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_EDIT_PASTE_BR_ROOT, _("Paste Branch Root"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_EDIT_INSERT_DATETIME, _("Insert date/time"), 'T', true, true, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_EDIT_FIND, _("Find"), 'F', false, true, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_EDIT_FIND_NEXT, _("Find Next"), GDK_F3, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_EDIT_REPLACE, _("Replace"), 'H', false, true, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_EDIT_LINK_WIZARD, _("Link Wizard"), 'L', false, true, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_EDIT_INSERT_PICTURE, _("Insert Picture"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_EDIT_ATTACHMENTS, _("Attachments"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_EDIT_SHORTCUTS, _("Configure Shortcuts"), 'K', false, false, true));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_EDIT_OPTIONS, _("Configure NoteCase"), GDK_F7, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_FORMAT_BOLD, _("Bold"), 'B', false, true, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_FORMAT_ITALIC, _("Italic"), 'I', false, true, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_FORMAT_UNDERLINE, _("Underline"), 'U', false, true, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_FORMAT_STRIKETHROUGH, _("Strikethrough"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_FORMAT_COLOR, _("Color"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_FORMAT_BKG_COLOR, _("Text Background Color"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_FORMAT_REMOVE, _("Remove Formatting"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_VIEW_FULLSCREEN, _("Full Screen"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_VIEW_TOOLBAR, _("View Toolbar"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_VIEW_NODE_TITLEBAR, _("View Node Title Bar"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_VIEW_STATUS_BAR, _("View Status Bar"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_VIEW_WRAP, _("Word Wrap"), 'W', false, true, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_VIEW_CHANGE_MODE, _("Change View Mode"), 'T', false, true, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_VIEW_WORD_COUNT, _("Calculate Word Count"), 'K', false, true, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_HELP_INDEX, _("Show help"), GDK_F1, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_HELP_ABOUT, _("About"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_GLOBAL, NC_ACTION_VISIT_WEBSITE, _("Visit Website"), 0, false, false, false));

		//tree only actions
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_TREE_WIDGET, NC_ACTION_TREE_NODE_NEW, _("New Node"), GDK_Insert, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_TREE_WIDGET, NC_ACTION_TREE_NODE_NEW_CHILD, _("New Child Node"), GDK_Insert, true, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_TREE_WIDGET, NC_ACTION_TREE_NODE_DELETE, _("Delete Node"), GDK_Delete, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_TREE_WIDGET, NC_ACTION_TREE_NODE_RENAME, _("Rename Node"), GDK_F2, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_TREE_WIDGET, NC_ACTION_TREE_NODE_MOVE_UP, _("Move Node Up"), GDK_Up, true, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_TREE_WIDGET, NC_ACTION_TREE_NODE_MOVE_DOWN, _("Move Node Down"), GDK_Down, true, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_TREE_WIDGET, NC_ACTION_TREE_NODE_MOVE_LEFT, _("Move Node Left"), GDK_Left, true, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_TREE_WIDGET, NC_ACTION_TREE_NODE_MOVE_RIGHT, _("Move Node Right"), GDK_Right, true, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_TREE_WIDGET, NC_ACTION_TREE_NODE_PROPERTIES, _("Show Node Properties"), GDK_Return, false, true, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_TREE_WIDGET, NC_ACTION_TREE_NODE_DELETE_FINISHED, _("Delete Completed Nodes"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_TREE_WIDGET, NC_ACTION_TREE_NODE_TOGGLE_FINISHED, _("Toggle Completed Status"), ' ', false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_TREE_WIDGET, NC_ACTION_TREE_SORT_CHILDREN_ASC, _("Sort Children Ascending"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_TREE_WIDGET, NC_ACTION_TREE_SORT_CHILDREN_DESC, _("Sort Children Descending"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_TREE_WIDGET, NC_ACTION_TREE_SORT_ROOT_ASC, _("Sort Tree Ascending"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_TREE_WIDGET, NC_ACTION_TREE_SORT_ROOT_DESC, _("Sort Tree Descending"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_TREE_WIDGET, NC_ACTION_TREE_EXPAND_ALL, _("Expand All"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_TREE_WIDGET, NC_ACTION_TREE_COLAPSE_ALL, _("Colapse All"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_TREE_WIDGET, NC_ACTION_EDIT_COPY_BR_STRUCT, _("Copy Branch Structure"), 0, false, false, false));
		g_lstDefAction.push_back(ShortcutDef(SH_CTX_TREE_WIDGET, NC_ACTION_EDIT_PASTE_BR_ROOT, _("Paste Branch Root"), 0, false, false, false));
	}
}

ShortcutsList::~ShortcutsList()
{
}

bool ShortcutsList::Load()
{
	clear();

	//if the shorcuts were never defined, set the default ones
	if(!g_objIni.SectionExists("Shortcuts")){
		*this = g_lstDefAction;
		Save();
		return false;
	}

	//read from INI
	int nEntries = 0;
	g_objIni.GetValue("Shortcuts", "NumEntries", nEntries);

	for(int i=0; i<nEntries; i++)
	{
		char szKey[100];
		ShortcutDef info;

		bool bValue;
		sprintf(szKey, "ModShift_%d", i+1);
		g_objIni.GetValue("Shortcuts", szKey, bValue);
		info.m_bModShift = bValue;	//FIX: VS2005 complains when using directly without temp variable
		sprintf(szKey, "ModCtrl_%d", i+1);
		g_objIni.GetValue("Shortcuts", szKey, bValue);
		info.m_bModCtrl = bValue;
		sprintf(szKey, "ModAlt_%d", i+1);
		g_objIni.GetValue("Shortcuts", szKey, bValue);
		info.m_bModAlt = bValue;
		sprintf(szKey, "Key_%d", i+1);
		g_objIni.GetValue("Shortcuts", szKey, info.m_nKey, 0);
		sprintf(szKey, "ActionID_%d", i+1);
		g_objIni.GetValue("Shortcuts", szKey, info.m_nActionID, 0);
		sprintf(szKey, "Context_%d", i+1);
		g_objIni.GetValue("Shortcuts", szKey, info.m_nCtx, 0);

		//forbid invalid and duplicate shortcuts
		if( info.IsValid(info.m_nCtx) /*&&
			FindByKey(info) < 0*/)
		{
			push_back(info);
		}
	}

	return true;
}

bool ShortcutsList::Save()
{
	int nEntries = size();
	g_objIni.SetValue("Shortcuts", "NumEntries", nEntries);

	for(int i=0; i<nEntries; i++)
	{
		char szKey[100];
		ShortcutDef &info = operator[](i);

		sprintf(szKey, "ModShift_%d", i+1);
		g_objIni.SetValue("Shortcuts", szKey, info.m_bModShift);
		sprintf(szKey, "ModCtrl_%d", i+1);
		g_objIni.SetValue("Shortcuts", szKey, info.m_bModCtrl);
		sprintf(szKey, "ModAlt_%d", i+1);
		g_objIni.SetValue("Shortcuts", szKey, info.m_bModAlt);
		sprintf(szKey, "Key_%d", i+1);
		g_objIni.SetValue("Shortcuts", szKey, info.m_nKey);
		sprintf(szKey, "ActionID_%d", i+1);
		g_objIni.SetValue("Shortcuts", szKey, info.m_nActionID);
		sprintf(szKey, "Context_%d", i+1);
		g_objIni.SetValue("Shortcuts", szKey, info.m_nCtx);
	}

	return true;
}

int	ShortcutsList::FindByKey(ShortcutDef &key, int nSkipLine, int nActionID)
{
	int nMax = size();
	for(int i=0; i<nMax; i++)
	{
		if(i != nSkipLine && operator[](i) == key)
		{
			//check if action matches (if required)
			if(nActionID >= 0 && operator[](i).m_nActionID != key.m_nActionID)
				continue;
			return i;
		}
	}
	return -1;
}

int	ShortcutsList::FindByAction(int nActionID, int nCtx)
{
	int nMax = size();
	for(int i=0; i<nMax; i++)
		if( operator[](i).m_nActionID == nActionID &&
			operator[](i).m_nCtx	  == nCtx)
			return i;
	return -1;
}

void ShortcutsList::UpdateAction(ShortcutDef &key)
{
	int nPos = FindByAction(key.m_nActionID, key.m_nCtx);
	if(nPos >= 0)
		operator[](nPos) = key;
	else
		push_back(key);
}

void ShortcutsList::ClearAction(int nActionID, int nCtx)
{
	int nPos = FindByAction(nActionID, nCtx);
	if(nPos >= 0)
		erase(begin() + nPos);
}

int	ShortcutsList::FindByContext(int nCtx, int nIdx)
{
	int nMax = size();
	for(int i=0; i<nMax; i++)
		if(operator[](i).m_nCtx == nCtx)	//return on first proper context (guess the index)
			return i+nIdx;
	return -1;
}
