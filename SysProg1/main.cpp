#include <iostream>
#include "Lexer.h"

int main() {
    std::string testCode = R"(program test
integer x, y
real z
x = 123
y = 0x1F
z = 3.14e-2
char_var = 'A'
str_var = 'HELLO'
! this is a comment
end program test
)";

    Lexer lexer(testCode);
    auto tokens = lexer.tokenize();

    for (const auto& t : tokens) {
        std::cout << "<" << t.value << ", " << tokenTypeToString(t.type)
            << ", line=" << t.line << ", col=" << t.col << ">\n";
    }

    return 0;
}