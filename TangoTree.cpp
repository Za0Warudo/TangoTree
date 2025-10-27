/**
 * How to use it:
 *
 * First off, type the tango tree size (must be a positive integer).
 *
 * After this you can perform search and show operation using:
 *
 * 1 <key>  - Search the key <key> in the tango tree
 * 2        - Show the current tango configuration
 *
 * Example:
 * 15
 * 1 4
 * 2
 * 1 10
 * 2
 * 1 1
 * 2
 */

/* Includes */
#include "TangoTree.h"
#include <iostream>


/* -------------------------------------------------- */

/* Sucessor & Predecessor */

int Predecessor(Node<int>* h, const int d) {
    if (!IsExternalOrDummy(h->left) && h->left->maxDepth >= d) return Predecessor(h->left, d);
    if (h->depth >= d) return !IsExternalOrDummy(h->left)? Max(h->left)->data : -1;
    const int pred = Predecessor(h->right, d);
    return pred > 0? pred : h->data;
}

int Successor(Node<int>* h, const int d) {
    if (!IsExternalOrDummy(h->right) && h->right->maxDepth >= d) return Successor(h->right, d);
    if (h->depth >= d) return !IsExternalOrDummy(h->right)? Min(h->right)->data : -1;
    const int suc = Successor(h->left, d);
    return suc > 0? suc : h->data;
}

/* -------------------------------------------------- */

/* Search & Tango */

Node<int>* SearchTango(Node<int>* root, const int k) {

    for (auto [q, p] = Search(root, k); IsExternal(q); std::tie(q, p) = Search(root, k))
        root = Tango(root, q, p); // Updates the preferred path

    return root;
}

Node<int>* Tango(Node<int>* h, Node<int>* q, Node<int>* p) {
    enum Side {LEFT, RIGHT}; Side side;

    if (p->left == q) {  // Save min(q).left pointer
        side = LEFT;
        q->type = REGULAR; // To perform Min operation
        Node<int>* min = Min(q);
        q->type = EXTERNAL;
        p->left = min->left;
        min->left = GetDummy<int>();
    }
    else { // Save max(q).right pointer
        side = RIGHT;
        q->type = REGULAR; // To perform Max operation
        Node<int>* max = Max(q);
        q->type = EXTERNAL;
        p->right = max->right;
        max->right = GetDummy<int>();
    }

    if (h->maxDepth < q->minDepth) { // Rx is empty
        if (side == LEFT) {
            auto [tl, pp, tg] = Split(h, p->data);

            q->type = REGULAR;
            Node<int>* tr = Join(q, pp, tg);

            auto [x, hh] = ExtractMin(tr);
            // x is min(q), the x->left & x->right pointers are safe (save in the beginning and by join operation)
            assert(IsDummy(x->left) && IsDummy(x->right));

            return Join(tl, x, hh);
        } else {
            auto [tl, pp, tg] = Split(h, p->data);

            q->type = REGULAR;
            Node<int>* tr = Join(tl, pp, q);

            auto [hh, x] = ExtractMax(tr);
            // x is max(q), the x->left & x->right pointers are safe (save in the beginning and by join operation)
            assert(IsDummy(x->left) && IsDummy(x->right));

            return Join(hh, x, tg);
        }
    } else { // Rx is not empty

        const int d = q->minDepth, l = Predecessor(h, d), r = Successor(h, d);

        Node<int>* taux; Node<int>* xl; Node<int>* tl; Node<int>* tpp;
        Node<int>* tm; Node<int>* xg; Node<int>* tg; Node<int>* tp;

        if (l == -1) { // don't have any keys smaller or equal to ll
            taux = h;
            xl = GetDummy<int>();
        }
        else std::tie(tl, xl, taux) = Split(h, l);

        if (r == -1) { // don't have any keys greater or equal to rr
            tm = taux;
            xg = GetDummy<int>();
        }
        else std::tie(tm, xg, tg) = Split(taux, r);


        tm->type = EXTERNAL;
        q->type = REGULAR;

        if (tm->data < q->data) {
            if (IsDummy(xl)) tp = tm; // Don't split in tl, xl
            else tp = Join(tl, xl, tm);

            if (IsDummy(xg)) tpp = tp;
            else tpp = Join(tp, xg, q);
            auto [hh, x] = ExtractMax(tpp);

            // x is max(q), the x->left & x->right pointers are safe (save in the beginning and by join operation)
            assert(IsDummy(x->left) && IsDummy(x->right));

            return Join(hh, x, tg);

        }
        else {
            if (IsDummy(xg)) tp = tm; // Don't split in xg, tg
            else tp = Join(tm, xg, tg);

            if (IsDummy(xl)) tpp = tp;
            else tpp = Join(q, xl, tp);
            auto [x, hh] = ExtractMin(tpp);

            assert(IsDummy(x->left) && IsDummy(x->right));

            return Join(tl, x, hh);
        }
    }
}

/* -------------------------------------------------- */

Node<int>* TangoBuild(const int n) {
    Node<int>* root = TangoBuildRec(1, n,  0);
    root->type = REGULAR;
    return root;
}

Node<int>* TangoBuildRec(const int l, const int r, const int d) {
    if (l > r) return GetDummy<int>();
    const int m = (l + r + 1)/2; // Get the ceil
    Node<int>* x = MakeNode(m);
    x->color = BLACK;
    x->type = EXTERNAL;
    x->depth = x->maxDepth = x->minDepth = d;
    x->left = TangoBuildRec(l, m-1, d+1);
    x->right = TangoBuildRec(m + 1, r, d+1);
    return x;
}

/* -------------------------------------------------- */

void ShowTangoRec(Node<int>* h, const int d) {
    if (!IsDummy(h)) {
        ShowTangoRec(h->left, d + 3);
        std::cout << std::string(d, ' ');
        std::cout << (h->type == REGULAR? RED_COLOR : RESET) << "(" << h->data << ")" << std::endl;
        ShowTangoRec(h->right, d + 3);
    }
}

void ShowTango(Node<int>* root) { ShowTangoRec(root, 0); }


/* -------------------------------------------------- */

int main () {
    int n; std::cin >> n;

    Node<int>* root = TangoBuild(n);

    int operation;

    while (std::cin >> operation) {
        switch (operation) {
            case 1: {
                int key; std::cin >> key;
                root = SearchTango(root, key);
                break;
            }
            case 2: {
                ShowTango(root);
                break;
            }
            default: {
                std::cout << "Invalid operation" << std::endl;
                break;
            }
        }
    }
    return EXIT_SUCCESS;
}