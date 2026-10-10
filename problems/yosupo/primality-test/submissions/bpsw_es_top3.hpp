#pragma once
// bpsw_es_single.hpp と同じ BPSW (底 2 の強擬素数判定と extra strong Lucas 判定、部品は _bpsw_es.hpp) で、鎖を短くしたもの。
// - 底 2 の指数 (n-1)/2^sA と Lucas の指数 (n+1)/2^sL を、先頭に 0 を詰めて同じ桁数に揃え、1 本のループで回す。
//   底 2 の鎖は x = 1、Lucas の鎖は (V_0, V_1) = (2, P) のまま、0 のビットでは値が変わらない。
//   ループを抜ける分岐が 3 回から 1 回になる。
// - 揃えた指数の上 3 桁は、2^e と V_k (k <= 8) を小さい整数のまま求めてから Montgomery 表現に直す。
//   P <= 20 なら V_8 < 2^35 なので、to_mont_small の商の見積もりが ±1 に収まる。
// - 最後の 2 乗の繰り返しは、底 2 と Lucas を同じループで回す。
#include "../common.hpp"
#include "_mr_single.hpp"
#include "_bpsw_es.hpp"

namespace bpsw_es_top3 {

constexpr int T = 3;
// kV[P][k] = V_k(P, 1) (k = 0, ..., 2^T)。
constexpr auto kV = [] {
  std::array<std::array<u64, (1 << T) + 1>, 21> t{};
  for (u64 P = 3; P <= 20; ++P) {
    t[P][0] = 2, t[P][1] = P;
    for (int k = 2; k <= (1 << T); ++k) t[P][k] = P * t[P][k - 1] - t[P][k - 2];
  }
  return t;
}();
static_assert(kV[20][1 << T] < (1ull << 36));

// n は奇数で 41^2 <= n < 2^60、37 以下の素数で割り切れない。
inline bool bpsw(u64 n) {
  using bpsw_es::mmul_add;
  using mr_single::mmul;
  using mr_single::to_mont_small;
  const u64 P = bpsw_es::select_P(n);
  if (P == 0) return false;
  const u64 ninv = mr_single::inv64(n), one = (0 - n) % n, mone = n - one;
  const double inv_n = 1.0 / (double)(i64)n;
  const u64 two = 2 * one >= n ? 2 * one - n : 2 * one;
  u64 Pm = to_mont_small(P, one, n, inv_n);
  Pm = Pm >= n ? Pm - n : Pm;
  const u64 nc2 = 2 * n - two, ncP = 2 * n - Pm;
  const int sA = __builtin_ctzll(n - 1), sL = __builtin_ctzll(n + 1);
  const u64 dA = (n - 1) >> sA, dL = (n + 1) >> sL;
  const u64 dmax = dA > dL ? dA : dL;
  const int L = 64 - __builtin_clzll(dmax);
  // 上 T 桁 (L <= T なら全部) を初期値に、残りの桁を上に詰めて読む。
  const int top = L > T ? L - T : 0;
  const u64 eA = dA >> top, eL = dL >> top;
  u64 pa = top ? dA << (64 - top) : 0, pl = top ? dL << (64 - top) : 0;
  u64 x = to_mont_small(1ull << eA, one, n, inv_n);
  // Lucas: (V_k, V_{k+1}) = prev ? (a, s) : (s, a)。
  u64 a, s, prev = 1;
  auto stepL = [&](u64 b) {
    const u64 sel = b == prev ? s : a;
    const u64 a2 = mmul_add(a, s, n, ninv, ncP);
    s = mmul_add(sel, sel, n, ninv, nc2);
    a = a2, prev = b;
  };
  if (P <= 20) {
    a = to_mont_small(kV[P][eL], one, n, inv_n);
    s = to_mont_small(kV[P][eL + 1], one, n, inv_n);
  } else {
    a = two, s = Pm;  // (V_0, V_1)
    for (int i = T - 1; i >= 0; --i) stepL(eL >> i & 1);
  }
  for (int i = 0; i < top; ++i) {
    x = mmul(x, x << (pa >> 63), n, ninv);
    pa <<= 1;
    stepL(pl >> 63);
    pl <<= 1;
  }
  auto norm3 = [n](u64 v) {  // (0, 3n) から [0, n) へ
    v = v >= n ? v - n : v;
    return v >= n ? v - n : v;
  };
  // 底 2: x ≡ ±1 か、2 乗を sA - 1 回までして -1 が出れば通る。
  x = norm3(x);
  bool okA = x == one || x == mone;
  int ra = sA - 1;
  // Lucas: (U_d ≡ 0 かつ V_d ≡ ±2) か、0 <= r < sL - 1 のどれかで V_{d 2^r} ≡ 0 なら通る。
  u64 V = norm3(prev ? a : s);
  const u64 W = norm3(prev ? s : a);
  bool okL = (V == two && W == Pm) || (V == n - two && W == n - Pm) || (sL >= 2 && V == 0);
  int rl = sL - 2;
  for (;;) {
    if (!okA && (ra <= 0 || x == one)) return false;
    if (!okL && (rl <= 0 || V == two)) return false;
    if (okA && okL) return true;
    if (!okA) {
      x = norm3(mmul(x, x, n, ninv));
      okA = x == mone, --ra;
    }
    if (!okL) {
      V = norm3(mmul_add(V, V, n, ninv, nc2));
      okL = V == 0, --rl;
    }
  }
}

}  // namespace bpsw_es_top3

inline bool is_prime_bpsw_es_top3(u64 n) {
  if (int c = mr_single::small_check(n); c >= 0) return c;
  return bpsw_es_top3::bpsw(n);
}

inline vector<bool> run(const vector<u64>& qs) {
  vector<bool> ans(qs.size());
  for (size_t i = 0; i < qs.size(); ++i) ans[i] = is_prime_bpsw_es_top3(qs[i]);
  return ans;
}
