// NeoLibrary の euler_split (neo/graph/euler_split.hpp) で解く。x 座標を左、y 座標を右の頂点にして、点ごとに辺を張る。二部グラフ
// なので、Euler 分割でどの頂点でも 2 色の辺の数の差が 1 以下になり、どの縦の線と横の線でも r と b の数の差が 1 以下になる。
#include <array>
#include <cstdio>
#include <string>
#include <vector>
#include "neo/graph/euler_split.hpp"
int main() {
 constexpr int C= 200000;
 int n;
 if(scanf("%d", &n) != 1) return 1;
 std::vector<std::array<int, 2>> es(n);
 for(auto& [x, y]: es) {
  if(scanf("%d %d", &x, &y) != 2) return 1;
  --x, y+= C - 1;
 }
 const std::vector<int> col= euler_split(2 * C, es);
 std::string out(n, 'r');
 for(int i= 0; i < n; ++i)
  if(col[i]) out[i]= 'b';
 out+= '\n';
 fwrite(out.data(), 1, out.size(), stdout);
}
