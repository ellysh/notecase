;
; NoteCase Windows setup script file (Inno Setup)
;

#define MyAppVer "1.9.8"

[Setup]
AppName=NoteCase
AppVerName=NoteCase v{#MyAppVer}
AppPublisherURL=http://notecase.sourceforge.net
AppSupportURL=http://notecase.sourceforge.net
AppUpdatesURL=http://notecase.sourceforge.net
AppCopyright=Copyright 2004-2008 by NoteCase developer team
DefaultDirName={pf}\NoteCase
DefaultGroupName=NoteCase
UninstallDisplayIcon={app}\notecase.exe
;it is possible to skip creating program group
AllowNoIcons=yes
; display this file before selection the destination
InfoBeforeFile=readme.txt
PrivilegesRequired=none
OutputBaseFilename=notecase-{#MyAppVer}_setup
LicenseFile=license.txt

[Tasks]
;Name: menuitems; Description: "Create a start menu items"; GroupDescription: "Additional items:";
Name: desktopicon; Description: "Create a &desktop icon"; GroupDescription: "Additional items:";
Name: quicklaunchicon; Description: "Create a &Quick Launch icon"; GroupDescription: "Additional items:"; Flags: unchecked
Name: startmenuicon; Description: "Create a &start menu icon"; GroupDescription: "Additional items:"; Flags: unchecked
Name: gtkinstall; Description: "Install GTK";

[Files]
Source: "notecase.exe"; DestDir: "{app}"; Flags: promptifolder
Source: "readme.txt"; DestDir: "{app}"; Flags: isreadme
Source: "help.ncd"; DestDir: "{app}"
Source: "notecase.url"; DestDir: "{app}"
Source: "msvcrt.dll"; DestDir: "{sys}"; Flags: onlyifdoesntexist uninsneveruninstall
Source: "locale\*.*"; DestDir: "{app}\locale"; Flags: recursesubdirs;

; GTK installation
;Source: "gtk-2.10.11-win32-1.exe"; Tasks: gtkinstall; DestDir: "{tmp}"; Flags: deleteafterinstall;
Source: "gtk\*.*"; DestDir: "c:\gtk\"; Tasks: gtkinstall; Flags: recursesubdirs;
; additionally copy these into the app directory to prevent "dll hell" problems
Source: "gtk\bin\iconv.dll"; DestDir: "{app}";
Source: "gtk\bin\intl.dll"; DestDir: "{app}";
Source: "gtk\bin\libxml2.dll"; DestDir: "{app}";
Source: "gtk\bin\xmlparse.dll"; DestDir: "{app}";

[Run]
; execute gtk installer if required
;Filename: "{tmp}\gtk-2.10.11-win32-1.exe";  Tasks: gtkinstall;  Flags: hidewizard skipifdoesntexist; AfterInstall: UpdateAppPath;

[Icons]
Name: "{group}\NoteCase"; Filename: "{app}\notecase.exe"; WorkingDir: "{app}";
Name: "{group}\Visit the Web Site"; Filename: "{app}\notecase.url";
Name: "{group}\Read me"; Filename: "{app}\readme.txt";
Name: "{group}\Uninstall NoteCase"; Filename: "{uninstallexe}";
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\NoteCase"; Filename: "{app}\notecase.exe"; Tasks: quicklaunchicon;  WorkingDir: "{app}"
Name: "{userdesktop}\NoteCase"; Filename: "{app}\notecase.exe"; Tasks: desktopicon; WorkingDir: "{app}"
Name: "{userstartmenu}\NoteCase"; Filename: "{app}\notecase.exe"; Tasks: startmenuicon; WorkingDir: "{app}"

[Registry]
; register .ncd format with notecase
Root: HKCR; Subkey: ".ncd"; ValueType: string; ValueName: ""; ValueData: "NoteCase.PlainFormat"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "NoteCase.PlainFormat"; ValueType: string; ValueName: ""; ValueData: "NoteCase unencrypted file format"; Flags: uninsdeletekey
Root: HKCR; Subkey: "NoteCase.PlainFormat\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\notecase.exe,0"
Root: HKCR; Subkey: "NoteCase.PlainFormat\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\notecase.exe"" ""%1"""
; register .nce format with notecase
Root: HKCR; Subkey: ".nce"; ValueType: string; ValueName: ""; ValueData: "NoteCase.EncryptedFormat"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "NoteCase.EncryptedFormat"; ValueType: string; ValueName: ""; ValueData: "NoteCase encrypted file format"; Flags: uninsdeletekey
Root: HKCR; Subkey: "NoteCase.EncryptedFormat\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\notecase.exe,0"
Root: HKCR; Subkey: "NoteCase.EncryptedFormat\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\notecase.exe"" ""%1"""
; This adds the GTK+ libraries to notecase.exe's path
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\App Paths\notecase.exe"; Flags: uninsdeletekeyifempty
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\App Paths\notecase.exe"; ValueType: string; ValueData: "{app}\notecase.exe"; Flags: uninsdeletevalue
; this one is a placeholder, its value is filled in UpdateAppPath();
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\App Paths\notecase.exe"; ValueType: string; ValueName: "Path"; ValueData: ""; Flags: uninsdeletevalue

; GTK+ installation
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; ValueType: string; ValueName: "GTK_BASEPATH"; ValueData: "C:\GTK"; Flags: uninsdeletevalue; Tasks: gtkinstall;
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; ValueType: string; ValueName: "Path"; ValueData: {code:UpdateGtkPath}; Tasks: gtkinstall;
Root: HKLM; Subkey: "Software\GTK\2.0"; ValueType: string; ValueName: "Path"; ValueData: "C:\GTK"; Flags: uninsdeletevalue; Tasks: gtkinstall;
Root: HKLM; Subkey: "Software\GTK\2.0"; ValueType: string; ValueName: "DllPath"; ValueData: "C:\GTK\bin"; Flags: uninsdeletevalue; Tasks: gtkinstall;
Root: HKLM; Subkey: "Software\GTK\2.0"; ValueType: string; ValueName: "Version"; ValueData: "2.12.1"; Flags: uninsdeletevalue; Tasks: gtkinstall;
Root: HKLM; Subkey: "Software\GTK\2.0"; ValueType: string; ValueName: "VendorVersion"; ValueData: "2.12.1-1"; Flags: uninsdeletevalue; Tasks: gtkinstall;

[Code]

var
  Exists: Boolean;
  GtkPath: String;
  oldpath: String;
  
function GetGtkInstalled(): Boolean;
begin
  Exists := RegQueryStringValue (HKLM, 'Software\GTK\2.0', 'Path', GtkPath);
  if not Exists then begin
    Exists := RegQueryStringValue (HKCU, 'Software\GTK\2.0', 'Path', GtkPath);
  end;
   Result := Exists
end;

procedure UpdateAppPath();
var
 AppPath: String;
 RegValue: String;
begin
    AppPath := ExpandConstant('{app}');

    GetGtkInstalled (); //first read Gtk Path

    //create and write registry key
    RegValue := AppPath;
    RegValue := RegValue + ';';
    RegValue := RegValue + GtkPath;
    RegValue := RegValue + '\bin';
    
    //MsgBox(RegValue, mbInformation, MB_OK);
    RegWriteStringValue (HKLM, 'Software\Microsoft\Windows\CurrentVersion\App Paths\notecase.exe', 'Path', RegValue);
end;

function UpdateGtkPath(Param: String): String;
begin
    // Get current path, split into an array
		//RegQueryStringValue(HKEY_LOCAL_MACHINE, 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment', 'Path', oldpath);
		oldpath := GetEnv('PATH');
		Result := 'c:\gtk\bin;' + oldpath;
end;

// this is an event function called directly by InnoSetup
function NeedRestart(): Boolean;
begin
	if IsTaskSelected('gtkinstall') then begin
		Result := True;
	end else begin
		Result := False;
	end;
end;
