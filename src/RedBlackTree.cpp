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
#include "RedBlackTree.h"
#include <map>

int main () {

    std::map<int, Node<int>*> trees; // map of trees

    int operation, id, val;

    while (std::cin >> operation) {
        switch (operation) {
            case 1: {  // Insert
                std::cin >> id >> val;
                if (trees.contains(id)) trees[id] = Insert(trees[id], val);
                else {
                    // Initializes the new tree and inserts val
                    trees[id] = Initialize<int>();
                    trees[id] = Insert(trees[id], val);
                }
                break;
            } case 2: { // Contain
                std::cin >> id >> val;
                std::cout << (Contains(trees.contains(id)? trees[id] : GetDummy<int>(), val)? "True" : "False") << std::endl;
                break;
            } case 3: { // Remove
                std::cin >> id >> val;
                if (trees.contains(id)) trees[id] = Remove(trees[id], val);
                else std::cout << "Invalid ID" << std::endl;
                break;
            } case 4: { // Join
                int id1, id2; std::cin >> id1 >> val >> id2;
                Node<int>* t3 = Join(
                    trees.contains(id)? trees[id1] : GetDummy<int>(),
                    new Node(val),
                    trees.contains(id)? trees[id2] : GetDummy<int>()
                    );
                Show(t3);
                break;
            } case 5: { // Split
                int k; std::cin >> id >> k;
                if (trees.contains(id)) {
                    auto [L, x, R] = Split(trees[id], k);
                    std::cout << "-------- L --------" << std::endl;
                    Show(L);
                    std::cout << "-------- x --------" << std::endl;
                    Show(x);
                    std::cout << "-------- R --------" << std::endl;
                    Show(R);
                    std::cout << "-------------------" << std::endl;
                }
                else std::cout << "Invalid ID" << std::endl;
                break;
            } case 6: {
                std::cin >> id;
                if (trees.contains(id))  Show(trees[id]);
                else std::cout << "Invalid ID" << std::endl;
                break;
            } default:
                std::cout << "Invalid Operation" << std::endl;
                break;
        }
    }
    return EXIT_SUCCESS;
}