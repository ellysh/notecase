////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: object of this class should be alive as long as program lives
//		 so that instance can be detected (borowed from gnome-volume-manager)
////////////////////////////////////////////////////////////////////////////

#include "SingleInstance.h"
#include "../lib/FilePath.h"
#include "../interface.h"
#include "../lib/EnumDirectory.h"
#include <stdlib.h>
#include <string.h>
#include <vector>

#ifndef WIN32
 #include <libgen.h>	//basename
#endif

bool OnLockFileEnum(const char *szFile, void *data);
const char *get_config_dir();

CSingleInstance::CSingleInstance(const char *szName)
{
//calculate PID
	unsigned int nProcessID = GetCurrentPID();

	std::string strDir = get_config_dir();
	EnsureTerminated(strDir, '/');

	//create our own .lock file ASAP
	char szBuffer[1024];
	sprintf(szBuffer, "%snotecase_n%d.lock", strDir.c_str(), nProcessID);
	FILE *pTmp = fopen(szBuffer, "w");
	if(pTmp)
		fclose(pTmp);

	//list existing .lock files
	std::vector<std::string> lstFiles;
	EnumDirectory(strDir.c_str(), OnLockFileEnum, (void *)&lstFiles, ENUM_LST_FILES);

	int nCount = lstFiles.size();
	for(int i=0; i<nCount; i++)
	{
			std::string strPath = lstFiles[i];

		#ifdef _WIN32
			//calculate base name
			std::string strFile(strPath);
			int nPos1 = strFile.find_last_of('\\');
			if(nPos1 > 0)
				strFile = strFile.substr(nPos1+1);
		#else
			std::string strFile(basename((char *)strPath.c_str()));
		#endif

			//try to extract PID from file name (if exists)
			unsigned int nPID = 0;
			std::string strPID = strFile;
			std::string::size_type nPos = strPID.rfind('_');
			if(nPos != std::string::npos){
				strPID = strPID.substr(nPos+1);
				if(!strPID.empty() && strPID.at(0) == 'n') //correct segment with PID
				{
					strPID = strPID.substr(1); // cut 'n' marker

					nPos = strPID.find('.');
					if(nPos != std::string::npos)
						strPID = strPID.substr(0, nPos);

					nPID = atoi(strPID.c_str());
				}
			}

			if(nPID <= 0){
				remove(strPath.c_str());
				continue;
			}
			if(nPID == nProcessID)
				continue;	// our own PID, ignore

			//check if PID belongs to a running application
			if(IsPIDRunning(nPID))
			{
				m_bAlreadyExists = true;
				return; // skip this file, app that created it is still running
			}
			else
				remove(strPath.c_str());
		}

		m_bAlreadyExists = false;	//no lock files with running PIDs
}

CSingleInstance::~CSingleInstance()
{
	//remove .lock file on exit

	//calculate PID
	unsigned int nProcessID = GetCurrentPID();

	std::string strDir = get_config_dir();
	EnsureTerminated(strDir, '/');

	char szBuffer[1024];
	sprintf(szBuffer, "%snotecase_n%d.lock", strDir.c_str(), nProcessID);
	remove(szBuffer);
}

bool CSingleInstance::ProgramAlreadyStarted()
{
	return m_bAlreadyExists;
}

bool OnLockFileEnum(const char *szFile, void *data)
{
	std::vector<std::string> *lstFiles = (std::vector<std::string> *)data;
	if(data)
	{
	#ifdef _WIN32
		//calculate base name
		std::string strFile(szFile);
		int nPos = strFile.find_last_of('\\');
		if(nPos > 0)
			strFile = strFile.substr(nPos+1);
	#else
		std::string strFile(basename((char *)szFile));
	#endif

		if(0 == strncmp(strFile.c_str(), "notecase_", strlen("notecase_")) &&
			 GetFileExt(strFile.c_str()) == ".lock")
		{
			lstFiles->push_back(szFile);
		}
	}

	return true;	//keep enumerating
}
