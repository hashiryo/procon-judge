#include <cstddef>
#include <iostream>
#include <vector>
#include "neo/base/ds/set/PersistentUnionFind.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N, Q;
 cin >> N >> Q;
 // 初期の配列 (N / 63 節ほど) と、union 1 回あたり 2 回の set (M = 64 で 3 段) の分。
 PersistentUnionFind::reserve(std::size_t(N) / 32 + std::size_t(Q) * 8 + 1);
 vector<PersistentUnionFind> uf(Q + 1);
 uf[0] = PersistentUnionFind(N);
 for(int i= 1; i <= Q; ++i) {
  int t, k, u, v;
  cin >> t >> k >> u >> v;
  ++k;
  if(t) cout << uf[k].same(u, v) << '\n';
  else uf[i] = uf[k].unite(u, v);
 }
}
