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
 ' independent. But the compiler is being ported to other platforms and is
 ' on its way to becoming a GCC front-end. Once the second task is done,
 ' this program will be cross-platform. FreeBASIC maintains close syntax
 ' across its platform. Code made in Windows doesn't need many (if any)
 ' changes to be compatible in *nix.
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
 ' This includes the generator.
 '/
#include "nevergen.bas" 

/'
 ' This initiates graphics mode. It is a 640x480 8-bit program with one page
 ' and you can't fullscreen it (unless windowed mode fails).
 '/
screen 18,8,1,GFX_NO_SWITCH

AssistDir = curdir
config(1)

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
Appdata = "C:\Documents and Settings\" + Username + "\Application Data\"
#ENDIF
if Neverpath = "" then
    print "[Y/N] Is this correct for usage with Neverball?"
    print curdir
    do
        sleep
        InType = lcase(inkey)
        if InType = "n" then
            cls
            print "Path can be relative or absolute."
            input "Where is the path";NeverPath
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
        print "A crash has occured within the assistant."
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
        print "What would you like to do?"
        color 15
        print "Run the ";:color 10:print "N";:color 15:print "everball game"
        print "Load a Neverball ";:color 10:print "r";:color 15:print "eplay"
        print "Run the Never";:color 10:print "p";:color 15:print "utt game"
        if MediumClear = 0 then
            color 8:print "Generate a ";:color 2:print "m";:color 8:print "ap";
            color 11:print " - Must clear Neverball Medium"
            color 15
        else
            print "Generate a ";:color 10:print "m";:color 15:print "ap"
        end if
        color 10:print "C";:color 15:print "ompile a map"

        #IFDEF __FB_WIN32__
        color 15:print "Locate 7-";:color 10:print "Z";:color 15:print "ip"
        if (Z7Path = "") OR (Z7Exe = "") then
            color 2
            print "U";:color 8:print "npack an archive";
            color 11:print " - Must locate 7-Zip"
        else
            color 10:print "U";:color 15:print "npack an archive";
            color 11:print " - Will overwrite without prompt"
        end if
        #ELSE
        color 8:print "Locate 7-";:color 2:print "Z";:color 8:print "ip";
        color 11:print " - 7-Zip is Windows only"
        color 2:print "U";:color 8:print "npack an archive";
        color 11:print " - 7-Zip is Windows only"
        #ENDIF

        color 15:print "E";:color 10:print "x";:color 15:print "it program"
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
            print "Exit code: "& Check
            sleep
        elseif multikey(SC_P) then
            clkey
            windowtitle "Neverassistant - Running Neverputt"
            #IFDEF __FB_WIN32__
            Check = exec("Neverputt.exe","")
            #ELSE
            Check = exec("neverputt","")
            #ENDIF
            print "Exit code: "& Check
            sleep

        elseif multikey(SC_C) then
            clkey
            windowtitle "Neverassistant - Compiling map"
            input "Which map do you want to compile? (without .map) ",Compile
            if (Compile < > "") then
                Compile = Compile + ".map"
                #IFDEF __FB_WIN32__
                Check = exec("Mapc.exe",Compile + " data")
                #ELSE
                Check = exec("mapc",Compile + " data")
                #ENDIF
                color 15
                print "Exit code: "& Check
                sleep
            end if

        elseif multikey(SC_R) then
            clkey
            windowtitle "Neverassistant - Load replay"
            input "Which replay do you want to watch? (without .nbr) ",Replay
            if (Replay < > "") then
                Replay = Replay + ".nbr"
                #IFDEF __FB_WIN32__
                Check = exec("Neverball.exe","-r " + Appdata + Replay)
                #ELSE
                Check = exec("neverball","-r data/.neverball-dev/" + Replay)
                #ENDIF
                color 15
                print "Exit code: "& Check
                sleep
            end if

        #IFDEF __FB_WIN32__
        elseif multikey(SC_Z) then
            clkey
            windowtitle "Neverassistant - Locate 7-Zip"
            print "Where do you want to locate 7-Zip? " + _
                  " (without \7z.exe, but where the 7z.exe is)"
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
            print "Which archive do you want to extract? (include extension)"
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