randomize timer
dim shared as ubyte XM, YM, XG, YG, BlockType, BlockSet, Rotation, XR, YR, ZR, Start, Finish, MusicSwitch, XSwitch, YSwitch, ZSwitch, Warning, Hold, Openings, Putt, MediumClear, MusicID, Perfect
dim shared as byte XP, YP, ZP
dim shared as uinteger Entity, LevelTime, Blocks, TargetCoins, Coins, MinimumLevelTime, Money
dim shared as longint PlacementFormula, GraphicFormula, GraphicTest(200000), PlacementTest(200000)
dim shared as string MapFile, Title, Song, Back, Grad, Shot, MusicFile, LevelMessage, WindowTitleM, Compile, Replay, LevelName, MusicPlay
dim shared as byte Check
const AssistCfg = "assist.ini"
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
 'These are related to your money.
 '/
dim shared as ushort MaxMoney, NBDataPath, TimeScore, CoinSore
dim shared as string Scorer

/'
 ' Nearly all subroutines are declared here.
 '/
declare sub base_gfx
declare sub cursor
declare sub direction
declare sub menu
declare sub map_generate_initial
declare sub map_generate
declare sub place_gfx(Added as integer)

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

/'
 ' This includes all the construction blocks I've implemented so far.
 '/
#include "neverblocks.bas"

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
		print #1, "( "& XP*128+XOff1;" "& YP*128+YOff1;" "& ZP*64+ZOff1;" ) ( "& XP*128+XOff2;" "& YP*128+YOff2;" "& ZP*64+ZOff2;" ) ( "& XP*128+XOff3;" "& YP*128+YOff3;" "& ZP*64+ZOff3;" ) mtrl/";Texture;" 0 0 0 0.5 0.5 ";134217728*Detail;" 0 0"
	else
		print #1, "( "& XP*128+XOff1;" "& YP*128+YOff1;" "& ZP*64+ZOff1;" ) ( "& XP*128+XOff2;" "& YP*128+YOff2;" "& ZP*64+ZOff2;" ) ( "& XP*128+XOff3;" "& YP*128+YOff3;" "& ZP*64+ZOff3;" ) mtrl/";Texture;" 0 0 0 0.5 0.5 0 0 0"
	end if
end sub
/'
 ' Don't even bother uncommenting it. This makes it easier for me to make
 ' future construction blocks.
 '/
/'
solid_face(0,0,0,0,0,0,0,0,0,"")
'/

declare sub clkey
sub clkey
	while (inkey < > ""):wend
end sub