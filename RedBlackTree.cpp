/*
 * Menu Interface
 *
 * 1 <id> <val> - Insert(t_id, val)
 * 2 <id> <val> - Contains(t_id, val)
 * 3 <id> <val> - Remove(t_id, val)
 * 4 <id> - Print(t_id)
 *
 */

#include "RedBlackTree.h"

#include <iostream>
#include <map>

int main () {
    std::map<int, Node<int>*> tree_map;
    int operation, id, val;
    while (std::cin >> operation) {
        switch (operation) {
            case 1:
                std::cin >> id >> val;
                if (tree_map.contains(id)) tree_map[id] = Insert(tree_map[id], val);
                else {
                    tree_map[id] = Initialize<int>();
                    tree_map[id] = Insert(tree_map[id], val);
                    assert(IsBalanced(tree_map[id]));
                }
                break;
            case 2:
                std::cin >> id >> val;
                if (tree_map.contains(id)) std::cout << (Contains(tree_map[id], val)? "True" : "False") << std::endl;
                else std::cout << "Invalid ID" << std::endl;
                break;
            case 3:
                std::cin >> id >> val;
                if (tree_map.contains(id)) {
                    tree_map[id] = Remove(tree_map[id], val);
                    assert(IsBalanced(tree_map[id]));
                }
                else std::cout << "Invalid ID" << std::endl;
                break;
            case 4:
                int id1, id2; std::cin >> id1 >> id2;
                if (tree_map.contains(id1) && tree_map.contains(id2)) {
                    Node<int>* joinedTree = Join(tree_map[id1], tree_map[id2]);
                    Show(joinedTree);
                    assert(IsBalanced(joinedTree));
                }
                else std::cout << "Invalid ID" << std::endl;
                break;
            case 5:
                std::cin >> id;
                Show(tree_map[id]);
                break;
            default:
                std::cout << "Invalid Operation" << std::endl;
                break;
        }
    }
}
