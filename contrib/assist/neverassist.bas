/'
 ' All of this code is made by Challenge Space Yard.
 '
 ' If you haven't already noticed, this program is written in a language other
 ' than C. In fact, this program is written in FreeBASIC. You actually need a
 ' different compiler (and I'm assuming the GCC doesn't already support this).
 ' Fortunately, the FreeBASIC compiler is available under the GPL.
 '
 ' Go to www.freebasic.net for the compiler (and optionally source).
 '
 ' Once you have the FBC's path into the shell, you can simply invoke
   fbc "neverassist.bas" [-g] [-s gui]
 ' Assuming no errors, you should have a binary.
 '
 ' Under Windows, the -s gui removes the console. On *nix, this switch
 ' currently does nothing.
 '
 ' Due to incapabilities of the compiler, this program is NOT platform
 ' independent. But the compiler is being ported to other platforms and is on
 ' its way to becoming a GCC front-end. Once the second task is done, this
 ' program will be on its way to being cross-platform. FreeBASIC maintains
 ' close syntax across across its platforms. Code made in Windows doesn't
 ' need many (if any) changes to be compatible in *nix. There was even an Xbox
 ' port of the compiler. Unfortunately, changes to the compiler has done
 ' broken backward compatibility, and compatibility has not yet been restored
 ' as of last check in. In case you're wondering, the port was made using
 ' OpenXDK. A BSD port is under way to being ported to, so this assistant will
 ' be compatible with BSD systems once the port becomes stable.
 '
 ' As with Neverball and Neverputt, you can still apply the -g switch to
 ' enable debugging and use the GDB (or a compatible program) to debug it.
 '
 ' Although parts of the program are still out of alignment with the standards
 ' set forth by rlk, this one works a lot better than what it was before it was
 ' released.
 '/
#IFDEF __FB_DOS__
#ERROR "Not for DOS." 'This prevents it from being compiled for DOS.
#ELSE
windowtitle "Neverassistant"
#DEFINE __SATISFY__ 'This forces this file to be chosen for compilation.
/'
 ' This includes all the contents of the FB Graphics Library and its constants.
 ' This file is part of the compiler. We don't need it in this package.
 '
 ' The namespace FB is utilized to allow actions under it to be used without
 ' prefixing it.
 '/
#include "fbgfx.bi"
using FB
/'
 ' This includes a lot of the much needed information. This file is not part of
 ' the compiler, and even if it was part of the compiler, its presence in this
 ' package overrides its presence in the compiler.
 '/
#include "neverassist.bi"
/'
 ' This establishes compatibility with VisualBASIC by including compatibility
 ' functions and constants. This file is part of the compiler. We don't need
 ' it in this package.
 '/
#include "vbcompat.bi"
/'
 ' This includes the generator. This file is not part of the compiler. It must
 ' be present in this package.
 '/
#include "nevergen.bas"

/'
 ' This initiates graphics mode. It is a 640x480 8-bit program with one page
 ' and you can't fullscreen it (unless windowed mode fails).
 '/
screen 18,8,1,GFX_NO_SWITCH

AssistDir = curdir
config(1)
inilang

/'
 ' This define is actually related to the system you're on.
 ' If you're on Windows, __FB_WIN32__ is automatically defined.
 ' If you're on DOS, __FB_DOS__ is automatically defined.
 ' If you're on Linux or GNU/Linux, __FB_LINUX__ is automatically defined.
 '/
#IFDEF __FB_WIN32__
'Incomplete: Attempt to get username and use the user folder.
dim shared as string Username, AppData
Username = ""
Appdata = lang("C:\Documents and Settings\") + Username + lang("\Application Data\")
#ENDIF
if Neverpath = "" then
    print lang("[Y/N] Is this correct for usage with Neverball?")
    print curdir
    do
        sleep
        InType = lcase(inkey)
        if InType = "n" then
            cls
            print lang("Path can be relative or absolute.")
            print lang("Where is the path? ");
            line input "",NeverPath
            chdir(NeverPath)
            config
            exit do
        elseif InType = "y" then
            NeverPath = "."
            config
            exit do
        end if
    loop
else
    chdir(NeverPath)
end if
menu

/'
 ' This sometimes will catch the errors of the program and notify the user
 ' that such an event occured. Subroutines which are destructors (or
 ' constructors) don't need to be declared.
 '/
sub catcher destructor
    if Perfect = 0 then
        print lang("A crash has occured within the assistant.")
        sleep
        end
    end if
end sub

sub menu
    if (MaxMoney < 50) then MaxMoney = 50
    clkey
    do
        Title = "Neverassistant - " + Time + " - " + Date
        windowtitle Title

        screenlock
        cls
        color 14
        print lang("What would you like to do?")
        color 15
        print "<N> ";lang("Run the Neverball game")
        print "<R> ";lang("Load a Neverball replay")
        print "<P> ";lang("Run the Neverputt game")
        if MediumClear = 0 then
            color 8:print "<M> ";lang("Generate a map");
            color 11:print lang(" - Must clear Neverball Medium")
            color 15
        else
            print "<M> ";lang("Generate a map")
        end if
        print "<C> ";lang("Compile a map")
        print "<D> ";lang("Relocate Directory")
        if LangFile = "" OR LangFile = "en.txt" then
            print "<L> Change language"
        else
            print "<L> ";lang("Change language");" (Change language)"
        endif

        #IFDEF __FB_WIN32__
        print "<Z> ";lang("Locate 7-Zip")
        if (Z7Path = "") OR (Z7Exe = "") then
            color 8
            print "<U> ";lang("Unpack an archive");
            color 11:print lang(" - Must locate 7-Zip")
        else
            color 15:print "<U> ";lang("Unpack an archive");
            color 11:print lang(" - Will overwrite without prompt")
        end if
        #ELSE
        color 8:print "<Z> ";lang("Locate 7-Zip");
        color 11:print lang(" - 7-Zip is Windows only")
        color 8:print "<U> ";lang("Unpack an archive");
        color 11:print lang(" - 7-Zip is Windows only")
        #ENDIF

        color 15:print "<X> ";lang("Exit program")
        print
        screenunlock
        sleep 20

        if multikey(SC_M) AND (MediumClear > 0) then
            map_generate
        elseif multikey(SC_N) then
            clkey
            windowtitle "Neverassistant - Running Neverball"
            #IFDEF __FB_WIN32__
            Check = exec("Neverball.exe","")
            #ELSE
            Check = exec("neverball","")
            #ENDIF
            print lang("Exit code: ")& Check
            sleep
        elseif multikey(SC_P) then
            clkey
            windowtitle "Neverassistant - Running Neverputt"
            #IFDEF __FB_WIN32__
            Check = exec("Neverputt.exe","")
            #ELSE
            Check = exec("neverputt","")
            #ENDIF
            print lang("Exit code: ")& Check
            sleep

        elseif multikey(SC_C) then
            clkey
            windowtitle "Neverassistant - Compiling map"
            print lang("Which map do you want to compile?") + _
                  lang(" (without .map)")
            input "",Compile
            if (Compile < > "") then
                Compile = Compile + ".map"
                #IFDEF __FB_WIN32__
                Check = exec("Mapc.exe",Compile + " data")
                #ELSE
                Check = exec("mapc",Compile + " data")
                #ENDIF
                color 15
                print lang("Exit code: ")& Check
                sleep
            end if

        elseif multikey(SC_R) then
            clkey
            windowtitle "Neverassistant - Load replay"
            print lang("Which replay do you want to watch?") + _
                  lang(" (without .nbr)")
            input "",Replay
            if (Replay < > "") then
                Replay = Replay + ".nbr"
                #IFDEF __FB_WIN32__
                Check = exec("Neverball.exe","-r " + Appdata + Replay)
                #ELSE
                Check = exec("neverball","-r data/.neverball-dev/" + Replay)
                #ENDIF
                color 15
                print lang("Exit code: ")& Check
                sleep
            end if
        elseif multikey(SC_D) then
            clkey
            windowtitle "Neverassistant - Relocate Directory"
            print lang("Path can be relative or absolute.")
            print lang("If relative, must be relative to old directory.")
            print lang("Where is the directory? ");
            line input "",NeverPath
            chdir(NeverPath)
            config
            color 15
        elseif multikey(SC_L) then
            clkey
            windowtitle "Neverassistant - Change Language"
            lang_select
            config
            color 15

        #IFDEF __FB_WIN32__
        elseif multikey(SC_Z) then
            clkey
            windowtitle "Neverassistant - Locate 7-Zip"
            print lang("Where do you want to locate 7-Zip?") + _
                  lang(" (without \7z.exe, but where the 7z.exe is)")
            color 44
            input "",Z7Path
            if (Z7Path < > "") then
                Z7Exe = Z7Path + "\7z.exe"
                Check = exec(Z7Exe,"")
                if (Check = -1) then
                    config(1)
                else
                    config
                end if
            end if
            color 15

        elseif multikey(SC_U) AND (Z7Path < > "") AND (Z7Exe < > "") then
            clkey
            windowtitle "Neverassistant - Unpack archive"
            print lang("Which archive do you want to extract?") + _
                  lang(" (include extension)")
            color 44
            input Unpack
            if (Unpack < > "") then
                Z7Exe = Z7Path + "\7z.exe"
                Check = exec(Z7Exe,"x -w{" +chr(34)+curdir+chr(34)+ "} -y " + _
                             Unpack)
                if (Check = -1) then
                    Z7Path = ""
                    Z7Exe = ""
                    config
                    sleep 2000
                end if
            end if
            color 15
        #ENDIF

        elseif multikey(SC_H) then
            'Force crash function.
            exit do
        elseif multikey(SC_X) OR multikey(SC_ESCAPE) OR _
               inkey = chr(255)+"k" then
            Perfect = 1
            end 0
        end if
    loop
end sub