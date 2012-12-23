////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements add/remove attachment Action
////////////////////////////////////////////////////////////////////////////

#ifndef DOCACTIONATT_H_
#define DOCACTIONATT_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "lib/DocActionBase.h"

#if _MSC_VER > 1000
  #pragma warning(disable: 4786)
#endif
#include <string>

class DocActionAtt : public DocActionBase
{
public:
	DocActionAtt();
	virtual ~DocActionAtt();

	virtual void Exec(bool bInteractive = false);
	virtual void Undo();

public:
	// action info
	bool	m_bAddOp;
	int		m_nNodeIdx;
	int		m_nAttIdx;
	AttInfo m_info;
};

#endif // DOCACTIONATT_H_
