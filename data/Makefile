
MAPC= ../mapc

all : sol/baby.sol sol/move.sol sol/flat.sol sol/spiral.sol

sol/baby.sol   : $(MAPC) map/baby.map
	$(MAPC) map/baby.map sol/baby.sol
sol/move.sol   : $(MAPC) map/move.map
	$(MAPC) map/move.map sol/move.sol
sol/flat.sol   : $(MAPC) map/flat.map
	$(MAPC) map/flat.map sol/flat.sol
sol/spiral.sol : $(MAPC) map/spiral.map
	$(MAPC) map/spiral.map sol/spiral.sol

clean :
	rm -f map/*.bak
	rm -f sol/*.sol
