////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements Gjots2 format I/O (reading only)
////////////////////////////////////////////////////////////////////////////

#include "FormatIOGjots2.h"
#include "DocumentIterator.h"
#include <stdio.h>	//fopen
#include <string.h>

#define  DOC_PARSE_BLANK		0
#define  DOC_PARSE_EXPECT_TITLE	1
#define  DOC_PARSE_EXPECT_TEXT	2

FormatIO_Gjots2::FormatIO_Gjots2()
{
	m_nCurParentID = -1;
	m_nCurNodeID = -1;
	m_nDocParseState = DOC_PARSE_BLANK;
}

FormatIO_Gjots2::~FormatIO_Gjots2()
{
}

int FormatIO_Gjots2::Load(const char *szFile, NoteDocument &doc)
{
	m_pDoc = &doc;  //store pointer

	//clear old document contents
	doc.Clear();

	FILE *pInFile = fopen(szFile, "r");
	if(NULL == pInFile)
		return DOC_LOAD_NOT_FOUND;

	char szBuffer[1024];

	//TOFIX error handling
	while(NULL != fgets(szBuffer, sizeof(szBuffer), pInFile))
	{
		RunLoop();	//enable progress painting events

		//kill new line char
		if('\n'== szBuffer[strlen(szBuffer)-1])
			szBuffer[strlen(szBuffer)-1] = '\0';

		ParseLine(szBuffer);
	}

	fclose(pInFile);
	return DOC_LOAD_OK;
}

int FormatIO_Gjots2::Save(const char *szFile, NoteDocument &doc)
{
	return DOC_SAVE_NOT_SUPPORTED;	//export not implemented
}

bool FormatIO_Gjots2::ParseLine(const char *szBuffer)
{
	//parse line by line
	if(0 == strcmp("\\NewEntry", szBuffer))
	{
		//create new node
		m_pDoc->NodeInsert(m_nCurParentID, -1);  //TOFIX error report
		m_nCurNodeID = m_pDoc->GetNodeByIdx(m_pDoc->GetNodeCount()-1).m_nID;
		m_nDocParseState = DOC_PARSE_EXPECT_TITLE;

	}
	else if(0 == strcmp("\\NewFolder", szBuffer))
	{
		//prepare for subnode branching
		m_nCurParentID = m_nCurNodeID;	//current node will become a new parent
		m_nCurNodeID   = -1;
	}
	else if(0 == strcmp("\\EndFolder", szBuffer))
	{
		//end of subnode branching (set parent's parent as a new parent node)
		int nIdx = m_pDoc->GetIdxFromID(m_nCurParentID);
		m_nCurParentID = m_pDoc->GetNodeByIdx(nIdx).m_nParentID;
		m_nCurNodeID   = -1;
	}
	else
	{
		//text or title
		if(DOC_PARSE_EXPECT_TITLE == m_nDocParseState)
		{
			//set node title
			int nIdx = m_pDoc->GetIdxFromID(m_nCurNodeID);
			if(nIdx >= 0)
				m_pDoc->GetNodeByIdx(nIdx).SetTitle(szBuffer);

			m_nDocParseState = DOC_PARSE_BLANK; //there might not be text
		}
		else
		{
			//set node text
			int nIdx = m_pDoc->GetIdxFromID(m_nCurNodeID);
			if(nIdx >= 0)
			{
				//append text line to the current node
				std::string strTxt = m_pDoc->GetNodeByIdx(nIdx).GetText();
				if(strTxt.empty())
				{
					strTxt = szBuffer;
				}
				else
				{
					strTxt += '\n';
					strTxt += szBuffer;
				}
				m_pDoc->GetNodeByIdx(nIdx).SetText(strTxt.c_str());
			}
		}
	}

	return true;
}
