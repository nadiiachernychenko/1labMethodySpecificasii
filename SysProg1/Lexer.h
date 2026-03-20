#pragma once

#include <string>
#include <vector>
#include <unordered_set>

enum class TokenType {
    Number,
    String,
    Char,
    Comment,
    Keyword,
    Operator,
    Punctuator,
    Identifier,
    Unknown
};

struct Token {
    std::string value;
    TokenType type;
    int line;
    int col;
};

std::string tokenTypeToString(TokenType t);

class Lexer {
private:
    std::string src;
    std::vector<Token> tokens;
    int pos = 0;
    int line = 1;
    int col = 1;

    std::unordered_set<std::string> keywords = {
        "program", "end", "subroutine", "function", "integer", "real", "doubleprecision",
        "character", "logical", "do", "if", "then", "else", "endif", "call", "return",
        "stop", "continue", "read", "write", "print", "module", "use", "implicit", "none"
    };

    std::vector<std::string> operators = {
        "==", "!=", "<=", ">=",
        "+", "-", "*", "/", "=", "<", ">"
    };

    std::unordered_set<char> punct = { '(', ')', ',', ';' };

    bool match(const std::string& pat) const;
    void add(const std::string& v, TokenType t, int tokenLine, int tokenCol);
    char peek(int off = 0) const;
    char get();

public:
    explicit Lexer(std::string s);

    static bool isHexChar(char c);
    std::vector<Token> tokenize();
};