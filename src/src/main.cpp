#include <iostream>
#include "tree.hpp"
#include "node.hpp"

int main() {
    std::cout << "Hello, World!" << std::endl;
    
    std::cout << (sizeof Tree::Node<int>(false)) << std::endl;
    return 0;
}
