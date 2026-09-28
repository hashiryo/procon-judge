// abc240-h の愚直解。dp[l][r] を「最後が S[l..r] の列の長さの最大」として、S[l..r] より前で終わり、
// 辞書順で小さい部分文字列 S[l'..r'] (r' < l) を全部試す。文字列は std::string_view でそのまま比べる。
// 接尾辞配列も、部分文字列の長さを B で打ち切ることもしないので、提出とは別の考え方になる。
// O(N^4) 回の比較なので、N が 80 までのときだけ使う。
#include <algorithm>
#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

int main() {
  int n;
  char buf[128];
  if (scanf("%d %127s", &n, buf) != 2) return 1;
  std::string s = buf;
  std::string_view v = s;
  // dp[l][r] (0 <= l <= r < n)
  std::vector<std::vector<int>> dp(n, std::vector<int>(n, 0));
  int best = 0;
  for (int l = 0; l < n; ++l)
    for (int r = l; r < n; ++r) {
      std::string_view cur = v.substr(l, r - l + 1);
      int d = 1;
      for (int l2 = 0; l2 < l; ++l2)
        for (int r2 = l2; r2 < l; ++r2)
          if (v.substr(l2, r2 - l2 + 1) < cur) d = std::max(d, dp[l2][r2] + 1);
      dp[l][r] = d;
      best = std::max(best, d);
    }
  printf("%d\n", best);
}
