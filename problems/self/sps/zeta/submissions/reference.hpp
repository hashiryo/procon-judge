#pragma once
#include "pj.hpp"

// 参照実装。1 bit ずつのスカラーのループ (Library にも NeoLibrary にも依存しない)。
struct Solver {
  int t;
  vector<i64> a;
  Solver(int t_, int, const vector<i64> &a_) : t(t_), a(a_) {}
  void run() {
    const size_t N = a.size();
    const i64 P = 998244353;
    for (size_t b = 1; b < N; b <<= 1)
      for (size_t s = 0; s < N; ++s) {
        if (!(s & b)) continue;
        i64 &lo = a[s ^ b], &hi = a[s];
        if (t == 0) hi = max(hi, lo);
        else if (t == 1) lo = min(lo, hi);
        else if (t == 2) hi = i64(u64(hi) + u64(lo));
        else if (t == 3) hi = (hi + lo) % P;
        else if (t == 4) lo = (lo - hi + P) % P;
        else {
          const i64 x = lo, y = hi;
          lo = (x + y) % P, hi = (x - y + P) % P;
        }
      }
  }
  vector<i64> answer() const { return a; }
};
