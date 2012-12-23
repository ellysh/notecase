////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Class for handling internationalisation issues (message catalogs)
////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
  #pragma warning(disable: 4996)	//ignore deprecated functions warning
#endif

#include "../config.h"
#include "GuiLanguage.h"
#include <algorithm>
#include <gtk/gtk.h>
#ifdef ENABLE_NLS
#include <libintl.h>
#endif
#include <string.h>

#include "../lib/EnumDirectory.h"

#ifdef _WIN32
 #include <io.h> //access
 #include <windows.h> 
 #define access _access
#else
 #include <libgen.h>	//basename on Archlinux
#endif 

#if defined (__FreeBSD__) || defined (__APPLE__)
 #include <libgen.h>		//basename
#endif

extern GuiLanguage g_lang;

bool OnDirEnum(const char *szDir, void *data);
#define ARRAY_COUNT(x) (sizeof(x)/sizeof(x[0]))

#ifndef _WIN32
	//define dummy versions of Windows language IDs
	#define LANG_AFRIKAANS	0
	#define LANG_ALBANIAN	0
	#define LANG_AMHARIC    0
 	#define LANG_ARABIC		0
	#define LANG_ARMENIAN	0
	#define LANG_BASQUE     0
	#define LANG_BELARUSIAN	0
	#define LANG_BENGALI	0
	#define LANG_BULGARIAN	0
	#define LANG_CATALAN	0
	#define LANG_CROATIAN	0
	#define LANG_CZECH		0
	#define LANG_DANISH		0
	#define LANG_DUTCH		0
	#define LANG_ENGLISH	0
	#define LANG_ESTONIAN	0
	#define LANG_ESPERANTO	0
	#define LANG_FARSI		0
	#define LANG_FRENCH		0
	#define LANG_FINNISH	0
	#define LANG_GEORGIAN	0
	#define LANG_GERMAN		0
	#define LANG_GREEK		0
	#define LANG_HEBREW		0
	#define LANG_HINDI		0
	#define LANG_HUNGARIAN	0
	#define LANG_ICELANDIC	0
	#define LANG_INDONESIAN	0
	#define LANG_ITALIAN	0
	#define LANG_JAPANESE	0
	#define LANG_KOREAN		0
	#define LANG_LATVIAN	0
	#define LANG_LITHUANIAN	0
	#define LANG_MACEDONIAN	0
	#define LANG_MONGOLIAN  0
	#define LANG_MALAY		0
	#define LANG_NORWEGIAN	0
	#define LANG_POLISH		0
	#define	LANG_PORTUGUESE	0
	#define LANG_ROMANIAN	0
	#define LANG_RUSSIAN	0
	#define LANG_SERBIAN	0
	#define LANG_SLOVAK		0
	#define LANG_SLOVENIAN	0
	#define LANG_SPANISH	0
	#define LANG_SWEDISH	0
	#define LANG_TAMIL		0
	#define LANG_THAI		0
	#define LANG_TURKISH	0
	#define LANG_UKRAINIAN	0
	#define LANG_URDU		0
	#define LANG_VIETNAMESE	0
	#define LANG_CHINESE	0

	//define dummy versions of Windows sublanguage IDs
	#define SUBLANG_DEFAULT				0
	#define SUBLANG_BENGALI_INDIA       0
	#define SUBLANG_BENGALI_BANGLADESH  0
	#define SUBLANG_CHINESE_TRADITIONAL 0
	#define SUBLANG_CHINESE_SIMPLIFIED  0
	#define SUBLANG_DUTCH				0
	#define SUBLANG_DUTCH_BELGIAN		0
	#define SUBLANG_ENGLISH_UK			0
	#define SUBLANG_ENGLISH_US			0
	#define SUBLANG_FRENCH				0
	#define SUBLANG_FRENCH_BELGIAN		0
	#define SUBLANG_FRENCH_CANADIAN		0
	#define SUBLANG_GERMAN				0
	#define SUBLANG_GERMAN_AUSTRIAN		0
	#define SUBLANG_GERMAN_SWISS		0
	#define SUBLANG_LITHUANIAN			0
	#define SUBLANG_KOREAN				0
	#define SUBLANG_MALAY_MALAYSIA		0
	#define SUBLANG_MALAY_BRUNEI_DARUSSALAM	0
	#define SUBLANG_NORWEGIAN_BOKMAL	0
	#define SUBLANG_NORWEGIAN_NYNORSK	0
	#define SUBLANG_PORTUGUESE			0
	#define SUBLANG_PORTUGUESE_BRAZILIAN	0
	#define SUBLANG_SPANISH				0
	#define SUBLANG_SWEDISH				0
	#define SUBLANG_SERBIAN_LATIN			0
#else
	//some codes are not defined in Visual C++ 6.0
	#ifndef LANG_AMHARIC
		#define LANG_AMHARIC 0x5e
	#endif
	#ifndef LANG_ESPERANTO
		#define LANG_ESPERANTO 0x8f
	#endif
	#ifndef LANG_MONGOLIAN
		#define LANG_MONGOLIAN 0x50
	#endif
	#ifndef SUBLANG_BENGALI_INDIA
		#define SUBLANG_BENGALI_INDIA 0x00
	#endif
	#ifndef SUBLANG_BENGALI_BANGLADESH
		#define SUBLANG_BENGALI_BANGLADESH 0x01
	#endif
#endif


typedef struct {
    const char *iso, *locale, *lang;
	int nLangID_W32;
	int nSubLangID_W32;
	bool bRTL;
} LanguageStruct;

//TOFIX add more languages, add native language name column?
static LanguageStruct isoLanguages[] = 
{
    {"af",	"af_AF",	"Afrikaans",	LANG_AFRIKAANS, SUBLANG_DEFAULT, false},
    {"sq",	"sq_AL",	"Albanian",		LANG_ALBANIAN,  SUBLANG_DEFAULT, false},
	{"am",  "am_ET",    "Amharic",      LANG_AMHARIC,   SUBLANG_DEFAULT, false},
    {"ar",	"ar",		"Arabic",	 	LANG_ARABIC,    SUBLANG_DEFAULT, true}, 
    {"hy",	"hy",		"Armenian",	 	LANG_ARMENIAN,  SUBLANG_DEFAULT, false}, 
	{"eu",	"eu",		"Basque",		LANG_BASQUE,	SUBLANG_DEFAULT, false}, 
    {"be",	"be_BE",	"Belarusian",	LANG_BELARUSIAN,SUBLANG_DEFAULT, false}, 
	{"bn",	"bn",		"Bengali",		LANG_BENGALI,   SUBLANG_DEFAULT, false}, 
	{"bn_IN",	"bn_IN",	"Bengali (India)",	LANG_BENGALI,   SUBLANG_BENGALI_INDIA, false}, 
	{"bn_BD",	"bn_BD",	"Bengali (Bangladesh)",	LANG_BENGALI,   SUBLANG_BENGALI_BANGLADESH, false}, 
    {"bg",	"bg_BG",	"Bulgarian",	LANG_BULGARIAN,	SUBLANG_DEFAULT, false},
    {"ca",	"ca_ES",	"Catalan",		LANG_CATALAN,	SUBLANG_DEFAULT, false},
    {"zh_CN",	"zh_CN",	"Chinese (simplified)",		LANG_CHINESE,	SUBLANG_CHINESE_SIMPLIFIED, false},
	{"zh_TW",	"zh_TW",	"Chinese (traditional)",	LANG_CHINESE,	SUBLANG_CHINESE_TRADITIONAL, false},
    {"hr",	"hr_HR",	"Croatian",		LANG_CROATIAN,	SUBLANG_DEFAULT, false},
    {"cs",	"cs_CZ",	"Czech",		LANG_CZECH,		SUBLANG_DEFAULT, false},
    {"da",	"da_DK",	"Danish",		LANG_DANISH,	SUBLANG_DEFAULT, false},
    {"nl",	"nl_NL",	"Dutch",		LANG_DUTCH,		SUBLANG_DUTCH, false},
    {"nl_BE","nl_BE",	"Dutch (Belgian)",	LANG_DUTCH,		SUBLANG_DUTCH_BELGIAN, false},
    {"en",	"en_GB",	"English",		LANG_ENGLISH,	SUBLANG_ENGLISH_UK, false},
    {"en_US","en_US",	"English (American)",LANG_ENGLISH,	SUBLANG_ENGLISH_US, false},
    {"et",	"et_EE",	"Estonian",		LANG_ESTONIAN,	SUBLANG_DEFAULT, false},
	{"eo",	"eo",		"Esperanto",	LANG_ESPERANTO,	SUBLANG_DEFAULT, false},
    {"fa",	"fa_IR",	"Farsi",		LANG_FARSI,		SUBLANG_DEFAULT, false},
    {"fi",	"fi_FI",	"Finnish",		LANG_FINNISH ,	SUBLANG_DEFAULT, false},
    {"fr",	"fr_FR",	"French",		LANG_FRENCH,	SUBLANG_FRENCH, false},
    {"fr_BE","fr_BE",	"French (Belgian)", LANG_FRENCH,	SUBLANG_FRENCH_BELGIAN, false},
    {"fr_CA","fr_CA",	"French (Canadian)", LANG_FRENCH,	SUBLANG_FRENCH_CANADIAN, false},
    {"ka",	"ka",		"Georgian",		LANG_GEORGIAN,	SUBLANG_DEFAULT, false}, 
    {"de",	"de_DE",	"German",		LANG_GERMAN,	SUBLANG_GERMAN, false},
    {"de_AT",	"de_AT",	"German (Austrian)",	LANG_GERMAN, SUBLANG_GERMAN_AUSTRIAN, false},
    {"de_CH",	"de_CH",	"German (Swiss)",	LANG_GERMAN, SUBLANG_GERMAN_SWISS, false},
    {"el",	"el_GR",	"Greek",		LANG_GREEK,		SUBLANG_DEFAULT, false}, 
    {"he",	"he_IL",	"Hebrew",		LANG_HEBREW,	SUBLANG_DEFAULT, true}, 
    {"hi",	"hi_IN",	"Hindi",		LANG_HINDI,		SUBLANG_DEFAULT, false}, 
    {"hu",	"hu_HU",	"Hungarian",	LANG_HUNGARIAN, SUBLANG_DEFAULT, false}, 
    {"is",	"is_IS",	"Icelandic",	LANG_ICELANDIC, SUBLANG_DEFAULT, false}, 
    {"id",	"id_ID",	"Indonesian",	LANG_INDONESIAN,SUBLANG_DEFAULT, false}, 
    {"it",	"it_IT",	"Italian",		LANG_ITALIAN,	SUBLANG_DEFAULT, false},
    {"ja",	"ja_JP",	"Japanese",		LANG_JAPANESE,	SUBLANG_DEFAULT, false}, 
    {"ko",	"ko_KR",	"Korean",		LANG_KOREAN,	SUBLANG_KOREAN, false}, 
    {"lv",	"lv_LV",	"Latvian",		LANG_LATVIAN,	SUBLANG_DEFAULT, false}, 
    {"lt",	"lt_LT",	"Lithuanian",	LANG_LITHUANIAN,SUBLANG_LITHUANIAN, false}, 
    {"mk",	"mk_MK",	"Macedonian (FYROM)",LANG_MACEDONIAN,SUBLANG_DEFAULT, false}, 
	{"mn",	"mn",   	"Mongolian",LANG_MONGOLIAN,SUBLANG_DEFAULT, false}, 
    {"ms",	"ms_MY",	"Malay (Malaysia)",		LANG_MALAY,		SUBLANG_MALAY_MALAYSIA, false}, 
    {"ms_BN","ms_BN",	"Malay (Brunei)",LANG_MALAY,		SUBLANG_MALAY_BRUNEI_DARUSSALAM, false}, 
    {"nb",	"nb_NO",	"Norwegian Bokmal",	LANG_NORWEGIAN, SUBLANG_NORWEGIAN_BOKMAL, false},
    {"nn",	"nn_NO",	"Norwegian Nynorsk",LANG_NORWEGIAN, SUBLANG_NORWEGIAN_NYNORSK, false},
    {"no",	"no_NO",	"Norwegian",	LANG_NORWEGIAN, SUBLANG_DEFAULT, false},
    {"pl",	"pl_PL",	"Polish",		LANG_POLISH,	SUBLANG_DEFAULT, false},
    {"pt",	"pt_PT",	"Portuguese",	LANG_PORTUGUESE,SUBLANG_PORTUGUESE, false}, 
    {"pt_BR", "pt_BR",	"Portuguese (Brazilian)",	LANG_PORTUGUESE,SUBLANG_PORTUGUESE_BRAZILIAN, false}, 
    {"ro",	"ro_RO",	"Romanian",		LANG_ROMANIAN,	SUBLANG_DEFAULT, false}, 
    {"ru",	"ru_RU",	"Russian",		LANG_RUSSIAN,	SUBLANG_DEFAULT, false}, 
    //TOFIX there seems to be no cross-platform compatible locale for Serbian language
    //"sr_CS@Latn" and "sr_CS" seem to be official codes, but do not work
#ifdef _WIN32
    {"sr",	"sr_CS",	"Serbian (Latin)",	LANG_SERBIAN,	SUBLANG_SERBIAN_LATIN, false},
#else
	//TOFIX Can not find proper locale for Fedora Core 3
    {"sr",	"sr_YU.UTF-8",	"Serbian (Latin)",		LANG_SERBIAN,	SUBLANG_SERBIAN_LATIN, false},
#endif
	//TOFIX {"sr_CS","sr_CS","Serbian (Cyrillic)",		LANG_SERBIAN,	SUBLANG_SERBIAN_CYRILLIC, false},
    {"sk",	"sk_SK",	"Slovak",		LANG_SLOVAK,	SUBLANG_DEFAULT, false}, 
    {"sl",	"sl_SI",	"Slovenian",	LANG_SLOVENIAN, SUBLANG_DEFAULT, false}, 
    {"es",	"es_ES",	"Spanish",		LANG_SPANISH,	SUBLANG_SPANISH, false}, 
    {"sv",	"sv_SE",	"Swedish",		LANG_SWEDISH,	SUBLANG_SWEDISH, false}, 
    {"th",	"th_TH",	"Thai",			LANG_THAI,		SUBLANG_DEFAULT, false}, 
    {"bo",	"bo",		"Tibetan",		0,	0, false}, 
    {"ta",	"ta_IN",	"Tamil",		LANG_TAMIL,		SUBLANG_DEFAULT, false}, 
    {"tr",	"tr_TR",	"Turkish",		LANG_TURKISH,	SUBLANG_DEFAULT, false}, 
    {"uk",	"uk_UA",	"Ukrainian",	LANG_UKRAINIAN,	SUBLANG_DEFAULT, false}, 
    {"ur",	"ur",		"Urdu",			LANG_URDU,		SUBLANG_DEFAULT, false}, 
    {"vi",	"vi_VN",	"Vietnamese",	LANG_VIETNAMESE,SUBLANG_DEFAULT, false}, 
};

// comparator class to sort locale list by language name (ascending)
class CmpLang
{
public:
	CmpLang(){};
	~CmpLang(){};

	bool operator ()(const std::string &a, const std::string &b){
		#ifdef _WIN32
			return (g_lang.GetLangName(a.c_str()) > g_lang.GetLangName(b.c_str()));
		#else
			return (g_lang.GetLangName(a.c_str()) < g_lang.GetLangName(b.c_str()));
		#endif
	}
};

GuiLanguage::GuiLanguage(const char *szPackage, const char *szLocaleDir, const char *szSourceISO)
{
	m_strPackage	= szPackage;
	m_strLocaleDir	= szLocaleDir;
	m_strDefaultISO = (NULL != szSourceISO)? szSourceISO : "en";
	m_bCatalogsListed = false;
	m_nLangIdx = -1;
}

GuiLanguage::~GuiLanguage()
{
}

//maps from locale code to the language name
const char *GuiLanguage::GetLangName(const char *locale)
{
	//match either short or long version of the locale
    for (unsigned int i=0; i<ARRAY_COUNT(isoLanguages); i++)
        if (0 == strcmp (isoLanguages[i].iso, locale) || 
			0 == strcmp (isoLanguages[i].locale, locale))
            return isoLanguages[i].lang;

    return "";    // not found
}

const char *GuiLanguage::GetLocaleCode(const char *iso)
{
	//match either short or long version of the locale
    for (unsigned int i=0; i<ARRAY_COUNT(isoLanguages); i++)
        if (0 == strcmp (isoLanguages[i].iso, iso))
            return isoLanguages[i].locale;

    return "";    // not found
}

int GuiLanguage::GetLocaleTableIdx(const char *locale)
{
	//match either short or long version of the locale
    for (unsigned int i=0; i<ARRAY_COUNT(isoLanguages); i++)
        if (0 == strcmp (isoLanguages[i].iso, locale) || 
			0 == strcmp (isoLanguages[i].locale, locale))
            return i;

    return -1;    // not found
}

void GuiLanguage::Initialize(const char *szIsoCode)
{
#ifdef ENABLE_NLS
	//ensure some language is selected
	if(NULL == szIsoCode || strlen(szIsoCode)==0){
		szIsoCode = m_strDefaultISO.c_str();	//default ISO (one used in the source code)
		//TOFIX query LANG environment variable to set default locale ?
	}

	// convert language ISO code to locale code
	const char *szLocaleCode = GetLocaleCode(szIsoCode);
	if(strlen(szLocaleCode) > 0)
	{
		std::string strCountry(szLocaleCode);
		std::string::size_type nPos = strCountry.find("_");
		if(nPos != std::string::npos)
			strCountry = strCountry.substr(0, nPos);

		//ensure language catalog is available
		if( !IsLocaleAvailable(szIsoCode) &&
			!IsLocaleAvailable(szLocaleCode) &&
			!IsLocaleAvailable(strCountry.c_str()) )
		{
			szIsoCode	 = m_strDefaultISO.c_str();	//default ISO (one used in the source code)
			szLocaleCode = GetLocaleCode(szIsoCode);//default locale
		}

		//set language localisation for this program
		putenv( g_strconcat("LANG=", szLocaleCode, (char *)NULL) );
		setlocale(LC_ALL, "");

		m_nLangIdx = GetLocaleTableIdx(szIsoCode);
	#ifdef _WIN32
		//FIX for Windows code
		if(m_nLangIdx >= 0)
			SetThreadLocale(MAKELCID(MAKELANGID(isoLanguages[m_nLangIdx].nLangID_W32, isoLanguages[m_nLangIdx].nSubLangID_W32), SORT_DEFAULT));
	#endif
	}

	// init NLS system settings
	bindtextdomain (m_strPackage.c_str(), m_strLocaleDir.c_str());
	bind_textdomain_codeset (m_strPackage.c_str(), "UTF-8");
	textdomain (m_strPackage.c_str());

	//store locale code for later lookup
	m_strCurLang = szIsoCode;
#endif
}

void GuiLanguage::ListAvailableCatalogs()
{
	if(m_bCatalogsListed)
		return;
	m_bCatalogsListed = true;
	
	//clear catalog list
	m_lstCatalogs.erase(m_lstCatalogs.begin(), m_lstCatalogs.end());	//empty the list

	//list all subdirectories in the locale directory (subdirectory name matches the locale name)
	std::vector<std::string> lstDirs;
	EnumDirectory(m_strLocaleDir.c_str(), OnDirEnum, (void *)&lstDirs, ENUM_LST_DIRS);
	
	//store each locale containing catalog for current package
	for( unsigned int i=0; i<lstDirs.size(); i++)
	{
		//check if message catalog exists for given locale subdirectory
		std::string strCatalog;
		#ifdef _WIN32
			strCatalog  = m_strLocaleDir;
			strCatalog += lstDirs[i];
			strCatalog += "\\LC_MESSAGES\\";
			strCatalog += m_strPackage;
			strCatalog += ".mo";
		#else
			char szBuffer[1024];
			snprintf(szBuffer, sizeof(szBuffer), "%s/%s/LC_MESSAGES/%s.mo",m_strLocaleDir.c_str(), lstDirs[i].c_str(), m_strPackage.c_str());
			strCatalog  = szBuffer;
		#endif
				
		if(0 == access(strCatalog.c_str(), 00))
			m_lstCatalogs.push_back(lstDirs[i]);	//add locale into the list of available catalogs
	}

	//sort list by language names
	std::sort(m_lstCatalogs.begin(), m_lstCatalogs.end(), CmpLang());
}

bool GuiLanguage::IsLocaleAvailable(const char *szLocale)
{
	std::string strCatalog;
#ifdef _WIN32
	strCatalog  = m_strLocaleDir;
	strCatalog += szLocale;
	strCatalog += "\\LC_MESSAGES\\";
	strCatalog += m_strPackage;
	strCatalog += ".mo";
#else
	char szBuffer[1024];
	snprintf(szBuffer, sizeof(szBuffer), "%s/%s/LC_MESSAGES/%s.mo",m_strLocaleDir.c_str(), szLocale, m_strPackage.c_str());
	strCatalog  = szBuffer;
#endif				
	
	return (0 == access(strCatalog.c_str(), 00));
	//return (-1 != GetLocaleIdx(szLocale)); //old implementation (slows the program startup)
}

int	GuiLanguage::GetLocaleIdx(const char *szLocale)
{
	for(unsigned int i=0; i<m_lstCatalogs.size(); i++)
	{
		const char *szCatalogEntry = m_lstCatalogs[i].c_str();
		if( 0 == strcmp(szCatalogEntry, szLocale) )
			return i;
	}

	return -1; //not found
}

int	GuiLanguage::GetLocaleTableIdx(int nLangID)
{
#ifdef _WIN32
	int nMax = ARRAY_COUNT(isoLanguages);
	for (int i=0; i<nMax; i++)
        if( nLangID == MAKELANGID(isoLanguages[i].nLangID_W32, isoLanguages[i].nSubLangID_W32))
			return i;
#endif
	return -1; //not found
}

bool OnDirEnum(const char *szDir, void *data)
{
	std::vector<std::string> *lstDirs = (std::vector<std::string> *)data;	
	if(data)
	{
		//calculate directory base name and add it into the list
		#ifdef _WIN32
			//calculate base name
			std::string strDir(szDir);
			int nPos = strDir.find_last_of('\\');
			if(nPos > 0)
				strDir = strDir.substr(nPos+1, 1000);
		#else
			std::string strDir(basename((char *)szDir));
		#endif	
				
		lstDirs->push_back(strDir);
	}
	
	return true;	//keep enumerating
}

const char *GuiLanguage::QueryLocale(int category)
{
	//return current locale settings
	//WARNING: on Windows this returns global system settings, not program's
	return setlocale(category, NULL);
}

const char *GuiLanguage::GetLocale()
{
	return m_strCurLang.c_str();
}

bool GuiLanguage::IsCurLangRTL()
{
	if(m_nLangIdx >= 0)
		return isoLanguages[m_nLangIdx].bRTL;
	return false;
}

void GuiLanguage::InitializeToSystemLang()
{
#ifdef _WIN32
	LANGID nLangID = GetUserDefaultLangID(); 
	int nIdx = GetLocaleTableIdx(nLangID);
	if(nIdx >= 0)
	{
		Initialize(isoLanguages[nIdx].iso);
	}
#else
	//query LANG environment variable to set default locale
	const char *szLocale = getenv("LANG"); //setlocale(LC_ALL, NULL);
	if(szLocale)
	{
		Initialize(szLocale);
	}
#endif
}
