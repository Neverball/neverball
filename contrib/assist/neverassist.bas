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
 ' Under Windows, the -s gui removes the console. On *nix, this switch does
 ' nothing.
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
windowtitle "Neverassistant Program"
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
 ' This includes some of the much needed information. This file is not part of
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
 ' This initiates graphics mode. It is a 640x480 8-bit program with one page
 ' and you can't fullscreen it (unless windowed mode fails).
 '/
screen 18,8,1,GFX_NO_SWITCH

/'
 ' This define is actually related to the system you're on.
 ' If you're on Windows, __FB_WIN32__ is automatically defined.
 ' If you're on DOS, __FB_DOS__ is automatically defined.
 ' If you're on Linux or GNU/Linux, __FB_LINUX__ is automatically defined.
 '/
#IFDEF __FB_WIN32__
open AssistCfg for input as #1
input #1, MediumClear
input #1, MaxMoney
input #1, Z7Path
input #1, Z7Exe
close #1
#ELSE
open AssistCfg for input as #1
input #1, MediumClear
input #1, MaxMoney
close #1
#ENDIF

#IFDEF __FB_WIN32__
'Incomplete: Attempt to get username and use the user folder.
dim shared as string Username, AppData
Username = "" 
Appdata = "C:\Documents and Settings\" + Username + "\Application Data\"
#ENDIF
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
        Title = "Neverassistant Program - " + Time + " - " + Date
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
            map_generate_initial
        elseif multikey(SC_N) then
            clkey
            windowtitle "Neverassistant Program - Running Neverball"
            #IFDEF __FB_WIN32__
            Check = exec("../Neverball.exe","")
            #ELSE
            Check = exec("../neverball","")
            #ENDIF
        elseif multikey(SC_P) then
            clkey
            windowtitle "Neverassistant Program - Running Neverputt"
            #IFDEF __FB_WIN32__
            Check = exec("../Neverputt.exe","")
            #ELSE
            Check = exec("../neverputt","")
            #ENDIF
            
        elseif multikey(SC_C) then
            clkey
            windowtitle "Neverassistant Program - Compiling map"
            input "Which map do you want to compile? (without .map) ",Compile
            if (Compile < > "") then
                Compile = Compile + ".map"
                #IFDEF __FB_WIN32__
                Check = exec("../Mapc.exe",Compile + " ../data")
                #ELSE
                Check = exec("../mapc",Compile + " ../data")
                #ENDIF
            end if
            color 15
            
        elseif multikey(SC_R) then
            clkey
            windowtitle "Neverassistant Program - Load replay"
            input "Which replay do you want to watch? (without .nbr) ",Replay
            if (Replay < > "") then
                Replay = Replay + ".nbr"
                #IFDEF __FB_WIN32__
                Check = exec("../Neverball.exe","-r " + Appdata + Replay)
                #ELSE
                Check = exec("../neverball","-r ../data/.neverball-dev/" + Replay)
                #ENDIF
            end if
            color 15
            
        #IFDEF __FB_WIN32__
        elseif multikey(SC_Z) then
            clkey
            windowtitle "Neverassistant Program - Locate 7-Zip"
            print "Where do you want to locate 7-Zip? (without \7z.exe, but where the 7z.exe is)"
            color 44
            input "",Z7Path
            if (Z7Path < > "") then
                Z7Exe = Z7Path + "\7z.exe"
                Check = exec(Z7Exe,"")
                if (Check = -1) then
                    open AssistCfg for input as #1
                    input #1, MediumClear
                    input #1, MaxMoney
                    input #1, Z7Path
                    input #1, Z7Exe
                    close #1
                else
                    open AssistCfg for output as #1
                    print #1, MediumClear
                    print #1, MaxMoney
                    print #1, Z7Path
                    print #1, Z7Exe
                    close #1
                end if
            end if
            color 15
            
        elseif multikey(SC_U) AND (Z7Path < > "") AND (Z7Exe < > "") then
            clkey
            windowtitle "Neverassistant Program - Unpack archive"
            print "Which archive do you want to extract? (include extension)"
            color 44
            input Unpack
            if (Unpack < > "") then
                Z7Exe = Z7Path + "\7z.exe"
                Check = exec(Z7Exe,"x -w{"+chr(34)+curdir+chr(34)+"} -y "+Unpack)
                if (Check = -1) then
                    Z7Path = ""
                    Z7Exe = ""
                    open AssistCfg for output as #1
                    print #1, MediumClear
                    print #1, MaxMoney
                    print #1, Z7Path
                    print #1, Z7Exe
                    close #1
                    sleep 2000
                end if
            end if
            color 15
        #ENDIF
        
        elseif multikey(SC_H) then
            'Force crash function.
            exit do
        elseif multikey(SC_X) OR multikey(SC_ESCAPE) OR inkey = chr(255)+"k" then
            Perfect = 1
            end 0
        end if
    loop
end sub

sub map_generate_initial
    /'
     ' This subroutine gives the initial information about the map.
     '
     ' This includes the name and environment.
     '/
    color 11
    clkey
    cls
    input "Which file would you like to enter";MapFile
    MapFile = MapFile + ".map"
    WindowTitleM = "Neverassistant Program - " + MapFile
    windowtitle WindowTitleM
    print "Which environment do you want?"
    print
    print "Clouds and [L]and"
    print "[O]ceans"
    print "[C]ity and Streets"
    print "Jupiter and [S]pace"
    print "[A]lien World"
    do
        sleep 10
        if multikey(SC_A) then
            Song = "bgm/track5.ogg"
            Back = "map-back/alien.sol"
            Grad = "back/alien.png"
            MusicPlay = "data/bgm/track5.ogg"
            exit do
        elseif multikey(SC_C) then
            Song = "bgm/track3.ogg"
            Back = "map-back/city.sol"
            Grad = "back/city.png"
            MusicPlay = "data/bgm/track3.ogg"
            exit do
        elseif multikey(SC_L) then
            Song = "bgm/track1.ogg"
            Back = "map-back/clouds.sol"
            Grad = "back/land.png"
            MusicPlay = "data/bgm/track1.ogg"
            exit do
        elseif multikey(SC_O) then
            Song = "bgm/track2.ogg"
            Back = "map-back/ocean.sol"
            Grad = "back/ocean.png"
            MusicPlay = "data/bgm/track2.ogg"
            exit do
        elseif multikey(SC_S) then
            Song = "bgm/track4.ogg"
            Back = "map-back/jupiter.sol"
            Grad = "back/space.png"
            MusicPlay = "data/bgm/track4.ogg"
            exit do
        elseif inkey = chr(255)+"k" then
            end
        end if
    loop
    map_generate
    color 15
end sub

sub map_generate
    cls
    Start = 0
    Finish = 0
    LevelTime = 0
    Blocks = 0
    Coins = 0
    TargetCoins = 0
    MinimumLevelTime = 0
    MusicSwitch = 0
    Money = 0
    Putt = 0
    for XP = -10 to 10
        for YP = -10 to 10
            for ZP = -10 to 20
                PlacementFormula = XP + (YP * 50) + (ZP * 2000) + 100000
                PlacementTest(PlacementFormula) = 0
            next
        next
    next
    XP = 0
    YP = 0
    ZP = 0
    BlockType = 1
    Rotation = 1
    BlockSet = 0
    XR = 1
    YR = 1
    ZR = 1
    open MapFile for output as #m
    
    do
        cls
        Warning = 0
        
        /'
         ' Due to an issue in an older version of FB, I used a formula for
         ' processing this as opposed to making three arrays.
         '/
        PlacementFormula = XP + (YP * 50) + (ZP * 2000) + 100000
        
        /'
         ' This allows hitting the X in the window to actually close the program.
         '/
        if inkey = chr(255)+"k" then end
        
        /'
         ' These move the cursor
         '/
        if multikey(SC_RIGHT) then
            XP += 1
        elseif multikey(SC_LEFT) then
            XP -= 1
        end if
        if multikey(SC_UP) then
            YP += 1
        elseif multikey(SC_DOWN) then
            YP -= 1
        end if
        if multikey(SC_PAGEUP) then
            ZP += 1
        elseif multikey(SC_PAGEDOWN) then
            ZP -= 1
        end if
        
        if (multikey(SC_PLUS) OR multikey(SC_EQUALS)) AND _
           (NOT multikey(SC_LSHIFT)) then
            LevelTime += 25
        elseif multikey(SC_MINUS) AND (NOT multikey(SC_LSHIFT)) then
            if LevelTime < 25 then LevelTime = 0 else LevelTime -= 25
        elseif (multikey(SC_PLUS) OR multikey(SC_EQUALS)) AND _
               multikey(SC_LSHIFT) then
            TargetCoins += 1
        elseif multikey(SC_MINUS) AND multikey(SC_LSHIFT) then
            if TargetCoins > 0 then TargetCoins -= 1
        end if
        if multikey(SC_SLASH) AND (Hold = 0) then
            Hold = 100
            if Openings > = 2 then Openings -= 2
        end if
                
        if (NOT multikey(SC_SLASH)) AND (Hold > 0) then Hold -= 1
        
        /'
         ' This prevents maps made with this program from getting away without
         ' certain items.
         '/
        if Start = 0 then Warning += 1
        if Finish = 0 then Warning += 1
        if TargetCoins > Coins then Warning += 1
        if MinimumLevelTime > LevelTime then Warning += 1
        
        /'
         ' If this had been complete, there would have been no commented out lines.
         '
         ' This allows you to switch block sets.
         '/
        if multikey(SC_F2) AND (Putt < > 1) then
            BlockSet = 0
        elseif multikey(SC_F3) AND (Putt < > 1) then
            /'
            BlockSet = 1
            '/
        elseif multikey(SC_F4) AND (Putt < > 1) then
            /'
            BlockSet = 2
            '/
        elseif multikey(SC_F5) AND (Putt < > 2) then
            /'
            BlockSet = 3
            '/
        end if
        
        /'
         ' Each letter of the English alphabet sets an indivudual block.
         '/
        if multikey(SC_A) then
            BlockType = 1
            if (BlockSet < > 1) then
                XR = 1
                YR = 1
                ZR = 1
            end if
        elseif multikey(SC_B) then
            BlockType = 2
            if (BlockSet < > 1) then
                XR = 1
                YR = 1
                ZR = 1
            end if
        elseif multikey(SC_C) then
            BlockType = 3
            if (BlockSet = 0) then
                XR = 1
                YR = 1
                ZR = 1
            end if
        elseif multikey(SC_D) then
            BlockType = 4
            if (BlockSet = 0) then
                XR = 1
                YR = 1
                ZR = 1
            end if
        elseif multikey(SC_E) then
            BlockType = 5
            if (BlockSet = 0) then
                XR = 1
                YR = 1
                ZR = 1
            end if
            
        elseif multikey(SC_F) then
            BlockType = 6
            if (BlockSet = 0) then
                XR = 1
                YR = 1
                ZR = 1
            end if
        elseif multikey(SC_G) then
            BlockType = 7
            if (BlockSet = 0) then
                XR = 1
                YR = 1
                ZR = 1
            end if
        elseif multikey(SC_H) then
            BlockType = 8
            if (BlockSet = 0) then
                XR = 1
                YR = 1
                ZR = 1
            end if
        elseif multikey(SC_I) then
            BlockType = 9
            if (BlockSet = 0) then
                XR = 1
                YR = 1
                ZR = 1
            end if
        elseif multikey(SC_J) then
            BlockType = 10
            if (BlockSet = 0) then
                XR = 1
                YR = 1
                ZR = 1
            end if
            
        elseif multikey(SC_K) then
            BlockType = 11
            if (BlockSet = 0) then
                XR = 1
                YR = 1
                ZR = 1
            end if
        elseif multikey(SC_L) then
            BlockType = 12
            if (BlockSet = 0) then
                XR = 1
                YR = 1
                ZR = 1
            end if
        elseif multikey(SC_M) then
            BlockType = 13
            if (BlockSet = 0) then
                XR = 1
                YR = 1
                ZR = 2
            end if
        elseif multikey(SC_N) then
            BlockType = 14
            if (BlockSet = 0) then
                XR = 1
                YR = 1
                ZR = 1
            end if
        elseif multikey(SC_O) then
            BlockType = 15
            if (BlockSet = 0) then
                XR = 1
                YR = 1
                ZR = 1
            end if
            
        elseif multikey(SC_P) then
            BlockType = 16
            if (BlockSet = 0) then
                XR = 1
                YR = 1
                ZR = 1
            end if
            
        elseif multikey(SC_Z) then
            BlockType = 26
            if (BlockSet = 0) then
                XR = 1
                YR = 1
                ZR = 1
            end if
        end if
        
        /'
         ' This rotates the block 
         '/
        if multikey(SC_CONTROL) then if Rotation < 4 then Rotation += 1 else Rotation = 1
        
        /'
         ' This keeps the cursor from going out of bounds. 
         '/
        if (XP < -11 + XR) then XP = -11 + XR
        if (YP < -11 + YR) then YP = -11 + YR
        if (ZP < -10) then ZP = -10
        if (XP > 10) then XP = 10
        if (YP > 10) then YP = 10
        if (ZP > 21 - ZR) then ZP = 21 - ZR
        
        /'
         ' Prints information on the right hand side.
         '/
        locate 1,54
        print "Location: ("&XP;","&YP;","&ZP;")"
        color 11
        locate 2,54
        print "* Press F1 for help."
        color 14
        locate 4,54
        print "Music unsupported."
        color 13
        locate 5,54
        print "Time: ";LevelTime
        locate 6,54
        print "Blocks: ";Blocks
        locate 7,54
        print "Coins: ";TargetCoins;"/";Coins
        locate 8,54
        print "$";Money;"/";MaxMoney
        
        locate 13,54
        if (BlockSet = 0) then color 10 else if (BlockSet = 1) then color 12 else if (Blockset = 2) then color 11 else if (Blockset = 3) then color 14
        line(448,238)-(502,292),9,bf
        line(449,239)-(501,291),0,bf
        line(548,238)-(602,292),9,bf
        line(549,239)-(601,291),0,bf
        
        if (BlockType = 1) AND (BlockSet < > 1) then
            print "Start"
            line(450,240)-(500,240),12
            line(450,240)-(450,290),10
            line(450,290)-(500,290),10
            line(500,240)-(500,290),10
            line(550,270)-(550,290),7
            line(550,270)-(600,270),7
            line(600,270)-(600,290),7
            line(550,290)-(600,290),7
            line(570,240)-(580,270),12,bf
        elseif (BlockType = 2) AND (BlockSet < > 1) then
            print "Flat straight $1"
            line(450,240)-(500,240),12
            line(450,240)-(450,290),10
            line(450,290)-(500,290),12
            line(500,240)-(500,290),10
            line(550,270)-(550,290),7
            line(550,270)-(600,270),7
            line(600,270)-(600,290),7
            line(550,290)-(600,290),7
        elseif (BlockType = 3) AND (BlockSet = 0) then
            print "90"+chr(248)+" simple flat turn $1"
            line(450,240)-(500,240),10
            line(450,240)-(450,290),12
            line(450,290)-(500,290),12
            line(500,240)-(500,290),10
            line(550,270)-(550,290),7
            line(550,270)-(600,270),7
            line(600,270)-(600,290),7
            line(550,290)-(600,290),7
        elseif (BlockType = 4) AND (BlockSet = 0) then
            print "Junction $1"
            line(450,240)-(500,240),10
            line(450,240)-(450,290),12
            line(450,290)-(500,290),12
            line(500,240)-(500,290),12
            line(550,270)-(550,290),7
            line(550,270)-(600,270),7
            line(600,270)-(600,290),7
            line(550,290)-(600,290),7
        elseif (BlockType = 5) AND (BlockSet = 0) then
            print "Cross $1"
            line(450,240)-(500,240),12
            line(450,240)-(450,290),12
            line(450,290)-(500,290),12
            line(500,240)-(500,290),12
            line(550,270)-(550,290),7
            line(550,270)-(600,270),7
            line(600,270)-(600,290),7
            line(550,290)-(600,290),7
            
        elseif (BlockType = 6) AND (BlockSet = 0) then
            print "Coined flat straight $1"
            line(450,240)-(500,240),12
            line(450,240)-(450,290),10
            line(450,290)-(500,290),12
            line(500,240)-(500,290),10
            pset(465,255),14
            pset(485,255),14
            pset(465,275),14
            pset(485,275),14
            line(550,270)-(550,290),7
            line(550,270)-(600,270),7
            line(600,270)-(600,290),7
            line(550,290)-(600,290),7
        elseif (BlockType = 7) AND (BlockSet = 0) then
            print "Coined 90"+chr(248)+" simple flat turn"
            locate 14,54
            print "$1"
            line(450,240)-(500,240),10
            line(450,240)-(450,290),12
            line(450,290)-(500,290),12
            line(500,240)-(500,290),10
            pset(465,255),14
            pset(485,255),14
            pset(465,275),14
            pset(485,275),14
            line(550,270)-(550,290),7
            line(550,270)-(600,270),7
            line(600,270)-(600,290),7
            line(550,290)-(600,290),7
        elseif (BlockType = 8) AND (BlockSet = 0) then
            print "Dead-end $1"
            line(450,240)-(500,240),10
            line(450,240)-(450,290),10
            line(450,290)-(500,290),12
            line(500,240)-(500,290),10
            line(550,270)-(550,290),7
            line(550,270)-(600,270),7
            line(600,270)-(600,290),7
            line(550,290)-(600,290),7
        elseif (BlockType = 9) AND (BlockSet = 0) then
            print "Coined Dead-end $1"
            line(450,240)-(500,240),10
            line(450,240)-(450,290),10
            line(450,290)-(500,290),12
            line(500,240)-(500,290),10
            pset(465,255),14
            pset(485,255),14
            pset(465,275),14
            pset(485,275),14
            line(550,270)-(550,290),7
            line(550,270)-(600,270),7
            line(600,270)-(600,290),7
            line(550,290)-(600,290),7
        elseif (BlockType = 10) AND (BlockSet = 0) then
            print "Small Jump $2"
            line(450,240)-(500,240),12
            line(450,240)-(450,250),10
            line(450,250)-(500,250),10
            line(500,240)-(500,250),10
            line(450,280)-(500,280),10
            line(450,280)-(450,290),10
            line(450,290)-(500,290),12
            line(500,280)-(500,290),10
            line(550,270)-(550,290),7
            line(550,270)-(560,260),7
            line(560,260)-(560,290),7
            line(550,290)-(560,290),7
            line(590,260)-(590,290),7
            line(590,260)-(600,270),7
            line(600,270)-(600,290),7
            line(590,290)-(600,290),7
            
        elseif (BlockType = 11) AND (BlockSet = 0) then
            print "Coined Junction $1"
            line(450,240)-(500,240),10
            line(450,240)-(450,290),12
            line(450,290)-(500,290),12
            line(500,240)-(500,290),12
            pset(465,255),14
            pset(485,255),14
            pset(465,275),14
            pset(485,275),14
            line(550,270)-(550,290),7
            line(550,270)-(600,270),7
            line(600,270)-(600,290),7
            line(550,290)-(600,290),7
        elseif (BlockType = 12) AND (BlockSet = 0) then
            print "Coined Cross $1"
            line(450,240)-(500,240),12
            line(450,240)-(450,290),12
            line(450,290)-(500,290),12
            line(500,240)-(500,290),12
            pset(465,255),14
            pset(485,255),14
            pset(465,275),14
            pset(485,275),14
            line(550,270)-(550,290),7
            line(550,270)-(600,270),7
            line(600,270)-(600,290),7
            line(550,290)-(600,290),7
        elseif (BlockType = 13) AND (BlockSet = 0) then
            print "Ramp straight $1"
            line(450,240)-(500,240),12
            line(450,240)-(450,290),10
            line(450,290)-(500,290),12
            line(500,240)-(500,290),10
            line(550,270)-(550,290),7
            line(550,270)-(600,250),7
            line(600,250)-(600,270),7
            line(550,290)-(600,270),7
        elseif (BlockType = 14) AND (BlockSet = 0) then
            print "Bump $1"
            line(450,240)-(500,240),12
            line(450,240)-(450,290),10
            line(450,290)-(500,290),12
            line(500,240)-(500,290),10
            line(550,270)-(550,290),7
            line(550,270)-(575,265),7
            line(575,265)-(600,270),7
            line(600,270)-(600,290),7
            line(550,290)-(600,290),7
        elseif (BlockType = 15) AND (BlockSet = 0) then
            print "Coined Bump $1"
            line(450,240)-(500,240),12
            line(450,240)-(450,290),10
            line(450,290)-(500,290),12
            line(500,240)-(500,290),10
            pset(465,255),14
            pset(485,255),14
            pset(465,275),14
            pset(485,275),14
            line(550,270)-(550,290),7
            line(550,270)-(575,265),7
            line(575,265)-(600,270),7
            line(600,270)-(600,290),7
            line(550,290)-(600,290),7
            
        elseif (BlockType = 16) AND (BlockSet = 0) then
            print "Flat straight with mover $3"
            line(450,240)-(500,240),12
            line(450,240)-(450,290),10
            line(450,290)-(500,290),12
            line(500,240)-(500,290),10
            line(462,265)-(488,265),6
            line(550,270)-(550,290),7
            line(550,270)-(600,270),7
            line(600,270)-(600,290),7
            line(550,290)-(600,290),7
            line(562,260)-(588,260),6
            
        elseif (BlockType = 26) AND (BlockSet = 0) then
            print "Dead-end Finish"
            line(450,240)-(500,240),10
            line(450,240)-(450,290),10
            line(450,290)-(500,290),12
            line(500,240)-(500,290),10
            line(550,270)-(550,290),7
            line(550,270)-(600,270),7
            line(600,270)-(600,290),7
            line(550,290)-(600,290),7
            line(570,240)-(580,270),13,bf
        elseif (BlockSet = 1) then
            print "Not used"
        end if
        color 15
        
        /'
         ' This gives information about each block.
         '/
        locate 28,1
        if (NOT multikey(SC_F1)) AND (NOT multikey(SC_TILDE)) then
            if (BlockType = 1) AND (BlockSet = 0) AND (Start = 1) then
                print " You've already placed this block."
            elseif (BlockType = 1) AND (BlockSet = 0) AND (Start = 0) then
                print " Every map made must have one of these. This is where the player start. This"
                print "block can't be rotated."
            elseif (BlockType = 2) AND (BlockSet = 0) then
                print " This is the most simple block. It is straight and flat."
            elseif (BlockType = 3) AND (BlockSet = 0) then
                print " This is the most simple turn. It is flat."
            elseif (BlockType = 4) AND (BlockSet = 0) then
                print " This is a 3-way block."
            elseif (BlockType = 5) AND (BlockSet = 0) then
                print " This is a 4-way block."
                
            elseif (BlockType = 6) AND (BlockSet = 0) then
                print " Like a Flat Straight, but with four coins."
            elseif (BlockType = 7) AND (BlockSet = 0) then
                print " Like a 90"+chr(248)+" simple flat turn, but with four coins."
            elseif (BlockType = 8) AND (BlockSet = 0) then
                print " This block only has one end. Be careful."
            elseif (BlockType = 9) AND (BlockSet = 0) then
                print " Like a Dead-end, but with four coins."
            elseif (BlockType = 10) AND (BlockSet = 0) then
                print " You can jump with this block."
                
            elseif (BlockType = 11) AND (BlockSet = 0) then
                print " Like a Junction, but with four coins."
            elseif (BlockType = 12) AND (BlockSet = 0) then
                print " Like a Cross, but with four coins."
            elseif (BlockType = 13) AND (BlockSet = 0) then
                print " This block allows you to change elevations."
            elseif (BlockType = 14) AND (BlockSet = 0) then
                print " This block has a bump."
            elseif (BlockType = 15) AND (BlockSet = 0) then
                print " This bump has coins on it."
                
            elseif (BlockType = 16) AND (BlockSet = 0) then
                print " Like a Flat Straight, but with a mover on top."
                
            elseif (BlockType = 26) AND (BlockSet = 0) then
                print " Every map made must have at least one of these. This is where the player"
                print "finishes."
                
            elseif (BlockType = 1) AND (BlockSet = 2) AND (Start = 1) then
                print " You've already placed this block."
            elseif (BlockType = 1) AND (BlockSet = 2) AND (Start = 0) AND (ZP < > 0) then
                print " This block must be placed at elevation level 0."
            elseif (BlockType = 1) AND (BlockSet = 2) AND (Start = 0) AND (ZP = 0) then
                print " Every map made must have one of these. This is where the player start. This"
                print "block can't be rotated."
            elseif (BlockType = 2) AND (BlockSet = 2) AND (ZP < > 0) then
                print " This block must be placed at elevation level 0."
            elseif (BlockType = 2) AND (BlockSet = 2) AND (ZP = 0) then
                print " This is a shiny block. It is straight and flat."
            end if
        elseif multikey(SC_F1) then
            clkey
            cls 0
            color 11
            print "Controls:"
            print "* CONTROL: rotate"
            print "* TAB: toggle music"
            print "* SPACEBAR: place block"
            print "* Arrow keys: move cursor"
            print "* PAGE UP and PAGE DOWN: change elevation.
            print "* Any English letter: change blocks."
            print "* PLUS and MINUS without LSHIFT: adjust time."
            print "* PLUS and MINUS with LSHIFT: adjust target coins."
            print "* F6 and F7: adjust music beat.
            print "* F8: restore normal music beat"
            print "* F2: change to "+chr(34)+"Basic Neverball"+chr(34)+" block set. You can't access this set if"
            print "  blocks from the "+chr(34)+"Neverputt"+chr(34)+" set are placed."
            print "* F3: change to "+chr(34)+"Advanced Neverball"+chr(34)+" block set. You can't access this set if"
            print "  blocks from the "+chr(34)+"Neverputt"+chr(34)+" set are placed."
            print "* F4: change to "+chr(34)+"Extra textures"+chr(34)+" block set. You can't access this set if blocks"
            print "  from the "+chr(34)+"Neverputt"+chr(34)+" set are placed."
            print "* F5: change to "+chr(34)+"Neverputt"+chr(34)+" block set. You can't access this set if blocks from"
            print "  other sets are placed."
            print "* ~: to check for issues. You can't save the map until you ensure there are no"
            print "  errors."
            color 15
            sleep
            cls
        elseif multikey(SC_TILDE) then
            clkey
            cls 0
            if Warning = 0 then
                color 48
                print "Check okay. There are no offending items."
            else
                color 40
                if Warning = 1 then
                    print "Check failed. There is 1 offending item."
                else
                    print "Check failed. There are ";Warning;" offending items."
                end if    
                color 42
                if Start = 0 then print "- Must have 1 start block."
                if Finish = 0 then print "- Must have at least 1 finish block."
                if TargetCoins > Coins then
                    print "- The number of required coins exceeds the coins present."
                end if
                if MinimumLevelTime > LevelTime then
                    print "- The time given for a level must equal or exceed ";MinimumLevelTime;"."
                end if
            end if
            color 44
            if Openings > 0 then
                print "- You have ";Openings;" openings that you haven't closed yet. If some of your blocks were"
                print "  intended to merge roads together, you can hit the "+chr(34)+"/"+chr(34)+" key to merge together."
            end if
            if Money * 1.2 > MaxMoney then
                print "- You are running low on money. You should stop building this map soon. You"
                print "only have $"& MaxMoney-Money ;" remaining. If you want more, you have to beat more levels"
                print "in the Neverball game. They will give you more money."
            end if
            color 15
            sleep
            cls
        end if
        
        base_gfx
        cursor
        direction
        if multikey(SC_SPACE) then place_block
        
        sleep 99
    loop until multikey(SC_ESCAPE)
    
    if (Warning = 0) then
        cls
        clkey
        color 10
        print "Success."
        color 15
        input "What would you like in your message";LevelMessage
        input "What is the name of this level";LevelName
        print #m, "// entity ";Entity;"
        print #m, "{"
        print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"worldspawn"+chr(34)
        if (LevelMessage < > "") then print #m, chr(34)+"message"+chr(34)+" "+chr(34);LevelMessage;chr(34)
        print #m, chr(34)+"time"+chr(34)+" "+chr(34);LevelTime;chr(34)
        print #m, chr(34)+"goal"+chr(34)+" "+chr(34);TargetCoins;chr(34)
        print #m, chr(34)+"back"+chr(34)+" "+chr(34);Back;chr(34)
        print #m, chr(34)+"grad"+chr(34)+" "+chr(34);Grad;chr(34)
        print #m, chr(34)+"song"+chr(34)+" "+chr(34);Song;chr(34)
        print #m, chr(34)+"levelname"+chr(34)+" "+chr(34);LevelName;chr(34)
        print #m, "}"
        close #m
        print "Your map is almost finished..."
        #IFDEF __FB_WIN32__
        Check = exec("../Mapc.exe",MapFile + " data")
        #ELSE
        Check = exec("../mapc",MapFile + " data")
        #ENDIF
        if (Check < > -1) then
            cls
            print "It is now ready for play."
            print
            print "If this is your first level, you need to create a set-XXX.txt in the data"
            print "folder with the following contents. Brackets are variables."
            color 11
            print "[Name of Set]"
            print "[Description]"
            print "[ID]"
            print "[Set Shot]"
            print "[Hard BT]","[Hard MT]","[Med BT]","[Med MT]""[Easy BT]","[Easy MT]"
            print "[Level files] (relative to data folder)"
            color 15
            print
            print "Add the name of the file to sets.txt"
            print
            print "It is preferable that you make a folder in the data directory to store all of"
            print "your levels."
            print
            print "If you already did this before, you only need to update the set-XXX.txt with"
            print "the new level."
            sleep
        end if
    else
        cls
        color 12
        print "Failed."
        color 15
        close #m
        kill(MapFile)
    end if
    if inkey = chr(27) then sleep
    clkey
    menu
end sub
sub place_gfx(Added as integer)
    GraphicFormula = PlacementFormula
    GraphicTest(GraphicFormula+Added) = PlacementTest(PlacementFormula)
end sub
sub base_gfx
    for XM = 0 to 21
        line(XM*20,0)-(XM*20,420),15
    next
    for YM = 0 to 21
        line(0,YM*20)-(420,YM*20),15
    next
    for XG = -10 to 10
        for YG = -10 to 10
            GraphicFormula = XG + (YG * 50) + (ZP * 2000) + 100000
            if GraphicTest(GraphicFormula) < > 0 then
                line((XG+10)*20+1,(-YG+10)*20+1)-((XG+11)*20-1,(-YG+9)*20-1),13,bf
            end if
        next
    next
end sub
sub cursor
    if PlacementTest(PlacementFormula) = 0 then
        line((XP+11-XR)*20,(-YP+11-YR)*20)-((XP+11)*20,(-YP+11)*20),10,b
    else
        line((XP+11-XR)*20,(-YP+11-YR)*20)-((XP+11)*20,(-YP+11)*20),12,b
    end if
end sub
sub direction
    if Rotation = 1 then
        line ((XP+10)*20+1,(-YP+10)*20+1)-((XP+11)*20-1,(-YP+10)*20+6),9,bf
    elseif Rotation = 2 then
        line ((XP+11)*20-1,(-YP+10)*20+1)-((XP+11)*20-6,(-YP+11)*20-1),9,bf
    elseif Rotation = 3 then
        line ((XP+10)*20+1,(-YP+11)*20-1)-((XP+11)*20-1,(-YP+11)*20-6),9,bf
    elseif Rotation = 4 then
        line ((XP+10)*20+1,(-YP+10)*20+1)-((XP+10)*20+6,(-YP+11)*20-1),9,bf
    end if
end sub