////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Defines atomic action performed on NoteCase document
//		 (base for Undo/Redo framework)
////////////////////////////////////////////////////////////////////////////

#ifndef DOCACTION_H_
#define DOCACTION_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "lib/DocActionBase.h"
#include "lib/NoteDocument.h"

#if _MSC_VER > 1000
  #pragma warning(disable: 4786)
#endif
#include <string>

//TOFIX separate class for each action?

//define all atomic actions in document editing
#define ACT_UNDEFINED	0
#define ACT_TEXT_INSERT	1
#define ACT_TEXT_DELETE	2
#define ACT_NODE_INSERT	3
#define ACT_NODE_RENAME	4
#define ACT_TREE_DELETE	5	//delete branch of nodes
#define ACT_TREE_MOVE	6	//move branch of nodes
#define ACT_TREE_IMPORT	7	//multiple nodes inserted
#define ACT_TREE_DND	8	//drag and drop (move branch of nodes) TOFIX merge with ACT_TREE_MOVE algorithm
#define ACT_TEXT_REPLACE 9	//replace text operation
#define ACT_TEXT_LINK_ADD	10
#define ACT_TEXT_LINK_REMOVE	11

//define move directions
#define MOVE_LEFT	1
#define MOVE_RIGHT	2
#define MOVE_UP		3
#define MOVE_DOWN	4


class DocAction : public DocActionBase
{
public:
	DocAction();
	virtual ~DocAction();

	virtual void Exec(bool bInteractive = false);
	virtual void Undo();
	//virtual void std::string GetDescription();

	void SetType(int nType){ m_nType = nType; }
	int  GetType(){ return m_nType; }

protected:
	void DoTextInsert();
	void DoTextDelete();
	void DoTreeInsert();
	void DoTreeDelete();
	void DoNodeRename();
	void DoNodeUnrename();
	void DoTreeMove();
	void DoTreeUnmove();
	void DoTreeImport();
	void DoTreeUnimport();
	void DoTreeDND(bool bForward = false);
	void DoTextReplace();
	void DoTextUnreplace();
	void DoLinkAdd();
	void DoLinkRemove();
	void DoTreeFinished();
	void DoTreeUnfinished();

protected:
	int	m_nType;	//type of action, see defines above
	bool m_bUndo;
	bool m_bInteractive;

public:
	//node text editing
	std::string	m_strNodeText;	//store inserted/deleted text
	int m_nTextStartPos;		//text offset inside the node
	bool m_bSelected;		//was text in case selected

	//additional data for node text deleting
	std::vector<PixInfo> m_lstPictures;
	std::vector<FmtInfo> m_lstFmt;
	LinkInfoList m_lstLinks;

	// node renaming/inserting/deleting
	int m_nNodeIndex;			//node position (placement)
	int m_nNodeSibling;			//sibling index (index in the )
	int m_nNodeID;				//node ID
	int m_nNodePID;				//node ID
	std::string	m_strNodeNameNew;	//store new node name (for rename/redo only)
	std::string	m_strNodeNameOld;	//store old node name (for rename/undo only)

	//replace info
	std::string	m_strFind;		//
	std::string	m_strReplace;	//
	int  m_nOffset;
	bool m_bNodeTitle;

	//additional data for node moving
	int m_nMoveDirection;

	//node DnD
	int m_nNewNodeID;
	int m_nNodeNewPID;			//node position (placement)
	int m_nNodeNewSibling;		//sibling index (index in the )

	//data for link info
	int  m_nLinkOffset;
	std::string	m_strLinkTitle;
	std::string	m_strLinkUrl;
	int  m_nLinkNodeID;

	NoteDocument m_objSubTree;	//when working with nodes
};

#endif // DOCACTION_H_
