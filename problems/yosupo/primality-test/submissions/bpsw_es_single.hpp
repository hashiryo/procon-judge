#pragma once
// 表を持たない BPSW。底 2 の強擬素数判定と、extra strong Lucas 判定を 1 つのループで同時に回す。
// Lucas 判定の P の選び方と、2^64 未満に反例が無いことの出典は _bpsw_es.hpp。
//
// Lucas 側は V 列だけを梯子で求める。(V_k, V_{k+1}) から
//   V_{2k} = V_k^2 - 2、V_{2k+1} = V_k V_{k+1} - P、V_{2k+2} = V_{k+1}^2 - 2
// なので、1 段は 2 つの独立な積になる。定数の引き算は還元の最後の足し算にまとめるので、
// 1 段の鎖は積 1 回ぶん (と選択 1 回) で、底 2 の鎖 (_mr_single.hpp と同じ形) とほぼ同じ長さになる。
// U_d ≡ 0 は、Q = 1 では D U_d = 2 V_{d+1} - P V_d なので、V_d ≡ ±2 のとき V_{d+1} ≡ ±P と同じ。
#include "../common.hpp"
#include "_mr_single.hpp"
#include "_bpsw_es.hpp"

namespace bpsw_es_single {

// n は奇数で 41^2 <= n < 2^60、37 以下の素数で割り切れない。
inline bool bpsw(u64 n) {
  using bpsw_es::mmul_add;
  using mr_single::mmul;
  const u64 P = bpsw_es::select_P(n);
  if (P == 0) return false;
  const u64 ninv = mr_single::inv64(n), one = (0 - n) % n, mone = n - one;
  const double inv_n = 1.0 / (double)(i64)n;
  const u64 two = 2 * one >= n ? 2 * one - n : 2 * one;
  u64 Pm = mr_single::to_mont_small(P, one, n, inv_n);
  Pm = Pm >= n ? Pm - n : Pm;
  const u64 nc2 = 2 * n - two, ncP = 2 * n - Pm;  // n + (-2 mod n)、n + (-P mod n)
  // 底 2 は (n-1) = dA 2^sA の dA を、Lucas は (n+1) = dL 2^sL の dL を、上の桁から読む。
  const int sA = __builtin_ctzll(n - 1), sL = __builtin_ctzll(n + 1);
  const u64 dA = (n - 1) >> sA, dL = (n + 1) >> sL;
  const int LA = 64 - __builtin_clzll(dA), LL = 64 - __builtin_clzll(dL);
  u64 da = dA << (64 - LA), dl = dL << (64 - LL);
  // 底 2: 最上位のビットは x = 2 で済ませる。
  u64 x = 2 * one;
  // Lucas: (V_k, V_{k+1}) = prev ? (a, s) : (s, a)。最上位のビットで k = 1、(V_1, V_2) = (P, P^2 - 2)。
  u64 a = Pm, s = mmul_add(Pm, Pm, n, ninv, nc2);
  u64 prev = 1;
  auto stepA = [&] {
    da <<= 1;
    x = mmul(x, x << (da >> 63), n, ninv);
  };
  auto stepL = [&] {
    dl <<= 1;
    const u64 b = dl >> 63;
    const u64 sel = b == prev ? s : a;
    const u64 a2 = mmul_add(a, s, n, ninv, ncP);  // V_{2k+1}
    s = mmul_add(sel, sel, n, ninv, nc2);          // V_{2k} か V_{2k+2}
    a = a2, prev = b;
  };
  const int common = (LA < LL ? LA : LL) - 1;
  for (int i = 0; i < common; ++i) stepA(), stepL();
  for (int i = common; i < LA - 1; ++i) stepA();
  for (int i = common; i < LL - 1; ++i) stepL();
  // 底 2 の判定
  x = x >= n ? x - n : x;
  bool pa = x == one || x == mone;
  for (int r = 1; !pa && r < sA; ++r) {
    if (x == one) return false;
    x = mmul(x, x, n, ninv);
    x = x >= n ? x - n : x;
    pa = x == mone;
  }
  if (!pa) return false;
  // extra strong Lucas の判定: (U_d ≡ 0 かつ V_d ≡ ±2) か、0 <= r < sL - 1 のどれかで V_{d 2^r} ≡ 0。
  auto norm3 = [n](u64 v) {  // (0, 3n) から [0, n) へ
    v = v >= n ? v - n : v;
    return v >= n ? v - n : v;
  };
  u64 V = norm3(prev ? a : s), W = norm3(prev ? s : a);
  if ((V == two && W == Pm) || (V == n - two && W == n - Pm)) return true;
  for (int r = 0; r < sL - 1; ++r) {
    if (V == 0) return true;
    if (V == two) return false;  // 以降はずっと 2
    V = norm3(mmul_add(V, V, n, ninv, nc2));
  }
  return false;
}

}  // namespace bpsw_es_single

inline bool is_prime_bpsw_es(u64 n) {
  if (int c = mr_single::small_check(n); c >= 0) return c;
  return bpsw_es_single::bpsw(n);
}

inline vector<bool> run(const vector<u64>& qs) {
  vector<bool> ans(qs.size());
  for (size_t i = 0; i < qs.size(); ++i) ans[i] = is_prime_bpsw_es(qs[i]);
  return ans;
}
