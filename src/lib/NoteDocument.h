////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Class that stores/manipulates NoteCase document contents
////////////////////////////////////////////////////////////////////////////

#ifndef NOTEDOCUMENT_H__
#define NOTEDOCUMENT_H__

//
// Document contains a list of nodes that can be accessed in more than one way:
//  - by using index of the node in the node list (not unique, since on node deletion index is reused)
//  - by using node ID (garaneteed to be unique during the lifetime of the document)
//  - by using node's recursive index (depth first search order)
//
// Hierarchy of the nodes within the document is described with node (NodeNode class) properties:
//  - node parent ID (m_nParentID)
//     indicates node's parent node, or equals -1 if the node has no parent (root node)
//  - node sibling index (m_nSiblingIdx)
//     defines ordering of the nodes having the same parent. Node with lower sibling index
//     comes before the node with higher sibling index and having the same parent node
//
// Several document formats are supported (all are implemented by inheriting from FormatIO_Base base class).
// Some formats are used only for importing and have no write support.
// Basic .ncd format is HTML based, and inherits HtmlParser class (overriding some parser event handlers)
//

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//TOFIX add current node idx inside ?

#include "FormatIOBase.h"
#include "NoteNode.h"
#include <vector>
#include <string>
#include <map>
#include "debug.h"

class FormatIO_Base;

typedef const char *(* PASS_CALLBACK)(const char *, bool);
typedef int (* FMT_CALLBACK)();

//supported formats
#define FORMAT_NONE					0
#define FORMAT_NOTECENTER_HTML		1	// .hnc
#define FORMAT_NOTECASE_HTML		2	// .ncd
#define FORMAT_NOTECASE_HTML_ENC	3	// .nce
#define FORMAT_GJOTS2				4	// .gjots2
#define FORMAT_STICKY				5	// .xml
#define FORMAT_TXT					6	// .txt
#define FORMAT_HTML					7	// .html
#define FORMAT_MMLX					8	// .MM
#define FORMAT_EXECUTABLE			9	// .exe on Windows

//define Load operation results
#define DOC_LOAD_NOT_SUPPORTED -1	//load not imlplemented
#define DOC_LOAD_OK             0	//OK
#define DOC_LOAD_ABORTED        1	//aborted loading new document
#define DOC_LOAD_NOT_FOUND      2	//file not found
#define DOC_LOAD_WRONG_PASSWORD 3	//invalid password
#define DOC_LOAD_UNKNOWN_FORMAT 4	//unsuported format
#define DOC_LOAD_FORMAT_ERROR   5	//parse error
#define DOC_LOAD_PERMISSIONS    6	//no read permissions
#define DOC_LOAD_ERROR          7	//other error

//define Save operation results
#define DOC_SAVE_NOT_SUPPORTED -1	//load not imlplemented
#define DOC_SAVE_OK				0	//OK
#define DOC_SAVE_ABORTED        1	//aborted saving new document
#define DOC_SAVE_UNKNOWN_FORMAT 2	//unsuported format
#define DOC_SAVE_ERR_OVERWRITE	3	//error overwriting destination file (two step saving)
#define DOC_SAVE_ERR_FILE_OPEN	4	//error opening destination file
#define DOC_SAVE_ERR_EXE_OPEN	5	//error opening your own executable
#define DOC_SAVE_ERROR			6	//other error

class NoteDocument
{
	friend class DocumentIterator;

public:
	NoteDocument();
	virtual ~NoteDocument();

	NoteDocument(const NoteDocument &doc);
	void operator = (const NoteDocument &doc);

	void AssignSubtree(NoteDocument &doc, int nIdx, bool bKeepIDs = false);
	void AssignSubtree(std::vector<NoteNode> &lstNodes);

	int  GetFormatFromFileName(const char *szPath);
	int  Load(const char *szPath, const char *szPassword = NULL, int nFormat = FORMAT_NONE);
	int  Save(const char *szPath, bool bRememberPath = true, int nBackupFiles = 0, const char *szPassword = NULL, int nFormat = FORMAT_NONE);
	void Close();

	void Dump();		//debugging helper
	void AssertValid();	//debugging helper

	void Clear();

	bool IsOpen();
	bool IsEmpty();
	bool IsModified();
	void SetModified(bool bModified);
	bool IsReadOnly();
	void SetReadOnly(bool bReadOnly);
	bool IsEncrypted();
	const char *GetPassword(){ return m_strPassword.c_str(); }
	void SetPassword(const char *szPass){ m_strPassword = szPass; }

	void SetPassCallback(PASS_CALLBACK pfn){ m_pfnPassCallback = pfn; }
	void SetFmtCallback(FMT_CALLBACK pfn){ m_pfnFmtCallback = pfn; }

	int  GetFormat();
	std::string	 GetPath(){ return m_strPath; };	//if attached to file
	void ResetPath(){ m_strPath = ""; };

	bool NodeInsert(int nParentID, int nSiblingIdx, bool bRefreshMap = true);
	bool NodeRename(int nID, const char *szTitle);
	bool NodeDelete(int nID);
	bool NodeMove(int nID, int nNewParentID, int nSiblingIdx);
	bool MoveNodeLeft(int nIdx);

	inline unsigned int GetNodeCount() const { return m_nNodesCount; };

	inline NoteNode &GetNodeByIdx(unsigned int nIdx){
		ASSERT(nIdx < GetNodeCount());
		return m_lstNodes[nIdx];
	}

	int GetTotalLinkCount();

	bool IsValidIndex(int nIdx);
	int  CalcNodeLevel(int nIdx);	//0..x (0 for root node)
	int  GetIdxFromID(int nID);
	bool Merge(NoteDocument &doc, int nParentID = -1, int nSiblingIdx = -1, bool bKeepIDs = false);

	void GetSubtree(int nIdx, std::vector<NoteNode> &lstNodes, bool bClearList = true);

	int  m_nCurrentNode;					//initialy active node (recursive index)
	int  m_nActiveNodeIdx;					//currently active node index
	int  m_nEmbedCSS;						//is CSS embedded (or linked), -1 means to read from INI
	std::string m_strCustomCSS;				//custom CSS path written on HTML export
	std::map<int, int>	m_mapIdIsTarget;	//how many internal links point to some node ID

	bool m_bModifiedSinceAutosave;			//any change since last autosave ?

protected:
	void NodeDeleteRecursive(int nID);

protected:
	FormatIO_Base *m_pObjLoader;
	std::vector<NoteNode> m_lstNodes;
	std::map<int, int>	m_mapId2Idx;		//fast conversion from ID to Idx
	unsigned int m_nNodesCount;
	bool m_bModified;
	bool m_bReadOnly;
	int  m_nIDGenerator;
	std::string	m_strPath;
	std::string	m_strPassword;
	PASS_CALLBACK m_pfnPassCallback;
	FMT_CALLBACK m_pfnFmtCallback;			// ast to choose format for the unknown .ext
};

#endif // NOTEDOCUMENT_H__
