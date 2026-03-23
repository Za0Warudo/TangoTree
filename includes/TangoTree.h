#ifndef TANGOTREE_H
#define TANGOTREE_H

/**
 * @file TangoTree.h
 *
 * @author Zawarudo (@zawarudo)
 *
 * @brief A header file for the Tango Tree data structure implementation in c++.
 * The implementation includes a Tango Tree class that supports a constructor method and a containing method.
 * A Tango Tree is a self-adjusting binary search tree that operates based on a reference tree and performs operations
 * based on the structure of the reference tree. The Tango Tree supports efficient search operations and can be used in
 * various applications where dynamic data structures are required.
 *
 * The Tango Tree has a fundamental importance in the field of data structures and algorithms, particularly in the
 * context of dynamic trees and self-adjusting data structures. It was the first data structure to achieve a concrete
 * partial result in the dynamic optimality conjecture, which states that there exists a binary search tree that can perform any
 * sequence of search operations in time proportional to the optimal offline binary search tree for that sequence.
 * 
 * @version 1.0
 * @date 2026-03-15
 */

// includes. //
#include "RedBlackTree.h"

/**
 * @brief A class representing a Tango Tree data structure. A Tango Tree is a self-adjusting binary search tree that
 * operates based in a reference tree and performs operations based on the structure of the reference tree. The Tango
 * Tree supports efficient search operations and can be used in various applications where dynamic data structures are
 * required.
 */
class TangoTree {
private:
  Node *root; // The Tango Tree's root node.

public:
  /**
   * @brief Construct a new Tango Tree object
   * @param n The number of nodes in the reference tree. The constructor will build a Tango Tree based on a reference
   * tree with n nodes (1 to n).
   */
  TangoTree(int n);

  /**
   * @brief Checks if the Tango Tree contains the given key. This operation performs a search for the specified key in
   * the Tango Tree. The search can modify the current tree struct using the Tango operation. See search in Tango
   * reference.
   *
   * @param key The key to search for in the Tango Tree.
   * @return true if the key is found in the Tango Tree, false otherwise.
   */
  bool contains(int key);
};

#endif TANGOTREE_H