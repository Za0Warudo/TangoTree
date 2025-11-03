# Compiler and flags
CC := g++
CFLAGS := -Wall -g --std=c++20

# Directories
ROOT_DIR := ../
SRC_DIR := $(ROOT_DIR)src
BIN_DIR := $(ROOT_DIR)bin

# Source files for each program
TANGO_SRC := $(SRC_DIR)/TangoTree.cpp
RB_SRC    := $(SRC_DIR)/RedBlackTree.cpp

# Object files
TANGO_OBJ := $(BIN_DIR)/TangoTree.o
RB_OBJ    := $(BIN_DIR)/RedBlackTree.o

# Executables
TANGO_EXE := $(BIN_DIR)/TangoTree
RB_EXE    := $(BIN_DIR)/RedBlackTree

# Default rule builds both
all: $(TANGO_EXE) $(RB_EXE)

# Build TangoTree executable
$(TANGO_EXE): $(TANGO_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $<

# Build RedBlackTree executable
$(RB_EXE): $(RB_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $<

# Compile source files into object files
$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure bin directory exists
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Clean
clean:
	rm -rf $(BIN_DIR)/*.o $(TANGO_EXE) $(RB_EXE)
