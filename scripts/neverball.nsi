OutFile "../neverball-${VERSION}-setup.exe"
Name "Neverball ${VERSION}"
LicenseData "COPYING.txt"
InstallDir "$PROGRAMFILES\Neverball"

XPStyle on

Page license
#Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section
    SetOutPath $INSTDIR

    File *.txt
    File /oname=AUTHORS.txt doc/AUTHORS.txt
    File /oname=MANUAL.txt doc/MANUAL.txt

    #File /r /x ".svn" /x "*.map" data
    File /r /x ".svn" data
    File /r /x ".svn" locale

    File *.exe *.dll

    WriteUninstaller $INSTDIR\uninstall.exe
SectionEnd

Section "Uninstall"
    RMDir /r $INSTDIR
SectionEnd

