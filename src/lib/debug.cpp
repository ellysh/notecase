////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Macros to help us debugging the program
////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <cassert>

#ifdef _WIN32
 #define vsnprintf _vsnprintf
 #include <windows.h>
#endif

void DbgOutput(const char *szText);

void Trace(const char *fmt, ...)
{
	char buffer[1024] = "";

	// create string using format and list of parameters
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer)-1, fmt, args);
	va_end(args);

	DbgOutput(buffer);
}

void Assert(int exp, const char *fmt, ...)
{
	if(!exp)
	{
		char buffer[1024] = "";

		// create string using format and list of parameters
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, sizeof(buffer)-1, fmt, args);
		va_end(args);

		DbgOutput(buffer);

	#ifdef _WIN32
		assert(false);	//trigger debugger event
	#else
		exit(1);
	#endif
    }
}

void DbgOutput(const char *szText)
{
#ifdef _WIN32
	OutputDebugString(szText);
#else
	printf("%s", szText);
#endif
}
