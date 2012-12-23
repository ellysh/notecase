////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: implementation of the circular buffer
//
////////////////////////////////////////////////////////////////////////////

#include "CircularBuffer.h"
#include "File64.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#ifdef _WIN32
 #define vsnprintf _vsnprintf
 #include <windows.h>
#else
 #include <sys/time.h>
 #include <unistd.h>
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CircularBuffer::CircularBuffer(int nSize)
{
	m_pszData = (char *)malloc(nSize);
	m_nSize   = nSize;

	m_nStart = 0;
	m_nEnd   = 0;
}

CircularBuffer::~CircularBuffer()
{
	if(m_pszData)
		free(m_pszData);
}

void CircularBuffer::Dump(const char *szFile)
{
	if(IsEmpty())
		return;

	File64 file;
	if(!file.Open(szFile, F64_WRITE|F64_OPEN_NEW))
		return;

	size_t nWritten;
	if(m_nEnd > m_nStart){
		nWritten = file.Write(m_pszData + m_nStart, m_nEnd-m_nStart);
	}
	else{
		nWritten = file.Write(m_pszData + m_nStart, m_nSize-m_nStart);
		nWritten = file.Write(m_pszData, m_nEnd);
	}
}

void CircularBuffer::Add(const char *szMsg)
{
	int nMsgSize = strlen(szMsg);
	if(nMsgSize > m_nSize)
		nMsgSize = m_nSize;

	//write after the end mark
	int nAvail1 = m_nSize - m_nEnd;
	if(nMsgSize <= nAvail1){
		memcpy(m_pszData + m_nEnd, szMsg, nMsgSize);

		//recalc new boundaries
		if(m_nStart > m_nEnd && m_nStart < (m_nEnd + nMsgSize))
			m_nStart = m_nEnd + nMsgSize;
		m_nEnd	+= nMsgSize;

	}
	else{
		memcpy(m_pszData + m_nEnd, szMsg, nAvail1);
		memcpy(m_pszData, szMsg + nAvail1, nMsgSize - nAvail1);

		//recalc new boundaries
		if(m_nStart > m_nEnd)
			m_nStart = nMsgSize - nAvail1;
		else if (m_nStart < m_nEnd && m_nStart < (nMsgSize - nAvail1))
			m_nStart = nMsgSize - nAvail1;
		m_nEnd = nMsgSize - nAvail1;
	}

}

void CircularBuffer::Printf(const char *fmt, ...)
{
	char buffer[1024] = "";

	// create string using format and list of parameters
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer)-1, fmt, args);
	va_end(args);

	AddMsg(buffer);
}

void CircularBuffer::AddMsg(const char *szMsg)
{
	char buffer[2024] = "";

	//TOFIX time in miliseconds (Win - _ftime, Linux - ?)
	// create string using format and list of parameters
#ifdef _WIN32
	SYSTEMTIME st;
	GetSystemTime(&st);
	sprintf(buffer, "%02d:%02d:%02d.%03d ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
#else
	struct timeval the_time;
	int result = gettimeofday(&the_time, NULL);
	if(result >= 0){
		time_t tvSec = the_time.tv_sec;
		struct tm *pTime = localtime(&tvSec);
		sprintf(buffer, "%02d:%02d:%02d", pTime->tm_hour, pTime->tm_min, pTime->tm_sec);

		//append miliseconds
		char szMilisec[10];
		sprintf(szMilisec, ".%03d ", (int)(the_time.tv_usec/1000));
		strcat(buffer, szMilisec);
	}
	else{
		time_t nTime = time(NULL);
		struct tm *pTime = localtime(&nTime);
		sprintf(buffer, "%02d:%02d:%02d ", pTime->tm_hour, pTime->tm_min, pTime->tm_sec);
	}
#endif

	strcat(buffer, szMsg);

	Add(buffer);
}
