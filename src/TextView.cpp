////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: GTK+ TextView wrapper class
////////////////////////////////////////////////////////////////////////////

#include "TextView.h"
#include "config.h"

#ifdef HAVE_GTKSOURCEVIEW
	#include <gtksourceview/gtksourceview.h>
	#include <gtksourceview/gtksourcelanguage.h>
	#include <gtksourceview/gtksourcelanguagemanager.h>
#endif

//declare signal handlers
void on_textview_edited (GtkTextBuffer *, gpointer data);
void on_delete_text (GtkTextBuffer *textbuffer, GtkTextIter *arg1, GtkTextIter *arg2, gpointer user_data);
void on_insert_text (GtkTextBuffer *textbuffer, GtkTextIter *arg1, gchar *arg2, gint arg3, gpointer user_data);
gint textview_keyboard_handler(GtkWidget *widget, GdkEventKey *event);
gboolean textview_mouse_click(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
gboolean textview_mouse_moved(GtkWidget *widget, GdkEventMotion *event, gpointer user_data);
void textview_cursor_moved(GtkTextView *widget, GtkMovementStep *step, gint count, gboolean extend_selection, gpointer user_data);
void textview_selection_changed(GtkTextBuffer *textbuffer, GtkTextIter *arg1, GtkTextMark *arg2, gpointer user_data);
gboolean textview_on_drop(GtkWidget *widget, GdkDragContext *drag_context, gint x, gint y, guint time, gpointer user_data);
void drag_data_received_handl(GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *selection_data, guint target_type, guint time, gpointer data);

extern GdkColor g_linkColor;

GdkAtom atomTextUriList;
GdkAtom atomString;

#ifdef HAVE_GTKSOURCEVIEW
GtkSourceBuffer *gtk_source_view_get_buffer(GtkSourceView *view)
{
	return GTK_SOURCE_BUFFER(GTK_TEXT_VIEW(view)->buffer);
}
#endif

TextView::TextView()
{
	m_pWidget = NULL;
}

TextView::~TextView()
{
}

void TextView::Create()
{
	GtkWidget *textview1;

	GTK_TEXTBUFFER_CLASS *buffer = GTK_TEXTBUFFER_CLASS_NEW (NULL);
	textview1 = GTK_TEXTVIEW_CLASS_NEW (buffer);
	gtk_widget_show (textview1);

	g_signal_connect (G_OBJECT (textview1), "key_press_event", G_CALLBACK (textview_keyboard_handler), NULL);
	g_signal_connect (G_OBJECT (textview1), "button-press-event", G_CALLBACK (textview_mouse_click), NULL);
	g_signal_connect (G_OBJECT (textview1), "motion-notify-event", G_CALLBACK (textview_mouse_moved), this);
	g_signal_connect (G_OBJECT (textview1), "drag-drop",			G_CALLBACK (textview_on_drop), this);
	g_signal_connect (G_OBJECT (textview1), "drag-data-received",	G_CALLBACK (drag_data_received_handl), this);

	//make the "textview1" a DnD destination
	gtk_drag_dest_add_text_targets(GTK_WIDGET(textview1));
	gtk_drag_dest_add_uri_targets(GTK_WIDGET(textview1));

	atomTextUriList = gdk_atom_intern("text/uri-list", FALSE);
	atomString		= gdk_atom_intern("STRING", FALSE);

#ifdef _NOKIA_MAEMO
	gtk_widget_tap_and_hold_setup(GTK_WIDGET(textview1), NULL, NULL, (GtkWidgetTapAndHoldFlags)(GTK_TAP_AND_HOLD_PASS_PRESS /*GTK_TAP_AND_HOLD_NONE | GTK_TAP_AND_HOLD_NO_INTERNALS*/));
	//g_signal_connect(treeview1, "tap-and-hold", G_CALLBACK(on_popup_menu), NULL);
#endif

	g_signal_connect(buffer, "changed", G_CALLBACK(on_textview_edited),  NULL); //track buffer changes

	//events to handle undo in textview
	g_signal_connect (buffer, "insert-text",  G_CALLBACK(on_insert_text), NULL);
	g_signal_connect (buffer, "delete-range", G_CALLBACK(on_delete_text), NULL);

	//we don't use marks by ourselves, so we can track movement of "insert" and "selection_bound" marks,
	g_signal_connect (buffer, "mark-set",	  G_CALLBACK(textview_selection_changed), NULL);

	m_pWidget = textview1;
}

void TextView::Clear()
{
	GtkTextView *textview =  (GtkTextView *)m_pWidget;
	GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);

	//delete previous content
	GtkTextIter itStart, itEnd;
	gtk_text_buffer_get_iter_at_offset(buffer1, &itStart, 0);
	gtk_text_buffer_get_iter_at_offset(buffer1, &itEnd, gtk_text_buffer_get_char_count(buffer1));

	gtk_text_buffer_remove_all_tags(buffer1, &itStart, &itEnd);

	g_signal_handlers_block_by_func(buffer1, (void *)on_delete_text, 0);
	g_signal_handlers_block_by_func(buffer1, (void *)on_textview_edited, 0);
	gtk_text_buffer_delete(buffer1, &itStart, &itEnd);
	g_signal_handlers_unblock_by_func(buffer1, (void *)on_delete_text, 0);
	g_signal_handlers_unblock_by_func(buffer1, (void *)on_textview_edited, 0);
}

std::string TextView::GetText()
{
	GtkTextView *textview =  (GtkTextView *)m_pWidget;
	GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);

	GtkTextIter itStart, itEnd;
	gtk_text_buffer_get_iter_at_offset(buffer1, &itStart, 0);
	gtk_text_buffer_get_iter_at_offset(buffer1, &itEnd, gtk_text_buffer_get_char_count(buffer1));

	gchar *pszData = gtk_text_buffer_get_text(buffer1, &itStart, &itEnd, FALSE);
	std::string strData = pszData;
	g_free(pszData);
	return strData;
}

void TextView::SetText(const gchar *szText)
{
	Clear();	// clear previous contents

	GtkTextView *textview =  (GtkTextView *)m_pWidget;
	GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);

	g_signal_handlers_block_by_func(buffer1, (void *)on_textview_edited, 0);
	g_signal_handlers_block_by_func(buffer1, (void *)on_insert_text, 0);

	gtk_text_buffer_insert_at_cursor(buffer1, szText, -1);
	gtk_text_view_place_cursor_onscreen(GTK_TEXT_VIEW(textview));

	g_signal_handlers_unblock_by_func(buffer1, (void *)on_textview_edited, 0);
	g_signal_handlers_unblock_by_func(buffer1, (void *)on_insert_text, 0);
}

void TextView::InsertText(int nOffset, const gchar *szText)
{
	GtkTextView *textview =  (GtkTextView *)m_pWidget;
	GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);

	GtkTextIter itStart;
	gtk_text_buffer_get_iter_at_offset(buffer1, &itStart, nOffset);

	g_signal_handlers_block_by_func(buffer1, (void *)on_textview_edited, 0);
	g_signal_handlers_block_by_func(buffer1, (void *)on_insert_text, 0);

	gtk_text_buffer_insert(buffer1, &itStart, szText, -1);
	gtk_text_view_place_cursor_onscreen(GTK_TEXT_VIEW(textview));

	g_signal_handlers_unblock_by_func(buffer1, (void *)on_insert_text, 0);
	g_signal_handlers_unblock_by_func(buffer1, (void *)on_textview_edited, 0);
}

void TextView::DeleteText(int nOffset, int nLength)
{
	GtkTextView *textview =  (GtkTextView *)m_pWidget;
	GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);

	GtkTextIter itStart, itEnd;
	gtk_text_buffer_get_iter_at_offset(buffer1, &itStart, nOffset);
	gtk_text_buffer_get_iter_at_offset(buffer1, &itEnd, nOffset+nLength);

	g_signal_handlers_block_by_func(buffer1, (void *)on_textview_edited, 0);
	g_signal_handlers_block_by_func(buffer1, (void *)on_delete_text, 0);

	gtk_text_buffer_delete(buffer1, &itStart, &itEnd);

	g_signal_handlers_unblock_by_func(buffer1, (void *)on_delete_text, 0);
	g_signal_handlers_unblock_by_func(buffer1, (void *)on_textview_edited, 0);
}

void TextView::ClipboardCut()
{
	if(IsEditable())
	{
		GtkTextView *textview =  (GtkTextView *)m_pWidget;
		GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);

		// get the clipboard object
		GtkClipboard *clipboard = gtk_widget_get_clipboard (m_pWidget, GDK_SELECTION_CLIPBOARD);
		gtk_text_buffer_cut_clipboard (buffer1, clipboard, true);
	}
}

void TextView::ClipboardCopy()
{
	GtkTextView *textview =  (GtkTextView *)m_pWidget;
	GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);

	// get the clipboard object
	GtkClipboard *clipboard = gtk_widget_get_clipboard (m_pWidget, GDK_SELECTION_CLIPBOARD);
	gtk_text_buffer_copy_clipboard (buffer1, clipboard);
}

void TextView::ClipboardPaste()
{
	if(IsEditable())
	{
		GtkTextView *textview =  (GtkTextView *)m_pWidget;
		GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);

		// get the clipboard object
		GtkClipboard *clipboard = gtk_widget_get_clipboard (m_pWidget, GDK_SELECTION_CLIPBOARD);
		gtk_text_buffer_paste_clipboard (buffer1, clipboard, NULL, true);
	}
}

void TextView::DeleteSelection()
{
	if(IsEditable())
	{
		GtkTextView *textview =  (GtkTextView *)m_pWidget;
		GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);
		gtk_text_buffer_delete_selection (buffer1, false, true);
	}
}

int TextView::GetSelectionEnd()
{
	GtkTextView *textview =  (GtkTextView *)m_pWidget;
	GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);

	GtkTextIter end;
	if(gtk_text_buffer_get_selection_bounds(buffer1, NULL, &end))
	{
		return gtk_text_iter_get_offset(&end);
	}
	return -1; //no selection
}

void TextView::SelectRange(int nStart, int nEnd)
{
	//select found text in a text view
	GtkTextView *textview =  (GtkTextView *)m_pWidget;
	GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);

	GtkTextIter iterStart, iterEnd;
	gtk_text_buffer_get_iter_at_offset(buffer1, &iterStart, nStart);
	gtk_text_buffer_get_iter_at_offset(buffer1, &iterEnd, nEnd);
#if GTK_CHECK_VERSION(2,4,0) //define minimal version for this api
	gtk_text_buffer_select_range(buffer1, &iterStart, &iterEnd);
#endif
}

void TextView::EnsureVisible(int nOffset)
{
	//scroll text view to ensure text at this offset is visible
	GtkTextView *textview =  (GtkTextView *)m_pWidget;
	GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);

	GtkTextIter iter;
	gtk_text_buffer_get_iter_at_offset(buffer1, &iter, nOffset);

	//FIX: scroll_to_iter does not work OK!
	//gtk_text_view_scroll_to_iter(textview, &iter, 0.0, TRUE, 0.0, 1.0);
	GtkTextMark*mark = gtk_text_buffer_create_mark(buffer1, NULL, &iter, TRUE);
	gtk_text_view_scroll_to_mark(textview, mark, 0.0, TRUE, 0.0, 0.0);
	gtk_text_buffer_delete_mark(buffer1, mark);
}

bool TextView::IsWrapped()
{
	GtkTextView *textview =  (GtkTextView *)m_pWidget;
	return (GTK_WRAP_NONE != gtk_text_view_get_wrap_mode(GTK_TEXT_VIEW(textview)));
}

bool TextView::GetModified()
{
	GtkTextView *textview =  (GtkTextView *)m_pWidget;
	GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);
	return (gtk_text_buffer_get_modified(buffer1) > 0);
}

void TextView::SetModified(bool bModified)
{
	GtkTextView *textview = (GtkTextView *)m_pWidget;
	GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);
	gtk_text_buffer_set_modified(buffer1, (bModified)? TRUE : FALSE);
}

void TextView::SetFocus()
{
 	gtk_widget_grab_focus( m_pWidget );

 	//view cursor in a TextView
 	gtk_text_view_scroll_mark_onscreen (
 		GTK_TEXT_VIEW(m_pWidget),
		gtk_text_buffer_get_insert( gtk_text_view_get_buffer( GTK_TEXT_VIEW(m_pWidget) ) )
	);
}

//
// cursor will be set to cursor position and selection to the other end of the selection.
// If there is not selection, selection will be set to same value as the cursor
//
void TextView::GetSelectionBounds(int& cursor, int& selection)
{
	GtkTextView *textview =  (GtkTextView *)m_pWidget;
	GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);
	GtkTextIter iter;

	gtk_text_buffer_get_iter_at_mark (buffer1, &iter, gtk_text_buffer_get_insert (buffer1) );
	cursor = gtk_text_iter_get_offset(&iter);

	gtk_text_buffer_get_iter_at_mark (buffer1, &iter, gtk_text_buffer_get_selection_bound (buffer1) );
	selection = gtk_text_iter_get_offset(&iter);
}

void TextView::RestoreSelectionBounds(unsigned int cursor, unsigned int selection)
{
	GtkTextView *textview =  (GtkTextView *)m_pWidget;
	GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);
	GtkTextIter cursIter, boundIter;

	gtk_text_buffer_get_iter_at_offset( buffer1, &cursIter, gint(cursor) );
	gtk_text_buffer_get_iter_at_offset( buffer1, &boundIter, gint(selection) );

	gtk_text_buffer_select_range ( buffer1, &cursIter, &boundIter );
}

//line number
int  TextView::GetScrollPos()
{
	int nBufferY = 0;
	gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(m_pWidget), GTK_TEXT_WINDOW_WIDGET, 0, 0, NULL, &nBufferY);
	GtkTextIter iter;
	gtk_text_view_get_line_at_y(GTK_TEXT_VIEW(m_pWidget), &iter, nBufferY, NULL);
	return gtk_text_iter_get_line(&iter);
}

void TextView::RestoreScrollPos(unsigned int nPos)
{
	GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_pWidget));

	GtkTextIter iter;
	gtk_text_buffer_get_iter_at_offset(buffer1, &iter, 0);
	gtk_text_iter_set_line(&iter, nPos);
	gtk_text_iter_set_line_offset(&iter, 0);

	EnsureVisible(gtk_text_iter_get_offset(&iter));
}

void TextView::SetEditable(bool bEditable)
{
	gtk_text_view_set_editable(GTK_TEXT_VIEW(m_pWidget), bEditable);

	//redraw required for special expose drawing "textview_expose_event_cb" based on editable state
	gtk_widget_queue_draw(m_pWidget);

	if(bEditable)
		GTK_WIDGET_SET_FLAGS(m_pWidget, GTK_CAN_FOCUS);
	else
		GTK_WIDGET_UNSET_FLAGS(m_pWidget, GTK_CAN_FOCUS);
}

bool TextView::IsEditable()
{
	return (gtk_text_view_get_editable(GTK_TEXT_VIEW(m_pWidget)) > 0);
}

std::string TextView::GetSelectedText()
{
	GtkTextView *textview =  (GtkTextView *)m_pWidget;
	GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);
	GtkTextIter iterStart, iterEnd;

	gtk_text_buffer_get_iter_at_mark (buffer1, &iterStart, gtk_text_buffer_get_insert (buffer1) );
	gtk_text_buffer_get_iter_at_mark (buffer1, &iterEnd, gtk_text_buffer_get_selection_bound (buffer1) );

	gchar *pszData = gtk_text_buffer_get_text(buffer1, &iterStart, &iterEnd, FALSE);
	std::string strData = pszData;
	g_free(pszData);
	return strData;
}

void TextView::ReplaceText(int nStart, int nEnd, const char *szNewTxt)
{
	GtkTextView *textview =  (GtkTextView *)m_pWidget;
	GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);

	//get the iterators
	GtkTextIter cursIter, boundIter;
	gtk_text_buffer_get_iter_at_offset(buffer1, &cursIter, nStart);
	gtk_text_buffer_get_iter_at_offset(buffer1, &boundIter, nEnd);

	gtk_text_buffer_delete(buffer1, &cursIter, &boundIter);
	gtk_text_buffer_insert(buffer1, &cursIter, szNewTxt, -1);
}


void TextView::SetTextUnderlined(int nStart, int nEnd)
{
	//
	// mark the text
	//
	GtkTextView *textview =  (GtkTextView *)m_pWidget;
	GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);
	//get the iterators
	GtkTextIter cursIter, boundIter;
	gtk_text_buffer_get_iter_at_offset(buffer1, &cursIter, nStart);
	gtk_text_buffer_get_iter_at_offset(buffer1, &boundIter, nEnd);

	char szColor[30];
	sprintf (szColor, "#%04x%04x%04x", g_linkColor.red, g_linkColor.green, g_linkColor.blue);

	//"linktag"
	GtkTextTag *tag = gtk_text_buffer_create_tag (buffer1, NULL,
			"foreground", szColor,
			"underline", PANGO_UNDERLINE_SINGLE,
			"underline-set", TRUE,
			NULL);
	gtk_text_buffer_apply_tag(buffer1, tag, &cursIter, &boundIter);
}

void TextView::RemoveTextStyles(int nStart, int nEnd)
{
	// unmark the text
	GtkTextView *textview =  (GtkTextView *)m_pWidget;
	GtkTextBuffer* buffer1 = gtk_text_view_get_buffer(textview);
	//get the iterators
	GtkTextIter cursIter, boundIter;
	gtk_text_buffer_get_iter_at_offset(buffer1, &cursIter, nStart);
	gtk_text_buffer_get_iter_at_offset(buffer1, &boundIter, nEnd);

	gtk_text_buffer_remove_all_tags (buffer1, &cursIter, &boundIter);
}

void TextView::SetTabWidth(int nWidth)
{
	if(nWidth < 1)
		return;

	PangoTabArray *tabs = pango_tab_array_new(1, TRUE);

	pango_tab_array_set_tab(tabs, 0, PANGO_TAB_LEFT, nWidth);
	gtk_text_view_set_tabs(GTK_TEXT_VIEW(m_pWidget), tabs);

	pango_tab_array_free(tabs);
}

int TextView::CalcTabWidthInPixels(int nNumChars)
{
	if (nNumChars <= 0)
		return -1;

	gchar *tab_string = g_strnfill(nNumChars, 'O');
	PangoLayout *layout = gtk_widget_create_pango_layout(m_pWidget, tab_string);
	g_free (tab_string);

	gint tab_width = 0;
	if (layout != NULL) {
		pango_layout_get_pixel_size(layout, &tab_width, NULL);
		g_object_unref(G_OBJECT(layout));
	} else
		tab_width = -1;

	return tab_width;
}

#ifdef HAVE_GTKSOURCEVIEW
void TextView::SetSourceLanguage(const gchar *szSourceLanguage)
{
    if(szSourceLanguage)
    {
        GtkSourceLanguageManager *manager = gtk_source_language_manager_get_default();
        GtkSourceLanguage *language = gtk_source_language_manager_get_language (manager, szSourceLanguage);
				GtkSourceView *textview =  (GtkSourceView *)m_pWidget;
				GtkSourceBuffer* buffer1 = gtk_source_view_get_buffer(textview);
        gtk_source_buffer_set_language(buffer1, language);
    }
}
#endif
