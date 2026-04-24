/**
 * @file RedBlackTree.cpp
 * @author Zawarudo (@zawarudo)
 * @version 0.1
 * @date 2026-03-14
 * @copyright Copyright (c) 2026
 *
 * Implementation of the Red-Black Tree data structure defined in RedBlackTree.h. The implementation includes the basic
 * operations of a Red-Black Tree, such as insertion, deletion of the minimum and maximum nodes, searching for a key,
 * and the auxiliary operations of join and split. The implementation is designed to be clear and correct, with a focus
 * on maintaining the properties of the Red-Black Tree and ensuring that the operations work correctly in the context of
 * their use in a Tango Tree implementation.
 */

// includes.
#include "RedBlackTree.h"
#include <cassert>
#include <climits>
#include <iostream>
#include <map>

// flag for debugging, set to true to enable assert tests, false to disable. The debug test may include some poor
// performance tests, so it is recommended to disable it when not needed.
#define DEBUG true

/**
 * @brief Define the nil node as a global variable. The nil node is a special node that represents the absence of a
 * child in the red-black tree. It is used to simplify the implementation by avoiding the need to check for null
 * pointers. The nil node is always black and is considered an external node. It has a key value that is not used (for
 * positive integer keys, we can use -1 for instance). The left and right pointers of the nil node point to itself,
 * creating a cycle. The depth and blackHeight of the nil node are set to -1. The maxDepth and minDepth are set to
 * SHORT_MIN and SHORT_MAX respectively. This allows us to treat the nil node as a leaf in the tree, which simplifies
 * the operation logic.
 */
Node *Node::nil = [] {
  Node *n = new Node(-1);     // Create a new node with key -1 (or any value, since it won't be used).

  n->color = BLACK;     // The nil node is always black.

  n->left = n->right = n;     // The left and right pointers of the nil node point to itself.

  n->isExternal = true;     // The nil node is considered an external node.

  n->depth = 0;     // The depth of the nil node is set to 0. This field is not used for the nil node, but we set it to
                    // 0 for consistency.

  n->minDepth = SHORT_MAX;     // The minimum depth of the nil node is set to the maximum possible value, since it is an
                               // external node and has no children.

  n->maxDepth = SHORT_MIN;     // The maximum depth of the nil node is set to the minimum possible value, since it is an
                               // external node and has no children.

  n->blackHeight = -1;     // The height of the nil node is set to -1, it's the unique node with height -1.

  return n;     // Return the pointer to the initialized nil node.
}();     // Initialize the nil node.

/* Headers of auxiliary functions. */

Node *balance(Node *h);

Node *rotateLeft(Node *h);

Node *rotateRight(Node *h);

Node *moveRedLeft(Node *h);

Node *moveRedRight(Node *h);

std::pair<Node *, Node *> detach(Node *h);

bool check(Node *root);

/* NewNode. */

Node *newNode(int key, int depth) {
  Node *x = new Node(key);                          // create new node.
  x->left = x->right = Node::nil;                   // initialize children to nil.
  x->isExternal = false;                            // set as an internal node.
  x->blackHeight = 0;                               // default black height for a new node.
  x->depth = x->minDepth = x->maxDepth = depth;     // set the depth of the new node.
  return x;
}

/* Search */

std::pair<Node *, Node *> search(Node *h, int key, Node *p) {
  if (h->isExternal || h->key == key)
    return {h, p};     // If the node is external or the key matches, return the node and its parent.
  if (key < h->key)
    return search(h->left, key, h);     // If the key is less than the current node's key, search in the left subtree.
  else
    return search(h->right, key,
                  h);     // If the key is greater than the current node's key, search in the right subtree.
}

/* Insert. */

/**
 * @brief Recursive method to insert a key into the subtree rooted at h. If we reach an external node, we create and
 * return a new node with the given key. If the key is less than the current node's key, we insert into the left
 * subtree; if it is greater, we insert into the right subtree. After inserting, we balance the subtree rooted at h to
 * maintain the properties of the Red-Black Tree.
 *
 * @param h The root of the subtree into which to insert the key.
 * @param key The key to insert into the subtree.
 * @return The new root of the subtree after inserting the key and balancing the tree.
 */
Node *insertRec(Node *h, int key) {
  if (h->isExternal)
    return newNode(key);     // If we reach the nil node, we create and return a new
                             // node with the given key.
  if (key < h->key)
    h->left = insertRec(h->left, key);     // If the key is less than the current node's
                                           // key, insert into the left subtree.
  else if (key > h->key)
    h->right = insertRec(h->right, key);     // If the key is greater than the current
                                             // node's key, insert into the right subtree.
  else
    return h;            // If the key is equal to the current node's key, we do not insert
                         // duplicates and simply return the current node.
  return balance(h);     // After insertion, we balance the subtree rooted at h to
                         // maintain the properties of the Red-Black Tree.
}

Node *insert(Node *root, int key) {
  root = insertRec(root,
                   key);     // Insert the key into the tree using the recursive insert function
  root->color = BLACK;       // Ensure the root of the tree is always black after insertion.
  return root;
}

/* Join. */

/**
 * @brief Recursive method for to apply the join operation. If the left tree has a greater black
 * height, apply the join recursively on the right subtree of the left tree. If the right tree has a
 * greater black height, apply the join recursively on the left subtree of the right tree. If both
 * trees have the same black height, join them directly with x as the separator. After joining,
 * balance the resulting tree to maintain the properties of the Red-Black Tree.
 *
 * @param leftTree The left subtree to join. Should contain only keys less than x's key.
 * @param x The node to use as the separator for the join. Should have a key value between the
 * maximum key of the left tree and the minimum key of the right tree.
 * @param rightTree The right subtree to join. Should contain only keys greater than x's key.
 * @return The new root of the joined tree after applying the join operation and balancing the tree.
 */
Node *joinRec(Node *leftTree, Node *x, Node *rightTree) {
  if (leftTree->blackHeight > rightTree->blackHeight) {
    leftTree->right = joinRec(leftTree->right, x,
                              rightTree);     // If the left tree has a greater black height, join the
                                              // left subtree of the left tree with x and the right tree.
    return balance(leftTree);
  }
  if (leftTree->blackHeight < rightTree->blackHeight) {
    rightTree->left = joinRec(leftTree, x,
                              rightTree->left);     // If the right tree has a greater black height, join the
                                                    // right tree with x and the right subtree of the right tree.
    return balance(rightTree);
  }
  x->left = leftTree;
  x->right = rightTree;
  x->color = RED;
  return balance(x);
}

Node *join(Node *leftTree, Node *x, Node *rightTree) {
  if (DEBUG)
    assert((leftTree->isExternal || max(leftTree)->key < x->key) &&
           (rightTree->isExternal || x->key < min(rightTree)->key));     // Ensure that all keys in the left tree are less than x's key and all keys
                                                                         // in the right tree are greater than x's key before joining. This test has
                                                                         // a cost greater than the join operation itself, but it is useful for
                                                                         // debugging and ensuring the correctness of the join operation.

  detach(x);     // Detach x from its current position in the tree to prepare for joining.

  Node *y = joinRec(leftTree, x,
                    rightTree);     // Join the left tree, the node x, and the right tree using the recursive join function.

  y->color = BLACK;     // Ensure the new root of the joined tree is black.

  check(y);

  return y;     // Return the new root of the joined tree.
}

/* Split */

/**
 * @brief Recursive method to apply the split operation. If the key is less than the current node's key, apply the split
 * recursively on the left subtree. If the key is greater than the current node's key, apply the split recursively on
 * the right subtree. If the key matches the current node's key, detach the left and right subtrees from the current
 * node and return them as the left and right trees of the split, with the current node as the separator. After
 * splitting, ensure that the properties of the Red-Black Tree are maintained by setting the colors of the relevant
 * nodes to black before returning.
 *
 * @param h The subtree to split. Should contain the key to split around.
 * @param key The key to split around. Should be present in the subtree rooted at h.
 * @return A tuple containing the left tree (with keys less than the given key), the node with the given key, and the
 * right tree (with keys greater than the given key) after applying the split operation.
 */
std::tuple<Node *, Node *, Node *> splitRec(Node *h, int key) {
  if (key < h->key) {
    auto [left, x, right] = splitRec(h->left, key);
    h->right->color = BLACK;     // Ensure the right child of h is black before joining.
    return {left, x, join(right, h, h->right)};
  }
  if (key > h->key) {
    auto [left, x, right] = splitRec(h->right, key);
    h->left->color = BLACK;     // Ensure the left child of h is black before joining.
    return {join(h->left, h, left), x, right};
  }

  auto [left, right] = detach(h);
  left->color = right->color = BLACK;     // Ensure the detached left and right subtrees are black before returning.

  return {left, h, right};     // Return the left subtree, the node with the given key, and the right subtree as a tuple.
}

std::tuple<Node *, Node *, Node *> split(Node *h, int key) {
  if (DEBUG)
    assert(search(h, key).first != Node::nil);     // Ensure that the key is present in the tree before splitting.

  auto [l, x, r] = splitRec(h, key);     // Split the tree using the recursive split function
  check(l);
  check(x);
  check(r);
  return {l, x, r};
}

/* DeleteMin */

/**
 * @brief Recursive method to delete the node with the minimum key in the subtree rooted at h. If the left child of h is
 * nil, then h is the minimum node, so we return nil to delete it. If the left child of h is black and its left child is
 * also black, we need to move a red link left to ensure that we are moving down a path with a red link, which allows us
 * to maintain the properties of the Red-Black Tree during deletion. After deleting the minimum node from the left
 * subtree, we balance the subtree rooted at h to maintain the properties of the Red-Black Tree.
 *
 * @param h The root of the subtree from which to delete the minimum node. Should not be nil.
 * @return The new root of the subtree after deleting the minimum node and balancing the tree.
 */
Node *deleteMinRec(Node *h) {
  if (h->left->isExternal)
    return h->right;     // If the left child of h is nil, then h is the minimum node, so we return it's right pointer
                         // (nil for nil, and useful for tango tree).
  if (h->left->color == BLACK &&
      h->left->left->color == BLACK)     // Guarantee that the left child of h is not a 2-node by moving a red link left if necessary.
    h = moveRedLeft(h);
  h->left = deleteMinRec(h->left);
  return balance(h);
}

Node *deleteMin(Node *root) {
  if (DEBUG)
    assert(root != Node::nil);     // Ensure that the root is not the nil node before deleting the minimum, since we
                                   // should never delete from an empty tree.
  if (root->left->color == BLACK)
    root->color = RED;     // ensure that the root is red before deleting the minimum, which allows us to maintain the
                           // properties of the Red-Black Tree during deletion.
  root = deleteMinRec(root);
  root->color = BLACK;
  return root;
}

/* DeleteMax */

/**
 * @brief Recursive method to delete the node with the maximum key in the subtree rooted at h. If the right child of h
 * is nil, then h is the maximum node, so we return nil to delete it. If the right child of h is black and its left
 * child is also black, we need to move a red link right to ensure that we are moving down a path with a red link, which
 * allows us to maintain the properties of the Red-Black Tree during deletion. After deleting the maximum node from the
 * right subtree, we balance the subtree rooted at h to maintain the properties of the Red-Black Tree.
 *
 * @param h The root of the subtree from which to delete the maximum node. Should not be nil.
 * @return The new root of the subtree after deleting the maximum node and balancing the tree.
 */
Node *deleteMaxRec(Node *h) {
  if (!h->left->isExternal && h->left->color == RED)
    h = rotateRight(h);     // If the left child of h is red, rotate right to ensure that we are moving down a path with a
                            // red link, which allows us to maintain the properties of the Red-Black Tree during deletion.
  if (h->right->isExternal)
    return h->left;                                                   // If the right child of h is nil, then h is the maximum
                                                                      // node, so we return it's left pointer (nil for nil, and useful for tango tree).
  if (h->right->color == BLACK && h->right->left->color == BLACK)     // Guarantee that the right child of h is not a
                                                                      // 2-node by moving a red link right if necessary.
    h = moveRedRight(h);
  h->right = deleteMaxRec(h->right);
  return balance(h);
}

Node *deleteMax(Node *root) {
  if (DEBUG)
    assert(root != Node::nil);     // Ensure that the root is not the nil node before deleting the maximum, since we
                                   // should never delete from an empty tree.
  if (root->left->color == BLACK)
    root->color = RED;
  root = deleteMaxRec(root);

  root->color = BLACK;
  return root;
}

/* DeleteNode. */

Node *deleteNode(Node *h, int key) {
  if (DEBUG)
    assert(search(h, key).first != Node::nil);     // Ensure that the key is present in the
                                                   // tree before attempting to delete it.

  auto [left, x, right] = split(h, key);
  // Join the left and right subtrees, excluding the node with the key.
  if (left->isExternal)
    return right;
  if (right->isExternal)
    return left;

  // Find a node to use as separator (use min from right), since the above test guarantee that both left and right trees
  // are not empty.
  int separator = min(right)->key;
  right = deleteMin(right);
  return join(left, newNode(separator), right);
}

/* ExtractMin. */
std::pair<Node *, Node *> extractMin(Node *root) {
  Node *minNode = min(root);
  root = deleteMin(root);
  return {root, minNode};
}

/* ExtractMax. */
std::pair<Node *, Node *> extractMax(Node *root) {
  Node *maxNode = max(root);
  root = deleteMax(root);
  return {root, maxNode};
}

/* Min. */

Node *min(Node *root) {
  while (!root->left->isExternal)
    root = root->left;     // Iteratively search for the minimum in the left subtree.

  return root;
}

/* Max. */

Node *max(Node *root) {
  while (!root->right->isExternal)
    root = root->right;     // Iteratively search for the maximum in the right subtree.

  return root;
}

/* Print. */

void print(Node *root, int indent) {
  if (root == Node::nil)
    return;
  print(root->right, indent + 4);
  if (indent)
    std::cout << std::string(indent, ' ');
  std::cout << '(' << root->key << ' ' << (root->color == RED ? "R" : "B") << ')' << "\n";
  print(root->left, indent + 4);
}

/* Update. */

void update(Node *h) {
  if (DEBUG)
    assert(!h->isExternal);     // Ensure that h is not an external node before updating, since we should never update a
                                // nil node or another tree root.

  // Update Red-Black Tree properties.
  h->blackHeight = std::max(h->left->blackHeight + (h->left->color == BLACK ? 1 : 0), h->right->blackHeight + 1);

  // Update Auxiliary tree properties.
  h->minDepth = std::min(std::min(h->left->isExternal ? SHORT_MAX : h->left->minDepth, h->right->isExternal ? SHORT_MAX : h->right->minDepth), h->depth);
  h->maxDepth = std::max(std::max(h->left->isExternal ? SHORT_MIN : h->left->maxDepth, h->right->isExternal ? SHORT_MIN : h->right->maxDepth), h->depth);
}

/* Auxiliary functions definitions */

/**
 * @brief Rotate a subtree to the left. This operation is used to maintain the balance of the Red-Black Tree. As a
 * precondition, the right child of h must be red.
 *
 * @param h The root of the subtree to rotate.
 * @return The new root of the subtree after rotation.
 * @note Time Complexity: O(1)
 */
Node *rotateLeft(Node *h) {
  if (DEBUG)
    assert(h->right->color == RED);     // Ensure that the right child of h is red before rotating left, if h is the nil node, this
                                        // assertion will fail, which is expected since we should never rotate left on a nil node.

  Node *x = h->right;      // Set x to the right child of h.
  h->right = x->left;      // Move x's left subtree to be h's right subtree.
  x->left = h;             // Make h the left child of x.
  x->color = h->color;     // Copy the color of h to x.
  h->color = RED;          // Set h's color to RED.
  update(h);               // Update the properties of h after rotation.
  update(x);               // Update the properties of x after rotation.
  return x;                // Return the new root of the subtree (x).
}

/**
 * @brief Rotate a subtree to the right. This operation is used to maintain the
 * balance of the Red-Black Tree. As a precondition, the left child of h must be
 * red.
 *
 * @param h The root of the subtree to rotate.
 * @return The new root of the subtree after rotation.
 * @note Time Complexity: O(1)
 */
Node *rotateRight(Node *h) {
  if (DEBUG)
    assert(h->left->color == RED);     // Ensure that the left child of h is red before rotating right,
                                       // if h is the nil node, this assertion will fail, which is
                                       // expected since we should never rotate right on a nil node.

  Node *x = h->left;       // Set x to the left child of h.
  h->left = x->right;      // Move x's right subtree to be h's left subtree.
  x->right = h;            // Make h the right child of x.
  x->color = h->color;     // Copy the color of h to x.
  h->color = RED;          // Set h's color to RED.
  update(h);               // Update the properties of h after rotation.
  update(x);               // Update the properties of x after rotation.
  return x;                // Return the new root of the subtree (x).
}

/**
 * @brief Flip the colors of a node and its children. This operation is used to
 * maintain the properties of the Red-Black Tree. As a precondition, the node h
 * must not be the nil node, and its color must be different from the colors of
 * its children.
 *
 * @param h The node whose colors are to be flipped.
 * @return The node after flipping colors.
 * @note Time Complexity: O(1)
 */
Node *flipColors(Node *h) {
  // assert(h->color != h->left->color &&
  //       h->color != h->right->color); // Ensure that h's color is different from its children's colors before
  //       flipping,
  // if h is the nil node, this assertion will fail, which is expected since we
  // should never flip colors on a nil node.

  Color c = h->color;                       // Save h's color.
  h->color = h->left->color;                // Flip h's color.
  h->left->color = h->right->color = c;     // flip h's children color.
  return h;
}

/**
 * @brief Balance the subtree rooted at node h. This function is called to fix local violations of the Red-Black Tree
 * properties.
 *s
 * @param h The root of the subtree to balance.
 * @return The new root of the subtree after balancing.
 * @note Time Complexity: O(1)
 */
Node *balance(Node *h) {
  if (h->right->color == RED)
    h = rotateLeft(h);     // fix right-leaning red link.
  if (h->left->color == RED && h->left->left->color == RED)
    h = rotateRight(h);     // fix two reds in a row on the left.
  if (h->left->color == RED && h->right->color == RED)
    h = flipColors(h);     // split 4-node.

  update(h);
  return h;
}

/**
 * @brief Move a red link left. This operation is used during deletion to ensure that the properties of the Red-Black
 * Tree are maintained. As a precondition, h must not be the nil node.
 *
 * @param h The node from which to move the red link left.
 * @return The node after moving the red link left.
 * @note Time Complexity: O(1)
 */
Node *moveRedLeft(Node *h) {
  if (DEBUG)
    assert(h != Node::nil);     // Ensure that h is not the nil node before moving red
                                // left, since we should never move red left on a nil node.

  h = flipColors(h);                      // Flip colors to prepare for moving red left.
  if (h->right->left->color == RED) {     // If the left child of the right child is
                                          // red, perform rotations and color flips.
    h->right = rotateRight(h->right);     // Rotate right on the right child.
    h = rotateLeft(h);                    // Rotate left on h.
    h = flipColors(h);                    // Flip colors to maintain Red-Black properties.
  }
  return h;
}

/**
 * @brief Move a red link right. This operation is used during deletion to ensure that the properties of the Red-Black
 * Tree are maintained. As a precondition, h must not be the nil node.
 *
 * @param h The node from which to move the red link right.
 * @return The node after moving the red link right.
 * @note Time Complexity: O(1)
 */
Node *moveRedRight(Node *h) {
  if (DEBUG)
    assert(h != Node::nil);     // Ensure that h is not the nil node before moving red right,
                                // since we should never move red right on a nil node.

  h = flipColors(h);                     // Flip colors to prepare for moving red right.
  if (h->left->left->color == RED) {     // If the left child of the left child is
                                         // red, perform rotations and color flips.
    h = rotateRight(h);                  // Rotate right on h.
    h = flipColors(h);                   // Flip colors to maintain Red-Black properties.
  }
  return h;
}

/**
 * @brief Detach the left and right subtrees from a given node. This function is used during the join operation to
 * prepare a node for joining by detaching its left and right subtrees and setting them to nil. As a precondition, h
 * must not be the nil node.
 *
 * @param h The node from which to detach the left and right subtrees. Should not be nil.
 * @return the detached left and right subtrees as a pair, where the first element is the left subtree and the second
 * element is the right subtree.
 * @note Time Complexity: O(1)
 */
std::pair<Node *, Node *> detach(Node *h) {
  Node *leftSubtree = h->left;            // Store the left subtree of h.
  Node *rightSubtree = h->right;          // Store the right subtree of h.
  h->left = h->right = Node::nil;         // Detach the left and right subtrees from h
                                          // by setting them to nil.
  h->color = BLACK;                       // Set h's color to BLACK.
  return {leftSubtree, rightSubtree};     // Return the detached left and right subtrees as a pair.
}

/**************************************************************
 * Functions for testing and debugging
 **************************************************************/

/**
 * @brief Validates if the Red-Black Tree (RBT) satisfies the Binary Search Tree (BST) property.
 * Ensures that for every node:
 * - All keys in the left subtree are strictly less than the node's key.
 * - All keys in the right subtree are strictly greater than the node's key.
 *
 * @param root The root of the subtree to validate.
 * @param min The lower bound for the current node's key.
 * @param max The upper bound for the current node's key.
 * @return true if the subtree is a valid BST; false otherwise.
 * @note Time Complexity: O(N), where N is the number of nodes in the subtree.
 */
bool isBST(Node *root, int min = INT_MIN, int max = INT_MAX) {
  if (root->isExternal)
    return true;
  if (root->key < min || root->key > max)
    return false;
  return isBST(root->left, min, root->key) && isBST(root->right, root->key, max);
}

/**
 * @brief Verifies the Black-Height property of the Red-Black Tree.
 * A subtree is considered balanced if every path from the current root
 * to a descendant leaf contains the same number of black nodes.
 *
 * @param root The root of the subtree to validate.
 * @return true if the black-height is consistent across all paths; false otherwise.
 * @note Time Complexity: O(N), where N is the number of nodes in the subtree.
 */
bool isBalanced(Node *root) {
  if (root->isExternal)
    return true;
  int leftBlackHeight = root->left->blackHeight;
  int rightBlackHeight = root->right->blackHeight;
  if (leftBlackHeight != rightBlackHeight)
    return false;
  return isBalanced(root->left) && isBalanced(root->right);
}

/**
 * @brief Validates the 2-3 tree structural invariants for the current subtree.
 * A valid Left-Leaning Red-Black Tree (LLRBT) must satisfy:
 * - No right-leaning red links (all red links must be left children).
 * - No two consecutive red links in any path (no "4-node" violations).
 *
 * @param root The root of the subtree to validate.
 * @return true if the subtree satisfies all LLRBT properties; false otherwise.
 * @note Time Complexity: O(N), where N is the total number of nodes in the subtree.
 */
bool is23(Node *root) {
  if (root->isExternal)
    return true;
  if ((root->color == RED && root->left->color == RED) || root->right->color == RED || (root->left->color == RED && root->right->color == RED))
    return false;
  return is23(root->left) && is23(root->right);
}

/**
 * @brief Performs a comprehensive validation of the Red-Black Tree (RBT) invariants.
 * This is a composite check that verifies the following structural and logical properties:
 * - Binary Search Tree (BST) property: Keys are correctly ordered.
 * - Balance property: Uniform black-height across all paths.
 * - 2-3 Tree (LLRBT) property: No right-leaning red links or consecutive red links.
 *
 * @param root The root of the subtree to validate.
 * @return true if all RBT invariants are satisfied; false otherwise.
 * @note Time Complexity: O(N), where N is the total number of nodes in the tree.
 */
bool check(Node *root) {
  if (!isBST(root))
    return false;
  if (!isBalanced(root))
    return false;
  if (!is23(root))
    return false;
  return true;
}