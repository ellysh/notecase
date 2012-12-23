////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements Blowfish encrypted NoteCase HTML format I/O
////////////////////////////////////////////////////////////////////////////

#include "SHA1.h"	// must be the top one, because its defines can cause linker error
#include "FormatIOEncHtml.h"
#include "IOProcess.h"
#include "IOLayerZlib.h"
#include "IOLayerEnc.h"
#include "IOLayerRedirect.h"
#include "File64.h"
#include "IOLayerFile64.h"
#include "DocumentIterator.h"
#include "debug.h"
#include <stdio.h>	//sprintf
#include <glib.h>	// bate order macros
#include "../gui/ProgressDlg.h"

#include <gtk/gtk.h>
int gtkMessageBox(const char *szText, int nButtons = GTK_BUTTONS_OK, int nIcon = GTK_MESSAGE_INFO); //TOFIX temp

void ParseRedirector(const char *, int);
static HTMLParser *g_pParser = NULL;

FormatIO_EncHTML::FormatIO_EncHTML()
{
	m_nOrigSize = 0;
	m_nEncFormatVersion = 3; //default
	m_nStartOffset = 0;
	m_nEndOffset = -1;
}

FormatIO_EncHTML::~FormatIO_EncHTML()
{
}

void FormatIO_EncHTML::SetPassword(const char *szPass)
{
	m_strPass = szPass;
}

int FormatIO_EncHTML::Load(const char *szFile, NoteDocument &doc)
{
	if(m_strPass.size()==0)
		return DOC_LOAD_WRONG_PASSWORD;	//password not set

	IOProcess objFile(MODE_READ);

	// attach file read layer
	IOLayerFile64 *pFileLayer = new IOLayerFile64;
	objFile.PushLayer(pFileLayer);

	// attach content decryption layer
	IOLayerEnc *pEncLayer = new IOLayerEnc;
	pEncLayer->SetPassword(m_strPass.c_str());
	objFile.PushLayer(pEncLayer);	// decryption step

	char szBuffer[1032] = "";
	int nRead = 0;

	m_pDoc = &doc;  //store pointer

	//clear old document contents
	doc.Clear();

	if(!pFileLayer->m_file.Open(szFile, F64_READ|F64_SHARE_READ|F64_OPEN_EXISTING)){
		return DOC_LOAD_NOT_FOUND;
	}
	if(m_nStartOffset > 0){
		pFileLayer->m_file.Seek(m_nStartOffset);
		pFileLayer->m_nEndOffset = m_nEndOffset;
	}

	//
	// read encrypted format header
	//

	// 1. format signature text "NOTECASE" (8 bytes)
	char szFormatCode[9];
	pFileLayer->m_file.Read(szFormatCode, 8);	// "explicit qualification"
	szFormatCode[8] = '\0';
	if(0 != strcmp(szFormatCode, "NOTECASE"))
		return DOC_LOAD_FORMAT_ERROR;	//

	//---- FORMAT VERSION DETECTION ----
	// 2. test for new version formats - read version string (4 bytes)
	char szFormatVer[5];
	pFileLayer->m_file.Read(szFormatVer, 4);	// "explicit qualification"
	szFormatVer[4] = '\0';
	if(0 == strcmp(szFormatVer, "enc3")){
		m_nEncFormatVersion = 3;
	}
	else if(0 == strcmp(szFormatVer, "enc2")){
		m_nEncFormatVersion = 2;
	}
	else{
		m_nEncFormatVersion = 1;	//oldest version of enc format
		pFileLayer->m_file.Seek(8 + m_nStartOffset);	// roll back
	}

	// if new format attach content decompression layer
	IOLayerZlib *pZlibLayer = NULL;
	if(m_nEncFormatVersion > 1)
	{
		pZlibLayer = new IOLayerZlib;
		objFile.PushLayer(pZlibLayer);
	}

	// attach redirector object
	IOLayerRedirect *pRedirectLayer = new IOLayerRedirect;
	pRedirectLayer->SetCallback(ParseRedirector);
	g_pParser = this;
	objFile.PushLayer(pRedirectLayer);

	std::string strPassSalted = m_strPass;

	// 3. read salt (4 bytes)
	if(3 == m_nEncFormatVersion)
	{
		int nSalt = 0;
		pFileLayer->m_file.Read((char *)&nSalt, sizeof(nSalt));

		TRACE("Salt read: %d\n", nSalt);

		//append salt to password
		unsigned char *szPass = (unsigned char *)&nSalt;
		strPassSalted += *szPass; szPass++;
		strPassSalted += *szPass; szPass++;
		strPassSalted += *szPass; szPass++;
		strPassSalted += *szPass;
	}

	// 4. SHA-1 password hash value (20 bytes)
	char szHashRead[20] = "";
	pFileLayer->m_file.Read(szHashRead, 20);
	CSHA1 hash;
	hash.Update((UINT_8 *)strPassSalted.c_str(), strPassSalted.size());
	hash.Final();

	if(0 != memcmp(szHashRead, hash.m_digest, 20))
		return DOC_LOAD_WRONG_PASSWORD;	//invalid password

	// 5. original (unencrpyted) file size (8 bytes)
	nRead = pFileLayer->m_file.Read((char *)&m_nOrigSize, sizeof(m_nOrigSize));
	ASSERT(nRead == sizeof(m_nOrigSize));

	// 6. compressed stream size (8 bytes)
	if(m_nEncFormatVersion > 1){
		nRead = pFileLayer->m_file.Read((char *)&(pZlibLayer->m_nTotalInBytes), sizeof(INT64));//TOFIX update at the end
		ASSERT(nRead == sizeof(INT64));

		//convert from network byte order (big-endian) to processor default
		if(G_BYTE_ORDER == G_LITTLE_ENDIAN)
			pZlibLayer->m_nTotalInBytes = GINT64_FROM_BE(pZlibLayer->m_nTotalInBytes);

		TRACE("Zlib stream size as read from file is: %d\n", (int)(pZlibLayer->m_nTotalInBytes)); //TOFIX printf flag for int64!
	}

	INT64 nReadTotal = 0;

	//
	// main reading loop
	//  (some layers cache data from one step and can return 0 bytes in some cycle)
	//TOFIX error handling
	bool bContinue = true;
	while(bContinue){
		ProgressDlg::SetCurrentSize(nReadTotal);
		RunLoop();	//enable progress painting events

		nRead = objFile.Read(szBuffer, 1024, bContinue);
		nReadTotal += nRead;
	}
	objFile.Finalize();

	if(m_nEncFormatVersion > 1)
	{
		TRACE("Zlib decompress total bytes input: %d\n", (int)(pZlibLayer->m_nProcessedInBytes)); //TOFIX printf flag for int64!
		TRACE("Zlib decompress total bytes output: %d\n", (int)(pZlibLayer->m_nProcessedOutBytes)); //TOFIX printf flag for int64!
	}

	ConvertInternalLinks();

	//TOFIX use m_nOrigSize
	return DOC_LOAD_OK;
}

int FormatIO_EncHTML::Save(const char *szFile, NoteDocument &doc)
{
	return Save(szFile, doc, false);
}

int FormatIO_EncHTML::Save(const char *szFile, NoteDocument &doc, bool bAppendMode)
{
	if(m_strPass.size()==0){
		ASSERT(false);	//this should not be allowed
		return DOC_SAVE_ERROR;	//password not set
	}

	m_pDoc = &doc;  //store pointer

	IOProcess objFile(MODE_WRITE);

	// attach content compression layer
	IOLayerZlib *pZlibLayer = NULL;
	if(m_nEncFormatVersion > 1){
		pZlibLayer = new IOLayerZlib;
		objFile.PushLayer(pZlibLayer);	// compress before encrypting
	}

	// attach content encryption layer
	IOLayerEnc *pEncLayer = new IOLayerEnc;
	pEncLayer->SetPassword(m_strPass.c_str());
	objFile.PushLayer(pEncLayer);	// encryption step

	// attach file write layer
	IOLayerFile64 *pFileLayer = new IOLayerFile64;
	objFile.PushLayer(pFileLayer);

	INT64 nSaveOffset = 0;
	if(bAppendMode){
		if(!pFileLayer->m_file.Open(szFile, F64_WRITE|F64_SHARE_READ|F64_OPEN_EXISTING))
			return DOC_SAVE_ERR_FILE_OPEN;
		nSaveOffset = pFileLayer->m_file.SeekEnd();	//preserve existing content
		ASSERT(nSaveOffset > 0);
	}
	else
		if(!pFileLayer->m_file.Open(szFile, F64_WRITE|F64_SHARE_READ|F64_OPEN_NEW))
			return DOC_SAVE_ERR_FILE_OPEN;

	//
	// write encrypted format header
	//

	// 1. format signature text "NOTECASE" (8 bytes)
	int nWritten = pFileLayer->m_file.Write("NOTECASE", 8);
	ASSERT(8 == nWritten);

	// 2. write format version string (4 bytes) - new format
	if(2 == m_nEncFormatVersion){
		nWritten = pFileLayer->m_file.Write("enc2", 4);
		ASSERT(4 == nWritten);
	}
	else if(3 == m_nEncFormatVersion){
		nWritten = pFileLayer->m_file.Write("enc3", 4);
		ASSERT(4 == nWritten);
	}

	std::string strPassSalted = m_strPass;

	// 3. calculate salt
	if(3 == m_nEncFormatVersion)
	{
		//generate new random salt (32bit)
		srand((unsigned)time(NULL));
		int nSalt = rand();
		if(G_BYTE_ORDER == G_LITTLE_ENDIAN)
			nSalt = GINT32_TO_BE(nSalt);

		TRACE("Salt write: %d\n", nSalt);

		//write salt field
		nWritten = pFileLayer->m_file.Write((char *)&nSalt, sizeof(nSalt));
		ASSERT(4 == nWritten);

		//append salt to password
		unsigned char *szPass = (unsigned char *)&nSalt;
		strPassSalted += *szPass; szPass++;
		strPassSalted += *szPass; szPass++;
		strPassSalted += *szPass; szPass++;
		strPassSalted += *szPass;
	}

	// 4. SHA-1 password hash value (20 bytes)
	CSHA1 hash;
	hash.Update((UINT_8 *)strPassSalted.c_str(), strPassSalted.size());
	hash.Final();
	nWritten = pFileLayer->m_file.Write((char *)hash.m_digest, 20);
	ASSERT(20 == nWritten);

	// 5. original (unencrpyted) file size (8 bytes)
	nWritten = pFileLayer->m_file.Write((char *)&m_nOrigSize, sizeof(m_nOrigSize));//TOFIX update at the end
	ASSERT(sizeof(m_nOrigSize) == nWritten);

	// 6. compressed stream size (8 bytes)
	if(m_nEncFormatVersion > 1)
	{
		INT64 nSize = 0;
		nWritten = pFileLayer->m_file.Write((char *)&(nSize), sizeof(INT64));
		ASSERT(sizeof(INT64) == nWritten);
	}

	//
	// Start writing the main part of the stream
	//

	WriteDocContents(objFile);
	objFile.Finalize();

	//TOFIX update m_nOrigSize

	// update item #5. compressed stream size (8 bytes)
	if(m_nEncFormatVersion > 1)
	{
		TRACE("Zlib stream bytes input: %d / output: %d\n", (int)(pZlibLayer->m_nProcessedInBytes), (int)(pZlibLayer->m_nProcessedOutBytes)); //TOFIX printf flag for int64!
		INT64 nPos = 0; //prevent "unused variable" warning
		if(2 == m_nEncFormatVersion){
			nPos = pFileLayer->m_file.Seek(40 + nSaveOffset);
			ASSERT((40 + nSaveOffset) == nPos);
		}
		else if(3 == m_nEncFormatVersion){
			nPos = pFileLayer->m_file.Seek(44 + nSaveOffset);
			ASSERT((44 + nSaveOffset) == nPos);
		}

		//convert from processor default to network byte order (big-endian)
		INT64 nSize = pZlibLayer->m_nProcessedOutBytes;
		if(G_BYTE_ORDER == G_LITTLE_ENDIAN)
			nSize = GINT64_TO_BE(nSize);

		nWritten = pFileLayer->m_file.Write((char *)&(nSize), sizeof(INT64));
		ASSERT(sizeof(INT64) == nWritten);
	}

	return DOC_SAVE_OK;
}

void ParseRedirector(const char *buf, int len)
{
	if(g_pParser)
		g_pParser->Parse(buf, len);
}
