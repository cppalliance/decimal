// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_COMPLEX_HPP
#define BOOST_DECIMAL_COMPLEX_HPP

#include <boost/decimal/decimal32.hpp>
#include <boost/decimal/decimal64.hpp>
#include <boost/decimal/decimal128.hpp>
#include <boost/decimal/cmath.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/type_traits.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <type_traits>
#include <complex>
#include <limits>
#endif

namespace boost {
namespace decimal {

template <typename T>
class complex
{
private:
    T real_;
    T imag_;

public:
    constexpr complex() noexcept = default;
    constexpr explicit complex(T real) noexcept : real_ {real}, imag_ {T{0}} {}
    constexpr complex(T real, T imag) noexcept : real_ {real}, imag_ {imag} {}
    constexpr complex& operator=(const complex& rhs)
    {
        real_ = rhs.real_;
        imag_ = rhs.imag_;
        return *this;
    }

    constexpr T real() const noexcept { return real_; }
    constexpr T imag() const noexcept { return imag_; }

    constexpr complex operator+() const { return *this; }
    constexpr complex operator-() const { return {-real_, -imag_}; }

    friend constexpr complex operator+(const complex& lhs, const complex& rhs) noexcept
    {
        return {lhs.real_ + rhs.real_, lhs.imag_ + rhs.imag_};
    }

    friend constexpr complex operator+(const complex& lhs, const T& rhs) noexcept
    {
        return {lhs.real_ + rhs, lhs.imag_};
    }

    friend constexpr complex operator+(const T& lhs, const complex& rhs) noexcept
    {
        return {lhs + rhs.real_, rhs.imag_};
    }

    friend constexpr complex operator-(const complex& lhs, const complex& rhs) noexcept
    {
        return {lhs.real_ - rhs.real_, lhs.imag_ - rhs.imag_};
    }

    friend constexpr complex operator-(const complex& lhs, const T& rhs) noexcept
    {
        return {lhs.real_ - rhs, lhs.imag_};
    }

    friend constexpr complex operator-(const T& lhs, const complex& rhs) noexcept
    {
        return {lhs - rhs.real_, -rhs.imag_};
    }

    friend constexpr complex operator*(const complex& lhs, const complex& rhs) noexcept
    {
        return {lhs.real_ * rhs.real_ - lhs.imag_ * rhs.imag_, lhs.imag_ * rhs.real_ + lhs.real_ * rhs.imag_};
    }

    friend constexpr complex operator*(const complex& lhs, const T& rhs) noexcept
    {
        return {lhs.real_ * rhs, lhs.imag_ * rhs};
    }

    friend constexpr complex operator*(const T& lhs, const complex& rhs) noexcept
    {
        return {lhs * rhs.real_, lhs * rhs.imag_};
    }

    friend constexpr complex operator/(const complex& lhs, const complex& rhs) noexcept
    {
        const T divisor = rhs.real_ * rhs.real_ + rhs.imag_ * rhs.imag_;
        const T real_part = (lhs.real_ * rhs.real_ + lhs.imag_ * rhs.imag_) / divisor;
        const T imag_part = (lhs.imag_ * rhs.real_ - lhs.real_ * rhs.imag_) / divisor;
        return {real_part, imag_part};
    }

    friend constexpr complex operator/(const complex& lhs, const T& rhs) noexcept
    {
        const T divisor = rhs * rhs;
        const T real_part = (lhs.real_ * rhs) / divisor;
        const T imag_part = (lhs.imag_ * rhs) / divisor;
        return {real_part, imag_part};
    }

    friend constexpr complex operator/(const T& lhs, const complex& rhs) noexcept
    {
        const T divisor = rhs.real_ * rhs.real_ + rhs.imag_ * rhs.imag_;
        const T real_part = (lhs * rhs.real_) / divisor;
        const T imag_part = -(lhs.real_ * rhs.imag_) / divisor;
        return {real_part, imag_part};
    }

    constexpr complex& operator+=(const complex& rhs) noexcept
    {
        *this = *this + rhs;
        return *this;
    }

    constexpr complex& operator+=(const T& rhs) noexcept
    {
        *this = *this + rhs;
        return *this;
    }

    constexpr complex& operator-=(const complex& rhs) noexcept
    {
        *this = *this - rhs;
        return *this;
    }

    constexpr complex& operator-=(const T& rhs) noexcept
    {
        *this = *this - rhs;
        return *this;
    }

    constexpr complex& operator*=(const complex& rhs) noexcept
    {
        *this = *this * rhs;
        return *this;
    }

    constexpr complex& operator*=(const T& rhs) noexcept
    {
        *this = *this * rhs;
        return *this;
    }

    constexpr complex& operator/=(const complex& rhs) noexcept
    {
        *this = *this / rhs;
        return *this;
    }

    constexpr complex& operator/=(const T& rhs) noexcept
    {
        *this = *this / rhs;
        return *this;
    }

    constexpr bool operator==(const complex& rhs) const noexcept
    {
        return real_ == rhs.real_ && imag_ == rhs.imag_;
    }

    constexpr bool operator!=(const complex& rhs) const noexcept
    {
        return !(*this == rhs);
    }

    constexpr bool operator==(const T& rhs) const noexcept
    {
        return real_ == rhs && imag_ == T{0};
    }

    constexpr bool operator!=(const T& rhs) const noexcept
    {
        return !(*this == rhs);
    }

    template <typename CharT, typename Traits>
    friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const complex& z)
    {
        std::basic_ostringstream<CharT, Traits> s;
        s.flags(os.flags());
        s.imbue(os.getloc());
        s.precision(os.precision());
        s << '(' << z.real_ << ',' << z.imag_ << ')';

        return os << s.str();
    }

    // Supported formats:
    // 1) real
    // 2) (real)
    // 3) (real, imag)
    template <typename CharT, typename Traits>
    friend std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, complex& z)
    {
        CharT ch {};
        T real = T{0};
        T imag = T{0};

        is >> std::ws;
        is.get(ch);

        if (ch == '(')
        {
            // Expecting a format like 2 or 3
            is >> std::ws >> real;
            is.get(ch);
            if (ch == ',')
            {
                // A comma indicates it's 3
                is >> std::ws >> imag;
                is.get(ch); // Should be ')'
            }
            else if (ch != ')')
            {
                // Syntax error: unexpected character
                is.setstate(std::ios_base::failbit);
                return is;
            }
        }
        else
        {
            // No parentheses, just a real number from format 1
            is.putback(ch);
            is >> real;
        }

        if (!is.fail())
        {
            z.real_ = real;
            z.imag_ = imag;
        }

        return is;
    }
};

// Polar is specialized for each type since libc++ uses just template<typename T> which leads
// to a compiler error

constexpr complex<decimal32> polar(const decimal32& rho, const decimal32& theta) noexcept
{
    return {rho * cos(theta), rho * sin(theta)};
}

constexpr complex<decimal64> polar(const decimal64& rho, const decimal64& theta) noexcept
{
    return {rho * cos(theta), rho * sin(theta)};
}

constexpr complex<decimal128> polar(const decimal128& rho, const decimal128& theta) noexcept
{
    return {rho * cos(theta), rho * sin(theta)};
}

template <typename T>
constexpr T real(const complex<T>& z) noexcept
{
    return z.real();
}

template <typename T>
constexpr T imag(const complex<T>& z) noexcept
{
    return z.imag();
}

template <typename T>
constexpr T abs(const complex<T>& z) noexcept
{
    return hypot(z.real(), z.imag());
}

template <typename T>
constexpr T arg(const complex<T>& z) noexcept
{
    return atan2(z.imag(), z.real());
}

template <typename T>
constexpr T norm(const complex<T>& z) noexcept
{
    return z.real() * z.real() + z.imag() * z.imag();
}

template <typename T>
constexpr complex<T> conj(const complex<T>& z) noexcept
{
    return {z.real(), -z.imag()};
}

template <typename T>
constexpr complex<T> proj(const complex<T>& z) noexcept
{
    if (isinf(z.real()) || isinf(z.imag()))
    {
        return {std::numeric_limits<T>::infinity(), copysign(T{0}, z.imag())};
    }

    return z;
}

template <typename T>
constexpr complex<T> exp(const complex<T>& z) noexcept
{
    return polar(exp(z.real()), z.imag());
}

template <typename T>
constexpr complex<T> log(const complex<T>& z) noexcept
{
    return {log(abs(z)), arg(z)};
}

template <typename T>
constexpr complex<T> log10(const complex<T>& z) noexcept
{
    return log(z) / log(T{10});
}

template <typename T>
constexpr complex<T> pow(const complex<T>& x, const complex<T>& y) noexcept
{
    return exp(y * log(x));
}

template <typename T>
constexpr complex<T> pow(const T& x, const complex<T>& y) noexcept
{
    const complex<T> new_x {x};
    return exp(y * log(new_x));
}

template <typename T>
constexpr complex<T> pow(const complex<T>& x, const T& y) noexcept
{
    const complex<T> new_y {y};
    return exp(new_y * log(x));
}

template <typename T>
constexpr complex<T> sqrt(const complex<T>& z) noexcept
{
    return polar(sqrt(abs(z)), arg(z) / 2);
}

template <typename T>
constexpr complex<T> sinh(const complex<T>& z) noexcept
{
    return {sinh(z.real()) * cos(z.imag()), cosh(z.real()) * sin(z.imag())};
}

template <typename T>
constexpr complex<T> cosh(const complex<T>& z) noexcept
{
    return {cosh(z.real()) * cos(z.imag()), sinh(z.real()) * sin(z.imag())};
}

template <typename T>
constexpr complex<T> tanh(const complex<T>& z) noexcept
{
    return {sinh(z) / cosh(z)};
}

} //namespace decimal
} //namespace boost

namespace std {

template <>
class complex<boost::decimal::decimal32> : public boost::decimal::complex<boost::decimal::decimal32>
{
public:
    using boost::decimal::complex<boost::decimal::decimal32>::complex;
};

template <>
class complex<boost::decimal::decimal64> : public boost::decimal::complex<boost::decimal::decimal64>
{
public:
    using boost::decimal::complex<boost::decimal::decimal64>::complex;
};

template <>
class complex<boost::decimal::decimal128> : public boost::decimal::complex<boost::decimal::decimal128>
{
    using boost::decimal::complex<boost::decimal::decimal128>::complex;
};

} // namespace std

#endif // BOOST_DECIMAL_COMPLEX_HPP
