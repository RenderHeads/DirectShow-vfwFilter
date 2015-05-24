; Installation Script (C) 2015 RenderHeads Ltd.  All Rights Reserved.
; ________________________________________________________________________

!define     PRODUCTNAME         "VFW DirectShow Filter"
!define     SHORTVERSION        "1.0.0"
!define     TITLE		"${PRODUCTNAME} ${SHORTVERSION}"

SetCompressor /Solid lzma
RequestExecutionLevel admin

InstallDir "$PROGRAMFILES\${PRODUCTNAME}"

!include x64.nsh

Function .onInit
	UserInfo::GetAccountType
	pop $0
	${If} $0 != "admin" ;Require admin rights on NT4+
	    MessageBox mb_iconstop "Administrator rights required!"
	    SetErrorLevel 740 ;ERROR_ELEVATION_REQUIRED
	    Quit
	${EndIf}
FunctionEnd

Name "${TITLE}"
Caption "${TITLE}"

OutFile "VFWFilterSetup.exe"

XPStyle on


; _____________________________
; Install Pages
;

PageEx instfiles
    Caption " - Installing"
PageExEnd

; ____________________________
; Program Files Operations
;

Section "FilesInstall"
	SetShellVarContext all
	SetOutPath $INSTDIR\x64
	; 64-bit install
	${if} ${RunningX64}
		File "x64\vfwWrapper.dll"
		ExecWait 'regsvr32.exe /s "$OUTDIR\vfwWrapper.dll"'
	${else}
	${endif}

	SetOutPath $INSTDIR\x86
	File "x86\vfwWrapper.dll"
	ExecWait 'regsvr32.exe /s "$OUTDIR\vfwWrapper.dll"'

SectionEnd


; _____________________________
; Registry Operations
;

Section "Registry"
	;MessageBox MB_OK "registry"
	SetOutPath $INSTDIR

	; Write the uninstall keys for Windows
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCTNAME}" "DisplayName" "${PRODUCTNAME}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCTNAME}" "UninstallString" '"$INSTDIR\uninstall.exe"'
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCTNAME}" "Publisher" 'RenderHeads Ltd'
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCTNAME}" "DisplayVersion" "1.0.0"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCTNAME}" "Version" 0x01000000
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCTNAME}" "VersionMajor" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCTNAME}" "VersionMinor" 0
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCTNAME}" "EstimatedSize" 1024
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCTNAME}" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCTNAME}" "NoRepair" 1
   	WriteUninstaller "$INSTDIR\uninstall.exe"

	CreateDirectory "$SMPROGRAMS\${PRODUCTNAME}"
	CreateShortCut "$SMPROGRAMS\${PRODUCTNAME}\Uninstall.lnk" "$INSTDIR\uninstall.exe"
SectionEnd

; _______________________
; Uninstall
;

UninstPage uninstConfirm
UninstPage instfiles

Section "Uninstall"

	SetShellVarContext all

	${if} ${RunningX64}	
		ExecWait 'regsvr32.exe /u /s "$INSTDIR\x64\vfwWrapper.dll"'
	${else}
	${endif}
	ExecWait 'regsvr32.exe /u /s "$INSTDIR\x86\vfwWrapper.dll"'

	; remove registry keys
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCTNAME}"

    ; remove the links from the start menu
    RMDir /r "$SMPROGRAMS\${PRODUCTNAME}"

    RMDir /r "$INSTDIR"	
SectionEnd