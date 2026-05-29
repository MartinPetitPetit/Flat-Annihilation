CXX      = g++
CXXFLAGS = -Wall -Wextra -std=c++17

SRC_DIR   = src
BUILD_DIR = build
OBJ_DIR   = $(BUILD_DIR)/obj
BIN_DIR   = $(BUILD_DIR)/bin

TARGET_NAME = flat_annihilation

# Détection de l'OS
ifeq ($(OS),Windows_NT)
	TARGET = $(BIN_DIR)/$(TARGET_NAME).exe
	LIBS   = -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_mixer
else
	TARGET = $(BIN_DIR)/$(TARGET_NAME)
	LIBS   = -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_mixer
endif

INCLUDES = \
	-I$(SRC_DIR) \
	-I$(SRC_DIR)/Game \
	-I$(SRC_DIR)/Frontend/Window \
	-I$(SRC_DIR)/Frontend/Renderer \
	-I$(SRC_DIR)/Frontend/EventManager \
	-I$(SRC_DIR)/Frontend/SelectionManager \
	-I$(SRC_DIR)/Frontend/UIManager \
	-I$(SRC_DIR)/Frontend/Sound \
	-I$(SRC_DIR)/Backend/Map \
	-I$(SRC_DIR)/Backend/Player \
	-I$(SRC_DIR)/Backend/Cell \
	-I$(SRC_DIR)/Backend/Resource \
	-I$(SRC_DIR)/Backend/ResourceManager \
	-I$(SRC_DIR)/Backend/TerrainSprite \
	-I$(SRC_DIR)/Backend/Building \
	-I$(SRC_DIR)/Backend/Entity \
	-I$(SRC_DIR)/Backend/Coordinate \
	-I$(SRC_DIR)/Backend/Unit \
	-I$(SRC_DIR)/Backend/Pathing \
	-I$(SRC_DIR)/Backend/AI \
	-I$(SRC_DIR)/Backend/Combat

# Tous les .cpp sont compilés automatiquement, sauf l'ancien doublon.
# Le pathfinding organisé est dans src/Backend/Pathing/Pathfinding.cpp.
SRCS = $(shell find $(SRC_DIR) -name "*.cpp")
SRCS := $(filter-out $(SRC_DIR)/Backend/Map/Pathfinding.cpp,$(SRCS))

OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -rf $(BIN_DIR)

re: fclean all

.PHONY: all run clean fclean re

-include $(DEPS)
