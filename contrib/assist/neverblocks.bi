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
                        print #m, quote("classname")+" "+quote("worldspawn")
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
                        ptf(1,-16,0,1,-16,-128,1,-144,0,1,6)
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
                        print #m, quote("classname")+" " + _
                            quote("info_player_start")
                        print #m, quote("origin")+" "+quote(str(XP*128-64)+_
                            " "+str(YP*128-64)+" "+str(ZP*64+24))
                        print #m, "}"
                        /'
                         ' This provides the first part of the initial camera.
                         '/
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, quote("classname")+" "+chr(34) + _
                            "info_player_intermission"+chr(34)
                        print #m, quote("origin")+" "+quote(str(XP*128-64)+_
                            " "+str(YP*128-64)+" "+str(ZP*64+224))
                        print #m, quote("target")+" "+quote("goal")
                        print #m, "}"

                    case 11
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, quote("classname")+" "+quote("worldspawn")
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
                        print #m, quote("classname")+" "+quote("worldspawn")
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
                        print #m, quote("classname")+" "+quote("worldspawn")
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
                        print #m, quote("classname")+" "+quote("worldspawn")
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
                        print #m, quote("classname")+" "+quote("worldspawn")
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
                        print #m, quote("classname")+" "+quote("worldspawn")
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
                        print #m, quote("classname")+" "+quote("worldspawn")
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
                        print #m, quote("classname")+" "+quote("worldspawn")
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
                        print #m, quote("classname")+" "+quote("worldspawn")
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
                        print #m, quote("classname")+" "+quote("worldspawn")
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
                        print #m, quote("classname")+" "+quote("worldspawn")
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
                        print #m, quote("classname")+" "+quote("worldspawn")
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
                        print #m, quote("classname")+" "+quote("worldspawn")
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
                        print #m, quote("classname")+" "+quote("worldspawn")
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
                        print #m, quote("classname")+" "+quote("worldspawn")
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
                         ' Even trains can be utilized in this assistant.
                         '/
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, quote("classname")+" "+quote("func_train")
                        print #m, quote("target")+" " + _
                            quote("path_corner"+str(Entity+1))
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
                        print #m, quote("classname")+" "+quote("path_corner")
                        select case Direction(XP,YP,ZP)
                        	case North
                                print #m, quote("origin")+" "+quote( _
                                    str(XP*128-32)+" "+str(YP*128-64)+" " + _
                                    str(ZP*64-16))
                            case West
                                print #m, quote("origin")+" "+quote( _
                                    str(XP*128-64)+" "+str(YP*128-96)+" " + _
                                    str(ZP*64-16))
                            case South
                                print #m, quote("origin")+" "+quote( _
                                    str(XP*128-96)+" "+str(YP*128-64)+" " + _
                                    str(ZP*64-16))
                        	case West
                                print #m, quote("origin")+" "+quote( _
                                    str(XP*128-64)+" "+str(YP*128-32)+" " + _
                                    str(ZP*64-16))
                        end select
                        print #m, quote("targetname")+" " + _
                            quote("path_corner"+str(Entity-1))
                        print #m, quote("targetname")+" " + _
                            quote("path_corner"+str(Entity))
                        print #m, quote("speed")+" "+quote("3")
                        print #m, "}"

                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, quote("classname")+" "+quote("path_corner")
                        select case Direction(XP,YP,ZP)
                        	case North
                                print #m, quote("origin")+" "+quote( _
                                    str(XP*128-96)+" "+str(YP*128-64)+" " + _
                                    str(ZP*64-16))
                            case West
                                print #m, quote("origin")+" "+quote( _
                                    str(XP*128-64)+" "+str(YP*128-32)+" " + _
                                    str(ZP*64-16))
                            case South
                                print #m, quote("origin")+" "+quote( _
                                    str(XP*128-32)+" "+str(YP*128-64)+" " + _
                                    str(ZP*64-16))
                        	case West
                                print #m, quote("origin")+" "+quote( _
                                    str(XP*128-64)+" "+str(YP*128-96)+" " + _
                                    str(ZP*64-16))
                        end select
                        print #m, quote("targetname")+" " + _
                            quote("path_corner"+str(Entity-1))
                        print #m, quote("targetname")+" " + _
                            quote("path_corner"+str(Entity-2))
                        print #m, quote("speed")+" "+quote("3")
                        print #m, "}"

                    case 43
                        /'
                         ' This block has a narrow center.
                         '/
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, quote("classname")+" "+quote("path_corner")
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
                        print #m, quote("classname")+" "+quote("path_corner")
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
                        print #m, quote("classname")+" "+quote("path_corner")
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
                        print #m, quote("classname")+" "+quote("path_corner")
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
                        print #m, quote("classname")+" "+quote("path_corner")
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
                        print #m, quote("classname")+" "+quote("path_corner")
                        print #m, "// brush 0"
                        print #m, "{"
                        select case Direction(XP,YP,ZP)
                            case NS
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,0)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,1)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,0)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                            case EW
                                ptf(0,-16,0,0,-16,-128,0,-144,0,0,2)
                                ptf(0,0,0,64,-128,0,64,0,0,-64,1)
                                ptf(0,0,0,64,0,0,-64,0,-128,64,0)
                                ptf(0,-128,-128,-16,0,-128,-16,-128,0,-16,1)
                                ptf(0,-128,-128,-64,0,-128,64,0,-128,-64,1)
                                ptf(0,-128,-128,-64,-128,0,-64,-128,-128,64,1)
                        end select

                        /'
                         ' Notice that the brush below is completely
                         ' detail. What makes it so is the first argument
                         ' of argument of ptf. See neverassist.bi for more
                         ' information.
                         '/
                        print #m, "}"
                        print #m, "// brush 1"
                        print #m, "{"
                        ptf(1,-16,0,1,-16,-128,1,-144,0,1,7)
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
                        print #m, quote("classname")+" " + _
                            quote("info_player_deathmatch")
                        print #m, quote("origin")+" "+quote(str(XP*128-64)+_
                            " "+str(YP*128-64)+" "+str(ZP*64+24))
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
                            print #m, quote("classname")+" " + _
                                quote("target_position")
                            print #m, quote("origin")+" "+quote(str(XP*128-64)+_
                                " "+str(YP*128-64)+" "+str(ZP*64+128))
                            print #m, quote("targetname")+" "+quote("goal")
                            print #m, "}"
                        end if
                        Finish(2) += 1

                    case 3
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, quote("classname")+" "+quote("path_corner")
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
                        ptf(1,-16,0,1,-16,-128,1,-144,0,1,7)
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
                        print #m, quote("classname")+" " + _
                            quote("info_player_deathmatch")
                        print #m, quote("origin")+" "+quote(str(XP*128-64)+_
                            " "+str(YP*128-64)+" "+str(ZP*64+24))
                        print #m, "}"

                        if Finish(2) = 0 then
                            print #m, "// entity ";Entity
                            Entity += 1
                            print #m, "{"
                            print #m, quote("classname")+" " + _
                                quote("target_position")
                            print #m, quote("origin")+" "+quote(str(XP*128-64)+_
                                " "+str(YP*128-64)+" "+str(ZP*64+128))
                            print #m, quote("targetname")+" "+quote("goal")
                            print #m, "}"
                        end if
                        Finish(2) += 1

                    case 4
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, quote("classname")+" "+quote("path_corner")
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
                        ptf(1,-16,0,1,-16,-128,1,-144,0,1,7)
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
                        print #m, quote("classname")+" " + _
                            quote("info_player_deathmatch")
                        print #m, quote("origin")+" "+quote(str(XP*128-64)+_
                            " "+str(YP*128-64)+" "+str(ZP*64+24))
                        print #m, "}"

                        if Finish(2) = 0 then
                            print #m, "// entity ";Entity
                            Entity += 1
                            print #m, "{"
                            print #m, quote("classname")+" " + _
                                quote("target_position")
                            print #m, quote("origin")+" "+quote(str(XP*128-64)+_
                                " "+str(YP*128-64)+" "+str(ZP*64+128))
                            print #m, quote("targetname")+" "+quote("goal")
                            print #m, "}"
                        end if
                        Finish(2) += 1

                    case 5
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, quote("classname")+" "+quote("path_corner")
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
                        ptf(1,-16,0,1,-16,-128,1,-144,0,1,7)
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
                        print #m, quote("classname")+" " + _
                            quote("info_player_deathmatch")
                        print #m, quote("origin")+" "+quote(str(XP*128-64)+_
                            " "+str(YP*128-64)+" "+str(ZP*64+24))
                        print #m, "}"

                        if Finish(2) = 0 then
                            print #m, "// entity ";Entity
                            Entity += 1
                            print #m, "{"
                            print #m, quote("classname")+" " + _
                                quote("target_position")
                            print #m, quote("origin")+" "+quote(str(XP*128-64)+_
                                " "+str(YP*128-64)+" "+str(ZP*64+128))
                            print #m, quote("targetname")+" "+quote("goal")
                            print #m, "}"
                        end if
                        Finish(2) += 1

                    case 6
                        print #m, "// entity ";Entity
                        Entity += 1
                        print #m, "{"
                        print #m, quote("classname")+" "+quote("path_corner")
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
                        ptf(1,-16,0,1,-16,-128,1,-144,0,1,7)
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
                        print #m, quote("classname")+" " + _
                            quote("info_player_deathmatch")
                        print #m, quote("origin")+" "+quote(str(XP*128-64)+_
                            " "+str(YP*128-64)+" "+str(ZP*64+24))
                        print #m, "}"

                        if Finish(2) = 0 then
                            print #m, "// entity ";Entity
                            Entity += 1
                            print #m, "{"
                            print #m, quote("classname")+" " + _
                                quote("target_position")
                            print #m, quote("origin")+" "+quote(str(XP*128-64)+_
                                " "+str(YP*128-64)+" "+str(ZP*64+128))
                            print #m, quote("targetname")+" "+quote("goal")
                            print #m, "}"
                        end if
                        Finish(2) += 1
                end select
            next XP
        next YP
    next ZP
end sub
