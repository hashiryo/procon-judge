// 今の Library の bipartite_matching で解く。飛行機を左、B の点を右に置き、時刻 T の位置が B の点に当たる向きごとに辺を
// 張る。B の点は座標で整列して二分探索で引く。答えの向きは、使った辺の番号から引く。
#include <algorithm>
#include <cstdio>
#include <utility>
#include <vector>
#include "mylib/graph/BipartiteGraph.hpp"
int main() {
 constexpr int DX[8]= {1, 1, 0, -1, -1, -1, 0, 1}, DY[8]= {0, 1, 1, 1, 0, -1, -1, -1};
 int n;
 long long t;
 if(scanf("%d %lld", &n, &t) != 2) return 1;
 std::vector<std::pair<long long, long long>> a(n), b(n);
 for(auto& [x, y]: a)
  if(scanf("%lld %lld", &x, &y) != 2) return 1;
 for(auto& [x, y]: b)
  if(scanf("%lld %lld", &x, &y) != 2) return 1;
 std::vector<int> ord(n);
 for(int j= 0; j < n; ++j) ord[j]= j;
 std::sort(ord.begin(), ord.end(), [&](int i, int j) { return b[i] < b[j]; });
 std::vector<std::pair<long long, long long>> sb(n);
 for(int k= 0; k < n; ++k) sb[k]= b[ord[k]];
 BipartiteGraph bg(n, n);
 std::vector<int> dir;
 for(int i= 0; i < n; ++i)
  for(int d= 0; d < 8; ++d) {
   std::pair<long long, long long> q{a[i].first + t * DX[d], a[i].second + t * DY[d]};
   if(auto p= std::lower_bound(sb.begin(), sb.end(), q); p != sb.end() && *p == q) bg.add_edge(i, n + ord[p - sb.begin()]), dir.push_back(d);
  }
 auto [mc, mate]= bipartite_matching(bg);
 if((int)mc.size() < n) return puts("No"), 0;
 std::vector<int> ans(n);
 for(int e: mc) ans[bg[e].first]= dir[e] + 1;
 puts("Yes");
 for(int i= 0; i < n; ++i) printf("%d%c", ans[i], i + 1 == n ? '\n' : ' ');
}
