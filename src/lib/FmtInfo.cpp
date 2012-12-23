////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Stores text formatting info
////////////////////////////////////////////////////////////////////////////

#include "FmtInfo.h"
#include "../TextView.h"
#include <algorithm>
#include <glib.h>
#include "debug.h"
#include <string.h>

extern TextView g_text;

void textview_selection_changed(GtkTextBuffer *textbuffer, GtkTextIter *arg1, GtkTextMark *arg2, gpointer user_data);
bool match_color(const GdkColor& color1, const GdkColor& color2);

//used to sort FmtInfoList class (always ascending by offset)
class FmtComparator{
public:
	bool operator()(const FmtInfo &a, const FmtInfo &b);
};

bool FmtComparator::operator()(const FmtInfo &a, const FmtInfo &b)
{
	//when on the same offset, we put the ending tag first (end of previous range)
	if(a.nOffset == b.nOffset)
		return !FmtInfoList::IsStartTag(a.nFormatTag);

	return (a.nOffset < b.nOffset);
}

FmtInfo::FmtInfo()
{
	nFormatTag = -1;
	nOffset = -1;
}

FmtInfo::~FmtInfo()
{
}

FmtInfo::FmtInfo(const FmtInfo &other)
{
	operator = (other);
}

void FmtInfo::operator = (const FmtInfo &other)
{
	if(this != &other){
		nFormatTag	= other.nFormatTag;
		nOffset		= other.nOffset;
		color		= other.color;
	}
}


FmtInfoList::FmtInfoList()
{
}

FmtInfoList::~FmtInfoList()
{
}

void FmtInfoList::operator =(const std::vector<FmtInfo> &other)
{
	*((std::vector<FmtInfo> *)this) = other;
	ASSERT(size() == other.size());
}

bool FmtInfoList::IsRangeFormatted(int nStart, int nEnd, int nFormatTag)
{
	int nEndTag = CalcFormatEndTag(nFormatTag);
	if(nEndTag < 0)
		return false;

	//find range that could span over this area
	int nSearchStart = 0;
	int nSize = size();
	int nIdxStart = -1;
	int nIdxEnd	  = -1;

	while(1)
	{
		//find start tag
		int i;
		for(i=nSearchStart; i<nSize; i++){
			if(operator[](i).nFormatTag == nFormatTag){
				nIdxStart = i; break;
			}
		}
		if(nIdxStart < 0)
			return false;

		//find matching end tag
		for(i=nIdxStart+1; i<nSize; i++){
			if(operator[](i).nFormatTag == nEndTag){
				nIdxEnd = i; break;
			}
		}
		if(nIdxEnd < 0)
			return false;

		if(operator[](nIdxEnd).nOffset >= nStart)
		{
			if(operator[](nIdxStart).nOffset <= nStart && operator[](nIdxEnd).nOffset >= nEnd)
				return true;
			else
				return false;
		}
		else
		{
			nSearchStart = nIdxEnd+1;
			nIdxStart = -1;
			nIdxEnd	  = -1;
		}
	}

	return false;
}

void FmtInfoList::AddFormatRange(int nStart, int nEnd, int nFormatTag, const GdkColor& color)
{
	if(FMT_NONE != nFormatTag)
		ClearFormatRange(nStart, nEnd, nFormatTag);

	//
	// mark the text
	//
	GtkTextView *textview =  (GtkTextView *)g_text.m_pWidget;
	GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);
	//get the iterators
	GtkTextIter cursIter, boundIter;
	gtk_text_buffer_get_iter_at_offset(buffer1, &cursIter, nStart);
	gtk_text_buffer_get_iter_at_offset(buffer1, &boundIter, nEnd);

	if(FMT_BOLD_BEGIN == nFormatTag)
	{
		//"boldtag"
		GtkTextTag *tag = gtk_text_buffer_create_tag (buffer1, NULL,
				"weight", PANGO_WEIGHT_BOLD,
				NULL);
		gtk_text_buffer_apply_tag(buffer1, tag, &cursIter, &boundIter);

		FmtInfo info;
		info.nFormatTag = FMT_BOLD_BEGIN;
		info.nOffset	= nStart;
		push_back(info);
		info.nFormatTag = FMT_BOLD_END;
		info.nOffset	= nEnd;
		push_back(info);
	}
	else if(FMT_ITALIC_BEGIN == nFormatTag)
	{
		//"italictag"
		GtkTextTag *tag = gtk_text_buffer_create_tag (buffer1, NULL,
				"style", PANGO_STYLE_ITALIC,
				NULL);
		gtk_text_buffer_apply_tag(buffer1, tag, &cursIter, &boundIter);

		FmtInfo info;
		info.nFormatTag = FMT_ITALIC_BEGIN;
		info.nOffset	= nStart;
		push_back(info);
		info.nFormatTag = FMT_ITALIC_END;
		info.nOffset	= nEnd;
		push_back(info);
	}
	else if(FMT_UNDERLINE_BEGIN == nFormatTag)
	{
		//"underlinetag"
		GtkTextTag *tag = gtk_text_buffer_create_tag (buffer1, NULL,
				"underline", PANGO_UNDERLINE_SINGLE,
				NULL);
		gtk_text_buffer_apply_tag(buffer1, tag, &cursIter, &boundIter);

		FmtInfo info;
		info.nFormatTag = FMT_UNDERLINE_BEGIN;
		info.nOffset	= nStart;
		push_back(info);
		info.nFormatTag = FMT_UNDERLINE_END;
		info.nOffset	= nEnd;
		push_back(info);
	}
	else if(FMT_STRIKE_BEGIN == nFormatTag)
	{
		//"strikethroughtag"
		GtkTextTag *tag = gtk_text_buffer_create_tag (buffer1, NULL,
							"strikethrough", TRUE,
							NULL);
		gtk_text_buffer_apply_tag(buffer1, tag, &cursIter, &boundIter);

		FmtInfo info;
		info.nFormatTag = FMT_STRIKE_BEGIN;
		info.nOffset	= nStart;
		push_back(info);
		info.nFormatTag = FMT_STRIKE_END;
		info.nOffset	= nEnd;
		push_back(info);
	}
	else if(FMT_TXT_COLOR_BEGIN == nFormatTag)
	{
		char szBuffer[200];
		sprintf(szBuffer, "#%04x%04x%04x", color.red, color.green, color.blue);

		//"colortag"
		GtkTextTag *tag = gtk_text_buffer_create_tag (buffer1, NULL,
				"foreground", szBuffer,
				NULL);
		gtk_text_buffer_apply_tag(buffer1, tag, &cursIter, &boundIter);

		FmtInfo info;
		info.color      = color;
		info.nFormatTag = FMT_TXT_COLOR_BEGIN;
		info.nOffset	= nStart;
		push_back(info);
		info.nFormatTag = FMT_TXT_COLOR_END;
		info.nOffset	= nEnd;
		push_back(info);
	}
	else if(FMT_TXT_BKG_COLOR_BEGIN == nFormatTag)
	{
		char szBuffer[200];
		sprintf(szBuffer, "#%04x%04x%04x", color.red, color.green, color.blue);

		//"bkg colortag"
		GtkTextTag *tag = gtk_text_buffer_create_tag (buffer1, NULL,
				"background", szBuffer,
				NULL);
		gtk_text_buffer_apply_tag(buffer1, tag, &cursIter, &boundIter);

		FmtInfo info;
		info.color      = color;
		info.nFormatTag = FMT_TXT_BKG_COLOR_BEGIN;
		info.nOffset	= nStart;
		push_back(info);
		info.nFormatTag = FMT_TXT_BKG_COLOR_END;
		info.nOffset	= nEnd;
		push_back(info);
	}
	else if(FMT_NONE == nFormatTag)
	{
		//clear all possible formatting
		ClearFormatRange(nStart, nEnd, FMT_BOLD_BEGIN);
		ClearFormatRange(nStart, nEnd, FMT_ITALIC_BEGIN);
		ClearFormatRange(nStart, nEnd, FMT_UNDERLINE_BEGIN);
		ClearFormatRange(nStart, nEnd, FMT_STRIKE_BEGIN);
		ClearFormatRange(nStart, nEnd, FMT_TXT_COLOR_BEGIN);
		ClearFormatRange(nStart, nEnd, FMT_TXT_BKG_COLOR_BEGIN);
	}

	Sort();

	//merge new range with any neighbouring/overlapping ranges
	//FMT_TXT_COLOR_BEGIN must be matched by color too
	if(FMT_NONE != nFormatTag){
		int nEndTag = CalcFormatEndTag(nFormatTag);
		int nCnt = size();
		int nPosStart = -1;
		//find starting pos for current tag
		int i;
		for(i=0; i<nCnt; i++){
			if(operator[](i).nOffset > nStart)
				break;
			if(operator[](i).nFormatTag == nFormatTag &&
			   ((nFormatTag != FMT_TXT_COLOR_BEGIN && nFormatTag != FMT_TXT_BKG_COLOR_BEGIN) || match_color(operator[](i).color, color)) &&
			   operator[](i).nOffset == nStart){
				nPosStart = i;
			}
		}
		ASSERT(nPosStart >= 0);
		//if you find similar tag before it, destroy this start tag
		//(there must no be any end tag between them (unless end tag is on the same pos))
		for(i=nPosStart-1; i>=0; i--){
			if(operator[](i).nFormatTag == nEndTag &&
			   ((nEndTag != FMT_TXT_COLOR_END && nEndTag != FMT_TXT_BKG_COLOR_END) || match_color(operator[](i).color, color))){
				if(operator[](i).nOffset == nStart){
					erase(begin()+i); nPosStart--; break; //
				}
				else
					break;
			}
			else if(operator[](i).nFormatTag == nFormatTag &&
				    ((nFormatTag != FMT_TXT_COLOR_BEGIN && nFormatTag != FMT_TXT_BKG_COLOR_BEGIN) || match_color(operator[](i).color, color)))
			{
				erase(begin()+nPosStart); nPosStart--; break; //
			}
		}
		//now handle situation of tag inside tag -> merge with wider tag around
		//example: <b>some text<b>more</b>text</b>
		for(i=nPosStart-1; i>=0; i--){
			if(operator[](i).nFormatTag == nEndTag &&
			   ((nFormatTag != FMT_TXT_COLOR_BEGIN && nFormatTag != FMT_TXT_BKG_COLOR_BEGIN) || match_color(operator[](i).color, color)))
				break;
			else if(operator[](i).nFormatTag == nFormatTag &&
			       	((nFormatTag != FMT_TXT_COLOR_BEGIN && nFormatTag != FMT_TXT_BKG_COLOR_BEGIN) || match_color(operator[](i).color, color)))
			{
				erase(begin()+nPosStart); nPosStart=i; break; //
			}
		}

		int nPosEnd = -1;
		//find ending pos for current tag
		nCnt = size();
		for(i=0; i<nCnt; i++){
			if(operator[](i).nOffset > nEnd)
				break;
			if(operator[](i).nFormatTag == nEndTag &&
			   ((nEndTag != FMT_TXT_COLOR_END && nEndTag != FMT_TXT_BKG_COLOR_END) || match_color(operator[](i).color, color)) &&
			   operator[](i).nOffset == nEnd)
			{
				nPosEnd = i; break;
			}
		}
		ASSERT(nPosEnd >= 0);
		//search any similar tag after it and destroy this end tag
		//(there must no be any start tag between them (unless end tag is on the same pos))
		for(i=nPosEnd+1; i<nCnt; i++){
			if(operator[](i).nFormatTag == nFormatTag &&
			   ((nFormatTag != FMT_TXT_COLOR_BEGIN && nFormatTag != FMT_TXT_BKG_COLOR_BEGIN) || match_color(operator[](i).color, color))){
				if(operator[](i).nOffset == nEnd)
				{
					erase(begin()+i); break; //
				}
				else
					break;
			}
			else if(operator[](i).nFormatTag == nEndTag &&
					((nEndTag != FMT_TXT_COLOR_END && nEndTag != FMT_TXT_BKG_COLOR_END) || match_color(operator[](i).color, color)))
			{
				erase(begin()+nPosEnd); nPosEnd=i-1; break; //
			}
		}
		//now handle situation of tag inside tag -> merge with wider tag around
		//example: <b>some text<b>more</b>text</b>
		nCnt = size();
		for(i=nPosEnd+1; i<nCnt; i++){
			if(operator[](i).nFormatTag == nFormatTag &&
			   ((nFormatTag != FMT_TXT_COLOR_BEGIN && nFormatTag != FMT_TXT_BKG_COLOR_BEGIN) || match_color(operator[](i).color, color)))
				break;
			else if(operator[](i).nFormatTag == nEndTag && ((nFormatTag != FMT_TXT_COLOR_BEGIN && nFormatTag != FMT_TXT_BKG_COLOR_BEGIN) || match_color(operator[](i).color, color)))
			{
				erase(begin()+nPosEnd); nPosEnd=i-1; break; //
			}
		}

		//destroy any start/end tag inside the new range
		nCnt = size();
		for(i=0; i<nCnt; i++){
			if((operator[](i).nFormatTag == nFormatTag ||
			   operator[](i).nFormatTag == nEndTag) &&
			   (operator[](i).nOffset > nStart &&
			    operator[](i).nOffset < nEnd))
			{
				erase(begin()+i); i--; nCnt--; //
			}
		}
	}

#ifdef _DEBUG
	AssertValid();
#endif
}

void FmtInfoList::ClearFormatRange(int nStart, int nEnd, int nFormatTag)
{
	int nEndTag = CalcFormatEndTag(nFormatTag);
	if(nEndTag < 0)
		return;

	//STEP 1: find first start format tag before the start
	int nPosStart  = -1;
	int i;
	int nCnt = size();
	for(i=0; i<nCnt; i++)
	{
		if(operator[](i).nFormatTag == nFormatTag)
		{
			if(operator[](i).nOffset < nStart)
				nPosStart = i;	// mark current favorite
			else
				break;	//passed the offset (list is sorted)
		}
	}

	if(nPosStart >= 0)
	{
		//see that there is no end tag between (unless it is on nStart offset)
		for(i=nPosStart+1; i<nCnt; i++)
		{
			if(operator[](i).nOffset > nStart)
				break;
			if(operator[](i).nFormatTag == nEndTag)
			{
				if(operator[](i).nOffset == nStart)
					break;	// found ending that starts exactly on offset
				else{
					nPosStart = -1;	//there is an ending tag between, no overlapping
					break;			//passed the offset (list is sorted)
				}
			}
		}
	}

	//STEP 2: find first end format tag after the end of new tag
	int nPosEnd = -1;
	for(i=0; i<nCnt; i++)
	{
		if(operator[](i).nFormatTag == nEndTag)
		{
			if(operator[](i).nOffset > nEnd){
				nPosEnd = i;	// mark current favorite
				break;
			}
		}
	}

	if(nPosEnd >= 0)
	{
		//see that there is no start tag between (unless it is on nEnd offset)
		for(i=nPosEnd-1; i>=0; i--)
		{
			if(operator[](i).nOffset > nEnd)
			{
				if(operator[](i).nFormatTag == nFormatTag)
				{
					if(operator[](i).nOffset == nEnd)
						break;	// found new start that starts exactly on offset
					else{
						nPosEnd = -1;	//there is an new start tag between, no overlapping
						break;			//passed the offset (list is sorted)
					}
				}
			}
		}
	}

	//now remove all start/end tags for this tag that are in the inside of the range
	for(i=0; i<nCnt; i++)
	{
		if( (operator[](i).nFormatTag == nFormatTag ||
			 operator[](i).nFormatTag == nEndTag) &&
			operator[](i).nOffset >= nStart &&
			operator[](i).nOffset <= nEnd)
		{
			if(i == nPosEnd)
				nPosEnd = -1;
			else
				nPosEnd --;
			erase(begin()+i); nCnt--; i--;
		}
	}

	if(nPosStart >= 0)
	{
		if(operator[](nPosStart).nOffset != nStart)
		{
			//if previous start pos found (that we overlap), insert new end tag to break the tag that we overlap
			FmtInfo info;
			info.nFormatTag = nEndTag;
			info.nOffset = nStart;
			info.color = operator[](nPosStart).color;
			push_back(info);	//pushed at the end, no need to nPosEnd++;
		}
	}

	if(nPosEnd >= 0)
	{
		if(operator[](nPosEnd).nOffset != nEnd)
		{
			//if next end pos found (that we overlap), insert new start tag to break the tag that we overlap
			FmtInfo info;
			info.nFormatTag = nFormatTag;
			info.nOffset = nEnd;
			info.color = operator[](nPosEnd).color;
			push_back(info);
		}
	}

	Sort();

#ifdef _DEBUG
	AssertValid();
#endif
}

void FmtInfoList::Sort()
{
    FmtComparator cmp;
    std::sort(begin(), end(), cmp);
}

void FmtInfoList::OnTxtInsert(int nOffset, int nCharsAdded, const char *szTxt)
{
	int nCount = size();
	for(int i=0; i<nCount; i++)
	{
		FmtInfo &info = operator[](i);

		//contents added before the tag
		if((nOffset <= info.nOffset) /*||
		   ((nOffset == info.nOffset) && FmtInfoList::IsStartTag(info.nFormatTag))*/)
			info.nOffset += nCharsAdded;
	}

#ifdef _DEBUG
	AssertValid();
#endif
}

void FmtInfoList::OnTxtDelete(int nOffset, int nCharsDeleted)
{
	int nCount = size();
	for(int i=0; i<nCount; i++)
	{
		FmtInfo &info = operator[](i);

		//shorten or remove the tags in this range
		//when contents are being deleted inside this tag
		if(nOffset <= info.nOffset)
		{
			if(nOffset + nCharsDeleted <= info.nOffset)
				info.nOffset -= nCharsDeleted;
			else{
				//should tag be deleted together with text or just moved
				if(IsStartTag(info.nFormatTag)){
					//find matching end tag
					int nEndTag = CalcFormatEndTag(info.nFormatTag);
					int nEndIdx = -1;
					for(int j=i+1; j<nCount; j++){
						if(operator[](j).nFormatTag == nEndTag){
							nEndIdx = j; break;
						}
					}
					ASSERT(nEndIdx > 0);
					if(nEndIdx > 0){
						if(nOffset + nCharsDeleted < operator[](nEndIdx).nOffset)
							info.nOffset = nOffset;	//just move start tag to the new pos
						else{
							//delete entire tag (both start and end)
							erase(begin()+nEndIdx);
							erase(begin()+i);
							i --; nCount -= 2;
						}
					}
				}
				else
					info.nOffset = nOffset;	//just move end tag to the new pos
			}
		}
	}

	textview_selection_changed(NULL, NULL, NULL, NULL);

#ifdef _DEBUG
	AssertValid();
#endif
}

void FmtInfoList::OnLinkCreated(int nOffset, int nLen)
{
	//TOFIX remove any formatting on the link range?
}

void FmtInfoList::OnTxtReplaced(int nStart, int nEnd, const char *szTxt)
{
	//int nSizeNew  = g_utf8_strlen(szTxt, -1);
	//OnTxtDelete(nStart, nEnd-nStart);
	//OnTxtInsert(nStart, nSizeNew, szTxt);

	int nSizeNew  = g_utf8_strlen(szTxt, -1);
	int nSizeDiff = nSizeNew - (nEnd - nStart);

	int nCount = size();
	for(int i=0; i<nCount; i++)
	{
		FmtInfo &info = operator[](i);

		if(nEnd < info.nOffset)
		{
			//contents added before the tag
			info.nOffset += nSizeDiff;
		}
		else if(nStart <= info.nOffset && nEnd >= info.nOffset)
		{
			//contents covering also tag area
			info.nOffset = nStart + nSizeDiff;
		}
	}

#ifdef _DEBUG
	AssertValid();
#endif
}

int FmtInfoList::GetTagCount(int nFormatTag)
{
	int nResult = 0;

	int nMaxCnt = size();
	for(int i=0; i<nMaxCnt; i++)
		if(operator[](i).nFormatTag == nFormatTag)
			nResult ++;

	return nResult;
}

void FmtInfoList::AssertValid()
{
#ifdef _DEBUG
	ASSERT(GetTagCount(FMT_BOLD_BEGIN)		== GetTagCount(FMT_BOLD_END));
	ASSERT(GetTagCount(FMT_ITALIC_BEGIN)	== GetTagCount(FMT_ITALIC_END));
	ASSERT(GetTagCount(FMT_UNDERLINE_BEGIN) == GetTagCount(FMT_UNDERLINE_END));
	ASSERT(GetTagCount(FMT_STRIKE_BEGIN)	== GetTagCount(FMT_STRIKE_END));
	ASSERT(GetTagCount(FMT_TXT_COLOR_BEGIN) == GetTagCount(FMT_TXT_COLOR_END));
	ASSERT(GetTagCount(FMT_TXT_BKG_COLOR_BEGIN) == GetTagCount(FMT_TXT_BKG_COLOR_END));
	//TOFIX more checks?, each tag must be terminated
#endif
}

bool match_color(const GdkColor& color1, const GdkColor& color2)
{
	return (color1.red == color2.red && color1.blue == color2.blue && color1.green == color2.green);
}

int FmtInfoList::FindOppositeTag(int nFormatTag, int nStart, bool bForward, bool bNoMiddleTags, int nAbortTag)
{
	ASSERT(0 <= nStart && nStart < (int)size());

	int	nSearchTag = CalcOppositeTag(nFormatTag);
	if(nSearchTag >= 0)
	{
		if(bForward){
			int nMax = size();
			for(int i=nStart; i<nMax; i++){
				if(bNoMiddleTags && operator[](i).nFormatTag == nFormatTag)
					return -1;
				if(operator[](i).nFormatTag == nAbortTag)
					return -1;
				if(operator[](i).nFormatTag == nSearchTag)
					return i;
			}
		}
		else{
			int nMin = 0;
			for(int i=nStart; i>=nMin; i--){
				if(bNoMiddleTags && operator[](i).nFormatTag == nFormatTag)
					return -1;
				if(operator[](i).nFormatTag == nAbortTag)
					return -1;
				if( operator[](i).nFormatTag == nSearchTag)
					return i;
			}
		}
	}

	return -1;
}
