#include "TangoTree.h"
#include "RedBlackTree.h"

/**
 * @brief Recursive method to build a Tango Tree based on a reference tree with the nodes in the range [l, r]. The
 * method takes the left and right bounds of the range and the current depth in the reference tree as parameters. It
 * constructs the Tango Tree by recursively dividing the range and creating nodes with the appropriate depth and min/max
 * depth values. The method returns a pointer to the root of the constructed Tango Tree.
 *
 * @param l The left bound of the range of keys to be included in the Tango Tree.
 * @param r The right bound of the range of keys to be included in the Tango Tree.
 * @param depth The current depth in the reference tree, used to set the depth and min/max depth values of the nodes in the Tango Tree.
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
 * depth greater than the given depth. If there is no predecessor with the given depth, it returns -1 as the predecessor
 * key.
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
    return {h->left->isExternal ? -1 : max(h->left), h->key};
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
std::pair<int, int> sucessor(Node *h, int depth) {
  if (h->right->maxDepth >= depth)
    return sucessor(h->right, depth);
  if (h->depth >= depth)
    return {h->right->isExternal ? -1 : min(h->right), h->key};
  auto [s, r] = sucessor(h->left, depth);
  return {s == -1 ? h->key : s, r};
}

TangoTree::TangoTree(int n) { root = buildTango(1, n); }

bool TangoTree::contain(int key) { return false; }