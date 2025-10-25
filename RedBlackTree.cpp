/*
 * Menu Interface
 *
 * 1 <id> <val>                     - Insert(t_id, val)
 * 2 <id> <val>                     - Contains(t_id, val)
 * 3 <id> <val>                     - Remove(t_id, val)
 * 4 <id> <key> <id>                - Join(t_id, Node(x_key), t_id)
 * 5 <id> <key>                     - Split(t_id, key)
 * 6 <id>                           - Print(t_id)
 */


#include "RedBlackTree.h"

#include <iostream>
#include <map>

int main () {
    std::map<int, Node<int>*> tree_map;
    int operation, id, val;
    while (std::cin >> operation) {
        switch (operation) {
            case 1: {
                std::cin >> id >> val;
                if (tree_map.contains(id)) tree_map[id] = Insert(tree_map[id], val);
                else {
                    tree_map[id] = Initialize<int>();
                    tree_map[id] = Insert(tree_map[id], val);
                }
                break;
            }
            case 2: {
                std::cin >> id >> val;
                std::cout << (Contains(tree_map.contains(id)? tree_map[id] : nullptr, val)? "True" : "False") << std::endl;
                break;
            }
            case 3: {
                std::cin >> id >> val;
                if (tree_map.contains(id)) { tree_map[id] = Remove(tree_map[id], val); }
                else std::cout << "Invalid ID" << std::endl;
                break;
            }
            case 4: {
                int id1, id2; std::cin >> id1 >> val >> id2;
                Node<int>* joinedTree = Join(
                                            tree_map.contains(id)? tree_map[id1] : nullptr,
                                            new Node<int>(val),
                                            tree_map.contains(id)? tree_map[id2] : nullptr
                                            );
                Show(joinedTree);
                break;
            }
            case 5: {
                int k; std::cin >> id >> k;
                if (tree_map.contains(id)) {
                    auto [L, x, R] = Split(tree_map[id], k);

                    std::cout << "L:" << std::endl;
                    Show(L);
                    std::cout << "x:" << std::endl;
                    Show(x);
                    std::cout << "R:" << std::endl;
                    Show(R);
                }
                else {
                    std::cout << "Invalid ID" << std::endl;
                }
                break;
            }
            case 6: {
                std::cin >> id;
                Show(tree_map[id]);
                break;
            }
            default:
                std::cout << "Invalid Operation" << std::endl;
                break;
        }
    }
}
