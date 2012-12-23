////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Class to store state for a single Find/Replace operation
////////////////////////////////////////////////////////////////////////////

#ifndef FINDREPLACEINFO_H_
#define FINDREPLACEINFO_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>

class FindReplaceInfo
{
public:
	FindReplaceInfo();
	virtual ~FindReplaceInfo();

	void Clear();

public:
	//search info
	std::string m_strFindText;
	std::string m_strReplaceText;
	bool m_bSearchKeywords;
	bool m_bFindSensitive;
	bool m_bFindDirectionDown;

	//search state
	int  m_bFindInTitle;
	bool m_bFirstNodeSearched;		// detects return on the first search node
	int  m_nStartNodeRecursiveIdx;
	int  m_nCurNodeRecursiveIdx;
	int  m_nFindBufferPos;
	int  m_nLastResultIdx;
	int  m_nLastResultPos;
};

#endif // FINDREPLACEINFO_H_
