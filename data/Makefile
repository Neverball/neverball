
install : sol/move.sol sol/flat.sol

sol/move.sol : map/move.map
	../bin/map2sol map/move.map sol/move.sol
sol/flat.sol : map/flat.map
	../bin/map2sol map/flat.map sol/flat.sol

clean :
	rm -f map/*.bak
	rm -f sol/*.sol

