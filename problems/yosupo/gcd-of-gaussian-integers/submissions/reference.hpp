#pragma once
// Z[i] の gcd を 2 進の形 (Stein の方法の Z[i] 版) で求める。割り算を使わない。
//
// a + b i が 1 + i で割り切れるのは a と b の偶奇が同じときで、商は ((a+b)/2) + ((b-a)/2) i。
// 両方から 1 + i を割れるだけ割って奇数にし、ノルムの大きいほうから小さいほうを引く。差は
// 両方が奇数なので必ず 1 + i で割れる。これを繰り返すとノルムが減って止まる。
// (1 + i) は共通に持っていた分だけ最後に掛け戻す。答えは単位倍の不定性があるが checker が許す。
//
// 引くときに 4 つの単位 (±1, ±i) のうち差のノルムが最小になるものを選ぶ。単位を掛けないと
// x = (1, 0)、y = (0, 1) のようにノルムが等しいとき差のノルムが増えて回り続ける。
// 4 つの候補は円周を 90 度ずつ分けるので、|x| >= |y| なら最小のノルムは |x| より小さい。
//
// |a|, |b| <= 1e9 なのでノルムは 2e18 未満、i64 に収まる。
#include <cstdint>

namespace gauss_internal {
using i64 = long long;

struct G {
  i64 a, b;
};

inline bool is_zero(G x) { return (x.a | x.b) == 0; }
inline bool even(G x) { return ((x.a ^ x.b) & 1) == 0; }
inline G div_1pi(G x) { return {(x.a + x.b) >> 1, (x.b - x.a) >> 1}; }
inline G mul_1pi(G x) { return {x.a - x.b, x.a + x.b}; }
inline i64 norm(G x) { return x.a * x.a + x.b * x.b; }

// x から u y (u = ±1, ±i) を引いて、差のノルムが最小になるものを返す。
// |x - u y|^2 = |x|^2 + |y|^2 - 2 Re(x conj(u y)) なので、Re(x conj(u y)) が最大の u を選ぶ。
// t = x conj(y) とすると、u = 1 で Re = t.a、u = i で Re = t.b、u = -1 と -i はその符号違い。
// だから 4 つのノルムを計算しなくてよく、掛け算 4 回と比較で決まる。
inline G sub_best_unit(G x, G y) {
  const i64 ta = x.a * y.a + x.b * y.b;  // Re(x conj y)
  const i64 tb = x.b * y.a - x.a * y.b;  // Im(x conj y)
  const i64 aa = ta < 0 ? -ta : ta, ab = tb < 0 ? -tb : tb;
  G u;
  if (aa >= ab) u = {ta >= 0 ? y.a : -y.a, ta >= 0 ? y.b : -y.b};
  else u = {tb >= 0 ? -y.b : y.b, tb >= 0 ? y.a : -y.a};  // i y = (-y.b, y.a)
  return {x.a - u.a, x.b - u.b};
}

inline G gcd(G x, G y) {
  if (is_zero(x)) return y;
  if (is_zero(y)) return x;
  int kx = 0, ky = 0;
  while (even(x)) x = div_1pi(x), ++kx;
  while (even(y)) y = div_1pi(y), ++ky;
  const int k = kx < ky ? kx : ky;
  if (norm(x) < norm(y)) {
    const G t = x;
    x = y, y = t;
  }
  while (!is_zero(y)) {
    x = sub_best_unit(x, y);
    if (is_zero(x)) {
      x = y;
      break;
    }
    while (even(x)) x = div_1pi(x);
    if (norm(x) < norm(y)) {
      const G t = x;
      x = y, y = t;
    }
  }
  for (int i = 0; i < k; ++i) x = mul_1pi(x);
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
