#pragma once
#include "pj.hpp"

// 参照実装。桁ごとに 1 つずつ、添字を昇順 (y ≥ x の向きは降順) に回して隣の桁の値から受け取る (Library にも NeoLibrary にも
// 依存しない)。メビウス変換は、受け取る向きを逆にして差を取る。
struct Solver {
  int t;
  vector<i64> a;
  Solver(int t_, int, const vector<i64> &a_) : t(t_), a(a_) {}
  void run() {
    static constexpr int KK[5] = {10, 3, 4, 5, 7};
    const long long k = KK[t], N = (long long)a.size(), P = 998244353;
    for (long long p = 1; p < N; p *= k) {
      if (t == 0 || t == 4) {
        for (long long x = 0; x < N; ++x)
          if (x / p % k) a[x] = t == 0 ? a[x] + a[x - p] : min(a[x], a[x - p]);
      } else if (t == 2) {
        for (long long x = N; x-- > 0;)
          if (x / p % k) a[x] = (a[x] - a[x - p] + P) % P;
      } else {
        for (long long x = N; x-- > 0;)
          if (x / p % k + 1 < k) a[x] = t == 1 ? max(a[x], a[x + p]) : i64(u64(a[x]) + u64(a[x + p]));
      }
    }
  }
  vector<i64> answer() const { return a; }
};
