////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements layer for actual reading/writing data into the file
////////////////////////////////////////////////////////////////////////////

#ifndef IOLAYERFILE64_H__
#define IOLAYERFILE64_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IOLayerBase.h"
#include "File64.h"

class IOLayerFile64 : public IOLayerBase
{
public:
	IOLayerFile64();
	virtual ~IOLayerFile64();

	virtual unsigned char *GetOutBuffer(){ return NULL; };
	virtual int Process(char *szBuffer, int nLen);
	virtual int Finalize();

public:
	File64 m_file;
	INT64  m_nEndOffset;
};

#endif // IOLAYERFILE64_H__
