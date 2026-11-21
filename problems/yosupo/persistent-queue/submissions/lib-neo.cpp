// NeoLibrary の neo/base/ds/seq/WeightBalancedTree.hpp (永続の形) で書いたもの。版を vector に持ち、置き場が伸びたら gc で詰める
// (この問題は版を全部残すので、詰めても減らない)。
#include <iostream>
#include <vector>
#include "neo/base/ds/seq/WeightBalancedTree.hpp"
using namespace std;
signed main() {
  cin.tie(0);
  ios::sync_with_stdio(0);
  int Q;
  cin >> Q;
  using P = WeightBalancedTree<int, false, true>;
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
    if (P::pool_empty()) P::gc(S);
  }
  return 0;
}
