#include <iostream>
#include "mylib/data_structure/SegmentTree.hpp"
using namespace std;
struct RminQ {
 using T= int;
 static T ti() { return 0x7fffffff; }
 static T op(T l, T r) { return min(l, r); }
};
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int n, q;
 cin >> n >> q;
 SegmentTree<RminQ> seg(n);
 while(q--) {
  int com, x, y;
  cin >> com >> x >> y;
  if(com) {
   cout << seg.prod(x, y + 1) << '\n';
  } else {
   seg.set(x, y);
  }
 }
 return 0;
}
