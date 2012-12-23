////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements Picture insert Action
////////////////////////////////////////////////////////////////////////////

#ifndef DOCACTIONPIX_H_
#define DOCACTIONPIX_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "lib/DocActionBase.h"

#if _MSC_VER > 1000
  #pragma warning(disable: 4786)
#endif
#include <string>

class DocActionPix : public DocActionBase
{
public:
	DocActionPix();
	virtual ~DocActionPix();

	virtual void Exec(bool bInteractive = false);
	virtual void Undo();

public:
	// action info
	int		m_nNodeIdx;
	//TOFIX use PixInfo for data below
	int		m_nOffset;
	bool	m_bUsePNG;
	GdkPixbuf *m_pixbuf;
};

#endif // DOCACTIONPIX_H_
