#pragma once

#include "token.h"
#include "ast.h"
#include <vector>
#include <string>

namespace cmm {

struct ParseError {
    int line;
    int col;
    std::string message;
};

class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    
    Program parse();
    const std::vector<ParseError>& get_errors() const { return errors_; }
    bool has_error() const { return !errors_.empty(); }

private:
    std::vector<Token> tokens_;
    std::size_t current_ = 0;
    std::vector<ParseError> errors_;

    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> declaration();
    std::unique_ptr<Stmt> assignment(Token name);
    std::unique_ptr<Stmt> show_stmt();
    std::unique_ptr<Stmt> check_stmt();
    std::unique_ptr<Stmt> until_stmt();
    std::unique_ptr<Stmt> give_stmt();
    std::unique_ptr<Stmt> stop_stmt();
    std::unique_ptr<Stmt> skip_stmt();
    std::unique_ptr<BlockStmt> block();

    std::unique_ptr<Expr> expr();
    std::unique_ptr<Expr> logical_or();
    std::unique_ptr<Expr> logical_and();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();
    std::unique_ptr<Expr> unary();
    std::unique_ptr<Expr> primary();

    bool is_type_token(TokenType type) const;
    bool consume_terminator(const std::string& message);

    bool match(TokenType type);
    bool check(TokenType type) const;
    bool is_at_end() const;
    Token advance();
    Token peek() const;
    Token previous() const;
    Token consume(TokenType type, const std::string& message);
    
    void error(const Token& token, const std::string& message);
    void synchronize();
    void skip_newlines();
};

} // namespace cmm
