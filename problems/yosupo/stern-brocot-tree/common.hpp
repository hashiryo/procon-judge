#pragma once
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
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
using u8 = unsigned char;
using u32 = unsigned;
using i64 = long long;
using u64 = unsigned long long;
using u128 = __uint128_t;

// yosupo Stern-Brocot Tree (https://judge.yosupo.jp/problem/stern_brocot_tree)
// T ≤ 10^5 個の問。入力の分数 a/b は 1 ≤ a, b ≤ 10^9 で既約。
//   ENCODE_PATH a b: 根 1/1 から a/b への道を、L と R の連の長さで返す。
//   DECODE_PATH k c_0 n_0 ... c_{k-1} n_{k-1}: 連の長さの列が表す分数を返す (c_i ≠ c_{i+1}、答えは 10^9 以下)。
//   LCA a b c d: a/b と c/d の最も深い共通の祖先を返す。
//   ANCESTOR k a b: a/b の祖先のうち深さ k (1 ≤ k ≤ 10^9) のものを返す (無ければ -1)。
//   RANGE a b: a/b の子孫の集まりである開区間の下限 f/g と上限 h/k を返す (0 は 0/1、+∞ は 1/0)。
// 入出力は計測の外で行う。道は連の長さを R、L、R、... の順に並べて表し、最初の R の連だけ 0 になりうる (a/b < 1 のとき)。
// 提出は run(in, out) で、答えを out から順に u32 で書き、書き終わりを返す。
//   ENCODE: 連の数 m と、R から始まる連の長さ m 個。ハーネスは 0 の連を飛ばして出力する。
//   DECODE、LCA: (a, b)。ANCESTOR: (a, b)、無ければ (0, 0)。RANGE: (f, g, h, k)。
enum : u8 { ENCODE = 0, DECODE = 1, LCA = 2, ANCESTOR = 3, RANGE = 4 };
struct Query {
    u8 type;
    u32 off, len;  // DECODE の道は path[off, off + len)
    u32 x[4];      // ENCODE、RANGE: (a, b)。LCA: (a, b, c, d)。ANCESTOR: (k, a, b)
};
struct Input {
    vector<Query> qs;
    vector<u32> path;
};
// a, b ≤ 10^9 の道の連の数の上限 (先頭の 0 を含む)。互除法の段の数が最も多いのは隣り合う Fibonacci 数の比で、
// F_43 / F_44 の 43 個。
constexpr u32 MAX_RUNS = 43;
