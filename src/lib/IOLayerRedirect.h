////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implement layer that redirects processing to some callback method
////////////////////////////////////////////////////////////////////////////

#ifndef IOLAYERREDIRECT_H__
#define IOLAYERREDIRECT_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IOLayerBase.h"

//define callback method for processing
typedef void (*PFN_PROCESS)(const char *, int);

class IOLayerRedirect : public IOLayerBase
{
public:
	IOLayerRedirect();
	virtual ~IOLayerRedirect();

	void SetCallback(PFN_PROCESS fn){ m_pfnProcess = fn; }

	virtual int Process(char *szBuffer, int nLen);
	virtual unsigned char *GetOutBuffer(){ return NULL; };
	virtual int Finalize(){ return 0; };

protected:
	PFN_PROCESS	m_pfnProcess;
};

#endif // IOLAYERREDIRECT_H__
