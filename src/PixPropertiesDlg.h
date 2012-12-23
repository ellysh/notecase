////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Dialog to prepare image for insertion into the document (resize + select format)
////////////////////////////////////////////////////////////////////////////

#ifndef PIXPROPERTIESDLG_H__
#define PIXPROPERTIESDLG_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "gui/Dialog.h"
#include <string>

class PixPropertiesDlg : public Dialog
{
public:
	PixPropertiesDlg();
	virtual ~PixPropertiesDlg();
	virtual void Create();

	void RecalcNewSize();

public:
	int		m_nOrigWidth;
	int		m_nOrigHeight;
	int		m_nNewWidth;
	int		m_nNewHeight;
	bool	m_bUsePNG;

protected:
	GtkWidget* create_node_properties_dialog ();
};

#endif // PixPropertiesDlg_H__
