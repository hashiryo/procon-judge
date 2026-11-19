// NeoLibrary の neo/data_structure/WeightBalancedTree.hpp (永続の形) で、説明のページの使い方に合わせて書いたもの。この問題は版を
// 全部残すので gc を呼ばず (詰めても減らない)、節の数の上限が見積もれるので reserve で置き場を先に取る (伸ばすときの写しを省く)。
// 今の Library の lib.cpp が大きさを決めた置き場を使うのと同じ形。gc を呼ぶ形は lib-neo.cpp。
#include <iostream>
#include <vector>
#include "neo/data_structure/WeightBalancedTree.hpp"
using namespace std;
signed main() {
  cin.tie(0);
  ios::sync_with_stdio(0);
  int Q;
  cin >> Q;
  using P = WeightBalancedTree<int, false, true>;
  P::reserve(1 << 24);
  vector<P> S(Q + 1);
  for (int i = 1; i <= Q; ++i) {
    int op, t;
    cin >> op >> t;
    S[i] = S[++t];
    if (op) {
      cout << S[i].pop_front() << '\n';
    } else {
      int x;
      cin >> x;
      S[i].push_back(x);
    }
  }
  return 0;
}
