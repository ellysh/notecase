////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements Blowfish encrypted NoteCase HTML format I/O
////////////////////////////////////////////////////////////////////////////

#ifndef FormatIOEncHTML_H__
#define FormatIOEncHTML_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FormatIOHtml.h"
#include "types.h"	//INT64

//
// encrypted HTML format support
//

class FormatIO_EncHTML : public FormatIO_HTML
{
public:
	FormatIO_EncHTML();
	virtual ~FormatIO_EncHTML();

	virtual int Load(const char *szFile, NoteDocument &doc);
	virtual int Save(const char *szFile, NoteDocument &doc);
	virtual int GetFormat(){ return FORMAT_NOTECASE_HTML_ENC; };

	void SetPassword(const char *szPass);
	int  Save(const char *szFile, NoteDocument &doc, bool bAppendMode);
	void SetLoadEmbedded(INT64 nStart, INT64 nEnd){ m_nStartOffset = nStart; m_nEndOffset = nEnd; }

protected:
	int m_nEncFormatVersion;
	std::string m_strPass;
	INT64 m_nOrigSize;	//unencrypted file size

	//load embedded doc
	INT64 m_nStartOffset;
	INT64 m_nEndOffset;
};

#endif // FORMATENCHTML_H__
