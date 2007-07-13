OutFile "../neverball-${VERSION}-setup.exe"
Name "Neverball ${VERSION}"
LicenseData "COPYING.txt"
InstallDir "$PROGRAMFILES\Neverball"

XPStyle on

Page license
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section "Neverball/Neverputt"
    SectionIn RO

    SetOutPath $INSTDIR

    File *.txt
    File /oname=AUTHORS.txt doc\AUTHORS.txt
    File /oname=MANUAL.txt doc\MANUAL.txt

    File /r /x .svn /x *.map /x obj data
    File /r locale

    File neverball.exe neverputt.exe *.dll

    File /oname=data\icon\neverball.ico dist\newneverball.ico
    File /oname=data\icon\neverputt.ico dist\newneverputt.ico

    WriteUninstaller $INSTDIR\uninstall.exe
SectionEnd

Section "Mapping tools (compiler, maps, ...)"
    SetOutPath $INSTDIR

    File mapc.exe

    SetOutPath "$INSTDIR\data"

    File /r /x ".svn" data\*.map
    File /r /x ".svn" data\obj
SectionEnd

Section "Create Start menu shortcuts"
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

Section "Uninstall"
    Delete "$SMPROGRAMS\Games\Neverball.lnk"
    Delete "$SMPROGRAMS\Games\Neverputt.lnk"
    RMDir  "$SMPROGRAMS\Games"

    # FIXME:  unsafe if the directory contains other-than-installed stuff
    RMDir /r $INSTDIR
SectionEnd

