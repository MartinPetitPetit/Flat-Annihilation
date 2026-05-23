cc    = g++
flags = -Wall -std=c++17
includes = -I./Frontend/Window \
	-I./Frontend/Renderer \
	-I./Frontend/EventManager \
	-I./Frontend/SelectionManager \
	-I./Frontend/UIManager \
	-I./Backend/Map \
	-I./Backend/Player \
	-I./Backend/Cell \
	-I./Game

# Détection de l'OS
ifeq ($(OS),Windows_NT)
	# Windows (MSYS2/MinGW)
	libs = -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_mixer
	exe_ext = .exe
else
	# Linux
	libs = -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_mixer
	exe_ext =
endif

srcs  = main.cpp \
	Game/Game.cpp \
	Frontend/Window/Window.cpp \
	Frontend/Renderer/Renderer.cpp \
	Frontend/EventManager/EventManager.cpp \
	Frontend/SelectionManager/SelectionManager.cpp \
	Frontend/UIManager/UIManager.cpp \
	Backend/Map/Map.cpp \
	Backend/Map/Mountain.cpp \
	Backend/Map/River.cpp \
	Backend/Map/Ravine.cpp \
	Backend/Map/Forest.cpp \
	Backend/Map/Bush.cpp \
	Backend/Cell/Cell.cpp \
	Backend/Player/Player.cpp \
	Frontend/Sound/Sound.cpp

hdrs  = Game/Game.hpp \
	Frontend/Window/Window.hpp \
	Frontend/Renderer/Renderer.hpp \
	Frontend/EventManager/EventManager.hpp \
	Frontend/SelectionManager/SelectionManager.hpp \
	Frontend/UIManager/UIManager.hpp \
	Backend/Map/Map.hpp \
	Backend/Player/Player.hpp \
	Backend/Cell/Cell.hpp \
	Frontend/Sound/Sound.hpp

exe   = flat_annihilation

$(exe)$(exe_ext) : $(srcs) $(hdrs)
	$(cc) $(flags) $(includes) $(srcs) -o $(exe)$(exe_ext) $(libs)

clean:
	rm -f $(exe)$(exe_ext)

.PHONY: clean