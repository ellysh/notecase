////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Execute any file path or URL
////////////////////////////////////////////////////////////////////////////

#include <glib.h>
#include "../ExecuteFile.h"
#include "../config.h"
#include "../lib/debug.h"
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <string.h>

#if defined(HAVE_GNOME_VFS)
 #include <libgnomevfs/gnome-vfs.h>
 #include <libgnomevfs/gnome-vfs-mime-handlers.h>
#else
 #include <map>
 static std::map<std::string, std::string> g_mapExt2Mime;
 static std::map<std::string, std::string> g_mapMime2Prog;
 static bool g_bMimeMapsLoaded = false;
 bool LoadMimeTypes(const char *szTypesFile, const char *szProgsFile);
#endif

void trim(std::string& str);
std::string GetFileExt(const char *);
bool Open(const std::string& path, const std::string& args);
std::string FileNameFromUTF8(const std::string& fileNameInUTF8);

bool ExecuteFile(const char *szFile, const char *szArgs, const char *szDir, void *nData)
{
	return Open(szFile, szArgs);
}

namespace Glib
{

template<typename T>
struct ListTraits
{
	typedef T CType;
	typedef T CppType;

	static CType ToCType(const CppType& value) { return value; }
	static CppType ToCppType(const CType& value) { return value; }
	static void ReleaseCType(const CType& ) {}
};

template<>
struct ListTraits<std::string>
{
	typedef char* CType;
	typedef std::string CppType;

	static CType ToCType(const CppType& value)
	{
		return static_cast<CType>(g_memdup(value.c_str(), value.size() + 1));
	}
	static CppType ToCppType(const CType& value)
	{
		return value;
	}
	static void ReleaseCType(const CType& value)
	{
		g_free(value);
	}
};

// GList wrapper.
// TODO: Consider using Glibmm instead of reimplementing wrappers.
template< typename T, typename Traits = ListTraits<T> >
class List
{
public:
	List() : m_handle(0) {}
	// transfers ownership
	List(const List& other) : m_handle(g_list_copy(other.m_handle))
	{
		other.m_handle = 0;
	}
	virtual ~List()
	{
		for (GList* node = m_handle; node != 0; node = node->next)
			Traits::ReleaseCType(static_cast<typename Traits::CType>(node->data));
		g_list_free(m_handle);
	}

	const List& operator=(const List& other)
	{
		List temp(other);
		Swap(temp);
	}

	void Swap(List& other)
	{
		std::swap(m_handle, other.m_handle);
	}

	GList* GetHandle() const
	{
		return m_handle;
	}

	void Append(const T& t)
	{
		m_handle = g_list_append(m_handle, Traits::ToCType(t));
	}

private:
	GList* m_handle;
};

} // namespace Glib

std::string FileNameFromUTF8(const std::string& fileNameInUTF8)
{
	std::string fileName;
	gsize bytesWritten = 0;

	// On Windows GLib uses UTF-8 for filenames, but as well as we operate
	// on native Windows API, conversion to the encoding used in current
	// locale is required.

#if defined(G_OS_WIN32)
	gchar *pFileName = g_locale_from_utf8(fileNameInUTF8.c_str(), -1, 0, &bytesWritten, 0);
#else
	gchar *pFileName = g_filename_from_utf8(fileNameInUTF8.c_str(), -1, 0, &bytesWritten, 0);
#endif

	if (!pFileName)
	{
		// In case of error leave the string intact and hope for the best.
		fileName = fileNameInUTF8;
	}
	else{
		fileName = pFileName;
		g_free(pFileName);
	}

	return fileName;
}

#if defined(HAVE_GNOME_VFS)

// Namespace for Gnome VFS wrappers.
namespace GnomeVFS
{

// Wrapper for Gnome VFS URI.
class URI
{
public:
	typedef GnomeVFSURI* (URI::*HandleMemPtr);

	URI() : m_handle(0) {}

	// Transfers ownership.
	explicit URI(GnomeVFSURI* handle) : m_handle(handle) {}

	explicit URI(const std::string& textURI) : m_handle(gnome_vfs_uri_new(textURI.c_str())) {}

	URI(const URI& other) : m_handle(other.m_handle)
	{
		if (m_handle != 0)
			gnome_vfs_uri_ref(m_handle);
	}

	virtual ~URI()
	{
		if (m_handle != 0)
			gnome_vfs_uri_unref(m_handle);
	}

	const URI& operator=(const URI& other)
	{
		URI temp(other);
		Swap(temp);
		return *this;
	}

	void Swap(URI& other)
	{
		std::swap(m_handle, other.m_handle);
	}

	// Provide this operator instead of operator bool to avoid undesired
	// conversions.
	operator HandleMemPtr() const
	{
		return m_handle != 0 ? &URI::m_handle : 0;
	}

	std::string ToString() const;

	void AppendPath(const std::string& path)
	{
		if (m_handle != 0)
		{
			URI newURI(gnome_vfs_uri_append_path(m_handle, path.c_str()));
			Swap(newURI);
		}
	}

	URI GetParent() const
	{
		return URI(m_handle != 0 ? gnome_vfs_uri_get_parent(m_handle) : 0);
	}

	GnomeVFSURI* GetHandle() const
	{
		return m_handle;
	}

private:
	GnomeVFSURI* m_handle;
};

std::string URI::ToString() const
{
	std::string str;
	if (m_handle != 0)
	{
		gchar *buffer = gnome_vfs_uri_to_string(m_handle, GNOME_VFS_URI_HIDE_NONE);
		str = buffer;
		g_free(buffer);
	}
	return str;
}

// Wrapper for Gnome VFS mime application.
class MimeApplication
{
public:
	typedef GnomeVFSMimeApplication* (MimeApplication::*HandleMemPtr);

	MimeApplication() : m_handle(0) {}

	// Transfers ownership
	explicit MimeApplication(GnomeVFSMimeApplication* handle) : m_handle(handle) {}

	MimeApplication(const MimeApplication& other) : m_handle(other.m_handle)
	{
		if (m_handle != 0)
			gnome_vfs_mime_application_copy(m_handle);
	}

	virtual ~MimeApplication()
	{
		if (m_handle != 0)
			gnome_vfs_mime_application_free(m_handle);
	}

	const MimeApplication& operator=(const MimeApplication& other)
	{
		MimeApplication temp(other);
		Swap(temp);
		return *this;
	}

	void Swap(MimeApplication& other)
	{
		std::swap(m_handle, other.m_handle);
	}

	// Provider this operator instead of operator bool to avoid undesired
	// conversions.
	operator HandleMemPtr() const
	{
		return m_handle != 0 ? &MimeApplication::m_handle : 0;
	}

	GnomeVFSResult Launch(const Glib::List<std::string>& uris) const
	{
		return gnome_vfs_mime_application_launch(m_handle, uris.GetHandle());
	}

private:
	GnomeVFSMimeApplication* m_handle;
};

// Determines the mime type of uri.
// Returns mime type or empty string if it can't be determined.
std::string GetMimeType(const URI& uri)
{
	std::string mimeType;
	// We don't need to free buffer.
	char* const buffer = gnome_vfs_get_mime_type(uri.ToString().c_str());
	if (buffer != 0)
		mimeType = buffer;
	return mimeType;
}

MimeApplication MimeGetDefaultApplication(const std::string& mimeType)
{
	return MimeApplication(
		gnome_vfs_mime_get_default_application(mimeType.c_str()));
}

} // namespace GnomeVFS

#endif // defined(HAVE_GNOME_VFS)

bool Execute(const std::string& path, const std::string& args, const std::string& dir)
{
	const char* const pathStr = path.c_str();
	std::vector<char> pathBuffer(pathStr, pathStr + path.size() + 1);

	const char* const argsStr = args.c_str();
	std::vector<char> argsBuffer(argsStr, argsStr + args.size() + 1);

	// TODO: support more than one argument
	std::vector<char*> argv;
	argv.push_back(&pathBuffer[0]);
	if (!args.empty())
		argv.push_back(&argsBuffer[0]);
	argv.push_back(0);

	return g_spawn_async(dir.c_str(), &argv[0], 0, G_SPAWN_SEARCH_PATH, 0, 0, 0, 0);
}

bool Open(const std::string& path, const std::string& args)
{
#if defined(HAVE_GNOME_VFS)
	TRACE("Using GnomeVFS\n");

	std::string mimeType;
	std::string strURI;

	// first try heuristic - http link would crash in GetMimeType
	    if (path.substr(0, 7) == "http://")
        	mimeType = "text/html";
	    else if (path.substr(0, 8) == "https://")
        	mimeType = "text/html";
	    else if (path.substr(0, 6) == "ftp://")
        	mimeType = "text/html";
	    else if (path.substr(0, 7) == "mailto:")
        	mimeType = "message/rfc822";
	else if(std::string::npos != path.find("://"))
	{
		GnomeVFS::URI uri(path);
		mimeType = GnomeVFS::GetMimeType(uri);
		strURI = uri.ToString();
		TRACE("URI:%s,host:%s\n", gnome_vfs_make_uri_from_input(path.c_str()),
			          gnome_vfs_uri_get_host_name(uri.GetHandle()));
		//TRACE("mime:%s\n",gnome_vfs_mime_type_from_name(path.c_str()));
	}
	else
	{
		//assume local file (no "://" found)
		GnomeVFS::URI uri("file:///");
		uri.AppendPath(path);
		mimeType = GnomeVFS::GetMimeType(uri);
		strURI = uri.ToString();
	}

	if(strURI.empty())
		strURI = path;

	TRACE("Open path:%s - URI:%s, mime type:%s\n", path.c_str(), strURI.c_str(), mimeType.c_str());
	if (!mimeType.empty())
	{
		GnomeVFS::MimeApplication app(GnomeVFS::MimeGetDefaultApplication(mimeType));
		if (app)
		{
			Glib::List<std::string> uris;
			uris.Append(strURI);
			return app.Launch(uris) == GNOME_VFS_OK;
		}
	}
	else
		return g_spawn_command_line_async(path.c_str(), 0);

	return g_spawn_command_line_async(FileNameFromUTF8(path).c_str(), 0);
#else
	if(!g_bMimeMapsLoaded){
		LoadMimeTypes("/etc/mime.types", "/etc/mailcap");
		g_bMimeMapsLoaded = true;
	}

	std::string strMime;
	// first try heuristic - http link would crash in GetMimeType
	    if (path.substr(0, 7) == "http://")
        	strMime = "text/html";
	    else if (path.substr(0, 8) == "https://")
        	strMime = "text/html";
	    else if (path.substr(0, 6) == "ftp://")
        	strMime = "text/html";
	    else if (path.substr(0, 7) == "mailto:")
        	strMime = "message/rfc822";
	else{
		std::string strExt = GetFileExt(path.c_str());
		if(!strExt.empty()) strExt = strExt.substr(1); // strip "."
		if(!strExt.empty()){
			TRACE("Ext: %s\n", strExt.c_str());
			strMime = g_mapExt2Mime[strExt];
		}
  }

		if(!strMime.empty()){
			TRACE("Mime: %s\n", strMime.c_str());
			std::string strProg = g_mapMime2Prog[strMime];
			if(strProg.empty()){
				//last chance - try alternative mime
				// image/jpeg -> image/*
				std::string::size_type nPos = strMime.find('/');
				if(nPos != std::string::npos){
					strMime = strMime.substr(0, nPos+1);
					strMime += "*";
					TRACE("Mime2: %s\n", strMime.c_str());
					strProg = g_mapMime2Prog[strMime];
				}
			}

			if(!strProg.empty()){
				TRACE("Prog: %s\n", strProg.c_str());
				char szBuffer[2048];
				sprintf(szBuffer, strProg.c_str(), path.c_str());
				TRACE("Exec: %s\n", szBuffer);
				return g_spawn_command_line_async(szBuffer, 0);	// TOFIX from utf8?
			}
		}

	return g_spawn_command_line_async(FileNameFromUTF8(path).c_str(), 0);
#endif
}

bool LoadMimeTypes(const char *szTypesFile, const char *szProgsFile)
{
#ifndef HAVE_GNOME_VFS
	//load MIME types
	FILE *pTFile = fopen(szTypesFile, "r");
	if(NULL == pTFile)
		return false;

	char szBuffer[1024];
	while(NULL != fgets(szBuffer, sizeof(szBuffer), pTFile))
	{
		//kill new line char
		if('\n'== szBuffer[strlen(szBuffer)-1])
			szBuffer[strlen(szBuffer)-1] = '\0';

		if( strlen(szBuffer) > 0 &&
			szBuffer[0] != '#')
		{
			std::string strLine = szBuffer;

			//extract mime type
			std::string strMime;
			std::string::size_type nPos = strLine.find('\t');
			if(nPos != std::string::npos)
				strMime = strLine.substr(0, nPos);
			trim(strMime);

			nPos = strLine.find_last_of('\t');
			if(nPos != std::string::npos)
				strLine = strLine.substr(nPos+1);

			//extract extensions (delimited with space)
			if(!strMime.empty()){
				while(std::string::npos != (nPos = strLine.find(' ')))
				{
					std::string strExt = strLine.substr(0, nPos);
					g_mapExt2Mime[strExt] = strMime;
					TRACE("Mime [%s]=[%s]\n", strExt.c_str(), strMime.c_str());
					strLine = strLine.substr(nPos+1);
				}
				if(!strLine.empty()){
					g_mapExt2Mime[strLine] = strMime;
					TRACE("Mime [%s]=[%s]\n", strLine.c_str(), strMime.c_str());
				}
			}
		}
	}
	fclose(pTFile);

	//load MIME handler programs
	FILE *pPFile = fopen(szProgsFile, "r");
	if(NULL == pPFile)
		return false;

	while(NULL != fgets(szBuffer, sizeof(szBuffer), pPFile))
	{
		//kill new line char
		if('\n'== szBuffer[strlen(szBuffer)-1])
			szBuffer[strlen(szBuffer)-1] = '\0';

		if( strlen(szBuffer) > 0 &&
			szBuffer[0] != '#')
		{
			std::string strLine = szBuffer;

			//extract mime type
			std::string strMime;
			std::string::size_type nPos = strLine.find("; ");
			if(nPos != std::string::npos){
				strMime = strLine.substr(0, nPos);
				trim(strMime);
				strLine = strLine.substr(nPos+2);
				nPos = strLine.find("; ");
				if(nPos != std::string::npos)
					strLine = strLine.substr(0, nPos);

				g_mapMime2Prog[strMime] = strLine;
				TRACE("Prog [%s]=[%s]\n", strMime.c_str(), strLine.c_str());
			}
		}
	}
	fclose(pPFile);
#endif

	return true;
}

void trim(std::string& str)
{
  std::string::size_type pos = str.find_last_not_of(' ');
  if(pos != std::string::npos) {
    str.erase(pos + 1);
    pos = str.find_first_not_of(' ');
    if(pos != std::string::npos) str.erase(0, pos);
  }
  else str.erase(str.begin(), str.end());
}
