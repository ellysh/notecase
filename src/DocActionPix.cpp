////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements Picture insert Action
////////////////////////////////////////////////////////////////////////////

#include "DocActionPix.h"
#include "callbacks.h"
#include "support.h"
#include "interface.h"
#include "lib/NoteDocument.h"
#include "lib/DocumentIterator.h"
#include "TextView.h"
#include "TreeView.h"
#include "gui/FileDialog.h"
#include "lib/debug.h"
#include <algorithm>
#include "PixPropertiesDlg.h"
#include "lib/FilePath.h"
#include "lib/IniFile.h"

#ifdef _WIN32
 #include <io.h>
 #define access _access
 #ifndef __MINGW32__
  #define strcasecmp stricmp
 #endif
#endif

extern NoteDocument g_doc;
extern GtkWidget *window1;
extern TextView g_text;
extern TreeView g_tree;
extern IniFile g_objIni;

int GetSelectedNodeIdx();

DocActionPix::DocActionPix()
{
	m_nNodeIdx	= -1;
	m_nOffset	= -1;
}

DocActionPix::~DocActionPix()
{
}

void DocActionPix::Exec(bool bInteractive)
{
	DocumentIterator it(*m_pDoc);

	if(bInteractive)
	{
		FileDialog dlg(true);
		dlg.SetTitle(_("Open picture"));

		//define file filters
		dlg.AddFilter(_("All supported image formats (*.png,*.jpg,*.gif)"), "*.png|*.jpg|*.gif");
		dlg.AddFilter(_("PNG format (*.png)"), "*.png");
		dlg.AddFilter(_("JPG format (*.jpg)"), "*.jpg");
		dlg.AddFilter(_("GIF format (*.gif)"), "*.gif");
		dlg.AddFilter(_("All files (*)"), "*");

		//set initial directory from INI (store last used)
		std::string strDefaultDir;
		std::string strDir;
		g_objIni.GetValue("Cache", "LastPictureDir", strDir, "");
		if(!strDir.empty() && 0 == access(strDir.c_str(), 00))
			strDefaultDir = strDir;
		else
			strDefaultDir = GetHomeDir();

		dlg.SetDirectory(strDefaultDir.c_str());

		if(dlg.DoModal())
		{
			const gchar *filename = dlg.GetFilename();
			strDefaultDir = dlg.GetDirectory();
			dlg.Close();

			//store last open directory
			g_objIni.SetValue("Cache", "LastPictureDir", strDefaultDir.c_str());
			g_objIni.Save();

			GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(filename, NULL);

			PixPropertiesDlg dlg1;
			dlg1.m_nOrigWidth  = gdk_pixbuf_get_width(pixbuf);
			dlg1.m_nOrigHeight = gdk_pixbuf_get_height(pixbuf);
			dlg1.m_bUsePNG = false;
			dlg1.Create();
			if(GTK_RESPONSE_OK != dlg1.ShowModal()){
				g_object_unref (G_OBJECT (pixbuf));
				return;
			}

			GdkPixbuf *destpix = gdk_pixbuf_scale_simple(pixbuf, dlg1.m_nNewWidth, dlg1.m_nNewHeight, GDK_INTERP_BILINEAR);
			g_object_unref (G_OBJECT (pixbuf));
			pixbuf = destpix;

			UpdateTextFromScreen();

			//search from last result selection (or start)
			int cursor = -1;
			int selection = -1;
			g_text.GetSelectionBounds(cursor, selection);
			if(selection >= 0)
				cursor = selection;

			//get the iterator
			GtkTextBuffer* buffer1 = gtk_text_view_get_buffer((GtkTextView *)g_text.m_pWidget);
			GtkTextIter cursIter;
			gtk_text_buffer_get_iter_at_offset(buffer1, &cursIter, cursor);

			gtk_text_buffer_insert_pixbuf(buffer1, &cursIter, pixbuf);

			//store base name for the picture (with changed extension if required)
			std::string strName = GetBaseName(filename);
			EnsureExtension(strName, dlg1.m_bUsePNG ? ".png" : ".jpg");

			PixInfo info;
			info.bUsePNG = dlg1.m_bUsePNG;
			info.nOffset = cursor;
			info.pixbuf  = pixbuf;
			info.strName = strName;
			g_doc.GetNodeByIdx(m_nNodeIdx).m_lstPictures.push_back(info);
			std::sort(g_doc.GetNodeByIdx(m_nNodeIdx).m_lstPictures.begin(), g_doc.GetNodeByIdx(m_nNodeIdx).m_lstPictures.end());

			m_nOffset = cursor;
			m_pixbuf  = pixbuf;
			m_bUsePNG = dlg1.m_bUsePNG;

			//increment offsets for all formatting marks
			int nSize = g_doc.GetNodeByIdx(m_nNodeIdx).m_lstTxtFmt.size();
			int i;
			for(i=0; i<nSize; i++){
				if(g_doc.GetNodeByIdx(m_nNodeIdx).m_lstTxtFmt[i].nOffset >= m_nOffset)
					g_doc.GetNodeByIdx(m_nNodeIdx).m_lstTxtFmt[i].nOffset ++;
			}

			//increment all the pictures after this one
			nSize = g_doc.GetNodeByIdx(m_nNodeIdx).m_lstPictures.size();
			for(i=0; i<nSize; i++){
				if(g_doc.GetNodeByIdx(m_nNodeIdx).m_lstPictures[i].nOffset > m_nOffset)
					g_doc.GetNodeByIdx(m_nNodeIdx).m_lstPictures[i].nOffset ++;
			}
		}
	}
	else
	{
		PixInfo info;
		info.bUsePNG = m_bUsePNG;
		info.nOffset = m_nOffset;
		info.pixbuf  = m_pixbuf;
		g_doc.GetNodeByIdx(m_nNodeIdx).m_lstPictures.push_back(info);

		//increment offsets for all formatting marks
		int nSize = g_doc.GetNodeByIdx(m_nNodeIdx).m_lstTxtFmt.size();
		int i;
		for(i=0; i<nSize; i++){
			if(g_doc.GetNodeByIdx(m_nNodeIdx).m_lstTxtFmt[i].nOffset >= m_nOffset)
				g_doc.GetNodeByIdx(m_nNodeIdx).m_lstTxtFmt[i].nOffset ++;
		}

		//increment all the pictures after this one
		nSize = g_doc.GetNodeByIdx(m_nNodeIdx).m_lstPictures.size();
		for(i=0; i<nSize; i++){
			if(g_doc.GetNodeByIdx(m_nNodeIdx).m_lstPictures[i].nOffset > m_nOffset)
				g_doc.GetNodeByIdx(m_nNodeIdx).m_lstPictures[i].nOffset ++;
		}

		if(m_nNodeIdx == GetSelectedNodeIdx()){
			//get the iterator
			GtkTextBuffer* buffer1 = gtk_text_view_get_buffer((GtkTextView *)g_text.m_pWidget);
			GtkTextIter cursIter;
			gtk_text_buffer_get_iter_at_offset(buffer1, &cursIter, m_nOffset);
			gtk_text_buffer_insert_pixbuf(buffer1, &cursIter, m_pixbuf);
		}
	}
}

void DocActionPix::Undo()
{
	//find and remove description
	int nPicCount = g_doc.GetNodeByIdx(m_nNodeIdx).m_lstPictures.size();
	for(int i=0; i<nPicCount; i++){
		if(g_doc.GetNodeByIdx(m_nNodeIdx).m_lstPictures[i].nOffset == m_nOffset)
		{
			//decrement offsets for all formatting marks
			int nOffset = g_doc.GetNodeByIdx(m_nNodeIdx).m_lstPictures[i].nOffset;
			int nSize = g_doc.GetNodeByIdx(m_nNodeIdx).m_lstTxtFmt.size();
			int j;
			for(j=0; j<nSize; j++){
				if(g_doc.GetNodeByIdx(m_nNodeIdx).m_lstTxtFmt[j].nOffset > nOffset)
					g_doc.GetNodeByIdx(m_nNodeIdx).m_lstTxtFmt[j].nOffset --;
			}

			//decrement all the pictures after this one
			nSize = g_doc.GetNodeByIdx(m_nNodeIdx).m_lstPictures.size();
			for(j=0; j<nSize; j++){
				if(g_doc.GetNodeByIdx(m_nNodeIdx).m_lstPictures[j].nOffset > nOffset)
					g_doc.GetNodeByIdx(m_nNodeIdx).m_lstPictures[j].nOffset --;
			}

			g_doc.GetNodeByIdx(m_nNodeIdx).m_lstPictures.erase(g_doc.GetNodeByIdx(m_nNodeIdx).m_lstPictures.begin() + i);
			break;
		}
	}

	//fix text view
	if(m_nNodeIdx == GetSelectedNodeIdx())
	{
		GtkTextBuffer* buffer1 = gtk_text_view_get_buffer((GtkTextView *)g_text.m_pWidget);

		//get the iterators
		GtkTextIter iter, iter2;
		gtk_text_buffer_get_iter_at_offset(buffer1, &iter, m_nOffset);
		gtk_text_buffer_get_iter_at_offset(buffer1, &iter2, m_nOffset+1);

		g_signal_handlers_block_by_func(buffer1, (void *)on_delete_text, 0);
		gtk_text_buffer_delete(buffer1, &iter, &iter2);
		g_signal_handlers_unblock_by_func(buffer1, (void *)on_delete_text, 0);
	}
}
