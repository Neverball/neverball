declare sub place_block
sub place_block
	if (BlockType = 1) AND (BlockSet = 0) AND (Start = 0) AND (YP < 10) AND PlacementTest(PlacementFormula) = 0 then
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"worldspawn"+chr(34)
		print #m, "// brush 0"
		print #m, "{"
		plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"arrow-green")
		plot_face(0,0,0,64,-128,0,64,0,0,-64,"invisible")
		plot_face(0,0,0,64,0,0,-64,0,-128,64,"turf-grey")
		plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
		plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"turf-grey")
		plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
		print #m, "}"
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"info_player_start"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-64;" "& YP*128-64;" "& ZP*64+24;chr(34)
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"info_player_intermission"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-64;" "& YP*128-64;" "& ZP*64+224;chr(34)
		print #m, chr(34)+"target"+chr(34)+" "+chr(34)+"goal"+chr(34)
		print #m, "}"
		Start = 1
		PlacementTest(PlacementFormula) = 1
		place_gfx(0)
		LevelTime += 150
		MinimumLevelTime += 50
		Blocks += 1
		Openings += 1
		Putt = 2
	elseif (BlockType = 2) AND (BlockSet = 0) AND (Money + 1 < = MaxMoney) AND PlacementTest(PlacementFormula) = 0 then
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"worldspawn"+chr(34)
		print #m, "// brush 0"
		print #m, "{"
		if (Rotation = 1) OR (Rotation = 3) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"turf-green")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"invisible")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"turf-grey")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"invisible")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
		elseif (Rotation = 2) OR (Rotation = 4) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"turf-green")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"turf-grey")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"invisible")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"turf-grey")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
		end if
		print #m, "}"
		print #m, "}"
		PlacementTest(PlacementFormula) = 2
		place_gfx(0)
		LevelTime += 100
		MinimumLevelTime += 75
		Money += 1
		Blocks += 1
		Putt = 2
	elseif (BlockType = 3) AND (BlockSet = 0) AND (Money + 1 < = MaxMoney) AND PlacementTest(PlacementFormula) = 0 then
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"worldspawn"+chr(34)
		print #m, "// brush 0"
		print #m, "{"
		if (Rotation = 1) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"turf-green")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"turf-grey")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"turf-grey")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"invisible")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"invisible")
		elseif (Rotation = 2) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"turf-green")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"invisible")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"turf-grey")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"turf-grey")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"invisible")
		elseif (Rotation = 3) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"turf-green")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"invisible")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"invisible")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"turf-grey")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
		elseif (Rotation = 4) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"turf-green")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"turf-grey")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"invisible")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"invisible")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
		end if
		print #m, "}"
		print #m, "}"
		PlacementTest(PlacementFormula) = 3
		place_gfx(0)
		LevelTime += 200
		MinimumLevelTime += 150
		Money += 1
		Blocks += 1
		Putt = 2
	elseif (BlockType = 4) AND (BlockSet = 0) AND (Money + 1 < = MaxMoney) AND PlacementTest(PlacementFormula) = 0 then
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"worldspawn"+chr(34)
		print #m, "// brush 0"
		print #m, "{"
		if (Rotation = 1) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"turf-green")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"turf-grey")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"invisible")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"invisible")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"invisible")
		elseif (Rotation = 2) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"turf-green")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"invisible")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"turf-grey")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"invisible")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"invisible")
		elseif (Rotation = 3) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"turf-green")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"invisible")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"invisible")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"turf-grey")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"invisible")
		elseif (Rotation = 4) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"turf-green")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"invisible")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"invisible")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"invisible")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
		end if
		print #m, "}"
		print #m, "}"
		PlacementTest(PlacementFormula) = 4
		place_gfx(0)
		LevelTime += 300
		MinimumLevelTime += 200
		Money += 1
		Blocks += 1
		Openings += 1
		Putt = 2
	elseif (BlockType = 5) AND (BlockSet = 0) AND (Money + 1 < = MaxMoney) AND PlacementTest(PlacementFormula) = 0 then
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"worldspawn"+chr(34)
		print #m, "// brush 0"
		print #m, "{"
		plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"turf-green")
		plot_face(0,0,0,64,-128,0,64,0,0,-64,"invisible")
		plot_face(0,0,0,64,0,0,-64,0,-128,64,"invisible")
		plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
		plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"invisible")
		plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"invisible")
		print #m, "}"
		print #m, "}"
		PlacementTest(PlacementFormula) = 5
		place_gfx(0)
		LevelTime += 400
		MinimumLevelTime += 300
		Blocks += 1
		Openings += 2
		Putt = 2
	elseif (BlockType = 6) AND (BlockSet = 0) AND (Money + 1 < = MaxMoney) AND PlacementTest(PlacementFormula) = 0 then
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"worldspawn"+chr(34)
		print #m, "// brush 0"
		print #m, "{"
		if (Rotation = 1) OR (Rotation = 3) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"coin-green-small")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"invisible")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"turf-grey")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"invisible")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
		elseif (Rotation = 2) OR (Rotation = 4) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"coin-green-small")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"turf-grey")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"invisible")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"turf-grey")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
		end if
		print #m, "}"
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-96;" "& YP*128-96;" "& ZP*64+24;chr(34)
		print #m, chr(34)+"light"+chr(34)+" "+chr(34)+"1"+chr(34)
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-32;" "& YP*128-96;" "& ZP*64+24;chr(34)
		print #m, chr(34)+"light"+chr(34)+" "+chr(34)+"1"+chr(34)
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-96;" "& YP*128-32;" "& ZP*64+24;chr(34)
		print #m, chr(34)+"light"+chr(34)+" "+chr(34)+"1"+chr(34)
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-32;" "& YP*128-32;" "& ZP*64+24;chr(34)
		print #m, chr(34)+"light"+chr(34)+" "+chr(34)+"1"+chr(34)
		print #m, "}"
		PlacementTest(PlacementFormula) = 6
		place_gfx(0)
		LevelTime += 175
		MinimumLevelTime += 125
		TargetCoins += 1
		Coins += 4
		Money += 1
		Blocks += 1
		Putt = 2
	elseif (BlockType = 7) AND (BlockSet = 0) AND (Money + 1 < = MaxMoney) AND PlacementTest(PlacementFormula) = 0 then
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"worldspawn"+chr(34)
		print #m, "// brush 0"
		print #m, "{"
		if (Rotation = 1) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"coin-green-small")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"turf-grey")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"turf-grey")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"invisible")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"invisible")
		elseif (Rotation = 2) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"coin-green-small")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"invisible")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"turf-grey")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"turf-grey")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"invisible")
		elseif (Rotation = 3) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"coin-green-small")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"invisible")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"invisible")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"turf-grey")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
		elseif (Rotation = 4) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"coin-green-small")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"turf-grey")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"invisible")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"invisible")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
		end if
		print #m, "}"
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-96;" "& YP*128-96;" "& ZP*64+24;chr(34)
		print #m, chr(34)+"light"+chr(34)+" "+chr(34)+"1"+chr(34)
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-32;" "& YP*128-96;" "& ZP*64+24;chr(34)
		print #m, chr(34)+"light"+chr(34)+" "+chr(34)+"1"+chr(34)
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-96;" "& YP*128-32;" "& ZP*64+24;chr(34)
		print #m, chr(34)+"light"+chr(34)+" "+chr(34)+"1"+chr(34)
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-32;" "& YP*128-32;" "& ZP*64+24;chr(34)
		print #m, chr(34)+"light"+chr(34)+" "+chr(34)+"1"+chr(34)
		print #m, "}"
		PlacementTest(PlacementFormula) = 7
		place_gfx(0)
		LevelTime += 275
		MinimumLevelTime += 200
		TargetCoins += 1
		Coins += 4
		Money += 1
		Blocks += 1
		Putt = 2
	elseif (BlockType = 8) AND (BlockSet = 0) AND (Money + 1 < = MaxMoney) AND PlacementTest(PlacementFormula) = 0 then
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"worldspawn"+chr(34)
		print #m, "// brush 0"
		print #m, "{"
		if (Rotation = 1) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"turf-green")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"turf-grey")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"turf-grey")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"invisible")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
		elseif (Rotation = 2) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"turf-green")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"turf-grey")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"turf-grey")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"turf-grey")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"invisible")
		elseif (Rotation = 3) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"turf-green")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"invisible")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"turf-grey")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"turf-grey")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
		elseif (Rotation = 4) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"turf-green")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"turf-grey")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"invisible")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"turf-grey")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
		end if
		print #m, "}"
		print #m, "}"
		PlacementTest(PlacementFormula) = 8
		place_gfx(0)
		LevelTime += 50
		Blocks += 1
		Money += 1
		Openings -= 1
		Putt = 2
	elseif (BlockType = 9) AND (BlockSet = 0) AND (Money + 1 < = MaxMoney) AND PlacementTest(PlacementFormula) = 0 then
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"worldspawn"+chr(34)
		print #m, "// brush 0"
		print #m, "{"
		if (Rotation = 1) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"coin-green-small")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"turf-grey")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"turf-grey")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"invisible")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
		elseif (Rotation = 2) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"coin-green-small")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"turf-grey")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"turf-grey")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"turf-grey")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"invisible")
		elseif (Rotation = 3) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"coin-green-small")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"invisible")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"turf-grey")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"turf-grey")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
		elseif (Rotation = 4) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"coin-green-small")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"turf-grey")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"invisible")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"turf-grey")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
		end if
		print #m, "}"
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-96;" "& YP*128-96;" "& ZP*64+24;chr(34)
		print #m, chr(34)+"light"+chr(34)+" "+chr(34)+"1"+chr(34)
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-32;" "& YP*128-96;" "& ZP*64+24;chr(34)
		print #m, chr(34)+"light"+chr(34)+" "+chr(34)+"1"+chr(34)
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-96;" "& YP*128-32;" "& ZP*64+24;chr(34)
		print #m, chr(34)+"light"+chr(34)+" "+chr(34)+"1"+chr(34)
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-32;" "& YP*128-32;" "& ZP*64+24;chr(34)
		print #m, chr(34)+"light"+chr(34)+" "+chr(34)+"1"+chr(34)
		print #m, "}"
		PlacementTest(PlacementFormula) = 9
		place_gfx(0)
		LevelTime += 150
		MinimumLevelTime += 100
		TargetCoins += 1
		Coins += 4
		Blocks += 1
		Money += 1
		Openings -= 1
		Putt = 2
	elseif (BlockType = 10) AND (BlockSet = 0) AND (Money + 2 < = MaxMoney) AND PlacementTest(PlacementFormula) = 0 then
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"worldspawn"+chr(34)
		print #m, "// brush 0"
		print #m, "{"
		if (Rotation = 1) OR (Rotation = 3) then
			plot_face(0,-128,0,0,0,0,0,0,-16,16,"yellow")
			plot_face(0,0,0,0,-128,0,0,0,0,-16,"invisible")
			plot_face(0,0,0,64,0,0,-64,0,-16,64,"yellow")
			plot_face(0,-128,-16,-16,0,-16,-16,-128,0,-16,"yellow")
			plot_face(0,-128,-16,-64,-128,-16,64,0,-16,64,"yellow")
			plot_face(0,-128,-16,-64,-128,0,-64,-128,-16,64,"yellow")
		elseif (Rotation = 2) OR (Rotation = 4) then
			plot_face(0,0,0,0,0,-128,0,-16,-128,16,"yellow")
			plot_face(0,0,0,16,-16,0,16,0,0,-16,"yellow")
			plot_face(0,0,-128,0,0,0,0,0,-128,-16,"invisible")
			plot_face(0,-16,-128,-16,0,-128,-16,-16,-64,-16,"yellow")
			plot_face(0,-16,-128,-16,-16,-128,16,0,-128,-16,"yellow")
			plot_face(0,-16,-128,-16,-16,-64,-16,-16,-128,16,"yellow")
		end if
		print #m, "}"
		print #m, "// brush 1"
		print #m, "{"
		if (Rotation = 1) OR (Rotation = 3) then
			plot_face(0,0,-128,0,-128,-128,0,-128,-112,16,"yellow")
			plot_face(0,0,-112,64,-128,-112,64,0,-112,-64,"yellow")
			plot_face(0,0,-112,64,0,-112,-64,0,-128,64,"yellow")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,-112,-16,"yellow")
			plot_face(0,-128,-128,0,0,-128,0,0,-128,-16,"invisible")
			plot_face(0,-128,-128,-64,-128,-112,-64,-128,-128,64,"yellow")
		elseif (Rotation = 2) OR (Rotation = 4) then
			plot_face(0,-128,-128,0,-128,0,0,-112,-128,16,"yellow")
			plot_face(0,-112,0,16,-128,0,16,-112,0,-16,"yellow")
			plot_face(0,-112,-64,16,-112,-64,-16,-112,-128,16,"yellow")
			plot_face(0,-128,-128,-16,-112,-128,-16,-128,-64,-16,"yellow")
			plot_face(0,-128,-128,-16,-128,-128,16,-112,-128,-16,"yellow")
			plot_face(0,-128,0,0,-128,-128,0,-128,0,-16,"invisible")
		end if
		print #m, "}"
		print #m, "}"
		PlacementTest(PlacementFormula) = 10
		place_gfx(0)
		LevelTime += 350
		MinimumLevelTime += 200
		Money += 2
		Blocks += 1
		Putt = 2
	elseif (BlockType = 11) AND (BlockSet = 0) AND (Money + 1 < = MaxMoney) AND PlacementTest(PlacementFormula) = 0 then
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"worldspawn"+chr(34)
		print #m, "// brush 0"
		print #m, "{"
		if (Rotation = 1) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"coin-green-small")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"turf-grey")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"invisible")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"invisible")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"invisible")
		elseif (Rotation = 2) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"coin-green-small")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"invisible")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"turf-grey")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"invisible")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"invisible")
		elseif (Rotation = 3) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"coin-green-small")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"invisible")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"invisible")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"turf-grey")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"invisible")
		elseif (Rotation = 4) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"coin-green-small")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"invisible")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"invisible")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"invisible")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
		end if
		print #m, "}"
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-96;" "& YP*128-96;" "& ZP*64+24;chr(34)
		print #m, chr(34)+"light"+chr(34)+" "+chr(34)+"1"+chr(34)
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-32;" "& YP*128-96;" "& ZP*64+24;chr(34)
		print #m, chr(34)+"light"+chr(34)+" "+chr(34)+"1"+chr(34)
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-96;" "& YP*128-32;" "& ZP*64+24;chr(34)
		print #m, chr(34)+"light"+chr(34)+" "+chr(34)+"1"+chr(34)
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-32;" "& YP*128-32;" "& ZP*64+24;chr(34)
		print #m, chr(34)+"light"+chr(34)+" "+chr(34)+"1"+chr(34)
		print #m, "}"
		PlacementTest(PlacementFormula) = 11
		place_gfx(0)
		LevelTime += 425
		MinimumLevelTime += 300
		TargetCoins += 1
		Coins += 4
		Blocks += 1
		Money += 1
		Openings += 1
		Putt = 2
	elseif (BlockType = 12) AND (BlockSet = 0) AND (Money + 1 < = MaxMoney) AND PlacementTest(PlacementFormula) = 0 then
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"worldspawn"+chr(34)
		print #m, "// brush 0"
		print #m, "{"
		plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"coin-green-small")
		plot_face(0,0,0,64,-128,0,64,0,0,-64,"invisible")
		plot_face(0,0,0,64,0,0,-64,0,-128,64,"invisible")
		plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
		plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"invisible")
		plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"invisible")
		print #m, "}"
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-96;" "& YP*128-96;" "& ZP*64+24;chr(34)
		print #m, chr(34)+"light"+chr(34)+" "+chr(34)+"1"+chr(34)
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-32;" "& YP*128-96;" "& ZP*64+24;chr(34)
		print #m, chr(34)+"light"+chr(34)+" "+chr(34)+"1"+chr(34)
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-96;" "& YP*128-32;" "& ZP*64+24;chr(34)
		print #m, chr(34)+"light"+chr(34)+" "+chr(34)+"1"+chr(34)
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-32;" "& YP*128-32;" "& ZP*64+24;chr(34)
		print #m, chr(34)+"light"+chr(34)+" "+chr(34)+"1"+chr(34)
		print #m, "}"
		PlacementTest(PlacementFormula) = 12
		place_gfx(0)
		LevelTime += 575
		MinimumLevelTime += 400
		TargetCoins += 1
		Coins += 4
		Blocks += 1
		Money += 1
		Openings += 2
		Putt = 2
	elseif (BlockType = 13) AND (BlockSet = 0) AND (Money + 1 < = MaxMoney) AND PlacementTest(PlacementFormula) = 0 AND PlacementTest(PlacementFormula+2000) = 0 then
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"worldspawn"+chr(34)
		print #m, "// brush 0"
		print #m, "{"
		if (Rotation = 1) then
			plot_face(0,-128,0,64,0,0,64,0,-128,0,"turf-green")
			plot_face(0,0,0,48,0,0,48,0,0,64,"invisible")
			plot_face(0,0,-128,0,0,0,0,0,0,-16,"turf-grey")
			plot_face(0,0,0,48,-128,0,48,0,-128,-16,"turf-grey")
			plot_face(0,0,-128,-64,-128,-128,64,0,-128,-64,"invisible")
			plot_face(0,0,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
		elseif (Rotation = 2) then
			plot_face(0,0,0,64,0,-128,64,-128,-128,0,"turf-green")
			plot_face(0,0,0,0,-128,0,0,0,0,-16,"turf-grey")
			plot_face(0,0,0,48,0,-128,48,0,-128,64,"invisible")
			plot_face(0,0,-128,48,0,0,48,-128,-128,-16,"turf-grey")
			plot_face(0,-128,-128,-64,-128,-128,64,0,-128,-64,"turf-grey")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"invisible")
		elseif (Rotation = 3) then
			plot_face(0,-128,0,0,0,0,0,0,-128,64,"turf-green")
			plot_face(0,0,0,0,-128,0,0,0,0,-16,"invisible")
			plot_face(0,0,-128,0,0,0,0,0,-128,-16,"turf-grey")
			plot_face(0,0,0,-16,-128,0,-16,-128,-128,48,"turf-grey")
			plot_face(0,0,-128,48,-128,-128,48,-128,-128,64,"invisible")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
		elseif (Rotation = 4) then
			plot_face(0,0,0,0,0,-128,0,-128,-128,64,"turf-green")
			plot_face(0,0,0,0,-128,0,0,0,0,-16,"turf-grey")
			plot_face(0,0,-128,0,0,0,0,0,-128,-16,"invisible")
			plot_face(0,-128,0,48,-128,-128,48,0,-128,-16,"turf-grey")
			plot_face(0,-128,-128,-64,-128,-128,64,0,-128,-64,"turf-grey")
			plot_face(0,-128,-128,48,-128,0,48,-128,0,64,"invisible")
		end if
		print #m, "}"
		print #m, "}"
		PlacementTest(PlacementFormula) = 13
		PlacementTest(PlacementFormula+2000) = 13
		place_gfx(0)
		place_gfx(2000)
		LevelTime += 150
		MinimumLevelTime += 75
		Money += 1
		Blocks += 1
		Putt = 2
	elseif (BlockType = 14) AND (BlockSet = 0) AND (Money + 1 < = MaxMoney) AND PlacementTest(PlacementFormula) = 0 then
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"worldspawn"+chr(34)
		print #m, "// brush 0"
		print #m, "{"
		if (Rotation = 1) OR (Rotation = 3) then
			plot_face(0,-128,-64,16,0,-64,16,0,-128,0,"turf-green")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"turf-grey")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,-128,-128,64,0,-128,-64,"invisible")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
			plot_face(0,0,-64,16,-128,-64,16,0,0,0,"turf-green")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"invisible")
		elseif (Rotation = 2) OR (Rotation = 4) then
			plot_face(0,-64,-128,16,-64,0,16,0,0,0,"turf-green")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"invisible")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,-128,-128,64,0,-128,-64,"turf-grey")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"turf-grey")
			plot_face(0,-64,0,16,-64,-128,16,-128,0,0,"turf-green")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"invisible")
		end if
		print #m, "}"
		print #m, "}"
		PlacementTest(PlacementFormula) = 15
		place_gfx(0)
		LevelTime += 125
		MinimumLevelTime += 75
		Money += 1
		Blocks += 1
		Putt = 2
	elseif (BlockType = 15) AND (BlockSet = 0) AND (Money + 1 < = MaxMoney) AND PlacementTest(PlacementFormula) = 0 then
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"worldspawn"+chr(34)
		print #m, "// brush 0"
		print #m, "{"
		if (Rotation = 1) OR (Rotation = 3) then
			plot_face(0,-128,-64,16,0,-64,16,0,-128,0,"coin-green-small")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"turf-grey")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,-128,-128,64,0,-128,-64,"invisible")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
			plot_face(0,0,-64,16,-128,-64,16,0,0,0,"coin-green-small")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"invisible")
		elseif (Rotation = 2) OR (Rotation = 4) then
			plot_face(0,-64,-128,16,-64,0,16,0,0,0,"coin-green-small")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"invisible")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,-128,-128,64,0,-128,-64,"turf-grey")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"turf-grey")
			plot_face(0,-64,0,16,-64,-128,16,-128,0,0,"coin-green-small")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"invisible")
		end if
		print #m, "}"
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-96;" "& YP*128-96;" "& ZP*64+32;chr(34)
		print #m, chr(34)+"light"+chr(34)+" "+chr(34)+"1"+chr(34)
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-32;" "& YP*128-96;" "& ZP*64+32;chr(34)
		print #m, chr(34)+"light"+chr(34)+" "+chr(34)+"1"+chr(34)
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-96;" "& YP*128-32;" "& ZP*64+32;chr(34)
		print #m, chr(34)+"light"+chr(34)+" "+chr(34)+"1"+chr(34)
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"light"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-32;" "& YP*128-32;" "& ZP*64+32;chr(34)
		print #m, chr(34)+"light"+chr(34)+" "+chr(34)+"1"+chr(34)
		print #m, "}"
		PlacementTest(PlacementFormula) = 15
		place_gfx(0)
		LevelTime += 225
		MinimumLevelTime += 150
		TargetCoins += 1
		Coins += 4
		Money += 1
		Blocks += 1
		Putt = 2
	elseif (BlockType = 16) AND (BlockSet = 0) AND (Money + 3 < = MaxMoney) AND PlacementTest(PlacementFormula) = 0 then
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"worldspawn"+chr(34)
		print #m, "// brush 0"
		print #m, "{"
		if (Rotation = 1) OR (Rotation = 3) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"turf-green")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"invisible")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"turf-grey")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"invisible")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
		elseif (Rotation = 2) OR (Rotation = 4) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"turf-green")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"turf-grey")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"invisible")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"turf-grey")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
		end if
		print #m, "}"
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"func_train"+chr(34)
		print #m, "// brush 0"
		print #m, "{"
		if (Rotation = 1) OR (Rotation = 3) then
			plot_face(0,-64,-32,64,-64,-96,64,-128,-32,64,"turf-grey")
			plot_face(0,-64,-32,16,-128,-32,16,-64,-32,0,"turf-grey")
			plot_face(0,-64,-32,16,-64,-32,0,-64,-96,16,"turf-grey")
			plot_face(0,-128,-96,0,-64,-96,0,-128,-32,0,"turf-grey")
			plot_face(0,-128,-96,0,-128,-96,16,-64,-96,0,"turf-grey")
			plot_face(0,-128,-96,0,-128,-32,0,-128,-96,16,"turf-grey")
		elseif (Rotation = 2) OR (Rotation = 4) then
			plot_face(0,-32,0,64,-32,-64,64,-96,0,64,"turf-grey")
			plot_face(0,-32,0,16,-96,0,16,-32,0,0,"turf-grey")
			plot_face(0,-32,0,16,-32,0,0,-32,-94,16,"turf-grey")
			plot_face(0,-96,-64,0,-32,-64,0,-96,0,0,"turf-grey")
			plot_face(0,-96,-64,0,-96,-64,16,-32,-64,0,"turf-grey")
			plot_face(0,-96,-64,0,-96,0,0,-96,-64,16,"turf-grey")
		end if
		print #m, "}"
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"path_corner"+chr(34)
		if (Rotation = 1) OR (Rotation = 3) then
			print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-32;" "& YP*128-64;" "& ZP*64-16;chr(34)
		elseif (Rotation = 2) OR (Rotation = 4) then
			print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-64;" "& YP*128-96;" "& ZP*64-16;chr(34)
		end if
		print #m, chr(34)+"targetname"+chr(34)+" "+chr(34)+"path_corner";Entity-1;chr(34)
		print #m, chr(34)+"target"+chr(34)+" "+chr(34)+"path_corner";Entity;chr(34)
		print #m, chr(34)+"speed"+chr(34)+" "+chr(34)+"3"+chr(34)
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"path_corner"+chr(34)
		if (Rotation = 1) OR (Rotation = 3) then
			print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-96;" "& YP*128-64;" "& ZP*64-16;chr(34)
		elseif (Rotation = 2) OR (Rotation = 4) then
			print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-64;" "& YP*128-32;" "& ZP*64-16;chr(34)
		end if
		print #m, chr(34)+"targetname"+chr(34)+" "+chr(34)+"path_corner";Entity-1;chr(34)
		print #m, chr(34)+"target"+chr(34)+" "+chr(34)+"path_corner";Entity-2;chr(34)
		print #m, chr(34)+"speed"+chr(34)+" "+chr(34)+"3"+chr(34)
		print #m, "}"
		PlacementTest(PlacementFormula) = 2
		place_gfx(0)
		LevelTime += 325
		MinimumLevelTime += 150
		Money += 3
		Blocks += 2
		Putt = 2
	elseif (BlockType = 26) AND (BlockSet = 0) AND PlacementTest(PlacementFormula) = 0 then
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"worldspawn"+chr(34)
		print #m, "// brush 0"
		print #m, "{"
		if (Rotation = 1) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"turf-green")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"turf-grey")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"turf-grey")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"invisible")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
		elseif (Rotation = 2) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"turf-green")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"turf-grey")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"turf-grey")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"turf-grey")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"invisible")
		elseif (Rotation = 3) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"turf-green")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"invisible")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"turf-grey")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"turf-grey")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
		elseif (Rotation = 4) then
			plot_face(0,-16,0,0,-16,-128,0,-144,0,0,"turf-green")
			plot_face(0,0,0,64,-128,0,64,0,0,-64,"turf-grey")
			plot_face(0,0,0,64,0,0,-64,0,-128,64,"invisible")
			plot_face(0,-128,-128,-16,0,-128,-16,-128,0,-16,"turf-grey")
			plot_face(0,-128,-128,-64,0,-128,64,0,-128,-64,"turf-grey")
			plot_face(0,-128,-128,-64,-128,0,-64,-128,-128,64,"turf-grey")
		end if
		print #m, "}"
		print #m, "// brush 1"
		print #m, "{"
		plot_face(1,-16,0,1,-16,-128,1,-144,0,1,"goal")
		plot_face(1,0,0,80,-128,0,80,0,0,-48,"invisible")
		plot_face(1,0,0,80,0,0,-48,0,-128,80,"invisible")
		plot_face(1,-128,-128,0,0,-128,0,-128,0,0,"invisible")
		plot_face(1,-128,-128,-48,0,-128,79,0,-128,-48,"invisible")
		plot_face(1,-128,-128,-48,-128,0,-49,-128,-128,80,"invisible")
		print #m, "}"
		print #m, "}"
		print #m, "// entity ";Entity;"
		Entity += 1
		print #m, "{"
		print #m, chr(34)+"classname"+chr(34)+" " + _
		          chr(34)+"info_player_deathmatch"+chr(34)
		print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-64;" ";
		          ""&YP*128-64;" "& ZP*64+24;chr(34)
		print #m, "}"
		if Finish = 0 then
			print #m, "// entity ";Entity;"
			Entity += 1
			print #m, "{"
			print #m, chr(34)+"classname"+chr(34)+" "+chr(34)+"target_position"+chr(34)
			print #m, chr(34)+"origin"+chr(34)+" "+chr(34)& XP*128-64;" "; _
			          ""& YP*128-64;" "& ZP*64+128;chr(34)
			print #m, chr(34)+"targetname"+chr(34)+" "+chr(34)+"goal"+chr(34)
			print #m, "}"
		end if
		PlacementTest(PlacementFormula) = 26
		place_gfx(0)
		LevelTime += 150
		MinimumLevelTime += 50
		Blocks += 1
		Finish += 1
		Openings -= 1
		Putt = 2
	end if
end sub
