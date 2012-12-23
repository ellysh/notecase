////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements NoteCase HTML format (and NoteCenter format) I/O
////////////////////////////////////////////////////////////////////////////

#ifndef FormatIOStickyNotes_H__
#define FormatIOStickyNotes_H__

#include "HtmlParser.h"
#include "FormatIOBase.h"

class FormatIO_StickyNotes : public HTMLParser, public FormatIO_Base
{
public:
	FormatIO_StickyNotes();
	virtual ~FormatIO_StickyNotes();

	virtual int Load(const char *szFile, NoteDocument &doc);
	virtual int Save(const char *szFile, NoteDocument &doc);
	virtual int GetFormat(){ return FORMAT_STICKY; };

	bool m_bNoteCenterMode;
	bool m_bHtmlExport;

protected:
	virtual void OnTagBegin(const char *szTag, const char *szParams = NULL);
	virtual void OnTagEnd(const char *szTag);
	virtual void OnComment(const char *szText);
	virtual void OnText(const char *szText);

protected:
	//document parse info
	int m_nCurParentID;
	int m_nCurNodeID;
	int m_nDocParseState;
	int m_nTagsParsedCount;		//used to detect error in documents
};

#endif
