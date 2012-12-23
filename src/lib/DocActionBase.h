////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Defines abstract class for atomic action performed on NoteCase document
//		 (base for Undo/Redo framework)
////////////////////////////////////////////////////////////////////////////

#ifndef DOCACTIONBASE_H_
#define DOCACTIONBASE_H_

#include "NoteDocument.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class DocActionBase
{
public:
	DocActionBase(){ m_pDoc = NULL; };
	virtual ~DocActionBase(){};

	virtual void Exec(bool bInteractive = false) = 0;
	virtual void Undo() = 0;

	void SetDoc(NoteDocument &doc){ m_pDoc = &doc; }

protected:
	NoteDocument *m_pDoc;	//pointer to the document
};

#endif // DOCACTIONBASE_H_
