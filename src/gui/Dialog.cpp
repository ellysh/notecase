////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements base class for GTK Dialog window (experimental)
////////////////////////////////////////////////////////////////////////////

#include "Dialog.h"

Dialog::Dialog()
{
	m_pDialog = NULL;
}

Dialog::~Dialog()
{
}

void Dialog::Create()
{
}

gint Dialog::ShowModal()
{
	return gtk_dialog_run(GTK_DIALOG(m_pDialog));
}

void Dialog::Destroy()
{
	if(m_pDialog)
		gtk_widget_destroy(m_pDialog);
	m_pDialog = NULL;
}

