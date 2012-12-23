////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements export to standalone executable
////////////////////////////////////////////////////////////////////////////

#ifndef FormatIOExecutable_H__
#define FormatIOExecutable_H__

#include "FormatIOBase.h"

class FormatIO_Executable : public FormatIO_Base
{
public:
	FormatIO_Executable();
	virtual ~FormatIO_Executable();

	virtual int Load(const char *szFile, NoteDocument &doc);
	virtual int Save(const char *szFile, NoteDocument &doc);
	virtual int GetFormat(){ return FORMAT_EXECUTABLE; };

	std::string m_strGeneratorApp;	// name of app read from the document
	std::string m_strGeneratorVer;	// version of app read from the document

protected:
	//document parse info
	int m_nCurParentID;
	int m_nCurNodeID;
	int m_nLastNodeLevel;
	int m_nCurTextLine;
	int m_nCoversionFailuresCnt;
};

#endif
