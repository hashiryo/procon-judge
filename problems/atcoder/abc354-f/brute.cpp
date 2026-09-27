// abc354-f の愚直解。f[i] = A_i で終わる増加部分列の最長、g[i] = A_i から始まる増加部分列の最長を
// O(N^2) の DP で求め、f[i] + g[i] - 1 が LIS の長さに等しい i を出す。
// 二分探索で長さごとの末尾を持つやり方 (longest_increasing_subsequence) をしないので、提出とは別の考え方になる。
#include <algorithm>
#include <cstdio>
#include <vector>

int main() {
  int t;
  if (scanf("%d", &t) != 1) return 1;
  while (t--) {
    int n;
    if (scanf("%d", &n) != 1) return 1;
    std::vector<int> a(n), f(n, 1), g(n, 1);
    for (int& x : a)
      if (scanf("%d", &x) != 1) return 1;
    for (int i = 0; i < n; ++i)
      for (int j = 0; j < i; ++j)
        if (a[j] < a[i]) f[i] = std::max(f[i], f[j] + 1);
    for (int i = n; i--;)
      for (int j = i + 1; j < n; ++j)
        if (a[j] > a[i]) g[i] = std::max(g[i], g[j] + 1);
    int len = *std::max_element(f.begin(), f.end());
    std::vector<int> ans;
    for (int i = 0; i < n; ++i)
      if (f[i] + g[i] - 1 == len) ans.push_back(i + 1);
    printf("%d\n", (int)ans.size());
    for (size_t i = 0; i < ans.size(); ++i) printf("%d%c", ans[i], i + 1 == ans.size() ? '\n' : ' ');
  }
}
