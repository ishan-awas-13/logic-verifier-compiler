#pragma once

#include "types.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <limits>
#include <optional>
#include <cmath>

namespace cmm {

struct RuntimeValue {
    Type type = Type::Void;
    int64_t num_val = 0;
    double dec_val = 0.0;
    bool bool_val = false;
    char letter_val = '\0';
    std::string word_val;

    static RuntimeValue make_num(int64_t v) {
        RuntimeValue val;
        val.type = Type::Num;
        val.num_val = v;
        return val;
    }

    static RuntimeValue make_decimal(double v) {
        RuntimeValue val;
        val.type = Type::Decimal;
        val.dec_val = v;
        return val;
    }

    static RuntimeValue make_bool(bool v) {
        RuntimeValue val;
        val.type = Type::Boolean;
        val.bool_val = v;
        return val;
    }

    static RuntimeValue make_letter(char v) {
        RuntimeValue val;
        val.type = Type::Letter;
        val.letter_val = v;
        return val;
    }

    static RuntimeValue make_word(const std::string& v) {
        RuntimeValue val;
        val.type = Type::Word;
        val.word_val = v;
        return val;
    }
};

inline std::string formatValue(const RuntimeValue& val) {
    switch (val.type) {
        case Type::Num:
            return std::to_string(val.num_val);
        case Type::Decimal: {
            char buf[64];
            snprintf(buf, sizeof(buf), "%.6g", val.dec_val);
            std::string s = buf;
            return s;
        }
        case Type::Boolean:
            return val.bool_val ? "true" : "false";
        case Type::Letter:
            return std::string(1, val.letter_val);
        case Type::Word:
            return val.word_val;
        default:
            return "";
    }
}

// Helpers for overflow-safe 64-bit integer arithmetic
inline bool safe_add_num(int64_t a, int64_t b, int64_t& res) {
    #if defined(__GNUC__) || defined(__clang__)
    return !__builtin_add_overflow(a, b, &res);
    #else
    if ((b > 0 && a > std::numeric_limits<int64_t>::max() - b) ||
        (b < 0 && a < std::numeric_limits<int64_t>::min() - b)) {
        return false;
    }
    res = a + b;
    return true;
    #endif
}

inline bool safe_sub_num(int64_t a, int64_t b, int64_t& res) {
    #if defined(__GNUC__) || defined(__clang__)
    return !__builtin_sub_overflow(a, b, &res);
    #else
    if ((b < 0 && a > std::numeric_limits<int64_t>::max() + b) ||
        (b > 0 && a < std::numeric_limits<int64_t>::min() + b)) {
        return false;
    }
    res = a - b;
    return true;
    #endif
}

inline bool safe_mul_num(int64_t a, int64_t b, int64_t& res) {
    #if defined(__GNUC__) || defined(__clang__)
    return !__builtin_mul_overflow(a, b, &res);
    #else
    if (a > 0) {
        if (b > 0) {
            if (a > std::numeric_limits<int64_t>::max() / b) return false;
        } else {
            if (b < std::numeric_limits<int64_t>::min() / a) return false;
        }
    } else {
        if (b > 0) {
            if (a < std::numeric_limits<int64_t>::min() / b) return false;
        } else {
            if (a != 0 && b < std::numeric_limits<int64_t>::max() / a) return false;
        }
    }
    res = a * b;
    return true;
    #endif
}

inline bool safe_div_num(int64_t a, int64_t b, int64_t& res) {
    if (b == 0) return false;
    if (a == std::numeric_limits<int64_t>::min() && b == -1) return false;
    res = a / b;
    return true;
}

inline bool safe_mod_num(int64_t a, int64_t b, int64_t& res) {
    if (b == 0) return false;
    if (a == std::numeric_limits<int64_t>::min() && b == -1) return false;
    res = a % b;
    return true;
}

} // namespace cmm

