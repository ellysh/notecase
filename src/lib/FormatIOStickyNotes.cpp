////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements NoteCase HTML format (and NoteCenter format) I/O
////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
  #pragma warning(disable: 4996)	//ignore deprecated functions warning
#endif

#include "FormatIOStickyNotes.h"
#include "IOProcess.h"
#include "IOLayerFile64.h"
#include "DocumentIterator.h"
#include <stdio.h>	//sprintf
#ifndef _WIN32
 #include <strings.h>	//strcasecmp
#else
 #define strcasecmp stricmp
#endif

#define  DOC_PARSE_BLANK		0
#define  DOC_PARSE_EXPECT_TITLE	1
#define  DOC_PARSE_EXPECT_TEXT	2

void DumpDocument(NoteDocument &doc, const char *szFile);
void DbgTrace(char *szFmt, ...);

FormatIO_StickyNotes::FormatIO_StickyNotes()
{
	m_nCurParentID = -1;
	m_nCurNodeID = -1;
	m_nDocParseState = DOC_PARSE_BLANK;
	m_nTagsParsedCount = 0;
}

FormatIO_StickyNotes::~FormatIO_StickyNotes()
{
}

int FormatIO_StickyNotes::Load(const char *szFile, NoteDocument &doc)
{
	m_pDoc = &doc;  //store pointer

	//clear old document contents
	doc.Clear();

	IOProcess objFile(MODE_READ);

	// attach file read layer
	IOLayerFile64 *pFileLayer = new IOLayerFile64;
	objFile.PushLayer(pFileLayer);

	if(!pFileLayer->m_file.Open(szFile, F64_READ|F64_SHARE_READ|F64_OPEN_EXISTING))
		return DOC_LOAD_NOT_FOUND;

	char szBuffer[1024];
	int nRead;

	//TOFIX better error handling
	bool bContinue = true;
	while(bContinue)
	{
		RunLoop();	//enable progress painting events

		// some layers cache data from one step and can return 0 bytes in some cycle
		nRead = objFile.Read(szBuffer, sizeof(szBuffer), bContinue);
		if(nRead > 0)
			Parse(szBuffer, nRead);
	}

	//detect errors in file format (should have at least some HTML tags inside)
	if(0 == m_nTagsParsedCount)
		return DOC_LOAD_FORMAT_ERROR;

	return DOC_LOAD_OK;
}

int FormatIO_StickyNotes::Save(const char *szFile, NoteDocument &doc)
{
	return DOC_SAVE_NOT_SUPPORTED;	//export not supported
}

// HTMLParser event handlers
void FormatIO_StickyNotes::OnTagBegin(const char *szTag, const char *szParams)
{
	m_nTagsParsedCount ++;

	//if proper tag, create new node
	if(0 == strcasecmp(szTag, "note"))	//new note
	{
		m_pDoc->NodeInsert(-1, -1);  //TOFIX error report
		int nIdx = m_pDoc->GetNodeCount()-1;
		m_nCurNodeID = m_pDoc->GetNodeByIdx(nIdx).m_nID;
		m_nDocParseState = DOC_PARSE_EXPECT_TEXT;

		//parse title from tag parameters
		std::string strTitle("New node");
		const char *szTitleStart = strstr(szParams, "title=\"");
		if(NULL != szTitleStart)
		{
			printf("params: %s\n",szParams);
			szTitleStart += strlen("title=\"");
			strTitle = szTitleStart;
			const char *szTitleEnd = strstr(szTitleStart, "\"");
			if(NULL != szTitleEnd)
				strTitle = strTitle.substr(0, szTitleEnd-szTitleStart);
		}
		m_pDoc->GetNodeByIdx(nIdx).SetTitle(strTitle.c_str());
	}
}

void FormatIO_StickyNotes::OnTagEnd(const char *szTag)
{
	//if proper tag, position on the parent node ?
	if(0 == strcasecmp(szTag, "note"))
	{
		m_nCurNodeID   = -1;
	}
}

void FormatIO_StickyNotes::OnComment(const char *szText)
{
}

void FormatIO_StickyNotes::OnText(const char *szText)
{
	std::string data(szText);

	//set text or title for current node
	if(DOC_PARSE_EXPECT_TEXT == m_nDocParseState)
	{
		HTMLParser::UnescapeChars(data);	//only for node text (NC format)

		//set node text
		int nIdx = m_pDoc->GetIdxFromID(m_nCurNodeID);
		if(nIdx >= 0)
			m_pDoc->GetNodeByIdx(nIdx).SetText(data.c_str());

		m_nDocParseState = DOC_PARSE_BLANK;
	}
}
