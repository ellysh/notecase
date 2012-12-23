////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: GTK+ TextView wrapper class
////////////////////////////////////////////////////////////////////////////

#ifndef TEXTVIEW_H__
#define TEXTVIEW_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gtk/gtk.h>
#include <string>
#include "config.h"
 
class TextView
{
public:
	TextView();
	virtual ~TextView();
	void Create();

	void Clear();
	void SetEditable(bool bEditable);
	bool IsEditable();

	void DeleteText(int nOffset, int nLength);

	std::string GetText();
	std::string GetSelectedText();

	void SetText(const gchar *szText);
	void InsertText(int nOffset, const gchar *szText);
	void ReplaceText(int nStart, int nEnd, const char *szNewTxt);

#ifdef HAVE_GTKSOURCEVIEW
	void SetSourceLanguage(const gchar *szSourceLanguage);
#endif

	int  GetSelectionEnd();
	void GetSelectionBounds(int& cursor, int& selection);
	void RestoreSelectionBounds(unsigned int cursor, unsigned int selection);
	void SelectRange(int nStart, int nEnd);
	void EnsureVisible(int nOffset);
	int  GetScrollPos();
	void RestoreScrollPos(unsigned int nPos);

	void SetTextUnderlined(int nStart, int nEnd);
	void RemoveTextStyles(int nStart, int nEnd);

	void SetTabWidth(int nWidth);	//width in pixels
	int	 CalcTabWidthInPixels(int nNumChars);

	bool IsWrapped();
	bool GetModified();
	void SetModified(bool bModified);

	void ClipboardCut();
	void ClipboardCopy();
	void ClipboardPaste();
	void DeleteSelection();

	void SetFocus();

public:
	GtkWidget *m_pWidget;
};

#endif // TEXTVIEW_H__
