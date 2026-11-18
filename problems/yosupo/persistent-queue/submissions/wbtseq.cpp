// 今の Library の WeightBalancedTree の中身を写し、伸びる置き場と gc を足した列の木の試作 (problems/_shared/seqtree/wbt_seq.hpp)
// で書いたもの。置き場が伸びたら gc で生きている版から届く節だけを詰める。
#include <iostream>
#include <vector>
#include "_shared/seqtree/wbt_seq.hpp"
using namespace std;
signed main() {
  cin.tie(0);
  ios::sync_with_stdio(0);
  int Q;
  cin >> Q;
  using P = WbtSeq<int>;
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
