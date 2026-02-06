#pragma once

#include <numeric>
#include <cstdlib>
#include <cstdint>
#include <iostream>

class Rational {
public:
    Rational() : numerator_(0), denominator_(1) {}

    Rational(int numerator, int denominator)
        : numerator_(numerator)
        , denominator_(denominator)
    {
        if (denominator_ == 0) {
            std::abort();
        }

        Reduction();
    }
    Rational(int value) : numerator_(value), denominator_(1) {}

    int GetNumerator() const {
        return numerator_;
    }

    int GetDenominator() const {
        return denominator_;
    }

    Rational Inv() const {
        int n = GetDenominator();
        int d = GetNumerator();
        if (GetNumerator() < 0) {
            n *= -1;
            d *= -1;
        }
        return {n, d};
    }

    Rational& operator=(int value) {
        *this = Rational(value);
        return *this;
    }

    friend Rational operator+(const Rational& r1, const Rational& r2);
    friend Rational operator-(const Rational& r1, const Rational& r2);
    friend Rational operator*(const Rational& r1, const Rational& r2);
    friend Rational operator/(const Rational& r1, const Rational& r2);

    Rational& operator+=(const Rational& rr) {
        *this = *this + rr;
        Reduction();
        return *this;
    }

    Rational& operator-=(const Rational& rr) {
        *this = *this - rr;
        Reduction();
        return *this;
    }

    Rational& operator*=(const Rational& rr) {
         *this = *this * rr;
        Reduction();
        return *this;
    }

    Rational& operator/=(const Rational& rr) {
        *this = *this / rr;
        Reduction();
        return *this;
    }

    Rational operator+() const {
        return *this;
    }
    Rational operator-() const {
        return { -numerator_, denominator_ };
    }
    friend std::istream& operator>>(std::istream& is, Rational& r);
    friend std::ostream& operator<<(std::ostream& os, const Rational& r);

private:

    void Reduction() {
        if (denominator_ < 0) {
            numerator_ = -numerator_;
            denominator_ = -denominator_;
        }
        const int divisor = std::gcd(numerator_, denominator_);
        numerator_ /= divisor;
        denominator_ /= divisor;
    }

private:
    int numerator_ = 0;
    int denominator_ = 1;
};

inline Rational operator+(const Rational& r1, const Rational& r2) {
    if (r1.GetDenominator() == r2.GetDenominator()) {
        return { r1.GetNumerator() + r2.GetNumerator(), r1.GetDenominator() };
    }
    return { (r1.GetNumerator() * r2.GetDenominator() +
             r2.GetNumerator() * r1.GetDenominator()),
            r1.GetDenominator() * r2.GetDenominator() };
}
inline Rational operator-(const Rational& r1, const Rational& r2) {
    if (r1.GetDenominator() == r2.GetDenominator()) {
        return { r1.GetNumerator() - r2.GetNumerator(), r1.GetDenominator() };
    }
    return { (r1.GetNumerator() * r2.GetDenominator() -
             r2.GetNumerator() * r1.GetDenominator()),
            r1.GetDenominator() * r2.GetDenominator() };
}
inline Rational operator*(const Rational& r1, const Rational& r2) {
    return {r1.GetNumerator() * r2.GetNumerator(),
            r1.GetDenominator() * r2.GetDenominator() };
}
inline Rational operator/(const Rational& r1, const Rational& r2) {
    return { r1.GetNumerator() * r2.GetDenominator(),
            r1.GetDenominator() * r2.GetNumerator() };
}

inline auto operator<=>(const Rational& lhs, const Rational& rhs) {
    auto l = static_cast<std::int64_t>(lhs.GetNumerator()) * rhs.GetDenominator();
    auto r = static_cast<std::int64_t>(rhs.GetNumerator()) * lhs.GetDenominator();
    return l <=> r;
}

inline auto operator==(const Rational& lhs, const Rational& rhs) {
    return static_cast<std::int64_t>(lhs.GetNumerator()) * rhs.GetDenominator() ==
           static_cast<std::int64_t>(lhs.GetDenominator()) * rhs.GetNumerator();
}

inline std::ostream& operator<<(std::ostream& os, const Rational& r) {
    using namespace std::literals;
    if (r.denominator_ == 1) {
        os << r.numerator_;
    }
    else {
        os << r.numerator_ << " / "s << r.denominator_;
    }
    return os;
}

inline std::istream& operator>>(std::istream& is, Rational& r) {
    int n, d;
    char div;

    if (!(is >> n)) {
        return is;
    }

    if (!(is >> std::ws >> div)) {
        r = Rational(n, 1);
        is.clear();
        return is;
    }

    if (div != '/') {
        r = Rational(n, 1);
        is.unget();
        return is;
    }

    if (!(is >> d) || (d == 0)) {
        is.setstate(std::ios::failbit);
        return is;
    }

    r = Rational(n, d);

    return is;
}
