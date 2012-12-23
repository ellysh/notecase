////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Enumerate files and directories in the given directory (recursive mode supported)
////////////////////////////////////////////////////////////////////////////

#include "EnumDirectory.h"
#define HAVE_READDIR_R

#include "../config.h"
#ifdef _WIN32
 #include <windows.h>
 #include <winbase.h>
 #include <io.h>		//_findfirst
 #include <stdio.h>     //_findfirst
#else
 #include <dirent.h>	// readdir_r()
 #include <libgen.h>	// basename()
#endif

#include <sys/stat.h>	// stat()
#include <errno.h>		// errno
#include <limits.h>
#include <string.h>		//
#include <stdio.h>		//


#ifdef _WIN32

//Win32 version
int EnumDirectory(const char *szPath, FN_ENUMDIR func, void *userdata, int nFilter)
{
	char szFileSpec[256];
	char dir[256] = "";			//buffer for dir name of found file

	//get current directory name
	strcpy(dir, szPath);
	char *pszDirName = strrchr(dir, '\\');
	if(pszDirName){
		if(pszDirName == dir+strlen(dir)){
			*pszDirName = '\0';
			pszDirName = strrchr(dir, '\\');
		}
		memmove(dir, pszDirName+1, strlen(pszDirName+1)+1);
	}

    // create string like "dir\*"
    strcpy(szFileSpec, szPath);
    if( '\\' != szFileSpec[strlen(szFileSpec)-1])
        strcat(szFileSpec, "\\");
    strcat(szFileSpec, "*");

    struct _finddata_t fileinfo;
    long lHandle = _findfirst(szFileSpec, &fileinfo);
	//TOFIX check for ENUM_ERR_DIR

    int nProceed = lHandle;
    while(-1L != nProceed)
    {
		char szFullPath[256];

		// ignore entries "." and ".."
		if (strcmp(fileinfo.name, ".") && strcmp(fileinfo.name, ".."))
		{
			// create string like "dir\file_name"
			strcpy(szFullPath, szPath);
			if( '\\' != szFullPath[strlen(szFullPath)-1])
				strcat(szFullPath, "\\");

			strcat(szFullPath, fileinfo.name);

			if(0 == (fileinfo.attrib & _A_SUBDIR))	// if file
			{
				if(nFilter & ENUM_LST_FILES)
					if(!func(szFullPath, userdata)){
						_findclose(lHandle);
						return ENUM_ERR_OK;	//abort request
					}
			}
			else	// if directory
			{
				if(nFilter & ENUM_LST_DIRS)
					if(!func(szFullPath, userdata)){
						_findclose(lHandle);
						return ENUM_ERR_OK;	//abort request
					}

				if(nFilter & ENUM_RECURSIVE)
				{
					int r = EnumDirectory(szFullPath, func, userdata, nFilter);
					if (r < 0) {
						//error
						_findclose(lHandle);
						return r;
					}
				}
			}
		}

        nProceed = _findnext(lHandle, &fileinfo); // find next
    }

	if(-1L != lHandle)
		_findclose(lHandle);
	return ENUM_ERR_OK;
}

#else

//Unix version
int EnumDirectory(const char *szPath, FN_ENUMDIR func, void *userdata, int nFilter)
{
	struct	dirent *resp;
	char	buf[sizeof(struct dirent)+MAXNAMLEN];	//dirent struct buffer

	char	tmp[MAXNAMLEN];	//buffer for full path of found file
	char	dir[MAXNAMLEN]; //buffer for dir name of found file
	struct	stat statbuf;
	int		r;

	//open given directory
	DIR *dirp = opendir(szPath);
	if (dirp == NULL) {
		//error: failed to open directory
		return ENUM_ERR_DIR;
	}

	// gate current subdirectory name
	strcpy(dir, basename((char *)szPath));

	//read contents for given subdirectory
#ifdef HAVE_READDIR_R
	//use recursive version for readdir function
	while ((readdir_r(dirp, (struct dirent *)buf, &resp) == 0) && (resp != NULL))
	{
#else
	// for platforms with no recursive support
	while ((resp = readdir(dirp)))
	{
		if (errno != 0) break;
#endif
		// ignore entries "." and ".."
		if ( strcmp(resp->d_name, ".") == 0 ||
		     strcmp(resp->d_name, "..") == 0)
			continue;

		//create full path
#ifdef HAVE_SNPRINTF
		snprintf(tmp, sizeof(tmp), "%s/%s", szPath, resp->d_name);
#else
		sprintf(tmp, "%s/%s", szPath, resp->d_name);
#endif

		// get info on current entry (is it directory or file)
		r = stat(tmp, &statbuf);
		if (r == -1) {
			//error: failed to stat the file tmp
			closedir(dirp);
			return ENUM_ERR_FILE;
		}

		if (S_ISDIR(statbuf.st_mode)) 	// if directory
		{
			if(nFilter & ENUM_LST_DIRS)
			{
				if(!func(tmp, userdata))	// pass the data to the callback method
				{
					closedir(dirp);
					return ENUM_ERR_OK;	//abort request by user
				}
			}

			if(nFilter & ENUM_RECURSIVE)
			{
				//recurse into subdirectory
				r = EnumDirectory(tmp, func, userdata, nFilter);
				if (r < 0) {
					//error
					closedir(dirp);
					return r;
				}
			}
		}
		else // if file
		{
			if(nFilter & ENUM_LST_FILES){
				if(!func(tmp, userdata))	// pass the data to the callback method
				{
					closedir(dirp);
					return ENUM_ERR_OK;	//abort request by user
				}
			}
		}
	}

	closedir(dirp);
	return ENUM_ERR_OK;
}

#endif
