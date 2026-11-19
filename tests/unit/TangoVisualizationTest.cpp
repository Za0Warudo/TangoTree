/**
 * @file TangoVisualizationTest.cpp
 *
 * @brief Unit tests for the Tango Tree copy that records the steps of its searches. They check two things: that the copy always builds exactly the same tree
 * as the original TangoTree, and that every recorded frame is a picture that can be drawn (a forest with every key once, in sorted order, with no link passing
 * over a node or over another link).
 */

#include "TangoVisualization.h"
#include "Helpers.h"
#include "TangoTree.h"
#include <algorithm>
#include <cmath>
#include <gtest/gtest.h>
#include <random>
#include <string>
#include <vector>

namespace Tests {

const std::vector<int> VISUAL_SIZES = {15, 31, 63};                                     // the sizes offered by the visualization
const std::vector<int> ALL_SIZES = {1, 2, 3, 4, 5, 6, 7, 8, 15, 16, 17, 31, 32, 63, 100};     // the copy itself works with any size

/* Helpers. */

/**
 * @brief Checks that two subtrees are identical: same keys, same shape, same colors and same marks.
 */
::testing::AssertionResult sameTree(Node *a, Node *b) {
  if (a == Node::nil || b == Node::nil) {
    if (a == b)
      return ::testing::AssertionSuccess();
    return ::testing::AssertionFailure() << "one tree has the key " << (a == Node::nil ? b->key : a->key) << " where the other has nothing";
  }
  if (a->key != b->key)
    return ::testing::AssertionFailure() << "key " << a->key << " in one tree is " << b->key << " in the other";
  if (a->color != b->color || a->isExternal != b->isExternal || a->blackHeight != b->blackHeight || a->depth != b->depth)
    return ::testing::AssertionFailure() << "key " << a->key << " has different fields in the two trees";
  ::testing::AssertionResult leftSide = sameTree(a->left, b->left);
  if (!leftSide)
    return leftSide;
  return sameTree(a->right, b->right);
}

/**
 * @brief The position of every node of a frame, computed like the page does: the column is the key, and the level is the depth inside its tree, plus the level
 * of the node its tree is drawn below.
 */
struct FrameLayout {
  std::vector<int> level;      // The level of each key, or -1 if the key was not reached.
  std::vector<int> parent;     // The parent of each key, or 0.
  bool valid = true;           // False if a key was reached twice, or an anchor forms a cycle.
};

void placeSubtree(const TraceFrame &frame, int key, int level, FrameLayout &layout) {
  if (key == 0)
    return;
  if (layout.level[key] != -1) {
    layout.valid = false;
    return;
  }
  layout.level[key] = level;
  for (int child : {frame.nodes[key].left, frame.nodes[key].right}) {
    if (child != 0)
      layout.parent[child] = key;
    placeSubtree(frame, child, level + 1, layout);
  }
}

FrameLayout computeLayout(const TraceFrame &frame) {
  const int n = static_cast<int>(frame.nodes.size()) - 1;
  FrameLayout layout{std::vector<int>(n + 1, -1), std::vector<int>(n + 1, 0)};

  // A tree can only be placed after the tree of its anchor, so repeat until nothing is left.
  std::vector<bool> placed(frame.roots.size(), false);
  for (size_t round = 0; round < frame.roots.size(); ++round) {
    for (size_t i = 0; i < frame.roots.size(); ++i) {
      const int anchor = frame.anchors[i];
      if (placed[i] || (anchor != 0 && layout.level[anchor] == -1))
        continue;
      placeSubtree(frame, frame.roots[i], anchor == 0 ? 0 : layout.level[anchor] + 1, layout);
      placed[i] = true;
    }
  }
  if (std::count(placed.begin(), placed.end(), false) > 0)
    layout.valid = false;
  return layout;
}

/**
 * @brief Collects, in-order, the keys of the subtree of a frame.
 */
void collectInOrder(const TraceFrame &frame, int key, std::vector<int> &keys) {
  if (key == 0 || keys.size() > frame.nodes.size())     // the size check stops a cycle from recursing forever
    return;
  collectInOrder(frame, frame.nodes[key].left, keys);
  keys.push_back(key);
  collectInOrder(frame, frame.nodes[key].right, keys);
}

/**
 * @brief Checks that the frame is a forest that holds every key exactly once, with every tree in sorted order.
 */
::testing::AssertionResult isValidForest(const TraceFrame &frame, int n) {
  if (static_cast<int>(frame.nodes.size()) != n + 1)
    return ::testing::AssertionFailure() << "the frame has " << frame.nodes.size() - 1 << " nodes instead of " << n;
  if (frame.roots.empty() || frame.roots.size() != frame.anchors.size())
    return ::testing::AssertionFailure() << "the frame has " << frame.roots.size() << " roots and " << frame.anchors.size() << " anchors";
  if (!std::is_sorted(frame.roots.begin(), frame.roots.end()))
    return ::testing::AssertionFailure() << "the roots are not in ascending order";
  if (frame.text.empty())
    return ::testing::AssertionFailure() << "the frame has no description";

  for (int key : frame.focus) {
    if (key < 1 || key > n)
      return ::testing::AssertionFailure() << "the focus has the key " << key << ", which is out of range";
  }
  for (int key : frame.path) {
    if (key < 1 || key > n)
      return ::testing::AssertionFailure() << "the path has the key " << key << ", which is out of range";
  }

  FrameLayout layout = computeLayout(frame);
  if (!layout.valid)
    return ::testing::AssertionFailure() << "a key is in two places, or a tree is drawn below itself";
  for (int key = 1; key <= n; ++key) {
    if (layout.level[key] == -1)
      return ::testing::AssertionFailure() << "key " << key << " is in no tree";
  }

  for (int root : frame.roots) {
    std::vector<int> keys;
    collectInOrder(frame, root, keys);
    if (!std::is_sorted(keys.begin(), keys.end()))
      return ::testing::AssertionFailure() << "the tree rooted at " << root << " is not in sorted order";
  }
  return ::testing::AssertionSuccess();
}

/**
 * @brief The distance from the point (px, py) to the segment from (ax, ay) to (bx, by).
 */
double distanceToSegment(double px, double py, double ax, double ay, double bx, double by) {
  const double dx = bx - ax, dy = by - ay;
  double t = ((px - ax) * dx + (py - ay) * dy) / (dx * dx + dy * dy);
  t = std::max(0.0, std::min(1.0, t));
  return std::hypot(px - (ax + t * dx), py - (ay + t * dy));
}

/**
 * @brief Checks if the segments a-b and c-d cross at a point that is not an end of either one.
 */
bool segmentsCross(double ax, double ay, double bx, double by, double cx, double cy, double dx, double dy) {
  auto side = [](double px, double py, double qx, double qy, double rx, double ry) { return (qx - px) * (ry - py) - (qy - py) * (rx - px); };
  const double d1 = side(cx, cy, dx, dy, ax, ay), d2 = side(cx, cy, dx, dy, bx, by);
  const double d3 = side(ax, ay, bx, by, cx, cy), d4 = side(ax, ay, bx, by, dx, dy);
  return ((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) && ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0));
}

/**
 * @brief Checks that the frame can be drawn cleanly, using the same sizes as the page: no link passes over a node, and no two links cross.
 */
::testing::AssertionResult isCleanDrawing(const TraceFrame &frame, int n) {
  // The sizes used by visualization/viewer.js.
  const double columnWidth = 1140.0 / n;
  const double radius = std::max(8.0, std::min(20.0, columnWidth * 0.46));
  const double rowHeight = std::max(44.0, radius * 3.2);

  FrameLayout layout = computeLayout(frame);
  auto x = [&](int key) { return key * columnWidth; };
  auto y = [&](int key) { return layout.level[key] * rowHeight; };

  for (int child = 1; child <= n; ++child) {
    const int parent = layout.parent[child];
    if (parent == 0)
      continue;

    for (int key = 1; key <= n; ++key) {
      if (key == child || key == parent)
        continue;
      if (distanceToSegment(x(key), y(key), x(child), y(child), x(parent), y(parent)) < radius + 1)
        return ::testing::AssertionFailure() << "the link from " << parent << " to " << child << " passes over the node " << key;
    }

    for (int other = child + 1; other <= n; ++other) {
      const int otherParent = layout.parent[other];
      if (otherParent == 0)
        continue;
      if (segmentsCross(x(child), y(child), x(parent), y(parent), x(other), y(other), x(otherParent), y(otherParent)))
        return ::testing::AssertionFailure() << "the link from " << parent << " to " << child << " crosses the link from " << otherParent << " to " << other;
    }
  }
  return ::testing::AssertionSuccess();
}

/**
 * @brief Checks that the frame shows exactly the given tree: one tree, drawn at the top, with the same links, colors and marks as the real nodes.
 */
::testing::AssertionResult showsTheTree(const TraceFrame &frame, Node *root) {
  if (frame.roots.size() != 1 || frame.roots[0] != root->key || frame.anchors[0] != 0)
    return ::testing::AssertionFailure() << "the frame is not a single tree rooted at " << root->key;

  std::vector<Node *> pending = {root};
  size_t count = 0;
  while (!pending.empty()) {
    Node *node = pending.back();
    pending.pop_back();
    ++count;
    const TraceNode &shown = frame.nodes[node->key];
    const int left = node->left == Node::nil ? 0 : node->left->key;
    const int right = node->right == Node::nil ? 0 : node->right->key;
    if (shown.left != left || shown.right != right)
      return ::testing::AssertionFailure() << "key " << node->key << " has the children " << left << " and " << right << ", but the frame shows " << shown.left
                                           << " and " << shown.right;
    if (shown.red != (node->color == RED) || shown.external != node->isExternal)
      return ::testing::AssertionFailure() << "key " << node->key << " is shown with the wrong color or mark";
    if (node->left != Node::nil)
      pending.push_back(node->left);
    if (node->right != Node::nil)
      pending.push_back(node->right);
  }
  if (count != frame.nodes.size() - 1)
    return ::testing::AssertionFailure() << "the tree has " << count << " nodes, but the frame has " << frame.nodes.size() - 1;
  return ::testing::AssertionSuccess();
}

/**
 * @brief Returns a sequence of searches that mixes random keys, scans and repeated keys, including keys that are not in the tree.
 */
std::vector<int> mixedSearches(int n, unsigned seed) {
  std::vector<int> searches = generateRandomKeys(n, seed);
  std::mt19937 generator(seed + 1);
  std::uniform_int_distribution<int> anyKey(0, n + 1);
  for (int i = 0; i < 2 * n; ++i)
    searches.push_back(anyKey(generator));
  for (int key = 1; key <= n; ++key)
    searches.push_back(key);
  for (int key = n; key >= 1; --key)
    searches.push_back(key);
  for (int i = 0; i < n; ++i)
    searches.push_back(i % 2 == 0 ? 1 + i / 2 : n - i / 2);
  searches.push_back(searches.back());
  return searches;
}

/* The copy against the original. */

TEST(TangoVisualizationTest, BuildsTheSameTreeAsTheTangoTree) {
  for (int n : ALL_SIZES) {
    SCOPED_TRACE("n=" + std::to_string(n));
    TangoTree original(n);
    TangoVisualization traced(n);
    ASSERT_TRUE(sameTree(original.root, traced.root));
    EXPECT_EQ(traced.getSize(), n);
  }
}

TEST(TangoVisualizationTest, EverySearchLeavesTheSameTreeAsTheTangoTree) {
  for (int n : ALL_SIZES) {
    TangoTree original(n);
    TangoVisualization traced(n);
    for (int key : mixedSearches(n, 100 + n)) {
      SCOPED_TRACE("n=" + std::to_string(n) + ", key=" + std::to_string(key));
      ASSERT_EQ(traced.contains(key), original.contains(key));
      ASSERT_TRUE(sameTree(original.root, traced.root));
    }
  }
}

/* The recorded frames. */

TEST(TangoVisualizationTest, NewTreeHasOneFrameWithTheWholeTree) {
  for (int n : ALL_SIZES) {
    SCOPED_TRACE("n=" + std::to_string(n));
    TangoVisualization traced(n);
    ASSERT_EQ(traced.getFrames().size(), 1u);
    ASSERT_TRUE(isValidForest(traced.getFrames().front(), n));
    EXPECT_TRUE(showsTheTree(traced.getFrames().front(), traced.root));
  }
}

TEST(TangoVisualizationTest, EveryFrameIsAValidForest) {
  for (int n : ALL_SIZES) {
    TangoVisualization traced(n);
    for (int key : mixedSearches(n, 200 + n)) {
      traced.search(key);
      const std::vector<TraceFrame> &frames = traced.getFrames();
      ASSERT_GE(frames.size(), 2u);
      for (size_t i = 0; i < frames.size(); ++i) {
        SCOPED_TRACE("n=" + std::to_string(n) + ", key=" + std::to_string(key) + ", frame=" + std::to_string(i) + ": " + frames[i].text);
        ASSERT_TRUE(isValidForest(frames[i], n));
        if (i + 1 < frames.size())
          ASSERT_FALSE(frames[i].context.empty());
      }
    }
  }
}

TEST(TangoVisualizationTest, SearchStartsAndEndsWithTheRealTree) {
  for (int n : ALL_SIZES) {
    TangoVisualization traced(n);
    TraceFrame previous = traced.getFrames().back();
    for (int key : mixedSearches(n, 300 + n)) {
      SCOPED_TRACE("n=" + std::to_string(n) + ", key=" + std::to_string(key));
      Node *rootBefore = traced.root;
      traced.search(key);

      // The first frame shows the tree as the previous search left it, since the first step only looks at the root.
      const TraceFrame &first = traced.getFrames().front();
      ASSERT_EQ(first.roots, std::vector<int>{rootBefore->key});
      for (int k = 1; k <= n; ++k) {
        ASSERT_EQ(first.nodes[k].left, previous.nodes[k].left);
        ASSERT_EQ(first.nodes[k].right, previous.nodes[k].right);
        ASSERT_EQ(first.nodes[k].red, previous.nodes[k].red);
      }

      // The last frame shows the tree as it really is now.
      ASSERT_TRUE(showsTheTree(traced.getFrames().back(), traced.root));
      previous = traced.getFrames().back();
    }
  }
}

TEST(TangoVisualizationTest, EveryFrameCanBeDrawnCleanly) {
  for (int n : VISUAL_SIZES) {
    TangoVisualization traced(n);
    for (int key : mixedSearches(n, 400 + n)) {
      traced.search(key);
      const std::vector<TraceFrame> &frames = traced.getFrames();
      for (size_t i = 0; i < frames.size(); ++i) {
        SCOPED_TRACE("n=" + std::to_string(n) + ", key=" + std::to_string(key) + ", frame=" + std::to_string(i) + ": " + frames[i].text);
        ASSERT_TRUE(isCleanDrawing(frames[i], n));
      }
    }
  }
}

TEST(TangoVisualizationTest, SearchPathIsOnlyShownWhileSearching) {
  TangoVisualization traced(31);
  traced.search(11);
  const std::vector<TraceFrame> &frames = traced.getFrames();
  EXPECT_FALSE(frames[1].path.empty());
  EXPECT_TRUE(frames.back().path.empty());
  EXPECT_EQ(frames.back().focus, std::vector<int>{11});
}

/* JSON. */

TEST(TangoVisualizationTest, JsonHasOneObjectPerFrame) {
  for (int n : VISUAL_SIZES) {
    SCOPED_TRACE("n=" + std::to_string(n));
    TangoVisualization traced(n);
    traced.search(n / 3);
    const std::string json = traced.toJson();

    size_t count = 0;
    for (size_t at = json.find("\"ctx\""); at != std::string::npos; at = json.find("\"ctx\"", at + 1))
      ++count;
    EXPECT_EQ(count, traced.getFrames().size());

    EXPECT_EQ(json.rfind("{\"n\":" + std::to_string(n) + ",\"key\":" + std::to_string(n / 3) + ",\"found\":true", 0), 0u);
    EXPECT_EQ(std::count(json.begin(), json.end(), '{'), std::count(json.begin(), json.end(), '}'));
    EXPECT_EQ(std::count(json.begin(), json.end(), '['), std::count(json.begin(), json.end(), ']'));
    EXPECT_EQ(std::count(json.begin(), json.end(), '"') % 2, 0);
  }
}

}     // namespace Tests
