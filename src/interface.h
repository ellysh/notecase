////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements interface creation (main window, password dialog, options dialog, ...)
////////////////////////////////////////////////////////////////////////////

#ifndef _INTERFACE_H__
#define _INTERFACE_H__

#include <gtk/gtk.h>

int  NodeIdxFromPath(GtkTreePath* path1);
bool PathFromNodeIdx(int nIdx, GtkTreePath *&path1);
bool IteratorFromNodeIdx(int nIdx, GtkTreeIter &iter);
void set_title_bar(const char *szText, bool bRed = false);
void set_status_bar(const char *szText, bool bRed = false);

int gtkMessageBox(const char *szText, int nButtons = GTK_BUTTONS_OK, int nIcon = GTK_MESSAGE_INFO);
int gtkMessageBoxYNC(const char *szText);
const char **InternalIcon_GetFromIdx(int nIdx);
int InternalIcon_Name2Index(const char *szName);
const char *GetLoadErrorString(int nErrCode);
const char *GetSaveErrorString(int nErrCode);
void ShowBusyCursor();
void HideBusyCursor();

unsigned int GetCurrentPID();
bool IsPIDRunning(unsigned int nPID);


// main.cpp
const char *calculate_autosave_filename();
const char *calculate_autosave_filename1();
const char *get_autosave_dir();
std::string GetHelpFile();
const char *GetIniFile();

#endif
