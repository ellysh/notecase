////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements layer for encrypting/decrypting the data using Blowfish algorithm
////////////////////////////////////////////////////////////////////////////

#include "IOLayerEnc.h"
#include "string.h"
#include "debug.h"

#ifndef min
 #define min(x,y) (((x)<(y))?(x):(y))
#endif

IOLayerEnc::IOLayerEnc()
{
	m_nDataSize = 0;
	m_nOutSize = 0;
}

IOLayerEnc::~IOLayerEnc()
{
}

int IOLayerEnc::Process(char *szBuffer, int nLen)
{
	int nBytes = 0;
	if(MODE_WRITE == m_nMode)
		nBytes = Write(szBuffer, nLen);
	else
		nBytes = Read(szBuffer, nLen);

	//store output size for this operation
	m_nLastOutBytes = nBytes;

	//NOTE: next layer is called within Read/Write
	return nBytes;
}

void IOLayerEnc::SetPassword(const char *szPass)
{
	m_crypt.Initialize((BYTE *)szPass, strlen(szPass));
}

int IOLayerEnc::Read(char *szBuffer, int nLen)
{
	TRACE("IOLayerEnc::Read (content size=%d)\n", nLen);

	//TOFIX full blocks for decode too?
	ASSERT(nLen % 8 == 0);

	int nRead = nLen;
	m_crypt.Decode((BYTE *)szBuffer, (BYTE *)szBuffer, nRead); //inplace decode
	//TRACE("IOLayerEnc::Read decoded=\n%s\n", szBuffer);

	//szBuffer[nRead] = '\0';
	m_nOutSize = nRead;

	m_nProcessedInBytes += nLen;

	//call next layer attached?
	if(NULL != m_pNextLayer && nRead > 0){
		nRead = m_pNextLayer->Process(szBuffer, nRead);
	}

	return nRead;
}

int IOLayerEnc::Write(char *szBuffer, int nLen)
{
	TRACE("IOLayerEnc::Write (content size=%d)\n", nLen);

	int nPos = 0;
	int nTotalSize = 0;
	int nTotalSizeExt = 0;
	int nAvailSpace = 0;

	m_nProcessedInBytes += nLen;

	while(nLen > 0)
	{
		nAvailSpace = ENC_BUFFER_SIZE - m_nDataSize;
		if(nAvailSpace > 0)
		{
			//prepare new buffer (append new data to its end)
			int nAdd = min(nLen, nAvailSpace);
			memcpy(m_szBuffer + m_nDataSize, szBuffer + nPos, nAdd);

			//refresh variables
			m_nDataSize += nAdd;
			nLen -= nAdd;
			nPos += nAdd;
			nAvailSpace = ENC_BUFFER_SIZE - m_nDataSize;
		}
		ASSERT(nAvailSpace >= 0);

		//flush data if buffer full
		if(0 == nAvailSpace)
		{
			int nBytes = WriteBuffer(false);
			nTotalSize += nBytes;

			//call next layer attached?
			if(NULL != m_pNextLayer && nBytes > 0){
				nTotalSizeExt += m_pNextLayer->Process(m_szBuffer, nBytes);
			}

			//shift unprocessed data to the begining of the buffer
			if(nBytes > 0)
				memcpy(m_szBuffer, m_szBuffer + nBytes, m_nDataSize);
		}
	}

	m_nOutSize = nTotalSize;
	return nTotalSizeExt;
}

int IOLayerEnc::Finalize()
{
	//write anything left in the buffer
	if(MODE_WRITE == m_nMode)
	{
		TRACE("IOLayerEnc::Finalize - size to be flushed=%d\n",m_nDataSize);

		int nBytes = WriteBuffer(true);

		//call next layer attached?
		if(NULL != m_pNextLayer){
			if(nBytes > 0)
				m_pNextLayer->Process(m_szBuffer, nBytes);
			return m_pNextLayer->Finalize();
		}

		return nBytes;
	}
	else{
		//call next layer attached?
		if(NULL != m_pNextLayer)
			return m_pNextLayer->Finalize();
	}

	return 0;
}

int IOLayerEnc::WriteBuffer(bool bFinalize)
{
	//unless we are in finalize mode, try to encrypt only full blocks of data (8 bytes)
	//keep the rest of the data in the buffer (wait for more data to arrive)

	//encode and write buffer into the file
	if(m_nDataSize > 0)
	{
		int nFullBlocksSize = (m_nDataSize / 8) * 8;
		int nOrigSize = (bFinalize)?  m_nDataSize : nFullBlocksSize;
		int nRequired = m_crypt.GetOutputLength(nOrigSize);

		// pad to full block with zeros
		int nIdx = nOrigSize;
		while (nIdx < nRequired)
			m_szBuffer[nIdx++] = 0;

		int nEncSize  = m_crypt.Encode((BYTE *)m_szBuffer, (BYTE *)m_szBuffer, nRequired);
		TRACE("IOLayerEnc::WriteBuffer in size=%d (%d), out size=%d\n", nOrigSize, nRequired, nEncSize);

		m_nDataSize -= nOrigSize;
		return nEncSize;
	}
	return 0;
}
