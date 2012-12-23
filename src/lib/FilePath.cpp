////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Methods to work with file path strings
////////////////////////////////////////////////////////////////////////////

#include "FilePath.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <stdlib.h>

#ifdef _WIN32
 #include <windows.h>
 #include <direct.h> //_mkdir
 #include <shlobj.h>
 #ifndef CSIDL_PROFILE
  #define CSIDL_PROFILE 40
 #endif
#else
 #include <unistd.h>
#endif

#define PATH_DELIMITERS "\\/"

std::string GetTempDirectory()
{
#ifdef _WIN32
	TCHAR szPathBuffer[MAX_PATH] = "";
	GetTempPath(MAX_PATH, szPathBuffer);
	return std::string (szPathBuffer);
#else
	return std::string ("/tmp/");
#endif
}

std::string GetHomeDir()
{
	static bool found = false;
    static std::string strDir;

    if (found)
        return strDir;
    else
    {
		//first try to see if the user has set special home destination
		//through the NOTECASE_HOME environment variable
		const char *szEnv = getenv("NOTECASE_HOME");
		if(NULL != szEnv)
			strDir = szEnv;

		if(strDir.empty())
			strDir = g_get_home_dir();

		found = true;
	}
    return strDir;
}

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

		if (snprintf(linkname, sizeof(linkname), "/proc/%d/exe", (int)pid) < 0)
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

bool IsRootDir(std::string &strDir)
{
    #ifdef _WIN32
        if(strDir.size() <= 3)
            return true;
    #else
        if(strDir.size() < 2)
            return true;
    #endif

    return false;
}

//NOTE: only for local paths
bool EnsureDirExists(std::string &strDir)
{
    if (IsRootDir (strDir))
        return true;    //no more searching (root/volume level)
    // remove ending / if exists
    if( strDir.substr(strDir.size()-1) == "/" || strDir.substr(strDir.size()-1) == "\\")
        strDir = strDir.substr(0, strDir.size()-1);

    // check if the directory already exists
#ifdef _WIN32
 #define S_IFDIR _S_IFDIR
#endif
    struct stat st;
    if(0 == g_stat(strDir.c_str(), &st)){
        //file exists, check if it is directory
        return (S_IFDIR == (st.st_mode & S_IFDIR));
    }

    // recursively check parent directory
    int nPos = strDir.find_last_of(PATH_DELIMITERS);
    if(-1 == nPos)
        return true;    //no more searching (root/volume level)

	std::string strParentDir = strDir.substr(0, nPos);
    if(!EnsureDirExists(strParentDir))
        return false;

    //now create this directory
#ifdef _WIN32
 #define mkdir _mkdir
#endif
#ifdef _WIN32
	return (0 == mkdir(strDir.c_str()));
#else
	return (0 == mkdir(strDir.c_str(), 0777));
#endif
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

std::string GetBaseName(const char *szPath)
{
	std::string strPath(szPath);
    EnsureNotTerminated(strPath);

	size_t nPos = strPath.find_last_of(PATH_DELIMITERS);
	if(nPos != std::string::npos)
		return strPath.substr(nPos+1);

	return strPath;	//no delimiter found
}

void EnsureExtension(std::string &strPath, const char *szExt)
{
	size_t nPos = strPath.find_last_of(".");
	if(nPos != std::string::npos)
		strPath = strPath.substr(0, nPos);	//strip old ext

	strPath += szExt;	//append new one
}


std::string GetFileExt(const char *szPath)
{
	std::string strPath(szPath);
	size_t nPos = strPath.find_last_of(".");
	if(nPos != std::string::npos)
		return strPath.substr(nPos);	//strip old ext

	return "";
}
