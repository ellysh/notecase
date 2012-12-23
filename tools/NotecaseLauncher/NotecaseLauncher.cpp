// NotecaseLauncher.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"

#include <io.h>
#include <string>
#include <windows.h>
#include <shellapi.h>

#define PATH_DELIMITERS "\\/"

std::string GetAppPath()
{
	static bool found = false;
    static std::string path;

    if (!found)
    {
#ifdef _WIN32
        char buf[512];
        *buf = '\0';
        ::GetModuleFileName(NULL, buf, 511);
        path = buf;
#else
		//code adapted from public domain code by Nicolai Haehnle <prefect_@gmx.net>
 		char linkname[64]; /* /proc/<pid>/exe */
		int ret;
		
		/* Get our PID and build the name of the link in /proc */
		pid_t pid = getpid();
			
		if (snprintf(linkname, sizeof(linkname), "/proc/%i/exe", pid) < 0)
			return path; //error

		/* Now read the symbolic link */
		char buffer[2048];
		ret = readlink(linkname, buffer, sizeof(buffer));
		
		/* In case of an error, leave the handling up to the caller */
		if (ret == -1)
			return path;
		
		/* Report insufficient buffer size */
		if (ret >= (int)sizeof(buffer))
			return path;

		buffer[ret] = '\0';	//terminate the buffer

		path = buffer;
#endif
        found = true;
    }

	return path; 
}

void EnsureTerminated(std::string &strPath, char cDelimiter)
{
    if(strPath.empty())
        strPath += cDelimiter;
    else{
        char cLast = strPath.at(strPath.size()-1);
        if(cLast != '\\' && cLast != '/')
            strPath += cDelimiter;
    }
}

void EnsureNotTerminated(std::string &strPath)
{
    if(!strPath.empty())
    {
        char cLast = strPath.at(strPath.size()-1);
        if(cLast == '\\' || cLast == '/')
            strPath = strPath.substr(0, strPath.size()-1);
    }
}

std::string GetParentDir(const char *szPath)
{
	std::string strPath(szPath);
    EnsureNotTerminated(strPath);

    size_t nPos = strPath.find_last_of(PATH_DELIMITERS);
    if(nPos > 0)
        strPath = strPath.substr(0, nPos);
    else
        strPath = "";

#ifdef _WIN32
    EnsureTerminated(strPath, '\\');
#else
	EnsureTerminated(strPath, '/');
#endif

    return strPath;
}

void ShowSystemErrorMessage(UINT code)
{
	std::string message;
	char *buffer = NULL;
	if (FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		0, code, 0, buffer, 0, 0) != 0)
	{
		message = buffer;
		LocalFree(buffer);
	}
	else message = "Unknown error.";
	
	::MessageBox(NULL, (LPCTSTR)message.c_str(), "Error", MB_OK);
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{

	//detect if Notecase was started in portable mode 
	//(with no GTK installed on computer, but copied to the same directory as the Notecase.exe)
	std::string strDir = GetAppPath();
	strDir = GetParentDir(strDir.c_str());

	std::string strGtk = strDir;
	strGtk += "gtk\\";

	if(0 == _access(strGtk.c_str(), 0))	// GTK directory exists
	{
		//set variable so Notecase can detect portable mode in an easy way
		_putenv("NOTECASE_PORTABLE=1");

		//set directory for INI file
		char szBuffer[10024];
		_snprintf(szBuffer, sizeof(szBuffer)-1, "NOTECASE_HOME=%s", strDir.c_str());
		szBuffer[sizeof(szBuffer)-1] = '\0';
		_putenv(szBuffer);

		//set directory for help file
		std::string strHelp = strDir + "config\\help.ncd";
		_snprintf(szBuffer, sizeof(szBuffer)-1, "NOTECASE_HELP=%s", strHelp.c_str());
		szBuffer[sizeof(szBuffer)-1] = '\0';
		_putenv(szBuffer);

		//set directory for ini file, unless already set
		const char *szEnv = getenv("NOTECASE_INI");
		if(NULL == szEnv){
			std::string strIni = strDir + "config\\notecase.ini";
			_snprintf(szBuffer, sizeof(szBuffer)-1, "NOTECASE_INI=%s", strIni.c_str());
			szBuffer[sizeof(szBuffer)-1] = '\0';
			_putenv(szBuffer);
		}
		
		//TOFIX do we need to preserve existing path
		const char *szPath = getenv("PATH");
		int nLen = strlen(szPath);
		std::string strGtkBin = strGtk + "bin\\";
		_snprintf(szBuffer, sizeof(szBuffer)-1, "PATH=%s;%s;%s", strGtk.c_str(), strGtkBin.c_str(), szPath);
		szBuffer[sizeof(szBuffer)-1] = '\0';
		_putenv(szBuffer);

		std::string strURL = strDir;
		strURL += "app\\Notecase.exe";

		HINSTANCE hInstance = ShellExecute(NULL, "open", strURL.c_str(), (NULL != lpCmdLine) ? lpCmdLine : "", "", SW_SHOW);
		DWORD result = reinterpret_cast<DWORD>(hInstance);
		if (result <= HINSTANCE_ERROR)
		{
			ShowSystemErrorMessage(result); // format and show system error message
			return 1;
		}
	}
	else
		MessageBox(NULL, "Failed to find GTK directory!", "ERROR", MB_OK);

	return 0;
}
