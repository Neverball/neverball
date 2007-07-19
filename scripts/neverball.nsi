#------------------------------------------------------------------------------

!include "LogicLib.nsh"

#------------------------------------------------------------------------------

!ifndef VERSION
!define VERSION "unknown"
!endif

!ifndef OUTFILE
!define OUTFILE "../neverball-${VERSION}-setup.exe"
!endif

!define UNINSTALL_REG_ROOT \
    "Software\Microsoft\Windows\CurrentVersion\Uninstall\Neverball"

#------------------------------------------------------------------------------

OutFile ${OUTFILE}
Name "Neverball ${VERSION}"
LicenseData "COPYING.txt"
InstallDir "$PROGRAMFILES\Neverball"

XPStyle on

#------------------------------------------------------------------------------

Page license
Page components
Page directory
Page instfiles

Function .onInit
    Call IsUserAdmin
    Pop $R0

    ${If} $R0 == "true"
        SetShellVarContext all
    ${EndIf}
FunctionEnd

Section "Neverball/Neverputt"
    SectionIn RO

    SetOutPath $INSTDIR

    File *.txt
    File /oname=AUTHORS.txt doc\AUTHORS.txt
    File /oname=MANUAL.txt doc\MANUAL.txt

    File /r /x .svn /x *.map /x obj data
    File /r locale

    File neverball.exe neverputt.exe *.dll

    File /oname=data\icon\neverball.ico dist\ico\neverball_basic_sizes.ico
    File /oname=data\icon\neverputt.ico dist\ico\neverputt_basic_sizes.ico

    WriteUninstaller $INSTDIR\uninstall.exe


    # http://nsis.sourceforge.net/Add_uninstall_information_to_Add/Remove_Programs

    WriteRegStr SHELL_CONTEXT ${UNINSTALL_REG_ROOT} \
        "DisplayName" "Neverball ${VERSION}"
    WriteRegStr SHELL_CONTEXT ${UNINSTALL_REG_ROOT} \
        "DisplayVersion" "${VERSION}"
    WriteRegStr SHELL_CONTEXT ${UNINSTALL_REG_ROOT} \
        "DisplayIcon" "$INSTDIR\data\icon\neverball.ico"
    WriteRegStr SHELL_CONTEXT ${UNINSTALL_REG_ROOT} \
        "UninstallString" "$INSTDIR\uninstall.exe"
    WriteRegStr SHELL_CONTEXT ${UNINSTALL_REG_ROOT} \
        "URLInfoAbout" "http://www.neverball.org/"
    WriteRegStr SHELL_CONTEXT ${UNINSTALL_REG_ROOT} \
        "URLUpdateInfo" "http://www.neverball.org/"
    WriteRegStr SHELL_CONTEXT ${UNINSTALL_REG_ROOT} \
        "HelpLink" "http://www.nevercorner.net/"
    WriteRegDWORD SHELL_CONTEXT ${UNINSTALL_REG_ROOT} \
        "NoModify" 1
    WriteRegDWORD SHELL_CONTEXT ${UNINSTALL_REG_ROOT} \
        "NoRepair" 1
SectionEnd

Section "Mapping tools (compiler, maps, ...)"
    SetOutPath $INSTDIR

    File mapc.exe

    SetOutPath "$INSTDIR\data"

    File /r /x ".svn" data\*.map
    File /r /x ".svn" data\obj
SectionEnd

SectionGroup "Create shortcuts"
    Section "In Start menu"
        # Reset to get a proper working directory for short-cuts
        SetOutPath $INSTDIR
    
        CreateDirectory "$SMPROGRAMS\Games"
    
        CreateShortcut \
            "$SMPROGRAMS\Games\Neverball.lnk" \
            "$INSTDIR\neverball.exe" ""       \
            "$INSTDIR\data\icon\neverball.ico"
    
        CreateShortcut \
            "$SMPROGRAMS\Games\Neverputt.lnk" \
            "$INSTDIR\neverputt.exe" ""       \
            "$INSTDIR\data\icon\neverputt.ico"
    SectionEnd

    Section "On desktop"
        # Reset to get a proper working directory for short-cuts
        SetOutPath $INSTDIR
    
        CreateShortcut \
            "$DESKTOP\Neverball.lnk"          \
            "$INSTDIR\neverball.exe" ""       \
            "$INSTDIR\data\icon\neverball.ico"
    
        CreateShortcut \
            "$DESKTOP\Neverputt.lnk"          \
            "$INSTDIR\neverputt.exe" ""       \
            "$INSTDIR\data\icon\neverputt.ico"
    SectionEnd
SectionGroupEnd

#------------------------------------------------------------------------------

UninstPage uninstConfirm
UninstPage instfiles

Function un.onInit
    Call un.IsUserAdmin
    Pop $R0

    ${If} $R0 == "true"
        SetShellVarContext all
    ${EndIf}
FunctionEnd

Section "Uninstall"
    Delete "$SMPROGRAMS\Games\Neverball.lnk"
    Delete "$SMPROGRAMS\Games\Neverputt.lnk"
    RMDir  "$SMPROGRAMS\Games"
    Delete "$DESKTOP\Neverball.lnk"
    Delete "$DESKTOP\Neverputt.lnk"

    DeleteRegKey SHELL_CONTEXT ${UNINSTALL_REG_ROOT}

    # FIXME:  unsafe if the directory contains other-than-installed stuff
    RMDir /r $INSTDIR
SectionEnd

#------------------------------------------------------------------------------

# URL:    http://nsis.sourceforge.net/IsUserAdmin
# Author: Lilla (lilla@earthlink.net) 2003-06-13

!macro IsUserAdmin un
Function ${un}IsUserAdmin
    Push $R0
    Push $R1
    Push $R2
 
    ClearErrors
    UserInfo::GetName
    IfErrors Win9x
    Pop $R1
    UserInfo::GetAccountType
    Pop $R2
 
    StrCmp $R2 "Admin" 0 Continue
    StrCpy $R0 "true"
    Goto Done
 
    Continue:
        StrCmp $R2 "" Win9x
        StrCpy $R0 "false"
        Goto Done
 
    Win9x:
        StrCpy $R0 "true"
 
    Done:

    Pop $R2
    Pop $R1
    Exch $R0
FunctionEnd
!macroend

!insertmacro IsUserAdmin ""
!insertmacro IsUserAdmin "un."

#------------------------------------------------------------------------------

# vim:sts=4:sw=4:et:nowrap:
