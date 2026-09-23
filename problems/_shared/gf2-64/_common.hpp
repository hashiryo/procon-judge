// 旧 judge (hashiryo/judge) の problems/gf2-64/_shared から移した、GF(2^64) ベンチの共通ヘッダ。
// 提出は ../../_shared/gf2-64/... の相対パスで include する。
#pragma once
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
#include <simde/x86/clmul.h>
#include <simde/x86/bmi.h>
#else
#include <immintrin.h>
#endif
#ifdef __x86_64__
#define GNU_TARGET(x) [[gnu::target(x)]]
#else
#define GNU_TARGET(x)
#endif

// bits/stdc++.h は Apple clang に無いので名指しで include する (旧 judge からの移植)。
// 一覧は Library の include/bits/stdc++.h (macOS 用のシム) と同じ。
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <algorithm>
#include <array>
#include <bitset>
#include <complex>
#include <deque>
#include <exception>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iomanip>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <istream>
#include <iterator>
#include <limits>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <new>
#include <numeric>
#include <ostream>
#include <queue>
#include <random>
#include <regex>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <valarray>
#include <vector>
#include <any>
#include <charconv>
#include <optional>
#include <string_view>
#include <variant>
#include <bit>
#include <compare>
#include <concepts>
#include <numbers>
#include <ranges>
#include <span>
using namespace std;
using u8= unsigned char;
using u16= unsigned short;
using u32= unsigned;
using i64= long long;
using u64= unsigned long long;
using u128= __uint128_t;

// 既約多項式 P(x) = x^64 + x^4 + x^3 + x + 1 (= 0x1B + x^64) を fix。
// 上位 4 bit を含まない 64-bit 表現での "x^64 mod P" の下位 64 bit:
constexpr uint64_t IRRED_LOW= 0x1Bu;
