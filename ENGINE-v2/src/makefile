cc    = g++
flags = -Wall -std=c++17
deps  = -I./Frontend/Window \
        -I./Frontend/Renderer \
        -I./Frontend/EventManager \
        -I./Frontend/SelectionManager \
        -I./Frontend/UIManager \
        -I./Backend/Map \
        -I./Backend/Player \
        -I./Game \
        -lSDL2 -lSDL2_ttf -lSDL2_image

srcs  = main.cpp \
        Game/Game.cpp \
        Frontend/Window/Window.cpp \
        Frontend/Renderer/Renderer.cpp \
        Frontend/EventManager/EventManager.cpp \
        Frontend/SelectionManager/SelectionManager.cpp \
        Frontend/UIManager/UIManager.cpp \
        Backend/Map/Map.cpp \
        Backend/Player/Player.cpp

hdrs  = Game/Game.hpp \
        Frontend/Window/Window.hpp \
        Frontend/Renderer/Renderer.hpp \
        Frontend/EventManager/EventManager.hpp \
        Frontend/SelectionManager/SelectionManager.hpp \
        Frontend/UIManager/UIManager.hpp \
        Backend/Map/Map.hpp \
        Backend/Player/Player.hpp

exe   = flat_annihilation

$(exe) : $(srcs) $(hdrs)
	$(cc) $(flags) $(srcs) -o $(exe) $(deps)

clean:
	rm -f $(exe)