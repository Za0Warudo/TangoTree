# Red-Black Tree Implementation

This project implements a Red-Black Tree data structure in C++. The Red-Black Tree is a balanced binary search tree that ensures efficient insertion, deletion, and search operations.

## Files

- **src/RedBlackTree.cpp**: Contains the implementation of the Red-Black Tree, including functions for insertion, deletion, searching, and balancing the tree.
- **src/RedBlackTree.h**: Contains the declarations for the Red-Black Tree and its associated functions, as well as the definition of the `Node` class used in the tree.
- **src/Node.cpp**: Implements the `Node` class, which represents the nodes in the Red-Black Tree.
- **Makefile**: Automates the build process, specifying how to compile the source files and link them to create the executable.

## Build Instructions

To compile the project, navigate to the project directory and run the following command:

```
make
```

This will create an executable named `RedBlackTree`.

## Usage

After building the project, you can run the executable with the following command:

```
./RedBlackTree
```

You can interact with the Red-Black Tree through the command line, using various operations such as insertion, deletion, and searching.

## Cleaning Up

To remove the compiled object files and the executable, run:

```
make clean
```

This will clean up the project directory by deleting all generated files.