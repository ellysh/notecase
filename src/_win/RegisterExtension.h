////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Class to register document format (*.ext) to be handled by given application
////////////////////////////////////////////////////////////////////////////

#ifndef _REGISTEREXTENSION_H__
#define _REGISTEREXTENSION_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if _MSC_VER > 1000
  #pragma warning(disable:4786)
#endif

#include <string>
#include <vector>

//TOFIX add extenison reading, removing

class RegisterFileExtension
{
public:
	RegisterFileExtension();
	virtual ~RegisterFileExtension();

	void ClearData();
	void SetFormatName(const char *szName){ m_strName = szName; };
	void SetExtension(const char *szExt){ m_strExt = szExt; };
	void SetDescription(const char *szDesc){ m_strDesc = szDesc; };
	void SetDefaultIcon(const char *szFile, int nIdx){ m_strIconApp = szFile; m_nIconIndex = nIdx; };
	void AddShellAction(const char *szName, const char *szCmd);
	void SetShellDefault(const char *szName){ m_strDefaultCmd = szName; };

	void UpdateRegistry();	//write to registry

public:
	bool IsRegistered(const char *szExt);	//check if extension already registered

	static bool Key_Remove(const char *szExt);		//removes all related keys
	static bool Key_CreateRoot(const char *szPath, const char *szName, const char *szValue);

protected:
	std::string	m_strExt;
	std::string	m_strName;
	std::string	m_strDesc;
	std::string	m_strIconApp;
	int			m_nIconIndex;
	std::vector<std::string> m_lstCmds;
	std::string	m_strDefaultCmd;
};

#endif // _REGISTEREXTENSION_H__
