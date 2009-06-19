/'
 ' The Level Generator is one of the Neverassistant's most useful features.
 '
 ' It significantly simplifies the level creation process. Although this makes
 ' level creation easier, it is not intended to replace GtkRadiant as it has
 ' a limited capacity in generating. You can still edit maps in Radiant, but
 ' Neverassistant allows easy generation of these maps.
 '
 ' For example, each individual "brush" as Radiant calls it or "lump" as
 ' Neverball calls does not really exist in the Assistant. It uses the term
 ' "construction block", which can be comprised of multiple lumps or entities.
 '
 ' Radiant allows virtually limitless freeform placement. The Neverassistant
 ' is presently limited to a 21x21x31 grid, with at most one
 ' "construction block" in each slot. Also, the Neverassistant makes one
 ' worldspawn entity for each construction block and one entity for metadata.
 ' (with extremely few exceptions).
 '
 ' Also, the Neverassistant presently has only a limited number of construction
 ' blocks (presently 31). This number will be increased as more ideas are
 ' available.
 '/

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
        dim as ubyte Hold(128)
        dim as string BlockSets(0 to 4) => {"Required Blocks (6)", _
            "Basic Blocks I (9)", "Basic Blocks II (8)", _
            "Advanced Blocks (7)", "Underground Blocks (1)"}
        /'
         ' This subroutine allows generation of a map.
         '/
        color rgb(0,255,255)
        clkey
        cls
        print lang("Which file would you like to enter? ");
        input "",MapFile
        MapFile = MapFile
        WindowTitleM = "Neverassistant - Map Generator: " + MapFile
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
                MusicPlay = "bgm/track5.ogg"
                exit do
            elseif multikey(SC_C) then
                Song = "bgm/track3.ogg"
                Back = "map-back/city.sol"
                Grad = "back/city.png"
                MusicPlay = "bgm/track3.ogg"
                exit do
            elseif multikey(SC_L) then
                Song = "bgm/track1.ogg"
                Back = "map-back/clouds.sol"
                Grad = "back/land.png"
                MusicPlay = "bgm/track1.ogg"
                exit do
            elseif multikey(SC_O) then
                Song = "bgm/track2.ogg"
                Back = "map-back/ocean.sol"
                Grad = "back/ocean.png"
                MusicPlay = "bgm/track2.ogg"
                exit do
            elseif multikey(SC_S) then
                Song = "bgm/track4.ogg"
                Back = "map-back/jupiter.sol"
                Grad = "back/space.png"
                MusicPlay = "bgm/track4.ogg"
                exit do
            elseif inkey = chr(255)+"k" then
                end
            end if
        loop
        music = Mix_LoadMUS("data/"+MusicPlay)
        MusicSwitch = 1
        Mix_PlayMusic(music, -1)

        color rgb(255,255,255)
        cls
        Start = 0
        Finish(1) = 0
        Finish(2) = 0
        LevelTime = 0
        Blocks = 0
        Coins = 0
        TargetCoins = 0
        MinimumLevelTime = 0
        UsedMoney = 0
        Putt = 0
        for XP = -10 to 10
            for YP = -10 to 10
                for ZP = -10 to 20
                    Contents(XP,YP,ZP) = 0
                    Direction(XP,YP,ZP) = 0
                next
            next
        next
        XP = 0
        YP = 0
        ZP = 0
        BlockType = 0
        Rotation = 1
        BlockSet = 0
        XR = 1
        YR = 1
        ZR = 1
        BlockDisplay = ImageCreate(128,128)
        do
            screenlock
            cls
            Warning = 0

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
            /'
             ' The left and right arrow keys move the cursor along the X-axis
             ' of the generator.
             '/
            if multikey(SC_UP) then
                YP += 1
            elseif multikey(SC_DOWN) then
                YP -= 1
            end if
            /'
             ' The up and down arrow keys move the cursor along the Y-axis
             ' of the generator.
             '/
            if multikey(SC_PAGEUP) then
                ZP += 1
            elseif multikey(SC_PAGEDOWN) then
                ZP -= 1
            end if
            /'
             ' The page up and page down keys move the cursor along the Z-axis
             ' of the generator. (elevation level)
             '
             ' Though you won't notice a change in the cursor graphics-wise,
             ' you will notice a change in the coordinates. Additionally, some
             ' contents may change to reflect the new elevation level.
             '/

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
            if multikey(SC_SLASH) AND (Hold(SC_SLASH) = 0) then
                Hold(SC_SLASH) = 100
                if Openings > = 2 then Openings -= 2
            end if

            /'
             ' This prevents maps made with this program from getting away
             ' without certain items.
             '/
            if Start = 0 then Warning += 1
            if Finish(1) = 0 then Warning += 1
            if TargetCoins > Coins then Warning += 1
            if MinimumLevelTime > LevelTime then Warning += 1

            /'
             ' This allows you to switch block sets.
             '/
            if multikey(SC_F2) AND (Putt < > 1) then
                BlockSet = 0
                BlockType = 0
                XR = 1
                YR = 1
                ZR = 1
                clear_preview
            elseif multikey(SC_F3) AND (Putt < > 1) then
                BlockSet = 1
                BlockType = 0
                XR = 1
                YR = 1
                ZR = 1
                clear_preview
            elseif multikey(SC_F4) AND (Putt < > 1) then
                BlockSet = 2
                BlockType = 0
                XR = 1
                YR = 1
                ZR = 1
                clear_preview
            elseif multikey(SC_F5) AND (Putt < > 1) then
                BlockSet = 3
                BlockType = 0
                XR = 1
                YR = 1
                ZR = 1
                clear_preview
            elseif multikey(SC_F6) AND (Putt < > 1) then
                BlockSet = 4
                BlockType = 0
                XR = 1
                YR = 1
                ZR = 1
                clear_preview
            end if

            /'
             ' Each number from 1 to 9 sets an indivudual block.
             '/
            if BlockSet = 0 then
                if multikey(SC_1) then
                    BlockType = 1
                    XR = 1
                    YR = 1
                    ZR = 1
                    bload(AssistDir+"/Start.bmp",BlockDisplay)
                elseif multikey(SC_2) then
                    BlockType = 2
                    XR = 1
                    YR = 1
                    ZR = 1
                    bload(AssistDir+"/Finish.bmp",BlockDisplay)
                elseif multikey(SC_3) then
                    BlockType = 3
                    XR = 1
                    YR = 1
                    ZR = 1
                    bload(AssistDir+"/Finish.bmp",BlockDisplay)

                elseif multikey(SC_4) then
                    BlockType = 4
                    XR = 1
                    YR = 1
                    ZR = 1
                    bload(AssistDir+"/Finish.bmp",BlockDisplay)
                elseif multikey(SC_5) then
                    BlockType = 5
                    XR = 1
                    YR = 1
                    ZR = 1
                    bload(AssistDir+"/Finish.bmp",BlockDisplay)
                elseif multikey(SC_6) then
                    BlockType = 6
                    XR = 1
                    YR = 1
                    ZR = 1
                    bload(AssistDir+"/Finish.bmp",BlockDisplay)
                end if

            elseif BlockSet = 1 then
                if multikey(SC_1) then
                    BlockType = 11
                    XR = 1
                    YR = 1
                    ZR = 1
                    bload(AssistDir+"/FlatNormal.bmp",BlockDisplay)
                elseif multikey(SC_2) then
                    BlockType = 12
                    XR = 1
                    YR = 1
                    ZR = 1
                    bload(AssistDir+"/FlatNormal.bmp",BlockDisplay)
                elseif multikey(SC_3) then
                    BlockType = 13
                    XR = 1
                    YR = 1
                    ZR = 1
                    bload(AssistDir+"/FlatNormal.bmp",BlockDisplay)
                elseif multikey(SC_4) then
                    BlockType = 14
                    XR = 1
                    YR = 1
                    ZR = 1
                    bload(AssistDir+"/FlatNormal.bmp",BlockDisplay)
                elseif multikey(SC_5) then
                    BlockType = 15
                    XR = 1
                    YR = 1
                    ZR = 1
                    bload(AssistDir+"/FlatNormal.bmp",BlockDisplay)

                elseif multikey(SC_6) then
                    BlockType = 16
                    XR = 1
                    YR = 1
                    ZR = 1
                    clear_preview
                elseif multikey(SC_7) then
                    BlockType = 17
                    XR = 1
                    YR = 1
                    ZR = 1
                    bload(AssistDir+"/FlatNormal.bmp",BlockDisplay)
                elseif multikey(SC_8) then
                    BlockType = 18
                    XR = 1
                    YR = 1
                    ZR = 1
                    clear_preview
                elseif multikey(SC_9) then
                    BlockType = 19
                    XR = 1
                    YR = 1
                    ZR = 1
                    clear_preview
                end if

            elseif BlockSet = 2 then
                if multikey(SC_1) then
                    BlockType = 21
                    XR = 1
                    YR = 1
                    ZR = 1
                    clear_preview
                elseif multikey(SC_2) then
                    BlockType = 22
                    XR = 1
                    YR = 1
                    ZR = 1
                    bload(AssistDir+"/Ramp.bmp",BlockDisplay)
                elseif multikey(SC_3) then
                    BlockType = 23
                    XR = 1
                    YR = 1
                    ZR = 1
                    bload(AssistDir+"/FlatNormal.bmp",BlockDisplay)
                elseif multikey(SC_4) then
                    BlockType = 24
                    XR = 1
                    YR = 1
                    ZR = 1
                    bload(AssistDir+"/FlatNormal.bmp",BlockDisplay)
                elseif multikey(SC_5) then
                    BlockType = 25
                    XR = 1
                    YR = 1
                    ZR = 1
                    clear_preview
                elseif multikey(SC_6) then
                    BlockType = 26
                    XR = 1
                    YR = 1
                    ZR = 1
                    clear_preview
                elseif multikey(SC_7) then
                    BlockType = 27
                    XR = 1
                    YR = 1
                    ZR = 1
                    clear_preview
                elseif multikey(SC_8) then
                    BlockType = 28
                    XR = 1
                    YR = 1
                    ZR = 1
                    clear_preview
                end if

            elseif BlockSet = 3 then
                if multikey(SC_1) then
                    BlockType = 41
                    XR = 1
                    YR = 1
                    ZR = 1
                    clear_preview
                elseif multikey(SC_2) then
                    BlockType = 42
                    XR = 1
                    YR = 1
                    ZR = 1
                    clear_preview
                elseif multikey(SC_3) then
                    BlockType = 43
                    XR = 1
                    YR = 1
                    ZR = 1
                    clear_preview
                elseif multikey(SC_4) then
                    BlockType = 44
                    XR = 2
                    YR = 2
                    ZR = 1
                    bload(AssistDir+"/ChicaneLTR.bmp",BlockDisplay)
                elseif multikey(SC_5) then
                    BlockType = 45
                    XR = 2
                    YR = 2
                    ZR = 1
                    bload(AssistDir+"/ChicaneRTL.bmp",BlockDisplay)
                elseif multikey(SC_6) then
                    BlockType = 46
                    XR = 3
                    YR = 3
                    ZR = 1
                    bload(AssistDir+"/ChicaneLTR.bmp",BlockDisplay)
                elseif multikey(SC_7) then
                    BlockType = 47
                    XR = 3
                    YR = 3
                    ZR = 1
                    bload(AssistDir+"/ChicaneRTL.bmp",BlockDisplay)
                end if

            elseif BlockSet = 4 then
                if multikey(SC_1) then
                    BlockType = 51
                    XR = 1
                    YR = 1
                    ZR = 1
                    clear_preview
                end if
            end if

            /'
             ' This rotates the block
             '/
            if multikey(SC_CONTROL) AND Hold(SC_CONTROL) = 0 then
                if Rotation < 4 then Rotation += 1 else Rotation = 1
                Hold(SC_CONTROL) = 1
            end if

            /'
             ' This toggles the music
             '/
            if multikey(SC_TAB) AND Hold(SC_TAB) = 0 then
                MusicSwitch = iif(MusicSwitch = 0,1,0)
                Hold(SC_TAB) = 1
                if MusicSwitch = 0 then
                    Mix_HaltMusic
                else
                    Mix_PlayMusic(music, -1)
                end if
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
            locate 4,54
            if MusicSwitch = 0 then
                color rgb(255,255,255)
                print lang("Music disabled")
            else
                color rgb(255,255,0)
                print lang("Music activated")
            end if
            color rgb(255,0,255)
            locate 5,54
            print lang("Time: ");LevelTime
            locate 6,54
            print lang("Blocks: ");Blocks
            locate 7,54
            print lang("Coins: ");TargetCoins;"/";Coins

            color rgb(255,255,255)
            locate 13,54
            print lang(BlockSets(BlockSet))
            locate 14,54
            if (BlockType = 0) then
                print lang("No block selected")
            elseif (BlockType = 1) then
                print lang("Start")
            elseif (BlockType = 2) then
                print lang("Flat straight Finish")
            elseif (BlockType = 3) then
                print lang("90deg turn Finish")
            elseif (BlockType = 4) then
                print lang("Junction Finish")
            elseif (BlockType = 5) then
                print lang("Cross Finish")
            elseif (BlockType = 6) then
                print lang("Dead-end Finish")

            elseif (BlockType = 11) then
                print lang("Flat straight")
            elseif (BlockType = 12) then
                print lang("90deg simple flat turn")
            elseif (BlockType = 13) then
                print lang("Junction")
            elseif (BlockType = 14)then
                print lang("Cross")
            elseif (BlockType = 15) then
                print lang("Coined flat straight")
            elseif (BlockType = 16) then
                print lang("Coined 90deg flat turn")
            elseif (BlockType = 17) then
                print lang("Dead-end")
            elseif (BlockType = 18) then
                print lang("Coined Dead-end")
            elseif (BlockType = 19) then
                print lang("Coined Junction")

            elseif (BlockType = 21) then
                print lang("Coined Cross")
            elseif (BlockType = 22) then
                print lang("Ramp straight")
            elseif (BlockType = 23) then
                print lang("Bump")
            elseif (BlockType = 24) then
                print lang("Coined Bump")
            elseif (BlockType = 25) then
                print lang("Vertical Platform")
            elseif (BlockType = 26) then
                print lang("Coined Vertical Platform")
            elseif (BlockType = 27) then
                print lang("Fast Vertical Platform")
            elseif (BlockType = 28) then
                print lang("Coined Fast V. Platform")

            elseif (BlockType = 41) then
                print lang("Small Jump")
            elseif (BlockType = 42) then
                print lang("Flat straight with mover")
            elseif (BlockType = 43) then
                print lang("Narrower")
            elseif (BlockType = 44) then
                print lang("Chicane LTR 2x2")
            elseif (BlockType = 45) then
                print lang("Chicane RTL 2x2")
            elseif (BlockType = 46) then
                print lang("Chicane LTR 3x3")
            elseif (BlockType = 47) then
                print lang("Chicane RTL 3x3")

            elseif (BlockType = 51) then
                print lang("Underground Straight")
            end if
            put (424,226),BlockDisplay,Trans

            /'
             ' This gives information about each block.
             '/
            locate 28,1
            if (NOT multikey(SC_F1)) AND (NOT multikey(SC_TILDE)) AND _
               (NOT multikey(SC_BACKSPACE)) then
                if (BlockType = 0) then
                    print lang("You have no block selected.")
                elseif (BlockType = 1) then
                    print lang("This is where the player starts. " + _
                        "Not rotatable.")
                elseif (BlockType = 2) then
                    print lang("This is where the player finishes at a " + _
                        "straight.")
                elseif (BlockType = 3) then
                    print lang("This is where the player finishes at a turn.")
                elseif (BlockType = 4) then
                    print lang("This is where the player finishes at a " + _
                        "junction.")
                elseif (BlockType = 5) then
                    print lang("This is where the player finishes at a cross.")
                elseif (BlockType = 6) then
                    print lang("This is where the player finishes at a " + _
                        "dead-end.")

                elseif (BlockType = 11) then
                    print lang("This is the most simple block. It is " + _
                        "straight and flat.")
                elseif (BlockType = 12) then
                    print lang("This is the most simple turn. It is flat.")
                elseif (BlockType = 13) then
                    print lang("This is the most simple 3-way block.")
                elseif (BlockType = 14) then
                    print lang("This is the most simple 4-way block.")
                elseif (BlockType = 15) then
                    print lang("Like a Flat Straight, but with four coins.")
                elseif (BlockType = 16) then
                    print lang("Like a 90deg flat turn, but with four coins.")
                elseif (BlockType = 17) then
                    print lang("This block only has one end. Be careful.")
                elseif (BlockType = 18) then
                    print lang("Like a Dead-end, but with four coins.")
                elseif (BlockType = 19) then
                    print lang("Like a Junction, but with four coins.")
                elseif (BlockType = 21) then
                    print lang("Like a Cross, but with four coins.")
                elseif (BlockType = 22) then
                    print lang("This block allows you to change elevations.")
                elseif (BlockType = 23) then
                    print lang("This block has a bump.")
                elseif (BlockType = 24) then
                    print lang("This bump has coins on it.")
                elseif (BlockType = 25) then
                    print lang("Vertical Platform that goes up and down.")
                elseif (BlockType = 26) then
                    print lang("Vertical platform with coins.")
                elseif (BlockType = 25) then
                    print lang("Vertical platform with a very high speed.")
                elseif (BlockType = 25) then
                    print lang("Vertical platform that combines coins and high speeds!")

                elseif (BlockType = 41) then
                    print lang("You can jump with this block.")
                elseif (BlockType = 42) then
                    print lang("Like a Flat Straight, but with a mover on top.")
                elseif (BlockType = 43) then
                    print lang("This block is narrow in the middle.")
                elseif (BlockType = 44) then
                    print lang("This 2x2 block shifts from left to right.")
                elseif (BlockType = 45) then
                    print lang("This 2x2 block shifts from right to left.")
                elseif (BlockType = 46) then
                    print lang("This 3x3 block shifts from left to right.")
                elseif (BlockType = 47) then
                    print lang("This 3x3 block shifts from right to left.")
                elseif (BlockType = 47) then
                    print lang("This straight carries a tunnel above it.")
                end if

            elseif multikey(SC_F1) then
                clkey
                screenunlock
                cls
                color rgb(0,255,255)
                print lang("Controls:")
                print lang("* CONTROL: rotate")
                print lang("* TAB: toggle music")
                print lang("* SPACEBAR: place block")
                print lang("* Arrow keys: move cursor")
                print lang("* PAGE UP and PAGE DOWN: change elevation.")
                print lang("* PLUS and MINUS without LSHIFT: adjust time.")
                print lang("* PLUS and MINUS with LSHIFT: adjust target coins.")
                print lang("* BACKSPACE: Clear map and reset settings.")

                print "* F2-F5: Changes block set."
                print "* 1-9: Changes block. Not all sets have 9 blocks" + _
                    " in them."
                print "* ~: to check for issues. You can't save the map " + _
                      "until you ensure there are no"
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
                    print lang("Check okay. There are no offending items.")
                else
                    color rgb(255,0,0)
                    print lang("Check failed. Check the following" + _
                        " offending items.")
                    color rgb(255,128,0)
                    if Start = 0 then
                        print lang("- Must have 1 start block.")
                    end if
                    if Finish(1) = 0 then
                        print lang("- Must have at least 1 finish block.")
                    end if
                    if TargetCoins > Coins then
                        print lang("- The number of required coins " + _
                              "exceeds the coins present.")
                    end if
                    if MinimumLevelTime > LevelTime then
                        print "- The time given for a level must equal or " + _
                              "exceed ";MinimumLevelTime;"."
                    end if
                end if
                color rgb(255,255,0)

                if Openings > 0 then
                    print "- You have ";Openings;" openings that you " + _
                          "haven't closed yet. If some of your blocks were"
                    print " intended to merge roads together, you can hit " + _
                          "the "+chr(34)+"/"+chr(34)+" key to merge together."
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
                        Finish(1) = 0
                        Finish(2) = 0
                        LevelTime = 0
                        Blocks = 0
                        Coins = 0
                        TargetCoins = 0
                        MinimumLevelTime = 0
                        MusicSwitch = 0
                        UsedMoney = 0
                        Putt = 0
                        Openings = 0

                        for XP = -10 to 10
                            for YP = -10 to 10
                                for ZP = -10 to 20
                                    Contents(XP,YP,ZP) = 0
                                next
                            next
                        next
                        XP = 0
                        YP = 0
                        ZP = 0
                        BlockSet = 0
                        BlockType = 0
                        Rotation = 1
                        XR = 1
                        YR = 1
                        ZR = 1
                        exit do
                    end if
                loop until multikey(SC_N)
            end if

            /'
             ' This places the block
             '/
            if multikey(SC_SPACE) then place_block
            for KID as ubyte = 1 to 128
                if Hold(KID) AND multikey(KID) = 0 then Hold(KID) = 0
            next KID

            /'
             ' Generator graphics
             '/
            base_gfx
            cursor
            direction_gfx
            screenunlock

            sleep 100,1

        loop until multikey(SC_ESCAPE)

        if (Warning = 0) then
            cls
            clkey
            color rgb(0,255,0)
            print "Success."
            color rgb(255,255,255)
            input "What would you like in your message";LevelMessage
            input "What is the name of this level";LevelName

            Check = open(MapFile+".map" for output as #m)
            if Check = 0 then
                compile_blocks
                print #m, "// entity ";Entity
                print #m, "{"
                print #m, chr(34)+"classname"+chr(34)+" " + _
                    chr(34)+"worldspawn"+chr(34)
                if (LevelMessage < > "") then
                    print #m, chr(34)+"message"+chr(34)+" " + _
                              chr(34);LevelMessage;chr(34)
                end if
                print #m, chr(34)+"time"+chr(34)+" "+chr(34);LevelTime;chr(34)
                print #m, chr(34)+"goal"+chr(34)+" "+chr(34);TargetCoins;chr(34)
                print #m, chr(34)+"back"+chr(34)+" "+chr(34);Back;chr(34)
                print #m, chr(34)+"grad"+chr(34)+" "+chr(34);Grad;chr(34)
                print #m, chr(34)+"song"+chr(34)+" "+chr(34);Song;chr(34)
                print #m, chr(34)+"coin_hs"+chr(34)+" "+chr(34);Coins;chr(34)
                print #m, chr(34)+"levelname"+chr(34)+" " + _
                    chr(34);LevelName;chr(34)
                print #m, "}"
                close #m

                print "Your map is almost finished..."
                #IFDEF __FB_WIN32__
                    Check = exec("Mapc.exe",MapFile + ".map data")
                #ELSE
                    Check = exec("mapc",MapFile + ".map data")
                #ENDIF

                if (Check < > -1) then
                    cls
                    print "It is now ready for play."
                    print
                    print "If this is your first level, you need to" + _
                          " create a set-XXX.txt in the data"
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
                else
                    cls
                    color rgb(255,0,0)
                    print lang("Unable to open the map for output.")
                    color rgb(255,255,255)
                end if
            end if
        else
            cls
            color rgb(255,0,0)
            print lang("Failed.")
            color rgb(255,255,255)
        end if
        if inkey = chr(27) then sleep
        clkey
        Mix_HaltMusic
        Mix_FreeMusic(music)
        music = NULL
        main_menu
        ImageDestroy(BlockDisplay)
    end sub
#ENDIF
