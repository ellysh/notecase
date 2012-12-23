////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Window class for Find dialog
////////////////////////////////////////////////////////////////////////////

#ifndef FindDialog_H_
#define FindDialog_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "gui/Dialog.h"

class FindDialog : public Dialog
{
public:
	FindDialog();
	virtual ~FindDialog();

	virtual void Create();
};

#endif // ExportDialog_H_
