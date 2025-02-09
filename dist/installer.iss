; Inno Setup script making SR3 installer for Windows

; Main path, should have SR subdirs:
;   bin\Release  config\  data\  docs\  redist\
; and files:  License.txt  Readme.md

; redist\ path should contain both VC_redist.x86.exe and VC_redist.x64.exe
; got from https://learn.microsoft.com/en-GB/cpp/windows/latest-supported-vc-redist?view=msvc-170#visual-studio-2015-2017-2019-and-2022
;
; DirectX files:  needed ?
;    DSETUP.dll, dsetup32.dll, DXSETUP.exe, dxupdate.cab
; and  (depending on which DX SDK was used when compiling OGRE):
;   Jun2010_D3DCompiler_43_x86.cab, Jun2010_d3dx*_43_x86.cab  both *_x64 too

;  paths
#define SR_Path   "D:\_\sr\_inst\"
;  test
;#define SR_Path   "D:\_\sr\_test\"
;  where to save installer
#define Out_Path  "D:\_\sr\"


#define SR_Name   "Stunt Rally"
#define SR_Exe    "\bin\Release\StuntRally3.exe"
#define SR_Editor "Track Editor"
#define SR_Ed_Exe "\bin\Release\SR-Editor3.exe"

#define SR_Version   "3.3"
#define SR_Publisher "Crystal Hammer"
#define SR_Url       "https://cryham.org/stuntrally/"

#define SR_Donate    "Donations"
#define SR_Docs      "Documentation"
#define SR_UrlDonate "https://cryham.org/donate/"
#define SR_UrlDocs   "https://github.com/stuntrally/stuntrally3/blob/main/docs/_menu.md"


[Setup]
; unique GUID that identifies this application. 
AppId={{6559F850-6DDD-4212-873C-1D35DC99A74E}
AppName={#SR_Name}
AppVersion={#SR_Version}
;AppVerName={#SR_Name} {#SR_Version}
AppPublisher={#SR_Publisher}
AppPublisherURL={#SR_Url}
AppSupportURL={#SR_Url}
AppUpdatesURL={#SR_Url}

DefaultDirName={autopf}\{#SR_Name} {#SR_Version}
; "ArchitecturesAllowed=x64compatible" specifies that Setup cannot run on anything but x64 and Windows 11 on Arm.
ArchitecturesAllowed=x64compatible
; "ArchitecturesInstallIn64BitMode=x64compatible" requests that the install be done in "64-bit mode" on x64 or Windows 11 on Arm,
; meaning it should use the native 64-bit Program Files directory and the 64-bit view of the registry.
ArchitecturesInstallIn64BitMode=x64compatible

LicenseFile={#SR_Path}License.txt
DisableProgramGroupPage=yes
; start menu  subdir:
DefaultGroupName={#SR_Name} {#SR_Version}

; Uncomment the following line to run in non administrative install mode (install for current user only.)
;PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog

SetupIconFile={#SR_Path}data\gui\stuntrally.ico
WizardStyle=modern
;WizardImageFile bmp

Compression=lzma
SolidCompression=yes
;Compression=lzma2/ultra64
;InternalCompressLevel=ultra
CompressionThreads=4

DiskSpanning=yes
SlicesPerDisk=1
DiskSliceSize=1051428580
;DiskSliceSize=1073741824

OutputDir={#Out_Path}
OutputBaseFilename={#SR_Name}-{#SR_Version}-installer


[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "armenian"; MessagesFile: "compiler:Languages\Armenian.isl"
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "bulgarian"; MessagesFile: "compiler:Languages\Bulgarian.isl"
Name: "catalan"; MessagesFile: "compiler:Languages\Catalan.isl"
Name: "corsican"; MessagesFile: "compiler:Languages\Corsican.isl"
Name: "czech"; MessagesFile: "compiler:Languages\Czech.isl"
Name: "danish"; MessagesFile: "compiler:Languages\Danish.isl"
Name: "dutch"; MessagesFile: "compiler:Languages\Dutch.isl"
Name: "finnish"; MessagesFile: "compiler:Languages\Finnish.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
Name: "hebrew"; MessagesFile: "compiler:Languages\Hebrew.isl"
Name: "hungarian"; MessagesFile: "compiler:Languages\Hungarian.isl"
Name: "icelandic"; MessagesFile: "compiler:Languages\Icelandic.isl"
Name: "italian"; MessagesFile: "compiler:Languages\Italian.isl"
Name: "japanese"; MessagesFile: "compiler:Languages\Japanese.isl"
Name: "korean"; MessagesFile: "compiler:Languages\Korean.isl"
Name: "norwegian"; MessagesFile: "compiler:Languages\Norwegian.isl"
Name: "polish"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "portuguese"; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "slovak"; MessagesFile: "compiler:Languages\Slovak.isl"
Name: "slovenian"; MessagesFile: "compiler:Languages\Slovenian.isl"
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "turkish"; MessagesFile: "compiler:Languages\Turkish.isl"
Name: "ukrainian"; MessagesFile: "compiler:Languages\Ukrainian.isl"


[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#SR_Path}*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs


[Icons]
Name: "{group}\{#SR_Name}"; Filename: "{app}{#SR_Exe}"
Name: "{group}\{#SR_Editor}"; Filename: "{app}{#SR_Ed_Exe}"

Name: "{group}\{cm:ProgramOnTheWeb,{#SR_Name}}"; Filename: "{#SR_Url}"
Name: "{group}\{#SR_Donate}"; Filename: "{#SR_UrlDonate}"
Name: "{group}\{#SR_Docs}"; Filename: "{#SR_UrlDocs}"
Name: "{group}\{cm:UninstallProgram,{#SR_Name}}"; Filename: "{uninstallexe}"

Name: "{autodesktop}\{#SR_Name}"; Filename: "{app}{#SR_Exe}"; Tasks: desktopicon
Name: "{autodesktop}\{#SR_Editor}"; Filename: "{app}{#SR_Ed_Exe}"; Tasks: desktopicon


[Run]
Filename: "{app}\redist\dxsetup.exe"; Parameters: "/silent"; Description: "{cm:LaunchProgram,DirectX redist}";
Filename: "{app}\redist\VC_redist.x86.exe"; Parameters: "/install /quiet /norestart"; Description: "{cm:LaunchProgram,VC x86 redist}";
Filename: "{app}\redist\VC_redist.x64.exe"; Parameters: "/install /quiet /norestart"; Description: "{cm:LaunchProgram,VC x64 redist}";
Filename: "{app}{#SR_Exe}"; Description: "{cm:LaunchProgram,{#StringChange(SR_Name, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

