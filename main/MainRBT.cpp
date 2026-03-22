/**
 * @file MainRBT.cpp
 * @author Zawarudo (@zawarudo)
 * @version 0.1
 * @date 2026-03-21
 * @copyright Copyright (c) 2026
 *
 * For tests purposes, you can use the following interface to interact with the Red-Black Tree implementation. The
 * interface allows you to build a Red-Black Tree with a given range of keys, insert keys, check for the presence of
 * keys, remove keys, join two trees with a separator key, and split a tree around a given key. The operations are
 * designed to be simple and straightforward for testing the functionality of the Red-Black Tree implementation.
 *
 * build:   0 <id> <l> <r> - Builds a RBT with id <id> that contains the keys in (l, r).
 *
 * insert:  1  <id> <k> - Inserts the key <k> in the tree with the id <id>.
 *
 * search:  2 <id> <k> - Checks if the key <k> is in the tree with the id <id>.
 *
 * delete:  3 <id> <k> - Removes the key <k> from the tree with the id <id>.
 *
 * join:    4 <id1> <k> <id2> - Joins the tree with id <id1> and the tree with id <id2> using a new node with key <k> as
 * separator. The resulting tree is stored in the tree with id <id1>, while the tree with id <id2> is cleared.
 *
 * split:   5 <id> <k> - Splits the tree with id <id> using the key <k>. The resulting left subtree, node with key <k>,
 * and right subtree are printed to the output, and the original tree with id <id> is cleared.
 */

// includes.
#include "RedBlackTree.h"
#include <cassert>
#include <iostream>
#include <map>

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

      if (DEBUG)
        std::cout << "Inserted " << val << " into tree " << id << std::endl;

      break;
    }
    case 2: { // Contains.
      std::cin >> id >> val;
      Node *tree = (trees.count(id) > 0) ? trees[id] : Node::nil;
      bool found = (search(tree, val).first != Node::nil);
      std::cout << (found ? "True" : "False") << std::endl;
      break;
    }
    case 3: { // Delete.
      std::cin >> id >> val;
      if (trees.count(id) > 0) {
        trees[id] = deleteNode(trees[id], val);
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
        assert(x->key == key); // Check the validity of the node with the given key.
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