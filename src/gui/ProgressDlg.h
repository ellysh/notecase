////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements progress for operations of unknown length
////////////////////////////////////////////////////////////////////////////

#ifndef PROGRESSDLG_H__
#define PROGRESSDLG_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gtk/gtk.h>
#include <string>

class ProgressDlg  
{
public:
	ProgressDlg(int nWaitMs = 200, const char *szTitle = NULL, GtkWidget *pParent = NULL, bool bCancelBtn = false);
	virtual ~ProgressDlg();

	static void SetTotalSize(int nSize);
	static void SetCurrentSize(int nSize);
	static bool IsCanceled(){ return m_bCanceled; };

	void Close();
	void DoCreateWindow();
	void OnCancel();

	int m_nStartTimer;

protected:
	int m_nProgressTimer;
	GtkWidget *m_pDialog;
	GtkWidget *m_pParent;
	std::string m_strTitle;
	bool m_bCancelBtn;

public:
	static int m_nTotalSize;
	static int m_nCurrentSize;
	static bool m_bCanceled;
};

#endif // PROGRESSDLG_H__
