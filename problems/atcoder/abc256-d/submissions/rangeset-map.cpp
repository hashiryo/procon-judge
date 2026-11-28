// RangeSet を std::map で持ち直した候補 (problems/_shared/rangeset/rangeset.hpp) で解く。
// 区間の列挙 (begin/end) を使う。lib.cpp (今の Library の RangeSet) と書き比べる。
#include <iostream>
#include "_shared/rangeset/rangeset.hpp"
using namespace std;
signed main() {
  cin.tie(0);
  ios::sync_with_stdio(0);
  int N;
  cin >> N;
  RangeSet<int> rs;
  for (int i = 0; i < N; ++i) {
    int L, R;
    cin >> L >> R;
    rs.insert(L, R - 1);
  }
  for (auto [l, r] : rs) cout << l << " " << r + 1 << '\n';
  return 0;
}
