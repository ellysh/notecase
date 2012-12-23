////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements add/remove attachment Action
////////////////////////////////////////////////////////////////////////////

#include "DocActionAtt.h"
#include "callbacks.h"
#include "support.h"
#include "interface.h"
#include "lib/NoteDocument.h"
#include "lib/debug.h"
#include "MainWnd.h"
#include <algorithm>

extern NoteDocument g_doc;
extern MainWindow g_wnd;
int GetSelectedNodeIdx();

DocActionAtt::DocActionAtt()
{
	m_nNodeIdx	= -1;
	m_bAddOp	= true;
	m_nAttIdx	= -1;
}

DocActionAtt::~DocActionAtt()
{
}

void DocActionAtt::Exec(bool bInteractive)
{
	if(m_bAddOp){
		g_doc.GetNodeByIdx(m_nNodeIdx).m_lstAttachments.insert(
			g_doc.GetNodeByIdx(m_nNodeIdx).m_lstAttachments.begin() + m_nAttIdx,
			m_info);
	}
	else{
		g_doc.GetNodeByIdx(m_nNodeIdx).m_lstAttachments.erase(
			g_doc.GetNodeByIdx(m_nNodeIdx).m_lstAttachments.begin() + m_nAttIdx);

	}

	//refresh toggle button state
	bool bAttExist = (g_doc.GetNodeByIdx(m_nNodeIdx).m_lstAttachments.size() > 0);
	g_signal_handlers_block_by_func(g_wnd.m_pToolbarAttachment, (void *)on_node_attachments_activate, 0);
	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(g_wnd.m_pToolbarAttachment), bAttExist);
	g_signal_handlers_unblock_by_func(g_wnd.m_pToolbarAttachment, (void *)on_node_attachments_activate, 0);
}

void DocActionAtt::Undo()
{
	//reverse operation
	if(m_bAddOp){
		g_doc.GetNodeByIdx(m_nNodeIdx).m_lstAttachments.erase(
			g_doc.GetNodeByIdx(m_nNodeIdx).m_lstAttachments.begin() + m_nAttIdx);
	}
	else{
		g_doc.GetNodeByIdx(m_nNodeIdx).m_lstAttachments.insert(
			g_doc.GetNodeByIdx(m_nNodeIdx).m_lstAttachments.begin() + m_nAttIdx,
			m_info);
	}

	//refresh toggle button state
	bool bAttExist = (g_doc.GetNodeByIdx(m_nNodeIdx).m_lstAttachments.size() > 0);
	g_signal_handlers_block_by_func(g_wnd.m_pToolbarAttachment, (void *)on_node_attachments_activate, 0);
	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(g_wnd.m_pToolbarAttachment), bAttExist);
	g_signal_handlers_unblock_by_func(g_wnd.m_pToolbarAttachment, (void *)on_node_attachments_activate, 0);
}
