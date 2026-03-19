/**
 * @file RedBlackTree.cpp
 * @author Zawarudo (@zawarudo)
 * @version 0.1
 * @date 2026-03-14
 * @copyright Copyright (c) 2026
 *
 * Interface:
 *
 * 0 <id> <l> <r> - Builds a RBT with id <id> that contains the keys in (l, r).
 *
 * 1 <id> <k> - Inserts the key <k> in the tree with the id <id>.
 *
 * 2 <id> <k> - Checks if the key <k> is in the tree with the id <id>.
 *
 * 3 <id> <k> - Removes the key <k> from the tree wuth the id <id>.
 *
 * 4 <id1> <k> <id2> - Joins the tree with id <id1> and the tree with id <id2> using a new node with key <k> as
 * separator. The resulting tree is stored in the tree with id <id1>, while the tree with id <id2> is cleared.
 *
 * 5 <id> <k> - Splits the tree with id <id> using the key <k>. The resulting left subtree, node with key <k>, and right
 * subtree are printed to the output, and the original tree with id <id> is cleared.
 */

// includes.
#include "RedBlackTree.h"
#include <cassert>
#include <climits>
#include <iostream>
#include <map>

// flag for debugging, set to true to enable assert tests, false to disable.
#define DEBUG true

/* Headers of auxiliary functions. */

Node *balance(Node *h);

Node *rotateLeft(Node *h);

Node *rotateRight(Node *h);

Node *moveRedLeft(Node *h);

Node *moveRedRight(Node *h);

int max(Node *root);

int min(Node *root);

std::pair<Node *, Node *> detach(Node *h);

/* NewNode. */

Node *newNode(int key, int depth) {
  Node *x = new Node(key);                      // create new node.
  x->left = x->right = Node::nil;               // initialize children to nil.
  x->isExternal = false;                        // set as an internal node.
  x->blackHeight = 1;                           // default black height for a new node.
  x->depth = x->minDepth = x->maxDepth = depth; // set the depth of the new node.
  return x;
}

/* Search */

Node *search(Node *h, int key) {
  if (h->isExternal || h->key == key)
    return h; // If the node is external or the key matches, return the node.
  if (key < h->key)
    return search(h->left, key); // If the key is less than the current node's
                                 // key, search in the left subtree.
  else
    return search(h->right, key); // If the key is greater than the current
                                  // node's key, search in the right subtree.
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
                              rightTree); // If the left tree has a greater black height, join the
                                          // right subtree of the left tree with x and the right tree.
    return balance(leftTree);
  }
  if (leftTree->blackHeight < rightTree->blackHeight) {
    rightTree->left = joinRec(leftTree, x,
                              rightTree->left); // If the right tree has a greater black height, join the
                                                // left tree with x and the left subtree of the right tree.
    return balance(rightTree);
  }
  x->left = leftTree;
  x->right = rightTree;
  x->color = RED;
  return balance(x);
}

Node *join(Node *leftTree, Node *x, Node *rightTree) {
  if (DEBUG)
    assert(max(leftTree) < x->key &&
           x->key < min(rightTree)); // Ensure that all keys in the left tree are less
                                     // than x's key and all keys in the right tree are
                                     // greater than x's key before joining. This test has a cost greater than the join
                                     // operation itself, but it is useful for debugging and ensuring the correctness of
                                     // the join operation.

  detach(x); // Detach x from its current position in the tree to prepare for
             // joining.

  Node *y = joinRec(leftTree, x,
                    rightTree); // Join the left tree, the node x, and the right
                                // tree using the recursive join function.

  y->color = BLACK; // Ensure the new root of the joined tree is black.

  return y; // Return the new root of the joined tree.
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
  if (h->key > key) {
    auto [left, x, right] = splitRec(h->left, key);
    h->right->color = BLACK; // Ensure the right child of h is black before joining.
    return {left, x, join(right, h, h->right)};
  }
  if (h->key < key) {
    auto [left, x, right] = splitRec(h->right, key);
    h->left->color = BLACK; // Ensure the left child of h is black before joining.
    return {join(h->left, h, left), x, right};
  }

  auto [left, right] = detach(h);
  left->color = right->color = BLACK; // Ensure the detached left and right
                                      // subtrees are black before returning.

  return {left, h, right}; // Return the left subtree, the node with the given
                           // key, and the right subtree as a tuple.
}

std::tuple<Node *, Node *, Node *> split(Node *h, int key) {
  if (DEBUG)
    assert(search(h, key) != Node::nil); // Ensure that the key is present in the tree before splitting.

  return splitRec(h, key); // Split the tree using the recursive split function.
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
    return Node::nil; // If the left child of h is nil, then h is the minimum
                      // node, so we return nil to delete it.
  if (h->left->color == BLACK &&
      h->left->left->color == BLACK) // Garantee that the left child of h is not a 2-node by moving
    // a red link left if necessary.
    h = moveRedLeft(h);
  h->left = deleteMinRec(h->left);
  return balance(h);
}

Node *deleteMin(Node *root) {
  if (DEBUG)
    assert(root != Node::nil); // Ensure that the root is not the nil node before
                               // deleting the minimum, since we should never
                               // delete from an empty tree.
  if (root->left->color == BLACK)
    root->color = RED; // ensure that the root is red before deleting the minimum, which allows us to maintain the
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
  if (h->left->color == RED)
    h = rotateRight(h); // If the left child of h is red, rotate right to ensure that we are
                        // moving down a path with a red link, which allows us to maintain
                        // the properties of the Red-Black Tree during deletion.
  if (h->right->isExternal)
    return Node::nil; // If the right child of h is nil, then h is the maximum
                      // node, so we return nil to delete it.
  if (h->right->color == BLACK && h->right->left->color == BLACK) // Garantee that the right child of h is not a 2-node
                                                                  // by moving a red link right if necessary.
    h = moveRedRight(h);
  h->right = deleteMaxRec(h->right);
  return balance(h);
}

Node *deleteMax(Node *root) {
  if (DEBUG)
    assert(root != Node::nil); // Ensure that the root is not the nil node before
                               // deleting the maximum, since we should never
                               // delete from an empty tree.
  if (root->left->color == BLACK)
    root->color = BLACK;
  root = deleteMaxRec(root);

  root->color = BLACK;
  return root;
}

/* Min. */

int min(Node *root) {
  while (!root->left->isExternal)
    root = root->left; // Iteratively search for the minimum in the left subtree.

  return root->key;
}

/* Max. */

int max(Node *root) {
  while (!root->right->isExternal)
    root = root->right; // Iteratively search for the maximum in the right subtree.

  return root->key;
}


/* Auxiliary functions definitions */

/**
 * @brief Updates the infos of the given node based on its children. This
 * function should be called after any modification to the subtree rooted at the
 * given node to ensure that the properties of the Red-Black Tree and the
 * auxiliary tree are maintained.
 *
 * @param h The node whose infos are to be updated.
 */
void update(Node *h) {
  if (DEBUG)
    assert(!h->isExternal); // Ensure that h is not an external node before updating, since we should never update a nil
                            // node or another tree root.
  
  // Update Red-Black Tree properties.
  h->blackHeight = std::max(h->left->blackHeight + h->left->color == BLACK ? 1 : 0, h->right->blackHeight + 1);

  // Update Auxiliary tree properties.
  h->minDepth = std::min(std::min(h->left->minDepth, h->right->minDepth), h->depth);
  h->maxDepth = std::max(std::max(h->left->maxDepth, h->right->maxDepth), h->depth);
}

/**
 * @brief Rotate a subtree to the left. This operation is used to maintain the
 * balance of the Red-Black Tree. As a precondition, the right child of h must
 * be red.
 *
 * @param h The root of the subtree to rotate.
 * @return The new root of the subtree after rotation.
 */
Node *rotateLeft(Node *h) {
  if (DEBUG)
    assert(h->right->color == RED); // Ensure that the right child of h is red before rotating left,
                                    // if h is the nil node, this assertion will fail, which is
                                    // expected since we should never rotate left on a nil node.

  Node *x = h->right;  // Set x to the right child of h.
  h->right = x->left;  // Move x's left subtree to be h's right subtree.
  x->left = h;         // Make h the left child of x.
  x->color = h->color; // Copy the color of h to x.
  h->color = RED;      // Set h's color to RED.
  update(h);           // Update the properties of h after rotation.
  update(x);           // Update the properties of x after rotation.
  return x;            // Return the new root of the subtree (x).
}

/**
 * @brief Rotate a subtree to the right. This operation is used to maintain the
 * balance of the Red-Black Tree. As a precondition, the left child of h must be
 * red.
 *
 * @param h The root of the subtree to rotate.
 * @return The new root of the subtree after rotation.
 */
Node *rotateRight(Node *h) {
  if (DEBUG)
    assert(h->left->color == RED); // Ensure that the left child of h is red before rotating right,
                                   // if h is the nil node, this assertion will fail, which is
                                   // expected since we should never rotate right on a nil node.

  Node *x = h->left;   // Set x to the left child of h.
  h->left = x->right;  // Move x's right subtree to be h's left subtree.
  x->right = h;        // Make h the right child of x.
  x->color = h->color; // Copy the color of h to x.
  h->color = RED;      // Set h's color to RED.
  update(h);           // Update the properties of h after rotation.
  update(x);           // Update the properties of x after rotation.
  return x;            // Return the new root of the subtree (x).
}

/**
 * @brief Flip the colors of a node and its children. This operation is used to
 * maintain the properties of the Red-Black Tree. As a precondition, the node h
 * must not be the nil node, and its color must be different from the colors of
 * its children.
 *
 * @param h The node whose colors are to be flipped.
 * @return The node after flipping colors.
 */
Node *flipColors(Node *h) {
  assert(h->color != h->left->color && h->color != h->right->color); // Ensure that h's color is different
                                                                     // from its children's colors before flipping, if
                                                                     // h is the nil node, this assertion will fail,
                                                                     // which is expected since we should never flip
                                                                     // colors on a nil node.

  Color c = h->color;                   // Save h's color.
  h->color = h->left->color;            // Flip h's color.
  h->left->color = h->right->color = c; // flip h's children color.
  return h;
}

/**
 * @brief Balance the subtree rooted at node h. This function is called to fix
 * local violations of the Red-Black Tree properties.
 *
 * @param h The root of the subtree to balance.
 * @return The new root of the subtree after balancing.
 */
Node *balance(Node *h) {
  if (h->right->color == RED)
    h = rotateLeft(h); // fix right-leaning red link.
  if (h->left->color == RED && h->left->left->color == RED)
    h = rotateRight(h); // fix two reds in a row on the left.
  if (h->left->color == RED && h->right->color == RED)
    h = flipColors(h); // split 4-node.
  return h;
}

/**
 * @brief Move a red link left. This operation is used during deletion to ensure
 * that the properties of the Red-Black Tree are maintained. As a precondition,
 * h must not be the nil node.
 *
 * @param h The node from which to move the red link left.
 * @return The node after moving the red link left.
 */
Node *moveRedLeft(Node *h) {
  if (DEBUG)
    assert(h != Node::nil); // Ensure that h is not the nil node before moving red
                            // left, since we should never move red left on a nil node.

  h = flipColors(h);                  // Flip colors to prepare for moving red left.
  if (h->right->left->color == RED) { // If the left child of the right child is
                                      // red, perform rotations and color flips.
    h->right = rotateRight(h->right); // Rotate right on the right child.
    h = rotateLeft(h);                // Rotate left on h.
    h = flipColors(h);                // Flip colors to maintain Red-Black properties.
  }
  return h;
}

/**
 * @brief Move a red link right. This operation is used during deletion to
 * ensure that the properties of the Red-Black Tree are maintained. As a
 * precondition, h must not be the nil node.
 *
 * @param h The node from which to move the red link right.
 * @return The node after moving the red link right.
 */
Node *moveRedRight(Node *h) {
  if (DEBUG)
    assert(h != Node::nil); // Ensure that h is not the nil node before moving red right,
                            // since we should never move red right on a nil node.

  h = flipColors(h);                  // Flip colors to prepare for moving red right.
  if (h->left->right->color == RED) { // If the right child of the left child is
                                      // red, perform rotations and color flips.
    h->left = rotateLeft(h->left);    // Rotate left on the left child.
    h = rotateRight(h);               // Rotate right on h.
    h = flipColors(h);                // Flip colors to maintain Red-Black properties.
  }
  return h;
}

std::pair<Node *, Node *> detach(Node *h) {
  Node *leftSubtree = h->left;        // Store the left subtree of h.
  Node *rightSubtree = h->right;      // Store the right subtree of h.
  h->left = h->right = Node::nil;     // Detach the left and right subtrees from h
                                      // by setting them to nil.
  h->color = BLACK;                   // Set h's color to BLACK.
  return {leftSubtree, rightSubtree}; // Return the detached left and right subtrees as a pair.
}

/**************************************************************
 * Functions for testing and debugging
 **************************************************************/

Node *insertRec(Node *h, int key) {
  if (h == Node::nil)
    return newNode(key); // If we reach a nil node, we create and return a new
                         // node with the given key.
  if (key < h->key)
    h->left = insertRec(h->left, key); // If the key is less than the current node's
                                       // key, insert into the left subtree.
  else if (key > h->key)
    h->right = insertRec(h->right, key); // If the key is greater than the current
                                         // node's key, insert into the right subtree.
  else
    return h;        // If the key is equal to the current node's key, we do not insert
                     // duplicates and simply return the current node.
  return balance(h); // After insertion, we balance the subtree rooted at h to
                     // maintain the properties of the Red-Black Tree.
}

Node *insert(Node *root, int key) {
  root = insertRec(root,
                   key); // Insert the key into the tree using the recursive insert function
  root->color = BLACK;   // Ensure the root of the tree is always black after insertion.
  return root;
}

/**
 * @brief Delete a node with a given key from the Red-Black Tree, while
 * maintaining the properties of the tree. As a precondition, the key must be
 * present in the tree. This functions uses the split and join operations to
 * remove the node with the given key and then join the resulting left and right
 * subtrees back together, showing the power of these operations in maintaining
 * the structure of the Red-Black Tree during deletion.
 *
 * @param h The root of the tree
 * @param key The key to delete
 * @return The new root of the tree after deletion
 */
Node *deleteNode(Node *h, int key) {
  if (DEBUG)
    assert(search(h, key) != Node::nil); // Ensure that the key is present in the
                                         // tree before attempting to delete it.

  auto [left, x, right] = split(h, key);
  // Join the left and right subtrees, excluding the node with the key.
  if (left == Node::nil)
    return right;
  if (right == Node::nil)
    return left;

  // Find a node to use as separator (use min from right).
  int separator = min(right);
  right = deleteMin(right);
  return join(left, newNode(separator), right);
}


void print(Node *root, int indent = 0) {
  if (root == Node::nil)
    return;
  print(root->right, indent + 4);
  if (indent)
    std::cout << std::string(indent, ' ');
  std::cout << '(' << root->key << ' ' << (root->color == RED ? "R" : "B") << ')' << "\n";
  print(root->left, indent + 4);
}

/**
 * @brief Check if the Red-Black Tree rooted at the given node is a valid binary
 * search tree. A binary search tree is valid if for every node, all keys in the
 * left subtree are less than the node's key, and all keys in the right subtree
 * are greater than the node's key.
 *
 * @param root The root of the subtree to check for the binary search tree
 * property.
 * @param min The minimum key value allowed for the current subtree (used for
 * recursive checks).
 * @param max The maximum key value allowed for the current subtree (used for
 * recursive checks).
 * @return true if the subtree is a valid binary search tree, false otherwise.
 */
bool isBST(Node *root, int min = INT_MIN, int max = INT_MAX) {
  if (root->isExternal)
    return true;
  if (root->key < min || root->key > max)
    return false;
  return isBST(root->left, min, root->key) && isBST(root->right, root->key, max);
}

/**
 * @brief Check if the Red-Black Tree rooted at the given node is balanced. A
 * Red-Black Tree is balanced if every path from the root to a leaf has the same
 * number of black nodes.
 *
 * @param root The root of the subtree to check for balance.
 * @return true if the subtree is balanced, false otherwise.
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
 * @brief Check if the Red-Black Tree rooted at the given node is a valid 2-3
 * tree. A Red-Black Tree is a valid 2-3 tree if it does not contain any
 * right-leaning red links and does not contain two red links in a row.
 *
 * @param root The root of the subtree to check for the 2-3 tree property.
 * @return true if the subtree is a valid 2-3 tree, false otherwise.
 */
bool is23(Node *root) {
  if (root->isExternal)
    return true;
  if ((root->color == RED && root->left->color == RED) || root->right->color == RED)
    return false;
  return is23(root->left) && is23(root->right);
}

/**
 * @brief Check if the Red-Black Tree rooted at the given node satisfies all the
 * properties of a valid Red-Black Tree, including being a binary search tree,
 * being balanced, and being a valid 2-3 tree.
 *
 * @param root The root of the subtree to check for validity.
 * @return true if the subtree is a valid Red-Black Tree, false otherwise.
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

/* Main test. */

int main() {
  std::map<int, Node *> trees; // Map to store trees by ID.

  int operation, id, val, id2, key;

  while (std::cin >> operation) {
    switch (operation) {
    case 0: {
      int l, r;
      std::cin >> id >> l >> r;
      if (trees.count(id) == 0)
        trees[id] = Node::nil;

      for (int i = l; i <= r; i++)
        trees[id] = insert(trees[id], i);

      break;
    }
    case 1: { // Insert.
      std::cin >> id >> val;
      if (trees.count(id) == 0)
        trees[id] = Node::nil;

      trees[id] = insert(trees[id], val);
      check(trees[id]); // Check the validity of the tree after insertion.

      if (DEBUG)
        std::cout << "Inserted " << val << " into tree " << id << std::endl;

      break;
    }
    case 2: { // Contains.
      std::cin >> id >> val;
      Node *tree = (trees.count(id) > 0) ? trees[id] : Node::nil;
      bool found = (search(tree, val) != Node::nil);
      std::cout << (found ? "True" : "False") << std::endl;
      break;
    }
    case 3: { // Delete.
      std::cin >> id >> val;
      if (trees.count(id) > 0) {
        trees[id] = deleteNode(trees[id], val);
        check(trees[id]); // Check the validity of the tree after deletion.
        if (DEBUG)
          std::cout << "Deleted " << val << " from tree " << id << std::endl;
      } else {
        std::cout << "Invalid ID" << std::endl;
      }
      break;
    }
    case 4: { // Join.
      int id1;
      std::cin >> id1 >> key >> id2;
      Node *t1 = (trees.count(id1) > 0) ? trees[id1] : Node::nil;
      Node *t2 = (trees.count(id2) > 0) ? trees[id2] : Node::nil;
      Node *separator = newNode(key);
      Node *result = join(t1, separator, t2);
      check(result);          // Check the validity of the joined tree.
      trees[id1] = result;    // Store result in first tree's ID.
      trees[id2] = Node::nil; // Clear second tree's ID.
      if (DEBUG) {
        std::cout << "Joined tree " << id1 << " and tree " << id2 << " with separator " << key << std::endl;
        print(result); // Print the resulting tree after join.
      }
      break;
    }
    case 5: { // Split.
      std::cin >> id >> key;
      if (trees.count(id) > 0) {
        auto [left, x, right] = split(trees[id], key);
        std::cout << "-------- L --------" << std::endl;
        print(left);
        std::cout << "-------- x --------" << std::endl;
        print(x);
        std::cout << "-------- R --------" << std::endl;
        print(right);
        std::cout << "-------------------" << std::endl;
        // Optionally store the split results.
        trees[id] = Node::nil; // Clear original tree's ID.
        check(left);           // Check the validity of the left subtree.
        assert(x->key == key); // Check the validity of the node with the given key.
        check(right);          // Check the validity of the right subtree.
      } else {
        std::cout << "Invalid ID" << std::endl;
      }
      break;
    }
    case 6: { // Print.
      std::cin >> id;
      if (trees.count(id) > 0) {
        print(trees[id]);
      } else {
        std::cout << "Invalid ID" << std::endl;
      }
      break;
    }
    default:
      std::cout << "Invalid Operation" << std::endl;
      break;
    }
  }

  return 0;
}