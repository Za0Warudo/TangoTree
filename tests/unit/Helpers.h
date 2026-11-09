#ifndef HELPERS_H
#define HELPERS_H

/**
 * @file Helpers.h
 *
 * @author Zawarudo (@zawarudo)
 *
 * @brief Helpers shared by the red-black tree and tango tree unit tests, to build test data and to traverse the trees.
 */

// includes.
#include "RedBlackTree.h"
#include "TangoTree.h"
#include <algorithm>
#include <numeric>
#include <random>
#include <vector>

/* Red-black tree helpers. */

/**
 * @brief Builds a tree by inserting the given keys in order.
 *
 * @param treeKeys The keys to insert.
 * @return The tree root.
 */
inline Node *buildTree(const std::vector<int> &treeKeys) {
  Node *tree = Node::nil;
  for (int key : treeKeys)
    tree = insert(tree, key);
  return tree;
}

/**
 * @brief Returns the keys 0 to n - 1 in a random order. The fixed seed keeps failures reproducible.
 *
 * @param n The number of keys.
 * @param seed The random generator seed.
 * @return The shuffled keys.
 */
inline std::vector<int> shuffledRange(int n, unsigned seed = 12345) {
  std::vector<int> result(n);
  std::iota(result.begin(), result.end(), 0);
  std::shuffle(result.begin(), result.end(), std::mt19937(seed));
  return result;
}

/* Tango tree helpers. */

/**
 * @brief Returns the keys 1 to n in a random order, reproducible by the given seed.
 *
 * @param n The number of keys.
 * @param seed The random generator seed.
 * @return The shuffled keys.
 */
inline std::vector<int> generateRandomKeys(int n, unsigned seed) {
  std::vector<int> keys(n);
  std::iota(keys.begin(), keys.end(), 1);
  std::mt19937 generator(seed);
  std::shuffle(keys.begin(), keys.end(), generator);
  return keys;
}

/**
 * @brief Collects, in-order, the nodes of an auxiliary subtree, stopping at the external nodes.
 *
 * @param h The subtree root node.
 * @param nodes The vector to store the collected nodes.
 */
inline void collectAuxNodes(Node *h, std::vector<Node *> &nodes) {
  if (h->isExternal)
    return;
  collectAuxNodes(h->left, nodes);
  nodes.push_back(h);
  collectAuxNodes(h->right, nodes);
}

/**
 * @brief Returns, in-order, the nodes of the given auxiliary tree. The root is always included, since the root of an
 * auxiliary tree is itself marked as external, unless it is the tango tree root.
 *
 * @param auxRoot The auxiliary tree root node.
 * @return The nodes of the auxiliary tree.
 */
inline std::vector<Node *> auxNodes(Node *auxRoot) {
  std::vector<Node *> nodes;
  collectAuxNodes(auxRoot->left, nodes);
  nodes.push_back(auxRoot);
  collectAuxNodes(auxRoot->right, nodes);
  return nodes;
}

/**
 * @brief Recursively collects, in-order, the root of every auxiliary tree in the given subtree: the external nodes and
 * the tango tree root.
 *
 * @param h The subtree root node.
 * @param tangoRoot The tango tree root node.
 * @param roots The vector to store the collected roots.
 */
inline void collectAuxRoots(Node *h, Node *tangoRoot, std::vector<Node *> &roots) {
  if (h == Node::nil)
    return;
  collectAuxRoots(h->left, tangoRoot, roots);
  if (h->isExternal || h == tangoRoot)
    roots.push_back(h);
  collectAuxRoots(h->right, tangoRoot, roots);
}

/**
 * @brief Returns, in-order, the root of every auxiliary tree in the given tango tree.
 *
 * @param tree The tango tree.
 * @return The auxiliary tree roots.
 */
inline std::vector<Node *> getAllAuxiliary(TangoTree &tree) {
  std::vector<Node *> roots;
  collectAuxRoots(tree.root, tree.root, roots);
  return roots;
}

/**
 * @brief The shape of the reference tree, captured from a newly built tango tree. Indexed by key.
 */
struct ReferenceTree {
  int n;
  std::vector<int> depth;       // The depth of each key.
  std::vector<int> parent;      // The parent key of each key, or 0 at the root.
  std::vector<int> byDepth;     // The keys, sorted by depth.

  /**
   * @brief Returns the keys in the path from the given key up to the reference tree root, including both.
   */
  std::vector<int> pathTo(int key) const {
    std::vector<int> path;
    for (int current = key; current != 0; current = parent.at(current))
      path.push_back(current);
    return path;
  }
};

/**
 * @brief Captures the reference tree of a newly built tango tree. Since a BST is determined by its keys and their
 * depths, inserting the keys in depth order rebuilds the same shape.
 *
 * @param tree The newly built tango tree.
 * @param n The number of keys in the tree.
 * @return The reference tree.
 */
inline ReferenceTree captureReference(TangoTree &tree, int n) {
  ReferenceTree ref{n, std::vector<int>(n + 1, -1), std::vector<int>(n + 1, 0), {}};
  for (Node *auxRoot : getAllAuxiliary(tree)) {
    for (Node *node : auxNodes(auxRoot)) {
      ref.depth.at(node->key) = node->depth;
      ref.byDepth.push_back(node->key);
    }
  }
  std::stable_sort(ref.byDepth.begin(), ref.byDepth.end(), [&](int a, int b) { return ref.depth.at(a) < ref.depth.at(b); });

  std::vector<int> left(n + 1, 0);
  std::vector<int> right(n + 1, 0);
  for (size_t i = 1; i < ref.byDepth.size(); ++i) {
    const int key = ref.byDepth[i];
    int current = ref.byDepth[0];     // simulate the BST insertion process to find the parent of the current key
    while (true) {
      int &next = key < current ? left.at(current) : right.at(current);
      if (next == 0) {
        next = key;
        ref.parent.at(key) = current;
        break;
      }
      current = next;
    }
  }
  return ref;
}

#endif     // HELPERS_H
