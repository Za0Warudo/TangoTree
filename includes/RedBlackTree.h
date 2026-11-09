#ifndef REDBLACKTREE_H
#define REDBLACKTREE_H

/**
 * @file RedBlackTree.h
 *
 * @author Zawarudo (@zawarudo)
 *
 * @brief A header file for a red-black tree, designed specifically as an auxiliary data structure for a tango tree. To align with the
 * requirements of the tango tree, some operations may have been omitted, while others were adapted to improve functional compatibility. This
 * implementation includes the following operations:
 *
 *  - newNode: Creates a red node with the given key and reference tree depth, and nil children. O(1).
 *  - search: Returns the node with the given key, or an external node if the key is not in the tree. O(log N).
 *  - parentSearch: Returns the parent of the node with the given key. The result is an external node if the tree is empty or the key is at the root. O(log N).
 *  - insert: Inserts a key, ignoring duplicates, and returns the new tree root. O(log N).
 *  - join: Merges a left tree, a middle node, and a right tree, given left keys < node key < right keys. O(|h1 - h2|), with h1 and h2 the tree heights.
 *  - split: Splits a tree around a key that must exist, returning the tree of smaller keys, the node with the key, and the tree of greater keys. O(log N).
 *  - deleteMin: Removes the node with the smallest key from a non-empty tree, returning the removed node and the new root. O(log N).
 *  - deleteMax: Removes the node with the largest key from a non-empty tree, returning the removed node and the new root. O(log N).
 *  - min: Returns the node with the smallest key in a non-empty subtree. O(log N).
 *  - max: Returns the node with the largest key in a non-empty subtree. O(log N).
 *  - keys: Returns all the keys of a subtree in ascending order. O(N).
 *  - print: Prints a tree sideways to standard output, with the right subtree on top and four spaces of indentation per level. O(N).
 *  - update: Recomputes a node's blackHeight, minDepth, and maxDepth from its children. O(1).
 *
 * The main reference source of this implementation is the red-black tree model by Sedgewick and Wayne, but this version prioritizes clarity and
 * correctness over raw performance optimization.
 *
 * @version 2.1
 * @date 2026-03-07
 */

// includes.
#include <tuple>
#include <vector>

// defines.
#define SHORT_MAX (short)32767      // maximum short number value.
#define SHORT_MIN (short)-32768     // minimum short number value.

/**
 * @brief Enum to encode the color of a node in the red-black tree. A node color can be either RED or BLACK.
 *
 * @note Using a boolean would improve memory efficiency; however, we opted for an enum to ensure the code remains readable and self-explanatory.
 */
enum Color { RED, BLACK };

/**
 * @brief Struct to represent a node in a red-black tree. Each node contains, at least, an integer key, pointers to the left and right children, and a color
 * field.
 *
 * @note Since this red-black tree is an auxiliary data structure for the tango tree, some extra fields like depth, minDepth, maxDepth and isExternal are
 * included in the node definition. These extra fields will be useful for the tango operations.
 * @note For simplicity we use an integer key, but it could be templated to support any comparable type.
 * @note Since a tree with n nodes has height at most 2*log(n), we can use a short to store the black height, which is more memory efficient than using an
 * integer.
 */
struct Node {
  int key;     // The node key value.

  Node *left;      // The left child pointer.
  Node *right;     // The right child pointer.
  Color color;     // The node color (RED or BLACK).

  short blackHeight;     // Black height of the node in the red-black tree.

  // Tango tree specific fields.

  short depth;         // The node depth in the reference tree.
  short minDepth;      // The subtree min depth in the reference tree.
  short maxDepth;      // The subtree max depth in the reference tree.
  bool isExternal;     // Flag to indicate if the node is an external node (nil or another tree root).

  static Node *nil;     // Static pointer to the nil node, shared among all node instances.

  // Constructor.
  Node(int k) : key(k), left(nullptr), right(nullptr), color(RED), blackHeight(0), depth(0), minDepth(0), maxDepth(0), isExternal(false) {}
};

// Red-black tree methods. //

/**
 * @brief Creates a new node with the given key and depth.
 *
 * @param key The new node key value.
 * @param depth The node depth (used in the tango tree construction).
 * @return A pointer to the newly created node.
 *
 * @note The new node is initialized with the following default values for its fields: The left and right children are set to the nil node, the color is set to
 * RED, the black height is set to 0, the isExternal flag is set to false, and the minDepth and maxDepth are set to the given depth.
 * @note Time Complexity: O(1).
 */
Node *newNode(int key, int depth = 0);

/**
 * @brief Searches for a node with a given key in the red-black tree. If the node is found, returns a pointer to the node. Otherwise, returns a pointer to an
 * external node.
 *
 * @param root The tree root node.
 * @param key The search key.
 * @return A pointer to the node if found, or an external node otherwise.
 *
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the tree.
 */
Node *search(Node *root, int key);

/**
 * @brief Searches for the parent node of the node with the given key in the red-black tree.
 *
 * @param root The tree root node.
 * @param key The search key.
 * @return A pointer to the parent node of the node with the given key if found, or the last visited node otherwise.
 *
 * @note The parent is an external node, if and only if the tree is empty or the key is at the tree root.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the tree.
 */
Node *parentSearch(Node *root, int key);

/**
 * @brief Inserts a key into the given red-black tree.
 *
 * @param root The tree root node.
 * @param key The key to insert into the tree.
 * @return The new root of the tree after inserting the key and maintaining the properties of the red-black tree.
 *
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the tree.
 */
Node *insert(Node *root, int key);

/**
 * @brief Joins two red-black trees and a node into a single red-black tree.
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
Node *join(Node *leftTree, Node *x, Node *rightTree);

/**
 * @brief Splits the given red-black tree into two trees and a node. The first tree contains all the nodes with keys smaller than the given key. The second one
 * contains all the nodes with keys greater than the given key. The node returned is the one with the given key.
 *
 * @param h The tree root node.
 * @param key The splitter key.
 * @return A tuple containing the left tree, the node with the given key, and the right tree.
 *
 * @note The returned node is black and has nil children.
 * @note Precondition: The key must exist in the tree before calling this function.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the tree.
 */
std::tuple<Node *, Node *, Node *> split(Node *h, int key);

/**
 * @brief Deletes and returns the node with the minimum key in the red-black tree.
 *
 * @param root The tree root node.
 * @return The minimum node and the new tree root after deleting the minimum node.
 *
 * @note Precondition: The tree must be non-empty.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the tree.
 */
std::pair<Node *, Node *> deleteMin(Node *root);

/**
 * @brief Deletes and returns the node with the maximum key in the red-black tree.
 *
 * @param root The tree root node.
 * @return The maximum node and the new tree root after deleting the maximum node.
 *
 * @note Precondition: The tree must be non-empty.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the tree.
 */
std::pair<Node *, Node *> deleteMax(Node *root);

/**
 * @brief Returns the node with the minimum key in the given subtree.
 *
 * @param root The subtree root node.
 * @return The minimum subtree node.
 *
 * @note Precondition: The subtree must be non-empty.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the subtree.
 */
Node *min(Node *root);

/**
 * @brief Returns the node with the maximum key in the given subtree.
 *
 * @param root The subtree root node.
 * @return The maximum subtree node.
 *
 * @note Precondition: The subtree must be non-empty.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the subtree.
 */
Node *max(Node *root);

/**
 * @brief Returns a vector containing all the keys in the given subtree, in sorted order.
 *
 * @param root The subtree root node.
 * @return A vector containing all the keys in the subtree, sorted in ascending order.
 *
 * @note Time Complexity: O(N), where N is the number of nodes in the subtree.
 */
std::vector<int> keys(Node *root);

/**
 * @brief Prints the given red-black tree.
 *
 * @param root The tree root node.
 *
 * @note Time Complexity: O(N), where N is the number of nodes in the tree.
 */
void print(Node *root);

/**
 * @brief Updates the given node fields based on its children.
 *
 * @param h The node.
 *
 * @note Time Complexity: O(1).
 */
void update(Node *h);

#endif     // REDBLACKTREE_H