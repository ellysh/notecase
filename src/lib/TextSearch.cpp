////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Class implements fast text buffer search method
////////////////////////////////////////////////////////////////////////////

#include "TextSearch.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "debug.h"

static gchar *utf8_strcasestr(const gchar *haystack, const gchar *needle);

TextSearch::TextSearch()
{
	m_dwStyle		= 0;
	m_pszPattern	= NULL;
	m_nPtrnSize		= 0;
}

TextSearch::~TextSearch()
{
}

void TextSearch::SetScanStyle(unsigned long dwStyle)
{
	//NOTE: you should NOT change style once the pattern is set
	//		(since that functions used current style setting)
	//ASSERT(NULL == m_pszPattern && 0 == m_nPtrnSize);

	m_dwStyle |= dwStyle;
}

void TextSearch::SetSearchPattern(const char *szText)
{
	m_pszPattern = (const gchar *)szText;
	m_nPtrnSize	 = g_utf8_strlen(szText, -1);	//TOFIX if NULL
}

void TextSearch::SetSearchPattern(const char *szBinary, unsigned int nSize)
{
	m_pszPattern = szBinary;
	m_nPtrnSize	 = nSize;
}

bool TextSearch::SetScanBuffer(const char *szBuffer, unsigned int nSize)
{
	m_pszBlock	= szBuffer;
	m_nBlkSize	= nSize;
	return (m_pszBlock != NULL && m_nBlkSize > 0);
}

//TOFIX use int64 for large memory
long TextSearch::Search(unsigned long nStartPos)
{
    const gchar
        *block   = m_pszBlock,		//  Concrete pointer to block data
        *pattern = m_pszPattern;	//  Concrete pointer to search value

    ASSERT (NULL != block);                 //  Expect non-NULL pointers, but
    ASSERT (NULL != pattern);               //  fail gracefully if not debugging

	if(nStartPos < 0 || nStartPos >= m_nBlkSize)
		return -1; //invalid position
    if (block == NULL || pattern == NULL)
        return -1;

	//move position by the required amount of characters
	block = g_utf8_offset_to_pointer(block, nStartPos);

    //  Pattern must be smaller or equal in size to string
    if (m_nBlkSize-nStartPos < m_nPtrnSize)
        return -1;                  //  Otherwise it's not found
    if (m_nPtrnSize == 0)           //  Empty patterns match at start
        return 0;

	//NOTE: case sensitive and case insensitive version
	gchar *szPos = NULL;
	if(m_dwStyle & FS_CASE_INSENSITIVE)
		szPos = utf8_strcasestr(block, pattern);
	else
		szPos = g_strstr_len(block, m_nBlkSize - nStartPos, pattern);

	if (szPos){
		long nPos = g_utf8_pointer_to_offset(block, szPos);
		return nPos + nStartPos;
	}
	return -1;	// Found nothing
}

void TextSearch::Clear()
{
	m_pszPattern = NULL;
	m_nPtrnSize  = 0;
	m_pszBlock	 = NULL;
	m_nBlkSize   = 0;
}


int utf8_strnicmp(const gchar *szStr1, const gchar *szStr2, int nLen)
{
	while(nLen --)
	{
		if(NULL == szStr1)
			return -1;
		if(NULL == szStr2)
			return 1;

		gunichar nC1 = g_unichar_toupper(g_utf8_get_char(szStr1));
		gunichar nC2 = g_unichar_toupper(g_utf8_get_char(szStr2));
		if(nC1 != nC2)
			return nC1-nC2;

		szStr1 = g_utf8_next_char(szStr1);
		szStr2 = g_utf8_next_char(szStr2);
	}

	return 0; // match
}

gchar *utf8_strcasestr(const gchar *haystack, const gchar *needle)
{
	size_t haystack_len = g_utf8_strlen(haystack, -1);
	size_t needle_len   = g_utf8_strlen(needle, -1);

	if (haystack_len < needle_len || needle_len == 0)
		return NULL;

	while (haystack_len >= needle_len) {
		if (0 == utf8_strnicmp(haystack, needle, needle_len))
			return (gchar *)haystack;
		else {
			haystack = g_utf8_next_char(haystack);
			haystack_len--;
		}
	}

	return NULL;
}
