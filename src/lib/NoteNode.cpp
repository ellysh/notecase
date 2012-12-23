////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Class describing single document node
////////////////////////////////////////////////////////////////////////////

#include "NoteNode.h"
#include "../DocAction.h"

NoteNode::NoteNode()
{
	m_nIconType = ICON_NONE;
	m_nSelStart = 0;
	m_nSelEnd = 0;
	m_nDateCreated = 0;
	m_nDateModified = 0;
	m_bFinished = false;
	m_bExpanded = false;
	m_nScrollPos = 0;
}

NoteNode::~NoteNode()
{
}

NoteNode::NoteNode(const NoteNode& other)
{
	operator =(other);
}

void NoteNode::operator =(const NoteNode& other)
{
	m_nID			= other.m_nID;
	m_nParentID		= other.m_nParentID;
	m_nSiblingIdx	= other.m_nSiblingIdx;

	CopyState(other);
}

void NoteNode::CopyState(const NoteNode& other)
{
	//take all data from other node (id, pid, dates are skipped)
	m_nDateCreated	= other.m_nDateCreated;
	m_nDateModified	= other.m_nDateModified;
	m_nIconType		= other.m_nIconType;
	m_strIconFile	= other.m_strIconFile;
	m_strTitle		= other.m_strTitle;
	m_strText		= other.m_strText;
	m_strKeywords	= other.m_strKeywords;
	m_lstLinks		= other.m_lstLinks;
	m_nSelStart		= other.m_nSelStart;
	m_nSelEnd		= other.m_nSelEnd;
	m_bFinished		= other.m_bFinished;
	m_bExpanded		= other.m_bExpanded;
	m_objLink		= other.m_objLink;
	m_lstPictures   = other.m_lstPictures;
	m_lstTxtFmt		= other.m_lstTxtFmt;
	m_lstAttachments= other.m_lstAttachments;
	m_nScrollPos	= other.m_nScrollPos;
	m_strSourceLanguage = other.m_strSourceLanguage;
}

void NoteNode::Clear()
{
	m_strTitle  = "";
	m_strText   = "";
	m_nSelStart = 0;
	m_nSelEnd	= 0;
	m_nIconType = ICON_NONE;
	m_nDateCreated = 0;
	m_nDateModified = 0;
	m_bFinished = false;
	m_bExpanded = false;
	m_strKeywords	= "";
	m_lstLinks.clear();
	m_objLink.Clear();
	m_lstPictures.clear();
	m_lstTxtFmt.clear();
	m_lstAttachments.clear();
	m_nScrollPos = 0;
	m_strSourceLanguage = "";
}

bool NoteNode::IsEmpty()
{
	return (m_strText.empty() && m_lstPictures.empty() && 0 == m_lstAttachments.size());
}

void NoteNode::SetTitle(const char *szTitle)
{
	m_strTitle = szTitle;
}

void NoteNode::SetText(const char *szText)
{
	m_strText = szText;
}

void NoteNode::SetKeywords(const char *szText)
{
	m_strKeywords = szText;
}

void NoteNode::SetSourceLanguage(const char *szText)
{
	m_strSourceLanguage = szText;
}

void NoteNode::SetSelStart(unsigned int selStart)
{
	m_nSelStart = selStart;
}

void NoteNode::SetSelEnd(unsigned int selEnd)
{
	m_nSelEnd = selEnd;
}

unsigned int NoteNode::GetSelStart()
{
	return m_nSelStart;
}

unsigned int NoteNode::GetSelEnd()
{
	return m_nSelEnd;
}

void NoteNode::ClearSel()
{
	m_nSelStart = 0;
	m_nSelEnd = 0;
}

void NoteNode::SetScrollPos(unsigned int nPos)
{
	m_nScrollPos = nPos;
}

unsigned int NoteNode::GetScrollPos()
{
	return m_nScrollPos;
}

void NoteNode::OnTxtInsert(int nOffset, int nCharsAdded, const char *szTxt)
{
	//refresh link list
	m_lstLinks.OnTxtInsert(nOffset, nCharsAdded, szTxt);

	//refresh picture list
	int i;
	int nPicCount = m_lstPictures.size();
	for(i=0; i<nPicCount; i++)
	{
		PixInfo &info = m_lstPictures[i];

		//contents added before the picture
		if(nOffset <= info.nOffset)
			info.nOffset += nCharsAdded;
	}

	//refresh formatting list
	m_lstTxtFmt.OnTxtInsert(nOffset, nCharsAdded, szTxt);
}

void NoteNode::OnTxtReplaced(int nStart, int nEnd, const char *szTxt)
{
	//refresh link list
	m_lstLinks.OnTxtReplaced(nStart, nEnd, szTxt);

	int nSizeNew  = g_utf8_strlen(szTxt, -1);
	int nSizeDiff = nSizeNew - (nEnd - nStart);

	//refresh picture list
	int i;
	int nPicCount = m_lstPictures.size();
	for(i=0; i<nPicCount; i++)
	{
		PixInfo &info = m_lstPictures[i];

		if(nEnd < info.nOffset)
		{
			//contents added before the pix
			info.nOffset += nSizeDiff;
		}
		else if(nStart <= info.nOffset && nEnd >= info.nOffset)
		{
			//replaced contents covering also pix area - destroy the pix

			//decrement offsets for all formatting marks
			int nOffset = m_lstPictures[i].nOffset;
			int nSize = m_lstTxtFmt.size();
			for(int j=0; j<nSize; j++){
				if(m_lstTxtFmt[j].nOffset > nOffset)
					m_lstTxtFmt[j].nOffset --;
			}

			m_lstPictures.erase(m_lstPictures.begin()+i);
			i --; nPicCount --;	//
		}
	}

	//refresh formatting list
	m_lstTxtFmt.OnTxtReplaced(nStart, nEnd, szTxt);
}

void NoteNode::OnTxtDelete(int nOffset, int nCharsDeleted, int nTotalDeleted, DocAction *pDelAction)
{
	//refresh link list
	m_lstLinks.OnTxtDelete(nOffset, nCharsDeleted);

	//refresh picture list
	int i;
	int nPicCount = m_lstPictures.size();
	for(i=0; i<nPicCount; i++)
	{
		PixInfo &info = m_lstPictures[i];

		//contents deleted before the picture
		if(nOffset <= info.nOffset)
		{
			if((nOffset + nTotalDeleted <= info.nOffset) && nTotalDeleted > 0)
				info.nOffset -= nTotalDeleted;
			else{
				//push a copy of image into UNDO action
				if(pDelAction)
					pDelAction->m_lstPictures.push_back(m_lstPictures[i]);

				//
				// pix deleted together with text
				//

				//decrement offsets for all formatting marks
				int nOffset = m_lstPictures[i].nOffset;
				int nSize = m_lstTxtFmt.size();
				for(int j=0; j<nSize; j++){
					if(m_lstTxtFmt[j].nOffset > nOffset)
						m_lstTxtFmt[j].nOffset --;
				}

				//pix deleted together with text
				m_lstPictures.erase(m_lstPictures.begin()+i);
				i --; nPicCount --;	//
			}
		}
	}

	//refresh formatting list
	m_lstTxtFmt.OnTxtDelete(nOffset, nCharsDeleted);
}
