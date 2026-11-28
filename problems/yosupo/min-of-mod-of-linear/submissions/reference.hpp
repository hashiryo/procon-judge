#pragma once
// min{ (a x + b) mod m | 0 <= x < n } を Euclid の再帰で求める。
//
// a x + b が m を初めて超える x を、商 y = floor((a x + b) / m) ごとに選ぶと、余りは
//   a - 1 - ((m y + a - 1 - b) mod a)
// になる。y は 1 <= y <= floor((a (n-1) + b) / m) を動く。最大も同じ形で、m y を初めて
// 超える直前の x を選ぶと余りは
//   m - 1 - ((m y - b - 1) mod a)
// になる。どちらも法が m から a に下がるので、Euclid と同じ段数で終わる。
// 最小は x = 0 のときの b と、最大は x = n-1 のときの (a (n-1) + b) mod m と比べる。
#include <cassert>

namespace min_mod_internal {
using u64 = unsigned long long;

u64 min_mod_impl(u64 n, u64 m, u64 a, u64 b);

// max{ (a x + b) mod m | 0 <= x < n }。0 <= a, b < m、n >= 1。
inline u64 max_mod(u64 n, u64 m, u64 a, u64 b) {
  if (a == 0) return b;
  const u64 u = a * (n - 1) + b;  // a < m <= 1e9、n <= 1e9 なので 1e18 未満
  const u64 last = u % m;         // x = n-1 のときの余り
  const u64 ymax = u / m;
  if (ymax == 0) return last;     // 1 度も m を超えないので単調増加
  // y = 1..ymax について、m y を初めて超える直前の x の余りの最小を引く。
  const u64 lo = min_mod_impl(ymax, a, m % a, (m % a + a - b % a - 1) % a);
  const u64 across = m - 1 - lo;
  return across > last ? across : last;
}

// min{ (a x + b) mod m | 0 <= x < n }。0 <= a, b < m、n >= 1。
inline u64 min_mod_impl(u64 n, u64 m, u64 a, u64 b) {
  if (a == 0) return b;
  const u64 u = a * (n - 1) + b;
  const u64 ymax = u / m;
  if (ymax == 0) return b;  // 1 度も m を超えないので単調増加
  // y = 1..ymax について、m y を初めて超える x の余りは 1 対 1 で a - 1 - (…)。
  const u64 hi = max_mod(ymax, a, m % a, (m % a + a - 1 - b % a) % a);
  const u64 across = a - 1 - hi;
  return across < b ? across : b;
}

}  // namespace min_mod_internal

struct Solver {
  u64 n, m, a, b, res = 0;
  Solver(u64 n_, u64 m_, u64 a_, u64 b_) : n(n_), m(m_), a(a_ % m_), b(b_ % m_) {}
  void run() { res = min_mod_internal::min_mod_impl(n, m, a, b); }
  u64 answer() const { return res; }
};
