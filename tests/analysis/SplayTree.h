#ifndef SPLAYTREE_H
#define SPLAYTREE_H

/**
 * @file SplayTree.h
 *
 * @brief A recursive splay tree, used as a baseline in the tree comparison. It follows the algorithm from the GeeksforGeeks article "Introduction to Splay tree
 * data structure": single rotations, a recursive splay, and an insert that splays and then splits the tree around the new key.
 *
 * To compare against another splay tree implementation, replace this class and keep its two public methods, insert and contains.
 *
 * To also count node accesses, a replacement should call COUNT_SPLAY_ACCESS once for each node on the search path.
 *
 * @note The nodes are not freed, because each benchmark process measures a single run and then exits.
 * @note A splay can recurse as deep as the tree height, which can reach the number of keys. The comparison script raises the stack limit to allow this.
 */

/**
 * @brief Counts a visit to a node, when the code is compiled with COUNT_NODE_ACCESSES. Rotations are not counted, as in the other trees.
 */
#ifdef COUNT_NODE_ACCESSES
extern long long nodeAccessCount;
#define COUNT_SPLAY_ACCESS() (++nodeAccessCount)
#else
#define COUNT_SPLAY_ACCESS()
#endif

/**
 * @brief A splay tree of integer keys. Every operation moves the last visited node to the root.
 */
class SplayTree {
public:
  /**
   * @brief Inserts a key into the tree, ignoring duplicates. The key ends up at the root.
   *
   * @param key The key to insert.
   *
   * @note Time Complexity: O(log(N)) amortized, where N is the number of nodes in the tree.
   */
  void insert(int key) {
    if (root == nullptr) {
      root = new Node{key, nullptr, nullptr};
      return;
    }

    root = splay(root, key);
    if (root->key == key)
      return;

    Node *x = new Node{key, nullptr, nullptr};
    if (key < root->key) {
      x->right = root;
      x->left = root->left;
      root->left = nullptr;
    } else {
      x->left = root;
      x->right = root->right;
      root->right = nullptr;
    }
    root = x;
  }

  /**
   * @brief Checks if the tree contains the given key. The search splays the last visited node to the root.
   *
   * @param key The key to search for.
   * @return True, if the key is in the tree; false, otherwise.
   *
   * @note Time Complexity: O(log(N)) amortized, where N is the number of nodes in the tree.
   */
  bool contains(int key) {
    root = splay(root, key);
    return root != nullptr && root->key == key;
  }

private:
  struct Node {
    int key;
    Node *left;
    Node *right;
  };

  Node *root = nullptr;

  /**
   * @brief Rotates a subtree to the right.
   *
   * @param h The subtree root node.
   * @return The new subtree root after the rotation.
   *
   * @note Precondition: h must have a left child.
   * @note Time Complexity: O(1).
   */
  static Node *rotateRight(Node *h) {
    Node *x = h->left;
    h->left = x->right;
    x->right = h;
    return x;
  }

  /**
   * @brief Rotates a subtree to the left.
   *
   * @param h The subtree root node.
   * @return The new subtree root after the rotation.
   *
   * @note Precondition: h must have a right child.
   * @note Time Complexity: O(1).
   */
  static Node *rotateLeft(Node *h) {
    Node *x = h->right;
    h->right = x->left;
    x->left = h;
    return x;
  }

  /**
   * @brief Recursively moves the node with the given key to the root of the subtree. If the key is not in the subtree, moves the last visited node instead. Each
   * call handles two levels of the search path, with a zig-zig or zig-zag step.
   *
   * @param h The subtree root node.
   * @param key The key to splay.
   * @return The new subtree root after the splay.
   *
   * @note Time Complexity: O(log(N)) amortized, where N is the number of nodes in the subtree.
   */
  static Node *splay(Node *h, int key) {
    if (h == nullptr)
      return h;

    COUNT_SPLAY_ACCESS();
    if (h->key == key)
      return h;

    if (key < h->key) {
      if (h->left == nullptr)
        return h;

      COUNT_SPLAY_ACCESS();
      if (key < h->left->key) {     // Zig-zig: the key is in the left subtree of the left child.
        h->left->left = splay(h->left->left, key);
        h = rotateRight(h);
      } else if (key > h->left->key) {     // Zig-zag: the key is in the right subtree of the left child.
        h->left->right = splay(h->left->right, key);
        if (h->left->right != nullptr)
          h->left = rotateLeft(h->left);
      }

      return h->left == nullptr ? h : rotateRight(h);
    }

    if (h->right == nullptr)
      return h;

    COUNT_SPLAY_ACCESS();
    if (key < h->right->key) {     // Zag-zig: the key is in the left subtree of the right child.
      h->right->left = splay(h->right->left, key);
      if (h->right->left != nullptr)
        h->right = rotateRight(h->right);
    } else if (key > h->right->key) {     // Zag-zag: the key is in the right subtree of the right child.
      h->right->right = splay(h->right->right, key);
      h = rotateLeft(h);
    }

    return h->right == nullptr ? h : rotateLeft(h);
  }
};

#endif     // SPLAYTREE_H
