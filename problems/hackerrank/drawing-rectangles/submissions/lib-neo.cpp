// NeoLibrary の BipartiteMatching (neo/graph/BipartiteMatching.hpp) で書いたもの。列 x を左、行 y を右に置き、塗られたマスを辺にする。
// 最小点被覆は min_vertex_cover() で、左 (列) の頂点が [0, N)、右 (行) の頂点が [N, 2N) の番号で番号の順に返る。
#include <algorithm>
#include <array>
#include <cstdio>
#include <vector>
#include "neo/graph/BipartiteMatching.hpp"
int main() {
 constexpr int N= 300'001;
 int n;
 if(scanf("%d", &n) != 1) return 1;
 // 列ごとに、塗られた行の区間を集めて合わせる。
 std::vector<std::vector<std::array<int, 2>>> ys(N);
 for(int i= 0; i < n; ++i) {
  int x1, y1, x2, y2;
  if(scanf("%d %d %d %d", &x1, &y1, &x2, &y2) != 4) return 1;
  for(int x= x1; x <= x2; ++x) ys[x].push_back({y1, y2});
 }
 std::vector<std::array<int, 2>> es;
 for(int x= 0; x < N; ++x) {
  auto& v= ys[x];
  std::sort(v.begin(), v.end());
  int hi= -1;
  for(auto [a, b]: v) {
   for(int y= std::max(a, hi + 1); y <= b; ++y) es.push_back({x, y});
   hi= std::max(hi, b);
  }
 }
 BipartiteMatching bm(N, N, es);
 std::vector<int> l, r;
 for(int v: bm.min_vertex_cover()) (v < N ? l : r).push_back(v < N ? v : v - N);
 std::printf("%d\n%d\n", (int)(l.size() + r.size()), (int)l.size());
 for(size_t i= 0; i < l.size(); ++i) std::printf(i ? " %d" : "%d", l[i]);
 std::printf("\n%d\n", (int)r.size());
 for(size_t i= 0; i < r.size(); ++i) std::printf(i ? " %d" : "%d", r[i]);
 std::printf("\n");
}
