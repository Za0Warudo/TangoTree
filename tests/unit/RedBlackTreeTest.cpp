#include "RedBlackTree.h"
#include "Checkers.h"
#include "Helpers.h"
#include <algorithm>
#include <gtest/gtest.h>
#include <numeric>
#include <vector>

/**************************************************************
 * Test Fixtures.
 **************************************************************/

/* Generic fixture. */
class RedBlackTreeTest : public ::testing::Test {
protected:
  Node *root;

  int n = 5;
  int initialKeys[5] = {4, 5, 7, 10, 15};

  void SetUp() override {
    root = Node::nil;
    for (int i = 0; i < n; i++)
      root = insert(root, initialKeys[i]);

    // Invariant check before each test.
    EXPECT_TRUE(check(root)) << "Invariants failed on SetUp!";
  }

  void TearDown() override {
    // Invariant check after each test.
    EXPECT_TRUE(check(root)) << "Invariants failed after test execution!";
  }
};

/* Split Fixture. */
class SplitTreeTest : public ::testing::Test {
protected:
  Node *root;
  Node *resultLeft;
  Node *resultRight;

  int n = 5;
  int initialKeys[5] = {4, 5, 7, 10, 15};

  void SetUp() override {
    // create tree.
    root = Node::nil;
    for (int i = 0; i < n; i++)
      root = insert(root, initialKeys[i]);

    resultLeft = Node::nil;
    resultRight = Node::nil;

    // Invariant check before each test.
    EXPECT_TRUE(check(root)) << "Invariants failed on SetUp!";
  }

  void TearDown() override {
    // Check the resulting split trees.
    EXPECT_TRUE(check(resultLeft)) << "Left tree invariants failed after Split!";
    EXPECT_TRUE(check(resultRight)) << "Right tree invariants failed after Split!";
  }
};

/* Join Fixture. */
class JoinTreeDifferentHeightsTest : public ::testing::Test {
protected:
  Node *leftTree;
  Node *x;
  Node *rightTree;
  Node *mergedTree;

  int keys_l[3] = {1, 2, 3};
  int key_m = 4;
  int keys_r[7] = {5, 6, 7, 8, 9, 10, 11};
  int keys_result[11] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

  void SetUp() override {
    // create left tree.
    leftTree = Node::nil;
    for (int i = 0; i < 3; i++)
      leftTree = insert(leftTree, keys_l[i]);

    // create middle node.
    x = newNode(key_m);

    // create right tree.
    rightTree = Node::nil;
    for (int i = 0; i < 7; i++)
      rightTree = insert(rightTree, keys_r[i]);

    mergedTree = Node::nil;

    // check invariants before each test.
    EXPECT_TRUE(check(leftTree)) << "Left tree invariants failed on SetUp!";
    EXPECT_TRUE(check(rightTree)) << "Right tree invariants failed on SetUp!";
  }

  void TearDown() override {
    if (mergedTree != Node::nil)
      EXPECT_TRUE(check(mergedTree)) << "Merged tree invariants failed after Join!";
  }
};

class JoinTreeSameHeightTest : public ::testing::Test {
protected:
  Node *leftTree;
  Node *x;
  Node *rightTree;
  Node *mergedTree;

  int keys_l[3] = {1, 2, 3};
  int key_m = 4;
  int keys_r[3] = {5, 6, 7};
  std::vector<int> keys_result = {1, 2, 3, 4, 5, 6, 7};

  void SetUp() override {
    // create left tree
    leftTree = Node::nil;
    for (int i = 0; i < 3; i++)
      leftTree = insert(leftTree, keys_l[i]);

    // create middle node
    x = newNode(key_m);

    // create right tree
    rightTree = Node::nil;
    for (int i = 0; i < 3; i++)
      rightTree = insert(rightTree, keys_r[i]);

    mergedTree = Node::nil;

    // check invariants before each test
    EXPECT_TRUE(check(leftTree)) << "Left tree invariants failed on SetUp!";
    EXPECT_TRUE(check(rightTree)) << "Right tree invariants failed on SetUp!";
  }

  void TearDown() override {
    if (mergedTree != Node::nil) {
      EXPECT_TRUE(check(mergedTree)) << "Merged tree invariants failed after Join!";
    }
  }
};

/* Depth Fixture. */
class DepthFieldsTest : public ::testing::Test {
protected:
  static constexpr int n = 64;
  Node *tree;

  static short depthOf(int key) { return (key * 7) % 31; }

  void SetUp() override {
    tree = Node::nil;
    for (int key = 0; key < n; key++)
      tree = join(tree, newNode(key, depthOf(key)), Node::nil);
  }

  // Checks the depth fields of every node, and the root's range against the depths of the tree's keys.
  void expectValidDepths(Node *t) {
    EXPECT_TRUE(check(t));
    EXPECT_TRUE(hasValidDepths(t));
    if (t == Node::nil)
      return;
    short expectedMin = SHORT_MAX;
    short expectedMax = SHORT_MIN;
    for (int key : keys(t)) {
      expectedMin = std::min(expectedMin, depthOf(key));
      expectedMax = std::max(expectedMax, depthOf(key));
    }
    EXPECT_EQ(t->minDepth, expectedMin);
    EXPECT_EQ(t->maxDepth, expectedMax);
  }
};

/* External Fixture. */
class ExternalChildTest : public ::testing::Test {
protected:
  Node *external;

  void SetUp() override {
    external = buildTree({10, 20, 30});     // The root is 20.
    external->isExternal = true;
    external->blackHeight = -1;
    external->minDepth = external->maxDepth = 99;
  }
};

namespace Tests {
namespace SearchTests {

TEST_F(RedBlackTreeTest, SearchSuccessValidKeyAtRoot) {
  Node *foundNode = search(root, 10);
  EXPECT_NE(foundNode, Node::nil);
  EXPECT_EQ(foundNode->key, 10);
}

TEST_F(RedBlackTreeTest, SearchSuccessValidKeyAtInnerNode) {
  Node *foundNode = search(root, 5);
  EXPECT_NE(foundNode, Node::nil);
  EXPECT_EQ(foundNode->key, 5);
}

TEST_F(RedBlackTreeTest, SearchSuccessValidKeyAtLeaf) {
  Node *foundNode = search(root, 7);
  EXPECT_EQ(foundNode->key, 7);
}

TEST_F(RedBlackTreeTest, SearchFailureExternalNode) {
  Node *notFoundNode = search(root, 20);
  EXPECT_EQ(notFoundNode, Node::nil);
}

TEST_F(RedBlackTreeTest, SearchFailureEmptyTree) {
  Node *notFoundNode = search(Node::nil, 1);
  EXPECT_EQ(notFoundNode, Node::nil);
}

}     // namespace SearchTests

namespace ParentSearchTests {

TEST_F(RedBlackTreeTest, ParentSearchSuccessInnerNode) {
  Node *parent = parentSearch(root, 7);
  EXPECT_NE(parent, Node::nil);
  EXPECT_EQ(parent->key, 5);
}

TEST_F(RedBlackTreeTest, ParentSearchSuccessExternalNode) {
  Node *parent = parentSearch(root, 2);
  EXPECT_NE(parent, Node::nil);
  EXPECT_EQ(parent->key, 4);
}

TEST_F(RedBlackTreeTest, ParentSearchFailureRootSearch) {
  Node *parent = parentSearch(root, 10);
  EXPECT_EQ(parent, Node::nil);
}

TEST_F(RedBlackTreeTest, ParentSearchFailureEmptyTree) {
  Node *parent = parentSearch(Node::nil, 1);
  EXPECT_EQ(parent, Node::nil);
}

}     // namespace ParentSearchTests

namespace InsertTests {

TEST_F(RedBlackTreeTest, InsertNewNodeGreaterThanMaxKey) {
  const int newKey = 20;
  Node *notFoundNode = search(root, newKey);
  EXPECT_EQ(notFoundNode, Node::nil);

  root = insert(root, newKey);

  Node *foundNode = search(root, newKey);
  EXPECT_EQ(foundNode->key, newKey);
}

TEST_F(RedBlackTreeTest, InsertNewNodeSmallerThanMinKey) {
  const int newKey = 1;
  Node *notFoundNode = search(root, newKey);
  EXPECT_EQ(notFoundNode, Node::nil);

  root = insert(root, newKey);

  Node *foundNode = search(root, newKey);
  EXPECT_EQ(foundNode->key, newKey);
}

TEST_F(RedBlackTreeTest, InsertNewNodeInMiddle) {
  const int newKey = 6;
  Node *notFoundNode = search(root, newKey);
  EXPECT_EQ(notFoundNode, Node::nil);

  root = insert(root, newKey);

  Node *foundNode = search(root, newKey);
  EXPECT_EQ(foundNode->key, newKey);
}

TEST_F(RedBlackTreeTest, InsertExistingKeyKeepsTreeUnchanged) {
  const std::vector<int> expected = {4, 5, 7, 10, 15};
  for (int i = 0; i < n; i++) {
    root = insert(root, initialKeys[i]);
    EXPECT_EQ(keys(root), expected);
    EXPECT_TRUE(check(root));
  }
}

}     // namespace InsertTests

namespace MinTests {

TEST_F(RedBlackTreeTest, GetMinValueMoreThanOneNode) {
  Node *minNode = min(root);
  EXPECT_EQ(minNode->key, 4);
}

TEST_F(RedBlackTreeTest, GetMinValueSingleNode) {
  Node *newRoot = newNode(1);
  Node *minNode = min(newRoot);
  EXPECT_EQ(minNode->key, 1);
}

/**
 * @note a min function call over an empty tree is not defined, so we don't test it here.
 */

}     // namespace MinTests

namespace MaxTests {

TEST_F(RedBlackTreeTest, GetMaxValueMoreThanOneNode) {
  Node *maxNode = max(root);
  EXPECT_EQ(maxNode->key, 15);
}

TEST_F(RedBlackTreeTest, GetMaxValueSingleNode) {
  Node *newRoot = newNode(1);
  Node *maxNode = max(newRoot);
  EXPECT_EQ(maxNode->key, 1);
}

/**
 * @note a max function call over an empty tree is not defined, so we don't test it here.
 */

}     // namespace MaxTests

namespace DeleteMinTests {

TEST_F(RedBlackTreeTest, DeleteMinRemovesSmallestKey) {
  auto [minNode, newRoot] = deleteMin(root);
  root = newRoot;     // TearDown validates the new tree.
  EXPECT_EQ(minNode->key, 4);
  EXPECT_EQ(keys(root), (std::vector<int>{5, 7, 10, 15}));
}

TEST_F(RedBlackTreeTest, DeleteMinUntilEmpty) {
  const int size = 100;
  std::vector<int> expected(size);
  std::iota(expected.begin(), expected.end(), 0);

  Node *tree = buildTree(shuffledRange(size));
  for (int key = 0; key < size; key++) {
    auto [minNode, newTree] = deleteMin(tree);
    tree = newTree;
    expected.erase(expected.begin());

    ASSERT_EQ(minNode->key, key);
    ASSERT_TRUE(check(tree)) << "Invariants failed after deleting key " << key;
    ASSERT_EQ(keys(tree), expected);
  }
  EXPECT_EQ(tree, Node::nil);
}

/**
 * @note a deleteMin function call over an empty tree is not defined, so we don't test it here.
 */

}     // namespace DeleteMinTests

namespace DeleteMaxTests {

TEST_F(RedBlackTreeTest, DeleteMaxRemovesLargestKey) {
  auto [maxNode, newRoot] = deleteMax(root);
  root = newRoot;     // TearDown validates the new tree.
  EXPECT_EQ(maxNode->key, 15);
  EXPECT_EQ(keys(root), (std::vector<int>{4, 5, 7, 10}));
}

TEST_F(RedBlackTreeTest, DeleteMaxUntilEmpty) {
  const int size = 100;
  std::vector<int> expected(size);
  std::iota(expected.begin(), expected.end(), 0);

  Node *tree = buildTree(shuffledRange(size));
  for (int key = size - 1; key >= 0; key--) {
    auto [maxNode, newTree] = deleteMax(tree);
    tree = newTree;
    expected.pop_back();

    ASSERT_EQ(maxNode->key, key);
    ASSERT_TRUE(check(tree)) << "Invariants failed after deleting key " << key;
    ASSERT_EQ(keys(tree), expected);
  }
  EXPECT_EQ(tree, Node::nil);
}

/**
 * @note a deleteMax function call over an empty tree is not defined, so we don't test it here.
 */

}     // namespace DeleteMaxTests

namespace SplitTests {

TEST_F(SplitTreeTest, SplitLeftSuccessfully) {
  // Splits at value (7), a leaf node.
  auto [lTree, splitNode, rTree] = split(root, 7);
  resultLeft = lTree;
  resultRight = rTree;

  EXPECT_EQ(splitNode->key, 7);
  EXPECT_EQ(keys(resultLeft), (std::vector<int>{4, 5}));
  EXPECT_EQ(keys(resultRight), (std::vector<int>{10, 15}));
}

TEST_F(SplitTreeTest, SplitAtInnerNodeSuccessfully) {
  // Splits at value (5), a inner node.
  auto [lTree, splitNode, rTree] = split(root, 5);
  resultLeft = lTree;
  resultRight = rTree;

  EXPECT_EQ(splitNode->key, 5);
  EXPECT_EQ(keys(resultLeft), (std::vector<int>{4}));
  EXPECT_EQ(keys(resultRight), (std::vector<int>{7, 10, 15}));
}

TEST_F(SplitTreeTest, SplitRootSuccessfully) {
  // Splits at value (10), the tree root.
  ASSERT_EQ(root->key, 10);
  auto [lTree, splitNode, rTree] = split(root, 10);
  resultLeft = lTree;
  resultRight = rTree;

  EXPECT_EQ(splitNode->key, 10);
  EXPECT_EQ(keys(resultLeft), (std::vector<int>{4, 5, 7}));
  EXPECT_EQ(keys(resultRight), (std::vector<int>{15}));
}

TEST_F(SplitTreeTest, SplitAtMinimumNode) {
  // Splits at the lowest value (4). Left tree should be empty.
  auto [lTree, splitNode, rTree] = split(root, 4);
  resultLeft = lTree;
  resultRight = rTree;

  EXPECT_EQ(resultLeft, Node::nil);
  EXPECT_EQ(splitNode->key, 4);
  EXPECT_EQ(keys(resultRight), (std::vector<int>{5, 7, 10, 15}));
}

TEST_F(SplitTreeTest, SplitAtMaximumNode) {
  // Splits at the highest value (15). Right tree should be empty.
  auto [lTree, splitNode, rTree] = split(root, 15);
  resultLeft = lTree;
  resultRight = rTree;

  EXPECT_EQ(resultRight, Node::nil);
  EXPECT_EQ(splitNode->key, 15);
  EXPECT_EQ(keys(resultLeft), (std::vector<int>{4, 5, 7, 10}));
}

TEST_F(SplitTreeTest, SplitAtSingleNodeTree) {
  // Splits a tree with a single node (10). Both left and right trees should be empty.
  Node *singleNodeTree = newNode(10);
  auto [lTree, splitNode, rTree] = split(singleNodeTree, 10);
  resultLeft = lTree;
  resultRight = rTree;

  EXPECT_EQ(resultLeft, Node::nil);
  EXPECT_EQ(resultRight, Node::nil);
  EXPECT_EQ(splitNode->key, 10);
}

/**
 * @note a split function call must at least contain the key to split at, so we don't test it here.
 */

}     // namespace SplitTests

namespace JoinTests {

TEST_F(JoinTreeSameHeightTest, JoinAtRootSuccessfully) {
  mergedTree = join(leftTree, x, rightTree);
  EXPECT_EQ(mergedTree->key, key_m);
  EXPECT_EQ(keys(mergedTree), keys_result);
}

TEST_F(JoinTreeSameHeightTest, JoinWithTallerLeftTree) {
  Node *tallLeft = buildTree({1, 2, 3, 4, 5, 6, 7});
  Node *shortRight = buildTree({9, 10, 11});
  ASSERT_GT(tallLeft->blackHeight, shortRight->blackHeight);

  mergedTree = join(tallLeft, newNode(8), shortRight);
  EXPECT_EQ(keys(mergedTree), (std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}));
}

TEST_F(JoinTreeSameHeightTest, JoinWithTallerRightTree) {
  Node *shortLeft = buildTree({1, 2, 3});
  Node *tallRight = buildTree({5, 6, 7, 8, 9, 10, 11});
  ASSERT_LT(shortLeft->blackHeight, tallRight->blackHeight);

  mergedTree = join(shortLeft, newNode(4), tallRight);
  EXPECT_EQ(keys(mergedTree), (std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}));
}

TEST_F(JoinTreeDifferentHeightsTest, JoinWithEmptyLeftTree) {
  // Corner Case, left tree is empty
  Node *emptyLeft = Node::nil;
  mergedTree = join(emptyLeft, x, rightTree);

  EXPECT_EQ(keys(mergedTree), (std::vector<int>{key_m, 5, 6, 7, 8, 9, 10, 11}));
}

TEST_F(JoinTreeDifferentHeightsTest, JoinWithEmptyRightTree) {
  // Corner Case, right tree is empty
  Node *emptyRight = Node::nil;
  mergedTree = join(leftTree, x, emptyRight);
  EXPECT_EQ(keys(mergedTree), (std::vector<int>{1, 2, 3, 4}));
}

TEST_F(JoinTreeDifferentHeightsTest, JoinBothTreesEmpty) {
  // Corner Case, both trees are empty, joining just the 'x' node
  Node *emptyLeft = Node::nil;
  Node *emptyRight = Node::nil;
  mergedTree = join(emptyLeft, x, emptyRight);

  std::vector<int> mergedKeys = keys(mergedTree);
  EXPECT_EQ(mergedKeys.size(), 1);
  EXPECT_EQ(mergedKeys[0], key_m);
}

/**
 * @note a join function call must at least contain the middle node, so we don't test it here.
 */

}     // namespace JoinTests

namespace SplitJoinRoundTripTests {

TEST(SplitJoinRoundTripTest, SplitAtEveryKeyThenJoinRestoresTree) {
  for (int size = 1; size <= 60; size++) {
    const std::vector<int> shuffled = shuffledRange(size, size);
    std::vector<int> sorted = shuffled;
    std::sort(sorted.begin(), sorted.end());

    for (int key = 0; key < size; key++) {
      auto [left, splitNode, right] = split(buildTree(shuffled), key);

      ASSERT_EQ(splitNode->key, key);
      ASSERT_TRUE(check(left)) << "Left tree invalid. size=" << size << " key=" << key;
      ASSERT_TRUE(check(right)) << "Right tree invalid. size=" << size << " key=" << key;
      ASSERT_EQ(keys(left), std::vector<int>(sorted.begin(), sorted.begin() + key));
      ASSERT_EQ(keys(right), std::vector<int>(sorted.begin() + key + 1, sorted.end()));

      Node *joined = join(left, splitNode, right);
      ASSERT_TRUE(check(joined)) << "Joined tree invalid. size=" << size << " key=" << key;
      ASSERT_EQ(keys(joined), sorted);
    }
  }
}

}     // namespace SplitJoinRoundTripTests

namespace DepthFieldsTests {

TEST_F(DepthFieldsTest, ValidAfterJoin) { expectValidDepths(tree); }

TEST_F(DepthFieldsTest, ValidAfterSplit) {
  auto [left, splitNode, right] = split(tree, 20);
  EXPECT_EQ(splitNode->key, 20);
  expectValidDepths(left);
  expectValidDepths(right);
  expectValidDepths(splitNode);
}

TEST_F(DepthFieldsTest, ValidAfterDeleteMin) {
  for (int i = 0; i < n; i++) {
    auto [minNode, newTree] = deleteMin(tree);
    tree = newTree;
    expectValidDepths(tree);
    expectValidDepths(minNode);
  }
}

TEST_F(DepthFieldsTest, ValidAfterDeleteMax) {
  for (int i = 0; i < n; i++) {
    auto [maxNode, newTree] = deleteMax(tree);
    tree = newTree;
    expectValidDepths(tree);
    expectValidDepths(maxNode);
  }
}

}     // namespace DepthFieldsTests

namespace ExternalChildTests {

TEST_F(ExternalChildTest, SearchStopsAtExternalChild) {
  Node *host = newNode(50);
  host->left = external;

  // The key 10 is inside the external tree, but search must not go past its root.
  EXPECT_EQ(search(host, 10), external);
  EXPECT_EQ(parentSearch(host, 10), host);
}

TEST_F(ExternalChildTest, MinStopsAtInternalChild) {
  Node *host = newNode(50);
  host->left = external;
  EXPECT_EQ(min(host), host);
}

TEST_F(ExternalChildTest, MaxStopsAtInternalChild) {
  Node *host = newNode(5);
  host->right = external;
  EXPECT_EQ(max(host), host);
}

TEST_F(ExternalChildTest, UpdateIgnoresExternalChild) {
  Node *host = newNode(50, 5);
  host->left = external;
  update(host);

  EXPECT_EQ(host->blackHeight, 0);
  EXPECT_EQ(host->minDepth, 5);
  EXPECT_EQ(host->maxDepth, 5);
}

TEST_F(ExternalChildTest, JoinKeepsExternalChildLinked) {
  // Same shape as the joins in cut(), with an external tree in place of the right tree.
  Node *merged = join(buildTree({1, 2, 3}), newNode(5), external);

  EXPECT_TRUE(check(merged));
  EXPECT_TRUE(external->isExternal);
  EXPECT_EQ(search(merged, 5)->key, 5);
  EXPECT_EQ(search(merged, 10), external);
}

TEST_F(ExternalChildTest, DeleteMinKeepsExternalChildLinked) {
  Node *merged = join(buildTree({1, 2, 3}), newNode(5), external);

  auto [minNode, newTree] = deleteMin(merged);

  EXPECT_EQ(minNode->key, 1);
  EXPECT_TRUE(check(newTree));
  EXPECT_EQ(search(newTree, 10), external);
}

}     // namespace ExternalChildTests

}     // namespace Tests