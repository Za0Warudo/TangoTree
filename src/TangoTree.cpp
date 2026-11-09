/**
 * @file TangoTree.cpp
 *
 * @author Zawarudo (@zawarudo)
 *
 * @brief Implements the tango tree interface defined in TangoTree.h. A search can restructure the tree using the tango operation, which cuts and pastes
 * preferred paths based on the structure of the reference tree. This implementation is designed to be clear and correct, with each preferred path stored in a
 * red-black auxiliary tree.
 *
 * @version 2.1
 * @date 2026-03-22
 * @copyright Copyright (c) 2026
 */

// includes.
#include "TangoTree.h"
#include "RedBlackTree.h"
#include <cassert>
#include <iostream>

void showRec(Node *root, int indent = 0);

/* Auxiliary functions. */

/**
 * @brief Recursively builds a tango tree with the keys in the range [l, r], following the shape of a complete reference tree. If the range is empty, returns the
 * nil node. Otherwise, chooses the middle key that keeps the reference tree complete and builds the left and right subtrees around it.
 *
 * @param l The left bound of the range of keys.
 * @param r The right bound of the range of keys.
 * @param depth The depth, in the reference tree, of the root of this range.
 * @return The root node of the built tree.
 *
 * @note Every node is built as a black, external node, so each one starts as its own auxiliary tree.
 * @note Time Complexity: O(N), where N is the number of keys in the range.
 */
Node *buildTango(int l, int r, int depth = 0) {
  if (l > r)
    return Node::nil;

  int n = r - l + 1;

  // Calculate the greatest power of 2 less than or equal to n: p = 2^(floor(log2(n))).
  int p = 1;
  while (p * 2 <= n)
    p <<= 1;
  int s = n - (p - 1);
  int m;

  if (s > p / 2) {
    m = l + p - 1;
  } else {
    m = l + (p / 2) - 1 + s;
  }

  Node *left = buildTango(l, m - 1, depth + 1);
  Node *right = buildTango(m + 1, r, depth + 1);
  Node *middle = newNode(m, depth);
  middle->left = left;
  middle->right = right;
  middle->color = BLACK;
  middle->depth = depth;
  middle->minDepth = depth;
  middle->maxDepth = depth;
  middle->isExternal = true;
  middle->blackHeight = -1;
  return middle;
}

/**
 * @brief Finds the predecessor, in the given auxiliary subtree, of the smallest key with reference tree depth at least the given depth.
 *
 * @param h The subtree root node.
 * @param depth The minimum reference tree depth.
 * @return The predecessor key, or -1 if there is no smaller key in the subtree.
 *
 * @note Precondition: The subtree must contain a node with reference tree depth at least the given depth.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the subtree.
 */
int predecessor(Node *h, int depth) {
  if (!h->left->isExternal && h->left->maxDepth >= depth)
    return predecessor(h->left, depth);
  if (h->depth >= depth)
    return h->left->isExternal ? -1 : max(h->left)->key;
  int p = predecessor(h->right, depth);
  return p == -1 ? h->key : p;
}

/**
 * @brief Finds the successor, in the given auxiliary subtree, of the largest key with reference tree depth at least the given depth.
 *
 * @param h The subtree root node.
 * @param depth The minimum reference tree depth.
 * @return The successor key, or -1 if there is no greater key in the subtree.
 *
 * @note Precondition: The subtree must contain a node with reference tree depth at least the given depth.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the subtree.
 */
int successor(Node *h, int depth) {
  if (!h->right->isExternal && h->right->maxDepth >= depth)
    return successor(h->right, depth);
  if (h->depth >= depth)
    return h->right->isExternal ? -1 : min(h->right)->key;
  int s = successor(h->left, depth);
  return s == -1 ? h->key : s;
}

/**
 * @brief Cuts the nodes with reference tree depth at least the given depth from the root preferred path. The removed nodes form a new auxiliary tree, which stays
 * linked below the root preferred path as an external node, so no keys are lost.
 *
 * @param root The tango tree root node.
 * @param depth The minimum reference tree depth of the nodes to remove.
 * @return The new tango tree root after removing the nodes.
 *
 * @note Precondition: The root preferred path must contain a node with reference tree depth at least the given depth.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the root preferred path.
 */
Node *cut(Node *root, int depth) {
  int pred = predecessor(root, depth);
  int succ = successor(root, depth);

  // Split the root into tl (< pred), xl (= pred), tm (> pred and < succ), xr (= succ), and tr (> succ). Some of them may be the nil node, if no key in the tree
  // satisfies the condition.

  Node *tl = Node::nil;
  Node *xl = Node::nil;
  Node *taux = root;

  if (pred > -1)
    std::tie(tl, xl, taux) = split(root, pred);

  Node *tm = taux;
  Node *xr = Node::nil;
  Node *tr = Node::nil;

  if (succ > -1)
    std::tie(tm, xr, tr) = split(tm, succ);

  // Mark tm as external and join the trees, ensuring that tm is not in the root preferred path anymore.

  tm->isExternal = true;
  tm->blackHeight = -1;

  Node *tt = tm;

  if (xl != Node::nil)
    tt = join(tl, xl, tm);

  Node *t = tt;

  if (xr != Node::nil)
    t = join(tt, xr, tr);

  return t;
}

/**
 * @brief Pastes the given auxiliary tree into the root preferred path.
 *
 * @param root The tango tree root node.
 * @param q The root node of the auxiliary tree to insert.
 * @param p The parent of q, which is the last node seen in the root preferred path before reaching q.
 * @return The new tango tree root after inserting the auxiliary tree.
 *
 * @note Precondition: All the reference tree depths in the root preferred path must be smaller than the minimum reference tree depth in q.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the root preferred path and in q.
 */
Node *paste(Node *root, Node *q, Node *p) {
  q->isExternal = false;
  update(q);     // Update the fields of q, since it is not an external node anymore.
  if (p->left == q) {
    auto [min, qq] = deleteMin(q);
    p->left = min->left;
    auto [tl, pp, tr] = split(root, p->key);
    Node *taux = join(qq, pp, tr);
    root = join(tl, min, taux);
  } else {
    auto [max, qq] = deleteMax(q);
    p->right = max->right;
    auto [tl, pp, tr] = split(root, p->key);
    Node *taux = join(tl, pp, qq);
    root = join(taux, max, tr);
  }
  return root;
}

/**
 * @brief Performs the tango operation, the core of the tango tree. Cuts the nodes that are not in the new root preferred path anymore, if there are any, and
 * pastes the given auxiliary tree into the root preferred path.
 *
 * @param root The tango tree root node.
 * @param q The root node of the auxiliary tree to insert into the root preferred path.
 * @return The new tango tree root after the tango operation.
 *
 * @note Precondition: q must be an external node directly below the root preferred path.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the tree.
 */
Node *tango(Node *root, Node *q) {
  if (root->maxDepth >= q->minDepth) {
    root = cut(root, q->minDepth);     // Remove all the keys that are not in the preferred path anymore.
  }
  Node *qq = search(root, q->key);
  Node *pp = parentSearch(root, q->key);     // Find the last node seen in the root preferred path before reaching q.
  root = paste(root, qq, pp);                // Insert the new keys into the root preferred path.
  return root;
}

/* Debug functions. */

/**
 * @brief Recursively prints the given subtree sideways, with the right subtree on top. Each node is printed with its key, whether it is external (E) or internal
 * (I), its black height, and its color.
 *
 * @param root The subtree root node.
 * @param indent The indentation of the current node, in spaces.
 *
 * @note Time Complexity: O(N), where N is the number of nodes in the subtree, including the nodes of the auxiliary trees below it.
 */
void showRec(Node *root, int indent) {
  if (root == Node::nil)
    return;
  showRec(root->right, indent + 4);
  std::cout << std::string(indent, ' ');
  std::cout << '(' << root->key << ',' << (root->isExternal ? "E" : "I") << ", " << root->blackHeight << ", " << (root->color == RED ? "RED" : "BLACK")
            << ")\n";
  showRec(root->left, indent + 4);
}

/* Tango tree operations. */

/* Constructor. */
TangoTree::TangoTree(int n) {
  root = buildTango(1, n);
  root->isExternal = false;
  root->blackHeight = 0;
  size = n;
}

/* Get Size. */
int TangoTree::getSize() { return size; }

/* Show. */
void TangoTree::show() { showRec(root); }

/* Search. */
Node *TangoTree::search(int key) {
  Node *q = ::search(root, key);
  while (q->isExternal && q != Node::nil) {     // Repeat until the search succeeds or fails.
    root = tango(root, q);                      // Perform a tango operation to update the root preferred path.
    q = ::search(root, key);                    // Update the q reference for the next iteration.
  }

  return q;
}

/* Contains. */
bool TangoTree::contains(int key) { return search(key) != Node::nil; }