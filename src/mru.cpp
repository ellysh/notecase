////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements "Most Recently Used" menu list
////////////////////////////////////////////////////////////////////////////

#include "mru.h"
#include "lib/IniFile.h"
#include "lib/FilePath.h"
#include <gtk/gtk.h>
#include "support.h"
#include "interface.h"
#include "config.h"

#ifdef _WIN32
 #ifndef __MINGW32__
   #ifndef snprintf
    #define snprintf _snprintf
   #endif
 #endif
#endif

extern GtkWidget *g_menuitem5;
extern IniFile g_objIni;

void on_MRU_activate (GtkMenuItem *menuitem, gpointer user_data);
bool LocaleToUtf8(const char *szTxt, std::string &strResult);

MRU::MRU()
{
}

MRU::~MRU()
{
}

int MRU::GetFileCount()
{
	return m_lstFiles.size();
}

void MRU::Clear()
{
	m_lstFiles.clear();

	Save();
	RebuildMenu();
}

void MRU::Load()
{
	m_lstFiles.clear();

	int nMRUCount = 0;
	if(g_objIni.GetValue("MRU", "Count", nMRUCount, 0))
	{
		//load only up to maximum number allowed
		if(nMRUCount > MRU_LIST_SIZE)
			nMRUCount = MRU_LIST_SIZE;

		std::string strKey, strValue;
		for(int i=0; i<nMRUCount; i++)
		{
			char szKey[30]="";
			snprintf(szKey, sizeof(szKey), "File%d", i+1);
			g_objIni.GetValue("MRU", szKey, strValue, "");

			int nIdx = FindEntry(strValue.c_str());	// additionally cleanup the list from duplicates
			if(nIdx < 0)
				m_lstFiles.push_back(strValue);
		}
	}
}

void MRU::Save()
{
	int nMRUCount = m_lstFiles.size();
	if(nMRUCount > MRU_LIST_SIZE)
		nMRUCount = MRU_LIST_SIZE;

	g_objIni.RemoveSection("MRU");
	g_objIni.SetValue("MRU", "Count", nMRUCount);

	std::string strKey;
	for(int i=0; i<nMRUCount; i++)
	{
		char szKey[30]="";
		snprintf(szKey, sizeof(szKey), "File%d", i+1);
		g_objIni.SetValue("MRU", szKey, m_lstFiles[i].c_str());
	}
}

void MRU::Change(const char *szFilename, bool bAdd)
{
	int nIdx = FindEntry(szFilename);
	//remove item from list
	if(nIdx >= 0)
		m_lstFiles.erase(m_lstFiles.begin()+nIdx);

	//add it again? (this way latest item goes to the top of the menu)
	if(bAdd){
		//keep the list maximal size
		if(m_lstFiles.size() > (unsigned int)MRU_LIST_SIZE)
			m_lstFiles.erase(m_lstFiles.begin()+m_lstFiles.size()-1);	//delete last entry

		m_lstFiles.insert(m_lstFiles.begin(), szFilename);
	}

	RebuildMenu();
}

void MRU::RebuildMenu()
{
	GtkWidget *mru_menuitem = g_menuitem5;

	//create and attach submenu
	GtkWidget *menuitem5;
	GtkWidget *menu5 = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (mru_menuitem), menu5);

	int i;
	int nMax = m_lstFiles.size();
	for(i=0; i<nMax; i++)
	{
		//append an item
		menuitem5 = gtk_menu_item_new_with_label (m_lstFiles[i].c_str());
		gtk_widget_show (menuitem5);
		gtk_container_add (GTK_CONTAINER (menu5), menuitem5);
		g_signal_connect (GTK_OBJECT (menuitem5),  "activate", G_CALLBACK (on_MRU_activate), (void *)i);
	}

	//append "Clear list" item
	if(nMax > 0)
	{
		menuitem5 = gtk_menu_item_new_with_label (_(" - Clear list - "));
		gtk_widget_show (menuitem5);
		gtk_container_add (GTK_CONTAINER (menu5), menuitem5);
		g_signal_connect (GTK_OBJECT (menuitem5),  "activate", G_CALLBACK (on_MRU_activate), (void *)i);
	}
	else
	{
		//append "( empty )" item
		menuitem5 = gtk_menu_item_new_with_label (_("( empty )"));
		gtk_widget_show (menuitem5);
		gtk_container_add (GTK_CONTAINER (menu5), menuitem5);
		gtk_widget_set_sensitive (menuitem5, FALSE);
	}
}

int MRU::FindEntry(const char* szFilename)
{
	//find the file in the list
	int nIdx = -1;
	for(unsigned int i=0; i<m_lstFiles.size(); i++)
	{
		if(0 == strcmp(m_lstFiles[i].c_str(), szFilename))
		{
			nIdx = (int)i;
			break;
		}
	}

	return nIdx;
}
