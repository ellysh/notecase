////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements iterator/helper methods for traversing NoteDocument object
////////////////////////////////////////////////////////////////////////////

#ifndef DOCUMENTITERATOR_H__
#define DOCUMENTITERATOR_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NoteDocument.h"

#define ROOT_PARENT_ID	-1

class DocumentIterator
{
public:
	DocumentIterator(NoteDocument &doc);
	virtual ~DocumentIterator();

	inline NoteDocument &GetDocument(){	return m_doc; }

	int GetChildCount(int nParentID, bool bRecursive = false);
	int GetChildIdx(int nParentID, int nSiblingIdx);

	bool IsSiblingByIdx(int nIdx1, int nIdx2);
	bool IsAncestorByIdx(int nAncestorIdx, int nIdx);
	int CalcNodeLevel(int nIdx);

	//TOFIX move some methods from NoteDocument
	NoteNode &GetNodeByID(int nID);

	//recursive index is when we count tree items in depth-first mode
	int RecursiveIdx2NodeIdx(int nIdx);
	int NodeIdx2RecursiveIdx(int nIdx);

protected:
	int RecursiveGetIdx(int nIdx, int nParentID, int &nCounter);

	NoteDocument &m_doc;
};

#endif // DOCUMENTITERATOR_H__
