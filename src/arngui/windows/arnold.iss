[Setup]
AppName=Arnold Emulator
AppId={{8A04F14E-DB96-4DDE-B6BF-002B6866DB14}
AppVersion=1.1
VersionInfoVersion=1.1
DefaultDirName={pf}\Arnold Emulator
DefaultGroupName=Arnold Emulator
UninstallDisplayIcon={app}\arnold.exe
Compression=lzma2
SolidCompression=yes
OutputDir=userdocs:ArnoldInstaller
AppCopyright=Copyright (C) 2017 Kevin Thacker
AppPublisher=Kevin Thacker
AppPublisherURL=http://www.cpctech.org.uk
SourceDir="../../../exe/Release/arnold"
RestartIfNeededByRun=no

[Files]
Source: "../../../src/arngui/windows/vcredist_x86.exe"; DestDir: {tmp}; Flags: deleteafterinstall
Source: "../../../changes.txt"; DestDir: "{app}"
Source: "arnold.exe"; DestDir: "{app}"
Source: "arnlogo.png"; DestDir: "{app}"
Source: "autostart.png"; DestDir: "{app}"
Source: "breakpoint.png"; DestDir: "{app}"
Source: "cart.png"; DestDir: "{app}"
Source: "cass.png"; DestDir: "{app}"
Source: "current.png"; DestDir: "{app}"
Source: "disk.png"; DestDir: "{app}"
Source: "debugger.png"; DestDir: "{app}"
Source: "disabledbreakpoint.png"; DestDir: "{app}"
Source: "rom.png"; DestDir: "{app}"
Source: "snapshotload.png"; DestDir: "{app}"
Source: "snapshotsave.png"; DestDir: "{app}"
Source: "roms.zip"; DestDir: "{app}"
Source: "GUIFrame.xrc"; DestDir: "{app}"
Source: "icons.dll"; DestDir: "{app}"
Source: "SDL2.dll"; DestDir: "{app}"
Source: "SDL2_image.dll"; DestDir: "{app}"
Source: "SDL2_ttf.dll"; DestDir: "{app}"
Source: "wxbase31u_net_vc_custom.dll"; DestDir: "{app}"
Source: "wxbase31u_vc_custom.dll"; DestDir: "{app}"
Source: "wxbase31u_xml_vc_custom.dll"; DestDir: "{app}"
Source: "wxmsw31u_adv_vc_custom.dll"; DestDir: "{app}"
Source: "wxmsw31u_aui_vc_custom.dll"; DestDir: "{app}"
Source: "wxmsw31u_core_vc_custom.dll"; DestDir: "{app}"
Source: "wxmsw31u_gl_vc_custom.dll"; DestDir: "{app}"
Source: "wxmsw31u_html_vc_custom.dll"; DestDir: "{app}"
Source: "wxmsw31u_media_vc_custom.dll"; DestDir: "{app}"
Source: "wxmsw31u_propgrid_vc_custom.dll"; DestDir: "{app}"
Source: "wxmsw31u_qa_vc_custom.dll"; DestDir: "{app}"
Source: "wxmsw31u_ribbon_vc_custom.dll"; DestDir: "{app}"
Source: "wxmsw31u_richtext_vc_custom.dll"; DestDir: "{app}"
Source: "wxmsw31u_stc_vc_custom.dll"; DestDir: "{app}"
Source: "wxmsw31u_webview_vc_custom.dll"; DestDir: "{app}"
Source: "wxmsw31u_xrc_vc_custom.dll"; DestDir: "{app}"
Source: "zlib1.dll"; DestDir: "{app}"

[Run]
Filename: "{tmp}\vcredist_x86.exe"; Parameters: "/q /norestart"

[Icons]
Name: "{group}\Arnold emulator"; Filename: "{app}\arnold.exe"
