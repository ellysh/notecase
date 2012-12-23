# NoteCase RPM spec file.

Name:           notecase
Version:        1.9.8
Release:        1%{?dist}
Summary:        A hierarchical text notes manager.

Group:          Applications/Productivity
License:        BSD
URL:            http://notecase.sourceforge.net/
Source:         notecase-1.9.8_src.tar.gz
Icon:           notecase.xpm
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:  gtk2-devel >= 2.4 desktop-file-utils gettext gnome-vfs2-devel
Requires:       gtk2 gettext
Requires(post): desktop-file-utils shared-mime-info
Requires(postun): desktop-file-utils shared-mime-info

%description
NoteCase is a hierarchical text notes manager (outliner). 
It helps you organize your everyday text notes into a single document with individual notes placed into a 
tree-like structure. To ensure your privacy an encrypted document format is supported along
with a standard unencrypted one.

%prep
%setup -n %{name}-%{version}

%build
%{__make} CFLAGS="$RPM_OPT_FLAGS" %{?_smp_mflags}

%install
%{__rm} -rf "%{buildroot}"
%{makeinstall}
desktop-file-install --vendor="" \
  --dir=%{buildroot}%{_datadir}/applications \
  --delete-original \
  --remove-category Office \
  %{buildroot}%{_datadir}/applications/notecase.desktop
%find_lang %{name}

%post
#add notecase into mime database
update-mime-database %{_datadir}/mime/ &> /dev/null || :
update-desktop-database &> /dev/null || :

%postun
#remove notecase from mime database
update-mime-database %{_datadir}/mime/ &> /dev/null || :
update-desktop-database &> /dev/null || :

%clean
%{__rm} -rf "%{buildroot}"

%files -f %{name}.lang
%defattr(-, root, root)
%{_bindir}/notecase
%dir %{_docdir}/notecase
%{_docdir}/notecase/help.ncd
%{_datadir}/pixmaps/notecase.xpm
%{_datadir}/applications/notecase.desktop
%{_datadir}/mime/packages/notecase.xml

%changelog
* Fri Jan 12 2007 Miroslav Rajcic <miroslav.rajcic@inet.hr>
- icon is now installed in /usr/share/pixmaps instead of /usr/share/icons
* Sat Oct 22 2005 Miroslav Rajcic <miroslav.rajcic@inet.hr>
- using .tar.gz instead of .zip for source archive
* Tue Aug 02 2005 Miroslav Rajcic <miroslav.rajcic@inet.hr>
- using %{?dist} macro instead of hardcoded distro code
* Mon Jun 06 2005 Miroslav Rajcic <miroslav.rajcic@inet.hr>
- using %find_lang macro instead of hardcoded list of .po files
* Thu Apr 14 2005 Miroslav Rajcic <miroslav.rajcic@inet.hr>
- updated build requirements information, added update-desktop-database call
* Thu Jan 20 2005 Miroslav Rajcic <miroslav.rajcic@inet.hr>
- updated packager and vendor fields, added new locale files to file list
* Wed Oct 20 2004 Neil Zanella <nzanella@users.sourceforge.net>
- Added desktop entry support
* Wed Oct 07 2004 Neil Zanella <nzanella@users.sourceforge.net>
- Moved some functionality out of SPEC file and into Makefile.
* Wed Oct 06 2004 Neil Zanella <nzanella@users.sourceforge.net>
- XPM icon is now more application-friendly and security fixes
* Tue Oct 04 2004 Neil Zanella <nzanella@users.sourceforge.net>
- minor modifications
* Sun Oct 03 2004 Neil Zanella <nzanella@users.sourceforge.net>
- initial release
