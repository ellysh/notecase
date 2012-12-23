////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Window class for Find/Replace dialog
////////////////////////////////////////////////////////////////////////////

#include "FindReplaceInfo.h"

FindReplaceInfo::FindReplaceInfo()
{
	Clear();
}

FindReplaceInfo::~FindReplaceInfo()
{
}

void FindReplaceInfo::Clear()
{
	m_strFindText = "";
	m_bFindInTitle = true;
	m_bFindSensitive = true;
	m_bSearchKeywords = false,
	m_bFindDirectionDown = true;
	m_bFirstNodeSearched = false;
	m_nStartNodeRecursiveIdx = -1;
	m_nCurNodeRecursiveIdx = -1;
	m_nFindBufferPos = 0;
	m_nLastResultIdx = -1;
	m_nLastResultPos = -1;
}
