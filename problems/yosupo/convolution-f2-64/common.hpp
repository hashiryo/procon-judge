#pragma once
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
#include <simde/x86/clmul.h>
#include <simde/x86/bmi.h>
#else
#include <immintrin.h>
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
#ifdef __x86_64__
#define GNU_TARGET(x) [[gnu::target(x)]]
#else
#define GNU_TARGET(x)
#endif

using namespace std;
using u8= unsigned char;
using u32= unsigned;
using i64= long long;
using u64= unsigned long long;
using u128= __uint128_t;

// yosupo Convolution over F_{2^64}
//   入力: n m
//         a_0 ... a_{n-1}
//         b_0 ... b_{m-1}
//   出力: c_0 ... c_{n+m-2}
//   ここで c_k = XOR_{i+j=k} a_i · b_j
//   · は F_{2^64} の積 (X^64 + X^4 + X^3 + X + 1 mod 削減多項式)
//
// アルゴリズム:
//   - 素朴: O(nm) で nested loop
//   - additive FFT (a.k.a. nim FFT, characteristic-2 FFT): O((n+m) log(n+m))
//     b[i+1] = b[i]^2 + b[i] で生成する基底列で Frobenius 構造を活用
