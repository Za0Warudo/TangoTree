# Tango Tree Implementation

## Overview

This project provides a functional implementation of the Tango Tree data structure. Its primary goal is to offer a clean
, readable codebase for developers and students who are already familiar with the theoretical concepts of the Tango Tree.

**Note:** This repository focuses purely on the implementation and does not include theoretical analysis or computational 
results. The accompanying comments and code explanations assume prior knowledge of the Tango Tree algorithms.

## Getting Started

### Prerequisites 

The project requires the following tools to be installed on your system:

C++ Compiler (C++20 Standard):

    A compiler supporting C++20 (e.g., GCC 11+ or Clang 13+).

Python (for Testing):

    Python 3.8 or higher is required to run the automated test generator script.

### Installation
Follow these steps to install and compile the project in your machine: 

#### Clone the repository
    git clone https://github.com/Za0Warudo/TangoTree.git 
    cd TangoTree 

#### Build 
    cd scripts
    make 

This last line will create a *bin* directory in your root project containing all the executable files. 

### How to use it
After the compilation steps you can run the executable file *TangoTree* in the bin directory using:

    ./TangoTree

The first line of the program expect the size *n* of the Tango Tree. The following lines accepts two types 
of operation: 

* 1  <*k*> : Search for the *k* key in the tree   
* 2 : Print current tree 

Take the example: 
    
    ./TangoTree 
    15      # keys goes from [1, 15] 
    2       # print 
    1 4     # search(4) 
    1 10
    1 2 
    1 9 
    2

So we can iteratively see how the tree is changing with each search.

### Tests 

For performance test and greater tree size and number of search we also provide 
the *RandomTestGenerator.py* that can generate a desired size with randomness based in 
a uniform distribution or a gaussian distribution. 

#### How to run it
The generator script can be run using the following specification:
* RandomTestGenerator.py <*n*> <*q*> <*d*>

Where: 
* *n* -  The tree size - *int*
* *q* -  How many queries - *int* 
* *d* -  The  distribution - 'u' or 'g'

For exemple we can generate a *2* queries input in a size *7* tree based in the 
uniform distribution with this command:

    python3 RandomTestGenerator.py 7 2 u 
    > 7 
    > 1 5 
    > 1 2 

# Contact and Acknowledgements
### Author/Maintainer

Github Profile: Za0Warudo 

Name: Vinicius Gomes Paz

Email: viniciusgomespaz24@gmail.com

### Acknowledgments

I would like to sincerely thank Dr.
Professor′s Cristina Gomes from IME-USP
for her invaluable guidance and support throughout the development of this project. Her expertise was
essential in shaping the core implementation strategy for the Tango Tree.

You can check some other projects and more about in her official page: https://www.ime.usp.br/~cris/