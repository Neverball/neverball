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
   fbc neverassist.bas [-g] [-s gui]
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
 ' need many (if any) changes to be compatible in *nix. BSD and Mac OS X ports
 ' are under way, so this assistant will be compatible with BSD and Mac OS X
 ' systems once the ports become stable.
 '
 ' As with Neverball and Neverputt, you can still apply the -g switch to
 ' enable debugging and use the GDB (or a compatible program) to debug it.
 '
 ' Although parts of the program are still out of alignment with the standards
 ' set forth by rlk, this one works a lot better than what it was before it was
 ' released.
 '
 ' Libraries needed for this program (2): 
 ' SDL (headers inclded in compiler)
 ' SDL_mixer (headers included in compiler)
 '/

#IFDEF __FB_DOS__
    #ERROR "Not for DOS." 'This prevents it from being compiled for DOS.
#ELSE
    windowtitle "Neverassistant"
    #DEFINE __SATISFY__ 'This forces this file to be chosen for compilation.
    /'
     ' This includes all the contents of the FB Graphics Library and its
     ' constants. This file is part of the compiler. We don't need it in this
     ' package.
     '
     ' The namespace FB is utilized to allow actions under it to be used
     ' without prefixing it.
     '/
    #include "fbgfx.bi"
    using FB
    /'
     ' This establishes compatibility with VisualBASIC by including
     ' compatibility functions and constants. This file is part of the
     ' compiler. We don't need it in this package.
     '/
    #include "vbcompat.bi"

    /'
     ' This includes a lot of the much needed information. This file is not part
     ' of the compiler, and even if it was part of the compiler, its presence in
     ' this package overrides its presence in the compiler.
     '/
    #include "neverassist.bi"
    /'
     ' This includes the generator. This file is not part of the compiler. It
     ' must be present in this package.
     '/
    #include "nevergen.bas"

    /'
     ' This initiates graphics mode. It is a 640x480 24-bit program.
     '/
    screen 18,24

    AssistDir = curdir
    config(1)
    inilang

    /'
     ' This define is actually related to the system you're on.
     ' If you're on Windows, __FB_WIN32__ is automatically defined.
     ' If you're on DOS, __FB_DOS__ is automatically defined.
     ' If you're on Linux or GNU/Linux, __FB_LINUX__ is automatically defined.
     '/

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

    #IF __FB_DEBUG__
        #IF defined(__FB_WIN32__) OR defined(__FB_DOS__)
            if FileExists("gdb.exe") then
                print lang("GDB detected. Do you want to use the GDB? (Y/N)")
                print lang("Hit N if you have it open or don't want to use it.")
                do
                    sleep 1
                    if multikey(SC_Y) then
                        run("gdb.exe","neverassist.exe")
                        Mix_CloseAudio
                        SDL_Quit
                        end 0
                    end if
                loop until multikey(SC_N)
            end if
        #ElSE
            if FileExists("gdb") then
                print lang("GDB detected. Do you want to use the GDB? (Y/N)")
                print lang("Hit N if you have it open or don't want to use it.")
                do
                    sleep 1
                    if multikey(SC_Y) then
                        run("gdb","neverassist")
                        Mix_CloseAudio
                        SDL_Quit
                        end 0
                    end if
                loop until multikey(SC_N)
            end if
        #ENDIF
    #ENDIF
    main_menu
    Mix_CloseAudio
    SDL_Quit

    sub main_menu
        clkey
        do
            Title = "Neverassistant - " + left(Time,5) + " - " + Date
            windowtitle Title

            screenlock
            cls
            color rgb(255,255,0)
            print lang("What would you like to do?")
            color rgb(255,255,255)
            print "<N> ";lang("Run the Neverball game")
            print "<R> ";lang("Load a Neverball replay")
            print "<P> ";lang("Run the Neverputt game")
            print "<M> ";lang("Generate a map")
            print "<C> ";lang("Compile a map")
            print "<D> ";lang("Relocate Directory")
            if LangFile = "" OR LangFile = "en" OR LangFile = "en.txt" then
                print "<L> Change language"
            else
                print "<L> ";lang("Change language");" (Change language)"
            endif
            print "<S> ";lang("View solutions directory")
            print "<V> ";lang("Convert a level to a set with that level")

            color rgb(128,128,128)
            print "<U> ";lang("Unpack an archive");
            color rgb(0,255,255)
            print lang(" - Disabled for the time being")

            color rgb(255,255,255):print "<X> ";lang("Exit program")
            print
            screenunlock
            sleep 20

            if multikey(SC_M) then
                map_generate
            elseif multikey(SC_N) then
                sleep 300,1
                clkey
                windowtitle "Neverassistant - Running Neverball"
                screen 0
                #IFDEF __FB_WIN32__
                    Check = exec("Neverball.exe","")
                #ELSE
                    Check = exec("neverball","")
                #ENDIF
                screen 18,24
                print lang("Exit code: ")& Check
                clkey
                sleep
            elseif multikey(SC_P) then
                sleep 300,1
                clkey
                windowtitle "Neverassistant - Running Neverputt"
                screen 0
                #IFDEF __FB_WIN32__
                    Check = exec("Neverputt.exe","")
                #ELSE
                    Check = exec("neverputt","")
                #ENDIF
                screen 18,24
                print lang("Exit code: ")& Check
                clkey
                sleep

            elseif multikey(SC_C) then
                clkey
                windowtitle "Neverassistant - Compiling map"
                print lang("Which map do you want to compile?") + _
                      lang(" (without extension)")
                input "",Compile
                if (Compile < > "") then
                    screen 0
                    Compile = Compile + ".map"
                    #IFDEF __FB_WIN32__
                        Check = exec("Mapc.exe",Compile + " data")
                    #ELSE
                        Check = exec("mapc",Compile + " data")
                    #ENDIF
                    color rgb(255,255,255)
                    screen 18,24
                    print lang("Exit code: ")& Check
                    sleep
                end if

            elseif multikey(SC_R) then
                clkey
                windowtitle "Neverassistant - Load replay"
                print lang("Which replay do you want to watch?") + _
                      lang(" (without extension)")
                color rgb(255,255,0)
                input "",Replay
                color rgb(255,255,255)
                if (Replay < > "") then
                    screen 0
                    Replay = Replay + ".nbr"
                    #IFDEF __FB_WIN32__
                        Check = exec("Neverball.exe","-r " + Replay)
                    #ELSE
                        Check = exec("neverball","-r " + Replay)
                    #ENDIF
                    screen 18,24
                    print lang("Exit code: ")& Check
                    sleep
                end if

            elseif multikey(SC_D) then
                clkey
                windowtitle "Neverassistant - Relocate Directory"
                print lang("Path can be relative or absolute.")
                print lang("If relative, must be relative to" + _
                    " assistant's directory.")
                print lang("Where is the directory? ");
                color rgb(255,255,0)
                line input "",NeverPath
                chdir(AssistDir)
                chdir(NeverPath)
                config
                color rgb(255,255,255)
            elseif multikey(SC_L) then
                clkey
                windowtitle "Neverassistant - Change Language"
                lang_select
                config
                color rgb(255,255,255)

            elseif multikey(SC_S) then
                clkey
                cls
                /'
                 ' Try to make it wrap the 80 characters if you translate
                 ' this string.
                 '/
                print lang("The solution directory contains a collection" + _
                    " of solutions to levels players may give a fit on.")
                print "<A> ";lang("Neverball Easy Level 09: " + _
                    "How to 100% the level")
                print "<B> ";lang("Neverball Easy Bonus Level 5: " + _
                    "How to 100% the level")
                print lang("Hit BACKSPACE to exit the directory.")
                print
                do
                    sleep 10
                    if multikey(SC_A) then
                        screen 0
                        #IFDEF __FB_WIN32__
                            Check = exec("Neverball.exe", "-r " + _
                                         AssistDir+"/solutions/cE09csy.nbr")
                        #ELSE
                            Check = exec("neverball", "-r " + _
                                         AssistDir+"/solutions/cE09csy.nbr")
                        #ENDIF
                        screen 18,24
                        clkey
                        print lang("Exit code of replay A: ")& Check
                        sleep
                        exit do
                    elseif multikey(SC_B) then
                        screen 0
                        #IFDEF __FB_WIN32__
                            Check = exec("Neverball.exe", "-r " + _
                                         AssistDir+"/solutions/cEB5csy.nbr")
                        #ELSE
                            Check = exec("neverball", "-r " + _
                                         AssistDir+"/solutions/cEB5csy.nbr")
                        #ENDIF
                        screen 18,24
                        clkey
                        print lang("Exit code of replay B: ")& Check
                        sleep
                        exit do
                    end if
                loop until multikey(SC_BACKSPACE)

            elseif multikey(SC_V) then
                clkey
                print lang("Important: Make sure the files are in root " + _
                           "directory of Neverball.")
                print lang("If you don't have a levelshot when it asks " + _
                           "you for it, leave it blank!")
                print
                print lang("Which level to convert? ");
                input "",MapFile
                print lang("Where is its corrosponding levelshot?")
                input "",ShotFile
                /'
                 ' This creates a totally random ID. As it is random, and
                 ' with a huge range, it will very rarely (if ever) conflict
                 ' with another level.
                 '/
                SingleLevelID = rnd * 4294967295
                open "data/set-"+str(SingleLevelID)+".txt" for output as #1
                print #1, "Single Levels - ";SingleLevelID
                print #1, "A single level. That's it.\\\\"
                print #1, SingleLevelID
                if ShotFile = "" then
                    print #1, "shot-singles/Question.png"
                else
                    print #1, "shot-";SingleLevelID;"/";ShotFile
                end if
                print #1, ""
                print #1, "map-";SingleLevelID;"/";MapFile
                close #1
                mkdir("data/map-"+str(SingleLevelID))
                if ShotFile < > "" then
                    mkdir("data/shot-"+str(SingleLevelID))
                end if
                FileCopy(MapFile,"data/map-"+str(SingleLevelID)+"/"+MapFile)
                if ShotFile < > "" then
                    FileCopy(ShotFile,_
                    "data/shot-"+str(SingleLevelID)+"/"+ShotFile)
                end if
/'
            #IFDEF __FB_WIN32__
                elseif multikey(SC_U) then
                    clkey
                    windowtitle "Neverassistant - Unpack archive"
                    print lang("Which archive do you want to extract?") + _
                        lang(" (include extension)")
                    color rgb(255,255,0)
                    input Unpack
                    color rgb(255,255,255)
            #ENDIF
'/
            elseif multikey(SC_X) OR multikey(SC_ESCAPE) OR _
                   inkey = chr(255)+"k" then
                end 0
            end if
        loop
    end sub
#ENDIF