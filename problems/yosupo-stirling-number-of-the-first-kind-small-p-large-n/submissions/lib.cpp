#include <iostream>
#include "mylib/number_theory/StirlingNumber.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int T, p;
 cin >> T >> p;
 StirlingNumber SN(p, 1, 0);
 while(T--) {
  long long n, k;
  cin >> n >> k;
  cout << SN.S1(n, k) << '\n';
 }
 return 0;
}
