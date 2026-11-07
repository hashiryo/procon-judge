#pragma once
// algos 共通: typedef とよく使うヘッダ。
// USE_SIMDE 環境では simde を先に include (float16_t 衝突回避)。
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

// この問題は、n 個の部屋と m 枚の扉 (扉は部屋 i と j をつなぎ、i から j へ移ると c_ij、j から i へ移ると c_ji だけ気分が上がる)
// について、通るたびに気分が上がる (重みの和が正の) 閉じた歩みのうち、部屋の数 (歩数) が最小のものの歩数を答える。そのような
// 歩みがなければ 0。歩みは同じ部屋を何度通ってもよく、最小は 2 (1 枚の扉の往復)。1 ≤ n ≤ 300、どの 2 部屋の間にも扉は
// 高々 1 枚、-10^4 ≤ c_ij, c_ji ≤ 10^4。
