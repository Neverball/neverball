#  NOTE:  this script doesn't do anything useful yet.  it will, some day.

OutFile "neverball-${VERSION}-setup.exe"

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
    File /r /x ".svn" /x "*.map" data
    File /r /x ".svn" locale
    File /r *.exe *.dll

    WriteUninstaller $INSTDIR\uninstaller.exe
SectionEnd

Section "Uninstall"
    RMDir /r $INSTDIR
SectionEnd

