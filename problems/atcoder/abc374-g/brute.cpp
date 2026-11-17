// atcoder-abc374-g の愚直解。品名を頂点、続けて書ける組を辺にしたグラフで、(今いる品名, ここまでに通った品名の集合) を
// 状態にした BFS で、1 本の文字列で覆える品名の集合を全部出す。そのうえで、集合をいくつ合わせれば全部の品名を覆えるかを
// 部分集合の DP で数える。強連結成分も Dilworth の定理も使わないので、提出とは別の考え方になる。品名が 10 個以下の入力で
// だけ使う (gen.py の seed 1000 以上)。
#include <algorithm>
#include <array>
#include <cstdio>
#include <vector>
int main() {
 int n;
 if(scanf("%d", &n) != 1) return 1;
 std::vector<std::array<char, 3>> s(n);
 for(auto& x: s)
  if(scanf("%2s", x.data()) != 1) return 1;
 if(n > 12) return 1;
 const int F= 1 << n;
 std::vector<char> seen((size_t)n * F, 0), cover(F, 0);
 std::vector<std::pair<int, int>> q;
 for(int v= 0; v < n; ++v) seen[(size_t)v * F + (1 << v)]= 1, q.push_back({v, 1 << v});
 for(size_t i= 0; i < q.size(); ++i) {
  auto [v, m]= q[i];
  cover[m]= 1;
  for(int w= 0; w < n; ++w)
   if(s[v][1] == s[w][0]) {
    const int t= m | 1 << w;
    if(!seen[(size_t)w * F + t]) seen[(size_t)w * F + t]= 1, q.push_back({w, t});
   }
 }
 std::vector<int> sets;
 for(int m= 1; m < F; ++m)
  if(cover[m]) sets.push_back(m);
 std::vector<int> dp(F, 1 << 29);
 dp[0]= 0;
 for(int m= 0; m < F; ++m)
  if(dp[m] < (1 << 29))
   for(int c: sets) dp[m | c]= std::min(dp[m | c], dp[m] + 1);
 printf("%d\n", dp[F - 1]);
}
