////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Object that manages document changes history list (Undo/Redo framework)
////////////////////////////////////////////////////////////////////////////

#ifndef DOCACTIONMANAGER_H_
#define DOCACTIONMANAGER_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DocActionBase.h"
#include <vector>

class DocActionManager
{
public:
	DocActionManager(int nMaxListSize);
	virtual ~DocActionManager();

	//info methods
	int GetPos() const { return m_nPos; }
	int GetTotal() const { return m_lstHistory.size(); }
	int GetMaximal() const { return m_nMaxListSize; }

	bool CanUndo();
	bool CanRedo();

	DocActionBase* GetAction(unsigned int nPos) const;
	DocActionBase* GetCurrentAction() const;

	//action methods
	void AddAction(DocActionBase *pAction);
	void Undo();
	void Redo();

	//other
	void Clear();

protected:
	void ClearRedoList();

protected:
	std::vector<DocActionBase *> m_lstHistory;	//list of both redo and undo actions
	unsigned int m_nPos;				//indicates first redo action in the list (divides undo from redo)
	unsigned int m_nMaxListSize;			//maximal allowed size of history list
};

#endif // DOCACTIONMANAGER_H_
