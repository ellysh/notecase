////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: implementation of the circular buffer
//
////////////////////////////////////////////////////////////////////////////

#ifndef CIRCULARBUFFER_H__
#define CIRCULARBUFFER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>

#define DEF_CIRC_BUFFER_SIZE	20000

class CircularBuffer
{
public:
	CircularBuffer(int nSize = DEF_CIRC_BUFFER_SIZE);
	virtual ~CircularBuffer();

	void AddMsg(const char *szMsg);
	void Add(const char *szMsg);
	void Printf(const char *fmt, ...);
	void Dump(const char *szFile);

	bool IsEmpty(){	return (m_nStart == m_nEnd); };
	int  GetUsedSize(){	return (m_nStart <= m_nEnd) ? (m_nEnd - m_nStart) : (m_nSize - m_nStart + m_nEnd); };
	int  GetFreeSize(){	return m_nSize - GetUsedSize(); };

protected:
	char *m_pszData;
	int	  m_nSize;
	int	  m_nStart;
	int	  m_nEnd;

};

#endif // CIRCULARBUFFER_H__
