cc = g++
flags = -Wall
objs = main.cpp display/display.cpp map/map.cpp start_menu/start_menu.cpp
dependencies = -I./display -I./map -lSDL2 -lSDL2_ttf
exe = flat_annihilation

$(exe) : $(objs)
	$(cc) $(flags) $(objs) -o $(exe) $(dependencies)