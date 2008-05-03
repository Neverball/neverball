#IFNDEF __SATISFY__
#ERROR Must compile neverassist.bas
#ELSE
/'
 ' Generator's resources
 '/
#include "nevergen.bi"
/'
 ' This includes all the construction blocks I've implemented so far.
 '/
#include "neverblocks.bas"
sub map_generate
    /'
     ' This subroutine allows generation of a map.
     '/
    color rgb(0,255,255)
    clkey
    cls
    print lang("Which file would you like to enter? ");
    input "",MapFile
    MapFile = MapFile + ".map"
    WindowTitleM = "Neverassistant Program - " + MapFile
    windowtitle WindowTitleM
    print lang("Which environment do you want?")
    print
    print "<L> ";lang("Clouds and Land")
    print "<O> ";lang("Oceans")
    print "<C> ";lang("City and Streets")
    print "<S> ";lang("Jupiter and Space")
    print "<A> ";lang("Alien World")
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

    color rgb(255,255,255)
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
    Check = open(MapFile for output as #m)
    if Check < > 0 then
        clkey
        print lang("Unable to open the map for output: ") + MapFile
        sleep
        exit sub
    end if
    BlockDisplay = ImageCreate(128,128)
    do
        screenlock
        cls
        Warning = 0

        /'
         ' Due to an issue in an older version of FB, I used a formula for
         ' processing this as opposed to making three arrays.
         '/
        PlacementFormula = XP + (YP * 50) + (ZP * 2000) + 100000

        /'
         ' This allows hitting the X in the window to actually close the
         ' program.
         '/
        if inkey = chr(255)+"k" then end

        /'
         ' These move the cursor.
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
         ' If this had been complete, there would have been no commented out
         ' lines.
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
        elseif multikey(SC_Q) then
            BlockType = 17
            if (BlockSet = 0) then
                XR = 1
                YR = 1
                ZR = 1
            end if
        elseif multikey(SC_R) then
            BlockType = 18
            if (BlockSet = 0) then
                XR = 2
                YR = 2
                ZR = 1
            end if
        elseif multikey(SC_S) then
            BlockType = 19
            if (BlockSet = 0) then
                XR = 2
                YR = 2
                ZR = 1
                bload(AssistDir+"/ChicaneRTL.bmp",BlockDisplay)
            end if

        elseif multikey(SC_V) then
            BlockType = 22
            if (BlockSet = 0) then
                XR = 1
                YR = 1
                ZR = 1
            end if
        elseif multikey(SC_W) then
            BlockType = 23
            if (BlockSet = 0) then
                XR = 1
                YR = 1
                ZR = 1
            end if
        elseif multikey(SC_X) then
            BlockType = 24
            if (BlockSet = 0) then
                XR = 1
                YR = 1
                ZR = 1
            end if
        elseif multikey(SC_Y) then
            BlockType = 25
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
        if multikey(SC_CONTROL) then
            if Rotation < 4 then Rotation += 1 else Rotation = 1
        end if

        /'
         ' This keeps the cursor from going out of bounds.
         '/
        if (XP < -10) then XP = -10
        if (YP < -11 + YR) then YP = -11 + YR
        if (ZP < -10) then ZP = -10
        if (XP > 11 - XR) then XP = 11 - XR
        if (YP > 10) then YP = 10
        if (ZP > 21 - ZR) then ZP = 21 - ZR

        /'
         ' Prints information on the right hand side.
         '/
        locate 1,54
        print lang("Location: ");"("&XP;","&YP;","&ZP;")"
        color rgb(0,255,255)
        locate 2,54
        print lang("* Press F1 for help.")
        color rgb(255,255,0)
        locate 4,54
        print lang("Music unsupported.")
        color rgb(255,0,255)
        locate 5,54
        print lang("Time: ");LevelTime
        locate 6,54
        print lang("Blocks: ");Blocks
        locate 7,54
        print lang("Coins: ");TargetCoins;"/";Coins
        locate 8,54
        print lang("$");Money;"/";MaxMoney

        locate 13,54
        if (BlockSet = 0) then
            color rgb(0,255,0)
        elseif (BlockSet = 1) then
            color rgb(255,0,0)
        elseif (Blockset = 2) then
            color rgb(0,255,255)
        elseif (Blockset = 3) then
            color rgb(255,255,0)
        end if

        if (BlockType = 1) AND (BlockSet < > 1) then
            print lang("Start")

        elseif (BlockType = 2) AND (BlockSet < > 1) then
            print lang("Flat straight $1")

        elseif (BlockType = 3) AND (BlockSet = 0) then
            print "90"+chr(248)+" simple flat turn $1"

        elseif (BlockType = 4) AND (BlockSet = 0) then
            print lang("Junction $1")

        elseif (BlockType = 5) AND (BlockSet = 0) then
            print lang("Cross $1")
            
        elseif (BlockType = 6) AND (BlockSet = 0) then
            print lang("Coined flat straight $1")

        elseif (BlockType = 7) AND (BlockSet = 0) then
            print "Coined 90"+chr(248)+" simple flat turn"
            locate 14,54
            print "$1"

        elseif (BlockType = 8) AND (BlockSet = 0) then
            print lang("Dead-end $1")

        elseif (BlockType = 9) AND (BlockSet = 0) then
            print lang("Coined Dead-end $1")

        elseif (BlockType = 10) AND (BlockSet = 0) then
            print lang("Small Jump $2")

        elseif (BlockType = 11) AND (BlockSet = 0) then
            print lang("Coined Junction $1")

        elseif (BlockType = 12) AND (BlockSet = 0) then
            print lang("Coined Cross $1")

        elseif (BlockType = 13) AND (BlockSet = 0) then
            print lang("Ramp straight $1")

        elseif (BlockType = 14) AND (BlockSet = 0) then
            print lang("Bump $1")

        elseif (BlockType = 15) AND (BlockSet = 0) then
            print lang("Coined Bump $1")

        elseif (BlockType = 16) AND (BlockSet = 0) then
            print lang("Flat straight with mover $3")

        elseif (BlockType = 17) AND (BlockSet = 0) then
            print lang("Narrower $3")

        elseif (BlockType = 18) AND (BlockSet = 0) then
            print lang("Chicane LTR $4")

        elseif (BlockType = 19) AND (BlockSet = 0) then
            print lang("Chicane RTL $4")

        elseif (BlockType = 22) AND (BlockSet = 0) then
            print lang("Flat straight Finish $1")

        elseif (BlockType = 23) AND (BlockSet = 0) then
            print "90"+chr(248)+" simple flat turn"
            locate 14,54
            print "Finish $1"

        elseif (BlockType = 24) AND (BlockSet = 0) then
            print lang("Junction Finish $1")

        elseif (BlockType = 25) AND (BlockSet = 0) then
            print lang("Cross Finish $1")

       elseif (BlockType = 26) AND (BlockSet = 0) then
            print lang("Dead-end Finish")

        elseif (BlockSet = 1) then
            print "Not used"
        end if
        put (424,222),BlockDisplay,Trans
        color rgb(255,255,255)

        /'
         ' This gives information about each block.
         '/
        locate 28,1
        if (NOT multikey(SC_F1)) AND (NOT multikey(SC_TILDE)) AND _
           (NOT multikey(SC_BACKSPACE)) then
            if (BlockType = 1) AND (BlockSet = 0) AND (Start = 1) then
                print "You've already placed this block."
            elseif (BlockType = 1) AND (BlockSet = 0) AND (Start = 0) then
                print "Every map made must have one of these. " + _
                      "This is where the player starts. This"
                print "block can't be rotated."
            elseif (BlockType = 2) AND (BlockSet = 0) then
                print lang("This is the most simple block. It is straight and flat.")
            elseif (BlockType = 3) AND (BlockSet = 0) then
                print lang("This is the most simple turn. It is flat.")
            elseif (BlockType = 4) AND (BlockSet = 0) then
                print lang("This is a 3-way block.")
            elseif (BlockType = 5) AND (BlockSet = 0) then
                print lang("This is a 4-way block.")

            elseif (BlockType = 6) AND (BlockSet = 0) then
                print lang("Like a Flat Straight, but with four coins.")
            elseif (BlockType = 7) AND (BlockSet = 0) then
                print "Like a 90"+chr(248)+" simple flat turn, " + _
                      "but with four coins."
            elseif (BlockType = 8) AND (BlockSet = 0) then
                print lang("This block only has one end. Be careful.")
            elseif (BlockType = 9) AND (BlockSet = 0) then
                print lang("Like a Dead-end, but with four coins.")
            elseif (BlockType = 10) AND (BlockSet = 0) then
                print lang("You can jump with this block.")

            elseif (BlockType = 11) AND (BlockSet = 0) then
                print lang("Like a Junction, but with four coins.")
            elseif (BlockType = 12) AND (BlockSet = 0) then
                print lang("Like a Cross, but with four coins.")
            elseif (BlockType = 13) AND (BlockSet = 0) then
                print lang("This block allows you to change elevations.")
            elseif (BlockType = 14) AND (BlockSet = 0) then
                print lang("This block has a bump.")
            elseif (BlockType = 15) AND (BlockSet = 0) then
                print lang("This bump has coins on it.")

            elseif (BlockType = 16) AND (BlockSet = 0) then
                print "Like a Flat Straight, but with a mover on top."
            elseif (BlockType = 17) AND (BlockSet = 0) then
                print lang("This block is narrow in the middle.")
            elseif (BlockType = 18) AND (BlockSet = 0) then
                print lang("This block shifts from left to right.")
            elseif (BlockType = 19) AND (BlockSet = 0) then
                print lang("This block shifts from right to left.")

            elseif (BlockType = 22) AND (BlockSet = 0) then
                print "Every map made must have at least one of these. " + _
                      "This is where the player"
                print "finishes at a straight."
            elseif (BlockType = 23) AND (BlockSet = 0) then
                print "Every map made must have at least one of these. " + _
                      "This is where the player"
                print "finishes at a turn."
            elseif (BlockType = 24) AND (BlockSet = 0) then
                print "Every map made must have at least one of these. " + _
                      "This is where the player"
                print "finishes at a junction."
            elseif (BlockType = 25) AND (BlockSet = 0) then
                print "Every map made must have at least one of these. " + _
                      "This is where the player"
                print "finishes at a cross."
            elseif (BlockType = 26) AND (BlockSet = 0) then
                print "Every map made must have at least one of these. " + _
                      "This is where the player"
                print "finishes at a dead-end."

            elseif (BlockType = 1) AND (BlockSet = 2) AND (Start = 1) then
                print "You've already placed this block."
            elseif (BlockType = 1) AND (BlockSet = 2) AND _
                   (Start = 0) AND (ZP < > 0) then
                print "This block must be placed at elevation level 0."
            elseif (BlockType = 1) AND (BlockSet = 2) AND _
                   (Start = 0) AND (ZP = 0) then
                print "Every map made must have one of these. " + _
                      "This is where the player starts. This"
                print "block can't be rotated."
            elseif (BlockType = 2) AND (BlockSet = 2) AND (ZP < > 0) then
                print "This block must be placed at elevation level 0."
            elseif (BlockType = 2) AND (BlockSet = 2) AND (ZP = 0) then
                print "This is a shiny block. It is straight and flat."
            end if

        elseif multikey(SC_F1) then
            clkey
            screenunlock
            cls
            color rgb(0,255,255)
            print "Controls:"
            print "* CONTROL: rotate"
            print "* TAB: toggle music (when its supported)"
            print "* SPACEBAR: place block"
            print "* Arrow keys: move cursor"
            print "* PAGE UP and PAGE DOWN: change elevation.
            print "* Any English letter: change blocks."
            print "* PLUS and MINUS without LSHIFT: adjust time."
            print "* PLUS and MINUS with LSHIFT: adjust target coins."
            print "* BACKSPACE: Clear map and reset settings."

            print "* F2: change to "+chr(34)+"Basic Neverball"+chr(34) + _
                  " block set. You can't access this set if"
            print " blocks from the "+chr(34)+"Neverputt"+chr(34)+" set " + _
                  "are placed."
            print "* F3: change to "+chr(34)+"Advanced Neverball"+chr(34) + _
                  " block set. You can't access this set if"
            print " blocks from the "+chr(34)+"Neverputt"+chr(34)+" set " + _
                  "are placed."
            print "* F4: change to "+chr(34)+"Extra textures"+chr(34) + _
                  " block set. You can't access this set if blocks"
            print " from the "+chr(34)+"Neverputt"+chr(34)+" set are placed."
            print "* F5: change to "+chr(34)+"Neverputt"+chr(34)+" block " + _
                  "set. You can't access this set if blocks from"
            print " other sets are placed."
            print "* ~: to check for issues. You can't save the map until " + _
                  "you ensure there are no"
            print " errors."
            color rgb(255,255,255)
            sleep
            cls

        elseif multikey(SC_TILDE) then
            clkey
            screenunlock
            cls
            if Warning = 0 then
                color rgb(0,255,0)
                print "Check okay. There are no offending items."
            else
                color rgb(255,0,0)
                if Warning = 1 then
                    print "Check failed. There is 1 offending item."
                else
                    print "Check failed. There are ";Warning;" offending items."
                end if
                color rgb(255,128,0)
                if Start = 0 then print "- Must have 1 start block."
                if Finish = 0 then print "- Must have at least 1 finish block."
                if TargetCoins > Coins then
                    print "- The number of required coins exceeds the " + _
                          "coins present."
                end if
                if MinimumLevelTime > LevelTime then
                    print "- The time given for a level must equal or " + _
                          "exceed ";MinimumLevelTime;"."
                end if
            end if
            color rgb(255,255,0)

            if Openings > 0 then
                print "- You have ";Openings;" openings that you haven't " + _
                      "closed yet. If some of your blocks were"
                print " intended to merge roads together, you can hit " + _
                      "the "+chr(34)+"/"+chr(34)+" key to merge together."
            end if
            if Money * 1.2 > MaxMoney then
                print "- You are running low on money. You should stop " + _
                      "building this map soon. You"
                print "only have $"& MaxMoney-Money ;" remaining. If you " + _
                      "want more, you have to beat more levels"
                print "in the Neverball game. They will give you more money."
            end if
            color rgb(255,255,255)
            sleep
            cls

        elseif multikey(SC_BACKSPACE) then
            color rgb(255,255,255)
            screenunlock
            cls
            print lang("[Y/N] Really clear whole map?")
            do
                sleep 20
                if multikey(SC_Y) then
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
                                PlacementFormula = XP + (YP * 50) + _
                                                  (ZP * 2000) + 100000
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
                    close #m
                    open MapFile for output as #m
                    exit do
                end if
            loop until multikey(SC_N)
        end if

        /'
         ' This places the block
         '/
        if multikey(SC_SPACE) then place_block

        /'
         ' Generator graphics
         '/
        base_gfx
        cursor
        direction
        screenunlock

        sleep 125,1

    loop until multikey(SC_ESCAPE)

    if (Warning = 0) then
        cls
        clkey
        color rgb(0,255,0)
        print "Success."
        color rgb(255,255,255)
        input "What would you like in your message";LevelMessage
        input "What is the name of this level";LevelName

        print #m, "// entity ";Entity;"
        print #m, "{"
        print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"worldspawn"+chr(34)
        if (LevelMessage < > "") then
            print #m, chr(34)+"message"+chr(34)+" " + _
                      chr(34);LevelMessage;chr(34)
        end if
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
        Check = exec("Mapc.exe",MapFile + " data")
        #ELSE
        Check = exec("mapc",MapFile + " data")
        #ENDIF

        if (Check < > -1) then
            cls
            print "It is now ready for play."
            print
            print "If this is your first level, you need to create a " + _
                  "set-XXX.txt in the data"
            print "folder with the following contents. " + _
                  "Brackets are variables."

            color rgb(0,255,255)
            print "[Name of Set]"
            print "[Description]"
            print "[ID]"
            print "[Set Shot]"
            print "[Hard BT]","[Hard MT]","[Med BT]", _
                  "[Med MT]","[Easy BT]","[Easy MT]"
            print "[Level files] (relative to data folder)"

            color rgb(255,255,255)
            print
            print "Add the name of the file to sets.txt"
            print
            print "It is preferable that you make a folder in the " + _
                  "data directory to store all of"
            print "your levels."
            print
            print "If you already did this before, you only need " + _
                  "to update the set-XXX.txt with"
            print "the new level."
            sleep
        end if

    else
        cls
        color rgb(255,0,0)
        print "Failed."
        color rgb(255,255,255)
        close #m
        kill(MapFile)
    end if
    if inkey = chr(27) then sleep
    clkey
    menu
    ImageDestroy(BlockDisplay)
end sub
#ENDIF