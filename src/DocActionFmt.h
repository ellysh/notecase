////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements text format Action
////////////////////////////////////////////////////////////////////////////

#ifndef DOCACTIONFMT_H_
#define DOCACTIONFMT_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "lib/DocActionBase.h"

#if _MSC_VER > 1000
  #pragma warning(disable: 4786)
#endif
#include <string>

class DocActionFmt : public DocActionBase
{
public:
	DocActionFmt();
	virtual ~DocActionFmt();

	virtual void Exec(bool bInteractive = false);
	virtual void Undo();

public:
	// action info
	int		m_nOperationTag;
	int		m_nNodeIdx;
	int		m_nStartOffset;
	int		m_nEndOffset;
	bool	m_bCtrlPressed;

	//
	std::vector<FmtInfo>	m_lstFmt;
	GdkColor				m_color;
};

#endif // DOCACTIONFMT_H_
