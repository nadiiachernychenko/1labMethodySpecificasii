#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>
#include <string>
#include "Lexer.h"

class LexerTest : public ::testing::Test {
protected:
    std::string input;
    std::vector<Token> tokens;

    void SetUp() override {
        input = "";
        tokens.clear();
    }

    void RunLexer() {
        Lexer lexer(input);
        tokens = lexer.tokenize();
    }
};

TEST_F(LexerTest, RecognizesKeywordAndIdentifier) {
    input = "program test";
    RunLexer();

    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].value, "program");
    EXPECT_EQ(tokens[0].type, TokenType::Keyword);
    EXPECT_EQ(tokens[1].value, "test");
    EXPECT_EQ(tokens[1].type, TokenType::Identifier);
}

TEST_F(LexerTest, RecognizesDecimalHexAndFloatNumbers) {
    input = "123 0x1F 3.14e-2";
    RunLexer();

    ASSERT_EQ(tokens.size(), 3u);
    EXPECT_EQ(tokens[0].type, TokenType::Number);
    EXPECT_EQ(tokens[0].value, "123");

    EXPECT_EQ(tokens[1].type, TokenType::Number);
    EXPECT_EQ(tokens[1].value, "0x1F");

    EXPECT_EQ(tokens[2].type, TokenType::Number);
    EXPECT_EQ(tokens[2].value, "3.14e-2");
}

TEST_F(LexerTest, RecognizesCharAndString) {
    input = "'A' 'HELLO'";
    RunLexer();

    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].type, TokenType::Char);
    EXPECT_EQ(tokens[0].value, "'A'");

    EXPECT_EQ(tokens[1].type, TokenType::String);
    EXPECT_EQ(tokens[1].value, "'HELLO'");
}

TEST_F(LexerTest, RecognizesComment) {
    input = "! this is a comment";
    RunLexer();

    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].type, TokenType::Comment);
    EXPECT_EQ(tokens[0].value, "! this is a comment");
}

TEST_F(LexerTest, RecognizesOperatorsAsWholeTokens) {
    input = "== != <= >= + - * / = < >";
    RunLexer();

    ASSERT_EQ(tokens.size(), 11u);

    std::vector<std::string> actual;
    for (const auto& t : tokens) {
        actual.push_back(t.value);
        EXPECT_EQ(t.type, TokenType::Operator);
    }

    std::vector<std::string> expected = {
        "==", "!=", "<=", ">=", "+", "-", "*", "/", "=", "<", ">"
    };

    EXPECT_EQ(actual, expected);
}

TEST_F(LexerTest, RecognizesPunctuators) {
    input = "( ) , ;";
    RunLexer();

    ASSERT_EQ(tokens.size(), 4u);
    EXPECT_EQ(tokens[0].type, TokenType::Punctuator);
    EXPECT_EQ(tokens[1].type, TokenType::Punctuator);
    EXPECT_EQ(tokens[2].type, TokenType::Punctuator);
    EXPECT_EQ(tokens[3].type, TokenType::Punctuator);
    EXPECT_EQ(tokens[0].value, "(");
    EXPECT_EQ(tokens[1].value, ")");
    EXPECT_EQ(tokens[2].value, ",");
    EXPECT_EQ(tokens[3].value, ";");
}

TEST_F(LexerTest, RecognizesUnknownSymbol) {
    input = "@";
    RunLexer();

    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].type, TokenType::Unknown);
    EXPECT_NE(tokens[0].type, TokenType::Identifier);
    EXPECT_EQ(tokens[0].value, "@");
}

TEST_F(LexerTest, TracksLineAndColumn) {
    input = "program test\ninteger x";
    RunLexer();

    ASSERT_GE(tokens.size(), 4u);

    EXPECT_EQ(tokens[0].value, "program");
    EXPECT_EQ(tokens[0].line, 1);
    EXPECT_EQ(tokens[0].col, 1);

    EXPECT_EQ(tokens[2].value, "integer");
    EXPECT_EQ(tokens[2].line, 2);
    EXPECT_EQ(tokens[2].col, 1);
}

TEST_F(LexerTest, ThrowsOnInvalidHexLiteral) {
    input = "0x";
    EXPECT_THROW(RunLexer(), std::runtime_error);
}

TEST_F(LexerTest, ThrowsOnUnterminatedStringLiteral) {
    input = "'HELLO";
    EXPECT_THROW(RunLexer(), std::runtime_error);
}

TEST_F(LexerTest, TokenSequenceTypes) {
    input = "program x = 5";
    RunLexer();

    std::vector<TokenType> types;
    for (const auto& t : tokens) {
        types.push_back(t.type);
    }

    std::vector<TokenType> expected = {
        TokenType::Keyword,
        TokenType::Identifier,
        TokenType::Operator,
        TokenType::Number
    };

    EXPECT_EQ(types, expected);
}

TEST_F(LexerTest, EmptyInput) {
    input = "";
    RunLexer();

    EXPECT_TRUE(tokens.empty());
}

class NumberLexerParameterizedTest : public ::testing::TestWithParam<const char*> {
};

TEST_P(NumberLexerParameterizedTest, RecognizesNumberVariants) {
    Lexer lexer(GetParam());
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].type, TokenType::Number);
    EXPECT_EQ(tokens[0].value, std::string(GetParam()));
}

INSTANTIATE_TEST_CASE_P(
    NumberInputs,
    NumberLexerParameterizedTest,
    ::testing::Values(
        "123",
        "0x1A",
        "3.14",
        "3.14e-2",
        ".5"
    )
);

class IdentifierLexerParameterizedTest : public ::testing::TestWithParam<const char*> {
};

TEST_P(IdentifierLexerParameterizedTest, RecognizesIdentifierVariants) {
    Lexer lexer(GetParam());
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].type, TokenType::Identifier);
    EXPECT_EQ(tokens[0].value, std::string(GetParam()));
}

static const char* identifierInputs[] = {
    "abc",
    "testVar",
    "x1",
    "value9"
};

INSTANTIATE_TEST_CASE_P(
    IdentifierInputs,
    IdentifierLexerParameterizedTest,
    ::testing::ValuesIn(identifierInputs)
);

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}