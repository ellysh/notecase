////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Abstract object that dynamically processes some content during I/O
//       Multiple processing objects can be layered to provide multiple
//       successive transformations to the stream being processed
////////////////////////////////////////////////////////////////////////////

#include "IOLayerBase.h"
#include <stdlib.h>

IOLayerBase::IOLayerBase()
{
	m_nMode      = MODE_UNK;
	m_pNextLayer = NULL;
	m_nLastOutBytes = 0;
	m_nProcessedInBytes = 0;
	m_nProcessedOutBytes = 0;
	m_nTotalInBytes = -1;	// unknown
}

IOLayerBase::~IOLayerBase()
{
}
