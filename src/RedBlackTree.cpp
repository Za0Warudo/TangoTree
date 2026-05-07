/**
 * @file RedBlackTree.cpp
 * @author Zawarudo (@zawarudo)
 * @version 2.0
 * @date 2026-03-14
 * @copyright Copyright (c) 2026
 *
 * Implementation of the red-black tree data structure defined in RedBlackTree.h. The implementation includes the basic operations of a red-black tree, such as
 * insertion, deletion of the min and max nodes, searching for a key, and the auxiliary operations join and split. This implementation is designed to be clear
 * and easy to understanding for the context the tango tree use.
 */

// includes.
#include "RedBlackTree.h"
#include <climits>
#include <iostream>

/**
 * @brief Defines the nil node. The nil node is a special node that represents the absence of a child in the red-black tree. It is used as a sentinel pointer.
 * The nil node is always black and external. Its key is not used (default -1 for this case). The left and right pointers of the nil node point to itself,
 * creating a cycle. The depth and black height of the nil node are set to -1. The max and min depth are set to SHORT_MIN and SHORT_MAX respectively.
 */
Node *Node::nil = [] {
  Node *n = new Node(-1);      // Create a new node with any key.
  n->color = BLACK;            // The nil node is always black.
  n->left = n->right = n;      // The left and right pointers of the nil node point to itself.
  n->isExternal = true;        // The nil node is an external node.
  n->depth = -1;               // The depth of the nil node is set to -1.
  n->minDepth = SHORT_MAX;     // The minimum depth of the nil node is set to the maximum possible value, since it is an external node and has no children.
  n->maxDepth = SHORT_MIN;     // The maximum depth of the nil node is set to the minimum possible value, since it is an external node and has no children.
  n->blackHeight = -1;         // The height of the nil node is set to -1, it's the only node with height -1.

  return n;     // Return the pointer to the initialized nil node.
}();     // Initialize the nil node.

/* Headers of auxiliary functions. */

Node *balance(Node *h);
Node *rotateLeft(Node *h);
Node *rotateRight(Node *h);
Node *moveRedLeft(Node *h);
Node *moveRedRight(Node *h);
std::pair<Node *, Node *> detach(Node *h);

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

/**
 * @brief Recursive method to search a key into the given subtree. If an external node is reached or found the node with the given key return it and its parent.
 * If the key is smaller than the current node's key, search into the left subtree; if it is greater, search into the right subtree.
 *
 * @param h The subtree root.
 * @param key The key to be search into the subtree.
 * @return A pointer to the node and to its parent if found, or an external node and the last visited node otherwise.
 */
std::pair<Node *, Node *> searchRec(Node *h, int key, Node *p = Node::nil) {
  if (h->isExternal || h->key == key)
    return {h, p};     // If the node is external or the key matches, return the node and its parent.
  if (key < h->key)
    return searchRec(h->left, key, h);     // If the key is smaller than the current node's key, search in the left subtree.
  else
    return searchRec(h->right, key, h);     // If the key is greater than the current node's key, search in the right subtree.
}

std::pair<Node *, Node *> search(Node *h, int key) { return searchRec(h, key); }

/* Insert. */

/**
 * @brief Recursive method to insert a key into the given subtree. If an external node is reached, creates and return a new node with the given key. If the key
 * is smaller than the current node's key, we insert into the left subtree; if it is greater, we insert into the right subtree. After inserting, balances the
 * subtree rooted at h to maintain the properties of the red-black tree.
 *
 * @param h The subtree root.
 * @param key The key to insert into the subtree.
 * @return The new root of the subtree after the insertion.
 */
Node *insertRec(Node *h, int key) {
  if (h->isExternal)
    return newNode(key);     // Reach the nil node, creates and return a new node with the given key.
  if (key < h->key)
    h->left = insertRec(h->left, key);     // If the key is smaller than the current node's key, insert into the left subtree.
  else if (key > h->key)
    h->right = insertRec(h->right, key);     // If the key is greater than the current node's key, insert into the right subtree.
  else
    return h;            // If the key is equal to the current node's key, do not insert duplicates and simply return the current node.
  return balance(h);     // After insertion, balances the subtree rooted at h to maintain the properties of the Red-Black Tree.
}

Node *insert(Node *root, int key) {
  root = insertRec(root, key);     // Insert the key into the tree using the recursive function.
  root->color = BLACK;             // Ensure that the tree root is always black after insertion.
  return root;
}

/* Join. */

/**
 * @brief Recursive join operation method. If the left tree has a greater black height, apply the join recursively on the right subtree of the left tree. If
 * the right tree has a greater black height, apply the join recursively on the left subtree of the right tree. If both trees have the same black height, join
 * them directly with x as the root node.
 *
 * @param leftTree The left subtree. Should contain only keys smaller than x's key.
 * @param x The node to use as the middle node for the join. Should have a key value between the maximum key of the left tree and the minimum key of the right
 * tree.
 * @param rightTree The right subtree. Should contain only keys greater than x's key.
 * @return The new root of the joined tree after applying the join operation.
 */
Node *joinRec(Node *leftTree, Node *x, Node *rightTree) {
  if (leftTree->blackHeight > rightTree->blackHeight) {
    leftTree->right = joinRec(
        leftTree->right, x, rightTree);     // If the left tree has a greater black height, joins the right subtree of the left tree with x and the right tree.
    return balance(leftTree);
  }
  if (leftTree->blackHeight < rightTree->blackHeight) {
    rightTree->left = joinRec(
        leftTree, x, rightTree->left);     // If the right tree has a greater black height, joins the left subtree of the right tree with x and the left tree.
    return balance(rightTree);
  }
  x->left = leftTree;
  x->right = rightTree;
  x->color = RED;
  return balance(x);
}

Node *join(Node *leftTree, Node *x, Node *rightTree) {
  detach(x);                                     // Ensure x pointers is assignable.
  Node *y = joinRec(leftTree, x, rightTree);     // Join the left tree, the node x, and the right tree using the recursive function.
  y->color = BLACK;                              // Ensure the new root of the joined tree is black.

  return y;     // Return the new root of the joined tree.
}

/* Split. */

/**
 * @brief Recursive split method. If the key is less than the current node's key, apply the split recursively on the left subtree. If the key is greater than
 * the current node's key, apply the split recursively on the right subtree. If the key matches the current node's key, detach the left and right subtrees from
 * the current node and return them, with the current node as the separator.
 *
 * @param h The subtree to split. Should contain the key to split around.
 * @param key The key to split around. Should be present in the subtree rooted at h.
 * @return A tuple containing the left tree (keys smaller than the given key), the node with the given key and the right tree (keys greater than the
 * given key) after applying the split operation.
 */
std::tuple<Node *, Node *, Node *> splitRec(Node *h, int key) {
  if (key < h->key) {
    auto [left, x, right] = splitRec(h->left, key);
    return {left, x, join(right, h, h->right)};
  }
  if (key > h->key) {
    auto [left, x, right] = splitRec(h->right, key);
    h->left->color = BLACK;     // Ensure the left child of h is black before joining.
    return {join(h->left, h, left), x, right};
  }

  auto [left, right] = detach(h);
  left->color = BLACK;         // Ensure that he left root subtree is black.
  return {left, h, right};     // Return the left subtree, the node with the given key, and the right subtree as a tuple.
}

std::tuple<Node *, Node *, Node *> split(Node *h, int key) {
  auto [l, x, r] = splitRec(h, key);     // Split the tree using the recursive function
  return {l, x, r};
}

/* DeleteMin */

/**
 * @brief Recursive method to delete the min node in the given subtree. If the left child of h is nil, then h is the min node, so we return
 * nil to delete it. If the left child of h is black and its left child is also black, moves a red link left to ensure that a path with red kink is moving down,
 * allowing to maintain the balance property.
 *
 * @param h The substree root. Should not be nil.
 * @return The new root of the subtree after deleting the minimum node.
 */
std::pair<Node *, Node *> deleteMinRec(Node *h) {
  if (h->left->isExternal)
    return {h, h->right};     // If the left child of h is nil, then h is the minimum node, so we return its right pointer (nil)
  if (h->left->color == BLACK &&
      h->left->left->color == BLACK)     // Guarantee that the left child of h is not a 2-node by moving a red link left if necessary.
    h = moveRedLeft(h);
  auto [min, x] = deleteMinRec(h->left);
  h->left = x;
  return {min, balance(h)};
}

std::pair<Node *, Node *> deleteMin(Node *root) {
  if (root->left->color == BLACK)
    root->color = RED;     // ensure that the root or the left children is red before deleting the minimum
  auto [min, h] = deleteMinRec(root);
  h->color = BLACK;
  return {min, h};
}

/* DeleteMax */

/**
 * @brief Recursive method to delete the max node in the subtree rooted at h. If the right child of h is nil, then h is the maximum node, so we return nil to
 * delete it. If the right child of h is black and its left child is also black, we need to move a red link to the right to ensure that we are moving down a
 * path with a red link. this allows to maintain the balance property during deletion.
 *
 * @param h The subtre root. Should not be nil.
 * @return The new root of the subtree after deleting the maximum node.
 */
std::pair<Node *, Node *> deleteMaxRec(Node *h) {
  if (!h->left->isExternal && h->left->color == RED)
    h = rotateRight(h);     // If the left child of h is red, rotate right to ensure that we are moving down a path with a red link, which allows us to
                            // maintain the balnce of the red-black tree during deletion.
  if (h->right->isExternal)
    return {h, h->left};     // If the right child of h is nil, then h is the maximum  node, so we return its left pointer (nil).
  if (h->right->color == BLACK &&
      h->right->left->color == BLACK)     // Guarantee that the right child of h is not a 2-node by moving a red link right if necessary.
    h = moveRedRight(h);
  auto [max, x] = deleteMaxRec(h->right);
  h->right = x;
  return {max, balance(h)};
}

std::pair<Node *, Node *> deleteMax(Node *root) {
  if (root->left->color == BLACK)
    root->color = RED;
  auto [max, h] = deleteMaxRec(root);
  h->color = BLACK;
  return {max, h};
}

/* Min. */

Node *min(Node *root) {
  while (!root->left->isExternal)
    root = root->left;     // Iteratively search for the min node in the left subtree.

  return root;
}

/* Max. */

Node *max(Node *root) {
  while (!root->right->isExternal)
    root = root->right;     // Iteratively search for the max node in the right subtree.

  return root;
}

/* Print. */

void printRec(Node *root, int indent, int step) {
  if (root != Node::nil) {
    printRec(root->right, indent + step, step);
    std::cout << std::string(indent, ' ') << '(' << root->key << ')' << "\n";
    printRec(root->left, indent + step, step);
  }
}

void print(Node *root) { return printRec(root, 0, 4); }

/* Update. */

void update(Node *h) {

  if (h->isExternal) {
    h->blackHeight = -1;
    h->minDepth = SHORT_MAX;
    h->maxDepth = SHORT_MIN;
  } else {
    h->blackHeight = std::max(h->left->blackHeight + (h->left->color == BLACK ? 1 : 0), h->right->blackHeight + 1);
    h->minDepth = std::min(std::min(h->left->isExternal ? SHORT_MAX : h->left->minDepth, h->right->isExternal ? SHORT_MAX : h->right->minDepth), h->depth);
    h->maxDepth = std::max(std::max(h->left->isExternal ? SHORT_MIN : h->left->maxDepth, h->right->isExternal ? SHORT_MIN : h->right->maxDepth), h->depth);
  }
}

/* Auxiliary functions definitions. */

/**
 * @brief Rotate a subtree to the left. This operation is used to maintain the balance of the red-black tree. As a precondition, the right child of h must be
 * red.
 *
 * @param h The root of the subtree to rotate.
 * @return The new root of the subtree after rotation.
 * @note Time Complexity: O(1)
 */
Node *rotateLeft(Node *h) {
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
 * @brief Rotate a subtree to the right. This operation is used to maintain the balance of the red-black tree. As a precondition, the left child of h must be
 * red.
 *
 * @param h The subtree root.
 * @return The new root of the subtree after rotation.
 * @note Time Complexity: O(1)
 */
Node *rotateRight(Node *h) {
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
 * @brief Flip the colors of the given node and its children. This operation is used to maintain balance property of a red-black tree. As a precondition, the
 * node h must not be the nil node, and its color must be different from its children colors.
 *
 * @param h The node.
 * @return The node after flipping colors.
 * @note Time Complexity: O(1)
 */
Node *flipColors(Node *h) {
  Color c = h->color;                       // Save h's color.
  h->color = h->left->color;                // Flip h's color.
  h->left->color = h->right->color = c;     // flip h's children color.
  return h;
}

/**
 * @brief Balance the given subtree. This function is called to fix local red-black tree properties violations.
 *
 * @param h The subtree root.
 * @return The new subtree root after the balance.
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
 * @brief Move a red link to the left. This operation is used during deletion to ensure that the properties of the red-black
 * tree are maintained. As a precondition, h must not be the nil node.
 *
 * @param h The subtree node.
 * @return The node after moving the red link left.
 * @note Time Complexity: O(1)
 */
Node *moveRedLeft(Node *h) {
  h = flipColors(h);                      // Flip colors to prepare for moving red left.
  if (h->right->left->color == RED) {     // If the left child of the right child is red, perform rotations and color flips.
    h->right = rotateRight(h->right);     // Rotate right on the right child.
    h = rotateLeft(h);                    // Rotate left on h.
    h = flipColors(h);                    // Flip colors to maintain Red-Black properties.
  }
  return h;
}

/**
 * @brief Move a red link to the right. This operation is used during deletion to ensure that the properties of the Red-Black
 * Tree are maintained. As a precondition, h must not be the nil node.
 *
 * @param h The subtree node.
 * @return The node after moving the red link right.
 * @note Time Complexity: O(1)
 */
Node *moveRedRight(Node *h) {
  h = flipColors(h);                     // Flip colors to prepare for moving red right.
  if (h->left->left->color == RED) {     // If the left child of the left child is red, perform rotations and color flips.
    h = rotateRight(h);                  // Rotate right on h.
    h = flipColors(h);                   // Flip colors to maintain Red-Black properties.
  }
  return h;
}

/**
 * @brief Detach the left and right subtrees from the given node.
 *
 * @param h The node. Should not be nil.
 * @return the detached left and right subtrees as a pair.
 * @note Time Complexity: O(1)
 */
std::pair<Node *, Node *> detach(Node *h) {
  Node *leftSubtree = h->left;            // Store the left subtree of h.
  Node *rightSubtree = h->right;          // Store the right subtree of h.
  h->left = h->right = Node::nil;         // Detach the left and right subtrees from h by setting them to nil.
  h->color = BLACK;                       // Set h's color to BLACK.
  return {leftSubtree, rightSubtree};     // Return the detached left and right subtrees as a pair.
}