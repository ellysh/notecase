////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Main method implementation (application startup)
////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "mru.h"
#include <gtk/gtk.h>
#include <libintl.h>

#include "interface.h"
#include "callbacks.h"
#include "support.h"
#include "lib/IniFile.h"
#include "lib/FilePath.h"
#include "lib/NoteDocument.h"
#include "gui/GuiLanguage.h"
#include "MainWnd.h"
#include "lib/CircularBuffer.h"
#include "lib/EnumDirectory.h"
#include "lib/Thread.h"
#include <signal.h>
#include <stdlib.h>

#ifdef _WIN32
 #include <io.h> //access
 #define access _access
#else
 #include <sys/resource.h>	//setrlimit
 #include <sys/mman.h>		//mlockall
 #include <unistd.h>		//access
#endif

#ifndef _WIN32
 #include <libgen.h>		//basename
#endif

#ifdef _WIN32
 #include "_win/SingleInstance.h"
 CSingleInstance g_instance("NoteCase");
#else
 #include "_unx/SingleInstance.h"
 CSingleInstance g_instance("/NoteCase");
#endif

#ifdef _NOKIA_MAEMO
	#include <libosso.h>
	// The main OSSO context of the application
	static osso_context_t *g_osso = NULL;
#endif

const char *get_locale_dir();

GtkWidget *window1;
extern MRU g_objMRU;
bool g_bMinimizeToTray = false;
extern MainWindow g_wnd;
extern NoteDocument g_doc;
CircularBuffer g_objCrashLog;
std::string m_strLogFile;
IniFile g_objIni;
unsigned int g_nProcessID = 0;
GuiLanguage g_lang(PACKAGE, get_locale_dir());

bool g_bInitialLoad = false;
bool g_bLoadEmbedded = false;
bool g_bUpdateMRU = false;
bool g_bDebugMode = false;
std::string g_strLoadDocumentFile;
std::string g_strLoadDocumentPass;
int g_nLoadDocumentNodeID;

int  load_file(const char *filename, const char *szPassword = NULL, int nCurNodeIdx = -1);
int  load_file_embedded(int nCurNodeIdx = -1);
void export_document(const NoteDocument &document, int nExportMode, const std::string &strExportFile, const std::string &strPostProcessScript, const std::string &strCSS, int nEmbedCSS = -1, bool bCheckOverwrite = true, bool bExportLinked = false, int nFormat = FORMAT_NONE, int nExportBranchIdx=-1);

class OpListLang : public Thread
{
protected:
	virtual void MainMethod();
};

void OpListLang::MainMethod()
{
	g_lang.ListAvailableCatalogs();
}

#ifdef HAVE_GNOME_VFS
 #include <libgnomevfs/gnome-vfs.h>
 #include <libgnomevfs/gnome-vfs-mime-handlers.h>
#endif

bool Utf8ToLocale(const char *szTxt, std::string &strResult);

bool is_portable_mode()
{
	bool bPortable = false;
	const char *szEnv = getenv("NOTECASE_PORTABLE");
	if(NULL != szEnv)
		bPortable = true;

	return bPortable;
}

//makes sense only on Win32/portable mode, returns launcher.exe path
std::string CalcLauncherPath()
{
	ASSERT(is_portable_mode());

	static std::string strLauncher;
	if(strLauncher.empty())
	{
		strLauncher = GetParentDir(GetAppPath().c_str());
		strLauncher = GetParentDir(strLauncher.c_str());
		strLauncher += "notecaseprolauncher.exe";
	}

	return strLauncher;
}

const char *GetIniFile()
{
	static std::string strDir;
	const char *szEnv = getenv("NOTECASE_INI");
	if(NULL != szEnv)
		strDir = szEnv;
	else
	{
		strDir  = GetHomeDir();

#ifdef _WIN32
		strDir += "\\.notecase\\notecase.ini";
#else
		strDir += "/.notecase/notecase.ini";
#endif
	}
	return strDir.c_str();
}

const char *get_config_dir()
{
	static std::string strDir;
		strDir  = GetHomeDir();

#ifdef _WIN32
		strDir += "\\.notecase\\";
#else
		strDir += "/.notecase/";
#endif

	return strDir.c_str();
}

std::string GetHelpFile()
{
	//prepare help document path
	std::string strHelp;

	//first try to see if the user has set special help file name path
	//through the NOTECASE_HELP environment variable
	const char *szEnv = getenv("NOTECASE_HELP");
	if(NULL != szEnv)
		strHelp = szEnv;

#ifndef _WIN32
	//Linux path is fixed and defined at compile time
	if(strHelp.empty()){
		strHelp  = DOC_DIR_PATH;
		strHelp += HELP_FILE_PATH;
	}
#endif

	//if file not defined in a fixed location or not found, calc path dynamically
	if( strHelp.empty() ||
	    0 != access(strHelp.c_str(), 0))
	{
		//Win32 and Linux debug version
		std::string strHelp1 = GetParentDir(GetAppPath().c_str());
		strHelp1 += "help.ncd";

	#ifndef _WIN32
		if(0 == access(strHelp1.c_str(), 0))
	#endif
			strHelp = strHelp1;
	}

	return strHelp;
}

const char * get_autosave_dir()
{
	static std::string strDir;
	strDir  = GetHomeDir();
#ifdef _WIN32
	strDir += "\\.notecase\\";
#else
	strDir += "/.notecase/";
#endif

	return strDir.c_str();
}

const char *calculate_autosave_filename()
{
	//there exists one autosave file per user
	static std::string strDir;
	strDir   = get_autosave_dir();
	strDir  += "autosave";

	char szBuffer[20];
	sprintf(szBuffer, "_n%d", g_nProcessID);

	strDir += szBuffer;
	strDir += ".ncd";

	return strDir.c_str();
}

//this is a version for encrypted format
const char *calculate_autosave_filename1()
{
	//there exists one autosave file per user
	static std::string strDir;
	strDir   = get_autosave_dir();
	strDir  += "autosave";

	char szBuffer[20];
	sprintf(szBuffer, "_n%d", g_nProcessID);

	strDir += szBuffer;
	strDir += ".nce";

	return strDir.c_str();
}

const char *get_locale_dir()
{
	static std::string strDir;
#ifdef _WIN32
	strDir  = GetAppPath();
	strDir  = GetParentDir(strDir.c_str());
	strDir += "locale\\";
#else
	strDir  = INSTALL_PREFIX;
	strDir += LOCALE_DIR;
#endif
	return strDir.c_str();
}

void signal_handler(int sig)
{
	//generate debug dump
	g_objCrashLog.Printf("VERSION INFO: Notecase %s\n", APP_VER_STR);
	g_objCrashLog.Dump(m_strLogFile.c_str());

	#ifndef _WIN32
		_exit(1);
	#endif
}

bool OnCrashLogEnum(const char *szFile, void *data)
{
	std::vector<std::string> *lstLogs = (std::vector<std::string> *)data;
	if(data)
	{
	#ifdef _WIN32
		//calculate base name
		std::string strFile(szFile);
		int nPos = strFile.find_last_of('\\');
		if(nPos > 0)
			strFile = strFile.substr(nPos+1);
	#else
		std::string strFile(basename((char *)szFile));
	#endif

		if(0 == strncmp(strFile.c_str(), "notecase_crash_", strlen("notecase_crash_"))){
			lstLogs->push_back(szFile);
		}
	}

	return true;	//keep enumerating
}

void CheckCrashLogFiles()
{
#ifdef GENERATE_CRASHLOG
	//
	// detect and report crash log files
	//
	static std::string strLogDir;
	strLogDir  = GetHomeDir();
	strLogDir += "/.notecase/";
	std::vector<std::string> lstLogs;
	EnumDirectory(strLogDir.c_str(), OnCrashLogEnum, (void *)&lstLogs, ENUM_LST_FILES);

	if(lstLogs.size() > 0){
		char szMsg[2048];
		sprintf(szMsg, _("One or more crash logs were found in:\n%s.\n\nPress Yes to delete those files\n(consider sending the files to the author with the crash description)."), strLogDir.c_str());
		gint result = gtkMessageBox(szMsg, GTK_BUTTONS_YES_NO);
		if(GTK_RESPONSE_YES == result){
			int nCnt = lstLogs.size();
			for(int i=0; i<nCnt; i++)
				remove(lstLogs[i].c_str());
		}
	}
#endif
}

#ifndef WIN32
#ifndef _OSX_BUILD
void lock_process_memory()
{
    int lock = 0;
    if (getuid() == 0)
    {
        lock = mlockall(MCL_CURRENT|MCL_FUTURE);
        if (lock == -1)
        {
            fprintf(stderr,"Running as root but unable to lock memory to prevent paging to swap\n");
            exit(-1);
        }
        lock = 1;
    }
	if (lock)
        fprintf(stderr,"NOTE: Running as root and memory is locked from paging to disk\n");
    else
        fprintf(stderr,"WARNING: Running unprivileged and memory is not locked from paging to disk\n");
}
#endif
#endif

int main (int argc, char *argv[])
{
#ifndef _WIN32
 #ifdef RLIMIT_DATA
	//UNIX: protect sensitive data to leak into possible core dump file
	struct rlimit rlim;
	getrlimit(RLIMIT_CORE, &rlim);
	rlim.rlim_max = rlim.rlim_cur = 0;
	if(setrlimit(RLIMIT_CORE, &rlim)){
		printf("ERROR: Failed to limit core dump size!\n");
		exit(-1);
	}
 #endif
#endif

#ifdef GENERATE_CRASHLOG
	//install signal handlers to catch crash situations
	signal(SIGABRT, signal_handler);
	signal(SIGFPE,  signal_handler);
	signal(SIGINT,  signal_handler);
	signal(SIGILL,  signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGSEGV, signal_handler);

	//pre-generate file name for the eventual crash dump
	m_strLogFile = GetHomeDir();
#ifdef _WIN32
	m_strLogFile += "\\.notecase\\notecase_crash_";
#else
	m_strLogFile += "/.notecase/notecase_crash_";
#endif
	char szBuffer[200];
	time_t nTime = time(NULL);
	tm *pCurTm = localtime(&nTime);
	sprintf(szBuffer, "%02d%02d%02d_%02d%02d%02d",
			pCurTm->tm_year%100,
			pCurTm->tm_mon + 1,
			pCurTm->tm_mday,
			pCurTm->tm_hour,
			pCurTm->tm_min,
			pCurTm->tm_sec);
	m_strLogFile += szBuffer;
	m_strLogFile += ".log";
#endif

	//calculate PID
	g_nProcessID = GetCurrentPID();
	g_objCrashLog.Printf("INFO: Proces ID = %d\n", g_nProcessID);


	//check if this exe has embedded document
	bool bStandalone = false;
	std::string strSrcExe = GetAppPath();
	FILE *pExe = fopen(strSrcExe.c_str(), "rb");
	if(NULL != pExe){
		//check "magic" marker at the end of the file
		fseek(pExe, -8, SEEK_END);
		char szBuffer[10];
		size_t nRead = 0;
		nRead = fread(szBuffer, 8, 1, pExe);
		szBuffer[8] = '\0';
		if(0 == strcmp("NOTECASE", szBuffer)){
			bStandalone = true;	//marker found
		}
		fclose(pExe);
	}

#ifdef _NOKIA_MAEMO
	// Initialize g_osso
    g_osso = osso_initialize("notecase", "1.0.0", TRUE, NULL);
    if(!g_osso)
    {
        g_printerr("osso_initialize failed.\n");
        //return 1;
    }
#endif

	g_objCrashLog.Printf("Trying to load INI = %s\n", GetIniFile());

	//check startup options
	g_objIni.Load(GetIniFile());

#ifndef WIN32
#ifndef _OSX_BUILD
	bool bLockMemory = false;
	g_objIni.GetValue("Security", "LockProcessMemory", bLockMemory);
	if(bLockMemory)
		lock_process_memory();
#endif
#endif

	g_objCrashLog.Printf("Done loading INI\n");

	//initialize language system
	bool bUseSystemLang;
	g_objIni.GetValue("Display", "UseSystemLanguage", bUseSystemLang);
	if(bUseSystemLang)
		g_lang.InitializeToSystemLang();
	else{
		std::string strLocale;
		g_objIni.GetValue("Display", "Language", strLocale, "");
		g_lang.Initialize(strLocale.c_str());
	}

	gdk_threads_enter();

	gtk_init (&argc, &argv);

	bool bAllowSingleInst = false;
	g_objIni.GetValue("Startup", "AllowSingleInstance", bAllowSingleInst);
	if(bAllowSingleInst && g_instance.ProgramAlreadyStarted())
	{
		gtkMessageBox(_("Another NoteCase instance detected! Exiting!"));
		return 0;
	}

	if(!bStandalone)
	{
		bool bBackupLoaded = autosave_check_crash();

		// check for command line arguments: file name to open
		if(!bBackupLoaded)
		{
			bool bExportFile = false;
			bool bExportLinks = false;
			bool bEmbedCSS = false;
			bool bExportBranchOnly = false;
			int  nExportBranchIdx = -1;
			int  nExportFormat = FORMAT_HTML;
			int  nCurNodeIdx = -1;
			std::string strExportFile;
			std::string strExportCSS;
			std::string strPassword;
			std::string strLoadFile;

			if(argc > 1)
			{
				for(int i=1; i<argc; i++)
				{
					if(0 == strcmp(argv[i], "--help"))
					{
						if(!g_wnd.m_pWidget) g_wnd.Create();
						on_help1_activate (NULL, NULL);	//load help
					}
					else if(0 == strcmp(argv[i], "--password"))
					{
						if(i+1 < argc){ //next token must be password
							strPassword = argv[i+1];
							i++; continue;
						}
						else break;	//invalid flag usage
					}
					else if(0 == strcmp(argv[i], "--export-file"))
					{
						if(i+1 < argc){ //next token must be file name
							strExportFile = argv[i+1];
							bExportFile = true;
							i++; continue;
						}
						else break;	//invalid flag usage
					}
					else if(0 == strcmp(argv[i], "--export-css"))
					{
						if(i+1 < argc){ //next token must be CSS path
							strExportCSS = argv[i+1];
							bExportFile = true;
							i++; continue;
						}
						else break;	//invalid flag usage
					}
					else if(0 == strcmp(argv[i], "--export-links"))
					{
						bExportLinks = true;
					}
					else if(0 == strcmp(argv[i], "--embed-css"))
					{
						bEmbedCSS = true;
					}
					else if(0 == strcmp(argv[i], "--export-branch"))
					{
						if(i+1 < argc){ //next token must be branch/node index
							nExportBranchIdx = atoi(argv[i+1]);
							bExportBranchOnly = true;
							i++; continue;
						}
						else break;	//invalid flag usage
					}
					else if(0 == strcmp(argv[i], "--export-format"))
					{
						if(i+1 < argc){ //next token must be branch/node index
							if(0 == strcmp(argv[i+1], "text"))
								nExportFormat = FORMAT_TXT;
							else if(0 == strcmp(argv[i+1], "exe"))
								nExportFormat = FORMAT_EXECUTABLE;
							else if(0 == strcmp(argv[i+1], "ncd"))
								nExportFormat = FORMAT_NOTECASE_HTML;
							i++; continue;
						}
						//else break;	//invalid flag usage
					}
					else if(0 == strcmp(argv[i], "--node"))
					{
						if(i+1 < argc){ //next token must be branch/node index
							nCurNodeIdx = atoi(argv[i+1]);
							i++; continue;
						}
						else break;	//invalid flag usage
					}
					else if(0 == strcmp(argv[i], "--debug"))
					{
						g_bDebugMode = true;
					}
					else
					{
						if(strLoadFile.size() > 0)
							strLoadFile += " ";
						strLoadFile += argv[i];
					}
				}	// end for loop

				if(bExportFile)
				{
					g_objCrashLog.AddMsg("Starting program in export mode\n");
					if(strLoadFile.size() > 0){
						int nResult = load_file(strLoadFile.c_str(), strPassword.c_str());
						if(DOC_LOAD_OK == nResult)
							export_document(g_doc, bExportBranchOnly? 0 /* branch*/ : 2 /*entire doc*/, strExportFile, "", strExportCSS, bEmbedCSS, false, bExportLinks, nExportFormat, nExportBranchIdx);
					}
					return 0;
				}

				g_objCrashLog.AddMsg("Starting program\n");

				if(strLoadFile.size() > 0){
					//gtkMessageBox(strLoadFile.c_str());
					//int nResult = load_file(strLoadFile.c_str(), strPassword.c_str(), nCurNodeIdx);
					//g_objMRU.Change(strLoadFile.c_str(), (DOC_LOAD_NOT_FOUND != nResult));//update MRU unless file not found

					g_bInitialLoad = true;
					g_bUpdateMRU = true;
					g_strLoadDocumentFile = strLoadFile;
					g_strLoadDocumentPass = strPassword;
					g_nLoadDocumentNodeID = nCurNodeIdx;
				}
				if(!g_wnd.m_pWidget) g_wnd.Create();
			}
			else{
				//check "reload last file" option
				g_objMRU.Load();

				bool bLoadLastDoc;
				if(g_objIni.GetValue("Startup", "ReloadLastDocument", bLoadLastDoc))
					if(bLoadLastDoc && (g_objMRU.GetSize() > 0)){
						g_bInitialLoad = true;
						g_bUpdateMRU = true;
						g_strLoadDocumentFile = g_objMRU.GetFile(0);
						g_nLoadDocumentNodeID = nCurNodeIdx;
					}

				if(!g_wnd.m_pWidget) g_wnd.Create();	// this will load MRU
			}
		}
	}
	else
	{
		//TOFIX read command line for embedded doc also ?
		//load document from .exe file image
		g_bInitialLoad = true;
		g_bLoadEmbedded = true;

		if(!g_wnd.m_pWidget) g_wnd.Create();
	}

	if(!g_wnd.m_pWidget) g_wnd.Create();

	CheckCrashLogFiles();

	OpListLang *pOpThread = new OpListLang;
	pOpThread->Run();

#ifdef HAVE_GNOME_VFS
	gnome_vfs_init();
#endif

	gtk_main ();

#ifdef HAVE_GNOME_VFS
	gnome_vfs_shutdown();
#endif

	gdk_threads_leave();

	delete pOpThread;

#ifdef _NOKIA_MAEMO
   osso_deinitialize(g_osso);
#endif

	//dump last actions?
	if(g_bDebugMode)
		signal_handler(0);

	return 0;
}

#ifdef _WIN32
#include <windows.h>
int APIENTRY WinMain( HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine, int nCmdShow)
{
	return main( __argc, __argv );
}
#endif
