#include "parser.h"
#include <charconv>

namespace cmm {

Parser::Parser(const std::vector<Token>& tokens) : tokens_(tokens) {}

Program Parser::parse() {
    Program program;
    while (!is_at_end()) {
        skip_newlines();
        if (is_at_end()) break;
        if (auto stmt = statement()) {
            program.statements.push_back(std::move(stmt));
        } else {
            synchronize();
        }
    }
    return program;
}

bool Parser::is_type_token(TokenType type) const {
    return type == TokenType::Num || type == TokenType::Decimal ||
           type == TokenType::Word || type == TokenType::Letter ||
           type == TokenType::Boolean || type == TokenType::List;
}

bool Parser::consume_terminator(const std::string& message) {
    if (match(TokenType::Semicolon)) {
        skip_newlines();
        return true;
    }
    if (match(TokenType::Newline)) {
        skip_newlines();
        return true;
    }
    if (check(TokenType::RightBrace) || is_at_end()) {
        return true;
    }
    error(peek(), message);
    return false;
}

std::unique_ptr<Stmt> Parser::statement() {
    if (check(TokenType::Fixed) || is_type_token(peek().type)) return declaration();
    if (match(TokenType::Show)) return show_stmt();
    if (match(TokenType::Check)) return check_stmt();
    if (match(TokenType::Until)) return until_stmt();
    if (match(TokenType::Give)) return give_stmt();
    if (match(TokenType::Stop)) return stop_stmt();
    if (match(TokenType::Skip)) return skip_stmt();
    
    if (match(TokenType::Identifier)) {
        Token name = previous();
        return assignment(name);
    }

    error(peek(), "Expected statement.");
    return nullptr;
}

std::unique_ptr<Stmt> Parser::declaration() {
    bool is_fixed = match(TokenType::Fixed);
    if (!is_type_token(peek().type)) {
        error(peek(), "Expected type name in variable declaration.");
        return nullptr;
    }
    Token type_token = advance();
    std::string type_name(type_token.lexeme);
    
    Token name = consume(TokenType::Identifier, "Expected variable name after type.");
    if (name.type == TokenType::Error) return nullptr;
    
    std::unique_ptr<Expr> initializer = nullptr;
    if (match(TokenType::Equal)) {
        initializer = expr();
        if (!initializer) return nullptr;
    }
    
    if (!consume_terminator("Expected ';' or newline after declaration.")) return nullptr;
    
    return std::make_unique<VarDecl>(type_token.line, type_token.col, std::move(type_name), std::string(name.lexeme), std::move(initializer), is_fixed);
}

std::unique_ptr<Stmt> Parser::assignment(Token name) {
    if (match(TokenType::PlusPlus)) {
        if (!consume_terminator("Expected ';' or newline after '++'.")) return nullptr;
        auto var = std::make_unique<VariableExpr>(name.line, name.col, std::string(name.lexeme));
        auto one = std::make_unique<NumberLiteral>(name.line, name.col, 1.0);
        Token plus_op{TokenType::Plus, "+", name.line, name.col};
        auto add = std::make_unique<BinaryExpr>(name.line, name.col, std::move(var), plus_op, std::move(one));
        return std::make_unique<AssignStmt>(name.line, name.col, std::string(name.lexeme), std::move(add), AssignKind::Increment);
    }
    if (match(TokenType::MinusMinus)) {
        if (!consume_terminator("Expected ';' or newline after '--'.")) return nullptr;
        auto var = std::make_unique<VariableExpr>(name.line, name.col, std::string(name.lexeme));
        auto one = std::make_unique<NumberLiteral>(name.line, name.col, 1.0);
        Token minus_op{TokenType::Minus, "-", name.line, name.col};
        auto sub = std::make_unique<BinaryExpr>(name.line, name.col, std::move(var), minus_op, std::move(one));
        return std::make_unique<AssignStmt>(name.line, name.col, std::string(name.lexeme), std::move(sub), AssignKind::Decrement);
    }

    if (consume(TokenType::Equal, "Expected '=' after identifier.").type == TokenType::Error) return nullptr;
    
    auto value = expr();
    if (!value) return nullptr;
    
    if (!consume_terminator("Expected ';' or newline after assignment.")) return nullptr;
    
    return std::make_unique<AssignStmt>(name.line, name.col, std::string(name.lexeme), std::move(value));
}

std::unique_ptr<Stmt> Parser::show_stmt() {
    Token show_token = previous();
    bool has_paren = match(TokenType::LeftParen);
    
    auto value = expr();
    if (!value) return nullptr;

    if (has_paren) {
        if (consume(TokenType::RightParen, "Expected ')' after 'show' argument.").type == TokenType::Error) {
            return nullptr;
        }
    }
    
    if (!consume_terminator("Expected ';' or newline after show statement.")) return nullptr;
    
    return std::make_unique<EmitStmt>(show_token.line, show_token.col, std::move(value));
}

std::unique_ptr<Stmt> Parser::check_stmt() {
    Token check_token = previous();
    bool has_paren = match(TokenType::LeftParen);
    
    auto condition = expr();
    if (!condition) return nullptr;

    if (has_paren) {
        if (consume(TokenType::RightParen, "Expected ')' after check condition.").type == TokenType::Error) {
            return nullptr;
        }
    }
    
    auto then_branch = block();
    if (!then_branch) return nullptr;
    
    std::unique_ptr<BlockStmt> else_branch = nullptr;
    if (match(TokenType::Otherwise)) {
        else_branch = block();
        if (!else_branch) return nullptr;
    }
    
    return std::make_unique<IfStmt>(check_token.line, check_token.col, std::move(condition), std::move(then_branch), std::move(else_branch));
}

std::unique_ptr<Stmt> Parser::until_stmt() {
    Token until_token = previous();
    bool has_paren = match(TokenType::LeftParen);
    
    auto condition = expr();
    if (!condition) return nullptr;

    if (has_paren) {
        if (consume(TokenType::RightParen, "Expected ')' after until condition.").type == TokenType::Error) {
            return nullptr;
        }
    }
    
    auto body = block();
    if (!body) return nullptr;
    
    return std::make_unique<WhileStmt>(until_token.line, until_token.col, std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::give_stmt() {
    Token give_token = previous();
    std::unique_ptr<Expr> val = nullptr;
    if (!check(TokenType::Semicolon) && !check(TokenType::Newline) && !is_at_end() && !check(TokenType::RightBrace)) {
        val = expr();
        if (!val) return nullptr;
    }
    if (!consume_terminator("Expected ';' or newline after give statement.")) return nullptr;
    return std::make_unique<GiveStmt>(give_token.line, give_token.col, std::move(val));
}

std::unique_ptr<Stmt> Parser::stop_stmt() {
    Token stop_token = previous();
    if (!consume_terminator("Expected ';' or newline after stop statement.")) return nullptr;
    return std::make_unique<StopStmt>(stop_token.line, stop_token.col);
}

std::unique_ptr<Stmt> Parser::skip_stmt() {
    Token skip_token = previous();
    if (!consume_terminator("Expected ';' or newline after skip statement.")) return nullptr;
    return std::make_unique<SkipStmt>(skip_token.line, skip_token.col);
}

std::unique_ptr<BlockStmt> Parser::block() {
    Token brace_token = consume(TokenType::LeftBrace, "Expected '{' before block.");
    if (brace_token.type == TokenType::Error) return nullptr;
    
    skip_newlines();
    
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!check(TokenType::RightBrace) && !is_at_end()) {
        if (auto stmt = statement()) {
            statements.push_back(std::move(stmt));
        } else {
            synchronize();
        }
        skip_newlines();
    }
    
    if (consume(TokenType::RightBrace, "Expected '}' after block.").type == TokenType::Error) return nullptr;
    
    match(TokenType::Newline);
    
    return std::make_unique<BlockStmt>(brace_token.line, brace_token.col, std::move(statements));
}

std::unique_ptr<Expr> Parser::expr() {
    return logical_or();
}

std::unique_ptr<Expr> Parser::logical_or() {
    auto left = logical_and();
    if (!left) return nullptr;
    
    while (match(TokenType::Or)) {
        Token op = previous();
        auto right = logical_and();
        if (!right) return nullptr;
        left = std::make_unique<BinaryExpr>(left->line, left->col, std::move(left), op, std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::logical_and() {
    auto left = equality();
    if (!left) return nullptr;
    
    while (match(TokenType::And)) {
        Token op = previous();
        auto right = equality();
        if (!right) return nullptr;
        left = std::make_unique<BinaryExpr>(left->line, left->col, std::move(left), op, std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::equality() {
    auto left = comparison();
    if (!left) return nullptr;

    while (match(TokenType::Equals) || match(TokenType::Notequals)) {
        Token op = previous();
        auto right = comparison();
        if (!right) return nullptr;
        left = std::make_unique<BinaryExpr>(left->line, left->col, std::move(left), op, std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::comparison() {
    auto left = term();
    if (!left) return nullptr;
    
    while (match(TokenType::Less) || match(TokenType::Greater) || 
           match(TokenType::LessEqual) || match(TokenType::GreaterEqual)) {
        Token op = previous();
        auto right = term();
        if (!right) return nullptr;
        
        left = std::make_unique<BinaryExpr>(left->line, left->col, std::move(left), op, std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expr> Parser::term() {
    auto left = factor();
    if (!left) return nullptr;
    
    while (match(TokenType::Plus) || match(TokenType::Minus)) {
        Token op = previous();
        auto right = factor();
        if (!right) return nullptr;
        
        left = std::make_unique<BinaryExpr>(left->line, left->col, std::move(left), op, std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expr> Parser::factor() {
    auto left = unary();
    if (!left) return nullptr;
    
    while (match(TokenType::Star) || match(TokenType::Slash) || match(TokenType::Percent)) {
        Token op = previous();
        auto right = unary();
        if (!right) return nullptr;
        
        left = std::make_unique<BinaryExpr>(left->line, left->col, std::move(left), op, std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expr> Parser::unary() {
    if (match(TokenType::Minus) || match(TokenType::Not)) {
        Token op = previous();
        auto right = unary();
        if (!right) return nullptr;
        return std::make_unique<UnaryExpr>(op.line, op.col, op, std::move(right));
    }
    
    return primary();
}

std::unique_ptr<Expr> Parser::primary() {
    if (match(TokenType::True)) {
        Token t = previous();
        return std::make_unique<BoolLiteral>(t.line, t.col, true);
    }
    if (match(TokenType::False)) {
        Token t = previous();
        return std::make_unique<BoolLiteral>(t.line, t.col, false);
    }
    if (match(TokenType::Number)) {
        Token t = previous();
        double val = 0.0;
        std::string s(t.lexeme);
        val = std::stod(s);
        bool is_dec = (t.lexeme.find('.') != std::string_view::npos);
        return std::make_unique<NumberLiteral>(t.line, t.col, val, is_dec);
    }
    if (match(TokenType::String)) {
        Token t = previous();
        std::string s(t.lexeme);
        if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
            s = s.substr(1, s.size() - 2);
        }
        return std::make_unique<StringLiteral>(t.line, t.col, std::move(s));
    }
    if (match(TokenType::Char)) {
        Token t = previous();
        char c = (t.lexeme.length() >= 2) ? t.lexeme[1] : '\0';
        return std::make_unique<CharLiteral>(t.line, t.col, c);
    }
    if (match(TokenType::Input)) {
        Token t = previous();
        if (match(TokenType::LeftParen)) {
            consume(TokenType::RightParen, "Expected ')' after 'input'.");
        }
        return std::make_unique<InputExpr>(t.line, t.col);
    }
    if (match(TokenType::Identifier)) {
        Token t = previous();
        return std::make_unique<VariableExpr>(t.line, t.col, std::string(t.lexeme));
    }
    if (match(TokenType::LeftParen)) {
        Token t = previous();
        auto inner = expr();
        if (!inner) return nullptr;
        consume(TokenType::RightParen, "Expected ')' after expression.");
        return std::make_unique<GroupingExpr>(t.line, t.col, std::move(inner));
    }
    
    error(peek(), "Expected expression.");
    return nullptr;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) const {
    if (is_at_end()) return type == TokenType::Eof;
    return peek().type == type;
}

bool Parser::is_at_end() const {
    return current_ >= tokens_.size() || tokens_[current_].type == TokenType::Eof;
}

Token Parser::advance() {
    if (!is_at_end()) current_++;
    return previous();
}

Token Parser::peek() const {
    if (current_ >= tokens_.size()) {
        return tokens_.back();
    }
    return tokens_[current_];
}

Token Parser::previous() const {
    return tokens_[current_ - 1];
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    error(peek(), message);
    return Token{TokenType::Error, "", peek().line, peek().col};
}

void Parser::error(const Token& token, const std::string& message) {
    std::string full_msg = "line " + std::to_string(token.line) + ", col " + std::to_string(token.col) + ": " + message;
    if (token.type == TokenType::Eof) {
        full_msg += " at end";
    } else if (token.type != TokenType::Newline) {
        full_msg += " found '" + std::string(token.lexeme) + "'";
    }
    errors_.push_back({token.line, token.col, full_msg});
}

void Parser::synchronize() {
    advance();
    while (!is_at_end()) {
        if (previous().type == TokenType::Newline || previous().type == TokenType::Semicolon) return;
        
        switch (peek().type) {
            case TokenType::Num:
            case TokenType::Decimal:
            case TokenType::Word:
            case TokenType::Letter:
            case TokenType::Boolean:
            case TokenType::List:
            case TokenType::Fixed:
            case TokenType::Show:
            case TokenType::Check:
            case TokenType::Until:
            case TokenType::Give:
            case TokenType::Stop:
            case TokenType::Skip:
            case TokenType::RightBrace:
                return;
            default:
                break;
        }
        
        advance();
    }
}

void Parser::skip_newlines() {
    while (check(TokenType::Newline)) {
        advance();
    }
}

} // namespace cmm
