declare sub compile_blocks
sub compile_blocks
    for ZP as byte = -10 to 20
        for YP as byte = -10 to 10
            for XP as byte = -10 to 10
                select case Contents(XP,YP,ZP)
                    case 1
                        print #m, "// entity ";Entity
                        /'
                         ' Increments entity. This allows entites to be
                         ' numbered correctly.
                         '/
                        Entity += 1
                        /'
                         ' Plots the brush(es) into a seperate worldspawn
                         ' entity.
                         '
                         ' The ptf subroutine has 11 arguments.
                         ' They're listed in the neverassist.bi header
                         ' file.
                         '/
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+_
                            " "+chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"
                        ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                        ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                        ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                        ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                        ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,1)
                        ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                        print #m, "}"

                        print #m, "// brush 1"
                        print #m, "{"
                        ptf(1,-16,0,1,-16,-128,1,-144,0,1,"arrow-green-light")
                        ptf(1,0,0,80,-128,0,80,0,0,-48,0)
                        ptf(1,0,0,80,0,0,-48,0,-128,80,0)
                        ptf(1,-128,-128,0,0,-128,0,-128,0,0,0)
                        ptf(1,-128,-128,-48,0,-128,79,0,-128,-48,0)
                        ptf(1,-128,-128,-48,-128,0,-49,-128,-128,80,0)
                        print #m, "}"
                        print #m, "}"

                        /'
                         ' This entity serves a different purpose. In this
                         ' case, its the starting point.
                         '/
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" "+chr(34) + _
                               "info_player_start"+chr(34)
                        print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-64; _
                               " "& YP*128-64;" "& ZP*64+24;chr(34)
                        print #m, "}"
                        /'
                         ' This provides the first part of the initial camera.
                         '/
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" "+chr(34) + _
                               "info_player_intermission"+chr(34)
                        print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-64; _
                               " "& YP*128-64;" "& ZP*64+224;chr(34)
                        print #m, chr(34)+"target"+chr(34)+" "+chr(34)+"goal"+chr(34)
                        print #m, "}"

                    case 11
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                            chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"
                        /'
                         ' Direction(XP,YP,ZP) is how the appearance of the
                         ' blocks can differ.
                         '
                         ' However, the block above (start block) is the
                         ' only block that can't be rotated. You always
                         ' start facing north, and the start block always
                         ' faces north.
                         '/
                        if Direction(XP,YP,ZP) = NS then
                            ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                            ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                            ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                            ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                            ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                            ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                        elseif Direction(XP,YP,ZP) = EW then
                            ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                            ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                            ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                            ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                            ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,1)
                            ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                        end if
                        print #m, "}"
                        print #m, "}"

                    case 12
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                            chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"

                        /'
                         ' This allows more flexibility in printing the blocks
                         ' with the appropriate textures. However, the block
                         ' above it would not be different for certain
                         ' directions, which is why its merged together.
                         '
                         ' This block does have it all different.
                         '/
                        select case Direction(XP,YP,ZP)
                            case 1
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,0)
                            case 2
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,1)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,0)
                            case 3
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,1)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                            case 4
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                        end select
                        print #m, "}"
                        print #m, "}"

                    case 13
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                            chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"
                        select case Direction(XP,YP,ZP)
                            case 1
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,0)
                            case 2
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,0)
                            case 3
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,1)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,0)
                            case 4
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                        end select
                        print #m, "}"
                        print #m, "}"

                    case 14
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                            chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"
                        /'
                         ' This is the cross block. It would be the same
                         ' no matter how you rotate it. As a result, there
                         ' are no internal statements keeping what's below
                         ' from printing to the file.
                         '/
                        ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                        ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                        ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                        ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                        ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                        ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,0)
                        print #m, "}"
                        print #m, "}"

                    case 15
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                            chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"
                        if Direction(XP,YP,ZP) = NS then
                            ptf(0,-16,0,0,-16,-128,0,-144,0,0,4)
                            ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                            ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                            ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                            ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                            ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                        elseif Direction(XP,YP,ZP) = EW then
                            ptf(0,-16,0,0,-16,-128,0,-144,0,0,4)
                            ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                            ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                            ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                            ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,1)
                            ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                        end if
                        print #m, "}"
                        print #m, "}"

                        /'
                         ' Four yellow coins.
                         '/
                        plot_coins

                    case 16
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                            chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"
                        select case Direction(XP,YP,ZP)
                            case 1
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,4)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,0)
                            case 2
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,4)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,1)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,0)
                            case 3
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,4)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,1)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                            case 4
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,4)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                        end select
                        print #m, "}"
                        print #m, "}"
                        plot_coins

                    case 17
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                            chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"

                        select case Direction(XP,YP,ZP)
                            case 1
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                            case 2
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,1)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,0)
                            case 3
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,1)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                            case 4
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,1)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                        end select
                        print #m, "}"
                        print #m, "}"

                    case 18
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                            chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"

                        select case Direction(XP,YP,ZP)
                            case 1
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,4)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                            case 2
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,4)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,1)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,0)
                            case 3
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,4)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,1)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                            case 4
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,4)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,1)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                        end select
                        print #m, "}"
                        print #m, "}"
                        plot_coins

                    case 41
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                            chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"
                        if Direction(XP,YP,ZP) = NS then
                            ptf(0,-128,0,0,0,0,0,0,-16,16,5)
                            ptf(0,0,0,0,-128,0,0,0,0,-16,0)
                            ptf(0,0,0,64,0,0,-64,0,-16,64,5)
                            ptf(0,-128,-16,-16,0,-16,-16,-128,0,-16,5)
                            ptf(0,-128,-16,-64,-128,-16,64,0,-16,64,5)
                            ptf(0,-128,-16,-64,-128,0,-64,-128,-16,64,5)
                        elseif Direction(XP,YP,ZP) = EW then
                            ptf(0,0,0,0,0,-128,0,-16,-128,16,5)
                            ptf(0,0,0,16,-16,0,16,0,0,-16,5)
                            ptf(0,0,-128,0,0,0,0,0,-128,-16,0)
                            ptf(0,-16,-128,-16,0,-128,-16,-16,-64,-16,5)
                            ptf(0,-16,-128,-16,-16,-128,16,0,-128,-16,5)
                            ptf(0,-16,-128,-16,-16,-64,-16,-16,-128,16,5)
                        end if
                        print #m, "}"

                        /'
                         ' Notice how this "lump" is present as well. This
                         ' block is two lumps. More lumps means a greater
                         ' complexity of the "block".
                         '/
                        print #m, "// brush 1"
                        print #m, "{"
                        if Direction(XP,YP,ZP) = NS then
                            ptf(0,0,-128,0,-128,-128,0,-128,-112,16,5)
                            ptf(0,0,-112,64,-128,-112,64,0,-112,-64,5)
                            ptf(0,0,-112,64,0,-112,-64,0,-128,64,5)
                            ptf(0,-128,-128,-16,0,-128,-16,-128,-112,-16,5)
                            ptf(0,-128,-128,0,0,-128,0,0,-128,-16,0)
                            ptf(0,-128,-128,-64,-128,-112,-64,-128,-128,64,5)
                        elseif Direction(XP,YP,ZP) = EW then
                            ptf(0,-128,-128,0,-128,0,0,-112,-128,16,5)
                            ptf(0,-112,0,16,-128,0,16,-112,0,-16,5)
                            ptf(0,-112,-64,16,-112,-64,-16,-112,-128,16,5)
                            ptf(0,-128,-128,-16,-112,-128,-16,-128,-64,-16,5)
                            ptf(0,-128,-128,-16,-128,-128,16,-112,-128,-16,5)
                            ptf(0,-128,0,0,-128,-128,0,-128,0,-16,0)
                        end if
                        print #m, "}"
                        print #m, "}"

                    case 19
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                            chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"
                        select case Direction(XP,YP,ZP)
                            case 1
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,4)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,0)
                            case 2
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,4)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,0)
                            case 3
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,4)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,1)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,0)
                            case 4
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,4)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                        end select
                        print #m, "}"
                        print #m, "}"
                        plot_coins

                    case 21
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                            chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"
                        ptf(0,-16,0,0,-16,-128,0,-144,0,0,4)
                        ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                        ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                        ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                        ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                        ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,0)
                        print #m, "}"
                        print #m, "}"
                        plot_coins

                    case 22
                        /'
                         ' This is the first block to take more than one space.
                         '
                         ' It also takes the space right above it. This is the
                         ' only implemented block (so far) with the ability to
                         ' change elevations.
                         '/

                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                            chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"

                        select case Direction(XP,YP,ZP)
                            case 1
                                ptf(0,-128,0,64,0,0,64,0,-128,0,2)
                                ptf(0,-128,0,48,0,0,48,0,0,64,0)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                                ptf(0,0,0,48,-128,0,48,0,-128,-16,1)
                                ptf(0,-128,-128,-64,-128,-128,64,0,-128,-64,0)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                            case 2
                                ptf(0,0,0,64,0,-128,64,-128,-128,0,2)
                                ptf(0,0,0,0,-128,0,0,0,0,-16,1)
                                ptf(0,0,0,48,0,-128,48,0,-128,64,0)
                                ptf(0,0,-128,48,0,0,48,-128,-128,-16,1)
                                ptf(0,-128,-128,-64,-128,-128,64,0,-128,-64,1)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,0)
                            case 3
                                ptf(0,-128,0,0,0,0,0,0,-128,64,2)
                                ptf(0,0,0,0,-128,0,0,0,0,-16,0)
                                ptf(0,0,-128,0,0,0,0,0,-128,-16,1)
                                ptf(0,0,0,-16,-128,0,-16,-128,-128,48,1)
                                ptf(0,0,-128,48,-128,-128,48,-128,-128,64,0)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                            case 4
                                ptf(0,0,0,0,0,-128,0,-128,-128,64,2)
                                ptf(0,0,0,0,-128,0,0,0,0,-16,1)
                                ptf(0,0,-128,0,0,0,0,0,-128,-16,0)
                                ptf(0,-128,0,48,-128,-128,48,0,-128,-16,1)
                                ptf(0,-128,-128,-64,-128,-128,64,0,-128,-64,1)
                                ptf(0,-128,-128,48,-128,0,48,-128,0,64,0)
                        end select
                        print #m, "}"
                        print #m, "}"

                    case 23
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                            chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"
                        if Direction(XP,YP,ZP) = NS then
                            ptf(0,-128,-64,16,0,-64,16,0,-128,0,2)
                            ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                            ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                            ptf(0,-128,-128,-64,-128,-128,64,0,-128,-64,0)
                            ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                            ptf(0,0,-64,16,-128,-64,16,0,0,0,2)
                            ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                        elseif Direction(XP,YP,ZP) = EW then
                            ptf(0,-64,-128,16,-64,0,16,0,0,0,2)
                            ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                            ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                            ptf(0,-128,-128,-64,-128,-128,64,0,-128,-64,1)
                            ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                            ptf(0,-64,0,16,-64,-128,16,-128,0,0,2)
                            ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,0)
                        end if
                        print #m, "}"
                        print #m, "}"

                    case 24
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                            chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"
                        if Direction(XP,YP,ZP) = NS then
                            ptf(0,-128,-64,16,0,-64,16,0,-128,0,4)
                            ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                            ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                            ptf(0,-128,-128,-64,-128,-128,64,0,-128,-64,0)
                            ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                            ptf(0,0,-64,16,-128,-64,16,0,0,0,4)
                            ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                        elseif Direction(XP,YP,ZP) = EW then
                            ptf(0,-64,-128,16,-64,0,16,0,0,0,4)
                            ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                            ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                            ptf(0,-128,-128,-64,-128,-128,64,0,-128,-64,1)
                            ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                            ptf(0,-64,0,16,-64,-128,16,-128,0,0,4)
                            ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,0)
                        end if
                        print #m, "}"
                        print #m, "}"
                        plot_coins(8)

                    case 42
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                            chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"
                        if Direction(XP,YP,ZP) = NS then
                            ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                            ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                            ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                            ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                            ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                            ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                        elseif Direction(XP,YP,ZP) = EW then
                            ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                            ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                            ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                            ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                            ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,1)
                            ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                        end if
                        print #m, "}"
                        print #m, "}"

                        /'
                         ' Even trains can be utilized in this assistant. I
                         ' should expand this, however, to allow all
                         ' directions. It doesn't do this yet.
                         '/
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"func_train"+chr(34)
                        print #m, chr(34)+"target"+chr(34)+" "+chr(34)+"path_corner"; _
                               ""& Entity+1;chr(34)
                        print #m, "// brush 0"
                        print #m, "{"
                        if Direction(XP,YP,ZP) = NS then
                            ptf(0,-64,-32,64,-64,-96,64,-128,-32,64,1)
                            ptf(0,-64,-32,16,-128,-32,16,-64,-32,0,1)
                            ptf(0,-64,-32,16,-64,-32,0,-64,-96,16,1)
                            ptf(0,-128,-96,0,-64,-96,0,-128,-32,0,1)
                            ptf(0,-128,-96,0,-128,-96,16,-64,-96,0,1)
                            ptf(0,-128,-96,0,-128,-32,0,-128,-96,16,1)
                        elseif Direction(XP,YP,ZP) = EW then
                            ptf(0,-32,0,64,-32,-64,64,-96,0,64,1)
                            ptf(0,-32,0,16,-96,0,16,-32,0,0,1)
                            ptf(0,-32,0,16,-32,0,0,-32,-94,16,1)
                            ptf(0,-96,-64,0,-32,-64,0,-96,0,0,1)
                            ptf(0,-96,-64,0,-96,-64,16,-32,-64,0,1)
                            ptf(0,-96,-64,0,-96,0,0,-96,-64,16,1)
                        end if
                        print #m, "}"
                        print #m, "}"

                        /'
                         ' Uses the entities needed for the mover.
                         '/
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"path_corner"+chr(34)
                        if Direction(XP,YP,ZP) = NS then
                            print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-32; _
                                      " "& YP*128-64;" "& ZP*64-16;chr(34)
                        elseif Direction(XP,YP,ZP) = EW then
                            print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-64; _
                                      " "& YP*128-96;" "& ZP*64-16;chr(34)
                        end if
                        print #m, chr(34)+"targetname"+chr(34)+" "+chr(34)+"path_corner"; _
                               ""& Entity-1;chr(34)
                        print #m, chr(34)+"target"+chr(34)+" "+chr(34)+"path_corner"; _
                               ""& Entity;chr(34)
                        print #m, chr(34)+"speed"+chr(34)+" "+chr(34)+"3"+chr(34)
                        print #m, "}"
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"path_corner"+chr(34)
                        if Direction(XP,YP,ZP) = NS then
                            print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-96; _
                                      " "& YP*128-64;" "& ZP*64-16;chr(34)
                        elseif Direction(XP,YP,ZP) = EW then
                            print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-64; _
                                      " "& YP*128-32;" "& ZP*64-16;chr(34)
                        end if
                        print #m, chr(34)+"targetname"+chr(34)+" "+chr(34)+"path_corner"; _
                               ""& Entity-1;chr(34)
                        print #m, chr(34)+"target"+chr(34)+" "+chr(34)+"path_corner"; _
                               ""& Entity-2;chr(34)
                        print #m, chr(34)+"speed"+chr(34)+" "+chr(34)+"3"+chr(34)
                        print #m, "}"

                    case 43
                        /'
                         ' This block has a narrow center.
                         '/
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                            chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"
                        if Direction(XP,YP,ZP) = NS then
                            ptf(0,0,16,0,0,-128,0,-128,16,0,3)
                            ptf(0,-128,-128,-16,0,-128,-16,-128,16,-16,1)
                            ptf(0,-48,-96,-16,0,-128,-16,-128,16,-16,1)
                            ptf(0,-80,-96,0,-80,-96,0,-80,-32,0,1)
                            ptf(0,-48,-32,-16,-48,-32,0,-80,-32,0,0)
                            ptf(0,-80,-96,-16,-80,-96,0,-48,-96,0,0)
                        elseif Direction(XP,YP,ZP) = EW then
                            ptf(0,0,0,0,0,-128,0,-128,0,0,3)
                            ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                            ptf(0,-80,-80,64,-48,-80,-64,-80,-80,-64,1)
                            ptf(0,-96,-64,64,-96,-80,-64,-96,-64,-64,0)
                            ptf(0,-32,-64,-64,-32,-80,-64,-32,-64,64,0)
                            ptf(0,-96,-48,-64,-64,-48,-64,-96,-48,64,1)
                        end if
                        print #m, "}"

                        print #m, "// brush 1"
                        print #m, "{"
                        if (Direction(XP,YP,ZP) = 1) OR _
                            (Direction(XP,YP,ZP) = 3) then
                            ptf(0,0,16,0,0,-128,0,-128,16,0,3)
                            ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                            ptf(0,-128,-128,-16,0,-128,-16,-128,16,-16,1)
                            ptf(0,-80,-32,0,-80,-32,-16,-128,0,-16,1)
                            ptf(0,-48,-32,-16,-48,-32,0,0,0,0,1)
                            ptf(0,-48,-32,0,-48,-32,-16,-80,-32,-16,0)
                        elseif (Direction(XP,YP,ZP) = 2) OR _
                            (Direction(XP,YP,ZP) = 4) then
                            ptf(0,0,0,0,0,-128,0,-128,0,0,3)
                            ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                            ptf(0,-128,-128,-16,-128,0,-16,-128,-128,0,0)
                            ptf(0,-96,-48,64,-128,0,-64,-96,-48,-64,1)
                            ptf(0,-96,-64,-64,-96,-80,-64,-96,-64,64,0)
                            ptf(0,-96,-80,-64,-128,-128,-64,-96,-80,64,1)
                        end if
                        print #m, "}"

                        /'
                         ' It takes three lumps to form a block that is
                         ' narrow in the middle.
                         '/
                        print #m, "// brush 2"
                        print #m, "{"
                        if Direction(XP,YP,ZP) = NS then
                            ptf(0,0,16,0,0,-128,0,-128,16,0,3)
                            ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                            ptf(0,-128,-128,-16,0,-128,-16,-128,16,-16,0)
                            ptf(0,-80,-96,-16,-80,-96,0,-128,-128,0,1)
                            ptf(0,-48,-96,0,-48,-96,-16,0,-128,-16,1)
                            ptf(0,-80,-96,0,-80,-96,-16,-48,-96,-16,0)
                        elseif Direction(XP,YP,ZP) = EW then
                            ptf(0,0,0,0,0,-128,0,-128,0,0,3)
                            ptf(0,0,0,0,0,0,-16,0,-128,0,0)
                            ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                            ptf(0,-32,-80,64,0,-128,-64,-32,-80,-64,1)
                            ptf(0,-32,-64,64,-32,-80,-64,-32,-64,-64,0)
                            ptf(0,0,0,64,-32,-48,-64,0,0,-64,1)
                        end if
                        print #m, "}"
                        print #m, "}"

                    case 44
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                            chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"
                        /'
                         ' This is a 2x2 block. It takes two spaces
                         ' horizontal and two spaces vertical.
                         '/
                        if Direction(XP,YP,ZP) = NS then
                            ptf(0,128,0,0,128,-128,0,-128,0,0,3)
                            ptf(0,0,0,0,0,0,-16,128,0,-16,0)
                            ptf(0,0,-256,-16,0,-256,0,128,0,0,1)
                            ptf(0,-128,-128,-16,128,-128,-16,-128,0,-16,1)
                            ptf(0,0,-256,0,0,-256,-16,-128,-256,0,0)
                            ptf(0,0,0,-16,0,0,0,-128,-256,0,1)
                        elseif Direction(XP,YP,ZP) = EW then
                            ptf(0,128,0,0,128,-128,0,-128,0,0,3)
                            ptf(0,128,-128,-16,128,-128,0,-128,0,0,1)
                            ptf(0,128,-128,0,128,-128,-16,128,-256,-16,0)
                            ptf(0,-128,-128,-16,128,-128,-16,-128,0,-16,1)
                            ptf(0,128,-256,0,128,-256,-16,-128,-128,0,1)
                            ptf(0,-128,-128,0,-128,-128,-16,-128,0,0,0)
                        end if
                        print #m, "}"
                        print #m, "}"

                    case 45
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                            chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"
                        /'
                         ' This is a 2x2 block. It takes two spaces
                         ' horizontal and two spaces vertical.
                         '/
                        if Direction(XP,YP,ZP) = NS then
                            ptf(0,128,0,0,128,-128,0,-128,0,0,3)
                            ptf(0,0,0,-16,0,0,0,-128,0,0,0)
                            ptf(0,0,0,0,0,0,-16,128,-256,-16,1)
                            ptf(0,-128,-128,-16,128,-128,-16,-128,0,-16,1)
                            ptf(0,0,-256,-16,0,-256,0,128,-256,0,0)
                            ptf(0,0,-256,0,0,-256,-16,-128,0,-16,1)
                        elseif Direction(XP,YP,ZP) = EW then
                            ptf(0,-128,-256,0,-128,-128,0,128,-128,0,3)
                            ptf(0,-128,-128,0,-128,-128,-16,128,0,-16,1)
                            ptf(0,128,-128,-16,128,-128,0,128,0,0,0)
                            ptf(0,-128,-128,-16,-128,-256,-16,128,-128,-16,1)
                            ptf(0,-128,-256,-16,-128,-256,0,128,-128,0,0)
                            ptf(0,-128,-128,0,-128,-128,-16,-128,-128,-16,0)
                        end if
                        print #m, "}"
                        print #m, "}"

                    case 46
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                            chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"
                        /'
                         ' This is a 3x3 block. It takes three spaces
                         ' horizontal and three spaces vertical.
                         '/
                        if Direction(XP,YP,ZP) = NS then
                            ptf(0,256,0,0,256,-128,0,0,0,0,3)
                            ptf(0,128,0,0,128,0,-16,256,0,-16,0)
                            ptf(0,0,-384,-16,0,-384,0,256,0,0,1)
                            ptf(0,0,-128,-16,256,-128,-16,0,0,-16,1)
                            ptf(0,0,-384,0,0,-384,-16,-128,-384,0,0)
                            ptf(0,-128,-384,0,-128,-384,-16,128,0,-16,1)
                        elseif Direction(XP,YP,ZP) = EW then
                            ptf(0,128,0,0,128,-128,0,-128,0,0,3)
                            ptf(0,256,-256,-16,256,-256,0,-128,0,0,1)
                            ptf(0,256,-256,0,256,-256,-16,256,-384,-16,0)
                            ptf(0,-128,-128,-16,128,-128,-16,-128,0,-16,1)
                            ptf(0,256,-384,0,256,-384,-16,-128,-128,0,1)
                            ptf(0,-128,-128,0,-128,-128,-16,-128,0,0,0)
                        end if
                        print #m, "}"
                        print #m, "}"

                    case 47
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                            chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"
                        /'
                         ' This is a 3x3 block. It takes three spaces
                         ' horizontal and three spaces vertical.
                         '/
                        if Direction(XP,YP,ZP) = NS then
                            ptf(0,128,0,0,128,-128,0,-128,0,0,3)
                            ptf(0,0,0,-16,0,0,0,-128,0,0,0)
                            ptf(0,256,-384,-16,256,-384,0,0,0,0,1)
                            ptf(0,-128,-128,-16,128,-128,-16,-128,0,-16,1)
                            ptf(0,128,-384,-16,128,-384,0,256,-384,0,0)
                            ptf(0,128,-384,0,128,-384,-16,-128,0,-16,1)
                        elseif Direction(XP,YP,ZP) = EW then
                            ptf(0,0,-256,0,0,-128,0,256,-128,0,3)
                            ptf(0,-128,-256,0,-128,-256,-16,256,0,-16,1)
                            ptf(0,256,-128,-16,256,-128,0,256,0,0,0)
                            ptf(0,0,-128,-16,0,-256,-16,256,-128,-16,1)
                            ptf(0,-128,-384,-16,-128,-384,0,256,-128,0,0)
                            ptf(0,-128,-256,-16,-128,-256,0,-128,-384,0,0)
                        end if
                        print #m, "}"
                        print #m, "}"

                    case 2
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                            chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"
                        if Direction(XP,YP,ZP) = NS then
                            ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                            ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                            ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                            ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                            ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                            ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                        elseif Direction(XP,YP,ZP) = EW then
                            ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                            ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                            ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                            ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                            ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,1)
                            ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                        end if

                        /'
                         ' Notice that the brush below is completely
                         ' detail. What makes it so is the first argument
                         ' of argument of ptf. See neverassist.bi for more
                         ' information.
                         '/
                        print #m, "}"
                        print #m, "// brush 1"
                        print #m, "{"
                        ptf(1,-16,0,1,-16,-128,1,-144,0,1,"goal")
                        ptf(1,0,0,80,-128,0,80,0,0,-48,0)
                        ptf(1,0,0,80,0,0,-48,0,-128,80,0)
                        ptf(1,-128,-128,0,0,-128,0,-128,0,0,0)
                        ptf(1,-128,-128,-48,0,-128,79,0,-128,-48,0)
                        ptf(1,-128,-128,-48,-128,0,-49,-128,-128,80,0)
                        print #m, "}"
                        print #m, "}"
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                               chr(34)+"info_player_deathmatch"+chr(34)
                        print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-64; _
                               " "&YP*128-64;" "& ZP*64+24;chr(34)
                        print #m, "}"

                        /'
                         ' This is the finishing touch of our initial
                         ' camera. This is only placed the first time the
                         ' assistant detects a finish block. There is no
                         ' limit on the number of finish blocks.
                         '/
                        if Finish(2) = 0 then
                            print #m, "// entity ";Entity
                            Entity += 1
                            print #m, "{"
                            print #m, chr(34)+"classname"+chr(34) + _
                                " "+chr(34)+"target_position"+chr(34)
                            print #m, chr(34)+"origin"+chr(34)+" " + _
                                chr(34)& XP*128-64;" "& YP*128-64;" "& _
                                ZP*64+128;chr(34)
                            print #m, chr(34)+"targetname"+chr(34)+" " + _
                                chr(34)+"goal"+chr(34)
                            print #m, "}"
                        end if
                        Finish(2) += 1

                    case 3
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                            chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"
                        select case Direction(XP,YP,ZP)
                            case North
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,0)
                            case East
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,1)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,0)
                            case South
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,1)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                            case West
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                        end select
                        print #m, "}"

                        print #m, "// brush 1"
                        print #m, "{"
                        ptf(1,-16,0,1,-16,-128,1,-144,0,1,"goal")
                        ptf(1,0,0,80,-128,0,80,0,0,-48,0)
                        ptf(1,0,0,80,0,0,-48,0,-128,80,0)
                        ptf(1,-128,-128,0,0,-128,0,-128,0,0,0)
                        ptf(1,-128,-128,-48,0,-128,79,0,-128,-48,0)
                        ptf(1,-128,-128,-48,-128,0,-49,-128,-128,80,0)
                        print #m, "}"
                        print #m, "}"
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                               chr(34)+"info_player_deathmatch"+chr(34)
                        print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-64; _
                               " "&YP*128-64;" "& ZP*64+24;chr(34)
                        print #m, "}"

                        if Finish(2) = 0 then
                            print #m, "// entity ";Entity
                            Entity += 1
                            print #m, "{"
                            print #m, chr(34)+"classname"+chr(34) + _
                                " "+chr(34)+"target_position"+chr(34)
                            print #m, chr(34)+"origin"+chr(34)+" " + _
                                chr(34)& XP*128-64;" "& YP*128-64;" "& _
                                ZP*64+128;chr(34)
                            print #m, chr(34)+"targetname"+chr(34)+" " + _
                                chr(34)+"goal"+chr(34)
                            print #m, "}"
                        end if
                        Finish(2) += 1

                    case 4
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                            chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"
                        select case Direction(XP,YP,ZP)
                            case North
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,0)
                            case East
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,0)
                            case South
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,1)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,0)
                            case West
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                        end select
                        print #m, "}"

                        print #m, "// brush 1"
                        print #m, "{"
                        ptf(1,-16,0,1,-16,-128,1,-144,0,1,"goal")
                        ptf(1,0,0,80,-128,0,80,0,0,-48,0)
                        ptf(1,0,0,80,0,0,-48,0,-128,80,0)
                        ptf(1,-128,-128,0,0,-128,0,-128,0,0,0)
                        ptf(1,-128,-128,-48,0,-128,79,0,-128,-48,0)
                        ptf(1,-128,-128,-48,-128,0,-49,-128,-128,80,0)
                        print #m, "}"
                        print #m, "}"
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                               chr(34)+"info_player_deathmatch"+chr(34)
                        print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-64; _
                               " "&YP*128-64;" "& ZP*64+24;chr(34)
                        print #m, "}"

                        if Finish(2) = 0 then
                            print #m, "// entity ";Entity
                            Entity += 1
                            print #m, "{"
                            print #m, chr(34)+"classname"+chr(34) + _
                                " "+chr(34)+"target_position"+chr(34)
                            print #m, chr(34)+"origin"+chr(34)+" " + _
                                chr(34)& XP*128-64;" "& YP*128-64;" "& _
                                ZP*64+128;chr(34)
                            print #m, chr(34)+"targetname"+chr(34)+" " + _
                                chr(34)+"goal"+chr(34)
                            print #m, "}"
                        end if
                        Finish(2) += 1

                    case 5
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                            chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"
                        ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                        ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                        ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                        ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                        ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                        ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,0)
                        print #m, "}"

                        print #m, "// brush 1"
                        print #m, "{"
                        ptf(1,-16,0,1,-16,-128,1,-144,0,1,"goal")
                        ptf(1,0,0,80,-128,0,80,0,0,-48,0)
                        ptf(1,0,0,80,0,0,-48,0,-128,80,0)
                        ptf(1,-128,-128,0,0,-128,0,-128,0,0,0)
                        ptf(1,-128,-128,-48,0,-128,79,0,-128,-48,0)
                        ptf(1,-128,-128,-48,-128,0,-49,-128,-128,80,0)
                        print #m, "}"
                        print #m, "}"
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                               chr(34)+"info_player_deathmatch"+chr(34)
                        print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-64; _
                               " "&YP*128-64;" "& ZP*64+24;chr(34)
                        print #m, "}"

                        if Finish(2) = 0 then
                            print #m, "// entity ";Entity
                            Entity += 1
                            print #m, "{"
                            print #m, chr(34)+"classname"+chr(34) + _
                                " "+chr(34)+"target_position"+chr(34)
                            print #m, chr(34)+"origin"+chr(34)+" " + _
                                chr(34)& XP*128-64;" "& YP*128-64;" "& _
                                ZP*64+128;chr(34)
                            print #m, chr(34)+"targetname"+chr(34)+" " + _
                                chr(34)+"goal"+chr(34)
                            print #m, "}"
                        end if
                        Finish(2) += 1

                    case 6
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                            chr(34)+"worldspawn"+chr(34)
                        print #m, "// brush 0"
                        print #m, "{"
                        select case Direction(XP,YP,ZP)
                            case North
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                            case East
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,1)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,0)
                            case South
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,1)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                            case West
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,1)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                        end select

                        print #m, "}"
                        print #m, "// brush 1"
                        print #m, "{"
                        ptf(1,-16,0,1,-16,-128,1,-144,0,1,"goal")
                        ptf(1,0,0,80,-128,0,80,0,0,-48,0)
                        ptf(1,0,0,80,0,0,-48,0,-128,80,0)
                        ptf(1,-128,-128,0,0,-128,0,-128,0,0,0)
                        ptf(1,-128,-128,-48,0,-128,79,0,-128,-48,0)
                        ptf(1,-128,-128,-48,-128,0,-49,-128,-128,80,0)
                        print #m, "}"
                        print #m, "}"
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, chr(34)+"classname"+chr(34)+" " + _
                               chr(34)+"info_player_deathmatch"+chr(34)
                        print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& _
                            XP*128-64;" "&YP*128-64;" "& ZP*64+24;chr(34)
                        print #m, "}"

                        if Finish(2) = 0 then
                            print #m, "// entity ";Entity
                            Entity += 1
                            print #m, "{"
                            print #m, chr(34)+"classname"+chr(34) + _
                                " "+chr(34)+"target_position"+chr(34)
                            print #m, chr(34)+"origin"+chr(34)+" " + _
                                chr(34)& XP*128-64;" "& YP*128-64;" "& _
                                ZP*64+128;chr(34)
                            print #m, chr(34)+"targetname"+chr(34)+" " + _
                                chr(34)+"goal"+chr(34)
                            print #m, "}"
                        end if
                        Finish(2) += 1
                end select
            next XP
        next YP
    next ZP
end sub
