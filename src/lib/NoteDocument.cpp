////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Class that stores/manipulates NoteCase document contents
////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
  #pragma warning(disable: 4996)	//ignore deprecated functions warning
#endif
#include "SHA1.h"	// must be the top one, because its defines can cause linker error
#include "NoteDocument.h"
#include "FormatIOHtml.h"
#include "FormatIOEncHtml.h"
#include "FormatIOGjots2.h"
#include "FormatIOStickyNotes.h"
#include "FormatIOTxt.h"
#include "FormatIOMMLX.h"
#include "FormatIOExecutable.h"
#include "DocumentIterator.h"
#include "FilePath.h"
#include "File64.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

//TOFIX remove GUI dependency through some callback?
#include "../gui/ProgressDlg.h"
#include "../support.h"

#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gstdio.h>

int gtkMessageBox(const char *szText, int nButtons = GTK_BUTTONS_OK, int nIcon = GTK_MESSAGE_INFO); //TOFIX temp
int cmpver(const char* szVer1, const char* szVer2);

extern GtkWidget *window1;
std::string GetAppPath();

#ifdef _WIN32
 #include <io.h>
 #ifndef __MINGW32__
  #define strcasecmp stricmp
 #endif
#else
 #include <sys/types.h>	//chmod
 #include <sys/stat.h>
#endif
#include <time.h>

NoteDocument::NoteDocument()
{
	m_pObjLoader = NULL;
	m_nIDGenerator = 0;	//first valid node ID
	m_nCurrentNode = -1;
	m_pfnPassCallback = NULL;
	m_pfnFmtCallback = NULL;
	m_nNodesCount = 0;
	m_nActiveNodeIdx = -1;
	m_nEmbedCSS = -1;
	m_bModified = false;
	m_bReadOnly = false;
	m_bModifiedSinceAutosave = false;
}

NoteDocument::~NoteDocument()
{
	Close();
}

NoteDocument::NoteDocument(const NoteDocument &doc)
{
	operator = (doc);
}

void NoteDocument::operator = (const NoteDocument &doc)
{
	//m_pObjLoader = doc.m_pObjLoader;	//TOFIX create new loader like this!
	m_pObjLoader	= NULL;

	m_nIDGenerator    = doc.m_nIDGenerator;
	m_nCurrentNode    = doc.m_nCurrentNode;
	m_pfnPassCallback = doc.m_pfnPassCallback;
	m_lstNodes        = doc.m_lstNodes;
	m_bModified       = doc.m_bModified;
	m_bModifiedSinceAutosave       = doc.m_bModifiedSinceAutosave;
	m_strPath         = doc.m_strPath;
	m_strPassword     = doc.m_strPassword;
	m_nNodesCount     = doc.m_nNodesCount;
	m_pfnPassCallback = doc.m_pfnPassCallback;
	m_mapIdIsTarget   = doc.m_mapIdIsTarget;
	m_mapId2Idx       = doc.m_mapId2Idx;
	m_bReadOnly		  = doc.m_bReadOnly;

#ifdef _DEBUG
	AssertValid();
#endif
}

void NoteDocument::Close()
{
	Clear();

	if(m_pObjLoader)
		delete m_pObjLoader;
	m_pObjLoader = NULL;

	m_strPath = "";	//forget path
	SetModified(false);
	m_bReadOnly = false;
}

void NoteDocument::Clear()
{
	m_lstNodes.erase(m_lstNodes.begin(), m_lstNodes.end());
	m_nNodesCount = 0;

	m_mapId2Idx.clear();
	m_mapIdIsTarget.clear();

	m_nIDGenerator = 0;	//first valid node ID
}

bool NoteDocument::IsEncrypted()
{
	if(m_pObjLoader)
		return (FORMAT_NOTECASE_HTML_ENC == m_pObjLoader->GetFormat());

	return false;
}

int NoteDocument::GetFormatFromFileName(const char *szPath)
{
	//calculate extension to determine format
	const char *szExt = strrchr(szPath, '.');
	if(NULL == szExt)
		return -1;	//error

	//match ext for supported formats
	if( 0 == strcasecmp(szExt, ".ncd"))
		return FORMAT_NOTECASE_HTML;
	else if(0 == strcasecmp(szExt, ".hnc"))
		return FORMAT_NOTECENTER_HTML;
	else if(0 == strcasecmp(szExt, ".nce"))
		return FORMAT_NOTECASE_HTML_ENC;
	else if(0 == strcasecmp(szExt, ".gjots2"))
		return FORMAT_GJOTS2;
	else if(0 == strcasecmp(szExt, ".xml"))
		return FORMAT_STICKY;
	else if(0 == strcasecmp(szExt, ".html"))
		return FORMAT_HTML;
	else if(0 == strcasecmp(szExt, ".txt"))
		return FORMAT_TXT;
	else if(0 == strcasecmp(szExt, ".MM"))
		return FORMAT_MMLX;
	else if(0 == strcasecmp(szExt, ".exe"))
		return FORMAT_EXECUTABLE;

	return -1;
}

int NoteDocument::Load(const char *szPath, const char *szPassword, int nFormat)
{
	Close();

#ifdef _WIN32
 #define access _access
#endif

	//check if file exists
	if(0 != g_access(szPath, 0))
		return DOC_LOAD_NOT_FOUND;

	//test if file readable
	if(0 != g_access(szPath, 04))
		return DOC_LOAD_PERMISSIONS;

	//TOFIX special factory fn.
	if(FORMAT_NONE == nFormat)
		nFormat = GetFormatFromFileName(szPath);
	if(nFormat <= 0 && NULL == m_pfnFmtCallback)
		return DOC_LOAD_UNKNOWN_FORMAT;

	m_strPassword = ""; //forget password

	if(FORMAT_NOTECASE_HTML == nFormat)
	{
		m_pObjLoader = new FormatIO_HTML;
	}
	else if(FORMAT_NOTECENTER_HTML == nFormat)
	{
		m_pObjLoader = new FormatIO_HTML;

		//set special flag for .hnc compatibility
		((FormatIO_HTML *)m_pObjLoader)->m_bNoteCenterMode = true;
	}
	else if(FORMAT_NOTECASE_HTML_ENC == nFormat)
	{
		//password required to load encrypted file format
		const char *szPass = NULL;
		if( NULL != szPassword &&
			strlen(szPassword) > 0)
		{
			szPass = szPassword;
		}
		else
		{
			ASSERT(NULL != m_pfnPassCallback);
			if(NULL == m_pfnPassCallback)
				return DOC_LOAD_ERROR;
			szPass = m_pfnPassCallback(szPath, false);
			if(NULL == szPass)
				return DOC_LOAD_ABORTED;	//Cancel
		}

		m_pObjLoader = new FormatIO_EncHTML;

		//set password to the loader
		((FormatIO_EncHTML *)m_pObjLoader)->SetPassword(szPass);

		m_strPassword = szPass;	//store password
	}
	else if(FORMAT_GJOTS2 == nFormat)
	{
		m_pObjLoader = new FormatIO_Gjots2;
	}
	else if(FORMAT_STICKY == nFormat)
	{
		m_pObjLoader = new FormatIO_StickyNotes;
	}
	else if(FORMAT_MMLX == nFormat)
	{
		m_pObjLoader = new FormatIO_MMLX;
	}
	else if(FORMAT_TXT == nFormat)
	{
		m_pObjLoader = new FormatIO_Txt;
	}
	else if(FORMAT_EXECUTABLE == nFormat)
	{
		//guess if the default password should be used (grab password hash SHA1)
		m_strPassword = "NOTECASE";	//store password

		std::string strSrcExe = GetAppPath();
		FILE *pExe = fopen(strSrcExe.c_str(), "rb");
		if(NULL != pExe)
		{
			//check "magic" marker at the end of the file
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
			nRead = fread(&nStandaloneOffset, sizeof(INT64), 1, pExe);
			//convert from network byte order (big-endian) to processor default
			if(G_BYTE_ORDER == G_LITTLE_ENDIAN)
				nStandaloneOffset = GINT64_FROM_BE(nStandaloneOffset);

			// assume format version 3 (used from 1.3.8)
			int nSalt = 0;
			fseek(pExe, (long)nStandaloneOffset + 12, SEEK_SET);
			nRead = fread(&nSalt, 4, 1, pExe);

			// 3. SHA-1 password hash value (20 bytes)
			fseek(pExe, (long)nStandaloneOffset + 16, SEEK_SET);
			char szHashRead[20] = "";
			nRead = fread(szHashRead, 1, 20, pExe);

			std::string strPassSalted = m_strPassword;

			//append salt to password
			unsigned char *szPass = (unsigned char *)&nSalt;
			strPassSalted += *szPass; szPass++;
			strPassSalted += *szPass; szPass++;
			strPassSalted += *szPass; szPass++;
			strPassSalted += *szPass;

			CSHA1 hash;
			hash.Update((UINT_8 *)strPassSalted.c_str(), strPassSalted.size());
			hash.Final();
			if(0 != memcmp(szHashRead, hash.m_digest, 20))
				m_strPassword = "";

			fclose(pExe);
		}

		if(m_strPassword.size() == 0)
		{
			const char *szPass = NULL;
			if( NULL != szPassword &&
				strlen(szPassword) > 0)
			{
				szPass = szPassword;
			}
			else
			{
				if(NULL == m_pfnPassCallback)
					return DOC_LOAD_ERROR;
				szPass = m_pfnPassCallback(szPath, false);
				if(NULL == szPass)
					return DOC_LOAD_ABORTED;	//Cancel
			}

			m_strPassword = szPass;
		}

		m_pObjLoader = new FormatIO_Executable;	// load from this .exe image
	}
	else
	{
		//load unknown format as text ?
		if(NULL != m_pfnFmtCallback)
		{
			int nFmt = m_pfnFmtCallback();
			if(FORMAT_TXT == nFmt)
			{
				m_pObjLoader = new FormatIO_Txt;
				nFormat = FORMAT_TXT;
			}
			else
				return DOC_LOAD_UNKNOWN_FORMAT;
		}
		else
			return DOC_LOAD_UNKNOWN_FORMAT;
	}

	//show wait dialog (only if not from cmd line)
#ifndef _NOKIA_MAEMO
	ProgressDlg dlg(200, _("Loading file. Please wait..."), window1);
	dlg.SetTotalSize(File64::FileSize(szPath));
#endif

	if(FORMAT_EXECUTABLE != nFormat)
		m_strPath = szPath;	//store path
	else
		m_strPath = "";

	int nResult = m_pObjLoader->Load(szPath, *this);
	if(DOC_LOAD_OK == nResult)
	{
		SetModified(false);	//just loaded
		m_nNodesCount = m_lstNodes.size();	//cache node count

		//check generator application, issue warning if loading document generated
		//by another application or newer version of this application
		if( FORMAT_NOTECASE_HTML == nFormat ||
			FORMAT_EXECUTABLE	 == nFormat ||
			FORMAT_NOTECASE_HTML_ENC == nFormat)
		{
			std::string	strGeneratorApp, strGeneratorVer;

			if(FORMAT_EXECUTABLE == nFormat){
				strGeneratorApp = ((FormatIO_Executable *)m_pObjLoader)->m_strGeneratorApp;
				strGeneratorVer = ((FormatIO_Executable *)m_pObjLoader)->m_strGeneratorVer;
			}
			else{
				strGeneratorApp = ((FormatIO_HTML *)m_pObjLoader)->m_strGeneratorApp;
				strGeneratorVer = ((FormatIO_HTML *)m_pObjLoader)->m_strGeneratorVer;
			}

			if( strGeneratorApp.size() > 0 &&
				(0 != strcmp(strGeneratorApp.c_str(), APP_NAME_STR) ||
				 0 <  cmpver(strGeneratorVer.c_str(), APP_VER_STR)))
			{
				gtkMessageBox(_("Warning:\nThis document was created either by a different application\nNotecase and Notecase Pro are not considered the same application),\nor by a newer version of this program.\n\nIf you edit this document, you might lose some formatting!"));
			}
		}
	}
	else{
		m_strPassword = ""; //forget password
		m_strPath = "";
	}

	return nResult;
}

int NoteDocument::Save(const char *szPath, bool bRememberPath, int nBackupFiles, const char *szPassword, int nFormat)
{
	//TOFIX special factory fn. ?

	//close previous loader object to prevent memory leaks
	if(m_pObjLoader)
		delete m_pObjLoader;
	m_pObjLoader = NULL;

	if(nFormat == FORMAT_NONE)
		nFormat = GetFormatFromFileName(szPath);
	if(nFormat <= 0)
		return DOC_SAVE_UNKNOWN_FORMAT;

	//ensure directory exists
	std::string strDir = GetParentDir(szPath);
	EnsureDirExists(strDir);

	// if the target file already exists implement 2 step saving
	// - first save to a temp file, then overwrite target file with the temp file
	// - this is good to preserve file data in case Notecase crashes while saving

	bool bTwoStepSave = false;
	if(0 == g_access(szPath, 0))
		bTwoStepSave = true;

	std::string strTargetPath(szPath);
	if(bTwoStepSave){
		strTargetPath += ".part";	//partial file
	}
	else
	{
		//quick FIX: delete file if it already exists
		//TOFIX more precise fix would be to test/fix file open flags when saving
		g_remove(szPath);
	}

	if(FORMAT_NOTECASE_HTML == nFormat)
	{
		m_pObjLoader = new FormatIO_HTML;
	}
	else if(FORMAT_HTML == nFormat)
	{
		m_pObjLoader = new FormatIO_HTML;

		//set special flag for .html export
		((FormatIO_HTML *)m_pObjLoader)->m_bHtmlExport = true;
	}
	else if(FORMAT_NOTECENTER_HTML == nFormat)
	{
		m_pObjLoader = new FormatIO_HTML;

		//set special flag for .hnc compatibility
		((FormatIO_HTML *)m_pObjLoader)->m_bNoteCenterMode = true;
	}
	else if(FORMAT_NOTECASE_HTML_ENC == nFormat)
	{
		//password required to load encrypted file format
		if(NULL == m_pfnPassCallback){
			ASSERT(false);	//we forgot to register callback
			return DOC_SAVE_ERROR;	//this should not happen, programmers fault
		}

		if( 0 == strcmp(szPath, m_strPath.c_str()) &&
			m_strPassword.size() > 0)
		{
			//password already exists for this path (stored)
		}
		else
		{
			if(NULL != szPassword)
				m_strPassword = szPassword;
			else
			{
				//ask for password
				const char *szPass = m_pfnPassCallback(szPath, true);
				if(NULL == szPass)
					return DOC_SAVE_ABORTED;	//Cancel
				m_strPassword = szPass;	//store password
			}
		}

		m_pObjLoader = new FormatIO_EncHTML;

		//set password to the loader
		((FormatIO_EncHTML *)m_pObjLoader)->SetPassword(m_strPassword.c_str());
	}
	else if(FORMAT_TXT == nFormat)
	{
		m_pObjLoader = new FormatIO_Txt;	// save as plain text
	}
	else if(FORMAT_EXECUTABLE == nFormat)
	{
		m_pObjLoader = new FormatIO_Executable;	// save as standalone exe file
	}
	else
		return DOC_SAVE_UNKNOWN_FORMAT;

	//show wait dialog (only if not from cmd line)
#ifndef _NOKIA_MAEMO
	ProgressDlg dlg(200, _("Saving file. Please wait..."), window1);
#endif

	if(bRememberPath)
		m_strPath = szPath;	//store path

	int nSaveRes = m_pObjLoader->Save(strTargetPath.c_str(), *this);
	if(DOC_SAVE_OK == nSaveRes)
	{
	#ifndef _WIN32
		//set default file permission
		mode_t mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH; //0x0644
		chmod(szPath, mode);
	#endif

		if(bTwoStepSave)
		{
			if(nBackupFiles > 0)
			{
				//calculate file name for backup file
				char szPathBak[1024];
				int nBakIdx = 0;
				time_t nTimeOldest = LONG_MAX;

				for(int i=1; i<nBackupFiles+1; i++)
				{
					sprintf(szPathBak, "%s.bak%d", szPath, i);

				#ifdef _WIN32
				 #define stat _stat
				 #define S_IFDIR _S_IFDIR
				#endif

					struct stat st;
					if(0 == stat(szPathBak, &st)){
						if(st.st_mtime < nTimeOldest){
							//path exists and minimal
							nBakIdx = i;
							nTimeOldest = st.st_mtime;
						}
					}
					else{
						nBakIdx = i; break;
					}
				}

				ASSERT(nBakIdx > 0);
				sprintf(szPathBak, "%s.bak%d", szPath, nBakIdx);

				//rename .bak file if already exists
				g_remove(szPathBak);

				//then rename existing file as .bak (to make room for new file)
				bool bRes = (0 == g_rename(szPath, szPathBak));
				if(!bRes)
				{
					//error creating backup file
					ASSERT(FALSE);
					g_remove(szPath);
				}
			}
			else
			{
				//quick FIX: delete target file if it already exists
				//TOFIX more precise fix would be to test/fix file open flags when saving
				g_remove(szPath);
			}

			//move temporary file to target path
			bool bRes = (0 == g_rename(strTargetPath.c_str(), szPath));
			if(!bRes)
				return DOC_SAVE_ERR_OVERWRITE; // Failed to owerwrite target file
		}

		if(bRememberPath)
			SetModified(false);	//just saved to known location
	}

	return nSaveRes;
}

bool NoteDocument::IsOpen()
{
	return (GetPath().size() > 0);
}

bool NoteDocument::IsEmpty()
{
	return (GetNodeCount() > 0);
}

bool NoteDocument::IsModified()
{
	return m_bModified;
}

void NoteDocument::SetModified(bool bModified)
{
	m_bModified = bModified;
	m_bModifiedSinceAutosave = bModified;
}

bool NoteDocument::IsReadOnly()
{
	return m_bReadOnly;
}

void NoteDocument::SetReadOnly(bool bReadOnly)
{
	m_bReadOnly = bReadOnly;
}

int NoteDocument::GetFormat()
{
	if(m_pObjLoader)
		return m_pObjLoader->GetFormat();
	return FORMAT_NONE;
}

bool NoteDocument::NodeInsert(int nParentID, int nSiblingIdx, bool bRefreshMap)
{
	DocumentIterator it(*this);

	int nCountBefore = GetNodeCount();

	NoteNode note;
	note.m_nID = m_nIDGenerator;
	note.m_nParentID = nParentID;

	//store time
	time_t nTime = time(NULL);
	note.m_nDateCreated  = nTime;
	note.m_nDateModified = nTime;

	if(nSiblingIdx < 0)
		note.m_nSiblingIdx = it.GetChildCount(nParentID);
	else
	{
		note.m_nSiblingIdx = nSiblingIdx;

		//refresh sibling indexes of the other sibling nodes
		for(unsigned int i=0; i<GetNodeCount(); i++)
		{
			if( m_lstNodes[i].m_nParentID == nParentID &&
				m_lstNodes[i].m_nSiblingIdx >= nSiblingIdx)
				m_lstNodes[i].m_nSiblingIdx ++;
		}
	}

	m_lstNodes.push_back(note);
	ASSERT((int)m_lstNodes.size() == nCountBefore + 1);

	if(bRefreshMap)
		m_mapId2Idx[m_nIDGenerator] = nCountBefore;

	m_nNodesCount  ++;
	m_nIDGenerator ++;
	SetModified(true);

	return true;
}

bool NoteDocument::NodeRename(int nID, const char *szTitle)
{
	//TOFIX not used, unfinished
	SetModified(true);
	return true;
}

bool NoteDocument::NodeDelete(int nID)
{
	int nIdx = GetIdxFromID(nID);
	if(nIdx < 0)
		return false;
	ASSERT(nIdx < (int)GetNodeCount());

	// store sibling and parent ID
	int nParentID   = GetNodeByIdx(nIdx).m_nParentID;
	int nSiblingIdx = GetNodeByIdx(nIdx).m_nSiblingIdx;

	//actual deletion
	NodeDeleteRecursive(nID);

	//refresh all top-level parent siblings
	unsigned int i;
	for(i=0; i<GetNodeCount(); i++)
	{
		if( GetNodeByIdx(i).m_nParentID == nParentID &&
		    GetNodeByIdx(i).m_nSiblingIdx > nSiblingIdx)
		{
			GetNodeByIdx(i).m_nSiblingIdx --;
		}
	}

#ifdef _DEBUG
	AssertValid();
#endif

	SetModified(true);
	return true;
}

void NoteDocument::NodeDeleteRecursive(int nID)
{
	//recursively delete all its children nodes
	int i = GetNodeCount()-1;
	while(i>=0)
	{
		if(GetNodeByIdx(i).m_nParentID == nID)
		{
			NodeDelete(GetNodeByIdx(i).m_nID);	//recurse into children node
			i = GetNodeCount()-1;	//scan again all over (index might have been changed)
		}
		else
			i--;
	}

	//now delete the node itself
	int nIdx = GetIdxFromID(nID);
	if(nIdx < 0)
		return;
	ASSERT(nIdx < (int)GetNodeCount());

	//erase the node
	m_lstNodes.erase(m_lstNodes.begin()+nIdx);
	m_nNodesCount --;

	//fix the Id2Idx map because deleting node
	//in the middle of the array changes index for everyone after that node
	std::map<int, int>::iterator it = m_mapId2Idx.find(nID);
	if(it != m_mapId2Idx.end())
		m_mapId2Idx.erase(it);						// remove mapping of deleted ID
	int nMax = GetNodeCount();
	for(i=nIdx; i<nMax; i++)
		m_mapId2Idx[GetNodeByIdx(i).m_nID] = i;		// refresh mapping of nodes behind the position being deleted
}

bool NoteDocument::NodeMove(int nID, int nNewParentID, int nSiblingIdx)
{
#ifdef _DEBUG
	TRACE("Move node ID=%d to PID=%d, SID=%d\n", nID, nNewParentID, nSiblingIdx);
	Dump();
#endif
	int nIdx = GetIdxFromID(nID);
	if(nIdx >= 0)
	{
		int nOldPID = GetNodeByIdx(nIdx).m_nParentID;
		int nOldSID = GetNodeByIdx(nIdx).m_nSiblingIdx;

		//redirect node position data
		GetNodeByIdx(nIdx).m_nParentID = nNewParentID;
		GetNodeByIdx(nIdx).m_nSiblingIdx = nSiblingIdx;

		if(nNewParentID == nOldPID)
		{
			//SIB algorithm when moving node within the same branch (a<b):
			//1. moving from a->b (to higher pos) -- where sib between a & b
			//2. moving from b->a (to lower  pos) ++ where sib between a & b
			bool bToHigherPos = nOldSID < nSiblingIdx;
			unsigned int i=0;

			if(bToHigherPos)
			{
				//decrement target sibling by one (because we deleted old position in lower pos than new)
				GetNodeByIdx(nIdx).m_nSiblingIdx --;
				nSiblingIdx --;

				for(i=0; i<GetNodeCount(); i++)
				{
					if(i != (unsigned int)nIdx && GetNodeByIdx(i).m_nParentID == nOldPID)
						if(GetNodeByIdx(i).m_nSiblingIdx >= nOldSID && GetNodeByIdx(i).m_nSiblingIdx <= nSiblingIdx)
							GetNodeByIdx(i).m_nSiblingIdx --;
				}
			}
			else
			{
				for(i=0; i<GetNodeCount(); i++)
				{
					if(i != (unsigned int)nIdx && GetNodeByIdx(i).m_nParentID == nOldPID)
						if(GetNodeByIdx(i).m_nSiblingIdx >= nSiblingIdx && GetNodeByIdx(i).m_nSiblingIdx <= nOldSID)
							GetNodeByIdx(i).m_nSiblingIdx ++;
				}
			}
		}
		else
		{
			//fix other sibling indexes for old position
			unsigned int i=0;
			for(i=0; i<GetNodeCount(); i++)
			{
				if(i != (unsigned int)nIdx && GetNodeByIdx(i).m_nParentID == nOldPID)
					if(GetNodeByIdx(i).m_nSiblingIdx >= nOldSID)
						GetNodeByIdx(i).m_nSiblingIdx --;
			}

			//fix other sibling indexes for new position
			for(i=0; i<GetNodeCount(); i++)
			{
				if(i != (unsigned int)nIdx && GetNodeByIdx(i).m_nParentID == nNewParentID)
					if(GetNodeByIdx(i).m_nSiblingIdx >= nSiblingIdx)
						GetNodeByIdx(i).m_nSiblingIdx ++;
			}
		}
	}

#ifdef _DEBUG
	TRACE("After node moving\n");
	Dump();
	AssertValid();
#endif

	SetModified(true);
	return true;
}

int NoteDocument::GetIdxFromID(int nID)
{
	std::map<int, int>::iterator it = m_mapId2Idx.find(nID);
	if(it != m_mapId2Idx.end())
		return (*it).second;

	return -1;
}

bool NoteDocument::Merge(NoteDocument &doc, int nParentID, int nSiblingIdx, bool bKeepIDs)
{
	DocumentIterator it(*this);

	int nIdOffset    = (bKeepIDs)? 0 : m_nIDGenerator;
	int nParentNodes = it.GetChildCount(nParentID);
	if(nSiblingIdx < 0)
		nSiblingIdx = nParentNodes;	// by default attach to the end of parent branch

	int nMaxNewID = -1;

	//append other object at the end of our
	int nCountExt = doc.GetNodeCount();
	for(int i=0; i<nCountExt; i++)
	{
		NoteNode &node = doc.GetNodeByIdx(i);

		//insert empty node in current doc
		if(node.m_nParentID == -1)
			NodeInsert(nParentID, node.m_nSiblingIdx + nSiblingIdx, false);
		else
			NodeInsert(-1, -1, false);
		int nIdx = GetNodeCount()-1;

		NoteNode &newNode = GetNodeByIdx(nIdx);


		if(bKeepIDs)
		{
			newNode.m_nID = node.m_nID;

			//preserve sibling index for non-root nodes
			if(node.m_nParentID >= 0)
				newNode.m_nSiblingIdx = node.m_nSiblingIdx;
		}
		else
		{
			newNode.m_nID += node.m_nID - i;
			if(newNode.m_nID > nMaxNewID)
				nMaxNewID = newNode.m_nID;

			ASSERT(newNode.m_nID >= nIdOffset);	//each new ID must be bigger than any existing before this op
		}

		//refresh map
		m_mapId2Idx[newNode.m_nID] = nIdx;

		//initialize it with other doc's data
		newNode.CopyState(node);

		//copy positioning info
		if(node.m_nParentID >= 0)
		{
			if(bKeepIDs)
				newNode.m_nParentID = node.m_nParentID; // non-root - attach to old/new parent (keeping old ID)
			else
				newNode.m_nParentID = node.m_nParentID + nIdOffset; // non-root - attach to old/new parent (having new ID)

			newNode.m_nSiblingIdx = node.m_nSiblingIdx;
		}
	}

	//make sure generator always creates unique IDs
	if(nMaxNewID >= m_nIDGenerator)
		m_nIDGenerator = nMaxNewID + 1;


#ifdef _DEBUG
	Dump();
	AssertValid();
#endif
	return true;
}

void NoteDocument::AssignSubtree(NoteDocument &doc, int nIdx, bool bKeepIDs)
{
	ASSERT(this != &doc);

	Clear(); //erase existing contents

	if(nIdx < 0)
		return;

	DocumentIterator it(doc);

	int nIdOffset = 0;
	int nSibOffset = - doc.GetNodeByIdx(nIdx).m_nSiblingIdx;

	int nMaxID = 0;

	//copy other object's subtree (starting at nIdx) to this object
	int nCount = doc.GetNodeCount();
	int i;
	for(i=0; i<nCount; i++)
	{
		if(!(nIdx==i || it.IsAncestorByIdx(nIdx, i)))
			continue;

		//insert empty node in current doc
		NodeInsert(-1, -1, false);
		int nIdxNew = GetNodeCount()-1;

		NoteNode &newNode = GetNodeByIdx(nIdxNew);
		NoteNode &node = doc.GetNodeByIdx(i);

		//initialize node data
		newNode.CopyState(node);

		if(bKeepIDs)
			newNode.m_nID = node.m_nID;

		m_mapId2Idx[newNode.m_nID] = nIdxNew;

		if(newNode.m_nID > nMaxID)
			nMaxID = newNode.m_nID;

		//copy positioning info
		if(nIdx == i)	// new root node
		{
			//make sure that the new tree is rooted (root nodes must have -1 for parent)
			newNode.m_nParentID	  = -1;
			newNode.m_nSiblingIdx = 0;
		}
		else
		{
			if(bKeepIDs)
			{
				newNode.m_nParentID	  = node.m_nParentID; // non-root - attach to old/new parent (keeping old ID)
				newNode.m_nSiblingIdx = node.m_nSiblingIdx;
			}
			else
			{
				if(node.m_nParentID >= 0)
					newNode.m_nParentID = node.m_nParentID + nIdOffset;
				else
					newNode.m_nParentID = -1;

				if(newNode.m_nParentID >= 0)
					newNode.m_nSiblingIdx = node.m_nSiblingIdx;
				else
					newNode.m_nSiblingIdx = node.m_nSiblingIdx + nSibOffset;
			}
		}
	}

	//fix generator if needed
	if(nMaxID >= m_nIDGenerator)
		m_nIDGenerator = nMaxID + 1;

#ifdef _DEBUG
	Dump();
	AssertValid();
#endif
}

//used for debugging
void NoteDocument::Dump()
{
#ifdef _DEBUG
	TRACE("ID:  PID, SIB :Title\n");

	int nCnt = GetNodeCount();
	for(int i=0; i<nCnt; i++)
	{
		NoteNode &node = GetNodeByIdx(i);

		TRACE("%3d: %3d, %3d :%s\n",
			node.m_nID,
			node.m_nParentID,
			node.m_nSiblingIdx,
			node.GetTitle().c_str());
	}
#endif
}

//
// debug code that test validity of the tree structure
// WARNING: some tests will give a false alarm if you run this
//			against some temporary tree objects like tree stored for Undo operation
//
void NoteDocument::AssertValid()
{
#ifdef _DEBUG
	int nCnt = GetNodeCount();
	ASSERT(nCnt == (int)m_lstNodes.size());	//cached value must match the real one
	if(nCnt < 1)
		return;

	DocumentIterator it(*this);

	//TEST 1: check if there exists at least 1 root node
	int nRootsCnt = 0;
	for(int i=0; i<nCnt; i++)
	{
		int nPID = GetNodeByIdx(i).m_nParentID;
		if(-1 == nPID)
			nRootsCnt++;
		else
		{
			//TEST 2: there must be no "hanging" nodes (parent ID must be valid)
			ASSERT(GetIdxFromID(nPID) >= 0); // is there a node with this PID in the tree

			//TEST 3: each ID value must be less than idgenerator value (ID of the future node)
			ASSERT(nPID < m_nIDGenerator);
		}

		int nID = GetNodeByIdx(i).m_nID;

		//TRACE("Testing node: %s\n", GetNodeByIdx(i).GetTitle().c_str());

		//TEST 3: each ID value must be less than idgenerator value (ID of the future node)
		ASSERT(nID < m_nIDGenerator);

		//TEST 4: each ID value must be >= 0
		ASSERT(nID >= 0);

		ASSERT(GetNodeByIdx(i).m_nSiblingIdx >= 0);

		//TEST 5: each node in the tree must have unique ID value
		for(int j=0; j<nCnt; j++){
			if(i != j)
				ASSERT(nID != GetNodeByIdx(j).m_nID);
		}

		//TEST 6: child nodes must have sibling Idx from 0 .. [child count - 1]
		int nChildCnt = it.GetChildCount(nID);
		for(int k=0; k<nChildCnt; k++){
			ASSERT(it.GetChildIdx(nID, k) >= 0);
		}

		//TEST 7: test if Idx value in the map object matches the real value
		ASSERT(i == GetIdxFromID(nID));

		//TEST 7: check if node formatting list is valid
		GetNodeByIdx(i).m_lstTxtFmt.AssertValid();
	}
	ASSERT(nRootsCnt > 0); // at least 1 root node

	//TEST 6: child nodes (for root) must have sibling Idx from 0 .. [child count - 1]
	int nChildCnt = it.GetChildCount(-1);
	for(int k=0; k<nChildCnt; k++){
		ASSERT(it.GetChildIdx(-1, k) >= 0);
	}

	//TOFIX add check for cyclic structures?, ...
#endif
}

int NoteDocument::GetTotalLinkCount()
{
	int nResult = 0;

	for(unsigned int i=0; i<GetNodeCount(); i++)
		nResult += m_lstNodes[i].m_lstLinks.size();

	return nResult;
}

void NoteDocument::GetSubtree(int nIdx, std::vector<NoteNode> &lstNodes, bool bClearList)
{
	if(bClearList)
		lstNodes.clear();

	if(nIdx >= 0)
	{
		lstNodes.push_back(m_lstNodes[nIdx]);	//push top node
		if(bClearList)
			lstNodes[0].m_nSiblingIdx = 0;		//reset root node sibling

		int nID = m_lstNodes[nIdx].m_nID;
		int nSize = m_nNodesCount;
		for(int i=0; i<nSize; i++)
		{
			if( i != nIdx &&
				m_lstNodes[i].m_nParentID == nID)
			{
				GetSubtree(i, lstNodes, false);
			}
		}
	}
}

void NoteDocument::AssignSubtree(std::vector<NoteNode> &lstNodes)
{
	m_lstNodes = lstNodes;
	m_nNodesCount = m_lstNodes.size();

	// rebuild map
	m_mapId2Idx.clear();
	for(int i=0; i<(int)GetNodeCount(); i++)
		m_mapId2Idx[GetNodeByIdx(i).m_nID] = i;

	//make sure tree is rooted (root node must have -1 parent)
	if(m_nNodesCount > 0)
	{
		m_lstNodes[0].m_nParentID = -1;
		m_lstNodes[0].m_nSiblingIdx = 0;		//reset root node sibling
	}
}

bool NoteDocument::MoveNodeLeft(int nIdx)
{
	int nParentID = GetNodeByIdx(nIdx).m_nParentID;
	if(nParentID > -1)	//parent exists
	{
		int nIdx2 = GetIdxFromID(nParentID);

		//change node position in the tree
		int nSIB = GetNodeByIdx(nIdx).m_nSiblingIdx;
		int nNewParentID = GetNodeByIdx(nIdx2).m_nParentID;
		GetNodeByIdx(nIdx).m_nParentID = nNewParentID;
		DocumentIterator itDoc(*this);

		//position new node as first sibling after its former parent!
		int nNewSIB = GetNodeByIdx(nIdx2).m_nSiblingIdx + 1;

		//refresh indexes of new siblings
		int i;
		int nCnt = GetNodeCount();
		for(i=0; i<nCnt; i++)
		{
			if( GetNodeByIdx(i).m_nParentID == nNewParentID &&
				GetNodeByIdx(i).m_nSiblingIdx >= nNewSIB && i != nIdx)
			{
				GetNodeByIdx(i).m_nSiblingIdx ++;
			}
		}

		//set new sibling idx for the node
		GetNodeByIdx(nIdx).m_nSiblingIdx = nNewSIB;

		//refresh indexes of previous siblings
		for(i=0; i<nCnt; i++)
		{
			if( GetNodeByIdx(i).m_nParentID == nParentID &&
				GetNodeByIdx(i).m_nSiblingIdx > nSIB)
			{
				GetNodeByIdx(i).m_nSiblingIdx --;
			}
		}

	#ifdef _DEBUG
		AssertValid();
	#endif
		return true;
	}

	return false;
}

bool NoteDocument::IsValidIndex(int nIdx)
{
	return (0 <= nIdx && nIdx < (int)GetNodeCount());
}

int NoteDocument::CalcNodeLevel(int nIdx)
{
	int nLevel = -1;

	while(IsValidIndex(nIdx))
	{
		nLevel ++;

		int nID = GetNodeByIdx(nIdx).m_nParentID;
		if(nID < 0)
			break;
		nIdx = GetIdxFromID(nID);
	}
	return nLevel;
}

//proper version string comparison ("1.5.9" < "1.5.10")
int cmpver(const char* szVer1, const char* szVer2)
{
	std::string strVer1(szVer1);
	std::string strVer2(szVer2);

	while(1){
		std::string strSegment1, strSegment2;

		//extract segment
		std::string::size_type nPos = strVer1.find(".");
		if(nPos != std::string::npos){
			strSegment1 = strVer1.substr(0, nPos);
			strVer1 = strVer1.substr(nPos+1);
		}
		else{
			strSegment1 = strVer1;
			strVer1 = "";
		}

		//extract segment2
		nPos = strVer2.find(".");
		if(nPos != std::string::npos){
			strSegment2 = strVer2.substr(0, nPos);
			strVer2 = strVer2.substr(nPos+1);
		}
		else{
			strSegment2 = strVer2;
			strVer2 = "";
		}

		//compare
		int nVer1 = 0;
		if(strSegment1.size() > 0)
			nVer1 = atoi(strSegment1.c_str());

		int nVer2 = 0;
		if(strSegment2.size() > 0)
			nVer2 = atoi(strSegment2.c_str());

		if(nVer1 > nVer2)
			return 1;
		if(nVer1 < nVer2)
			return -1;

		if(0 == strVer1.size() && 0 == strVer2.size())
			break;
	}

	return 0; //must be equal
}
