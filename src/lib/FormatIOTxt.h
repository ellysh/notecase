////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements plain text format I/O (reading only)
////////////////////////////////////////////////////////////////////////////

#ifndef FormatIOTxt_H__
#define FormatIOTxt_H__

#include "FormatIOBase.h"

class FormatIO_Txt : public FormatIO_Base
{
public:
	FormatIO_Txt();
	virtual ~FormatIO_Txt();

	virtual int Load(const char *szFile, NoteDocument &doc);
	virtual int Save(const char *szFile, NoteDocument &doc);
	virtual int GetFormat(){ return FORMAT_TXT; };
};

#endif
