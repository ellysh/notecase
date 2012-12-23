# Microsoft Developer Studio Project File - Name="LibNote" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=LibNote - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "LibNote.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "LibNote.mak" CFG="LibNote - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "LibNote - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "LibNote - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "LibNote - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x41a /d "NDEBUG"
# ADD RSC /l 0x41a /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "LibNote - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x41a /d "_DEBUG"
# ADD RSC /l 0x41a /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "LibNote - Win32 Release"
# Name "LibNote - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\blowfish.cpp
# End Source File
# Begin Source File

SOURCE=.\DocumentIterator.cpp
# End Source File
# Begin Source File

SOURCE=.\DocumentList.cpp
# End Source File
# Begin Source File

SOURCE=.\File64.cpp
# End Source File
# Begin Source File

SOURCE=.\FormatEncHtml.cpp
# End Source File
# Begin Source File

SOURCE=.\FormatHtml.cpp
# End Source File
# Begin Source File

SOURCE=.\HtmlParser.cpp
# End Source File
# Begin Source File

SOURCE=.\LibNote.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteDocument.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteFormatIO.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteNode.cpp
# End Source File
# Begin Source File

SOURCE=.\SHA1.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\blowfish.h
# End Source File
# Begin Source File

SOURCE=.\blowfish2.h
# End Source File
# Begin Source File

SOURCE=.\DocumentIterator.h
# End Source File
# Begin Source File

SOURCE=.\DocumentList.h
# End Source File
# Begin Source File

SOURCE=.\File64.h
# End Source File
# Begin Source File

SOURCE=.\FormatEncHtml.h
# End Source File
# Begin Source File

SOURCE=.\FormatHtml.h
# End Source File
# Begin Source File

SOURCE=.\HtmlParser.h
# End Source File
# Begin Source File

SOURCE=.\LibNote.h
# End Source File
# Begin Source File

SOURCE=.\NoteDocument.h
# End Source File
# Begin Source File

SOURCE=.\NoteFormatIO.h
# End Source File
# Begin Source File

SOURCE=.\NoteNode.h
# End Source File
# Begin Source File

SOURCE=.\SHA1.h
# End Source File
# End Group
# End Target
# End Project
