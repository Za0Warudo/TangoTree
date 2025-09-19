//
// Created by zawarudo on 9/17/25.
//

#ifndef TANGOTREE_H
#define TANGOTREE_H

#include "RedBlackTree.h"

class TangoTree {
private:
    Node<int>* root;


public:

    explicit TangoTree(int n);

    void show();

    Node<int>* search(const int key);
};

#endif //TANGOTREE_H
