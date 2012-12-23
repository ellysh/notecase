////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Enumerate files and directories in the given directory (recursive mode supported)
////////////////////////////////////////////////////////////////////////////

#ifndef ENUMDIRECTORY_H_
#define ENUMDIRECTORY_H_

//TOFIX clean documentation, cleaner code/variables, merge code for Win and Unix?

//enumeration description/filtering flags
#define ENUM_LST_FILES	0x01
#define ENUM_LST_DIRS	0x02
#define ENUM_LST_ALL	(ENUM_LST_FILES|ENUM_LST_DIRS)
#define ENUM_RECURSIVE	0x04

//enumeratio error codes
#define ENUM_ERR_OK	0
#define ENUM_ERR_DIR	-1	//failed to open dir
#define ENUM_ERR_FILE	-2	//failed to stat file

//callback method definiton
//  parameters: file path, user data
//     returns: true - keep enumerating, false - stop enumerating
typedef bool (* FN_ENUMDIR)(const char *, void *);

// For each file in given directory call user's callback method
int EnumDirectory(const char *szPath, FN_ENUMDIR func, void *userdata, int nFilter = ENUM_LST_FILES);

#endif  //ENUMDIRECTORY_H_
