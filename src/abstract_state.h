#pragma once

#include "interval.h"
#include "types.h"
#include <string>
#include <unordered_map>
#include <optional>

namespace cmm {

enum class TriBool {
    False = 0,
    True = 1,
    Unknown = 2
};

inline std::string to_string(TriBool tb) {
    switch (tb) {
        case TriBool::True: return "true";
        case TriBool::False: return "false";
        case TriBool::Unknown: return "unknown";
    }
    return "unknown";
}

inline TriBool tri_not(TriBool tb) {
    if (tb == TriBool::True) return TriBool::False;
    if (tb == TriBool::False) return TriBool::True;
    return TriBool::Unknown;
}

inline TriBool tri_and(TriBool a, TriBool b) {
    if (a == TriBool::False || b == TriBool::False) return TriBool::False;
    if (a == TriBool::True && b == TriBool::True) return TriBool::True;
    return TriBool::Unknown;
}

inline TriBool tri_or(TriBool a, TriBool b) {
    if (a == TriBool::True || b == TriBool::True) return TriBool::True;
    if (a == TriBool::False && b == TriBool::False) return TriBool::False;
    return TriBool::Unknown;
}

struct AbstractValue {
    Type type = Type::Void;
    Interval interval;
    TriBool tri_bool = TriBool::Unknown;
    std::optional<char> letter_val;
    bool is_bottom = false;
    bool from_input = false;

    static AbstractValue bottom() {
        AbstractValue v;
        v.is_bottom = true;
        return v;
    }

    static AbstractValue make_num(int64_t n, bool tainted = false) {
        AbstractValue v;
        v.type = Type::Num;
        v.interval = Interval::point(n, true);
        v.from_input = tainted;
        return v;
    }

    static AbstractValue make_num_interval(Interval iv, bool tainted = false) {
        AbstractValue v;
        v.type = Type::Num;
        v.interval = iv;
        v.from_input = tainted;
        return v;
    }

    static AbstractValue make_decimal(double d, bool tainted = false) {
        AbstractValue v;
        v.type = Type::Decimal;
        v.interval = Interval::point(d, false);
        v.from_input = tainted;
        return v;
    }

    static AbstractValue make_decimal_interval(Interval iv, bool tainted = false) {
        AbstractValue v;
        v.type = Type::Decimal;
        v.interval = iv;
        v.from_input = tainted;
        return v;
    }

    static AbstractValue make_bool(TriBool tb, bool tainted = false) {
        AbstractValue v;
        v.type = Type::Boolean;
        v.tri_bool = tb;
        v.from_input = tainted;
        return v;
    }

    static AbstractValue make_letter(char c, bool tainted = false) {
        AbstractValue v;
        v.type = Type::Letter;
        v.letter_val = c;
        v.from_input = tainted;
        return v;
    }

    static AbstractValue make_unknown(Type t, bool tainted = false) {
        AbstractValue v;
        v.type = t;
        if (t == Type::Num) v.interval = Interval::all(true);
        else if (t == Type::Decimal) v.interval = Interval::all(false);
        else if (t == Type::Boolean) v.tri_bool = TriBool::Unknown;
        else if (t == Type::Letter) v.letter_val = std::nullopt;
        v.from_input = tainted;
        return v;
    }

    AbstractValue join(const AbstractValue& other) const {
        if (is_bottom) return other;
        if (other.is_bottom) return *this;

        AbstractValue res;
        res.type = type;
        res.from_input = from_input || other.from_input;

        if (type == Type::Num || type == Type::Decimal) {
            res.interval = interval.join(other.interval);
        } else if (type == Type::Boolean) {
            if (tri_bool == other.tri_bool) res.tri_bool = tri_bool;
            else res.tri_bool = TriBool::Unknown;
        } else if (type == Type::Letter) {
            if (letter_val == other.letter_val) res.letter_val = letter_val;
            else res.letter_val = std::nullopt;
        }
        return res;
    }

    AbstractValue widen(const AbstractValue& next) const {
        if (is_bottom) return next;
        if (next.is_bottom) return *this;

        AbstractValue res;
        res.type = type;
        res.from_input = from_input || next.from_input;

        if (type == Type::Num || type == Type::Decimal) {
            res.interval = interval.widen(next.interval);
        } else {
            return join(next);
        }
        return res;
    }

    bool operator==(const AbstractValue& other) const {
        if (is_bottom != other.is_bottom) return false;
        if (is_bottom && other.is_bottom) return true;
        if (type != other.type || from_input != other.from_input) return false;
        if (type == Type::Num || type == Type::Decimal) return interval == other.interval;
        if (type == Type::Boolean) return tri_bool == other.tri_bool;
        if (type == Type::Letter) return letter_val == other.letter_val;
        return true;
    }
    bool operator!=(const AbstractValue& other) const { return !(*this == other); }

    std::string to_string() const {
        if (is_bottom) return "bottom";
        std::string s;
        if (type == Type::Num || type == Type::Decimal) {
            s = interval.to_string();
        } else if (type == Type::Boolean) {
            s = cmm::to_string(tri_bool);
        } else if (type == Type::Letter) {
            if (letter_val) s = std::string("'") + *letter_val + "'";
            else s = "unknown";
        } else if (type == Type::Word) {
            s = "unknown";
        } else if (type == Type::List) {
            s = "unknown";
        } else {
            s = "unknown";
        }
        if (from_input) {
            s += " (input)";
        }
        return s;
    }
};

class AbstractState {
public:
    bool is_bottom = false;
    std::unordered_map<std::string, AbstractValue> env;

    static AbstractState bottom() {
        AbstractState st;
        st.is_bottom = true;
        return st;
    }

    AbstractValue get(const std::string& name) const {
        if (is_bottom) return AbstractValue::bottom();
        auto it = env.find(name);
        if (it != env.end()) return it->second;
        return AbstractValue::make_unknown(Type::Void, false);
    }

    void set(const std::string& name, AbstractValue val) {
        if (is_bottom) return;
        env[name] = std::move(val);
    }

    AbstractState join(const AbstractState& other) const {
        if (is_bottom) return other;
        if (other.is_bottom) return *this;

        AbstractState res;
        for (const auto& [k, v] : env) {
            auto it = other.env.find(k);
            if (it != other.env.end()) {
                res.env[k] = v.join(it->second);
            } else {
                res.env[k] = v;
            }
        }
        for (const auto& [k, v] : other.env) {
            if (res.env.find(k) == res.env.end()) {
                res.env[k] = v;
            }
        }
        return res;
    }

    AbstractState widen(const AbstractState& next) const {
        if (is_bottom) return next;
        if (next.is_bottom) return *this;

        AbstractState res;
        for (const auto& [k, v] : env) {
            auto it = next.env.find(k);
            if (it != next.env.end()) {
                res.env[k] = v.widen(it->second);
            } else {
                res.env[k] = v;
            }
        }
        for (const auto& [k, v] : next.env) {
            if (res.env.find(k) == res.env.end()) {
                res.env[k] = v;
            }
        }
        return res;
    }

    bool operator==(const AbstractState& other) const {
        if (is_bottom != other.is_bottom) return false;
        if (is_bottom && other.is_bottom) return true;
        if (env.size() != other.env.size()) return false;
        for (const auto& [k, v] : env) {
            auto it = other.env.find(k);
            if (it == other.env.end() || it->second != v) return false;
        }
        return true;
    }
    bool operator!=(const AbstractState& other) const { return !(*this == other); }
};

} // namespace cmm

