// 長方形の最小。点を葉に置く完全 2 分木の KD 木 (problems/_shared/kd-tree/kd_tree_2d.hpp) で解く。
// lib.cpp (今の Library の KDTree) と書き比べる。範囲は両端を含む。
#include <algorithm>
#include <array>
#include <iostream>
#include <vector>
#include "_shared/kd-tree/kd_tree_2d.hpp"
using namespace std;

struct RminQ {
  using T = int;
  static T ti() { return 0x7fffffff; }
  static T op(T l, T r) { return min(l, r); }
};

signed main() {
  cin.tie(0);
  ios::sync_with_stdio(0);
  for (int r, c, q; cin >> r >> c >> q && r;) {
    vector<int> x(r * c), y(r * c), v(r * c);
    for (int i = 0; i < r; i++)
      for (int j = 0; j < c; j++) {
        const int k = i * c + j;
        x[k] = i;
        y[k] = j;
        cin >> v[k];
      }
    KdTree2D<RminQ> kdt(x, y, v);
    while (q--) {
      int r1, c1, r2, c2;
      cin >> r1 >> c1 >> r2 >> c2;
      cout << kdt.prod(r1, c1, r2 + 1, c2 + 1) << '\n';
    }
  }
  return 0;
}
