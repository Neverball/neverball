#IFNDEF __SATISFY__
    #ERROR Must compile neverassist.bas
#ELSE
    /'
     ' Welcome to the construction blocks source file.
     '/
    #include "neverblocks.bi"

    declare sub place_block
    sub place_block
        /'
         ' Should the money system ever come back, I'm keeping some of the code
         ' intact. Until then, you may assume infinite money.
         '/
        dim as integer Money = 1e+9
        /'
         ' BlockType is the type of the block you're using.
         '
         ' BlockSet is the set of blocks you're using.
         '
         ' Start is exclusive to the start block. It allows you to place the
         ' block only once.
         '
         ' XP is the X Position of the assistant's cursor.
         '
         ' YP is the Y Position of the assistant's cursor. It's requirement is
         ' also exclusive to the start block.
         '
         ' ZP is the Z Position of the assistant's cursor.
         '
         ' UsedMoney is the current amount of money you expended. Money is the
         ' maximum you can spend. Money spent on each map is deducted from your
         ' account, so use them wisely.
         '
         ' Contents prevents you from overriding blocks with each other.
         '/

        if (BlockType = 1) AND (Start = 0) AND (YP < 10) AND _
            Contents(XP,YP,ZP) = 0 then
            /'
             ' Plonks the block in its own memory.
             '/
            Start = 1
            Contents(XP,YP,ZP) = 1
            Direction(XP,YP,ZP) = 1
            LevelTime += 150
            MinimumLevelTime += 50
            Blocks += 1
            Openings += 1
            Putt = 2
        elseif (BlockType = 2) AND (UsedMoney + 1 < = Money) AND _
            Contents(XP,YP,ZP) = 0 then
            Contents(XP,YP,ZP) = 2
            Direction(XP,YP,ZP) = Rotation
            Finish(1) += 1
            LevelTime += 125
            MinimumLevelTime += 100
            UsedMoney += 1
            Blocks += 1
            Putt = 2
        elseif (BlockType = 3) AND (UsedMoney + 1 < = Money) AND _
            Contents(XP,YP,ZP) = 0 then
            Contents(XP,YP,ZP) = 3
            Direction(XP,YP,ZP) = Rotation
            Finish(1) += 1
            LevelTime += 225
            MinimumLevelTime += 175
            UsedMoney += 1
            Blocks += 1
            Putt = 2

        elseif (BlockType = 4) AND (UsedMoney + 1 < = Money) AND _
            Contents(XP,YP,ZP) = 0 then
            Contents(XP,YP,ZP) = 24
            Direction(XP,YP,ZP) = Rotation
            Finish(1) += 1
            LevelTime += 325
            MinimumLevelTime += 225
            UsedMoney += 1
            Blocks += 1
            Openings += 1
            Putt = 2
        elseif (BlockType = 5) AND (UsedMoney + 1 < = Money) AND _
            Contents(XP,YP,ZP) = 0 then
            Contents(XP,YP,ZP) = 5
            Direction(XP,YP,ZP) = Rotation
            Finish(1) += 1
            LevelTime += 425
            MinimumLevelTime += 325
            Blocks += 1
            Openings += 2
            Putt = 2
        elseif (BlockType = 6) AND (Openings > 0) AND _
            Contents(XP,YP,ZP) = 0 then
            /'
             ' I didn't put a Money statement on this block because it doesn't
             ' cost any. However, you must have a free opening.
             '/
            Contents(XP,YP,ZP) = 6
            Direction(XP,YP,ZP) = Rotation
            Finish(1) += 1
            LevelTime += 150
            MinimumLevelTime += 50
            Blocks += 1
            Finish(1) += 1
            Openings -= 1
            Putt = 2

        elseif (BlockType = 11) AND _
            (UsedMoney + 1 < = Money) AND _
            Contents(XP,YP,ZP) = 0 then
            Contents(XP,YP,ZP) = 11
            Direction(XP,YP,ZP) = Rotation
            LevelTime += 100
            MinimumLevelTime += 75
            UsedMoney += 1
            Blocks += 1
            Putt = 2
        elseif (BlockType = 12) AND _
            (UsedMoney + 1 < = Money) AND _
            Contents(XP,YP,ZP) = 0 then
            Contents(XP,YP,ZP) = 12
            Direction(XP,YP,ZP) = Rotation
            LevelTime += 200
            MinimumLevelTime += 150
            UsedMoney += 1
            Blocks += 1
            Putt = 2
        elseif (BlockType = 13) AND _
            (UsedMoney + 1 < = Money) AND _
            Contents(XP,YP,ZP) = 0 then
            Contents(XP,YP,ZP) = 13
            Direction(XP,YP,ZP) = Rotation
            LevelTime += 300
            MinimumLevelTime += 200
            UsedMoney += 1
            Blocks += 1
            Openings += 1
            Putt = 2

        elseif (BlockType = 14) AND _
            (UsedMoney + 1 < = Money) AND _
            Contents(XP,YP,ZP) = 0 then
            Contents(XP,YP,ZP) = 14
            Direction(XP,YP,ZP) = Rotation
            LevelTime += 400
            MinimumLevelTime += 300
            Blocks += 1
            Openings += 2
            Putt = 2
        elseif (BlockType = 15) AND _
            (UsedMoney + 1 < = Money) AND _
            Contents(XP,YP,ZP) = 0 then
            Contents(XP,YP,ZP) = 15
            Direction(XP,YP,ZP) = Rotation
            LevelTime += 175
            MinimumLevelTime += 125
            TargetCoins += 1
            Coins += 4
            UsedMoney += 1
            Blocks += 1
            Putt = 2
        elseif (BlockType = 16) AND _
            (UsedMoney + 1 < = Money) AND _
            Contents(XP,YP,ZP) = 0 then
            Contents(XP,YP,ZP) = 16
            Direction(XP,YP,ZP) = Rotation
            LevelTime += 275
            MinimumLevelTime += 200
            TargetCoins += 1
            Coins += 4
            UsedMoney += 1
            Blocks += 1
            Putt = 2

        elseif (BlockType = 17) AND (Openings > 0) AND _
            (UsedMoney + 1 < = Money) AND _
            Contents(XP,YP,ZP) = 0 then
            Contents(XP,YP,ZP) = 17
            Direction(XP,YP,ZP) = Rotation
            LevelTime += 50
            Blocks += 1
            UsedMoney += 1
            Openings -= 1
            Putt = 2
        elseif (BlockType = 18) AND (Openings > 0) AND _
            (UsedMoney + 1 < = Money) AND _
            Contents(XP,YP,ZP) = 0 then
            Contents(XP,YP,ZP) = 18
            Direction(XP,YP,ZP) = Rotation
            LevelTime += 150
            MinimumLevelTime += 100
            TargetCoins += 1
            Coins += 4
            Blocks += 1
            UsedMoney += 1
            Openings -= 1
            Putt = 2
        elseif (BlockType = 19) AND _
            (UsedMoney + 1 < = Money) AND _
            Contents(XP,YP,ZP) = 0 then
            Contents(XP,YP,ZP) = 19
            Direction(XP,YP,ZP) = Rotation
            LevelTime += 425
            MinimumLevelTime += 300
            TargetCoins += 1
            Coins += 4
            Blocks += 1
            UsedMoney += 1
            Openings += 1
            Putt = 2

        elseif (BlockType = 21) AND _
            (UsedMoney + 1 < = Money) AND _
            Contents(XP,YP,ZP) = 0 then
            Contents(XP,YP,ZP) = 21
            Direction(XP,YP,ZP) = Rotation
            LevelTime += 575
            MinimumLevelTime += 400
            TargetCoins += 1
            Coins += 4
            Blocks += 1
            UsedMoney += 1
            Openings += 2
            Putt = 2
        elseif (BlockType = 22) AND _
            (UsedMoney + 1 < = Money) AND _
            Contents(XP,YP,ZP) = 0 AND _
            Contents(XP,YP,ZP+1) = 0 then
            Contents(XP,YP,ZP) = 22
            Contents(XP,YP,ZP+1) = 255
            /'
             ' This is a 1x1x2 block. It takes the space above it.
             '/
            Direction(XP,YP,ZP) = Rotation
            LevelTime += 150
            MinimumLevelTime += 75
            UsedMoney += 1
            Blocks += 1
            Putt = 2
        elseif (BlockType = 23) AND _
            (UsedMoney + 1 < = Money) AND _
            Contents(XP,YP,ZP) = 0 then
            Contents(XP,YP,ZP) = 23
            Direction(XP,YP,ZP) = Rotation
            LevelTime += 125
            MinimumLevelTime += 75
            UsedMoney += 1
            Blocks += 1
            Putt = 2

        elseif (BlockType = 24) AND _
            (UsedMoney + 1 < = Money) AND _
            Contents(XP,YP,ZP) = 0 then
            Contents(XP,YP,ZP) = 24
            Direction(XP,YP,ZP) = Rotation
            LevelTime += 225
            MinimumLevelTime += 150
            TargetCoins += 1
            Coins += 4
            UsedMoney += 1
            Blocks += 1
            Putt = 2
        elseif (BlockType = 25) AND _
            (UsedMoney + 2 < = Money) AND _
            Contents(XP,YP,ZP) = 0 AND _
            Contents(XP,YP,ZP+1) = 0 then
            Contents(XP,YP,ZP) = 25
            Contents(XP,YP,ZP+1) = 255
            Direction(XP,YP,ZP) = Rotation
            LevelTime += 350
            MinimumLevelTime += 200
            UsedMoney += 2
            Blocks += 2
            Putt = 2
        elseif (BlockType = 26) AND _
            (UsedMoney + 2 < = Money) AND _
            Contents(XP,YP,ZP) = 0 AND _
            Contents(XP,YP,ZP+1) = 0 then
            Contents(XP,YP,ZP) = 26
            Contents(XP,YP,ZP+1) = 255
            Direction(XP,YP,ZP) = Rotation
            LevelTime += 500
            MinimumLevelTime += 300
            TargetCoins += 2
            Coins += 8
            UsedMoney += 2
            Blocks += 2
            Putt = 2

        elseif (BlockType = 27) AND _
            (UsedMoney + 2 < = Money) AND _
            Contents(XP,YP,ZP) = 0 AND _
            Contents(XP,YP,ZP+1) = 0 then
            Contents(XP,YP,ZP) = 27
            Contents(XP,YP,ZP+1) = 255
            Direction(XP,YP,ZP) = Rotation
            LevelTime += 100
            MinimumLevelTime += 50
            UsedMoney += 2
            Blocks += 2
            Putt = 2
        elseif (BlockType = 28) AND _
            (UsedMoney + 2 < = Money) AND _
            Contents(XP,YP,ZP) = 0 AND _
            Contents(XP,YP,ZP+1) = 0 then
            Contents(XP,YP,ZP) = 28
            Contents(XP,YP,ZP+1) = 255
            Direction(XP,YP,ZP) = Rotation
            LevelTime += 200
            MinimumLevelTime += 100
            TargetCoins += 2
            Coins += 8
            UsedMoney += 2
            Blocks += 2
            Putt = 2

        elseif (BlockType = 41) AND _
            (UsedMoney + 2 < = Money) AND _
            Contents(XP,YP,ZP) = 0 then
            Contents(XP,YP,ZP) = 41
            Direction(XP,YP,ZP) = Rotation
            LevelTime += 350
            MinimumLevelTime += 200
            UsedMoney += 2
            Blocks += 1
            Putt = 2
        elseif (BlockType = 42) AND _
            (UsedMoney + 3 < = Money) AND _
            Contents(XP,YP,ZP) = 0 then
            Contents(XP,YP,ZP) = 42
            Direction(XP,YP,ZP) = Rotation
            LevelTime += 325
            MinimumLevelTime += 150
            UsedMoney += 3
            Blocks += 2
            Putt = 2
        elseif (BlockType = 43) AND _
            (UsedMoney + 3 < = Money) AND _
            Contents(XP,YP,ZP) = 0 then
            Contents(XP,YP,ZP) = 43
            Direction(XP,YP,ZP) = Rotation
            LevelTime += 100
            MinimumLevelTime += 75
            UsedMoney += 3
            Blocks += 3
            Putt = 2

        elseif (BlockType = 44) AND _
            (UsedMoney + 4 < = Money) AND _
            Contents(XP,YP,ZP) = 0 AND _
            Contents(XP+1,YP,ZP) = 0 AND _
            Contents(XP,YP-1,ZP) = 0 AND _
            Contents(XP+1,YP-1,ZP) = 0 then
            /'
             ' This is a 2x2x1 block. It takes two spaces horizontal and two
             ' spaces vertical.
             '/
            Contents(XP,YP,ZP) = 44
            Contents(XP+1,YP,ZP) = 255
            Contents(XP,YP-1,ZP) = 255
            Contents(XP+1,YP-1,ZP) = 255
            Direction(XP,YP,ZP) = Rotation
            LevelTime += 150
            MinimumLevelTime += 100
            UsedMoney += 4
            Blocks += 1
            Putt = 2
        elseif (BlockType = 45) AND _
            (UsedMoney + 4 < = Money) AND _
            Contents(XP,YP,ZP) = 0 AND _
            Contents(XP+1,YP,ZP) = 0 AND _
            Contents(XP,YP-1,ZP) = 0 AND _
            Contents(XP+1,YP-1,ZP) = 0 then
            /'
             ' This is a 2x2x1 block. It takes two spaces horizontal and two
             ' spaces vertical.
             '/
            Contents(XP,YP,ZP) = 45
            Contents(XP+1,YP,ZP) = 255
            Contents(XP,YP-1,ZP) = 255
            Contents(XP+1,YP-1,ZP) = 255
            Direction(XP,YP,ZP) = Rotation
            LevelTime += 150
            MinimumLevelTime += 100
            UsedMoney += 4
            Blocks += 1
            Putt = 2

        elseif (BlockType = 46) AND _
            (UsedMoney + 9 < = Money) AND _
            Contents(XP,YP,ZP) = 0 AND _
            Contents(XP+1,YP,ZP) = 0 AND _
            Contents(XP+2,YP,ZP) = 0 AND _
            Contents(XP,YP-1,ZP) = 0 AND _
            Contents(XP+1,YP-1,ZP) = 0 AND _
            Contents(XP+2,YP-1,ZP) = 0 AND _
            Contents(XP,YP-2,ZP) = 0 AND _
            Contents(XP+1,YP-2,ZP) = 0 AND _
            Contents(XP+2,YP-2,ZP) = 0 then
            /'
             ' This is a 3x3x1 block. It takes three spaces horizontal and
             ' three spaces vertical.
             '/
            Contents(XP,YP,ZP) = 46
            Contents(XP+1,YP,ZP) = 255
            Contents(XP+2,YP,ZP) = 255
            Contents(XP,YP-1,ZP) = 255
            Contents(XP+1,YP-1,ZP) = 255
            Contents(XP+2,YP-1,ZP) = 255
            Contents(XP,YP-2,ZP) = 255
            Contents(XP+1,YP-2,ZP) = 255
            Contents(XP+2,YP-2,ZP) = 255
            Direction(XP,YP,ZP) = Rotation
            LevelTime += 200
            MinimumLevelTime += 150
            UsedMoney += 9
            Blocks += 1
            Putt = 2

        elseif (BlockType = 47) AND _
            (UsedMoney + 9 < = Money) AND _
            Contents(XP,YP,ZP) = 0 AND _
            Contents(XP+1,YP,ZP) = 0 AND _
            Contents(XP+2,YP,ZP) = 0 AND _
            Contents(XP,YP-1,ZP) = 0 AND _
            Contents(XP+1,YP-1,ZP) = 0 AND _
            Contents(XP+2,YP-1,ZP) = 0 AND _
            Contents(XP,YP-2,ZP) = 0 AND _
            Contents(XP+1,YP-2,ZP) = 0 AND _
            Contents(XP+2,YP-2,ZP) = 0 then
            /'
             ' This is a 3x3x1 block. It takes three spaces horizontal and
             ' three spaces vertical.
             '/
            Contents(XP,YP,ZP) = 47
            Contents(XP+1,YP,ZP) = 255
            Contents(XP+2,YP,ZP) = 255
            Contents(XP,YP-1,ZP) = 255
            Contents(XP+1,YP-1,ZP) = 255
            Contents(XP+2,YP-1,ZP) = 255
            Contents(XP,YP-2,ZP) = 255
            Contents(XP+1,YP-2,ZP) = 255
            Contents(XP+2,YP-2,ZP) = 255
            Direction(XP,YP,ZP) = Rotation
            LevelTime += 200
            MinimumLevelTime += 150
            UsedMoney += 9
            Blocks += 1
            Putt = 2
        end if
    end sub
#ENDIF