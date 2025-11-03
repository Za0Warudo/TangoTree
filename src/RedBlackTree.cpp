/*
 * Menu Interface
 *
 * 1 <id> <val>                     - Insert(t_id, val)
 * 2 <id> <val>                     - Contains(t_id, val)
 * 3 <id> <val>                     - Remove(t_id, val)
 * 4 <id> <key> <id>                - Join(t_id, Node(x_key), t_id)
 * 5 <id> <key>                     - Split(t_id, key)
 * 6 <id>                           - Print(t_id)
 *
 * Example:
 * 1 1 1
 * 1 1 3
 * 1 1 7
 * 6 1
 * 1 2 10
 * 1 2 15
 * 1 2 20
 * 6 2
 * 4 1 9 2
 * 6 3
 */

/* Includes */
#include "../RedBlackTree.h"
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
                    // Initializes the new tree
                    tree_map[id] = Initialize<int>();
                    tree_map[id] = Insert(tree_map[id], val);
                }
                break;
            }
            case 2: {
                std::cin >> id >> val;
                std::cout << (Contains((tree_map.contains(id)? tree_map[id] : GetDummy<int>()), val)? "True" : "False") << std::endl;
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
                                            tree_map.contains(id)? tree_map[id1] : GetDummy<int>(),
                                            new Node<int>(val),
                                            tree_map.contains(id)? tree_map[id2] : GetDummy<int>()
                                            );
                Show(joinedTree);
                break;
            }
            case 5: {
                int k; std::cin >> id >> k;
                if (tree_map.contains(id)) {
                    auto [L, x, R] = Split(tree_map[id], k);
                    std::cout << "L tree:" << std::endl;
                    Show(L);
                    std::cout << "x node:" << std::endl;
                    Show(x);
                    std::cout << "R tree:" << std::endl;
                    Show(R);
                }
                else std::cout << "Invalid ID" << std::endl;

                break;
            }
            case 6: {
                std::cin >> id;
                if (tree_map.contains(id)) { Show(tree_map[id]); }
                else std::cout << "Invalid ID" << std::endl;
                break;
            }
            default:
                std::cout << "Invalid Operation" << std::endl;
                break;
        }
    }
    return EXIT_SUCCESS;
}