#include <iostream>
#include "mylib/number_theory/is_prime.hpp"

using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int n;
 cin >> n;
 while(n--) {
  long long x;
  cin >> x;
  cout << x << " " << is_prime(x) << '\n';
 }
 return 0;
}
