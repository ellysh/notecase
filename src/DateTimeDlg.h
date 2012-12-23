////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Window class for Date/Time insert dialog
////////////////////////////////////////////////////////////////////////////

#ifndef DateTimeDlg_H_
#define DateTimeDlg_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "gui/Dialog.h"
#include <string>

class DateTimeDlg : public Dialog
{
public:
	DateTimeDlg();
	virtual ~DateTimeDlg();
	virtual void Create();

	void OnFormatChanged();
	std::string GetValue(){ return m_strValue; }
	static std::string GetValueForFormat(int nDateTimeFmt);

protected:
	GtkWidget* create_date_insert_popup (GtkWidget* parent = NULL);

protected:
	std::string m_strValue;
};

#endif // ExportDialog_H_
