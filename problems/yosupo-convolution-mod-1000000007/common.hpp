#pragma once
// algos 共通: typedef とよく使うヘッダ。
//
// USE_SIMDE 環境 (ARM CI) では simde を <bits/stdc++.h> より先に include する。
// これにより arm_neon.h が <stdfloat> より先にパースされ、`float16_t` が
// arm_neon.h の ::float16_t としてのみ可視になる。後で `using namespace std;` で
// std::float16_t がグローバルに来ても、parse 済みの arm_neon.h 内の参照は影響を
// 受けない (シンボル解決は参照時に行われる)。
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
#include <simde/x86/bmi.h>
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

// この問題は mod 10^9+7。NTT-friendly でないので NTT を直接使えない:
// - 複素 FFT (double) + Karatsuba 3-split
// - 多素数 NTT + Garner CRT
// などで対応する。
constexpr u32 MOD = 1'000'000'007;
