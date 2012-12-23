////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements basic HTML parser class
////////////////////////////////////////////////////////////////////////////

#ifndef HTMLParser_H__
#define HTMLParser_H__

//
// This class is a simple HTML parser, that is being fed with buffer chunks.
// Result is a series of events (virtual method calls) that signalize
// occurence of HTML entites like: tag, tag ending, comment or ordinary text in the stream
//
// In order to use this parser, you must inherit this class and override its event handlers
// to perform custom processing of HTML parse events.
//
// Additionally, this class offers two methods for escaping and unescaping of some special
// characters (for example char '<' is "escaped" to string "&lt;" when saving HTML document to file)
// These methods rely on internal character conversion table, sorted by numerical value
// of the character, in order to speed up algorithm by using binary-search on the table.
// Characters are UTF-8, and they can be multibyte - more than one byte long.
//

#include <string>

class HTMLParser
{
public:
	HTMLParser();
	virtual ~HTMLParser();

	void Clear();
	bool Parse(const char *szBuffer, int len);
	void Finalize();

	static void EscapeChars(std::string &data);
	static void UnescapeChars(std::string &data);
	static void EscapeURI(std::string &data);
	static void UnescapeURI(std::string &data);
	static bool ExtractParam(const std::string &data, const char *szParam, std::string &resValue);

	bool m_bAllowUnescapedInPreTag;

protected:
	virtual void OnTagBegin(const char *szTag, const char *szParams = NULL) = 0;
	virtual void OnTagEnd(const char *szTag) = 0;
	virtual void OnComment(const char *szText) = 0;
	virtual void OnText(const char *szText) = 0;

protected:
	int m_nState;
	std::string m_strData;
	bool m_bInsidePreTag;
};

#endif
