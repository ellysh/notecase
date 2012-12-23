////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements layer to compress/decompress Zlib compressed data
////////////////////////////////////////////////////////////////////////////

#include "IOLayerZlib.h"
#include <stdlib.h>
#include <memory.h>
#include "debug.h"

IOLayerZlib::IOLayerZlib()
{
	m_err = Z_OK;
	m_nMode = MODE_UNK;
	m_bInitialised = false;
	m_nPackLevel   = Z_DEFAULT_COMPRESSION;
	memset(&m_stream, 0, sizeof(z_stream));
}

IOLayerZlib::~IOLayerZlib()
{
}

void IOLayerZlib::Initialize(int nMode)
{
	IOLayerBase::Initialize(nMode);

	//stream object initialization
	if(!m_bInitialised)
	{
		if(MODE_WRITE == m_nMode)
			m_err = deflateInit(&m_stream, m_nPackLevel);	//compression
		else
			m_err = inflateInit(&m_stream);				//decompression

		ASSERT(m_err == Z_OK);  // check state
		m_stream.avail_in = 0;
		m_stream.avail_out = m_nOutLength;
		m_stream.next_out = m_szOutBuffer;

		m_bInitialised = true;
	}
}

int IOLayerZlib::Process(char *szBuffer, int nLen)
{
	int nBytes = 0;

	//if stream length is known in advance, process only up to X bytes
	if(m_nTotalInBytes >= 0)
	{
		if((m_nProcessedInBytes + nLen) > m_nTotalInBytes){
			TRACE(" IOLayerZlib::Process Reached end of stream!!\n");
			nLen = m_nTotalInBytes - m_nProcessedInBytes;
		}
	}

	if(nLen <= 0) return 0;

	//do the operation
	if(MODE_WRITE == m_nMode)
		nBytes = Write(szBuffer, nLen);
	else
		nBytes = Read(szBuffer, nLen);

	//store output size for this operation
	m_nLastOutBytes = nBytes;
	m_nProcessedInBytes += nLen;
	m_nProcessedOutBytes += nBytes;

	return 0;
}

int IOLayerZlib::Read(char *szBuffer, int nLen)
{
	//decompress content
	TRACE("IOLayerZlib::Read (size %d bytes)\n", nLen);
	ASSERT(nLen > 0);
	if(nLen < 1)
		return 0;

	ASSERT(m_stream.avail_in == 0);
	m_stream.next_in  = (unsigned char *)szBuffer;
	m_stream.avail_in = nLen;

	int nTotalBytes = 0;
	bool bSucess = false;
	bSucess = DoProcessing(Z_NO_FLUSH, nTotalBytes);
	// TOFIX err handling
	return nTotalBytes;
}

int IOLayerZlib::Write(char *szBuffer, int nLen)
{
	//compress content
	TRACE("IOLayerZlib::Write (%d bytes)\n", nLen);
	ASSERT(nLen > 0);
	if(nLen < 1)
		return 0;

	ASSERT(m_stream.avail_in == 0);
	m_stream.next_in  = (unsigned char *)szBuffer;
	m_stream.avail_in = nLen;

	int nTotalBytes = 0;
	bool bSucess = false;
	bSucess = DoProcessing(Z_NO_FLUSH, nTotalBytes);
	// TOFIX err handling
	return nTotalBytes;
}

int IOLayerZlib::Finalize()
{
	int nTotalBytes = 0;
	bool bSucess = false;
	TRACE("IOLayerZlib::Finalize start\n");
	bSucess = DoProcessing(Z_FINISH, nTotalBytes);
	TRACE("IOLayerZlib::Finalize (%d bytes)\n", nTotalBytes);

	// cleanup stream memory
	if(MODE_WRITE == m_nMode)
		deflateEnd(&m_stream);
	else
		inflateEnd(&m_stream);

	//call next layer attached?
	if(NULL != m_pNextLayer)
		return m_pNextLayer->Finalize();

	return nTotalBytes;
}

bool IOLayerZlib::DoProcessing(int nFlag, int &nTotal)
{
	int nTotalBytes = 0;
	int nTotalBytesExt = 0;

	m_err = Z_OK;

	for(;;)
	{
		m_stream.avail_out = m_nOutLength;
		m_stream.next_out = m_szOutBuffer;

		if(MODE_WRITE == m_nMode)
			m_err = deflate(&m_stream, nFlag);
		else
			m_err = inflate(&m_stream, nFlag);

		unsigned int nBytes = m_nOutLength - m_stream.avail_out;
		TRACE(" IOLayerZlib::DoProcessing zlib output: %d bytes, zlib stream state: %s\n", nBytes, m_stream.msg);

		nTotalBytes += nBytes;
		m_nLastOutBytes = nBytes;
		m_nProcessedOutBytes += nBytes;

		//call next layer attached?
		if(NULL != m_pNextLayer && nBytes > 0)
			nTotalBytesExt += m_pNextLayer->Process((char *)m_szOutBuffer, nBytes);

		//control looping here
		if(m_err != Z_OK)
			break;
		if(nFlag == Z_NO_FLUSH && m_stream.avail_in == 0)
			break;
	}

	//TRACE(" IOLayerZlib::DoProcessing end (zlib unprocessed: %d bytes)\n", m_stream.avail_in);
	nTotal = nTotalBytes;
	return true;
}
