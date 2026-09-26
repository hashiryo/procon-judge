#pragma once
#include "_shared/modulo-test/_common.hpp"
#include <numeric>
// libstdc++ の std::gcd。中身は __detail::__gcd の Euclidean ベース実装。
inline u64 run(u64 a, u64 b) { return std::gcd(a, b); }
