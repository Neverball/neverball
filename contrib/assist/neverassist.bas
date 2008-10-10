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
 ' as of last update. In case you're wondering, the port was made using
 ' OpenXDK. BSD and Mac OS X ports are under way, so this assistant will
 ' be compatible with BSD and Mac OS X systems once the ports become stable.
 '
 ' As with Neverball and Neverputt, you can still apply the -g switch to
 ' enable debugging and use the GDB (or a compatible program) to debug it.
 '
 ' Although parts of the program are still out of alignment with the standards
 ' set forth by rlk, this one works a lot better than what it was before it was
 ' released.
 '
 ' Libraries needed for this program: None
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
     ' This establishes compatibility with VisualBASIC by including compatibility
     ' functions and constants. This file is part of the compiler. We don't need
     ' it in this package.
     '/
    #include "vbcompat.bi"

    /'
     ' This includes a lot of the much needed information. This file is not part of
     ' the compiler, and even if it was part of the compiler, its presence in this
     ' package overrides its presence in the compiler.
     '/
    #include "neverassist.bi"
    /'
     ' This includes the generator. This file is not part of the compiler. It must
     ' be present in this package.
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

    if Username = "" then
        #IFDEF __FB_WIN32__
            mkdir("users")
            shell("attrib +h users")
        #ELSE
            mkdir(".users")
        #ENDIF
        input "What's your name";Username
        input "Where is your high score data kept";UserData
        mkdir(UserCfg + Username)
        Money = 20
        user_data
        config
    else
        user_data(1)
    end if
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
                        end 0
                    end if
                loop until multikey(SC_N)
            end if
        #ENDIF
    #ENDIF
    main_menu

    sub main_menu
        clkey
        do
            Title = "Neverassistant - " + left(Time,5) + " - " + Date
            windowtitle Title

            screenlock
            cls
            color rgb(0,255,255)
            locate 1,1
            print Username
            locate 1,70
            print using "##########!";Money,chr(4);
            color rgb(255,255,0)
            locate 3,1
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

            #IFDEF __FB_WIN32__
                print "<Z> ";lang("Locate 7-Zip")
                if (Z7Path = "") OR (Z7Exe = "") then
                    color rgb(128,128,128)
                    print "<U> ";lang("Unpack an archive");
                    color rgb(0,255,255):print lang(" - Must locate 7-Zip")
                else
                    color rgb(255,255,255):print "<U> ";lang("Unpack an archive");
                    color rgb(0,255,255):print lang(" - Will overwrite without prompt")
                end if
            #ELSE
                color rgb(128,128,128):print "<Z> ";lang("Locate 7-Zip");
                color rgb(0,255,255):print lang(" - 7-Zip is Windows only")
                color rgb(128,128,128):print "<U> ";lang("Unpack an archive");
                color rgb(0,255,255):print lang(" - 7-Zip is Windows only")
            #ENDIF

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
                print lang("Which mode?")
                print "<P> ";lang("Practice mode")
                print "<O> ";lang("Official mode (costs 10 diamonds; requires Challenge Mode)")
                do
                    if multikey(SC_O) AND Money > = 10 then
                        Official = 1
                        Money -= 10
                        exit do
                    end if
                    sleep 20
                loop until multikey(SC_P)
                screen 0
                #IFDEF __FB_WIN32__
                    Check = exec("Neverball.exe","")
                #ELSE
                    Check = exec("neverball","")
                #ENDIF
                screen 18,24
                print lang("Exit code: ")& Check
                clkey
                if Official = 1 then
                    Official = 0
                    print lang("How many coins? (input 0 for failure)  ");
                    input "", ChallengeCoins
                    if ChallengeCoins > 0 then
                        print lang("How many seconds? ");
                        input "",ChallengeTime
                        Money += ((ChallengeCoins / ChallengeTime * 60) - 0.499)
                        print using "&: #### &";lang("Awarded"), _
                            ((ChallengeCoins / ChallengeTime * 60) - 0.499), _
                            lang("diamonds")
                    end if
                    user_data
                end if
                open UserData + "/neverballhs-easy" for input as #5
                for ID as ubyte = 1 to 50
                    line input #5, DataScan
                next ID
                if right(DataScan,4) < > "Hard" AND FullBonus(1) = 0 AND eof(5) = 0 then
                    print lang("Congratulations! You 100% Neverball Easy Bonus Level I.")
                    print lang("Special Bonus: 10 diamonds")
                    FullBonus(1) = 1
                    Money += 10
                    user_data
                end if
                for ID as ubyte = 1 to 45
                    line input #5, DataScan
                next ID
                if right(DataScan,4) < > "Hard" AND FullBonus(2) = 0 AND eof(5) = 0 then
                    print lang("Congratulations! You 100% Neverball Easy Bonus Level II.")
                    print lang("Special Bonus: 20 diamonds")
                    FullBonus(2) = 1
                    Money += 20
                    user_data
                end if
                for ID as ubyte = 1 to 45
                    line input #5, DataScan
                next ID
                if right(DataScan,4) < > "Hard" AND FullBonus(3) = 0 AND eof(5) = 0 then
                    print lang("Impressive! You 100% Neverball Easy Bonus Level III.")
                    print lang("Special Bonus: 30 diamonds")
                    FullBonus(3) = 1
                    Money += 30
                    user_data
                end if
                for ID as ubyte = 1 to 45
                    line input #5, DataScan
                next ID
                if right(DataScan,4) < > "Hard" AND FullBonus(4) = 0 AND eof(5) = 0 then
                    print lang("Impressive!! You 100% Neverball Easy Bonus Level IV.")
                    print lang("Special Bonus: 40 diamonds")
                    FullBonus(4) = 1
                    Money += 40
                    user_data
                end if
                for ID as ubyte = 1 to 45
                    line input #5, DataScan
                next ID
                if right(DataScan,4) < > "Hard" AND FullBonus(5) = 0 AND eof(5) = 0 then
                    print lang("Awesome!!! You 100% Neverball Easy Bonus Level V.")
                    print lang("Special Bonus: 50 diamonds")
                    FullBonus(5) = 1
                    Money += 50
                    user_data
                end if
                close #5
                open UserData + "/neverballhs-medium" for input as #5
                for ID as ubyte = 1 to 50
                    line input #5, DataScan
                next ID
                if right(DataScan,4) < > "Hard" AND FullBonus(6) = 0 AND eof(5) = 0 then
                    print lang("Congratulations! You 100% Neverball Medium Bonus Level I.")
                    print lang("Special Bonus: 60 diamonds")
                    FullBonus(6) = 1
                    Money += 60
                    user_data
                end if
                for ID as ubyte = 1 to 45
                    line input #5, DataScan
                next ID
                if right(DataScan,4) < > "Hard" AND FullBonus(7) = 0 AND eof(5) = 0 then
                    print lang("Congratulations! You 100% Neverball Medium Bonus Level II.")
                    print lang("Special Bonus: 70 diamonds")
                    FullBonus(7) = 1
                    Money += 70
                    user_data
                end if
                for ID as ubyte = 1 to 45
                    line input #5, DataScan
                next ID
                if right(DataScan,4) < > "Hard" AND FullBonus(8) = 0 AND eof(5) = 0 then
                    print lang("Impressive! You 100% Neverball Medium Bonus Level III.")
                    print lang("Special Bonus: 80 diamonds")
                    FullBonus(8) = 1
                    Money += 80
                    user_data
                end if
                for ID as ubyte = 1 to 45
                    line input #5, DataScan
                next ID
                if right(DataScan,4) < > "Hard" AND FullBonus(9) = 0 AND eof(5) = 0 then
                    print lang("Impressive!! You 100% Neverball Medium Bonus Level IV.")
                    print lang("Special Bonus: 90 diamonds")
                    FullBonus(9) = 1
                    Money += 90
                    user_data
                end if
                for ID as ubyte = 1 to 45
                    line input #5, DataScan
                next ID
                if right(DataScan,4) < > "Hard" AND FullBonus(10) = 0 AND eof(5) = 0 then
                    print lang("Awesome!!! You 100% Neverball Medium Bonus Level V.")
                    print lang("Special Bonus: 100 diamonds")
                    FullBonus(10) = 1
                    Money += 100
                    user_data
                end if
                close #5
                open UserData + "/neverballhs-hard" for input as #5
                for ID as ubyte = 1 to 50
                    line input #5, DataScan
                next ID
                if right(DataScan,4) < > "Hard" AND FullBonus(11) = 0 AND eof(5) = 0 then
                    print lang("Congratulations! You 100% Neverball Hard Bonus Level I.")
                    print lang("Special Bonus: 110 diamonds")
                    FullBonus(11) = 1
                    Money += 110
                    user_data
                end if
                for ID as ubyte = 1 to 45
                    line input #5, DataScan
                next ID
                if right(DataScan,4) < > "Hard" AND FullBonus(12) = 0 AND eof(5) = 0 then
                    print lang("Congratulations! You 100% Neverball Hard Bonus Level II.")
                    print lang("Special Bonus: 120 diamonds")
                    FullBonus(12) = 1
                    Money += 120
                    user_data
                end if
                for ID as ubyte = 1 to 45
                    line input #5, DataScan
                next ID
                if right(DataScan,4) < > "Hard" AND FullBonus(13) = 0 AND eof(5) = 0 then
                    print lang("Impressive! You 100% Neverball Hard Bonus Level III.")
                    print lang("Special Bonus: 130 diamonds")
                    FullBonus(13) = 1
                    Money += 130
                    user_data
                end if
                for ID as ubyte = 1 to 45
                    line input #5, DataScan
                next ID
                if right(DataScan,4) < > "Hard" AND FullBonus(14) = 0 AND eof(5) = 0 then
                    print lang("Impressive!! You cleared Neverball Hard Bonus Level IV.")
                    print lang("Special Bonus: 140 diamonds")
                    FullBonus(14) = 1
                    Money += 140
                    user_data
                end if
                for ID as ubyte = 1 to 45
                    line input #5, DataScan
                next ID
                if right(DataScan,4) < > "Hard" AND FullBonus(15) = 0 AND eof(5) = 0 then
                    print lang("Awesome!!! You 100% Neverball Hard Bonus Level V.")
                    print lang("Special Bonus: 150 diamonds")
                    FullBonus(15) = 1
                    Money += 150
                    user_data
                end if
                close #5
                open UserData + "/neverballhs-mym" for input as #5
                for ID as ubyte = 1 to 50
                    line input #5, DataScan
                next ID
                if right(DataScan,4) < > "Hard" AND FullBonus(16) = 0 AND eof(5) = 0 then
                    print lang("Congratulations! You 100% Mehdi's Levels Bonus Level I.")
                    print lang("Special Bonus: 160 diamonds")
                    FullBonus(16) = 1
                    Money += 160
                    user_data
                end if
                for ID as ubyte = 1 to 45
                    line input #5, DataScan
                next ID
                if right(DataScan,4) < > "Hard" AND FullBonus(17) = 0 AND eof(5) = 0 then
                    print lang("Congratulations! You 100% Mehdi's Levels Bonus Level II.")
                    print lang("Special Bonus: 170 diamonds")
                    FullBonus(17) = 1
                    Money += 170
                    user_data
                end if
                for ID as ubyte = 1 to 45
                    line input #5, DataScan
                next ID
                if right(DataScan,4) < > "Hard" AND FullBonus(18) = 0 AND eof(5) = 0 then
                    print lang("Impressive! You 100% Mehdi's Levels Bonus Level III.")
                    print lang("Special Bonus: 180 diamonds")
                    FullBonus(18) = 1
                    Money += 180
                    user_data
                end if
                for ID as ubyte = 1 to 45
                    line input #5, DataScan
                next ID
                if right(DataScan,4) < > "Hard" AND FullBonus(19) = 0 AND eof(5) = 0 then
                    print lang("Impressive!! You 100% Mehdi's Levels Bonus Level IV.")
                    print lang("Special Bonus: 190 diamonds")
                    FullBonus(19) = 1
                    Money += 190
                    user_data
                end if
                for ID as ubyte = 1 to 45
                    line input #5, DataScan
                next ID
                if right(DataScan,4) < > "Hard" AND FullBonus(20) = 0 AND eof(5) = 0 then
                    print lang("Awesome!!! You 100% Mehdi's Levels Bonus Level V.")
                    print lang("Special Bonus: 200 diamonds")
                    FullBonus(20) = 1
                    Money += 200
                    user_data
                end if
                close #5
                open UserData + "/neverballhs-mym2" for input as #5
                for ID as ubyte = 1 to 50
                    line input #5, DataScan
                next ID
                if right(DataScan,4) < > "Hard" AND FullBonus(21) = 0 AND eof(5) = 0 then
                    print lang("Congratulations! You 100% Mehdi's Levels Suite Bonus Level I.")
                    print lang("Special Bonus: 210 diamonds")
                    FullBonus(21) = 1
                    Money += 210
                    user_data
                end if
                for ID as ubyte = 1 to 45
                    line input #5, DataScan
                next ID
                if right(DataScan,4) < > "Hard" AND FullBonus(22) = 0 AND eof(5) = 0 then
                    print lang("Congratulations! You 100% Mehdi's Levels Suite Bonus Level II.")
                    print lang("Special Bonus: 220 diamonds")
                    FullBonus(22) = 1
                    Money += 220
                    user_data
                end if
                for ID as ubyte = 1 to 45
                    line input #5, DataScan
                next ID
                if right(DataScan,4) < > "Hard" AND FullBonus(23) = 0 AND eof(5) = 0 then
                    print lang("Impressive! You 100% Mehdi's Levels Suite Bonus Level III.")
                    print lang("Special Bonus: 230 diamonds")
                    FullBonus(23) = 1
                    Money += 230
                    user_data
                end if
                for ID as ubyte = 1 to 45
                    line input #5, DataScan
                next ID
                if right(DataScan,4) < > "Hard" AND FullBonus(24) = 0 AND eof(5) = 0 then
                    print lang("Impressive!! You 100% Mehdi's Levels Suite Bonus Level IV.")
                    print lang("Special Bonus: 240 diamonds")
                    FullBonus(24) = 1
                    Money += 240
                    user_data
                end if
                for ID as ubyte = 1 to 45
                    line input #5, DataScan
                next ID
                if right(DataScan,4) < > "Hard" AND FullBonus(25) = 0 AND eof(5) = 0 then
                    print lang("Awesome!!! You 100% Mehdi's Levels Suite Bonus Level V.")
                    print lang("Special Bonus: 250 diamonds")
                    FullBonus(25) = 1
                    Money += 250
                    user_data
                end if
                close #5
                sleep
            elseif multikey(SC_P) then
                sleep 300,1
                clkey
                windowtitle "Neverassistant - Running Neverputt"
                print lang("Which mode?")
                print "<P> ";lang("Practice mode")
                print "<O> ";lang("Official mode (costs 10 diamonds)")
                do
                    if multikey(SC_O) AND Money > = 10 then
                        Official = 1
                        Money -= 10
                        exit do
                    end if
                    sleep 20
                loop until multikey(SC_P)
                screen 0
                #IFDEF __FB_WIN32__
                    Check = exec("Neverputt.exe","")
                #ELSE
                    Check = exec("neverputt","")
                #ENDIF
                screen 18,24
                print lang("Exit code: ")& Check
                clkey
                if Official = 1 then
                    Official = 0
                    print lang("What was your score (relative to par)? ");
                    input "",DiffPar
                    if DiffPar < -1 then
                        print lang("You netted ")& abs(DiffPar);lang(" strokes under par. ")&DiffPar*-2;lang(" diamonds rewarded.")
                    elseif DiffPar = -1 then
                        print lang("You netted 1 stroke under par. 2 diamonds rewarded.")
                    elseif DiffPar = 0 then
                        print lang("You netted par. No reward.")
                    elseif DiffPar = 1 then
                        print lang("You netted 1 stroke over par. 2 diamonds fined.")
                    elseif DiffPar > 1 then
                        print lang("You netted ")&DiffPar;lang(" strokes under par. ")&DiffPar*2;lang(" diamonds fined.")
                    end if
                    Money -= DiffPar * 2
                    user_data
                end if
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
                print lang("If relative, must be relative to assistant's directory.")
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
                'Try to make it wrap the 80 characters if you translate this string
                print lang("The solution directory contains a collection of " + _
                    "solutions to levels players may give a fit on.")
                print "<A> ";lang("Neverball Easy Level 10: How to 100% the level")
                print "<B> ";lang("Neverball Easy Bonus Level 5: How to 100% the level")
                print lang("Each view costs 2 diamonds. Use them wisely.")
                print lang("Hit BACKSPACE to exit the directory.")
                print
                do
                    sleep 10
                    if multikey(SC_A) AND Money > = 2 then
                        Money -= 2
                        user_data
                        screen 0
                        #IFDEF __FB_WIN32__
                            Check = exec("Neverball.exe", _
                                         "-r "+AssistDir+"/solutions/cE10csy.nbr")
                        #ELSE
                            Check = exec("neverball", _
                                         "-r "+AssistDir+"/solutions/cE10csy.nbr")
                        #ENDIF
                        screen 18,24
                        clkey
                        print lang("Exit code of replay A: ")& Check
                        sleep
                        exit do
                    elseif multikey(SC_B) AND Money > = 2 then
                        Money -= 2
                        user_data
                        screen 0
                        #IFDEF __FB_WIN32__
                            Check = exec("Neverball.exe", _
                                         "-r "+AssistDir+"/solutions/cEB5csy.nbr")
                        #ELSE
                            Check = exec("neverball", _
                                         "-r "+AssistDir+"/solutions/cEB5csy.nbr")
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
                print lang("If you don't have a levelshot when it asks you " + _
                           "for it, leave it blank!")
                print
                print lang("Which level to convert? ");
                input "",MapFile
                print lang("Where is its corrosponding levelshot?")
                input "",ShotFile
                /'
                 ' This creates a totally random ID. As it is random, and with a
                 ' huge range, it will very rarely conflict with another level.
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
                    FileCopy(ShotFile,"data/shot-"+str(SingleLevelID)+"/"+ShotFile)
                end if

            #IFDEF __FB_WIN32__
                elseif multikey(SC_Z) then
                    clkey
                    windowtitle "Neverassistant - Locate 7-Zip"
                    print lang("Where do you want to locate 7-Zip?") + _
                          lang(" (without \7z.exe, but where the 7z.exe is)")
                    color rgb(255,255,0)
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
                    color rgb(255,255,255)

                elseif multikey(SC_U) AND (Z7Path < > "") AND (Z7Exe < > "") then
                    clkey
                    windowtitle "Neverassistant - Unpack archive"
                    print lang("Which archive do you want to extract?") + _
                          lang(" (include extension)")
                    color rgb(255,255,0)
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
                    color rgb(255,255,255)
            #ENDIF

            elseif multikey(SC_H) then
                'Force crash function.
                end -2
            elseif multikey(SC_X) OR multikey(SC_ESCAPE) OR _
                   inkey = chr(255)+"k" then
                end 0
            end if
        loop
    end sub
#ENDIF