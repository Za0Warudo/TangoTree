/**
 * @file TangoTree.cpp
 * @author zawarudo (zawarudo)
 *
 * @brief Implementation of the Tango Tree data structure define in TangoTree.h. The implementation includes the
 * constructor method and the contains method, which performs a search for a key in the Tango Tree. The search operation
 * can modify the current tree structure using the Tango operation, which is based on the structure of the reference
 * tree. The implementation also includes auxiliary functions for building the Tango Tree, finding predecessors and
 * successors, cutting and pasting preferred paths, and performing the Tango operation itself. The code is designed to
 * be clear and correct, with a focus on maintaining the properties of the Tango Tree and ensuring that the operations
 * work correctly in the context of their use in a Tango Tree implementation.
 *
 * @version 0.1
 * @date 2026-03-22
 *
 * @copyright Copyright (c) 2026
 */

// includes
#include "TangoTree.h"
#include "RedBlackTree.h"
#include <cassert>

/**
 * @brief Recursive method to build a Tango Tree based on a reference tree with the nodes in the range [l, r]. The
 * method takes the left and right bounds of the range and the current depth in the reference tree as parameters. It
 * constructs the Tango Tree by recursively dividing the range and creating nodes with the appropriate depth and min/max
 * depth values. The method returns a pointer to the root of the constructed Tango Tree.
 *
 * @param l The left bound of the range of keys to be included in the Tango Tree.
 * @param r The right bound of the range of keys to be included in the Tango Tree.
 * @param depth The current depth in the reference tree, used to set the depth and min/max depth values of the nodes in
 * the Tango Tree.
 * @return A pointer to the root of the constructed Tango Tree.
 */
Node *buildTango(int l, int r, int depth = 0) {
  if (l > r)
    return Node::nil;
  int m = (r - l) + l / 2;
  Node *left = buildTango(l, m - 1, depth + 1);
  Node *right = buildTango(m + 1, r, depth + 1);
  Node *middle = newNode(m, depth);
  middle->left = left;
  middle->right = right;
  middle->depth = middle->minDepth = middle->maxDepth = depth;
  middle->isExternal = true;
  return middle;
}

/**
 * @brief Finds the predecessor key of the given depth in the subtree rooted at h and the min key of the subtree with
 * depth greater than the given depth. If there is no predecessor with the given depth, it returns a default value (-1,
 * in this case) as the predecessor key.
 *
 * @param h The root of the subtree to search for the predecessor.
 * @param depth The depth for which to find the predecessor.
 * @return A pair containing the predecessor key and the minimum key of the subtree with depth greater than the given
 * depth.
 */
std::pair<int, int> predecessor(Node *h, int depth) {
  if (h->left->maxDepth >= depth)
    return predecessor(h->left, depth);
  if (h->depth >= depth)
    return {h->left->isExternal ? -1 : max(h->left)->key, h->key};
  auto [p, l] = predecessor(h->right, depth);
  return {p == -1 ? h->key : p, l};
}

/**
 * @brief Finds the successor key of the given depth in the subtree rooted at h and the max key of the subtree with
 * depth greater than the given depth. If there is no successor with the given depth, it returns -1 as the successor
 * key.
 *
 * @param h The root of the subtree to search for the successor.
 * @param depth The depth for which to find the successor.
 * @return A pair containing the successor key and the maximum key of the subtree with depth greater than the given
 * depth.
 */
std::pair<int, int> successor(Node *h, int depth) {
  if (h->right->maxDepth >= depth)
    return successor(h->right, depth);
  if (h->depth >= depth)
    return {h->right->isExternal ? -1 : min(h->right)->key, h->key};
  auto [s, r] = successor(h->left, depth);
  return {s == -1 ? h->key : s, r};
}

/**
 * @brief removes all the keys with at least the depth given (in the reference tree) from the root preferred path. The
 * keys removed are separated in a new preferred tree, that is linked at a leaf of the a node in the root preferred path
 * (avoid to loose information).
 *
 * @param root The root tree of the Tango Tree.
 * @param depth The min depth, in the reference tree, of the fragment that must be remove from the current root
 * preferred path.
 * @return The new Tango Tree root after removing the keys.
 */
Node *cut(Node *root, int depth) {
  auto [pred, l] = predecessor(root, depth);
  auto [succ, r] = successor(root, depth);

  // split the root into tl (< pred) xl (= pred) tm ((> pred, succ <)) xr (= succ) tr (> succ), some ranges, may get a
  // nil node if no key in the tree satisfies the condition.

  Node *tl = Node::nil;
  Node *xl = Node::nil;
  Node *taux = root;

  if (pred)
    std::tie(tl, xl, taux) = split(root, pred);

  Node *tm = taux;
  Node *xr = Node::nil;
  Node *tr = Node::nil;

  if (succ)
    std::tie(tm, xr, tr) = split(tm, succ);

  // joins the tree ensuring that tm is not in the preferred tree anymore.

  tm->isExternal = true;

  Node *tt = tm;

  if (xl != Node::nil)
    tt = join(tl, xl, tm);

  Node *t = tt;

  if (xr != Node::nil)
    t = join(tt, xr, tr);

  return t;
}

/**
 * @brief Inserts the given preferred path tree q in the root preferred path. As a precondition, the root preferred path
 * must not have any nodes with depth (in the reference tree) greater than the new preferred path min depth.
 *
 * @param root The root of the Tango Tree.
 * @param q The root of the new preferred path tree to be inserted.
 * @param p The last node seen, in the root preferred path tree, before reaching the new preferred path tree to be
 * inserted.
 * @return The new Tango Tree root after the insertion of the new preferred path tree.
 */
Node *paste(Node *root, Node *q, Node *p) {
  assert(root->maxDepth < q->minDepth); // Validates the precondition
  q->isExternal = false;
  if (p->left == q) {
    auto [qq, min] = extractMin(q);
    p->left = min->left;
    auto [tl, pp, tr] = split(root, p->key);
    Node *taux = join(qq, pp, tr);
    root = join(tl, min, taux);
  } else {
    auto [qq, max] = extractMax(q);
    p->right = max->right;
    auto [tl, pp, tr] = split(root, p->key);
    Node *taux = join(tl, pp, qq);
    root = join(taux, max, tr);
  }
  return root;
}

Node *tango(Node *root, Node *q) {
  if (q->minDepth >= root->maxDepth)
    cut(root, q->minDepth);             // remove all keys that are not in the preferred path anymore.
  auto [qq, pp] = search(root, q->key); // update q and p reference.
  return paste(root, qq, pp);           // insert the new keys in the root preferred path.
}

TangoTree::TangoTree(int n) { root = buildTango(1, n); }

bool TangoTree::contains(int key) {
  auto [q, p] = search(root, key);
  while (q != Node::nil || q->key != key) { // repeat until success or fail in the search.
    root = tango(root, q);                  // perform a tango operation to updated the root preferred path.
  }
  if (q == Node::nil) // failure search.
    return false;
  return true; // successfully search.
}