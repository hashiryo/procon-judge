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
using i32 = int32_t;
using i64 = long long;
using u64 = unsigned long long;
using u128 = __uint128_t;

// 自前実験: 法 998244353 固定で、nCr (0 <= r <= n <= N) を Q 個答える。
//   入力: N Q / n_1 r_1 / ... / n_Q r_Q
//   出力: nCr mod 998244353 を 1 行ずつ
//   表の構築 (N! まで) と問いの両方を計測する。
