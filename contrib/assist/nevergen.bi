/'
 ' Generator resources
 '/
const North = 1
const East = 2
const South = 3
const West = 4
const NS = North OR South
const EW = East OR West
sub base_gfx
    for XM = 0 to 21
        line(XM*20,0)-(XM*20,420),rgb(255,255,255)
    next
    for YM = 0 to 21
        line(0,YM*20)-(420,YM*20),rgb(255,255,255)
    next
    for XG = -10 to 10
        for YG = -10 to 10
            if Contents(XG,YG,ZP) < > 0 then
                line((XG+10)*20+1,(-YG+10)*20+1) - _
                    ((XG+11)*20-1,(-YG+11)*20-1),rgb(255,0,255),bf
            end if
        next
    next
end sub
sub cursor
    if Contents(XP,YP,ZP) = 0 then
        line((XP+10)*20,(-YP+10)*20)-((XP+10+XR)*20,(-YP+10+YR)*20), _
            rgb(0,255,0),b
    else
        line((XP+10)*20,(-YP+10)*20)-((XP+10+XR)*20,(-YP+10+YR)*20), _
            rgb(255,0,0),b
    end if
end sub
sub direction_gfx
    if Rotation = 1 then
        line ((XP+10)*20+1,(-YP+10)*20+1)-((XP+11)*20-1,(-YP+10)*20+6), _
            rgb(0,0,255),bf
    elseif Rotation = 2 then
        line ((XP+11)*20-1,(-YP+10)*20+1)-((XP+11)*20-6,(-YP+11)*20-1), _
            rgb(0,0,255),bf
    elseif Rotation = 3 then
        line ((XP+10)*20+1,(-YP+11)*20-1)-((XP+11)*20-1,(-YP+11)*20-6), _
            rgb(0,0,255),bf
    elseif Rotation = 4 then
        line ((XP+10)*20+1,(-YP+10)*20+1)-((XP+10)*20+6,(-YP+11)*20-1), _
            rgb(0,0,255),bf
    end if
end sub
sub clear_preview
    line BlockDisplay,(0,0)-(127,127),rgb(0,0,0),bf
end sub