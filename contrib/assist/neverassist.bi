/'
 ' This includes language support.
 '/
#include "lang.bi"
randomize timer
dim shared as ubyte XM, YM, BlockType, BlockSet, Rotation, XR, YR, _
    ZR, Start, Finish(2), MusicSwitch, XSwitch, YSwitch, ZSwitch, Warning, _
    Hold, Openings, Putt, MusicID
dim shared as byte XP, YP, ZP, XG, YG
dim shared as uinteger Entity, LevelTime, Blocks, TargetCoins, Coins, _
    MinimumLevelTime, UsedMoney, SingleLevelID
dim shared as longint PlacementTest(-11 to 11,-11 to 11,-11 to 21), _
    Contents(-11 to 11,-11 to 11,-11 to 21), _
    Direction(-11 to 11,-11 to 11,-11 to 21)
dim shared as string MapFile, Title, Song, Back, Grad, Shot, MusicFile, _
    LevelMessage, WindowTitleM, Compile, Replay, LevelName, MusicPlay, _
    Neverpath, InType, AssistDir, ShotFile
dim shared as integer Check
dim shared as any ptr BlockDisplay
#IFDEF __FB_WIN32__
    const AssistCfg = "assist.ini"
#ELSE
    const AssistCfg = "assistrc"
#ENDIF
const m = 1
const c = 2
const s = 3

/'
 ' Nearly all subroutines are declared here.
 '/
declare sub base_gfx
declare sub cursor
declare sub direction_gfx
declare sub main_menu
declare sub clear_preview
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
 ' it defaults to 0 when omitted.
 '/
declare sub ptf(Detail as ubyte, XOff1 as short, YOff1 as short, _
    ZOff1 as short, XOff2 as short, YOff2 as short, ZOff2 as short, _
    XOff3 as short, YOff3 as short, ZOff3 as short, _
    TexID as ubyte = 0)

sub ptf(Detail as ubyte, XOff1 as short, YOff1 as short, _
    ZOff1 as short, XOff2 as short, YOff2 as short, ZOff2 as short, _
    XOff3 as short, YOff3 as short, ZOff3 as short, _
    TexID as ubyte = 0)
    dim as string Texture(0 to 7) => {"invisible", "turf-grey", _
        "turf-green", "turf-green-dark", "coin-green-small", "yellow", _
        "arrow-green-light","goal"}
    'Plots a face.
    if Detail < 2 then
        print #m, "( "& XP*128+XOff1;" "& YP*128+YOff1;" "; _
        ""& ZP*64+ZOff1;" ) ( "& XP*128+XOff2;" "& YP*128+YOff2;" "; _
        ""& ZP*64+ZOff2;" ) ( "& XP*128+XOff3;" "& YP*128+YOff3;" "; _
        ""& ZP*64+ZOff3;" ) mtrl/";Texture(TexID);" 0 0 0 0.5 0.5 "; _
        ""& 134217728*Detail;" 0 0"
    else
        print #m, "( "& XP*128+XOff1;" "& YP*128+YOff1;" "; _
        ""& ZP*64+ZOff1;" ) ( "& XP*128+XOff2;" "& YP*128+YOff2;" "; _
        ""& ZP*64+ZOff2;" ) ( "& XP*128+XOff3;" "& YP*128+YOff3;" "; _
        ""& ZP*64+ZOff3;" ) mtrl/";Texture(TexID);" 0 0 0 0.5 0.5 0 0 0"
    end if
end sub

declare sub plot_coins(ZOff as short = 0, Value as ubyte = 1)
sub plot_coins(ZOff as short = 0, Value as ubyte = 1)
    print #m, "// entity ";Entity
    Entity += 1
    print #m, "{"
    print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
    print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-96; _
        " "& YP*128-96;" "& ZP*64+24+ZOff;chr(34)
    print #m, chr(34)+"light"+chr(34)+" "+chr(34)+str(Value)+chr(34)
    print #m, "}"
    print #m, "// entity ";Entity
    Entity += 1
    print #m, "{"
    print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
    print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-32; _
        " "& YP*128-96;" "& ZP*64+24+ZOff;chr(34)
    print #m, chr(34)+"light"+chr(34)+" "+chr(34)+str(Value)+chr(34)
    print #m, "}"
    print #m, "// entity ";Entity
    Entity += 1
    print #m, "{"
    print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
    print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-96; _
        " "& YP*128-32;" "& ZP*64+24+ZOff;chr(34)
    print #m, chr(34)+"light"+chr(34)+" "+chr(34)+str(Value)+chr(34)
    print #m, "}"
    print #m, "// entity ";Entity
    Entity += 1
    print #m, "{"
    print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
    print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-32; _
        " "& YP*128-32;" "& ZP*64+24+ZOff;chr(34)
    print #m, chr(34)+"light"+chr(34)+" "+chr(34)+str(Value)+chr(34)
    print #m, "}"
end sub
/'
 ' Don't even bother uncommenting it. This makes it easier for me to make
 ' future construction blocks.
 '/
/'
ptf(0,0,0,0,0,0,0,0,0,0,0)
'/

declare sub clkey
sub clkey
    while (inkey < > ""):wend
end sub

declare function quote(Argument as string) as string
function quote(Argument as string) as string
    return chr(34)+Argument+chr(34)
end function

/'
 ' This allows configuration management. 0 saves the data (default) and any
 ' other value imports the data.
 '/
sub config(Switch as ubyte = 0)
    dim as ubyte ConvertID
    if Switch = 0 then
        #IFDEF __FB_WIN32__
            open AssistDir + "\" + AssistCfg for output as #c
            print #c, NeverPath
            print #c, LangFile
            print #c, Z7Path
            print #c, Z7Exe
            close #c
        #ELSE
            open AssistDir + "/" + AssistCfg for output as #c
            print #c, NeverPath
            print #c, LangFile
            close #c
        #ENDIF
    else
        #IFDEF __FB_WIN32__
            open AssistDir + "\" + AssistCfg for input as #c
            line input #c, NeverPath
            input #c, LangFile
            input #c, Z7Path
            input #c, Z7Exe
            close #c
        #ELSE
            open AssistDir + "/" + AssistCfg for input as #c
            line input #c, NeverPath
            input #c, LangFile
            close #c
        #ENDIF
    end if
end sub

/'
 ' These lines includes SDL and its child library SDL_mixer. The headers are
 ' pre-packaged with the compiler, so it isn't needed in this package. You may
 ' have to use dynamic linking.
 '/ 
#include "SDL\SDL.bi"
#include "SDL\SDL_mixer.bi"
dim shared music as Mix_Music ptr
music = NULL
dim audio_rate as integer
dim audio_format as Uint16
dim audio_channels as integer
dim audio_buffers as integer
audio_rate = 44100
audio_format = AUDIO_S16
audio_channels = 2
audio_buffers = 4096
/'
 ' Only audio needs to be initialized, since a seperate graphics window will
 ' be used.
 '/
SDL_Init(SDL_INIT_AUDIO)
if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) then
    open err as #1
    print #1, "Unable to open audio!"
    close #1
    end 1
end if
