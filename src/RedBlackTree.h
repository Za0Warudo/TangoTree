/**
 * @file RedBlackTree.h
 * @brief Defines the Red-Black Tree key structure
 *
 * This file provides the declaration of the Red-Black Tree key structure,
 * including the basic core functions and Split and Join operations used in Tango Tree key structure.
 *
 * @date November 2025
 * @version 1.0.1
 * @author Za0Warudo
 * @note This implementation is design to feed the Tango Tree key structure needs
 */

#ifndef REDBLACKTREE_H
#define REDBLACKTREE_H

/* Includes */
#include <algorithm>
#include <climits>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <tuple>

/*---------------------------------------------*/

/* Auxiliary encodes */

/**
 * @enum Color
 * @brief Encode link colors
 */
enum Color {
    RED,        // Red link
    BLACK       // Black link
};

/**
 * @enum Type
 * @brief Encode node type
 */
enum Type {
    REGULAR,        // In tree
    EXTERNAL,       // Out tree
    DUMMY           // Out absolute (null like)
};

/*---------------------------------------------*/

/* Node definition */

/**
 * @brief Node struct
 * @tparam T key type
 */
template <typename T>
struct Node {
    T key;                              // Key
    Node *left, *right;                 // Left & Right children
    int size;                           // Tree size
    int height;                         // Black height
    int depth;                          // Depth (used only in tango tree)
    int maxDepth;                       // Tree max depth
    int minDepth;                       // Tree min depth
    Color color;                        // Parent link color
    Type type;                          // Node type

    /**
     * @brief Builds a new node with k key
     * @param k node key
     * @param c node color, default is RED
     * @param t node type, default is REGULAR
     */
    explicit Node(T k, const Color c = RED,  const Type t = REGULAR):
    key(k),
    left(nullptr),
    right(nullptr),
    size(1),
    height(0),
    depth(INT_MAX),
    maxDepth(-INT_MAX),
    minDepth(INT_MAX),
    color(c),
    type(t) {}
};

/*---------------------------------------------*/

/* Dummy node */

/**
 * @brief The dummy node
 */
template <typename T>
Node<T>* dummy = new Node<T>(T(), BLACK, DUMMY);

/**
 * @brief Get the dummy node (wrapper)
 */
template <typename T>
Node<T>* GetDummy() { return dummy<T>; }

/*---------------------------------------------*/

/* Constructor */

/**
 * @brief Initializes a new empty left leaning Red-Black Tree
 * @return the root node of new tree (dummy)
 */
template <typename T>
Node<T>* Initialize() { return GetDummy<T>(); }

/**
 * @brief Initializes a new node (fill the left and right pointers with the dummy node)
 * @param key node key
 * @return the new node
 */
template <typename T>
Node<T>* BuildNode(T key) {
    auto* node = new Node<T>(key);

    // set children to dummy
    node->left = GetDummy<T>();
    node->right = GetDummy<T>();

    return node;
}

/*---------------------------------------------*/

/* Helpers functions */

/**
 * @brief Checks if @c x node is the dummy node
 * @param x the node
 * @return @c true or @c false
 */
template <typename T>
bool IsDummy(const Node<T>* x) { return x->type == DUMMY; }

/**
 * @brief Checks if @c x node is an external node
 * @param x the node
 * @return @c true or @c false
 */
template <typename T>
bool IsExternal(const Node<T>* x) { return x->type == EXTERNAL; }

/**
 * @brief Checks if the @c x node is an external or the dummy node
 * @param x the node
 * @return @c true or @c false
 */
template <typename T>
bool IsExternalOrDummy(const Node<T>* x) {return IsExternal<T>(x) || IsDummy(x); }

/**
 * @brief Checks if the @c x node is an empty tree
 * @param t the tree root node
 * @return @c true or @c false
 */
template <typename T>
bool IsEmpty(const Node<T>* t) { return IsExternalOrDummy(t); }

/**
 * @brief Get @c x node size
 *
 * Empty nodes has size equal to 0
 *
 * @param x the node
 * @return tree size
 */
template <typename T>
int Size(const Node<T>* x) { return !IsEmpty(x)? x->size: 0; }

/**
 * @brief Get @c x node black height
 *
 * Empty nodes has height equal to -1
 *
 * @param x the node
 * @return tree height
 */
template <typename T>
int Height(const Node<T>* x) { return !IsEmpty(x)? x->height: -1; }

/**
 * @brief Get @c x subtree min depth
 *
 * Empty subtrees have min depth equal to inf (@c INT_MAX)
 *
 * @param x the node
 * @return tree min depth
 */
template <typename T>
int MinDepth(const Node<T>* x) { return !IsEmpty(x)? x->minDepth: INT_MAX; }

/**
 * @brief Get @c x subtree max depth
 *
 * Empty subtrees have max depth equal to -inf (@c -INT_MAX)
 *
 * @param x the node
 * @return tree max depth
 */
template <typename T>
int MaxDepth(const Node<T>* x) { return !IsEmpty(x)? x->maxDepth: -INT_MAX; }

/**
 * @brief Get @c x node depth
 *
 * Empty subtrees have depth equal to -inf (@c -INT_MAX)
 *
 * @param x the node
 * @return node depth
 */
template <typename T>
int Depth(const Node<T>* x) { return !IsEmpty(x)? x->depth: -INT_MAX; }

/**
 * @brief Checks if the @c x node parent link is a red link
 *
 * An empty subtree parent link is defined black
 *
 * @param x the node
 * @return @c true or @c false
 */
template <typename T>
bool IsRedLink(const Node<T>* x) { return !IsEmpty(x)? x->color == RED : false; }

/**
 * @brief Updates @c x node size
 * @param x the node
 */
template <typename T>
void UpdateSize(Node<T>* x) { if (!IsEmpty(x)) x->size = Size(x->left) + Size(x->right) + 1; }

/**
 * @brief Updates @c x node black height
 * @param x the node
 */
template <typename T>
void UpdateHeight(Node<T>* x) {
    if (!IsEmpty(x)) {
        const int leftHeight = Height(x->left) + IsRedLink(x->left)? 0 : 1;
        const int rightHeight = !IsEmpty(x->right)? Height(x->right) + 1 : 0;
        x->height = std::max(leftHeight, rightHeight);
    }
}

/**
 * @brief Updates node min and max depth
 * @param x the node
 */
template <typename T>
void UpdateDepth(Node<T>* x) {
    if (!IsEmpty(x)) {
        x->minDepth = std::min(x->depth, std::min(MinDepth(x->left), MinDepth(x->right)));
        x->maxDepth = std::max(x->depth, std::max(MaxDepth(x->left), MaxDepth(x->right)));
    }
}

/**
 * @brief Updates size, height and depth of @c x node
 */
template <typename T>
void Update(Node<T>* x) {
    if (!IsEmpty(x)) {
        UpdateSize(x);
        UpdateHeight(x);
        UpdateDepth(x);
    }
}

/**
 * @brief Removes the left and right children of the given node and return it
 * @param x the node
 * @return a pair containing the left and the right children
 */
template <typename T>
std::pair<Node<T>*, Node<T>*> Detach(Node<T>* x) {
    assert(!IsDummy(x));
    Node<T>* l = x->left;   // save left pointer
    Node<T>* r = x->right;  // save right pointer
    x->left = GetDummy<T>();
    x->right = GetDummy<T>();
    x->color = BLACK;
    Update(x);
    return std::make_pair(l, r);
}

/*---------------------------------------------*/

/* Red-Black properties functions */

/**
 * @brief Rotates the @c h tree to the right
 * @param h the tree node
 * @return the new subtree after the rotation
 */
template <typename T>
Node<T>* RotateRight(Node<T>* h) {
    assert(IsRedLink(h->left)); // only rotates red links

    // rotation
    Node<T>* y = h->left;
    h->left = y->right;
    y->right = h;
    y->color = h->color;
    h->color = RED;

    // update info
    Update(h);
    Update(y);

    return y;
}

/**
 * @brief Rotates the @c h tree to the left
 * @param h the tree node
 * @return the new subtree after the rotation
 */
template <typename T>
Node<T>* RotateLeft(Node<T>* h) {
    assert(IsRedLink(h->right)); // only rotates red links

    // rotation
    Node<T>* y = h->right;
    h->right = y->left;
    y->left = h;
    y->color = h->color;
    h->color = RED;

    // update info
    Update(h);
    Update(y);

    return y;
}

/**
 * @brief Make @c h.left or one of its children a red link
 * @param h the tree node
 * @return the subtree after the operation
 */
template <typename T>
Node<T>* MoveRedLeft (Node<T>* h) {
    assert(IsRedLink(h) && !IsRedLink(h->left) && !IsRedLink(h->left->left)); // only move red link if not already in there

    FlipColors(h); // throw h red link to it's children

    if (IsRedLink(h->right->left)) {
        // got a 4-node (need fix)
        h->right = RotateRight(h->right);
        h = RotateLeft(h);
        FlipColors(h);
    }
    return h;
}

/**
 * @brief Make @c h.right or one of its children a red link
 * @param h the tree node
 * @return the subtree after the operation
 */
template <typename T>
Node<T>* MoveRedRight (Node<T>* h) {
    assert(IsRedLink(h) && !IsRedLink(h->right) && !IsRedLink(h->right->left)); // only mode red link if not already in there

    FlipColors(h); // throw h red link to it's children

    if (IsRedLink(h->left->left)) {
        // got a 4-node (need fix)
        h = RotateRight(h);
        FlipColors(h);
    }
    return h;
}

/**
 * @brief Flip @c h node and it's children colors
 * @param h the tree node
 */
template <typename T>
void FlipColors(Node<T>* h) {

    assert(!IsEmpty(h) && !IsEmpty(h->left) && !IsEmpty(h->right)); // h and it's children must be no empty
    assert(h->color != h->left->color && h->color != h->right->color); // children color must be opposite to h color

    h->color = (h->color == RED) ? BLACK : RED;
    h->left->color = h->left->color == RED ? BLACK : RED;
    h->right->color = h->right->color == RED ? BLACK : RED;
}


/**
 * @brief Fix the Red-Black property in node @c x
 * @param x the tree node
 * @return the balanced subtree
 */
template<typename T>
Node<T>* Balance(Node<T>* x) {
    if (!IsEmpty(x)) {

        if (!IsRedLink(x->left) && IsRedLink(x->right)) x = RotateLeft(x);
        if (IsRedLink(x->left) && IsRedLink((x->left)->left)) x = RotateRight(x);
        if (IsRedLink(x->left) && IsRedLink(x->right)) FlipColors(x);

        Update(x);
    }
    return x;
}

/*---------------------------------------------*/

/* Insert functions */

/**
 * @brief Inserts the key in the tree @c t
 * @param h the tree root node
 * @param key the new key to be inserted in the tree
 * @return the new root tree
 */
template <typename T>
Node<T>* Insert(Node<T>* h, T key) {
    h = InsertRec(h, key);
    h->color = BLACK;

    assert(Check(h));

    return h;
}

/**
 * @brief Recursive auxiliar insert method
 * @param h the tree node
 * @param key the new key to be inserted
 * @return the root subtree
 */
template <typename T>
Node<T>* InsertRec(Node<T>* h, T key) {
    if (IsEmpty(h)) return BuildNode(key);

    if (key < h->key) h->left = InsertRec(h->left, key);
    else if (key > h->key) h->right = InsertRec(h->right, key);
    else h->key = key; // unique key

    return Balance(h);
}

/*---------------------------------------------*/

/* Query functions */

/**
 * @brief Checks if the tree @h contains the @c key key
 * @param h the tree
 * @param key the key to be search
 * @return @c true or @c false.
 */
template <typename T>
bool Contains(Node<T>* h, T key) { return !IsEmpty(Search(h, key).first); }

/**
 * @brief Finds the node containing the given key, and it's parent, return the dummy node otherwise
 * @param h the tree
 * @param key the new key to be search
 * @param p the current level parent
 * @return the node and it's parent
 */
template <typename T>
std::pair<Node<T>*, Node<T>*> Search(Node<T>* h, T key, Node<T>* p = GetDummy<T>()) {
    if (!IsEmpty(h)) {
        if (key < h->key) return Search(h->left, key, h);
        if (key > h->key) return Search(h->right, key, h);
    }
    return {h, p};
}

/**
 * @brief Returns the minimum element in the tree
 * @param t tree node
 * @return the tree max element
 * @throw std::runtime_error if the tree is empty
 */
template <typename T>
Node<T>* Min(Node<T>* t) {
    if (IsEmpty(t)) throw std::runtime_error("Min in an empty");
    while (!IsEmpty(t->left)) t = t->left;

    return t;
}

/**
 * @brief Returns the maximum element in the tree
 * @param t tree node root node
 * @return the tree max element
 * @throw std::runtime_error if the tree is empty
 */
template <typename T>
Node<T>* Max(Node<T>* t) {
    if (IsEmpty(t)) throw std::runtime_error("Min in an empty");
    while (!IsEmpty(t->right)) t = t->right;

    return t;
}

/*---------------------------------------------*/

/* Remove functions */

/**
 * @brief Removes min element of the tree @c t
 * @param t the tree node
 * @return the tree node after the remove operation
 * @throw std::runtime_error if the tree is empty
 */
template <typename T>
Node<T>* RemoveMin(Node<T>* t) {
    if (IsEmpty(t)) throw std::runtime_error("Remove from an empty");

    if (!IsRedLink(t->left) && !IsRedLink(t->right)) t->color = RED;

    t = RemoveMinRec(t);
    t->color = BLACK;

    assert(Check(t));

    return t;
}

/**
 * @brief Recursive method to remove min
 * @param h the tree node
 * @return the tree node after the remove operation
 */
template <typename T>
Node<T>* RemoveMinRec(Node<T>* h) {
    if (IsEmpty(h->left)) return h->right; // save pointer
    if (!IsRedLink(h->left) && !IsRedLink(h->left->left)) h = MoveRedLeft(h);
    h->left = RemoveMinRec(h->left);
    return Balance(h);
}

/**
 * @brief Removes max element of the tree @c t
 * @param t the tree root node
 * @return the tree node after the remove operation
 * @throw std::runtime_error if the tree is empty
 */
template <typename T>
Node<T>* RemoveMax(Node<T>* t) {
    if (IsEmpty(t)) throw std::runtime_error("Remove from an empty");

    if (!IsRedLink(t->left) && !IsRedLink(t->right)) t->color = RED;

    t = RemoveMaxRec(t);
    t->color = BLACK;

    assert(Check(t));

    return t;
}

/**
 * @brief Recursive method to remove max
 * @param h the tree node
 * @return the tree node after the remove operation
 */
template <typename T>
Node<T>* RemoveMaxRec(Node<T>* h) {
    if (IsRedLink(h->left)) h = RotateRight(h);

    if (IsEmpty(h->right)) return h->left; // save pointer

    if (!IsRedLink(h->right) && !IsRedLink(h->right->left)) h = MoveRedRight(h);

    h->right = RemoveMaxRec(h->right);

    return Balance(h);
}

/**
 * @brief Removes the element with the given key from tree @c h
 * @param h the tree
 * @param key the key
 * @return the tree node after the remove operation
 */
template <typename T>
Node<T>* Remove(Node<T>* h, T key) {
    if (!Contains(h, key)) return h;

    if (!IsRedLink(h->left) && !IsRedLink(h->right)) h->color = RED;

    h = RemoveRec(h, key);
    h->color = BLACK;

    assert(Check(h));

    return h;
}

/**
 * @brief Recursive method for remove
 * @param h the tree node
 * @param key the key
 * @return the tree node after the remove
 */
template <typename T>
Node<T>* RemoveRec(Node<T>* h, T key) {
    if (key < h->key) {
        if (!IsRedLink(h->left) && !IsRedLink(h->left->left)) h = MoveRedLeft(h);
        h->left = RemoveRec(h->left, key);
    }
    else {
        if (IsRedLink(h->left)) h = RotateRight(h);
        if (key == h->key && IsEmpty(h->right)) return h->right;
        if (!IsRedLink(h->right) && !IsRedLink(h->right->left) ) h = MoveRedRight(h);
        if (key == h->key) {
            Node<T>* x = Min(h->right);
            h->key = x->key;
            h->right = RemoveMinRec(h->right);
        }
        else h->right = RemoveRec(h->right, key);
    }
    return Balance(h);
}

/**
 * @brief Splits the minimum element of the tree t from the remaining tree
 * @param t the tree root node
 * @return the min node and the new tree root
 */
template <typename T>
std::pair<Node<T>*, Node<T>*> ExtractMin(Node<T>* t) {
    if (IsEmpty(t)) throw std::runtime_error("Extract from an empty");
    Node<T>* min = Min(t);
    Node<T>* h = RemoveMin(t);
    return std::make_pair(min, h);
}


/**
 * @brief Splits the maximum element of the BST t from the remaining tree
 * @param t the tree root node
 * @return the new tree root and the max node
 */
template <typename T>
std::pair<Node<T>*, Node<T>*> ExtractMax(Node<T>* t) {
    if (IsEmpty(t)) throw std::runtime_error("Extract from an empty");
    Node<T>* max = Max(t);
    Node<T>* h = RemoveMax(t);
    return std::make_pair( max, h);
}

/* Split & Join functions */

/**
 * @brief Given two trees @c t1, @c t2 and a node @c x,
 * where for every key in each tree is valid @c t1 < @c x < @c t2 Returns a union tree of @c t1, @c t2 and @c x
 * @param t1 first tree root node
 * @param x node satisfying t1 < x < t2
 * @param t2 second tree root node
 * @return the union tree
 */
template <typename T>
Node<T>* Join(Node<T>* t1, Node<T>* x, Node<T>* t2) {

    assert(IsDummy(x->left) && IsDummy(x->right)); // x must have free pointers
    assert(IsEmpty(t1) || Max(t1)->key < x->key);  // t1 < x
    assert(IsEmpty(t2) || Min(t2)->key > x->key);  // t2 > x

    Node<T>* root = JoinRec(t1, x,  t2);
    root->color = BLACK;

    assert(Check(root));

    return root;
}

/**
 * @brief Join recursive method
 * @param t1 first tree root node
 * @param x node satisfying t1 < x < t2
 * @param t2 second tree root node
 * @return the union subtree
 */
template <typename T>
Node<T>* JoinRec(Node<T>* t1, Node<T>* x, Node<T>* t2) {
    if (Height(t1) < Height(t2)) {
        t2->left = JoinRec(t1, x, t2->left);
        return Balance(t2);
    }
    if (Height(t1) > Height(t2)) {
        t1->right = JoinRec(t1->right, x,  t2);
        return Balance(t1);
    }

    x->color = RED; // keeps height balance diff less than or equal to 1
    x->left = t1;
    x->right = t2;

    return Balance(x);
}

/**
 * @brief Splits the rooted tree at node @c y into two trees @c L, @c R and a node @c x. Where all keys in @c L are less than @c k,
 * all keys in @c R are greater than @c k and @c x.key = @c  k
 * @param y the root of the tree
 * @param k the key. k must be a key in the tree
 * @return a triple (L, x, R) as described
 * @throw std::runtime_error if the key k is not in the tree.
 */
template <typename T>
std::tuple<Node<T>*, Node<T>*, Node<T>*> Split(Node<T>* y, T k) {

    assert(Contains(y, k)); // k must be a key in the y tree

    if (!Contains(y, k)) throw std::runtime_error("key not found");

    auto [L, x, R] = SplitRec(y, k);

    assert(Check(L) && Check(x) && Check(R));

    return std::make_tuple(L, x, R);
}

/**
 * @brief Split recursive method
 * @param h the subtree node
 * @param k the key
 * @return a triple (L, x, R) as described
 */
template <typename T>
std::tuple<Node<T>*, Node<T>*, Node<T>*> SplitRec(Node<T>* h, T k) {
    if (h->key < k) {
        auto [L, x, R] = SplitRec(h->right, k);
        auto [l, r] = Detach(h);
        l->color = BLACK;
        return std::make_tuple(Join(l, h, L), x, R);
    }
    if (h->key > k) {
        auto [L, x, R] = SplitRec(h->left, k);
        auto [l, r] = Detach(h);
        r->color = BLACK;
        return std::make_tuple(L, x, Join(R, h, r));
    }

    auto [l, r] = Detach(h);
    l->color = BLACK;
    r->color = BLACK;

    return std::make_tuple(l, h, r);
}

/*---------------------------------------------*/


/* Show functions */

/**
 * @brief Shows the tree
 * @param t the tree
 */
template <typename T>
void Show(Node<T>* t) {
    if (!IsEmpty(t)) Show(t, 0);
}

/**
 * @brief Recursive print with space.
 * @param t the tree
 * @param s the number of spaces
 */
template <typename T>
void Show(Node<T>* t, const int s) {
    if (!IsEmpty(t)) {
        Show(t->left, s + 3);
        std::cout << std::string(s, ' ') << "(" << t->key
        << ", c=" << t->color
        << ", t=" << t->type
        << ", min=" << t->minDepth
        << ", max=" << t->maxDepth
        <<  ")" << std::endl;
        Show(t->right, s + 3);
    }
}

/*---------------------------------------------*/

/* Check functions */

/**
 * @brief Check if the tree is a BST
 * @param t the tree root node
 * @return @c true or @c false
 */
template <typename T>
bool IsBST(Node<T>* t) {
    if (IsEmpty(t)) return true;
    return IsBSTRec(t, GetDummy<T>(), GetDummy<T>());
}

/**
 * @brief Recursive method to check if the subtree h is a BST
 * @param h the subtree root node
 * @param min the subtree min
 * @param max the subtree max
 * @return @c true or @c false
 */
template <typename T>
bool IsBSTRec(Node<T>* h, Node<T>* min, Node<T>* max) {
    if (IsEmpty(h)) return true;
    if (!IsEmpty(min) && h->key < min->key) return false;
    if (!IsEmpty(max) && h->key > max->key) return false;
    return IsBSTRec(h->left, min, h) && IsBSTRec(h->right, h, max);
}

/**
 * @brief Checks if the tree is balanced
 * @param t tree root node
 * @return @c true or @c false
 */
template <typename T>
bool IsBalance(Node<T>* t) {
    int black = 0;
    Node<T>* x = t;
    while (!IsEmpty(x)) {
        if (!IsRedLink(x)) black++;
        x = x->left;
    }
    return IsBalance(t, black);
}

/**
 * @brief Recursive method to check if the tree is balanced
 * @param h the subtree node
 * @param black number of black links in the path
 * @return @c true or @c false
 */
template <typename T>
bool IsBalance(Node<T>* h, int black) {
    if (IsEmpty(h)) return black == 0;
    if (!IsRedLink(h)) black--;
    return IsBalance(h->left, black) && IsBalance(h->right, black);
}

/**
 * @brief Checks if the tree is a 2-3 tree
 * @param r the tree root
 * @return @c true or @c false
 */
template <typename T>
bool Is23(Node<T>* r) { return Is23Rec(r); }

/**
 * @brief Recursive method to check if the tree is a 2-3 tree.
 * @param h the subtree node
 * @return @c true or @c false
 */
template <typename T>
bool Is23Rec(Node<T>* h) {
    if (IsEmpty(h)) return true;
    if (IsRedLink(h->right)) return false;
    if (IsRedLink(h) && IsRedLink(h->left)) return false;
    return Is23(h->left) && Is23(h->right);
}

/**
 * @brief Checks if the tree is a left-leaning red-black tree
 * @param r the tree root
 * @return @c true or @c false
 */
template <typename T>
bool Check(Node<T>* r) { return IsBalance(r) && Is23(r) &&  IsBST(r); }

#endif //REDBLACKTREE_H