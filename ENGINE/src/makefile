cc = g++
flags = -Wall
dependencies = -I./display -I./map -I./startmenu -lSDL2 -lSDL2_ttf -lSDL2_image
exe = flat_annihilation

# Fichiers sources et headers
srcs = main.cpp display/display.cpp map/map.cpp startmenu/startmenu.cpp
hdrs = display/display.hpp map/map.hpp startmenu/startmenu.hpp

$(exe) : $(srcs) $(hdrs)
	$(cc) $(flags) $(srcs) -o $(exe) $(dependencies)

clean:
	rm -f $(exe)