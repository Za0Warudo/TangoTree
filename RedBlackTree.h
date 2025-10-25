#ifndef REDBLACKTREE_H
#define REDBLACKTREE_H

/* Includes */
#include <algorithm>
#include <cassert>
#include <iostream>
#include <queue>
#include <tuple>
#include <bits/ostream.tcc>
#include <climits>

/**
 * @enum Color
 * @brief Data type to encode colors.
 */
enum Color {
    RED,       // Red link color
    BLACK      // Black link color
};

/**
 * @enum NodeType
 * @brief Data type for node type
 */
enum NodeType {
    REGULAR,
    EXTERNAL,
    DUMMY
};

/**
 * @brief Node struct for Red-Black-Tree.
 * @tparam T data type (also the key for the BST)
 */
template <typename T>
struct Node {
    T data;                             // Node's data and key
    Node *left, *right;                 // Left & Right children
    int size;                           // Node subtree size
    int height;                         // Node black height
    int depth;                          // Node depth (used only in tango tree)
    int maxDepth;                       // Tree max depth
    int minDepth;                       // Tree min depth
    Color color;                        // Parent link color
    NodeType type;                      // Node's type

    /**
     * @brief Builds a new node.
     * @param data information to be saved in the node
     * @param c Node's colors, default is RED
     * @param t Node's type, default is REGULAR
     */
    explicit Node(T data, const Color c = RED,  const NodeType t = REGULAR):
    data(data), left(nullptr), right(nullptr),
    size(1), height(0), depth(INT_MAX), maxDepth(-INT_MAX), minDepth(INT_MAX),
    color(c), type(t) {}
};

/**
 * @brief The dummy node, works similarly to null.
 */
template <typename T>
Node<T>* dummy = new Node<T>(T(), BLACK, DUMMY);

/**
 * @brief Returns the dummy node
 */
template <typename T>
Node<T>* GetDummy() { return dummy<T>; }

/* Constructor */

/**
 * @brief Initializes a new empty BST.
 * @return the representation of an empty BST
 */
template <typename T>
Node<T>* Initialize() { return GetDummy<T>(); }

/**
 * @brief Initializes a new node.
 * @param data the data to be stored in the node
 * @return the node
 */
template <typename T>
Node<T>* MakeNode(T data) {
    Node<T>* n = new Node<T>(data);
    n->left = GetDummy<T>();
    n->right = GetDummy<T>();
    return n;
}

/* Helpers functions */

/**
 * @brief Returns @c true if the @c x node is the dummy node, @c false otherwise.
 * @param x the node
 * @return @c true or @c false
 */
template <typename T>
bool IsDummy(Node<T>* x) {return x->type == DUMMY; }

/**
 * @brief Returns @c true if the node @c x is an external node, @c false otherwise.
 * @param x the node
 * @return @c true or @c false
 */
template <typename T>
bool IsExternal(Node<T>* x) { return x->type == EXTERNAL; }

/**
 * @brief Returns @c true if the node @c x is an external node or the dummy node, @c false otherwise.
 * @param x the node
 * @return @c true or @c false
 */
template <typename T>
bool IsExternalOrDummy(Node<T>* x) {return IsExternal<T>(x) || IsDummy(x); }

/**
 * @brief Returns @c true if the tree is empty, @c false otherwise.
 * @param t the tree root node
 * @return @c true or @c false
 */
template <typename T>
bool IsEmpty(Node<T>* t) { return IsExternalOrDummy(t); }

/**
 * @brief Get @c x node size if exists, 0 otherwise.
 * @param x the node
 * @return tree size
 */
template <typename T>
int Size(Node<T>* x) { return !IsExternalOrDummy(x)? x->size: 0; }

/**
 * @brief Get @c x node black height if exists, 0 otherwise.
 * @param x the node
 * @return tree height
 */
template <typename T>
int Height(Node<T>* x) { return !IsExternalOrDummy(x)? x->height: -1; }

/**
 * @brief Get @c x subtree min depth
 * @param x the node
 * @return tree min depth
 */
template <typename T>
int MinDepth(Node<T>* x) { return !IsExternalOrDummy(x)? x->minDepth: INT_MAX; }

/**
 * @brief Get @c x subtree max depth
 * @param x the node
 * @return tree max depth
 */
template <typename T>
int MaxDepth(Node<T>* x) { return !IsExternalOrDummy(x)? x->maxDepth: -INT_MAX; }

/**
 * @brief Get @c x node depth
 * @param x the node
 * @return node's depth
 */
template <typename T>
int Depth(Node<T>* x) { return !IsExternalOrDummy(x)? x->depth: -INT_MAX; }

/**
 * @brief @c true if the x node parent link is a red link, @c false otherwise.
 * @param x the node
 * @return @c true or @c false
 */
template <typename T>
bool IsRedLink(Node<T>* x) { return !IsExternalOrDummy(x)? x->color == RED : false; }

/**
 * @brief Updates size variable of node @c x.
 * @param x the node
 */
template <typename T>
void UpdateSize(Node<T>* x) { if (!IsDummy(x)) x->size = Size(x->left) + Size(x->right) + 1; }

/**
 * @brief Updates black height variable of node @c x.
 * @param x the node
 */
template <typename T>
void UpdateHeight(Node<T>* x) {
    if (!IsDummy(x)) x->height = std::max(
        Height(x->left) + IsRedLink(x->left)? 0 : 1,
        !IsDummy(x->right)? Height(x->right) + 1 : 0
        );
}

/**
 * @brief Updates node's min and max depth
 * @param x the node
 */
template <typename T>
void UpdateDepth(Node<T>* x) {
    if (!IsDummy(x)) {
        x->minDepth = std::min(x->depth, std::min(MinDepth(x->left), MinDepth(x->right)));
        x->maxDepth = std::max(x->depth, std::max(MaxDepth(x->left), MaxDepth(x->right)));
    }
}

/**
 * @brief Removes the left and right children of the given node and update the node's information.
 * @param x the node
 */
template <typename T>
void Detach(Node<T>* x) {
    if (!IsDummy(x)) {
        x->left = GetDummy<T>(); x->right = GetDummy<T>();
        UpdateHeight(x); UpdateSize(x); UpdateDepth(x);
        x->color = BLACK;
    }
}

/* Red-Black-Property functions */

/**
 * @brief Rotates the @c h tree to the right.
 * @param h the tree node
 * @return the new subtree after the rotation
 */
template <typename T>
Node<T>* RotateRight(Node<T>* h) {
    // assert(!IsExternalOrDummy(h) && IsRedLink(h->left) && !IsRedLink(h->right)); // conditions to do the right rotation

    Node<T>* y = h->left;
    h->left = y->right;
    y->right = h;
    y->color = h->color;
    h->color = RED;
    y->size = h->size;

    UpdateSize(h);
    UpdateHeight(h);
    UpdateHeight(y);
    UpdateDepth(h);
    UpdateDepth(y);

    return y;
}

/**
 * @brief Rotates the @c h tree to the left.
 * @param h the tree node
 * @return the new subtree after the rotation
 */
template <typename T>
Node<T>* RotateLeft(Node<T>* h) {
    // assert(!IsExternalOrDummy(h) && IsRedLink(h->right) && !IsRedLink(h->left)); // conditions to do the left rotation

    Node<T>* y = h->right;
    h->right = y->left;
    y->left = h;
    y->color = h->color;
    h->color = RED;
    y->size = h->size;

    UpdateSize(h);
    UpdateHeight(h);
    UpdateHeight(y);
    UpdateDepth(h);
    UpdateDepth(y);

    return y;
}

/**
 * @brief Make @c h.left or one of its children a red link.
 * @param h the tree node
 * @return the subtree after the operation
 */
template <typename T>
Node<T>* MoveRedLeft (Node<T>* h) {
    assert(!IsDummy(h) && IsRedLink(h) && !IsRedLink(h->left) && !IsRedLink(h->left->left)); // conditions to do the operation

    FlipColors(h);
    if (IsRedLink(h->right->left)) {
        h->right = RotateRight(h->right);
        h = RotateLeft(h);
        FlipColors(h);
    }
    return h;
}

/**
 * @brief Make @c h.right or one of its children a red link.
 * @param h the tree node
 * @return the subtree after the operation
 */
template <typename T>
Node<T>* MoveRedRight (Node<T>* h) {
    assert(!IsDummy(h) && IsRedLink(h) && !IsRedLink(h->right) && !IsRedLink(h->right->left)); // conditions to do the operation

    FlipColors(h);
    if (IsRedLink(h->left->left)) {
        h = RotateRight(h);
        FlipColors(h);
    }
    return h;
}

/**
 * @brief Flip @c h node and it's children colors.
 * @param h the tree node
 */
template <typename T>
void FlipColors(Node<T>* h) {

    // conditions to make the flip
    assert(!IsExternalOrDummy(h) && !IsExternalOrDummy(h->left) && !IsExternalOrDummy(h->right));
    assert(
    IsRedLink(h->left) && IsRedLink(h->right) && !IsRedLink(h) ||
    !IsRedLink(h->left) && !IsRedLink(h->right) && IsRedLink(h)
    );

    h->color = h->color == RED ? BLACK : RED;
    h->left->color = h->left->color == RED ? BLACK : RED;
    h->right->color = h->right->color == RED ? BLACK : RED;
}


/**
 * @brief Fix the Red-Black-Property in node @c x.
 * @param x the tree node
 * @return subtree after the balance operation
 */
template<typename T>
Node<T>* Balance(Node<T>* x) {
    if (!IsExternalOrDummy(x)) {
        if (!IsRedLink(x->left) && IsRedLink(x->right)) x = RotateLeft(x);
        if (IsRedLink(x->left) && IsRedLink((x->left)->left)) x = RotateRight(x);
        if (IsRedLink(x->left) && IsRedLink(x->right)) FlipColors(x);

        UpdateSize(x);
        UpdateHeight(x);
        UpdateDepth(x);
    }
    return x;
}

/* Insert functions */

/**
 * @brief Inserts the data in the tree @c t.
 * @param h the tree root node
 * @param data the new data to be inserted in the tree
 * @return the tree with data added
 */
template <typename T>
Node<T>* Insert(Node<T>* h, T data) {
    h = InsertRec(h, data);
    h->color = BLACK;
    assert(Check(h));
    return h;
}

/**
 * @brief Recursive auxiliar insert method.
 * @param h the tree node
 * @param data the new data to be inserted
 * @return the tree with data added
 */
template <typename T>
Node<T>* InsertRec(Node<T>* h, T data) {
    if (IsDummy(h)) return MakeNode(data);

    if (data < h->data) h->left = InsertRec(h->left, data);
    else if (data > h->data) h->right = InsertRec(h->right, data);
    else h->data = data;

    return Balance(h);
}

/* Query functions */

/**
 * @brief Return @c true if tree h contains data x, @c false otherwise.
 * @param h the tree
 * @param data the new data to be search
 * @return @c true or @c false.
 */
template <typename T>
bool Contains(Node<T>* h, T data) { return !IsDummy(Search(h, data).first); }

/**
 * @brief Find node containing the given data, @c nullptr otherwise.
 * @param h the tree
 * @param data the new data to be search
 * @param p the current level parent
 * @return the node containing the data search or @c nullptr
 */
template <typename T>
std::pair<Node<T>*, Node<T>*> Search(Node<T>* h, T data, Node<T>* p = GetDummy<T>()) {
    if (!IsExternalOrDummy(h)) {
        if (data < h->data) return Search(h->left, data, h);
        if (data > h->data) return Search(h->right, data, h);
    }
    return {h, p};
}

/**
 * @brief Returns the minimum element in the tree.
 * @param t tree node;
 * @return the minimum element in the tree
 * @throw std::runtime_error if the tree is empty
 */
template <typename T>
Node<T>* Min(Node<T>* t) {
    if (IsEmpty(t)) throw std::runtime_error("Min in an empty");
    while (!IsExternalOrDummy(t->left)) {
        t = t->left;
        std::cout << t->data << std::endl;
    }
    return t;
}

/**
 * @brief Returns the maximum element in the tree.
 * @param t tree node root node
 * @return the maximum element in the tree
 * @throw std::runtime_error if the tree is empty
 */
template <typename T>
Node<T>* Max(Node<T>* t) {
    if (IsEmpty(t)) throw std::runtime_error("Min in an empty");
    while (!IsExternalOrDummy(t->right)) t = t->right;
    return t;
}

/**
 * @brief return all data in the tree
 * @param r the tree root
 * @param q the queue to be store the keys
 */
template <typename T>
void GetAllKeys(Node<T>* r, std::queue<T> q) {
    if (!IsDummy(r)) GetAllKeysRec(r, q);
}

/**
 * @brief Recursive method to get all key in the tree
 * @param h the subtree root
 * @param q the queue to be store the keys
 */
template <typename T>
void GetAllKeysRec(Node<T>* h, std::queue<T> q) {
    if (!IsDummy(h)) {
        GetAllKeysRec(h->left, q);
        q.push(h->data);
        GetAllKeysRec(h->right, q);
    }
}

/* Remove functions */

/**
 * @brief Removes min element of the tree @c t.
 * @param t the tree node
 * @return the node of the tree after the remove
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
 * @brief Recursive method to remove min.
 * @param h the tree node
 * @return the node of the subtree after the remove
 */
template <typename T>
Node<T>* RemoveMinRec(Node<T>* h) {
    if (IsExternalOrDummy(h->left)) return h->right;

    if (!IsRedLink(h->left) && !IsRedLink(h->left->left)) h = MoveRedLeft(h);
    h->left = RemoveMinRec(h->left);
    return Balance(h);
}

/**
 * @brief Removes max element of the tree @c t.
 * @param t the tree root node
 * @return the node of the tree after the remove
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
 * @brief Recursive method to remove max.
 * @param h the tree node
 * @return the node of the tree after the remove
 */
template <typename T>
Node<T>* RemoveMaxRec(Node<T>* h) {
    if (IsRedLink(h->left)) h = RotateRight(h);

    if (IsExternalOrDummy(h->right)) return GetDummy<T>();

    if (!IsRedLink(h->right) && !IsRedLink(h->right->left)) h = MoveRedRight(h);

    h->right = RemoveMaxRec(h->right);

    return Balance(h);
}

/**
 * @brief Removes the element with the key given from tree @c h.
 * @param h the tree
 * @param key the key
 * @return the node of the tree after the remove
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
 * @brief Recursive method for remove.
 * @param h the tree node
 * @param key the key
 * @return the node of the subtree after the remove
 */
template <typename T>
Node<T>* RemoveRec(Node<T>* h, T key) {
    if (key < h->data) {
        if (!IsRedLink(h->left) && !IsRedLink(h->left->left)) h = MoveRedLeft(h);
        h->left = RemoveRec(h->left, key);
    }
    else {
        if (IsRedLink(h->left)) h = RotateRight(h);
        if (key == h->data && IsDummy(h->right)) return h->right;
        if (!IsRedLink(h->right) && !IsRedLink(h->right->left) ) h = MoveRedRight(h);
        if (key == h->data) {
            Node<T>* x = Min(h->right);
            h->data = x->data;
            h->right = RemoveMinRec(h->right);
        }
        else h->right = RemoveRec(h->right, key);
    }
    return Balance(h);
}

/**
 * @brief Splits the minimum element of the BST t from the remaining tree.
 * @param t the tree root node
 * @return A pair {MinNode, T}
 */
template <typename T>
std::pair<Node<T>*, Node<T>*> ExtractMin(Node<T>* t) {
    if (IsEmpty(t)) throw std::runtime_error("Extract from an empty");
    Node<T>* minNode = Min(t);
    Node<T>* h = RemoveMin(t);
    minNode->right = GetDummy<T>();
    return {minNode, h};
}


/**
 * @brief Splits the maximum element of the BST t from the remaining tree.
 * @param t the tree root node
 * @return A pair {T, MaxNode}
 */
template <typename T>
std::pair<Node<T>*, Node<T>*> ExtractMax(Node<T>* t) {
    if (IsEmpty(t)) throw std::runtime_error("Extract from an empty");
    Node<T>* maxNode = Max(t);
    auto [h, node, empty] = Split(t, maxNode->data);
    node->left = GetDummy<T>();
    node->right = GetDummy<T>();
    return {h, node};
}

/* Split & Join functions */

/**
 * @brief Given two trees @c t1, @c t2 and a node @c x, where for every key in each tree is valid in @c t1 < @c x < @c t2.
 * Returns a union tree of @c t1, @c t2 and @c x.
 * @param t1 root node of the first tree
 * @param x node where t1 < x < t2
 * @param t2 root node of the second tree
 * @return the union tree
 */
template <typename T>
Node<T>* Join(Node<T>* t1, Node<T>* x, Node<T>* t2) {

    Node<T>* root = JoinRec(t1, x,  t2);
    root->color = BLACK; // root is always a black link condition

    return root;
}

/**
 * @brief Recursive method to join two trees.
 * @param t1 node of the first tree
 * @param x node where t1 < x < t2
 * @param t2 node of the second tree
 * @return the union subtree
 */
template <typename T>
Node<T>* JoinRec(Node<T>* t1, Node<T>* x, Node<T>* t2) {
    std::cout << t1->data << " " << t2->data << std::endl;

    if (Height(t1) < Height(t2)) {
        t2->left = JoinRec(t1, x, t2->left);
        return Balance(t2);
    }
    if (Height(t1) > Height(t2)) {
        t1->right = JoinRec(t1->right, x,  t2);
        return Balance(t1);
    }
    x->color = RED;
    x->left = t1;
    x->right = t2;


    return Balance(x);
}


/**
 * @brief Splits the rooted tree at node @c y into two trees @c L, @c R and a node @c x. Where all keys in @c L are less than @c k,
 * all keys in @c R are greater than @c k and @c x.key = @c  k.
 * @param y the root of the tree
 * @param k the key. k must be a key in the tree
 * @return a triple (L, x, R) as described
 * @throw std::runtime_error if the key k is not in the tree.
 */
template <typename T>
std::tuple<Node<T>*, Node<T>*, Node<T>*> Split(Node<T>* y, T k) {
    if (!Contains(y, k)) throw std::runtime_error("key not found");
    auto [L, x, R] = SplitRec(y, k);
    return std::make_tuple(L, x, R);
}

/**
 * @brief Recursive method for split.
 * @param h the subtree node
 * @param k the key
 * @return a triple (L, x, R) as described
 */
template <typename T>
std::tuple<Node<T>*, Node<T>*, Node<T>*> SplitRec(Node<T>* h, T k) {
    if (h->data < k) {
        auto [L, x, R] = SplitRec(h->right, k);
        Node<T>* ll = h->left; ll->color = BLACK;
        Detach(h); // clean pointers
        return std::make_tuple(Join(ll, h, L), x, R);
    }
    if (h->data > k) {
        auto [L, x, R] = SplitRec(h->left, k);
        Node<T>* rr = h->right; rr->color = BLACK;
        Detach(h); // clean pointers
        return std::make_tuple(L, x, Join(R, h, rr));
    }
    // save pointers
    Node<T>* hl = h->left; hl->color = BLACK;
    Node<T>* hr = h->right; hr->color = BLACK;

    Detach(h);

    return std::make_tuple(hl, h, hr);
}


/* Show functions */

/**
 * @brief Shows the tree.
 * @param t the tree
 */
template <typename T>
void Show(Node<T>* t) {
    if (IsExternalOrDummy(t)) return;
    Show(t, 0);
}

/**
 * @brief Recursive print with space.
 * @param t the tree
 * @param s the number of spaces
 */
template <typename T>
void Show(Node<T>* t, const int s) {
    if (IsExternalOrDummy(t)) return;
    Show(t->left, s + 3);
    std::cout << std::string(s, ' ') << "(" << t->data
    << ", c=" << t->color
    << ", t=" << t->type
    << ", min=" << t->minDepth
    << ", max=" << t->maxDepth
    <<  ")" << std::endl;
    Show(t->right, s + 3);
}

/* Check function */

/**
 * @brief Check if the tree is a BST.
 * @param t the tree root node
 * @return @c true or @c false
 */
template <typename T>
bool IsBST(Node<T>* t) {
    if (IsEmpty(t)) return true;
    return IsBSTRec(t, GetDummy<T>(), GetDummy<T>());
}

/**
 * @brief Recursive method to check if the subtree h is a BST.
 * @param h the subtree root node
 * @param minNode the node with min key find so far
 * @param maxNode the node with max key find so far
 * @return @c true or @c false
 */
template <typename T>
bool IsBSTRec(Node<T>* h, Node<T>* minNode, Node<T>* maxNode) {
    if (IsDummy(h)) return true;
    if (!IsDummy(minNode) && h->data < minNode->data) return false;
    if (!IsDummy(maxNode) && h->data > maxNode->data) return false;
    return IsBSTRec(h->left, minNode, h) && IsBSTRec(h->right, h, maxNode);
}

/**
 * @brief Checks if the tree is balanced.
 * @param t tree root node
 * @return @c true or @c false
 */
template <typename T>
bool IsBalanced(Node<T>* t) {
    int black = 0;
    Node<T>* x = t;
    while (!IsExternalOrDummy(x)) {
        if (!IsRedLink(x)) black++;
        x = x->left;
    }
    return IsBalanced(t, black);
}

/**
 * @brief Recursive method to check if the tree is balanced.
 * @param h the subtree node
 * @param black number of black links in the path
 * @return @c true or @c false
 */
template <typename T>
bool IsBalanced(Node<T>* h, int black) {
    if (IsExternalOrDummy(h)) return black == 0;
    if (!IsRedLink(h)) black--;
    return IsBalanced(h->left, black) && IsBalanced(h->right, black);
}

/**
 * @brief Checks if the tree is a 2-3 tree.
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
    if (IsExternalOrDummy(h)) return true;
    if (IsRedLink(h->right)) return false;
    if (IsRedLink(h) && IsRedLink(h->left)) return false;
    return Is23(h->left) && Is23(h->right);
}

/**
 * @brief Checks if the tree is a left-leaning red-black tree.
 * @param r the tree root
 * @return @c true or @c false
 */
template <typename T>
bool Check(Node<T>* r) { return IsBalanced(r) && Is23(r) && IsBST(r); }

#endif //REDBLACKTREE_H