// abc268-h の愚直解。S の中の T_i の出現を全部、素直に 1 文字ずつ比べて区間 [l, r] として探し、どの区間にも
// * が 1 つは入るような * の最少個数を、* を置く位置を左から決める DP で求める。dp[j] は、j に * を置き、
// j より左で終わる区間を全部潰したときの最少個数。直前の * が i なら、i と j の間に丸ごと収まる区間が
// 無いことが条件になる。Aho-Corasick も、一致したら状態を戻す貪欲も使わないので、提出とは別の考え方になる。
// O(|S| Σ|T_i| + |S|^2) なので、小さい入力でだけ使う。
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

int main() {
  std::string s;
  int n;
  if (!(std::cin >> s >> n)) return 1;
  std::vector<std::string> t(n);
  for (auto& x : t)
    if (!(std::cin >> x)) return 1;
  const int len = s.size(), INF = 1 << 30;
  // first_end[l]: 始まりが l 以上の区間の終わりの最小。
  std::vector<int> first_end(len + 1, INF);
  for (int l = 0; l < len; ++l)
    for (const auto& x : t)
      if (l + (int)x.size() <= len && s.compare(l, x.size(), x) == 0)
        first_end[l] = std::min(first_end[l], l + (int)x.size() - 1);
  for (int l = len - 1; l >= 0; --l) first_end[l] = std::min(first_end[l], first_end[l + 1]);
  // dp[i + 1] が位置 i に * を置いたとき。dp[0] は「まだ置いていない」(i = -1)。
  std::vector<int> dp(len + 1, INF);
  dp[0] = 0;
  for (int j = 0; j < len; ++j)
    for (int i = -1; i < j; ++i)
      if (dp[i + 1] < INF && first_end[i + 1] >= j) dp[j + 1] = std::min(dp[j + 1], dp[i + 1] + 1);
  int ans = INF;
  for (int i = -1; i < len; ++i)
    if (dp[i + 1] < INF && first_end[i + 1] >= len) ans = std::min(ans, dp[i + 1]);
  printf("%d\n", ans);
}
