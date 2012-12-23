////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Implements text format Action
////////////////////////////////////////////////////////////////////////////

#include "DocActionFmt.h"
#include "callbacks.h"
#include "support.h"
#include "interface.h"
#include "lib/NoteDocument.h"
#include "lib/DocumentIterator.h"
#include "TextView.h"
#include "TreeView.h"
#include "lib/debug.h"
#include <algorithm>

#ifdef _WIN32
 #include <io.h>
 #ifndef __MINGW32__
  #define strcasecmp stricmp
 #endif
#endif

#ifndef min
 #define min(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef max
 #define max(a,b) (((a)>(b))?(a):(b))
#endif

extern GtkWidget *window1;
extern NoteDocument g_doc;
extern TextView g_text;
int GetSelectedNodeIdx();
static GdkColor g_rgbTextColor = { 0, 0xFFFF, 0, 0 };	//red color as default
static GdkColor g_rgbTextBgColor = { 0, 0xFFFF, 0, 0 };	//red color as default

void textview_selection_changed(GtkTextBuffer *textbuffer, GtkTextIter *arg1, GtkTextMark *arg2, gpointer user_data);

DocActionFmt::DocActionFmt()
{
	m_nNodeIdx		= -1;
	m_nOperationTag	= -1;
	m_bCtrlPressed	= false;
}

DocActionFmt::~DocActionFmt()
{
}

void DocActionFmt::Exec(bool bInteractive)
{
	bool bExists = false;

	if(bInteractive)
	{
		m_lstFmt = g_doc.GetNodeByIdx(m_nNodeIdx).m_lstTxtFmt;

		int nCursor = 0;
		int nSelection = 0;

		g_text.GetSelectionBounds(nCursor, nSelection);
		if(nCursor != nSelection)
		{
			int nStart  = min(nCursor, nSelection);
			int nEnd    = max(nCursor, nSelection);

			m_nStartOffset = nStart;
			m_nEndOffset   = nEnd;

			bExists = g_doc.GetNodeByIdx(m_nNodeIdx).m_lstTxtFmt.IsRangeFormatted(m_nStartOffset, m_nEndOffset, m_nOperationTag);

			//start color picker if required
			GdkColor color;
			if( (!bExists || m_bCtrlPressed) &&
				(FMT_TXT_COLOR_BEGIN == m_nOperationTag || FMT_TXT_BKG_COLOR_BEGIN == m_nOperationTag))
			{
				if(m_bCtrlPressed)
				{
					if(FMT_TXT_COLOR_BEGIN == m_nOperationTag)
						color = g_rgbTextColor;
					else
						color = g_rgbTextBgColor;
				}
				else
				{
					GtkWidget *picker = gtk_color_selection_dialog_new  (_("Pick text color"));
					gtk_window_set_modal (GTK_WINDOW (picker), TRUE);
					gtk_window_set_destroy_with_parent (GTK_WINDOW (picker), TRUE);
				#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
					gtk_window_set_skip_taskbar_hint (GTK_WINDOW (picker), TRUE);
				#endif
					gtk_window_set_skip_pager_hint (GTK_WINDOW (picker), TRUE);
					gtk_window_set_type_hint (GTK_WINDOW (picker), GDK_WINDOW_TYPE_HINT_DIALOG);
					gtk_window_set_transient_for(GTK_WINDOW (picker), GTK_WINDOW(window1));   //set parent
				#if GTK_CHECK_VERSION(2,4,0) //new API TOFIX set proper version
					#ifndef _WIN32  //TOFIX API is buggy on Win32 (kills modal dialog state)
					  //gtk_window_set_keep_above(GTK_WINDOW (picker), TRUE);
					#endif
				#endif
					gtk_widget_realize(picker);
					gdk_window_set_decorations(picker->window, (GdkWMDecoration)(GDK_DECOR_BORDER|GDK_DECOR_TITLE));

					GtkColorSelection *colorsel = GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (picker)->colorsel);
					if(FMT_TXT_COLOR_BEGIN == m_nOperationTag){
						gtk_color_selection_set_previous_color(GTK_COLOR_SELECTION(colorsel), &g_rgbTextColor);
						gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(colorsel), &g_rgbTextColor);
					}
					else
					{
						gtk_color_selection_set_previous_color(GTK_COLOR_SELECTION(colorsel), &g_rgbTextBgColor);
						gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(colorsel), &g_rgbTextBgColor);
					}
					gtk_color_selection_set_has_palette (GTK_COLOR_SELECTION(colorsel), TRUE);

					if(GTK_RESPONSE_OK != gtk_dialog_run(GTK_DIALOG(picker))){
						gtk_widget_destroy(picker);
						return;
					}
					gtk_color_selection_get_current_color(colorsel, &color);
					gtk_widget_destroy(picker);
				}

				m_color = color;
				if(FMT_TXT_COLOR_BEGIN == m_nOperationTag)
					g_rgbTextColor = color;		//remember last used color
				else
					g_rgbTextBgColor = color;		//remember last used color
			}
		}
	}

	bExists = g_doc.GetNodeByIdx(m_nNodeIdx).m_lstTxtFmt.IsRangeFormatted(m_nStartOffset, m_nEndOffset, m_nOperationTag);
	if(bExists){
		g_doc.GetNodeByIdx(m_nNodeIdx).m_lstTxtFmt.ClearFormatRange(m_nStartOffset, m_nEndOffset, m_nOperationTag);
		RefreshTextFormat(g_doc.GetNodeByIdx(m_nNodeIdx), false, m_nStartOffset, m_nEndOffset);
	}
	else{
		g_doc.GetNodeByIdx(m_nNodeIdx).m_lstTxtFmt.AddFormatRange(m_nStartOffset, m_nEndOffset, m_nOperationTag, m_color);
		if(FMT_NONE == m_nOperationTag)
			RefreshTextFormat(g_doc.GetNodeByIdx(m_nNodeIdx), false, m_nStartOffset, m_nEndOffset);
	}

	textview_selection_changed(NULL, NULL, NULL, NULL);

	TRACE("DocActionFmt::Exec\n");
}

void DocActionFmt::Undo()
{
	//remove start fmt from the node list
	int nCount = g_doc.GetNodeByIdx(m_nNodeIdx).m_lstTxtFmt.size();
	int i, nStart;
	nStart = -1;
	for(i=0; i<nCount; i++)
	{
		if( m_nStartOffset	== g_doc.GetNodeByIdx(m_nNodeIdx).m_lstTxtFmt[i].nOffset &&
			m_nOperationTag == g_doc.GetNodeByIdx(m_nNodeIdx).m_lstTxtFmt[i].nFormatTag)
		{
			g_doc.GetNodeByIdx(m_nNodeIdx).m_lstTxtFmt.erase(g_doc.GetNodeByIdx(m_nNodeIdx).m_lstTxtFmt.begin()+i);
			nCount--;
			nStart = -1;
			break;
		}
	}

	//remove end tag
	if(nStart >= 0)
	{
		int nEndTag = FmtInfoList::CalcFormatEndTag(m_nOperationTag);
		ASSERT(nEndTag >= 0);

		for(i=nStart; i<nCount; i++)
		{
			if( m_nEndOffset == g_doc.GetNodeByIdx(m_nNodeIdx).m_lstTxtFmt[i].nOffset &&
				nEndTag		 == g_doc.GetNodeByIdx(m_nNodeIdx).m_lstTxtFmt[i].nFormatTag)
			{
				g_doc.GetNodeByIdx(m_nNodeIdx).m_lstTxtFmt.erase(g_doc.GetNodeByIdx(m_nNodeIdx).m_lstTxtFmt.begin()+i);
				break;
			}
		}
	}

	//refresh text format
	//TOFIX this is not optimal for long node text, we should refresh only the range where this tag was placed
	if(m_nNodeIdx == GetSelectedNodeIdx())
	{
		RefreshTextFormat(g_doc.GetNodeByIdx(m_nNodeIdx), false, m_nStartOffset, m_nEndOffset);
		textview_selection_changed(NULL, NULL, NULL, NULL);
	}
}
