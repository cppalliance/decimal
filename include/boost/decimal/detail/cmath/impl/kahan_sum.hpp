//  (C) Copyright John Maddock 2005-2006.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_DECIMAL_DETAIL_CMATH_IMPL_KAHAN_SUM
#define BOOST_DECIMAL_DETAIL_CMATH_IMPL_KAHAN_SUM

namespace boost {
namespace decimal {
namespace tools {


//
// Algorithm kahan_sum_series invokes Functor func until the N'th
// term is too small to have any effect on the total, the terms
// are added using the Kahan summation method.
//
// CAUTION: Optimizing compilers combined with extended-precision
// machine registers conspire to render this algorithm partly broken:
// double rounding of intermediate terms (first to a long double machine
// register, and then to a double result) cause the rounding error computed
// by the algorithm to be off by up to 1ulp.  However, this occurs rarely, and
// in any case the result is still much better than a naive summation.
//
template <typename Functor>
constexpr typename Functor::result_type kahan_sum_series(Functor& func, int bits) noexcept
{
    using result_type = typename Functor::result_type;

    result_type factor = pow(result_type(2), result_type(bits));
    result_type result = func();
    result_type next_term {};
    result_type y {};
    result_type t {};
    result_type carry = 0;

    do
    {
        next_term = func();
        y = next_term - carry;
        t = result + y;
        carry = t - result;
        carry -= y;
        result = t;
    } while (fabs(result) < fabs(factor * next_term));

    return result;
}

} //namespace tools
} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_IMPL_KAHAN_SUM
