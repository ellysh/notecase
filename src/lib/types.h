////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Portable definition of some needed types
////////////////////////////////////////////////////////////////////////////

#ifndef TYPES_H__
#define TYPES_H__

#ifdef _WIN32
 #include <windows.h>
 typedef __int64 INT64;
#else
 typedef long long INT64;
#endif

#endif	// TYPES_H__
