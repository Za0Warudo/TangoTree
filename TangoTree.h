/**
 * @file TangoTree.h
 * @brief Defines the Tango Tree data structure.
 *
 * The file provides the declaration for the Tango Tree data structure, including the constructor and search operation
 *
 * @date October 2025
 * @version 1.0.1
 * @author Za0Warudo
 * @note This implementation assumes integer keys for simplicity.
 */

#ifndef TANGOTREE_H
#define TANGOTREE_H

/* Includes */
#include "RedBlackTree.h"

// Colors variable (Terminal)
const std::string RESET = "\033[0m";
const std::string RED_COLOR = "\033[31m";

// DEBUG variable
constexpr bool DEBUG = false;

/* -------------------------------------------------- */

/* Build functions */

/**
 * @brief Builds a tango tree with keys in [1, n]
 *
 * The only node in the initial preferred path is the root node.
 *
 * @param n The last key in the tree
 * @return Tango tree with keys in [1, n]
 */
Node<int>* TangoBuild(int n);


/**
 * @brief Recursive method to build the tango tree
 *
 * @param l Min key in the range
 * @param r Max key in the range
 * @param d Current depth
 * @return The subtree root
 */
Node<int>* TangoBuildRec(int l, int r, int d);

/* -------------------------------------------------- */

/* Search functions */

/**
 * @brief Find the smallest key in the tree with depth at least d, and it's predecessor
 *
 * Finds, if the tree contains one, the predecessor key of the h tree and d depth.
 *
 * The predecessor is defined as the greatest key smaller than the smallest key that has depth greater than d.
 *
 * If the tree do not contain a predecessor, the function returns -1.
 *
 * @pre The h tree must contain at least a node that has depth greater than d.
 *
 * @param h Tree root node
 * @param d Cut depth
 * @return The predecessor key
 */
std::pair<int, int> Predecessor(const Node<int> *h, int d);

/**
 * @brief Recursive method to obtain the l and l' of d depth in the tree h
 *
 * @param h the subtree root node
 * @param d the cut depth
 * @return the predecessor key
 */
std::pair<int, int> PredecessorRec(const Node<int>* h, int d);


/**
 * @brief Find the greatest key in the tree with depth at least d, and it's successor
 *
 * Finds, if the tree contains one, the successor key of the h tree and d depth.
 *
 * The successor is defined as the smallest key greater than the greatest key that has depth greater than d.
 *
 * If the tree do not contain a successor, the function returns -1.
 *
 * @pre The h tree must contain at least a node that has depth greater than d.
 *
 * @param h Tree root node
 * @param d Cut depth
 * @return The successor key
 */
std::pair<int, int> Successor(const Node<int>* h, int d);

/**
 * @brief Recursive method to obtain r and r' of d depth in the h tree
 * @param h the subtree root node
 * @param d the cut depth
 * @return the sucessor key
 */
std::pair<int, int> SuccessorRec(const Node<int> *h, int d);


/**
 * @brief Search the key k in the tango tree given
 *
 * @param root The tree root
 * @param k The key to be search
 * @return The new tree root after the search
 */
Node<int>* SearchTango(Node<int>* root, int k);

/**
 * @brief Perform the tango operation for the tree h and q new tree
 *
 * Applies the tango operation to the tree h,
 * removing the old preferred path from three t and attaching the q tree path
 *
 * @param h The tree root node
 * @param q The new path tree root node
 * @param p The parent node of q
 * @return The root node of the tree after the tango operation
 */
Node<int>* Tango(Node<int>* h, Node<int>* q, Node<int>* p);

/* -------------------------------------------------- */

/* Show functions */

/**
 * @brief Recursive method to build a tango tree
 *
 * @param h Current subtree node
 * @param d Current subtree depth
 */
void ShowTangoRec(Node<int>* h, int d);

/**
 * @brief Prints the tango tree given
 *
 * Show preferred every node in a preferred path in red.
 *
 * @param root Tree root node
 */
void ShowTango(Node<int>* root);

/* -------------------------------------------------- */

#endif //TANGOTREE_H