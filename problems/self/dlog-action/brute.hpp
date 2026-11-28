// 愚直解 (提出ではない)。参照実装と小さい入力 (seed 1000 以上) で突き合わせる。
#pragma once
#include "pj.hpp"

namespace brute_impl {
inline array<i64, 4> mat_mul(const array<i64, 4> &f, const array<i64, 4> &g, i64 p) {
  array<i64, 4> r;
  r[0] = (f[0] * g[0] + f[1] * g[2]) % p;
  r[1] = (f[0] * g[1] + f[1] * g[3]) % p;
  r[2] = (f[2] * g[0] + f[3] * g[2]) % p;
  r[3] = (f[2] * g[1] + f[3] * g[3]) % p;
  return r;
}
}  // namespace brute_impl

struct Solver {
  long long affine(long long p, long long a, long long b, long long s, long long t, long long N) {
    long long cur = s % p;
    for (long long n = 0; n < N; ++n) {
      if (cur == t % p) return n;
      cur = (a * cur + b) % p;
    }
    return -1;
  }
  long long matrix(long long p, const array<long long, 4> &A, const array<long long, 4> &S,
                   const array<long long, 4> &T, long long N) {
    array<long long, 4> cur = S;
    for (long long n = 0; n < N; ++n) {
      if (cur == T) return n;
      cur = brute_impl::mat_mul(A, cur, p);
    }
    return -1;
  }
  long long permutation(const vector<int> &p, const vector<int> &s, const vector<int> &t, long long N) {
    vector<int> cur = s;
    for (long long n = 0; n < N; ++n) {
      if (cur == t) return n;
      vector<int> nxt(p.size());
      for (size_t i = 0; i < p.size(); ++i) nxt[i] = cur[p[i]];
      cur.swap(nxt);
    }
    return -1;
  }
};
