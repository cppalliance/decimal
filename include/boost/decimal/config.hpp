// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_CONFIG_HPP
#define BOOST_DECIMAL_CONFIG_HPP

#include <boost/config.hpp>

// This header implements separate compilation features as described in
// http://www.boost.org/more/separate_compilation.html

#if defined(BOOST_ALL_DYN_LINK) || defined(BOOST_DECIMAL_DYN_LINK)
# if defined(BOOST_DECIMAL_SOURCE)
#  define BOOST_DECIMAL_DECL BOOST_SYMBOL_EXPORT
# else
#  define BOOST_DECIMAL_DECL BOOST_SYMBOL_IMPORT
# endif
#else
# define BOOST_DECIMAL_DECL
#endif

// Autolink

#if !defined(BOOST_DECIMAL_SOURCE) && !defined(BOOST_ALL_NO_LIB) && !defined(BOOST_DECIMAL_NO_LIB)

#define BOOST_LIB_NAME boost_decimal

#if defined(BOOST_ALL_DYN_LINK) || defined(BOOST_DECIMAL_DYN_LINK)
# define BOOST_DYN_LINK
#endif

#include <boost/config/auto_link.hpp>

#endif

#endif // BOOST_DECIMAL_CONFIG_HPP
