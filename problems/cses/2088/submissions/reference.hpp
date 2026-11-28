#pragma once
#include "pj.hpp"

// 想定解。dp[l][r] = min_{l <= k < r} dp[l][k] + dp[k+1][r] + (S[r+1] - S[l])、dp[l][l] = 0。
// 費用が区間和なので quadrangle inequality が等号で成り立ち、最適な k は
// opt[l][r-1] <= opt[l][r] <= opt[l+1][r] を満たす。各長さの層で opt の範囲を絞る Knuth の最適化で O(n^2)。
// dp は三角配列 (n(n+1)/2 個) に置き、opt は直前の長さの層だけ持つ。
// 期待出力を作る側なので、Library を include しない。
struct Solver {
  int n;
  vector<i64> pre;  // pre[i] = 先頭 i 個の和
  vector<int> off;  // off[l] は dp の行 l の先頭。dp[l][r] は off[l] + (r - l)
  vector<i64> dp;
  i64 ans = 0;

  explicit Solver(const vector<i64> &x) : n((int)x.size()), pre(n + 1), off(n + 1) {
    for (int i = 0; i < n; ++i) pre[i + 1] = pre[i] + x[i];
    for (int l = 0; l < n; ++l) off[l + 1] = off[l] + (n - l);
    dp.assign((size_t)off[n], 0);
  }

  void run() {
    if (n <= 1) {
      ans = 0;
      return;
    }
    vector<int> opt(n), nopt(n);
    for (int l = 0; l < n; ++l) opt[l] = l;
    for (int len = 2; len <= n; ++len) {
      for (int l = 0, r = len - 1; r < n; ++l, ++r) {
        const int lo = opt[l], hi = min(opt[l + 1], r - 1);
        const int base_l = off[l];
        int bk = lo;
        i64 bv = dp[base_l + (lo - l)] + dp[off[lo + 1] + (r - lo - 1)];
        for (int k = lo + 1; k <= hi; ++k) {
          const i64 v = dp[base_l + (k - l)] + dp[off[k + 1] + (r - k - 1)];
          if (v < bv) bv = v, bk = k;
        }
        dp[base_l + (r - l)] = bv + (pre[r + 1] - pre[l]);
        nopt[l] = bk;
      }
      swap(opt, nopt);
    }
    ans = dp[n - 1];
  }

  i64 answer() const { return ans; }
};
