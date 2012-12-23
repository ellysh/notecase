////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Window class for Options dialog
////////////////////////////////////////////////////////////////////////////

#ifndef OptionsDialog_H_
#define OptionsDialog_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "gui/Dialog.h"
#include <string>

class OptionsDialog : public Dialog
{
public:
	OptionsDialog();
	virtual ~OptionsDialog();

	virtual void Create();

	void OnDialogInit();
	void OnDialogOK();

	int  GetIconType();
	void UpdatePreview();
	const char *GetIconValue();

	void SetIconType(int nType);
	void SetIconValue(const char *szValue);

protected:
	GtkWidget* create_options_dialog (GtkWidget* parent=NULL);
	std::string m_strLocale;
};

#endif // OptionsDialog_H_
