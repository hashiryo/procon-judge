// RangeSet を std::map で持ち直した候補 (problems/_shared/rangeset/rangeset.hpp) で解く。
// 隣接する区間は併合しない (merge = false)。lib.cpp (今の Library の RangeSet) と書き比べる。
#include <algorithm>
#include <array>
#include <iostream>
#include "_shared/rangeset/rangeset.hpp"
using namespace std;

signed main() {
  cin.tie(0);
  ios::sync_with_stdio(0);
  int N, M, Q;
  cin >> N >> M >> Q;
  array<int, 5> query[M + Q];
  for (int i = 0; i < M; ++i) {
    int D, A, B;
    cin >> D >> A >> B;
    query[i] = {D, 1, A, B, -1};
  }
  for (int i = 0; i < Q; ++i) {
    int E, S, T;
    cin >> E >> S >> T;
    query[i + M] = {E, 0, S, T, i};
  }
  sort(query, query + M + Q);
  bool ans[Q];
  RangeSet<int, false> rs;
  for (auto [d, t, a, b, i] : query) {
    if (t)
      rs.insert(a, b);
    else {
      if (a >= b)
        ans[i] = true;
      else
        ans[i] = rs.covered_by(a, b);
    }
  }
  for (int i = 0; i < Q; ++i) cout << (ans[i] ? "Yes" : "No") << '\n';
  return 0;
}
