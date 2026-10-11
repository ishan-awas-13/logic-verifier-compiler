#include "lexer.h"
#include <unordered_map>
#include <cctype>

namespace cmm {

std::string to_string(TokenType type) {
    switch (type) {
        case TokenType::Num: return "Num";
        case TokenType::Decimal: return "Decimal";
        case TokenType::Word: return "Word";
        case TokenType::Letter: return "Letter";
        case TokenType::Boolean: return "Boolean";
        case TokenType::List: return "List";
        case TokenType::Fixed: return "Fixed";
        case TokenType::Check: return "Check";
        case TokenType::Otherwise: return "Otherwise";
        case TokenType::Until: return "Until";
        case TokenType::Foreach: return "Foreach";
        case TokenType::DoUntil: return "DoUntil";
        case TokenType::Stop: return "Stop";
        case TokenType::Skip: return "Skip";
        case TokenType::Show: return "Show";
        case TokenType::Input: return "Input";
        case TokenType::Give: return "Give";
        case TokenType::True: return "True";
        case TokenType::False: return "False";
        case TokenType::Identifier: return "Identifier";
        case TokenType::Number: return "Number";
        case TokenType::String: return "String";
        case TokenType::Char: return "Char";
        case TokenType::Plus: return "Plus";
        case TokenType::Minus: return "Minus";
        case TokenType::Star: return "Star";
        case TokenType::Slash: return "Slash";
        case TokenType::Percent: return "Percent";
        case TokenType::Equal: return "Equal";
        case TokenType::Equals: return "Equals";
        case TokenType::Notequals: return "Notequals";
        case TokenType::Greater: return "Greater";
        case TokenType::Less: return "Less";
        case TokenType::GreaterEqual: return "GreaterEqual";
        case TokenType::LessEqual: return "LessEqual";
        case TokenType::And: return "And";
        case TokenType::Or: return "Or";
        case TokenType::Not: return "Not";
        case TokenType::PlusPlus: return "PlusPlus";
        case TokenType::MinusMinus: return "MinusMinus";
        case TokenType::LeftBrace: return "LeftBrace";
        case TokenType::RightBrace: return "RightBrace";
        case TokenType::LeftParen: return "LeftParen";
        case TokenType::RightParen: return "RightParen";
        case TokenType::LeftBracket: return "LeftBracket";
        case TokenType::RightBracket: return "RightBracket";
        case TokenType::Comma: return "Comma";
        case TokenType::Semicolon: return "Semicolon";
        case TokenType::Newline: return "Newline";
        case TokenType::Error: return "Error";
        case TokenType::Eof: return "Eof";
        default: return "Unknown";
    }
}

static const std::unordered_map<std::string_view, TokenType> keywords = {
    {"num", TokenType::Num},
    {"decimal", TokenType::Decimal},
    {"word", TokenType::Word},
    {"letter", TokenType::Letter},
    {"boolean", TokenType::Boolean},
    {"list", TokenType::List},
    {"fixed", TokenType::Fixed},
    {"check", TokenType::Check},
    {"otherwise", TokenType::Otherwise},
    {"until", TokenType::Until},
    {"foreach", TokenType::Foreach},
    {"do_until", TokenType::DoUntil},
    {"stop", TokenType::Stop},
    {"skip", TokenType::Skip},
    {"show", TokenType::Show},
    {"input", TokenType::Input},
    {"give", TokenType::Give},
    {"true", TokenType::True},
    {"false", TokenType::False},
    {"equals", TokenType::Equals},
    {"notequals", TokenType::Notequals},
    {"and", TokenType::And},
    {"or", TokenType::Or},
    {"not", TokenType::Not}
};

Lexer::Lexer(std::string_view source) : source_(source) {}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (current_ < source_.length()) {
        skip_whitespace_and_comments();
        if (current_ >= source_.length()) break;
        
        start_ = current_;
        col_start_ = col_current_;
        
        char c = advance();
        
        if (c == '\n') {
            if (!tokens.empty() && tokens.back().type != TokenType::Newline) {
                tokens.push_back(make_token(TokenType::Newline));
            }
            line_++;
            col_current_ = 1;
            continue;
        }

        if (std::isalpha(c) || c == '_') {
            tokens.push_back(identifier());
            continue;
        }
        if (std::isdigit(c)) {
            tokens.push_back(number());
            continue;
        }
        
        switch (c) {
            case '{': tokens.push_back(make_token(TokenType::LeftBrace)); break;
            case '}': tokens.push_back(make_token(TokenType::RightBrace)); break;
            case '(': tokens.push_back(make_token(TokenType::LeftParen)); break;
            case ')': tokens.push_back(make_token(TokenType::RightParen)); break;
            case '[': tokens.push_back(make_token(TokenType::LeftBracket)); break;
            case ']': tokens.push_back(make_token(TokenType::RightBracket)); break;
            case ',': tokens.push_back(make_token(TokenType::Comma)); break;
            case ';': tokens.push_back(make_token(TokenType::Semicolon)); break;
            case '%': tokens.push_back(make_token(TokenType::Percent)); break;
            case '+': 
                tokens.push_back(make_token(match('+') ? TokenType::PlusPlus : TokenType::Plus));
                break;
            case '-': 
                tokens.push_back(make_token(match('-') ? TokenType::MinusMinus : TokenType::Minus));
                break;
            case '*': tokens.push_back(make_token(TokenType::Star)); break;
            case '/': tokens.push_back(make_token(TokenType::Slash)); break;
            case '<': 
                tokens.push_back(make_token(match('=') ? TokenType::LessEqual : TokenType::Less));
                break;
            case '>': 
                tokens.push_back(make_token(match('=') ? TokenType::GreaterEqual : TokenType::Greater));
                break;
            case '=': 
                tokens.push_back(make_token(match('=') ? TokenType::Equals : TokenType::Equal));
                break;
            case '!':
                if (match('=')) {
                    tokens.push_back(make_token(TokenType::Notequals));
                } else {
                    tokens.push_back(error_token("Unexpected character."));
                }
                break;
            case '"':
                tokens.push_back(string());
                break;
            case '\'':
                tokens.push_back(character());
                break;
            default:
                tokens.push_back(error_token("Unexpected character."));
                break;
        }
    }
    
    start_ = current_;
    col_start_ = col_current_;
    tokens.push_back(make_token(TokenType::Eof));
    return tokens;
}

char Lexer::advance() {
    char c = source_[current_++];
    col_current_++;
    return c;
}

char Lexer::peek() const {
    if (current_ >= source_.length()) return '\0';
    return source_[current_];
}

char Lexer::peek_next() const {
    if (current_ + 1 >= source_.length()) return '\0';
    return source_[current_ + 1];
}

bool Lexer::match(char expected) {
    if (peek() != expected) return false;
    current_++;
    col_current_++;
    return true;
}

void Lexer::skip_whitespace_and_comments() {
    while (true) {
        char c = peek();
        if (c == ' ' || c == '\r' || c == '\t') {
            advance();
        } else if (c == '#') {
            while (peek() != '\n' && peek() != '\0') advance();
        } else if (c == '$') {
            advance(); // consume opening $
            while (peek() != '$' && peek() != '\0') {
                if (peek() == '\n') {
                    line_++;
                    col_current_ = 0;
                }
                advance();
            }
            if (peek() == '$') advance(); // consume closing $
        } else {
            break;
        }
    }
}

Token Lexer::number() {
    while (std::isdigit(peek())) advance();

    if (peek() == '.' && std::isdigit(peek_next())) {
        advance(); // consume '.'
        while (std::isdigit(peek())) advance();
    }

    return make_token(TokenType::Number);
}

Token Lexer::string() {
    while (peek() != '"' && peek() != '\0') {
        if (peek() == '\n') {
            line_++;
            col_current_ = 0;
        }
        if (peek() == '\\' && peek_next() != '\0') {
            advance();
        }
        advance();
    }

    if (peek() == '\0') {
        return error_token("Unterminated string.");
    }

    advance(); // the closing quote
    return make_token(TokenType::String);
}

Token Lexer::character() {
    if (peek() == '\0' || peek() == '\n') {
        return error_token("Unterminated character literal.");
    }
    if (peek() == '\\') {
        advance();
    }
    advance();
    if (peek() != '\'') {
        return error_token("Unterminated character literal.");
    }
    advance(); // closing '
    return make_token(TokenType::Char);
}

Token Lexer::identifier() {
    while (std::isalnum(peek()) || peek() == '_') advance();

    std::string_view text = source_.substr(start_, current_ - start_);
    auto it = keywords.find(text);
    TokenType type = (it != keywords.end()) ? it->second : TokenType::Identifier;

    return make_token(type);
}

Token Lexer::make_token(TokenType type) {
    return Token{
        type,
        source_.substr(start_, current_ - start_),
        line_,
        col_start_
    };
}

Token Lexer::error_token(const char* message) {
    return Token{
        TokenType::Error,
        std::string_view(message),
        line_,
        col_start_
    };
}

} // namespace cmm
