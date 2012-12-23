////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements NoteCase HTML format (and NoteCenter format) I/O
////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
  #pragma warning(disable: 4786)
  #pragma warning(disable: 4996)	//ignore deprecated functions warning
#endif

#include "FormatIOHtml.h"
#include "IOLayerFile64.h"
#include "LinkInfo.h"
#include <stdio.h>	//sprintf
#include <stdlib.h>	//sprintf
#include <gtk/gtk.h>
#include "IniFile.h"
#include "Base64.h"
#include "../gui/ProgressDlg.h"

#include "../config.h"
#include "../support.h"	//TOFIX dependent on non-core

#ifndef _WIN32
 #include <strings.h>	//strcasecmp
#else
 #ifndef __MINGW32__
   #ifndef snprintf
    #define snprintf _snprintf
   #endif
 #endif
 #define strcasecmp stricmp
#endif
#include "debug.h"

#ifdef _WIN32
 #include <io.h>	// access
 #define access _access
#endif

#ifndef min
 #define min(x,y) (((x)<(y))?(x):(y))
#endif

static bool g_bConvertImageLinks = false;
static int g_nTocDepth = 0;
static bool g_bUseTOC = false;

extern IniFile g_objIni;
extern bool g_bExportDataAsSeparateFile;

static std::string g_strTabReplacement;

int InternalIcon_Name2Index(const char *szName);
const char *InternalIcon_Index2Name(int nIndex);
std::string GetParentDir(const char *szPath);
int gtkMessageBox(const char *szText, int nButtons = GTK_BUTTONS_OK, int nIcon = GTK_MESSAGE_INFO);

#define  DOC_PARSE_BLANK		0
#define  DOC_PARSE_EXPECT_TITLE	1
#define  DOC_PARSE_EXPECT_TEXT	2
#define  DOC_PARSE_INSIDE_LINK	3

void replaceall(std::string &strData, const char *szFind, const char *szReplace);
void WriteTOC(DocumentIterator &it, IOProcess &file, int nParentID);

typedef struct {
	std::string strText;
	bool bIsText;
} tSegment;

FormatIO_HTML::FormatIO_HTML()
{
	m_nCurParentIdx = -1;
	m_nCurNodeIdx = -1;
	m_nDocParseState = DOC_PARSE_BLANK;
	m_bNoteCenterMode = false;
	m_bHtmlExport = false;
	m_nTagsParsedCount = 0;
	m_nLinkStart = -1;
	m_nLinkIdx = -1;
	m_nExportImgCount = 0;
	m_nExportAttCount = 0;
	m_nPicCountInCurNode = 0;
	m_bExportOverwrite = false;
	m_bBRTagUsed = false;
}

FormatIO_HTML::~FormatIO_HTML()
{
}

int FormatIO_HTML::Load(const char *szFile, NoteDocument &doc)
{
	m_pDoc = &doc;  //store pointer

	//clear old document contents
	doc.Clear();

	if(m_bNoteCenterMode){
		m_bAllowUnescapedInPreTag = true;
		m_bBRTagUsed = false;
	}

	IOProcess objFile(MODE_READ);

	// attach file read layer
	IOLayerFile64 *pFileLayer = new IOLayerFile64;
	objFile.PushLayer(pFileLayer);

	if(!pFileLayer->m_file.Open(szFile, F64_READ|F64_SHARE_READ|F64_OPEN_EXISTING))
		return DOC_LOAD_NOT_FOUND;

	char szBuffer[1024];
	int nRead;
	INT64 nReadTotal = 0;

	//TOFIX better error handling
	bool bContinue = true;
	while(bContinue)
	{
		ProgressDlg::SetCurrentSize(nReadTotal);
		RunLoop();	//enable progress painting events

		//NOTE: single layer, no need to Finalize
		// some layers cache data from one step and can return 0 bytes in some cycle
		nRead = objFile.Read(szBuffer, sizeof(szBuffer), bContinue);
		if(nRead > 0){
			Parse(szBuffer, nRead);
			nReadTotal += nRead;
		}
	}

	ConvertInternalLinks();

	//detect errors in file format (should have at least some HTML tags inside)
	if(0 == m_nTagsParsedCount)
		return DOC_LOAD_FORMAT_ERROR;

#ifdef _DEBUG
	doc.AssertValid();
#endif

	return DOC_LOAD_OK;
}

int FormatIO_HTML::Save(const char *szFile, NoteDocument &doc)
{
	m_pDoc = &doc;  //store pointer
	m_strFile = szFile;
	m_nExportImgCount = 0;
	m_nExportAttCount = 0;
	m_bExportOverwrite = false;

	IOProcess objFile(MODE_WRITE);

	// attach file write layer
	IOLayerFile64 *pFileLayer = new IOLayerFile64;
	objFile.PushLayer(pFileLayer);

	if(!pFileLayer->m_file.Open(szFile, F64_WRITE|F64_SHARE_READ|F64_OPEN_NEW))
		return DOC_SAVE_ERR_FILE_OPEN;

#ifdef _DEBUG
	doc.AssertValid();
#endif

	WriteDocContents(objFile);

	return DOC_SAVE_OK;
}

void FormatIO_HTML::WriteDocContents(IOProcess &objFile)
{
	ASSERT(NULL != m_pDoc);

	RunLoop();	//enable progress painting events

	//write header
	if(!m_bHtmlExport)
	{
		if(m_bNoteCenterMode)
			objFile.WriteString("<!DOCTYPE NoteCenter-File>\n");
		else
			objFile.WriteString("<!DOCTYPE NoteCase-File>\n");

		//calc and write selected node "index"
		if(m_pDoc->m_nCurrentNode >= 0){
			char szLastNote[100];
			snprintf(szLastNote, sizeof(szLastNote), "<!--LastNote:%d-->\n", m_pDoc->m_nCurrentNode);
			objFile.WriteString(szLastNote);
		}
	}
	else
		objFile.WriteString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">");

	//write inital HTML tags including HEAD section
	objFile.WriteString("<HTML>\n<HEAD>\n");

	//define used charset
	objFile.WriteString("<meta content=\"text/html;charset=UTF-8\" http-equiv=\"Content-Type\">\n");

	//define application that generated this document
	objFile.WriteString("<meta name=\"generator\" content=\"");
	objFile.WriteString(APP_NAME_STR);
	objFile.WriteString(" ");
	objFile.WriteString(APP_VER_STR);
	objFile.WriteString("\">\n"); //<meta name="generator" content="WordPress 2.0.5" />

	if(m_bHtmlExport)
	{
		objFile.WriteString("<TITLE>");

		//calculate document title from file name
		std::string strTitle = m_strFile;
		std::string::size_type nPos = strTitle.find_last_of("\\/");
		if(nPos != std::string::npos)
			strTitle = strTitle.substr(nPos+1);
		nPos = strTitle.find_last_of(".");
		if(nPos != std::string::npos)
			strTitle = strTitle.substr(0, nPos);

		objFile.WriteString(strTitle.c_str());

		objFile.WriteString("</TITLE>\n");
	}
	else
		objFile.WriteString("<TITLE></TITLE>\n");


	//these export flags are used for HTML format only
	bool bEmbedCSS;
	if(m_pDoc->m_nEmbedCSS >= 0)
		bEmbedCSS = (m_pDoc->m_nEmbedCSS > 0);
	else
		g_objIni.GetValue("Export", "EmbedCSS", bEmbedCSS);
	g_objIni.GetValue("Export", "UseTOC", g_bUseTOC);
	g_objIni.GetValue("Export", "TOCDepth", g_nTocDepth, 1);
	g_objIni.GetValue("Export", "ConvertImageLinks", g_bConvertImageLinks);

	//calculate tab replacement string
	int nTabSize;
	g_objIni.GetValue("Export", "TabSize", nTabSize, 3);
	g_strTabReplacement = "";
	for(int i=0; i<nTabSize; i++)
		g_strTabReplacement += " ";//g_strTabReplacement += "&nbsp;";

	if(m_bHtmlExport)
	{
		if(m_pDoc->m_strCustomCSS.size() > 0)
		{
			if(bEmbedCSS)
			{
				objFile.WriteString("<style type=\"text/css\">\n");
				objFile.WriteString("<!--\n");

				//write CSS file contents directly into the exported document
				FILE *pCSS = fopen(m_pDoc->m_strCustomCSS.c_str(), "r");
				if(NULL != pCSS){
					int nRead;
					char szBuffer[500];
					while((nRead = fread(szBuffer, 1, sizeof(szBuffer)-1, pCSS)) > 0){
						szBuffer[nRead] = '\0';
						objFile.Write(szBuffer, nRead);
					}
					fclose(pCSS);
				}

				objFile.WriteString("-->\n");
				objFile.WriteString("</style>\n");
			}
			else
			{
				objFile.WriteString("<link rel=\"stylesheet\" type=\"text/css\" href=\"");
				objFile.WriteString(m_pDoc->m_strCustomCSS.c_str());
				objFile.WriteString("\">\n");
			}
		}
	}

	objFile.WriteString("</HEAD>\n<BODY>\n");

	//write read-only fiag (this one is global - per document)
	if(m_pDoc->IsReadOnly())
		objFile.WriteString("<!--property:read_only-->\n");

	DocumentIterator it(*m_pDoc);

	//write TOC
	if(m_bHtmlExport && g_bUseTOC && g_nTocDepth > 0)
		WriteTOC(it, objFile, -1);

	//recursively iterate through document node tree
	WriteLevel(it, objFile, -1, m_bHtmlExport, m_pDoc->m_strCustomCSS.size() > 0);

	//write footer
	objFile.WriteString("</BODY>\n</HTML>\n");
}

// HTMLParser event handlers
void FormatIO_HTML::OnTagBegin(const char *szTag, const char *szParams)
{
	//TRACE("FormatIO_HTML:[%s] tag start\n", szTag);
	m_nTagsParsedCount ++;

	//if proper tag, create new node
	if(0 == strcasecmp(szTag, "DL"))	//DL indicates sublevel branching
	{
		m_nCurParentIdx = m_nCurNodeIdx;
		m_nCurNodeIdx   = -1;
	}
	else if(0 == strcasecmp(szTag, "DT"))	//DT indicates new node title coming
	{
		int nPID = (m_nCurParentIdx >= 0) ? m_pDoc->GetNodeByIdx(m_nCurParentIdx).m_nID : -1;

		m_pDoc->NodeInsert(nPID, -1);
		m_nCurNodeIdx = m_pDoc->GetNodeCount()-1;
		m_nDocParseState = DOC_PARSE_EXPECT_TITLE;
		m_nPicCountInCurNode = 0;
		ASSERT(m_nCurNodeIdx >= 0);
	}
	else if(0 == strcasecmp(szTag, "DD"))	//DD indicates new node text coming
	{
		m_nDocParseState = DOC_PARSE_EXPECT_TEXT;
	}
	else if(0 == strcasecmp(szTag, "A"))	//A indicates hyperlink
	{
		ASSERT(m_nCurNodeIdx >= 0);

		if(DOC_PARSE_EXPECT_TITLE == m_nDocParseState)
		{
			//skip if this link marks the node target (name="#1" link)
			//we do not parse the link name but assume that the index was not tampered by user
			//each internal link is the recursive index on the node

			//else if this is the real link inside the node title
			m_strLinkURL = szParams;

			std::string::size_type nPos = m_strLinkURL.find("title=\"");
			if(nPos != std::string::npos)
			{
				std::string::size_type nEnd = m_strLinkURL.find("\"", nPos+strlen("title=\""));
				if(nEnd != std::string::npos){
					m_strLinkTitle = m_strLinkURL.substr(nPos+strlen("title=\""), nEnd-nPos-strlen("title=\""));
					HTMLParser::UnescapeChars(m_strLinkTitle);
				}
			}

			nPos = m_strLinkURL.find("href=\"");
			if(nPos != std::string::npos)
			{
				//m_nDocParseState = DOC_PARSE_INSIDE_LINK;

				m_strLinkURL = m_strLinkURL.substr(nPos+strlen("href=\""));
				if(m_strLinkURL.at(m_strLinkURL.size()-1) == '\"')
					m_strLinkURL = m_strLinkURL.substr(0, m_strLinkURL.size()-1);

				HTMLParser::UnescapeURI(m_strLinkURL);

				m_nLinkStart = 0;
			}
		}
		else
		{
			//link inside the node text
			m_nDocParseState = DOC_PARSE_INSIDE_LINK;

			m_strLinkURL = szParams;

			std::string::size_type nPos = m_strLinkURL.find("href=\"");
			if(nPos != std::string::npos)
			{
				m_strLinkURL = m_strLinkURL.substr(nPos+strlen("href=\""));
				if(m_strLinkURL.at(m_strLinkURL.size()-1) == '\"')
					m_strLinkURL = m_strLinkURL.substr(0, m_strLinkURL.size()-1);

				HTMLParser::UnescapeURI(m_strLinkURL);

				m_nLinkStart = -1;
				if(m_nCurNodeIdx >= 0)
					m_nLinkStart = g_utf8_strlen(m_pDoc->GetNodeByIdx(m_nCurNodeIdx).GetText().c_str(), -1);
			}
		}
	}
	else if(0 == strcasecmp(szTag, "BR")) // line break
	{
		m_bBRTagUsed = true;
		ASSERT(m_nCurNodeIdx >= 0);

		if(m_nCurNodeIdx >= 0){
			if(DOC_PARSE_EXPECT_TITLE == m_nDocParseState)
				m_pDoc->GetNodeByIdx(m_nCurNodeIdx).GetTitle() += "\n";
			else
				m_pDoc->GetNodeByIdx(m_nCurNodeIdx).GetText() += "\n";
		}
	}
	else if(0 == strcasecmp(szTag, "IMG")) // embedded image
	{
		ASSERT(m_nCurNodeIdx >= 0);
		if(m_nCurNodeIdx >= 0)
		{
			//extract image title
			std::string strTitle;
			const char *szTitlePos = strstr(szParams, "title=\"");
			if(szTitlePos){
				szTitlePos += strlen("title=\""); //point to the field data
				const char *szEnd = strstr(szTitlePos, "\"");
				if(szEnd){
					strTitle.assign(szTitlePos, szEnd-szTitlePos);
				}
			}

			bool bUsePNG = false;
			const char *szPos = strstr(szParams, "src=\"data:image/png;base64,");
			if(!szPos)
				szPos = strstr(szParams, "src=\"data:image/jpeg;base64,");
			else
				bUsePNG = true;
			if(szPos)
			{
				szPos = strstr(szPos, ",") + 1; //point to the data
				std::string strData = szPos;

				const char *szEnd = strstr(szPos, "\"");
				if(szEnd){
					int nEndOffset = szEnd-szPos;
					strData.erase(nEndOffset);
				}

				int nBufSize = strData.size() + 1;
				char *szDecode = new char[nBufSize];
				//TOFIX handle error

				CBase64 convert;
				int nSize = convert.Decode(strData.c_str(), szDecode);

				//load data
				GdkPixbufLoader *loader = gdk_pixbuf_loader_new ();
				if (!gdk_pixbuf_loader_write(loader, (guchar *)szDecode, nSize, NULL)){
					//TOFIX err handle
				}
				gdk_pixbuf_loader_close(loader, NULL);

				GdkPixbuf *pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);
				if (!pixbuf) {
					//TOFIX err handle
				}
				gdk_pixbuf_ref (pixbuf); // make sure we keep a reference to it
				g_object_unref (G_OBJECT (loader));

				PixInfo info;
				info.bUsePNG = bUsePNG;
				info.nOffset = m_nPicCountInCurNode + g_utf8_strlen(m_pDoc->GetNodeByIdx(m_nCurNodeIdx).GetText().c_str(), -1);
				info.pixbuf  = pixbuf;
				info.strName = strTitle;
				m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_lstPictures.push_back(info);

				delete [] szDecode;

				m_nPicCountInCurNode ++;
			}
		}
	}
	else if(0 == strcasecmp(szTag, "B"))	//bold text begins
	{
		ASSERT(m_nCurNodeIdx >= 0);
		if(m_nCurNodeIdx >= 0)
		{
			FmtInfo info;
			info.nFormatTag = FMT_BOLD_BEGIN;
			info.nOffset = m_nPicCountInCurNode + g_utf8_strlen(m_pDoc->GetNodeByIdx(m_nCurNodeIdx).GetText().c_str(), -1);
			m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_lstTxtFmt.push_back(info);

			//TRACE("BOLD begin at %d for node:%d\n", info.nOffset, m_nCurNodeIdx);
		}
	}
	else if(0 == strcasecmp(szTag, "I"))	//italic text begins
	{
		ASSERT(m_nCurNodeIdx >= 0);
		if(m_nCurNodeIdx >= 0)
		{
			FmtInfo info;
			info.nFormatTag = FMT_ITALIC_BEGIN;
			info.nOffset = m_nPicCountInCurNode + g_utf8_strlen(m_pDoc->GetNodeByIdx(m_nCurNodeIdx).GetText().c_str(), -1);
			m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_lstTxtFmt.push_back(info);
		}
	}
	else if(0 == strcasecmp(szTag, "U"))	//underlined text begins
	{
		ASSERT(m_nCurNodeIdx >= 0);
		if(m_nCurNodeIdx >= 0)
		{
			FmtInfo info;
			info.nFormatTag = FMT_UNDERLINE_BEGIN;
			info.nOffset = m_nPicCountInCurNode + g_utf8_strlen(m_pDoc->GetNodeByIdx(m_nCurNodeIdx).GetText().c_str(), -1);
			m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_lstTxtFmt.push_back(info);
		}
	}
	else if(0 == strcasecmp(szTag, "S"))	//strikethrough text begins
	{
		ASSERT(m_nCurNodeIdx >= 0);
		if(m_nCurNodeIdx >= 0)
		{
			FmtInfo info;
			info.nFormatTag = FMT_STRIKE_BEGIN;
			info.nOffset = m_nPicCountInCurNode + g_utf8_strlen(m_pDoc->GetNodeByIdx(m_nCurNodeIdx).GetText().c_str(), -1);
			m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_lstTxtFmt.push_back(info);

			//TRACE("STRIKE begin at %d for node:%d\n", info.nOffset, m_nCurNodeIdx);
		}
	}
	else if(0 == strcasecmp(szTag, "font"))	//font color tag begins
	{
		ASSERT(m_nCurNodeIdx >= 0);
		if(m_nCurNodeIdx >= 0)
		{
			const char *szColor = strstr(szParams, "color=\"");
			if(szColor)
			{
				szColor += strlen("color=\"");
				std::string strData = szColor;

				const char *szEnd = strstr(szColor, "\"");
				if(szEnd){
					int nEndOffset = szEnd-szColor;
					strData.erase(nEndOffset);
				}

				GdkColor color;
				gdk_color_parse (strData.c_str(), &color);

				FmtInfo info;
				info.color = color;
				info.nFormatTag = FMT_TXT_COLOR_BEGIN;
				info.nOffset = m_nPicCountInCurNode + g_utf8_strlen(m_pDoc->GetNodeByIdx(m_nCurNodeIdx).GetText().c_str(), -1);
				m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_lstTxtFmt.push_back(info);
			}
		}
	}
	else if(0 == strcasecmp(szTag, "meta"))	//header tag, defines generator app
	{
		//extract generator info (application name and version)
		const char *szGeneratorPos = strstr(szParams, "name=\"generator\"");
		if(szGeneratorPos)
		{
			szGeneratorPos += strlen("name=\"generator\"");
			szGeneratorPos = strstr(szGeneratorPos, "content=\"");
			if(szGeneratorPos)
			{
				szGeneratorPos += strlen("content=\""); //point to the field data
				const char *szEnd = strstr(szGeneratorPos, "\"");
				if(szEnd){
					std::string strTitle;
					strTitle.assign(szGeneratorPos, szEnd-szGeneratorPos);

					std::string::size_type nPos = strTitle.find_last_of(" ");
					if(nPos != std::string::npos){
						m_strGeneratorVer = strTitle.substr(nPos+1);
						m_strGeneratorApp = strTitle.substr(0, nPos);
					}
				}
			}
		}
	}
	else if(0 == strcasecmp(szTag, "span"))	//font background color tag begins
	{
		ASSERT(m_nCurNodeIdx >= 0);
		if(m_nCurNodeIdx >= 0)
		{
			const char *szColor = strstr(szParams, "style=\"background-color:");
			if(szColor)
			{
				szColor += strlen("style=\"background-color:");
				std::string strData = szColor;

				const char *szEnd = strstr(szColor, "\"");
				if(szEnd){
					int nEndOffset = szEnd-szColor;
					strData.erase(nEndOffset);
				}

				GdkColor color;
				gdk_color_parse (strData.c_str(), &color);

				FmtInfo info;
				info.color = color;
				info.nFormatTag = FMT_TXT_BKG_COLOR_BEGIN;
				info.nOffset = m_nPicCountInCurNode + g_utf8_strlen(m_pDoc->GetNodeByIdx(m_nCurNodeIdx).GetText().c_str(), -1);
				m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_lstTxtFmt.push_back(info);
			}
			else{
				//new tag for text color using span
				szColor = strstr(szParams, "style=\"color:");
				if(szColor)
				{
					szColor += strlen("style=\"color:");
					std::string strData = szColor;

					const char *szEnd = strstr(szColor, "\"");
					if(szEnd){
						int nEndOffset = szEnd-szColor;
						strData.erase(nEndOffset);
					}

					GdkColor color;
					gdk_color_parse (strData.c_str(), &color);

					FmtInfo info;
					info.color = color;
					info.nFormatTag = FMT_TXT_COLOR_BEGIN;
					info.nOffset = m_nPicCountInCurNode + g_utf8_strlen(m_pDoc->GetNodeByIdx(m_nCurNodeIdx).GetText().c_str(), -1);
					m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_lstTxtFmt.push_back(info);
				}
			}
		}
	}
}

void FormatIO_HTML::OnTagEnd(const char *szTag)
{
	//TRACE("FormatIO_HTML:[%s] tag end\n", szTag);

	//if proper tag, position on the parent node ?
	if(0 == strcasecmp(szTag, "DL"))	// </DL> indicates end of sublevel branching
	{
		if(m_nCurParentIdx >= 0){
			int nParentID  = m_pDoc->GetNodeByIdx(m_nCurParentIdx).m_nParentID;
			m_nCurParentIdx = m_pDoc->GetIdxFromID(nParentID);
		}
		//m_nCurNodeID   = -1;
	}
	else if(0 == strcasecmp(szTag, "DT"))	//DT indicates new node title coming
	{
	}
	else if(0 == strcasecmp(szTag, "A"))	//A indicates hyperlink
	{
		if(std::string::npos != m_strLinkURL.find("name="))
			return; //this is a link anchor, not the real link, ignore

		if(m_nCurNodeIdx >= 0)
		{
			int nTargetNodeIdx = -1;
			if(m_strLinkURL.size() > 0)
			{
				if(m_strLinkURL.at(0) == '#')	//link to internal node
				{
					nTargetNodeIdx = atoi(m_strLinkURL.c_str()+1);
				}
				else if(m_strLinkURL.substr(0, 5) == "data:")	//attachment link
				{
					size_t nPos = m_strLinkURL.find("base64,");
					if(nPos != std::string::npos)
						m_strLinkData = m_strLinkURL.substr(nPos + strlen("base64,"));
				}
			}

			if(m_strLinkData.size()>0)	//attachment
			{
				//decode attachment
				int nBufSize = m_strLinkData.size() + 1;
				char *szDecode = new char[nBufSize];
				//TOFIX handle error

				CBase64 convert;
				int nSize = convert.Decode(m_strLinkData.c_str(), szDecode);

				//attachment
				AttInfo info;
				info.m_strData.append(szDecode, nSize);
				info.m_strName = m_strLinkTitle;
				info.nDataSize = info.m_strData.size();
				//TOFIX parse date

				delete [] szDecode;

				m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_lstAttachments.push_back(info);
			}
			else	//link
			{
				ASSERT(m_strLinkText.size() > 0);

				LinkInfo info;
				info.m_strText      = m_strLinkText;
				info.m_nStartOffset = m_nLinkStart;
				if(nTargetNodeIdx >= 0)
					info.m_nTargetNodeID = nTargetNodeIdx;	//will be converted from idx to id later
				else
					info.m_strTargetURL = m_strLinkURL;
				info.RefreshLength();

				if(DOC_PARSE_EXPECT_TITLE == m_nDocParseState)
					m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_objLink = info;
				else
					m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_lstLinks.push_back(info);
			}

			//clear internal data
			m_strLinkURL = "";
			m_strLinkText = "";
			m_strLinkData = "";
			m_strLinkTitle= "";
			m_nLinkStart = -1;
		}

		//m_nDocParseState = DOC_PARSE_EXPECT_TEXT; // more text?
	}
	else if(0 == strcasecmp(szTag, "B"))	//bold text ends
	{
		ASSERT(m_nCurNodeIdx >= 0);
		if(m_nCurNodeIdx >= 0)
		{
			FmtInfo info;
			info.nFormatTag = FMT_BOLD_END;
			info.nOffset = m_nPicCountInCurNode + g_utf8_strlen(m_pDoc->GetNodeByIdx(m_nCurNodeIdx).GetText().c_str(), -1);
			m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_lstTxtFmt.push_back(info);

			//TRACE("BOLD end at %d for node:%d\n", info.nOffset, m_nCurNodeIdx);
		}
	}
	else if(0 == strcasecmp(szTag, "I"))	//italic text ends
	{
		ASSERT(m_nCurNodeIdx >= 0);
		if(m_nCurNodeIdx >= 0)
		{
			FmtInfo info;
			info.nFormatTag = FMT_ITALIC_END;
			info.nOffset = m_nPicCountInCurNode + g_utf8_strlen(m_pDoc->GetNodeByIdx(m_nCurNodeIdx).GetText().c_str(), -1);
			m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_lstTxtFmt.push_back(info);
		}
	}
	else if(0 == strcasecmp(szTag, "U"))	//underlined text ends
	{
		ASSERT(m_nCurNodeIdx >= 0);
		if(m_nCurNodeIdx >= 0)
		{
			FmtInfo info;
			info.nFormatTag = FMT_UNDERLINE_END;
			info.nOffset = m_nPicCountInCurNode + g_utf8_strlen(m_pDoc->GetNodeByIdx(m_nCurNodeIdx).GetText().c_str(), -1);
			m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_lstTxtFmt.push_back(info);
		}
	}
	else if(0 == strcasecmp(szTag, "S"))	//strikethrough text ends
	{
		ASSERT(m_nCurNodeIdx >= 0);
		if(m_nCurNodeIdx >= 0)
		{
			FmtInfo info;
			info.nFormatTag = FMT_STRIKE_END;
			info.nOffset = m_nPicCountInCurNode + g_utf8_strlen(m_pDoc->GetNodeByIdx(m_nCurNodeIdx).GetText().c_str(), -1);
			m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_lstTxtFmt.push_back(info);

			//TRACE("STRIKE end at %d for node:%d\n", info.nOffset, m_nCurNodeIdx);
		}
	}
	else if(0 == strcasecmp(szTag, "font"))	//font color tag begins
	{
		ASSERT(m_nCurNodeIdx >= 0);
		if(m_nCurNodeIdx >= 0)
		{
			FmtInfo info;
			info.nFormatTag = FMT_TXT_COLOR_END;
			info.nOffset = m_nPicCountInCurNode + g_utf8_strlen(m_pDoc->GetNodeByIdx(m_nCurNodeIdx).GetText().c_str(), -1);
			m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_lstTxtFmt.push_back(info);
		}
	}
	else if(0 == strcasecmp(szTag, "span"))	//font bkg color tag ends
	{
		ASSERT(m_nCurNodeIdx >= 0);
		if(m_nCurNodeIdx >= 0)
		{
			//find out what of the two span based tags are we closing
			FmtInfoList &list = m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_lstTxtFmt;
			int nCnt = list.size()-1;

			int nPos1 = list.FindOppositeTag(FMT_TXT_BKG_COLOR_END, nCnt, false, true);
			int nPos2 = list.FindOppositeTag(FMT_TXT_COLOR_END, nCnt, false, true);

			if(nPos1 > nPos2)
			{
				FmtInfo info;
				info.nFormatTag = FMT_TXT_BKG_COLOR_END;
				info.nOffset = m_nPicCountInCurNode + g_utf8_strlen(m_pDoc->GetNodeByIdx(m_nCurNodeIdx).GetText().c_str(), -1);
				info.color = list[nPos1].color;
				m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_lstTxtFmt.push_back(info);
			}
			else if(nPos2 >= 0)
			{
				FmtInfo info;
				info.nFormatTag = FMT_TXT_COLOR_END;
				info.nOffset = m_nPicCountInCurNode + g_utf8_strlen(m_pDoc->GetNodeByIdx(m_nCurNodeIdx).GetText().c_str(), -1);
				info.color = list[nPos2].color;
				m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_lstTxtFmt.push_back(info);
			}
		}
	}
}

void FormatIO_HTML::OnComment(const char *szText)
{
	//TRACE("FormatIO_HTML:[%s] HTML comment\n", szText);

	//extract info from comments <!--LastNote:4-->
	if(0 == strncmp(szText, "LastNote:", strlen("LastNote:")))
	{
		m_pDoc->m_nCurrentNode = atoi(szText+strlen("LastNote:"));
	}
	else if (0 == strncmp(szText, "property:icon_file=", strlen("property:icon_file=")))
	{
		ASSERT(m_nCurNodeIdx >= 0);

		//example tag: "<!--property:icon_file=/root/aaa.xpm-->"
		//TRACE("icon_file->%s\n",szText);
		m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_nIconType = ICON_CUSTOM;
		m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_strIconFile = szText + strlen("property:icon_file=");

		//convert to absolute path if not already absolute
		std::string strAbsPath = m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_strIconFile;
		if(strAbsPath.size()>0){
			if(strAbsPath.at(0) != '/' && (strAbsPath.size()>1 && strAbsPath.at(1) != ':')){
				std::string strDocPath = m_pDoc->GetPath();
				strDocPath = GetParentDir(strDocPath.c_str());
				strAbsPath = strDocPath + strAbsPath;
			#ifdef _WIN32
				replaceall(strAbsPath, "/", "\\");
			#endif
				m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_strIconFile = strAbsPath;
			}
		}
	}
	else if (0 == strncmp(szText, "property:icon_internal=", strlen("property:icon_internal=")))
	{
		ASSERT(m_nCurNodeIdx >= 0);

		//TRACE("icon_internal->%s\n",szText);
		//example tag: "<!--property:icon_internal=blank-->"
		DocumentIterator it(*m_pDoc);
		int nIdx = InternalIcon_Name2Index(szText + strlen("property:icon_internal="));
		if(nIdx >= 0)
			m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_nIconType = nIdx;
	}
	else if (0 == strncmp(szText, "property:keywords=", strlen("property:keywords=")))
	{
		ASSERT(m_nCurNodeIdx >= 0);

		//get keywords and cleanup empty ones
		std::string strKeywords = szText + strlen("property:keywords=");
		std::string::size_type nPos = strKeywords.find(";;");
		while(std::string::npos != nPos){
			strKeywords.erase(nPos, 1);
			nPos = strKeywords.find(";;");
		}
		if(strKeywords == ";")
			strKeywords = "";

		//append keywords (multiple tags can exist)
		if(strKeywords.size()){
			if(m_pDoc->GetNodeByIdx(m_nCurNodeIdx).GetKeywords().size() > 0)
				m_pDoc->GetNodeByIdx(m_nCurNodeIdx).GetKeywords() += ";";
			m_pDoc->GetNodeByIdx(m_nCurNodeIdx).GetKeywords() += strKeywords;
		}
	}
	else if (0 == strncmp(szText, "property:date_created=", strlen("property:date_created=")))
	{
		ASSERT(m_nCurNodeIdx >= 0);

		std::string strDate = szText + strlen("property:date_created=");
		m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_nDateCreated = atol(strDate.c_str());
	}
	else if (0 == strncmp(szText, "property:date_modified=", strlen("property:date_modified=")))
	{
		ASSERT(m_nCurNodeIdx >= 0);

		std::string strDate = szText + strlen("property:date_modified=");
		m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_nDateModified = atol(strDate.c_str());
	}
	else if (0 == strncmp(szText, "property:finished=", strlen("property:finished=")))
	{
		ASSERT(m_nCurNodeIdx >= 0);

		std::string strDate = szText + strlen("property:finished=");
		m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_bFinished = (atoi(strDate.c_str()) != 0);
	}
	else if (0 == strncmp(szText, "property:read_only", strlen("property:read_only")))
	{
		if(m_nCurNodeIdx < 0)
			m_pDoc->SetReadOnly(true);
	}
	else if (0 == strncmp(szText, "property:expanded", strlen("property:expanded")))
	{
		ASSERT(m_nCurNodeIdx >= 0);

		if(m_nCurNodeIdx >= 0)
			m_pDoc->GetNodeByIdx(m_nCurNodeIdx).m_bExpanded = true;
	}
	else if (0 == strncmp(szText, "property:syntax_highlight=", strlen("property:syntax_highlight=")))
	{
		ASSERT(m_nCurNodeIdx >= 0);

		if(m_nCurNodeIdx >= 0){
			std::string strLang = szText + strlen("property:syntax_highlight=");
			HTMLParser::UnescapeChars(strLang);
			m_pDoc->GetNodeByIdx(m_nCurNodeIdx).GetSourceLanguage() = strLang;
		}
	}
}

void FormatIO_HTML::OnText(const char *szText)
{
	std::string data(szText);
	//TRACE("FormatIO_HTML:[%s] HTML text\n", data.c_str());

	//NoteCenter Text format: remove \r or \n from text start and text end
	if(0 == strncmp(data.c_str(), "\r", 1))
		data = data.substr(1, data.size()-1);
	else if(0 == strncmp(data.c_str(), "\n", 1))
		data = data.substr(1, data.size()-1);
	if(0 == strncmp(data.c_str()+data.size()-1, "\r", 1))
		data = data.substr(0, data.size()-1);
	else if(0 == strncmp(data.c_str()+data.size()-1, "\n", 1))
		data = data.substr(0, data.size()-1);

	if(data.size() == 0)
		return;

	//set text or title for current node
	if(DOC_PARSE_EXPECT_TITLE == m_nDocParseState)
	{
		replaceall(data, "\r", "");
		replaceall(data, "\n", "");

		m_strLinkText = data;	//in case we are having link in the title

		//TRACE("FormatIO_HTML:[%s] is node title \n", szText);
		if(!m_bNoteCenterMode)
			HTMLParser::UnescapeChars(data);

		//set node title
		ASSERT(m_nCurNodeIdx >= 0);
		if(m_nCurNodeIdx >= 0)
			m_pDoc->GetNodeByIdx(m_nCurNodeIdx).GetTitle() += data;

		//m_nDocParseState = DOC_PARSE_BLANK;
	}
	else if(DOC_PARSE_EXPECT_TEXT == m_nDocParseState)
	{
		//append node text
		if(m_nCurNodeIdx >= 0)
		{
			HTMLParser::UnescapeChars(data);	//only for node text (NC format)

			if(m_bBRTagUsed){
				//ignore new line chars inside node text when using <br> as line delimiter -> new format
				replaceall(data, "\r", "");
				replaceall(data, "\n", "");
			}
			//TRACE("FormatIO_HTML:[%s] is node text\n", data.c_str());

			m_pDoc->GetNodeByIdx(m_nCurNodeIdx).GetText() += data;
		}

		//m_nDocParseState = DOC_PARSE_BLANK;
	}
	else if(DOC_PARSE_INSIDE_LINK == m_nDocParseState)
	{
		HTMLParser::UnescapeChars(data);	//only for node text (NC format)
		m_strLinkText = data;

		//TRACE("FormatIO_HTML:[%s] is node text (inside link)\n", data.c_str());

		//append node text
		ASSERT(m_nCurNodeIdx >= 0);
		if(m_nCurNodeIdx >= 0){
			m_pDoc->GetNodeByIdx(m_nCurNodeIdx).GetText() += data;
		}
	}
}

void FormatIO_HTML::ConvertInternalLinks()
{
	//
	// convert recursive node index (as read from document file)
	// to the node ID value (as stored in memory, because ID is invariant)
	// -> this is done once after the file was loaded from file
	//
	DocumentIterator it(*m_pDoc);

	unsigned int nCount = m_pDoc->GetNodeCount();
	for(unsigned int i=0; i<nCount; i++)
	{
		LinkInfoList &links = m_pDoc->GetNodeByIdx(i).m_lstLinks;
		int nLinksCnt = links.size();
		for(int j=0; j<nLinksCnt; j++)
		{
			if(links[j].m_nTargetNodeID >= 0){
				int nIdx = it.RecursiveIdx2NodeIdx(links[j].m_nTargetNodeID);
				if(nIdx >= 0){
					links[j].m_nTargetNodeID = m_pDoc->GetNodeByIdx(nIdx).m_nID;

					//increment "linked" count for this ID
					std::map<int, int>::iterator it2 = m_pDoc->m_mapIdIsTarget.find(links[j].m_nTargetNodeID);
					if(it2 == m_pDoc->m_mapIdIsTarget.end())
						m_pDoc->m_mapIdIsTarget[links[j].m_nTargetNodeID] = 1;
					else
						m_pDoc->m_mapIdIsTarget[links[j].m_nTargetNodeID] ++;
				}
			}
		}
	}
}

void FormatIO_HTML::WriteLevel(DocumentIterator &it, IOProcess &file, int nParentID, bool bHtmlExport, bool bCustomCSS)
{
	std::string strData;
	int nSiblingIdx = 0;

	int nIdx = it.GetChildIdx(nParentID, 0);  //first child
	if(nIdx < 0)
		return;

	FormatIO_Base::RunLoop();	//enable progress painting events

	if(bHtmlExport && bCustomCSS)
	{
		//export with ID names based on the level, so that the nodes can be styled
		int nLevel = it.GetDocument().CalcNodeLevel(nIdx);
		char szBuffer[100];
		sprintf(szBuffer, "<DL id=\"Level%02d\">\n", nLevel+1);
		file.WriteString(szBuffer);
	}
	else
		file.WriteString("<DL>\n");

	while(nIdx >= 0)
	{
		NoteNode &node = it.GetDocument().GetNodeByIdx(nIdx);

		//find if current node is targeted by an internal link
		bool bTarget = false;
		if(bHtmlExport && g_bUseTOC && g_nTocDepth > 0){
			int nDepth = it.CalcNodeLevel(nIdx);
			if(nDepth+1 <= g_nTocDepth)
				bTarget = true;
		}
		if(!bTarget){
			std::map<int, int>::iterator it2 = it.GetDocument().m_mapIdIsTarget.find(node.m_nID);
			if(it2 != it.GetDocument().m_mapIdIsTarget.end())
				bTarget = ((*it2).second > 0);
		}

		//write node title
		strData = node.GetTitle();
		//TOFIX if(!m_bNoteCenterMode)
			HTMLParser::EscapeChars(strData);

		//support for multiline node titles
		replaceall(strData, "\r\n", "\r");
		replaceall(strData, "\n", "\r");
		replaceall(strData, "\r", "<BR>\n");

		if(bHtmlExport && !bCustomCSS)
			file.WriteString("<DT style=\"font-weight: bold;\">");
		else
			file.WriteString("<DT>");

		// if the node is a link target assign the name to it
		if(bTarget){
			int nRecursiveIdx = it.NodeIdx2RecursiveIdx(nIdx);
			char szBuffer[100];
			sprintf(szBuffer, "<A name=\"%d\"></A>", nRecursiveIdx);
			file.WriteString(szBuffer);
		}

		// if the node is a link (source)
		bool bLink = node.m_objLink.IsValid();
		if(bLink){
			std::string strLink("<A href=\"");

			//check if internal link became invalid (exporting single node or similar)
			bool bLinkOK = true;
			if( node.m_objLink.m_strTargetURL.empty() &&
				node.m_objLink.m_nTargetNodeID >= 0)
			{
				int nIdx = it.GetDocument().GetIdxFromID(node.m_objLink.m_nTargetNodeID);
				if(nIdx < 0)
					bLinkOK = false;
			}
			if(m_bHtmlExport && !bLinkOK)
			{
				strLink += "javascript:;\" onclick=\"LinkWarning()";
			}
			else
			{
				char szBuffer[20];
				if(node.m_objLink.m_nTargetNodeID >= 0){
					//internal link is stored as recursive node index
					int nIdx = it.GetDocument().GetIdxFromID(node.m_objLink.m_nTargetNodeID);
					sprintf(szBuffer, "#%d", it.NodeIdx2RecursiveIdx(nIdx));
					strLink += szBuffer;
				}
				else{
					std::string strLinkURI = node.m_objLink.m_strTargetURL;
					HTMLParser::EscapeURI(strLinkURI);
					strLink += strLinkURI;
				}
			}

			strLink += "\">";
			file.WriteString(strLink.c_str());
		}

		file.WriteString(strData.c_str());

		if(bLink){
			file.WriteString("</A>");
		}
		file.WriteString("</DT>\n");

		//write node parameters
		if(!bHtmlExport)
		{
			//write node icon setting
			if(ICON_NONE != node.m_nIconType)
			{
				if(ICON_CUSTOM == node.m_nIconType)
				{
					//convert from absolute to relative path (if simple match)
					std::string strDocPath = m_pDoc->GetPath();
					strDocPath = GetParentDir(strDocPath.c_str());
					std::string strRelPath = node.m_strIconFile;
					if(0 == strcasecmp(strRelPath.substr(0, strDocPath.size()).c_str(), strDocPath.c_str())){
						strRelPath = strRelPath.substr(strDocPath.size());
						replaceall(strRelPath, "\\", "/");
					}

					file.WriteString("<!--property:icon_file=");
					file.WriteString(strRelPath.c_str());
					file.WriteString("-->\n");
				}
				else
				{
					file.WriteString("<!--property:icon_internal=");
					file.WriteString(InternalIcon_Index2Name(node.m_nIconType));
					file.WriteString("-->\n");
				}
			}

			//write node keywords
			std::string strKeywords = node.GetKeywords();
			if(strKeywords.size()>0)
			{
				file.WriteString("<!--property:keywords=");
				file.WriteString(strKeywords.c_str());
				file.WriteString("-->\n");
			}

			//write note syntax highlighting language
			if(!node.GetSourceLanguage().empty()){
				std::string strLang = node.GetSourceLanguage();
				HTMLParser::EscapeChars(strLang);
				file.WriteString("<!--property:syntax_highlight=");
				file.WriteString(strLang.c_str());
				file.WriteString("-->\n");
			}

			//write node dates
			char szBuffer[20];
			snprintf(szBuffer, sizeof(szBuffer), "%d", (int)node.m_nDateCreated);//TOFIX? (long)
			file.WriteString("<!--property:date_created=");
			file.WriteString(szBuffer);
			file.WriteString("-->\n");

			snprintf(szBuffer, sizeof(szBuffer), "%d", (int)node.m_nDateModified); //TOFIX? (long)
			file.WriteString("<!--property:date_modified=");
			file.WriteString(szBuffer);
			file.WriteString("-->\n");

			if(node.m_bFinished){	// save only when set (TOFIX can remove =1 in the future)
				snprintf(szBuffer, sizeof(szBuffer), "%d", node.m_bFinished);
				file.WriteString("<!--property:finished=");
				file.WriteString(szBuffer);
				file.WriteString("-->\n");
			}

			if(node.m_bExpanded)	// save only when set
				file.WriteString("<!--property:expanded-->\n");
		}

		//
		// write node attachments
		//
		int nAttCount = node.m_lstAttachments.size();
		int i;
		for(i=0; i<nAttCount; i++)
		{
			std::string strTitle = node.m_lstAttachments[i].m_strName;
			HTMLParser::EscapeChars(strTitle);

			file.WriteString("<A title=\"");
			file.WriteString(strTitle.c_str());

			if(bHtmlExport && g_bExportDataAsSeparateFile)
			{
				//save attachment as file
				m_nExportAttCount ++;
				std::string strDir = GetParentDir(m_strFile.c_str());
				char szBuffer[1024];
				char szLocName[1024];

				if(node.m_lstAttachments[i].m_strName.size() > 0)
				{
					//use original name if it exists
					sprintf(szBuffer, "%s%s", strDir.c_str(), node.m_lstAttachments[i].m_strName.c_str());
					sprintf(szLocName, "%s", node.m_lstAttachments[i].m_strName.c_str());

				}
				else{
					sprintf(szBuffer, "%sAttachment%02d", strDir.c_str(), m_nExportAttCount);
					sprintf(szLocName, "Attachment%02d", m_nExportAttCount);
				}

				std::string strHref(szLocName);

				bool bWriteFile = true;
				if(!m_bExportOverwrite && 0 == access(szBuffer, 00)){
					char szMsg[1024];
					sprintf(szMsg, _("File %s\nalready exists. Overwrite this and all other files?"), szBuffer);
					if(GTK_RESPONSE_YES != gtkMessageBox(szMsg, GTK_BUTTONS_YES_NO))
						bWriteFile = false;
					else
						m_bExportOverwrite = true;
				}

				if(bWriteFile)
				{
					FILE *pOut = fopen(szBuffer, "wb");
					if(pOut){
						int nSize = node.m_lstAttachments[i].nDataSize;
						const char *szData = node.m_lstAttachments[i].m_strData.c_str();

						int nPos = 0;
						int nChunkSize = 2048;
						int nLoopCnt = 0;
						size_t nWritten = 0;

						while(nPos < nSize-1){
							int nSaveStep  = min(nChunkSize, nSize-nPos);
							nWritten = fwrite(szData+nPos, 1, nSaveStep, pOut);
							nPos += nSaveStep;
							nLoopCnt++;

							if(nLoopCnt%10 == 0)
								FormatIO_Base::RunLoop();
						}

						fclose(pOut);
					}
				}

				//write link to file
				file.WriteString("\" href=\"");
				file.WriteString(strHref.c_str());
			}
			else
			{
				file.WriteString("\" href=\"data:application/octet-stream;base64,");

				//write data as base64 encoded
				CBase64 convert;
				file.WriteString(convert.Encode(node.m_lstAttachments[i].m_strData.c_str(),
												node.m_lstAttachments[i].nDataSize).c_str());
			}

			file.WriteString("\">");
			if(bHtmlExport)
			{
				//write link text like "Attachment#75"
				file.WriteString(_("Attachment"));
				char szBuffer[20];
				sprintf(szBuffer, " #%02d", m_nExportAttCount);
				file.WriteString(szBuffer);
			}
			file.WriteString("</A>\n");
		}

		//
		// write node text
		//
		strData = node.GetText();

		std::vector<tSegment> lstSegments;
		if(strData.size() > 0){
			tSegment info;
			info.strText = strData;
			info.bIsText = true;

			lstSegments.push_back(info);
		}

		//before escaping, modify contents to include all node links
		//<A href="http://belle.monitor.hr/0510.htm">Belle de Jour</A>
		int nLinks = node.m_lstLinks.size();
		if(nLinks > 0)
		{
			//sort links by offset in ascending order
			node.m_lstLinks.Sort();

			//break node text into segments and then insert the links tags to the right places
			for(i=0; i<nLinks; i++)
			{
				LinkInfo &link = node.m_lstLinks[i];

				//
				// keep breaking text within the segments list
				//

				//find segment for given offset (ignore link segments)
				int nSegmentIdx = -1;
				int nOffset = link.m_nStartOffset;
				int nOffsetTot = 0;
				for(unsigned int j=0; j<lstSegments.size(); j++)
				{
					if(!lstSegments[j].bIsText)
						continue;	//skip tags

					std::string &strTmp = lstSegments[j].strText;
					int nSize = g_utf8_strlen(strTmp.c_str(), -1);
					if(nOffsetTot + nSize > nOffset){
						nSegmentIdx = j;
						break;
					}
					nOffsetTot += nSize; //keep looking
				}

				//now break the segment into subsegments
				if(nSegmentIdx >= 0)
				{
					std::string strTmp = lstSegments[nSegmentIdx].strText;

					//convert character offset to the buffer offset
					int nPosBuf;
					const char *szBuf = strTmp.c_str();
					gchar* pszPos = g_utf8_offset_to_pointer(szBuf, link.m_nStartOffset-nOffsetTot);
					nPosBuf = pszPos - szBuf;
					ASSERT(nPosBuf >= 0);

					//erase original segment
					bool bText = lstSegments[nSegmentIdx].bIsText;
					lstSegments.erase(lstSegments.begin()+nSegmentIdx);

					//replace it with up to 3 new segments (content before,link,content after)
					if(nPosBuf>0)
					{
						tSegment info;
						info.strText = strTmp.substr(0, nPosBuf);
						info.bIsText = bText;

						lstSegments.insert(lstSegments.begin()+nSegmentIdx, info);
						nSegmentIdx++;
					}

					bool bSkipLink = false;
					if( bHtmlExport &&
						g_bConvertImageLinks &&
						link.m_nTargetNodeID < 0)
					{
						std::string strExt = link.m_strTargetURL;
						std::string::size_type nPos = strExt.find_last_of(".");
						if(nPos != std::string::npos)
							strExt = strExt.substr(nPos);

						//if the linked file is an image
						if( strExt.size() > 0 &&
							(0 == strcasecmp(strExt.c_str(), ".JPG")  ||
							 0 == strcasecmp(strExt.c_str(), ".JPEG") ||
							 0 == strcasecmp(strExt.c_str(), ".PNG")  ||
							 0 == strcasecmp(strExt.c_str(), ".GIF")))
						{
							std::string strLink;

							//save link as an image
							strLink += "<img alt=\"\" src=\"";
							strLink += link.m_strTargetURL;
							strLink += "\">";

							tSegment info;
							info.strText = strLink;
							info.bIsText = false;

							lstSegments.push_back(info);
							bSkipLink = true;
						}
					}
					if(!bSkipLink)
					{
						std::string strLink = "<A href=\"";

						//check if internal link became invalid (exporting single node or similar)
						bool bLinkOK = true;
						if( link.m_strTargetURL.empty() &&
							link.m_nTargetNodeID >= 0)
						{
							int nIdx = it.GetDocument().GetIdxFromID(link.m_nTargetNodeID);
							if(nIdx < 0)
								bLinkOK = false;
						}
						if(m_bHtmlExport && !bLinkOK)
						{
							strLink += "javascript:;\" onclick=\"LinkWarning()";
						}
						else
						{
							//link segment
							char szBuffer[20];
							if(link.m_nTargetNodeID >= 0){
								//internal link is stored as recursive node index
								int nIdx = it.GetDocument().GetIdxFromID(link.m_nTargetNodeID);
								sprintf(szBuffer, "#%d", it.NodeIdx2RecursiveIdx(nIdx));
								strLink += szBuffer;
							}
							else{
								std::string strLinkURI = link.m_strTargetURL;
								HTMLParser::EscapeURI(strLinkURI);
								strLink += strLinkURI;
							}
						}
						strLink += "\">";

						tSegment info;
						info.strText = strLink;
						info.bIsText = false;
						lstSegments.insert(lstSegments.begin()+nSegmentIdx, info);
						nSegmentIdx++;

						info.strText = link.m_strText;
						info.bIsText = true;
						lstSegments.insert(lstSegments.begin()+nSegmentIdx, info);
						nSegmentIdx++;

						info.strText = "</A>";
						info.bIsText = false;
						lstSegments.insert(lstSegments.begin()+nSegmentIdx, info);
						nSegmentIdx++;
					}

					//remaining data
					strTmp = strTmp.substr(nPosBuf+link.m_strText.size());
					if(strTmp.size() > 0){
						tSegment info;
						info.strText = strTmp;
						info.bIsText = bText;
						lstSegments.insert(lstSegments.begin()+nSegmentIdx, info);
					}
				}
			}
		}

		//insert additional picture segments
		int nPicsCount = node.m_lstPictures.size();
		if(nPicsCount > 0)
		{
			//break node text into segments and then insert the links tags to the right places
			int i;
			for(i=0; i<nPicsCount; i++)
			{
				PixInfo &pix = node.m_lstPictures[i];

				//
				// keep breaking text within the segments list
				//

				int nPicsBeforeThis = i;

				//find segment for given offset (ignore link segments)
				int nSegmentIdx = -1;
				int nOffset = pix.nOffset;
				int nOffsetTot = 0;
				for(unsigned int j=0; j<lstSegments.size(); j++)
				{
					if(!lstSegments[j].bIsText)
						continue;	//skip tags

					std::string &strTmp = lstSegments[j].strText;
					int nSize = g_utf8_strlen(strTmp.c_str(), -1);
					if(nOffsetTot + nSize + nPicsBeforeThis > nOffset){
						nSegmentIdx = j;
						break;
					}
					nOffsetTot += nSize; //keep looking
				}

				//now break the segment into subsegments
				if(nSegmentIdx >= 0)
				{
					std::string strTmp = lstSegments[nSegmentIdx].strText;

					//convert character offset to the buffer offset
					int nPosBuf;
					const char *szBuf = strTmp.c_str();
					gchar* pszPos = g_utf8_offset_to_pointer(szBuf, pix.nOffset-nOffsetTot-nPicsBeforeThis);
					nPosBuf = pszPos - szBuf;
					ASSERT(nPosBuf >= 0);

					//erase original segment
					bool bText = lstSegments[nSegmentIdx].bIsText;
					lstSegments.erase(lstSegments.begin()+nSegmentIdx);

					//replace it with up to 3 new segments (content before,link,content after)
					if(nPosBuf>0){
						tSegment info;
						info.strText  = strTmp.substr(0, nPosBuf);
						info.bIsText = bText;
						lstSegments.insert(lstSegments.begin()+nSegmentIdx, info);
						nSegmentIdx++;
					}

					//picture segment
					std::string strPixData;
					if(bHtmlExport && g_bExportDataAsSeparateFile)
					{
						//save picture as separate file
						m_nExportImgCount ++;
						std::string strDir = GetParentDir(m_strFile.c_str());
						char szPath[1024];
						char szBuffer[200];

						if(pix.strName.size() > 0)
						{
							//use original name if it exists
							sprintf(szPath, "%s%s", strDir.c_str(), pix.strName.c_str());
							snprintf(szBuffer, sizeof(szBuffer), "%s", pix.strName.c_str());
						}
						else{
							if(pix.bUsePNG)
								sprintf(szBuffer, "Image%02d.png", m_nExportImgCount);
							else
								sprintf(szBuffer, "Image%02d.jpg", m_nExportImgCount);
							sprintf(szPath, "%s%s", strDir.c_str(), szBuffer);
						}

						bool bWriteFile = true;
						if(!m_bExportOverwrite && 0 == access(szPath, 00)){
							char szMsg[2024];
							sprintf(szMsg, _("File %s\nalready exists. Overwrite this and all other files?"), szPath);
							if(GTK_RESPONSE_YES != gtkMessageBox(szMsg, GTK_BUTTONS_YES_NO))
								bWriteFile = false;
							else
								m_bExportOverwrite = true;
						}

						if(bWriteFile){
							#if GTK_CHECK_VERSION(2,8,0) //new "compression" param TOFIX set proper version
								gdk_pixbuf_save(pix.pixbuf, szPath, pix.bUsePNG? "png" : "jpeg", NULL, pix.bUsePNG? "compression" : NULL,  pix.bUsePNG? "9" : NULL, (char *)NULL);	//TOFIX check error
							#else
								gdk_pixbuf_save(pix.pixbuf, szPath, pix.bUsePNG? "png" : "jpeg", NULL, (char *)NULL);	//TOFIX check error
							#endif
						}

						strPixData  = "<img alt=\"\" title=\"";
						strPixData += pix.strName;
						strPixData += "\" src=\"";
						strPixData += szBuffer;
						strPixData += "\">";
					}
					else
					{
						gchar *buffer = NULL;
						gsize buffer_size = 0;
					#if GTK_CHECK_VERSION(2,8,0) //new "compression" param TOFIX set proper version
						if(!gdk_pixbuf_save_to_buffer(pix.pixbuf, &buffer, &buffer_size, pix.bUsePNG? "png" : "jpeg", NULL, pix.bUsePNG? "compression" : NULL,  pix.bUsePNG? "9" : NULL, (char *)NULL)){
					#else
						if(!gdk_pixbuf_save_to_buffer(pix.pixbuf, &buffer, &buffer_size, pix.bUsePNG? "png" : "jpeg", NULL, (char *)NULL)){
					#endif
							//TOFIX handle error
							ASSERT(false);
						}

						//convert to base64 and save
						if(buffer){
							CBase64 convert;
							strPixData += "<img title=\"";
							strPixData += pix.strName;
							strPixData += pix.bUsePNG? "\" src=\"data:image/png;base64," : "\" src=\"data:image/jpeg;base64,";
							strPixData += convert.Encode(buffer, buffer_size);
							strPixData += "\">";

							g_free(buffer);	//TOFIX unref?
						}
					}

					tSegment info;
					info.strText = strPixData;
					info.bIsText = false;
					lstSegments.insert(lstSegments.begin()+nSegmentIdx, info);
					nSegmentIdx++;

					//remaining data
					strTmp = strTmp.substr(nPosBuf);
					if(strTmp.size() > 0){
						tSegment info;
						info.strText = strTmp;
						info.bIsText = bText;
						lstSegments.insert(lstSegments.begin()+nSegmentIdx, info);
					}
				}
				else	//picture is at the end, insert new picture segment
				{
					std::string strPixData;
					if(bHtmlExport && g_bExportDataAsSeparateFile)
					{
						//save picture as separate file
						m_nExportImgCount ++;
						std::string strDir = GetParentDir(m_strFile.c_str());
						char szPath[1024];
						char szBuffer[200];

						if(pix.strName.size() > 0)
						{
							//use original name if it exists
							sprintf(szPath, "%s%s", strDir.c_str(), pix.strName.c_str());
							snprintf(szBuffer, sizeof(szBuffer), "%s", pix.strName.c_str());
						}
						else{
							sprintf(szBuffer, "Image%02d.png", m_nExportImgCount);
							sprintf(szPath, "%s%s", strDir.c_str(), szBuffer);
						}

						bool bWriteFile = true;
						if(!m_bExportOverwrite && 0 == access(szPath, 00)){
							char szMsg[2024];
							sprintf(szMsg, _("File %s\nalready exists. Overwrite this and all other files?"), szPath);
							if(GTK_RESPONSE_YES != gtkMessageBox(szMsg, GTK_BUTTONS_YES_NO))
								bWriteFile = false;
							else
								m_bExportOverwrite = true;
						}

						if(bWriteFile){
							#if GTK_CHECK_VERSION(2,8,0) //new "compression" param TOFIX set proper version
								gdk_pixbuf_save(pix.pixbuf, szPath, pix.bUsePNG? "png" : "jpeg", NULL, pix.bUsePNG? "compression" : NULL,  pix.bUsePNG? "9" : NULL, (char *)NULL);	//TOFIX check error
							#else
								gdk_pixbuf_save(pix.pixbuf, szPath, pix.bUsePNG? "png" : "jpeg", NULL, (char *)NULL);	//TOFIX check error
							#endif
						}

						strPixData  = "<img alt=\"\" src=\"";
						strPixData += szBuffer;
						strPixData += "\">";
					}
					else
					{
						gchar *buffer = NULL;
						gsize buffer_size = 0;
					#if GTK_CHECK_VERSION(2,8,0) //new "compression" param TOFIX set proper version
						if(!gdk_pixbuf_save_to_buffer(pix.pixbuf, &buffer, &buffer_size, pix.bUsePNG? "png" : "jpeg", NULL, pix.bUsePNG? "compression" : NULL,  pix.bUsePNG? "9" : NULL, (char *)NULL)){
					#else
						if(!gdk_pixbuf_save_to_buffer(pix.pixbuf, &buffer, &buffer_size, pix.bUsePNG? "png" : "jpeg", NULL, (char *)NULL)){
					#endif
							//TOFIX handle error
							ASSERT(false);
						}

						//convert to base64 and save
						if(buffer){
							CBase64 convert;
							strPixData += "<img title=\"";
							strPixData += pix.strName;
							strPixData += pix.bUsePNG? "\" src=\"data:image/png;base64," : "\" src=\"data:image/jpeg;base64,";
							strPixData += convert.Encode(buffer, buffer_size);
							strPixData += "\">";

							g_free(buffer);	//TOFIX unref?
						}
					}

					tSegment info;
					info.strText = strPixData;
					info.bIsText = false;
					lstSegments.push_back(info);
					nSegmentIdx++;
				}
			}
		}

		//insert additional formatting segments
		int nFormattingCount = node.m_lstTxtFmt.size();
		if(nFormattingCount > 0)
		{
			//break node text into segments and then insert the links tags to the right places
			int i;
			for(i=0; i<nFormattingCount; i++)
			{
				FmtInfo &fmt = node.m_lstTxtFmt[i];

				int nPicsBeforeThis = 0;
				for(int k=0; k<nPicsCount; k++)
					if(node.m_lstPictures[k].nOffset < fmt.nOffset)
						nPicsBeforeThis ++;
					else
						break;

				//
				// keep breaking text within the segments list
				//

				//find segment for given offset (ignore link segments)
				int nSegmentIdx = -1;
				int nOffset = fmt.nOffset;
				int nOffsetTot = 0;
				for(unsigned int j=0; j<lstSegments.size(); j++)
				{
					if(!lstSegments[j].bIsText)
						continue;	//skip tags

					std::string &strTmp = lstSegments[j].strText;
					int nSize = g_utf8_strlen(strTmp.c_str(), -1);
					if(nOffsetTot + nSize + nPicsBeforeThis > nOffset){
						nSegmentIdx = j;
						break;
					}
					nOffsetTot += nSize; //keep looking
				}

				//now break the segment into subsegments
				if(nSegmentIdx >= 0)
				{
					std::string strTmp = lstSegments[nSegmentIdx].strText;

					//convert character offset to the buffer offset
					int nPosBuf;
					const char *szBuf = strTmp.c_str();
					gchar* pszPos = g_utf8_offset_to_pointer(szBuf, fmt.nOffset-nOffsetTot-nPicsBeforeThis);
					nPosBuf = pszPos - szBuf;
					ASSERT(nPosBuf >= 0);

					//erase original segment
					bool bText = lstSegments[nSegmentIdx].bIsText;
					lstSegments.erase(lstSegments.begin()+nSegmentIdx);

					//replace it with up to 3 new segments (content before,link,content after)
					if(nPosBuf>0){
						tSegment info;
						info.strText  = strTmp.substr(0, nPosBuf);
						info.bIsText = bText;
						lstSegments.insert(lstSegments.begin()+nSegmentIdx, info);
						nSegmentIdx++;
					}

					//formatting segment
					std::string strFmtData;
					if(FMT_BOLD_BEGIN == fmt.nFormatTag)
						strFmtData += "<b>";
					else if(FMT_BOLD_END == fmt.nFormatTag)
						strFmtData += "</b>";
					else if(FMT_ITALIC_BEGIN == fmt.nFormatTag)
						strFmtData += "<i>";
					else if(FMT_ITALIC_END == fmt.nFormatTag)
						strFmtData += "</i>";
					else if(FMT_UNDERLINE_BEGIN == fmt.nFormatTag)
						strFmtData += "<u>";
					else if(FMT_UNDERLINE_END == fmt.nFormatTag)
						strFmtData += "</u>";
					else if(FMT_STRIKE_BEGIN == fmt.nFormatTag)
						strFmtData += "<s>";
					else if(FMT_STRIKE_END == fmt.nFormatTag)
						strFmtData += "</s>";
					else if(FMT_TXT_COLOR_BEGIN == fmt.nFormatTag){
						char szBuffer[200];
						//sprintf(szBuffer, "<font color=\"#%02x%02x%02x\">", fmt.color.red/256, fmt.color.green/256, fmt.color.blue/256);
						sprintf(szBuffer, "<span style=\"color:#%02x%02x%02x\">", fmt.color.red/256, fmt.color.green/256, fmt.color.blue/256);
						strFmtData += szBuffer;
					}
					else if(FMT_TXT_COLOR_END == fmt.nFormatTag)
						strFmtData += "</span>";
					else if (FMT_TXT_BKG_COLOR_BEGIN == fmt.nFormatTag){
						char szBuffer[200];
						sprintf(szBuffer, "<span style=\"background-color:#%02x%02x%02x\">", fmt.color.red/256, fmt.color.green/256, fmt.color.blue/256);
						strFmtData += szBuffer;
					}
					else if (FMT_TXT_BKG_COLOR_END == fmt.nFormatTag)
						strFmtData += "</span>";

					//TOFIX more tags

					tSegment info;
					info.strText = strFmtData;
					info.bIsText = false;
					lstSegments.insert(lstSegments.begin()+nSegmentIdx, info);
					nSegmentIdx++;

					//remaining data
					strTmp = strTmp.substr(nPosBuf);
					if(strTmp.size() > 0){
						tSegment info;
						info.strText = strTmp;
						info.bIsText = bText;
						lstSegments.insert(lstSegments.begin()+nSegmentIdx, info);
					}
				}
				else	//picture is at the end, insert new picture segment
				{
					//formatting segment
					std::string strFmtData;
					if(FMT_BOLD_BEGIN == fmt.nFormatTag)
						strFmtData += "<b>";
					else if(FMT_BOLD_END == fmt.nFormatTag)
						strFmtData += "</b>";
					else if(FMT_ITALIC_BEGIN == fmt.nFormatTag)
						strFmtData += "<i>";
					else if(FMT_ITALIC_END == fmt.nFormatTag)
						strFmtData += "</i>";
					else if(FMT_UNDERLINE_BEGIN == fmt.nFormatTag)
						strFmtData += "<u>";
					else if(FMT_UNDERLINE_END == fmt.nFormatTag)
						strFmtData += "</u>";
					else if(FMT_STRIKE_BEGIN == fmt.nFormatTag)
						strFmtData += "<s>";
					else if(FMT_STRIKE_END == fmt.nFormatTag)
						strFmtData += "</s>";
					else if(FMT_TXT_COLOR_BEGIN == fmt.nFormatTag){
						char szBuffer[200];
						//sprintf(szBuffer, "<font color=\"#%02x%02x%02x\">", fmt.color.red/256, fmt.color.green/256, fmt.color.blue/256);
						sprintf(szBuffer, "<span style=\"color:#%02x%02x%02x\">", fmt.color.red/256, fmt.color.green/256, fmt.color.blue/256);
						strFmtData += szBuffer;
					}
					else if(FMT_TXT_COLOR_END == fmt.nFormatTag)
						strFmtData += "</span>";
					else if (FMT_TXT_BKG_COLOR_BEGIN == fmt.nFormatTag){
						char szBuffer[200];
						sprintf(szBuffer, "<span style=\"background-color:#%02x%02x%02x\">", fmt.color.red/256, fmt.color.green/256, fmt.color.blue/256);
						strFmtData += szBuffer;
					}
					else if (FMT_TXT_BKG_COLOR_END == fmt.nFormatTag)
						strFmtData += "</span>";
					//TOFIX more tags

					tSegment info;
					info.strText = strFmtData;
					info.bIsText = false;
					lstSegments.push_back(info);
					nSegmentIdx++;
				}
			}
		}

		//escape segments (skipping link tags) and concatenate back to single string
		strData = "";
		unsigned int nMax = lstSegments.size();
		for(unsigned int k=0; k<nMax; k++)
		{
			std::string &strTmp = lstSegments[k].strText;
			ASSERT(strTmp.size() > 0);
			if(lstSegments[k].bIsText)	//skip link tags TOFIX what if < is in the text?
			{
				HTMLParser::EscapeChars(strTmp);

				//replace "\r\n","\n" with HTML tag
				replaceall(strTmp, "\r\n", "\r");
				replaceall(strTmp, "\n", "\r");
				replaceall(strTmp, "\r", "<BR>\n");

				if(bHtmlExport){
					//convert spaces to non-breaking space
					//replaceall(strTmp, " ", "&nbsp;");
					replaceall(strTmp, "\t", g_strTabReplacement.c_str());
				}
			}
			strData += strTmp;
		}

		//do not write empty <DD></DD>
		int nChildren = it.GetChildCount(node.m_nID);
		if(nChildren > 0 || strData.size() > 0)
		{
			file.WriteString("<DD>\n");
			file.WriteString(strData.c_str());

			//recursively write children of this node
			WriteLevel(it, file, node.m_nID, bHtmlExport, bCustomCSS);

			file.WriteString("\n</DD>\n");
		}

		//go to the next sibling node
		nSiblingIdx ++;
		nIdx = it.GetChildIdx(nParentID, nSiblingIdx);
	}

	//
	file.WriteString("</DL>\n");
}

void FormatIO_HTML::WriteTOC(DocumentIterator &it, IOProcess &file, int nParentID)
{
	int nIdx = it.GetChildIdx(nParentID, 0);  //first child
	if(nIdx < 0)
		return;

	//check for recursion termination
	int nDepth = it.CalcNodeLevel(nIdx);
	if(nDepth+1 > g_nTocDepth)
		return;

	FormatIO_Base::RunLoop();	//enable progress painting events

	int nSiblingIdx = 0;

	if(0 == nDepth){
		file.WriteString("<hr></hr>");	// horiz. line
		file.WriteString("<b>");
		file.WriteString(_("Contents"));
		file.WriteString("</b>\n<br>\n");
	}

	//if(nDepth > 0)
		file.WriteString("<ul>\n");

	while(nIdx >= 0)
	{
		NoteNode &node = it.GetDocument().GetNodeByIdx(nIdx);

		//if(nDepth > 0)
			file.WriteString("<li>\n");

		//write link to the node
		file.WriteString("<A href=\"");
		char szBuffer[30];
		sprintf(szBuffer, "#%d", it.NodeIdx2RecursiveIdx(nIdx));
		file.WriteString(szBuffer);
		file.WriteString("\">");

		//link text
		std::string strData = node.GetTitle();
		HTMLParser::EscapeChars(strData);
		file.WriteString(strData.c_str());

		file.WriteString("</A>\n");

		//recursively write children of this node
		if(nDepth+2 <= g_nTocDepth)
			WriteTOC(it, file, node.m_nID);

		//if(nDepth > 0)
			file.WriteString("</li>\n");

		//go to the next sibling node
		nSiblingIdx ++;
		nIdx = it.GetChildIdx(nParentID, nSiblingIdx);
	}

	//if(nDepth > 0)
		file.WriteString("</ul>\n");

	if(0 == nDepth){
		file.WriteString("<hr></hr>");	// horiz. line
	}
}

void replaceall(std::string &strData, const char *szFind, const char *szReplace)
{
	//strData.replace(strData.begin(), strData.end(), szFind, szReplace);

	int nSrcSize = strlen(szFind);
	if(nSrcSize > 0)
	{
		int nDstSize = strlen(szReplace);

		size_t nPos = strData.find(szFind);
		while(nPos != std::string::npos)
		{
			//strData.replace(nPos, nSrcSize, szReplace);
			strData.erase(nPos, nSrcSize);
			strData.insert(nPos, szReplace);

			nPos = strData.find(szFind, nPos+nDstSize); //keep looking forward
		}
	}
}
