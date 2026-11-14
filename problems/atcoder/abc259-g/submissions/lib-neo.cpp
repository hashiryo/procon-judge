// NeoLibrary の BinaryOptimization (neo/combinatorial_optimization/BinaryOptimization.hpp) で解く。行 r を選ぶなら x_r = 1、
// 列 c を選ぶなら y_c = 1 とし、得点の符号を反転した費用を最小にする。1 変数の項は行と列の和、2 変数の項は両方で選んだ
// マスの差し引き A_{r,c} (負なら禁止の大きな費用)。この項は劣モジュラでないが、行と列が二部に分かれているので、
// BinaryOptimization が列の側を反転して直す。
#include <iostream>
#include <vector>
#include "neo/combinatorial_optimization/BinaryOptimization.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int h, w;
 cin >> h >> w;
 vector<vector<long long>> a(h, vector<long long>(w));
 vector<long long> rs(h), cs(w);
 for(int r= 0; r < h; ++r)
  for(int c= 0; c < w; ++c) cin >> a[r][c], rs[r]+= a[r][c], cs[c]+= a[r][c];
 const long long INF= 1e18;
 BinaryOptimization<long long> bo(h + w);
 for(int r= 0; r < h; ++r) bo.add(r, 0, -rs[r]);
 for(int c= 0; c < w; ++c) bo.add(h + c, 0, -cs[c]);
 for(int r= 0; r < h; ++r)
  for(int c= 0; c < w; ++c) bo.add(r, h + c, 0, 0, 0, a[r][c] >= 0 ? a[r][c] : INF);
 cout << -*bo.solve() << '\n';
}
