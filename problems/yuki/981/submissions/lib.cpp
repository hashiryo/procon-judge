#include <iostream>
#include "mylib/number_theory/mod_kth_root.hpp"
using namespace std;
int main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int T;
 cin >> T;
 while(T--) {
  long long p, a;
  unsigned long long k;
  cin >> p >> k >> a;
  cout << mod_kth_root(a, k, p) << '\n';
 }
 return 0;
}
