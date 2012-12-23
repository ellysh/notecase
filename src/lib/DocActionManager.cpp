////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Object that manages document changes history list (Undo/Redo framework)
////////////////////////////////////////////////////////////////////////////

#include "DocActionManager.h"

DocActionManager::DocActionManager(int nMaxListSize)
{
	m_nMaxListSize = nMaxListSize;
	m_nPos = 0;
}

DocActionManager::~DocActionManager()
{
	Clear();
}

void DocActionManager::Clear()
{
	//delete all action objects
	for(unsigned int i=0; i<m_lstHistory.size(); i++)
		delete m_lstHistory[i];
	m_lstHistory.clear();	//clear pointer list
	m_nPos = 0;
}

bool DocActionManager::CanUndo()
{
	return (m_lstHistory.size() > 0) && (m_nPos > 0);
}

bool DocActionManager::CanRedo()
{
	return (m_lstHistory.size() > 0) && ((m_lstHistory.size() - m_nPos) > 0);
}

void DocActionManager::AddAction(DocActionBase *pAction)
{
	//each new action destroys redo part of the list
	ClearRedoList();

	//push new undo action into the list
	m_lstHistory.push_back(pAction);
	m_nPos ++;

	//check if we overgrown allowed list size
	if(m_lstHistory.size() > m_nMaxListSize)
	{
		//delete oldest entry
		delete m_lstHistory[0];
		m_lstHistory.erase(m_lstHistory.begin(), m_lstHistory.begin()+1);
		m_nPos --;
	}
}

void DocActionManager::Undo()
{
	if(CanUndo())
	{
		m_nPos --;
		m_lstHistory[m_nPos]->Undo();
	}
}

void DocActionManager::Redo()
{
	if(CanRedo())
	{
		m_lstHistory[m_nPos]->Exec();
		m_nPos ++;
	}
}

void DocActionManager::ClearRedoList()
{
	// clear redo part of the list
	if(CanRedo()){
		for(unsigned int i=m_nPos; i<m_lstHistory.size(); i++)
			delete m_lstHistory[i];
		m_lstHistory.erase(m_lstHistory.begin()+m_nPos, m_lstHistory.end());
	}
}

// returns a pointer to the specified action.
DocActionBase* DocActionManager::GetAction(unsigned int nPos) const
{
	// make sure that index is valid
	if(nPos < 0 || nPos > m_lstHistory.size()) return NULL;

	return m_lstHistory[nPos];
}

// returns a pointer to the current action.
DocActionBase* DocActionManager::GetCurrentAction() const
{
	// make sure that current undo action exists
	if(m_nPos < 1 || m_nPos > m_lstHistory.size()) return NULL;

	return m_lstHistory[m_nPos - 1];
}
