////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Abstract object that dynamically processes some content during I/O
//       Multiple processing objects can be layered to provide multiple
//       successive transformations to the stream being processed
////////////////////////////////////////////////////////////////////////////

#ifndef IOLayerBase_H__
#define IOLayerBase_H__

#include <stdlib.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MODE_UNK	0
#define MODE_READ	1
#define MODE_WRITE	2

#include "types.h"

class IOLayerBase
{
public:
	IOLayerBase();
	virtual ~IOLayerBase();

	virtual void Initialize(int nMode){ m_nMode = nMode; }
	virtual unsigned char *GetOutBuffer() = 0;

	virtual int Process(char *szBuffer, int nLen) = 0;
	virtual int Finalize() = 0;


public:
	int m_nMode;
	int m_nLastOutBytes;		// output size for last operation only
	INT64 m_nTotalInBytes;		// total bytes to process (if known in advance)

	INT64 m_nProcessedInBytes;	// total processed (input) bytes
	INT64 m_nProcessedOutBytes;	// total processed (output) bytes

	IOLayerBase *m_pNextLayer;
};

#endif // FILEIOLAYER_H__
