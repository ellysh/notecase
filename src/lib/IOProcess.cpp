////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements content reading/writing with multiple layers of processing
////////////////////////////////////////////////////////////////////////////

#include "IOProcess.h"
#include "debug.h"
#include <string.h>

IOProcess::IOProcess(int nMode)
{
	m_nMode = nMode;
}

IOProcess::~IOProcess()
{
	CleanLayerList();
}

void IOProcess::PushLayer(IOLayerBase *layer)
{
	// attach to layer before it (if empty, attach to this object)
	if(m_lstLayers.size() > 0)
	{
		IOLayerBase *pPrev = m_lstLayers[m_lstLayers.size()-1];
		pPrev->m_pNextLayer = layer;
	}

	// add object at the end of the list
	m_lstLayers.push_back(layer);

	// init layer
	layer->Initialize(m_nMode);
}

void IOProcess::CleanLayerList()
{
	for(unsigned int i=0; i<m_lstLayers.size(); i++)
		delete m_lstLayers[i];
	m_lstLayers.clear();
}

unsigned char *IOProcess::GetLastLayerData()
{
	if(m_lstLayers.size() > 0)	// if layers processed
	{
		IOLayerBase *pLast = m_lstLayers[m_lstLayers.size()-1];

		unsigned int nIdx = m_lstLayers.size()-1;
		while(NULL == pLast->GetOutBuffer())
		{
			nIdx --;
			if(nIdx >= 0 && m_lstLayers.size() > nIdx)
				pLast = m_lstLayers[nIdx];
			else
				break;
		}
		return pLast->GetOutBuffer();
	}
	return NULL;
}

int IOProcess::Read(char *szBuffer, int nLen, bool &bContinue)
{
	bContinue = false;
	int nRead = 0;
	if(m_lstLayers.size() > 0)	// if layers processed
	{
		//propagates action automatically down the layer chain
		IOLayerBase *pFirst = m_lstLayers[0];
		nRead = pFirst->Process(szBuffer, nLen);

		//first layer is the main content reader, if it has nothing more to read
		//stop the reading
		if(pFirst->m_nLastOutBytes > 0)
			bContinue = true;
	}

	return nRead;
}

int IOProcess::Write(const char *szBuffer, int nLen)
{
	//perform write operation with additional content preprocessing by each layer in the stack
	//allow each layer to transform the content (in strict layer order)

	int nWrite = 0;
	if(m_lstLayers.size() > 0)	// if layers processed
	{
		//propagates action automatically down the layer chain
		IOLayerBase *pFirst = m_lstLayers[0];
		nWrite = pFirst->Process((char *)szBuffer, nLen);
	}
	return nWrite;
}

int IOProcess::WriteString(const char *szBuffer)
{
	int nLen = strlen(szBuffer);
	if(nLen > 0)
		return Write(szBuffer, nLen);
	return nLen;
}

int IOProcess::Finalize()
{
	int nBytes = 0;

	//if(MODE_WRITE == m_nMode)
	{
		if(m_lstLayers.size() > 0)	// if layers processed
		{
			//propagates action automatically down the layer chain
			IOLayerBase *pFirst = m_lstLayers[0];
			nBytes = pFirst->Finalize();
		}
	}

	return nBytes;
}
