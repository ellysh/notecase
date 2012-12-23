# Microsoft Developer Studio Project File - Name="NoteCase" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=NoteCase - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "NoteCase.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "NoteCase.mak" CFG="NoteCase - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "NoteCase - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "NoteCase - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "NoteCase - Win32 Profile" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "NoteCase - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "bin/Release"
# PROP Intermediate_Dir "bin/Release/obj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "C:\GTK\INCLUDE" /I "C:\GTK\INCLUDE\GLIB-2.0" /I "C:\GTK\INCLUDE\GTK-2.0" /I "C:\GTK\LIB\GLIB-2.0\INCLUDE" /I "C:\GTK\LIB\GTK-2.0\INCLUDE" /I "C:\GTK\INCLUDE\PANGO-1.0" /I "C:\GTK\INCLUDE\ATK-1.0" /I "C:\GTK\include\cairo\\" /I "c:\gtk\include\gtksourceview-2.0\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_INTL_REDIRECT_MACROS" /D "ENABLE_NLS" /D "HAVE_GTKSOURCEVIEW" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41a /d "NDEBUG"
# ADD RSC /l 0x41a /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib gtk-win32-2.0.lib gdk-win32-2.0.lib glib-2.0.lib gobject-2.0.lib atk-1.0.lib gdk_pixbuf-2.0.lib pango-1.0.lib intl.lib cairo.lib pangocairo-1.0.lib libgtksourceview-2.0-0.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"LIBCMT" /libpath:"C:\gtk\lib\\"

!ELSEIF  "$(CFG)" == "NoteCase - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "bin/Debug"
# PROP Intermediate_Dir "bin/Debug/obj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "C:\GTK\INCLUDE" /I "C:\GTK\INCLUDE\GLIB-2.0" /I "C:\GTK\INCLUDE\GTK-2.0" /I "C:\GTK\LIB\GLIB-2.0\INCLUDE" /I "C:\GTK\LIB\GTK-2.0\INCLUDE" /I "C:\GTK\INCLUDE\PANGO-1.0" /I "C:\GTK\INCLUDE\ATK-1.0" /I "C:\GTK\include\cairo\\" /I "c:\gtk\include\gtksourceview-2.0\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_INTL_REDIRECT_MACROS" /D "ENABLE_NLS" /D "HAVE_GTKSOURCEVIEW" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41a /d "_DEBUG"
# ADD RSC /l 0x41a /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib gtk-win32-2.0.lib gdk-win32-2.0.lib glib-2.0.lib gobject-2.0.lib atk-1.0.lib gdk_pixbuf-2.0.lib pango-1.0.lib intl.lib cairo.lib pangocairo-1.0.lib  libgtksourceview-2.0-0.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBCMTD" /pdbtype:sept /libpath:"C:\gtk\lib\\"

!ELSEIF  "$(CFG)" == "NoteCase - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "NoteCase___Win32_Profile"
# PROP BASE Intermediate_Dir "NoteCase___Win32_Profile"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "bin/Profile"
# PROP Intermediate_Dir "bin/Profile/obj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "C:\GTK\INCLUDE" /I "C:\GTK\INCLUDE\GLIB-2.0" /I "C:\GTK\INCLUDE\GTK-2.0" /I "C:\GTK\LIB\GLIB-2.0\INCLUDE" /I "C:\GTK\LIB\GTK-2.0\INCLUDE" /I "C:\GTK\INCLUDE\PANGO-1.0" /I "C:\GTK\INCLUDE\ATK-1.0" /I "C:\GTK\include\cairo\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_INTL_REDIRECT_MACROS" /D "ENABLE_NLS" /YX /FD /c
# ADD CPP /nologo /W3 /GR /GX /O2 /I "C:\GTK\INCLUDE" /I "C:\GTK\INCLUDE\GLIB-2.0" /I "C:\GTK\INCLUDE\GTK-2.0" /I "C:\GTK\LIB\GLIB-2.0\INCLUDE" /I "C:\GTK\LIB\GTK-2.0\INCLUDE" /I "C:\GTK\INCLUDE\PANGO-1.0" /I "C:\GTK\INCLUDE\ATK-1.0" /I "C:\GTK\include\cairo\\" /I "c:\gtk\include\gtksourceview-2.0\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_INTL_REDIRECT_MACROS" /D "ENABLE_NLS" /D "HAVE_GTKSOURCEVIEW" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41a /d "NDEBUG"
# ADD RSC /l 0x41a /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib gtk-win32-2.0.lib gdk-win32-2.0.lib glib-2.0.lib gobject-2.0.lib atk-1.0.lib gdk_pixbuf-2.0.lib pango-1.0.lib intl.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"LIBCMT" /libpath:"C:\gtk\lib\\"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib gtk-win32-2.0.lib gdk-win32-2.0.lib glib-2.0.lib gobject-2.0.lib atk-1.0.lib gdk_pixbuf-2.0.lib pango-1.0.lib intl.lib libgtksourceview-2.0-0.lib /nologo /subsystem:windows /profile /machine:I386 /nodefaultlib:"LIBCMT" /libpath:"C:\gtk\lib\\"

!ENDIF 

# Begin Target

# Name "NoteCase - Win32 Release"
# Name "NoteCase - Win32 Debug"
# Name "NoteCase - Win32 Profile"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\AboutDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\Base64.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\blowfish.cpp
# End Source File
# Begin Source File

SOURCE=.\src\_win\bootstart.cpp
# End Source File
# Begin Source File

SOURCE=.\src\callbacks.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\CircularBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DateTimeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\debug.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gui\Dialog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DocAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DocActionAtt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DocActionFinish.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DocActionFinishDel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DocActionFmt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\DocActionManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DocActionPix.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DocActionSort.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\DocumentIterator.cpp
# End Source File
# Begin Source File

SOURCE=.\src\EditDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\EnumDirectory.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\File64.cpp
# End Source File
# Begin Source File

SOURCE=.\src\FileAttachmentDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gui\FileDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\FileExportDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\FilePath.cpp
# End Source File
# Begin Source File

SOURCE=.\src\FileSaveAsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\FindDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\FindReplaceDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\FindReplaceInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\FmtInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\FormatIOBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\FormatIOEncHtml.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\FormatIOExecutable.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\FormatIOGjots2.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\FormatIOHtml.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\FormatIOMMLX.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\FormatIOStickyNotes.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\FormatIOTxt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gui\GuiLanguage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\HtmlParser.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\IniFile.cpp
# End Source File
# Begin Source File

SOURCE=.\src\interface.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\IOLayerBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\IOLayerEnc.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\IOLayerFile64.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\IOLayerRedirect.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\IOLayerZlib.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\IOProcess.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\LinkInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\LinkPropertiesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\main.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MainWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\src\mru.cpp
# End Source File
# Begin Source File

SOURCE=.\src\NodePropertiesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\res\notecase.rc
# End Source File
# Begin Source File

SOURCE=.\src\lib\NoteDocument.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\NoteNode.cpp
# End Source File
# Begin Source File

SOURCE=.\src\OptionsDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PasswordDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PixPropertiesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PortableTrayIcon.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gui\ProgressDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\_win\RegisterExtension.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\SHA1.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ShortcutsList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ShortcutsListDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\_win\SingleInstanceWin.cpp
# End Source File
# Begin Source File

SOURCE=.\src\support.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\TextSearch.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TextView.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lib\Thread.cpp
# End Source File
# Begin Source File

SOURCE=.\src\_win\TrayIcon.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TreeView.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\AboutDlg.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\Base64.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\blowfish.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\blowfish2.h
# End Source File
# Begin Source File

SOURCE=.\src\_win\bootstart.h
# End Source File
# Begin Source File

SOURCE=.\src\callbacks.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\CircularBuffer.h
# End Source File
# Begin Source File

SOURCE=.\src\config.h
# End Source File
# Begin Source File

SOURCE=.\src\DateTimeDlg.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\debug.h
# End Source File
# Begin Source File

SOURCE=.\src\gui\Dialog.h
# End Source File
# Begin Source File

SOURCE=.\src\DocAction.h
# End Source File
# Begin Source File

SOURCE=.\src\DocActionAtt.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\DocActionBase.h
# End Source File
# Begin Source File

SOURCE=.\src\DocActionFinish.h
# End Source File
# Begin Source File

SOURCE=.\src\DocActionFinishDel.h
# End Source File
# Begin Source File

SOURCE=.\src\DocActionFmt.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\DocActionManager.h
# End Source File
# Begin Source File

SOURCE=.\src\DocActionPix.h
# End Source File
# Begin Source File

SOURCE=.\src\DocActionSort.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\DocumentIterator.h
# End Source File
# Begin Source File

SOURCE=.\src\EditDlg.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\EnumDirectory.h
# End Source File
# Begin Source File

SOURCE=.\src\ExecuteFile.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\File64.h
# End Source File
# Begin Source File

SOURCE=.\src\FileAttachmentDlg.h
# End Source File
# Begin Source File

SOURCE=.\src\gui\FileDialog.h
# End Source File
# Begin Source File

SOURCE=.\src\FileExportDlg.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\FilePath.h
# End Source File
# Begin Source File

SOURCE=.\src\FileSaveAsDlg.h
# End Source File
# Begin Source File

SOURCE=.\src\FindDialog.h
# End Source File
# Begin Source File

SOURCE=.\src\FindReplaceDlg.h
# End Source File
# Begin Source File

SOURCE=.\src\FindReplaceInfo.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\FmtInfo.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\FormatIOBase.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\FormatIOEncHtml.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\FormatIOExecutable.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\FormatIOGjots2.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\FormatIOHtml.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\FormatIOMMLX.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\FormatIOStickyNotes.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\FormatIOTxt.h
# End Source File
# Begin Source File

SOURCE=.\src\gui\GuiLanguage.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\HtmlParser.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\IniFile.h
# End Source File
# Begin Source File

SOURCE=.\src\interface.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\IOLayerBase.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\IOLayerEnc.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\IOLayerFile64.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\IOLayerRedirect.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\IOLayerZlib.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\IOProcess.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\LinkInfo.h
# End Source File
# Begin Source File

SOURCE=.\src\LinkPropertiesDlg.h
# End Source File
# Begin Source File

SOURCE=.\src\MainWnd.h
# End Source File
# Begin Source File

SOURCE=.\src\mru.h
# End Source File
# Begin Source File

SOURCE=.\src\NodePropertiesDlg.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\NoteDocument.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\NoteNode.h
# End Source File
# Begin Source File

SOURCE=.\src\OptionsDialog.h
# End Source File
# Begin Source File

SOURCE=.\src\PasswordDialog.h
# End Source File
# Begin Source File

SOURCE=.\src\PixPropertiesDlg.h
# End Source File
# Begin Source File

SOURCE=.\src\PortableTrayIcon.h
# End Source File
# Begin Source File

SOURCE=.\src\gui\ProgressDlg.h
# End Source File
# Begin Source File

SOURCE=.\src\_win\RegisterExtension.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\SHA1.h
# End Source File
# Begin Source File

SOURCE=.\src\ShortcutsList.h
# End Source File
# Begin Source File

SOURCE=.\src\ShortcutsListDlg.h
# End Source File
# Begin Source File

SOURCE=.\src\_win\SingleInstance.h
# End Source File
# Begin Source File

SOURCE=.\src\support.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\TextSearch.h
# End Source File
# Begin Source File

SOURCE=.\src\TextView.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\Thread.h
# End Source File
# Begin Source File

SOURCE=.\src\_win\TrayIcon.h
# End Source File
# Begin Source File

SOURCE=.\src\TreeView.h
# End Source File
# Begin Source File

SOURCE=.\src\lib\types.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\notecase.ico
# End Source File
# End Group
# End Target
# End Project
