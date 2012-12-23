////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: this header defines paths for some required files
////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_H__
#define CONFIG_H__

#define APP_NAME_STR "NoteCase"
#define APP_VER_STR "1.9.8"

#define PACKAGE "notecase"	//NLS package definition
#define UNDO_LIST_SIZE 10
#define GENERATE_CRASHLOG	// comment this to suppress crash log generation
#define MRU_LIST_SIZE  10	// maximal size for recent documents list

//#define HAVE_GTKSOURCEVIEW
#ifdef HAVE_GTKSOURCEVIEW
 #include <gtksourceview/gtksourceview.h>
 #include <gtksourceview/gtksourcebuffer.h>
 GtkSourceBuffer *gtk_source_view_get_buffer(GtkSourceView *view);

 #define GTK_TEXTVIEW_CLASS		GtkSourceView
 #define GTK_TEXTBUFFER_CLASS	GtkSourceBuffer
 #define GTK_TEXT_GET_BUFFER	gtk_source_view_get_buffer
 #define GTK_TEXTBUFFER_CLASS_NEW gtk_source_buffer_new	
 #define GTK_TEXTVIEW_CLASS_NEW	gtk_source_view_new_with_buffer
#else
 #define GTK_TEXTVIEW_CLASS		GtkTextView
 #define GTK_TEXTBUFFER_CLASS	GtkTextBuffer
 #define GTK_TEXT_GET_BUFFER	gtk_text_view_get_buffer
 #define GTK_TEXTBUFFER_CLASS_NEW gtk_text_buffer_new	
 #define GTK_TEXTVIEW_CLASS_NEW	gtk_text_view_new_with_buffer
#endif

#ifndef _WIN32
//#define _ZAURUS_BUILD
//#define _OSX_BUILD
//#define _NOKIA_MAEMO

#define INSTALL_PREFIX "/usr"
//#define HAVE_GNOME_VFS
#endif

#ifdef _WIN32
	#define HELP_FILE_PATH "" 	//calculated dynamically
#else
	#define DOC_DIR_PATH "/usr/share/doc/notecase/"
	#define HELP_FILE_PATH "help.ncd"	//relative to DOC_DIR_PATH
#endif

#ifdef _WIN32
	#define LOCALE_DIR "" 	//calculated dynamically
#else
	#define LOCALE_DIR "/share/locale/"	//relative to INSTALL_PREFIX
#endif

#ifndef __MINGW32__
 #define HAVE_SNPRINTF
#endif

#endif
