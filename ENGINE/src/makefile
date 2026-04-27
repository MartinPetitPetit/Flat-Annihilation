cc = g++
flags = -Wall
dependencies = -I./display -I./map -I./start_menu -lSDL2 -lSDL2_ttf
exe = flat_annihilation

# Fichiers sources et headers
srcs = main.cpp display/display.cpp map/map.cpp start_menu/start_menu.cpp
hdrs = display/display.hpp map/map.hpp start_menu/start_menu.hpp

$(exe) : $(srcs) $(hdrs)
	$(cc) $(flags) $(srcs) -o $(exe) $(dependencies)

clean:
	rm -f $(exe)