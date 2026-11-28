// 3 辺すべてが真に小さい別の箱があるかの判定。3 次元 KD 木 (problems/_shared/kd-tree/kd_tree_3d.hpp) で、
// 直方体 [0, h) x [0, w) x [0, d) に点があるかを見る。lib-kdt-3d.cpp (今の Library の 3 次元 KDTree) と書き比べる。
#include <algorithm>
#include <array>
#include <iostream>
#include <vector>
#include "_shared/kd-tree/kd_tree_3d.hpp"
using namespace std;

struct ROrQ {
  using T = bool;
  static T ti() { return false; }
  static T op(T l, T r) { return l || r; }
};

signed main() {
  cin.tie(0);
  ios::sync_with_stdio(0);
  int N;
  cin >> N;
  vector<int> x(N), y(N), z(N);
  vector<bool> one(N, true);
  vector<array<int, 3>> boxes(N);
  for (int i = 0; i < N; ++i) {
    int a[3];
    cin >> a[0] >> a[1] >> a[2];
    sort(a, a + 3);
    boxes[i] = {a[0], a[1], a[2]};
    x[i] = a[0];
    y[i] = a[1];
    z[i] = a[2];
  }
  KdTree3D<ROrQ> kd(x, y, z, one);
  for (auto [h, w, d] : boxes) {
    // 3 辺すべてが真に小さい箱を探すので、範囲は [0, h) などの半開でよい。
    if (kd.prod(0, 0, 0, h, w, d)) {
      cout << "Yes\n";
      return 0;
    }
  }
  cout << "No\n";
  return 0;
}
