#include <iostream>
#include "mylib/misc/Period.hpp"
#include "mylib/data_structure/BinaryIndexedTree_RangeAdd.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N;
 cin >> N;
 vector<int> A(N);
 for(int i= 0; i < N; i++) cin >> A[i], --A[i];
 Period p(A);
 BinaryIndexedTree_RangeAdd<int> bit(N);
 for(int i= 0; i < N; ++i) {
  cout << bit[p(i)] << '\n';
  for(auto [l, r]: p.path_upto_cycle(i)) bit.add_range(l, r, 1);
 }
 return 0;
}
