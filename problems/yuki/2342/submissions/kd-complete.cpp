// 双対の verify。点 (HLD の位置, 深さ) に affine の作用を積み、点の値を読む KD 木
// (problems/_shared/kd-tree/kd_dual_2d.hpp) で解く。lib.cpp (今の Library の KDTree の双対) と書き比べる。
#include <iostream>
#include <tuple>
#include <vector>
#include "mylib/algebra/ModInt.hpp"
#include "mylib/graph/Graph.hpp"
#include "mylib/graph/HeavyLightDecomposition.hpp"
#include "_shared/kd-tree/kd_dual_2d.hpp"
using namespace std;
using Mint = ModInt<998244353>;

signed main() {
  cin.tie(0);
  ios::sync_with_stdio(0);
  int N, Q;
  cin >> N >> Q;
  Graph g(N, N - 1);
  for (int i = 0; i < N - 1; ++i) cin >> g[i], --g[i];
  HeavyLightDecomposition tree(g, 0);
  vector<int> sx(N), sy(N);
  vector<unsigned> init(N);
  for (int v = 0; v < N; ++v) {
    Mint X;
    cin >> X;
    sx[v] = tree.to_seq(v);
    sy[v] = tree.depth(v);
    init[v] = (unsigned)X.val();
  }
  KdDual2D<KdRingMod> kd(sx, sy, init);
  while (Q--) {
    int t;
    cin >> t;
    if (t == 1) {
      int V;
      cin >> V, --V;
      cout << kd.get(V) << '\n';
    } else if (t == 2) {
      int V, K, C, D;
      cin >> V >> K >> C >> D, --V;
      for (int i = 0; i <= K; ++i) {
        const int p = tree.parent(V);
        auto [l, r] = tree.subtree(V);
        const int d = tree.depth(V);
        if (p == -1) {
          kd.apply(l, d, r, d + K - i + 1, (unsigned)C, (unsigned)D);
          break;
        }
        kd.apply(l, d + K - i - 1, r, d + K - i + 1, (unsigned)C, (unsigned)D);
        V = p;
      }
    } else if (t == 3) {
      int V, C, D;
      cin >> V >> C >> D, --V;
      auto [l, r] = tree.subtree(V);
      kd.apply(l, 0, r, N + 1, (unsigned)C, (unsigned)D);
    } else {
      int U, V, C, D;
      cin >> U >> V >> C >> D, --U, --V;
      for (auto [a, b] : tree.path(U, V)) {
        if (a > b) swap(a, b);
        kd.apply(a, 0, b + 1, N + 1, (unsigned)C, (unsigned)D);
      }
    }
  }
  return 0;
}
