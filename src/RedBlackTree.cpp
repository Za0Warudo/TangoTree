/**
 * @file RedBlackTree.cpp
 *
 * @author Zawarudo (@zawarudo)
 *
 * @brief Implements the red-black tree interface defined in RedBlackTree.h. This implementation is designed to be clear and easy to understand in the context
 * of the tango tree auxiliary tree.
 *
 * @version 2.0
 * @date 2026-03-14
 * @copyright Copyright (c) 2026
 */

// includes.
#include "RedBlackTree.h"
#include <iostream>

/**
 * @brief Defines the nil node. The nil node is a special node that represents the absence of a child in the red-black tree. It is used as a sentinel pointer.
 *
 * @note The nil node is always black and external. Its key is not used (default -1 for this case). The left and right pointers of the nil node point to itself,
 * creating a cycle. The depth and black height of the nil node are set to -1. The max and min depth are set to SHORT_MIN and SHORT_MAX respectively.
 */
Node *Node::nil = [] {
  Node *n = new Node(-1);
  n->color = BLACK;
  n->left = n->right = n;
  n->isExternal = true;
  n->depth = -1;
  n->minDepth = SHORT_MAX;
  n->maxDepth = SHORT_MIN;
  n->blackHeight = -1;

  return n;
}();     // Initialize the nil node.

/* Headers of auxiliary functions. */

Node *balance(Node *h);
Node *rotateLeft(Node *h);
Node *rotateRight(Node *h);
Node *moveRedLeft(Node *h);
Node *moveRedRight(Node *h);
Node *cleanUp(Node *h);

/* NewNode. */

Node *newNode(int key, int depth) {
  Node *x = new Node(key);
  x->left = x->right = Node::nil;
  x->isExternal = false;
  x->blackHeight = 0;
  x->depth = x->minDepth = x->maxDepth = depth;
  return x;
}

/* Search. */

/**
 * @brief Recursively searches for a key in the given subtree. If an external node is reached or the node with the given key is found, returns it. If the key
 * is smaller than the current node's key, searches the left subtree. If it is greater, searches the right subtree.
 *
 * @param h The subtree root node.
 * @param key The search key.
 * @return A pointer to the node if found, or an external node otherwise.
 *
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the subtree.
 */
Node *searchRec(Node *h, int key) {
  if (h->isExternal || h->key == key)
    return h;
  if (key < h->key)
    return searchRec(h->left, key);
  else
    return searchRec(h->right, key);
}

Node *search(Node *h, int key) { return searchRec(h, key); }

/* ParentSearch. */

/**
 * @brief Recursively searches for the parent of the node with the given key in the given subtree. If an external node is reached or the node with the given
 * key is found, returns the parent of the current node.
 *
 * @param h The subtree root node.
 * @param key The search key.
 * @param parent The parent node of the current node.
 * @return A pointer to the parent node of the node with the given key if found, or the last visited node otherwise.
 *
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the subtree.
 */
Node *parentSearchRec(Node *h, int key, Node *parent = Node::nil) {
  if (h->isExternal || h->key == key)
    return parent;
  if (key < h->key)
    return parentSearchRec(h->left, key, h);
  else
    return parentSearchRec(h->right, key, h);
}

Node *parentSearch(Node *h, int key) { return parentSearchRec(h, key); }

/* Insert. */

/**
 * @brief Recursively inserts a key into the given subtree. If an external node is reached, creates and returns a new node with the given key. If the key is
 * smaller than the current node's key, inserts into the left subtree. If it is greater, inserts into the right subtree. If it is equal, leaves the subtree
 * unchanged. After inserting, balances the subtree to maintain the properties of the red-black tree.
 *
 * @param h The subtree root node.
 * @param key The key to insert into the subtree.
 * @return The new root of the subtree after inserting the key.
 *
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the subtree.
 */
Node *insertRec(Node *h, int key) {
  if (h->isExternal)
    return newNode(key);
  if (key < h->key)
    h->left = insertRec(h->left, key);
  else if (key > h->key)
    h->right = insertRec(h->right, key);
  else
    return h;
  return balance(h);
}

Node *insert(Node *root, int key) {
  root = insertRec(root, key);
  root->color = BLACK;     // Ensure that the tree root is always black after insertion.
  return root;
}

/* Join. */

/**
 * @brief Recursively joins two red-black trees and a node. If the left tree has a greater black height, applies the join on the right subtree of the left
 * tree. If the right tree has a greater black height, applies the join on the left subtree of the right tree. If both trees have the same black height, joins
 * them directly with x as the root node.
 *
 * @param leftTree The left tree root node.
 * @param x The middle node.
 * @param rightTree The right tree root node.
 * @return The root of the new tree formed by joining the left tree, the node, and the right tree.
 *
 * @note Precondition: All the keys in the left tree must be smaller than the key in the given node. All the keys in the right tree must be greater than the key
 * in the given node.
 * @note Time Complexity: O(|h1 - h2|), where h1 is the height of the left tree and h2 is the height of the right tree.
 */
Node *joinRec(Node *leftTree, Node *x, Node *rightTree) {
  if (leftTree->blackHeight > rightTree->blackHeight) {
    leftTree->right = joinRec(leftTree->right, x, rightTree);
    return balance(leftTree);
  }
  if (leftTree->blackHeight < rightTree->blackHeight) {
    rightTree->left = joinRec(leftTree, x, rightTree->left);
    return balance(rightTree);
  }
  x->left = leftTree;
  x->right = rightTree;
  x->color = RED;
  return balance(x);
}

Node *join(Node *leftTree, Node *x, Node *rightTree) {
  Node *y = joinRec(leftTree, x, rightTree);
  y->color = BLACK;     // Ensure the new root of the joined tree is black.

  return y;
}

/* Split. */

/**
 * @brief Recursively splits the given subtree around a key. If the key is smaller than the current node's key, applies the split on the left subtree. If the
 * key is greater than the current node's key, applies the split on the right subtree. If the key matches the current node's key, returns its left and right
 * subtrees, with the current node as the separator.
 *
 * @param h The subtree root node.
 * @param key The splitter key.
 * @return A tuple containing the left tree (keys smaller than the given key), the node with the given key, and the right tree (keys greater than the given
 * key).
 *
 * @note The returned separator node keeps its child pointers. The split function clears them afterwards.
 * @note Precondition: The key must exist in the subtree before calling this function.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the subtree.
 */
std::tuple<Node *, Node *, Node *> splitRec(Node *h, int key) {
  if (key < h->key) {
    auto [left, x, right] = splitRec(h->left, key);
    return {left, x, join(right, h, h->right)};
  }
  if (key > h->key) {
    auto [left, x, right] = splitRec(h->right, key);
    h->left->color = BLACK;     // Ensure the left child of h is black before joining.
    return {join(h->left, h, left), x, right};
  }

  Node *left = h->left;
  Node *right = h->right;
  left->color = BLACK;     // Ensure that the left subtree root is black.
  return {left, h, right};
}

std::tuple<Node *, Node *, Node *> split(Node *h, int key) {
  auto [l, x, r] = splitRec(h, key);
  return {l, cleanUp(x), r};
}

/* DeleteMin. */

/**
 * @brief Recursively deletes the node with the minimum key in the given subtree. If the left child of h is external, then h is the minimum node, so returns it
 * with its right child as the new subtree root. If the left child of h is black and its left child is also black, moves a red link left to ensure that the
 * descent follows a path with a red link, which maintains the balance property.
 *
 * @param h The subtree root node.
 * @return The minimum node and the new subtree root after deleting the minimum node.
 *
 * @note Precondition: The subtree must be non-empty.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the subtree.
 */
std::pair<Node *, Node *> deleteMinRec(Node *h) {
  if (h->left->isExternal)
    return {h, h->right};     // If the left child of h is external, then h is the minimum node, so we return its right child as the new subtree root.
  if (h->left->color == BLACK && h->left->left->color == BLACK)
    h = moveRedLeft(h);
  auto [min, x] = deleteMinRec(h->left);
  h->left = x;
  return {min, balance(h)};
}

std::pair<Node *, Node *> deleteMin(Node *root) {
  if (root->left->color == BLACK)
    root->color = RED;     // Ensure that the root or its left child is red before deleting the minimum.
  auto [min, h] = deleteMinRec(root);
  h->color = BLACK;
  min->color = BLACK;
  return {min, h};
}

/* DeleteMax. */

/**
 * @brief Recursively deletes the node with the maximum key in the given subtree. If the left child of h is red, rotates right. If the right child of h is then
 * external, h is the maximum node, so returns it with its left child as the new subtree root. If the right child of h is black and its left child is also
 * black, moves a red link right to ensure that the descent follows a path with a red link, which maintains the balance property.
 *
 * @param h The subtree root node.
 * @return The maximum node and the new subtree root after deleting the maximum node.
 *
 * @note Precondition: The subtree must be non-empty.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the subtree.
 */
std::pair<Node *, Node *> deleteMaxRec(Node *h) {
  if (!h->left->isExternal && h->left->color == RED)
    h = rotateRight(h);
  if (h->right->isExternal)
    return {h, h->left};     // If the right child of h is external, then h is the maximum node, so we return its left child as the new subtree root.
  if (h->right->color == BLACK && h->right->left->color == BLACK)
    h = moveRedRight(h);
  auto [max, x] = deleteMaxRec(h->right);
  h->right = x;
  return {max, balance(h)};
}

std::pair<Node *, Node *> deleteMax(Node *root) {
  if (root->left->color == BLACK)
    root->color = RED;
  auto [max, h] = deleteMaxRec(root);
  h->color = BLACK;
  max->color = BLACK;
  return {max, h};
}

/* Min. */

Node *min(Node *root) {
  while (!root->left->isExternal)
    root = root->left;

  return root;
}

/* Max. */

Node *max(Node *root) {
  while (!root->right->isExternal)
    root = root->right;

  return root;
}

/* Keys. */

/**
 * @brief Recursively collects the keys of the given subtree with an in-order traversal. The keys are stored in the result vector.
 *
 * @param root The subtree root node.
 * @param result The vector to store the collected keys.
 *
 * @note Time Complexity: O(N), where N is the number of nodes in the subtree.
 */
void keysRec(Node *root, std::vector<int> &result) {
  if (root != Node::nil) {
    keysRec(root->left, result);
    result.push_back(root->key);
    keysRec(root->right, result);
  }
}

std::vector<int> keys(Node *root) {
  std::vector<int> result;
  keysRec(root, result);
  return result;
}

/* Print. */

/**
 * @brief Recursively prints the given subtree sideways, with the right subtree on top.
 *
 * @param root The subtree root node.
 * @param indent The indentation of the current node, in spaces.
 * @param step The indentation added at each level, in spaces.
 *
 * @note Time Complexity: O(N), where N is the number of nodes in the subtree.
 */
void printRec(Node *root, int indent, int step) {
  if (root != Node::nil) {
    printRec(root->right, indent + step, step);
    std::cout << std::string(indent, ' ') << '(' << root->key << ')' << "\n";
    printRec(root->left, indent + step, step);
  }
}

void print(Node *root) { return printRec(root, 0, 4); }

/* Update. */

void update(Node *h) {

  if (h->isExternal) {
    h->blackHeight = -1;
    h->minDepth = SHORT_MAX;
    h->maxDepth = SHORT_MIN;
  } else {
    h->blackHeight = std::max(h->left->blackHeight + (h->left->color == BLACK ? 1 : 0), h->right->blackHeight + 1);
    h->minDepth = std::min(std::min(h->left->isExternal ? SHORT_MAX : h->left->minDepth, h->right->isExternal ? SHORT_MAX : h->right->minDepth), h->depth);
    h->maxDepth = std::max(std::max(h->left->isExternal ? SHORT_MIN : h->left->maxDepth, h->right->isExternal ? SHORT_MIN : h->right->maxDepth), h->depth);
  }
}

/* Auxiliary functions definitions. */

/**
 * @brief Rotates a subtree to the left. This operation is used to maintain the balance of the red-black tree.
 *
 * @param h The subtree root node.
 * @return The new subtree root after the rotation.
 *
 * @note Precondition: The right child of h must be red.
 * @note Time Complexity: O(1).
 */
Node *rotateLeft(Node *h) {
  Node *x = h->right;
  h->right = x->left;
  x->left = h;
  x->color = h->color;
  h->color = RED;
  update(h);     // Update the properties of h after rotation.
  update(x);     // Update the properties of x after rotation.
  return x;
}

/**
 * @brief Rotates a subtree to the right. This operation is used to maintain the balance of the red-black tree.
 *
 * @param h The subtree root node.
 * @return The new subtree root after the rotation.
 *
 * @note Precondition: The left child of h must be red.
 * @note Time Complexity: O(1).
 */
Node *rotateRight(Node *h) {
  Node *x = h->left;
  h->left = x->right;
  x->right = h;
  x->color = h->color;
  h->color = RED;
  update(h);     // Update the properties of h after rotation.
  update(x);     // Update the properties of x after rotation.
  return x;
}

/**
 * @brief Flips the colors of the given node and its children. This operation is used to maintain the balance property of the red-black tree.
 *
 * @param h The node.
 * @return The node after flipping the colors.
 *
 * @note Precondition: h must not be the nil node, and its color must be different from its children's colors.
 * @note Time Complexity: O(1).
 */
Node *flipColors(Node *h) {
  Color c = h->color;
  h->color = h->left->color;
  h->left->color = h->right->color = c;
  return h;
}

/**
 * @brief Balances the given subtree. This function is called to fix local violations of the red-black tree properties.
 *
 * @param h The subtree root node.
 * @return The new subtree root after the balance.
 *
 * @note Time Complexity: O(1).
 */
Node *balance(Node *h) {
  if (h->right->color == RED)
    h = rotateLeft(h);     // Fix right-leaning red link.
  if (h->left->color == RED && h->left->left->color == RED)
    h = rotateRight(h);     // Fix two reds in a row on the left.
  if (h->left->color == RED && h->right->color == RED)
    h = flipColors(h);     // Split 4-node.

  update(h);
  return h;
}

/**
 * @brief Moves a red link to the left. This operation is used during deletion to ensure that the properties of the red-black tree are maintained.
 *
 * @param h The subtree root node.
 * @return The new subtree root after moving the red link left.
 *
 * @note Precondition: h must not be the nil node.
 * @note Time Complexity: O(1).
 */
Node *moveRedLeft(Node *h) {
  h = flipColors(h);
  if (h->right->left->color == RED) {
    h->right = rotateRight(h->right);
    h = rotateLeft(h);
    h = flipColors(h);
  }
  return h;
}

/**
 * @brief Moves a red link to the right. This operation is used during deletion to ensure that the properties of the red-black tree are maintained.
 *
 * @param h The subtree root node.
 * @return The new subtree root after moving the red link right.
 *
 * @note Precondition: h must not be the nil node.
 * @note Time Complexity: O(1).
 */
Node *moveRedRight(Node *h) {
  h = flipColors(h);
  if (h->left->left->color == RED) {
    h = rotateRight(h);
    h = flipColors(h);
  }
  return h;
}

/**
 * @brief Sets the children of the given node to the nil node, sets its color to black, and recomputes its fields. External nodes are left unchanged.
 *
 * @param h The node.
 * @return The node after the clean up.
 *
 * @note Time Complexity: O(1).
 */
Node *cleanUp(Node *h) {
  if (!h->isExternal) {
    h->left = Node::nil;
    h->right = Node::nil;
    h->color = BLACK;
    update(h);
  }
  return h;
}