// 期待出力を作る参照実装。今の Library の DiscreteLogarithm (maspy の方法) を固定したもの。
// ジェネレータか base.cpp を直したときだけ作り直されるよう、ライブラリは include しない。
#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>
using namespace std;

namespace reference_impl {
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

inline array<i64, 4> mat_mul(const array<i64, 4> &f, const array<i64, 4> &g, i64 p) {
  array<i64, 4> r;
  r[0] = (f[0] * g[0] + f[1] * g[2]) % p;
  r[1] = (f[0] * g[1] + f[1] * g[3]) % p;
  r[2] = (f[2] * g[0] + f[3] * g[2]) % p;
  r[3] = (f[2] * g[1] + f[3] * g[3]) % p;
  return r;
}
}  // namespace reference_impl

struct Solver {
  long long affine(long long p, long long a, long long b, long long s, long long t, long long N) {
    auto act = [p](array<long long, 2> f, long long x) { return (f[0] * x + f[1]) % p; };
    auto op = [p](array<long long, 2> f, array<long long, 2> g) {
      return array<long long, 2>{f[0] * g[0] % p, (f[0] * g[1] + f[1]) % p};
    };
    auto hash = [](long long x) { return (unsigned)x; };
    return reference_impl::solve(array<long long, 2>{a % p, b % p}, s, t, N, op, act, hash);
  }
  long long matrix(long long p, const array<long long, 4> &A, const array<long long, 4> &S,
                   const array<long long, 4> &T, long long N) {
    auto act = [p](const array<long long, 4> &f, const array<long long, 4> &v) {
      return reference_impl::mat_mul(f, v, p);
    };
    auto op = [p](const array<long long, 4> &f, const array<long long, 4> &g) {
      return reference_impl::mat_mul(f, g, p);
    };
    auto hash = [](const array<long long, 4> &v) { return (unsigned)v[0]; };
    return reference_impl::solve(A, S, T, N, op, act, hash);
  }
  long long permutation(const vector<int> &p, const vector<int> &s, const vector<int> &t, long long N) {
    auto act = [](const vector<int> &f, const vector<int> &v) {
      vector<int> r(f.size());
      for (size_t i = 0; i < f.size(); ++i) r[i] = v[f[i]];
      return r;
    };
    auto op = [](const vector<int> &f, const vector<int> &g) {
      vector<int> r(f.size());
      for (size_t i = 0; i < f.size(); ++i) r[i] = g[f[i]];
      return r;
    };
    auto hash = [](const vector<int> &v) {
      unsigned h = (unsigned)v[0];
      if (v.size() > 1) h = h * 1000u + (unsigned)v[1];
      if (v.size() > 2) h = h * 31u + (unsigned)v[2];
      return h;
    };
    return reference_impl::solve(p, s, t, N, op, act, hash);
  }
};
