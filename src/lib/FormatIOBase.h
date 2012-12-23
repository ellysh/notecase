////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Abstract class for different document formats I/O
////////////////////////////////////////////////////////////////////////////

#ifndef NODEFORMATIO_H__
#define NODEFORMATIO_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NoteDocument.h"
class NoteDocument;

class FormatIO_Base
{
public:
	FormatIO_Base();
	virtual ~FormatIO_Base();

	virtual int Load(const char *szFile, NoteDocument &doc) = 0;
	virtual int Save(const char *szFile, NoteDocument &doc) = 0;
	virtual int GetFormat() = 0;

	static void RunLoop();

protected:
	NoteDocument *m_pDoc;
};

#endif // NODEFORMATIO_H__
