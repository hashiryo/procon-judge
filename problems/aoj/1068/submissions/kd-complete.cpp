// 長方形の最小。neo の KdTree (neo/base/ds/kd/KdTree.hpp、1 点 1 ノード) で解く。
// lib.cpp (今の Library の KDTree) と書き比べる。範囲は両端を含む。
#include <algorithm>
#include <array>
#include <iostream>
#include <vector>
#include "neo/base/ds/kd/KdTree.hpp"
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
    vector<array<int, 2>> pt(r * c);
    vector<int> v(r * c);
    for (int i = 0; i < r; i++)
      for (int j = 0; j < c; j++) {
        const int k = i * c + j;
        pt[k] = {i, j};
        cin >> v[k];
      }
    KdTree<int, 2, RminQ> kd(pt, v);
    while (q--) {
      int r1, c1, r2, c2;
      cin >> r1 >> c1 >> r2 >> c2;
      cout << kd.prod({r1, c1}, {r2 + 1, c2 + 1}) << '\n';
    }
  }
  return 0;
}
