CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g -Iincludes

# Directories
SRC_DIR := src
MAIN_DIR := main
BUILD_DIR := build
BIN_DIR := bin

# ===== SOURCE DEFINITIONS =====

# Core libraries
RBT_SRC := $(SRC_DIR)/RedBlackTree.cpp
TANGO_SRC := $(SRC_DIR)/TangoTree.cpp

# Main files
RBT_MAIN := $(MAIN_DIR)/MainRBT.cpp
TT_MAIN := $(MAIN_DIR)/MainTT.cpp

# ===== OBJECT FILES =====

RBT_OBJ_LIB := $(BUILD_DIR)/RedBlackTree.o
TANGO_OBJ := $(BUILD_DIR)/TangoTree.o

RBT_MAIN_OBJ := $(BUILD_DIR)/MainRBT.o
TT_MAIN_OBJ := $(BUILD_DIR)/MainTT.o

# ===== TARGETS =====

RBT_TARGET := $(BIN_DIR)/RedBlackTree
TT_TARGET := $(BIN_DIR)/TangoTree

# ===== DEFAULT =====

all: $(RBT_TARGET) $(TT_TARGET)

# ===== BUILD RULES =====

# --- RedBlackTree executable ---
$(RBT_TARGET): $(RBT_OBJ_LIB) $(RBT_MAIN_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# --- TangoTree executable ---
$(TT_TARGET): $(RBT_OBJ_LIB) $(TANGO_OBJ) $(TT_MAIN_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# ===== COMPILATION RULES =====

# Compile src/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile main/
$(BUILD_DIR)/%.o: $(MAIN_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ===== CLEAN =====

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# ===== PHONY TARGETS =====

.PHONY: all clean RedBlackTree TangoTree

# Convenience commands
RedBlackTree: $(RBT_TARGET)
TangoTree: $(TT_TARGET)