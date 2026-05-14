#include <gtest/gtest.h>
#include <vector>
#include <algorithm>
#include <random>
#include "TangoTree.h"

class TangoTreeTest : public ::testing::Test {
protected:
    // Helper to generate a random permutation of keys 1 to n
    std::vector<int> generate_random_keys(int n) {
        std::vector<int> keys(n);
        for (int i = 0; i < n; ++i) keys[i] = i + 1;
        
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(keys.begin(), keys.end(), g);
        return keys;
    }
};

// Test if a tree of size 1 works correctly
TEST_F(TangoTreeTest, SingleElementTree) {
    TangoTree tree(1);
    EXPECT_TRUE(tree.contains(1));
    EXPECT_FALSE(tree.contains(0));
    EXPECT_FALSE(tree.contains(2));
}

// Test keys at the boundaries of the reference tree
TEST_F(TangoTreeTest, BoundaryKeys) {
    int n = 100;
    TangoTree tree(n);
    EXPECT_TRUE(tree.contains(1));   // Smallest
    EXPECT_TRUE(tree.contains(n));   // Largest
    EXPECT_FALSE(tree.contains(0));   // Out of bounds low
    EXPECT_FALSE(tree.contains(n+1)); // Out of bounds high
}

TEST_F(TangoTreeTest, MassiveRandomStressTest) {
    const int N = 10;      // Number of nodes
    const int QUERIES = 50; // Total lookups
    
    TangoTree tree(N);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, N);

    // Perform massive amount of random lookups
    for (int i = 0; i < QUERIES; ++i) {
        int key = dis(gen);
        // Every key within [1, N] must be found
        ASSERT_TRUE(tree.contains(key)) << "Failed to find key: " << key;
    }
}