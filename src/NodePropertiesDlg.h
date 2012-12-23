////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Dialog to view/edit document node properties
////////////////////////////////////////////////////////////////////////////

#ifndef NODEPROPERTIESDLG_H__
#define NODEPROPERTIESDLG_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "config.h"
#include "gui/Dialog.h"
#include <string>

#define ICON_NONE    -2
#define ICON_CUSTOM  -1
#define ICON_INTERNAL 0
#define ICON_INTERNAL_FIRST 0
#define ICON_INTERNAL_LAST	5

class NodePropertiesDlg : public Dialog
{
public:
	NodePropertiesDlg();
	virtual ~NodePropertiesDlg();
	virtual void Create();

	std::string GetNodeTitle();

	#ifdef HAVE_GTKSOURCEVIEW
		std::string GetSourceLanguage();
	#endif

	int GetIconType();
	void SetIconType(int nType);

	const char *GetIconValue();
	void SetIconValue(const char *szValue);

	void UpdatePreview();

	void KeywordAdd();
	void KeywordRemove();
	void RefreshKeywords();

public:
	#ifdef HAVE_GTKSOURCEVIEW
	std::string m_strSourceLanguage;
	#endif	
	std::string m_strTitle;
	std::string m_strKeywords;
	time_t	m_nCreated;
	time_t	m_nModified;
	bool	m_bFinished;

protected:
	GtkWidget* create_node_properties_dialog ();
};

#endif // NODEPROPERTIESDLG_H__
