#pragma once
// nos_sort と nos_radix が共有する DP。prev[i] は i より前で a[i] と同じ値が最後に出た位置 (無ければ -1)。空も含めた相異なる部分列の
// 数を dp[i] (a[0, i) のもの) とすると、dp[i + 1] = 2 dp[i] - dp[prev[i]] (prev[i] が無ければ引かない) で、答えは dp[n] - 1。
// 異なる部分列の数え上げの実装を比べた記録は algo-notes の notes/number-of-subsequences.md にある。
#include <vector>

namespace nos_dp {
constexpr unsigned P = 998244353;
inline unsigned count(const std::vector<int> &prev) {
  const int n = int(prev.size());
  std::vector<unsigned> dp(n + 1);
  dp[0] = 1;
  for (int i = 0; i < n; ++i) {
    unsigned x = dp[i] * 2;
    x -= x >= P ? P : 0;
    if (prev[i] >= 0) {
      x += P - dp[prev[i]];
      x -= x >= P ? P : 0;
    }
    dp[i + 1] = x;
  }
  return dp[n] == 0 ? P - 1 : dp[n] - 1;
}
}  // namespace nos_dp
