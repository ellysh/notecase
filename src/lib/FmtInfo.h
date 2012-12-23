////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Stores text formatting info
////////////////////////////////////////////////////////////////////////////

#ifndef FMTINFO_H_
#define FMTINFO_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if _MSC_VER > 1000
  #pragma warning(disable:4786)
#endif

#include <string>
#include <vector>
#include <gtk/gtk.h>

//define format tags
#define FMT_NONE			0
#define FMT_BOLD_BEGIN		1
#define FMT_BOLD_END		2
#define FMT_ITALIC_BEGIN	3
#define FMT_ITALIC_END		4
#define FMT_UNDERLINE_BEGIN	5
#define FMT_UNDERLINE_END	6
#define FMT_TXT_COLOR_BEGIN	7
#define FMT_TXT_COLOR_END	8
#define FMT_STRIKE_BEGIN	9
#define FMT_STRIKE_END		10
#define FMT_TXT_BKG_COLOR_BEGIN	15
#define FMT_TXT_BKG_COLOR_END	16

class FmtInfo
{
public:
	FmtInfo();
	FmtInfo(const FmtInfo &other);
	virtual ~FmtInfo();

	void operator = (const FmtInfo &other);

public:
	int nFormatTag;
	GdkColor color;
	int nOffset;
};

class FmtInfoList : public std::vector<FmtInfo>
{
public:
	FmtInfoList();
	virtual ~FmtInfoList();

	void operator =(const std::vector<FmtInfo> &other);

	//TOFIX move to FmtInfo?
	static inline bool IsStartTag(int nFormatTag){
		return (nFormatTag%2 == 1); //odd code belongs to start tags
	}
	static inline int  CalcFormatEndTag(int nFormatTag){
		if(IsStartTag(nFormatTag))
			return nFormatTag+1;
		return -1;
	}
	static inline int	CalcFormatStartTag(int nFormatTag){
		if(!IsStartTag(nFormatTag))
			return nFormatTag-1;
		return -1;
	}
	static inline int	CalcOppositeTag(int nFormatTag){
		if(IsStartTag(nFormatTag))
			return CalcFormatEndTag(nFormatTag);
		return CalcFormatStartTag(nFormatTag);
	}

	void Sort();

	bool IsRangeFormatted(int nStart, int nEnd, int nFormatTag);
	void AddFormatRange(int nStart, int nEnd, int nFormatTag, const GdkColor& color);
	void ClearFormatRange(int nStart, int nEnd, int nFormatTag);

	//adapt to events within the node text buffer
	void OnTxtInsert(int nOffset, int nCharsAdded, const char *szTxt);
	void OnTxtReplaced(int nStart, int nEnd, const char *szTxt);
	void OnTxtDelete(int nOffset, int nCharsDeleted);
	void OnLinkCreated(int nOffset, int nLen);

	int FindOppositeTag(int nFormatTag, int nStart, bool bForward, bool bNoMiddleTags, int nAbortTag = -1);

	int GetTagCount(int nFormatTag);
	void AssertValid();	//debugging helper
};

#endif // FMTINFO_H_
