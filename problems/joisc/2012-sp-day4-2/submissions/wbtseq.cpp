// 融合永続, split, split3
// 今の Library の WeightBalancedTree の中身を写し、伸びる置き場と gc を足した列の木の試作 (problems/_shared/seqtree/wbt_seq.hpp)
// で書いたもの。置き場が伸びたら gc で生きている版から届く節だけを詰める。
#include <iostream>
#include <string>
#include <tuple>
#include "_shared/seqtree/wbt_seq.hpp"
using namespace std;
signed main() {
  cin.tie(0);
  ios::sync_with_stdio(false);
  using P = WbtSeq<char>;
  int M;
  cin >> M;
  string str;
  cin >> str;
  P s(str.begin(), str.end());
  int N;
  cin >> N;
  while (N--) {
    int a, b, c;
    cin >> a >> b >> c;
    auto [x, y, z] = s.split3(a, b);
    tie(x, z) = s.split(c);
    s = x + y + z;
    if (s.size() > M) s = s.split(M).first;
    if (P::pool_empty()) P::gc(s);
  }
  const auto ans = s.dump();
  cout << string(ans.begin(), ans.end()) << '\n';
  return 0;
}
