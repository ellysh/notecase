////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements content reading/writing with multiple layers of processing
////////////////////////////////////////////////////////////////////////////

#ifndef IOProcess_H__
#define IOProcess_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IOLayerBase.h"
#include <vector>

class IOProcess
{
public:
	IOProcess(int nMode);
	virtual ~IOProcess();

	void PushLayer(IOLayerBase *layer);
	void CleanLayerList();

	int Read(char *szBuffer, int nLen, bool &bContinue);
	int Write(const char *szBuffer, int nLen);
	int WriteString(const char *szBuffer);
	int Finalize();

	unsigned char *GetLastLayerData();

protected:
	std::vector<IOLayerBase *> m_lstLayers;
	int m_nMode;
};

#endif // FILE64TRANSFORM_H__
