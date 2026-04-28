# Tango Tree 

## Overview
This project provides a functional implementation of the **Tango Tree** data structure.

Originally introduced by **Erik D. Demaine, Dion Harmon, John Iacono, and Mihai Pătrașcu**, the Tango Tree is a binary search tree designed to achieve competitive performance with the optimal binary search tree for any access sequence.

The main idea behind the Tango Tree is to represent each **preferred path** as a balanced binary search tree. A preferred path consists of nodes that were most recently traversed during searches in the reference tree for a given access sequence.

To maintain the invariant that nodes belonging to the same preferred path remain grouped together, the structure uses an update procedure—commonly referred to as **tango**—which adjusts the preferred path trees after each search operation.

The Tango Tree is particularly important because it was the first binary search tree structure proven to achieve an **(O(log log n))-competitive ratio** with respect to the optimal offline binary search tree for any search sequence. This marked one of the first major advances toward resolving the **dynamic optimality conjecture**.

<p align="center">
  <img src="https://cdn11.bigcommerce.com/s-yzgoj/images/stencil/1280x1280/products/1844782/3712381/apigql5nd__30458.1626799763.jpg?c=2" alt="Tango Tree structure" width="300"/>
</p>

## Getting Started

### Prerequisites 

Before building the project, ensure you have the following installed:

* CMake (v3.14 or higher)

* C++ Compiler: A compiler supporting C++17 (e.g., GCC 7+, Clang 5+, or MSVC 2017+).

* Git: Required by CMake to fetch GoogleTest automatically.

## Installation & Building

This project uses CMake to automate the build process. Follow these steps to compile:

1. **Clone the repository:**
    ```bash
    git clone https://github.com/Za0Warudo/RedBlackTree.git
    cd RedBlackTree
    ```

2. **Configure the project:**
This step creates a build directory and generates the necessary build files.
    ```bash
     cmake -S . -B build
     ```
3. **Build the project:**
This compiles the source code and the test suite.
    ```bash
    cmake --build build
    ```

### Running Tests

Since GoogleTest is integrated via CMake, you can run the automated tests using ctest from within the build directory:

```bash
cd build
ctest --output-on-failure
```

### How to use it

After the compilation steps you can run the executable file *TangoTree* in the bin directory using:

    ./TangoTree

The first line of the program expect the size *n* of the Tango Tree. The following lines accepts two types 
of operation: 

* 1  <*k*> : Search for the *k* key in the tree   
* 2 : Print current tree 

Take the example: 

```bash
./TangoTree 
15      # keys goes from [1, 15] 
2       # print 
1 4     # search(4) 
1 10
1 2 
1 9 
2
```

So we can iteratively see how the tree is changing with each search.

we also provide an interface for our red-black tree implementation, you can check in the MainRBT.cpp file.


##  Contact and Acknowledgements
### Author/Maintainer

Github Profile: Za0Warudo 

Student: Vinicius Gomes Paz

Email: viniciusgomespaz24@gmail.com, viniciusgomes24@usp.br

### Acknowledgments

I would like to sincerely thank Dr.
Professor′s Cristina Gomes from IME-USP
for her invaluable guidance and support throughout the development of this project. Her expertise was
essential in shaping the core implementation strategy for the Tango Tree.

You can check some other projects and more about in her official page: https://www.ime.usp.br/~cris/