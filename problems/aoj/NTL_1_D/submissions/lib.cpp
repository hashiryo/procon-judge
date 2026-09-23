#include <iostream>
#include "mylib/number_theory/Factors.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 long long n;
 cin >> n;
 cout << totient(n) << '\n';
 return 0;
}
