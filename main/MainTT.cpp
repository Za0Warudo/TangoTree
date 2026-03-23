/**
 * @file MainTT.cpp
 * @author ZaWarudo (@zawarudo)
 * @brief
 * @version 0.1
 * @date 2026-03-22
 * @copyright Copyright (c) 2026
 *
 * For tests purposes, you can use this program to check if a key is present in a tango tree created.
 *
 * How to use it:
 * Since the Tango Tree include keys from 1 to n, first you must an integer to be the n value. After this, the program
 * will construct a Tango Tree structure and you can type any integer you wanna search in the tree.
 *
 * * Example:
 * * input:
 *      10
 *      1
 *      11
 *      7
 *      5
 *      13
 * * output:
 *      True
 *      False
 *      True
 *      True
 *      False
 *      True
 *
 * Note:
 * This is a kind of a silly test, but can be used to show the power of the Tango Tree to perform a sequence of access.
 */

// includes.
#include "TangoTree.h"
#include <iostream>

int main() {
  int n, key;
  std::cin >> n;
  TangoTree t = TangoTree(n);

  while (std::cin >> key) {
    std::cout << (t.contains(key) ? "True" : "False") << std::endl;
  }
  return 0;
}