#include <iostream>
#include "mylib/data_structure/UnionFind.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int n, q;
 cin >> n >> q;
 UnionFind uf(n);
 for(int i= 0; i < q; i++) {
  int c, x, y;
  cin >> c >> x >> y;
  if(c) cout << uf.connected(x, y) << "\n";
  else uf.unite(x, y);
 }
 return 0;
}
