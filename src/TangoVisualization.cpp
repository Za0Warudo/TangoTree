/**
 * @file TangoVisualization.cpp
 *
 * @brief Implements the interface defined in TangoVisualization.h: a copy of the Tango Tree that records every step of a search.
 *
 * The file has two parts:
 *
 *  - The tracer, which records the frames. It keeps its own picture of the forest (which node is below which), updates it at every step, and saves a frame.
 *  - The algorithm, copied from TangoTree.cpp and RedBlackTree.cpp. Every function that changes the tree is copied here with one extra parameter, the tracer,
 *    and one extra call for each step it takes. Nothing else is changed, so both versions always build the same tree. The functions with no step to show
 *    (newNode, update, min, max, parentSearch, and the search that is not shown) are used directly from RedBlackTree.h.
 *
 * The tracer needs its own picture of the forest because the pointers of the nodes are not always up to date in the middle of an operation. The functions are
 * recursive, and a parent only receives its new child when the recursive call returns. Until then, the parent still points to the old child.
 *
 * @version 1.0
 * @date 2026-11-15
 */

// includes.
#include "TangoVisualization.h"
#include "RedBlackTree.h"
#include <cassert>
#include <string>
#include <tuple>
#include <vector>

namespace {

/* The tracer. */

/**
 * @brief Returns the key of the given node, or 0 for the nil node.
 */
int keyOf(Node *x) { return x == Node::nil ? 0 : x->key; }

/**
 * @brief Returns the key of the given node as text.
 */
std::string name(Node *x) { return std::to_string(x->key); }

/**
 * @brief Describes the tree with the given root, to be used in the middle of a sentence.
 */
std::string treeName(Node *root) {
  if (root == Node::nil)
    return "an empty tree";
  if (root->isExternal)
    return "the auxiliary tree of " + name(root);
  return "the tree rooted at " + name(root);
}

/**
 * @brief Records the steps of a search as frames.
 *
 * The tracer keeps the forest in its own arrays, indexed by key. Each method below is called by the algorithm right after it takes the step the method is
 * named after. The method applies the same step to the arrays and saves a frame. The colors and the external marks are read from the real nodes.
 *
 * A tree of the forest that is not attached to anything has an anchor: the key of the node it is drawn below, or 0 if it is drawn at the top. For example,
 * while the search works on an auxiliary tree that hangs below the node p, that tree and the parts it is broken into have p as their anchor.
 */
class Tracer {
public:
  /**
   * @brief Creates a tracer for the tree with the given root. The pointers of the tree must be up to date, which is true between two searches.
   *
   * @param nodes The nodes of the tree, indexed by key.
   * @param root The tango tree root node.
   * @param frames The vector to store the recorded frames.
   */
  Tracer(const std::vector<Node *> &nodes, Node *root, std::vector<TraceFrame> &frames) : nodes(nodes), frames(frames) {
    n = static_cast<int>(nodes.size()) - 1;
    left.assign(n + 1, 0);
    right.assign(n + 1, 0);
    parent.assign(n + 1, 0);
    anchor.assign(n + 1, 0);
    isRoot.assign(n + 1, false);
    for (int k = 1; k <= n; ++k) {
      left[k] = keyOf(nodes[k]->left);
      right[k] = keyOf(nodes[k]->right);
      if (left[k] != 0)
        parent[left[k]] = k;
      if (right[k] != 0)
        parent[right[k]] = k;
    }
    isRoot[root->key] = true;
  }

  /**
   * @brief Saves a frame with the current forest.
   *
   * @param text What the step does.
   * @param focus The nodes the step acts on. The nil node is ignored.
   */
  void step(const std::string &text, const std::vector<Node *> &focus = {}) {
    TraceFrame frame;
    frame.context = context;
    frame.text = text;
    frame.nodes.resize(n + 1);
    for (int k = 1; k <= n; ++k) {
      frame.nodes[k].left = left[k];
      frame.nodes[k].right = right[k];
      frame.nodes[k].red = nodes[k]->color == RED;
      frame.nodes[k].external = nodes[k]->isExternal;
      if (isRoot[k]) {
        frame.roots.push_back(k);
        frame.anchors.push_back(anchor[k]);
      }
    }
    for (Node *node : focus) {
      if (node != Node::nil)
        frame.focus.push_back(node->key);
    }
    frame.path = path;
    frames.push_back(frame);
  }

  /* Search steps. */

  void searchBegin(int key) { context.push_back("search " + std::to_string(key)); }

  void searchEnterTree(Node *h, int key) {
    if (anchor[h->key] == 0)
      step("Search for " + std::to_string(key) + ". Start at " + name(h) + ", the root of the tango tree.", {h});
    else
      step("Continue in the auxiliary tree of " + name(h) + ". It is drawn apart from the tree above while the search works on it.", {h});
  }

  void searchStep(Node *h, int key) {
    visit(h);
    const bool goLeft = key < h->key;
    step(std::to_string(key) + (goLeft ? " is smaller than " : " is greater than ") + name(h) + ": go " + (goLeft ? "left." : "right."), {h});
  }

  void searchEnd(Node *h, int key) {
    if (h == Node::nil) {
      step("Reached an empty position: " + std::to_string(key) + " is not in the tree.");
    } else if (h->isExternal) {
      visit(h);
      step("Reached " + name(h) + ", the root of another auxiliary tree. " + std::to_string(key) + " is not on this path, so the search goes on below.", {h});
    } else {
      visit(h);
      step("Found " + name(h) + " in this auxiliary tree.", {h});
    }
  }

  /**
   * @brief The search moves to the auxiliary tree of q, which hangs below p. From now on that tree is drawn apart, below p.
   */
  void searchGoDown(Node *p, Node *q) { detach(q->key, p->key); }

  /**
   * @brief The search is done with the auxiliary tree of q, so it hangs below p again.
   */
  void searchComeBack(Node *p, bool onLeft, Node *q) {
    path.clear();
    attach(p->key, onLeft, q->key);
    step("Back in the tree above: " + treeName(q) + " hangs below " + name(p) + " again. The tango operation now brings it into this path.", {q, p});
  }

  void searchFinish(int key, Node *result) {
    path.clear();
    if (result != Node::nil)
      step("Done. " + name(result) + " is now in the top auxiliary tree, together with every node that is above it in the reference tree.", {result});
    else
      step("Done. " + std::to_string(key) + " is not in the tree. The nodes the search went through are now in the top auxiliary tree.");
    context.pop_back();
  }

  /* Tango steps. */

  void tangoBegin(Node *root, Node *q) {
    context.push_back("tango " + name(q));
    if (root->maxDepth < q->minDepth)
      step("No cut is needed: this path ends at depth " + std::to_string(root->maxDepth) + " of the reference tree, and " + treeName(q) +
               " starts at depth " + std::to_string(q->minDepth) + ".",
           {q});
  }

  void tangoEnd() { context.pop_back(); }

  void cutBegin(Node *root, int depth, int pred, int succ) {
    context.push_back("cut depth " + std::to_string(depth) + "+");
    std::string range;
    if (pred > -1 && succ > -1)
      range = "the keys between " + std::to_string(pred) + " and " + std::to_string(succ);
    else if (pred > -1)
      range = "the keys greater than " + std::to_string(pred);
    else
      range = "the keys smaller than " + std::to_string(succ);
    std::vector<Node *> deep;
    collectDeep(root, depth, deep);
    step("Cut: the nodes at depth " + std::to_string(depth) + " or more in the reference tree leave this path. They are " + range + ".", deep);
  }

  void cutMiddle(Node *tm) {
    step("The nodes that left the path are now " + treeName(tm) + ". From here on it is treated as a single leaf.", {tm});
  }

  void cutEnd(Node *tm) {
    step("Cut done. " + name(tm) + " and the nodes that left with it hang below the path as their own auxiliary tree.", {tm});
    context.pop_back();
  }

  void pasteBegin(Node *q, Node *p) {
    context.push_back("paste " + name(q));
    const bool onLeft = p->left == q;
    detach(q->key, p->key);
    step("Paste: the tree of " + name(q) + ", which hung on the " + (onLeft ? "left" : "right") + " of " + name(p) + ", will be merged into the path.", {q, p});
  }

  /**
   * @brief The subtree that hung on one side of the removed node now hangs on the same side of p. The removed node and the rest of its tree are then drawn
   * one level lower, below the moved subtree, so the new link does not pass over them.
   */
  void pasteMove(Node *removed, Node *rest, Node *p, bool onLeft) {
    const int moved = onLeft ? left[removed->key] : right[removed->key];
    if (moved == 0)
      return;
    cut(removed->key, moved);
    attach(p->key, onLeft, moved);
    anchor[removed->key] = moved;
    if (rest != Node::nil)
      anchor[rest->key] = moved;
    const std::string side = onLeft ? "left" : "right";
    step(capitalized(treeName(nodes[moved])) + ", which hung on the " + side + " of " + name(removed) + ", now hangs from " + name(p) + ".", {nodes[moved], p});
  }

  /**
   * @brief Shows the parts that the paste is about to join, side by side and in key order.
   *
   * @param p The node the path was split around.
   * @param parts The roots of the parts, in key order. Empty parts are given as the nil node.
   */
  void pasteParts(Node *p, const std::vector<Node *> &parts) {
    std::string names;
    for (Node *part : parts) {
      if (part == Node::nil)
        continue;
      if (isRoot[part->key])
        anchor[part->key] = anchor[p->key];
      if (!names.empty())
        names += ", ";
      names += (left[part->key] == 0 && right[part->key] == 0 && !part->isExternal) ? "the node " + name(part) : treeName(part);
    }
    step("Every part is now a separate tree. In key order: " + names + ". Two joins put them together.", parts);
  }

  void pasteEnd(Node *root) {
    step("Paste done: the two paths are now one auxiliary tree, rooted at " + name(root) + ".", {root});
    context.pop_back();
  }

  /* Red-black tree steps. */

  void rotateLeft(Node *h, Node *x, const char *why) {
    const int hk = h->key, xk = x->key;
    assert(right[hk] == xk);
    const int middle = left[xk];
    right[hk] = middle;
    if (middle != 0)
      parent[middle] = hk;
    putInPlaceOf(hk, xk);
    left[xk] = hk;
    parent[hk] = xk;
    step(std::string(why) + ": rotate left at " + name(h) + ". " + name(x) + " moves up and " + name(h) + " moves down.", {h, x});
  }

  void rotateRight(Node *h, Node *x, const char *why) {
    const int hk = h->key, xk = x->key;
    assert(left[hk] == xk);
    const int middle = right[xk];
    left[hk] = middle;
    if (middle != 0)
      parent[middle] = hk;
    putInPlaceOf(hk, xk);
    right[xk] = hk;
    parent[hk] = xk;
    step(std::string(why) + ": rotate right at " + name(h) + ". " + name(x) + " moves up and " + name(h) + " moves down.", {h, x});
  }

  void flipColors(Node *h, const char *why) { step(std::string(why) + ": flip the colors of " + name(h) + " and its two children.", {h, h->left, h->right}); }

  void blacken(Node *node, const char *why) { step(name(node) + " becomes black, " + why + ".", {node}); }

  void joinBegin(Node *leftTree, Node *x, Node *rightTree) {
    context.push_back("join " + name(x));
    slotParent = 0;
    step("Join " + treeName(leftTree) + ", the node " + name(x) + " and " + treeName(rightTree) + " into one tree.", {leftTree, x, rightTree});
  }

  /**
   * @brief The join goes one level down the taller tree, looking for the place of x.
   *
   * @param tree The current node of the taller tree.
   * @param x The node being joined.
   * @param leftIsTaller True if the taller tree is the left one. The join then goes down its right side.
   */
  void joinDescend(Node *tree, Node *x, bool leftIsTaller) {
    slotParent = tree->key;
    slotOnLeft = !leftIsTaller;
    step(std::string("The ") + (leftIsTaller ? "left" : "right") + " tree is taller at " + name(tree) + ": go down its " + (leftIsTaller ? "right" : "left") +
             " side to find the place of " + name(x) + ".",
         {tree, x});
  }

  /**
   * @brief The node x receives its two children and takes its place: the one found by joinDescend, or the root of the joined tree.
   */
  void joinAttach(Node *x, Node *leftTree, Node *rightTree) {
    const int xk = x->key, l = keyOf(leftTree), r = keyOf(rightTree);
    const int pieceAnchor = anchor[rootOf(xk)];

    // x lets go of a child that will not stay below it. That child takes the place of x.
    if (left[xk] != 0 && left[xk] != l) {
      const int child = left[xk];
      left[xk] = 0;
      putInPlaceOf(xk, child);
    }
    if (right[xk] != 0 && right[xk] != r) {
      const int child = right[xk];
      right[xk] = 0;
      putInPlaceOf(xk, child);
    }

    // x takes its new children.
    if (l != 0 && left[xk] != l) {
      take(l);
      left[xk] = l;
      parent[l] = xk;
    }
    if (r != 0 && right[xk] != r) {
      take(r);
      right[xk] = r;
      parent[r] = xk;
    }

    // x goes to the place found by the descent, or stays as the root of the joined tree.
    if (slotParent != 0) {
      assert(slot(slotParent, slotOnLeft) == 0);
      isRoot[xk] = false;
      slot(slotParent, slotOnLeft) = xk;
      parent[xk] = slotParent;
    } else if (!isRoot[xk]) {
      isRoot[xk] = true;
      anchor[xk] = pieceAnchor;
    }

    if (l == 0 && r == 0)
      step("Put " + name(x) + " here, as a red leaf.", {x});
    else
      step("Put " + name(x) + " here, as a red node, with " + (l == 0 ? "nothing" : name(leftTree)) + " on its left and " +
               (r == 0 ? "nothing" : name(rightTree)) + " on its right.",
           {x, leftTree, rightTree});
  }

  void joinEnd(Node *root, bool rootWasRed) {
    if (rootWasRed)
      step(name(root) + " becomes black, as the root of the joined tree.", {root});
    context.pop_back();
  }

  void splitBegin(Node *h, int key) {
    context.push_back("split " + std::to_string(key));
    step("Split " + treeName(h) + " at " + std::to_string(key) + ": one tree with the smaller keys, the node " + std::to_string(key) +
             " alone, and one tree with the greater keys.",
         {nodes[key]});
  }

  /**
   * @brief The split goes from h to the given child. The link between them is cut, since they end up in different parts.
   */
  void splitDescend(Node *h, Node *child, int key) {
    const bool goLeft = key < h->key;
    cut(h->key, keyOf(child));
    step(std::to_string(key) + (goLeft ? " is smaller than " : " is greater than ") + name(h) + ", so " + name(h) + " and its " + (goLeft ? "right" : "left") +
             " side belong to the " + (goLeft ? "greater" : "smaller") + " keys. Cut the link below " + name(h) + " and go " + (goLeft ? "left." : "right."),
         {h, child});
  }

  void splitFound(Node *h, Node *leftTree, Node *rightTree, bool leftWasRed) {
    cut(h->key, keyOf(leftTree));
    cut(h->key, keyOf(rightTree));
    std::string text = "Reached " + name(h) + ": ";
    text += (leftTree == Node::nil && rightTree == Node::nil) ? "it has nothing below it." : "what is below it is detached.";
    if (leftWasRed)
      text += " " + name(leftTree) + " becomes black, as the root of its own tree.";
    step(text, {h, leftTree, rightTree});
  }

  void splitEnd(Node *leftTree, Node *x, Node *rightTree) {
    step("Split done: " + treeName(leftTree) + ", the node " + name(x) + ", and " + treeName(rightTree) + ".", {leftTree, x, rightTree});
    context.pop_back();
  }

  void deleteBegin(Node *root, bool isMin) {
    context.push_back(isMin ? "delete min" : "delete max");
    step(std::string("Take the ") + (isMin ? "smallest" : "greatest") + " key out of " + treeName(root) + ".", {root});
  }

  void deleteRootRed(Node *root) { step(name(root) + " turns red, so the deletion can go down the tree.", {root}); }

  /**
   * @brief The node h, which holds the smallest key of its tree, is taken out. Its right child takes its place.
   */
  void removeMin(Node *h) {
    const int hk = h->key;
    const int pieceAnchor = anchor[rootOf(hk)];
    const int child = right[hk];
    right[hk] = 0;
    putInPlaceOf(hk, child);
    isRoot[hk] = true;
    anchor[hk] = pieceAnchor;
    step(name(h) + " is the smallest key: take it out." + (child != 0 ? " " + name(nodes[child]) + " takes its place." : ""), {h});
  }

  /**
   * @brief The node h, which holds the greatest key of its tree, is taken out. Its left child takes its place.
   */
  void removeMax(Node *h) {
    const int hk = h->key;
    const int pieceAnchor = anchor[rootOf(hk)];
    const int child = left[hk];
    left[hk] = 0;
    putInPlaceOf(hk, child);
    isRoot[hk] = true;
    anchor[hk] = pieceAnchor;
    step(name(h) + " is the greatest key: take it out." + (child != 0 ? " " + name(nodes[child]) + " takes its place." : ""), {h});
  }

  void deleteEnd(Node *removed, Node *rest) {
    step(name(removed) + " is out, as a black node. " + (rest == Node::nil ? "The tree had no other key." : "What is left is " + treeName(rest) + "."),
         {removed, rest});
    context.pop_back();
  }

private:
  const std::vector<Node *> &nodes;     // The real nodes, indexed by key.
  std::vector<TraceFrame> &frames;      // The recorded frames.
  int n;                                // The number of keys.

  std::vector<int> left;        // left[k] is the key of the left child of k in the forest, or 0.
  std::vector<int> right;       // right[k] is the key of the right child of k in the forest, or 0.
  std::vector<int> parent;      // parent[k] is the key of the parent of k in the forest, or 0.
  std::vector<int> anchor;      // anchor[k] is only used when k is a root: the key of the node its tree is drawn below, or 0.
  std::vector<bool> isRoot;     // isRoot[k] is true if k is the root of a tree of the forest.

  std::vector<std::string> context;     // The operations in progress.
  std::vector<int> path;                // The keys visited by the search so far.
  int slotParent = 0;                   // The node below which the current join places its node, or 0 if it becomes a root.
  bool slotOnLeft = false;              // The side of slotParent where the node is placed.

  int &slot(int p, bool onLeft) { return onLeft ? left[p] : right[p]; }

  int rootOf(int k) const {
    while (parent[k] != 0)
      k = parent[k];
    return k;
  }

  void visit(Node *h) {
    if (path.empty() || path.back() != h->key)
      path.push_back(h->key);
  }

  /**
   * @brief Puts b where a is: below the parent of a, or as the root of its tree. Afterwards a is not in the forest, until the caller places it again.
   */
  void putInPlaceOf(int a, int b) {
    const int p = parent[a];
    if (p != 0) {
      slot(p, left[p] == a) = b;
      parent[a] = 0;
    } else {
      isRoot[a] = false;
    }
    if (b != 0) {
      parent[b] = p;
      isRoot[b] = p == 0;
      if (p == 0)
        anchor[b] = anchor[a];
    }
  }

  /**
   * @brief Removes c from where it is, so the caller can place it somewhere else.
   */
  void take(int c) {
    if (parent[c] != 0) {
      slot(parent[c], left[parent[c]] == c) = 0;
      parent[c] = 0;
    } else {
      isRoot[c] = false;
    }
  }

  /**
   * @brief Cuts the link between h and its child c. The subtree of c becomes a separate tree, drawn next to the tree of h.
   */
  void cut(int h, int c) {
    if (c == 0)
      return;
    assert(parent[c] == h);
    detach(c, anchor[rootOf(h)]);
  }

  /**
   * @brief Detaches the subtree of c from its parent. It becomes a separate tree, drawn below the given anchor.
   */
  void detach(int c, int newAnchor) {
    assert(parent[c] != 0);
    take(c);
    isRoot[c] = true;
    anchor[c] = newAnchor;
  }

  /**
   * @brief Attaches the separate tree with root c below p, on the given side.
   */
  void attach(int p, bool onLeft, int c) {
    assert(isRoot[c] && slot(p, onLeft) == 0);
    isRoot[c] = false;
    parent[c] = p;
    slot(p, onLeft) = c;
  }

  /**
   * @brief Collects, in-order, the nodes of an auxiliary tree with reference tree depth at least the given depth.
   */
  void collectDeep(Node *h, int depth, std::vector<Node *> &result) const {
    if (h->isExternal)
      return;
    collectDeep(h->left, depth, result);
    if (h->depth >= depth)
      result.push_back(h);
    collectDeep(h->right, depth, result);
  }

  static std::string capitalized(std::string text) {
    if (!text.empty() && text[0] >= 'a' && text[0] <= 'z')
      text[0] = static_cast<char>(text[0] - 'a' + 'A');
    return text;
  }
};

/* The red-black tree operations, copied from RedBlackTree.cpp with the tracer calls added. */

/**
 * @brief Rotates a subtree to the left. See rotateLeft in RedBlackTree.cpp.
 *
 * @param trace The tracer that records the step.
 * @param h The subtree root node.
 * @param why The reason for the rotation, shown in the recorded step.
 * @return The new subtree root after the rotation.
 */
Node *rotateLeft(Tracer &trace, Node *h, const char *why) {
  Node *x = h->right;
  h->right = x->left;
  x->left = h;
  x->color = h->color;
  h->color = RED;
  update(h);     // Update the properties of h after rotation.
  update(x);     // Update the properties of x after rotation.
  trace.rotateLeft(h, x, why);
  return x;
}

/**
 * @brief Rotates a subtree to the right. See rotateRight in RedBlackTree.cpp.
 *
 * @param trace The tracer that records the step.
 * @param h The subtree root node.
 * @param why The reason for the rotation, shown in the recorded step.
 * @return The new subtree root after the rotation.
 */
Node *rotateRight(Tracer &trace, Node *h, const char *why) {
  Node *x = h->left;
  h->left = x->right;
  x->right = h;
  x->color = h->color;
  h->color = RED;
  update(h);     // Update the properties of h after rotation.
  update(x);     // Update the properties of x after rotation.
  trace.rotateRight(h, x, why);
  return x;
}

/**
 * @brief Flips the colors of the given node and its children. See flipColors in RedBlackTree.cpp.
 *
 * @param trace The tracer that records the step.
 * @param h The node.
 * @param why The reason for the flip, shown in the recorded step.
 * @return The node after flipping the colors.
 */
Node *flipColors(Tracer &trace, Node *h, const char *why) {
  Color c = h->color;
  h->color = h->left->color;
  h->left->color = h->right->color = c;
  trace.flipColors(h, why);
  return h;
}

/**
 * @brief Balances the given subtree. See balance in RedBlackTree.cpp.
 *
 * @param trace The tracer that records the steps.
 * @param h The subtree root node.
 * @return The new subtree root after the balance.
 */
Node *balance(Tracer &trace, Node *h) {
  if (h->right->color == RED)
    h = rotateLeft(trace, h, "A red link leans to the right");     // Fix right-leaning red link.
  if (h->left->color == RED && h->left->left->color == RED)
    h = rotateRight(trace, h, "Two red links in a row");     // Fix two reds in a row on the left.
  if (h->left->color == RED && h->right->color == RED)
    h = flipColors(trace, h, "Both children are red");     // Split 4-node.

  update(h);
  return h;
}

/**
 * @brief Moves a red link to the left. See moveRedLeft in RedBlackTree.cpp.
 *
 * @param trace The tracer that records the steps.
 * @param h The subtree root node.
 * @return The new subtree root after moving the red link left.
 */
Node *moveRedLeft(Tracer &trace, Node *h) {
  h = flipColors(trace, h, "To carry a red link down to the left");
  if (h->right->left->color == RED) {
    h->right = rotateRight(trace, h->right, "To bring a red link to the left side");
    h = rotateLeft(trace, h, "To bring a red link to the left side");
    h = flipColors(trace, h, "To restore the colors");
  }
  return h;
}

/**
 * @brief Moves a red link to the right. See moveRedRight in RedBlackTree.cpp.
 *
 * @param trace The tracer that records the steps.
 * @param h The subtree root node.
 * @return The new subtree root after moving the red link right.
 */
Node *moveRedRight(Tracer &trace, Node *h) {
  h = flipColors(trace, h, "To carry a red link down to the right");
  if (h->left->left->color == RED) {
    h = rotateRight(trace, h, "To bring a red link to the right side");
    h = flipColors(trace, h, "To restore the colors");
  }
  return h;
}

/**
 * @brief Sets the children of the given node to the nil node, sets its color to black, and recomputes its fields. See cleanUp in RedBlackTree.cpp.
 *
 * @param h The node.
 * @return The node after the clean up.
 */
Node *cleanUp(Node *h) {
  if (!h->isExternal) {
    h->left = Node::nil;
    h->right = Node::nil;
    h->color = BLACK;
    update(h);
  }
  return h;
}

/* Search. */

/**
 * @brief Recursively searches for a key in the given subtree. See searchRec in RedBlackTree.cpp.
 *
 * @param trace The tracer that records one step for each visited node.
 * @param h The subtree root node.
 * @param key The search key.
 * @return A pointer to the node if found, or an external node otherwise.
 */
Node *search(Tracer &trace, Node *h, int key) {
  if (h->isExternal || h->key == key) {
    trace.searchEnd(h, key);
    return h;
  }
  trace.searchStep(h, key);
  if (key < h->key)
    return search(trace, h->left, key);
  else
    return search(trace, h->right, key);
}

/* Join. */

/**
 * @brief Recursively joins two red-black trees and a node. See joinRec in RedBlackTree.cpp.
 *
 * @param trace The tracer that records the steps.
 * @param leftTree The left tree root node.
 * @param x The middle node.
 * @param rightTree The right tree root node.
 * @return The root of the new tree formed by joining the left tree, the node, and the right tree.
 */
Node *joinRec(Tracer &trace, Node *leftTree, Node *x, Node *rightTree) {
  if (leftTree->blackHeight > rightTree->blackHeight) {
    trace.joinDescend(leftTree, x, true);
    leftTree->right = joinRec(trace, leftTree->right, x, rightTree);
    return balance(trace, leftTree);
  }
  if (leftTree->blackHeight < rightTree->blackHeight) {
    trace.joinDescend(rightTree, x, false);
    rightTree->left = joinRec(trace, leftTree, x, rightTree->left);
    return balance(trace, rightTree);
  }
  x->left = leftTree;
  x->right = rightTree;
  x->color = RED;
  trace.joinAttach(x, leftTree, rightTree);
  return balance(trace, x);
}

/**
 * @brief Joins two red-black trees and a node into a single red-black tree. See join in RedBlackTree.h.
 *
 * @param trace The tracer that records the steps.
 * @param leftTree The left tree root node.
 * @param x The middle node.
 * @param rightTree The right tree root node.
 * @return The root of the new tree formed by joining the left tree, the node, and the right tree.
 */
Node *join(Tracer &trace, Node *leftTree, Node *x, Node *rightTree) {
  trace.joinBegin(leftTree, x, rightTree);
  Node *y = joinRec(trace, leftTree, x, rightTree);
  const bool rootWasRed = y->color == RED;
  y->color = BLACK;     // Ensure the new root of the joined tree is black.
  trace.joinEnd(y, rootWasRed);

  return y;
}

/* Split. */

/**
 * @brief Recursively splits the given subtree around a key. See splitRec in RedBlackTree.cpp.
 *
 * @param trace The tracer that records the steps.
 * @param h The subtree root node.
 * @param key The splitter key.
 * @return A tuple containing the left tree (keys smaller than the given key), the node with the given key, and the right tree (keys greater than the given
 * key).
 */
std::tuple<Node *, Node *, Node *> splitRec(Tracer &trace, Node *h, int key) {
  if (key < h->key) {
    trace.splitDescend(h, h->left, key);
    auto [left, x, right] = splitRec(trace, h->left, key);
    return {left, x, join(trace, right, h, h->right)};
  }
  if (key > h->key) {
    trace.splitDescend(h, h->right, key);
    auto [left, x, right] = splitRec(trace, h->right, key);
    const bool leftWasRed = h->left->color == RED;
    h->left->color = BLACK;     // Ensure the left child of h is black before joining.
    if (leftWasRed)
      trace.blacken(h->left, "so it can be the root of a tree in the next join");
    return {join(trace, h->left, h, left), x, right};
  }

  Node *left = h->left;
  Node *right = h->right;
  const bool leftWasRed = left->color == RED;
  left->color = BLACK;     // Ensure that the left subtree root is black.
  trace.splitFound(h, left, right, leftWasRed);
  return {left, h, right};
}

/**
 * @brief Splits the given red-black tree into two trees and a node. See split in RedBlackTree.h.
 *
 * @param trace The tracer that records the steps.
 * @param h The tree root node.
 * @param key The splitter key.
 * @return A tuple containing the left tree, the node with the given key, and the right tree.
 */
std::tuple<Node *, Node *, Node *> split(Tracer &trace, Node *h, int key) {
  trace.splitBegin(h, key);
  auto [l, x, r] = splitRec(trace, h, key);
  cleanUp(x);
  trace.splitEnd(l, x, r);
  return {l, x, r};
}

/* DeleteMin. */

/**
 * @brief Recursively deletes the node with the minimum key in the given subtree. See deleteMinRec in RedBlackTree.cpp.
 *
 * @param trace The tracer that records the steps.
 * @param h The subtree root node.
 * @return The minimum node and the new subtree root after deleting the minimum node.
 */
std::pair<Node *, Node *> deleteMinRec(Tracer &trace, Node *h) {
  if (h->left->isExternal) {
    trace.removeMin(h);
    return {h, h->right};     // If the left child of h is external, then h is the minimum node, so we return its right child as the new subtree root.
  }
  if (h->left->color == BLACK && h->left->left->color == BLACK)
    h = moveRedLeft(trace, h);
  auto [min, x] = deleteMinRec(trace, h->left);
  h->left = x;
  return {min, balance(trace, h)};
}

/**
 * @brief Deletes and returns the node with the minimum key in the red-black tree. See deleteMin in RedBlackTree.h.
 *
 * @param trace The tracer that records the steps.
 * @param root The tree root node.
 * @return The minimum node and the new tree root after deleting the minimum node.
 */
std::pair<Node *, Node *> deleteMin(Tracer &trace, Node *root) {
  trace.deleteBegin(root, true);
  if (root->left->color == BLACK) {
    root->color = RED;     // Ensure that the root or its left child is red before deleting the minimum.
    trace.deleteRootRed(root);
  }
  auto [min, h] = deleteMinRec(trace, root);
  h->color = BLACK;
  min->color = BLACK;
  trace.deleteEnd(min, h);
  return {min, h};
}

/* DeleteMax. */

/**
 * @brief Recursively deletes the node with the maximum key in the given subtree. See deleteMaxRec in RedBlackTree.cpp.
 *
 * @param trace The tracer that records the steps.
 * @param h The subtree root node.
 * @return The maximum node and the new subtree root after deleting the maximum node.
 */
std::pair<Node *, Node *> deleteMaxRec(Tracer &trace, Node *h) {
  if (!h->left->isExternal && h->left->color == RED)
    h = rotateRight(trace, h, "To lean the red link to the right");
  if (h->right->isExternal) {
    trace.removeMax(h);
    return {h, h->left};     // If the right child of h is external, then h is the maximum node, so we return its left child as the new subtree root.
  }
  if (h->right->color == BLACK && h->right->left->color == BLACK)
    h = moveRedRight(trace, h);
  auto [max, x] = deleteMaxRec(trace, h->right);
  h->right = x;
  return {max, balance(trace, h)};
}

/**
 * @brief Deletes and returns the node with the maximum key in the red-black tree. See deleteMax in RedBlackTree.h.
 *
 * @param trace The tracer that records the steps.
 * @param root The tree root node.
 * @return The maximum node and the new tree root after deleting the maximum node.
 */
std::pair<Node *, Node *> deleteMax(Tracer &trace, Node *root) {
  trace.deleteBegin(root, false);
  if (root->left->color == BLACK) {
    root->color = RED;
    trace.deleteRootRed(root);
  }
  auto [max, h] = deleteMaxRec(trace, root);
  h->color = BLACK;
  max->color = BLACK;
  trace.deleteEnd(max, h);
  return {max, h};
}

/* The tango tree operations, copied from TangoTree.cpp with the tracer calls added. */

/**
 * @brief Recursively builds a tango tree with the keys in the range [l, r], following the shape of a complete reference tree. See buildTango in TangoTree.cpp.
 *
 * @param l The left bound of the range of keys.
 * @param r The right bound of the range of keys.
 * @param depth The depth, in the reference tree, of the root of this range.
 * @return The root node of the built tree.
 */
Node *buildTango(int l, int r, int depth = 0) {
  if (l > r)
    return Node::nil;

  int n = r - l + 1;

  // Calculate the greatest power of 2 less than or equal to n: p = 2^(floor(log2(n))).
  int p = 1;
  while (p * 2 <= n)
    p <<= 1;
  int s = n - (p - 1);
  int m;

  if (s > p / 2) {
    m = l + p - 1;
  } else {
    m = l + (p / 2) - 1 + s;
  }

  Node *left = buildTango(l, m - 1, depth + 1);
  Node *right = buildTango(m + 1, r, depth + 1);
  Node *middle = newNode(m, depth);
  middle->left = left;
  middle->right = right;
  middle->color = BLACK;
  middle->depth = depth;
  middle->minDepth = depth;
  middle->maxDepth = depth;
  middle->isExternal = true;
  middle->blackHeight = -1;
  return middle;
}

/**
 * @brief Finds the predecessor, in the given auxiliary subtree, of the smallest key with reference tree depth at least the given depth. See predecessor in
 * TangoTree.cpp.
 *
 * @param h The subtree root node.
 * @param depth The minimum reference tree depth.
 * @return The predecessor key, or -1 if there is no smaller key in the subtree.
 */
int predecessor(Node *h, int depth) {
  if (!h->left->isExternal && h->left->maxDepth >= depth)
    return predecessor(h->left, depth);
  if (h->depth >= depth)
    return h->left->isExternal ? -1 : max(h->left)->key;
  int p = predecessor(h->right, depth);
  return p == -1 ? h->key : p;
}

/**
 * @brief Finds the successor, in the given auxiliary subtree, of the largest key with reference tree depth at least the given depth. See successor in
 * TangoTree.cpp.
 *
 * @param h The subtree root node.
 * @param depth The minimum reference tree depth.
 * @return The successor key, or -1 if there is no greater key in the subtree.
 */
int successor(Node *h, int depth) {
  if (!h->right->isExternal && h->right->maxDepth >= depth)
    return successor(h->right, depth);
  if (h->depth >= depth)
    return h->right->isExternal ? -1 : min(h->right)->key;
  int s = successor(h->left, depth);
  return s == -1 ? h->key : s;
}

/**
 * @brief Cuts the nodes with reference tree depth at least the given depth from the root preferred path. See cut in TangoTree.cpp.
 *
 * @param trace The tracer that records the steps.
 * @param root The tango tree root node.
 * @param depth The minimum reference tree depth of the nodes to remove.
 * @return The new tango tree root after removing the nodes.
 */
Node *cut(Tracer &trace, Node *root, int depth) {
  int pred = predecessor(root, depth);
  int succ = successor(root, depth);
  trace.cutBegin(root, depth, pred, succ);

  // Split the root into tl (< pred), xl (= pred), tm (> pred and < succ), xr (= succ), and tr (> succ). Some of them may be the nil node, if no key in the tree
  // satisfies the condition.

  Node *tl = Node::nil;
  Node *xl = Node::nil;
  Node *taux = root;

  if (pred > -1)
    std::tie(tl, xl, taux) = split(trace, root, pred);

  Node *tm = taux;
  Node *xr = Node::nil;
  Node *tr = Node::nil;

  if (succ > -1)
    std::tie(tm, xr, tr) = split(trace, tm, succ);

  // Mark tm as external and join the trees, ensuring that tm is not in the root preferred path anymore.

  tm->isExternal = true;
  tm->blackHeight = -1;
  trace.cutMiddle(tm);

  Node *tt = tm;

  if (xl != Node::nil)
    tt = join(trace, tl, xl, tm);

  Node *t = tt;

  if (xr != Node::nil)
    t = join(trace, tt, xr, tr);

  trace.cutEnd(tm);
  return t;
}

/**
 * @brief Pastes the given auxiliary tree into the root preferred path. See paste in TangoTree.cpp.
 *
 * @param trace The tracer that records the steps.
 * @param root The tango tree root node.
 * @param q The root node of the auxiliary tree to insert.
 * @param p The parent of q, which is the last node seen in the root preferred path before reaching q.
 * @return The new tango tree root after inserting the auxiliary tree.
 */
Node *paste(Tracer &trace, Node *root, Node *q, Node *p) {
  q->isExternal = false;
  update(q);     // Update the fields of q, since it is not an external node anymore.
  trace.pasteBegin(q, p);
  if (p->left == q) {
    auto [min, qq] = deleteMin(trace, q);
    p->left = min->left;
    trace.pasteMove(min, qq, p, true);
    auto [tl, pp, tr] = split(trace, root, p->key);
    trace.pasteParts(pp, {tl, min, qq, pp, tr});
    Node *taux = join(trace, qq, pp, tr);
    root = join(trace, tl, min, taux);
  } else {
    auto [max, qq] = deleteMax(trace, q);
    p->right = max->right;
    trace.pasteMove(max, qq, p, false);
    auto [tl, pp, tr] = split(trace, root, p->key);
    trace.pasteParts(pp, {tl, pp, qq, max, tr});
    Node *taux = join(trace, tl, pp, qq);
    root = join(trace, taux, max, tr);
  }
  trace.pasteEnd(root);
  return root;
}

/**
 * @brief Performs the tango operation, the core of the tango tree. See tango in TangoTree.cpp.
 *
 * @param trace The tracer that records the steps.
 * @param root The tango tree root node.
 * @param q The root node of the auxiliary tree to insert into the root preferred path.
 * @return The new tango tree root after the tango operation.
 */
Node *tango(Tracer &trace, Node *root, Node *q) {
  trace.tangoBegin(root, q);
  if (root->maxDepth >= q->minDepth) {
    root = cut(trace, root, q->minDepth);     // Remove all the keys that are not in the preferred path anymore.
  }
  auto [pp, qq] = parentSearch(root, q->key);     // Find the last node seen in the root preferred path before reaching q.
  root = paste(trace, root, qq, pp);         // Insert the new keys into the root preferred path.
  trace.tangoEnd();
  return root;
}

/**
 * @brief Treats h as an internal auxiliary tree root and, if the key lies in an auxiliary tree below it, first restructures that tree recursively and then
 * tangos it into h. See searchRecTango in TangoTree.cpp.
 *
 * @param trace The tracer that records the steps.
 * @param h The subtree root node.
 * @param key The key to search for in the subtree.
 * @return The new subtree root after the search and possible restructuring.
 */
Node *searchRecTango(Tracer &trace, Node *h, int key) {
  if (h == Node::nil)
    return h;

  h->isExternal = false;
  update(h);     // h may have been an external root (blackHeight == -1), so recompute its fields.
  trace.searchEnterTree(h, key);

  Node *q = search(trace, h, key);
  if (q == Node::nil || !q->isExternal)     // Key found in h's auxiliary tree, or not in the tree at all.
    return h;

  auto [p, qq] = parentSearch(h, key);
  bool isLeft = p->left == q;

  trace.searchGoDown(p, q);
  q = searchRecTango(trace, q, key);

  q->isExternal = true;
  q->blackHeight = -1;

  if (isLeft)
    p->left = q;
  else
    p->right = q;
  trace.searchComeBack(p, isLeft, q);

  return tango(trace, h, q);
}

/* JSON. */

/**
 * @brief Appends the given text to the output as a JSON string, with its quotes.
 */
void appendJsonString(std::string &out, const std::string &text) {
  out += '"';
  for (char c : text) {
    if (c == '"' || c == '\\') {
      out += '\\';
      out += c;
    } else if (c == '\n') {
      out += "\\n";
    } else if (static_cast<unsigned char>(c) >= 0x20) {
      out += c;
    }
  }
  out += '"';
}

/**
 * @brief Appends the given numbers to the output as a JSON array, starting at the given position.
 */
void appendJsonArray(std::string &out, const std::vector<int> &values, size_t first = 0) {
  out += '[';
  for (size_t i = first; i < values.size(); ++i) {
    if (i > first)
      out += ',';
    out += std::to_string(values[i]);
  }
  out += ']';
}

/**
 * @brief Stores every node of the given subtree in the vector, at the position of its key.
 */
void collectNodes(Node *h, std::vector<Node *> &nodes) {
  if (h == Node::nil)
    return;
  nodes[h->key] = h;
  collectNodes(h->left, nodes);
  collectNodes(h->right, nodes);
}

}     // namespace

/* Tango visualization operations. */

/* Constructor. */
TangoVisualization::TangoVisualization(int n) {
  root = buildTango(1, n);
  root->isExternal = false;
  root->blackHeight = 0;
  size = n;
  lastKey = 0;
  lastFound = false;

  nodes.assign(n + 1, nullptr);
  collectNodes(root, nodes);

  Tracer trace(nodes, root, frames);
  trace.step("A new tango tree with the keys 1 to " + std::to_string(n) + ". Every key starts as its own auxiliary tree.");
}

/* Destructor. */
TangoVisualization::~TangoVisualization() {
  for (Node *node : nodes)
    delete node;
}

/* Get Size. */
int TangoVisualization::getSize() { return size; }

/* Search. */
Node *TangoVisualization::search(int key) {
  frames.clear();
  Tracer trace(nodes, root, frames);

  trace.searchBegin(key);
  root = searchRecTango(trace, root, key);
  Node *result = ::search(root, key);
  trace.searchFinish(key, result);

  lastKey = key;
  lastFound = result != Node::nil;
  return result;
}

/* Contains. */
bool TangoVisualization::contains(int key) { return search(key) != Node::nil; }

/* Get Frames. */
const std::vector<TraceFrame> &TangoVisualization::getFrames() const { return frames; }

/* Get Depths. */
std::vector<int> TangoVisualization::getDepths() const {
  std::vector<int> depths(size + 1, 0);
  for (int k = 1; k <= size; ++k)
    depths[k] = nodes[k]->depth;
  return depths;
}

/* To JSON. */
std::string TangoVisualization::toJson() const {
  std::string out = "{\"n\":" + std::to_string(size) + ",\"key\":" + std::to_string(lastKey) + ",\"found\":" + (lastFound ? "true" : "false") + ",\"depth\":";
  appendJsonArray(out, getDepths(), 1);
  out += ",\"frames\":[";
  for (size_t i = 0; i < frames.size(); ++i) {
    const TraceFrame &frame = frames[i];
    std::vector<int> left, right;
    std::string colors, externals;
    for (int k = 1; k <= size; ++k) {
      left.push_back(frame.nodes[k].left);
      right.push_back(frame.nodes[k].right);
      colors += frame.nodes[k].red ? 'r' : 'b';
      externals += frame.nodes[k].external ? '1' : '0';
    }

    out += i == 0 ? "{\"ctx\":[" : ",{\"ctx\":[";
    for (size_t j = 0; j < frame.context.size(); ++j) {
      if (j > 0)
        out += ',';
      appendJsonString(out, frame.context[j]);
    }
    out += "],\"text\":";
    appendJsonString(out, frame.text);
    out += ",\"l\":";
    appendJsonArray(out, left);
    out += ",\"r\":";
    appendJsonArray(out, right);
    out += ",\"c\":\"" + colors + "\",\"e\":\"" + externals + "\",\"roots\":";
    appendJsonArray(out, frame.roots);
    out += ",\"anchors\":";
    appendJsonArray(out, frame.anchors);
    out += ",\"focus\":";
    appendJsonArray(out, frame.focus);
    out += ",\"path\":";
    appendJsonArray(out, frame.path);
    out += '}';
  }
  out += "]}";
  return out;
}
