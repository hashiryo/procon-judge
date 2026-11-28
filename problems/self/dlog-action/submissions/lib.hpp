// 今の Library の DiscreteLogarithm を使う版 (比較の基準)。
#pragma once
#include <array>
#include <vector>
#include "mylib/algebra/DiscreteLogarithm.hpp"
using namespace std;

namespace lib_impl {
inline array<long long, 4> mat_mul(const array<long long, 4> &f, const array<long long, 4> &g, long long p) {
  array<long long, 4> r;
  r[0] = (f[0] * g[0] + f[1] * g[2]) % p;
  r[1] = (f[0] * g[1] + f[1] * g[3]) % p;
  r[2] = (f[2] * g[0] + f[3] * g[2]) % p;
  r[3] = (f[2] * g[1] + f[3] * g[3]) % p;
  return r;
}
}  // namespace lib_impl

struct Solver {
  long long affine(long long p, long long a, long long b, long long s, long long t, long long N) {
    auto mp = [p](array<long long, 2> f, long long x) { return (f[0] * x + f[1]) % p; };
    auto op = [p](array<long long, 2> f, array<long long, 2> g) {
      return array<long long, 2>{f[0] * g[0] % p, (f[0] * g[1] + f[1]) % p};
    };
    auto hash = [](long long x) { return (int)(unsigned)x; };
    DiscreteLogarithm log(mp, op, hash, N);
    return log(array<long long, 2>{a % p, b % p}, s, t, N);
  }
  long long matrix(long long p, const array<long long, 4> &A, const array<long long, 4> &S,
                   const array<long long, 4> &T, long long N) {
    auto mp = [p](array<long long, 4> f, array<long long, 4> v) { return lib_impl::mat_mul(f, v, p); };
    auto op = [p](array<long long, 4> f, array<long long, 4> g) { return lib_impl::mat_mul(f, g, p); };
    auto hash = [](array<long long, 4> v) { return (int)(unsigned)v[0]; };
    DiscreteLogarithm log(mp, op, hash, N);
    return log(A, S, T, N);
  }
  long long permutation(const vector<int> &p, const vector<int> &s, const vector<int> &t, long long N) {
    auto mp = [](const vector<int> &f, const vector<int> &v) {
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
      return (int)h;
    };
    DiscreteLogarithm log(mp, op, hash, N);
    return log(p, s, t, N);
  }
};
