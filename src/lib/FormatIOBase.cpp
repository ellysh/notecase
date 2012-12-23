////////////////////////////////////////////////////////////////////////////
// NoteCase notes manager project <http://notecase.sf.net>
//
// This code is licensed under BSD license.See "license.txt" for more details.
//
// File: Abstract class for different document formats I/O
////////////////////////////////////////////////////////////////////////////

#include "FormatIOBase.h"
#include <gtk/gtk.h>	//TOFI glib only?

FormatIO_Base::FormatIO_Base()
{
}

FormatIO_Base::~FormatIO_Base()
{
}

void FormatIO_Base::RunLoop()
{
	//run main loop to repaint progress when loading big documents
	int nLoops=0;
	while(gtk_main_iteration_do(false) && nLoops<20) nLoops++;
}
