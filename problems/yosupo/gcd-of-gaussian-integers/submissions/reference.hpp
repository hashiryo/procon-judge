#pragma once
// Z[i] の Euclid。x を y で割るとき、x * conj(y) / norm(y) の各成分を最近整数に丸めて商にする。
// 剰余のノルムは norm(y) の半分以下になるので、ノルムが単調に減って止まる。
// 答えは単位倍だけの不定性があるが、checker が ±1, ±i 倍を許す。
#include <cstdint>

namespace gauss_internal {
using i64 = long long;
using i128 = __int128_t;

// n / d を最近整数に丸める。d > 0。
inline i64 div_round(i128 n, i128 d) {
  if (n >= 0) return (i64)((n + d / 2) / d);
  return -(i64)((-n + d / 2) / d);
}

struct G {
  i64 a, b;
};

// x = a + b i を y = c + d i で割った商。
inline G quo(G x, G y) {
  const i128 n = (i128)y.a * y.a + (i128)y.b * y.b;
  return {div_round((i128)x.a * y.a + (i128)x.b * y.b, n),
          div_round((i128)x.b * y.a - (i128)x.a * y.b, n)};
}

inline G sub(G x, G y) { return {x.a - y.a, x.b - y.b}; }
inline G mul(G x, G y) { return {x.a * y.a - x.b * y.b, x.a * y.b + x.b * y.a}; }
inline bool zero(G x) { return x.a == 0 && x.b == 0; }

inline G gcd(G x, G y) {
  while (!zero(y)) {
    const G q = quo(x, y);
    const G r = sub(x, mul(q, y));
    x = y;
    y = r;
  }
  return x;
}

}  // namespace gauss_internal

struct Solver {
  gauss_internal::G p, q, g{0, 0};

  Solver(long long a1, long long b1, long long a2, long long b2)
      : p{a1, b1}, q{a2, b2} {}

  void run() { g = gauss_internal::gcd(p, q); }
  long long x() const { return g.a; }
  long long y() const { return g.b; }
};
