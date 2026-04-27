#include <gtest/gtest.h>
#include "RedBlackTree.h"

TEST(RedBlackTree, InsertAndSearch) {
  Node *root = Node::nil;
  root = insert(root, 5);
  root = insert(root, 3);
  root = insert(root, 8);

  auto [foundNode, parent] = search(root, 5);
  EXPECT_NE(foundNode, Node::nil);
  EXPECT_EQ(foundNode->key, 5);

  auto [notFoundNode, notFoundParent] = search(root, 10);
  EXPECT_EQ(notFoundNode, Node::nil);
}

TEST(RedBlackTree, DeleteExistingNode) {
  Node *root = Node::nil;
  root = insert(root, 10);
  root = insert(root, 4);
  root = insert(root, 15);

  root = deleteNode(root, 4);

  auto [foundNode, parent] = search(root, 4);
  EXPECT_EQ(foundNode, Node::nil);
}

TEST(RedBlackTree, DeleteMinAndMax) {
  Node *root = Node::nil;
  root = insert(root, 20);
  root = insert(root, 10);
  root = insert(root, 30);

  root = deleteMin(root);
  auto [minNode, minParent] = search(root, 10);
  EXPECT_EQ(minNode, Node::nil);

  root = deleteMax(root);
  auto [maxNode, maxParent] = search(root, 30);
  EXPECT_EQ(maxNode, Node::nil);
}
