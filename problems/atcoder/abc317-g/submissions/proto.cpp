// _shared/flow/proto_bipartite.hpp の試作の API で書いたもの。行を左、数を右に置いてマスごとに辺を張り (多重辺になる)、
// 完全マッチングを取っては使った辺を除くことを M 回繰り返す。M 正則の二部多重グラフから完全マッチングを除いても
// 正則のままなので、毎回完全マッチングが取れる。
#include <array>
#include <cstdio>
#include <vector>
#include "_shared/flow/proto_bipartite.hpp"
int main() {
 int n, m;
 if(scanf("%d %d", &n, &m) != 2) return 1;
 std::vector<std::array<int, 2>> es(n * m);
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < m; ++j) {
   int a;
   if(scanf("%d", &a) != 1) return 1;
   es[i * m + j]= {i, a - 1};
  }
 std::vector<std::vector<int>> b(n, std::vector<int>(m));
 for(int c= 0; c < m; ++c) {
  proto::BipartiteMatching bm(n, n, es);
  if(bm.size() < n) return puts("No"), 0;  // 正則なので起きない
  std::vector<char> used(es.size());
  for(int i: bm.matching()) b[es[i][0]][c]= es[i][1] + 1, used[i]= 1;
  std::vector<std::array<int, 2>> rest;
  rest.reserve(es.size() - n);
  for(size_t i= 0; i < es.size(); ++i)
   if(!used[i]) rest.push_back(es[i]);
  es.swap(rest);
 }
 puts("Yes");
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < m; ++j) printf("%d%c", b[i][j], j + 1 == m ? '\n' : ' ');
}
