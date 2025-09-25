#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <cctype>
using namespace std;

enum class TokenType {
    Number, String, Char, Comment,
    Keyword, Operator, Punctuator, Identifier, Unknown
};

struct Token {
    string value;
    TokenType type;
    int line, col;
};

string tokenTypeToString(TokenType t) {
    switch (t) {
    case TokenType::Number:       return "Number";
    case TokenType::String:       return "String";
    case TokenType::Char:         return "Char";
    case TokenType::Comment:      return "Comment";
    case TokenType::Keyword:      return "Keyword";
    case TokenType::Operator:     return "Operator";
    case TokenType::Punctuator:   return "Punctuator";
    case TokenType::Identifier:   return "Identifier";
    default:                      return "Unknown";
    }
}

class Lexer {
    string src;
    vector<Token> tokens;
    int pos = 0, line = 1, col = 1;

    unordered_set<string> keywords = {
        "program","end","subroutine","function","integer","real","doubleprecision",
        "character","logical","do","if","then","else","endif","call","return",
        "stop","continue","read","write","print","module","use","implicit","none"
    };

    vector<string> operators = {
        "==","!=","<=",">=",
        "+","-","*","/","=","<",">"
    };

    unordered_set<char> punct = { '(',')',',',';' };

    bool match(const string& pat) {
        return src.compare(pos, pat.size(), pat) == 0;
    }

    void add(const string& v, TokenType t) {
        tokens.push_back({ v,t,line,col });
    }

    char peek(int off = 0) {
        if (pos + off < (int)src.size()) return src[pos + off];
        return '\0';
    }

    char get() {
        char c = peek();
        pos++;
        if (c == '\n') { line++; col = 1; }
        else col++;
        return c;
    }

public:
    Lexer(string s) : src(move(s)) {
        sort(operators.begin(), operators.end(), [](const string& a, const string& b) {
            if (a.size() != b.size()) return a.size() > b.size();
            return a > b;
            });
    }

    static bool isHexChar(char c) {
        return isxdigit((unsigned char)c);
    }

    vector<Token> tokenize() {
        while (pos < (int)src.size()) {
            char c = peek();

            if (isspace((unsigned char)c)) { get(); continue; }

            if ((c == '0' && (match("0x") || match("0X"))) ||
                isdigit((unsigned char)c) ||
                (c == '.' && isdigit((unsigned char)peek(1)))) {

                string num;

                if (match("0x") || match("0X")) {
                    num += get(); // '0'
                    num += get(); // 'x' или 'X'
                    while (isHexChar(peek())) num += get();
                    add(num, TokenType::Number);
                    continue;
                }

                if (isdigit((unsigned char)c)) {
                    num += get(); 
                    while (isdigit((unsigned char)peek())) num += get();
                }
                else if (c == '.') {
                    num += get(); 
                    while (isdigit((unsigned char)peek())) num += get();
                }

                if (peek() == '.') {
                    num += get();
                    while (isdigit((unsigned char)peek())) num += get();
                }

                if (peek() == 'e' || peek() == 'E' || peek() == 'd' || peek() == 'D') {
                    num += get();
                    if (peek() == '+' || peek() == '-') num += get();
                    while (isdigit((unsigned char)peek())) num += get();
                }

                add(num, TokenType::Number);
                continue;
            }

            if (isalpha((unsigned char)c)) {
                string id;
                id += get(); 
                while (isalnum((unsigned char)peek()) || peek() == '_') id += get();
                string low = id;
                for (auto& ch : low) ch = tolower((unsigned char)ch);
                if (keywords.count(low)) add(id, TokenType::Keyword);
                else add(id, TokenType::Identifier);
                continue;
            }

            if (c == '!') {
                string s;
                s += get(); 
                while (peek() != '\n' && peek() != '\0') s += get();
                add(s, TokenType::Comment);
                continue;
            }

            if (c == '\'') {
                string s;
                s += get(); 
                while (peek() != '\0') {
                    if (peek() == '\'' && peek(1) == '\'') {
                        s += get();
                        s += get(); 
                        continue;
                    }
                    if (peek() == '\'') { s += get(); break; }
                    s += get();
                }
                size_t inner_len = 0;
                if (s.size() >= 2) {
                    inner_len = s.size() - 2;
                }
                if (inner_len <= 1) add(s, TokenType::Char);
                else add(s, TokenType::String);
                continue;
            }

            bool found = false;
            for (auto& op : operators) {
                if (match(op)) {
                    string s;
                    for (int i = 0; i < (int)op.size(); ++i) s += get();
                    add(s, TokenType::Operator);
                    found = true;
                    break;
                }
            }
            if (found) continue;

            if (punct.count(c)) {
                string s; s += get();
                add(s, TokenType::Punctuator);
                continue;
            }

            {
                string s; s += get();
                add(s, TokenType::Unknown);
            }
        }
        return tokens;
    }
};

int main() {
    string testCode = R"(program test
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

    for (auto& t : tokens) {
        cout << "<" << t.value << ", " << tokenTypeToString(t.type) << ">\n";
    }
    return 0;
}
