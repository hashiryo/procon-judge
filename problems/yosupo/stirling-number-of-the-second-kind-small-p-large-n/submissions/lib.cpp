#include <iostream>
#include "mylib/number_theory/StirlingNumber.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int T, p;
 cin >> T >> p;
 StirlingNumber SN(p, 0, 1);
 while(T--) {
  long long n, k;
  cin >> n >> k;
  cout << SN.S2(n, k) << '\n';
 }
 return 0;
}
