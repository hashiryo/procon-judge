// _shared/flow/proto_bipartite.hpp の試作の API で書いたもの。辺の張り方は lib と同じで、答えの向きは matching() が返す
// 辺の番号から引く。
#include <algorithm>
#include <array>
#include <cstdio>
#include <utility>
#include <vector>
#include "_shared/flow/proto_bipartite.hpp"
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
 std::vector<std::array<int, 2>> es;
 std::vector<int> dir;
 for(int i= 0; i < n; ++i)
  for(int d= 0; d < 8; ++d) {
   std::pair<long long, long long> q{a[i].first + t * DX[d], a[i].second + t * DY[d]};
   if(auto p= std::lower_bound(sb.begin(), sb.end(), q); p != sb.end() && *p == q) es.push_back({i, ord[p - sb.begin()]}), dir.push_back(d);
  }
 proto::BipartiteMatching bm(n, n, es);
 if(bm.size() < n) return puts("No"), 0;
 std::vector<int> ans(n);
 for(int i: bm.matching()) ans[es[i][0]]= dir[i] + 1;
 puts("Yes");
 for(int i= 0; i < n; ++i) printf("%d%c", ans[i], i + 1 == n ? '\n' : ' ');
}
