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
// このベンチの家族が使う x86 の命令 (x86-64-v3 の命令、pclmul、vpclmulqdq) は、環境のオプションで
// 渡す土台に入っているので宣言しない。土台に無い命令 (gfni など) を使う提出は、このヘッダを最初に
// include する前に GF2_64_EXTRA_TARGETS を定義する。ファイルの最後で、以降に定義する関数 (共通ヘッダ、
// 提出、base.cpp) にその命令を付ける領域を開き、base.cpp の最後の GF2_64_TARGET_END で閉じる。
//   #define GF2_64_EXTRA_TARGETS "gfni"
//   #include "_shared/gf2-64/_common.hpp"
// include したあとで clang の attribute push を重ねても効かない。clang は push を入れ子にすると
// 一番外側の target だけを使う。

// 名前空間に置く 256 bit の定数を初期化するためのもの。_mm256_setr_epi8 で初期化すると、その
// 処理はコンパイラが作る関数の中で走る。clang の attribute push はその関数に target を付けない
// ので、土台をオプションで渡さずに組むと (-march なしの clang)、AVX が無いと言われて CE になる。
// x86 では関数を呼ばないベクタのリテラルにして、定数として初期化する。値の並びは _mm256_setr_epi8
// と同じ。構造体の static inline メンバで使う _mm256_set_epi64x と _mm256_set1_epi64x も同じ形で用意する。
#if defined(__x86_64__) && !defined(USE_SIMDE)
typedef char gf2_64_i8x32 __attribute__((vector_size(32)));
#define GF2_64_M256_SETR_EPI8(...) ((__m256i)(gf2_64_i8x32){__VA_ARGS__})
#define GF2_64_M256_SET_EPI64X(e3, e2, e1, e0) ((__m256i){(long long)(e0), (long long)(e1), (long long)(e2), (long long)(e3)})
#define GF2_64_M256_SET1_EPI64X(x) GF2_64_M256_SET_EPI64X(x, x, x, x)
#else
#define GF2_64_M256_SETR_EPI8(...) _mm256_setr_epi8(__VA_ARGS__)
#define GF2_64_M256_SET_EPI64X(e3, e2, e1, e0) _mm256_set_epi64x(e3, e2, e1, e0)
#define GF2_64_M256_SET1_EPI64X(x) _mm256_set1_epi64x(x)
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

// 土台に無い命令の領域を開く。GCC の #pragma GCC target は翻訳単位の終わりまで効く。clang の
// #pragma clang attribute push は翻訳単位の中で pop しないとエラーになるので、各問題の
// base.cpp の最後に GF2_64_TARGET_END を置いて閉じる。clang の push は GCC の pragma と
// 違って __GFNI__ などのマクロを立てないので、家族のコードは機能のマクロで分岐しない
// (x86 かどうかは __x86_64__ で見る)。
#define GF2_64_PRAGMA_(x) _Pragma(#x)
#define GF2_64_PRAGMA(x) GF2_64_PRAGMA_(x)
#if defined(__x86_64__) && !defined(USE_SIMDE) && defined(GF2_64_EXTRA_TARGETS)
#if defined(__clang__)
GF2_64_PRAGMA(clang attribute push(__attribute__((target(GF2_64_EXTRA_TARGETS))), apply_to = function))
#define GF2_64_TARGET_END _Pragma("clang attribute pop")
#else
GF2_64_PRAGMA(GCC target(GF2_64_EXTRA_TARGETS))
#define GF2_64_TARGET_END
#endif
#else
#define GF2_64_TARGET_END
#endif
