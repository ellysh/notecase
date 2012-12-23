////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements layer for encrypting/decrypting the data using Blowfish algorithm
////////////////////////////////////////////////////////////////////////////

#ifndef IOLayerEnc_H__
#define IOLayerEnc_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IOLayerBase.h"
#include "blowfish.h"

#define ENC_BUFFER_SIZE	1024

class IOLayerEnc : public IOLayerBase
{
public:
	IOLayerEnc();
	virtual ~IOLayerEnc();

	void SetPassword(const char *szPass);

	virtual unsigned char *GetOutBuffer(){ return NULL; };
	virtual int Process(char *szBuffer, int nLen);
	virtual int Finalize();

protected:
	int Read(char *szBuffer, int nLen);
	int Write(char *szBuffer, int nLen);
	int WriteBuffer(bool bFinalize = false);

protected:
	CBlowFish m_crypt;
	char m_szBuffer[ENC_BUFFER_SIZE];
	int  m_nDataSize;
	int  m_nOutSize;
};

#endif // FILE64ENC_H__
