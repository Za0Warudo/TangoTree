#ifndef TANGOVISUALIZATION_H
#define TANGOVISUALIZATION_H

/**
 * @file TangoVisualization.h
 *
 * @brief A header file for a copy of the Tango Tree that records every step of a search, so the search can be shown as an animation. The algorithm is the same
 * as in TangoTree.h and RedBlackTree.h: the same searches, cuts, pastes, splits, joins, rotations and color changes, in the same order. The only difference is
 * that each of those steps also saves a picture of the tree, called a frame.
 *
 * A search does not keep the tree in one piece. A split breaks a tree in three parts, and a join puts parts back together. So a frame holds a forest: one or
 * more separate trees that, together, contain every key exactly once.
 *
 * This copy is meant for small trees. Each frame stores the whole tree, so a search costs O(N) memory per step.
 *
 * @version 1.0
 * @date 2026-11-15
 */

// includes.
#include "RedBlackTree.h"
#include <string>
#include <vector>

/**
 * @brief The state of one node in a frame.
 */
struct TraceNode {
  int left = 0;              // The key of the left child, or 0 if there is none.
  int right = 0;             // The key of the right child, or 0 if there is none.
  bool red = false;          // True if the node is red, false if it is black.
  bool external = false;     // True if the node is marked as the root of an auxiliary tree.
};

/**
 * @brief One step of a search: the whole forest right after the step, and a description of what the step did.
 */
struct TraceFrame {
  std::vector<std::string> context;     // The operations in progress, from the outermost to the innermost. For example: search 5, tango 6, cut, split 4.
  std::string text;                     // What this step does, in one or two sentences.
  std::vector<TraceNode> nodes;         // nodes[k] is the state of the key k. The position 0 is not used.
  std::vector<int> roots;               // The root key of each separate tree in the forest, in ascending order.
  std::vector<int> anchors;             // anchors[i] is the key of the node that the tree roots[i] is drawn below, or 0 if it is drawn at the top.
  std::vector<int> focus;               // The keys this step acts on.
  std::vector<int> path;                // The keys visited by the search so far.
};

/**
 * @brief A class representing a Tango Tree that records the steps of its searches. It has the same operations as the TangoTree class, plus the methods to read
 * the recorded frames.
 */
class TangoVisualization {
public:
  Node *root;     // The Tango Tree's root node.
  int size;       // The size of the Tango Tree.

  /**
   * @brief Construct a new Tango Visualization object, with one frame that shows the newly built tree.
   * @param n The number of nodes in the reference tree. The constructor will build a Tango Tree based on a reference tree with n nodes (1 to n).
   */
  TangoVisualization(int n);

  /**
   * @brief Frees the nodes of the tree.
   */
  ~TangoVisualization();

  // A copy would share the nodes with the original, so copies are not allowed.
  TangoVisualization(const TangoVisualization &) = delete;
  TangoVisualization &operator=(const TangoVisualization &) = delete;

  // Operations. //

  /**
   * @brief Searches for the given key in the Tango Tree, and records every step of the search. The frames of the previous search are discarded.
   *
   * @param key The key to search for in the Tango Tree.
   * @return A pointer to the node with the given key if found, or the nil node otherwise.
   */
  Node *search(int key);

  /**
   * @brief Checks if the Tango Tree contains the given key. This operation performs a search for the specified key, so it also records its steps.
   *
   * @param key The key to search for in the Tango Tree.
   * @return true if the key is found in the Tango Tree, false otherwise.
   */
  bool contains(int key);

  /**
   * @brief Gets the size of the Tango Tree.
   * @return The number of nodes in the Tango Tree.
   */
  int getSize();

  // Recorded steps. //

  /**
   * @brief Gets the frames recorded by the last search, in the order they happened. Before the first search, there is a single frame with the newly built
   * tree.
   * @return The recorded frames.
   */
  const std::vector<TraceFrame> &getFrames() const;

  /**
   * @brief Gets the depth of every key in the reference tree.
   * @return A vector where the position k holds the depth of the key k. The position 0 is not used.
   */
  std::vector<int> getDepths() const;

  /**
   * @brief Writes the recorded frames as JSON, in the format read by the page in the visualization directory.
   * @return The JSON text.
   */
  std::string toJson() const;

private:
  std::vector<Node *> nodes;         // nodes[k] is the node with the key k. The position 0 is not used.
  std::vector<TraceFrame> frames;    // The frames recorded by the last search.
  int lastKey;                       // The key of the last search, or 0 before the first one.
  bool lastFound;                    // True if the last search found its key.
};

#endif     // TANGOVISUALIZATION_H
