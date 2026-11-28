// 期待出力を作る参照実装。lib-neo.cpp のロジックを、今の Library の手法 (maspy の BSGS) を写した solve で固定したもの。
// ライブラリは include しない。
#include <array>
#include <cmath>
#include <cstdio>
#include <utility>
#include <vector>
using namespace std;

namespace ref_impl {
using u32 = unsigned;
using i64 = long long;

template <class X, class S, class Op, class Act, class Hash>
i64 solve(const X &x, S s, const S &t, i64 N, const Op &op, const Act &act, const Hash &hash) {
  if (N <= 0) return -1;
  if (s == t) return 0;
  const int m = 1 << (31 - __builtin_clz(int(std::sqrt((double)N) + 1))), mask = m - 1;
  vector<S> val(m), vs(m);
  vector<int> os(m + 1), so(m);
  S s1 = t;
  for (int i = 0; i < m; ++i) ++os[so[i] = (u32)hash(val[i] = s1 = act(x, s1)) & mask];
  for (int i = 0; i < m; ++i) os[i + 1] += os[i];
  for (int i = 0; i < m; ++i) vs[--os[so[i]]] = std::move(val[i]);
  X y = x;
  for (int k = m; k >>= 1;) y = op(y, y);
  bool failed = false;
  for (i64 n = 0;; s = s1) {
    for (int a = (u32)hash(s1 = act(y, s)) & mask, j = os[a]; j < os[a + 1]; ++j) {
      if (s1 == vs[j]) {
        for (int i = 0;; s = act(x, s)) {
          if (s == t) return n + i < N ? n + i : -1;
          if (++i == m) break;
        }
        if (failed) return -1;
        failed = true;
        break;
      }
    }
    if ((n += m) >= N) break;
  }
  return -1;
}
}  // namespace ref_impl

using Cx = array<long long, 2>;

signed main() {
  long long A, B, C, D, p;
  if (scanf("%lld %lld %lld %lld %lld", &A, &B, &C, &D, &p) != 5) return 1;
  auto mul = [p](Cx x, Cx y) {
    long long re = (x[0] * y[0] - x[1] * y[1]) % p;
    if (re < 0) re += p;
    long long im = (x[0] * y[1] + x[1] * y[0]) % p;
    return Cx{re, im};
  };
  auto cpow = [&](Cx x, long long e) {
    Cx r{1 % p, 0};
    while (e) {
      if (e & 1) r = mul(r, x);
      x = mul(x, x);
      e >>= 1;
    }
    return r;
  };
  auto norm = [p](Cx x) { return (x[0] * x[0] + x[1] * x[1]) % p; };
  auto hash = [](Cx x) { return (unsigned)x[0]; };
  const Cx z{A % p, B % p}, w{C % p, D % p};
  const long long det_z = norm(z), det_w = norm(w);
  long long ans = -1;
  if (det_z == 0) {
    ans = ref_impl::solve(z, Cx{1 % p, 0}, w, 2 * p + 1, mul, mul, hash);
  } else {
    auto sact = [p](long long x, long long y) { return x * y % p; };
    auto shash = [](long long x) { return (unsigned)x; };
    const long long r = ref_impl::solve(det_z, 1 % p, det_w, p, sact, sact, shash);
    const long long m0 = ref_impl::solve(det_z, det_z, 1 % p, p, sact, sact, shash) + 1;
    if (r >= 0) {
      const Cx X = cpow(z, m0), S = cpow(z, r);
      const long long q = ref_impl::solve(X, S, w, 2 * p + 1, mul, mul, hash);
      if (q >= 0) ans = m0 * q + r;
    }
  }
  printf("%lld\n", ans);
  return 0;
}
