#ifndef CHECKERS_H
#define CHECKERS_H

/**
 * @file Checkers.h
 *
 * @author Zawarudo (@zawarudo)
 *
 * @brief Checkers for the red-black tree and tango tree invariants, shared by both unit tests. The given root is always
 * checked, even when it is external, so the root of an auxiliary tree can be passed directly. External nodes below the
 * given root are treated as nil leaves, so a check never crosses into another auxiliary tree.
 */

// includes.
#include "Helpers.h"
#include "RedBlackTree.h"
#include "TangoTree.h"
#include <algorithm>
#include <climits>
#include <gtest/gtest.h>
#include <numeric>
#include <set>
#include <vector>

/* Red-black tree checkers. */

/**
 * @brief Constant to represent an invalid black height in the red-black tree.
 */
const int INVALID_BLACK_HEIGHT = -2;

/**
 * @brief Check if the given RBT is a BST.
 * Ensures that for every node:
 * - All keys in the left subtree are strictly less than the node's key.
 * - All keys in the right subtree are strictly greater than the node's key.
 *
 * @param root The tree root.
 * @param min The exclusive lower bound for the current node's key.
 * @param max The exclusive upper bound for the current node's key.
 * @return True, if the subtree is a valid BST; false, otherwise.
 * @note Time Complexity: O(N), where N is the number of nodes in the tree.
 */
inline bool isBST(Node *root, long long min = LLONG_MIN, long long max = LLONG_MAX) {
  if (root == Node::nil)
    return true;
  if (root->key <= min || root->key >= max)
    return false;
  return (root->left->isExternal || isBST(root->left, min, root->key)) && (root->right->isExternal || isBST(root->right, root->key, max));
}

/**
 * @brief Counts the black height of a subtree from its nodes.
 *
 * @param root The subtree root.
 * @return The black height of the subtree, disconsidering the root itself, or INVALID_BLACK_HEIGHT if two paths have
 * different black heights or a cached blackHeight field doesn't match the counted value.
 * @note The cached blackHeight of an external root is not compared, since it is always -1.
 * @note Time Complexity: O(N), where N is the number of nodes in the subtree.
 */
inline int countBlackHeight(Node *root) {
  if (root == Node::nil)
    return -1;
  const int left = root->left->isExternal ? -1 : countBlackHeight(root->left);
  const int right = root->right->isExternal ? -1 : countBlackHeight(root->right);
  if (left == INVALID_BLACK_HEIGHT || right == INVALID_BLACK_HEIGHT)
    return INVALID_BLACK_HEIGHT;
  const int leftPath = left + (root->left->color == BLACK ? 1 : 0);
  const int rightPath = right + (root->right->color == BLACK ? 1 : 0);
  if (leftPath != rightPath || (!root->isExternal && root->blackHeight != leftPath))
    return INVALID_BLACK_HEIGHT;
  return leftPath;
}

/**
 * @brief Check if the given RBT is a balanced tree.
 * A subtree is balanced if every path from the current root, disconsidering the node itself,
 * to NIL contains the same number of black nodes.
 *
 * @param root The tree root.
 * @return True, if the black height is consistent across all paths; false, otherwise.
 * @note Time Complexity: O(N), where N is the number of nodes in the subtree.
 */
inline bool isBalanced(Node *root) { return countBlackHeight(root) != INVALID_BLACK_HEIGHT; }

/**
 * @brief Check the 2-3 tree structural invariants in the given RBT.
 * A valid LLRBT tree must satisfy two conditions:
 * - No right-leaning red links (all red links must be left children).
 * - No two consecutive red links in any path (no "4-node" violations).
 *
 * @param root The tree root.
 * @return True, if the subtree satisfies all LLRBT properties; false, otherwise.
 * @note Time Complexity: O(N), where N is the total number of nodes in the subtree.
 */
inline bool is23(Node *root) {
  if (root == Node::nil)
    return true;
  const bool rootRed = root->color == RED;
  const bool leftRed = root->left->color == RED;
  const bool rightRed = root->right->color == RED;
  if ((rootRed && leftRed) || rightRed)
    return false;
  return (root->left->isExternal || is23(root->left)) && (root->right->isExternal || is23(root->right));
}

/**
 * @brief Performs a 4-check validation of the RBT invariants into the given tree.
 * The composite checks verifies the following structural and logical properties:
 * - Root property: The tree root is black.
 * - Binary Search Tree (BST) property: Keys are correctly ordered.
 * - Balance property: Uniform black-height across all paths.
 * - 2-3 Tree (LLRBT) property: No right-leaning red links or consecutive red links.
 *
 * @param root The tree root.
 * @return true, if all RBT invariants are satisfied; false, otherwise.
 * @note Time Complexity: O(N), where N is the total number of nodes in the tree.
 */
inline bool check(Node *root) { return root->color == BLACK && isBST(root) && isBalanced(root) && is23(root); }

/**
 * @brief Check that minDepth and maxDepth are correct for every node in the subtree.
 *
 * @param root The subtree root.
 * @return True, if every node's depth range covers exactly itself and its internal children; false, otherwise.
 * @note Time Complexity: O(N), where N is the number of nodes in the subtree.
 */
inline bool hasValidDepths(Node *root) {
  if (root == Node::nil)
    return true;
  short expectedMin = root->depth;
  short expectedMax = root->depth;
  for (Node *child : {root->left, root->right}) {
    if (!child->isExternal) {
      expectedMin = std::min(expectedMin, child->minDepth);
      expectedMax = std::max(expectedMax, child->maxDepth);
    }
  }
  if (root->minDepth != expectedMin || root->maxDepth != expectedMax)
    return false;
  return (root->left->isExternal || hasValidDepths(root->left)) && (root->right->isExternal || hasValidDepths(root->right));
}

/* Tango tree checkers. */

/**
 * @brief Checks that the given auxiliary tree is a valid left-leaning red-black tree.
 * External nodes below the root are treated as nil leaves, so each auxiliary tree is validated independently.
 *
 * @param auxRoot The auxiliary tree root node.
 * @return Success, if all the invariants hold; a failure naming the first violated one, otherwise.
 */
inline ::testing::AssertionResult isValidRbt(Node *auxRoot) {
  const int key = auxRoot->key;
  if (auxRoot->isExternal && auxRoot->blackHeight != -1)
    return ::testing::AssertionFailure() << "auxiliary tree root " << key << " is external but has cached black height " << auxRoot->blackHeight;

  if (auxRoot->color != BLACK)
    return ::testing::AssertionFailure() << "auxiliary tree root " << key << " is red";
  if (!isBST(auxRoot))
    return ::testing::AssertionFailure() << "auxiliary tree rooted at " << key << " breaks the BST order";
  if (!isBalanced(auxRoot))
    return ::testing::AssertionFailure() << "auxiliary tree rooted at " << key << " has inconsistent black heights";
  if (!is23(auxRoot))
    return ::testing::AssertionFailure() << "auxiliary tree rooted at " << key << " has a right-leaning or double red link";
  return ::testing::AssertionSuccess();
}

/**
 * @brief Checks that the depths of the given auxiliary tree are exactly the range [l, r], each one appearing once, and
 * that the cached minDepth and maxDepth of every node match the nodes below it.
 *
 * @param auxRoot The auxiliary tree root node.
 * @return Success, if the depth range is complete and the cached values are correct; a failure, otherwise.
 */
inline ::testing::AssertionResult hasCompleteDepthRange(Node *auxRoot) {
  std::vector<int> depths;
  for (Node *node : auxNodes(auxRoot))
    depths.push_back(node->depth);
  std::sort(depths.begin(), depths.end());

  const int l = depths.front();
  const int r = depths.back();
  for (size_t i = 0; i < depths.size(); ++i) {
    if (depths[i] != l + static_cast<int>(i))
      return ::testing::AssertionFailure() << "auxiliary tree rooted at " << auxRoot->key << " does not hold every depth in [" << l << ", " << r << "]";
  }

  if (!hasValidDepths(auxRoot))
    return ::testing::AssertionFailure() << "auxiliary tree rooted at " << auxRoot->key << " has wrong cached depths";
  return ::testing::AssertionSuccess();
}

/**
 * @brief Checks that the given auxiliary tree is a contiguous descending path of the reference tree.
 *
 * @param auxRoot The auxiliary tree root node.
 * @param ref The reference tree.
 * @return Success, if the nodes form a reference tree path and kept their depths; a failure, otherwise.
 */
inline ::testing::AssertionResult isPreferredPath(Node *auxRoot, const ReferenceTree &ref) {
  std::vector<Node *> path = auxNodes(auxRoot);
  std::sort(path.begin(), path.end(), [](Node *a, Node *b) { return a->depth < b->depth; });

  for (size_t i = 0; i < path.size(); ++i) {
    if (path[i]->key < 1 || path[i]->key > ref.n)
      return ::testing::AssertionFailure() << "key " << path[i]->key << " is out of range";
    if (path[i]->depth != ref.depth.at(path[i]->key))
      return ::testing::AssertionFailure() << "key " << path[i]->key << " changed its depth to " << path[i]->depth;
    if (i > 0 && ref.parent.at(path[i]->key) != path[i - 1]->key)
      return ::testing::AssertionFailure() << "key " << path[i]->key << " follows " << path[i - 1]->key << " in a preferred path, but is not its child in the reference tree";
  }
  return ::testing::AssertionSuccess();
}

/**
 * @brief Performs a complete validation of the tango tree:
 * - The tango tree root is not external, and its path starts at the reference tree root.
 * - Every auxiliary tree is a valid red-black tree.
 * - Every auxiliary tree holds every depth in its range, and is a path of the reference tree.
 * - The auxiliary trees partition the keys, and the whole tree is sorted.
 *
 * @param tree The tango tree.
 * @param ref The reference tree.
 * @return Success, if the tree is valid; a failure describing the first violation, otherwise.
 * @note Time Complexity: O(N * log(N)), where N is the number of nodes in the tree.
 */
inline ::testing::AssertionResult isValidTango(TangoTree &tree, const ReferenceTree &ref) {
  if (tree.root->isExternal)
    return ::testing::AssertionFailure() << "the tango tree root must not be external";
  if (tree.root->minDepth != 0)
    return ::testing::AssertionFailure() << "the root path must start at depth 0, but starts at " << tree.root->minDepth;

  std::vector<bool> seen(ref.n + 1, false);
  for (Node *auxRoot : getAllAuxiliary(tree)) {
    ::testing::AssertionResult validRbt = isValidRbt(auxRoot);
    if (!validRbt)
      return validRbt;
    ::testing::AssertionResult completeRange = hasCompleteDepthRange(auxRoot);
    if (!completeRange)
      return completeRange;
    ::testing::AssertionResult preferredPath = isPreferredPath(auxRoot, ref);
    if (!preferredPath)
      return preferredPath;

    for (Node *node : auxNodes(auxRoot)) {
      if (seen.at(node->key))
        return ::testing::AssertionFailure() << "key " << node->key << " appears in more than one place";
      seen.at(node->key) = true;
    }
  }

  for (int key = 1; key <= ref.n; ++key) {
    if (!seen[key])
      return ::testing::AssertionFailure() << "key " << key << " is in no auxiliary tree";
  }

  std::vector<int> expected(ref.n);
  std::iota(expected.begin(), expected.end(), 1);
  if (keys(tree.root) != expected)
    return ::testing::AssertionFailure() << "the in-order traversal is not the sorted keys";
  return ::testing::AssertionSuccess();
}

/**
 * @brief Searches for a key and validates the whole tree afterwards. For a present key, also checks that the root path
 * now holds the whole reference tree path to it.
 *
 * @param tree The tango tree.
 * @param ref The reference tree.
 * @param key The key to search for.
 * @return Success, if the result and the resulting tree are correct; a failure, otherwise.
 */
inline ::testing::AssertionResult accessAndValidate(TangoTree &tree, const ReferenceTree &ref, int key) {
  const bool expected = key >= 1 && key <= ref.n;
  if (tree.contains(key) != expected)
    return ::testing::AssertionFailure() << "contains(" << key << ") should be " << (expected ? "true" : "false");

  ::testing::AssertionResult valid = isValidTango(tree, ref);
  if (!valid)
    return ::testing::AssertionFailure() << "after accessing " << key << ": " << valid.message();

  if (expected) {
    std::set<int> rootKeys;
    for (Node *node : auxNodes(tree.root))
      rootKeys.insert(node->key);
    for (int ancestor : ref.pathTo(key)) {
      if (rootKeys.count(ancestor) == 0)
        return ::testing::AssertionFailure() << "after accessing " << key << ": the root path is missing " << ancestor;
    }
  }
  return ::testing::AssertionSuccess();
}

#endif     // CHECKERS_H
