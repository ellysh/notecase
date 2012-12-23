////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements Gjots2 format I/O (reading only)
////////////////////////////////////////////////////////////////////////////

#ifndef FormatIOGjots2_H__
#define FormatIOGjots2_H__

#include "FormatIOBase.h"

class FormatIO_Gjots2 : public FormatIO_Base
{
public:
	FormatIO_Gjots2();
	virtual ~FormatIO_Gjots2();

	virtual int Load(const char *szFile, NoteDocument &doc);
	virtual int Save(const char *szFile, NoteDocument &doc);
	virtual int GetFormat(){ return FORMAT_GJOTS2; };

protected:
	bool ParseLine(const char *szBuffer);

protected:
	//document parse info
	int m_nCurParentID;
	int m_nCurNodeID;
	int m_nDocParseState;
};

#endif
