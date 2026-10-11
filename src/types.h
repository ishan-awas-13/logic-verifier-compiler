#pragma once

#include <string>

namespace cmm {

enum class Type {
    Num,
    Decimal,
    Word,
    Letter,
    Boolean,
    List,
    Void,
    Error
};

inline std::string to_string(Type type) {
    switch (type) {
        case Type::Num: return "num";
        case Type::Decimal: return "decimal";
        case Type::Word: return "word";
        case Type::Letter: return "letter";
        case Type::Boolean: return "boolean";
        case Type::List: return "list";
        case Type::Void: return "void";
        case Type::Error: return "<error>";
        default: return "unknown";
    }
}

inline Type type_from_string(const std::string& name) {
    if (name == "num") return Type::Num;
    if (name == "decimal") return Type::Decimal;
    if (name == "word") return Type::Word;
    if (name == "letter") return Type::Letter;
    if (name == "boolean") return Type::Boolean;
    if (name == "list") return Type::List;
    return Type::Error;
}

inline bool is_numeric(Type type) {
    return type == Type::Num || type == Type::Decimal;
}

// Can source_type be assigned or initialized to target_type?
// Allowed implicit conversion: num to decimal only.
inline bool is_assignable(Type target_type, Type source_type) {
    if (target_type == Type::Error || source_type == Type::Error) return true;
    if (target_type == source_type) return true;
    if (target_type == Type::Decimal && source_type == Type::Num) return true;
    return false;
}

} // namespace cmm

