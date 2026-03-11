#include "RedBlackTree.h"
#include <cassert>
#include <climits>
#include <iostream>

/**************************************************************
 * Axuliary functions definitions
 *************************************************************/

/**
 * @brief Rotate a subtree to the left. This operation is used to maintain the
 * balance of the Red-Black Tree.
 * @param h The root of the subtree to rotate.
 * @return The new root of the subtree after rotation.
 */
Node *rotateLeft(Node *h);

/**
 * @brief Rotate a subtree to the right. This operation is used to maintain the
 * balance of the Red-Black Tree.
 * @param h The root of the subtree to rotate.
 * @return The new root of the subtree after rotation.
 */
Node *rotateRight(Node *h);

/**
 * @brief Flip the colors of a node and its children. This operation is used to
 * maintain the properties of the Red-Black Tree.
 * @param h The node whose colors are to be flipped.
 * @return The node after flipping colors.
 */
Node *flipColors(Node *h);

/**
 * @brief Balance the subtree rooted at node h. This function is called to fix
 * local violations of the Red-Black Tree properties.
 * @param h The root of the subtree to balance.
 * @return The new root of the subtree after balancing.
 */
Node *balance(Node *h);

/**
 * @brief Move a red link left. This operation is used during deletion to ensure
 * that the properties of the Red-Black Tree are maintained.
 * @param h The node from which to move the red link left.
 * @return The node after moving the red link left.
 */
Node *moveRedLeft(Node *h);

/**
 * @brief Move a red link right. This operation is used during deletion to
 * ensure that the properties of the Red-Black Tree are maintained.
 * @param h The node from which to move the red link right.
 * @return The node after moving the red link right.
 */
Node *moveRedRight(Node *h);

/**
 * @brief Return the minimum key value in the subtree rooted at the given node.
 * If the subtree is empty, return a large value to indicate no minimum.
 * @param root The root of the subtree to search.
 * @return int The minimum key value in the subtree.
 */
int min(Node *root);

/**
 * @brief Return the maximum key value in the subtree rooted at the given node.
 * If the subtree is empty, return a small value to indicate no maximum.
 * @param root The root of the subtree to search.
 * @return int The maximum key value in the subtree.
 */
int max(Node *root);

/**************************************************************
 * Implementation of the main operations of the Red-Black Tree
 *************************************************************/

Node *newNode(int key) {
  Node *x = new Node(key);        // create new node
  x->left = x->right = Node::nil; // initialize children to nil
  x->isExternal = false;          // set as an internal node
  x->blackHeight = 1;             // default black height for a new node
  return x;
}

Node *rotateLeft(Node *h) {
  assert(h->right->color ==
         RED); // Ensure that the right child of h is red before rotating left,
               // if h is the nil node, this assertion will fail, which is
               // expected since we should never rotate left on a nil node
  Node *x = h->right;  // Set x to the right child of h
  h->right = x->left;  // Move x's left subtree to be h's right subtree
  x->left = h;         // Make h the left child of x
  x->color = h->color; // Copy the color of h to x
  h->color = RED;      // Set h's color to RED
  return x;            // Return the new root of the subtree (x)
}

Node *rotateRight(Node *h) {
  assert(h->left->color ==
         RED); // Ensure that the left child of h is red before rotating right,
               // if h is the nil node, this assertion will fail, which is
               // expected since we should never rotate right on a nil node
  Node *x = h->left;   // Set x to the left child of h
  h->left = x->right;  // Move x's right subtree to be h's left subtree
  x->right = h;        // Make h the right child of x
  x->color = h->color; // Copy the color of h to x
  h->color = RED;      // Set h's color to RED
  return x;            // Return the new root of the subtree (x)
}

Node *flipColors(Node *h) {
  assert(h->color != h->left->color &&
         h->color !=
             h->right->color); // Ensure that h's color is different
                               // from its children's colors before flipping, if
                               // h is the nil node, this assertion will fail,
                               // which is expected since we should never flip
                               // colors on a nil node
  Color c = h->color;          // Save h's color
  h->color = h->left->color;   // Flip h's color
  h->left->color = h->right->color = c; // flip h's children color
  return h;
}

Node *balance(Node *h) {
  if (h->right->color == RED)
    h = rotateLeft(h); // fix right-leaning red link
  if (h->left->color == RED && h->left->left->color == RED)
    h = rotateRight(h); // fix two reds in a row on the left
  if (h->left->color == RED && h->right->color == RED)
    h = flipColors(h); // split 4-node
  return h;
}

// move red left and right

Node *moveRedLeft(Node *h) {
  assert(h !=
         Node::nil); // Ensure that h is not the nil node before moving red
                     // left, since we should never move red left on a nil node
  h = flipColors(h); // Flip colors to prepare for moving red left
  if (h->right->left->color == RED) { // If the left child of the right child is
                                      // red, perform rotations and color flips
    h->right = rotateRight(h->right); // Rotate right on the right child
    h = rotateLeft(h);                // Rotate left on h
    h = flipColors(h); // Flip colors to maintain Red-Black properties
  }
  return h;
}

Node *moveRedRight(Node *h) {
  assert(
      h !=
      Node::nil); // Ensure that h is not the nil node before moving red right,
                  // since we should never move red right on a nil node
  h = flipColors(h); // Flip colors to prepare for moving red right
  if (h->left->right->color == RED) { // If the right child of the left child is
                                      // red, perform rotations and color flips
    h->left = rotateLeft(h->left);    // Rotate left on the left child
    h = rotateRight(h);               // Rotate right on h
    h = flipColors(h); // Flip colors to maintain Red-Black properties
  }
  return h;
}

// operations

std::pair<Node *, Node *> detach(Node *h) {
  Node *leftSubtree = h->left;    // Store the left subtree of h
  Node *rightSubtree = h->right;  // Store the right subtree of h
  h->left = h->right = Node::nil; // Detach the left and right subtrees from h
                                  // by setting them to nil]
  h->color = BLACK;               // Set h's color to BLACK
  return {
      leftSubtree,
      rightSubtree}; // Return the detached left and right subtrees as a pair
}

Node *search(Node *h, int key) {
  if (h->isExternal || h->key == key)
    return h; // If the node is external or the key matches, return the node
  if (key < h->key)
    return search(h->left, key); // If the key is less than the current node's
                                 // key, search in the left subtree
  else
    return search(h->right, key); // If the key is greater than the current
                                  // node's key, search in the right subtree
}

/**************************************************************
 * Join and split *
 **************************************************************/

Node *joinRec(Node *leftTree, Node *x, Node *rightTree) {
  if (leftTree->blackHeight > rightTree->blackHeight) {
    leftTree->right = joinRec(
        leftTree->right, x,
        rightTree); // If the left tree has a greater black height, join the
                    // right subtree of the left tree with x and the right tree
    return balance(leftTree);
  }
  if (leftTree->blackHeight < rightTree->blackHeight) {
    rightTree->left = joinRec(
        leftTree, x,
        rightTree
            ->left); // If the right tree has a greater black height, join the
                     // left tree with x and the left subtree of the right tree
    return balance(rightTree);
  }
  x->left = leftTree;
  x->right = rightTree;
  x->color = RED;
  return balance(x);
}

Node *join(Node *leftTree, Node *x, Node *rightTree) {
  assert(min(leftTree) < x->key &&
         x->key <
             max(rightTree)); // Ensure that all keys in the left tree are less
                              // than x's key and all keys in the right tree are
                              // greater than x's key before joining

  detach(x); // Detach x from its current position in the tree to prepare for
             // joining
  Node *y = joinRec(leftTree, x,
                    rightTree); // Join the left tree, the node x, and the right
                                // tree using the recursive join function
  y->color = BLACK; // Ensure the new root of the joined tree is black
  return y;         // Return the new root of the joined tree
}

std::tuple<Node *, Node *, Node *> split(Node *h, int key) {
  if (h->key > key) {
    auto [left, x, right] = split(h->left, key);
    h->right->color =
        BLACK; // Ensure the right child of h is black before joining
    return {left, x, join(right, h, h->right)};
  }
  if (h->key < key) {
    auto [left, x, right] = split(h->right, key);
    h->left->color =
        BLACK; // Ensure the left child of h is black before joining
    return {join(left, h, h->left), x, right};
  }

  auto [left, right] = detach(h);
  left->color = right->color = BLACK; // Ensure the detached left and right
                                      // subtrees are black before returning
  return {left, h, right}; // Return the left subtree, the node with the given
                           // key, and the right subtree as a tuple
}

Node *deleteMinRec(Node *h) {
  if (h->left == Node::nil)
    return Node::nil; // If the left child of h is nil, then h is the minimum
                      // node, so we return nil to delete it
  if (h->left->color == BLACK &&
      h->left->left->color ==
          BLACK) // Garantee that the left child of h is not a 2-node by moving
    // a red link left if necessary
    h = moveRedLeft(h);
  h->left = deleteMinRec(h->left);
  return balance(h);
}

Node *deleteMin(Node *root) {
  assert(root != Node::nil); // Ensure that the root is not the nil node before
                             // deleting the minimum, since we should never
                             // delete from an empty tree
  return deleteMinRec(root);
}

Node *deleteMaxRec(Node *h) {
  if (h->left->color == RED)
    h = rotateRight(
        h); // If the left child of h is red, rotate right to ensure that we are
            // moving down a path with a red link, which allows us to maintain
            // the properties of the Red-Black Tree during deletion
  if (h->right == Node::nil)
    return Node::nil; // If the right child of h is nil, then h is the maximum
                      // node, so we return nil to delete it
  if (h->right->color == BLACK &&
      h->right->left->color ==
          BLACK) // Garantee that the right child of h is not a 2-node by moving
                 // a red link right if necessary
    h = moveRedRight(h);
  h->right = deleteMaxRec(h->right);
  return balance(h);
}

Node *deleteMax(Node *root) {
  assert(root != Node::nil); // Ensure that the root is not the nil node before
                             // deleting the maximum, since we should never
                             // delete from an empty tree
  return deleteMaxRec(root);
}

// assert functions for testing and debugging

/**
 * @brief Check if the Red-Black Tree rooted at the given node is a valid binary
 * search tree. A binary search tree is valid if for every node, all keys in the
 * left subtree are less than the node's key, and all keys in the right subtree
 * are greater than the node's key.
 * @param root The root of the subtree to check for the binary search tree
 * property.
 * @param min The minimum key value allowed for the current subtree (used for
 * recursive checks).
 * @param max The maximum key value allowed for the current subtree (used for
 * recursive checks).
 * @return true if the subtree is a valid binary search tree, false otherwise.
 */
bool isBST(Node *root, int min = INT_MIN, int max = INT_MAX) {
  if (root == Node::nil)
    return true;
  if (root->key < min || root->key > max)
    return false;
  return isBST(root->left, min, root->key) &&
         isBST(root->right, root->key, max);
}

/**
 * @brief Check if the Red-Black Tree rooted at the given node is balanced. A
 * Red-Black Tree is balanced if every path from the root to a leaf has the same
 * number of black nodes.
 * @param root The root of the subtree to check for balance.
 * @return true if the subtree is balanced, false otherwise.
 */
bool isBalanced(Node *root) {
  if (root == Node::nil)
    return true;
  int leftBlackHeight = root->left->blackHeight;
  int rightBlackHeight = root->right->blackHeight;
  if (leftBlackHeight != rightBlackHeight)
    return false;
  return isBalanced(root->left) && isBalanced(root->right);
}

bool is23(Node *root) {
  if (root == Node::nil)
    return true;
  if ((root->color == RED && root->left->color == RED) ||
      root->right->color == RED)
    return false;
  return is23(root->left) && is23(root->right);
}

bool check(Node *root) {
  if (!isBST(root))
    return false;
  if (!isBalanced(root))
    return false;
  if (!is23(root))
    return false;
  return true;
}

// functions for testing and debugging

Node *insert(Node *root, int key);
Node *insertRec(Node *h, int key);
void print(Node *root, int indent = 0);

Node *insert(Node *root, int key) {
  root = insertRec(
      root,
      key); // Insert the key into the tree using the recursive insert function
  root->color =
      BLACK; // Ensure the root of the tree is always black after insertion
  return root;
}

Node *insertRec(Node *h, int key) {
  if (h == Node::nil)
    return newNode(key); // If we reach a nil node, we create and return a new
                         // node with the given key
  if (key < h->key)
    h->left =
        insertRec(h->left, key); // If the key is less than the current node's
                                 // key, insert into the left subtree
  else if (key > h->key)
    h->right =
        insertRec(h->right, key); // If the key is greater than the current
                                  // node's key, insert into the right subtree
  else
    return h; // If the key is equal to the current node's key, we do not insert
              // duplicates and simply return the current node
  return balance(h); // After insertion, we balance the subtree rooted at h to
                     // maintain the properties of the Red-Black Tree
}

void print(Node *root, int indent) {
  if (root == Node::nil)
    return;
  print(root->right, indent + 4);
  if (indent)
    std::cout << std::string(indent, ' ');
  std::cout << '(' << root->key << ' ' << (root->color == RED ? "R" : "B")
            << ')' << "\n";
  print(root->left, indent + 4);
}

int min(Node *root) {
  if (root == Node::nil)
    return INT_MAX; // If the tree is empty, return a large value to indicate no
                    // minimum
  if (root->left == Node::nil)
    return root
        ->key; // If the left child is nil, then the current node is the minimum
  return min(
      root->left); // Recursively search for the minimum in the left subtree
}

int max(Node *root) {
  if (root == Node::nil)
    return INT_MIN; // If the tree is empty, return a small value to indicate no
                    // maximum
  if (root->right == Node::nil)
    return root->key; // If the right child is nil, then the current node is the
                      // maximum
  return max(
      root->right); // Recursively search for the maximum in the right subtree
}

int main() {
  int n, m;
  std::cin >> n >> m;

  Node *root1 = Node::nil;
  Node *root2 = Node::nil;

  for (int i = 1; i <= n; ++i) { // create a Red-Black Tree by inserting n keys
                                 // read from standard input
    root1 = insert(root1, i);
  }

  for (int i = n + 1; i <= m; ++i) {
    root2 = insert(root2, i);
  }

  check(root1);
  check(root2);

  int op;

  while (std::cin >> op) {
    if (op == 1) {
      int key;
      std::cin >> key;
      auto [left, x, right] = split(root1, key);
      root1 = join(left, x, right);
    } else if (op == 2) {
      int key;
      std::cin >> key;
      auto [left, x, right] = split(root2, key);
      root2 = join(left, x, right);
    } else if (op == 3) {
      print(root1);
    } else if (op == 4) {
      print(root2);
    }
  }
  return 0;
}