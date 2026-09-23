#include <iostream>
#include <algorithm>
#include <numeric>
#include "mylib/data_structure/UnionFind.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int N, M;
 cin >> N >> M;
 UnionFind uf(N);
 int s[M], t[M];
 long long w[M];
 for(int i= 0; i < M; i++) cin >> s[i] >> t[i] >> w[i];
 long long ans= 0;
 int ord[M];
 iota(ord, ord + M, 0), sort(ord, ord + M, [&](int l, int r) { return w[l] < w[r]; });
 for(int i: ord)
  if(uf.unite(s[i], t[i])) ans+= w[i];
 cout << ans << '\n';
 return 0;
}
