////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Macros to help us debugging the program
////////////////////////////////////////////////////////////////////////////

#ifndef DEBUGLIB_H
#define DEBUGLIB_H

//functions used within macros belo
void Trace(const char *fmt, ...);
void Assert(int exp, const char *fmt, ...);

#ifdef _DEBUG

#define TRACE             Trace
#define INFO(x)           TRACE("%s (%d): ", __FILE__, __LINE__); TRACE(x); TRACE("\n")
#define ASSERT(x)         Assert(x, "%s (%d): Assert failed at: %s\n", __FILE__, __LINE__, #x)
#define ASSERTMSG(x,msg)  Assert(x,msg)

#else

#define TRACE             (1) ? (void)0 : Trace
#define INFO(x)           (1) ? (void)0 : Trace("","")
#define ASSERT(x)         (1) ? (void)0 : Assert(true,"")
#define ASSERTMSG(x,msg)  (1) ? (void)0 : Assert

#endif // _DEBUG

#endif	// DEBUGLIB_H
