#pragma once
// 参照実装 (ライブラリに依存しない)。dp[S] = f(c_S) (1 + sum_{T ⊊ S} dp[T])、f(c) = c (f(c - 1) + 1)、答えは sum_S dp[S]。
// 真の部分集合の和は、添字の小さい順に値を足しながら問う構造 (足した添字を 2 進の 1 の bit ごとの揃った区画に分け、区画ごとに中の値の
// ゼータ変換を持つ。問いは O(m)、足すのは均して O(m)) で求める。
#include "pj.hpp"
struct Solver {
  static constexpr u32 P = 1000000007;
  int m;
  vector<int> cnt;
  u32 ans = 0;
  Solver(int m_, const vector<int> &cnt_) : m(m_), cnt(cnt_) {}
  void run() {
    const size_t N = size_t(1) << m;
    int mx = 0;
    for (int c : cnt) mx = max(mx, c);
    vector<u32> f(mx + 1, 0);
    for (int c = 1; c <= mx; ++c) f[c] = u32(u64(c) * (f[c - 1] + 1) % P);
    vector<u32> y(N, 0);  // 足した値の区画ごとのゼータ変換
    size_t sz = 0;
    u64 total = 0;
    auto sub = [&](size_t x) {  // sum_{j < sz, j ⊆ x}
      u64 r = 0;
      size_t c = 0;
      for (int b = m; b >= 0; --b)
        if (sz >> b & 1) {
          if (!(c & ~x)) r += y[c | (x & ((size_t(1) << b) - 1))];
          c |= size_t(1) << b;
        }
      return r % P;
    };
    auto push = [&](u32 v) {
      y[sz++] = v;
      for (int b = 0; b < m && !(sz >> b & 1); ++b) {
        const size_t w = size_t(1) << b;
        u32 *H = y.data() + sz - w;
        const u32 *L = H - w;
        for (size_t t = 0; t < w; ++t) H[t] = (H[t] + L[t]) % P;
      }
    };
    for (size_t s = 0; s < N; ++s) {
      const u32 d = u32(u64(f[cnt[s]]) * ((1 + sub(s)) % P) % P);  // sub(s) は真の部分集合 (s はまだ足していない)
      push(d);
      total += d;
    }
    ans = u32(total % P);
  }
  u32 answer() const { return ans; }
};
