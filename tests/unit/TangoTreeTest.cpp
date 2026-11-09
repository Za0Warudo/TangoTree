#include "TangoTree.h"
#include "Checkers.h"
#include "Helpers.h"
#include <algorithm>
#include <gtest/gtest.h>
#include <random>
#include <set>
#include <string>
#include <vector>

const std::vector<int> TREE_SIZES = {1, 2, 3, 4, 5, 6, 7, 8, 15, 16, 17, 31, 32, 63, 100, 127, 255};

/* Construction. */
namespace Tests {

  TEST(TangoTreeTest, ConstructedTreesAreValid) {
  for (int n = 1; n <= 128; ++n) {              // construct trees of sizes 1 to 128
    SCOPED_TRACE("n=" + std::to_string(n));     // indicate the value of n in case of failure
    TangoTree tree(n);
    ReferenceTree ref = captureReference(tree, n);
    ASSERT_TRUE(isValidTango(tree, ref));
    EXPECT_EQ(tree.getSize(), n);
  }
}

TEST(TangoTreeTest, ConstructedTreeHasOneAuxiliaryTreePerKey) {
  for (int n : TREE_SIZES) {
    SCOPED_TRACE("n=" + std::to_string(n));
    TangoTree tree(n);
    EXPECT_EQ(getAllAuxiliary(tree).size(), static_cast<size_t>(n));     // every key should be in its own auxiliary tree
  }
}

TEST(TangoTreeTest, ReferenceTreeIsComplete) {
  for (int n = 1; n <= 128; ++n) {
    SCOPED_TRACE("n=" + std::to_string(n));
    TangoTree tree(n);
    ReferenceTree ref = captureReference(tree, n);

    // In a complete tree, the heap indices of the nodes are exactly 1 to n.
    std::vector<int> index(n + 1, 0);
    std::set<int> indices;
    for (int key : ref.byDepth) {
      const int parent = ref.parent[key];
      if (parent == 0) {
        EXPECT_EQ(ref.depth[key], 0);
        index[key] = 1;
      } else {
        EXPECT_EQ(ref.depth[key], ref.depth[parent] + 1);
        index[key] = 2 * index[parent] + (key > parent ? 1 : 0);
      }
      indices.insert(index[key]);
    }
    ASSERT_EQ(indices.size(), static_cast<size_t>(n));
    EXPECT_EQ(*indices.begin(), 1);
    EXPECT_EQ(*indices.rbegin(), n);
  }
}

/* Single accesses. */

TEST(TangoTreeTest, SingleElementTree) {
  TangoTree tree(1);
  ReferenceTree ref = captureReference(tree, 1);
  ASSERT_TRUE(isValidTango(tree, ref));
  EXPECT_TRUE(accessAndValidate(tree, ref, 1));
  EXPECT_TRUE(accessAndValidate(tree, ref, 0));
  EXPECT_TRUE(accessAndValidate(tree, ref, 2));
}

TEST(TangoTreeTest, BoundaryKeys) {
  const int n = 100;
  TangoTree tree(n);
  ReferenceTree ref = captureReference(tree, n);
  ASSERT_TRUE(isValidTango(tree, ref));
  EXPECT_TRUE(accessAndValidate(tree, ref, 1));
  EXPECT_TRUE(accessAndValidate(tree, ref, n));
  EXPECT_TRUE(accessAndValidate(tree, ref, 0));
  EXPECT_TRUE(accessAndValidate(tree, ref, n + 1));
}

TEST(TangoTreeTest, MissingKeysKeepTheTreeValid) {
  const int n = 63;
  TangoTree tree(n);
  ReferenceTree ref = captureReference(tree, n);
  ASSERT_TRUE(isValidTango(tree, ref));
  for (int key : {0, -1, -1000, n + 1, n + 1000})
    ASSERT_TRUE(accessAndValidate(tree, ref, key));
  for (int key = 1; key <= n; key += 7)
    ASSERT_TRUE(accessAndValidate(tree, ref, key));
  for (int key : {0, n + 1})
    ASSERT_TRUE(accessAndValidate(tree, ref, key));
}

TEST(TangoTreeTest, FirstAccessBuildsExactlyTheSearchPath) {
  for (int n : TREE_SIZES) {
    for (int key = 1; key <= n; ++key) {
      SCOPED_TRACE("n=" + std::to_string(n) + " key=" + std::to_string(key));
      TangoTree tree(n);
      ReferenceTree ref = captureReference(tree, n);
      ASSERT_TRUE(isValidTango(tree, ref));
      ASSERT_TRUE(accessAndValidate(tree, ref, key));

      std::vector<int> rootKeys;
      for (Node *node : auxNodes(tree.root))
        rootKeys.push_back(node->key);
      std::vector<int> expected = ref.pathTo(key);
      std::sort(expected.begin(), expected.end());
      ASSERT_EQ(rootKeys, expected);
    }
  }
}

TEST(TangoTreeTest, RepeatedAccessDoesNotChangeTheTree) {
  const int n = 127;
  TangoTree tree(n);
  ReferenceTree ref = captureReference(tree, n);
  ASSERT_TRUE(isValidTango(tree, ref));
  for (int key = 1; key <= n; key += 5) {
    ASSERT_TRUE(accessAndValidate(tree, ref, key));
    const std::vector<Node *> before = getAllAuxiliary(tree);
    ASSERT_TRUE(accessAndValidate(tree, ref, key));
    ASSERT_EQ(getAllAuxiliary(tree), before) << "key=" << key;
  }
}

/* Access sequences. */

TEST(TangoTreeTest, AscendingAccess) {
  for (int n : TREE_SIZES) {
    SCOPED_TRACE("n=" + std::to_string(n));
    TangoTree tree(n);
    ReferenceTree ref = captureReference(tree, n);
    ASSERT_TRUE(isValidTango(tree, ref));
    for (int key = 1; key <= n; ++key)
      ASSERT_TRUE(accessAndValidate(tree, ref, key));
  }
}

TEST(TangoTreeTest, DescendingAccess) {
  for (int n : TREE_SIZES) {
    SCOPED_TRACE("n=" + std::to_string(n));
    TangoTree tree(n);
    ReferenceTree ref = captureReference(tree, n);
    ASSERT_TRUE(isValidTango(tree, ref));
    for (int key = n; key >= 1; --key)
      ASSERT_TRUE(accessAndValidate(tree, ref, key));
  }
}

TEST(TangoTreeTest, AlternatingExtremes) {
  for (int n : TREE_SIZES) {
    SCOPED_TRACE("n=" + std::to_string(n));
    TangoTree tree(n);
    ReferenceTree ref = captureReference(tree, n);
    ASSERT_TRUE(isValidTango(tree, ref));
    for (int low = 1, high = n; low <= high; ++low, --high) {
      ASSERT_TRUE(accessAndValidate(tree, ref, low));
      ASSERT_TRUE(accessAndValidate(tree, ref, high));
    }
  }
}

TEST(TangoTreeTest, BitReversalAccess) {
  for (int bits = 1; bits <= 8; ++bits) {
    const int n = (1 << bits) - 1;
    SCOPED_TRACE("n=" + std::to_string(n));
    TangoTree tree(n);
    ReferenceTree ref = captureReference(tree, n);
    ASSERT_TRUE(isValidTango(tree, ref));
    for (int i = 1; i <= n; ++i) {
      int reversed = 0;
      for (int bit = 0; bit < bits; ++bit) {
        if (i & (1 << bit))
          reversed |= 1 << (bits - 1 - bit);
      }
      ASSERT_TRUE(accessAndValidate(tree, ref, reversed));
    }
  }
}

TEST(TangoTreeTest, AllPairsOfAccesses) {
  for (int n : {1, 2, 3, 4, 5, 7, 8, 15, 16, 31}) {
    for (int first = 1; first <= n; ++first) {
      for (int second = 1; second <= n; ++second) {
        SCOPED_TRACE("n=" + std::to_string(n) + " first=" + std::to_string(first) + " second=" + std::to_string(second));
        TangoTree tree(n);
        ReferenceTree ref = captureReference(tree, n);
        ASSERT_TRUE(isValidTango(tree, ref));
        ASSERT_TRUE(accessAndValidate(tree, ref, first));
        ASSERT_TRUE(accessAndValidate(tree, ref, second));
      }
    }
  }
}

TEST(TangoTreeTest, RandomPermutationAccess) {
  for (int n : TREE_SIZES) {
    for (unsigned seed = 1; seed <= 5; ++seed) {
      SCOPED_TRACE("n=" + std::to_string(n) + " seed=" + std::to_string(seed));
      TangoTree tree(n);
      ReferenceTree ref = captureReference(tree, n);
      ASSERT_TRUE(isValidTango(tree, ref));
      for (int key : generateRandomKeys(n, seed))
        ASSERT_TRUE(accessAndValidate(tree, ref, key));
    }
  }
}

TEST(TangoTreeTest, MassiveRandomStressTest) {
  struct Scenario {
    int n;
    int queries;
  };

  for (Scenario scenario : {Scenario{10, 2000}, Scenario{100, 2000}, Scenario{1023, 500}}) {
    for (unsigned seed = 1; seed <= 3; ++seed) {
      SCOPED_TRACE("n=" + std::to_string(scenario.n) + " seed=" + std::to_string(seed));
      TangoTree tree(scenario.n);
      ReferenceTree ref = captureReference(tree, scenario.n);
      ASSERT_TRUE(isValidTango(tree, ref));

      std::mt19937 generator(seed);
      // The range includes missing keys around the valid ones.
      std::uniform_int_distribution<> distribution(0, scenario.n + 1);
      for (int query = 0; query < scenario.queries; ++query)
        ASSERT_TRUE(accessAndValidate(tree, ref, distribution(generator))) << "query=" << query;
    }
  }
}

}     // namespace Tests