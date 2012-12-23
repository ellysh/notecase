////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements file operations with large file support (64-bit size)
////////////////////////////////////////////////////////////////////////////

#include "File64.h"
#include "debug.h"

#include <stdlib.h>

#ifdef _WIN32
 #ifndef __MINGW32__
  #include <io.h>
  #include <glib.h>
  #include <glib/gstdio.h>
 #else
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <unistd.h>
 #endif
#else
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <unistd.h>
#endif

#include <fcntl.h>
#include <errno.h>

File64::File64()
{
	m_nFile	= -1;
}

File64::~File64()
{
	Close();
}

INT64 File64::FileSize(const char *szPath)
{
	//TOFIX 64bit support
    struct stat statbuf;
	int r = stat(szPath, &statbuf);
    if (r == -1)
    {
        //dTRACE("Failed finding status of: %s\n[errno = %d]\n", full_path, errno);
        return -1;
    }
	return statbuf.st_size;
}

bool File64::Open(const char *szPath, unsigned long nFlags)
{
	int nOpenFlags = 0;
	if(nFlags & F64_READ)
		nOpenFlags |= O_RDONLY;
	if(nFlags & F64_WRITE)
		nOpenFlags |= O_WRONLY;
	if((nFlags & (F64_READ|F64_WRITE)) == (F64_READ|F64_WRITE))
		nOpenFlags = O_RDWR;
	if(nFlags & F64_OPEN_NEW)
		nOpenFlags |= O_CREAT;
	if((nFlags & (F64_OPEN_NEW|F64_OPEN_EXISTING)) == (F64_OPEN_NEW|F64_OPEN_EXISTING))
		nOpenFlags |= O_TRUNC;

 #ifdef O_LARGEFILE
	nOpenFlags |= O_LARGEFILE;
 #endif
 #ifdef _WIN32
	nOpenFlags |= _O_BINARY;
 #endif

	int pmode = 0600; //default permissions are set to 0600
#ifdef _WIN32
	pmode = _S_IREAD | _S_IWRITE;
#endif

//"The foremost problem is GTK+ and GLib APIs that take or return C file
//descriptors (or FILE pointers). If your code uses a different C
//runtime, the int returned by g_open() or the FILE* returned by
//g_fopen() won't mean anything to your code"
#ifdef G_OS_WIN32
  if (G_WIN32_HAVE_WIDECHAR_API ())
    {
      wchar_t *wfilename = (wchar_t *)g_utf8_to_utf16 (szPath, -1, NULL, NULL, NULL);
      m_nFile	= _wopen (wfilename, nOpenFlags, pmode);
      int save_errno = errno;

	  TRACE("Error: %s\n", strerror(save_errno));
      g_free (wfilename);

      errno = save_errno;
    }
  else
    {
      gchar *cp_filename = g_locale_from_utf8 (szPath, -1, NULL, NULL, NULL);
      m_nFile	= open (cp_filename, nOpenFlags, pmode);
      int save_errno = errno;

      g_free (cp_filename);

      errno = save_errno;
    }
#else
	m_nFile	= open(szPath, nOpenFlags, pmode);
#endif

	return (m_nFile != -1);
}

void File64::Close()
{
	if(IsOpen())
		close(m_nFile);
	m_nFile = -1;
}

bool File64::IsOpen() const
{
	return (m_nFile != -1);
}

void File64::Flush()
{
	ASSERT(IsOpen());

#ifdef _WIN32
	//::FlushFileBuffers(m_hFile);
#else
	fsync(m_nFile);
#endif
}

int	 File64::Read(char *szBuffer, int nLen)
{
	ASSERT(IsOpen());

	return read(m_nFile, (void *)szBuffer, nLen);
}

int	File64::Write(const char *szBuffer, int nLen)
{
	ASSERT(IsOpen());

	int nWritten;

	nWritten = write(m_nFile, szBuffer, nLen);
	while(-1 == nWritten && EINTR == errno)
		nWritten = write(m_nFile, szBuffer, nLen);	// retry if interrupted

	return nWritten;
}

INT64 File64::Seek(INT64 offset)
{
	ASSERT(IsOpen());

	return lseek(m_nFile, offset, SEEK_SET);
}

INT64 File64::SeekEnd()
{
	ASSERT(IsOpen());

	return lseek(m_nFile, 0, SEEK_END);
}

INT64 File64::GetPosition() const
{
	ASSERT(IsOpen());

	return lseek(m_nFile, 0, SEEK_CUR);	//just return pos
}

int File64::ReadLine(char *szBuffer, int nLen)
{
	INT64 nPos = GetPosition();

	int nRead = Read(szBuffer, nLen);
	if(nRead > 0)
	{
		szBuffer[nLen-1] = '\0';

		char *szPos = strchr(szBuffer, '\n');
		if(szPos){
			int nBufPos = szPos - szBuffer;
			if(nBufPos < nLen-1){
				if(nBufPos > 0 && szBuffer[nBufPos-1] == '\r')
					*(szPos-1) = '\0';
				else
					*(szPos) = '\0';
			}
			Seek(nPos + nBufPos + 1);
		}
	}

	return nRead;
}
