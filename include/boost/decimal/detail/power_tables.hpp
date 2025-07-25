// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_POWER_TABLES_HPP
#define BOOST_DECIMAL_DETAIL_POWER_TABLES_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include "int128.hpp"
#include <boost/decimal/detail/u256.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <cstdint>
#endif

namespace boost {
namespace decimal {
namespace detail {

namespace impl {

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t powers_of_10_u32[10] =
{
    UINT32_C(1), UINT32_C(10), UINT32_C(100), UINT32_C(1000), UINT32_C(10000), UINT32_C(100000), UINT32_C(1000000),
    UINT32_C(10000000), UINT32_C(100000000), UINT32_C(1000000000),
};

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t powers_of_10[20] =
{
    UINT64_C(1), UINT64_C(10), UINT64_C(100), UINT64_C(1000), UINT64_C(10000), UINT64_C(100000), UINT64_C(1000000),
    UINT64_C(10000000), UINT64_C(100000000), UINT64_C(1000000000), UINT64_C(10000000000), UINT64_C(100000000000),
    UINT64_C(1000000000000), UINT64_C(10000000000000), UINT64_C(100000000000000), UINT64_C(1000000000000000),
    UINT64_C(10000000000000000), UINT64_C(100000000000000000), UINT64_C(1000000000000000000),
    UINT64_C(10000000000000000000)
};

BOOST_DECIMAL_CONSTEXPR_VARIABLE boost::int128::uint128_t BOOST_DECIMAL_DETAIL_INT128_pow10[] =
{
    boost::int128::uint128_t {UINT64_C(0), UINT64_C(1)},
    boost::int128::uint128_t {UINT64_C(0), UINT64_C(10)},
    boost::int128::uint128_t {UINT64_C(0), UINT64_C(100)},
    boost::int128::uint128_t {UINT64_C(0), UINT64_C(1000)},
    boost::int128::uint128_t {UINT64_C(0), UINT64_C(10000)},
    boost::int128::uint128_t {UINT64_C(0), UINT64_C(100000)},
    boost::int128::uint128_t {UINT64_C(0), UINT64_C(1000000)},
    boost::int128::uint128_t {UINT64_C(0), UINT64_C(10000000)},
    boost::int128::uint128_t {UINT64_C(0), UINT64_C(100000000)},
    boost::int128::uint128_t {UINT64_C(0), UINT64_C(1000000000)},
    boost::int128::uint128_t {UINT64_C(0), UINT64_C(10000000000)},
    boost::int128::uint128_t {UINT64_C(0), UINT64_C(100000000000)},
    boost::int128::uint128_t {UINT64_C(0), UINT64_C(1000000000000)},
    boost::int128::uint128_t {UINT64_C(0), UINT64_C(10000000000000)},
    boost::int128::uint128_t {UINT64_C(0), UINT64_C(100000000000000)},
    boost::int128::uint128_t {UINT64_C(0), UINT64_C(1000000000000000)},
    boost::int128::uint128_t {UINT64_C(0), UINT64_C(10000000000000000)},
    boost::int128::uint128_t {UINT64_C(0), UINT64_C(100000000000000000)},
    boost::int128::uint128_t {UINT64_C(0), UINT64_C(1000000000000000000)},
    boost::int128::uint128_t {UINT64_C(0), UINT64_C(10000000000000000000)},
    boost::int128::uint128_t {UINT64_C(5), UINT64_C(7766279631452241920)},
    boost::int128::uint128_t {UINT64_C(54), UINT64_C(3875820019684212736)},
    boost::int128::uint128_t {UINT64_C(542), UINT64_C(1864712049423024128)},
    boost::int128::uint128_t {UINT64_C(5421), UINT64_C(200376420520689664)},
    boost::int128::uint128_t {UINT64_C(54210), UINT64_C(2003764205206896640)},
    boost::int128::uint128_t {UINT64_C(542101), UINT64_C(1590897978359414784)},
    boost::int128::uint128_t {UINT64_C(5421010), UINT64_C(15908979783594147840)},
    boost::int128::uint128_t {UINT64_C(54210108), UINT64_C(11515845246265065472)},
    boost::int128::uint128_t {UINT64_C(542101086), UINT64_C(4477988020393345024)},
    boost::int128::uint128_t {UINT64_C(5421010862), UINT64_C(7886392056514347008)},
    boost::int128::uint128_t {UINT64_C(54210108624), UINT64_C(5076944270305263616)},
    boost::int128::uint128_t {UINT64_C(542101086242), UINT64_C(13875954555633532928)},
    boost::int128::uint128_t {UINT64_C(5421010862427), UINT64_C(9632337040368467968)},
    boost::int128::uint128_t {UINT64_C(54210108624275), UINT64_C(4089650035136921600)},
    boost::int128::uint128_t {UINT64_C(542101086242752), UINT64_C(4003012203950112768)},
    boost::int128::uint128_t {UINT64_C(5421010862427522), UINT64_C(3136633892082024448)},
    boost::int128::uint128_t {UINT64_C(54210108624275221), UINT64_C(12919594847110692864)},
    boost::int128::uint128_t {UINT64_C(542101086242752217), UINT64_C(68739955140067328)},
    boost::int128::uint128_t {UINT64_C(5421010862427522170), UINT64_C(687399551400673280)},
    boost::int128::uint128_t {UINT64_C(17316620476856118468), UINT64_C(6873995514006732800)},
};

#ifdef BOOST_DECIMAL_HAS_INT128

BOOST_DECIMAL_CONSTEXPR_VARIABLE detail::builtin_uint128_t builtin_128_pow10[] = {
    detail::builtin_uint128_t(1),
    detail::builtin_uint128_t(10),
    detail::builtin_uint128_t(100),
    detail::builtin_uint128_t(1000),
    detail::builtin_uint128_t(10000),
    detail::builtin_uint128_t(100000),
    detail::builtin_uint128_t(1000000),
    detail::builtin_uint128_t(10000000),
    detail::builtin_uint128_t(100000000),
    detail::builtin_uint128_t(1000000000),
    detail::builtin_uint128_t(10000000000),
    detail::builtin_uint128_t(100000000000),
    detail::builtin_uint128_t(1000000000000),
    detail::builtin_uint128_t(10000000000000),
    detail::builtin_uint128_t(100000000000000),
    detail::builtin_uint128_t(1000000000000000),
    detail::builtin_uint128_t(10000000000000000),
    detail::builtin_uint128_t(100000000000000000),
    detail::builtin_uint128_t(1000000000000000000),
    detail::builtin_uint128_t(10000000000000000000ULL),
    detail::builtin_uint128_t(10000000000000000000ULL) * detail::builtin_uint128_t(10),
    detail::builtin_uint128_t(10000000000000000000ULL) * detail::builtin_uint128_t(100),
    detail::builtin_uint128_t(10000000000000000000ULL) * detail::builtin_uint128_t(1000),
    detail::builtin_uint128_t(10000000000000000000ULL) * detail::builtin_uint128_t(10000),
    detail::builtin_uint128_t(10000000000000000000ULL) * detail::builtin_uint128_t(100000),
    detail::builtin_uint128_t(10000000000000000000ULL) * detail::builtin_uint128_t(1000000),
    detail::builtin_uint128_t(10000000000000000000ULL) * detail::builtin_uint128_t(10000000),
    detail::builtin_uint128_t(10000000000000000000ULL) * detail::builtin_uint128_t(100000000),
    detail::builtin_uint128_t(10000000000000000000ULL) * detail::builtin_uint128_t(1000000000),
    detail::builtin_uint128_t(10000000000000000000ULL) * detail::builtin_uint128_t(10000000000),
    detail::builtin_uint128_t(10000000000000000000ULL) * detail::builtin_uint128_t(100000000000),
    detail::builtin_uint128_t(10000000000000000000ULL) * detail::builtin_uint128_t(1000000000000),
    detail::builtin_uint128_t(10000000000000000000ULL) * detail::builtin_uint128_t(10000000000000),
    detail::builtin_uint128_t(10000000000000000000ULL) * detail::builtin_uint128_t(100000000000000),
    detail::builtin_uint128_t(10000000000000000000ULL) * detail::builtin_uint128_t(1000000000000000),
    detail::builtin_uint128_t(10000000000000000000ULL) * detail::builtin_uint128_t(10000000000000000),
    detail::builtin_uint128_t(10000000000000000000ULL) * detail::builtin_uint128_t(100000000000000000),
    detail::builtin_uint128_t(10000000000000000000ULL) * detail::builtin_uint128_t(1000000000000000000),
    detail::builtin_uint128_t(10000000000000000000ULL) * detail::builtin_uint128_t(10000000000000000000ULL),
    detail::builtin_uint128_t(10000000000000000000ULL) * detail::builtin_uint128_t(10000000000000000000ULL) * detail::builtin_uint128_t(10ULL),
};

static_assert(sizeof(builtin_128_pow10) == sizeof(boost::decimal::detail::builtin_uint128_t) * 40, "Should have 10^0 to 10^39");

#endif

BOOST_DECIMAL_CONSTEXPR_VARIABLE u256 u256_pow_10[] = {
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(0), UINT64_C(1)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(0), UINT64_C(10)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(0), UINT64_C(100)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(0), UINT64_C(1000)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(0), UINT64_C(10000)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(0), UINT64_C(100000)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(0), UINT64_C(1000000)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(0), UINT64_C(10000000)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(0), UINT64_C(100000000)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(0), UINT64_C(1000000000)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(0), UINT64_C(10000000000)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(0), UINT64_C(100000000000)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(0), UINT64_C(1000000000000)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(0), UINT64_C(10000000000000)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(0), UINT64_C(100000000000000)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(0), UINT64_C(1000000000000000)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(0), UINT64_C(10000000000000000)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(0), UINT64_C(100000000000000000)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(0), UINT64_C(1000000000000000000)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(0), UINT64_C(10000000000000000000)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(5), UINT64_C(7766279631452241920)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(54), UINT64_C(3875820019684212736)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(542), UINT64_C(1864712049423024128)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(5421), UINT64_C(200376420520689664)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(54210), UINT64_C(2003764205206896640)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(542101), UINT64_C(1590897978359414784)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(5421010), UINT64_C(15908979783594147840)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(54210108), UINT64_C(11515845246265065472)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(542101086), UINT64_C(4477988020393345024)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(5421010862), UINT64_C(7886392056514347008)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(54210108624), UINT64_C(5076944270305263616)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(542101086242), UINT64_C(13875954555633532928)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(5421010862427), UINT64_C(9632337040368467968)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(54210108624275), UINT64_C(4089650035136921600)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(542101086242752), UINT64_C(4003012203950112768)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(5421010862427522), UINT64_C(3136633892082024448)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(54210108624275221), UINT64_C(12919594847110692864)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(542101086242752217), UINT64_C(68739955140067328)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(0)}, boost::int128::uint128_t{UINT64_C(5421010862427522170), UINT64_C(687399551400673280)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(2)}, boost::int128::uint128_t{UINT64_C(17316620476856118468), UINT64_C(6873995514006732800)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(29)}, boost::int128::uint128_t{UINT64_C(7145508105175220139), UINT64_C(13399722918938673152)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(293)}, boost::int128::uint128_t{UINT64_C(16114848830623546549), UINT64_C(4870020673419870208)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(2938)}, boost::int128::uint128_t{UINT64_C(13574535716559052564), UINT64_C(11806718586779598848)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(29387)}, boost::int128::uint128_t{UINT64_C(6618148649623664334), UINT64_C(7386721425538678784)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(293873)}, boost::int128::uint128_t{UINT64_C(10841254275107988496), UINT64_C(80237960548581376)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(2938735)}, boost::int128::uint128_t{UINT64_C(16178822382532126880), UINT64_C(802379605485813760)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(29387358)}, boost::int128::uint128_t{UINT64_C(14214271235644855872), UINT64_C(8023796054858137600)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(293873587)}, boost::int128::uint128_t{UINT64_C(13015503840481697412), UINT64_C(6450984253743169536)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(2938735877)}, boost::int128::uint128_t{UINT64_C(1027829888850112811), UINT64_C(9169610316303040512)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(29387358770)}, boost::int128::uint128_t{UINT64_C(10278298888501128114), UINT64_C(17909126868192198656)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(293873587705)}, boost::int128::uint128_t{UINT64_C(10549268516463523069), UINT64_C(13070572018536022016)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(2938735877055)}, boost::int128::uint128_t{UINT64_C(13258964796087472617), UINT64_C(1578511669393358848)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(29387358770557)}, boost::int128::uint128_t{UINT64_C(3462439444907864858), UINT64_C(15785116693933588480)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(293873587705571)}, boost::int128::uint128_t{UINT64_C(16177650375369096972), UINT64_C(10277214349659471872)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(2938735877055718)}, boost::int128::uint128_t{UINT64_C(14202551164014556797), UINT64_C(10538423128046960640)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(29387358770557187)}, boost::int128::uint128_t{UINT64_C(12898303124178706663), UINT64_C(13150510911921848320)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(293873587705571876)}, boost::int128::uint128_t{UINT64_C(18302566799529756941), UINT64_C(2377900603251621888)}},
   u256{boost::int128::uint128_t{UINT64_C(0), UINT64_C(2938735877055718769)}, boost::int128::uint128_t{UINT64_C(17004971331911604867), UINT64_C(5332261958806667264)}},
   u256{boost::int128::uint128_t{UINT64_C(1), UINT64_C(10940614696847636083)}, boost::int128::uint128_t{UINT64_C(4029016655730084128), UINT64_C(16429131440647569408)}},
   u256{boost::int128::uint128_t{UINT64_C(15), UINT64_C(17172426599928602752)}, boost::int128::uint128_t{UINT64_C(3396678409881738056), UINT64_C(16717361816799281152)}},
   u256{boost::int128::uint128_t{UINT64_C(159), UINT64_C(5703569335900062977)}, boost::int128::uint128_t{UINT64_C(15520040025107828953), UINT64_C(1152921504606846976)}},
   u256{boost::int128::uint128_t{UINT64_C(1593), UINT64_C(1695461137871974930)}, boost::int128::uint128_t{UINT64_C(7626447661401876602), UINT64_C(11529215046068469760)}},
   u256{boost::int128::uint128_t{UINT64_C(15930), UINT64_C(16954611378719749304)}, boost::int128::uint128_t{UINT64_C(2477500319180559562), UINT64_C(4611686018427387904)}},
   u256{boost::int128::uint128_t{UINT64_C(159309), UINT64_C(3525417123811528497)}, boost::int128::uint128_t{UINT64_C(6328259118096044006), UINT64_C(9223372036854775808)}},
   u256{boost::int128::uint128_t{UINT64_C(1593091), UINT64_C(16807427164405733357)}, boost::int128::uint128_t{UINT64_C(7942358959831785217), UINT64_C(0)}},
   u256{boost::int128::uint128_t{UINT64_C(15930919), UINT64_C(2053574980671369030)}, boost::int128::uint128_t{UINT64_C(5636613303479645706), UINT64_C(0)}},
   u256{boost::int128::uint128_t{UINT64_C(159309191), UINT64_C(2089005733004138687)}, boost::int128::uint128_t{UINT64_C(1025900813667802212), UINT64_C(0)}},
   u256{boost::int128::uint128_t{UINT64_C(1593091911), UINT64_C(2443313256331835254)}, boost::int128::uint128_t{UINT64_C(10259008136678022120), UINT64_C(0)}},
   u256{boost::int128::uint128_t{UINT64_C(15930919111), UINT64_C(5986388489608800929)}, boost::int128::uint128_t{UINT64_C(10356360998232463120), UINT64_C(0)}},
   u256{boost::int128::uint128_t{UINT64_C(159309191113), UINT64_C(4523652674959354447)}, boost::int128::uint128_t{UINT64_C(11329889613776873120), UINT64_C(0)}},
   u256{boost::int128::uint128_t{UINT64_C(1593091911132), UINT64_C(8343038602174441244)}, boost::int128::uint128_t{UINT64_C(2618431695511421504), UINT64_C(0)}},
   u256{boost::int128::uint128_t{UINT64_C(15930919111324), UINT64_C(9643409726906205977)}, boost::int128::uint128_t{UINT64_C(7737572881404663424), UINT64_C(0)}},
   u256{boost::int128::uint128_t{UINT64_C(159309191113245), UINT64_C(4200376900514301694)}, boost::int128::uint128_t{UINT64_C(3588752519208427776), UINT64_C(0)}},
   u256{boost::int128::uint128_t{UINT64_C(1593091911132452), UINT64_C(5110280857723913709)}, boost::int128::uint128_t{UINT64_C(17440781118374726144), UINT64_C(0)}},
   u256{boost::int128::uint128_t{UINT64_C(15930919111324522), UINT64_C(14209320429820033867)}, boost::int128::uint128_t{UINT64_C(8387114520361296896), UINT64_C(0)}},
   u256{boost::int128::uint128_t{UINT64_C(159309191113245227), UINT64_C(12965995782233477362)}, boost::int128::uint128_t{UINT64_C(10084168908774762496), UINT64_C(0)}},
   u256{boost::int128::uint128_t{UINT64_C(1593091911132452277), UINT64_C(532749306367912313)}, boost::int128::uint128_t{UINT64_C(8607968719199866880), UINT64_C(0)}},
   u256{boost::int128::uint128_t{UINT64_C(15930919111324522770), UINT64_C(5327493063679123134)}, boost::int128::uint128_t{UINT64_C(12292710897160462336), UINT64_C(0)}},
};

} // namespace impl

#if defined(__GNUC__) && __GNUC__ >= 7
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Warray-bounds"
#endif

template <typename T>
constexpr auto pow10(T n) noexcept -> T
{
    return static_cast<T>(impl::powers_of_10[static_cast<std::size_t>(n)]);
}

template <>
constexpr auto pow10(const boost::int128::uint128_t n) noexcept -> boost::int128::uint128_t
{
    return impl::BOOST_DECIMAL_DETAIL_INT128_pow10[static_cast<std::size_t>(n.low)];
}

#ifdef BOOST_DECIMAL_HAS_INT128

template <>
constexpr auto pow10(detail::builtin_uint128_t n) noexcept -> detail::builtin_uint128_t
{
    return impl::builtin_128_pow10[static_cast<std::size_t>(n)];
}

#endif

constexpr auto pow10(const u256& n) noexcept -> u256
{
    return impl::u256_pow_10[n[3]];
}

#if defined(__GNUC__) && __GNUC__ >= 7
#  pragma GCC diagnostic pop
#endif

} // namespace detail
} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_POWER_TABLES_HPP
