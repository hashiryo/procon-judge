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
//
// 入力フォーマット:
//   T 行のクエリ。各クエリは 5 種類のいずれか:
//   ENCODE_PATH a b
//     有理数 a/b を Stern-Brocot 木の根からの L/R パスとして出力。
//     path = "L k1 R k2 ..." の形式 (連続する同方向は連長圧縮)。
//   DECODE_PATH k1 k2 ... kN
//     L/R 連の長さ列から rational a/b を復元して出力。
//   LCA a1 b1 a2 b2
//     2 つの rational a1/b1, a2/b2 の LCA in SB tree、答えの rational を出力。
//   ANCESTOR depth a b
//     a/b の祖先で深さ depth のもの (= 一定数親に上がった先) を出力。 depth が
//     深すぎる場合は -1。
//   RANGE a b
//     a/b の Stern-Brocot interval (lx ly rx ry) を出力 (= mediant が a/b になる
//     左右の Stern-Brocot 隣接 fraction)。
//
// I/O が複雑なので harness は struct Solver::run() で全部処理させる。
