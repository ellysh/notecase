////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements iterator/helper methods for traversing NoteDocument object
////////////////////////////////////////////////////////////////////////////

#include "DocumentIterator.h"
#include "debug.h"

DocumentIterator::DocumentIterator(NoteDocument &doc) : m_doc(doc)
{
}

DocumentIterator::~DocumentIterator()
{
}

int DocumentIterator::GetChildCount(int nParentID, bool bRecursive)
{
	int nCount = 0;
	for(unsigned int i=0; i<m_doc.GetNodeCount(); i++)
	{
		if(m_doc.m_lstNodes[i].m_nParentID == nParentID)
		{
			nCount++;
			if(bRecursive)
				nCount += GetChildCount(m_doc.m_lstNodes[i].m_nID, true);
		}
	}
	return nCount;
}

int DocumentIterator::GetChildIdx(int nParentID, int nSiblingIdx)
{
	for(unsigned int i=0; i<m_doc.GetNodeCount(); i++)
	{
		if(m_doc.m_lstNodes[i].m_nParentID == nParentID)
		{
			if(m_doc.m_lstNodes[i].m_nSiblingIdx == nSiblingIdx)
				return i;
		}
	}

	return -1;
}

int DocumentIterator::NodeIdx2RecursiveIdx(int nIdx)
{
	int nRecursiveIdx = -1;

	while(nIdx >= 0)
	{
		int nParentID   = m_doc.GetNodeByIdx(nIdx).m_nParentID;
		int nSiblingIdx = m_doc.GetNodeByIdx(nIdx).m_nSiblingIdx;

		//count the current node
		nRecursiveIdx ++;

		//get "previous" node
		nIdx = -1;
		if(nSiblingIdx > 0){
			nIdx = GetChildIdx(nParentID, nSiblingIdx-1);	//previous sibling

			//recursively go to the last node child
			int nID = m_doc.GetNodeByIdx(nIdx).m_nID;
			int nChildCount = GetChildCount(nID);
			while(nChildCount > 0)
			{
				nIdx = GetChildIdx(nID, nChildCount-1);
				nID = m_doc.GetNodeByIdx(nIdx).m_nID;
				nChildCount = GetChildCount(nID);
			}
		}
		else
			nIdx = m_doc.GetIdxFromID(nParentID);					//
		if(nIdx < 0)
			break;
	}

	return nRecursiveIdx;
}

int DocumentIterator::RecursiveIdx2NodeIdx(int nIdx)
{
	int nCounter = -1;
	return RecursiveGetIdx(nIdx, ROOT_PARENT_ID, nCounter);
}

int DocumentIterator::RecursiveGetIdx(int nRIdx, int nParentID, int &nCounter)
{
	int nChildCnt = GetChildCount(nParentID);
	if(nChildCnt > 0)
	{
		//recurse into children
		for(int i=0; i<nChildCnt; i++)
		{
			nCounter ++;

			int nChildIdx = GetChildIdx(nParentID, i);
			if(nChildIdx >= 0)
			{
				if(nCounter == nRIdx)
					return nChildIdx;

				int nChildID = m_doc.GetNodeByIdx(nChildIdx).m_nID;
				int nIdxNode = RecursiveGetIdx(nRIdx, nChildID, nCounter);
				if(nIdxNode >= 0)
					return nIdxNode;
			}
		}
	}

	return -1; //not found at this level
}

bool DocumentIterator::IsSiblingByIdx(int nIdx1, int nIdx2)
{
	//siblings are nodes with the same aprent
	return (m_doc.GetNodeByIdx(nIdx1).m_nParentID == m_doc.GetNodeByIdx(nIdx2).m_nParentID);
}

bool DocumentIterator::IsAncestorByIdx(int nAncestorIdx, int nIdx)
{
	int nAncestorID = m_doc.GetNodeByIdx(nAncestorIdx).m_nID;

	//check each parent node up to the tree root
	int nNodeIdx = nIdx;
	while (nNodeIdx >= 0)
	{
		int nPID = m_doc.GetNodeByIdx(nNodeIdx).m_nParentID;
		if(nPID == nAncestorID)
			return true;	//parent or grandparent or ...

		nNodeIdx = m_doc.GetIdxFromID(nPID); //recurse to the parent node
	}

	return false; //not an ancestor
}

NoteNode &DocumentIterator::GetNodeByID(int nID)
{
	int nIdx = m_doc.GetIdxFromID(nID);
	ASSERT(nIdx >= 0);
	return m_doc.GetNodeByIdx(nIdx);
}

int DocumentIterator::CalcNodeLevel(int nIdx)
{
	int nLevel = 0;
	int nParentID = m_doc.GetNodeByIdx(nIdx).m_nParentID;
	while(nParentID >= 0){
		nLevel ++;
		nParentID = GetNodeByID(nParentID).m_nParentID;
	}
	return nLevel;
}
