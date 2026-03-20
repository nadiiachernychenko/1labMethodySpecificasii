#include "Lexer.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <utility>

std::string tokenTypeToString(TokenType t) {
    switch (t) {
    case TokenType::Number:     return "Number";
    case TokenType::String:     return "String";
    case TokenType::Char:       return "Char";
    case TokenType::Comment:    return "Comment";
    case TokenType::Keyword:    return "Keyword";
    case TokenType::Operator:   return "Operator";
    case TokenType::Punctuator: return "Punctuator";
    case TokenType::Identifier: return "Identifier";
    default:                    return "Unknown";
    }
}

Lexer::Lexer(std::string s) : src(std::move(s)) {
    std::sort(operators.begin(), operators.end(),
        [](const std::string& a, const std::string& b) {
            if (a.size() != b.size()) {
                return a.size() > b.size();
            }
            return a > b;
        });
}

bool Lexer::match(const std::string& pat) const {
    return src.compare(pos, pat.size(), pat) == 0;
}

void Lexer::add(const std::string& v, TokenType t, int tokenLine, int tokenCol) {
    tokens.push_back({ v, t, tokenLine, tokenCol });
}

char Lexer::peek(int off) const {
    if (off < 0) {
        return '\0';
    }

    size_t index = static_cast<size_t>(pos) + static_cast<size_t>(off);
    if (index < src.size()) {
        return src[index];
    }

    return '\0';
}

char Lexer::get() {
    char c = peek();
    pos++;

    if (c == '\n') {
        line++;
        col = 1;
    }
    else {
        col++;
    }

    return c;
}

bool Lexer::isHexChar(char c) {
    return std::isxdigit(static_cast<unsigned char>(c)) != 0;
}

std::vector<Token> Lexer::tokenize() {
    tokens.clear();
    pos = 0;
    line = 1;
    col = 1;

    while (pos < static_cast<int>(src.size())) {
        char c = peek();

        if (std::isspace(static_cast<unsigned char>(c))) {
            get();
            continue;
        }

        int tokenLine = line;
        int tokenCol = col;

        if ((c == '0' && (match("0x") || match("0X"))) ||
            std::isdigit(static_cast<unsigned char>(c)) ||
            (c == '.' && std::isdigit(static_cast<unsigned char>(peek(1))))) {

            std::string num;

            if (match("0x") || match("0X")) {
                num += get();
                num += get();

                if (!isHexChar(peek())) {
                    throw std::runtime_error(
                        "Invalid hexadecimal literal at line " +
                        std::to_string(tokenLine) + ", col " +
                        std::to_string(tokenCol)
                    );
                }

                while (isHexChar(peek())) {
                    num += get();
                }

                add(num, TokenType::Number, tokenLine, tokenCol);
                continue;
            }

            if (std::isdigit(static_cast<unsigned char>(c))) {
                num += get();
                while (std::isdigit(static_cast<unsigned char>(peek()))) {
                    num += get();
                }
            }
            else if (c == '.') {
                num += get();
                while (std::isdigit(static_cast<unsigned char>(peek()))) {
                    num += get();
                }
            }

            if (peek() == '.') {
                num += get();
                while (std::isdigit(static_cast<unsigned char>(peek()))) {
                    num += get();
                }
            }

            if (peek() == 'e' || peek() == 'E' || peek() == 'd' || peek() == 'D') {
                num += get();

                if (peek() == '+' || peek() == '-') {
                    num += get();
                }

                if (!std::isdigit(static_cast<unsigned char>(peek()))) {
                    throw std::runtime_error(
                        "Invalid exponent in number at line " +
                        std::to_string(tokenLine) + ", col " +
                        std::to_string(tokenCol)
                    );
                }

                while (std::isdigit(static_cast<unsigned char>(peek()))) {
                    num += get();
                }
            }

            add(num, TokenType::Number, tokenLine, tokenCol);
            continue;
        }

        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            std::string id;
            id += get();

            while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_') {
                id += get();
            }

            std::string low = id;
            for (char& ch : low) {
                ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
            }

            if (keywords.count(low)) {
                add(id, TokenType::Keyword, tokenLine, tokenCol);
            }
            else {
                add(id, TokenType::Identifier, tokenLine, tokenCol);
            }

            continue;
        }

        if (c == '!' && peek(1) != '=') {
            std::string s;
            s += get();

            while (peek() != '\n' && peek() != '\0') {
                s += get();
            }

            add(s, TokenType::Comment, tokenLine, tokenCol);
            continue;
        }
        if (c == '\'') {
            std::string s;
            s += get();

            bool closed = false;

            while (peek() != '\0') {
                if (peek() == '\'' && peek(1) == '\'') {
                    s += get();
                    s += get();
                    continue;
                }

                if (peek() == '\'') {
                    s += get();
                    closed = true;
                    break;
                }

                s += get();
            }

            if (!closed) {
                throw std::runtime_error(
                    "Unterminated string/char literal at line " +
                    std::to_string(tokenLine) + ", col " +
                    std::to_string(tokenCol)
                );
            }

            size_t innerLen = (s.size() >= 2) ? (s.size() - 2) : 0;
            if (innerLen <= 1) {
                add(s, TokenType::Char, tokenLine, tokenCol);
            }
            else {
                add(s, TokenType::String, tokenLine, tokenCol);
            }

            continue;
        }

        bool found = false;
        for (const auto& op : operators) {
            if (match(op)) {
                std::string s;
                for (int i = 0; i < static_cast<int>(op.size()); ++i) {
                    s += get();
                }

                add(s, TokenType::Operator, tokenLine, tokenCol);
                found = true;
                break;
            }
        }

        if (found) {
            continue;
        }

        if (punct.count(c)) {
            std::string s;
            s += get();
            add(s, TokenType::Punctuator, tokenLine, tokenCol);
            continue;
        }

        std::string s;
        s += get();
        add(s, TokenType::Unknown, tokenLine, tokenCol);
    }

    return tokens;
}