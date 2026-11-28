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

using Aff = array<long long, 2>;

signed main() {
  int T;
  if (scanf("%d", &T) != 1) return 1;
  while (T--) {
    long long s, t, P;
    if (scanf("%lld %lld %lld", &s, &t, &P) != 3) return 1;
    Aff f[3];
    for (int c = 0; c < 3; ++c)
      if (scanf("%lld %lld", &f[c][0], &f[c][1]) != 2) return 1;
    auto apply = [P](Aff m, long long x) { return (m[0] * x + m[1]) % P; };
    auto compose = [P](Aff m1, Aff m2) {
      return Aff{m1[0] * m2[0] % P, (m1[0] * m2[1] + m1[1]) % P};
    };
    Aff g[3];
    for (int c = 0; c < 3; ++c) g[c] = compose(f[(c + 2) % 3], compose(f[(c + 1) % 3], f[c]));
    auto hash = [](long long x) { return (unsigned)x; };
    long long best = -1, x = s;
    for (int r = 0; r < 3; ++r) {
      if (r > 0) x = apply(f[r - 1], x);
      const long long q = ref_impl::solve(g[r], x, t, P, compose, apply, hash);
      if (q >= 0) {
        const long long n = 3 * q + r;
        if (best < 0 || n < best) best = n;
      }
    }
    printf("%lld\n", best);
  }
  return 0;
}
