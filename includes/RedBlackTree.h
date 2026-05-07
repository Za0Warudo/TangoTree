#ifndef REDBLACKTREE_H
#define REDBLACKTREE_H

/**
 * @file RedBlackTree.h
 *
 * @author Zawarudo (@zawarudo)
 *
 * @brief A header file for a Red-Black Tree implementation in C++, designed specifically as an auxiliary data structure for a Tango Tree. To align with the
 * requirements of the Tango Tree, several standard operations have been omitted, while others were adapted to improve functional compatibility. This
 * implementation includes core operations such as search, deleteMin, and deleteMax, alongside specialized split and join functions. Based on the Red-Black Tree
 * model by Sedgewick and Wayne, this version prioritizes clarity and correctness over raw performance optimization.
 *
 * @version 2.0
 * @date 2026-03-07
 */

// includes.
#include <tuple>

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
 * @note Since the red-black tree is an auxiliary data structure for the tango tree, some extra fields like depth, minDepth, maxDepth and isExternal are include
 * in the node definition. This extra fields will be useful for the tango operations.
 */
struct Node {
  int key;     // The node key value. For simplicity we use an integer key, but it could be templated to support any comparable type.

  Node *left;      // The left child pointer.
  Node *right;     // The right child pointer.
  Color color;     // The node color (RED or BLACK).

  short blackHeight;     // Black height of the node in the red-black tree. Since a tree with n nodes has height at most 2*log(n), we can use a short to store
                         // the black height, which is more memory efficient than use an integer.

  // Tango Tree specific fields.

  short depth;         // The node depth in the reference tree.
  short minDepth;      // The subtree min depth in the reference tree.
  short maxDepth;      // The subtree max depth in the reference tree.
  bool isExternal;     // Flag to indicate if the node is an external node (nil or another tree root).

  static Node *nil;     // Static pointer to the nil node, shared among all nodes instances.

  // Constructor.
  Node(int k) : key(k), left(nullptr), right(nullptr), color(RED), blackHeight(0), depth(0), minDepth(0), maxDepth(0), isExternal(false) {}
};

// Red-Black Tree methods. //

/**
 * @brief Create a new node with the given key and depth. The new node is initialized with de folowing default values for its fields: The left and right
 * children are set to the nil node, the color is set to RED, the black height is set to 0, the isExternal flag is set to false, and the minDepth and maxDepth
 * are set to the given depth.
 *
 * @param key The new node key value.
 * @param depth The node depth (used in the the tango tree construction)
 * @return A pointer to the newly created node.
 * @note Time Complexity: O(1).
 */
Node *newNode(int key, int depth = 0);

/**
 * @brief Search for a node with a given key in the red-black tree. It returns a pointer to the node and it's parent if found it, or an external node and the
 * last visited node otherwise.
 *
 * @param h The subtree root node.
 * @param key The search for key.
 * @return A pointer to the node and to its parent if found, or an external node and the last visited node otherwise.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the subtree.
 */
std::pair<Node *, Node *> search(Node *root, int key);

/**
 * @brief inserts a key into the given red-black tree.
 *
 * @param root The tree root.
 * @param key The key to insert into the tree.
 * @return The new root of the tree after inserting the key and maintaining the properties of the Red-Black Tree.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the subtree.
 */
Node *insert(Node *root, int key);

/**
 * @brief Joins two Red-Black Trees and a node into a single tree. As a precondition, all keys in the left tree must be smaller than the given node's key, and
 * all keys in the right tree must be greater than the given node's key.
 *
 * @param leftTree The root of the left tree.
 * @param x The middle node.
 * @param rightTree The root of the right tree.
 * @return The root of the new tree formed by joining the left tree, the node, and the right tree.
 * @note Precondition: All the keys in the left tree must be smaller than the key in the given node. All the keys in the right tree must be greater than the key
 * in the given node.
 * @note Time Complexity: O(|h1 - h2|), where h1 is the black height of the left tree and h2 is the black height of the right tree.
 */
Node *join(Node *leftTree, Node *x, Node *rightTree);

/**
 * @brief Splits the given red-black tree into two trees and a node. The first tree contains all the nodes with keys smaller than the given key. The second one
 * contains all the nodes with keys greater than the given key. The node return is the one with the given key. As a precondition, the key must be present
 * in the tree.
 *
 * @param h The subtree root.
 * @param key The splitter key.
 * @return A tuple containing the left tree, the node with the given key, and the right tree.
 * @note Precondition: The key must exist in the tree before calling this function.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the subtree.

 */
std::tuple<Node *, Node *, Node *> split(Node *h, int key);

/**
 * @brief Delete and return the node with the min key in the red-black tree.
 *
 * @param root The subtree root.
 * @return The min node and the new subtree root after deleting the minimum node.
 * @note Precondition: The tree must be non-empty.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the subtree.
 */
std::pair<Node *, Node*> deleteMin(Node *root);

/**
 * @brief Delete and return the node with the maximum key in the red-black tree.
 *
 * @param root The subtree root.
 * @return The max node and the new subtree root after deleting the maximum node.
 * @note Precondition: The tree must not be empty.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the subtree.
 */
std::pair<Node *, Node*> deleteMax(Node *root);

/**
 * @brief Return the minimum key value in the given subtree.
 *
 * @param root The subtree root.
 * @return The minimum subtree node.
 * @note Precondition: The tree must be non-empty.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the subtree.
 */
Node *min(Node *root);
/**
 * @brief Return the maximum key value in the given subtree.
 *
 * @param root The subtree root.
 * @return int The maximum subtree node.
 * @note Precondition: The tree must be non-empty.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the subtree.
 */
Node *max(Node *root);

/**
 * @brief Prints the given red-black tree.
 *
 * @param root The tree root.
 */
void print(Node *root);

/**
 * @brief Updates the given node fields based on its children.
 *
 * @param h The node.
 * @note Time Complexity: O(1)
 */
void update(Node *h);

#endif     // REDBLACKTREE_H