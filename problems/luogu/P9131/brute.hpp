#pragma once
// 愚直解 (pj testdata crosscheck 用)。dp[S] = f(c_S) (1 + sum_{T ⊊ S} dp[T]) を 3^m で (m <= 6)。f は c 個の問題から選んで並べる数を、
// 順列の数の和 sum_{k=1}^{c} c! / (c - k)! で直接求める。
#include "pj.hpp"
struct Solver {
  static constexpr u64 P = 1000000007;
  int m;
  vector<int> cnt;
  u32 ans = 0;
  Solver(int m_, const vector<int> &cnt_) : m(m_), cnt(cnt_) {}
  void run() {
    const int N = 1 << m;
    vector<u64> dp(N, 0);
    u64 total = 0;
    for (int s = 0; s < N; ++s) {
      u64 f = 0;
      for (int k = 1; k <= cnt[s]; ++k) {
        u64 p = 1;
        for (int i = 0; i < k; ++i) p = p * (cnt[s] - i) % P;
        f = (f + p) % P;
      }
      u64 sum = 1;
      for (int t = (s - 1) & s; t != s; t = (t - 1) & s) {
        sum = (sum + dp[t]) % P;
        if (!t) break;
      }
      dp[s] = f * sum % P;
      total = (total + dp[s]) % P;
    }
    ans = u32(total);
  }
  u32 answer() const { return ans; }
};
