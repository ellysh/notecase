////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements "Most Recently Used" menu list
////////////////////////////////////////////////////////////////////////////

#ifndef _MRU_H__
#define _MRU_H__

#if _MSC_VER > 1000
  #pragma warning(disable: 4786)
#endif

#include <string>
#include <vector>

class MRU
{
public:
	MRU();
	~MRU();

	int  GetFileCount();

	void Load();
	void Save();
	void Clear();
	void Change(const char *szFilename, bool bAdd);
	int  FindEntry(const char* szFilename);

	void RebuildMenu();
	const char *GetFile(int nIdx){ return m_lstFiles[nIdx].c_str(); }
	int  GetSize(){ return m_lstFiles.size(); }

protected:
	std::vector<std::string> m_lstFiles;
};

#endif
