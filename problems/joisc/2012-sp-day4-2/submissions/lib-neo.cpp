// 融合永続, split, split3
// NeoLibrary の neo/data_structure/WeightBalancedTree.hpp (永続の形) で書いたもの。置き場が伸びたら gc で今の列から届く節だけを詰める。
#include <iostream>
#include <string>
#include <tuple>
#include "neo/data_structure/WeightBalancedTree.hpp"
using namespace std;
signed main() {
  cin.tie(0);
  ios::sync_with_stdio(false);
  using P = WeightBalancedTree<char, false, true>;
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
