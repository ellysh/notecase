////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements layer to compress/decompress Zlib compressed data
////////////////////////////////////////////////////////////////////////////

#ifndef IOLayerZlib_H__
#define IOLayerZlib_H__

//
// Use single object only for reading or only for writing (do not mix calls!!!)
//

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IOLayerBase.h"
#include "zlib/zlib.h"

class IOLayerZlib : public IOLayerBase
{
public:
	IOLayerZlib();
	virtual ~IOLayerZlib();

	virtual void Initialize(int nMode);
	virtual unsigned char *GetOutBuffer(){ return m_szOutBuffer; };
	virtual int  Process(char *szBuffer, int nLen);
	virtual int  Finalize();

protected:
	int Read(char *szBuffer, int nLen);
	int Write(char *szBuffer, int nLen);
	bool DoProcessing(int nFlag, int &nTotal);

protected:
	struct z_stream_s m_stream;

	bool m_bInitialised;
	int  m_nPackLevel;
	int  m_err;

	enum { m_nOutLength = 4096 };
	unsigned char m_szOutBuffer[m_nOutLength];
};

#endif // FILE64ZLIB_H__
