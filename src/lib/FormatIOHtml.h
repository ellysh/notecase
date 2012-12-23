////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements NoteCase HTML format (and NoteCenter format) I/O
////////////////////////////////////////////////////////////////////////////

#ifndef FormatIOHTML_H__
#define FormatIOHTML_H__

#include "HtmlParser.h"
#include "FormatIOBase.h"
#include "IOProcess.h"
#include "DocumentIterator.h"

class FormatIO_HTML : public HTMLParser, public FormatIO_Base
{
public:
	FormatIO_HTML();
	virtual ~FormatIO_HTML();

	virtual int Load(const char *szFile, NoteDocument &doc);
	virtual int Save(const char *szFile, NoteDocument &doc);
	virtual int GetFormat(){ return FORMAT_HTML; };

	bool m_bNoteCenterMode;
	bool m_bHtmlExport;
	std::string m_strFile;
	bool m_bBRTagUsed;

	std::string m_strGeneratorApp;	// name of app read from the document
	std::string m_strGeneratorVer;	// version of app read from the document

protected:
	virtual void OnTagBegin(const char *szTag, const char *szParams = NULL);
	virtual void OnTagEnd(const char *szTag);
	virtual void OnComment(const char *szText);
	virtual void OnText(const char *szText);
	void WriteDocContents(IOProcess &objFile);
	void ConvertInternalLinks();

	void WriteTOC(DocumentIterator &it, IOProcess &file, int nParentID);
	void WriteLevel(DocumentIterator &it, IOProcess &file, int nParentID, bool bHtmlExport = false, bool bCustomCSS = false);

protected:
	//document parse info
	int m_nCurParentIdx;
	int m_nCurNodeIdx;
	int m_nDocParseState;
	int m_nTagsParsedCount;		//used to detect error in documents
	int m_nPicCountInCurNode;

	//temp vars
	std::string m_strLinkURL;
	std::string m_strLinkText;
	std::string m_strLinkData;	// for attachment link (embedded data)
	std::string m_strLinkTitle;
	int         m_nLinkStart;
	int         m_nLinkIdx;
	int			m_nExportImgCount;	//gives exported images unique ID (name)
	int			m_nExportAttCount;	//gives exported attachments unique ID (name)
	bool		m_bExportOverwrite;	//auto overwrite all attachments and images
};

#endif
