////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Class to register document format (*.ext) to be handled by given application
////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
  #pragma warning(disable: 4996)	//ignore deprecated functions warning
#endif

#ifdef _WIN32
 #include <io.h>	//_access
 #define access _access
#endif

#include "RegisterExtension.h"
#include <windows.h>

RegisterFileExtension::RegisterFileExtension()
{
	ClearData();
}

RegisterFileExtension::~RegisterFileExtension()
{
}

void RegisterFileExtension::ClearData()
{
	m_strName	 = "";
	m_strExt	 = "";
	m_strDesc	 = "";
	m_strIconApp = "";
	m_nIconIndex = -1;
	m_lstCmds.clear();
}

void RegisterFileExtension::AddShellAction(const char *szName, const char *szCmd)
{
	//encode both string into one
	std::string strData(szName);
	strData += "\t";
	strData += szCmd;

	//store it
	m_lstCmds.push_back(strData);
}

void RegisterFileExtension::UpdateRegistry()
{
	Key_CreateRoot(m_strExt.c_str(),  "", m_strName.c_str());	//write ext key
	Key_CreateRoot(m_strName.c_str(), "", m_strDesc.c_str());	//write description key

	if(m_nIconIndex != -1)
	{
		std::string strKey(m_strName);
		strKey += "\\DefaultIcon";

		char buffer[20];
		std::string strVal(m_strIconApp);
		strVal += ",";
		strVal += itoa(m_nIconIndex, buffer, 10);

		Key_CreateRoot(strKey.c_str(), "", strVal.c_str());		//write default icon
	}

	for(unsigned int i=0; i<m_lstCmds.size(); i++)
	{
		int nDivider = m_lstCmds[i].find("\t");

		std::string strKey(m_strName);
		strKey += "\\Shell\\";
		strKey += m_lstCmds[i].substr(0, nDivider);

		//TOFIX action can have a description
		Key_CreateRoot(strKey.c_str(), "", "");					//write action name

		strKey += "\\command";
		Key_CreateRoot(strKey.c_str(), "", m_lstCmds[i].substr(nDivider+1,2000).c_str());		//write action command subkey
	}

	//sett default shell action - wrtie action name under "/Shell/"
	if(!m_strDefaultCmd.empty())
	{
		std::string strKey(m_strName);
		strKey += "\\Shell\\";

		Key_CreateRoot(strKey.c_str(), "", m_strDefaultCmd.c_str()); //write action name
	}
}

//create registry key, and write name/value pair (of string type) into it
//registry path starts at root (hardcoded limit), and can be up to 4 levels deep (RegCreateKey limitation)
bool RegisterFileExtension::Key_CreateRoot(const char *szPath, const char *szName, const char *szValue)
{
	HKEY	hBase = HKEY_CLASSES_ROOT;	// handle to the parent key
	HKEY	hKey;						// handle to the open registry key

	LONG lRes = RegCreateKey(hBase, szPath, &hKey);
	if(lRes == ERROR_SUCCESS)
	{
		//write the key/value
		if( NULL != szValue &&
			'\0' != *szValue)
		{
			RegSetValue(hKey, szName, REG_SZ, szValue, strlen(szValue));
		}

		RegCloseKey(hKey);
		return true;
	}

	return false;
}

bool RegisterFileExtension::IsRegistered(const char *szExt)
{
	HKEY hKey; // handle to the open registry key
	if(ERROR_SUCCESS == RegOpenKey(HKEY_CLASSES_ROOT, szExt, &hKey))
	{
		//check if our program is the main handler
		char szValue[2048];
		DWORD dwValueSize = sizeof(szValue);
		LONG lRes = RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE)szValue, &dwValueSize);
        if(ERROR_SUCCESS == lRes)
		{
			if(0 == strcmp(m_strName.c_str(), szValue))
			{
				//check if the the app path for this format is valid one
				std::string strKey2 = m_strName;
				strKey2 += "\\Shell\\open\\command";

				HKEY hKey2;
				lRes = RegOpenKey(HKEY_CLASSES_ROOT, strKey2.c_str(), &hKey2);
				if(ERROR_SUCCESS == lRes)
				{
					dwValueSize = sizeof(szValue);

					lRes = RegQueryValueEx(hKey2, NULL, NULL, NULL, (LPBYTE)szValue, &dwValueSize);
					if(ERROR_SUCCESS == lRes)
					{
						std::string strApp(szValue);
						strApp = strApp.substr(0, strApp.size()-5); // strip ' "%1"' frpm cmd string

						//check if the the value - app path is valid one
						if(0 != access(strApp.c_str(), 00))
							lRes = 1000;	// some error
					}

					RegCloseKey(hKey2);
				}

				RegCloseKey(hKey);
				return (ERROR_SUCCESS == lRes);
			}
		}

		RegCloseKey(hKey);
	}
	return false;
}
