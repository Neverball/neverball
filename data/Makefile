
install : sol/move.sol sol/flat.sol sol/three.sol

sol/move.sol : map/move.map
	../bin/map2sol map/move.map sol/move.sol

sol/flat.sol : map/flat.map
	../bin/map2sol map/flat.map sol/flat.sol

sol/three.sol : map/three.map
	../bin/map2sol map/three.map sol/three.sol

clean :
	rm -f map/*.bak
	rm -f sol/*.sol

