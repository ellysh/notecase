#
# NoteCase hierarchical notes manager
# (http://notecase.sf.net)
#
# Usage: type "make help" for description of make options
#
# This makefile was tested on Linux (gcc/g++), Windows (Mingw/Msys), OSX, OpenSolaris and FreeBSD
# Use "gmake" to compile on OSX and FreeBSD
#
VERSION=1.9.8

# define install target dirs
prefix=/usr
bindir=$(prefix)/bin
datadir=$(prefix)/share

# define top of RPM build area
topdir=/usr/src/redhat
docdir=$(datadir)/doc
TEST_TOP_DIR=$(shell ls $(topdir) 2>/dev/null)
ifneq ($(TEST_TOP_DIR),)
else
 #rpm build on Suse Linux
 topdir=/usr/src/packages
 docdir=$(datadir)/doc/packages

 TEST_TOP_DIR=$(shell ls $(topdir) 2>/dev/null)
 ifneq ($(TEST_TOP_DIR),)
 else
  topdir=$(HOME)/rpm/
  docdir=$(datadir)/doc
 endif
endif

# define source archive name
archive=notecase-$(VERSION)

#
AUTODETECT_GTKSOURCEVIEW=1
HAVE_GTKSOURCEVIEW=

ifneq ($(AUTODETECT_GTKSOURCEVIEW),)
 TEST_SRCVIEW_VER=$(shell pkg-config --modversion gtksourceview-2.0 2>/dev/null)
 ifneq ($(TEST_SRCVIEW_VER),)
  HAVE_GTKSOURCEVIEW=1
 endif
endif

#comment these two lines to remove gnome-vfs support
#AUTODETECT_GNOME_VFS=1
#HAVE_GNOME_VFS=1

ifneq ($(AUTODETECT_GNOME_VFS),)
 TEST_GNOME_VFS=$(shell pkg-config --libs gnome-vfs-2.0 2>/dev/null)
 ifneq ($(TEST_GNOME_VFS),)
  HAVE_GNOME_VFS=1
 else
  HAVE_GNOME_VFS=
 endif
endif

# To profile NoteCase:
# 1. uncomment "PROFILE=..." line below 
# 2. comment out "LDFLAGS=..." line
# 3. comment out "strip $(BIN)/notecase" line
# 4. rebuild application
# 5. start and use application
# 6. profiling result is stored in a file "gmon.out" 
# 7. exec "gprof ./bin/notecase > aaa" to write results in a readable form into file "aaa"
#PROFILE=-pg

#DEBUG=-g -D_DEBUG

BYTE_ORDER=-DSHA1_LITTLE_ENDIAN -DORDER_DCBA

ifeq ($(CXX),)
 CXX=g++
endif

PLATFORM="Linux"

TESTZAURUS=$(shell uname -a | grep zaurus)
ifneq ($(TESTZAURUS),)
  ZAURUS_BUILD=1
  PLATFORM="Zaurus"
endif

TESTFREEBSD=$(shell uname -a | grep FreeBSD)
ifneq ($(TESTFREEBSD),)
  FREEBSD_BUILD=1
  PLATFORM="FreeBsd"
endif

TESTDARWIN=$(shell uname -a | grep Darwin)
ifneq ($(TESTDARWIN),)
  OSX_BUILD=1
  PLATFORM="OSX"
endif

TESTSOLARIS=$(shell uname -a | grep SunOS)
ifneq ($(TESTSOLARIS),)
  SOLARIS_BUILD=1
  PLATFORM="Solaris"
endif

TESTDEB=$(shell which dpkg 2>/dev/null)
ifneq ($(TESTDEB),)
  HAVEDEB=1
endif

#define proper make (use gmake for freebsd)
MAKE=make
ifneq ($(FREEBSD_BUILD),)
 MAKE=gmake
endif

# properly calculate architecture
ifdef HAVEDEB
 ARCH=$(shell dpkg --print-architecture 2>/dev/null)
 ifeq ($(ARCH),ppc-darwin)
	 BYTE_ORDER=-DSHA1_BIG_ENDIAN -DORDER_ABCD
   OSX_BUILD=1
   PLATFORM="OSX"
 else  
  TESTOSX=$(shell uname 2>/dev/null)
  ifeq ($(ARCH),Darwin)
    OSX_BUILD=1
    PLATFORM="OSX"
    ARCH=$(shell uname -m 2>/dev/null)
  else   
   ifeq ($(ARCH),armel)
		TEST=$(shell pkg-config --cflags hildon-1 2>/dev/null)
    ifeq ($(TEST),)
		 TEST=$(shell pkg-config --cflags hildon-libs 2>/dev/null)
		 ifeq ($(TEST),)
		 else 
			 NOKIA_MAEMO_BUILD=1
			 PLATFORM="Nokia Maemo"
		 endif	
    else
		 NOKIA_MAEMO_BUILD=2
		 PLATFORM="Nokia Maemo 2008"
    endif 
   endif
  endif
 endif
else
 ARCH=$(shell uname -m) 
endif

ifdef WINDIR
 EXE=.exe
ifeq ($(CFLAGS),)
 CFLAGS+=-Wall -fno-exceptions -fno-rtti -mms-bitfields -mwindows
endif
else
 EXE=
ifeq ($(CFLAGS),)
 CFLAGS+=-Wall
endif
endif 

CFLAGS += $(BYTE_ORDER)

ifdef OSX_BUILD
 EXE=.x11app
endif 

FLAGS=$(CFLAGS)
BIN=./bin
OBJ=./bin

#define proper install flags
INSTALL=install -D
ifdef OSX_BUILD
 INSTALL=install -c
endif 
ifdef FREEBSD_BUILD
 INSTALL=install
endif
ifdef SOLARIS_BUILD
 INSTALL=ginstall
endif

#define proper sed (use gnused for Darwin?)
SED=sed -i
ifneq ($(OSX_BUILD),)
	SED=sed -i.bak
endif
ifneq ($(FREEBSD_BUILD),)
  SED=sed -i.bak
endif
ifneq ($(SOLARIS_BUILD),)
  SED=/opt/csw/bin/gsed -i.bak
endif

MD5SUM=md5sum
ifneq ($(OSX_BUILD),)
   MD5SUM=gmd5sum
endif
MD5SUM_EXISTS=$(shell which $(MD5SUM)) 

ifneq ($(SOLARIS_BUILD),)
 #rpm build on OpenSolaris
 topdir=$(HOME)/packages
 docdir=$(datadir)/doc
endif
 
# define processing output
Q=@
QC=@echo '===> Compiling $<'; 
QL=@echo '===> Linking $@'; 

# using "--as-needed" to cut down link requirements (check by "readelf -d /usr/bin/notecase | grep NEEDED") 
ifeq ($(LDFLAGS),)
ifdef OSX_BUILD
 LDFLAGS=-Os -Wl -bind_at_load
else
 LDFLAGS=-Os -Wl,--as-needed
endif

ifneq ($(NOKIA_MAEMO_BUILD),)
 LDFLAGS += -ansi
endif
endif

# define compile/link command line with flags
CC=$(QC) $(CXX) $(DEBUG) $(PROFILE) -DENABLE_NLS
ifdef WINDIR
 LD=$(QL) $(CXX) $(DEBUG) $(PROFILE) $(LDFLAGS) -L$(OBJ) -lz 
else
 ifdef FREEBSD_BUILD
  LD=$(QL) $(CXX) $(DEBUG) $(PROFILE) $(LDFLAGS)
 else
  LD=$(QL) $(CXX) $(DEBUG) $(PROFILE) $(LDFLAGS) -ldl
 endif
endif

ifndef SOLARIS_BUILD 
ifndef OSX_BUILD 
 ifndef WINDIR
   LD+= -pthread
 endif
endif
endif 

#
# define default project to be built
#
all: $(BIN)/notecase$(EXE)

#
# list of all objects to build (notecase dependencies)
# WARNING: link can fail on Mingw if you put libz.a before IOLayerZlib.o (who dependes on libz.a)
#

OBJS= $(OBJ)/main.o $(OBJ)/support.o $(OBJ)/callbacks.o $(OBJ)/interface.o $(OBJ)/mru.o \
 $(OBJ)/EnumDirectory.o $(OBJ)/DocActionManager.o $(OBJ)/DocAction.o $(OBJ)/DocActionSort.o \
 $(OBJ)/OptionsDialog.o $(OBJ)/GuiLanguage.o $(OBJ)/TextSearch.o $(OBJ)/FileDialog.o \
 $(OBJ)/Dialog.o $(OBJ)/FindDialog.o $(OBJ)/PasswordDialog.o $(OBJ)/FilePath.o \
 $(OBJ)/IniFile.o $(OBJ)/blowfish.o $(OBJ)/DocumentIterator.o $(OBJ)/FormatIOExecutable.o \
 $(OBJ)/IOLayerEnc.o $(OBJ)/File64.o $(OBJ)/FormatIOEncHtml.o $(OBJ)/FormatIOHtml.o \
 $(OBJ)/FormatIOGjots2.o $(OBJ)/HtmlParser.o $(OBJ)/NoteDocument.o $(OBJ)/FormatIOBase.o \
 $(OBJ)/NoteNode.o $(OBJ)/SHA1.o $(OBJ)/PortableTrayIcon.o $(OBJ)/Base64.o\
 $(OBJ)/SingleInstance.o $(OBJ)/TreeView.o $(OBJ)/TextView.o $(OBJ)/MainWnd.o \
 $(OBJ)/FormatIOStickyNotes.o $(OBJ)/NodePropertiesDlg.o $(OBJ)/debug.o \
 $(OBJ)/IOProcess.o $(OBJ)/IOLayerZlib.o $(OBJ)/IOLayerFile64.o $(OBJ)/IOLayerBase.o \
 $(OBJ)/IOLayerRedirect.o $(OBJ)/FileExportDlg.o $(OBJ)/FormatIOTxt.o $(OBJ)/FindReplaceDlg.o \
 $(OBJ)/FindReplaceInfo.o $(OBJ)/LinkInfo.o $(OBJ)/LinkPropertiesDlg.o $(OBJ)/ExecuteFile.o \
 $(OBJ)/FileSaveAsDlg.o $(OBJ)/CircularBuffer.o $(OBJ)/FormatIOMMLX.o \
 $(OBJ)/ProgressDlg.o $(OBJ)/DocActionPix.o $(OBJ)/DocActionFinish.o $(OBJ)/DocActionFmt.o $(OBJ)/FmtInfo.o \
 $(OBJ)/FileAttachmentDlg.o $(OBJ)/DocActionAtt.o $(OBJ)/PixPropertiesDlg.o $(OBJ)/DocActionFinishDel.o \
 $(OBJ)/ShortcutsList.o $(OBJ)/ShortcutsListDlg.o $(OBJ)/DateTimeDlg.o $(OBJ)/AboutDlg.o $(OBJ)/EditDlg.o \
 $(OBJ)/Thread.o

ifdef WINDIR
 OBJS += $(OBJ)/resources.o $(OBJ)/TrayIcon.o $(OBJ)/bootstart.o $(OBJ)/RegisterExtension.o
else
 OBJS += $(OBJ)/eggtrayicon.o
endif

SRCPATH=./src/lib
GUIPATH=./src

ifdef WINDIR
 # undef VFS on windows
 HAVE_GNOME_VFS=
endif

LIBS=gtk+-2.0 glib-2.0 gthread-2.0 x11 zlib

ifneq ($(HAVE_GNOME_VFS),)
 LIBS += gnome-vfs-2.0
endif
ifneq ($(HAVE_GTKSOURCEVIEW),)
 LIBS += gtksourceview-2.0
endif
ifeq ($(NOKIA_MAEMO_BUILD),1)
 LIBS += hildon-libs hildon-fm libosso 
else 
 ifeq ($(NOKIA_MAEMO_BUILD),2)
 	LIBS += hildon-1 hildon-fm-2 libosso 
 endif
endif  

LIBSCFLAGS=`pkg-config --cflags $(LIBS) 2>/dev/null` 
GTKCFLAGS=$(LIBSCFLAGS) -I/target/include  -I/sw/include 
GTKLIBS=`pkg-config --libs $(LIBS)`

#
# link program binary
#
ifdef WINDIR
$(BIN)/notecase$(EXE): $(OBJS)
	$(LD) $(OBJS) -o $(BIN)/notecase$(EXE) $(FLAGS) $(LDFLAGS) $(GTKLIBS)
else
$(BIN)/notecase$(EXE): updatesrc $(OBJS)
	$(LD) $(OBJS) -o $(BIN)/notecase$(EXE) $(FLAGS) $(LDFLAGS) $(GTKLIBS) > /dev/null
endif	

#
# compile individual objects
#
$(OBJ)/DocActionManager.o: $(SRCPATH)/DocActionManager.cpp $(SRCPATH)/DocActionManager.h
	$(CC) $(FLAGS) -c $(SRCPATH)/DocActionManager.cpp -o $(OBJ)/DocActionManager.o  $(GTKCFLAGS)

$(OBJ)/DocAction.o: $(GUIPATH)/DocAction.cpp $(GUIPATH)/DocAction.h
	$(CC) $(FLAGS) -c $(GUIPATH)/DocAction.cpp -o $(OBJ)/DocAction.o $(GTKCFLAGS)

$(OBJ)/DocActionSort.o: $(GUIPATH)/DocActionSort.cpp $(GUIPATH)/DocActionSort.h
	$(CC) $(FLAGS) -c $(GUIPATH)/DocActionSort.cpp -o $(OBJ)/DocActionSort.o $(GTKCFLAGS)

$(OBJ)/DocActionPix.o: $(GUIPATH)/DocActionPix.cpp $(GUIPATH)/DocActionPix.h
	$(CC) $(FLAGS) -c $(GUIPATH)/DocActionPix.cpp -o $(OBJ)/DocActionPix.o $(GTKCFLAGS)

$(OBJ)/DocActionFinish.o: $(GUIPATH)/DocActionFinish.cpp $(GUIPATH)/DocActionFinish.h
	$(CC) $(FLAGS) -c $(GUIPATH)/DocActionFinish.cpp -o $(OBJ)/DocActionFinish.o $(GTKCFLAGS)

$(OBJ)/FindDialog.o: $(GUIPATH)/FindDialog.cpp
	$(CC) $(FLAGS) -c $(GUIPATH)/FindDialog.cpp -o $(OBJ)/FindDialog.o $(GTKCFLAGS)

$(OBJ)/FindReplaceDlg.o: $(GUIPATH)/FindReplaceDlg.cpp
	$(CC) $(FLAGS) -c $(GUIPATH)/FindReplaceDlg.cpp -o $(OBJ)/FindReplaceDlg.o $(GTKCFLAGS)

$(OBJ)/FindReplaceInfo.o: $(GUIPATH)/FindReplaceInfo.cpp
	$(CC) $(FLAGS) -c $(GUIPATH)/FindReplaceInfo.cpp -o $(OBJ)/FindReplaceInfo.o $(GTKCFLAGS)

$(OBJ)/OptionsDialog.o: $(GUIPATH)/OptionsDialog.cpp
	$(CC) $(FLAGS) -c $(GUIPATH)/OptionsDialog.cpp -o $(OBJ)/OptionsDialog.o $(GTKCFLAGS)

$(OBJ)/EnumDirectory.o: $(GUIPATH)/lib/EnumDirectory.cpp
	$(CC) $(FLAGS) -c $(GUIPATH)/lib/EnumDirectory.cpp -o $(OBJ)/EnumDirectory.o $(GTKCFLAGS)

$(OBJ)/GuiLanguage.o: $(GUIPATH)/gui/GuiLanguage.cpp
	$(CC) $(FLAGS) -c $(GUIPATH)/gui/GuiLanguage.cpp -o $(OBJ)/GuiLanguage.o $(GTKCFLAGS)

$(OBJ)/ProgressDlg.o: $(GUIPATH)/gui/ProgressDlg.cpp
	$(CC) $(FLAGS) -c $(GUIPATH)/gui/ProgressDlg.cpp -o $(OBJ)/ProgressDlg.o $(GTKCFLAGS)

$(OBJ)/TextSearch.o: $(GUIPATH)/lib/TextSearch.cpp
	$(CC) $(FLAGS) -c $(GUIPATH)/lib/TextSearch.cpp -o $(OBJ)/TextSearch.o $(GTKCFLAGS)

$(OBJ)/Dialog.o: $(GUIPATH)/gui/Dialog.cpp
	$(CC) $(FLAGS) -c $(GUIPATH)/gui/Dialog.cpp -o $(OBJ)/Dialog.o $(GTKCFLAGS)

$(OBJ)/PasswordDialog.o: $(GUIPATH)/PasswordDialog.cpp
	$(CC) $(FLAGS) -c $(GUIPATH)/PasswordDialog.cpp -o $(OBJ)/PasswordDialog.o $(GTKCFLAGS)

$(OBJ)/FileDialog.o: $(GUIPATH)/gui/FileDialog.cpp
	$(CC) $(FLAGS) -c $(GUIPATH)/gui/FileDialog.cpp -o $(OBJ)/FileDialog.o $(GTKCFLAGS)

$(OBJ)/main.o: $(GUIPATH)/main.cpp
	$(CC) $(FLAGS) -c $(GUIPATH)/main.cpp -o $(OBJ)/main.o $(GTKCFLAGS)

$(OBJ)/callbacks.o: $(GUIPATH)/callbacks.cpp $(GUIPATH)/callbacks.h
	$(CC) $(FLAGS) -c $(GUIPATH)/callbacks.cpp -o $(OBJ)/callbacks.o $(GTKCFLAGS)

$(OBJ)/interface.o: $(GUIPATH)/interface.cpp $(GUIPATH)/interface.h
	$(CC) $(FLAGS) -c $(GUIPATH)/interface.cpp -o $(OBJ)/interface.o $(GTKCFLAGS)

$(OBJ)/support.o: $(GUIPATH)/support.cpp $(GUIPATH)/support.h
	$(CC) $(FLAGS) -c $(GUIPATH)/support.cpp -o $(OBJ)/support.o $(GTKCFLAGS)

$(OBJ)/mru.o: $(GUIPATH)/mru.cpp $(GUIPATH)/mru.h
	$(CC) $(FLAGS) -c $(GUIPATH)/mru.cpp -o $(OBJ)/mru.o $(GTKCFLAGS)
		
$(OBJ)/IniFile.o: $(SRCPATH)/IniFile.cpp $(SRCPATH)/IniFile.h
	$(CC) $(FLAGS) -c $(SRCPATH)/IniFile.cpp -o $(OBJ)/IniFile.o
	
$(OBJ)/blowfish.o: $(SRCPATH)/blowfish.cpp $(SRCPATH)/blowfish.h
	$(CC) $(FLAGS) -c $(SRCPATH)/blowfish.cpp -o $(OBJ)/blowfish.o

$(OBJ)/DocumentIterator.o: $(SRCPATH)/DocumentIterator.cpp $(SRCPATH)/DocumentIterator.h
	$(CC) $(FLAGS) -c $(SRCPATH)/DocumentIterator.cpp -o $(OBJ)/DocumentIterator.o  $(GTKCFLAGS)
  
$(OBJ)/FilePath.o: $(SRCPATH)/FilePath.cpp $(SRCPATH)/FilePath.h
	$(CC) $(FLAGS) -c $(SRCPATH)/FilePath.cpp -o $(OBJ)/FilePath.o $(GTKCFLAGS)
	
$(OBJ)/IOLayerEnc.o: $(SRCPATH)/IOLayerEnc.cpp $(SRCPATH)/IOLayerEnc.h
	$(CC) $(FLAGS) -c $(SRCPATH)/IOLayerEnc.cpp -o $(OBJ)/IOLayerEnc.o
	
$(OBJ)/File64.o: $(SRCPATH)/File64.cpp $(SRCPATH)/File64.h
	$(CC) $(FLAGS) -c $(SRCPATH)/File64.cpp -o $(OBJ)/File64.o

$(OBJ)/FormatIOGjots2.o: $(SRCPATH)/FormatIOGjots2.cpp $(SRCPATH)/FormatIOGjots2.h
	$(CC) $(FLAGS) -c $(SRCPATH)/FormatIOGjots2.cpp -o $(OBJ)/FormatIOGjots2.o $(GTKCFLAGS)
	 
$(OBJ)/FormatIOEncHtml.o: $(SRCPATH)/FormatIOEncHtml.cpp $(SRCPATH)/FormatIOEncHtml.h
	$(CC) $(FLAGS) -c $(SRCPATH)/FormatIOEncHtml.cpp -o $(OBJ)/FormatIOEncHtml.o $(GTKCFLAGS)
  
$(OBJ)/FormatIOHtml.o: $(SRCPATH)/FormatIOHtml.cpp $(SRCPATH)/FormatIOHtml.h
	$(CC) $(FLAGS) -c $(SRCPATH)/FormatIOHtml.cpp -o $(OBJ)/FormatIOHtml.o $(GTKCFLAGS)
 
$(OBJ)/HtmlParser.o: $(SRCPATH)/HtmlParser.cpp $(SRCPATH)/HtmlParser.h
	$(CC) $(FLAGS) -c $(SRCPATH)/HtmlParser.cpp -o $(OBJ)/HtmlParser.o $(GTKCFLAGS)
  
$(OBJ)/NoteDocument.o: $(SRCPATH)/NoteDocument.cpp $(SRCPATH)/NoteDocument.h
	$(CC) $(FLAGS) -c $(SRCPATH)/NoteDocument.cpp -o $(OBJ)/NoteDocument.o $(GTKCFLAGS)
  
$(OBJ)/FormatIOBase.o: $(SRCPATH)/FormatIOBase.cpp $(SRCPATH)/FormatIOBase.h
	$(CC) $(FLAGS) -c $(SRCPATH)/FormatIOBase.cpp -o $(OBJ)/FormatIOBase.o $(GTKCFLAGS)
  
$(OBJ)/NoteNode.o: $(SRCPATH)/NoteNode.cpp $(SRCPATH)/NoteNode.h
	$(CC) $(FLAGS) -c $(SRCPATH)/NoteNode.cpp -o $(OBJ)/NoteNode.o $(GTKCFLAGS)
  
$(OBJ)/SHA1.o: $(SRCPATH)/SHA1.cpp $(SRCPATH)/SHA1.h
	$(CC) $(FLAGS) -c $(SRCPATH)/SHA1.cpp -o $(OBJ)/SHA1.o

$(OBJ)/eggtrayicon.o: $(GUIPATH)/_unx/eggtrayicon.cpp
	$(CC) $(FLAGS) -c $(GUIPATH)/_unx/eggtrayicon.cpp -o $(OBJ)/eggtrayicon.o $(GTKCFLAGS)

$(OBJ)/TrayIcon.o: $(GUIPATH)/_win/TrayIcon.cpp
	$(CC) $(FLAGS) -c $(GUIPATH)/_win/TrayIcon.cpp -o $(OBJ)/TrayIcon.o $(GTKCFLAGS)

$(OBJ)/PortableTrayIcon.o: $(GUIPATH)/PortableTrayIcon.cpp
	$(CC) $(FLAGS) -c $(GUIPATH)/PortableTrayIcon.cpp -o $(OBJ)/PortableTrayIcon.o $(GTKCFLAGS)

$(OBJ)/Base64.o: $(SRCPATH)/Base64.cpp
	$(CC) $(FLAGS) -c $(SRCPATH)/Base64.cpp -o $(OBJ)/Base64.o $(GTKCFLAGS)

ifdef WINDIR

$(OBJ)/SingleInstance.o: $(GUIPATH)/_win/SingleInstanceWin.cpp $(GUIPATH)/_win/SingleInstance.h
	$(CC) $(FLAGS) -c $(GUIPATH)/_win/SingleInstanceWin.cpp -o $(OBJ)/SingleInstance.o
	
else

$(OBJ)/SingleInstance.o: $(GUIPATH)/_unx/SingleInstance.cpp $(GUIPATH)/_unx/SingleInstance.h
	$(CC) $(FLAGS) -c $(GUIPATH)/_unx/SingleInstance.cpp -o $(OBJ)/SingleInstance.o  $(GTKCFLAGS)

endif

$(OBJ)/TreeView.o: $(GUIPATH)/TreeView.cpp $(GUIPATH)/TreeView.h
	$(CC) $(FLAGS) -c $(GUIPATH)/TreeView.cpp -o $(OBJ)/TreeView.o  $(GTKCFLAGS)

$(OBJ)/TextView.o: $(GUIPATH)/TextView.cpp $(GUIPATH)/TextView.h
	$(CC) $(FLAGS) -c $(GUIPATH)/TextView.cpp -o $(OBJ)/TextView.o  $(GTKCFLAGS)

$(OBJ)/MainWnd.o: $(GUIPATH)/MainWnd.cpp $(GUIPATH)/MainWnd.h
	$(CC) $(FLAGS) -c $(GUIPATH)/MainWnd.cpp -o $(OBJ)/MainWnd.o  $(GTKCFLAGS)
 
$(OBJ)/FormatIOStickyNotes.o: $(SRCPATH)/FormatIOStickyNotes.cpp $(SRCPATH)/FormatIOStickyNotes.h
	$(CC) $(FLAGS) -c $(SRCPATH)/FormatIOStickyNotes.cpp -o $(OBJ)/FormatIOStickyNotes.o  $(GTKCFLAGS)

$(OBJ)/NodePropertiesDlg.o: $(GUIPATH)/NodePropertiesDlg.cpp $(GUIPATH)/NodePropertiesDlg.h
	$(CC) $(FLAGS) -c $(GUIPATH)/NodePropertiesDlg.cpp -o $(OBJ)/NodePropertiesDlg.o  $(GTKCFLAGS)

$(OBJ)/debug.o: $(SRCPATH)/debug.cpp $(SRCPATH)/debug.h
	$(CC) $(FLAGS) -c $(SRCPATH)/debug.cpp -o $(OBJ)/debug.o $(GTKCFLAGS)

$(OBJ)/IOProcess.o: $(SRCPATH)/IOProcess.cpp $(SRCPATH)/IOProcess.h
	$(CC) $(FLAGS) -c $(SRCPATH)/IOProcess.cpp -o $(OBJ)/IOProcess.o $(GTKCFLAGS)

$(OBJ)/IOLayerZlib.o: $(SRCPATH)/IOLayerZlib.cpp $(SRCPATH)/IOLayerZlib.h
	$(CC) $(FLAGS) -c $(SRCPATH)/IOLayerZlib.cpp -o $(OBJ)/IOLayerZlib.o $(GTKCFLAGS)

$(OBJ)/IOLayerFile64.o: $(SRCPATH)/IOLayerFile64.cpp $(SRCPATH)/IOLayerFile64.h
	$(CC) $(FLAGS) -c $(SRCPATH)/IOLayerFile64.cpp -o $(OBJ)/IOLayerFile64.o $(GTKCFLAGS)

$(OBJ)/IOLayerBase.o: $(SRCPATH)/IOLayerBase.cpp $(SRCPATH)/IOLayerBase.h
	$(CC) $(FLAGS) -c $(SRCPATH)/IOLayerBase.cpp -o $(OBJ)/IOLayerBase.o $(GTKCFLAGS)

$(OBJ)/FileExportDlg.o: $(GUIPATH)/FileExportDlg.cpp
	$(CC) $(FLAGS) -c $(GUIPATH)/FileExportDlg.cpp -o $(OBJ)/FileExportDlg.o $(GTKCFLAGS)

$(OBJ)/IOLayerRedirect.o: $(SRCPATH)/IOLayerRedirect.cpp $(SRCPATH)/IOLayerRedirect.h
	$(CC) $(FLAGS) -c $(SRCPATH)/IOLayerRedirect.cpp -o $(OBJ)/IOLayerRedirect.o $(GTKCFLAGS)

$(OBJ)/FormatIOTxt.o: $(SRCPATH)/FormatIOTxt.cpp $(SRCPATH)/FormatIOTxt.h
	$(CC) $(FLAGS) -c $(SRCPATH)/FormatIOTxt.cpp -o $(OBJ)/FormatIOTxt.o  $(GTKCFLAGS)

$(OBJ)/FormatIOExecutable.o: $(SRCPATH)/FormatIOExecutable.cpp $(SRCPATH)/FormatIOExecutable.h
	$(CC) $(FLAGS) -c $(SRCPATH)/FormatIOExecutable.cpp -o $(OBJ)/FormatIOExecutable.o  $(GTKCFLAGS)

$(OBJ)/LinkInfo.o: $(SRCPATH)/LinkInfo.cpp $(SRCPATH)/LinkInfo.h
	$(CC) $(FLAGS) -c $(SRCPATH)/LinkInfo.cpp -o $(OBJ)/LinkInfo.o  $(GTKCFLAGS)

$(OBJ)/LinkPropertiesDlg.o: $(GUIPATH)/LinkPropertiesDlg.cpp $(GUIPATH)/LinkPropertiesDlg.h
	$(CC) $(FLAGS) -c $(GUIPATH)/LinkPropertiesDlg.cpp -o $(OBJ)/LinkPropertiesDlg.o $(GTKCFLAGS)

$(OBJ)/ExecuteFile.o: $(GUIPATH)/_unx/ExecuteFile.cpp $(GUIPATH)/ExecuteFile.h
	$(CC) $(FLAGS) -c $(GUIPATH)/_unx/ExecuteFile.cpp -o $(OBJ)/ExecuteFile.o $(GTKCFLAGS)

$(OBJ)/FileSaveAsDlg.o: $(GUIPATH)/FileSaveAsDlg.cpp
	$(CC) $(FLAGS) -c $(GUIPATH)/FileSaveAsDlg.cpp -o $(OBJ)/FileSaveAsDlg.o $(GTKCFLAGS)

$(OBJ)/bootstart.o: $(GUIPATH)/_win/bootstart.cpp $(GUIPATH)/_win/bootstart.h
	$(CC) $(FLAGS) -c $(GUIPATH)/_win/bootstart.cpp -o $(OBJ)/bootstart.o

$(OBJ)/RegisterExtension.o: $(GUIPATH)/_win/RegisterExtension.cpp $(GUIPATH)/_win/RegisterExtension.h
	$(CC) $(FLAGS) -c $(GUIPATH)/_win/RegisterExtension.cpp -o $(OBJ)/RegisterExtension.o

$(OBJ)/CircularBuffer.o: $(SRCPATH)/CircularBuffer.cpp $(SRCPATH)/CircularBuffer.h
	$(CC) $(FLAGS) -c $(SRCPATH)/CircularBuffer.cpp -o $(OBJ)/CircularBuffer.o

$(OBJ)/FormatIOMMLX.o: $(SRCPATH)/FormatIOMMLX.cpp $(SRCPATH)/FormatIOMMLX.h
	$(CC) $(FLAGS) -c $(SRCPATH)/FormatIOMMLX.cpp -o $(OBJ)/FormatIOMMLX.o  $(GTKCFLAGS)

$(OBJ)/DocActionFmt.o: $(GUIPATH)/DocActionFmt.cpp $(GUIPATH)/DocActionFmt.h
	$(CC) $(FLAGS) -c $(GUIPATH)/DocActionFmt.cpp -o $(OBJ)/DocActionFmt.o $(GTKCFLAGS)

$(OBJ)/DocActionAtt.o: $(GUIPATH)/DocActionAtt.cpp $(GUIPATH)/DocActionAtt.h
	$(CC) $(FLAGS) -c $(GUIPATH)/DocActionAtt.cpp -o $(OBJ)/DocActionAtt.o $(GTKCFLAGS)

$(OBJ)/FmtInfo.o: $(SRCPATH)/FmtInfo.cpp $(SRCPATH)/FmtInfo.h
	$(CC) $(FLAGS) -c $(SRCPATH)/FmtInfo.cpp -o $(OBJ)/FmtInfo.o $(GTKCFLAGS)

$(OBJ)/FileAttachmentDlg.o: $(GUIPATH)/FileAttachmentDlg.cpp $(GUIPATH)/FileAttachmentDlg.h
	$(CC) $(FLAGS) -c $(GUIPATH)/FileAttachmentDlg.cpp -o $(OBJ)/FileAttachmentDlg.o $(GTKCFLAGS)

$(OBJ)/PixPropertiesDlg.o: $(GUIPATH)/PixPropertiesDlg.cpp $(GUIPATH)/PixPropertiesDlg.h
	$(CC) $(FLAGS) -c $(GUIPATH)/PixPropertiesDlg.cpp -o $(OBJ)/PixPropertiesDlg.o  $(GTKCFLAGS)

$(OBJ)/DocActionFinishDel.o: $(GUIPATH)/DocActionFinishDel.cpp $(GUIPATH)/DocActionFinishDel.h
	$(CC) $(FLAGS) -c $(GUIPATH)/DocActionFinishDel.cpp -o $(OBJ)/DocActionFinishDel.o $(GTKCFLAGS)

$(OBJ)/ShortcutsList.o: $(GUIPATH)/ShortcutsList.cpp $(GUIPATH)/ShortcutsList.h
	$(CC) $(FLAGS) -c $(GUIPATH)/ShortcutsList.cpp -o $(OBJ)/ShortcutsList.o  $(GTKCFLAGS)

$(OBJ)/ShortcutsListDlg.o: $(GUIPATH)/ShortcutsListDlg.cpp $(GUIPATH)/ShortcutsListDlg.h
	$(CC) $(FLAGS) -c $(GUIPATH)/ShortcutsListDlg.cpp -o $(OBJ)/ShortcutsListDlg.o  $(GTKCFLAGS)

$(OBJ)/DateTimeDlg.o: $(GUIPATH)/DateTimeDlg.cpp $(GUIPATH)/DateTimeDlg.h
	$(CC) $(FLAGS) -c $(GUIPATH)/DateTimeDlg.cpp -o $(OBJ)/DateTimeDlg.o  $(GTKCFLAGS)

$(OBJ)/AboutDlg.o: $(GUIPATH)/AboutDlg.cpp $(GUIPATH)/AboutDlg.h
	$(CC) $(FLAGS) -c $(GUIPATH)/AboutDlg.cpp -o $(OBJ)/AboutDlg.o  $(GTKCFLAGS)

$(OBJ)/Thread.o: $(SRCPATH)/Thread.cpp $(SRCPATH)/Thread.h
	$(CC) $(FLAGS) -c $(SRCPATH)/Thread.cpp -o $(OBJ)/Thread.o $(GTKCFLAGS)

# compile Windows resources
$(OBJ)/resources.o: ./res/notecase.rc
	windres --include-dir ./res -o $(OBJ)/resources.o ./res/notecase.rc

$(OBJ)/EditDlg.o: $(GUIPATH)/EditDlg.cpp $(GUIPATH)/EditDlg.h
	$(CC) $(FLAGS) -c $(GUIPATH)/EditDlg.cpp -o $(OBJ)/EditDlg.o  $(GTKCFLAGS)

#
# help printout
#
help:
	@echo "Usage:"
	@echo "make             - builds application binaries"
	@echo "make clean       - delete built application and object files"
	@echo "make install     - install binaries into the official directories"
	@echo "make uninstall   - uninstall binaries from the official directories"
	@echo "make rpm         - create source and binary rpms from this source"
	@echo "make deb         - create a deb package from source"
	@echo "make help        - prints this help"
	@echo "make poinstall   - create and install translation catalogs (.mo)"
	@echo "make pouninstall - uninstall translation catalogs (.mo)"
	@echo "make convertsrc  - convert source code files to DOS text format"
	@echo ""
	@echo "To create rpm as non-root user use this form (override path to your rpm build directory):"
	@echo "  make rpm topdir=~/rpm"

#
# install/uninstall application translations
# (dynamically listing available .po files inside ./po directory)
#

poinstall:
	@for i in `ls po/*.po`; do \
		loc=`basename $$i | cut -f1 -d.`; \
		rm -f po/messages.mo; \
		msgfmt -o po/messages.mo $$i; \
		echo "Installing: $(DESTDIR)$(datadir)/locale/$$loc/LC_MESSAGES/notecase.mo";\
		if [ "$(OSX_BUILD)" = "1" ] || [ "$(FREEBSD_BUILD)" = "1" ] || [ "$(SOLARIS_BUILD)" = "1" ]; then \
			mkdir -p $(DESTDIR)$(datadir)/locale/$$loc/LC_MESSAGES/; \
		fi; \
		$(INSTALL) -m 644 po/messages.mo "$(DESTDIR)$(datadir)/locale/$$loc/LC_MESSAGES/notecase.mo"; \
	done; \
	rm -f po/messages.mo; 
	@echo done installing translations

pouninstall:
	@rm -vf $(DESTDIR)$(datadir)/locale/*/LC_MESSAGES/notecase.mo
	@echo done uninstalling translations
	
#
# install application
#
install: $(BIN)/notecase$(EXE) poinstall
	install -d -m 755 $(DESTDIR)$(bindir)
	install -d -m 755 $(DESTDIR)$(datadir)/doc/notecase/
	install -d -m 755 $(DESTDIR)$(datadir)/applications/
	install -d -m 755 $(DESTDIR)$(datadir)/mime/packages/
	install -d -m 755 $(DESTDIR)$(datadir)/pixmaps/
	$(INSTALL) -m 755 $(BIN)/notecase$(EXE) "$(DESTDIR)$(bindir)/notecase$(EXE)"
	$(INSTALL) -m 644 docs/help.ncd "$(DESTDIR)$(docdir)/notecase/help.ncd"
	$(INSTALL) -m 644 docs/notecase.xml "$(DESTDIR)$(datadir)/mime/packages/notecase.xml"
	$(Q)if [ -n "$(NOKIA_MAEMO_BUILD)" ]; then \
		$(INSTALL) -m 644 docs/notecase.desktop "$(DESTDIR)$(datadir)/applications/hildon/notecase.desktop"; \
		$(INSTALL) -m 644 docs/notecase.service "$(DESTDIR)$(datadir)/dbus-1/services/notecase.service"; \
		$(INSTALL) -m 644 docs/notecase.png "$(DESTDIR)$(datadir)/icons/hicolor/26x26/hildon/notecase.png"; \
		$(INSTALL) -m 644 docs/notecase.png "$(DESTDIR)$(datadir)/pixmaps/notecase.png"; \
	else \
		$(INSTALL) -m 644 docs/notecase.desktop "$(DESTDIR)$(datadir)/applications/notecase.desktop"; \
	fi;
	$(INSTALL) -m 644 res/notecase.xpm "$(DESTDIR)$(datadir)/pixmaps/notecase.xpm"
	@echo done installing

#
# uninstall application
#
uninstall:	pouninstall
	$(Q)if [ -n "$(NOKIA_MAEMO_BUILD)"]; then \
		rm -f  "$(DESTDIR)$(datadir)/applications/hildon/notecase.desktop" \
		rm -f  "$(DESTDIR)$(datadir)/dbus-1/services/notecase.service"; \
		rm -f  "$(DESTDIR)$(datadir)/icons/hicolor/26x26/hildon/notecase.png"; \
		rm -f  "$(DESTDIR)$(datadir)/pixmaps/notecase.png"; \
	else \
		rm -f  "$(DESTDIR)$(datadir)/applications/notecase.desktop"; \
	fi;
	rm -f  "$(DESTDIR)$(bindir)/notecase$(EXE)"
	rm -f  "$(DESTDIR)$(datadir)/pixmaps/notecase.xpm"
	rm -rf "$(DESTDIR)$(datadir)/doc/notecase"
	@echo done uninstalling

#
# install application and register MIME formats
#
install2: install updatemime


#
# build RPM and SRPM files (need not be superuser)
#
rpm:	pack
	cp -f docs/notecase.spec $(topdir)/SPECS/notecase-$(VERSION).spec
	mv -f ../$(archive)_src.tar.gz $(topdir)/SOURCES
	cp -f ./res/notecase.xpm $(topdir)/SOURCES
	cp -f ./docs/notecase.desktop $(topdir)/SOURCES
	rpmbuild --clean --define "_topdir $(topdir)" -ba "$(topdir)/SPECS/notecase-$(VERSION).spec"
	rm -f $(topdir)/SPECS/notecase-$(VERSION).spec
	rm -f $(topdir)/SOURCES/$(archive)_src.tar.gz
	rm -f $(topdir)/SOURCES/notecase.xpm
	rm -f $(topdir)/SOURCES/notecase.desktop
	mv -f $(topdir)/SRPMS/notecase-$(VERSION)*.src.rpm ../
	mv -f $(topdir)/RPMS/*/notecase-$(VERSION)*.rpm ../
	$(Q)if [ -x $(topdir)/RPMS/*/notecase-debuginfo-$(VERSION)*.rpm ]; then \
		mv -f $(topdir)/RPMS/*/notecase-debuginfo-$(VERSION)*.rpm ../; \
	fi;	
	@echo Done, look for your RPMs and SRPM under ../ 

#
# pack sources in parent directory (need not be superuser)
#
pack: clean updatesrc
	@cd ..; rm -f $(archive)_src.tar.gz; tar cfz $(archive)_src.tar.gz notecase-$(VERSION)
	@echo "Done, see archive ../$(archive)_src.tar.gz"

packdos: clean updatesrc convertsrcdos
	@cd ..; rm -f $(archive)_src.zip; zip -rq $(archive)_src.zip notecase-$(VERSION)
	@echo "Done, see archive ../$(archive)_src.zip"

#
# - update version strings from makefile value
#   1. automaticaly update version information inside .spec file
#   2. automaticaly update "Source:" label inside .spec file
#   3. automaticaly update version information inside "config.h" header file
#   4. write $(prefix) value into the config.h source file
#   5. automatically detect ZAURUS platform and update "config.h" definition
#   6. update version in the control file used to build Sharp Zaurus .ipk package
#   7. update version in the control file used to build Debian/Ubuntu .deb package
#
updatesrc:
	$(Q)if [ "$(LIBSCFLAGS)" = "" ]; then \
		echo 'pkg-config error detected $(shell pkg-config --cflags $(LIBS))';\
		exit 1;\
	fi;  
	$(Q)if [ "$(HAVE_GTKSOURCEVIEW)" = "1" ]; then \
		if [[ "$(TEST_SRCVIEW_VER)" < "2.4.1" ]]; then \
		echo 'ERROR: gtksourceview package is too old ($(shell pkg-config --modversion gtksourceview-2.0))!';\
		exit 1;\
		fi;\
	fi;
	$(Q)echo 'Configuration: GNOME_VFS=$(HAVE_GNOME_VFS);GTKSOURCEVIEW=$(HAVE_GTKSOURCEVIEW);DEBUG=$(DEBUG);PROFILE=$(PROFILE);BYTE_ORDER=$(BYTE_ORDER);PLATFORM=$(PLATFORM)' 
	$(Q)$(SED) 's/^Version:.*/Version:        $(VERSION)/' ./docs/notecase.spec
	$(Q)$(SED) 's/^Source:.*/Source:         $(archive)_src.tar.gz/' ./docs/notecase.spec
	$(Q)$(SED) 's/\#define APP_VER_STR .*/\#define APP_VER_STR "$(VERSION)"/' ./src/config.h
	$(Q)if [ "$(HAVE_GTKSOURCEVIEW)" = "" ]; then \
		$(SED) 's:[/]*#define HAVE_GTKSOURCEVIEW.*://#define HAVE_GTKSOURCEVIEW:' ./src/config.h; \
	else \
		$(SED) 's:[/]*#define HAVE_GTKSOURCEVIEW.*:#define HAVE_GTKSOURCEVIEW:' ./src/config.h; \
	fi;
	#$(Q)$(SED) 's:\#define INSTALL_PREFIX .*:\#define INSTALL_PREFIX "$(DESTDIR)$(prefix)":' ./src/config.h
	$(Q)if [ "$(HAVE_GNOME_VFS)" = "" ]; then \
		$(SED) 's:[/]*#define HAVE_GNOME_VFS.*://#define HAVE_GNOME_VFS:' ./src/config.h; \
	else \
		$(SED) 's:[/]*#define HAVE_GNOME_VFS.*:#define HAVE_GNOME_VFS:' ./src/config.h; \
	fi;
	$(Q)if [ "$(ZAURUS_BUILD)" = "" ]; then \
		$(SED) 's:[/]*#define _ZAURUS_BUILD.*://#define _ZAURUS_BUILD:' ./src/config.h; \
	else \
		$(SED) 's:[/]*#define _ZAURUS_BUILD.*:#define _ZAURUS_BUILD:' ./src/config.h; \
	fi;
	$(Q)if [ "$(OSX_BUILD)" = "" ]; then \
		$(SED) 's:[/]*#define _OSX_BUILD.*://#define _OSX_BUILD:' ./src/config.h; \
	else \
		$(SED) 's:[/]*#define _OSX_BUILD.*:#define _OSX_BUILD:' ./src/config.h; \
	fi;
	$(Q)if [ "$(NOKIA_MAEMO_BUILD)" = "" ]; then \
		$(SED) 's:[/]*#define _NOKIA_MAEMO.*://#define _NOKIA_MAEMO:' ./src/config.h; \
		$(SED) 's/^Section: .*/Section: editors/' ./deb/control; \
		$(SED) 's/^Icon=notecase.*/Icon=notecase/' ./docs/notecase.desktop; \
	else \
		$(SED) 's:[/]*#define _NOKIA_MAEMO.*:#define _NOKIA_MAEMO $(NOKIA_MAEMO_BUILD):' ./src/config.h; \
		$(SED) 's#^Section: .*#Section: maemo/editors#' ./deb/control; \
		if [ "$(NOKIA_MAEMO_BUILD)" = "1" ]; then \
			$(SED) 's/^Icon=notecase.*/Icon=notecase.xpm/' ./docs/notecase.desktop; \
		else \
			$(SED) 's/^Icon=notecase.*/Icon=notecase/' ./docs/notecase.desktop; \
		fi; \
	fi;
	$(Q)$(SED) 's:[/]*#define DOC_DIR_PATH.*:#define DOC_DIR_PATH "$(docdir)/notecase/":' ./src/config.h
	$(Q)$(SED) 's/^Version: .*/Version: $(VERSION)/' ./docs/control
	$(Q)$(SED) 's/^Version: .*/Version: $(VERSION)/' ./deb/control
	$(Q)$(SED) 's/^Architecture: .*/Architecture: $(ARCH)/' ./deb/control 

#
# - convert sources to DOS text format (this is multiplatform project)
#
convertsrcdos:
	@unix2dos -q -k $(SRCPATH)/*.cpp $(SRCPATH)/*.h $(GUIPATH)/*.cpp $(GUIPATH)/*.h $(GUIPATH)/gui/*.cpp $(GUIPATH)/gui/*.h

#
# - optional: convert sources to Unix text format (this is multiplatform project)
#
convertsrc:
	@dos2unix -q -k $(SRCPATH)/*.cpp $(SRCPATH)/*.h $(GUIPATH)/*.cpp $(GUIPATH)/*.h $(GUIPATH)/gui/*.cpp $(GUIPATH)/gui/*.h ./Makefile
	
#
# - update Linux mime/desktop system
#
updatemime:
	update-mime-database "$(datadir)/mime/"
	update-desktop-database

#
# debugging: build and start under valgrind
#
check: notecase$(EXE)
	$(Q)valgrind --leak-check=full --error-limit=no ./bin/notecase$(EXE)

#
# create Zaurus package (.ipk)
#	(more info at http://handhelds.org/moin/moin.cgi/Ipkg)
#
# Note: first "make" command forces generating correct help file path in "config.h"

ipk: updatesrc touch
	$(Q)newipk notecase
	$(Q)rm -f ./notecase/control/control
	$(Q)cp ./docs/control ./notecase/control/control
	$(Q)make
	$(Q)make DESTDIR=`pwd`/notecase/data install
	$(Q)strip `pwd`/notecase/data/usr/bin/notecase
	$(Q)makeipk notecase
	$(Q)mv notecase*.ipk ../
	$(Q)echo "Done building .ipk package, look for it at ../ folder"

#
# create Zaurus package (.ipk) for weeXpc 
#	(more info at http://www.hermocom.com/en/services/weexpc/)
#
# Diffs to standard ipk:
# - uses interrim directory IPK (throught the patched tools)
# - delete translations to save space
# - additional file installed (wasc script related)
# - modify .desktop file so that notecase is started through the wasc launcher script
#
# Note: first "make" command forces generating correct help file path in "config.h"

#uncomment the line below if you use patched version of new IPK 
#IPK=IPK/

weexpc: updatesrc touch
	$(Q)newipk notecase
	$(Q)rm -f ./$(IPK)notecase/control/control
	$(Q)cp ./docs/control ./$(IPK)notecase/control/control
	$(Q)mkdir -p ./$(IPK)notecase/data/usr/share/wasc/
	$(Q)cp ./docs/wasc ./$(IPK)notecase/data/usr/share/wasc/notecase
	$(Q)make
	$(Q)make DESTDIR=`pwd`/$(IPK)notecase/data install
	$(Q)strip `pwd`/$(IPK)notecase/data/usr/bin/notecase
	$(Q)rm -rf ./$(IPK)notecase/data/$(datadir)/locale/
	$(Q)sed -i 's/^Exec=.*/Exec=wasc "notecase"/' ./$(IPK)notecase/data/$(datadir)/applications/notecase.desktop
	$(Q)makeipk notecase
	$(Q)mv ./$(IPK)notecase*.ipk ../
	$(Q)echo "Done building weeXpc package, look for it at ../ folder"

#
# create Debian/Ubuntu package (.deb)
#
# Note: first "make" command forces generating correct help file path in "config.h"
#
DEBSIZE:=$(shell du -ks ./notecase/ | cut -f 1)

deb:
	$(Q)mkdir ./notecase
	$(Q)mkdir ./notecase/data
	$(Q)make
	$(Q)make DESTDIR=./notecase/data install
	$(Q)rm -rf ./notecase/data/DEBIAN
	$(Q)mkdir ./notecase/data/DEBIAN
	$(Q)$(INSTALL) -m 755 ./deb/* ./notecase/data/DEBIAN/
	$(Q)if [ -n "$(MD5SUM_EXISTS)" ]; then \
		$(MD5SUM) `find ./notecase/data/usr -type f` > ./notecase/data/DEBIAN/md5sums; \
	fi;
	$(SED) 's;^Installed-Size: .*;Installed-Size: $(DEBSIZE);' ./notecase/data/DEBIAN/control
	$(Q)dpkg -b ./notecase/data notecase
	$(Q)mv ./notecase/notecase*.deb ../
	$(Q)rm -rf ./notecase

#
# create tar.gz archive of a binary build
#
binarchive:
	$(Q)rm -rf ./notecase-$(VERSION)
	$(Q)mkdir ./notecase-$(VERSION)
	$(Q)$(MAKE)
	$(Q)$(MAKE) DESTDIR=./notecase-$(VERSION)/ install
	@cd notecase-$(VERSION); tar cfz $(archive).tar.gz usr; cp $(archive).tar.gz ../../$(archive).tar.gz; cd ..
	$(Q)rm -rf ./notecase-$(VERSION)
	@echo "Done, see archive ../$(archive).tar.gz" 

#
# create .dmg archive for Mac OSX
#
# Note: first "make" command forces generating correct help file path in "config.h"
#
dmg:
	$(Q)rm -f ../notecase-$(VERSION).dmg
	$(Q)rm -rf ./notecase
	$(Q)mkdir  ./notecase
	$(Q)make
	$(Q)make DESTDIR=./notecase/ install
	$(Q)/usr/bin/hdiutil create -fs HFS+ -srcfolder ./notecase -volname notecase-$(VERSION) ../notecase-$(VERSION).dmg

#
# this is a virtual target that builds all packages related to Mac OSX
#
mac: deb dmg


#
# this is a virtual target that builds all packages related to Sharp Zaurus
# (both builds have the same name, so I had to rename them)
#
zaurus: touch
	$(Q)make ipk
	$(Q)mv ../notecase*.ipk ../ipk_$(archive)_armv5tel.ipk
	$(Q)make weexpc
	$(Q)mv ../notecase*.ipk ../wee_$(archive)_armv5tel.ipk 
	 
#
# helper to adapt to date changes
#
touch:
	$(Q)touch ./Makefile ./src/* ./src/lib/* ./src/gui/* ./src/_unx/*

#
# clean up the source tree
#
clean:
	@echo cleaning up
	$(Q)rm -f $(OBJ)/*.o $(OBJ)/*.d $(OBJ)/*.a $(BIN)/core.* $(BIN)/notecase$(EXE) ./gmon.out
	$(Q)rm -rf ./notecase

.PHONY : all help poinstall pouninstall install uninstall rpm pack packdos updatesrcdos updatesrc updatemime clean ipk deb weexpc check install2
