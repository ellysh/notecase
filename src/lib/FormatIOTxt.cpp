////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements plain text format I/O
////////////////////////////////////////////////////////////////////////////

#include "FormatIOTxt.h"
#include "DocumentIterator.h"
#include "debug.h"
#include <stdio.h>	//fopen

#ifdef _WIN32
 #ifndef __MINGW32__
   #ifndef snprintf
    #define snprintf _snprintf
   #endif
 #endif
#endif

#include "../support.h"	//TOFIX dependent on non-core

FormatIO_Txt::FormatIO_Txt()
{
}

FormatIO_Txt::~FormatIO_Txt()
{
}

int FormatIO_Txt::Load(const char *szFile, NoteDocument &doc)
{
	m_pDoc = &doc;  //store pointer

	//clear old document contents
	doc.Clear();

	FILE *pInFile = fopen(szFile, "r");
	if(NULL == pInFile)
		return DOC_LOAD_NOT_FOUND;

	//create new node - load entire text file as a single node
	m_pDoc->NodeInsert(-1, -1);  //TOFIX error report
	int nIdx = m_pDoc->GetNodeCount()-1;
	m_pDoc->GetNodeByIdx(nIdx).SetTitle(_("New node"));

	char szBuffer[1024];

	//TOFIX error handling
	while(NULL != fgets(szBuffer, sizeof(szBuffer), pInFile))
	{
		RunLoop();	//enable progress painting events

		//append text
		m_pDoc->GetNodeByIdx(nIdx).GetText() += szBuffer;
	}

	fclose(pInFile);
	return DOC_LOAD_OK;
}

int FormatIO_Txt::Save(const char *szFile, NoteDocument &doc)
{
	FILE *pOutFile = fopen(szFile, "w");
	if(NULL == pOutFile)
		return DOC_SAVE_ERR_FILE_OPEN;

	DocumentIterator it(doc);

	//write node contents in depth search order (recursive index)
	int nCnt = doc.GetNodeCount();

	for(int i=0; i<nCnt; i++)
	{
		int nIdx = it.RecursiveIdx2NodeIdx(i);
		ASSERT(nIdx >= 0);

		//calculate depth info "2.1.5"
		int nIdxTmp = nIdx;
		std::string strIndex;
		char szBuffer[20];
		for(;;)
		{
			NoteNode &note = doc.GetNodeByIdx(nIdxTmp);

			// add current sibling at the start of the string
			int nSIB = note.m_nSiblingIdx;
			snprintf(szBuffer, sizeof(szBuffer), "%d.", nSIB+1);
			std::string strTmp(strIndex);
			strIndex  = szBuffer;
			strIndex += strTmp;

			// go to parent node
			int nPID = note.m_nParentID;
			if(nPID < 0)
				break;
			nIdxTmp = doc.GetIdxFromID(nPID);
		}
		strIndex.erase(strIndex.begin()+strIndex.size()-1);

		NoteNode &theNote = doc.GetNodeByIdx(nIdx);

		size_t nRes;

		//write node title line
		std::string strTitle = theNote.GetTitle();
		nRes = fwrite("* ", 2, 1, pOutFile);
		nRes = fwrite(strIndex.c_str(), strIndex.size(), 1, pOutFile);
		nRes = fwrite(": ", 2, 1, pOutFile);
		if(strTitle.size() > 0)
			nRes = fwrite(strTitle.c_str(), strTitle.size(), 1, pOutFile);
		nRes = fwrite("\n\n", 2, 1, pOutFile);
		//fprintf(pOutFile, "* %s: %s\n\n", strIndex.c_str(), strTitle.c_str());

		//write	node contents
		std::string strText = theNote.GetText();
		if(strText.size() > 0)
			nRes = fwrite(strText.c_str(), strText.size(), 1, pOutFile);
		nRes = fwrite("\n\n", 2, 1, pOutFile);
		//fprintf(pOutFile, "%s\n\n", theNote.GetText().c_str());
	}

	fclose(pOutFile);
	return DOC_SAVE_OK;
}
