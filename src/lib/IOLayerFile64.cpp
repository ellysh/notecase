////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements layer for actual reading/writing data into the file
////////////////////////////////////////////////////////////////////////////

#include "IOLayerFile64.h"
#include "debug.h"

IOLayerFile64::IOLayerFile64()
{
	m_nEndOffset = -1;
}

IOLayerFile64::~IOLayerFile64()
{
}

int IOLayerFile64::Process(char *szBuffer, int nLen)
{
	int nBytes = 0;

	if(MODE_WRITE == m_nMode)
	{
		nBytes = m_file.Write(szBuffer, nLen);
		//TRACE("IOLayerFile64::Write (%d bytes)\n", nLen);
	}
	else
	{
		nBytes = m_file.Read(szBuffer, nLen);

		//when we have read limit defined
		if(nBytes > 0 && m_nEndOffset > 0){
			INT64 nPos = m_file.GetPosition();
			if(nPos > m_nEndOffset){
				nBytes -= (int)(nPos-m_nEndOffset);
				if(nBytes < 0)
					nBytes = 0;
			}
		}

		//TRACE("IOLayerFile64::Read  (buffer size=%d, read bytes=%d)\n", nLen, nBytes);
	}

	m_nProcessedInBytes += nBytes;
	m_nProcessedOutBytes += nBytes;

	//store output size for this operation
	m_nLastOutBytes = nBytes;

	//call next layer attached?
	if(NULL != m_pNextLayer && nBytes > 0)
		return m_pNextLayer->Process(szBuffer, nBytes);

	return nBytes;
}

int IOLayerFile64::Finalize()
{
	TRACE("IOLayerFile64::Finalize (pass forward)\n");

	//call next layer attached?
	if(NULL != m_pNextLayer)
		return m_pNextLayer->Finalize();
	return 0;
}
