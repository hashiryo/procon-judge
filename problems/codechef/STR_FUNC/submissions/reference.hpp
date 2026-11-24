#pragma once
// 参照実装 (ライブラリに依存しない)。添字の小さい順に、真の部分集合の G の和 sg と F^2 の和 sf を、足した添字を 2 進の 1 の bit ごとの
// 揃った区画に分けて区画ごとに中の値のゼータ変換を持つ構造で問い (O(log N))、F[i] = A[i]^2 + sg^2、G[i] = sf + F[i]^2 として足す。
#include "pj.hpp"
struct Solver {
  static constexpr u32 P = 1000000007;
  vector<int> a;
  u32 ans = 0;
  explicit Solver(const vector<int> &a_) : a(a_) {}
  void run() {
    const size_t n = a.size();
    int m = 0;
    while ((size_t(1) << m) < n) ++m;
    const size_t N = size_t(1) << m;
    vector<u32> yg(N, 0), yf(N, 0);
    size_t sz = 0;
    auto sub = [&](const vector<u32> &y, size_t x) {  // sum_{j < sz, j ⊆ x}
      u64 r = 0;
      size_t c = 0;
      for (int b = m; b >= 0; --b)
        if (sz >> b & 1) {
          if (!(c & ~x)) r += y[c | (x & ((size_t(1) << b) - 1))];
          c |= size_t(1) << b;
        }
      return r % P;
    };
    auto push = [&](vector<u32> &y, u32 v) {
      y[sz] = v;
      size_t s2 = sz + 1;
      for (int b = 0; b < m && !(s2 >> b & 1); ++b) {
        const size_t w = size_t(1) << b;
        u32 *H = y.data() + s2 - w;
        const u32 *L = H - w;
        for (size_t t = 0; t < w; ++t) H[t] = (H[t] + L[t]) % P;
      }
    };
    u64 total = 0;
    for (size_t i = 0; i < n; ++i) {
      const u64 sg = sub(yg, i), sf = sub(yf, i);
      const u64 F = (u64(a[i]) * a[i] + sg * sg) % P;
      const u64 F2 = F * F % P;
      const u64 G = (sf + F2) % P;
      push(yg, u32(G)), push(yf, u32(F2));
      ++sz;
      total = (total + i % P * F % P * G) % P;
    }
    ans = u32(total);
  }
  u32 answer() const { return ans; }
};
