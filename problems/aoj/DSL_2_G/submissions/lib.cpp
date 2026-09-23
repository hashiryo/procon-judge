#include <iostream>
#include "mylib/data_structure/BinaryIndexedTree_RangeAdd.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N, Q;
 cin >> N >> Q;
 BinaryIndexedTree_RangeAdd<long long> bit(N);
 while(Q--) {
  int op, s, t;
  cin >> op >> s >> t;
  --s;
  if(op) cout << bit.sum(s, t) << '\n';
  else {
   long long x;
   cin >> x;
   bit.add_range(s, t, x);
  }
 }
}
