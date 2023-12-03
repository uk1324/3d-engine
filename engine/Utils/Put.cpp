#include <engine/Utils/Put.hpp>

void putnn(std::ostream& os, const char* format) {
 	os << format;
}

void putNewline(std::ostream& os) {
    os << '\n';
}

void putNewline() {
    putNewline(std::cout);
}
