#include <gtest/gtest.h>
#include <climits>
#include "RedBlackTree.h"

/**************************************************************
 * Functions for testing and debugging.
 **************************************************************/

/**
 * @brief Validates if the given tree is a binary search tree.
 * Ensures that for every node:
 * - All keys in the left subtree are strictly less than the node's key.
 * - All keys in the right subtree are strictly greater than the node's key.
 *
 * @param root The tree root.
 * @param min The lower bound for the current node's key.
 * @param max The upper bound for the current node's key.
 * @return True, if the subtree is a valid BST; false, otherwise.
 * @note Time Complexity: O(N), where N is the number of nodes in the subtree.
 */
bool isBST(Node *root, int min = INT_MIN, int max = INT_MAX) {
    if (root->isExternal) return true;
    if (root->key < min || root->key > max) return false;
    return isBST(root->left, min, root->key) && isBST(root->right, root->key, max);
}

/**
 * @brief Verifies if the given red-black tree is a balanced tree.
 * A subtree is considered balanced if every path from the current root
 * to a descendant leaf contains the same number of black nodes.
 *
 * @param root The tree root.
 * @return true, if the black-height is consistent across all paths; false, otherwise.
 * @note Time Complexity: O(N), where N is the number of nodes in the subtree.
 */
bool isBalanced(Node *root) {
    if (root->isExternal) return true;
    if (root->left->blackHeight + (root->left->color == BLACK? 1 : 0) != root->right->blackHeight + 1) return false;
    return isBalanced(root->left) && isBalanced(root->right);
}


/**
 * @brief Validates the 2-3 tree structural invariants in the given subtree.
 * A valid left-leaning red-black tree must satisfy the following conditions:
 * - No right-leaning red links (all red links must be left children).
 * - No two consecutive red links in any path (no "4-node" violations).
 *
 * @param root The tree root.
 * @return true, if the subtree satisfies all LLRBT properties; false, otherwise.
 * @note Time Complexity: O(N), where N is the total number of nodes in the subtree.
 */
bool is23(Node *root) {
    if (root->isExternal) return true;
    if ((root->color == RED && root->left->color == RED) || 
        root->right->color == RED) {
        return false;
    }
    return is23(root->left) && is23(root->right);
}

/**
 * @brief Performs a 3-check validation of the red-black tree invariants into the given tree.
 * The composite checks verifies the following structural and logical properties:
 * - Binary Search Tree (BST) property: Keys are correctly ordered.
 * - Balance property: Uniform black-height across all paths.
 * - 2-3 Tree (LLRBT) property: No right-leaning red links or consecutive red links.
 *
 * @param root The tree root.
 * @return true, if all RBT invariants are satisfied; false, otherwise.
 * @note Time Complexity: O(N), where N is the total number of nodes in the tree.
 */
bool check(Node *root) {
    if (root == Node::nil) return true; // Safety check for nil root.
    if (!isBST(root)) std::cout << "Is not BST" << std::endl; 
    if (!isBalanced(root)) std::cout << "Is not balanced" << std::endl;
    if (!is23(root)) std::cout << "Is not 2-3" << std::endl;
    return isBST(root) && isBalanced(root) && is23(root);
}

/**************************************************************
 * Test Fixtures.
 **************************************************************/

/* Generic fixture. */
class RedBlackTreeTest : public ::testing::Test {
protected:
    Node* root; 

    int n = 5; 
    int keys[5] = {4, 5, 7, 10, 15}; 

    void SetUp() override {
        root = Node::nil;
        for (int i = 0; i < n; i++)
            root = insert(root, keys[i]); 

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
    Node* root;
    Node* resultLeft;
    Node* resultRight;

    int n = 5; 
    int keys[5] = {4, 5, 7, 10, 15}; 

    void SetUp() override {
        root = Node::nil;
        for (int i = 0; i < n; i++)
            root = insert(root, keys[i]); 
        
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
class JoinTreeTest : public ::testing::Test {
protected:
    Node* leftTree;
    Node* x; 
    Node* rightTree;
    Node* mergedTree;

    int keys_l[3] = {1, 2, 3}; 
    int key_m = 4; 
    int keys_r[7] = {5, 6, 7, 8, 9, 10, 11};
    int keys[11] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}; 

    void SetUp() override {
        leftTree =  Node::nil;
        for (int i = 0; i < 3; i++)
            leftTree = insert(leftTree, keys_l[i]); 

        x = newNode(key_m); 

        rightTree = Node::nil;
        for (int i = 0; i < 7; i++)
            rightTree = insert(rightTree, keys_r[i]); 

        mergedTree = Node::nil;

        // Invariant check before each test.
        EXPECT_TRUE(check(leftTree)) << "Left tree invariants failed on SetUp!"; ;
        EXPECT_TRUE(check(rightTree)) << "Right tree invariants failed on SetUp!"; ;
    }

    void TearDown() override {
        // Check the resulting joined tree
        if (mergedTree != Node::nil) {
            EXPECT_TRUE(check(mergedTree)) << "Merged tree invariants failed after Join!";
        }
    }
};


/**************************************************************
 * Base binary serch tree operations Tests (Search, Insert, DeleteMin, DeleteMax). 
 **************************************************************/

 /* Search */

TEST_F(RedBlackTreeTest, SearchSuccess1) {
    auto [foundNode, parent] = search(root, 10);
    EXPECT_NE(foundNode, Node::nil);
    EXPECT_EQ(foundNode->key, 10); 
}

TEST_F(RedBlackTreeTest, SearchSuccess2) {
    auto [foundNode, parent] = search(root, 7);
    EXPECT_EQ(foundNode->key, 7);
}

TEST_F(RedBlackTreeTest, SearchFailure1) {
    auto [notFoundNode, parent] = search(root, 20); 
    EXPECT_EQ(notFoundNode, Node::nil); 
}

TEST_F(RedBlackTreeTest, SearchFailure2) {
    auto [notFoundNode, parent] = search(root, 1); 
    EXPECT_EQ(notFoundNode, Node::nil); 
}
/* Insert */

TEST_F(RedBlackTreeTest, InsertNewNode1) {
    const int newKey = 20;  
    auto [notFoundNode, parent1] = search(root, newKey); 
    EXPECT_EQ(notFoundNode, Node::nil);    
    
    root = insert(root, newKey); 
    
    auto [foundNode, parent2] = search(root, newKey); 
    EXPECT_EQ(foundNode->key, newKey);  
}

TEST_F(RedBlackTreeTest, InsertNewNode2) {
    const int newKey = 1;  
    auto [notFoundNode, parent1] = search(root, newKey); 
    EXPECT_EQ(notFoundNode, Node::nil); 

    root = insert(root, newKey); 
    
    auto [foundNode, parent2] = search(root, newKey); 
    EXPECT_EQ(foundNode->key, newKey); 
}

TEST_F(RedBlackTreeTest, ReplaceAlreadyExistingKey1) {
    const int key = 7;  
    auto [foundNode1, parent1] = search(root, key); 
    EXPECT_EQ(foundNode1->key, key); 

    root = insert(root, key); 

    auto [foundNode2, parent2] = search(root, key); 
    EXPECT_EQ(foundNode2->key, key); 
}

/* Min & Max */

TEST_F(RedBlackTreeTest, GetMinValue1) {
    Node* minNode = min(root);
    EXPECT_EQ(minNode->key, 4);  
}

TEST_F(RedBlackTreeTest, GetMinValue2) {
    Node* newRoot = newNode(1); 
    Node* minNode = min(newRoot);
    EXPECT_EQ(minNode->key, 1);  
}

TEST_F(RedBlackTreeTest, GetMaxValue1) {
    Node* maxNode = max(root); 
    EXPECT_EQ(maxNode->key, 15);
}

TEST_F(RedBlackTreeTest, GetMaxValue2) {
    Node* newRoot = newNode(1); 
    Node* maxNode = max(newRoot);
    EXPECT_EQ(maxNode->key, 1);  
}

/* DeleteMin & DeleteMax */

TEST_F(RedBlackTreeTest, deleteMinValue1) {
    auto [minNode, newRoot] = deleteMin(root);
    EXPECT_EQ(minNode->key, 4); 
    for (int i = 0; i < n; i++) {
        auto [node, parent] = search(newRoot, keys[i]); 
        if (node == Node::nil) EXPECT_EQ(keys[i], 4);
        else EXPECT_EQ(keys[i], node->key); 
    }
}

TEST_F(RedBlackTreeTest, deleteMaxValue1) {
    auto [maxNode, newRoot] = deleteMax(root);
    EXPECT_EQ(maxNode->key, 15); 
    for (int i = 0; i < n; i++) {
        auto [node, parent] = search(newRoot, keys[i]); 
        if (node == Node::nil) EXPECT_EQ(keys[i], 15);
        else EXPECT_EQ(keys[i], node->key); 
    }
}

/**************************************************************
 * Split Operations Tests
 **************************************************************/

TEST_F(SplitTreeTest, SplitMiddleSuccessfully) {
    // Splits at a middle value (7)
    auto [lTree, splitNode, rTree] = split(root, 7);
    resultLeft = lTree;
    resultRight = rTree;

    auto [leftNode, _1] = search(resultLeft, 5);
    auto [rightNode, _2] = search(resultRight, 10);
    auto [notFoundRight, _3] = search(resultRight, 5); // Should not be in right tree

    EXPECT_NE(leftNode, Node::nil);
    EXPECT_EQ(splitNode->key, 7);
    EXPECT_NE(rightNode, Node::nil);
    EXPECT_EQ(notFoundRight, Node::nil);
}

TEST_F(SplitTreeTest, SplitAtMinimumNode) {
    // Splits at the lowest value (3). Left tree should be empty.
    auto [lTree, splitNode, rTree] = split(root, 4);
    resultLeft = lTree;
    resultRight = rTree;

    EXPECT_EQ(resultLeft, Node::nil);
    EXPECT_EQ(splitNode->key, 4);
    
    auto [rightNode, _1] = search(resultRight, 5);
    EXPECT_NE(rightNode, Node::nil);
}

TEST_F(SplitTreeTest, SplitAtMaximumNode) {
    // Splits at the highest value (15). Right tree should be empty.
    auto [lTree, splitNode, rTree] = split(root, 15);
    resultLeft = lTree;
    resultRight = rTree;

    EXPECT_EQ(resultRight, Node::nil);
    EXPECT_EQ(splitNode->key, 15);
    
    auto [leftNode, _1] = search(resultLeft, 10);
    EXPECT_NE(leftNode, Node::nil);
}

/**************************************************************
 * Join Operations Tests
 **************************************************************/

TEST_F(JoinTreeTest, JoinSuccess) {
    // Standard join of two valid trees and a middle node
    mergedTree = join(leftTree, x, rightTree);

    for (int i = 0; i < 11; i++) 
        EXPECT_EQ(search(mergedTree, keys[i]).first->key, keys[i]);
}

TEST_F(JoinTreeTest, JoinWithEmptyLeftTree) {
    // Corner Case, left tree is empty
    Node* emptyLeft = Node::nil;
    mergedTree = join(emptyLeft, x, rightTree);

    for (int i = 0; i < 8; i++) 
        EXPECT_EQ(search(mergedTree, i + 4).first->key, i + 4);
}

TEST_F(JoinTreeTest, JoinWithEmptyRightTree) {
    // Corner Case, right tree is empty
    Node* emptyRight = Node::nil;
    mergedTree = join(leftTree, x, emptyRight);
    for (int i = 0; i < 4; i++) 
        EXPECT_EQ(search(mergedTree, i + 1).first->key, i + 1);

}

TEST_F(JoinTreeTest, JoinBothTreesEmpty) {
    // Corner Case, both trees are empty, joining just the 'x' node
    Node* emptyLeft = Node::nil;
    Node* emptyRight = Node::nil;
    mergedTree = join(emptyLeft, x, emptyRight);

    EXPECT_EQ(mergedTree->key, key_m);
    EXPECT_EQ(mergedTree->left, Node::nil);
    EXPECT_EQ(mergedTree->right, Node::nil);
}