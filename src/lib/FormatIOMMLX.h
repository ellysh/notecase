////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements MM/LX Mindmap/Outline document format I/O (reading only)
////////////////////////////////////////////////////////////////////////////

#ifndef FormatIOMMLX_H__
#define FormatIOMMLX_H__

#include "FormatIOBase.h"

class FormatIO_MMLX : public FormatIO_Base
{
public:
	FormatIO_MMLX();
	virtual ~FormatIO_MMLX();

	virtual int Load(const char *szFile, NoteDocument &doc);
	virtual int Save(const char *szFile, NoteDocument &doc);
	virtual int GetFormat(){ return FORMAT_MMLX; };

protected:
	void ParseLine(char *szBuffer);

protected:
	//document parse info
	int m_nCurParentID;
	int m_nCurNodeID;
	int m_nLastNodeLevel;
	int m_nCurTextLine;
	int m_nCoversionFailuresCnt;
};

#endif
