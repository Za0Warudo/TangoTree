#ifndef REDBLACKTREE_H
#define REDBLACKTREE_H

/**
 * @file RedBlackTree.h
 *
 * @author Zawarudo (@zawarudo)
 *
 * @brief A header file for the Red-Black Tree data structure implementation in
 * C++. The implementation has the purpose to be an auxiliary data structure for
 * the implementation of Tango Tree. Some operations are omitted since they are
 * not necessary for the implementation of Tango Tree. For instance the model
 * contains only operations such as search, deleteMin, deleteMax and some extra
 * operations like split and join. The implementation is based on the standard
 * Red-Black Tree data structure, given by Sedgewick and Wayne in their book
 * "Algorithms". Also the focus is not the optimization or the performance, but
 * rather clarity and correctness for the Tango Tree implementation.
 *
 * @version 1.0
 * @date 2026-03-07
 */

// includes and namespace
#include <tuple>

#define SHORT_MAX 32767  // maximum value for a short number.
#define SHORT_MIN -32768 // minimum value for a short number.

/**
 * @brief Enum to represent the color of a node in the Red-Black Tree. A node
 * can be either RED or BLACK. One could use a boolean instead, but using an
 * enum makes the code more readable and self-explanatory.
 */
enum Color { RED, BLACK };

/**
 * @brief Struct to represent a node in the Red-Black Tree. Each node contains
 * an integer key, pointers to the left and right children, and a color (RED or
 * BLACK) at least.
 */
struct Node {
  int key; // The key value of the node. For simplicity we use an integer key,
           // but it could be templated to support any comparable type.

  Node *left;  // Pointer to the left child.
  Node *right; // Pointer to the right child.
  Color color; // RED or BLACK.

  short blackHeight; // Black height of the node in the Red-Black Tree. Since a
                     // tree with n nodes has height at most 2*log(n), we can
                     // use a short to store the black height, which is more
                     // memory efficient than an integer.

  // Tango Tree specific fields.

  short depth; // Depth of the node in the Reference Tree.

  short minDepth; // Minimum depth of the subtree rooted at this node in the
                  // Reference Tree.

  short maxDepth; // Maximum depth of the subtree rooted at this node in the
                  // Reference Tree.

  bool isExternal; // Flag to indicate if the node is an external node (nil or
                   // another tree root).

  // Define nil node
  static Node *nil; // Static pointer to the nil node, shared among all nodes instances.

  Node(int k) : key(k), left(nullptr), right(nullptr), color(RED), blackHeight(1), depth(0), minDepth(0), maxDepth(0), isExternal(false) {}
};

/**
 * @brief Define the nil node as a global variable. The nil node is a special
 * node that represents the absence of a child in the red-black-tree. It is used
 * to simplify the implementation by avoiding the need to check for null
 * pointers. The nil node is always black and has a key value that is not used
 * (e.g., -1). The left and right pointers of the nil node point to itself, and
 * it is considered an external node. The depth and
 * blackHeight of the nil node are all set to 0. The maxDepth and minDepth are
 * set to be the SHORT_MIN and SHORT_MAX respectively. This allows us to treat
 * the nil node as a leaf in the tree, which simplifies the logic for the
 * operations.
 *
 */
Node *Node::nil = [] {
  Node *n = new Node(-1); // Create a new node with key -1 (or any value, since
                          // it won't be used).

  n->color = BLACK; // The nil node is always black.

  n->left = n->right = n; // The left and right pointers of the nil node point to itself.

  n->isExternal = true; // The nil node is considered an external node
  n->depth = 0;         // The depth of the nil node is set to 0. This field is not used for the nil node,
                        // but we set it to 0 for consistency.

  n->minDepth = SHORT_MAX; // The minimum depth of the nil node is set to the
                           // maximum possible value, since it is an external
                           // node and has no children.

  n->maxDepth = SHORT_MIN; // The maximum depth of the nil node is set to the
                           // minimum possible value, since it is an external
                           // node and has no children.

  n->blackHeight = 0; // The height of the nil node is set to 0, it's the unique
                      // node with height 0.

  return n; // Return the pointer to the initialized nil node.
}(); // Initialize the nil node.

// Red-Black Tree methods. //

/**
 * @brief Create a new node with the given key and depth. The new node is initialized with
 * default values for its fields.
 *
 * @param key The key value for the new node.
 * @param depth The node depth (used in the construct of the tango tree)
 * @return A pointer to the newly created node.
 */
Node *newNode(int key, int depth = 0);

/**
 * @brief Search for a node with a given key in the Red-Black Tree. It returns a
 * pointer to the node if found, or external node otherwise, meaning that the
 * key is not present in the tree.
 *
 * @param h The root of the subtree to search.
 * @param key The key value to search for.
 * @return A pointer to the node with the given key, or an external node if not
 * found.
 */
Node *search(Node *root, int key);

/**
 * @brief Split the Red-Black Tree into two trees and a node. The first tree
 * contains all the nodes with keys less than the given key, the second tree
 * contains all the nodes with keys greater than the given key, and the node
 * with the given key is returned as well. As a precondition, the key must be
 * present in the tree.
 *
 * @param h The root of the subtree to split.
 * @param key The key value to split the tree around.
 * @return A tuple containing the left tree, the node with the given key, and
 * the right tree.
 */
std::tuple<Node *, Node *, Node *> split(Node *h, int key);

/**
 * @brief Join two Red-Black Trees and a node into a single tree. As a
 * precondition, all keys in the left tree must be less than the given node's
 * key, and all keys in the right tree must be greater than the given node's
 * key.
 *
 * @param leftTree The root of the left tree to join.
 * @param x The node to join between the two trees.
 * @param rightTree The root of the right tree to join.
 * @return The root of the new tree formed by joining the left tree, the node,
 * and the right tree.
 */
Node *join(Node *leftTree, Node *x, Node *rightTree);

/**
 * @brief Delete the node with the minimum key in the Red-Black Tree. As a
 * precondition, the tree must not be empty.
 *
 * @param root The root of the subtree from which to delete the minimum node.
 * @return The root of the subtree after deleting the minimum node.
 */
Node *deleteMin(Node *root);

/**
 * @brief Delete the node with the maximum key in the Red-Black Tree. As a
 * precondition, the tree must not be empty.
 *
 * @param root The root of the subtree from which to delete the maximum node.
 * @return The root of the subtree after deleting the maximum node.
 */
Node *deleteMax(Node *root);

#endif // REDBLACKTREE_H