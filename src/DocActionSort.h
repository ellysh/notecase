////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements Node Sort Action
////////////////////////////////////////////////////////////////////////////

#ifndef DOCACTIONSORT_H_
#define DOCACTIONSORT_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "lib/DocActionBase.h"

#if _MSC_VER > 1000
  #pragma warning(disable: 4786)
#endif
#include <string>

class DocActionSort : public DocActionBase
{
public:
	DocActionSort();
	virtual ~DocActionSort();

	virtual void Exec(bool bInteractive = false);
	virtual void Undo();

public:
	// action info
	int		m_nNodeIdx;
	bool	m_bAscending;
	//bool	m_bRecursive;	//TOFIX add support

	// undo info
	std::vector<int> m_lstOldIdxOrder;
	std::vector<int> m_lstNewIdxOrder;
};

#endif // DOCACTIONSORT_H_
