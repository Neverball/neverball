/'
 ' This includes language support.
 '/
#include "lang.bi"
randomize timer
dim shared as ubyte XM, YM, XG, YG, BlockType, BlockSet, Rotation, XR, YR, _
    ZR, Start, Finish, MusicSwitch, XSwitch, YSwitch, ZSwitch, Warning, Hold, _
    Openings, Putt, MusicID, Official, FullBonus(25)
dim shared as byte XP, YP, ZP, DiffPar
dim shared as uinteger Entity, LevelTime, Blocks, TargetCoins, Coins, _
    MinimumLevelTime, UsedMoney, SingleLevelID
dim shared as integer Money
dim shared as longint PlacementFormula, PlacementTest(-11 to 11,_
    -11 to 11,-11 to 21)
dim shared as string MapFile, Title, Song, Back, Grad, Shot, MusicFile, _
    LevelMessage, WindowTitleM, Compile, Replay, LevelName, MusicPlay, _
    Neverpath, InType, AssistDir, ShotFile, Username, UserData, DataScan
dim shared as byte Check
dim shared as any ptr BlockDisplay
#IFDEF __FB_WIN32__
    const AssistCfg = "assist.ini"
    const UserCfg = "users\"
#ELSE
    const AssistCfg = "assistrc"
    const UserCfg = ".users/"
#ENDIF
const m = 1
#IFDEF __FB_WIN32__
    /'
    ' 7-Zip-dependent resources
    ' Z7Path and Z7Exe could have been 7ZPath and 7ZExe, but variables can NOT
    ' start with a number.
    '/
    dim shared as string Z7Path, Z7Exe, Unpack
#ENDIF
/'
 'These are related to Official Mode.
 '/
dim shared as ushort ChallengeCoins
dim shared as double ChallengeTime

/'
 ' Nearly all subroutines are declared here.
 '/
declare sub base_gfx
declare sub cursor
declare sub direction
declare sub menu
declare sub map_generate
declare sub config(Switch as ubyte = 0)

/'
 ' As you can see, this subroutine has 11 arguments. I was able to segment
 ' this into multiple lines, but each end of line had to have an underscore
 ' for it to be handled correctly. The only exception towards this rule is
 ' the very last line.
 '
 ' The first argument determines whether it should be detail or not. 1 makes
 ' it detail. Any other value makes it structural.
 '
 ' The next nine arguments are the offsets. These offsets are what makes up
 ' these faces, which in turn makes the lumps.
 '
 ' The last argument applies a texture to it. This argument is optional, and
 ' it defaults to invisible.
 '/
declare sub plot_face(Detail as ubyte, XOff1 as short, YOff1 as short, _
    ZOff1 as short, XOff2 as short, YOff2 as short, ZOff2 as short, _
    XOff3 as short, YOff3 as short, ZOff3 as short, _
    Texture as string = "invisible")

sub edge(EdgeDirection as ubyte)
/'
 ' WIP: This allows the trimming you see in regular levels to be handled
 ' automatically with this program.
 '/
end sub
sub plot_face(Detail as ubyte, XOff1 as short, YOff1 as short, _
    ZOff1 as short, XOff2 as short, YOff2 as short, ZOff2 as short, _
    XOff3 as short, YOff3 as short, ZOff3 as short, _
    Texture as string = "invisible")
    'Plots a face.
    if Detail < 2 then
        print #m, "( "& XP*128+XOff1;" "& YP*128+YOff1;" "; _
        ""& ZP*64+ZOff1;" ) ( "& XP*128+XOff2;" "& YP*128+YOff2;" "; _
        ""& ZP*64+ZOff2;" ) ( "& XP*128+XOff3;" "& YP*128+YOff3;" "; _
        ""& ZP*64+ZOff3;" ) mtrl/";Texture;" 0 0 0 0.5 0.5 "; _
        ""& 134217728*Detail;" 0 0"
    else
        print #m, "( "& XP*128+XOff1;" "& YP*128+YOff1;" "; _
        ""& ZP*64+ZOff1;" ) ( "& XP*128+XOff2;" "& YP*128+YOff2;" "; _
        ""& ZP*64+ZOff2;" ) ( "& XP*128+XOff3;" "& YP*128+YOff3;" "; _
        ""& ZP*64+ZOff3;" ) mtrl/";Texture;" 0 0 0 0.5 0.5 0 0 0"
    end if
end sub
/'
 ' Don't even bother uncommenting it. This makes it easier for me to make
 ' future construction blocks.
 '/
/'
plot_face(0,0,0,0,0,0,0,0,0,0,"")
'/

declare sub clkey
sub clkey
    while (inkey < > ""):wend
end sub

/'
 ' This allows configuration management. 0 saves the data (default) and any
 ' other value imports the data.
 '/
sub user_data(Switch as ubyte = 0)
    if Switch = 0 then
        #IFDEF __FB_WIN32__
            open AssistDir + "\" + UserCfg + Username + "\" + "user.ini" for output as #3
        #ELSE
            open AssistDir + "/" + UserCfg + Username + "/" + "userrc" for output as #3
        #ENDIF
        print #3, ""& Money
        print #3, UserData
        for ID as ubyte = 1 to 25
            print #3, FullBonus(ID)
        next ID
        close #3
    else
        #IFDEF __FB_WIN32__
            open AssistDir + "\" + UserCfg + Username + "\" + "user.ini" for input as #3
        #ELSE
            open AssistDir + "/" + UserCfg + Username + "/" + "userrc" for input as #3
        #ENDIF
        input #3, Money
        input #3, UserData
        for ID as ubyte = 1 to 25
            input #3, FullBonus(ID)
        next ID
        close #3
    end if
end sub
sub config(Switch as ubyte = 0)
    dim as ubyte ConvertID
    if Switch = 0 then
        #IFDEF __FB_WIN32__
            open AssistDir + "\" + AssistCfg for output as #2
            print #2, Username
            print #2, NeverPath
            print #2, LangFile
            print #2, Z7Path
            print #2, Z7Exe
            close #2
        #ELSE
            open AssistDir + "/" + AssistCfg for output as #2
            print #2, Username
            print #2, NeverPath
            print #2, LangFile
            close #2
        #ENDIF
    else
        #IFDEF __FB_WIN32__
            open AssistDir + "\" + AssistCfg for input as #2
            input #2, Username
            line input #2, NeverPath
            input #2, LangFile
            input #2, Z7Path
            input #2, Z7Exe
            close #2
        #ELSE
            open AssistDir + "/" + AssistCfg for input as #2
            input #2, Username
            line input #2, NeverPath
            input #2, LangFile
            close #2
        #ENDIF
    end if
end sub