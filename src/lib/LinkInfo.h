////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Stores info for the single hyperlink
////////////////////////////////////////////////////////////////////////////

#ifndef LINKINFO_H_
#define LINKINFO_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if _MSC_VER > 1000
  #pragma warning(disable:4786)
#endif

#include <string>
#include <vector>

class LinkInfo
{
public:
	LinkInfo();
	LinkInfo(const LinkInfo &other);
	virtual ~LinkInfo();

	bool IsValid();
	void Clear();

	void operator = (const LinkInfo &other);
	void RefreshLength();

	std::string m_strText;
	std::string m_strTargetURL;
	int			m_nTargetNodeID;

	//position/size in utf8 character (NOT buffer offset)
	int  m_nStartOffset;
	int  m_nTextLength;
};

class LinkInfoList : public std::vector<LinkInfo>
{
public:
	LinkInfoList();
	virtual ~LinkInfoList();

	void Sort();

	int  Find(int nCharOffset);
	bool IsRangeOccupied(int nCharOffset, int nLen);	//any link in the range

	//API to work with ranges
	static bool RangesOverlap(int nA, int nB, int nC, int nD);

	//adapt to events within the node text buffer
	void OnTxtInsert(int nOffset, int nCharsAdded, const char *szTxt);
	void OnTxtReplaced(int nStart, int nEnd, const char *szTxt);
	void OnTxtDelete(int nOffset, int nCharsDeleted);
	void OnLinkCreated(int nOffset, int nLen);
};

#endif // LINKINFO_H_
