#ifndef REDBLACKTREE_H
#define REDBLACKTREE_H

/* Includes */
#include <algorithm>
#include <cassert>
#include <iostream>
#include <tuple>
#include <bits/ostream.tcc>

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
    Color color;                        // Parent link color
    NodeType type;                      // Node's type

    /**
     * @brief Build a new node.
     * @param data information to be saved in the node
     * @param t Node's type, default is regular
     */
    explicit Node(T data, const NodeType t = REGULAR): data(data), left(nullptr), right(nullptr), size(1), height(0), color(RED), type(t) {}
};

/*
template <typename T>
Node<T>* dummy = new Node<T>(T(), DUMMY);
*/

/* Constructor */

/**
 * @brief Initializes a new empty BST.
 * @return the representation of an empty BST
 */
template <typename T>
Node<T>* Initialize() { return nullptr; }

/* Helpers functions */

/**
 * @brief Returns @c true if the tree is empty, @c false otherwise.
 * @param t the tree root node
 * @return @c true or @c false
 */
template <typename T>
bool IsEmpty(Node<T>* t) { return t == nullptr; }

/**
 * @brief Get x node size if exists, 0 otherwise.
 * @param x the node
 * @return tree size
 */
template <typename T>
int Size(Node<T>* x) { return x? x->size: 0; }

/**
 * @brief Get x node height if exists, -1 otherwise.
 * @param x the node
 * @return tree height
 */
template <typename T>
int Height(Node<T>* x) { return x? x->height: -1; }

/**
 * @internal
 * @brief @c true if the x node parent link is a red link, @c false otherwise.
 * @param x the node
 * @return @c true or @c false
 */
template <typename T>
bool IsRedLink(Node<T>* x) { return x? x->color == RED : false; }

/**
 * @internal
 * @brief Updates size variable of node x.
 * @param x the node
 */
template <typename T>
void UpdateSize(Node<T>* x) {
    if (x) x->size = Size(x->left) + Size(x->right) + 1;
}

/**
 * @internal
 * @brief Updates height variable of node x.
 * @param x the node
 */
template <typename T>
void UpdateHeight(Node<T>* x) {
    if (x) x->height = std::max(Height(x->left) + IsRedLink(x->left)? 0 : 1, x->right? Height(x->right) + 1 : 0);
}

/**
 * @internal
 * @brief Removes the left and right children of the given node and update the node's information.
 * @param x the node
 */
template <typename T>
void Detach(Node<T>* x) {
    if (x) {
        x->left = nullptr; x->right = nullptr;
        UpdateHeight(x); UpdateSize(x);
    }
}

/* Red-Black-Property functions */

/**
 * @internal
 * @brief Rotates the h tree to the right.
 * @param h the tree node
 * @return the new subtree after the rotation
 */
template <typename T>
Node<T>* RotateRight(Node<T>* h) {
    assert(h && IsRedLink(h->left) && !IsRedLink(h->right)); // conditions to do the right rotation

    Node<T>* y = h->left;
    h->left = y->right;
    y->right = h;
    y->color = h->color;
    h->color = RED;
    y->size = h->size;

    UpdateSize(h);
    UpdateHeight(h);
    UpdateHeight(y);

    return y;
}

/**
 * @internal
 * @brief Rotates the h tree to the left.
 * @param h the tree node
 * @return the new subtree after the rotation
 */
template <typename T>
Node<T>* RotateLeft(Node<T>* h) {
    assert(h && IsRedLink(h->right) && !IsRedLink(h->left)); // conditions to do the right rotation

    Node<T>* y = h->right;
    h->right = y->left;
    y->left = h;
    y->color = h->color;
    h->color = RED;
    y->size = h->size;

    UpdateSize(h);
    UpdateHeight(h);
    UpdateHeight(y);

    return y;
}

/**
 * @internal
 * @brief Make h.left or one of its children a red link.
 * @param h the tree node
 * @return the subtree after the operation
 */
template <typename T>
Node<T>* MoveRedLeft (Node<T>* h) {
    assert(h && IsRedLink(h) && !IsRedLink(h->left) && !IsRedLink(h->left->left)); // conditions to do the operation

    FlipColors(h);
    if (IsRedLink(h->right->left)) {
        h->right = RotateRight(h->right);
        h = RotateLeft(h);
        FlipColors(h);
    }
    return h;
}

/**
 * @internal
 * @brief Make h.right or one of its children a red link.
 * @param h the tree node
 * @return the subtree after the operation
 */
template <typename T>
Node<T>* MoveRedRight (Node<T>* h) {
    assert(h && IsRedLink(h) && !IsRedLink(h->right) && !IsRedLink(h->right->left)); // conditions to do the operation

    FlipColors(h);
    if (IsRedLink(h->left->left)) {
        h = RotateRight(h);
        FlipColors(h);
    }
    return h;
}

/**
 * @internal
 * @brief Flip h node and it's children colors.
 * @param h the tree node
 */
template <typename T>
void FlipColors(Node<T>* h) {

    // conditions to make the flip
    assert(h && h->left && h->right);
    assert(
    IsRedLink(h->left) && IsRedLink(h->right) && !IsRedLink(h) ||
    !IsRedLink(h->left) && !IsRedLink(h->right) && IsRedLink(h)
    );

    h->color = h->color == RED ? BLACK : RED;
    h->left->color = h->left->color == RED ? BLACK : RED;
    h->right->color = h->right->color == RED ? BLACK : RED;
}


/**
 * @internal
 * @brief Fix the Red-Black-Property in node x.
 * @param x the tree node
 * @return subtree after the balance operation
 */
template<typename T>
Node<T>* Balance(Node<T>* x) {
    if (!x) return nullptr;

    if (!IsRedLink(x->left) && IsRedLink(x->right)) x = RotateLeft(x);
    if (IsRedLink(x->left) && IsRedLink((x->left)->left)) x = RotateRight(x);
    if (IsRedLink(x->left) && IsRedLink(x->right)) FlipColors(x);

    UpdateSize(x);
    UpdateHeight(x);

    return x;
}

/* Insert functions */

/**
 * @brief Inserts the data in the tree t.
 * @param h the tree root node
 * @param data the new data to be inserted in the tree
 * @return the tree with data added
 */
template <typename T>
Node<T>* Insert(Node<T>* h, T data) {
    h = InsertRec(h, data);
    if (h) h->color = BLACK;
    return h;
}

/**
 * @internal
 * @brief Recursive auxiliar insert method.
 * @param h the tree node
 * @param data the new data to be inserted
 * @return the tree with data added
 */
template <typename T>
Node<T>* InsertRec(Node<T>* h, T data) {
    if (!h) return new Node<T>(data);

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
bool Contains(Node<T>* h, T data) { return Search(h, data) != nullptr; }

/**
 * @brief Find node containing the given data, @c nullptr otherwise.
 * @param h the tree
 * @param data the new data to be search
 * @return the node containing the data search or @c nullptr
 */
template <typename T>
Node<T>* Search(Node<T>* h, T data) {
    if (!h) return nullptr;
    if (data < h->data) return Search(h->left, data);
    if (data > h->data) return Search(h->right, data);
    return h;
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
    while (t->left) t = t->left;
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
    while (t->right) t = t->right;
    return t;
}

/* Remove functions */

/**
 * @brief Removes min element of the tree t.
 * @param t the tree node
 * @return the node of the tree after the remove
 * @throw std::runtime_error if the tree is empty
 */
template <typename T>
Node<T>* RemoveMin(Node<T>* t) {
    if (IsEmpty(t)) throw std::runtime_error("Remove from an empty");

    if (!IsRedLink(t->left) && !IsRedLink(t->right)) t->color = RED;

    t = RemoveMinRec(t);
    if (!IsEmpty(t)) t->color = BLACK;
    return t;
}

/**
 * @internal
 * @brief Recursive method to remove min.
 * @param h the tree node
 * @return the node of the subtree after the remove
 */
template <typename T>
Node<T>* RemoveMinRec(Node<T>* h) {
    if (!h->left) return nullptr;

    if (!IsRedLink(h->left) && !IsRedLink(h->left->left)) h = MoveRedLeft(h);
    h->left = RemoveMinRec(h->left);
    return Balance(h);
}

/**
 * @brief Removes max element of the tree t.
 * @param t the tree root node
 * @return the node of the tree after the remove
 * @throw std::runtime_error if the tree is empty
 */
template <typename T>
Node<T>* RemoveMax(Node<T>* t) {
    if (IsEmpty(t)) throw std::runtime_error("Remove from an empty");

    if (!IsRedLink(t->left) && !IsRedLink(t->right)) t->color = RED;

    t = RemoveMaxRec(t);
    if (!IsEmpty(t)) t->color = BLACK;
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

    if (!h->right) return nullptr;

    if (!IsRedLink(h->right) && !IsRedLink(h->right->left)) h = MoveRedRight(h);

    h->right = RemoveMaxRec(h->right);

    return Balance(h);
}

/**
 * @brief Removes the element with the key given from tree h.
 * @param h the tree
 * @param key the key
 * @return the node of the tree after the remove
 */
template <typename T>
Node<T>* Remove(Node<T>* h, T key) {
    if (!Contains(h, key)) return h;

    if (!IsRedLink(h->left) && !IsRedLink(h->right)) h->color = RED;

    h = RemoveRec(h, key);
    if (!IsEmpty(h)) h->color = BLACK;
    return h;
}

/**
 * @internal
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
        if (key == h->data && !h->right) return nullptr;
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

/* Split & Join functions */

/**
 * @brief Given two trees t1, t2 and a node x, where for every key in each tree is valid in t1 < x < t2.
 * Returns a union tree of t1, t2 and x.
 * @param t1 root node of the first tree
 * @param x node where t1 < x < t2
 * @param t2 root node of the second tree
 * @return the union tree
 */
template <typename T>
Node<T>* Join(Node<T>* t1, Node<T>* x, Node<T>* t2) {
    if (!t1 && !t2) return x;
    if (!t1) return Insert(t2, x->data);
    if (!t2) return Insert(t1, x->data);

    Node<T>* root = JoinRec(t1, x,  t2);
    if (!IsEmpty(root)) root->color = BLACK; // root is always a black link condition
    return root;
}

/**
 * @internal
 * @brief Recursive method to join two trees.
 * @param t1 node of the first tree
 * @param x node where t1 < x < t2
 * @param t2 node of the second tree
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

    x->color = RED;
    x->left = t1;
    x->right = t2;

    return Balance(x);
}


/**
 * @brief Splits the rooted tree at node y into two trees L, R and a node x. Where all keys in L are less than k,
 * all keys in R are greater than k and x.key = k.
 * @param y the root of the tree
 * @param k the key. k must be a key in the tree
 * @return a triple (L, x, R) as described
 * @throw std::runtime_error if the key k is not in the tree.
 */
template <typename T>
std::tuple<Node<T>*, Node<T>*, Node<T>*> Split(Node<T>* y, T k) {
    if (!Contains(y, k)) throw std::runtime_error("key not found");
    auto [L, x, R] = SplitRec(y, k);

    // root is always a black link condition
    if (L) L->color = BLACK;
    if (R) R->color = BLACK;

    x->color = BLACK;

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
        Node<T>* ll = h->left; if (ll) ll->color = BLACK;
        Detach(h); // clean pointers
        return std::make_tuple(Join(ll, h, L), x, R);
    }
    if (h->data > k) {
        auto [L, x, R] = SplitRec(h->left, k);
        Node<T>* rr = h->right; if (rr) rr->color = BLACK;
        Detach(h); // clean pointers
        return std::make_tuple(L, x, Join(R, h, rr));
    }
    // save pointers
    Node<T>* hl = h->left; if (hl) hl->color = BLACK;
    Node<T>* hr = h->right; if (hr) hr->color = BLACK;

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
    if (!t) return;
    Show(t, 0);
}

/**
 * @internal
 * @brief Recursive print with space.
 * @param t the tree
 * @param s the number of spaces
 */
template <typename T>
void Show(Node<T>* t, const int s) {
    if (!t) return;
    Show(t->left, s + 3);
    std::cout << std::string(s, ' ') << "(" << t->data << ", " << (t->color == RED? "RED" : "BLACK") << ")" << std::endl;
    Show(t->right, s + 3);
}

/* Check function */

/**
 * @internal
 * @brief Checks if the tree is balanced
 * @param t tree root node
 * @return @c true or @c false
 */
template <typename T>
bool IsBalanced(Node<T>* t) {
    int black = 0;
    Node<T>* x = t;
    while (x != nullptr) {
        if (!IsRedLink(x)) black++;
        x = x->left;
    }
    return IsBalanced(t, black);
}

/**
 * @internal
 * @brief Recursive method to check if the tree is balanced
 * @param t the subtree node
 * @param black number of black links in the path
 * @return @c true or @c false
 */
template <typename T>
bool IsBalanced(Node<T>* h, int black) {
    if (!h) return black == 0;
    if (!IsRedLink(h)) black--;
    return IsBalanced(h->left, black) && IsBalanced(h->right, black);
}

/**
 * @internal
 * @brief Checks if the tree is a 2-3 tree
 * @param r the tree root
 * @return @c true or @c false
 */
template <typename T>
bool Is23(Node<T>* r) { return Is23Rec(r); }

/**
 * @internal
 * @brief Recursive method to check if the tree is a 2-3 tree
 * @param h the subtree node
 * @return @c true or @c false
 */
template <typename T>
bool Is23Rec(Node<T>* h) {
    if (!h) return true;
    if (IsRedLink(h->right)) return false;
    if (IsRedLink(h) && IsRedLink(h->left)) return false;
    return Is23(h->left) && Is23(h->right);
}

/**
 * @internal
 * @brief Checks if the tree is a left-leaning red-black tree
 * @page r the root tree
 *@return @c true or @c false
 */
template <typename T>
bool Check(Node<T>* r) { return IsBalanced(r) && Is23(r); }

#endif //REDBLACKTREE_H