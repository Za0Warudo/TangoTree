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

std::pair<int, int> Predecessor(const Node<int>*h, const int d) {
    assert(MaxDepth(h) >= d);   // checks if the tree h got Rx defined not null
    return PredecessorRec(h, d);
}

std::pair<int, int> PredecessorRec(const Node<int>* h, const int d) {
    if (MaxDepth(h->left) >= d) return Predecessor(h->left, d);
    if (h->depth >= d) return std::make_pair(!IsEmpty(h->left)? Max(h->left)->data : -1 , h->data);
    auto [pred, l] = Predecessor(h->right, d);
    return pred != -1? std::make_pair(pred, l) : std::make_pair(h->data, l);
}

std::pair<int, int> Successor(const Node<int>* h, const int d) {
    assert(MaxDepth(h) >= d);
    return SuccessorRec(h, d);
}

std::pair<int, int> SuccessorRec(const Node<int>* h, const int d) {
    if (MaxDepth(h->right) >= d) return Successor(h->right, d);
    if (h->depth >= d) return std::make_pair(!IsEmpty(h->right)? Min(h->right)->data : -1, h->data);
    auto [suc, r] = Successor(h->left, d);
    return suc != -1? std::make_pair(suc, r) : std::make_pair(h->data, r);
}

/* -------------------------------------------------- */

/* Search & Tango */

Node<int>* SearchTango(Node<int>* root, const int k) {

    for (auto [q, p] = Search(root, k); IsExternal(q); std::tie(q, p) = Search(root, k)) {
        root = Tango(root, q, p); // updates the preferred path
    }

    return root;
}

Node<int>* Tango(Node<int>* h, Node<int>* q, Node<int>* p) {

    if (h->maxDepth < q->minDepth) { // Rx is empty

        q->type = REGULAR;
        if (p->left == q) {
            auto [min, qq] = ExtractMin(q); // min will be an auxiliar node in join
            p->left = min->left; // save pointer
            min->left = GetDummy<int>();
            Detach(min);

            auto [tl, y, tr] = Split(h, p->data);

            Node<int>* taux = Join(qq, y, tr);

            return Join(tl, min, taux);
        } else {
            auto [qq, max] = ExtractMax(q); // max will be an auxiliar node in join
            p->right = max->right;
            max->right = GetDummy<int>();
            Detach(max);

            auto [tl, y, tr] = Split(h, p->data);

            Node<int>* taux = Join(tl, y, qq);

            return Join(taux, max, tr);
        }

    } else { // Rx is not empty

        const int d = q->minDepth;
        auto [pred, l] = Predecessor(h, d);
        auto [succ, r] = Successor(h, d);

        Node<int>* qq; Node<int>* max; Node<int>* min;

        q->type = REGULAR;
        enum Side {LEFT, RIGHT};
        Side side;
        if (p->left == q) side = LEFT;
        else side = RIGHT;
        if (l < q->data) {
            std::tie(qq, max) = ExtractMax(q); // max will be an auxiliar node in join
            if (side == LEFT) p->left = max->right;
            else p->right = max->right;
            Detach(max);
        } else {
            std::tie(min, qq) = ExtractMin(q); // min will be an auxiliar node in join
            if (side == LEFT) p->left = min->left;
            else p->right = min->left;
            Detach(min);
        }

        Node<int>* ta; Node<int>* xl; Node<int>* tl; Node<int>* tpp;
        Node<int>* tm; Node<int>* xr; Node<int>* tr; Node<int>* tp;

        if (pred == -1) { // don't have any keys smaller or equal to l
            ta = h;
            xl = GetDummy<int>();
        }
        else std::tie(tl, xl, ta) = Split(h, pred);

        if (succ == -1) { // don't have any keys greater or equal to r
            tm = ta;
            xr = GetDummy<int>();
        }
        else std::tie(tm, xr, tr) = Split(ta, succ);

        tm->type = EXTERNAL;

        if (tm->data < q->data) {
            if (IsDummy(xl)) tp = tm; // Don't split in tl, xl
            else tp = Join(tl, xl, tm);

            assert(!IsDummy(xr));
            tpp = Join(tp, xr, qq);

            return Join(tpp, max, tr);

        } else {
            if (IsDummy(xr)) tp = tm; // Don't split in xg, tg
            else tp = Join(tm, xr, tr);

            assert(!IsDummy(xl));
            tpp = Join(qq, xl, tp);

            return Join(tl, min, tpp);
        }
    }
}

/* -------------------------------------------------- */

/* Build functions */

Node<int>* TangoBuild(const int n) {
    Node<int>* root = TangoBuildRec(1, n,  0);
    root->type = REGULAR;
    return root;
}

Node<int>* TangoBuildRec(const int l, const int r, const int d) {
    if (l > r) return GetDummy<int>();
    const int m = (l + r + 1)/2; // Get the ceil
    Node<int>* x = BuildNode(m);
    x->color = BLACK;
    x->type = EXTERNAL;
    x->depth = x->maxDepth = x->minDepth = d;
    x->left = TangoBuildRec(l, m - 1, d+1);
    x->right = TangoBuildRec(m + 1, r, d+1);
    return x;
}

/* -------------------------------------------------- */

/* Show functions */

void ShowTangoRec(Node<int>* h, const int d) {
    if (!IsDummy(h)) {
        ShowTangoRec(h->left, d + 3);
        std::cout << std::string(d, ' ');
        std::cout << (h->type == REGULAR? RED_COLOR : RESET) << "(" << h->data << ", d=" << h->depth <<  ")" << std::endl;
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
                if (DEBUG) {
                    std::cout << "-------------------------------------------------------" << std::endl;
                    std::cout << "Search for the key: " << key << std::endl;
                }
                root = SearchTango(root, key);
                if (DEBUG) {
                    ShowTango(root);
                    std::cout << "-------------------------------------------------------" << std::endl;
                }
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