////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements MM/LX Mindmap/Outline document format I/O (reading only)
////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
  #pragma warning(disable: 4996)	//ignore deprecated functions warning
#endif

#include "FormatIOMMLX.h"
#include "DocumentIterator.h"
#include "debug.h"
#include "../config.h"
#include "../support.h"
#include <stdio.h>	//fopen
#include <glib.h>
#include <gtk/gtk.h>
#include <string.h>

#ifdef _WIN32
 #include <io.h>
 #ifndef __MINGW32__
  #define strcasecmp stricmp
 #endif
#endif

void replaceall(std::string &strData, const char *szFind, const char *szReplace);
int gtkMessageBox(const char *szText, int nButtons = GTK_BUTTONS_OK, int nIcon = GTK_MESSAGE_INFO);

FormatIO_MMLX::FormatIO_MMLX()
{
	m_nCurParentID = -1;	//TOFIX use Idx for faster operation
	m_nCurNodeID = -1;
	m_nLastNodeLevel = -1;
	m_nCurTextLine = 0;
	m_nCoversionFailuresCnt = 0;
}

FormatIO_MMLX::~FormatIO_MMLX()
{
}

int FormatIO_MMLX::Load(const char *szFile, NoteDocument &doc)
{
	m_pDoc = &doc;  //store pointer

	//clear old document contents
	doc.Clear();

	FILE *pInFile = fopen(szFile, "r");
	if(NULL == pInFile)
		return DOC_LOAD_NOT_FOUND;

	m_nCurTextLine = 0;

	char szBuffer[1024];

	//TOFIX error handling
	while(NULL != fgets(szBuffer, sizeof(szBuffer), pInFile))
	{
		RunLoop();	//enable progress painting events

		//kill new line char(s)
		if('\n'== szBuffer[strlen(szBuffer)-1])
			szBuffer[strlen(szBuffer)-1] = '\0';
		if('\r'== szBuffer[strlen(szBuffer)-1])
			szBuffer[strlen(szBuffer)-1] = '\0';

		m_nCurTextLine ++;
		ParseLine(szBuffer);
	}
	fclose(pInFile);

#ifdef _ZAURUS_BUILD
	if(m_nCoversionFailuresCnt > 0)
		gtkMessageBox(_("There were problems converting text from ibm850 to utf-8.\nConsider installing \"glibc-gconv-ibm850\" package"));
#endif

	return DOC_LOAD_OK;
}

int FormatIO_MMLX::Save(const char *szFile, NoteDocument &doc)
{
	return DOC_SAVE_NOT_SUPPORTED;	//export not implemented
}

void FormatIO_MMLX::ParseLine(char *szBuffer)
{
	int nLen = (szBuffer) ? strlen(szBuffer) : 0;

	bool bIsText = false;
	if(0 == nLen || (nLen > 0 && szBuffer[0] != ' ')){
		bIsText = true;
	}
	//calculate depth of the new node (number of spaces before title)
	//NOTE: it might turn out that the text is not really a text ;)
	int nSpaces = 0;
	if(!bIsText){
		char *szPos = szBuffer;
		if(szPos){
			while(*szPos==' '){
				nSpaces ++; szPos ++;
			}
		}
		if(nSpaces%3 != 0)
			bIsText = true;	// it is probably a text then
	}

	if(bIsText)
	{
		//append the text line to the current node
		int nIdx = m_pDoc->GetIdxFromID(m_nCurNodeID);
		if(nIdx >= 0)
		{
			//parse out <NF1> tag (equals to empty line)
			if(0 == strcmp("<NF1>", szBuffer)){
				m_pDoc->GetNodeByIdx(nIdx).GetText() += "\n";
				return;
			}
			//parse out <NF0> tag
			if(0 == strcmp("<NF0>", szBuffer))
				return;

			//handle MM/LX subformat with text line indented and prefixed with '|'
			const char *szPos = szBuffer;
			if(szBuffer[0] == '|'){
				szPos++;
				int nMax = (m_nLastNodeLevel + 2) * 3 - 1;
				for(int i=0; i<nMax; i++)
					if(*szPos == ' ')
						szPos++;
					else
						break;
			}

			//append text line to the current node
			gchar *strUtf8 = g_convert(szPos, strlen (szPos), "UTF-8", "ibm850", NULL, NULL, NULL);
			ASSERT(NULL != strUtf8);
			if(NULL != strUtf8){
				m_pDoc->GetNodeByIdx(nIdx).GetText() += strUtf8;
				m_pDoc->GetNodeByIdx(nIdx).GetText() += "\n";
				g_free(strUtf8);
			}
			else	//on error, fall back to unconverted text
			{
				m_nCoversionFailuresCnt ++;
				m_pDoc->GetNodeByIdx(nIdx).GetText() += szPos;
				m_pDoc->GetNodeByIdx(nIdx).GetText() += "\n";
			}
		}
	}
	else{
		char *szPos = szBuffer + nSpaces;

		//calculate depth of the new node
		int nLevel = nSpaces/3 - 1; //starts from 0

		std::string strLinkUrl;
		bool bFinished = false;

		//check for link type 1  (link to another map file)
		char *szPosLink1 = strchr(szPos, 0x18);
		if(NULL != szPosLink1){
			*szPosLink1 = '\0';
			szPosLink1 ++;

			//check if link points to some subnode within the map
			char *szHash = strchr(szPosLink1, '#');
			if(NULL != szHash)
				*szHash = '\0';	//cut the hash part (currently not supported by notecase)

			//extract link URL
			gchar *strLinkUtf8 = g_convert(szPosLink1, strlen (szPosLink1), "UTF-8", "ibm850", NULL, NULL, NULL);
			ASSERT(NULL != strLinkUtf8);
			if(NULL != strLinkUtf8){
				strLinkUrl = strLinkUtf8;
				g_free(strLinkUtf8);
			}
			else{	//on error, fall back to unconverted text
				m_nCoversionFailuresCnt ++;
				strLinkUrl = szPosLink1;
			}
		}
		else{
			// check for link type 2 (link to ordinary file)
			szPosLink1 = strchr(szPos, 0x19);
			if(NULL != szPosLink1){
				*szPosLink1 = '\0';

				//extract link URL
				szPosLink1 ++;
				gchar *strLinkUtf8 = g_convert(szPosLink1, strlen (szPosLink1), "UTF-8", "ibm850", NULL, NULL, NULL);
				ASSERT(NULL != strLinkUtf8);
				if(NULL != strLinkUtf8){
					strLinkUrl = strLinkUtf8;
					g_free(strLinkUtf8);
				}
				else{	//on error, fall back to unconverted text
					m_nCoversionFailuresCnt ++;
					strLinkUrl = szPosLink1;
				}
			}
		}

		//check for finished char
		char *szPosFinished = strchr(szPos, 0xFB);
		if(NULL != szPosFinished){
			*szPosFinished = '\0';
			bFinished = true;
		}

	#ifdef _DEBUG
		if(strlen (szPos) == 0)
			TRACE("ERROR: Invalid MM/LX file. Empty title detected!\n");
	#endif

		//convert text before using it
		gchar *strUtf8 = g_convert(szPos, strlen (szPos), "UTF-8", "ibm850", NULL, NULL, NULL);
		ASSERT(NULL != strUtf8);

	#ifdef _DEBUG
		if(!strUtf8)
			TRACE("ERROR: Failed to convert from ibm850\n");
	#endif

		if(nLevel > m_nLastNodeLevel)
		{
			//insert node as child of current node
			m_pDoc->NodeInsert(m_nCurNodeID, -1);
			m_nCurParentID = m_nCurNodeID;	//current node will become a new parent
			m_nCurNodeID   = m_pDoc->GetNodeByIdx(m_pDoc->GetNodeCount()-1).m_nID;
		}
		else if(nLevel == m_nLastNodeLevel)
		{
			//insert node as sibling of current node
			m_pDoc->NodeInsert(m_nCurParentID, -1);
			m_nCurNodeID   = m_pDoc->GetNodeByIdx(m_pDoc->GetNodeCount()-1).m_nID;
		}
		else{
			DocumentIterator it(*m_pDoc);

			//step back X levels and create child there
			int nDiff = m_nLastNodeLevel - nLevel;
			while(nDiff > 0 && m_nCurParentID >= 0){
				m_nCurParentID = it.GetNodeByID(m_nCurParentID).m_nParentID;
				nDiff --;
			}

			m_pDoc->NodeInsert(m_nCurParentID, -1);
			m_nCurNodeID   = m_pDoc->GetNodeByIdx(m_pDoc->GetNodeCount()-1).m_nID;
		}

		//set link
		if(strLinkUrl.size() > 0){
			m_pDoc->GetNodeByIdx(m_pDoc->GetNodeCount()-1).m_objLink.m_strTargetURL = strLinkUrl;
		}

		//set finished
		if(bFinished)
			m_pDoc->GetNodeByIdx(m_pDoc->GetNodeCount()-1).m_bFinished = true;

		//set title
		bool bFree = true;
		if(NULL == strUtf8){	//on error, fall back to unconverted text
			m_nCoversionFailuresCnt ++;
			strUtf8 = szPos;
			bFree = false;
		}
		if(NULL != strUtf8){
			std::string strTitle(strUtf8);

			//remove hotkey (&) cahracter if found
			replaceall(strTitle, "&", "");

			//recognize if the title is an icon
			if(strTitle.size() > 4){
				if(0 == strcasecmp(strTitle.substr(strTitle.size()-4).c_str(), ".icn")){
					strTitle.insert(0, "[ICON: ");
					strTitle += "]";
				}
			}

			m_pDoc->GetNodeByIdx(m_pDoc->GetNodeCount()-1).SetTitle(strTitle.c_str());
			if(bFree)
				g_free(strUtf8);
		}

		m_nLastNodeLevel = nLevel;
	}
}
