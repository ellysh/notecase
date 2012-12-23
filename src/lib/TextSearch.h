////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Class implements text buffer search method
////////////////////////////////////////////////////////////////////////////

#ifndef TEXTSEARCH_H__
#define TEXTSEARCH_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <memory.h>
#include <glib.h>

//possible find styles
#define FS_CASE_INSENSITIVE	1

class TextSearch
{
public:
	TextSearch();
	virtual ~TextSearch();

	//TOFIX Get... funkction, Clear (+close the mapping), more styles?, debug timing
    void SetScanStyle(unsigned long dwStyle);

	void SetSearchPattern(const char *szText);
    void SetSearchPattern(const char *szBinary, unsigned int nSize);
    bool SetScanBuffer(const char *szBuffer, unsigned int nSize);

	long Search(unsigned long nStartPos = 0);	//tofix int64

	void Clear();

protected:
	unsigned long	m_dwStyle;
	size_t			m_shift[256];

	const gchar *m_pszPattern;
	unsigned int	m_nPtrnSize;
	const gchar *m_pszBlock;
	unsigned int	m_nBlkSize;
};

#endif // TEXTSEARCH_H__
