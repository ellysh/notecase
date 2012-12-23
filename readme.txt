===========================
     NoteCase README
===========================

   NoteCase is a free hierarchical note manager program. It is published
   as open source and licensed under BSD-like license. Program is written
   in portable C++ (using GTK 2.x toolkit) and is currently compiled on 
   Windows and Linux operating systems.

   For more details on the project, consider browsing program web pages,
   or using help.ncd document (program help file).
   Project license can be found in the "docs" directory as license.txt.
   
   Big thanks goes to the <http://www.famfamfam.com/lab/icons/> site for some great free
   icons used in this product.

   You might also consider using an advanced commercial version named Notecase Pro
   available at: http://www.virtual-sky.com

-----------------
  Features
-----------------

  - rich format editing with support for embedded images
  - unencrypted and encrypted document formats supported
  - NoteCenter unencrypted format supported (www.notecenter.net)
  - multiple text node operations are supported (moving up/down/...)
  - import gjots2 documents (requires .gjots2 extension on document)
  - multilanguage support
  - undo/redo support

-----------------
  Installation
-----------------

  Windows:

	 Easiest wayt o install the program on Windows is to use installer version. Just run the installer, 
	 and this version will setup everything for you (inlcuign the GTK installation).

	 When installing from Windows binary archive, just unpack the archive contents and start the program. 
   This version requires GTK toolkit installation (available at <http://gladewin32.sourceforge.net/>).
   If you wish to compile the sources yourself download and install latest development package. If not, 
   you'll need only runtime GTK package version.

   Portable version of Notecase must be started through the "notecaselauncher.exe" file.
   
  Other OS:
   
   Notecase is built for a number ot Linux distros. At the project website, find the binary package 
   for your distro (or some similar distro) and install it either by double-clicking the package in GUI,
   or using the commands of your package manager tool (rpm, dpkg or similar).
   
   If the build is an ordinary .tar.gz archive, just unpack this archive to the "/" directory.

-----------------
  Compiling
-----------------

   1. Notecase compilation on Ubuntu
	- run "sudo apt-get install make g++ libgtk2.0-dev libgnomevfs2-dev" to install packages needed to compile notecase
	- download and unpack latest notecase sources
	- cd to notecase directory
	- run "sudo make install" (or just "make" if you do not wish to install the software)
	- result of running "make" is the "./bin/notecase" application file
	- as result of running "make install", application is both built and installed to your system
	- start notecase with "notecase" or "/usr/bin/notease" in terminal

-----------------
  Updates
-----------------

   The latest version of NoteCase can be found at <http://notecase.sf.net>
   Please send any suggestions/bug reports to e-mail 
   <miroslav.rajcic@inet.hr>
