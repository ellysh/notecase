////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Stores info for the single hyperlink
////////////////////////////////////////////////////////////////////////////

#include "LinkInfo.h"
#include <algorithm>
#include <glib.h>

//used to sort LinkInfoList class (always ascending by offset)
class LinkComparator{
public:
	bool operator()(const LinkInfo &a, const LinkInfo &b);
};

bool LinkComparator::operator()(const LinkInfo &a, const LinkInfo &b)
{
	return (a.m_nStartOffset < b.m_nStartOffset);
}

LinkInfo::LinkInfo()
{
	m_nStartOffset = -1;
	m_nTextLength  = -1;
	m_nTargetNodeID = -1;
}

LinkInfo::~LinkInfo()
{
}

LinkInfo::LinkInfo(const LinkInfo &other)
{
	operator = (other);
}

void LinkInfo::operator = (const LinkInfo &other)
{
	if(this != &other){
		m_nStartOffset  = other.m_nStartOffset;
		m_nTextLength	= other.m_nTextLength;
		m_nTargetNodeID = other.m_nTargetNodeID;
		m_strTargetURL  = other.m_strTargetURL;
		m_strText       = other.m_strText;
	}
}

void LinkInfo::RefreshLength()
{
	//refresh length of string in UTF8 characters
	m_nTextLength = g_utf8_strlen(m_strText.c_str(), -1);
}

bool LinkInfo::IsValid()
{
	return (m_nTargetNodeID >= 0 || m_strTargetURL.size()>0);
}

void LinkInfo::Clear()
{
	m_strText       = "";
	m_strTargetURL  = "";
	m_nStartOffset  = -1;
	m_nTextLength   = -1;
	m_nTargetNodeID = -1;
}

LinkInfoList::LinkInfoList()
{
}

LinkInfoList::~LinkInfoList()
{
}

int LinkInfoList::Find(int nCharOffset)
{
	//search first matching link
	for(unsigned int i=0; i<size(); i++)
	{
		int nStart = operator[](i).m_nStartOffset;
		int nEnd   = nStart + operator[](i).m_nTextLength;

		//check if the character index is within link position
		if(nCharOffset >= nStart && nCharOffset < nEnd)
			return i;
	}

	return -1;
}

bool LinkInfoList::RangesOverlap(int nA, int nB, int nC, int nD)
{
	//check if the two ranges [a,b) and [c,d) intersect
	return ((nC >= nA && nC < nB) || (nD >= nA && nD < nB) ||
			(nA >= nC && nA < nD) || (nB >= nC && nB < nD));
}

bool LinkInfoList::IsRangeOccupied(int nCharOffset, int nLen)
{
	for(unsigned int i=0; i<size(); i++)
	{
		int nA = operator[](i).m_nStartOffset;
		int nB = nA + operator[](i).m_nTextLength;
		int nC = nCharOffset;
		int nD = nC + nLen;

		if(RangesOverlap(nA, nB, nC, nD))
			return true;	//two ranges are intersected
	}

	return false;	//no link inside this range
}

void LinkInfoList::Sort()
{
    LinkComparator cmp;
    std::sort(begin(), end(), cmp);
}

void LinkInfoList::OnTxtInsert(int nOffset, int nCharsAdded, const char *szTxt)
{
	for(unsigned int i=0; i<size(); i++)
	{
		LinkInfo &info = operator [](i);

		if(nOffset <= info.m_nStartOffset)
		{
			//contents added before the link
			info.m_nStartOffset += nCharsAdded;
		}
		else if(nOffset < info.m_nStartOffset + info.m_nTextLength)
		{
			//contents added inside the link
			unsigned int nOffsetRel = nOffset - info.m_nStartOffset;
			gchar *szInsStart = g_utf8_offset_to_pointer(info.m_strText.c_str(), nOffsetRel);
			int nInsStartByte = szInsStart - info.m_strText.c_str();

			info.m_strText.insert(nInsStartByte, szTxt);
			info.RefreshLength();
		}
	}
}

void LinkInfoList::OnTxtDelete(int nOffset, int nCharsDeleted)
{
	int nA = nOffset;
	int nB = nA + nCharsDeleted;

	unsigned int nSize = size();
	for(unsigned int i=0; i<nSize; i++)
	{
		LinkInfo &info = operator [](i);

		int nC = info.m_nStartOffset;
		int nD = nC + info.m_nTextLength;

		if(nB <= nC)	// no overlapping, deleted segment is before the link
		{
			//contents deleted before the link
			info.m_nStartOffset -= nCharsDeleted;
		}
		else if(nA > nC && nA < nD)	// overlapping, deleted segment starts inside the link
		{
			//contents deleted inside the link (remove chars from nOffset up until nOffset + nCharsDel)
			unsigned int nOffsetRel = nOffset - info.m_nStartOffset;
			gchar *szCutStart = g_utf8_offset_to_pointer(info.m_strText.c_str(), nOffsetRel);
			int nCutStartByte = szCutStart - info.m_strText.c_str();
			int nCutEndByte = -1;
			if(nOffsetRel + nCharsDeleted < (unsigned int)info.m_nTextLength){
				gchar *szCutEnd   = g_utf8_offset_to_pointer(info.m_strText.c_str(), nOffsetRel + nCharsDeleted);
				nCutEndByte = szCutEnd - info.m_strText.c_str();
				//info.m_strText = info.m_strText.substr(nCutStartByte, nCutEndByte-nCutStartByte);
				info.m_strText.erase(nCutStartByte, nCutEndByte-nCutStartByte);
			}
			else
				//info.m_strText = info.m_strText.substr(0, nCutStartByte);
				info.m_strText.erase(nCutStartByte);

			info.RefreshLength();
		}
		else if(nA <= nC && nB > nC)	// overlapping, deleted segment starts before the link, but overlaps on the link
		{
			if(nB >= nD)	// entire link is deleted
			{
				erase(begin()+i);
				i --;	//
				nSize --;
			}
			else	// part of the link (starting from link beginning) is deleted
			{
				//contents deleted inside the link (remove chars from nOffset up until nOffset + nCharsDel)
				unsigned int nOffsetRel = nB - info.m_nStartOffset;
				gchar *szCutEnd = g_utf8_offset_to_pointer(info.m_strText.c_str(), nOffsetRel);
				int nCutEndByte = szCutEnd - info.m_strText.c_str();;
				info.m_nStartOffset = nA; // link start was moved
				info.m_strText  = info.m_strText.substr(nCutEndByte);
				info.RefreshLength();
			}
		}
	}
}

//make sure no link exists at the given range, shrink the links or split them into two
void LinkInfoList::OnLinkCreated(int nOffset, int nLen)
{
	int nA = nOffset;
	int nB = nA + nLen;

	int nSize = size();
	for(int i=0; i<nSize; i++)
	{
		LinkInfo &info = operator [](i);

		int nC = info.m_nStartOffset;
		int nD = nC + info.m_nTextLength;

		if(RangesOverlap(nA, nB, nC, nD))
		{
			if(nA <= nC && nB < nD)	// cut some chars from the link start
			{
				int nDiff = nB - nC;
				//convert to pointer offset
				gchar *szPos = g_utf8_offset_to_pointer(info.m_strText.c_str(), nDiff);
				int nDiffByte = szPos - info.m_strText.c_str();

				info.m_nStartOffset = nB;
				info.m_strText = info.m_strText.substr(nDiffByte);
				info.RefreshLength();
			}
			else if(nA <= nC && nB >= nD) // destroy old link, new one covers old one
			{
				erase(begin()+i);
				i --;	nSize --;	//
			}
			else if(nA > nC && nB < nD)  // split link into two sublinks
			{
				int nDiff1 = nA - nC;
				//convert to pointer offset
				gchar *szPos1 = g_utf8_offset_to_pointer(info.m_strText.c_str(), nDiff1);
				int nDiffByte1 = szPos1 - info.m_strText.c_str();

				LinkInfo lnkNew(info);
				lnkNew.m_nStartOffset = nC;
				lnkNew.m_strText = info.m_strText.substr(0, nDiffByte1);
				lnkNew.RefreshLength();

				int nDiff2 = nB - nC;
				//convert to pointer offset
				gchar *szPos2 = g_utf8_offset_to_pointer(info.m_strText.c_str(), nDiff2);
				int nDiffByte2 = szPos2 - info.m_strText.c_str();

				info.m_nStartOffset = nB;
				info.m_strText = info.m_strText.substr(nDiffByte2);
				info.RefreshLength();

				push_back(lnkNew);
			}
			else if(nA > nC && nB >= nD)	//  cut chars from the end of the link
			{
				int nDiff1 = nA - nC;
				//convert to pointer offset
				gchar *szPos1 = g_utf8_offset_to_pointer(info.m_strText.c_str(), nDiff1);
				int nDiffByte1 = szPos1 - info.m_strText.c_str();

				info.m_strText = info.m_strText.substr(0, nDiffByte1);
				info.RefreshLength();
			}
		}
	}
}

void LinkInfoList::OnTxtReplaced(int nStart, int nEnd, const char *szTxt)
{
	int nSizeNew  = g_utf8_strlen(szTxt, -1);
	int nSizeDiff = nSizeNew - (nEnd - nStart);

	for(unsigned int i=0; i<size(); i++)
	{
		LinkInfo &info = operator [](i);

		if(nEnd < info.m_nStartOffset)
		{
			//contents added before the link
			info.m_nStartOffset += nSizeDiff;
		}
		else if(RangesOverlap(nStart, nEnd, info.m_nStartOffset, info.m_nStartOffset + info.m_nTextLength))
		{
			//replaced contents covering also link area - destroy the link
			erase(begin()+i);
			i --;	//
		}
	}
}
