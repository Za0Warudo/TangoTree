#ifndef REDBLACKTREE_H
#define REDBLACKTREE_H

/**
 * @file RedBlackTree.h
 *
 * @author Zawarudo (@zawarudo)
 *
 * @brief A header file for the Red-Black Tree data structure implementation in C++. The implementation has the purpose
 * to be an auxiliary data structure for  the implementation of Tango Tree. Some operations are omitted since they are
 * not necessary for the implementation of Tango Tree, other ones are an adaptation that improves the efficiency of
 * the Tango Tree implementation. For instance the model contains only operations such as search, deleteMin, deleteMax
 * and some extra operations like split and join. The implementation is based on the standard Red-Black Tree data
 * structure, given by Sedgewick and Wayne in their book "Algorithms". Also the focus is not the optimization or the
 * performance, but rather clarity and correctness for the Tango Tree implementation.
 *
 * @version 1.0
 * @date 2026-03-07
 */

// includes and namespace.
#include <tuple>

// defines
#define SHORT_MAX (short)32767  // maximum value for a short number.
#define SHORT_MIN (short)-32768 // minimum value for a short number.
#define DEBUG true       // flag for debugging

/**
 * @brief Enum to represent the color of a node in the Red-Black Tree. A node can be either RED or BLACK.
 *
 * @note Using a boolean instead improves the memory efficiency, but using an enum makes the code more readable and
 * self-explanatory.
 */
enum Color { RED, BLACK };

/**
 * @brief Struct to represent a node in the Red-Black Tree. Each node contains, at least, an integer key, pointers to
 * the left and right children, and a color field.
 *
 * @note Since the red-black tree is an auxiliary data structure for the tango tree, some extra fields like
 * depth, minDepth, MaxDepth and isExternal are include in the node definition. This extra fields will be useful for
 * the tango operations.
 */
struct Node {
  int key; // The key value of the node. For simplicity we use an integer key, but it could be templated to support any
           // comparable type.

  Node *left;  // Pointer to the left child.
  Node *right; // Pointer to the right child.
  Color color; // The color of the node (RED or BLACK).

  short blackHeight; // Black height of the node in the Red-Black Tree. Since a tree with n nodes has height at most
                     // 2*log(n), we can use a short to store the black height, which is more memory efficient than use
                     // an integer.

  // Tango Tree specific fields.

  short depth; // Depth of the node in the Reference Tree.

  short minDepth; // Minimum depth of the subtree rooted at this node in the Reference Tree.

  short maxDepth; // Maximum depth of the subtree rooted at this node in the Reference Tree.

  bool isExternal; // Flag to indicate if the node is an external node (nil or another tree root).

  // Define nil node.
  static Node *nil; // Static pointer to the nil node, shared among all nodes instances.

  // constructor.
  Node(int k)
      : key(k), left(nullptr), right(nullptr), color(RED), blackHeight(1), depth(0), minDepth(0), maxDepth(0),
        isExternal(false) {}
};

// Red-Black Tree methods. //

/**
 * @brief Create a new node with the given key and depth. The new node is initialized with default values for its
 * fields. The left and right children are set to the nil node, the color is set to RED, the black height is set to 1,
 * and the minDepth and maxDepth are set to the given depth. The new node is considered an internal node (isExternal =
 * false).
 *
 * @param key The key value for the new node.
 * @param depth The node depth (used in the construct of the tango tree)
 * @return A pointer to the newly created node.
 * @note Time Complexity: O(1).
 */
Node *newNode(int key, int depth = 0);

/**
 * @brief Search for a node with a given key in the Red-Black Tree. It returns a pointer to the node and to it's parent
 * if found it, or an external node and the last visited node otherwise, meaning that the key is not present in the
 * tree.
 *
 * @param h The root of the subtree to search.
 * @param key The key value to search for.
 * @return A pointer to the node and to its parent if found, or an external node and the last visited node otherwise,
 * meaning that the key is not present in the tree.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the subtree.
 */
std::pair<Node *, Node *> search(Node *root, int key, Node *p = Node::nil);

/**
 * @brief inserts a key into the Red-Black Tree rooted at the given node. This function uses a recursive helper function
 * to perform the actual insertion and then ensures that the root of the tree is always black after insertion to
 * maintain the properties of the Red-Black Tree. As a precondition, the key to be inserted must not already be present
 * in the tree. The function returns the new root of the treeafter insertion.
 *
 * @param root The root of the Red-Black Tree into which the key is to be inserted.
 * @param key The key to insert into the tree. Should not be already present in the tree.
 * @return The new root of the tree after inserting the key and maintaining the properties of the Red-Black Tree.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the subtree.
 */
Node *insert(Node *root, int key);

/**
 * @brief Joins two Red-Black Trees and a node into a single tree. As a precondition, all keys in the left tree must be
 * less than the given node's key, and all keys in the right tree must be greater than the given node's key.
 *
 * @param leftTree The root of the left tree to join.
 * @param x The node to join between the two trees.
 * @param rightTree The root of the right tree to join.
 * @return The root of the new tree formed by joining the left tree, the node, and the right tree.
 * @note Precondition: All the keys in the left tree must be smaller than the key in the given node. All the keys in the
 * right tree must be greater than the key in the given node.
 * @note Time Complexity: O(|h1 - h2|), where h1 is the black-height of the left tree and h2 is the black-height of the
 * right tree.
 */
Node *join(Node *leftTree, Node *x, Node *rightTree);

/**
 * @brief Splits the Red-Black Tree into two trees and a node. The first tree contains all the nodes with keys less than
 * the given key, the second tree contains all the nodes with keys greater than the given key, and the node with the
 * given key is returned as well. As a precondition, the key must be present in the tree.
 *
 * @param h The root of the subtree to split.
 * @param key The key value to split the tree around.
 * @return A tuple containing the left tree, the node with the given key, and the right tree.
 * @note Precondition: The key must exist in the tree before calling this function.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the subtree.

 */
std::tuple<Node *, Node *, Node *> split(Node *h, int key);

/**
 * @brief Delete the node with the minimum key in the Red-Black Tree.
 *
 * @param root The root of the subtree from which to delete the minimum node.
 * @return The root of the subtree after deleting the minimum node.
 * @note Precondition: The tree must not be empty.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the subtree.
 */
Node *deleteMin(Node *root);

/**
 * @brief Delete the node with the maximum key in the Red-Black Tree.
 *
 * @param root The root of the subtree from which to delete the maximum node.
 * @return The root of the subtree after deleting the maximum node.
 * @note Precondition: The tree must not be empty.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the subtree.
 */
Node *deleteMax(Node *root);

/**
 * @brief Removes the node associated with the given key while maintaining tree invariants.
 *
 * This implementation demonstrates the efficiency of the Split/Join paradigm:
 * It partitions the tree to isolate the target key and then merges the remaining
 * subtrees to restore the Red-Black structure.
 *
 * @param h The current root of the subtree.
 * @param key The key to be deleted.
 * @return The new root of the tree after the join operation.
 *
 * @note Precondition: The key must exist in the tree before calling this function.
 * @note Time Complexity: O(log N), where N is the number of nodes in the subtree.
 */
Node *deleteNode(Node *h, int key);

/**
 * @brief Removes the minimum key node, and return a pointer to the tree new root and pointer to removed node.
 *
 * @param h The current root of the tree.
 * @return A pair, the first element is the new tree root and the second one is the a min node pointer.
 */
std::pair<Node *, Node *> extractMin(Node *h);

/**
 * @brief Removes the maximum key node, and return a pointer to the tree new root and pointer to removed node.
 *
 * @param h The current root of the tree.
 * @return A pair, the first element is the new tree root and the second one is the a max node pointer.
 */
std::pair<Node *, Node *> extractMax(Node *h);

/**
 * @brief Return the minimum key value in the subtree rooted at the given node.
 *
 * @param root The root of the subtree to search.
 * @return The minimum node in the subtree.
 * @note Precondition: The tree must not be empty.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the subtree.
 */
Node* min(Node *root);
/**
 * @brief Return the maximum key value in the subtree rooted at the given node.
 *
 * @param root The root of the subtree to search.
 * @return int The maximum node in the subtree.
 * @note Precondition: The tree must not be empty.
 * @note Time Complexity: O(log(N)), where N is the number of nodes in the subtree.
 */
Node* max(Node *root);

/**
 * @brief Prints the Red-Black Tree rooted at the given node.
 *
 * @param root The root of the tree to print.
 * @param indent The indentation level for printing.
 */
void print(Node *root, int indent = 0);

/**
 * @brief Updates the infos of the given node based on its children. This function should be called after any
 * modification to the subtree rooted at the given node to ensure that the properties of the Red-Black Tree and the
 * auxiliary tree are maintained.
 *
 * @param h The node whose infos are to be updated.
 * @note Time Complexity: O(1)
 */
void update(Node *h);

#endif // REDBLACKTREE_H