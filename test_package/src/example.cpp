#include "tiny_pointers/tiny_pointers.hpp"

#include <iostream>

int main()
{
    if (tiny_pointers::entry_point()) {
        std::cout << "Test package is working!" << std::endl;
    }
    return 0;
}
