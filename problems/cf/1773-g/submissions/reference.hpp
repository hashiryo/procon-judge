#pragma once
// 参照実装 (ライブラリに依存しない)。残っている人の集合 S (Genie を含む) を大きい方から見て、dp[S] を S に着く確率とする。S を変える質問
// (S との共通部分が空でも S でもない) の数 eff(S) = n - #{q ⊆ ~S} - #{q ⊇ S} のうち、次に来るものは一様なので、Genie が残る遷移
// S → T = S ∩ q (q は Genie が正解、T ≠ S) へ dp[S] #{q : q ∩ S = T} / eff(S) を足す。eff(S) = 0 の S では dp[S] を答えに足す。
// #{q : q ∩ S = T} は、S の部分集合 T' ごとの #{q ⊆ T' ∪ ~S} (cnt の部分集合の和) を S の中でメビウス変換して求める (整数で正確)。
// S ごとに O(|S| 2^{|S|}) で、全体で O(m 3^m)。
#include <algorithm>
#include <numeric>
#include "pj.hpp"
struct Solver {
  int n, m;
  vector<u32> q;
  double ans = 0;
  Solver(int n_, int m_, const vector<u32> &q_) : n(n_), m(m_), q(q_) {}
  void run() {
    const u32 full = (1u << m) - 1;
    const size_t N = size_t(1) << m;
    vector<int> sub(N, 0), sup(N, 0);
    for (u32 x : q) ++sub[x], ++sup[x];
    for (int b = 0; b < m; ++b)
      for (size_t a = 0; a < N; ++a) {
        if (a >> b & 1) sub[a] += sub[a ^ (size_t(1) << b)];
        else sup[a] += sup[a | (size_t(1) << b)];
      }
    vector<double> dp(N, 0.0);
    dp[full] = 1.0;
    vector<int> f(N);
    vector<u32> tm(N);
    long double total = 0;
    for (u32 S = full;; S -= 2) {  // Genie (bit 0) を含む S だけ
      if (dp[S] != 0.0) {
        const int eff = n - sub[full ^ S] - sup[S];
        if (eff == 0) total += dp[S];
        else {
          const double w = dp[S] / eff;
          int bits[20], s = 0;
          for (u32 t = S; t; t &= t - 1) bits[s++] = __builtin_ctz(t);
          const u32 rest = full ^ S;
          tm[0] = 0, f[0] = sub[rest];
          for (u32 k = 1; k < (1u << s); ++k) {
            tm[k] = tm[k & (k - 1)] | (1u << bits[__builtin_ctz(k)]);
            f[k] = sub[tm[k] | rest];
          }
          for (int b = 0; b < s; ++b)
            for (u32 k = 0; k < (1u << s); ++k)
              if (k >> b & 1) f[k] -= f[k ^ (1u << b)];
          for (u32 k = 1; k + 1 < (1u << s); k += 2) dp[tm[k]] += w * f[k];  // Genie を含み、S 自身でない T
        }
      }
      if (S == 1) break;
    }
    ans = double(total);
  }
  double answer() const { return ans; }
};
