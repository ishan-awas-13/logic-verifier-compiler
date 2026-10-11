#pragma once
#include <string_view>
#include <string>

namespace cmm {

enum class TokenType {
    // Data Types
    Num,
    Decimal,
    Word,
    Letter,
    Boolean,
    List,

    // Modifiers
    Fixed,

    // Control Flow
    Check,
    Otherwise,
    Until,
    Foreach,
    DoUntil,
    Stop,
    Skip,

    // IO / Functions
    Show,
    Input,
    Give,

    // Boolean Literals
    True,
    False,

    // Identifiers and Literals
    Identifier,
    Number,
    String,
    Char,

    // Operators
    Plus,
    Minus,
    Star,
    Slash,
    Percent,
    Equal,
    Equals,       // 'equals' or '=='
    Notequals,    // 'notequals' or '!='
    Greater,
    Less,
    GreaterEqual, // '>='
    LessEqual,    // '<='
    And,          // 'and'
    Or,           // 'or'
    Not,          // 'not'
    PlusPlus,     // '++'
    MinusMinus,   // '--'

    // Punctuation
    LeftBrace,
    RightBrace,
    LeftParen,
    RightParen,
    LeftBracket,
    RightBracket,
    Comma,
    Semicolon,

    // Special
    Newline,
    Error,
    Eof
};

struct Token {
    TokenType type;
    std::string_view lexeme;
    int line;
    int col;
};

std::string to_string(TokenType type);

} // namespace cmm
