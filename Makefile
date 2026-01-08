 # Compiler Settings
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Iinclude

# Directories
SRC_DIR = src
OBJ_DIR = build
BIN_DIR = bin

# Source Discovery
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
# Object Generation
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Target Executable (Windows needs .exe)
TARGET = $(BIN_DIR)\mem_sim.exe

# Default Rule
all: $(TARGET)

# Linking Rule
$(TARGET): $(OBJS)
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	@echo "Linking..."
	$(CXX) $(OBJS) -o $(TARGET)
	@echo "Build successful! Run with: $(TARGET)"

# Compilation Rule
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Cleanup (Windows commands)
clean:
	@if exist $(OBJ_DIR) rd /s /q $(OBJ_DIR)
	@if exist $(BIN_DIR) rd /s /q $(BIN_DIR)

.PHONY: all clean