////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements export to standalone executable
////////////////////////////////////////////////////////////////////////////

#include "FormatIOExecutable.h"
#include "FormatIOEncHtml.h"
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
#else
 #include <sys/types.h>	//chmod
 #include <sys/stat.h>
#endif

std::string GetAppPath();

FormatIO_Executable::FormatIO_Executable()
{
	m_nCurParentID = -1;	//TOFIX use Idx for faster operation
	m_nCurNodeID = -1;
	m_nLastNodeLevel = -1;
	m_nCurTextLine = 0;
	m_nCoversionFailuresCnt = 0;
}

FormatIO_Executable::~FormatIO_Executable()
{
}

int FormatIO_Executable::Load(const char *szFile, NoteDocument &doc)
{
	std::string strSrcExe = GetAppPath();
	FILE *pExe = fopen(strSrcExe.c_str(), "rb");
	if(NULL != pExe)
	{
		//check "magic" marker adt the end of the file
		fseek(pExe, -8, SEEK_END);
		char szBuffer[10];
		size_t nRead = 0;
		nRead = fread(szBuffer, 8, 1, pExe);
		szBuffer[8] = '\0';
		if(0 != strcmp("NOTECASE", szBuffer)){
			fclose(pExe);
			return DOC_LOAD_FORMAT_ERROR;
		}

		//marker is correct, read document start offset
		INT64 nStandaloneOffset = -1;

		fseek(pExe, -16, SEEK_END);
		int nEndOffset = ftell(pExe);

		nRead = fread(&nStandaloneOffset, sizeof(INT64), 1, pExe);
		//convert from network byte order (big-endian) to processor default
		if(G_BYTE_ORDER == G_LITTLE_ENDIAN)
			nStandaloneOffset = GINT64_FROM_BE(nStandaloneOffset);
		fclose(pExe);

		//now load the document
		FormatIO_Base *pLoader = new FormatIO_EncHTML;
		((FormatIO_EncHTML *)pLoader)->SetLoadEmbedded(nStandaloneOffset, (INT64)nEndOffset);
		((FormatIO_EncHTML *)pLoader)->SetPassword(doc.GetPassword());
		int nRes = pLoader->Load(strSrcExe.c_str(), doc);

		//copy data from the real loader
		m_strGeneratorApp = ((FormatIO_EncHTML *)pLoader)->m_strGeneratorApp;
		m_strGeneratorVer = ((FormatIO_EncHTML *)pLoader)->m_strGeneratorVer;

		delete pLoader;
		return nRes;
	}

	return DOC_LOAD_ERROR;
}

int FormatIO_Executable::Save(const char *szFile, NoteDocument &doc)
{
	//
	// first create a copy of current Notecase application file
	//
	std::string strSrcExe = GetAppPath();
	FILE *pExe = fopen(strSrcExe.c_str(), "rb");
	if(NULL == pExe)
		return DOC_SAVE_ERR_EXE_OPEN;

	//check if this exe has already embedded document (that is not being copied)
	// - check "magic" marker at the end of the file
	INT64 nStandaloneOffset = -1;
	fseek(pExe, -8, SEEK_END);
	char szBuffer1[10];
	int nRead = 0;
	nRead = fread(szBuffer1, 8, 1, pExe);
	szBuffer1[8] = '\0';
	if(0 == strcmp("NOTECASE", szBuffer1)){
		//marker is correct, read document start offset
		fseek(pExe, -16, SEEK_END);
		nRead = fread(&nStandaloneOffset, sizeof(INT64), 1, pExe);
		//convert from network byte order (big-endian) to processor default
		if(G_BYTE_ORDER == G_LITTLE_ENDIAN)
			nStandaloneOffset = GINT64_FROM_BE(nStandaloneOffset);
	}
	fseek(pExe, 0, SEEK_SET);

	FILE *pOut = fopen(szFile, "wb");
	if(NULL == pOut){
		fclose(pExe);
		return DOC_SAVE_ERR_FILE_OPEN;
	}

	int nTotal = 0;
	char szBuffer[10000];
	size_t nWritten = 0;
	while((nRead = fread(szBuffer, 1, sizeof(szBuffer), pExe)) > 0){
		if(nStandaloneOffset >= 0 && (nTotal + nRead) > nStandaloneOffset){
			nWritten = fwrite(szBuffer, 1, (size_t)(nStandaloneOffset-nTotal), pOut);
			break;
		}
		else{
			nWritten = fwrite(szBuffer, 1, nRead, pOut);
			nTotal += nRead;
		}
	}
	fclose(pExe);

	//
	// next append the document
	//
	INT64 nDocStartPos = ftell(pOut);
	fclose(pOut);

	//Document is always in stored in EncHtml format (because it is compressed)
	//if user does not have a password we use default one "NOTECASE"
	std::string strPass = doc.GetPassword();
	if(strPass.size() == 0)
		strPass = "NOTECASE";

	FormatIO_EncHTML exporter;
	exporter.SetPassword(strPass.c_str());
	int nRes = exporter.Save(szFile, doc, true);
	if(DOC_SAVE_OK != nRes){
		remove(szFile);	//remove output file on error
		return nRes;
	}

	//now write a header to recognize document embedded in the exe
	pOut = fopen(szFile, "ab");
	if(NULL == pOut){
		remove(szFile);	//remove output file on error
		return DOC_SAVE_ERR_FILE_OPEN;
	}

	//write offset of the begining of the document
	//convert from processor default to network byte order (big-endian)
	if(G_BYTE_ORDER == G_LITTLE_ENDIAN)
		nDocStartPos = GINT64_TO_BE(nDocStartPos);
	nWritten = fwrite(&nDocStartPos, sizeof(INT64), 1, pOut);

	//write "magic" letters to mark embedded document
	nWritten = fwrite("NOTECASE", strlen("NOTECASE"), 1, pOut);
	fclose(pOut);

#ifndef _WIN32
	//set default file permission (note that chmod(755) would faild for me)
	mode_t mode = S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IROTH; //0x0744
	chmod(szFile, mode);
#endif


	return DOC_SAVE_OK;
}
