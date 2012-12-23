////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements Action to delete all "finished" nodes
////////////////////////////////////////////////////////////////////////////

#ifndef DOCACTIONFINISHDEL_H_
#define DOCACTIONFINISHDEL_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "lib/DocActionBase.h"

#if _MSC_VER > 1000
  #pragma warning(disable: 4786)
#endif
#include <string>

class DocActionFinishDel : public DocActionBase
{
public:
	DocActionFinishDel();
	virtual ~DocActionFinishDel();

	virtual void Exec(bool bInteractive = false);
	virtual void Undo();

	void DoTreeFinished();

public:
	int			 m_nSelIdx;
	NoteDocument m_objOrigTree;
	int			 m_nDeletedCnt;	//deleted count
};

#endif // DOCACTIONFINISHDEL_H_
