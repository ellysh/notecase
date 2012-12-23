////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements Action to mark node "finished"
////////////////////////////////////////////////////////////////////////////

#ifndef DOCACTIONFINISH_H_
#define DOCACTIONFINISH_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "lib/DocActionBase.h"

#if _MSC_VER > 1000
  #pragma warning(disable: 4786)
#endif
#include <string>

class DocActionFinish : public DocActionBase
{
public:
	DocActionFinish();
	virtual ~DocActionFinish();

	virtual void Exec(bool bInteractive = false);
	virtual void Undo();

	void DoTreeFinished();

public:
	// action info
	int		m_nNodeIndex;
	bool	m_bRecursiveFinished;
	NoteDocument m_objSubTree;
};

#endif // DOCACTIONFINISH_H_
