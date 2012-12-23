////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: object of this class should be alive as long as program lives
//		 so that instance can be detected (by using named mutex object)
////////////////////////////////////////////////////////////////////////////

#ifndef __SINGLEINSTANCE_H__
#define __SINGLEINSTANCE_H__

#include <windows.h>

class CSingleInstance
{
public:
    CSingleInstance(const char *szName);
    ~CSingleInstance();

	bool ProgramAlreadyStarted();

protected:
    HANDLE  m_hMutex;
	bool	m_bAlreadyExists;
};

#endif // __SINGLEINSTANCE_H__
