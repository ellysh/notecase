////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Methods to work with file path strings
////////////////////////////////////////////////////////////////////////////

#ifndef _FILE_PATH__
#define _FILE_PATH__

#include <string>

std::string GetHomeDir();
std::string GetAppPath();

bool EnsureDirExists(std::string &strDir);
std::string GetParentDir(const char *szPath);
std::string GetBaseName(const char *szPath);
std::string GetFileExt(const char *szPath);
std::string GetTempDirectory();

void EnsureTerminated(std::string &strPath, char cDelimiter = '/');
void EnsureNotTerminated(std::string &strPath);
void EnsureExtension(std::string &strPath, const char *szExt);

#endif  //_FILE_PATH__
