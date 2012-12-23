////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implement layer that redirects processing to some callback method
////////////////////////////////////////////////////////////////////////////

#include "IOLayerRedirect.h"

IOLayerRedirect::IOLayerRedirect()
{
	m_pfnProcess = NULL;
}

IOLayerRedirect::~IOLayerRedirect()
{
}

int IOLayerRedirect::Process(char *szBuffer, int nLen)
{
	//redirect processing to some callback
	if(m_pfnProcess)
		m_pfnProcess(szBuffer, nLen);
	return nLen;
}
