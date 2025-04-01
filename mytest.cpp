#include <iostream>
#include <sstream>
#include "bitarray.h"

int main() {
    BitArray<> b3 = BitArray<>{};
    std::ostringstream os;
    os << "101" << 'a' << "0101";
    std::istringstream is{os.str()};
    b3 = BitArray<>{};
    is >> b3;
    is.get();
    is >> b3;
    b3.to_string();
    os.str("");
    os << 'a';
    std::istringstream is2{os.str()};
    is2 >> b3;
    (!is2);
    std::cout << b3.to_string() << std::endl;

    std::cout << (b3).to_string() << std::endl;

    return 0;
}