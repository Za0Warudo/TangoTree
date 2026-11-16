/**
 * @file TreeBenchmark.cpp
 *
 * @brief Measures a single run of one tree over one access sequence. It is meant to be driven by compare_trees.py, which runs it many times, so every
 * measurement starts in a new process.
 *
 * Usage: TreeBenchmark <tree> <workload> <n> <queries> <seed>
 *  - tree: tango, rbt, or splay.
 *  - workload: random, sequential, bitreversal, or zipf.
 *  - n: The number of keys, which are the integers from 1 to n.
 *  - queries: The number of searches to perform.
 *  - seed: The random seed. The same seed gives every tree the same insertion order and the same searches.
 *
 * Output: A single line with the build time in milliseconds, the search time in milliseconds, the number of searches that found their key, and the number of
 * node accesses made by the searches. The last value is always 0, unless the program is compiled with COUNT_NODE_ACCESSES.
 */

// includes.
#include "RedBlackTree.h"
#include "SplayTree.h"
#include "TangoTree.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <exception>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

/* Tree adapters. Each one builds a tree with the given keys and exposes the same search method, so the measurement is the same code for all of them. */

struct TangoAdapter {
  TangoTree tree;

  explicit TangoAdapter(const std::vector<int> &keys) : tree(static_cast<int>(keys.size())) {}

  bool contains(int key) { return tree.contains(key); }
};

struct RedBlackAdapter {
  Node *root = Node::nil;

  explicit RedBlackAdapter(const std::vector<int> &keys) {
    for (int key : keys)
      root = insert(root, key);
  }

  bool contains(int key) { return !search(root, key)->isExternal; }
};

struct SplayAdapter {
  SplayTree tree;

  explicit SplayAdapter(const std::vector<int> &keys) {
    for (int key : keys)
      tree.insert(key);
  }

  bool contains(int key) { return tree.contains(key); }
};

/* Workloads. */

/**
 * @brief Returns the keys 1 to n in a random order.
 *
 * @param n The number of keys.
 * @param generator The random generator.
 * @return The shuffled keys.
 */
std::vector<int> shuffledKeys(int n, std::mt19937 &generator) {
  std::vector<int> keys(n);
  std::iota(keys.begin(), keys.end(), 1);
  std::shuffle(keys.begin(), keys.end(), generator);
  return keys;
}

/**
 * @brief Generates searches for uniformly random keys.
 *
 * @param n The number of keys.
 * @param queries The number of searches.
 * @param generator The random generator.
 * @return The keys to search for.
 */
std::vector<int> randomWorkload(int n, int queries, std::mt19937 &generator) {
  std::uniform_int_distribution<int> distribution(1, n);
  std::vector<int> result(queries);
  for (int &key : result)
    key = distribution(generator);
  return result;
}

/**
 * @brief Generates searches that scan the keys in ascending order, starting over at the end.
 *
 * @param n The number of keys.
 * @param queries The number of searches.
 * @return The keys to search for.
 */
std::vector<int> sequentialWorkload(int n, int queries) {
  std::vector<int> result(queries);
  for (int i = 0; i < queries; ++i)
    result[i] = i % n + 1;
  return result;
}

/**
 * @brief Generates searches in bit-reversal order, starting over at the end. This is a classic hard sequence for binary search trees, since consecutive searches
 * keep alternating between far apart parts of the tree.
 *
 * @param n The number of keys.
 * @param queries The number of searches.
 * @return The keys to search for.
 */
std::vector<int> bitReversalWorkload(int n, int queries) {
  int bits = 0;
  while ((1 << bits) <= n)
    ++bits;

  std::vector<int> cycle;
  for (int i = 1; i < (1 << bits); ++i) {
    int reversed = 0;
    for (int bit = 0; bit < bits; ++bit) {
      if (i & (1 << bit))
        reversed |= 1 << (bits - 1 - bit);
    }
    if (reversed <= n)
      cycle.push_back(reversed);
  }

  std::vector<int> result(queries);
  for (int i = 0; i < queries; ++i)
    result[i] = cycle[i % cycle.size()];
  return result;
}

/**
 * @brief Generates searches where a few keys are very popular, following approximately a Zipf distribution. The popular keys are spread randomly over the key
 * range, so they are not all neighbors in the tree.
 *
 * @param n The number of keys.
 * @param queries The number of searches.
 * @param generator The random generator.
 * @return The keys to search for.
 */
std::vector<int> zipfWorkload(int n, int queries, std::mt19937 &generator) {
  const std::vector<int> keyOfRank = shuffledKeys(n, generator);
  std::uniform_real_distribution<double> unit(0.0, 1.0);
  std::vector<int> result(queries);
  for (int &key : result) {
    const int rank = std::min(n, static_cast<int>(std::pow(n, unit(generator))));
    key = keyOfRank[rank - 1];
  }
  return result;
}

/* Measurement. */

/**
 * @brief Returns the number of node accesses so far.
 *
 * @return The number of accesses, or 0 if the program is compiled without counting them.
 */
long long nodeAccessesSoFar() {
#ifdef COUNT_NODE_ACCESSES
  return nodeAccessCount;
#else
  return 0;
#endif
}

/**
 * @brief Builds a tree, performs the searches, and prints the result. The workload is already generated at this point, so it is not part of the time.
 *
 * @tparam Tree The tree adapter to measure.
 * @param insertionOrder The keys, in the order the tree should insert them.
 * @param queries The keys to search for.
 */
template <typename Tree>
void measure(const std::vector<int> &insertionOrder, const std::vector<int> &queries) {
  using Clock = std::chrono::steady_clock;
  using Milliseconds = std::chrono::duration<double, std::milli>;

  const auto start = Clock::now();
  Tree tree(insertionOrder);
  const auto built = Clock::now();
  const long long accessesBeforeSearches = nodeAccessesSoFar();

  long long found = 0;
  for (int key : queries)
    found += tree.contains(key) ? 1 : 0;
  const auto finished = Clock::now();

  std::cout << std::fixed << std::setprecision(3) << Milliseconds(built - start).count() << ' ' << Milliseconds(finished - built).count() << ' ' << found << ' ' << nodeAccessesSoFar() - accessesBeforeSearches << '\n';
}

/**
 * @brief Prints how to call the program, to the error output.
 *
 * @param program The name of the program.
 * @return The exit code for invalid arguments.
 */
int usage(const char *program) {
  std::cerr << "Usage: " << program << " <tango|rbt|splay> <random|sequential|bitreversal|zipf> <n> <queries> <seed>\n";
  return 2;
}

int main(int argc, char **argv) {
  if (argc != 6)
    return usage(argv[0]);

  const std::string tree = argv[1];
  const std::string workload = argv[2];
  int n = 0;
  int queries = 0;
  unsigned seed = 0;
  try {
    n = std::stoi(argv[3]);
    queries = std::stoi(argv[4]);
    seed = static_cast<unsigned>(std::stoul(argv[5]));
  } catch (const std::exception &) {
    return usage(argv[0]);
  }
  if (n < 1 || queries < 1)
    return usage(argv[0]);

  std::mt19937 generator(seed);
  const std::vector<int> insertionOrder = shuffledKeys(n, generator);

  std::vector<int> searches;
  if (workload == "random")
    searches = randomWorkload(n, queries, generator);
  else if (workload == "sequential")
    searches = sequentialWorkload(n, queries);
  else if (workload == "bitreversal")
    searches = bitReversalWorkload(n, queries);
  else if (workload == "zipf")
    searches = zipfWorkload(n, queries, generator);
  else
    return usage(argv[0]);

  if (tree == "tango")
    measure<TangoAdapter>(insertionOrder, searches);
  else if (tree == "rbt")
    measure<RedBlackAdapter>(insertionOrder, searches);
  else if (tree == "splay")
    measure<SplayAdapter>(insertionOrder, searches);
  else
    return usage(argv[0]);

  return 0;
}
