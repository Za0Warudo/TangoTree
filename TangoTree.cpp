//
// Created by zawarudo on 9/17/25.
//

#include "TangoTree.h"

/* Auxiliary functions */
Node<int>* TangoBuild(int l, int r, int d);

TangoTree::TangoTree(int n) {
    root = TangoBuild(1, n,  0);
    root->type = REGULAR;
}

void TangoTree::show() { Show(root);}

Node<int> * TangoTree::search(const int key) {
    Node<int>* h = Search(root, key);
    while (h->data != key) {
        // TODO
        h = Search(h, key);
    }
    return h;
}


/* Auxiliary implementation */

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
    TangoTree tree(15);
    tree.show();
    return 0;
}