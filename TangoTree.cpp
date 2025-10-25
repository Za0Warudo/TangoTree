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
 *
 */

#include "TangoTree.h"

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

/********************************/

/* Tango */

Node<int>* Tango(Node<int>* h, Node<int>* q, Node<int>* p) {
    enum Side {LEFT, RIGHT};

    Side side;
    if (p->left == q) {  // save min(q).left pointer
        side = LEFT;
        q->type = REGULAR;
        Node<int>* min = Min(q);
        p->left = min->left;
        min->left = GetDummy<int>();
        q->type = EXTERNAL;
    }
    else { // save max(q).right pointer
        side = RIGHT;
        q->type = REGULAR;
        Node<int>* max = Max(q);
        p->right = max->right;
        max->right = GetDummy<int>();
        q->type = EXTERNAL;
    }

    if (h->maxDepth < q->minDepth) { // Rx is empty
        if (side == LEFT) { // p->left was equal to q
            auto [tl, pp, tg] = Split(h, p->data);
            q->type = REGULAR;
            Node<int>* tr = Join(q, pp, tg);
            auto [x, hh] = ExtractMin(tr);
            assert(IsDummy(x->left) && IsDummy(x->right));
            return Join(tl, x, hh);
        }
        else { // p->right was equal to q
            auto [tl, pp, tg] = Split(h, p->data);
            q->type = REGULAR;
            Node<int>* tr = Join(tl, pp, q);
            auto [hh, x] = ExtractMax(tr);
            assert(IsDummy(x->left) && IsDummy(x->right));
            return Join(hh, x, tg);
        }
    }
    else { // Rx is not empty

        const int d = q->minDepth;
        const int ll = Predecessor(h, d);
        const int rr = Successor(h, d);

        Node<int>* taux; Node<int>* xl; Node<int>* tl;
        Node<int>* tlr; Node<int>* xr; Node<int>* tg; Node<int>* tp;

        if (ll == -1) { // don't have any keys smaller or equal to ll
            taux = h;
            xl = GetDummy<int>();
        }
        else  std::tie(tl, xl, taux) = Split(h, ll);

        if (rr == -1) { // don't have any keys greater or equal to rr
            tlr = taux;
            xr = GetDummy<int>();
        }
        else std::tie(tlr, xr, tg) = Split(taux, rr);


        tlr->type = EXTERNAL;
        q->type = REGULAR;

        if (tlr->data < q->data) {
            if (IsDummy(xl)) tp = tlr;
            else tp = Join(tl, xl, tlr);

            Node<int>* tpp = Join(tp, xr, q);
            auto [hh, xaux] = ExtractMax(tpp);

            assert(IsDummy(xaux->left) && IsDummy(xaux->right));

            return Join(hh, xaux, tg);

        }
        else {
            if (IsDummy(xr)) tp = tlr;
            else tp = Join(tlr, xr, tg);

            Node<int>* tpp = Join(q, xl, tp);
            auto [xaux, hh] = ExtractMin(tpp);

            assert(IsDummy(xaux->left) && IsDummy(xaux->right));

            return Join(tl, xaux, hh);
        }
    }
}

/********************************/


Node<int>* BuildTangoTree(const int n) {
    Node<int>* root = TangoBuild(1, n,  0);
    root->type = REGULAR;
    return root;
}

void ShowTangoRec(Node<int>* h, const int depth) {
    if (!IsDummy(h)) {
        ShowTangoRec(h->left, depth + 3);
        std::cout << std::string(depth, ' ');
        std::cout << (h->type == REGULAR? RED_COLOR : RESET) << "(" << h->data << ")" << std::endl;
        ShowTangoRec(h->right, depth + 3);
    }
}

void ShowTango(Node<int>* root) { ShowTangoRec(root, 0);}

Node<int>* SearchTango(Node<int>* root, const int key) {
    auto [q, p] = Search(root, key);
    while (IsExternal(q)) {
        root = Tango(root, q, p);
        ShowTango(root);
        std::tie(q, p) = Search(root, key);
    }
    return root;
}

Node<int>* TangoBuild(const int l, const int r, const int d) {
    if (l > r) return GetDummy<int>();
    const int m = (l + r + 1)/2; // get the ceil
    Node<int>* x = MakeNode(m);
    x->color = BLACK;
    x->type = EXTERNAL;
    x->depth = x->maxDepth = x->minDepth = d;
    x->left = TangoBuild(l, m-1, d+1);
    x->right = TangoBuild(m + 1, r, d+1);
    return x;
}

int main () {
    int n; std::cin >> n;

    Node<int>* root = BuildTangoTree(n);

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