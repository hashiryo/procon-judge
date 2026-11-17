// 診断用。lib-neo と同じ辺で NeoLibrary の BipartiteMatching を組み、大きさだけ使う (道を組まない)。lib-neo との差が
// min_path_cover の道を組む分になる。関係は推移的なので、最大反鎖の大きさ w は 2n - (マッチングの大きさ)。比べ終えたら消す。
#include <array>
#include <cstdio>
#include <string>
#include <vector>
#include "common.hpp"
#include "neo/combinatorial_optimization/BipartiteMatching.hpp"
int main() {
 int t;
 if(scanf("%d", &t) != 1) return 1;
 std::vector<int> pos(50001, -1);
 std::string out;
 while(t--) {
  int n;
  if(scanf("%d", &n) != 1) return 1;
  std::vector<int> a(n);
  for(int& x: a)
   if(scanf("%d", &x) != 1) return 1;
  std::vector<std::array<int, 2>> es;
  cf1630f_edges(a, pos, [&](int u, int v) { es.push_back({u, v}); });
  const int w= 2 * n - BipartiteMatching(2 * n, 2 * n, es).size();
  out+= std::to_string(n - w), out+= '\n';
 }
 fputs(out.c_str(), stdout);
}
