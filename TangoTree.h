/**
 * @file TangoTree.h
 * @brief Defines the Tango Tree data structure.
 *
 * The file provides the declaration for the Tango Tree data structure, including the constructor and search operation
 *
 * @date October 25, 2025
 * @version 1.0.1
 * @author Za0Warudo
 * @note This implementation assumes integer keys for simplicity.
 */

#ifndef TANGOTREE_H
#define TANGOTREE_H

/* Includes */
#include "RedBlackTree.h"
#include <iostream>

// Colors variable (Terminal)
const std::string RESET = "\033[0m";
const std::string RED_COLOR = "\033[31m";


Node<int>* TangoBuild(int l, int r, int d);


/**
 * @brief Find the predecessor key of the h tree and d depth
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
int Predecessor(const Node<int>* h, int d);


/**
 * @brief Find the successor key of the h tree and d depth
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
int Successor(const Node<int>* h, int d);

/**
 * @brief Perform the tango operation for the tree h and q new tree
 *
 * Applies the tango operation to the tree h,
 * removing the old preferred path from three t and attaching the q tree path
 *
 * @param h the tree root node
 * @param q the new path tree root node
 * @param p the parent node of q
 * @return the root node of the tree after the tango operation
 */
Node<int>* Tango(Node<int>* h, Node<int> *q, Node<int> *p);

Node<int>* BuildTangoTree(const int n);

void ShowTangoRec(Node<int>* h, const int depth);

void ShowTango(Node<int>* root);

Node<int>* SearchTango(Node<int>* root, const int key);

Node<int>* TangoBuild(const int l, const int r, const int d);

#endif //TANGOTREE_H
