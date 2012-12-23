////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements basic HTML parser class
////////////////////////////////////////////////////////////////////////////

#include "HtmlParser.h"
#include "debug.h"
#include <vector>
#include <algorithm>
#include <glib.h>
#include <gtk/gtk.h>
#include <string.h>

#ifndef _WIN32
 #include <strings.h>	//strcasecmp
#else
 #define strcasecmp stricmp
#endif

void replaceall(std::string &strData, const char *szFind, const char *szReplace);

// parser states
#define PARSER_STATE_BLANK      0
#define PARSER_STATE_INSIDE_TAG 1
#define PARSER_STATE_INSIDE_COMMENT 2

// Html escape sequences table
typedef struct {
	gunichar  cLetter;
	const char *szEscape;
} HtmlEscape;

static std::vector<HtmlEscape> g_lstTableSort2;

//table is sorted by first field to enable binary search
static const HtmlEscape _table_char[] =
{
	{'\"',	"&quot;"},  //=34
	{'&',	"&amp;"},   //=38
	{'<',	"&lt;"},    //=60
	{'>',	"&gt;"},    //=62
	{' ',	"&nbsp;"},  //non-breaking space
	{161,	"&iexcl;"}, //'¡' - inverted exclamation mark
	{162,	"&cent;"},  //'¢'
	{163,	"&pound;"}, //'£'
 	{164,	"&curren;"},//'¤'
	{165,	"&yen;"},   //'¥'
	{166,	"&brvbar;"},//'¦' - broken (vertical) bar
	{167,	"&sect;"},  //'§' - section sign
	{168,	"&uml;"},   //'¨' - umlaut
	{169,	"&copy;"},  //'©' - copyright sign
	{170,	"&ordf;"},  //'ª' - feminine ordinal
	{171,	"&laquo;"}, //'«' - left guillemet
	{174,	"&reg;"},   //'®' - registered sign

	{176,	"&deg;"},   //'°' - degree sign
	{177,	"&plusmn;"},//'±' - plus or minus
	{178,	"&sup2;"},  //'²' - superscript two
	{179,	"&sup3;"},  //'³' - superscript three

	{187,	"&raquo;"}, //'»' - right guillemet

	{192,	"&Agrave;"},//'À'
	{193,	"&Aacute;"},//'Á'
	{194,	"&Acirc;"}, //'Â'
	{195,	"&Atilde;"},//'Ã'
	{196,	"&Auml;"},  //'Ä'
	{197,	"&Aring;"}, //'Å'
	{198,	"&AElig;"}, //'Æ'
	{199,	"&Ccedil;"},//'Ç'
	{200,	"&Egrave;"},//'È'
	{201,	"&Eacute;"},//'É'
	{202,	"&Ecirc;"}, //'Ê'
	{203,	"&Euml;"},  //'Ë'
	{204,	"&Igrave;"},//'Ì'
	{205,	"&Iacute;"},//'Í'
	{206,	"&Icirc;"}, //'Î'
	{207,	"&Iuml;"},  //'Ï'
	{208,	"&ETH;"},   //'Ð' - capital Eth, Icelandic
	{209,	"&Ntilde;"},//'Ñ'
	{210,	"&Ograve;"},//'Ò'
	{211,	"&Oacute;"},//'Ó'
	{212,	"&Ocirc;"}, //'Ô'
	{213,	"&Otilde;"},//'Õ'
	{214,	"&Ouml;"},  //'Ö'
	{215,	"&times;"}, //'×' - multiply sign
	{216,	"&Oslash;"},//'Ø'
	{217,	"&Ugrave;"},//'Ù'
	{218,	"&Uacute;"},//'Ú'
	{219,	"&Ucirc;"}, //'Û'
	{220,	"&Uuml;"},  //'Ü'
	{221,	"&Yacute;"},//'Ý'
	{222,	"&THORN;"}, //'Þ' - capital THORN, Icelandic
	{223,	"&szlig;"}, //'ß'
	{224,	"&agrave;"},//'à'
	{225,	"&aacute;"},//'á'
	{226,	"&acirc;"}, //'â'
	{227,	"&atilde;"},//'ã'
	{228,	"&auml;"},  //'ä'
	{229,	"&aring;"}, //'å'

	{230,	"&aelig;"}, //'æ'
	{231,	"&ccedil;"},//'ç'
	{232,	"&egrave;"},//'è'
	{233,	"&eacute;"},//'é'
	{234,	"&ecirc;"}, //'ê'
	{235,	"&euml;"},  //'ë'
	{236,	"&igrave;"},//'ì'
	{237,	"&iacute;"},//'í'
	{238,	"&icirc;"}, //'î'
	{239,	"&iuml;"},  //'ï'
	{240,	"&eth;"},   //'ð' - small eth, Icelandic
	{241,	"&ntilde;"},//'ñ'
	{242,	"&ograve;"},//'ò'
	{243,	"&oacute;"},//'ó'
	{244,	"&ocirc;"},	//'ô'
	{245,	"&otilde;"},//'õ'
	{246,	"&ouml;"},  //'ö'

	{248,	"&oslash;"},//'ø'
	{249,	"&ugrave;"},//'ù'
	{250,	"&uacute;"},//'ú'
	{251,	"&ucirc;"}, //'û'
	{252,	"&uuml;"},  //'ü'
	{253,	"&yacute;"},//'ý'
	{254,	"&thorn;"}, //'þ' - small thorn, Icelandic
	{255,	"&yuml;"},  //'ÿ'

	{338,	"&OElig;"}, //'Œ'
	{339,	"&oelig;"}, //'œ'
 	{352,	"&Scaron;"},//'Š'
 	{353,	"&scaron;"},//'š'
	{376,	"&Yuml;"},  //'Ÿ'
 	{402,	"&fnof;"},  //'ƒ'

 	{8211,	"&#8211;"}, //'–' - en dash (demi-cadratin)
 	{8212,	"&#8212;"}, //'—' - em dash (cadratin)

 	{8249,	"&#8249;"}, //'‹' - left single guillemet
 	{8250,	"&#8250;"}, //'›' - right single guillemet
 	{8364,	"&euro;"},  //'€'
 	{8482,	"&trade;"}, //'™' - trademark

	//TOFIX add more chars if needed
};

static int hexVal(char ch)
{
    if ((ch >= 'a') && (ch <= 'f'))
        return (ch - 'a') + 10;
    else if ((ch >= 'A') && (ch <= 'F'))
        return (ch - 'A') + 10;
    else if ((ch >= '0') && (ch <= '9'))
        return (ch - '0');
    return -1;
} // hexVal

#define SIZE_OF(x) (sizeof(x)/sizeof(x[0]))

static int table_bin_search_char(gunichar chFind, int nLeft = 0, int nRight = SIZE_OF(_table_char)-1);
static int table_bin_search_escape(const char *szFind, int nLeft = 0, int nRight = SIZE_OF(_table_char)-1);

class TblComparator{
public:
	bool operator()(const HtmlEscape &a, const HtmlEscape &b)
	{
		//operator < (is a<b ?)
		return (strcmp(a.szEscape, b.szEscape) < 0);
	};
};

HTMLParser::HTMLParser()
{
	Clear();

	m_bAllowUnescapedInPreTag = false;
	m_bInsidePreTag = false;

	//create new sort table (create only once - global object)
	if(g_lstTableSort2.empty())
	{
		for(unsigned int i=0; i<SIZE_OF(_table_char); i++)
			g_lstTableSort2.push_back(_table_char[i]);
		TblComparator cmp;
		std::sort(g_lstTableSort2.begin(), g_lstTableSort2.end(), cmp);
	}
}

HTMLParser::~HTMLParser()
{
}

void HTMLParser::Clear()
{
	m_nState = PARSER_STATE_BLANK;
	m_strData.erase(m_strData.begin(), m_strData.end());
}

bool HTMLParser::Parse(const char *szBuffer, int len)
{
	if(len < 0)
		return false;

	for(int i=0; i<len; i++)
	{
		if(PARSER_STATE_INSIDE_COMMENT == m_nState)
		{
			if( m_strData.size()>2 &&
				0 == strncmp("--", m_strData.substr(m_strData.size()-2).c_str(), 2))	//tag ends as comment
			{
				TRACE("HTML Parser: Comment ended\n");
				m_strData.erase(m_strData.size()-2, 2); //remove "--" ending
				OnComment(m_strData.c_str()+3);	//trigger event
				m_strData.erase(m_strData.begin(), m_strData.end());
				m_nState = PARSER_STATE_BLANK;
			}
			else
				m_strData += szBuffer[i];
		}
		else if(PARSER_STATE_INSIDE_TAG == m_nState)
		{
			//check for the end of tag
			if(szBuffer[i] == '>'){
				if(0 == strncmp("!--", m_strData.c_str(), 3))	//is tag comment
				{
					m_nState = PARSER_STATE_INSIDE_COMMENT;

					if(0 == strncmp("--", m_strData.substr(m_strData.size()-2).c_str(), 2))	//tag ends as comment
					{
						TRACE("HTML Parser: Comment ended\n");
						m_strData.erase(m_strData.size()-2, 2); //remove "--" ending
						OnComment(m_strData.c_str()+3);	//trigger event
						m_strData.erase(m_strData.begin(), m_strData.end());
						m_nState = PARSER_STATE_BLANK;
					}
					else
						m_strData += szBuffer[i];
				}
				else if(!m_strData.empty() && m_nState != PARSER_STATE_INSIDE_COMMENT)
				{
					if(m_strData.at(0) == '/')	//is ending tag
					{
						//strip everything after first space within tag to get real tag name
						std::string strTag(m_strData.c_str()+1);
						int nPos = strTag.find_first_of(' ');
						if(nPos >= 0)
							strTag.erase(strTag.begin()+nPos);

						if(!m_bAllowUnescapedInPreTag || (m_bAllowUnescapedInPreTag && !m_bInsidePreTag) || (0 == strcasecmp(strTag.c_str(), "PRE")))
						{
							TRACE("HTML Parser: Tag end found (%s)\n", strTag.c_str());
							OnTagEnd(strTag.c_str());	//trigger event
							m_strData.erase(m_strData.begin(), m_strData.end());
							if(0 == strcasecmp(strTag.c_str(), "PRE"))
								m_bInsidePreTag = false;
						}
						else
						{
							TRACE("HTML Parser: Push text (%s)\n", m_strData.c_str());
							m_nState = PARSER_STATE_BLANK;
							OnText(m_strData.c_str());	//trigger event for previous contents
							m_strData.erase(m_strData.begin(), m_strData.end());
						}
					}
					else if(m_nState != PARSER_STATE_INSIDE_COMMENT)
					{
						std::string strTag(m_strData.c_str());
						std::string strParams;

						int nPos = strTag.find_first_of(' ');
						if(nPos >= 0){
							strTag = strTag.substr(0, nPos);
							strParams = m_strData.substr(nPos);
						}

						if(!m_bAllowUnescapedInPreTag || (m_bAllowUnescapedInPreTag && !m_bInsidePreTag)){
							TRACE("HTML Parser: Tag start found (%s)[%s]\n", strTag.c_str(), strParams.c_str());
							OnTagBegin(strTag.c_str(), strParams.c_str());	//trigger event
							m_strData.erase(m_strData.begin(), m_strData.end());
							m_nState = PARSER_STATE_BLANK;
							if(0 == strcasecmp(strTag.c_str(), "PRE"))
								m_bInsidePreTag = true;
						}
						else{
							m_nState = PARSER_STATE_BLANK;
							TRACE("HTML Parser: Push text1 (%s)\n", m_strData.c_str());
							OnText(m_strData.c_str());	//trigger event for previous contents
							m_strData.erase(m_strData.begin(), m_strData.end());
						}
					}
				}

				if(PARSER_STATE_INSIDE_COMMENT != m_nState)
					Clear();
			}
			else
				m_strData += szBuffer[i];
		}
		else
		{
			//check for the start of tag
			if(szBuffer[i] == '<' && m_nState == PARSER_STATE_BLANK)
			{
				if(!m_strData.empty())
				{
					TRACE("HTML Parser: Push text2 (%s)\n", m_strData.c_str());
					OnText(m_strData.c_str());	//trigger event for previous contents
					m_strData.erase(m_strData.begin(), m_strData.end());
				}
				m_nState = PARSER_STATE_INSIDE_TAG;
			}
			else{
				m_strData += szBuffer[i];
			}
		}
	}

	return true;
}

void HTMLParser::Finalize()
{
	if(!m_strData.empty() && m_nState == PARSER_STATE_BLANK)
		OnText(m_strData.c_str());	//trigger event for previous contents
	m_strData.erase(m_strData.begin(), m_strData.end());
}

void HTMLParser::EscapeURI(std::string &data)
{
	//TOFIX replace non-ASCII characters by converting each byte to %HH, where HH is the hexadecimal notation of the byte value
	replaceall(data, " ", "%20");
	replaceall(data, "&", "&amp;");
}

void HTMLParser::UnescapeURI(std::string &data)
{
#if GTK_CHECK_VERSION(2,16,0)
	char *szRes = g_uri_unescape_string(data.c_str(), NULL);
	if(szRes){
		data = szRes;
		g_free(szRes);
	}
#else
	int nStart = 0;
	unsigned int nSize = data.size();
	std::string::size_type nPos;
	while ((nPos = data.find('%', nStart)) != std::string::npos)
	{
		if(nPos + 2 < nSize) // two chars after %
		{
			int a, b;
			if ((a = hexVal(data[nPos+1])) != -1)
			{
				if ((b = hexVal(data[nPos+2])) != -1)
				{
					gunichar cChar = ((a * 16) + b);
					//gchar szText[10];
					//int nWritten = g_unichar_to_utf8(cChar, szText);
					//szText[nWritten] = '\0';
					data.erase(nPos, 3);
					//data.insert(nPos, szText);
					data.insert(data.begin()+nPos, (char)cChar);

					//nStart = nPos + nWritten;
					nStart = nPos + 1;
					nSize -= 3;
					//nSize += nWritten;
					nSize += 1;
				}
				else
					nStart = nPos + 3;
			}
			else
				nStart = nPos + 3;
		}
		else
			break;
	}

	//TOFIX
	replaceall(data, "&amp;", "&");
#endif
}

void HTMLParser::EscapeChars(std::string &data)
{
	unsigned int nPos, nWidth;
	const char *szStart  = data.c_str();
	const char *szString = szStart;

	//using UTF-8 characters
	while(NULL != szString && '\0' != *szString)
	{
		int nSkip = 0;
		gunichar chLetter = g_utf8_get_char (szString);
		const char *szNext = g_utf8_find_next_char(szString, NULL);

		//TRACE("String to escape: %s\n", szString);

		int nRes = table_bin_search_char(chLetter);
		if(nRes >= 0)
		{
			//replace escape sequence with original special char
			nPos   = szString - szStart;
			nWidth = szNext - szString;
			nSkip  = strlen(_table_char[nRes].szEscape);

			//TRACE("Escape: %d (width=%d) to %s\n", chLetter, nWidth, _table_char[nRes].szEscape);

			//FIX: data = data.substr(0, nPos) + _table_char[nRes].szEscape + data.substr(nPos+nWidth, 1000000);
			data.erase(nPos, nWidth);
			data.insert(nPos, _table_char[nRes].szEscape);

			//TRACE("Escaped line: %s\n", data.c_str());

			szStart  = data.c_str();  //in case string was reallocated
			szString = szStart + nPos + nSkip;
		}
		else
			szString = szNext;
	}
}

void HTMLParser::UnescapeChars(std::string &data)
{
	unsigned int nPos = 0;
	while(1)
	{
		int nPosStart = data.find('&', nPos);
		if(nPosStart < 0)
			break;

		int nPosEnd = data.find(';', nPosStart+1);
		if(nPosEnd >= 0)
		{
			//extract escape sequence
			std::string strChar = data.substr(nPosStart, nPosEnd-nPosStart+1);
			//TRACE("Escape sequence %s found!\n", strChar.c_str());

			int nRes = table_bin_search_escape(strChar.c_str());
			if(nRes >= 0)
			{
				//replace escape sequence with original UTF-8 character
				char szBuffer[20];
				int nBytes = g_unichar_to_utf8(g_lstTableSort2[nRes].cLetter, szBuffer);
				szBuffer[nBytes] = '\0';

				//FIX: data = data.substr(0, nPosStart) + szBuffer + data.substr(nPosEnd+1, 1000000);
				data.erase(nPosStart, nPosEnd+1-nPosStart);
				data.insert(nPosStart, szBuffer);
			}
			else
				TRACE("ERROR: HTML escape sequence %s is not supported yet!\n", strChar.c_str());
		}
		else
			break;	//no sequence found

		nPos = nPosStart+1;
	}
}

//use binary search to speed up convertion
int table_bin_search_char(gunichar chFind, int nLeft, int nRight)
{
	if(nLeft > nRight) return -1;	//no match found

	//check middle of the range
	int nMid = (nLeft + nRight)/2;
	if(chFind == _table_char[nMid].cLetter)
		return nMid;	//match found

	if(nLeft == nRight) return -1;	//no match found

	if(chFind < _table_char[nMid].cLetter)
		return table_bin_search_char(chFind, nLeft, nMid-1);	//search lower half
	else
		return table_bin_search_char(chFind, nMid+1, nRight);	//search upper half
}

int table_bin_search_escape(const char *szFind, int nLeft, int nRight)
{
	//TRACE("bin search [Escape:%s], l=%d, r=%d\n", szFind, nLeft, nRight);

	if(nLeft > nRight) {
		//TRACE("bin search: no match found\n");
		return -1;	//no match found
	}

	//check middle of the range
	int nMid = (nLeft + nRight)/2;
	if(0 == strcmp(szFind, g_lstTableSort2[nMid].szEscape)){
		//TRACE("bin search found [Escape:%s], m=%d\n", szFind, nMid);
		return nMid;	//match found
	}

	if(nLeft == nRight){
		//TRACE("bin search: no match found\n");
		return -1;	//no match found
	}

	if(strcmp(szFind, g_lstTableSort2[nMid].szEscape) < 0)
	{
		//TRACE("Search lower half, mid[%d]=%s\n", nMid, g_lstTableSort2[nMid].szEscape);
		return table_bin_search_escape(szFind, nLeft, nMid-1);	//search lower half
	}
	else{
		//TRACE("Search upper half, mid[%d]=%s\n", nMid, g_lstTableSort2[nMid].szEscape);
		return table_bin_search_escape(szFind, nMid+1, nRight);	//search upper half
	}
}

bool HTMLParser::ExtractParam(const std::string &data, const char *szParam, std::string &resValue)
{
	std::string strPattern = szParam;
	strPattern += "=\"";

	std::string::size_type nPos = data.find(strPattern.c_str());
	if(nPos != std::string::npos)
	{
		std::string::size_type nEnd = data.find("\"", nPos+strPattern.size());
		if(nEnd != std::string::npos){
			resValue = data.substr(nPos+strPattern.size(), nEnd-nPos-strPattern.size());
			return true;
		}
	}
	return false; // not found
}
