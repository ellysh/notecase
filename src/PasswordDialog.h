////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Window class for Password input
////////////////////////////////////////////////////////////////////////////

#ifndef PASSWORDDIALOG_H_
#define PASSWORDDIALOG_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "gui/Dialog.h"
#include <string>

#define MODE_PASS_VERIFY	0	// type the password once (up to X unsuccessful tries)
#define MODE_PASS_NEW		1	// type the new password twice (two entry widgets) to eliminate typing errors
#define MODE_PASS_CHANGE	2	// type the old password once, new password twice (three entry widgets)

class PasswordDialog : public Dialog
{
public:
	PasswordDialog(int nMode = MODE_PASS_VERIFY, int nTries = 1);
	virtual ~PasswordDialog();

	void SetLabel(const char *szText);
	void Destroy();

	const char *GetPassword();

	virtual void Create();

protected:
	GtkWidget* create_password_dialog (GtkWidget* parent1 = NULL);

public:
	int  m_nDialogMode;		// see defines above
	int	 m_nNumTries;		// how much times can user fail entering pass
	int	 m_nEntryCount;
	std::string m_strOldPass;	// when changing password
};

#endif // PASSWORDDIALOG_H_
