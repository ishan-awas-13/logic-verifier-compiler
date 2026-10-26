#pragma once

#include <cmath>
#include <limits>
#include <string>
#include <algorithm>
#include <iostream>
#include <cstdint>

namespace cmm {

class Interval {
public:
    double lo = 0.0;
    double hi = 0.0;
    bool is_empty = false;
    bool is_num = true; // true if bounds represent integer/num, false if decimal

    static constexpr double INF = std::numeric_limits<double>::infinity();

    Interval() : lo(0.0), hi(0.0), is_empty(false), is_num(true) {}
    Interval(double l, double h, bool is_num = true, bool empty = false)
        : lo(l), hi(h), is_empty(empty), is_num(is_num) {
        if (!is_empty && lo > hi) {
            is_empty = true;
        }
    }

    static Interval empty(bool is_num = true) {
        Interval iv(0, 0, is_num, true);
        return iv;
    }

    static Interval all(bool is_num = true) {
        return Interval(-INF, INF, is_num, false);
    }

    static Interval point(double val, bool is_num = true) {
        return Interval(val, val, is_num, false);
    }

    bool is_all() const {
        return !is_empty && lo == -INF && hi == INF;
    }

    bool is_singleton() const {
        return !is_empty && lo == hi && !std::isinf(lo);
    }

    bool contains(double val) const {
        if (is_empty) return false;
        return val >= lo && val <= hi;
    }

    bool contains_zero() const {
        return contains(0.0);
    }

    Interval join(const Interval& other) const {
        if (is_empty) return other;
        if (other.is_empty) return *this;
        bool num_flag = is_num && other.is_num;
        return Interval(std::min(lo, other.lo), std::max(hi, other.hi), num_flag, false);
    }

    Interval meet(const Interval& other) const {
        if (is_empty || other.is_empty) return empty(is_num && other.is_num);
        double nlo = std::max(lo, other.lo);
        double nhi = std::min(hi, other.hi);
        if (nlo > nhi) return empty(is_num && other.is_num);
        return Interval(nlo, nhi, is_num && other.is_num, false);
    }

    Interval widen(const Interval& next) const {
        if (is_empty) return next;
        if (next.is_empty) return *this;
        double nlo = (next.lo < lo) ? -INF : lo;
        double nhi = (next.hi > hi) ? INF : hi;
        return Interval(nlo, nhi, is_num && next.is_num, false);
    }

    Interval negate() const {
        if (is_empty) return *this;
        return Interval(-hi, -lo, is_num, false);
    }

    Interval add(const Interval& other) const {
        if (is_empty || other.is_empty) return empty(is_num && other.is_num);
        return Interval(lo + other.lo, hi + other.hi, is_num && other.is_num, false);
    }

    Interval sub(const Interval& other) const {
        if (is_empty || other.is_empty) return empty(is_num && other.is_num);
        return Interval(lo - other.hi, hi - other.lo, is_num && other.is_num, false);
    }

    Interval mul(const Interval& other) const {
        if (is_empty || other.is_empty) return empty(is_num && other.is_num);
        if (is_singleton() && lo == 0.0) return point(0.0, is_num && other.is_num);
        if (other.is_singleton() && other.lo == 0.0) return point(0.0, is_num && other.is_num);

        auto safe_mul = [](double a, double b) -> double {
            if (a == 0.0 || b == 0.0) return 0.0;
            return a * b;
        };

        double p1 = safe_mul(lo, other.lo);
        double p2 = safe_mul(lo, other.hi);
        double p3 = safe_mul(hi, other.lo);
        double p4 = safe_mul(hi, other.hi);
        return Interval(std::min({p1, p2, p3, p4}), std::max({p1, p2, p3, p4}), is_num && other.is_num, false);
    }

    Interval div(const Interval& other) const {
        if (is_empty || other.is_empty) return empty(is_num && other.is_num);
        if (other.contains_zero()) {
            return all(is_num && other.is_num);
        }
        double p1 = lo / other.lo;
        double p2 = lo / other.hi;
        double p3 = hi / other.lo;
        double p4 = hi / other.hi;
        double nlo = std::min({p1, p2, p3, p4});
        double nhi = std::max({p1, p2, p3, p4});
        if (is_num && other.is_num) {
            nlo = std::trunc(nlo);
            nhi = std::trunc(nhi);
        }
        return Interval(nlo, nhi, is_num && other.is_num, false);
    }

    Interval mod(const Interval& other) const {
        if (is_empty || other.is_empty) return empty(is_num && other.is_num);
        if (other.contains_zero()) return all(is_num && other.is_num);
        double max_d = std::max(std::abs(other.lo), std::abs(other.hi));
        return Interval(-(max_d - 1), max_d - 1, true, false);
    }

    bool operator==(const Interval& other) const {
        if (is_empty && other.is_empty) return true;
        if (is_empty != other.is_empty) return false;
        return lo == other.lo && hi == other.hi && is_num == other.is_num;
    }
    bool operator!=(const Interval& other) const { return !(*this == other); }

    std::string to_string() const {
        if (is_empty) return "empty";
        if (is_singleton()) {
            if (is_num) return std::to_string((int64_t)lo);
            std::string s = std::to_string(lo);
            if (s.find('.') == std::string::npos) s += ".0";
            return s;
        }
        std::string s = "[";
        if (lo == -INF) s += "-inf";
        else if (is_num) s += std::to_string((int64_t)lo);
        else s += std::to_string(lo);
        s += ", ";
        if (hi == INF) s += "+inf";
        else if (is_num) s += std::to_string((int64_t)hi);
        else s += std::to_string(hi);
        s += "]";
        return s;
    }
};

} // namespace cmm
