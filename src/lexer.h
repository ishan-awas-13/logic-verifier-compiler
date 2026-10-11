#pragma once
#include "token.h"
#include <vector>
#include <string_view>

namespace cmm {

class Lexer {
public:
    explicit Lexer(std::string_view source);
    std::vector<Token> tokenize();

private:
    char advance();
    char peek() const;
    char peek_next() const;
    bool match(char expected);
    void skip_whitespace_and_comments();
    Token number();
    Token string();
    Token character();
    Token identifier();
    Token make_token(TokenType type);
    Token error_token(const char* message);

    std::string_view source_;
    size_t start_ = 0;
    size_t current_ = 0;
    int line_ = 1;
    int col_start_ = 1;
    int col_current_ = 1;
};

} // namespace cmm
