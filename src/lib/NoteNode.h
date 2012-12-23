////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Class describing single document node
////////////////////////////////////////////////////////////////////////////

#ifndef NOTENODE_H__
#define NOTENODE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LinkInfo.h"
#include "FmtInfo.h"
#include <gtk/gtk.h>

typedef struct PixInfo{
	GdkPixbuf *pixbuf;
	int nOffset;
	std::string strName;
	bool bUsePNG;	//prefer JPG, but allow PNG for translarent images

	bool operator <(const struct PixInfo &other) const { return (nOffset < other.nOffset); }
} PixInfo;

typedef struct{
	std::string m_strName;
	std::string m_strData;
	time_t nTime;
	int nDataSize;	//cached size
} AttInfo;

class DocAction;

//define icon types
#define ICON_NONE			-2
#define ICON_CUSTOM			-1
#define ICON_INTERNAL_FIRST 0
#define ICON_INTERNAL_LAST	5

class NoteNode
{
public:
	NoteNode();
	NoteNode(const NoteNode& other);
	virtual ~NoteNode();

	void operator =(const NoteNode& other);

	bool IsEmpty();
	void Clear();
	void CopyState(const NoteNode& other);

	inline std::string& GetTitle()    { return m_strTitle;    };
	inline std::string& GetText()     { return m_strText;     };
	inline std::string& GetKeywords() { return m_strKeywords; };
	inline std::string& GetSourceLanguage() { return m_strSourceLanguage; };

	void SetTitle(const char *szTitle);
	void SetText(const char *szText);
	void SetKeywords(const char *szText);
	void SetSourceLanguage(const char *szText);
	bool IsRootNode(){ return -1 == m_nParentID; }

	// get and set cursor position and selection bound
	void SetSelStart(unsigned int selStart);
	void SetSelEnd(unsigned int selEnd);
	unsigned int GetSelStart();
	unsigned int GetSelEnd();
	void ClearSel();
	void SetScrollPos(unsigned int nPos);
	unsigned int GetScrollPos();

	void OnTxtInsert(int nOffset, int nCharsAdded, const char *szTxt);
	void OnTxtReplaced(int nStart, int nEnd, const char *szTxt);
	void OnTxtDelete(int nOffset, int nCharsDeleted, int nTotalDeleted, DocAction *pDelAction = NULL);

	LinkInfoList m_lstLinks;	// hyperlinks within the text of the node
	LinkInfo	 m_objLink;		// hyperlink on the node title (invalid if the node title is not a link)
	FmtInfoList			 m_lstTxtFmt;	//rich text formatting info
	std::vector<PixInfo> m_lstPictures;
	std::vector<AttInfo> m_lstAttachments;

public:
	int m_nID;		// node ID value (unique in a single document during its entire lifetime)
	int m_nParentID;	// node parent ID value (defines node hierarchy)
	int m_nSiblingIdx;	// index that orders the nodes having the same parent

	//additional node attributes
	int m_nIconType;
	std::string m_strIconFile;

	time_t	m_nDateCreated;
	time_t	m_nDateModified;

	bool	m_bFinished;	//finished nodes have "strikethrough" font style
	bool	m_bExpanded;

protected:
	std::string m_strTitle;		// node title
	std::string m_strText;		// node text
	std::string m_strKeywords;	// keywords attached to the node (separated with ';' divider)
	std::string m_strSourceLanguage;	// node language (source code)

	// remember cursor position and selection bound
	unsigned int m_nSelStart, m_nSelEnd;
	unsigned int m_nScrollPos;				// remember scroll position
};

#endif // NOTENODE_H__
