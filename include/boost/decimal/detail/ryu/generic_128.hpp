// Copyright 2018 - 2023 Ulf Adams
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_RYU_GENERIC_128_HPP
#define BOOST_DECIMAL_DETAIL_RYU_GENERIC_128_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/integer_search_trees.hpp>
#include <boost/decimal/detail/emulated128.hpp>
#include <cstdint>

#define BOOST_DECIMAL_POW5_TABLE_SIZE 56
#define BOOST_DECIMAL_POW5_BITCOUNT 249
#define BOOST_DECIMAL_POW5_INV_BITCOUNT 249

namespace boost { namespace decimal { namespace detail { namespace ryu {

#ifdef BOOST_DECIMAL_HAS_INT128
using unsigned_128_type = uint128_t;
#else
using unsigned_128_type = uint128;
#endif
    
// These tables are ~4.5 kByte total, compared to ~160 kByte for the full tables.
//
// There's no way to define 128-bit constants in C, so we use little-endian
// pairs of 64-bit constants.

static constexpr std::uint64_t GENERIC_POW5_TABLE[BOOST_DECIMAL_POW5_TABLE_SIZE][2] = {
{ static_cast<std::uint64_t>(UINT64_C(                   1)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(                   5)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(                  25)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(                 125)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(                 625)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(                3125)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(               15625)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(               78125)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(              390625)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(             1953125)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(             9765625)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(            48828125)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(           244140625)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(          1220703125)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(          6103515625)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(         30517578125)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(        152587890625)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(        762939453125)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(       3814697265625)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(      19073486328125)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(      95367431640625)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(     476837158203125)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(    2384185791015625)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(   11920928955078125)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(   59604644775390625)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(  298023223876953125)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C( 1490116119384765625)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C( 7450580596923828125)), static_cast<std::uint64_t>(UINT64_C(                   0)) },
{ static_cast<std::uint64_t>(UINT64_C(  359414837200037393)), static_cast<std::uint64_t>(UINT64_C(                   2)) },
{ static_cast<std::uint64_t>(UINT64_C( 1797074186000186965)), static_cast<std::uint64_t>(UINT64_C(                  10)) },
{ static_cast<std::uint64_t>(UINT64_C( 8985370930000934825)), static_cast<std::uint64_t>(UINT64_C(                  50)) },
{ static_cast<std::uint64_t>(UINT64_C( 8033366502585570893)), static_cast<std::uint64_t>(UINT64_C(                 252)) },
{ static_cast<std::uint64_t>(UINT64_C( 3273344365508751233)), static_cast<std::uint64_t>(UINT64_C(                1262)) },
{ static_cast<std::uint64_t>(UINT64_C(16366721827543756165)), static_cast<std::uint64_t>(UINT64_C(                6310)) },
{ static_cast<std::uint64_t>(UINT64_C( 8046632842880574361)), static_cast<std::uint64_t>(UINT64_C(               31554)) },
{ static_cast<std::uint64_t>(UINT64_C( 3339676066983768573)), static_cast<std::uint64_t>(UINT64_C(              157772)) },
{ static_cast<std::uint64_t>(UINT64_C(16698380334918842865)), static_cast<std::uint64_t>(UINT64_C(              788860)) },
{ static_cast<std::uint64_t>(UINT64_C( 9704925379756007861)), static_cast<std::uint64_t>(UINT64_C(             3944304)) },
{ static_cast<std::uint64_t>(UINT64_C(11631138751360936073)), static_cast<std::uint64_t>(UINT64_C(            19721522)) },
{ static_cast<std::uint64_t>(UINT64_C( 2815461535676025517)), static_cast<std::uint64_t>(UINT64_C(            98607613)) },
{ static_cast<std::uint64_t>(UINT64_C(14077307678380127585)), static_cast<std::uint64_t>(UINT64_C(           493038065)) },
{ static_cast<std::uint64_t>(UINT64_C(15046306170771983077)), static_cast<std::uint64_t>(UINT64_C(          2465190328)) },
{ static_cast<std::uint64_t>(UINT64_C( 1444554559021708921)), static_cast<std::uint64_t>(UINT64_C(         12325951644)) },
{ static_cast<std::uint64_t>(UINT64_C( 7222772795108544605)), static_cast<std::uint64_t>(UINT64_C(         61629758220)) },
{ static_cast<std::uint64_t>(UINT64_C(17667119901833171409)), static_cast<std::uint64_t>(UINT64_C(        308148791101)) },
{ static_cast<std::uint64_t>(UINT64_C(14548623214327650581)), static_cast<std::uint64_t>(UINT64_C(       1540743955509)) },
{ static_cast<std::uint64_t>(UINT64_C(17402883850509598057)), static_cast<std::uint64_t>(UINT64_C(       7703719777548)) },
{ static_cast<std::uint64_t>(UINT64_C(13227442957709783821)), static_cast<std::uint64_t>(UINT64_C(      38518598887744)) },
{ static_cast<std::uint64_t>(UINT64_C(10796982567420264257)), static_cast<std::uint64_t>(UINT64_C(     192592994438723)) },
{ static_cast<std::uint64_t>(UINT64_C(17091424689682218053)), static_cast<std::uint64_t>(UINT64_C(     962964972193617)) },
{ static_cast<std::uint64_t>(UINT64_C(11670147153572883801)), static_cast<std::uint64_t>(UINT64_C(    4814824860968089)) },
{ static_cast<std::uint64_t>(UINT64_C( 3010503546735764157)), static_cast<std::uint64_t>(UINT64_C(   24074124304840448)) },
{ static_cast<std::uint64_t>(UINT64_C(15052517733678820785)), static_cast<std::uint64_t>(UINT64_C(  120370621524202240)) },
{ static_cast<std::uint64_t>(UINT64_C( 1475612373555897461)), static_cast<std::uint64_t>(UINT64_C(  601853107621011204)) },
{ static_cast<std::uint64_t>(UINT64_C( 7378061867779487305)), static_cast<std::uint64_t>(UINT64_C( 3009265538105056020)) },
{ static_cast<std::uint64_t>(UINT64_C(18443565265187884909)), static_cast<std::uint64_t>(UINT64_C(15046327690525280101)) }
};

static constexpr std::uint64_t GENERIC_POW5_SPLIT[89][4] = {
{ static_cast<std::uint64_t>(UINT64_C(                   0)), static_cast<std::uint64_t>(UINT64_C(                   0)), static_cast<std::uint64_t>(UINT64_C(                   0)),   static_cast<std::uint64_t>(UINT64_C( 72057594037927936)) },
{ static_cast<std::uint64_t>(UINT64_C(                   0)), static_cast<std::uint64_t>(UINT64_C( 5206161169240293376)), static_cast<std::uint64_t>(UINT64_C( 4575641699882439235)),   static_cast<std::uint64_t>(UINT64_C( 73468396926392969)) },
{ static_cast<std::uint64_t>(UINT64_C( 3360510775605221349)), static_cast<std::uint64_t>(UINT64_C( 6983200512169538081)), static_cast<std::uint64_t>(UINT64_C( 4325643253124434363)),   static_cast<std::uint64_t>(UINT64_C( 74906821675075173)) },
{ static_cast<std::uint64_t>(UINT64_C(11917660854915489451)), static_cast<std::uint64_t>(UINT64_C( 9652941469841108803)), static_cast<std::uint64_t>(UINT64_C(  946308467778435600)),   static_cast<std::uint64_t>(UINT64_C( 76373409087490117)) },
{ static_cast<std::uint64_t>(UINT64_C( 1994853395185689235)), static_cast<std::uint64_t>(UINT64_C(16102657350889591545)), static_cast<std::uint64_t>(UINT64_C( 6847013871814915412)),   static_cast<std::uint64_t>(UINT64_C( 77868710555449746)) },
{ static_cast<std::uint64_t>(UINT64_C(  958415760277438274)), static_cast<std::uint64_t>(UINT64_C(15059347134713823592)), static_cast<std::uint64_t>(UINT64_C( 7329070255463483331)),   static_cast<std::uint64_t>(UINT64_C( 79393288266368765)) },
{ static_cast<std::uint64_t>(UINT64_C( 2065144883315240188)), static_cast<std::uint64_t>(UINT64_C( 7145278325844925976)), static_cast<std::uint64_t>(UINT64_C(14718454754511147343)),   static_cast<std::uint64_t>(UINT64_C( 80947715414629833)) },
{ static_cast<std::uint64_t>(UINT64_C( 8980391188862868935)), static_cast<std::uint64_t>(UINT64_C(13709057401304208685)), static_cast<std::uint64_t>(UINT64_C( 8230434828742694591)),   static_cast<std::uint64_t>(UINT64_C( 82532576417087045)) },
{ static_cast<std::uint64_t>(UINT64_C(  432148644612782575)), static_cast<std::uint64_t>(UINT64_C( 7960151582448466064)), static_cast<std::uint64_t>(UINT64_C(12056089168559840552)),   static_cast<std::uint64_t>(UINT64_C( 84148467132788711)) },
{ static_cast<std::uint64_t>(UINT64_C(  484109300864744403)), static_cast<std::uint64_t>(UINT64_C(15010663910730448582)), static_cast<std::uint64_t>(UINT64_C(16824949663447227068)),   static_cast<std::uint64_t>(UINT64_C( 85795995087002057)) },
{ static_cast<std::uint64_t>(UINT64_C(14793711725276144220)), static_cast<std::uint64_t>(UINT64_C(16494403799991899904)), static_cast<std::uint64_t>(UINT64_C(10145107106505865967)),   static_cast<std::uint64_t>(UINT64_C( 87475779699624060)) },
{ static_cast<std::uint64_t>(UINT64_C(15427548291869817042)), static_cast<std::uint64_t>(UINT64_C(12330588654550505203)), static_cast<std::uint64_t>(UINT64_C(13980791795114552342)),   static_cast<std::uint64_t>(UINT64_C( 89188452518064298)) },
{ static_cast<std::uint64_t>(UINT64_C( 9979404135116626552)), static_cast<std::uint64_t>(UINT64_C(13477446383271537499)), static_cast<std::uint64_t>(UINT64_C(14459862802511591337)),   static_cast<std::uint64_t>(UINT64_C( 90934657454687378)) },
{ static_cast<std::uint64_t>(UINT64_C(12385121150303452775)), static_cast<std::uint64_t>(UINT64_C( 9097130814231585614)), static_cast<std::uint64_t>(UINT64_C( 6523855782339765207)),   static_cast<std::uint64_t>(UINT64_C( 92715051028904201)) },
{ static_cast<std::uint64_t>(UINT64_C( 1822931022538209743)), static_cast<std::uint64_t>(UINT64_C(16062974719797586441)), static_cast<std::uint64_t>(UINT64_C( 3619180286173516788)),   static_cast<std::uint64_t>(UINT64_C( 94530302614003091)) },
{ static_cast<std::uint64_t>(UINT64_C(12318611738248470829)), static_cast<std::uint64_t>(UINT64_C(13330752208259324507)), static_cast<std::uint64_t>(UINT64_C(10986694768744162601)),   static_cast<std::uint64_t>(UINT64_C( 96381094688813589)) },
{ static_cast<std::uint64_t>(UINT64_C(13684493829640282333)), static_cast<std::uint64_t>(UINT64_C( 7674802078297225834)), static_cast<std::uint64_t>(UINT64_C(15208116197624593182)),   static_cast<std::uint64_t>(UINT64_C( 98268123094297527)) },
{ static_cast<std::uint64_t>(UINT64_C( 5408877057066295332)), static_cast<std::uint64_t>(UINT64_C( 6470124174091971006)), static_cast<std::uint64_t>(UINT64_C(15112713923117703147)),   static_cast<std::uint64_t>(UINT64_C(100192097295163851)) },
{ static_cast<std::uint64_t>(UINT64_C(11407083166564425062)), static_cast<std::uint64_t>(UINT64_C(18189998238742408185)), static_cast<std::uint64_t>(UINT64_C( 4337638702446708282)),   static_cast<std::uint64_t>(UINT64_C(102153740646605557)) },
{ static_cast<std::uint64_t>(UINT64_C( 4112405898036935485)), static_cast<std::uint64_t>(UINT64_C(  924624216579956435)), static_cast<std::uint64_t>(UINT64_C(14251108172073737125)),   static_cast<std::uint64_t>(UINT64_C(104153790666259019)) },
{ static_cast<std::uint64_t>(UINT64_C(16996739107011444789)), static_cast<std::uint64_t>(UINT64_C(10015944118339042475)), static_cast<std::uint64_t>(UINT64_C( 2395188869672266257)),   static_cast<std::uint64_t>(UINT64_C(106192999311487969)) },
{ static_cast<std::uint64_t>(UINT64_C( 4588314690421337879)), static_cast<std::uint64_t>(UINT64_C( 5339991768263654604)), static_cast<std::uint64_t>(UINT64_C(15441007590670620066)),   static_cast<std::uint64_t>(UINT64_C(108272133262096356)) },
{ static_cast<std::uint64_t>(UINT64_C( 2286159977890359825)), static_cast<std::uint64_t>(UINT64_C(14329706763185060248)), static_cast<std::uint64_t>(UINT64_C( 5980012964059367667)),   static_cast<std::uint64_t>(UINT64_C(110391974208576409)) },
{ static_cast<std::uint64_t>(UINT64_C( 9654767503237031099)), static_cast<std::uint64_t>(UINT64_C(11293544302844823188)), static_cast<std::uint64_t>(UINT64_C(11739932712678287805)),   static_cast<std::uint64_t>(UINT64_C(112553319146000238)) },
{ static_cast<std::uint64_t>(UINT64_C(11362964448496095896)), static_cast<std::uint64_t>(UINT64_C( 7990659682315657680)), static_cast<std::uint64_t>(UINT64_C(  251480263940996374)),   static_cast<std::uint64_t>(UINT64_C(114756980673665505)) },
{ static_cast<std::uint64_t>(UINT64_C( 1423410421096377129)), static_cast<std::uint64_t>(UINT64_C(14274395557581462179)), static_cast<std::uint64_t>(UINT64_C(16553482793602208894)),   static_cast<std::uint64_t>(UINT64_C(117003787300607788)) },
{ static_cast<std::uint64_t>(UINT64_C( 2070444190619093137)), static_cast<std::uint64_t>(UINT64_C(11517140404712147401)), static_cast<std::uint64_t>(UINT64_C(11657844572835578076)),   static_cast<std::uint64_t>(UINT64_C(119294583757094535)) },
{ static_cast<std::uint64_t>(UINT64_C( 7648316884775828921)), static_cast<std::uint64_t>(UINT64_C(15264332483297977688)), static_cast<std::uint64_t>(UINT64_C(  247182277434709002)),   static_cast<std::uint64_t>(UINT64_C(121630231312217685)) },
{ static_cast<std::uint64_t>(UINT64_C(17410896758132241352)), static_cast<std::uint64_t>(UINT64_C(10923914482914417070)), static_cast<std::uint64_t>(UINT64_C(13976383996795783649)),   static_cast<std::uint64_t>(UINT64_C(124011608097704390)) },
{ static_cast<std::uint64_t>(UINT64_C( 9542674537907272703)), static_cast<std::uint64_t>(UINT64_C( 3079432708831728956)), static_cast<std::uint64_t>(UINT64_C(14235189590642919676)),   static_cast<std::uint64_t>(UINT64_C(126439609438067572)) },
{ static_cast<std::uint64_t>(UINT64_C(10364666969937261816)), static_cast<std::uint64_t>(UINT64_C( 8464573184892924210)), static_cast<std::uint64_t>(UINT64_C(12758646866025101190)),   static_cast<std::uint64_t>(UINT64_C(128915148187220428)) },
{ static_cast<std::uint64_t>(UINT64_C(14720354822146013883)), static_cast<std::uint64_t>(UINT64_C(11480204489231511423)), static_cast<std::uint64_t>(UINT64_C( 7449876034836187038)),   static_cast<std::uint64_t>(UINT64_C(131439155071681461)) },
{ static_cast<std::uint64_t>(UINT64_C( 1692907053653558553)), static_cast<std::uint64_t>(UINT64_C(17835392458598425233)), static_cast<std::uint64_t>(UINT64_C( 1754856712536736598)),   static_cast<std::uint64_t>(UINT64_C(134012579040499057)) },
{ static_cast<std::uint64_t>(UINT64_C( 5620591334531458755)), static_cast<std::uint64_t>(UINT64_C(11361776175667106627)), static_cast<std::uint64_t>(UINT64_C(13350215315297937856)),   static_cast<std::uint64_t>(UINT64_C(136636387622027174)) },
{ static_cast<std::uint64_t>(UINT64_C(17455759733928092601)), static_cast<std::uint64_t>(UINT64_C(10362573084069962561)), static_cast<std::uint64_t>(UINT64_C(11246018728801810510)),   static_cast<std::uint64_t>(UINT64_C(139311567287686283)) },
{ static_cast<std::uint64_t>(UINT64_C( 2465404073814044982)), static_cast<std::uint64_t>(UINT64_C(17694822665274381860)), static_cast<std::uint64_t>(UINT64_C( 1509954037718722697)),   static_cast<std::uint64_t>(UINT64_C(142039123822846312)) },
{ static_cast<std::uint64_t>(UINT64_C( 2152236053329638369)), static_cast<std::uint64_t>(UINT64_C(11202280800589637091)), static_cast<std::uint64_t>(UINT64_C(16388426812920420176)),   static_cast<std::uint64_t>(UINT64_C( 72410041352485523)) },
{ static_cast<std::uint64_t>(UINT64_C(17319024055671609028)), static_cast<std::uint64_t>(UINT64_C(10944982848661280484)), static_cast<std::uint64_t>(UINT64_C( 2457150158022562661)),   static_cast<std::uint64_t>(UINT64_C( 73827744744583080)) },
{ static_cast<std::uint64_t>(UINT64_C(17511219308535248024)), static_cast<std::uint64_t>(UINT64_C( 5122059497846768077)), static_cast<std::uint64_t>(UINT64_C( 2089605804219668451)),   static_cast<std::uint64_t>(UINT64_C( 75273205100637900)) },
{ static_cast<std::uint64_t>(UINT64_C(10082673333144031533)), static_cast<std::uint64_t>(UINT64_C(14429008783411894887)), static_cast<std::uint64_t>(UINT64_C(12842832230171903890)),   static_cast<std::uint64_t>(UINT64_C( 76746965869337783)) },
{ static_cast<std::uint64_t>(UINT64_C(16196653406315961184)), static_cast<std::uint64_t>(UINT64_C(10260180891682904501)), static_cast<std::uint64_t>(UINT64_C(10537411930446752461)),   static_cast<std::uint64_t>(UINT64_C( 78249581139456266)) },
{ static_cast<std::uint64_t>(UINT64_C(15084422041749743389)), static_cast<std::uint64_t>(UINT64_C(  234835370106753111)), static_cast<std::uint64_t>(UINT64_C(16662517110286225617)),   static_cast<std::uint64_t>(UINT64_C( 79781615848172976)) },
{ static_cast<std::uint64_t>(UINT64_C( 8199644021067702606)), static_cast<std::uint64_t>(UINT64_C( 3787318116274991885)), static_cast<std::uint64_t>(UINT64_C( 7438130039325743106)),   static_cast<std::uint64_t>(UINT64_C( 81343645993472659)) },
{ static_cast<std::uint64_t>(UINT64_C(12039493937039359765)), static_cast<std::uint64_t>(UINT64_C( 9773822153580393709)), static_cast<std::uint64_t>(UINT64_C( 5945428874398357806)),   static_cast<std::uint64_t>(UINT64_C( 82936258850702722)) },
{ static_cast<std::uint64_t>(UINT64_C(  984543865091303961)), static_cast<std::uint64_t>(UINT64_C( 7975107621689454830)), static_cast<std::uint64_t>(UINT64_C( 6556665988501773347)),   static_cast<std::uint64_t>(UINT64_C( 84560053193370726)) },
{ static_cast<std::uint64_t>(UINT64_C( 9633317878125234244)), static_cast<std::uint64_t>(UINT64_C(16099592426808915028)), static_cast<std::uint64_t>(UINT64_C( 9706674539190598200)),   static_cast<std::uint64_t>(UINT64_C( 86215639518264828)) },
{ static_cast<std::uint64_t>(UINT64_C( 6860695058870476186)), static_cast<std::uint64_t>(UINT64_C( 4471839111886709592)), static_cast<std::uint64_t>(UINT64_C( 7828342285492709568)),   static_cast<std::uint64_t>(UINT64_C( 87903640274981819)) },
{ static_cast<std::uint64_t>(UINT64_C(14583324717644598331)), static_cast<std::uint64_t>(UINT64_C( 4496120889473451238)), static_cast<std::uint64_t>(UINT64_C( 5290040788305728466)),   static_cast<std::uint64_t>(UINT64_C( 89624690099949049)) },
{ static_cast<std::uint64_t>(UINT64_C(18093669366515003715)), static_cast<std::uint64_t>(UINT64_C(12879506572606942994)), static_cast<std::uint64_t>(UINT64_C(18005739787089675377)),   static_cast<std::uint64_t>(UINT64_C( 91379436055028227)) },
{ static_cast<std::uint64_t>(UINT64_C(17997493966862379937)), static_cast<std::uint64_t>(UINT64_C(14646222655265145582)), static_cast<std::uint64_t>(UINT64_C(10265023312844161858)),   static_cast<std::uint64_t>(UINT64_C( 93168537870790806)) },
{ static_cast<std::uint64_t>(UINT64_C(12283848109039722318)), static_cast<std::uint64_t>(UINT64_C(11290258077250314935)), static_cast<std::uint64_t>(UINT64_C( 9878160025624946825)),   static_cast<std::uint64_t>(UINT64_C( 94992668194556404)) },
{ static_cast<std::uint64_t>(UINT64_C( 8087752761883078164)), static_cast<std::uint64_t>(UINT64_C( 5262596608437575693)), static_cast<std::uint64_t>(UINT64_C(11093553063763274413)),   static_cast<std::uint64_t>(UINT64_C( 96852512843287537)) },
{ static_cast<std::uint64_t>(UINT64_C(15027787746776840781)), static_cast<std::uint64_t>(UINT64_C(12250273651168257752)), static_cast<std::uint64_t>(UINT64_C( 9290470558712181914)),   static_cast<std::uint64_t>(UINT64_C( 98748771061435726)) },
{ static_cast<std::uint64_t>(UINT64_C(15003915578366724489)), static_cast<std::uint64_t>(UINT64_C( 2937334162439764327)), static_cast<std::uint64_t>(UINT64_C( 5404085603526796602)),   static_cast<std::uint64_t>(UINT64_C(100682155783835929)) },
{ static_cast<std::uint64_t>(UINT64_C( 5225610465224746757)), static_cast<std::uint64_t>(UINT64_C(14932114897406142027)), static_cast<std::uint64_t>(UINT64_C( 2774647558180708010)),   static_cast<std::uint64_t>(UINT64_C(102653393903748137)) },
{ static_cast<std::uint64_t>(UINT64_C(17112957703385190360)), static_cast<std::uint64_t>(UINT64_C(12069082008339002412)), static_cast<std::uint64_t>(UINT64_C( 3901112447086388439)),   static_cast<std::uint64_t>(UINT64_C(104663226546146909)) },
{ static_cast<std::uint64_t>(UINT64_C( 4062324464323300238)), static_cast<std::uint64_t>(UINT64_C( 3992768146772240329)), static_cast<std::uint64_t>(UINT64_C(15757196565593695724)),   static_cast<std::uint64_t>(UINT64_C(106712409346361594)) },
{ static_cast<std::uint64_t>(UINT64_C( 5525364615810306701)), static_cast<std::uint64_t>(UINT64_C(11855206026704935156)), static_cast<std::uint64_t>(UINT64_C(11344868740897365300)),   static_cast<std::uint64_t>(UINT64_C(108801712734172003)) },
{ static_cast<std::uint64_t>(UINT64_C( 9274143661888462646)), static_cast<std::uint64_t>(UINT64_C( 4478365862348432381)), static_cast<std::uint64_t>(UINT64_C(18010077872551661771)),   static_cast<std::uint64_t>(UINT64_C(110931922223466333)) },
{ static_cast<std::uint64_t>(UINT64_C(12604141221930060148)), static_cast<std::uint64_t>(UINT64_C( 8930937759942591500)), static_cast<std::uint64_t>(UINT64_C( 9382183116147201338)),   static_cast<std::uint64_t>(UINT64_C(113103838707570263)) },
{ static_cast<std::uint64_t>(UINT64_C(14513929377491886653)), static_cast<std::uint64_t>(UINT64_C( 1410646149696279084)), static_cast<std::uint64_t>(UINT64_C(  587092196850797612)),   static_cast<std::uint64_t>(UINT64_C(115318278760358235)) },
{ static_cast<std::uint64_t>(UINT64_C( 2226851524999454362)), static_cast<std::uint64_t>(UINT64_C( 7717102471110805679)), static_cast<std::uint64_t>(UINT64_C( 7187441550995571734)),   static_cast<std::uint64_t>(UINT64_C(117576074943260147)) },
{ static_cast<std::uint64_t>(UINT64_C( 5527526061344932763)), static_cast<std::uint64_t>(UINT64_C( 2347100676188369132)), static_cast<std::uint64_t>(UINT64_C(16976241418824030445)),   static_cast<std::uint64_t>(UINT64_C(119878076118278875)) },
{ static_cast<std::uint64_t>(UINT64_C( 6088479778147221611)), static_cast<std::uint64_t>(UINT64_C(17669593130014777580)), static_cast<std::uint64_t>(UINT64_C(10991124207197663546)),   static_cast<std::uint64_t>(UINT64_C(122225147767136307)) },
{ static_cast<std::uint64_t>(UINT64_C(11107734086759692041)), static_cast<std::uint64_t>(UINT64_C( 3391795220306863431)), static_cast<std::uint64_t>(UINT64_C(17233960908859089158)),   static_cast<std::uint64_t>(UINT64_C(124618172316667879)) },
{ static_cast<std::uint64_t>(UINT64_C( 7913172514655155198)), static_cast<std::uint64_t>(UINT64_C(17726879005381242552)), static_cast<std::uint64_t>(UINT64_C(  641069866244011540)),   static_cast<std::uint64_t>(UINT64_C(127058049470587962)) },
{ static_cast<std::uint64_t>(UINT64_C(12596991768458713949)), static_cast<std::uint64_t>(UINT64_C(15714785522479904446)), static_cast<std::uint64_t>(UINT64_C( 6035972567136116512)),   static_cast<std::uint64_t>(UINT64_C(129545696547750811)) },
{ static_cast<std::uint64_t>(UINT64_C(16901996933781815980)), static_cast<std::uint64_t>(UINT64_C( 4275085211437148707)), static_cast<std::uint64_t>(UINT64_C(14091642539965169063)),   static_cast<std::uint64_t>(UINT64_C(132082048827034281)) },
{ static_cast<std::uint64_t>(UINT64_C( 7524574627987869240)), static_cast<std::uint64_t>(UINT64_C(15661204384239316051)), static_cast<std::uint64_t>(UINT64_C( 2444526454225712267)),   static_cast<std::uint64_t>(UINT64_C(134668059898975949)) },
{ static_cast<std::uint64_t>(UINT64_C( 8199251625090479942)), static_cast<std::uint64_t>(UINT64_C( 6803282222165044067)), static_cast<std::uint64_t>(UINT64_C(16064817666437851504)),   static_cast<std::uint64_t>(UINT64_C(137304702024293857)) },
{ static_cast<std::uint64_t>(UINT64_C( 4453256673338111920)), static_cast<std::uint64_t>(UINT64_C(15269922543084434181)), static_cast<std::uint64_t>(UINT64_C( 3139961729834750852)),   static_cast<std::uint64_t>(UINT64_C(139992966499426682)) },
{ static_cast<std::uint64_t>(UINT64_C(15841763546372731299)), static_cast<std::uint64_t>(UINT64_C( 3013174075437671812)), static_cast<std::uint64_t>(UINT64_C( 4383755396295695606)),   static_cast<std::uint64_t>(UINT64_C(142733864029230733)) },
{ static_cast<std::uint64_t>(UINT64_C( 9771896230907310329)), static_cast<std::uint64_t>(UINT64_C( 4900659362437687569)), static_cast<std::uint64_t>(UINT64_C(12386126719044266361)),   static_cast<std::uint64_t>(UINT64_C( 72764212553486967)) },
{ static_cast<std::uint64_t>(UINT64_C( 9420455527449565190)), static_cast<std::uint64_t>(UINT64_C( 1859606122611023693)), static_cast<std::uint64_t>(UINT64_C( 6555040298902684281)),   static_cast<std::uint64_t>(UINT64_C( 74188850200884818)) },
{ static_cast<std::uint64_t>(UINT64_C( 5146105983135678095)), static_cast<std::uint64_t>(UINT64_C( 2287300449992174951)), static_cast<std::uint64_t>(UINT64_C( 4325371679080264751)),   static_cast<std::uint64_t>(UINT64_C( 75641380576797959)) },
{ static_cast<std::uint64_t>(UINT64_C(11019359372592553360)), static_cast<std::uint64_t>(UINT64_C( 8422686425957443718)), static_cast<std::uint64_t>(UINT64_C( 7175176077944048210)),   static_cast<std::uint64_t>(UINT64_C( 77122349788024458)) },
{ static_cast<std::uint64_t>(UINT64_C(11005742969399620716)), static_cast<std::uint64_t>(UINT64_C( 4132174559240043701)), static_cast<std::uint64_t>(UINT64_C( 9372258443096612118)),   static_cast<std::uint64_t>(UINT64_C( 78632314633490790)) },
{ static_cast<std::uint64_t>(UINT64_C( 8887589641394725840)), static_cast<std::uint64_t>(UINT64_C( 8029899502466543662)), static_cast<std::uint64_t>(UINT64_C(14582206497241572853)),   static_cast<std::uint64_t>(UINT64_C( 80171842813591127)) },
{ static_cast<std::uint64_t>(UINT64_C(  360247523705545899)), static_cast<std::uint64_t>(UINT64_C(12568341805293354211)), static_cast<std::uint64_t>(UINT64_C(14653258284762517866)),   static_cast<std::uint64_t>(UINT64_C( 81741513143625247)) },
{ static_cast<std::uint64_t>(UINT64_C(12314272731984275834)), static_cast<std::uint64_t>(UINT64_C( 4740745023227177044)), static_cast<std::uint64_t>(UINT64_C( 6141631472368337539)),   static_cast<std::uint64_t>(UINT64_C( 83341915771415304)) },
{ static_cast<std::uint64_t>(UINT64_C(  441052047733984759)), static_cast<std::uint64_t>(UINT64_C( 7940090120939869826)), static_cast<std::uint64_t>(UINT64_C(11750200619921094248)),   static_cast<std::uint64_t>(UINT64_C( 84973652399183278)) },
{ static_cast<std::uint64_t>(UINT64_C( 3436657868127012749)), static_cast<std::uint64_t>(UINT64_C( 9187006432149937667)), static_cast<std::uint64_t>(UINT64_C(16389726097323041290)),   static_cast<std::uint64_t>(UINT64_C( 86637336509772529)) },
{ static_cast<std::uint64_t>(UINT64_C(13490220260784534044)), static_cast<std::uint64_t>(UINT64_C(15339072891382896702)), static_cast<std::uint64_t>(UINT64_C( 8846102360835316895)),   static_cast<std::uint64_t>(UINT64_C( 88333593597298497)) },
{ static_cast<std::uint64_t>(UINT64_C( 4125672032094859833)), static_cast<std::uint64_t>(UINT64_C(  158347675704003277)), static_cast<std::uint64_t>(UINT64_C(10592598512749774447)),   static_cast<std::uint64_t>(UINT64_C( 90063061402315272)) },
{ static_cast<std::uint64_t>(UINT64_C(12189928252974395775)), static_cast<std::uint64_t>(UINT64_C( 2386931199439295891)), static_cast<std::uint64_t>(UINT64_C( 7009030566469913276)),   static_cast<std::uint64_t>(UINT64_C( 91826390151586454)) },
{ static_cast<std::uint64_t>(UINT64_C( 9256479608339282969)), static_cast<std::uint64_t>(UINT64_C( 2844900158963599229)), static_cast<std::uint64_t>(UINT64_C(11148388908923225596)),   static_cast<std::uint64_t>(UINT64_C( 93624242802550437)) },
{ static_cast<std::uint64_t>(UINT64_C(11584393507658707408)), static_cast<std::uint64_t>(UINT64_C( 2863659090805147914)), static_cast<std::uint64_t>(UINT64_C( 9873421561981063551)),   static_cast<std::uint64_t>(UINT64_C( 95457295292572042)) },
{ static_cast<std::uint64_t>(UINT64_C(13984297296943171390)), static_cast<std::uint64_t>(UINT64_C( 1931468383973130608)), static_cast<std::uint64_t>(UINT64_C(12905719743235082319)),   static_cast<std::uint64_t>(UINT64_C( 97326236793074198)) },
{ static_cast<std::uint64_t>(UINT64_C( 5837045222254987499)), static_cast<std::uint64_t>(UINT64_C(10213498696735864176)), static_cast<std::uint64_t>(UINT64_C(14893951506257020749)),   static_cast<std::uint64_t>(UINT64_C( 99231769968645227)) }
};

// Unfortunately, the results are sometimes off by one or two. We use an additional
// lookup table to store those cases and adjust the result.
static constexpr std::uint64_t POW5_ERRORS[156] = {
static_cast<std::uint64_t>(UINT64_C(0x0000000000000000)), static_cast<std::uint64_t>(UINT64_C(0x0000000000000000)), static_cast<std::uint64_t>(UINT64_C(0x0000000000000000)), static_cast<std::uint64_t>(UINT64_C(0x9555596400000000)),
static_cast<std::uint64_t>(UINT64_C(0x65a6569525565555)), static_cast<std::uint64_t>(UINT64_C(0x4415551445449655)), static_cast<std::uint64_t>(UINT64_C(0x5105015504144541)), static_cast<std::uint64_t>(UINT64_C(0x65a69969a6965964)),
static_cast<std::uint64_t>(UINT64_C(0x5054955969959656)), static_cast<std::uint64_t>(UINT64_C(0x5105154515554145)), static_cast<std::uint64_t>(UINT64_C(0x4055511051591555)), static_cast<std::uint64_t>(UINT64_C(0x5500514455550115)),
static_cast<std::uint64_t>(UINT64_C(0x0041140014145515)), static_cast<std::uint64_t>(UINT64_C(0x1005440545511051)), static_cast<std::uint64_t>(UINT64_C(0x0014405450411004)), static_cast<std::uint64_t>(UINT64_C(0x0414440010500000)),
static_cast<std::uint64_t>(UINT64_C(0x0044000440010040)), static_cast<std::uint64_t>(UINT64_C(0x5551155000004001)), static_cast<std::uint64_t>(UINT64_C(0x4554555454544114)), static_cast<std::uint64_t>(UINT64_C(0x5150045544005441)),
static_cast<std::uint64_t>(UINT64_C(0x0001111400054501)), static_cast<std::uint64_t>(UINT64_C(0x6550955555554554)), static_cast<std::uint64_t>(UINT64_C(0x1504159645559559)), static_cast<std::uint64_t>(UINT64_C(0x4105055141454545)),
static_cast<std::uint64_t>(UINT64_C(0x1411541410405454)), static_cast<std::uint64_t>(UINT64_C(0x0415555044545555)), static_cast<std::uint64_t>(UINT64_C(0x0014154115405550)), static_cast<std::uint64_t>(UINT64_C(0x1540055040411445)),
static_cast<std::uint64_t>(UINT64_C(0x0000000500000000)), static_cast<std::uint64_t>(UINT64_C(0x5644000000000000)), static_cast<std::uint64_t>(UINT64_C(0x1155555591596555)), static_cast<std::uint64_t>(UINT64_C(0x0410440054569565)),
static_cast<std::uint64_t>(UINT64_C(0x5145100010010005)), static_cast<std::uint64_t>(UINT64_C(0x0555041405500150)), static_cast<std::uint64_t>(UINT64_C(0x4141450455140450)), static_cast<std::uint64_t>(UINT64_C(0x0000000144000140)),
static_cast<std::uint64_t>(UINT64_C(0x5114004001105410)), static_cast<std::uint64_t>(UINT64_C(0x4444100404005504)), static_cast<std::uint64_t>(UINT64_C(0x0414014410001015)), static_cast<std::uint64_t>(UINT64_C(0x5145055155555015)),
static_cast<std::uint64_t>(UINT64_C(0x0141041444445540)), static_cast<std::uint64_t>(UINT64_C(0x0000100451541414)), static_cast<std::uint64_t>(UINT64_C(0x4105041104155550)), static_cast<std::uint64_t>(UINT64_C(0x0500501150451145)),
static_cast<std::uint64_t>(UINT64_C(0x1001050000004114)), static_cast<std::uint64_t>(UINT64_C(0x5551504400141045)), static_cast<std::uint64_t>(UINT64_C(0x5110545410151454)), static_cast<std::uint64_t>(UINT64_C(0x0100001400004040)),
static_cast<std::uint64_t>(UINT64_C(0x5040010111040000)), static_cast<std::uint64_t>(UINT64_C(0x0140000150541100)), static_cast<std::uint64_t>(UINT64_C(0x4400140400104110)), static_cast<std::uint64_t>(UINT64_C(0x5011014405545004)),
static_cast<std::uint64_t>(UINT64_C(0x0000000044155440)), static_cast<std::uint64_t>(UINT64_C(0x0000000010000000)), static_cast<std::uint64_t>(UINT64_C(0x1100401444440001)), static_cast<std::uint64_t>(UINT64_C(0x0040401010055111)),
static_cast<std::uint64_t>(UINT64_C(0x5155155551405454)), static_cast<std::uint64_t>(UINT64_C(0x0444440015514411)), static_cast<std::uint64_t>(UINT64_C(0x0054505054014101)), static_cast<std::uint64_t>(UINT64_C(0x0451015441115511)),
static_cast<std::uint64_t>(UINT64_C(0x1541411401140551)), static_cast<std::uint64_t>(UINT64_C(0x4155104514445110)), static_cast<std::uint64_t>(UINT64_C(0x4141145450145515)), static_cast<std::uint64_t>(UINT64_C(0x5451445055155050)),
static_cast<std::uint64_t>(UINT64_C(0x4400515554110054)), static_cast<std::uint64_t>(UINT64_C(0x5111145104501151)), static_cast<std::uint64_t>(UINT64_C(0x565a655455500501)), static_cast<std::uint64_t>(UINT64_C(0x5565555555525955)),
static_cast<std::uint64_t>(UINT64_C(0x0550511500405695)), static_cast<std::uint64_t>(UINT64_C(0x4415504051054544)), static_cast<std::uint64_t>(UINT64_C(0x6555595965555554)), static_cast<std::uint64_t>(UINT64_C(0x0100915915555655)),
static_cast<std::uint64_t>(UINT64_C(0x5540001510001001)), static_cast<std::uint64_t>(UINT64_C(0x5450051414000544)), static_cast<std::uint64_t>(UINT64_C(0x1405010555555551)), static_cast<std::uint64_t>(UINT64_C(0x5555515555644155)),
static_cast<std::uint64_t>(UINT64_C(0x5555055595496555)), static_cast<std::uint64_t>(UINT64_C(0x5451045004415000)), static_cast<std::uint64_t>(UINT64_C(0x5450510144040144)), static_cast<std::uint64_t>(UINT64_C(0x5554155555556455)),
static_cast<std::uint64_t>(UINT64_C(0x5051555495415555)), static_cast<std::uint64_t>(UINT64_C(0x5555554555555545)), static_cast<std::uint64_t>(UINT64_C(0x0000000010005455)), static_cast<std::uint64_t>(UINT64_C(0x4000005000040000)),
static_cast<std::uint64_t>(UINT64_C(0x5565555555555954)), static_cast<std::uint64_t>(UINT64_C(0x5554559555555505)), static_cast<std::uint64_t>(UINT64_C(0x9645545495552555)), static_cast<std::uint64_t>(UINT64_C(0x4000400055955564)),
static_cast<std::uint64_t>(UINT64_C(0x0040000000000001)), static_cast<std::uint64_t>(UINT64_C(0x4004100100000000)), static_cast<std::uint64_t>(UINT64_C(0x5540040440000411)), static_cast<std::uint64_t>(UINT64_C(0x4565555955545644)),
static_cast<std::uint64_t>(UINT64_C(0x1140659549651556)), static_cast<std::uint64_t>(UINT64_C(0x0100000410010000)), static_cast<std::uint64_t>(UINT64_C(0x5555515400004001)), static_cast<std::uint64_t>(UINT64_C(0x5955545555155255)),
static_cast<std::uint64_t>(UINT64_C(0x5151055545505556)), static_cast<std::uint64_t>(UINT64_C(0x5051454510554515)), static_cast<std::uint64_t>(UINT64_C(0x0501500050415554)), static_cast<std::uint64_t>(UINT64_C(0x5044154005441005)),
static_cast<std::uint64_t>(UINT64_C(0x1455445450550455)), static_cast<std::uint64_t>(UINT64_C(0x0010144055144545)), static_cast<std::uint64_t>(UINT64_C(0x0000401100000004)), static_cast<std::uint64_t>(UINT64_C(0x1050145050000010)),
static_cast<std::uint64_t>(UINT64_C(0x0415004554011540)), static_cast<std::uint64_t>(UINT64_C(0x1000510100151150)), static_cast<std::uint64_t>(UINT64_C(0x0100040400001144)), static_cast<std::uint64_t>(UINT64_C(0x0000000000000000)),
static_cast<std::uint64_t>(UINT64_C(0x0550004400000100)), static_cast<std::uint64_t>(UINT64_C(0x0151145041451151)), static_cast<std::uint64_t>(UINT64_C(0x0000400400005450)), static_cast<std::uint64_t>(UINT64_C(0x0000100044010004)),
static_cast<std::uint64_t>(UINT64_C(0x0100054100050040)), static_cast<std::uint64_t>(UINT64_C(0x0504400005410010)), static_cast<std::uint64_t>(UINT64_C(0x4011410445500105)), static_cast<std::uint64_t>(UINT64_C(0x0000404000144411)),
static_cast<std::uint64_t>(UINT64_C(0x0101504404500000)), static_cast<std::uint64_t>(UINT64_C(0x0000005044400400)), static_cast<std::uint64_t>(UINT64_C(0x0000000014000100)), static_cast<std::uint64_t>(UINT64_C(0x0404440414000000)),
static_cast<std::uint64_t>(UINT64_C(0x5554100410000140)), static_cast<std::uint64_t>(UINT64_C(0x4555455544505555)), static_cast<std::uint64_t>(UINT64_C(0x5454105055455455)), static_cast<std::uint64_t>(UINT64_C(0x0115454155454015)),
static_cast<std::uint64_t>(UINT64_C(0x4404110000045100)), static_cast<std::uint64_t>(UINT64_C(0x4400001100101501)), static_cast<std::uint64_t>(UINT64_C(0x6596955956966a94)), static_cast<std::uint64_t>(UINT64_C(0x0040655955665965)),
static_cast<std::uint64_t>(UINT64_C(0x5554144400100155)), static_cast<std::uint64_t>(UINT64_C(0xa549495401011041)), static_cast<std::uint64_t>(UINT64_C(0x5596555565955555)), static_cast<std::uint64_t>(UINT64_C(0x5569965959549555)),
static_cast<std::uint64_t>(UINT64_C(0x969565a655555456)), static_cast<std::uint64_t>(UINT64_C(0x0000001000000000)), static_cast<std::uint64_t>(UINT64_C(0x0000000040000140)), static_cast<std::uint64_t>(UINT64_C(0x0000040100000000)),
static_cast<std::uint64_t>(UINT64_C(0x1415454400000000)), static_cast<std::uint64_t>(UINT64_C(0x5410415411454114)), static_cast<std::uint64_t>(UINT64_C(0x0400040104000154)), static_cast<std::uint64_t>(UINT64_C(0x0504045000000411)),
static_cast<std::uint64_t>(UINT64_C(0x0000001000000010)), static_cast<std::uint64_t>(UINT64_C(0x5554000000001040)), static_cast<std::uint64_t>(UINT64_C(0x5549155551556595)), static_cast<std::uint64_t>(UINT64_C(0x1455541055515555)),
static_cast<std::uint64_t>(UINT64_C(0x0510555454554541)), static_cast<std::uint64_t>(UINT64_C(0x9555555555540455)), static_cast<std::uint64_t>(UINT64_C(0x6455456555556465)), static_cast<std::uint64_t>(UINT64_C(0x4524565555654514)),
static_cast<std::uint64_t>(UINT64_C(0x5554655255559545)), static_cast<std::uint64_t>(UINT64_C(0x9555455441155556)), static_cast<std::uint64_t>(UINT64_C(0x0000000051515555)), static_cast<std::uint64_t>(UINT64_C(0x0010005040000550)),
static_cast<std::uint64_t>(UINT64_C(0x5044044040000000)), static_cast<std::uint64_t>(UINT64_C(0x1045040440010500)), static_cast<std::uint64_t>(UINT64_C(0x0000400000040000)), static_cast<std::uint64_t>(UINT64_C(0x0000000000000000))
};

static constexpr std::uint64_t GENERIC_POW5_INV_SPLIT[89][4] = {
{ static_cast<std::uint64_t>(UINT64_C(                   0)), static_cast<std::uint64_t>(UINT64_C(                   0)), static_cast<std::uint64_t>(UINT64_C(                   0)),   static_cast<std::uint64_t>(UINT64_C(144115188075855872)) },
{ static_cast<std::uint64_t>(UINT64_C( 1573859546583440065)), static_cast<std::uint64_t>(UINT64_C( 2691002611772552616)), static_cast<std::uint64_t>(UINT64_C( 6763753280790178510)),   static_cast<std::uint64_t>(UINT64_C(141347765182270746)) },
{ static_cast<std::uint64_t>(UINT64_C(12960290449513840412)), static_cast<std::uint64_t>(UINT64_C(12345512957918226762)), static_cast<std::uint64_t>(UINT64_C(18057899791198622765)),   static_cast<std::uint64_t>(UINT64_C(138633484706040742)) },
{ static_cast<std::uint64_t>(UINT64_C( 7615871757716765416)), static_cast<std::uint64_t>(UINT64_C( 9507132263365501332)), static_cast<std::uint64_t>(UINT64_C( 4879801712092008245)),   static_cast<std::uint64_t>(UINT64_C(135971326161092377)) },
{ static_cast<std::uint64_t>(UINT64_C( 7869961150745287587)), static_cast<std::uint64_t>(UINT64_C( 5804035291554591636)), static_cast<std::uint64_t>(UINT64_C( 8883897266325833928)),   static_cast<std::uint64_t>(UINT64_C(133360288657597085)) },
{ static_cast<std::uint64_t>(UINT64_C( 2942118023529634767)), static_cast<std::uint64_t>(UINT64_C(15128191429820565086)), static_cast<std::uint64_t>(UINT64_C(10638459445243230718)),   static_cast<std::uint64_t>(UINT64_C(130799390525667397)) },
{ static_cast<std::uint64_t>(UINT64_C(14188759758411913794)), static_cast<std::uint64_t>(UINT64_C( 5362791266439207815)), static_cast<std::uint64_t>(UINT64_C( 8068821289119264054)),   static_cast<std::uint64_t>(UINT64_C(128287668946279217)) },
{ static_cast<std::uint64_t>(UINT64_C( 7183196927902545212)), static_cast<std::uint64_t>(UINT64_C( 1952291723540117099)), static_cast<std::uint64_t>(UINT64_C(12075928209936341512)),   static_cast<std::uint64_t>(UINT64_C(125824179589281448)) },
{ static_cast<std::uint64_t>(UINT64_C( 5672588001402349748)), static_cast<std::uint64_t>(UINT64_C(17892323620748423487)), static_cast<std::uint64_t>(UINT64_C( 9874578446960390364)),   static_cast<std::uint64_t>(UINT64_C(123407996258356868)) },
{ static_cast<std::uint64_t>(UINT64_C( 4442590541217566325)), static_cast<std::uint64_t>(UINT64_C( 4558254706293456445)), static_cast<std::uint64_t>(UINT64_C(10343828952663182727)),   static_cast<std::uint64_t>(UINT64_C(121038210542800766)) },
{ static_cast<std::uint64_t>(UINT64_C( 3005560928406962566)), static_cast<std::uint64_t>(UINT64_C( 2082271027139057888)), static_cast<std::uint64_t>(UINT64_C(13961184524927245081)),   static_cast<std::uint64_t>(UINT64_C(118713931475986426)) },
{ static_cast<std::uint64_t>(UINT64_C(13299058168408384786)), static_cast<std::uint64_t>(UINT64_C(17834349496131278595)), static_cast<std::uint64_t>(UINT64_C( 9029906103900731664)),   static_cast<std::uint64_t>(UINT64_C(116434285200389047)) },
{ static_cast<std::uint64_t>(UINT64_C( 5414878118283973035)), static_cast<std::uint64_t>(UINT64_C(13079825470227392078)), static_cast<std::uint64_t>(UINT64_C(17897304791683760280)),   static_cast<std::uint64_t>(UINT64_C(114198414639042157)) },
{ static_cast<std::uint64_t>(UINT64_C(14609755883382484834)), static_cast<std::uint64_t>(UINT64_C(14991702445765844156)), static_cast<std::uint64_t>(UINT64_C( 3269802549772755411)),   static_cast<std::uint64_t>(UINT64_C(112005479173303009)) },
{ static_cast<std::uint64_t>(UINT64_C(15967774957605076027)), static_cast<std::uint64_t>(UINT64_C( 2511532636717499923)), static_cast<std::uint64_t>(UINT64_C(16221038267832563171)),   static_cast<std::uint64_t>(UINT64_C(109854654326805788)) },
{ static_cast<std::uint64_t>(UINT64_C( 9269330061621627145)), static_cast<std::uint64_t>(UINT64_C( 3332501053426257392)), static_cast<std::uint64_t>(UINT64_C(16223281189403734630)),   static_cast<std::uint64_t>(UINT64_C(107745131455483836)) },
{ static_cast<std::uint64_t>(UINT64_C(16739559299223642282)), static_cast<std::uint64_t>(UINT64_C( 1873986623300664530)), static_cast<std::uint64_t>(UINT64_C( 6546709159471442872)),   static_cast<std::uint64_t>(UINT64_C(105676117443544318)) },
{ static_cast<std::uint64_t>(UINT64_C(17116435360051202055)), static_cast<std::uint64_t>(UINT64_C( 1359075105581853924)), static_cast<std::uint64_t>(UINT64_C( 2038341371621886470)),   static_cast<std::uint64_t>(UINT64_C(103646834405281051)) },
{ static_cast<std::uint64_t>(UINT64_C(17144715798009627550)), static_cast<std::uint64_t>(UINT64_C( 3201623802661132408)), static_cast<std::uint64_t>(UINT64_C( 9757551605154622431)),   static_cast<std::uint64_t>(UINT64_C(101656519392613377)) },
{ static_cast<std::uint64_t>(UINT64_C(17580479792687825857)), static_cast<std::uint64_t>(UINT64_C( 6546633380567327312)), static_cast<std::uint64_t>(UINT64_C(15099972427870912398)),   static_cast<std::uint64_t>(UINT64_C( 99704424108241124)) },
{ static_cast<std::uint64_t>(UINT64_C( 9726477118325522902)), static_cast<std::uint64_t>(UINT64_C(14578369026754005435)), static_cast<std::uint64_t>(UINT64_C(11728055595254428803)),   static_cast<std::uint64_t>(UINT64_C( 97789814624307808)) },
{ static_cast<std::uint64_t>(UINT64_C(  134593949518343635)), static_cast<std::uint64_t>(UINT64_C( 5715151379816901985)), static_cast<std::uint64_t>(UINT64_C( 1660163707976377376)),   static_cast<std::uint64_t>(UINT64_C( 95911971106466306)) },
{ static_cast<std::uint64_t>(UINT64_C( 5515914027713859358)), static_cast<std::uint64_t>(UINT64_C( 7124354893273815720)), static_cast<std::uint64_t>(UINT64_C( 5548463282858794077)),   static_cast<std::uint64_t>(UINT64_C( 94070187543243255)) },
{ static_cast<std::uint64_t>(UINT64_C( 6188403395862945512)), static_cast<std::uint64_t>(UINT64_C( 5681264392632320838)), static_cast<std::uint64_t>(UINT64_C(15417410852121406654)),   static_cast<std::uint64_t>(UINT64_C( 92263771480600430)) },
{ static_cast<std::uint64_t>(UINT64_C(15908890877468271457)), static_cast<std::uint64_t>(UINT64_C(10398888261125597540)), static_cast<std::uint64_t>(UINT64_C( 4817794962769172309)),   static_cast<std::uint64_t>(UINT64_C( 90492043761593298)) },
{ static_cast<std::uint64_t>(UINT64_C( 1413077535082201005)), static_cast<std::uint64_t>(UINT64_C(12675058125384151580)), static_cast<std::uint64_t>(UINT64_C( 7731426132303759597)),   static_cast<std::uint64_t>(UINT64_C( 88754338271028867)) },
{ static_cast<std::uint64_t>(UINT64_C( 1486733163972670293)), static_cast<std::uint64_t>(UINT64_C(11369385300195092554)), static_cast<std::uint64_t>(UINT64_C(11610016711694864110)),   static_cast<std::uint64_t>(UINT64_C( 87050001685026843)) },
{ static_cast<std::uint64_t>(UINT64_C( 8788596583757589684)), static_cast<std::uint64_t>(UINT64_C( 3978580923851924802)), static_cast<std::uint64_t>(UINT64_C( 9255162428306775812)),   static_cast<std::uint64_t>(UINT64_C( 85378393225389919)) },
{ static_cast<std::uint64_t>(UINT64_C( 7203518319660962120)), static_cast<std::uint64_t>(UINT64_C(15044736224407683725)), static_cast<std::uint64_t>(UINT64_C( 2488132019818199792)),   static_cast<std::uint64_t>(UINT64_C( 83738884418690858)) },
{ static_cast<std::uint64_t>(UINT64_C( 4004175967662388707)), static_cast<std::uint64_t>(UINT64_C(18236988667757575407)), static_cast<std::uint64_t>(UINT64_C(15613100370957482671)),   static_cast<std::uint64_t>(UINT64_C( 82130858859985791)) },
{ static_cast<std::uint64_t>(UINT64_C(18371903370586036463)), static_cast<std::uint64_t>(UINT64_C(   53497579022921640)), static_cast<std::uint64_t>(UINT64_C(16465963977267203307)),   static_cast<std::uint64_t>(UINT64_C( 80553711981064899)) },
{ static_cast<std::uint64_t>(UINT64_C(10170778323887491315)), static_cast<std::uint64_t>(UINT64_C( 1999668801648976001)), static_cast<std::uint64_t>(UINT64_C(10209763593579456445)),   static_cast<std::uint64_t>(UINT64_C( 79006850823153334)) },
{ static_cast<std::uint64_t>(UINT64_C(17108131712433974546)), static_cast<std::uint64_t>(UINT64_C(16825784443029944237)), static_cast<std::uint64_t>(UINT64_C( 2078700786753338945)),   static_cast<std::uint64_t>(UINT64_C( 77489693813976938)) },
{ static_cast<std::uint64_t>(UINT64_C(17221789422665858532)), static_cast<std::uint64_t>(UINT64_C(12145427517550446164)), static_cast<std::uint64_t>(UINT64_C( 5391414622238668005)),   static_cast<std::uint64_t>(UINT64_C( 76001670549108934)) },
{ static_cast<std::uint64_t>(UINT64_C( 4859588996898795878)), static_cast<std::uint64_t>(UINT64_C( 1715798948121313204)), static_cast<std::uint64_t>(UINT64_C( 3950858167455137171)),   static_cast<std::uint64_t>(UINT64_C( 74542221577515387)) },
{ static_cast<std::uint64_t>(UINT64_C(13513469241795711526)), static_cast<std::uint64_t>(UINT64_C(  631367850494860526)), static_cast<std::uint64_t>(UINT64_C(10517278915021816160)),   static_cast<std::uint64_t>(UINT64_C( 73110798191218799)) },
{ static_cast<std::uint64_t>(UINT64_C(11757513142672073111)), static_cast<std::uint64_t>(UINT64_C( 2581974932255022228)), static_cast<std::uint64_t>(UINT64_C(17498959383193606459)),   static_cast<std::uint64_t>(UINT64_C(143413724438001539)) },
{ static_cast<std::uint64_t>(UINT64_C(14524355192525042817)), static_cast<std::uint64_t>(UINT64_C( 5640643347559376447)), static_cast<std::uint64_t>(UINT64_C( 1309659274756813016)),   static_cast<std::uint64_t>(UINT64_C(140659771648132296)) },
{ static_cast<std::uint64_t>(UINT64_C( 2765095348461978538)), static_cast<std::uint64_t>(UINT64_C(11021111021896007722)), static_cast<std::uint64_t>(UINT64_C( 3224303603779962366)),   static_cast<std::uint64_t>(UINT64_C(137958702611185230)) },
{ static_cast<std::uint64_t>(UINT64_C(12373410389187981037)), static_cast<std::uint64_t>(UINT64_C(13679193545685856195)), static_cast<std::uint64_t>(UINT64_C(11644609038462631561)),   static_cast<std::uint64_t>(UINT64_C(135309501808182158)) },
{ static_cast<std::uint64_t>(UINT64_C(12813176257562780151)), static_cast<std::uint64_t>(UINT64_C( 3754199046160268020)), static_cast<std::uint64_t>(UINT64_C( 9954691079802960722)),   static_cast<std::uint64_t>(UINT64_C(132711173221007413)) },
{ static_cast<std::uint64_t>(UINT64_C(17557452279667723458)), static_cast<std::uint64_t>(UINT64_C( 3237799193992485824)), static_cast<std::uint64_t>(UINT64_C(17893947919029030695)),   static_cast<std::uint64_t>(UINT64_C(130162739957935629)) },
{ static_cast<std::uint64_t>(UINT64_C(14634200999559435155)), static_cast<std::uint64_t>(UINT64_C( 4123869946105211004)), static_cast<std::uint64_t>(UINT64_C( 6955301747350769239)),   static_cast<std::uint64_t>(UINT64_C(127663243886350468)) },
{ static_cast<std::uint64_t>(UINT64_C( 2185352760627740240)), static_cast<std::uint64_t>(UINT64_C( 2864813346878886844)), static_cast<std::uint64_t>(UINT64_C(13049218671329690184)),   static_cast<std::uint64_t>(UINT64_C(125211745272516185)) },
{ static_cast<std::uint64_t>(UINT64_C( 6143438674322183002)), static_cast<std::uint64_t>(UINT64_C(10464733336980678750)), static_cast<std::uint64_t>(UINT64_C( 6982925169933978309)),   static_cast<std::uint64_t>(UINT64_C(122807322428266620)) },
{ static_cast<std::uint64_t>(UINT64_C( 1099509117817174576)), static_cast<std::uint64_t>(UINT64_C(10202656147550524081)), static_cast<std::uint64_t>(UINT64_C(  754997032816608484)),   static_cast<std::uint64_t>(UINT64_C(120449071364478757)) },
{ static_cast<std::uint64_t>(UINT64_C( 2410631293559367023)), static_cast<std::uint64_t>(UINT64_C(17407273750261453804)), static_cast<std::uint64_t>(UINT64_C(15307291918933463037)),   static_cast<std::uint64_t>(UINT64_C(118136105451200587)) },
{ static_cast<std::uint64_t>(UINT64_C(12224968375134586697)), static_cast<std::uint64_t>(UINT64_C( 1664436604907828062)), static_cast<std::uint64_t>(UINT64_C(11506086230137787358)),   static_cast<std::uint64_t>(UINT64_C(115867555084305488)) },
{ static_cast<std::uint64_t>(UINT64_C( 3495926216898000888)), static_cast<std::uint64_t>(UINT64_C(18392536965197424288)), static_cast<std::uint64_t>(UINT64_C(10992889188570643156)),   static_cast<std::uint64_t>(UINT64_C(113642567358547782)) },
{ static_cast<std::uint64_t>(UINT64_C( 8744506286256259680)), static_cast<std::uint64_t>(UINT64_C( 3966568369496879937)), static_cast<std::uint64_t>(UINT64_C(18342264969761820037)),   static_cast<std::uint64_t>(UINT64_C(111460305746896569)) },
{ static_cast<std::uint64_t>(UINT64_C( 7689600520560455039)), static_cast<std::uint64_t>(UINT64_C( 5254331190877624630)), static_cast<std::uint64_t>(UINT64_C( 9628558080573245556)),   static_cast<std::uint64_t>(UINT64_C(109319949786027263)) },
{ static_cast<std::uint64_t>(UINT64_C(11862637625618819436)), static_cast<std::uint64_t>(UINT64_C( 3456120362318976488)), static_cast<std::uint64_t>(UINT64_C(14690471063106001082)),   static_cast<std::uint64_t>(UINT64_C(107220694767852583)) },
{ static_cast<std::uint64_t>(UINT64_C( 5697330450030126444)), static_cast<std::uint64_t>(UINT64_C(12424082405392918899)), static_cast<std::uint64_t>(UINT64_C(  358204170751754904)),   static_cast<std::uint64_t>(UINT64_C(105161751436977040)) },
{ static_cast<std::uint64_t>(UINT64_C(11257457505097373622)), static_cast<std::uint64_t>(UINT64_C(15373192700214208870)), static_cast<std::uint64_t>(UINT64_C(  671619062372033814)),   static_cast<std::uint64_t>(UINT64_C(103142345693961148)) },
{ static_cast<std::uint64_t>(UINT64_C(16850355018477166700)), static_cast<std::uint64_t>(UINT64_C( 1913910419361963966)), static_cast<std::uint64_t>(UINT64_C( 4550257919755970531)),   static_cast<std::uint64_t>(UINT64_C(101161718304283822)) },
{ static_cast<std::uint64_t>(UINT64_C( 9670835567561997011)), static_cast<std::uint64_t>(UINT64_C(10584031339132130638)), static_cast<std::uint64_t>(UINT64_C( 3060560222974851757)),   static_cast<std::uint64_t>(UINT64_C( 99219124612893520)) },
{ static_cast<std::uint64_t>(UINT64_C( 7698686577353054710)), static_cast<std::uint64_t>(UINT64_C(11689292838639130817)), static_cast<std::uint64_t>(UINT64_C(11806331021588878241)),   static_cast<std::uint64_t>(UINT64_C( 97313834264240819)) },
{ static_cast<std::uint64_t>(UINT64_C(12233569599615692137)), static_cast<std::uint64_t>(UINT64_C( 3347791226108469959)), static_cast<std::uint64_t>(UINT64_C(10333904326094451110)),   static_cast<std::uint64_t>(UINT64_C( 95445130927687169)) },
{ static_cast<std::uint64_t>(UINT64_C(13049400362825383933)), static_cast<std::uint64_t>(UINT64_C(17142621313007799680)), static_cast<std::uint64_t>(UINT64_C( 3790542585289224168)),   static_cast<std::uint64_t>(UINT64_C( 93612312028186576)) },
{ static_cast<std::uint64_t>(UINT64_C(12430457242474442072)), static_cast<std::uint64_t>(UINT64_C( 5625077542189557960)), static_cast<std::uint64_t>(UINT64_C(14765055286236672238)),   static_cast<std::uint64_t>(UINT64_C( 91814688482138969)) },
{ static_cast<std::uint64_t>(UINT64_C( 4759444137752473128)), static_cast<std::uint64_t>(UINT64_C( 2230562561567025078)), static_cast<std::uint64_t>(UINT64_C( 4954443037339580076)),   static_cast<std::uint64_t>(UINT64_C( 90051584438315940)) },
{ static_cast<std::uint64_t>(UINT64_C( 7246913525170274758)), static_cast<std::uint64_t>(UINT64_C( 8910297835195760709)), static_cast<std::uint64_t>(UINT64_C( 4015904029508858381)),   static_cast<std::uint64_t>(UINT64_C( 88322337023761438)) },
{ static_cast<std::uint64_t>(UINT64_C(12854430245836432067)), static_cast<std::uint64_t>(UINT64_C( 8135139748065431455)), static_cast<std::uint64_t>(UINT64_C(11548083631386317976)),   static_cast<std::uint64_t>(UINT64_C( 86626296094571907)) },
{ static_cast<std::uint64_t>(UINT64_C( 4848827254502687803)), static_cast<std::uint64_t>(UINT64_C( 4789491250196085625)), static_cast<std::uint64_t>(UINT64_C( 3988192420450664125)),   static_cast<std::uint64_t>(UINT64_C( 84962823991462151)) },
{ static_cast<std::uint64_t>(UINT64_C( 7435538409611286684)), static_cast<std::uint64_t>(UINT64_C(  904061756819742353)), static_cast<std::uint64_t>(UINT64_C(14598026519493048444)),   static_cast<std::uint64_t>(UINT64_C( 83331295300025028)) },
{ static_cast<std::uint64_t>(UINT64_C(11042616160352530997)), static_cast<std::uint64_t>(UINT64_C( 8948390828345326218)), static_cast<std::uint64_t>(UINT64_C(10052651191118271927)),   static_cast<std::uint64_t>(UINT64_C( 81731096615594853)) },
{ static_cast<std::uint64_t>(UINT64_C(11059348291563778943)), static_cast<std::uint64_t>(UINT64_C(11696515766184685544)), static_cast<std::uint64_t>(UINT64_C( 3783210511290897367)),   static_cast<std::uint64_t>(UINT64_C( 80161626312626082)) },
{ static_cast<std::uint64_t>(UINT64_C( 7020010856491885826)), static_cast<std::uint64_t>(UINT64_C( 5025093219346041680)), static_cast<std::uint64_t>(UINT64_C( 8960210401638911765)),   static_cast<std::uint64_t>(UINT64_C( 78622294318500592)) },
{ static_cast<std::uint64_t>(UINT64_C(17732844474490699984)), static_cast<std::uint64_t>(UINT64_C( 7820866704994446502)), static_cast<std::uint64_t>(UINT64_C( 6088373186798844243)),   static_cast<std::uint64_t>(UINT64_C( 77112521891678506)) },
{ static_cast<std::uint64_t>(UINT64_C(  688278527545590501)), static_cast<std::uint64_t>(UINT64_C( 3045610706602776618)), static_cast<std::uint64_t>(UINT64_C( 8684243536999567610)),   static_cast<std::uint64_t>(UINT64_C( 75631741404109150)) },
{ static_cast<std::uint64_t>(UINT64_C( 2734573255120657297)), static_cast<std::uint64_t>(UINT64_C( 3903146411440697663)), static_cast<std::uint64_t>(UINT64_C( 9470794821691856713)),   static_cast<std::uint64_t>(UINT64_C( 74179396127820347)) },
{ static_cast<std::uint64_t>(UINT64_C(15996457521023071259)), static_cast<std::uint64_t>(UINT64_C( 4776627823451271680)), static_cast<std::uint64_t>(UINT64_C(12394856457265744744)),   static_cast<std::uint64_t>(UINT64_C( 72754940025605801)) },
{ static_cast<std::uint64_t>(UINT64_C(13492065758834518331)), static_cast<std::uint64_t>(UINT64_C( 7390517611012222399)), static_cast<std::uint64_t>(UINT64_C( 1630485387832860230)),   static_cast<std::uint64_t>(UINT64_C(142715675091463768)) },
{ static_cast<std::uint64_t>(UINT64_C(13665021627282055864)), static_cast<std::uint64_t>(UINT64_C( 9897834675523659302)), static_cast<std::uint64_t>(UINT64_C(17907668136755296849)),   static_cast<std::uint64_t>(UINT64_C(139975126841173266)) },
{ static_cast<std::uint64_t>(UINT64_C( 9603773719399446181)), static_cast<std::uint64_t>(UINT64_C(10771916301484339398)), static_cast<std::uint64_t>(UINT64_C(10672699855989487527)),   static_cast<std::uint64_t>(UINT64_C(137287204938390542)) },
{ static_cast<std::uint64_t>(UINT64_C( 3630218541553511265)), static_cast<std::uint64_t>(UINT64_C( 8139010004241080614)), static_cast<std::uint64_t>(UINT64_C( 2876479648932814543)),   static_cast<std::uint64_t>(UINT64_C(134650898807055963)) },
{ static_cast<std::uint64_t>(UINT64_C( 8318835909686377084)), static_cast<std::uint64_t>(UINT64_C( 9525369258927993371)), static_cast<std::uint64_t>(UINT64_C( 2796120270400437057)),   static_cast<std::uint64_t>(UINT64_C(132065217277054270)) },
{ static_cast<std::uint64_t>(UINT64_C(11190003059043290163)), static_cast<std::uint64_t>(UINT64_C(12424345635599592110)), static_cast<std::uint64_t>(UINT64_C(12539346395388933763)),   static_cast<std::uint64_t>(UINT64_C(129529188211565064)) },
{ static_cast<std::uint64_t>(UINT64_C( 8701968833973242276)), static_cast<std::uint64_t>(UINT64_C(  820569587086330727)), static_cast<std::uint64_t>(UINT64_C( 2315591597351480110)),   static_cast<std::uint64_t>(UINT64_C(127041858141569228)) },
{ static_cast<std::uint64_t>(UINT64_C( 5115113890115690487)), static_cast<std::uint64_t>(UINT64_C(16906305245394587826)), static_cast<std::uint64_t>(UINT64_C( 9899749468931071388)),   static_cast<std::uint64_t>(UINT64_C(124602291907373862)) },
{ static_cast<std::uint64_t>(UINT64_C(15543535488939245974)), static_cast<std::uint64_t>(UINT64_C(10945189844466391399)), static_cast<std::uint64_t>(UINT64_C( 3553863472349432246)),   static_cast<std::uint64_t>(UINT64_C(122209572307020975)) },
{ static_cast<std::uint64_t>(UINT64_C( 7709257252608325038)), static_cast<std::uint64_t>(UINT64_C( 1191832167690640880)), static_cast<std::uint64_t>(UINT64_C(15077137020234258537)),   static_cast<std::uint64_t>(UINT64_C(119862799751447719)) },
{ static_cast<std::uint64_t>(UINT64_C( 7541333244210021737)), static_cast<std::uint64_t>(UINT64_C( 9790054727902174575)), static_cast<std::uint64_t>(UINT64_C( 5160944773155322014)),   static_cast<std::uint64_t>(UINT64_C(117561091926268545)) },
{ static_cast<std::uint64_t>(UINT64_C(12297384708782857832)), static_cast<std::uint64_t>(UINT64_C( 1281328873123467374)), static_cast<std::uint64_t>(UINT64_C( 4827925254630475769)),   static_cast<std::uint64_t>(UINT64_C(115303583460052092)) },
{ static_cast<std::uint64_t>(UINT64_C(13243237906232367265)), static_cast<std::uint64_t>(UINT64_C(15873887428139547641)), static_cast<std::uint64_t>(UINT64_C( 3607993172301799599)),   static_cast<std::uint64_t>(UINT64_C(113089425598968120)) },
{ static_cast<std::uint64_t>(UINT64_C(11384616453739611114)), static_cast<std::uint64_t>(UINT64_C(15184114243769211033)), static_cast<std::uint64_t>(UINT64_C(13148448124803481057)),   static_cast<std::uint64_t>(UINT64_C(110917785887682141)) },
{ static_cast<std::uint64_t>(UINT64_C(17727970963596660683)), static_cast<std::uint64_t>(UINT64_C( 1196965221832671990)), static_cast<std::uint64_t>(UINT64_C(14537830463956404138)),   static_cast<std::uint64_t>(UINT64_C(108787847856377790)) },
{ static_cast<std::uint64_t>(UINT64_C(17241367586707330931)), static_cast<std::uint64_t>(UINT64_C( 8880584684128262874)), static_cast<std::uint64_t>(UINT64_C(11173506540726547818)),   static_cast<std::uint64_t>(UINT64_C(106698810713789254)) },
{ static_cast<std::uint64_t>(UINT64_C( 7184427196661305643)), static_cast<std::uint64_t>(UINT64_C(14332510582433188173)), static_cast<std::uint64_t>(UINT64_C(14230167953789677901)),   static_cast<std::uint64_t>(UINT64_C(104649889046128358)) }
};

static constexpr std::uint64_t POW5_INV_ERRORS[154] = {
static_cast<std::uint64_t>(UINT64_C(0x1144155514145504)), static_cast<std::uint64_t>(UINT64_C(0x0000541555401141)), static_cast<std::uint64_t>(UINT64_C(0x0000000000000000)), static_cast<std::uint64_t>(UINT64_C(0x0154454000000000)),
static_cast<std::uint64_t>(UINT64_C(0x4114105515544440)), static_cast<std::uint64_t>(UINT64_C(0x0001001111500415)), static_cast<std::uint64_t>(UINT64_C(0x4041411410011000)), static_cast<std::uint64_t>(UINT64_C(0x5550114515155014)),
static_cast<std::uint64_t>(UINT64_C(0x1404100041554551)), static_cast<std::uint64_t>(UINT64_C(0x0515000450404410)), static_cast<std::uint64_t>(UINT64_C(0x5054544401140004)), static_cast<std::uint64_t>(UINT64_C(0x5155501005555105)),
static_cast<std::uint64_t>(UINT64_C(0x1144141000105515)), static_cast<std::uint64_t>(UINT64_C(0x0541500000500000)), static_cast<std::uint64_t>(UINT64_C(0x1104105540444140)), static_cast<std::uint64_t>(UINT64_C(0x4000015055514110)),
static_cast<std::uint64_t>(UINT64_C(0x0054010450004005)), static_cast<std::uint64_t>(UINT64_C(0x4155515404100005)), static_cast<std::uint64_t>(UINT64_C(0x5155145045155555)), static_cast<std::uint64_t>(UINT64_C(0x1511555515440558)),
static_cast<std::uint64_t>(UINT64_C(0x5558544555515555)), static_cast<std::uint64_t>(UINT64_C(0x0000000000000010)), static_cast<std::uint64_t>(UINT64_C(0x5004000000000050)), static_cast<std::uint64_t>(UINT64_C(0x1415510100000010)),
static_cast<std::uint64_t>(UINT64_C(0x4545555444514500)), static_cast<std::uint64_t>(UINT64_C(0x5155151555555551)), static_cast<std::uint64_t>(UINT64_C(0x1441540144044554)), static_cast<std::uint64_t>(UINT64_C(0x5150104045544400)),
static_cast<std::uint64_t>(UINT64_C(0x5450545401444040)), static_cast<std::uint64_t>(UINT64_C(0x5554455045501400)), static_cast<std::uint64_t>(UINT64_C(0x4655155555555145)), static_cast<std::uint64_t>(UINT64_C(0x1000010055455055)),
static_cast<std::uint64_t>(UINT64_C(0x1000004000055004)), static_cast<std::uint64_t>(UINT64_C(0x4455405104000005)), static_cast<std::uint64_t>(UINT64_C(0x4500114504150545)), static_cast<std::uint64_t>(UINT64_C(0x0000000014000000)),
static_cast<std::uint64_t>(UINT64_C(0x5450000000000000)), static_cast<std::uint64_t>(UINT64_C(0x5514551511445555)), static_cast<std::uint64_t>(UINT64_C(0x4111501040555451)), static_cast<std::uint64_t>(UINT64_C(0x4515445500054444)),
static_cast<std::uint64_t>(UINT64_C(0x5101500104100441)), static_cast<std::uint64_t>(UINT64_C(0x1545115155545055)), static_cast<std::uint64_t>(UINT64_C(0x0000000000000000)), static_cast<std::uint64_t>(UINT64_C(0x1554000000100000)),
static_cast<std::uint64_t>(UINT64_C(0x5555545595551555)), static_cast<std::uint64_t>(UINT64_C(0x5555051851455955)), static_cast<std::uint64_t>(UINT64_C(0x5555555555555559)), static_cast<std::uint64_t>(UINT64_C(0x0000400011001555)),
static_cast<std::uint64_t>(UINT64_C(0x0000004400040000)), static_cast<std::uint64_t>(UINT64_C(0x5455511555554554)), static_cast<std::uint64_t>(UINT64_C(0x5614555544115445)), static_cast<std::uint64_t>(UINT64_C(0x6455156145555155)),
static_cast<std::uint64_t>(UINT64_C(0x5455855455415455)), static_cast<std::uint64_t>(UINT64_C(0x5515555144555545)), static_cast<std::uint64_t>(UINT64_C(0x0114400000145155)), static_cast<std::uint64_t>(UINT64_C(0x0000051000450511)),
static_cast<std::uint64_t>(UINT64_C(0x4455154554445100)), static_cast<std::uint64_t>(UINT64_C(0x4554150141544455)), static_cast<std::uint64_t>(UINT64_C(0x65955555559a5965)), static_cast<std::uint64_t>(UINT64_C(0x5555555854559559)),
static_cast<std::uint64_t>(UINT64_C(0x9569654559616595)), static_cast<std::uint64_t>(UINT64_C(0x1040044040005565)), static_cast<std::uint64_t>(UINT64_C(0x1010010500011044)), static_cast<std::uint64_t>(UINT64_C(0x1554015545154540)),
static_cast<std::uint64_t>(UINT64_C(0x4440555401545441)), static_cast<std::uint64_t>(UINT64_C(0x1014441450550105)), static_cast<std::uint64_t>(UINT64_C(0x4545400410504145)), static_cast<std::uint64_t>(UINT64_C(0x5015111541040151)),
static_cast<std::uint64_t>(UINT64_C(0x5145051154000410)), static_cast<std::uint64_t>(UINT64_C(0x1040001044545044)), static_cast<std::uint64_t>(UINT64_C(0x4001400000151410)), static_cast<std::uint64_t>(UINT64_C(0x0540000044040000)),
static_cast<std::uint64_t>(UINT64_C(0x0510555454411544)), static_cast<std::uint64_t>(UINT64_C(0x0400054054141550)), static_cast<std::uint64_t>(UINT64_C(0x1001041145001100)), static_cast<std::uint64_t>(UINT64_C(0x0000000140000000)),
static_cast<std::uint64_t>(UINT64_C(0x0000000014100000)), static_cast<std::uint64_t>(UINT64_C(0x1544005454000140)), static_cast<std::uint64_t>(UINT64_C(0x4050055505445145)), static_cast<std::uint64_t>(UINT64_C(0x0011511104504155)),
static_cast<std::uint64_t>(UINT64_C(0x5505544415045055)), static_cast<std::uint64_t>(UINT64_C(0x1155154445515554)), static_cast<std::uint64_t>(UINT64_C(0x0000000000004555)), static_cast<std::uint64_t>(UINT64_C(0x0000000000000000)),
static_cast<std::uint64_t>(UINT64_C(0x5101010510400004)), static_cast<std::uint64_t>(UINT64_C(0x1514045044440400)), static_cast<std::uint64_t>(UINT64_C(0x5515519555515555)), static_cast<std::uint64_t>(UINT64_C(0x4554545441555545)),
static_cast<std::uint64_t>(UINT64_C(0x1551055955551515)), static_cast<std::uint64_t>(UINT64_C(0x0150000011505515)), static_cast<std::uint64_t>(UINT64_C(0x0044005040400000)), static_cast<std::uint64_t>(UINT64_C(0x0004001004010050)),
static_cast<std::uint64_t>(UINT64_C(0x0000051004450414)), static_cast<std::uint64_t>(UINT64_C(0x0114001101001144)), static_cast<std::uint64_t>(UINT64_C(0x0401000001000001)), static_cast<std::uint64_t>(UINT64_C(0x4500010001000401)),
static_cast<std::uint64_t>(UINT64_C(0x0004100000005000)), static_cast<std::uint64_t>(UINT64_C(0x0105000441101100)), static_cast<std::uint64_t>(UINT64_C(0x0455455550454540)), static_cast<std::uint64_t>(UINT64_C(0x5404050144105505)),
static_cast<std::uint64_t>(UINT64_C(0x4101510540555455)), static_cast<std::uint64_t>(UINT64_C(0x1055541411451555)), static_cast<std::uint64_t>(UINT64_C(0x5451445110115505)), static_cast<std::uint64_t>(UINT64_C(0x1154110010101545)),
static_cast<std::uint64_t>(UINT64_C(0x1145140450054055)), static_cast<std::uint64_t>(UINT64_C(0x5555565415551554)), static_cast<std::uint64_t>(UINT64_C(0x1550559555555555)), static_cast<std::uint64_t>(UINT64_C(0x5555541545045141)),
static_cast<std::uint64_t>(UINT64_C(0x4555455450500100)), static_cast<std::uint64_t>(UINT64_C(0x5510454545554555)), static_cast<std::uint64_t>(UINT64_C(0x1510140115045455)), static_cast<std::uint64_t>(UINT64_C(0x1001050040111510)),
static_cast<std::uint64_t>(UINT64_C(0x5555454555555504)), static_cast<std::uint64_t>(UINT64_C(0x9954155545515554)), static_cast<std::uint64_t>(UINT64_C(0x6596656555555555)), static_cast<std::uint64_t>(UINT64_C(0x0140410051555559)),
static_cast<std::uint64_t>(UINT64_C(0x0011104010001544)), static_cast<std::uint64_t>(UINT64_C(0x965669659a680501)), static_cast<std::uint64_t>(UINT64_C(0x5655a55955556955)), static_cast<std::uint64_t>(UINT64_C(0x4015111014404514)),
static_cast<std::uint64_t>(UINT64_C(0x1414155554505145)), static_cast<std::uint64_t>(UINT64_C(0x0540040011051404)), static_cast<std::uint64_t>(UINT64_C(0x1010000000015005)), static_cast<std::uint64_t>(UINT64_C(0x0010054050004410)),
static_cast<std::uint64_t>(UINT64_C(0x5041104014000100)), static_cast<std::uint64_t>(UINT64_C(0x4440010500100001)), static_cast<std::uint64_t>(UINT64_C(0x1155510504545554)), static_cast<std::uint64_t>(UINT64_C(0x0450151545115541)),
static_cast<std::uint64_t>(UINT64_C(0x4000100400110440)), static_cast<std::uint64_t>(UINT64_C(0x1004440010514440)), static_cast<std::uint64_t>(UINT64_C(0x0000115050450000)), static_cast<std::uint64_t>(UINT64_C(0x0545404455541500)),
static_cast<std::uint64_t>(UINT64_C(0x1051051555505101)), static_cast<std::uint64_t>(UINT64_C(0x5505144554544144)), static_cast<std::uint64_t>(UINT64_C(0x4550545555515550)), static_cast<std::uint64_t>(UINT64_C(0x0015400450045445)),
static_cast<std::uint64_t>(UINT64_C(0x4514155400554415)), static_cast<std::uint64_t>(UINT64_C(0x4555055051050151)), static_cast<std::uint64_t>(UINT64_C(0x1511441450001014)), static_cast<std::uint64_t>(UINT64_C(0x4544554510404414)),
static_cast<std::uint64_t>(UINT64_C(0x4115115545545450)), static_cast<std::uint64_t>(UINT64_C(0x5500541555551555)), static_cast<std::uint64_t>(UINT64_C(0x5550010544155015)), static_cast<std::uint64_t>(UINT64_C(0x0144414045545500)),
static_cast<std::uint64_t>(UINT64_C(0x4154050001050150)), static_cast<std::uint64_t>(UINT64_C(0x5550511111000145)), static_cast<std::uint64_t>(UINT64_C(0x1114504055000151)), static_cast<std::uint64_t>(UINT64_C(0x5104041101451040)),
static_cast<std::uint64_t>(UINT64_C(0x0010501401051441)), static_cast<std::uint64_t>(UINT64_C(0x0010501450504401)), static_cast<std::uint64_t>(UINT64_C(0x4554585440044444)), static_cast<std::uint64_t>(UINT64_C(0x5155555951450455)),
static_cast<std::uint64_t>(UINT64_C(0x0040000400105555)), static_cast<std::uint64_t>(UINT64_C(0x0000000000000001)),
};

// Returns e == 0 ? 1 : ceil(log_2(5^e)); requires 0 <= e <= 32768.
static constexpr auto pow5bits(const std::uint32_t e) noexcept -> std::uint32_t
{
    assert(e <= 1 << 15);
    return static_cast<std::uint32_t>(((e * UINT64_C(163391164108059)) >> 46) + 1);
}

static constexpr
auto mul_128_256_shift(
        const std::uint64_t* const a, const std::uint64_t* const b,
        const std::uint32_t shift, const std::uint32_t corr,
        std::uint64_t* const result) noexcept -> void
{
    assert(shift > 0);
    assert(shift < 256);
    const unsigned_128_type b00 = ((unsigned_128_type) a[0]) * b[0]; // 0
    const unsigned_128_type b01 = ((unsigned_128_type) a[0]) * b[1]; // 64
    const unsigned_128_type b02 = ((unsigned_128_type) a[0]) * b[2]; // 128
    const unsigned_128_type b03 = ((unsigned_128_type) a[0]) * b[3]; // 196
    const unsigned_128_type b10 = ((unsigned_128_type) a[1]) * b[0]; // 64
    const unsigned_128_type b11 = ((unsigned_128_type) a[1]) * b[1]; // 128
    const unsigned_128_type b12 = ((unsigned_128_type) a[1]) * b[2]; // 196
    const unsigned_128_type b13 = ((unsigned_128_type) a[1]) * b[3]; // 256

    const unsigned_128_type s0 = b00;       // 0   x
    const unsigned_128_type s1 = b01 + b10; // 64  x
    const unsigned_128_type c1 = s1 < b01;  // 196 x
    const unsigned_128_type s2 = b02 + b11; // 128 x
    const unsigned_128_type c2 = s2 < b02;  // 256 x
    const unsigned_128_type s3 = b03 + b12; // 196 x
    const unsigned_128_type c3 = s3 < b03;  // 324 x

    const unsigned_128_type p0 = s0 + (s1 << 64);                                // 0
    const unsigned_128_type d0 = p0 < b00;                                       // 128
    const unsigned_128_type q1 = s2 + (s1 >> 64) + (s3 << 64);                   // 128
    const unsigned_128_type d1 = q1 < s2;                                        // 256
    const unsigned_128_type p1 = q1 + (c1 << 64) + d0;                           // 128
    const unsigned_128_type d2 = p1 < q1;                                        // 256
    const unsigned_128_type p2 = b13 + (s3 >> 64) + c2 + (c3 << 64) + d1 + d2;   // 256

    if (shift < 128)
    {
        const unsigned_128_type r0 = corr + ((p0 >> shift) | (p1 << (128 - shift)));
        const unsigned_128_type r1 = ((p1 >> shift) | (p2 << (128 - shift))) + (r0 < corr);
        result[0] = (std::uint64_t) r0;
        result[1] = (std::uint64_t) (r0 >> 64);
        result[2] = (std::uint64_t) r1;
        result[3] = (std::uint64_t) (r1 >> 64);
    }
    else if (shift == 128)
    {
        const unsigned_128_type r0 = corr + p1;
        const unsigned_128_type r1 = p2 + (r0 < corr);
        result[0] = (std::uint64_t) r0;
        result[1] = (std::uint64_t) (r0 >> 64);
        result[2] = (std::uint64_t) r1;
        result[3] = (std::uint64_t) (r1 >> 64);
    }
    else
    {
        const unsigned_128_type r0 = corr + ((p1 >> (shift - 128)) | (p2 << (256 - shift)));
        const unsigned_128_type r1 = (p2 >> (shift - 128)) + (r0 < corr);
        result[0] = (std::uint64_t) r0;
        result[1] = (std::uint64_t) (r0 >> 64);
        result[2] = (std::uint64_t) r1;
        result[3] = (std::uint64_t) (r1 >> 64);
    }
}

// Computes 5^i in the form required by Ryu, and stores it in the given pointer.
static constexpr auto generic_computePow5(const std::uint32_t i, std::uint64_t* const result) noexcept -> void
{
    const std::uint32_t base = i / BOOST_DECIMAL_POW5_TABLE_SIZE;
    const std::uint32_t base2 = base * BOOST_DECIMAL_POW5_TABLE_SIZE;
    const std::uint64_t* const mul = GENERIC_POW5_SPLIT[base];
    if (i == base2)
    {
        result[0] = mul[0];
        result[1] = mul[1];
        result[2] = mul[2];
        result[3] = mul[3];
    }
    else
    {
        const std::uint32_t offset = i - base2;
        const std::uint64_t* const m = GENERIC_POW5_TABLE[offset];
        const std::uint32_t delta = pow5bits(i) - pow5bits(base2);
        const auto corr = (std::uint32_t) ((POW5_ERRORS[i / 32] >> (2 * (i % 32))) & 3);
        mul_128_256_shift(m, mul, delta, corr, result);
    }
}

// Computes 5^-i in the form required by Ryu, and stores it in the given pointer.
static constexpr auto generic_computeInvPow5(const std::uint32_t i, std::uint64_t* const result) noexcept -> void
{
    const std::uint32_t base = (i + BOOST_DECIMAL_POW5_TABLE_SIZE - 1) / BOOST_DECIMAL_POW5_TABLE_SIZE;
    const std::uint32_t base2 = base * BOOST_DECIMAL_POW5_TABLE_SIZE;
    const std::uint64_t* const mul = GENERIC_POW5_INV_SPLIT[base]; // 1/5^base2
    if (i == base2)
    {
        result[0] = mul[0] + 1;
        result[1] = mul[1];
        result[2] = mul[2];
        result[3] = mul[3];
    }
    else
    {
        const std::uint32_t offset = base2 - i;
        const std::uint64_t* const m = GENERIC_POW5_TABLE[offset]; // 5^offset
        const std::uint32_t delta = pow5bits(base2) - pow5bits(i);
        const std::uint32_t corr = (std::uint32_t) ((POW5_INV_ERRORS[i / 32] >> (2 * (i % 32))) & 3) + 1;
        mul_128_256_shift(m, mul, delta, corr, result);
    }
}

static constexpr auto pow5Factor(unsigned_128_type value) noexcept -> std::uint32_t
{
    for (std::uint32_t count = 0; value > 0; ++count)
    {
        if (value % 5 != 0)
        {
            return count;
        }
        value /= 5;
    }
    return 0;
}

// Returns true if value is divisible by 5^p.
static constexpr auto multipleOfPowerOf5(const unsigned_128_type value, const std::uint32_t p) noexcept -> bool
{
    // I tried a case distinction on p, but there was no performance difference.
    return pow5Factor(value) >= p;
}

// Returns true if value is divisible by 2^p.
static constexpr auto multipleOfPowerOf2(const unsigned_128_type value, const std::uint32_t p) noexcept -> bool
{
    return (value & ((((unsigned_128_type) 1) << p) - 1)) == 0;
}

static constexpr
auto mulShift(const unsigned_128_type m, const std::uint64_t* const mul, const int32_t j) noexcept -> unsigned_128_type
{
    assert(j > 128);
    std::uint64_t a[2] {};
    a[0] = (std::uint64_t) m;
    a[1] = (std::uint64_t) (m >> 64);
    std::uint64_t result[4] {};
    mul_128_256_shift(a, mul, j, 0, result);
    return (((unsigned_128_type) result[1]) << 64) | result[0];
}

// Returns floor(log_10(2^e)).
static constexpr auto log10Pow2(const int32_t e) noexcept -> std::uint32_t
{
    // The first value this approximation fails for is 2^1651 which is just greater than 10^297.
    assert(e >= 0);
    assert(e <= 1 << 15);
    return (std::uint32_t) ((((std::uint64_t) e) * UINT64_C(169464822037455)) >> 49);
}

// Returns floor(log_10(5^e)).
static constexpr auto log10Pow5(const int32_t e) noexcept -> std::uint32_t
{
    // The first value this approximation fails for is 5^2621 which is just greater than 10^1832.
    assert(e >= 0);
    assert(e <= 1 << 15);
    return (std::uint32_t) ((((std::uint64_t) e) * UINT64_C(196742565691928)) >> 48);
}

}}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_RYU_GENERIC_128_HPP
